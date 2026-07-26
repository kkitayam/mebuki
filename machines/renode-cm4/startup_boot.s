/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

    .syntax unified
    .cpu cortex-m4
    .thumb

    .section .isr_vector, "a"
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   0x20020000                          /* Initial stack pointer (SRAM end, 128KB) */
    .long   reset_handler
    /* Exception handlers (Cortex-M4 handlers, not used) */
    .long   0                                   /* NMI */
    .long   0                                   /* HardFault */
    .long   0                                   /* MemManage */
    .long   0                                   /* BusFault */
    .long   0                                   /* UsageFault */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* SVCall */
    .long   0                                   /* Reserved */
    .long   0                                   /* Reserved */
    .long   0                                   /* PendSV */
    .long   0                                   /* SysTick */

    /* IRQ handlers (not used in Renode) */
    .long   0                                   /* IRQ0-239 space */

    .text
    .align 2
    .globl reset_handler
    .thumb_func
reset_handler:
    /* Jump to the main program */
    bl      main

    /* If main() returns */
    b       .

    .end
