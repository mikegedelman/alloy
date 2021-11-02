C_FILES := $(shell find src -type f -name "*.c")
ASM_FILES := $(shell find src -type f -name "*.asm")
OBJ_FILES := $(shell find . -type f -name "*.o")

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master

CC := $(DOCKER) i686-elf-gcc -g

all: multiboot

asm: $(ASM_FILES)
	nasm -f elf32 -o multiboot_asm.o src/multiboot.asm
	nasm -f elf32 -o idt_asm.o src/arch/x86/idt.asm

compile: asm $(C_FILES)
	$(CC) -c $(C_FILES) -I./include -std=gnu11 -ffreestanding -O2 -Wall -Wextra  # -Wl,--oformat=binary

compile-test: asm $(C_FILES)
	$(CC) -DTEST -c $(C_FILES) -I./include -std=gnu11 -ffreestanding -O2 -Wall -Wextra  # -Wl,--oformat=binary

clean:
	rm -rf *.o *.bin *.img *.s

multiboot: compile
	$(CC) -T multiboot.ld -o os-multiboot.bin -ffreestanding -O2 -nostdlib $(shell find . -type f -name "*.o") -lgcc

multiboot-test: compile-test
	$(CC) -T multiboot.ld -o os-multiboot-test.bin -ffreestanding -O2 -nostdlib $(shell find . -type f -name "*.o") -lgcc

run: multiboot
	qemu-system-i386 -kernel os-multiboot.bin -serial stdio

test: multiboot-test
	qemu-system-i386 -kernel os-multiboot-test.bin -serial stdio