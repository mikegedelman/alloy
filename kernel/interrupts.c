#include <kernel/all.h>

typedef void (*interrupt_handler_fn) (void*);

#define EOT 4

/// Interrupt handler function table - so we can register custom interrupt handleres
/// from within the kernel
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

/// Register an interrupt handler to be called on interrupt irq_num
void register_interrupt_handler(size_t irq_num, interrupt_handler_fn fn) {
    registered_interrupts[irq_num] = fn;
}

/// Send End-of-Interrupt to the CPU
/// This tells the PIC we're ready for more interrupts.
/// Note that EFLAGS must also be set appropriately to receive
/// interrupts.
/// https://wiki.osdev.org/8259_PIC
void send_eoi(uint32_t irq) {
    // We must notify both the master and slave PIC for irqs >= 8.
    if (irq >= 8) {
        pic2_eoi();
    }

    pic1_eoi();
}

/// System call handler
/// This invoked from our generic interrupt handler and will
/// call the appropriate logic based on syscall_no
int syscall(uint32_t syscall_no, void *a, void *b, void *c, void *_d, ProcessCPUState *cpu_state) {
    if (syscall_no == WRITE) {
        return proc_write((int) a, (char*)b, (int) c);
    } else if (syscall_no == READ) {
        // For now, don't even check if data is already available in the given fd -
        // just block and let the scheduler figure it out.
        block_process(cpu_state, (int) a, (uint32_t) b, (int) c);
        return 0;
    } else if (syscall_no == EXIT) {
        exit_process();
        return 0; // unreachable
    }
}

/// Our main entry point for all interrupts
/// This is called in idt_asm.asm, where we also set up the isr table.
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
    uint32_t _current_esp, // Unused, but pushed anyway by pusha
    uint32_t ebx,
    uint32_t edx,
    uint32_t ecx,
    uint32_t eax,
    uint32_t eip,
    uint32_t _cs,   // Unused
    uint32_t eflags,
    uint32_t esp,
    uint32_t _ds    // Unused
) {
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

    if (x < 32) {
        printf("exception %d error code: %x\n", x, info);
        exit_qemu();
    }

    // If we registered a handler for this interrupt in code elsewhere,
    // then just call that and then return
    // This is probably used mostly by device drivers to handle hardware
    // interrupts. E.g.: the rtl8139 driver uses this.
    if (registered_interrupts[x] != NULL) {
        registered_interrupts[x]((void*) info);
        send_eoi(x);
        return 0;
    }

    if (x == 33) {
        // Keyboard interrupt (IRQ1)
        // TODO: handle keyboard input here - map scancodes to ASCII chars, etc
        // scancode = inb(0x60);
        // serial_write(&com1, "?");
        inb(0x60);
        send_eoi(x);
        return 0;
    } else if (x == 32) {
        // Hardware timer interrupt (IRQ0)
        // Grab the current CPU state and pass it off to the scheduler
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

        // We'll send the EOI now to acknowledge the interrupt, so
        // that the scheduler doesn't have to worry about it
        // This is because we'll probably jump into user code before
        // returning to this function.
        send_eoi(x);
        schedule(&state);
        return 0; // This should be unreachable
    } else if (x == 0x80) {
        // System call
        // We must save the CPU state because the system call cause the
        // calling process to block.
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
        send_eoi(x); // Same reasoning as the hardware timer above for sending EOI now
        return syscall(state.eax, (void*) syscall1, (void*) syscall2, (void*) syscall3, (void*) syscall4, &state);
    } else {
        // All other currently unhandled interrupts
        send_eoi(x);
        return 0;
    }
}
