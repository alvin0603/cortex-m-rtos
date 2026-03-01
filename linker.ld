MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 256K
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}
_stack_top = ORIGIN(SRAM) + LENGTH(SRAM);

SECTIONS
{
    .isr_vector :
    {
        KEEP(*(.isr_vector))
    } > FLASH
    .text :
    {
        *(.text)
        *(.text*)
    } > FLASH
    .rodata :
    {
        *(.rodata)
        *(.rodata*)
    } > FLASH
    .data :
    {
        _data_start = .;
        *(.data)
        *(.data*)
        _data_end = .;
    } > SRAM AT> FLASH
    _data_load = LOADADDR(.data);
    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        _bss_end = .;
    } > SRAM
}
