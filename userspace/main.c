#define SYSCALL_INT 0x80
#define PRINT_SYSCALL 1
#define EXIT_SYSCALL 2


void main() {
    print("Hello from userspace");
    while(1) {
        asm volatile ("hlt");
    }
}
