MEMORY
{
    FLASH (rx) : ORIGIN = 0x08040008, LENGTH = 0xBFFF8
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x20000
}
ASSERT(__bss_end <= ORIGIN(SRAM) + LENGTH(SRAM) - 0x4000, "Insufficient stack space")

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    .data :
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > SRAM AT > FLASH
    __data_rom_start = LOADADDR(.data);
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SRAM
}
