MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x00008000
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text.reset)
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    .ramfunc :
    {
        . = ALIGN(4);
        __text = .;
        *(.ramfunc*)
        . = ALIGN(4);
        __etext = .;
    } > SRAM AT > FLASH
    __text_load = LOADADDR(.ramfunc);

    .data :
    {
        . = ALIGN(4);
        __data = .;
        *(.data*)
        . = ALIGN(4);
        __edata = .;
    } > SRAM AT > FLASH
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        __bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __ebss = .;
    } > SRAM

    .stack (NOLOAD) :
    {
        . = ALIGN(8);
        . += 0x2000;
    } > SRAM
}

ASSERT(SIZEOF(.isr_vector) <= 0x400, "vector table exceeds reserved space");
ASSERT(SIZEOF(.text) + SIZEOF(.isr_vector) + SIZEOF(.ramfunc) + SIZEOF(.data) <= LENGTH(FLASH),
       "boot image exceeds reserved flash");
ASSERT(__ebss <= ORIGIN(SRAM) + LENGTH(SRAM), "RAM overflow");
