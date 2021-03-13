//! ATA PIO driver
//! It's dumb and slow, but it's enough to load stuff from disk

use lazy_static::lazy_static;
use spin::Mutex;


use super::{BlockErr,BlockRead};
use crate::cpu::{disable_int, enable_int, Port, io_wait};

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

#[derive(Copy, Clone, Debug)]
pub enum DriveSelect {
    Master,
    Slave,
}

lazy_static! {
    pub static ref ATA1: Mutex<AtaPio> = Mutex::new(AtaPio::new(0x1F0));
    // I wanted to make logic that would allow you to dynamically select different ATA buses,
    // but this macro creates completely new types for each ref, so the logic got ugly. Maybe
    // doable with generics
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

    fn read_command(&mut self, drive: &DriveSelect, lba: usize, sectors: u8) -> u8 {
        let drive_select = match drive {
            DriveSelect::Master => 0xE0,
            DriveSelect::Slave => 0xF0,
        };

        self.drive_select
            .write(drive_select | ((lba >> 24) & 0xF) as u8);
        self.sectors.write(sectors);
        self.lba_lo.write(lba as u8);
        self.lba_mid.write((lba >> 8) as u8);
        self.lba_hi.write((lba >> 16) as u8);
        self.command.write(COMMAND_READ);

        // 400ns delay. See: https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
        for _ in 0..14 {
            self.command.read();
        }
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

// pub fn identify(drive: DriveSelect) -> Result<(), BlockErr> {
//     disable_int();
//     let mut ata = ATA1;
//     ata.identify_command(drive).unwrap();
//     enable_int();
//     Ok(())
// }

#[derive(Copy, Clone, Debug)]
pub struct AtaBlockDevice {
    drive: DriveSelect,
}

impl AtaBlockDevice {
    pub fn new(drive: DriveSelect) -> AtaBlockDevice {
        AtaBlockDevice {
            drive
        }
    }
}

impl BlockRead for AtaBlockDevice {
    fn block_read(&self, lba: usize, bytes_to_read: usize, buf: &mut [u8]) -> Result<usize, BlockErr> {
        let mut bytes_read = 0;

        while bytes_read < bytes_to_read {
            let bytes_left = bytes_to_read - bytes_read;
            // Can only read 255 sectors at a time
            let bytes_to_read_this_loop = core::cmp::min(bytes_left, 255*512);

            bytes_read += read_bytes(
                &self.drive,
                lba + (bytes_read / 512),
                bytes_to_read_this_loop,
                &mut buf[bytes_read..bytes_read+bytes_to_read_this_loop]
            )?;
        }
        Ok(bytes_read)
    }
}


/// Read bytes starting at lba into a slice
fn read_bytes(
    drive: &DriveSelect,
    lba: usize,
    bytes_to_read: usize,
    buf: &mut [u8],
) -> Result<usize, BlockErr> {
    let mut ata = ATA1.lock();
    assert!(bytes_to_read <= 512*255, "{}", bytes_to_read); // Only can read 255 sectors at a time
    let sectors: u8  = ((bytes_to_read + 511) / 512) as u8;

    disable_int();
    ata.ata_wait_bsy();
    let status = ata.read_command(drive, lba, sectors);

    if status == 0 {
        return Err(BlockErr::InvalidDrive);
    }
    if (status & 1) == 1 {
        return Err(BlockErr::ReadError);
    }
    if (status & STATUS_DRF) == 1 {
        return Err(BlockErr::DriveFaultError);
    }

    let mut bytes_read = 0;
    for sector in 0..sectors {
        ata.ata_wait_bsy();
        ata.ata_wait_drq();

        // let buf_as_u16 = unsafe { buf.as_ptr().offset(sector as isize * 512) as *mut u16 };
        let mut tmp_buf = [0u16; 256];
        for i in 0..256 {
            let data = ata.read_word();
            io_wait();
            tmp_buf[i] = data;
        }

        unsafe {
            let tmp_as_u8 = unsafe { tmp_buf.as_ptr() as *const u8 };
            let dest = buf.as_mut_ptr().offset(sector as isize * 512);
            let bytes_to_copy = core::cmp::min(bytes_to_read - bytes_read, 512);
            compiler_builtins::mem::memcpy(dest, tmp_as_u8, bytes_to_copy);
            bytes_read += bytes_to_copy;
        }
    }
    enable_int();
    Ok(bytes_read)
}
