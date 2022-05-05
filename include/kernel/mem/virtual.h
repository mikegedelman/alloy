#ifndef _KERNEL_MEM_VIRTUAL_H
#define _KERNEL_MEM_VIRTUAL_H

#define BASE_VIRTUAL_ADDRESS 0xC0000000

enum PageDirFlags {
        PAGEDIR_PRESENT = 1,
        PAGEDIR_WRITE = 1 << 1,
        PAGEDIR_USER = 1 << 2,
        PAGEDIR_WRITE_THROUGH = 1 << 3,
        PAGEDIR_DISABLE_CACHE = 1 << 4,
        PAGEDIR_ACCESSED = 1 << 5,
        // 6: always 0
        PAGEDIR__4M_PAGE = 1 << 7,
};

enum PageFlags {
        PAGE_PRESENT = 1,
        PAGE_WRITE = 1 << 1,
        PAGE_USER = 1 << 2,
        PAGE_WRITE_THROUGH = 1 << 3,
        PAGE_DISABLE_CACHE = 1 << 4,
        PAGE_ACCESSED = 1 << 5,
        PAGE_DIRTY = 1 << 6,
        // 7: always 0
        PAGE_GLOBAL = 1 << 8,
};

void virtualmem_init();
void *virtualmem_alloc_page();
void virtualmem_free(void *virtual_addr);
void alloc_4mb(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
void virtualmem_map(uint32_t physical, uint32_t virtual, uint32_t flags);
void virtualmem_map_kernel(uint32_t *page_directory);

extern void *kernel_start;
extern void *kernel_end;

#endif
