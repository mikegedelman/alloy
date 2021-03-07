pub mod ata;
pub mod pic_8259;
pub mod ps2;
pub mod uart_16550;


use alloc::vec::Vec;

#[derive(Debug)]
pub enum BlockErr {
    ReadError,
    DriveFaultError,
    InvalidDrive,
    NotAta,
    UnknownError,
}

pub trait BlockRead {
    fn read_blocks(&self, lba: usize, num_block: usize) -> Result<Vec<u8>, BlockErr>;
}
