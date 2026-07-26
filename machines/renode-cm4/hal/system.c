/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "system.h"

/*
 * - In the Renode environment, the CPU clock is 32MHz by default (no configuration needed)
 * - NVIC is initialized in platform.repl (no configuration needed)
 * - prepare_handoff: preparation before handing off control to the user application
 */

/**
 * @brief PRIMASK register (interrupt mask register)
 *
 * ARM Cortex-M interrupt control:
 * - bit 0 = 1: interrupts disabled
 * - bit 0 = 0: interrupts enabled
 */
static inline void __disable_irq(void)
{
    __asm__ volatile("cpsid i" : : : "memory");
}

static inline void __enable_irq(void)
{
    __asm__ volatile("cpsie i" : : : "memory");
}

void _init(void) {
    /* A dummy function for C runtime initialization */
}

void system_init(void)
{
    /* System initialization code can be added here if needed */
}

void system_reset(void)
{
    /*
     * Cortex-M4 system reset
     * NVIC_SystemReset() is CMSIS dependent, so implement directly with register operations
     */
    const uint32_t AIRCR_VECTKEY = 0x5FAU;
    const uint32_t AIRCR_SYSRESETREQ = (1U << 2);
    volatile uint32_t* AIRCR = (volatile uint32_t*)0xE000ED0C;

    /* Set VECTKEY and set the SYSRESETREQ bit */
    *AIRCR = (AIRCR_VECTKEY << 16) | AIRCR_SYSRESETREQ;

    /* infinite loop until reset occurs */
    while (1) {
        __asm__ volatile("nop");
    }
}

void prepare_handoff(void)
{
    /*
     * Preparation before handing off control to the user application
     * - global interrupt disable
     *   (Requirement: interrupts are not used)
     */
    __disable_irq();
}

__attribute__((noreturn))
void jump_to_firmware(uint32_t entry_point)
{
    const uint32_t* vector_table = (const uint32_t*)entry_point;
    __asm__ volatile ("MSR msp, %0" : : "r" (vector_table[0]) : );
    typedef void (*app_entry_t)(void);
    app_entry_t app_entry = (app_entry_t)vector_table[1];
    app_entry();
    halt();
}

void halt(void)
{
    /*
     * System halt: disable interrupts and enter infinite loop
     * Reduce power consumption with WFI (Wait For Interrupt)
     */
    __disable_irq();

    while (1) {
        __asm__ volatile("wfi");
    }
}
