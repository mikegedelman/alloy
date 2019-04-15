BITS 16

header:
    jmp boot
    nop

OEMLabel                db "LABEL   "       ; Disk label
BytesPerSector          dw 512              ; Bytes per sector
SectorsPerCluster       db 1                ; Sectors per cluster
ReservedForBoot         dw 1                ; Reserved sectors for boot record
NumberOfFats            db 1                ; Number of copies of the FAT
RootDirEntries          dw 0                ; Number of entries in root dir
                                            ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors          dw 2880             ; Number of logical sectors
MediumByte              db 0F0h             ; Medium descriptor byte
SectorsPerFat           dw 9                ; Sectors per FAT
SectorsPerTrack         dw 18               ; Sectors per track (36/cylinder)
Sides                   dw 1                ; Number of sides/heads
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
