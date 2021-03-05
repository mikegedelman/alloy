//! We could probably use macros here to make things fancy but for now we'll
//! just manully copy our tests to run into the run_tests() function.
// use alloc::{Vec,vec};
use alloc::vec::Vec;
use alloc::boxed::Box;

use crate::cpu::Port;
use crate::drivers::ata;
use crate::mem;

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
    let mut tests: Vec<Box<dyn Testable>> = vec![
        Box::new(&test_read_invalid_hdd),
        Box::new(&test_bsf),
        Box::new(&test_allocate_kernel_page),
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
/// a104 8c1d 4b58
/// 213b 05cc 7845
// /// 1731 ecda 1ea7
// fn test_ata_read_hdd() {
//     let mut buf = [0u8; 512*3];
//     let mut ata1 = ata::AtaPio::new(0x1F0);
//     unsafe {
//         ata::read_sectors_direct(ata::DriveSelect::Master, 0, 3, buf.as_mut_ptr()).unwrap();
//     };
//     let expected: [u8; 5] = [0xa1, 0x04, 0x8c, 0x1d, 0x4b];
//     assert_eq!(&buf[0..5], &expected);

//     let expected_2nd_sector: [u8; 5] = [0x21, 0x3b, 0x05, 0xcc, 0x78];
//     assert_eq!(&buf[512..512+5], &expected_2nd_sector);

//     let expected_3rd_sector: [u8; 5] = [0x17, 0x31, 0xec, 0xda, 0x1e];
//     assert_eq!(&buf[1024..1024+5], &expected_3rd_sector);
// }

fn test_read_invalid_hdd() {
    let mut buf = [0u8; 512];
    let mut ata1 = ata::AtaPio::new(0x1F0);
    unsafe {
        let read_result = ata::read_sectors_direct(ata::DriveSelect::Slave, 0, 1, buf.as_mut_ptr());
        assert!(read_result.is_err());
    }
    // TODO: identify is being weird
    // ata::identify(ata::DriveSelect::Slave).unwrap();
}

/// Allocate a 4MB page and verify we can write to the
/// beginning and end of it
fn test_allocate_kernel_page() {
    let addr = mem::virt::alloc_kernel_page();
    unsafe {
        let test = addr as *mut u32;
        *test = 0xdeadbeef;
        let test2 = (addr + (4*1024*1024)-4) as *mut u32;
        *test2 = 0xdeadbeef;
    }
    mem::virt::free_kernel_page(addr);
}
