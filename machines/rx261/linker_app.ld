MEMORY
{
	RAM  : ORIGIN = 0x4,        LENGTH = 0x1fffc
	DROM : ORIGIN = 0x00100000, LENGTH = 0x2000
	PROM : ORIGIN = 0xFFF80000, LENGTH = 0x80000
}

ISP_SIZE = 2K;
USP_SIZE = 4K;
SLOT_SIZE = 128K;

ENTRY_ORIGIN = ORIGIN(PROM) + 8;

__isp_top   = ORIGIN(RAM) + LENGTH(RAM);
__isp_limit = __isp_top - ISP_SIZE;
__usp_top   = __isp_limit;
__usp_limit = __usp_top - USP_SIZE;

PROVIDE(_PrivilegedInst_Handler = 0xFFFFFFFF);
PROVIDE(_Access_Handler         = 0xFFFFFFFF);
PROVIDE(_UndefinedInst_Handler  = 0xFFFFFFFF);
PROVIDE(_FloatingPoint_Handler  = 0xFFFFFFFF);
PROVIDE(_NMI_Handler            = 0xFFFFFFFF);

ENTRY(_Reset_Handler)

SECTIONS
{
	.reset ENTRY_ORIGIN :
	{
		. = ALIGN(4);
		*(.text.reset)
		*(.text.system_init)
		. = ALIGN(4);
	} > PROM = 0xFF

	. = ORIGIN(RAM);
	.text.ram ALIGN(4) :
	{
		__text = .;
		*(.text.flash_type1_pe_ram)
		. = ALIGN(4);
		__etext = .;
	} > RAM AT > PROM = 0xFF
	__text_load = LOADADDR(.text.ram);

	. = ENTRY_ORIGIN + SIZEOF(.reset) + SIZEOF(.text.ram);
	.text ALIGN(4) :
	{
		*(.text) *(.text.*)
		. = ALIGN(4);
	} > PROM = 0xFF

	.exception_vector ALIGN(4) :
	{
		LONG(_PrivilegedInst_Handler)
		LONG(_Access_Handler)
		. += 4;
		LONG(_UndefinedInst_Handler)
		. += 4;
		LONG(_FloatingPoint_Handler)
		. += 16;
		LONG(_NMI_Handler)
		LONG(_Reset_Handler)
	} > PROM = 0xFF
	__ext_base = ADDR(.exception_vector) - 0x50;

	.interrupt_vector ALIGN(4) :
	{
		*(.interrupt_vector) *(.interrupt_vector.*)
	} > PROM = 0xFF
	__int_base = ADDR(.interrupt_vector);

	.rodata ALIGN(4) :
	{
		*(.rodata) *(.rodata.*)
		. = ALIGN(4);
		__erodata = .;
	} > PROM = 0xFF

	.eh_frame ALIGN(4) :
	{
		*(.eh_frame)
		*(.eh_frame.*)
		. = ALIGN(4);
	} > PROM = 0xFF

	/*
	 * Legacy initialization/finalization sections.
	 */
	.init ALIGN(4) :
	{
		KEEP(*(.init))
		. = ALIGN(4);
	} > PROM = 0xFF

	.fini ALIGN(4) :
	{
		KEEP(*(.fini))
		. = ALIGN(4);
	} > PROM = 0xFF

	/*
	 * C/C++ initialization arrays.
	 */
	.preinit_array ALIGN(4) :
	{
		PROVIDE_HIDDEN(__preinit_array_start = .);
		KEEP(*(.preinit_array))
		KEEP(*(.preinit_array.*))
		PROVIDE_HIDDEN(__preinit_array_end = .);
		. = ALIGN(4);
	} > PROM = 0xFF

	.init_array ALIGN(4) :
	{
		PROVIDE_HIDDEN(__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN(__init_array_end = .);
		. = ALIGN(4);
	} > PROM = 0xFF

	.fini_array ALIGN(4) :
	{
		PROVIDE_HIDDEN(__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN(__fini_array_end = .);
		. = ALIGN(4);
	} > PROM = 0xFF

	. = ORIGIN(RAM) + SIZEOF(.text.ram);
	.data ALIGN(4) :
	{
		__data = .;
		*(.data) *(.data.*)
		*(.sdata) *(.sdata.*)
		. = ALIGN(4);
		__edata = .;
	} > RAM AT > PROM = 0xFF
	__data_load = LOADADDR(.data);

	.bss ALIGN(4) (NOLOAD) :
	{
		__bss = .;
		*(.bss) *(.bss.*)
		*(.sbss) *(.sbss.*)
		*(COMMON)
		. = ALIGN(4);
		__ebss = .;
		__end = .;
	} > RAM

	__heap_start = ALIGN(__end,4);
}

ASSERT(__usp_limit >= __ebss, "RAM overflow");
ASSERT(__edata < ORIGIN(PROM) + SLOT_SIZE , "Firmware image overlaps the next slot");