/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"

#include <string.h>

#include "mebuki_config.h"
#include "flash_type1.h"

static size_t flash_erase_size(uintptr_t addr)
{
    if ((addr >= MBK_DATA0_BASE &&
         addr < (MBK_DATA0_BASE + MBK_BLOCK_SIZE_BFL)) ||
        (addr >= MBK_DATA1_BASE &&
         addr < (MBK_DATA1_BASE + MBK_BLOCK_SIZE_BFL))) {
        return MBK_BLOCK_SIZE_BFL;
    }

    if (addr >= TANEUE_PROGRESS_BASE &&
        addr < (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE)) {
        return MBK_BLOCK_SIZE_PROGRESS;
    }

    return MBK_BLOCK_SIZE_SLOT;
}

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
    const size_t erase_size = flash_erase_size(addr);
    const int result = flash_type1_erase(addr, erase_size);

    return (result == (int)erase_size) ? 0 : result;
}

bool hal_flash_is_blank(uintptr_t addr)
{
    return flash_type1_is_blank(addr, flash_erase_size(addr));
}
