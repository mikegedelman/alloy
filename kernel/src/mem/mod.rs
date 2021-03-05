use bitflags::bitflags;

use crate::cpu;
// use crate::externs::loader::boot_page_directory;

pub mod physical;

const BASE_VIRTUAL_ADDRESS: u32 = 0xC0000000;

static mut kernel_page_directory: [u32; 1024] = [0u32; 1024];

bitflags! {
    pub struct PageDirFlags: u32 {
        const PRESENT = 1;
        const WRITE = 1 << 1;
        const USER = 1 << 2;
        const WRITE_THROUGH = 1 << 3;
        const DISABLE_CACHE = 1 << 4;
        const ACCESSED = 1 << 5;
        // 6: always 0
        const _4M_PAGE = 1 << 7;
    }
}

// impl fmt::Display for PageDirFlags {
//     fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
//         let mut s = "";

//     }
// }

bitflags! {
    struct PageFlags: u32 {
        const PRESENT = 1;
        const WRITE = 1 << 1;
        const USER = 1 << 2;
        const WRITE_THROUGH = 1 << 3;
        const DISABLE_CACHE = 1 << 4;
        const ACCESSED = 1 << 5;
        const DIRTY = 1 << 6;
        // 7: always 0
        const GLOBAL = 1 << 8;
    }
}

// #[repr(C)]
// pub struct PageDirectory {

// }

pub struct Manager {

}

pub fn alloc_page() {
    let frame_addr = physical::alloc();

}

pub fn print_page_directory_entry(entry_num: u32) {
    unsafe {
        // let directory_ptr = addr as *mut [u32; 1024];
        // let directory = *directory_ptr;
        let entry: u32 = kernel_page_directory[entry_num as usize];
        println!("Page dir entry: {:#x}, flags {:#?}", entry & 0xFFFFF000, PageDirFlags::from_bits_truncate(entry));
    }
}

fn page_directory_entry(addr: u32, flags: PageDirFlags) -> u32 {
    if addr & 0x3FFFFF != 0 {
        panic!("Invalid page directory address: must be 4MB-aligned: {:#x}", addr);
    }

    addr & flags.bits()
}

pub fn init_kernel_page_dir() {
    let kernel_pagedir_num = (BASE_VIRTUAL_ADDRESS >> 22) as usize;
    println!("kernel_pagedir_num {:#x}", kernel_pagedir_num);
    let flags = PageDirFlags::PRESENT | PageDirFlags::WRITE | PageDirFlags::_4M_PAGE;
    unsafe {
        kernel_page_directory[kernel_pagedir_num] = page_directory_entry(0, flags);
        kernel_page_directory[kernel_pagedir_num] = page_directory_entry(0x400000, flags);
        kernel_page_directory[kernel_pagedir_num] = page_directory_entry(0x400000*2, flags);
        kernel_page_directory[kernel_pagedir_num] = page_directory_entry(0x400000*3, flags);
        load_page_directory(kernel_page_directory.as_ptr() as u32 - BASE_VIRTUAL_ADDRESS);
    }
}

fn load_page_directory(addr: u32) {
    cpu::set_cr3(addr);
}

// unsafe fn addr_to_page_directory(addr: u32) -> &'static [u32; 1024] {

// }