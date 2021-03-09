//! Wrap some common asm instructions in functions for ease of use/readability

pub mod interrupts;
pub mod idt;
pub mod gdt;

use lazy_static::lazy_static;
use spin::Mutex;

lazy_static! {
    static ref INT_STATUS: Mutex<IntStatus> = Mutex::new(IntStatus { enabled: false });
}

struct IntStatus {
    pub enabled: bool
}

/// A port allows us to create a named variable for a 16-bit port
/// which can make code more readable
pub struct Port {
    port: u16,
}

impl Port {
    pub fn new(port: u16) -> Port {
        Port { port }
    }

    pub fn write(&mut self, value: u8) {
        outb(self.port, value);
    }

    pub fn read(&mut self) -> u8 {
        inb(self.port)
    }

    pub fn read_word(&mut self) -> u16 {
        inw(self.port)
    }
}

pub fn outb(port: u16, value: u8) {
    unsafe {
        asm!(
            "out dx, al",
            in("dx") port,
            in("al") value,
        );
    }
}

pub fn inb(port: u16) -> u8 {
    let val: u8;
    unsafe {
        asm!(
            "in al, dx",
            out("al") val,
            in("dx") port,
        );
    }
    val
}

pub fn inw(port: u16) -> u16 {
    let val: u16;
    unsafe {
        asm!(
            "in ax, dx",
            out("ax") val,
            in("dx") port,
        );
    }
    val
}

// /// Enable interrupts
// /// Unsafe because you need to be sure that interrupts have been set up properly
// /// before enabling.
// /// To reenable interrupts after a temporary disable, use reenable_int()
// pub unsafe fn enable_int() {
//     let mut int_status = INT_STATUS.lock();
//     asm!("sti");
//     int_status.enabled = true;
// }
pub unsafe fn enable_int() {
    let mut int_status = INT_STATUS.lock();
    asm!("sti");
    int_status.enabled = true;
}

pub fn reenable_int() {
    let int_status = INT_STATUS.lock();
    if int_status.enabled {
        unsafe { asm!("sti"); }
    }
}

// pub fn disable_int() {
//     unsafe {
//         asm!("cli");
//     }
// }

pub fn disable_interrupts_during<F>(f: F) -> () where
    F: Fn() -> () {
        disable_int();
        f();
        reenable_int();
    }




pub fn disable_int() {
    unsafe {
        asm!("cli");
    }
}

pub fn hlt() {
    unsafe {
        asm!("hlt");
    }
}

pub fn get_cr2() -> u32 {
    let val: u32;
    unsafe {
        asm!(
            "mov {0}, cr3",
            out(reg) val
        );
    }
    val
}

#[allow(dead_code)]
pub fn get_cr3() -> u32 {
    let val: u32;
    unsafe {
        asm!(
            "mov {0}, cr3",
            out(reg) val
        );
    }
    val
}

pub unsafe fn set_cr3(val: u32) {
    asm!(
        "mov cr3, {0}",
        in(reg) val
    );
}

pub unsafe fn invlpg(addr: u32) {
    asm!(
        "invlpg [{}]",
        in(reg) &addr,
        options(nostack),
    );
}

// TODO: this is probably dumb
pub fn io_wait() {
    unsafe {
        asm!(
            "nop",
            "nop",
            "nop",
        );
    }
}
