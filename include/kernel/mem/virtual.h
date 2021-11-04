#pragma once

#define BASE_VIRTUAL_ADDRESS 0xC0000000

void virtualmem_init();
void *virtualmem_alloc_page();
void virtualmem_free(void *virtual_addr);

extern void *kernel_start;
extern void *kernel_end;
