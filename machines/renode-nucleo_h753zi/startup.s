/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

    .syntax unified
    .cpu cortex-m7
    .thumb

    .section .isr_vector, "a", %progbits
    .align 2
    .long 0x20020000
    .long reset_handler
    .rept 14
    .long 0
    .endr

    .text
    .align 2
    .global reset_handler
    .thumb_func
reset_handler:
    ldr r0, =__data_start
    ldr r1, =__data_end
    ldr r2, =__data_rom_start
1:
    cmp r0, r1
    bcs 2f
    ldr r3, [r2], #4
    str r3, [r0], #4
    b 1b
2:
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    movs r2, #0
3:
    cmp r0, r1
    bcs 4f
    str r2, [r0], #4
    adds r0, r0, #4
    b 3b
4:
    bl system_init
    bl __libc_init_array
    bl main
5:
    b 5b
