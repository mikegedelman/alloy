
.extern errno

.text
.globl __alloy_syscall
__alloy_syscall:
__alloy_syscall:
    push %ebp
    mov %esp, %ebp
    push 24(%ebp)
    push 20(%ebp)
    push 16(%ebp)
    push 12(%ebp)
    mov 8(%ebp), %eax
    int $0x80
    movl 0, errno
    leave
    ret
