use alloc::vec::Vec;
use alloc::string::String;
use crate::drivers::{BlockRead, BlockErr};

pub mod ustar;


#[derive(Clone, Debug)]
pub struct File<R: BlockRead> {
    block_reader: R,
    lba: usize,
    size: usize,
    name: String,
    // cur_pos: usize,
}

impl<R: BlockRead> File<R> {
    pub fn read(&self) -> Result<Vec<u8>, BlockErr> {
        let num_blocks = (self.size + 512 - 1) / 512;
        let mut blocks = self.block_reader.read_blocks(self.lba, num_blocks).unwrap();
        blocks.drain(self.size..);
        Ok(blocks)
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
