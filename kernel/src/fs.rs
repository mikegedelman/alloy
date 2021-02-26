use crate::cpu::{inb,outb,inw};

const STATUS_BSY: u8 = 0x80;
const STATUS_RDY: u8 = 0x40;

pub fn read_sectors_ata_pio(lba: u32) -> [u16; 256] // , u8 sector_count)
{

    unsafe {
        ata_wait_bsy();
        outb(0x1F6,0xE0 | ((lba >>24) & 0xF) as u8);
        outb(0x1F2, 1); // sector_count);
        outb(0x1F3, lba as u8);
        outb(0x1F4, (lba >> 8) as u8);
        outb(0x1F5, (lba >> 16) as u8);
        outb(0x1F7,0x20); // Send the read command

        let mut buf = [0u16; 256];
        ata_wait_bsy();
        ata_wait_drq();
        for i in 0..256 {
            buf[i] = inw(0x1F0);
        }
        buf
    }
}

unsafe fn ata_wait_bsy() { // Wait for bsy to be 0
	while (inb(0x1F7) & STATUS_BSY) != 0 {}
}
unsafe fn ata_wait_drq() { // Wait fot drq to be 1
	while (inb(0x1F7) & STATUS_RDY) == 0 {}
}