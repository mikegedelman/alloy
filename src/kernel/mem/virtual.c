#include <stddef.h>
#include <stdint.h>
#include <kernel/mem/physical.h>
#include <kernel/mem/virtual.h>
#include <kernel/cpu.h>
#include <kernel/stdio.h>

// #define BASE_VIRTUAL_ADDRESS 0
#define _4MB (4 * 1024 * 1024)

// lazy_static! {
//     pub static ref VMGR: Mutex<VirtualManager> = Mutex::new(VirtualManager::new());
// }
// lazy_static! {
//     static ref KERNEL_PAGE_DIRECTORY: Mutex<KernelPageDirectory> =
//         Mutex::new(KernelPageDirectory{ entries: [0u32; 1024] });
// }

static uint32_t __attribute__((aligned(4096))) page_directory[1024];
static uint32_t end_of_kernel_virtual;

// static mut KERNEL_PAGE_DIRECTORY: KernelPageDirectory = KernelPageDirectory {
//     entries: [0u32; 1024],
// };

// #[repr(C, align(4096))]
// struct KernelPageDirectory {
//     pub entries: [u32; 1024],
// }



// pub struct VirtualManager {
//     pub end_of_kernel_virtual: u32,
//     page_directory: &'static mut KernelPageDirectory,
// }

// impl VirtualManager {
//     pub fn new() -> VirtualManager {
//         let end_of_kernel_virtual =
//             unsafe { ((&kernel_end as *const u32) as u32 & 0xFFC00000) + _4MB };
//         VirtualManager {
//             end_of_kernel_virtual,
//             page_directory: unsafe { &mut KERNEL_PAGE_DIRECTORY },
//         }
//     }

uint32_t page_directory_entry(uint32_t addr, uint32_t flags) {
    // if (addr % 0x400000) != 0 {
    //     panic!(
    //         "Invalid page directory physical address: must be 4MB-aligned: {:#x}",
    //         addr
    //     );
    // }

    return addr | flags;
}

void alloc_4mb(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    size_t pagedir_num = virtual_addr >> 22;
    // info!(
    //     "Allocating 4mb pagedir at addr {:#x}, physical {:#x}",
    //     virtual_addr, physical_addr
    // );

    page_directory[pagedir_num] = page_directory_entry(physical_addr, flags);
}

void free_4mb(void *virtual_addr) {
    size_t pagedir_num = (uint32_t) virtual_addr >> 22;
    // info!("Freeing 4mb pagedir at addr {:#x}", virtual_addr);

    page_directory[pagedir_num] = 0;
    // invlpg(virtual_addr);
}

void *virtualmem_alloc_page() {
    AllocResult alloc_result = alloc_contiguous_4mb();
    if (alloc_result.status != ALLOC_SUCCESS) {
        return 0;  // TODO maybe return another struct here
    }

    uint32_t physical_addr = alloc_result.addr;
    uint32_t virtual_addr = end_of_kernel_virtual;
    uint32_t flags = PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE;
    alloc_4mb(virtual_addr, physical_addr, flags);
    end_of_kernel_virtual += _4MB;

    return (void*) virtual_addr;
}

void virtualmem_free(void *virtual_addr) {
    free_4mb(virtual_addr);
    end_of_kernel_virtual -= _4MB;
}

/// Return the physical memory address of the kernel page directory
// uint32_t page_dir_addr() {
//     // This approach only works because our page directory lands in the first 4MB of
//     // kernel memory, so we know its specific offset. If we put a page table in a
//     // new page we alloc'ed somewhere else, we'll have to figure out the original
//     // base address for that page and use that for translation.
//     return page_directory - BASE_VIRTUAL_ADDRESS;
// }


/// Initialize the page directory to be used for kernel code
void virtualmem_init() {
    end_of_kernel_virtual = ((uint32_t)&kernel_end & 0xFFC00000) + _4MB;

    size_t kernel_pagedir_num = (BASE_VIRTUAL_ADDRESS >> 22);
    uint32_t flags = PAGEDIR_PRESENT | PAGEDIR_WRITE | PAGEDIR__4M_PAGE;
    page_directory[kernel_pagedir_num] = page_directory_entry(0, flags);

    // let start = (&kernel_start as *const u32) as u32;
    // let end = (&kernel_end as *const u32) as u32;
    uint32_t kernel_size = &kernel_end - &kernel_start;

    if (kernel_size > _4MB) {
        uint32_t addtl_pages = (kernel_size + _4MB - 1) / _4MB;
        // info!("Mapping {} additional pages for the kernel", addtl_pages);
        for (uint32_t i = 0; i < addtl_pages; i++) {
            page_directory[kernel_pagedir_num + i] = page_directory_entry(_4MB * i, flags);
        }
    }

    // Reserve the physical frames containing the page directory
    // This must be a *PHYSICAL* address, so we have translate it from virtual.
    // printf("setting cr3 to %x", (void*) page_directory - BASE_VIRTUAL_ADDRESS);
    set_cr3((void*) page_directory - BASE_VIRTUAL_ADDRESS);
}
