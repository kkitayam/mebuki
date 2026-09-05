/*
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2014 Renesas Electronics Corporation.
 * Derived from Renesas FIT r_flash_rx (Flash Type 1 / no-FCU).
 */

#ifndef FLASH_TYPE1_H
#define FLASH_TYPE1_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Blank check: default (0) is CPU SWHILE.B. Set
 * FLASH_TYPE1_USE_HW_BLANK_CHECK=1 for the flash controller command.
 * At 64 MHz, SWHILE.B is faster for CF and DF; HW may help only at
 * a much lower CPU clock.
 */

/* Enable Data Flash access. Call once before write or erase. */
void flash_type1_init(void);

/*
 * Write data to Code Flash or Data Flash.
 * address : destination
 * data    : source buffer
 * size    : byte count (CF must be a multiple of 8)
 * Returns 0 on success, or a negative value on error.
 */
int flash_type1_write(uintptr_t address, const void *data, size_t size);

/*
 * Erase flash that starts at address.
 * size is the number of bytes to erase. The driver floors size to the
 * erase unit (DF: 256 bytes, CF: 2048 bytes) and erases that many bytes.
 * Returns the number of bytes erased, or a negative value on error.
 */
int flash_type1_erase(uintptr_t address, size_t size);

/*
 * Return true if [address, address + size) is blank (0xFF).
 * size is the number of bytes to check. On error, returns false.
 */
bool flash_type1_is_blank(uintptr_t address, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* FLASH_TYPE1_H */
