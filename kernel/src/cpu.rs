//! Wrap some common asm instructions in functions for ease of use/readability

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

pub fn enable_int() {
    unsafe { asm!("sti"); }
}

pub fn disable_int() {
    unsafe { asm!("cli"); }
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