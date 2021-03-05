#![no_std]
#![feature(asm)]
#![feature(alloc_error_handler)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;

/// term and serial modules are first so that other modules can see the
/// println! and serial_println! macros
#[macro_use]
mod term;

#[macro_use]
mod serial;

#[macro_use]
mod logging;

#[allow(dead_code)]
#[allow(unused_unsafe)]
mod drivers;

mod boot;
mod cpu;
mod externs;
mod int;
mod multiboot;
#[allow(dead_code)]
mod mem;

/// This module will only be compiled in if we've specified
/// --features test to cargo build.
#[cfg(feature = "test")]
mod test;

/// Setup routines: load GDT, IDT, remap PIC, ..
unsafe fn init(multiboot_info_ptr: *const multiboot::MultibootInfo, magic: u32) {
    externs::utils_asm::load_gdt();
    boot::setup_idt();
    drivers::pic_8259::remap_int(32, 40);
    cpu::enable_int();
    term::disable_cursor();

    logging::init().unwrap();
    // Don't log until interrupts are created for the first time.
    // Our println! function is currently dumb and disables them and
    // re-enables no matter what. TODO: handle this case
    info!("Early init complete:\n  - loaded GDT and IDT\n  - remapped PIC interrupt vectors\n  - enabled interrupts");
    assert_eq!(magic, multiboot::MAGIC, "Unexpected magic value {:#x} - expected {:#x}", magic, multiboot::MAGIC);


    let multiboot_info: &multiboot::MultibootInfo = unsafe { &*multiboot_info_ptr };
    mem::physical::load_multiboot_mmap(multiboot_info);

    let freef = mem::physical::num_free_frames();
    info!("{} physical frames are free - {} MB", freef, (freef * 0x1000) / 1024 / 1024);

    mem::virt::init_kernel_page_dir();
    mem::heap::init();
}

use log::info;

use alloc::vec;

/// Main entrypoint for our kernel from loader.asm
#[cfg(not(feature = "test"))]
#[no_mangle]
pub unsafe extern "C" fn kernel_main(multiboot_info_ptr: *const multiboot::MultibootInfo, magic: u32) -> ! {
    init(multiboot_info_ptr, magic);

    let z = vec![1, 2, 3];
    println!("Welcome to alloy!");

    for i in z {
        println!("{}", i);
    }

    loop {
        cpu::hlt();
    }
}

/// Invoked when something calls panic!
#[cfg(not(feature = "test"))]
#[panic_handler]
fn panic(info: &::core::panic::PanicInfo) -> ! {
    println!("{}", info);
    cpu::disable_int();
    loop {
        cpu::hlt();
    }
}

/// Main entrypoint for the kernel when in test mode
#[cfg(feature = "test")]
#[no_mangle]
pub unsafe extern "C" fn kernel_main(multiboot_info_ptr: *const multiboot::MultibootInfo, magic: u32) -> ! {
    init(multiboot_info_ptr, magic);
    test::run_tests();
    loop {
        cpu::hlt();
    }
}
