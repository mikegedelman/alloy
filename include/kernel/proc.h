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
void timer_schedule(ProcessCPUState *cpu_state);
void block_process(ProcessCPUState *cpu_state, int fd, uint32_t read_ptr, int read_bytes);
void begin_scheduling();
void exit_process();
int proc_write(int fd, char *buf, int len);

#endif
