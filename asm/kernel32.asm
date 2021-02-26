bits 32

section .text
global _start:function (_start.end - _start)
_start:
    mov esp, stack_top

    extern kernel_main
    call kernel_main

    cli
.hang:
    hlt
    jmp .hang
.end:


section .bss align=16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

