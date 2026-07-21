ENTRY(ResetISR)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x0000B008, LENGTH = 0x00001FF8
  SRAM  (rwx) : ORIGIN = 0x10000000, LENGTH = 0x00002000
}

__StackTop = ORIGIN(SRAM) + LENGTH(SRAM);
PROVIDE(__valid_user_code_checksum = 0 - (__StackTop + (ResetISR + 1) + (NMI_Handler + 1) + (HardFault_Handler + 1)));

SECTIONS
{
  .isr_vector ORIGIN(FLASH) :
  {
    KEEP(*(.isr_vector))
  } > FLASH

  .text :
  {
    *(.text*)
    *(.rodata*)
    *(.glue_7)
    *(.glue_7t)
    KEEP(*(.init))
    KEEP(*(.fini))
    . = ALIGN(4);
    __preinit_array_start = .;
    KEEP(*(.preinit_array*))
    __preinit_array_end = .;
    __init_array_start = .;
    KEEP(*(.init_array*))
    __init_array_end = .;
    __fini_array_start = .;
    KEEP(*(.fini_array*))
    __fini_array_end = .;
    . = ALIGN(4);
  } > FLASH

  .ARM.exidx :
  {
    *(.ARM.exidx*)
  } > FLASH

  .data : ALIGN(4)
  {
    __data_start = .;
    *(.data*)
    __data_end = .;
  } > SRAM AT > FLASH

  __data_load_start = LOADADDR(.data);

  .bss (NOLOAD) : ALIGN(4)
  {
    __bss_start = .;
    *(.bss*)
    *(COMMON)
    __bss_end = .;
  } > SRAM
}