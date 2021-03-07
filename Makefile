
KERNEL_SRC := $(shell find kernel/src -type f -name "*.rs")
KERNEL_ASM := $(shell find kernel/asm -type f -name "*.asm")


TARGET := target/x86-alloy/debug
BUILD := $(TARGET)/build
KERNEL_OBJ := $(TARGET)/libkernel.a
ASM_OBJ :=$ $(BUILD)/loader.o $(BUILD)/utils.o
KERNEL := $(TARGET)/kernel.bin

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master
# LD := $(DOCKER) i686-elf-ld
# If you want to build your own binutils and skip Docker, uncomment the following
# line.
LD := i686-elf-ld
# The commented-out C stuff is laying around from previously using some C
# files to test stuff or sketch out functionality before writing in Rust.
# Leaving these around in case I want to do that again.
# CC := i686-elf-gcc -g
# C_SRC := idt.c
# C_OBJ := idt.o

all: $(KERNEL)

clean:
	cargo clean
	rm -rf *.bin *.o *.img isodir/
	rm -rf kernel/resources/*.tar

# Targets to build general kernel object files
# $(C_OBJ): $(C_SRC)
# 	$(CC) -c $(C_SRC) -std=gnu99 -ffreestanding -Wall -Wextra

# Build our Rust kernel: cd into kernel dir and build from there.
# There's probably a better way to integrate cargo into this makefile.
$(KERNEL_OBJ): $(KERNEL_SRC)
	cargo build -p kernel --features verbose

# There must be a better way to do this without having to mostly repeat
# the above target
test_obj: $(KERNEL_SRC)
	cargo build -p kernel --features test

# Assembly files
$(ASM_OBJ): kernel/asm/utils.asm kernel/asm/loader.asm
	nasm -g -f elf32 kernel/asm/utils.asm -o $(BUILD)/utils.o
	nasm -g -f elf32 kernel/asm/loader.asm -o $(BUILD)/loader.o

# Link the kernel into a multiboot format
# Warning: Do *NOT* name this multiboot.bin. When qemu sees multiboot.bin in the current dir,
# weird things seem to happen
$(KERNEL): $(KERNEL_OBJ) $(ASM_OBJ) kernel/multiboot.ld # $(C_OBJ)
	$(LD) -T kernel/multiboot.ld -o $(TARGET)/kernel.bin $(ASM_OBJ) $(KERNEL_OBJ)

# Build the kernel with test feature, which will run tests in kernel_main instead of our
# normal setup
# This is here because I couldn't figure out how to get cargo test to work with this.
# Cargo test really wants to build a binary for you and run it, but we're building a staticlib
# and linking it before we can run, so that approach doens't really work..
$(TARGET)/kernel-test.bin: $(ASM_OBJ) test_obj
	$(LD) -T kernel/multiboot.ld -o $(TARGET)/kernel-test.bin $(ASM_OBJ) $(KERNEL_OBJ)

test_kernel: $(TARGET)/kernel-test.bin

# Run QEMU and exec the "hello_rs" binary inside our OS
run: $(KERNEL) kernel/resources/hello_rust.tar
	qemu-system-i386 -kernel $(KERNEL) -hda kernel/resources/hello_rust.tar -serial stdio

# The above, but attach gdb for debugging, breaking at kernel_main by default
debug: $(KERNEL) kernel/resources/hello_rust.tar
	scripts/debug.sh $(KERNEL)

# Run tests. This prints to stdout and doesn't actually fire up a QEMU gui
test: test_kernel
	scripts/test.sh $(TARGET)/kernel-test.bin

# Build & link the hello assembly program and package it into a TAR
kernel/resources/hello.tar: kernel/resources/hello.asm
	nasm -g -f elf32 kernel/resources/hello.asm -o $(BUILD)/hello.o
	$(LD) -o $(BUILD)/hello $(BUILD)/hello.o
	tar -cf kernel/resources/hello.tar -C $(BUILD) hello

# Build our hello_rs program
$(TARGET)/hello_rs: hello_rs/src/main.rs alloy_std/src/lib.rs
	cargo build -p hello_rs

# Package hello_rs program into a tar so the kernel can read and exec it
kernel/resources/hello_rust.tar: $(TARGET)/hello_rs
	tar -cf kernel/resources/hello_rust.tar -C $(TARGET) hello_rs

# To use these targets, change the first line of multiboot.ld to:
# ENTRY(_loader)
# Actually even after doing that, these are still broken
# $(TARGET)/alloy.iso: $(KERNEL)
# 	scripts/iso.sh $(KERNEL) $(TARGET)

# cd: $(TARGET)/alloy.iso

# run_cd: $(TARGET)/
# 	qemu-system-i386 -cdrom $(TARGET)/alloy.iso

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
