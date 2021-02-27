pub struct Port {
    port: u16
}

impl Port {
    pub fn new(port: u16) -> Port {
        Port { port }
    }

    pub fn write(&mut self, value: u8) {
        unsafe{ outb(self.port, value); }
    }

    pub fn read(&mut self) -> u8 {
        unsafe{ inb(self.port) }
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

pub unsafe fn inw(port: u16) -> u16 {
    let val: u16;
    asm!(
        "in ax, dx",
        out("ax") val,
        in("dx") port,
     );
     val
}

pub fn enable_int() {
    unsafe{ asm!("sti"); }
}

pub fn disable_int() {
    unsafe{ asm!("cli"); }
}
