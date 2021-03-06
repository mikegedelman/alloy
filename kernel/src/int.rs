//! Interrupt handlers
//! Best interrupts info I can find:
//! https://alex.dzyoba.com/blog/os-interrupts/
use log::info;

use crate::cpu;
use crate::drivers::pic_8259;
use crate::drivers::ps2;

fn send_eoi(irq: u32) {
    if irq >= 8 {
        pic_8259::pic2_eoi();
    }

    pic_8259::pic1_eoi();
}

fn syscall_write(data: *const u8) {
    println!("syscall 1, data addr: {:#x}", data as u32);
    unsafe {
        let mut x = data;
        loop {
            let val = *x;
            if val == 0 {
                break;
            }
            print!("{}", val as char);
            x = x.offset(1);
        }
    }
    println!("done");
}

#[no_mangle]
pub unsafe extern "C" fn _syscall(num: u32, data: *const u8) {
    match num {
        0x1 => syscall_write(data),
        _ => {
            info!("syscall {} not supported", num);
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn isr_handler(x: u32, info: u32) {
    match x {
        14 => {
            let addr = cpu::get_cr2();
            panic!("Page fault at address: {:#x}", addr);
        }
        // Just for testing: asm!("int 50") should cause "sycall 50" to be printed
        0x80 => println!("syscall 0x80 {:#x}", info),
        // IRQs
        32 => {} // IRQ0 is hardware timer: ignore for now
        33 => {
            // IRQ1 is keyboard input
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
