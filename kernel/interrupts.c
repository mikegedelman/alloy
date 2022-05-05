#include <kernel/all.h>

typedef void (*interrupt_handler_fn) (void*);

static interrupt_handler_fn registered_interrupts[256];


#define SYSCALL 0x80

enum Syscalls {
    EXIT = 1,
    CLOSE = 2,
    EXECVE = 3,
    FORK = 4,
    FSTAT = 5,
    GETPID = 6,
    ISATTY = 7,
    KILL = 8,
    LINK = 9,
    LSEEK = 10,
    OPEN = 11,
    READ = 12,
    SBRK = 13,
    STAT = 14,
    TIMES = 15,
    UNLINK = 16,
    WAIT = 17,
    WRITE = 18,
    GETTIMEOFDAY = 19
};


void init_interrupts() {
    for (int i = 0; i < 256; i++) {
        registered_interrupts[i] = NULL;
    }
}

void register_interrupt_handler(size_t irq_num, interrupt_handler_fn fn) {
    registered_interrupts[irq_num] = fn;
}

void send_eoi(uint32_t irq) {
    if (irq >= 8) {
        pic2_eoi();
    }

    pic2_eoi();
    pic1_eoi();
}

void isr_handler(uint32_t x, uint32_t info, uint32_t eax, uint32_t ecx, uint32_t edx, uint32_t ebx, uint32_t esp, uint32_t ebp, uint32_t esi, uint32_t edi, uint16_t cs, uint32_t eip)  {
    // term_putchar('?');)
    // char scancode;

    // if (x != 0x20) {
    //     printf("**INTERRUPT %x \n", x);
    // }

    // printf("%x %x %x %x %x\n", eax, ebx, ecx, edx, esp);

    if (x < 32) {
        printf("exception %d error code: %x", x, info);
        outl(0xf4, 0x10);
    }

    if (registered_interrupts[x] != NULL) {
        registered_interrupts[x]((void*) info);
    }

    ProcessCPUState state;
    switch (x) {
        case 32:
            state.eax = eax;
            state.ebx = ebx;
            state.ecx = ecx;
            state.edx = edx;
            state.esp = esp;
            state.ebp = ebp;
            state.esi = esi;
            state.edi = edi;
            state.eip = eip;
            send_eoi(x);
            next_process(&state);
            break;
        case 33:
            // scancode = inb(0x60);
            // serial_write(&com1, "?");
            inb(0x60);
            break;
    }

    send_eoi(x);
}

int _syscall(uint32_t syscall_no, void *a, void *b, void *c, void *d) {
    printf("syscall %d\n", syscall_no);
    switch(syscall_no) {
        case WRITE:
//            printf("called WRITE (%d) with args: %x, %x, %x, %x\n", syscall_no, (int)a, (int)b, (int)c, (int)d);
            io_wait();
            int fd = (int) a;
            char *ptr = (char *)b;
            int len = (int) c;
            if (fd == 1) {
                for (int i = 0; i < len; i++) {
                    putchar(ptr[i]);
                }
            }
            return 0;
        case EXIT:
            printf("exit\n");
            outl(0xf4, 0x10);
            return 0; // unreachable
    }
}
