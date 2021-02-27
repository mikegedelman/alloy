// Best interrupts info
// https://alex.dzyoba.com/blog/os-interrupts/


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

use crate::cpu;
use crate::drivers::ps2;

const PIC1: u16 = 0x20;	/* IO base address for master PIC */
const PIC2: u16 = 0xA0;		/* IO base address for slave PIC */
const PIC1_COMMAND: u16 = PIC1;
const PIC1_DATA: u16 = (PIC1+1);
const PIC2_COMMAND: u16 = PIC2;
const PIC2_DATA: u16 = (PIC2+1);
const PIC_EOI: u8 = 0x20; // End-of-interrupt command code


fn pic_send_eoi(irq: u32) {
	if irq >= 8 {
		cpu::outb(PIC2_COMMAND, PIC_EOI);
    }

	cpu::outb(PIC1_COMMAND, PIC_EOI);
}

#[no_mangle]
pub unsafe extern "C"
fn isr_handler(x: u32) {
    match x {
        50 => println!("syscall 50"),
        32 => {},  // Hardware timer: ignore for now
        33 => {
            let scancode = cpu::inb(0x60);
            ps2::process_scancode(scancode);
        },
        _ => {},
    }
    pic_send_eoi(x);
}

#[no_mangle]
pub unsafe extern "C"
fn _exit() {

}