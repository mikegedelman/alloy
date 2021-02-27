C_SRC := idt.c
C_OBJ := idt.o
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
$(C_OBJ): $(C_SRC)
	$(CC) -c $(C_SRC) -std=gnu99 -ffreestanding -Wall -Wextra

$(KERNEL_OBJ): $(KERNEL_SRC)
	cd kernel && \
	cargo build --target x86-mike.json --features verbose && \
	cd ..

test_obj: $(KERNEL_SRC)
	cd kernel && \
	cargo build --target x86-mike.json --features verbose,test && \
	cd ..

# === Multiboot section ===
# Multiboot header
asm_obj: asm/multiboot.asm asm/utils.asm asm/loader.asm
	nasm -g -f elf32 asm/multiboot.asm -o multiboot.o
	nasm -g -f elf32 asm/utils.asm -o utils.o
	nasm -g -f elf32 asm/loader.asm -o loader.o

# Link the kernel into a multiboot format
# Warning: Do *NOT* name this multiboot.bin. When qemu sees multiboot.bin in the current dir,
# weird things seem to happen
multiboot: asm_obj $(KERNEL_OBJ) $(BOOT_OBJ) $(C_OBJ)
	$(CC) -T link/multiboot.ld -o os-multiboot.bin -ffreestanding -nostdlib multiboot.o utils.o loader.o $(C_OBJ) $(KERNEL_OBJ) -lgcc

# TODO: clean up these test targets
multiboot_test: asm_obj test_obj $(BOOT_OBJ) $(C_OBJ)
	$(CC) -T link/multiboot.ld -o test-multiboot.bin -ffreestanding -nostdlib multiboot.o utils.o loader.o $(C_OBJ) $(KERNEL_OBJ) -lgcc

run: multiboot
	qemu-system-i386 -kernel os-multiboot.bin

test: multiboot_test
	qemu-system-i386 -kernel test-multiboot.bin


cd: multiboot
	scripts/iso.sh

bochs: cd
	bochs


# === Custom bootloader section (not used currently) ===
# Build the bootloader
boot.bin: asm/bootloader.asm
	nasm -f bin -o boot.bin asm/bootloader.asm

# Link the kernel into a flat binary
kernel.bin: $(KERNEL_OBJ) kernel32.o
	$(LD) -T link/linker.ld -o kernel.bin $(KERNEL_OBJ)

# Make a floppy image containing our custom bootloader and kernel
# For now, paste the kernel directly after the bootsector (no fs)
custom: boot.bin kernel.bin
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
	dd if=kernel.bin of=floppy.img seek=1 conv=notrunc

run_custom: floppy
	qemu-system-i386 -fda floppy.img
