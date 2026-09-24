/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2005-2017 Texas Instruments Incorporated
 * All rights reserved.
 */
#include "msp432e401y_flash.h"
#include "msp432e401y.h"          /* CMSIS device header only */

#define FLASH_ERASE_SIZE   0x4000U   /* 16 KiB */
#define FLASH_BANK_SIZE    0x80000U  /* 512 KiB */
#define FLASH_WRKEY        0xA4420000U

/* Translate logical address to physical address for FMA.
 * Logical 0x00000-0x7FFFF = bank 0, 0x80000-0xFFFFF = bank 1.
 * When FMME=1 the mapping is inverted. */
static uintptr_t flash_phys(uintptr_t addr)
{
    bool mirror = (FLASH_CTRL->CONF & FLASH_CONF_FMME) != 0;

    if (addr < FLASH_BANK_SIZE) {
        /* logical bank 0 */
        return mirror ? (addr + FLASH_BANK_SIZE) : addr;
    } else {
        /* logical bank 1 */
        return mirror ? (addr - FLASH_BANK_SIZE) : addr;
    }
}

void msp432_flash_init(void)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC |
                         FLASH_FCMISC_VOLTMISC |
                         FLASH_FCMISC_ERMISC |
                         FLASH_FCMISC_INVDMISC |
                         FLASH_FCMISC_PROGMISC;
}

/* Erase one 16 KiB sector. Must run from RAM. */
__attribute__((section(".ramfunc"), noinline))
static int flash_erase_one(uintptr_t phys)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC |
                         FLASH_FCMISC_VOLTMISC |
                         FLASH_FCMISC_ERMISC;

    FLASH_CTRL->FMA = (uint32_t)phys;
    FLASH_CTRL->FMC = FLASH_WRKEY | FLASH_FMC_ERASE;

    while (FLASH_CTRL->FMC & FLASH_FMC_ERASE)
        ;

    if (FLASH_CTRL->FCRIS & (FLASH_FCRIS_ARIS |
                             FLASH_FCRIS_VOLTRIS |
                             FLASH_FCRIS_ERRIS))
        return MSP432_FLASH_FAILURE;
    return 0;
}

int msp432_flash_erase(uintptr_t address, size_t size)
{
    if (size == 0)
        return 0;
    if (address & (FLASH_ERASE_SIZE - 1))
        return MSP432_FLASH_ALIGN;

    uintptr_t end = address + size;
    while (address < end) {
        uintptr_t phys = flash_phys(address);
        int rc = flash_erase_one(phys);
        if (rc != 0)
            return rc;
        address += FLASH_ERASE_SIZE;
    }
    return 0;
}

/* Program one write-buffer (≤32 words). Must run from RAM. */
__attribute__((section(".ramfunc"), noinline))
static int flash_prog_buf(uintptr_t phys, const uint32_t *src, size_t words)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC |
                         FLASH_FCMISC_VOLTMISC |
                         FLASH_FCMISC_INVDMISC |
                         FLASH_FCMISC_PROGMISC;

    FLASH_CTRL->FMA = (uint32_t)(phys & ~0x7FU);

    size_t i;
    for (i = 0; i < words; i++) {
        uint32_t off = (phys + i * 4) & 0x7C;
        FLASH_CTRL->FWBN[off / 4] = src[i];
    }

    FLASH_CTRL->FMC2 = FLASH_WRKEY | FLASH_FMC2_WRBUF;
    while (FLASH_CTRL->FMC2 & FLASH_FMC2_WRBUF)
        ;

    if (FLASH_CTRL->FCRIS & (FLASH_FCRIS_ARIS |
                             FLASH_FCRIS_VOLTRIS |
                             FLASH_FCRIS_INVDRIS |
                             FLASH_FCRIS_PROGRIS))
        return MSP432_FLASH_FAILURE;
    return 0;
}

int msp432_flash_write(uintptr_t address, const void *data, size_t size)
{
    if (size == 0)
        return 0;
    if ((address & 3) || (size & 3) || data == NULL)
        return MSP432_FLASH_ALIGN;

    const uint32_t *src = (const uint32_t *)data;
    while (size) {
        uintptr_t phys = flash_phys(address);
        size_t words = 32 - ((phys >> 2) & 31);
        if (words > size / 4)
            words = size / 4;

        int rc = flash_prog_buf(phys, src, words);
        if (rc != 0)
            return rc;

        src     += words;
        address += words * 4;
        size    -= words * 4;
    }
    return 0;
}

bool msp432_flash_is_blank(uintptr_t address, size_t size)
{
    const uint8_t *p = (const uint8_t *)address;
    while (size--) {
        if (*p++ != 0xFF)
            return false;
    }
    return true;
}

int msp432_flash_swap_bank(void)
{
    /* Prefetch must be idle; simple invalidate is enough for most cases. */
    FLASH_CTRL->CONF ^= FLASH_CONF_FMME;
    __DSB();
    __ISB();
    return 0;
}
