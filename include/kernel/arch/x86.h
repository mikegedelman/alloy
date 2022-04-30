#ifndef _KERNEL_ARCH_X86_H
#define _KERNEL_ARCH_X86_H

void init_gdt();
void init_idt();

void save_kernel_stack(uint32_t esp);

#endif