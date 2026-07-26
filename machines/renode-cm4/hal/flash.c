/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"
#include "target_config.h"
#include <string.h>

/**
 * Renode's MappedMemory is writable and simulates Flash characteristics:
 * - Read: implemented with memcpy (XiP)
 * - Write: direct memory operation + verify
 * - Erase: fill the entire sector with 0xFF
 */

/* ============================================================================
 * Parameter validation macros
 * ========================================================================== */

/* Check if the address is within the Flash region */
#if FLASH_BASE == 0
# define IS_FLASH_ADDR(addr) ((addr) < FLASH_SIZE)
#else
# define IS_FLASH_ADDR(addr) ((addr) >= FLASH_BASE && (addr) < (FLASH_BASE + FLASH_SIZE))
#endif

/* Check if the range is within the Flash region */
#define IS_FLASH_RANGE(addr, len) \
    (IS_FLASH_ADDR(addr) && IS_FLASH_ADDR((addr) + (len) - 1))

/* Check if the address is sector-aligned */
#define IS_SECTOR_ALIGNED(addr) \
    (((addr) & (MBK_BLOCK_SIZE - 1)) == 0)

/* ============================================================================
 * Implementation
 * ========================================================================== */

int hal_flash_read(uint32_t addr, void* buf, size_t len)
{
    if (buf == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (!IS_FLASH_RANGE(addr, len)) {
        return -1;
    }

    /* XiP (memory-mapped): memcpy for reading */
    memcpy(buf, (const void*)addr, len);

    return 0;
}

int hal_flash_write(uint32_t addr, const void* data, size_t len)
{
    /* Parameter validation */
    if (data == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (!IS_FLASH_RANGE(addr, len)) {
        return -1;
    }

    /*
     * Flash characteristics check: 0->1 transitions are not allowed
     * However, Renode MappedMemory has no such restriction, so pre-check is omitted
     * (To be added for real board support)
     */

    /* Memory write */
    memcpy((void*)addr, data, len);

    /* Verify: read back and check */
    if (memcmp((const void*)addr, data, len) != 0) {
        return -1;
    }

    return 0;
}

int hal_flash_erase_sector(uint32_t addr)
{
    /* Parameter validation */
    if (!IS_FLASH_ADDR(addr)) {
        return -1;
    }

    if (!IS_SECTOR_ALIGNED(addr)) {
        return -1;
    }

    /* Fill the entire sector with 0xFF */
    memset((void*)addr, 0xFF, MBK_BLOCK_SIZE);

    /* Verify */
    for (size_t i = 0; i < MBK_BLOCK_SIZE; i++) {
        if (((unsigned char*)addr)[i] != 0xFF) {
            return -1;
        }
    }

    return 0;
}
