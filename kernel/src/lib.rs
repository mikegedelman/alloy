#![no_std]
#![feature(asm)]

/// Log messages will be printed to the terminal if verbose feature
/// is enabled. Eventually, this will probably route into a kernel
/// log file somewhere
/// Crate-wide should be defined before any other modules so that those
/// modules can see them.
#[macro_use]
mod macros {
    macro_rules! log {
        ($($arg:tt)*) => (
            #[cfg(feature = "verbose")]
            {
                println!($($arg)*);
            }
        );
    }
}

/// term and serial modules are first so that other modules can see the
/// println! and serial_println! macros
#[macro_use]
mod term;

#[macro_use]
mod serial;

#[allow(dead_code)]
#[allow(unused_unsafe)]
mod drivers;

mod boot;
mod cpu;
mod externs;
mod int;

/// This module will only be compiled in if we've specified
/// --features test to cargo build.
#[cfg(feature = "test")]
mod test;

/// Setup routines: load GDT, IDT, remap PIC, ..
fn init() {
    unsafe {
        externs::utils_asm::load_gdt();
        boot::setup_idt();
        log!("Loaded GDT and IDT.");

        drivers::pic_8259::remap_int(32, 40);
        cpu::enable_int();
        log!("Enabled interrupts.");
    }
    term::disable_cursor();
}

/// Main entrypoint for our kernel from loader.asm
#[cfg(not(feature = "test"))]
#[no_mangle]
pub unsafe extern "C" fn kernel_main() -> ! {
    init();
    println!("Welcome to alloy!");

    loop {
        cpu::hlt();
    }
}

/// Invoked when something calls panic!
#[cfg(not(feature = "test"))]
#[panic_handler]
fn panic(info: &::core::panic::PanicInfo) -> ! {
    println!("{}", info);
    loop {
        cpu::hlt();
    }
}

/// Main entrypoint for the kernel when in test mode
#[cfg(feature = "test")]
#[no_mangle]
pub unsafe extern "C" fn kernel_main() -> ! {
    boot();
    test::run_tests();
    loop {
        cpu::hlt();
    }
}
