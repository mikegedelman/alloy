pub const PHYSICAL_BITMAP_SZ: usize = 131072; //  0x100000000 / 0x1000 / 8;

extern "C" {
    pub static mut physical_mem_bitmap: [u8; PHYSICAL_BITMAP_SZ];
    pub static kernel_start: u32;
    pub static kernel_end: u32;
}
