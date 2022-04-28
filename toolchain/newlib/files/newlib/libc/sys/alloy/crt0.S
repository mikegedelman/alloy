.text

.extern main
.extern _exit

.globl _start
.type _start, @function
_start:
    sub $64, %esp
    call main
    call _exit