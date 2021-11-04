/*
 * Adapted from:
 * https://wiki.osdev.org/Higher_Half_x86_Bare_Bones_(Backup)
 * TODO: switching to _loader works with GRUB; make it work with both
 */
ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
   /* The kernel will live at 3GB + 1MB in the virtual
      address space, which will be mapped to 1MB in the
      physical address space. */
   . = 0xC0100000;

    kernel_start = ALIGN(4K);
   .text : AT(ADDR(.text) - 0xC0000000) {
        . = ALIGN(4);
        LONG(0x1BADB002)
        LONG(0x00000003)
        LONG(0xE4524FFB)
       *(.text)
       *(.rodata*)
   }

   .data ALIGN (0x1000) : AT(ADDR(.data) - 0xC0000000) {
       *(.data)
   }

   .bss : AT(ADDR(.bss) - 0xC0000000) {
       _sbss = .;
       *(COMMON)
       *(.bss)
       _ebss = .;
   }
}

kernel_end = .;
