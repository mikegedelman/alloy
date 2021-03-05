//! ATA PIO driver

use lazy_static::lazy_static;
use spin::Mutex;


use crate::cpu::{enable_int, disable_int, Port};
use crate::externs::utils_asm;

const COMMAND_READ: u8 = 0x20;
const COMMAND_IDENTIFY: u8 = 0xEC;

const STATUS_ERR: u8 = 1;
const STATUS_DRQ: u8 = 1 << 3;
const STATUS_DRF: u8 = 1 << 5;
const STATUS_BSY: u8 = 1 << 7;


pub struct AtaPio {
    data: Port,
    error: Port,
    sectors: Port,
    lba_lo: Port,
    lba_mid: Port,
    lba_hi: Port,
    drive_select: Port,
    command: Port,
    // TODO: probably save state of last drive select, and skip
    // selecting again if we're reading the same drive, because
    // selecting is slow
}

#[derive(Debug)]
pub enum AtaErr {
    ReadError,
    DriveFaultError,
    InvalidDrive,
    NotAta,
    UnknownError,
}

// pub enum BusSelect {
//     Bus1,
//     Bus2,
// }

pub enum DriveSelect {
    Master,
    Slave,
}

lazy_static! {
    pub static ref ATA1: Mutex<AtaPio> = Mutex::new(AtaPio::new(0x1F0));
    // pub static ref ATA2: Mutex<AtaPio> = Mutex::new(AtaPio::new(0x170));
}


impl AtaPio {
    pub fn new(base: u16) -> AtaPio {
        AtaPio {
            data: Port::new(base),
            error: Port::new(base + 1),
            sectors: Port::new(base + 2),
            lba_lo: Port::new(base + 3),
            lba_mid: Port::new(base + 4),
            lba_hi: Port::new(base + 5),
            drive_select: Port::new(base + 6),
            command: Port::new(base + 7),
        }
    }

    fn read_command(&mut self, drive: DriveSelect, lba: u32, sectors: u8) -> u8 {
        let drive_select = match drive {
            DriveSelect::Master => 0xE0,
            DriveSelect::Slave => 0xF0,
        };

        self.drive_select.write(drive_select | ((lba >> 24) & 0xF) as u8);
        self.sectors.write(sectors);
        self.lba_lo.write(lba as u8);
        self.lba_mid.write((lba >> 8) as u8);
        self.lba_hi.write((lba >> 16) as u8);
        self.command.write(COMMAND_READ);

        // 400ns delay. See: https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
        for _ in 0..14 { self.command.read(); }
        self.command.read()
    }

    fn read_word(&mut self) -> u16 {
        self.data.read_word()
    }

    fn read_error(&mut self) -> u8 {
        self.error.read()
    }

    fn ata_wait_bsy(&mut self) {
        // Wait for bsy to be 0
        while (self.command.read() & STATUS_BSY) != 0 {}
    }
    fn ata_wait_drq(&mut self) {
        // Wait fot drq to be 1
        while (self.command.read() & STATUS_DRQ) == 0 {}
    }
}


// pub fn identify(drive: DriveSelect) -> Result<(), AtaErr> {
//     disable_int();
//     let mut ata = ATA1;
//     ata.identify_command(drive).unwrap();
//     enable_int();
//     Ok(())
// }



pub unsafe fn read_sectors_direct(drive: DriveSelect, lba: u32, sectors: u8, mem_ptr: *mut u8) -> Result<(), AtaErr>
{
    let mut ata = ATA1.lock();
    let mut mem = mem_ptr as *mut u16;

    disable_int();
    ata.ata_wait_bsy();
    let status = ata.read_command(drive, lba, sectors);
    // let error = ata.read_error();
    if status == 0 {
        return Err(AtaErr::InvalidDrive);
    }
    if (status & 1) == 1 {
        return Err(AtaErr::ReadError);
    }
    if (status & STATUS_DRF) == 1 {
        return Err(AtaErr::DriveFaultError);
    }

    for _ in 0..sectors {
        ata.ata_wait_bsy();
        ata.ata_wait_drq();

        for _ in 0..256 {
            let data = ata.read_word();
            utils_asm::io_wait();

            *mem = data;
            mem = mem.offset(1);
        }
    }
    enable_int();
    Ok(())
}
