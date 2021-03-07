//! We could probably use macros here to make things fancy but for now we'll
//! just manully copy our tests to run into the run_tests() function.
// use alloc::{Vec,vec};
use alloc::boxed::Box;
use alloc::vec::Vec;

use crate::cpu::Port;
use crate::drivers::ata;
use crate::mem;
use crate::fs;
use crate::string;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[repr(u8)]
enum QemuExitCode {
    Success = 0x10,
    Failed = 0x11,
}

fn exit_qemu(exit_code: QemuExitCode) {
    let mut port = Port::new(0xf4);
    port.write(exit_code as u8);
}

trait Testable {
    fn run(&self) -> ();
}

impl<T> Testable for T
where
    T: Fn(),
{
    fn run(&self) {
        serial_print!("{}...\t", core::any::type_name::<T>());
        self();
        serial_println!("[ok]");
    }
}

#[panic_handler]
fn panic(info: &::core::panic::PanicInfo) -> ! {
    serial_println!("[failed]\n");
    serial_println!("Error: {}\n", info);
    exit_qemu(QemuExitCode::Failed);
    loop {}
}

pub fn run_tests() {
    let tests: Vec<Box<dyn Testable>> = vec![
        // Box::new(&test_ata_read_hdd),
        Box::new(&test_read_invalid_hdd),
        Box::new(&test_bsf),
        Box::new(&test_allocate_kernel_page),
        Box::new(&test_read_ustar),
    ];

    for test in &tests {
        test.run();
    }

    exit_qemu(QemuExitCode::Success);
}

use crate::mem::physical::first_free_bit;
pub fn test_bsf() {
    assert_eq!(first_free_bit(0b11111110), 0);
    assert_eq!(first_free_bit(0b11111101), 1);
    assert_eq!(first_free_bit(0b11111011), 2);
    assert_eq!(first_free_bit(0b11110111), 3);
    assert_eq!(first_free_bit(0b01101111), 4);
    assert_eq!(first_free_bit(0b11011111), 5);
    assert_eq!(first_free_bit(0b10111111), 6);
    assert_eq!(first_free_bit(0b01111111), 7);
}

/// Sectors beginning bytes:
/// c316 fbd4 27
/// ee63 c5b0 3a
/// 48c8 6fa8 ed
// fn test_ata_read_hdd() {
//     let ata1 = ata::AtaBlockDevice::new(ata::DriveSelect::Master);
//     let blocks: Vec<u8> = ata1.read_blocks(0, 3).unwrap();
//     assert_eq!(blocks.len(), 512*3);

//     let expected = vec![0xc3, 0x16, 0xfb, 0xd4, 0x27];
//     assert_eq!(&blocks[0..5], &expected);

//     let expected_2nd_sector = vec![0xee, 0x63, 0xc5, 0xb0, 0x3a];
//     assert_eq!(&blocks[512..512+5], &expected_2nd_sector);

//     let expected_3rd_sector = vec![0x48, 0xc8, 0x6f, 0xa8, 0xed];
//     assert_eq!(&blocks[1024..1024+5], &expected_3rd_sector);
// }

/// Test our TAR-reading capabilities
/// This is possible because the test script has:
/// -hda kernel/resources/test/test.tar
fn test_read_ustar() {
    let ata1 = ata::AtaBlockDevice::new(ata::DriveSelect::Master);
    let ustar = fs::ustar::UStarFs::new(ata1);
    let entries = ustar.ls();
    assert_eq!(entries, vec!["hello", "file2", "kernel"]);
    let file = ustar.open("hello").unwrap();
    assert_eq!("hello\n", string::cstr_to_string(&file.read().unwrap()));

    let file = ustar.open("file2").unwrap();
    assert_eq!("a string\n", string::cstr_to_string(&file.read().unwrap()));

    // Test opening large file
    let file = ustar.open("kernel").unwrap();
    serial_println!("{:#?}", file);
    file.read().unwrap();

    match ustar.open("other file") {
        Ok(bad_file) => panic!("Expected no files, opened {:#?} instead", bad_file),
        Err(fs::FileErr::NoSuchFile) => {},
        // Err(e) => panic!("Expected {:#?} err, got {:#?}", fs::FileErr::NoSuchFile, e),
    };
}


fn test_read_invalid_hdd() {
    let mut buf = [0u8; 512];
    unsafe {
        let read_result = ata::read_sectors_direct(ata::DriveSelect::Slave, 0, 1, buf.as_mut_ptr());
        assert!(read_result.is_err());
    }
    // TODO: identify doesn't work
    // ata::identify(ata::DriveSelect::Slave).unwrap();
}

/// Allocate a 4MB page and verify we can write to the
/// beginning and end of it
fn test_allocate_kernel_page() {
    let addr = mem::virt::alloc_kernel_page();
    unsafe {
        let test = addr as *mut u32;
        *test = 0xdeadbeef;
        let test2 = (addr + (4 * 1024 * 1024) - 4) as *mut u32;
        *test2 = 0xdeadbeef;
    }
    mem::virt::free_kernel_page(addr);
}
