/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"

#include <string.h>

#include "flash_type1.h"

void hal_flash_init(void)
{
    flash_type1_init();
}

int hal_flash_read(uintptr_t addr, void* buf, size_t len)
{
    if (buf == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    memcpy(buf, (const void*)addr, len);
    return 0;
}

int hal_flash_write(uintptr_t addr, const void* data, size_t len)
{
    return flash_type1_write(addr, data, len);
}

int hal_flash_erase_sector(uintptr_t addr)
{
    return flash_type1_erase_sector(addr);
}
