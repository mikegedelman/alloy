pub mod ata;
pub mod pic_8259;
pub mod ps2;
pub mod uart_16550;

use alloc::vec::Vec;

/// A generic error thrown by block devices
#[derive(Debug)]
pub enum BlockErr {
    ReadError,
    DriveFaultError,
    InvalidDrive,
    NotAta,
    UnknownError,
}

/// A block device can read blocks into a vec. See comments in ata.rs, but basically
/// this should be changed to read into an array or slice.
pub trait BlockRead {
    fn read_blocks(&self, lba: usize, num_block: usize) -> Result<Vec<u8>, BlockErr>;
    // fn read(&self, lba: usize, num_blocks: usize, buf: &mut [u8]) -> Result<usize, BlockErr>;
}
