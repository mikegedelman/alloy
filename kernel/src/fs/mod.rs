use alloc::vec::Vec;
use alloc::string::String;
use crate::drivers::storage::{BlockRead, BlockErr};

pub mod ustar;
pub mod block;

#[derive(Clone, Debug)]
pub struct File<R: BlockRead> {
    pub block_reader: R,
    pub lba: usize,
    pub size: usize,
    pub name: String,
}

impl<R: BlockRead> File<R> {
    pub fn read(&self) -> Result<Vec<u8>, BlockErr> {
        let mut buf = vec![0u8; self.size];
        let read_size = self.block_reader.block_read(self.lba, self.size, &mut buf).unwrap();
        assert_eq!(read_size, self.size);
        Ok(buf)
    }
}

pub trait Filesystem<R: BlockRead> {
    fn ls(&self, dir: &str) -> Vec<u8>;
    fn open(&mut self, file: &str) -> File<R>;
}

#[derive(Clone, Copy, Debug)]
pub enum FileErr {
    NoSuchFile,
}
