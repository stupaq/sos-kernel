OUTPUT_FORMAT("elf32-i386")
ENTRY(start)

highmem = 0xFF000000; _highmem = 0xFF000000; __highmem = 0xFF000000;

SECTIONS
{
	. = 0x100000;

	.setup ALIGN (4096) :
	{
		*(.setup)
	}
	
	. += highmem;

	.text ALIGN (4096) : AT(ADDR(.text) - highmem)
	{
        code = .; _code = .; __code = .;
		*(.text)
	}

	.data ALIGN (4096) : AT(ADDR(.data) - highmem)
	{
        data = .; _data = .; __data = .;
		*(.data)
		*(.rodata)
	}
	
	.bss ALIGN (4096) : AT(ADDR(.bss) - highmem)
	{
        bss = .; _bss = .; __bss = .;
        *(.bss)
	}
	
	. = ALIGN (4096);
	
    end = .; _end = .; __end = .;
}
