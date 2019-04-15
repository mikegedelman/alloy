boot:
    mov ax, 80h    ; Set up 4K stack space after kernel
    add ax, 288       ; (4096 + 512) / 16 bytes per paragraph
    mov ss, ax
    mov sp, 4096

    mov ax, 80h     ; Set data segment to where we're loaded
    mov ds, ax

    call cls
    mov si, hi_msg
    call print_string
    jmp $

hi_msg db 'Hi, I am a kernel', 0

cls:
    pusha
    mov ah, 0x00
    mov al, 0x03  ; text mode 80x25 16 colours
    int 0x10
    popa
    ret

print_string:         ; Routine: output string in SI to screen
    pusha
    mov ah, 0Eh       ; int 10h 'print char' function

.repeat:
    lodsb         ; Get character from string
    cmp al, 0
    je .done      ; If char is zero, end of string
    int 10h           ; Otherwise, print it
    jmp .repeat
.done:
    popa
    ret
