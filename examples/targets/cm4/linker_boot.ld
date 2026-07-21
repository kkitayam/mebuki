/**
 * @file linker_boot.ld
 * @brief Boot Software 用リンカスクリプト
 *
 * メモリレイアウト:
 * - Flash: 0x00000000 - 0x00020000 (128KB, Boot)
 * - SRAM:  0x20000000 - 0x20020000 (128KB)
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 0x20000    /* 128KB */
    SRAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x20000    /* 128KB */
}

SECTIONS
{
    /* ISR ベクタテーブル */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* コード */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* ARM exception unwinding table (出力不要だが含める) */
    .ARM.exidx :
    {
        . = ALIGN(4);
        KEEP(*(.ARM.exidx*))
        . = ALIGN(4);
    } > FLASH

    /* 未初期化データ */
    .bss :
    {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > SRAM

    .stack (NOLOAD) :
    {
        . = ALIGN(4);
        . += 0x2000;    /* 8KB スタック */
        __stack_end = .;
    } > SRAM
}
