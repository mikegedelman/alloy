use alloc::vec::Vec;
use alloc::string::String;
use alloc::string::ToString;

use crate::fs::{BlockRead,File,FileErr};
use crate::string;

// https://wiki.osdev.org/USTAR
#[derive(Clone, Copy, Debug)]
#[repr(C, packed(2))]
pub struct UStarHeader {
    pub file_name: [u8; 100],
    pub file_mode: [u8; 8],
    pub owner_uid: [u8; 8],
    pub group_uid: [u8; 8],
    pub file_size: [u8; 12],
    pub last_mod: [u8; 12],
    pub checksum: [u8; 8],
    pub type_flag: u8,
    pub linked_file_name: [u8; 100],
    pub ustar_ind: [u8; 6],
    pub version: u16,
    pub owner_name: [u8; 32],
    pub group_name: [u8; 32],
    pub device_major: [u8; 8],
    pub device_minor: [u8; 8],
}

impl UStarHeader {
    fn get_size(&self) -> u64 {
        // The last character is always a space for some dumb reason - chop it off
         oct2bin(&self.file_size[..11])
    }
}

#[derive(Clone, Copy, Debug)]
pub struct UStarFs<R: BlockRead> {
    block_reader: R,
}

pub struct UStarIterator<R: BlockRead> {
    fs: UStarFs<R>,
    next_header_block: usize,
}

impl<R: Clone + BlockRead> UStarFs<R> {
    pub fn new(block_reader: R) -> UStarFs<R> {
        UStarFs {
            block_reader
        }
    }

    fn read_header(&self, lba: usize) -> UStarHeader {
        let header_buf = self.block_reader.read_blocks(lba, 1).unwrap().as_ptr();
        unsafe {
            let header_ptr = header_buf as *const UStarHeader;
            let header: UStarHeader = *header_ptr;
            header
        }
    }

    fn entries(&self) -> UStarIterator<R> {
        UStarIterator { fs: self.clone(), next_header_block: 0 }
    }

    #[allow(dead_code)]
    pub fn ls(&self) -> Vec<String> {
        self.entries().map(|(_, hdr)| string::cstr_to_string(&hdr.file_name)).collect()
    }

    pub fn open(&self, file_name: &str) -> Result<File<R>, FileErr> {
        for (header_lba, header) in self.entries() {
            if string::cstr_to_string(&header.file_name) == file_name {
                return Ok(File {
                    block_reader: self.block_reader.clone(),
                    lba: header_lba + 1,
                    size: header.get_size() as usize,
                    name: file_name.to_string(),
                });
            }
        }
        Err(FileErr::NoSuchFile)
    }
}


impl<R: Clone + BlockRead> Iterator for UStarIterator<R> {
    type Item = (usize, UStarHeader);

    /// Just roll with it until we find an issue
    fn next(&mut self) -> Option<Self::Item> {
        let cur_block = self.next_header_block;
        let cur_header = self.fs.read_header(cur_block);
        if &cur_header.ustar_ind[..5] != "ustar".as_bytes() {
            return None;
        }
        let size = cur_header.get_size();
        if size == 0 {
            return None;
        }
        self.next_header_block += ((cur_header.get_size() as usize + 511) / 512) + 1;
        Some((cur_block, cur_header))
    }
}


pub fn oct2bin(ustar_oct: &[u8]) -> u64 {
    let mut n: u64 = 0;
    for c in ustar_oct {
        n *= 8;
        n += (c - b'0') as u64;
    }
    n
}
