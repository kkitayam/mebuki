/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"
#include "mebuki_config.h"
#include <stdbool.h>
#include <string.h>

#ifndef TANEUE_PROGRESS_BASE
#  define TANEUE_PROGRESS_BASE \
      (((MBK_DATA0_BASE > MBK_DATA1_BASE) ? MBK_DATA0_BASE : MBK_DATA1_BASE) + MBK_BLOCK_SIZE_BFL)
#endif

#ifndef TANEUE_PROGRESS_SIZE
#  define TANEUE_PROGRESS_SIZE MBK_BLOCK_SIZE_PROGRESS
#endif

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

struct flash_region {
    uint32_t base;
    size_t size;
    size_t erase_size;
};

static bool in_region(uint32_t addr, const struct flash_region* region)
{
    const uint32_t end = region->base + (uint32_t)region->size;
    return addr >= region->base && addr < end;
}

static bool flash_region_for_address(uint32_t addr, struct flash_region* out)
{
    const struct flash_region regions[] = {
        {MBK_DATA0_BASE, MBK_BLOCK_SIZE_BFL, MBK_BLOCK_SIZE_BFL},
        {MBK_DATA1_BASE, MBK_BLOCK_SIZE_BFL, MBK_BLOCK_SIZE_BFL},
        {TANEUE_PROGRESS_BASE, TANEUE_PROGRESS_SIZE, MBK_BLOCK_SIZE_PROGRESS},
        {MBK_SLOT0_BASE, MBK_SLOT_SIZE, MBK_BLOCK_SIZE_SLOT},
        {MBK_SLOT1_BASE, MBK_SLOT_SIZE, MBK_BLOCK_SIZE_SLOT},
    };

    for (size_t i = 0; i < (sizeof(regions) / sizeof(regions[0])); i++) {
        if (in_region(addr, &regions[i])) {
            *out = regions[i];
            return true;
        }
    }

    return false;
}

/* ============================================================================
 * Implementation
 * ========================================================================== */

void hal_flash_init(void)
{
}

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
    struct flash_region region;

    /* Parameter validation */
    if (!IS_FLASH_ADDR(addr)) {
        return -1;
    }

    if (!flash_region_for_address(addr, &region)) {
        return -1;
    }

    if (((size_t)(addr - region.base) % region.erase_size) != 0U) {
        return -1;
    }

    if (!IS_FLASH_RANGE(addr, region.erase_size)) {
        return -1;
    }

    if (((uintptr_t)addr + region.erase_size) > ((uintptr_t)region.base + region.size)) {
        return -1;
    }

    /* Fill the entire sector with 0xFF */
    memset((void*)addr, 0xFF, region.erase_size);

    /* Verify */
    for (size_t i = 0; i < region.erase_size; i++) {
        if (((unsigned char*)addr)[i] != 0xFF) {
            return -1;
        }
    }

    return 0;
}
