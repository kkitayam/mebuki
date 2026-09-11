MEMORY
{
	RAM  : ORIGIN = 0x4,        LENGTH = 0x3fffc
	DROM : ORIGIN = 0x00100000, LENGTH = 0x8000
	OFSM : ORIGIN = 0xFE7F5D00, LENGTH = 0x80
	PROM : ORIGIN = 0xFFE00000, LENGTH = 0x200000
}

ISP_SIZE = 2K;
USP_SIZE = 4K;
BOOT_SIZE = 32K;

BOOT_ORIGIN = ORIGIN(PROM) + LENGTH(PROM) - BOOT_SIZE;
VECTOR_BASE = 0xFFFFFF80;

__isp_top = ORIGIN(RAM) + LENGTH(RAM);
__isp_limit = __isp_top - ISP_SIZE;
__usp_top = __isp_limit;
__usp_limit = __usp_top - USP_SIZE;

PROVIDE(_PrivilegedInst_Handler = 0xFFFFFFFF);
PROVIDE(_Access_Handler         = 0xFFFFFFFF);
PROVIDE(_UndefinedInst_Handler  = 0xFFFFFFFF);
PROVIDE(_FloatingPoint_Handler  = 0xFFFFFFFF);
PROVIDE(_NMI_Handler            = 0xFFFFFFFF);

ENTRY(_Reset_Handler)

SECTIONS
{
	.reset BOOT_ORIGIN :
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
		*(.text.flash_type4_pe_ram)
		. = ALIGN(4);
		__etext = .;
	} > RAM AT > PROM = 0xFF
	__text_load = LOADADDR(.text.ram);

	. = BOOT_ORIGIN + SIZEOF(.reset) + SIZEOF(.text.ram);
	.text ALIGN(4) :
	{
		*(.text) *(.text.*)
		. = ALIGN(4);
	} > PROM = 0xFF

	.rodata ALIGN(4) :
	{
		*(.rodata) *(.rodata.*)
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

	.fixed_vector VECTOR_BASE + 0x50:
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
}

ASSERT(LOADADDR(.data) + SIZEOF(.data) <= VECTOR_BASE, "Boot image overlaps fixed vectors");
ASSERT(__usp_limit >= __ebss, "RAM overflow");
ASSERT(SIZEOF(.data) == 0, "Initialized global/static variables are forbidden in the bootloader.");
