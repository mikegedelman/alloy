bits 32
; section .text
global kernel_main
kernel_main:
    cli
    mov eax, 0xB8000
    mov word [eax], 0x0F42

    jmp $

    ; mov ax, 0xB800
    ; mov gs, ax

    ; mov ax, 0xB800
    ; mov byte [gs:0], 0x42
    ; mov byte [gs:1], 0x0F

    ; jmp $
