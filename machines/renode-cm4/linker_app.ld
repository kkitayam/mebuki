/*
 * linker script for a user application software
 *
 * mebuki image header is 8 bytes, so start at 0x20008
 *
 * memory layout:
 * - Flash: 0x00020008 - 0x00040000 (128KB, Slot 0)
 * - SRAM:  0x20000000 - 0x20020000 (128KB)
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00020008, LENGTH = 0x20000    /* 128KB (Slot 0) */
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x20000    /* 128KB */
}

SECTIONS
{
    /* ISR vector table (optional: defined by the user application) */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* code */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* ARM exception unwinding table */
    .ARM.exidx :
    {
        . = ALIGN(4);
        KEEP(*(.ARM.exidx*))
        . = ALIGN(4);
    } > FLASH

    /* Initialized data */
    .data :
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > SRAM AT > FLASH

    __data_rom_start = LOADADDR(.data);

    /* Uninitialized data */
    .bss :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SRAM

    /* Heap and stack */
    .heap (NOLOAD) :
    {
        . = ALIGN(4);
        __heap_start = .;
        . += 0x1000;    /* 4KB heap */
        __heap_end = .;
    } > SRAM

    .stack (NOLOAD) :
    {
        . = ALIGN(4);
        . += 0x2000;    /* 8KB stack */
        __stack_end = .;
    } > SRAM
}
