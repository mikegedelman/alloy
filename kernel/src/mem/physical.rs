use lazy_static::lazy_static;
use log::info;
use spin::Mutex;

use super::virt::BASE_VIRTUAL_ADDRESS;
use crate::externs::loader::{kernel_end, kernel_start};
use crate::multiboot::MultibootInfo;

const _4MB: u32 = 4*1024*1024;
const PHYSICAL_BITMAP_SZ: usize = 131_072;

// Assume all memory is reserved until we find out otherwise from multiboot.
// This might be problematic, but we're erring on the safe side.
// This is a static because we want it stored in bss somewhere, not on the stack,
// because it's pretty big.
// Static mut is bad, but it's just passed in to the physical manager struct,
// which is properly mutexted
static mut _BITMAP_INTERNAL: [u8; PHYSICAL_BITMAP_SZ] = [0xFFu8; PHYSICAL_BITMAP_SZ];

lazy_static! {
    static ref PMGR: Mutex<PhysicalManager> =
        Mutex::new(PhysicalManager::new(unsafe { &mut _BITMAP_INTERNAL }));
}


/// Calculate the frame num (ie, index in our bitmap) for the given address
fn frame_num_for_addr(addr: u32) -> usize {
    ((addr & 0xFFFFF000) / 0x1000) as usize
}

/// Use the "bit scan forward" instruction to find the first nonzero bit
pub fn first_free_bit(byte: u8) -> u8 {
    let output: u32;
    unsafe {
        asm!(
            "bsf {0}, {1}",
            out(reg) output,
            // Notice the ! - we want the first *0*, but bsf gives you the first 1,
            // so invert first
            in(reg) !byte as u32,
        );
    }
    output as u8
}

/// Use a bitmap to remember which physical frames are free
/// Note that this supports 4KB pages, but our kernel's virtual memory
/// logic is currently dumb and allocates only at the 4MB level.
///
/// 1 bit per 4KB page. 1024 * 1024 * 1024 / 4 * 1024 ->
/// 1048576 frames/bits
/// divide by 8 --> 131072 bytes (128KB)
struct PhysicalManager {
    // 0: free
    // 1: reserved
    bitmap: &'static mut [u8; PHYSICAL_BITMAP_SZ], // *1024],
    num_free: u32,
}

impl PhysicalManager {
    /// Create a new PhysicalManager
    pub fn new(bitmap: &'static mut [u8; PHYSICAL_BITMAP_SZ]) -> PhysicalManager {
        PhysicalManager {
            bitmap,
            num_free: 1048576,
        }
    }

    /// Using the memory map given from multiboot, indicate which physical memory is
    /// free in our bitmap
    pub fn load_multiboot_mmap(&mut self, mb_info: &MultibootInfo) {
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

    /// Pull in the kernel_start and kernel_end addresses, remembering that they are virtual
    /// addresses instead of physical, and use them to mark the underlying physical frames
    /// as reserved
    fn reserve_kernel_frames(&mut self) {
        unsafe {
            let start = (&kernel_start as *const u32) as u32 - BASE_VIRTUAL_ADDRESS;
            let end = (&kernel_end as *const u32) as u32 - BASE_VIRTUAL_ADDRESS;
            info!("Kernel physical address range: {:#x} - {:#x}", start, end);
            self.reserve_range(start, end);
        }
    }

    /// Free the physical frames spanning the given addresses
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

    /// Return the number of free physical frames, which, when multiplied by 4096,
    /// will indicate the amoutn of free memory in the system in bytes
    pub fn num_free_frames(&self) -> u32 {
        return self.num_free;
    }

    /// Reserve the physical frames underlying the given range of physical addresses
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

    /// Mark the frame at frame_num as free
    pub fn free(&mut self, frame_num: usize) {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        self.bitmap[bitmap_byte] = self.bitmap[bitmap_byte] & !(1 << bitmap_bit);
        self.num_free += 1;
    }

    /// Mark the frame at frame_num as reserved
    pub fn reserve(&mut self, frame_num: usize) {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        self.bitmap[bitmap_byte] = self.bitmap[bitmap_byte] | (1 << bitmap_bit);
        self.num_free -= 1;
    }

    /// Test whether the frame at frame_num is reserved
    pub fn is_free(&self, frame_num: usize) -> bool {
        let bitmap_byte = frame_num / 8;
        let bitmap_bit = frame_num % 8;
        (self.bitmap[bitmap_byte] & (1 << bitmap_bit)) != 0
    }
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

/// This kernel is lazy and only allocates space 4MB at a time. This is extremely memory
/// inefficient, but it's a lot easier to implement. Since our physical memory manager
/// was written with 4KB granularity, we'll have to do a little math to find a contiuous
/// 4MB chunk
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
