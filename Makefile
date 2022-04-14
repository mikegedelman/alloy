VPATH := $(shell find kernel -type d)
C_OBJ := $(shell find kernel -type f -name "*.c" | xargs basename -s .c | xargs -I '{}' echo '{}.o')
ASM_OBJ := $(shell find kernel -type f -name "*.asm" | xargs basename -s .asm | xargs -I '{}' echo '{}.o')
OBJECTS := $(C_OBJ) $(ASM_OBJ) # $(GO_OBJ)

RUST_SRC := $(shell find kernel-rust -type f -name "*.rs")

DOCKER := docker run -v $(shell pwd):/work -w /work mikegedelman/alloy:master

CC := i686-elf-gcc -g
CFLAGS := -I./include -std=gnu11 -ffreestanding -O -Wall -Wextra

all: os-multiboot.bin

os-multiboot.bin: $(OBJECTS) kernel-rust/target/x86-alloy/debug/libkernel.a
	$(CC) -T multiboot-hh.ld -o $@ -ffreestanding -O -nostdlib $(OBJECTS) -lgcc -l:libkernel.a -L./kernel-rust/target/x86-alloy/debug

%.o: %.asm
	nasm -f elf32 -o $@ $^

%.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $^

# user.elf: userspace/main.c userspace/crt0.S
# 	i686-elf-gcc -I./include -std=gnu11 -ffreestanding -O -Wall -Wextra userspace/main.c userspace/crt0.S -nostdlib -o user.elf

user.bin:  userspace/crt0.S userspace/main.c
	i686-elf-gcc -I./include -std=gnu11 -ffreestanding -O -Wall -Wextra $^ -nostdlib -o user.bin
# 	i686-elf-objcopy user.elf -O binary user.bin


kernel-rust/target/x86-alloy/debug/libkernel.a: $(RUST_SRC)
	cd kernel-rust && \
		cargo build && \
		cd ..

.PHONY: clean run rust

rust: kernel-rust/target/x86-alloy/debug/libkernel.a

run: os-multiboot.bin
	qemu-system-i386 -kernel os-multiboot.bin -serial stdio -hda hdd.img

clean:
	rm -rf *.o *.bin *.iso isodir