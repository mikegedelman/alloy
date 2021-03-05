OUTPUT_FORMAT("binary")
ENTRY(_start)
/* OUTPUT_FORMAT(elf32-i386) */
SECTIONS
{
    . = 0x800;
	.text . : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss :
	{
		*(COMMON)
		*(.bss)
	}
}
