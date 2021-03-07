//! Interrupt handlers
//! Best interrupts info I can find:
//! https://alex.dzyoba.com/blog/os-interrupts/
use log::info;

use crate::cpu;
use crate::drivers::pic_8259;
use crate::drivers::ps2;

/// Send EOI "end of interrupt" to the PIC to let it know
/// we've processed it. Without this, it will just keep
/// handing us the same scancode for keypresses, for example.
fn send_eoi(irq: u32) {
    if irq >= 8 {
        pic_8259::pic2_eoi();
    }

    pic_8259::pic1_eoi();
}

/// Print the *null-terminated* string stored at pointer data
fn syscall_print(data: *const u8) {
    info!("print string addr: {:#x}", data as u32);
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
}

/// Dummy function for _exit
fn syscall_exit() {
    cpu::disable_int();
    info!("Program exited");
    // Clean up stuff for the current PID - dealloc the page we created
    loop { cpu::hlt(); }
}

/// Primary syscall handler: route to the proper logic and pass along data
#[no_mangle]
pub unsafe extern "C" fn _syscall(num: u32, data: *const u8) {
    match num {
        0x1 => syscall_print(data),
        0x2 => syscall_exit(),
        _ => {
            info!("syscall {} not supported", num);
        }
    }
}

/// Our handler for all interrupts except syscall (int 0x80)
/// We handle a few interrupts specially and just drop the rest
/// See utils.asm for how this is called
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
