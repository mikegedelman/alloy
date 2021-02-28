
KERNEL_SRC := $(shell find kernel/src -type f -name "*.rs")
KERNEL_OBJ := kernel/target/x86-alloy/debug/liballoy.a
ASM_OBJ := multiboot.o utils.o loader.o

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master
LD := $(DOCKER) i686-elf-ld
# If you want to build your own binutils and skip Docker, uncomment the following
# line.
# LD := i686-elf-ld
# The commented-out C stuff is laying around from previously using some C
# files to test stuff or sketch out functionality before writing in Rust.
# Leaving these around in case I want to do that again.
# CC := i686-elf-gcc -g
# C_SRC := idt.c
# C_OBJ := idt.o

all: multiboot

clean:
	cd kernel && \
	cargo clean && \
	cd ..
	rm -rf *.bin *.o *.img isodir/

# Targets to build general kernel object files
# $(C_OBJ): $(C_SRC)
# 	$(CC) -c $(C_SRC) -std=gnu99 -ffreestanding -Wall -Wextra

# Build our Rust kernel: cd into kernel dir and build from there.
# There's probably a better way to integrate cargo into this makefile.
$(KERNEL_OBJ): $(KERNEL_SRC)
	cd kernel && \
	cargo build --target x86-alloy.json --features verbose && \
	cd ..

# There must be a better way to do this without having to mostly repeat
# the above target
test_obj: $(KERNEL_SRC)
	cd kernel && \
	cargo build --target x86-alloy.json --features test && \
	cd ..

# Assembly files
$(ASM_OBJ): asm/multiboot.asm asm/utils.asm asm/loader.asm
	nasm -g -f elf32 asm/multiboot.asm -o multiboot.o
	nasm -g -f elf32 asm/utils.asm -o utils.o
	nasm -g -f elf32 asm/loader.asm -o loader.o

# Link the kernel into a multiboot format
# Warning: Do *NOT* name this multiboot.bin. When qemu sees multiboot.bin in the current dir,
# weird things seem to happen
multiboot: $(ASM_OBJ) $(KERNEL_OBJ) $(BOOT_OBJ) # $(C_OBJ)
	$(LD) -T multiboot.ld -o alloy.bin $(ASM_OBJ) $(KERNEL_OBJ)

multiboot_test: $(ASM_OBJ) test_obj $(BOOT_OBJ)
	$(LD) -T multiboot.ld -o alloy-test.bin $(ASM_OBJ) $(KERNEL_OBJ)

run: multiboot
	qemu-system-i386 -kernel alloy.bin

test: multiboot_test
	scripts/test.sh alloy-test.bin

# To use these targets, change the first line of multiboot.ld to:
# ENTRY(_loader)
# This will be fixed eventually..
# cd: multiboot
# 	scripts/iso.sh alloy.bin

# run_cd: cd
# 	qemu-system-i386 -cdrom alloy.bin

# First: brew install bochs
# bochs can be useful for specific debugging tasks, but generally
# qemu is faster and easier
bochs: cd
	bochs

# You shouldn't need this unless you change the Dockerfile for some reason
# GCC takes ~20 mins to build on my machine. It's actually not being used right now,
# but it's convenient to have in the image. If you don't need it, you can comment out
# the "build GCC" section before rebuilding
docker:
	docker build .
