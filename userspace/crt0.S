.text

.globl _exit
.type _exit, @function
_exit:
    mov %eax, 0x2
    int 0x80
    ; unreachable

.globl _start
.type _start, @function
_start:
    sub $64, %esp
    call main
    call _exit