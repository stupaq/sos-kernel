ENTRY(main)
SECTIONS
{
    .text 0x10000000 :
    {
        *(.text)
        . = ALIGN(4096);
    }
    .data :
    {
        *(.data)
        *(.rodata)
        . = ALIGN(4096);
    }
    .bss :
    {
        *(.bss)
        . = ALIGN(4096);
    }
}
