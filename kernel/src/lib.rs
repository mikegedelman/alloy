#![no_std]
#![no_main]
#![feature(asm)]

#[panic_handler]fn panic(info: &::core::panic::PanicInfo)->!{
    println!("{}", info);
    loop {}
}

#[macro_use]
mod term;
mod cpu;
mod drivers;
mod int;

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
    term::init();
}

// Default representation, alignment lowered to 2.
// #[repr(C, packed(2))]
// struct IdtEntry {
//     handler_low: u16,
//     some: u16,
//     some: u8,
//     some: u8,
//     some: u16
// }

// fn build_idt() {

// }


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
    // Figure out why keyboard input isn't prompting an IRQ message.
    // Maybe the BIOS interrupts aren't around anymoree
    boot();

    test::run_tests();

    asm!("hlt");
    loop {}
}





// #[cfg(test)]
// fn test_runner(tests: &[&dyn Fn()]) {
//     println!("Running {} tests", tests.len());
//     for test in tests {
//         test();
//     }
// }