#![no_std]
#![feature(asm)]

mod ata;
mod cpu;

const SECTORS_TO_READ: u32 = 8192; // 4MB worth: 4 * 1024 * 1024
const TARGET_MEM: u32 = 0x10000; // load the kernel here

#[no_mangle]
pub extern "C" fn stage2_main() {
    let mut vga_mem = 0xB8000 as *mut u8;
    unsafe {
        *vga_mem = b'.';
        *(vga_mem.offset(1)) = 0xF;
    }

    // let mem_start = TARGET_MEM as *mut u8;
    // let mut ata1 = ata::AtaPio::new(0x1F0);
    // unsafe {
    //     let num_loops = SECTORS_TO_READ/0x100;
    //     for sector_batch in 0..num_loops {
    //         // We might read a little extra but it's ok for now
    //         ata::read_sectors_direct(&mut ata1, ata::DriveSelect::Master, 0, 0xFF, mem_start);
    //     }
    // }
}

/// Invoked when something calls panic!
#[cfg(not(feature = "test"))]
#[panic_handler]
fn panic(info: &::core::panic::PanicInfo) -> ! {
    loop { //
        cpu::hlt();
    }
}
