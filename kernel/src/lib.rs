#![no_std]
// #![no_main]
#![feature(asm)]
#![feature(alloc_error_handler)]
#![feature(rustc_private)]
#![feature(const_raw_ptr_to_usize_cast)]

#[allow(unused_imports)] #[macro_use] extern crate alloc;
extern crate compiler_builtins;

/// io module is first so that other modules can see the
/// println! and serial_println! macros
#[macro_use] pub mod io;

#[allow(dead_code)]
#[allow(unused_unsafe)] pub mod drivers;

pub mod cpu;
pub mod externs;
#[allow(dead_code)] pub mod mem;
pub mod multiboot;
pub mod fs;
pub mod proc;
pub mod string;

/// This module will only be compiled in if we've specified
/// --features test to cargo build.
#[cfg(feature = "test")] mod test;

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