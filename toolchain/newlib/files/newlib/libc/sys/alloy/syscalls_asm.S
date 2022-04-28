
.extern errno

.text
.globl write
write:
write:
    push %ebp
    mov %esp, %ebp
    push 16(%ebp)
    push 12(%ebp)
    push 8(%ebp)
    mov $18, %eax
    int $0x80
    movl %eax, errno
    leave
    ret