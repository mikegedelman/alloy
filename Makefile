all: floppy

floppy: boot kernel
	dd if=/dev/zero of=floppy.img bs=1024 count=1440
	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
	dd if=kernel.bin of=floppy.img seek=1 count=1 conv=notrunc

boot: boot.asm
	nasm -f bin -o boot.bin boot.asm

kernel: kernel.asm
	nasm -f bin -o kernel.bin kernel.asm

kernel-boot:
	nasm -f bin -o kernel-boot.bin kernel-boot.asm
	dd if=/dev/zero of=kernel-floppy.img bs=1024 count=1440
	dd if=kernel-boot.bin of=kernel-floppy.img seek=0 count=1 conv=notrunc

clean:
	rm -rf kernel.bin kernel-boot.bin floppy.img kernel-floppy.img


# boot: boot.s
# 	i386-elf-as boot.s -o boot.o
# 
# kernel: kernel.c
# 	i386-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
# 
# link: boot.o kernel.o
# 	i386-elf-gcc -T linker.ld -o os.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
# 
# img: os.bin
# 	mkdir -p isodir/boot/grub && \
# 	cp os.bin isodir/boot/os.bin && \
# 	cp grub.cfg isodir/boot/grub/grub.cfg && \
# 	grub-mkrescue -o os.iso isodir
# 
# clean:
# 	rm -rf boot.o kernel.o os.bin isodir
# 
# raw:
# 	nasm -f bin -o boot.bin boot.asm
# 	nasm -f bin -o kernel.bin kernel-bare.asm
# 	dd if=/dev/zero of=floppy.img bs=1024 count=1440
# 	dd if=boot.bin of=floppy.img seek=0 count=1 conv=notrunc
# 	dd if=kernel.bin of=floppy.img seek=1 count=1 conv=notrunc
# 	cp floppy.img iso
# 	genisoimage -quiet -V 'MYOS' -input-charset iso8859-1 -o myos.iso -b floppy.img -hide floppy.img iso/
