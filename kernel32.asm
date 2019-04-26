bits 32

section .bss align=16
stack_bottom:
resb 16384 ; 16 KiB
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    ; mov word [gs:0xB8000], 0x0F42

    mov esp, stack_top

    extern kernel_main
    call kernel_main

    cli
.hang:
    hlt
    jmp .hang
.end:
