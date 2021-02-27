#![no_std]
#![feature(asm)]

#[cfg(not(feature="test"))]
#[panic_handler]
fn panic(info: &::core::panic::PanicInfo)->!{
    println!("{}", info);
    loop {}
}

#[macro_use]
mod term;
mod cpu;
#[allow(dead_code)]
#[allow(unused_unsafe)]
mod drivers;
mod int;
#[macro_use]
mod serial;

#[cfg(feature="test")]
mod test;

extern "C" {
    fn load_gdt();
    fn setup_idt();
}

fn boot() {
    unsafe {
        load_gdt();
        setup_idt();
        cpu::enable_int();
    }
    term::disable_cursor();
}


#[cfg(not(feature="test"))]
#[no_mangle]
pub unsafe extern "C"
fn kernel_main () -> !
{
    // Figure out why keyboard input isn't prompting an IRQ message.
    // Maybe the BIOS interrupts aren't around anymoree
    boot();

    println!("Welcome to ros");
    asm!("int 50");
    println!("Welcome to ros");

    loop {}
}


#[cfg(feature="test")]
#[no_mangle]
pub unsafe extern "C"
fn kernel_main () -> !
{
    boot();
    test::run_tests();
    loop {}
}
