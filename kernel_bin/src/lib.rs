#![no_std]

#[macro_use] extern crate kernel;
use kernel::{cpu,mem,proc,drivers,io,multiboot};
use log::info;

/// Setup routines: load GDT, IDT, remap PIC, ..
unsafe fn init(multiboot_info_ptr: *const multiboot::MultibootInfo, magic: u32) {
    cpu::gdt::init();
    cpu::idt::init();
    drivers::pic_8259::remap_int(32, 40);
    cpu::enable_int();
    io::term::disable_cursor();

    io::logging::init().unwrap();
    // Don't log until interrupts are created for the first time.
    // Our println! function is currently dumb and disables them and
    // re-enables no matter what. TODO: handle this case
    cpu::disable_int();
    info!("Early init complete:\n  - loaded GDT and IDT\n  - remapped PIC interrupt vectors\n  - enabled interrupts");
    assert_eq!(
        magic,
        multiboot::MAGIC, //
        "Unexpected magic value {:#x} - expected {:#x}",
        magic,
        multiboot::MAGIC
    );

    let multiboot_info: &multiboot::MultibootInfo = &*multiboot_info_ptr;
    mem::physical::load_multiboot_mmap(multiboot_info);

    let freef = mem::physical::num_free_frames();
    info!(
        "{} physical frames are free - {} MB",
        freef,
        (freef * 0x1000) / 1024 / 1024
    );

    mem::virt::init_kernel_page_dir();
    mem::heap::init();
}

/// Main entrypoint for our kernel from loader.asm
#[cfg(not(feature = "test"))]
#[no_mangle]
pub unsafe extern "C" fn kernel_main(
    multiboot_info_ptr: *const multiboot::MultibootInfo,
    magic: u32,
) -> ! {
    init(multiboot_info_ptr, magic);
    println!("Welcome to alloy!");

    proc::exec::exec("hello_rs");

    loop {
        cpu::hlt();
    }
}

/// Main entrypoint for the kernel when in test mode
#[cfg(feature = "test")]
#[no_mangle]
pub unsafe extern "C" fn kernel_main(
    multiboot_info_ptr: *const multiboot::MultibootInfo,
    magic: u32,
) -> ! {
    init(multiboot_info_ptr, magic);
    test::run_tests();
    loop {
        cpu::hlt();
    }
}
