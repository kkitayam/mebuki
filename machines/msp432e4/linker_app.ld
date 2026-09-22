MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00020008, LENGTH = 0x0003FFF8
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

SECTIONS
{
    .isr_vector : { KEEP(*(.isr_vector)) } > FLASH
    .text : { *(.text*) *(.rodata*) . = ALIGN(4); } > FLASH
    .data : { __data = .; *(.data*) . = ALIGN(4); __edata = .; } > SRAM AT > FLASH
    __data_load = LOADADDR(.data);
    .bss (NOLOAD) : { __bss = .; *(.bss*) *(COMMON) . = ALIGN(4); __ebss = .; } > SRAM
    .stack (NOLOAD) : { . = ALIGN(8); . += 0x2000; } > SRAM
}

ASSERT(__ebss <= ORIGIN(SRAM) + LENGTH(SRAM), "RAM overflow");
