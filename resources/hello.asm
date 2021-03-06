global _start
section .text
_start:
    mov ebx, hi
    mov eax, 1
    int 0x80
    hlt

hi db 'hi', 0