pub unsafe fn outb(port: u16, value: u8) {
    asm!(
        "out dx, al",
        in("dx") port,
        in("al") value,
     );
}

pub unsafe fn inb(port: u16) -> u8 {
    let val: u8;
    asm!(
        "in al, dx",
        out("al") val,
        in("dx") port,
     );
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