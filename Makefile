SYSROOT := ./alloy
SHELL := /bin/bash
PATH := $SYSROOT/usr/local/bin:$(PATH)

VPATH := $(shell find kernel -type d)
C_OBJ := $(shell find kernel -type f -name "*.c" | xargs basename -s .c | xargs -I '{}' echo '{}.o')
ASM_OBJ := $(shell find kernel -type f -name "*.asm" | xargs basename -s .asm | xargs -I '{}' echo '{}.o')
OBJECTS := $(C_OBJ) $(ASM_OBJ) # $(GO_OBJ)

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master


CC := i686-alloy-gcc
CFLAGS := -I./include -std=gnu11 -ffreestanding -O -Wall -Wextra -Wpedantic -g

all: os-multiboot.bin userspace

os-multiboot.bin: $(OBJECTS)
	$(CC) -T multiboot-hh.ld -o $@ -ffreestanding -O -nostdlib $(OBJECTS) -lgcc

%.o: %.asm
	nasm -f elf32 -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY: clean run userspace

userspace:
	$(MAKE) -C $@
	scripts/update-hdd.sh

run: os-multiboot.bin
	qemu-system-i386 -kernel os-multiboot.bin -serial stdio -hda hdd.img

clean:
	rm -rf *.o *.bin *.iso isodir
	$(MAKE) -C userspace clean
