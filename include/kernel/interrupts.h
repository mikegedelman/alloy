#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H

void register_interrupt_handler(size_t irq_num, void (*fn)(void*));

#endif
