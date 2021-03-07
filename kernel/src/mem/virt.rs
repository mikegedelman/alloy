use bitflags::bitflags;
use lazy_static::lazy_static;
use log::info;
use spin::Mutex;

use super::physical;
use crate::cpu;
use crate::externs::loader::{kernel_end, kernel_start};

pub const BASE_VIRTUAL_ADDRESS: u32 = 0xC0000000;
pub const _4MB: u32 = 4 * 1024 * 1024;

lazy_static! {
    pub static ref VMGR: Mutex<VirtualManager> = Mutex::new(VirtualManager::new());
}
// lazy_static! {
//     static ref KERNEL_PAGE_DIRECTORY: Mutex<KernelPageDirectory> =
//         Mutex::new(KernelPageDirectory{ entries: [0u32; 1024] });
// }

static mut KERNEL_PAGE_DIRECTORY: KernelPageDirectory = KernelPageDirectory {
    entries: [0u32; 1024],
};

#[repr(C, align(4096))]
struct KernelPageDirectory {
    pub entries: [u32; 1024],
}

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

pub struct VirtualManager {
    pub end_of_kernel_virtual: u32,
    page_directory: &'static mut KernelPageDirectory,
}

impl VirtualManager {
    pub fn new() -> VirtualManager {
        let end_of_kernel_virtual =
            unsafe { ((&kernel_end as *const u32) as u32 & 0xFFC00000) + _4MB };
        VirtualManager {
            end_of_kernel_virtual,
            page_directory: unsafe { &mut KERNEL_PAGE_DIRECTORY },
        }
    }

    pub fn alloc_kernel_page(&mut self) -> u32 {
        let physical_addr = physical::alloc_contiguous_4mb().unwrap(); // TODO
        let virtual_addr = self.end_of_kernel_virtual;
        let flags = PageDirFlags::PRESENT | PageDirFlags::WRITE | PageDirFlags::_4M_PAGE;

        VirtualManager::alloc_4mb(virtual_addr, physical_addr, flags);
        self.end_of_kernel_virtual += _4MB;
        virtual_addr
    }

    pub fn free_kernel_page(&mut self, virtual_addr: u32) {
        VirtualManager::free_4mb(virtual_addr);
        self.end_of_kernel_virtual -= _4MB;
    }

    /// Return the physical memory address of the kernel page directory
    pub fn page_dir_addr(&self) -> u32 {
        // This approach only works because our page directory lands in the first 4MB of
        // kernel memory, so we know its specific offset. If we put a page table in a
        // new page we alloc'ed somewhere else, we'll have to figure out the original
        // base address for that page and use that for translation.
        self.page_directory.entries.as_ptr() as u32 - BASE_VIRTUAL_ADDRESS
    }

    pub fn alloc_4mb(virtual_addr: u32, physical_addr: u32, flags: PageDirFlags) {
        let pagedir_num = virtual_addr >> 22;
        info!(
            "Allocating 4mb pagedir at addr {:#x}, physical {:#x}",
            virtual_addr, physical_addr
        );

        unsafe {
            // let mut pagedir = KERNEL_PAGE_DIRECTORY.lock();
            KERNEL_PAGE_DIRECTORY.entries[pagedir_num as usize] =
                page_directory_entry(physical_addr, flags);
        }
    }

    fn free_4mb(virtual_addr: u32) {
        let pagedir_num = virtual_addr >> 22;
        info!("Freeing 4mb pagedir at addr {:#x}", virtual_addr);

        unsafe {
            // let mut pagedir = KERNEL_PAGE_DIRECTORY.lock();
            KERNEL_PAGE_DIRECTORY.entries[pagedir_num as usize] = 0;
        }
    }

    fn write_entry(&mut self, idx: usize, entry: u32) {
        self.page_directory.entries[idx] = entry;
    }

    fn get_entry(&self, idx: usize) -> u32 {
        // assert!(idx < 1024, "Invalid page directory index: {:#x}", idx);
        self.page_directory.entries[idx]
    }

    pub fn print_page_directory_entry(&self, idx: u32) {
        let entry: u32 = self.get_entry(idx as usize);
        println!(
            "Page dir entry: {:#x}, flags {:#?}",
            entry & 0xFFFFF000,
            PageDirFlags::from_bits_truncate(entry)
        );
    }
}

fn page_directory_entry(addr: u32, flags: PageDirFlags) -> u32 {
    if (addr % 0x400000) != 0 {
        panic!(
            "Invalid page directory physical address: must be 4MB-aligned: {:#x}",
            addr
        );
    }

    addr | flags.bits()
}

unsafe fn load_page_directory(addr: u32) {
    info!("Setting cr3 to {:#x}", addr);
    cpu::set_cr3(addr);
}

unsafe fn invlpg(addr: u32) {
    cpu::invlpg(addr);
}

/// Initialize the page directory to be used for kernel code
pub unsafe fn init_kernel_page_dir() {
    let kernel_pagedir_num = (BASE_VIRTUAL_ADDRESS >> 22) as usize;
    let flags = PageDirFlags::PRESENT | PageDirFlags::WRITE | PageDirFlags::_4M_PAGE;
    // let mut pagedir = KERNEL_PAGE_DIRECTORY.lock();
    let mut vmgr = VMGR.lock();
    vmgr.write_entry(kernel_pagedir_num, page_directory_entry(0, flags));

    let start = (&kernel_start as *const u32) as u32;
    let end = (&kernel_end as *const u32) as u32;
    let kernel_size = end - start;

    if kernel_size > _4MB {
        let addtl_pages = (kernel_size + _4MB - 1) / _4MB;
        info!("Mapping {} additional pages for the kernel", addtl_pages);
        for i in 0..addtl_pages {
            let idx = kernel_pagedir_num + i as usize;
            vmgr.write_entry(idx, page_directory_entry(_4MB * i, flags));
        }
    }

    // Reserve the physical frames containing the page directory
    load_page_directory(vmgr.page_dir_addr());

}

pub fn alloc_kernel_page() -> u32 {
    let mut vmgr = VMGR.lock();
    vmgr.alloc_kernel_page()
}

pub fn free_kernel_page(addr: u32) {
    let mut vmgr = VMGR.lock();
    vmgr.free_kernel_page(addr);
}
