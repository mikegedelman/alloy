#![no_std]
#![feature(asm)]
#[panic_handler]fn panic(_:&::core::panic::PanicInfo)->!{loop{}}

mod term;
mod cpu;
mod fs;

#[no_mangle] pub unsafe extern "C"
fn kernel_main () -> !
{
    term::init();
    println!("Welcome to ros");

    let buf = fs::read_sectors_ata_pio(0u32);
    println!("{:?}", &buf[0..5]);
    println!("ayy lmfao");
    println!("Hello world from having not smashed the stack");

    loop {}
}