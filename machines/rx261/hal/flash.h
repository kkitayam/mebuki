/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef FLASH_H
#define FLASH_H

#include <stddef.h>
#include <stdint.h>

void hal_flash_init(void);
int hal_flash_read(uintptr_t addr, void* buf, size_t len);
int hal_flash_write(uintptr_t addr, const void* data, size_t len);
int hal_flash_erase_sector(uintptr_t addr);

#endif /* FLASH_H */
