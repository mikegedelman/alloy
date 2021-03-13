use crate::drivers::storage::{BlockRead, BlockErr};

pub struct BlockDevice<R: BlockRead> {
    block_reader: R,
    pub partition_table: [PartitionTableEntry; 4],
}

#[repr(C, packed(2))]
#[derive(Clone, Copy, Debug)]
pub struct RawMBR {
    bootcode: [u8; 440],
    signature: u32,
    reserved: u16,
    partition_table: [PartitionTableEntry; 4],
    valid_bootsector: u16,
}

#[repr(C, packed(2))]
#[derive(Clone, Copy, Debug)]
pub struct PartitionTableEntry {
    drive_attributes: u8,
    parition_start_chs: [u8; 3],
    partition_type: u8,
    partition_last_sector_chs: [u8; 3],
    parition_start_lba: u32,
    num_sectors: u32,
}


impl<R: BlockRead> BlockDevice<R> {
    pub fn new(block_reader: R) -> Result<BlockDevice<R>, BlockErr> {
        // let mut buf = [0u8; 512];
        let mut buf = vec![0u8; 512];
        block_reader.block_read(0, 512, &mut buf)?;
        serial_println!("{:x?}", &buf[446..446+16]);
        let mbr = unsafe {
            *(buf.as_ptr() as *const RawMBR)
        };
        // serial_println!("sig {:#x}", mbr.signature);
            
        Ok(BlockDevice {
            block_reader,
            partition_table: unsafe { mbr.partition_table.clone() },
        })
    }
}
