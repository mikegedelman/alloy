//! Just use phil-opp's linked_list_allocator for now
use linked_list_allocator::LockedHeap;
use log::info;

use super::virt;

#[global_allocator]
static ALLOCATOR: LockedHeap = LockedHeap::empty();

#[alloc_error_handler]
fn alloc_error_handler(layout: alloc::alloc::Layout) -> ! {
    panic!("allocation error: {:?}", layout)
}

const HEAP_SIZE: usize = 12 * 1024 * 1024;

pub unsafe fn init() {
    let first_heap_page_addr = virt::alloc_kernel_page();
    virt::alloc_kernel_page();
    virt::alloc_kernel_page();
    info!(
        "Initializing a 4MB heap starting at {:#x}",
        first_heap_page_addr
    );

    ALLOCATOR
        .lock()
        .init(first_heap_page_addr as usize, HEAP_SIZE);
}
