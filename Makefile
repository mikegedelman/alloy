# C_SRC := $(shell find c -type f -name "*.c")
# C_OBJ := $(shell find c -type f -name "*.o")
KERNEL_SRC := $(shell find kernel/src -type f -name "*.rs")
KERNEL_OBJ := kernel/target/x86-mike/debug/libros.a
CC := i686-elf-gcc -g
LD := i686-elf-ld

all: multiboot

clean:
	cd kernel && \
	cargo clean && \
	cd ..
	rm -rf *.bin *.o *.img isodir/

# Targets to build general kernel object files
# $(C_OBJ): $(C_SRC)
# 	cd c && \
# 	$(CC) -c ata/ata.c ports.c -std=gnu99 -ffreestanding -Wall -Wextra && \
# 	cd ..

$(KERNEL_OBJ): $(KERNEL_SRC)
	cd kernel && \
	cargo build --target x86-mike.json && \
	cd ..

# === Multiboot section ===
# Multiboot header
multiboot.o: asm/multiboot.asm
	nasm -f elf32 -o multiboot.o asm/multiboot.asm

# Link the kernel into a multiboot format
# Warning: Do *NOT* name this multiboot.bin. When qemu sees multiboot.bin in the current dir,
# weird things seem to happen
multiboot: multiboot.o $(KERNEL_OBJ) $(BOOT_OBJ)
	$(CC) -T link/multiboot.ld -o os-multiboot.bin -ffreestanding -nostdlib multiboot.o $(KERNEL_OBJ) -lgcc

run: multiboot
	qemu-system-i386 -kernel os-multiboot.bin


# === Custom bootloader section ===
# Build the bootloader
boot.bin: asm/bootloader.asm
	nasm -f bin -o boot.bin asm/bootloader.asm

# Link the kernel into a flat binary
kernel.bin: $(KERNEL_OBJ) kernel32.o
	$(LD) -T link/linker.ld -o kernel.bin kernel32.o $(KERNEL_OBJ)

# Code containing _start, which sets up the stack and heap and calls into the main kernel
kernel32.o: asm/kernel32.asm
	nasm -felf32 asm/kernel32.asm -o kernel32.o

# Make a floppy image containing our custom bootloader and kernel
# For now, paste the kernel directly after the bootsector (no fs)
custom: boot.bin kernel.bin
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
	dd if=kernel.bin of=floppy.img seek=1 conv=notrunc

run_custom: floppy
	qemu-system-i386 -fda floppy.img
