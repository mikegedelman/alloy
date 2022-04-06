.text

.globl _exit
.type _exit, @function
_exit:
    mov $2, %eax
    int $128

.globl _start
.type _start, @function
_start:
    sub $64, %esp
    call main
    call _exit