#include <kernel/all.h>

typedef void (*interrupt_handler_fn) (void*);

#define EOT 4

static interrupt_handler_fn registered_interrupts[256];
static uint8_t *write_buffer;
static size_t write_buffer_size = 1024 * 1024; // 1MB
static bool write_buffer_dirty;


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

    pic1_eoi();
}

uint32_t isr_handler(
    uint32_t x,
    uint32_t info,
    uint32_t syscall1,
    uint32_t syscall2,
    uint32_t syscall3,
    uint32_t syscall4,
    uint32_t edi,
    uint32_t esi,
    uint32_t ebp,
    uint32_t _current_esp,
    uint32_t ebx,
    uint32_t edx,
    uint32_t ecx,
    uint32_t eax,
    uint32_t eip,
    uint32_t cs,
    uint32_t eflags,
    uint32_t esp,
    uint32_t ds
) {
    ProcessCPUState state;
    state.eax = eax;
    state.ebx = ebx;
    state.ecx = ecx;
    state.edx = edx;
    state.esp = esp;
    state.ebp = ebp;
    state.esi = esi;
    state.edi = edi;
    state.eip = eip;
    state.eflags = eflags;
    // term_putchar('?');)
    // char scancode;

    // printf("**INTERRUPT %x\n", x);
    // printf("**INTERRUPT %x - info,syscalls,eax,ebx...: %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x, %x\n",
    //     x,
    //     info,
    //     syscall1,
    //     syscall2,
    //     syscall3,
    //     syscall4,
    //     eax,
    //     ebx,
    //     ecx,
    //     edx,
    //     esp,
    //     ebp,
    //     esi,
    //     edi,
    //     eip
    // );

    // printf("%x %x %x %x %x\n", eax, ebx, ecx, edx, esp);

    if (x < 32) {
        printf("exception %d error code: %x\n", x, info);
        outl(0xf4, 0x10);
    }

    if (registered_interrupts[x] != NULL) {
        registered_interrupts[x]((void*) info);
    }


    switch (x) {
        case 32:
            send_eoi(x);
            timer_schedule(&state);
            break;
        case 33:
            // scancode = inb(0x60);
            // serial_write(&com1, "?");
            inb(0x60);
            break;
        case 0x80:
            send_eoi(x);
            return _syscall(state.eax, (void*) syscall1, (void*) syscall2, (void*) syscall3, (void*) syscall4, &state);
    }

    send_eoi(x);
    return 0;
}

int _syscall(uint32_t syscall_no, void *a, void *b, void *c, void *d, ProcessCPUState *cpu_state) {
    // printf("syscall %d\n", syscall_no);
    if (syscall_no == WRITE) {
        // printf("called WRITE (%d) with args: %x, %x, %x, %x\n", syscall_no, (int)a, (int)b, (int)c, (int)d);
        int fd = (int) a;
        char *ptr = (char *)b;
        int len = (int) c;
        return proc_write(fd, ptr, len);
    } else if (syscall_no == READ) {
        int fd = (int) a;
        uint8_t *ptr = (uint8_t*) b;
        int len = (int) c;

        printf("called READ (%d) with args: %x, %x, %x, %x\n", syscall_no, (int)a, (int)b, (int)c, (int)d);
        if (write_buffer_dirty) {
            for (size_t i = 0; i < len; i++) {
                if (write_buffer[i] == EOT) {
                    break;
                }
                *ptr = write_buffer[i];
            }
            write_buffer_dirty = false;
        } else {
            // void block_process(ProcessCPUState *cpu_state, int fd, uint32_t read_ptr, int read_bytes)
            printf("block_process fd ptr %x %x\n", fd, ptr);
            block_process(cpu_state, fd, (uint32_t) ptr, len);
        }
        return 0;
    } else if (syscall_no == EXIT) {
        exit_process();
        return 0; // unreachable
    }
}
