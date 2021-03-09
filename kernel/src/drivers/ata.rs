//! ATA PIO driver
//! It's dumb and slow, but it's enough to load stuff from disk

use lazy_static::lazy_static;
use spin::Mutex;
use alloc::vec::Vec;

use crate::drivers::{BlockErr,BlockRead};
use crate::cpu::{disable_int, reenable_int, Port, io_wait};

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
    // TODO: don't read to a vec. Due to the way vecs work, this ends up taking up ~2x
    // as much memory as the file.
    fn read_blocks(&self, lba: usize, num_blocks: usize) -> Result<Vec<u8>, BlockErr> {
        let mut blocks: Vec<u8> = vec![];
        let mut total_sectors_to_read = num_blocks;
        while total_sectors_to_read > 0 {
            let cur_sectors_to_read = core::cmp::min(total_sectors_to_read, 255) as u8;
            let mut read_blocks = unsafe {  read_sectors_vec(&self.drive, lba, cur_sectors_to_read)? };
            blocks.append(&mut read_blocks);
            total_sectors_to_read -= cur_sectors_to_read as usize;
        }
        Ok(blocks)
    }

    // fn read(&self, lba: usize, num_blocks: usize, buf: &mut [u8]) -> Result<usize, BlockErr> {
    //     let mut total_sectors_to_read = num_blocks;
    //     let mut cur_sector = 0;
    //     let bytes_read = 0;
    //     while total_sectors_to_read > 0 {
    //         let cur_sectors_to_read = core::cmp::min(total_sectors_to_read, 255) as u8;
    //         unsafe {  read_sectors(&self.drive, lba, cur_sectors_to_read, &mut buf[cur_sector*512..(cur_sector+1)*512])? };
    //         total_sectors_to_read -= cur_sectors_to_read as usize;
    //         cur_setor += 1
    //     }
    // }
}

/// Read sectors starting at lba into a vec
/// This turned out to be an *awful* idea. Due to how vecs work, roughly 2x the
/// amount of data being read is actually allocated on the heap.
/// TODO: This should read in to a buffer, not a vec.
pub unsafe fn read_sectors_vec(
    drive: &DriveSelect,
    lba: usize,
    sectors: u8,
) -> Result<Vec<u8>, BlockErr> {
    let mut ata = ATA1.lock();

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

    let mut ret: Vec<u8> = vec![];
    for _ in 0..sectors {
        ata.ata_wait_bsy();
        ata.ata_wait_drq();

        for _ in 0..256 {
            let data = ata.read_word();
            io_wait();

            ret.push(data as u8);
            ret.push((data >> 8) as u8);
        }
    }
    reenable_int();
    Ok(ret)
}

pub unsafe fn read_sectors(
    drive: &DriveSelect,
    lba: usize,
    sectors: usize,
    buf: &mut [u8],
) -> Result<usize, BlockErr> {
    let mut ata = ATA1.lock();

    disable_int();
    ata.ata_wait_bsy();
    let status = ata.read_command(drive, lba, sectors as u8);

    if status == 0 {
        return Err(BlockErr::InvalidDrive);
    }
    if (status & 1) == 1 {
        return Err(BlockErr::ReadError);
    }
    if (status & STATUS_DRF) == 1 {
        return Err(BlockErr::DriveFaultError);
    }

    for i in 0..sectors {
        ata.ata_wait_bsy();
        ata.ata_wait_drq();

        for _ in 0..256 {
            let data = ata.read_word();
            io_wait();

            buf[i] = data as u8;
            buf[i + 1] = (data >> 8) as u8;
        }
    }
    reenable_int();
    Ok(sectors * 512)
}