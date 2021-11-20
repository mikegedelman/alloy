#define SYSCALL_INT 0x80
#define PRINT_SYSCALL 1
#define EXIT_SYSCALL 2

void _exit() {
    asm volatile (
        "mov %0, %%eax\n\t"
        "int %1\n\t"
        :: "i" (PRINT_SYSCALL),
           "i" (SYSCALL_INT)
    );
}

/// Put the address of s in ebx and then syscall #1 "print"
/// TOOD: A macro for syscalls would be nice
void print(char *s) {
        // asm volatile (
        //     "mov ebx, [{0}]\n\t"
        //     "mov eax, {syscall_num}\n\t"
        //     "int {syscall}\n\t"
        //     :: in(reg) &s,
        //     syscall_num = const 0x1,
        //     syscall = const SYSCALL_INT,
        // );
    asm volatile ( "movl %0, %%ebx\n\t"
                   "movl %1, %%eax\n\t"
                   "int %2"
                   :: "m" (s),
                      "i" (PRINT_SYSCALL),
                      "i" (SYSCALL_INT)
                   : "eax", "ebx" );
}


void main() {
    print("Hello from userspace");
    while(1) {
        asm volatile ("hlt");
    }
}
