BITS 16

header:
    jmp boot
    nop

OEMLabel                db "LABEL   "       ; Disk label
BytesPerSector          dw 512              ; Bytes per sector
SectorsPerCluster       db 1                ; Sectors per cluster
ReservedForBoot         dw 1                ; Reserved sectors for boot record
NumberOfFats            db 2                ; Number of copies of the FAT
RootDirEntries          dw 0                ; Number of entries in root dir
                                            ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors          dw 2880             ; Number of logical sectors
MediumByte              db 0F0h             ; Medium descriptor byte
SectorsPerFat           dw 9                ; Sectors per FAT
SectorsPerTrack         dw 18               ; Sectors per track (36/cylinder)
Sides                   dw 2                ; Number of sides/heads
HiddenSectors           dd 0                ; Number of hidden sectors
LargeSectors            dd 0                ; Number of LBA sectors
DriveNo                 dw 0                ; Drive No: 0
Signature               db 41               ; Drive signature: 41 for floppy
VolumeID                dd 00000000h        ; Volume ID: any number
VolumeLabel             db "OS         "    ; Volume Label: any 11 chars
FileSystem              db "FAT12   "       ; File system type: don't change!

boot:
    mov ax, 07C0h     ; Set up 4K stack space after this bootloader
    add ax, 288       ; (4096 + 512) / 16 bytes per paragraph
    mov ss, ax
    mov sp, 4096

    mov ax, 07C0h     ; Set data segment to where we're loaded
    mov ds, ax
    ; xor ax, ax        ; not sure if setting DS=0 is better than above
    ; mov ds, ax

    xor ax, ax
    mov es, ax
    mov bx, 800h

    mov ah, 02h
    mov al, 1         ; Num sectors to read
    mov ch, 0         ; Cylinder
    mov cl, 2         ; Sector
    mov dh, 0         ; Head
    ; mov dl, 0         ; Drive (0 is floppy)

    int 13h

    ; mov bx, 800h  ; alternate way to inspect memory, using
    ; mov dx, [bx]  ; info registers
    ; jmp $

    jnc goto_kernel
    mov si, err_msg
    call print_string

goto_kernel:
    ; Switch to protected mode and then jump to kernel
;     cli
;     mov eax, DATASEL16
;     mov ds, eax
; 	mov es, eax
; 	mov fs, eax
; 	mov gs, eax
; 	mov ss, eax
; 
;     mov eax, cr0
;     mov [savcr0], eax
;     and eax, 0x7FFFFFF
;     mov cr0, eax
;     
;     jmp 0:finish_prot_mode
; 
; finish_prot_mode:
;     mov sp, 0x8000		; pick a stack pointer.
; 	mov ax, 0		; Reset segment registers to 0.
; 	mov ds, ax
; 	mov es, ax
; 	mov fs, ax
; 	mov gs, ax
; 	mov ss, ax
; 	lidt [idt_real]
; 	sti

    jmp 0:800h


; savcr0 dd 0
err_msg db 'An error occurred reading from disk (int 13h)', 0

print_string:         ; Routine: output string in SI to screen
    mov ah, 0Eh       ; int 10h 'print char' function

.repeat:
    lodsb         ; Get character from string
    cmp al, 0
    je .done      ; If char is zero, end of string
    int 10h           ; Otherwise, print it
    jmp .repeat
.done:
    ret

    times 510-($-$$) db 0 ; Pad remainder of boot sector with 0s
    dw 0xAA55     ; The standard PC boot signature
