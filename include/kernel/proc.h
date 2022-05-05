#ifndef _KERNEL_PROC_H
#define _KERNEL_PROC_H

#include <stdint.h>

typedef uint32_t pid_t;

typedef struct {
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
} ProcessCPUState;

pid_t schedule_process(uint8_t *buf);
void next_process(ProcessCPUState *state);

#endif