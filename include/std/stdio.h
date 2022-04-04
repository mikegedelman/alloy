#ifndef _KERNEL_STDIO_H
#define _KERNEL_STDIO_H

#define SYSCALL 0x80
#define PRINT_SYSCALL 1

void print(const char *s) {
    asm volatile ( "movl %0, %%ebx\n\t"
    	           "movl %1, %%eax\n\t"
    	           "int %2"
                   : 
                   : "r" (s), "i" (PRINT_SYSCALL), "i" (SYSCALL));
}

#endif
