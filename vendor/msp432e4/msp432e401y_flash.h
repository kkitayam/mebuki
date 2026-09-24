/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2005-2017 Texas Instruments Incorporated
 * All rights reserved.
 */
#ifndef MSP432E401Y_FLASH_H
#define MSP432E401Y_FLASH_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define MSP432_FLASH_PARAM   (-1)
#define MSP432_FLASH_TIMEOUT (-3)
#define MSP432_FLASH_FAILURE (-4)
#define MSP432_FLASH_ALIGN   (-5)

/* Initialise flash controller (clear sticky status). */
void msp432_flash_init(void);

/* Program size bytes at address. size and address must be 4-byte aligned. */
int msp432_flash_write(uintptr_t address, const void *data, size_t size);

/* Erase the 16 KiB block(s) that cover [address, address+size). */
int msp432_flash_erase(uintptr_t address, size_t size);

/* Return true when every byte in the range reads 0xFF. */
bool msp432_flash_is_blank(uintptr_t address, size_t size);

/* Toggle flash mirror mode (FMME). Both banks must contain identical code. */
int msp432_flash_swap_bank(void);

#endif /* MSP432E401Y_FLASH_H */
