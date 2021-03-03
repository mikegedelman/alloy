//! Interrupt handlers
//! Best interrupts info I can find:
//! https://alex.dzyoba.com/blog/os-interrupts/

use crate::cpu;
use crate::drivers::pic_8259;
use crate::drivers::ps2;

fn send_eoi(irq: u32) {
    if irq >= 8 {
        pic_8259::pic2_eoi();
    }

    pic_8259::pic1_eoi();
}

#[no_mangle]
pub unsafe extern "C" fn isr_handler(x: u32, info: u32) {
    match x {
        14 => panic!("Page fault at address: {:#x}", info),
        // Just for testing: asm!("int 50") should cause "sycall 50" to be printed
        50 => println!("syscall 50"),
        // IRQs
        32 => {} // IRQ0 is hardware timer: ignore for now
        33 => {  // IRQ1 is keyboard input
            let scancode = cpu::inb(0x60);
            ps2::process_scancode(scancode);
        }
        _ => {
            if x < 32 {
                panic!("Unhandled CPU exception no: {} - code {}", x, info);
            }
        }
    }
    send_eoi(x);
}
