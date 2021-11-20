.text

.globl _start
.type _start, @function
_start:
    sub $64, %esp
    call main
