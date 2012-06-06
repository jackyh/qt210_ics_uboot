OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0xD0020010;
	. = ALIGN(4);
	.text   :
	{
		Init.o (.text)
		*(.text)
	}

	. = ALIGN(4);
	rodata = .;
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	data_start = .;
	.data : { *(.data) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = ALIGN(4);
	bss_start = .;
	.bss : { *(.bss) }
	bss_end = .;
}
