#pragma once

void virtualmem_init();
void *virtualmem_alloc_page();
void virtualmem_free(void *virtual_addr);

extern uint32_t kernel_start;
extern uint32_t kernel_end;
