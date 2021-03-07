global _start
_start:
    mov ebx, hello
    mov eax, 0x1 ; "print" syscall - expects pointer to a null-terminated string in ebx
    int 0x80
    mov eax, 0x2 ; "exit" syscall
    int 0x80 ;


hello db 'Hello from fake userspace assembly', 10, 0
