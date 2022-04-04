VPATH := $(shell find src -type d)
C_OBJ := $(shell find src -type f -name "*.c" | xargs basename -s .c | xargs -I '{}' echo '{}.o')
ASM_OBJ := $(shell find src -type f -name "*.asm" | xargs basename -s .asm | xargs -I '{}' echo '{}.o')
OBJECTS := $(C_OBJ) $(ASM_OBJ) # $(GO_OBJ)

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master

CC := i686-elf-gcc -g
CFLAGS := -I./include -std=gnu11 -ffreestanding -O -Wall -Wextra

all: os-multiboot.bin

os-multiboot.bin: $(OBJECTS)
	$(CC) -T multiboot-hh.ld -o $@ -ffreestanding -O -nostdlib $(OBJECTS) -lgcc

%.o: %.asm
	nasm -f elf32 -o $@ $^

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $^

# %.o: %.go
# 	$(GC) -o $@ $(CFLAGS) -c $^

.PHONY: clean run

run: os-multiboot.bin
	qemu-system-i386 -kernel os-multiboot.bin -serial stdio -hda hdd.img

# test: multiboot-test
# 	qemu-system-i386 -kernel os-multiboot-test.bin -serial stdio

clean:
	rm -rf *.o *.bin