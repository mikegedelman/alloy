//! Commands for the 8259 PIC (Programmable Interrupt Controller)
//! Adapted from https://wiki.osdev.org/8259_PIC

use crate::cpu::{inb, outb, io_wait};

const PIC1: u16 = 0x20; // IO base address for master PIC
const PIC2: u16 = 0xA0; // IO base address for slave PIC
const PIC1_COMMAND: u16 = PIC1;
const PIC1_DATA: u16 = PIC1 + 1;
const PIC2_COMMAND: u16 = PIC2;
const PIC2_DATA: u16 = PIC2 + 1;
const PIC_EOI: u8 = 0x20; // End-of-interrupt command code

const ICW1_ICW4: u8 = 0x01; // ICW4 (not) needed
const ICW1_SINGLE: u8 = 0x02; // Single (cascade) mode
const ICW1_INTERVAL4: u8 = 0x04; // Call address interval 4 (8)
const ICW1_LEVEL: u8 = 0x08; // Level triggered (edge) mode
const ICW1_INIT: u8 = 0x10; // Initialization - required!

const ICW4_8086: u8 = 0x01; // 8086/88 (MCS-80/85) mode
const ICW4_AUTO: u8 = 0x02; // Auto (normal) EOI
const ICW4_BUF_SLAVE: u8 = 0x08; // Buffered mode/slave
const ICW4_BUF_MASTER: u8 = 0x0C; // Buffered mode/master
const ICW4_SFNM: u8 = 0x10; // Special fully nested (not)

/// End of input command for PIC1
pub fn pic1_eoi() {
    outb(PIC1_COMMAND, PIC_EOI);
}

/// End of input command for PIC1
pub fn pic2_eoi() {
    outb(PIC2_COMMAND, PIC_EOI);
}

/// Remap interrupt vectors
pub fn remap_int(offset1: u8, offset2: u8) {
    unsafe {
        // Save interrupt masks
        let pic1_mask = inb(PIC1_DATA);
        let pic2_mask = inb(PIC2_DATA);

        // starts the initialization sequence (in cascade mode)
        outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
        io_wait();
        outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
        io_wait();

        // ICW2: Master PIC vector offset
        outb(PIC1_DATA, offset1);
        io_wait();

        // ICW2: Slave PIC vector offset
        outb(PIC2_DATA, offset2);
        io_wait();

        // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
        outb(PIC1_DATA, 4);
        io_wait();
        // ICW3: tell Slave PIC its cascade identity (0000 0010)
        outb(PIC2_DATA, 2);
        io_wait();

        outb(PIC1_DATA, ICW4_8086);
        io_wait();
        outb(PIC2_DATA, ICW4_8086);
        io_wait();

        // Restore saved masks
        outb(PIC1_DATA, pic1_mask);
        outb(PIC2_DATA, pic2_mask);
    }
}
