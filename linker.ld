OUTPUT_FORMAT("binary") 
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
