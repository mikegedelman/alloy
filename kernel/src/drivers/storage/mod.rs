pub mod ata;

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
    fn block_read(&self, lba: usize, bytes: usize, buf: &mut [u8]) -> Result<usize, BlockErr>;
}
