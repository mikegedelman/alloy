all: floppy

# For now, paste the kernel directly after the bootsector (no fs)
floppy: boot kernel
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
	dd if=os.bin of=floppy.img seek=1 conv=notrunc

boot: bootloader.asm
	nasm -f bin -o boot.bin bootloader.asm

# i386-elf-gcc is a cross compiler that should have been set up on host
kernel: kernel32.asm
	nasm -f elf32 -o kernel32_asm.o kernel32.asm
	i386-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i386-elf-gcc -T linker.ld -o os.bin -ffreestanding -O2 -nostdlib kernel32_asm.o kernel.o -lgcc

clean:
	rm -rf *.o *.bin *.img


# Saving some old make targets just in case

multiboot: kernel
	nasm -f elf32 -o multiboot.o multiboot.asm
	i386-elf-gcc -T linker.ld -o os-multiboot.bin -ffreestanding -O2 -nostdlib multiboot.o kernel.o -lgcc

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
