ENTRY(_start)

SECTIONS {
    . = 1M;

    kernel_start = .;
    .text :
    {
        . = ALIGN(4);
        LONG(0x1BADB002)
        LONG(0x00000003)
        LONG(0xE4524FFB)
        *(.text)
    }
    kernel_end = .;
}
