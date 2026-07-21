/* Cortex-M4 application startup (GNU AS) */

	.syntax unified
	.cpu cortex-m4
	.thumb

	/* Weak alias helper for core exceptions */
	.macro def_irq_handler handler_name
		.weak   \handler_name
		.thumb_set \handler_name, Default_Handler
	.endm

	.section .isr_vector, "a", %progbits
	.align 2
	.globl __isr_vector
__isr_vector:
	/* Stack pointer */
	.long   0x20020000

	/* Reset */
	.long   reset_handler

	/* Core exceptions */
	.long   NMI_Handler
	.long   HardFault_Handler
	.long   MemManage_Handler
	.long   BusFault_Handler
	.long   UsageFault_Handler
	.long   0
	.long   0
	.long   0
	.long   0
	.long   SVC_Handler
	.long   0
	.long   0
	.long   PendSV_Handler
	.long   SysTick_Handler

	.text
	.align 2
	.globl reset_handler
	.type reset_handler, %function
	.thumb_func
reset_handler:
	/* Copy .data from flash to SRAM */
	ldr     r0, =__data_start
	ldr     r1, =__data_end
	ldr     r2, =__data_rom_start
1:
	cmp     r0, r1
	bcs     2f
	ldr     r3, [r2], #4
	str     r3, [r0], #4
	b       1b
2:
	/* Zero .bss */
	ldr     r0, =__bss_start
	ldr     r1, =__bss_end
	movs    r2, #0
3:
	cmp     r0, r1
	bcs     4f
	str     r2, [r0], #4
	adds    r0, r0, #4
	b       3b
4:
	/* System initialization before main */
	bl      system_init
	bl      __libc_init_array
	bl      main
5:
	b       5b
	.size   reset_handler, .-reset_handler

	/* Default handlers (weak aliases) */
	def_irq_handler NMI_Handler
	def_irq_handler HardFault_Handler
	def_irq_handler MemManage_Handler
	def_irq_handler BusFault_Handler
	def_irq_handler UsageFault_Handler
	def_irq_handler SVC_Handler
	def_irq_handler PendSV_Handler
	def_irq_handler SysTick_Handler

	.section .text.Default_Handler, "ax", %progbits
	.align 2
	.weak   Default_Handler
	.type   Default_Handler, %function
Default_Handler:
	b       .
	.size   Default_Handler, .-Default_Handler

	.end
