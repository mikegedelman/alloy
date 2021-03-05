pub const PHYSICAL_BITMAP_SZ: usize = 131072; //  0x100000000 / 0x1000 / 8;
pub const PAGE_DIRECTORY_SZ: usize = 1048576; //

extern "C" {
    pub static mut physical_mem_bitmap: [u8; PHYSICAL_BITMAP_SZ];
    // pub static mut boot_page_directory: [u8; 1024];
}