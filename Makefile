C_FILES := $(shell find . -type f -name "*.c")
ASM_FILES := kernel32.asm
OBJ_FILES := kernel32.o kernel.o term.o

# i386-elf-gcc is a cross compiler that should have been set up on host
CC := i386-elf-gcc

all: floppy

# For now, paste the kernel directly after the bootsector (no fs)
floppy: boot.bin kernel.bin
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
	dd if=kernel.bin of=floppy.img seek=1 conv=notrunc

boot.bin: boot/bootloader.asm
	nasm -f bin -o boot.bin boot/bootloader.asm

$(OBJ_FILES): $(ASM_FILES) $(C_FILES)
	nasm -felf32 $(ASM_FILES)
	i386-elf-gcc -c $(C_FILES) -I./include -std=gnu99 -ffreestanding -O2 -Wall -Wextra  # -Wl,--oformat=binary

kernel.bin: $(OBJ_FILES)
	i386-elf-gcc -T linker.ld -o kernel.bin -ffreestanding -O2 -nostdlib $(OBJ_FILES) -lgcc

clean:
	rm -rf *.o *.bin *.img

# Saving some old make targets just in case

multiboot: $(OBJ_FILES)
	nasm -f elf32 -o multiboot.o boot/multiboot.asm
	i386-elf-gcc -T multiboot.ld -o os-multiboot.bin -ffreestanding -O2 -nostdlib multiboot.o kernel.o term.o -lgcc

# old: boot
# 	nasm -f bin -o kernel.bin kernel32.asm
# 	dd if=/dev/zero of=floppy.img bs=1024 count=1440
# 	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
# 	dd if=kernel.bin of=floppy.img seek=1 conv=notrunc
#
# kernel-boot:
# 	nasm -f bin -o kernel-boot.bin kernel-boot.asm
# 	dd if=/dev/zero of=kernel-floppy.img bs=1024 count=1440
# 	dd if=kernel-boot.bin of=kernel-floppy.img seek=0 count=1 conv=notrunc
