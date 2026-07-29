/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "mock_hal_flash.h"
#include <string.h>
#include <stdio.h>

static bool error_injection_enabled = false;
static uint32_t write_count = 0;
static uint32_t erase_count = 0;

#define MOCK_FLASH_ERASE_LOG_SIZE 64U
static uintptr_t erase_log[MOCK_FLASH_ERASE_LOG_SIZE];
static size_t erase_log_count = 0U;

static bool address_in_range(uintptr_t address, uintptr_t base, size_t size)
{
    return (address >= base) && (address < (base + size));
}

static bool range_in_range(uintptr_t address, size_t size, uintptr_t base, size_t region_size)
{
    return (address >= base) && ((address + size) <= (base + region_size));
}

static bool address_is_writable(uintptr_t address)
{
    return address_in_range(address, MBK_DATA0_BASE, MBK_BLOCK_SIZE) ||
           address_in_range(address, MBK_DATA1_BASE, MBK_BLOCK_SIZE) ||
           address_in_range(address, TANEUE_PROGRESS_BASE, TANEUE_PROGRESS_SIZE) ||
           address_in_range(address, MBK_SLOT0_BASE, MBK_SLOT_SIZE) ||
           address_in_range(address, MBK_SLOT1_BASE, MBK_SLOT_SIZE);
}

static bool sector_range_is_erasable(uintptr_t sector_addr)
{
    return range_in_range(sector_addr, MOCK_FLASH_SECTOR_SIZE, MBK_DATA0_BASE, MBK_BLOCK_SIZE) ||
           range_in_range(sector_addr, MOCK_FLASH_SECTOR_SIZE, MBK_DATA1_BASE, MBK_BLOCK_SIZE) ||
           range_in_range(sector_addr, MOCK_FLASH_SECTOR_SIZE, TANEUE_PROGRESS_BASE, TANEUE_PROGRESS_SIZE) ||
           range_in_range(sector_addr, MOCK_FLASH_SECTOR_SIZE, MBK_SLOT0_BASE, MBK_SLOT_SIZE) ||
           range_in_range(sector_addr, MOCK_FLASH_SECTOR_SIZE, MBK_SLOT1_BASE, MBK_SLOT_SIZE);
}

struct mapped_region {
    uintptr_t base;
    size_t size;
};

#define MOCK_MAX_MAPPED_REGIONS 8U
static struct mapped_region mapped_regions[MOCK_MAX_MAPPED_REGIONS];
static size_t mapped_region_count = 0U;

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# include <windows.h>

static void *map_memory(uintptr_t address, size_t size)
{
    return VirtualAlloc(
        (void *)address,
        size,
        MEM_RESERVE | MEM_COMMIT,
        PAGE_READWRITE);
}
# if 0
static void unmap_memory(uintptr_t address, size_t size)
{
    (void)size;

    if (address) {
        VirtualFree((void *)address, 0, MEM_RELEASE);
    }
}
# endif

static size_t get_mapping_granularity(void)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (size_t)info.dwAllocationGranularity;
}
#else
# include <sys/mman.h>
# include <unistd.h>

static void *map_memory(uintptr_t address, size_t size)
{
    void *ptr = mmap(
        (void *)address,
        size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED,
        -1,
        0);

    return (ptr == MAP_FAILED) ? NULL : ptr;
}

static void unmap_memory(uintptr_t address, size_t size)
{
    if (address) {
        munmap((void *)address, size);
    }
}

static size_t get_mapping_granularity(void)
{
    return (size_t)sysconf(_SC_PAGESIZE);
}
#endif

static uintptr_t align_down(uintptr_t value, size_t alignment)
{
    return value & ~((uintptr_t)alignment - 1U);
}

static uintptr_t align_up(uintptr_t value, size_t alignment)
{
    return (value + (uintptr_t)alignment - 1U) & ~((uintptr_t)alignment - 1U);
}

static bool map_memory_once(uintptr_t address, size_t size)
{
    const size_t granularity = get_mapping_granularity();
    const uintptr_t map_base = align_down(address, granularity);
    const uintptr_t map_end = align_up(address + size, granularity);
    const size_t map_size = (size_t)(map_end - map_base);

    for (size_t i = 0; i < mapped_region_count; ++i) {
        const uintptr_t base = mapped_regions[i].base;
        const uintptr_t end = base + mapped_regions[i].size;
        if (map_base >= base && map_end <= end) {
            return true;
        }
    }

    if (map_memory(map_base, map_size) != (void*)map_base) {
        return false;
    }

    if (mapped_region_count < MOCK_MAX_MAPPED_REGIONS) {
        mapped_regions[mapped_region_count].base = map_base;
        mapped_regions[mapped_region_count].size = map_size;
        mapped_region_count++;
    }
    return true;
}

int hal_flash_init(void)
{
    if (!mock_flash_reset()) {
        return -1;
    }
    return 0;
}

int hal_flash_write(uintptr_t address, const void* data, size_t size)
{
    if (error_injection_enabled) {
        error_injection_enabled = false;
        return -1;
    }

    if (!address_is_writable(address)) {
        fprintf(stderr, "Mock Flash: Write out of bounds at 0x%p\n", (void*)address);
        return -1;
    }
    if (data == NULL) {
        return -1;
    }

    uint8_t* dst = (uint8_t*)address;
    /* Simulate flash characteristics: only 1->0 changes are allowed */
    const uint8_t* src = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        /* Check that bits can only change from 1->0 */
        uint8_t current = dst[i];
        uint8_t new_val = src[i];

        /* Error if trying to change 0->1 (erase required) */
        /* Check if any 0 bits in current are being set to 1 in new_val */
        if ((~current & new_val) != 0) {
            fprintf(stderr, "Mock Flash: Cannot write 0->1 without erase at 0x%p\n",
                    (void*)(address + (uintptr_t)i));
            return -1;
        }

        dst[i] = new_val;
    }

    write_count++;
    return 0;
}

