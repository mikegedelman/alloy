use lazy_static::lazy_static;
use log::info;
use spin::Mutex;

use super::virt::{BASE_VIRTUAL_ADDRESS, _4MB};
use crate::externs::loader::{kernel_end, kernel_start, physical_mem_bitmap, PHYSICAL_BITMAP_SZ};
use crate::multiboot::MultibootInfo;

lazy_static! {
    static ref PMGR: Mutex<PhysicalManager> =
        Mutex::new(PhysicalManager::new(unsafe { &mut physical_mem_bitmap }));
}

// 1 bit per 4KB page. 1024 * 1024 * 1024 / 4 * 1024 ->
// 1048576 frames/bits
// divide by 8 --> 131072 bytes (128KB)
struct PhysicalManager {
    // 0: free
    // 1: reserved
    bitmap: &'static mut [u8; PHYSICAL_BITMAP_SZ], // *1024],
    num_free: u32,
}

fn frame_num_for_addr(addr: u32) -> usize {
    ((addr & 0xFFFFF000) / 0x1000) as usize
}

pub fn first_free_bit(byte: u8) -> u8 {
    let output: u32;
    unsafe {
        asm!(
            "bsf {0}, {1}",
            out(reg) output,
            in(reg) !byte as u32,
        );
    }
    output as u8
}

impl PhysicalManager {
    pub fn new(bitmap: &'static mut [u8; PHYSICAL_BITMAP_SZ]) -> PhysicalManager {
        PhysicalManager {
            bitmap, // : *bitmap,
            num_free: 1048576,
        }
    }

    pub fn load_multiboot_mmap(&mut self, mb_info: &MultibootInfo) {
        // Assume all memory is reserved until we find out otherwise from multiboot.
        // This might be problematic, but we're erring on the safe side.
        for i in 0..PHYSICAL_BITMAP_SZ {
            self.bitmap[i] = 0xFF;
        }
        self.num_free = 0;

        // Multiboot *says* it's guaranteed to give you all available RAM via mmap
        for mmap_entry in mb_info.iter_mmap() {
            if mmap_entry.type_ == 1 {
                // We'll ignore type >1 - just free the stuff that mmap says is free
                self.free_range(
                    mmap_entry.addr as u32,
                    mmap_entry.addr as u32 + mmap_entry.len as u32,
                )
            }
        }
        self.reserve_kernel_frames();
    }

    fn reserve_kernel_frames(&mut self) {
        unsafe {
            let start = (&kernel_start as *const u32) as u32 - BASE_VIRTUAL_ADDRESS;
            let end = (&kernel_end as *const u32) as u32 - BASE_VIRTUAL_ADDRESS;
            info!("Kernel physical address range: {:#x} - {:#x}", start, end);
            self.reserve_range(start, end);
        }
    }

    pub fn free_range(&mut self, lo_address: u32, hi_address: u32) {
        let lo_frame_num = frame_num_for_addr(lo_address);
        let hi_frame_num = frame_num_for_addr(hi_address);

        let lo_frame_byte = lo_frame_num / 8;
        let lo_frame_bit = lo_frame_num % 8;
        self.bitmap[lo_frame_byte] = self.bitmap[lo_frame_byte] & (0xFF << lo_frame_bit);

        let hi_frame_byte = hi_frame_num / 8;
        let hi_frame_bit = hi_frame_num % 8;
        self.bitmap[hi_frame_byte] = self.bitmap[hi_frame_byte] & (0xFF >> hi_frame_bit);

        for frame_byte in lo_frame_byte + 1..hi_frame_byte {
            self.bitmap[frame_byte] = 0;
        }
        self.num_free += hi_frame_num as u32 - lo_frame_num as u32;
    }

    pub fn num_free_frames(&self) -> u32 {
        return self.num_free;
    }

    pub fn reserve_range(&mut self, lo_address: u32, hi_address: u32) {
        let lo_frame_num = frame_num_for_addr(lo_address);
        let hi_frame_num = frame_num_for_addr(hi_address);

        let lo_frame_byte = lo_frame_num / 8;
        let lo_frame_bit = lo_frame_num % 8;
        self.bitmap[lo_frame_byte] = self.bitmap[lo_frame_byte] | (0xFF << lo_frame_bit);

        let hi_frame_byte = hi_frame_num / 8;
        let hi_frame_bit = hi_frame_num % 8;
        self.bitmap[hi_frame_byte] = self.bitmap[hi_frame_byte] | (0xFF >> hi_frame_bit);

        for frame_byte in lo_frame_byte + 1..hi_frame_byte {
            self.bitmap[frame_byte] = 0xFF;
        }
        self.num_free -= hi_frame_num as u32 - lo_frame_num as u32;
    }

    pub fn free(&mut self, frame_num: usize) {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        self.bitmap[bitmap_byte] = self.bitmap[bitmap_byte] & !(1 << bitmap_bit);
        self.num_free += 1;
    }

    pub fn reserve(&mut self, frame_num: usize) {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        self.bitmap[bitmap_byte] = self.bitmap[bitmap_byte] | (1 << bitmap_bit);
        self.num_free -= 1;
    }

    pub fn is_free(&self, frame_num: usize) -> bool {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        (self.bitmap[bitmap_byte] & (1 << bitmap_bit)) != 0
    }

    // pub fn get_bitmap(&self) -> &'static [u8; PHYSICAL_BITMAP_SZ] {
    //     self.bitmap
    // }
}

pub fn load_multiboot_mmap(mb_info: &MultibootInfo) {
    let mut pmgr = PMGR.lock();
    pmgr.load_multiboot_mmap(mb_info);
}

pub fn num_free_frames() -> u32 {
    let pmgr = PMGR.lock();
    pmgr.num_free_frames()
}

/// Reserve a frame for use; return its starting address
/// Currently O(n). Not a huge deal but would be a problem if we were in 64-bit space.
pub fn alloc() -> Option<u32> {
    let mut pmgr = PMGR.lock();
    let byte_with_free_frame_idx = match pmgr.bitmap.iter().position(|x| x < &0xFF) {
        Some(x) => x,
        None => {
            return None;
        }
    } as u32;

    let byte_with_free_frame = pmgr.bitmap[byte_with_free_frame_idx as usize];
    let free_bit = first_free_bit(byte_with_free_frame as u8);
    let frame: u32 = byte_with_free_frame_idx * 8 + free_bit as u32;
    pmgr.reserve(frame as usize);
    Some(frame * 0x1000)
}

pub fn alloc_contiguous_4mb() -> Option<u32> {
    let mut pmgr = PMGR.lock();
    for _4mb_frame in 0..1023 {
        let mut used = false;
        for idx in _4mb_frame * 128..(_4mb_frame + 1) * 128 {
            if pmgr.bitmap[idx] != 0 {
                used = true;
                break;
            }
        }
        if !used {
            let addr = (_4mb_frame * 1024 * 0x1000) as u32;
            info!(
                "Reserving physical memory range {:#x} - {:#x}",
                addr,
                addr + _4MB
            );
            pmgr.reserve_range(addr, addr + _4MB);
            return Some(addr);
        }
    }
    None
}
