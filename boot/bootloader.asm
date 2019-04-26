[org 0x7c00]
BITS 16

%define KERNEL_MEM 100000h

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

    xor ax, ax
    mov es, ax
    mov bx, 800h

    mov ah, 02h
    mov al, 16        ; Num sectors to read
    mov ch, 0         ; Cylinder
    mov cl, 2         ; Sector
    mov dh, 0         ; Head

    int 13h

    jnc enable_a20
    mov si, err_msg
    call print_string

enable_a20:
    cli
    ; Alternate A20 way? there are like 10 ways to do this
    ; in al, 0x64
    ; test al, 0x2
    ; jnz enable_a20
    ; mov al, 0xdf
    ; out al, 0x64

    ; "Fast A20"
    in al, 0x92
    or al, 2
    out 0x92, al

enter_protected:
    ; set gs segment to 0, then use that as base for our gdtinfo pointer
    xor bx, bx
    mov gs, bx
    lgdt [gs:gdtinfo]   ; load gdt register

    ; set protected mode bit!
    mov eax, cr0
    or  al, 1
    mov cr0, eax

    ; far jump to clear prefetch
    jmp 0x08:finish_prot
    nop
    nop

finish_prot:
    [bits 32]
    ; set all data segments to 2nd GDT entry
    mov bx, 0x10
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    mov ss, bx

    ; I think we're getting lucky that the first ELF program header is
    ; our text section; we might need to loop through the program headers
    ; and check that they're loadable first.
    ; mov eax, [KERNEL_MEM + 0x1C] ; eax now points to ELF program header tbl
    ; add eax, 4 ; eax now points to offset of first ELF section
    ; mov ebx, [KERNEL_MEM + eax]
    ; add ebx, KERNEL_MEM
 
    ; we can jump directly to ebx because it's still in our code segment
    jmp 0x08:0x800

err_msg db 'An error occurred reading from disk', 0
kernel_start_ptr dw 0

gdtinfo:
   dw gdt_end - gdt   ; limit
   dd gdt             ;start of table

gdt         dd 0,0  ; entry 0 is always unused
            db 0xff, 0xff, 0, 0, 0, 0x9a,      0xcf,      0 ; cs
            db 0xff, 0xff, 0, 0, 0, 0x92,      0xcf,      0 ; ds
gdt_end:
            dd 0

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

boot_sig:
    times 510-($-$$) db 0 ; Pad remainder of boot sector with 0s
    dw 0xAA55     ; The standard PC boot signature