int hal_flash_erase_sector(uintptr_t address)
{
    if (error_injection_enabled) {
        error_injection_enabled = false;
        return -1;
    }

    if (!address_is_writable(address)) {
        fprintf(stderr, "Mock Flash: Erase out of bounds at 0x%p\n", (void*)address);
        return -1;
    }
    /* Align to sector boundary */
    uintptr_t sector_addr = (address / MOCK_FLASH_SECTOR_SIZE) * MOCK_FLASH_SECTOR_SIZE;
    if (!sector_range_is_erasable(sector_addr)) {
        fprintf(stderr, "Mock Flash: Erase out of bounds (addr=0x%p)\n", (void*)address);
        return -1;
    }

    /* Fill the sector with 0xFF */
    memset((void*)sector_addr, 0xFF, MOCK_FLASH_SECTOR_SIZE);

    if (erase_log_count < MOCK_FLASH_ERASE_LOG_SIZE) {
        erase_log[erase_log_count++] = sector_addr;
    }

    erase_count++;
    return 0;
}

int hal_flash_erase_all(void)
{
    if (error_injection_enabled) {
        error_injection_enabled = false;
        return -1;
    }

    memset((void*)MBK_DATA0_BASE, 0xFF, MBK_BLOCK_SIZE);
    memset((void*)MBK_DATA1_BASE, 0xFF, MBK_BLOCK_SIZE);
    memset((void*)TANEUE_PROGRESS_BASE, 0xFF, TANEUE_PROGRESS_SIZE);
    memset((void*)MBK_SLOT0_BASE, 0xFF, MBK_SLOT_SIZE);
    memset((void*)MBK_SLOT1_BASE, 0xFF, MBK_SLOT_SIZE);
    erase_count++;
    return 0;
}

/* Mock control functions */

bool mock_flash_reset(void)
{
    static bool initialized = false;
    if (!initialized) {
        if (!map_memory_once(MBK_DATA0_BASE, MBK_BLOCK_SIZE)) {
            fprintf(stderr, "Mock Flash: Failed to map bfl sector 0 at 0x%p\n", (void*)MBK_DATA0_BASE);
            return false;
        }
        if (!map_memory_once(MBK_DATA1_BASE, MBK_BLOCK_SIZE)) {
            fprintf(stderr, "Mock Flash: Failed to map bfl sector 1 at 0x%p\n", (void*)MBK_DATA1_BASE);
            return false;
        }
        if (!map_memory_once(TANEUE_PROGRESS_BASE, TANEUE_PROGRESS_SIZE)) {
            fprintf(stderr, "Mock Flash: Failed to map progress area at 0x%p\n", (void*)TANEUE_PROGRESS_BASE);
            return false;
        }
        if (!map_memory_once(MBK_SLOT0_BASE, MBK_SLOT_SIZE)) {
            fprintf(stderr, "Mock Flash: Failed to map slot 0 at 0x%p\n", (void*)MBK_SLOT0_BASE);
            return false;
        }
        if (!map_memory_once(MBK_SLOT1_BASE, MBK_SLOT_SIZE)) {
            fprintf(stderr, "Mock Flash: Failed to map slot 1 at 0x%p\n", (void*)MBK_SLOT1_BASE);
            return false;
        }
        initialized = true;
    }
    memset((void*)MBK_DATA0_BASE, 0xFF, MBK_BLOCK_SIZE);
    memset((void*)MBK_DATA1_BASE, 0xFF, MBK_BLOCK_SIZE);
    memset((void*)TANEUE_PROGRESS_BASE, 0xFF, TANEUE_PROGRESS_SIZE);
    memset((void*)MBK_SLOT0_BASE, 0xFF, MBK_SLOT_SIZE);
    memset((void*)MBK_SLOT1_BASE, 0xFF, MBK_SLOT_SIZE);
    error_injection_enabled = false;
    write_count = 0;
    erase_count = 0;
    return true;
}

void mock_flash_set_memory(uintptr_t address, const void* data, size_t size)
{
    if (!address_is_writable(address)) {
        fprintf(stderr, "Mock Flash: Write out of bounds at 0x%p\n", (void*)address);
        return;
    }
    if (data == NULL) {
        return;
    }

    memcpy((void*)address, data, size);
}

void mock_flash_inject_error(bool enable)
{
    error_injection_enabled = enable;
}

uint32_t mock_flash_get_write_count(void)
{
    return write_count;
}

uint32_t mock_flash_get_erase_count(void)
{
    return erase_count;
}

void mock_flash_reset_stats(void)
{
    write_count = 0;
    erase_count = 0;
    erase_log_count = 0U;
}

uint32_t mock_flash_count_erases_in_range(uintptr_t base, size_t size)
{
    uint32_t count = 0U;

    for (size_t i = 0U; i < erase_log_count; ++i) {
        if (erase_log[i] >= base && erase_log[i] < base + size) {
            count++;
        }
    }

    return count;
}
