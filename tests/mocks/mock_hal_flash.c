/**
 * @file mock_hal_flash.c
 * @brief Flash HAL Mock Implementation
 */

#include "mock_hal_flash.h"
#include <string.h>
#include <stdio.h>

/* Internal mock storage */
static bool error_injection_enabled = false;
static uint32_t write_count = 0;
static uint32_t erase_count = 0;

#define MOCK_META_REGION_START \
    ((MBK_DATA_BASE < TANEUE_PROGRESS_BASE) ? MBK_DATA_BASE : TANEUE_PROGRESS_BASE)
#define MOCK_META_REGION_END \
    (((MBK_DATA_BASE + MBK_BLOCK_SIZE * 2U) > (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE)) \
        ? (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2U) \
        : (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE))
#define MOCK_META_REGION_SIZE (MOCK_META_REGION_END - MOCK_META_REGION_START)

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
#endif

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

    if ((address < MBK_DATA_BASE || address >= (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2)) &&
        (address < TANEUE_PROGRESS_BASE || address >= (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE)) &&
        (address < MBK_SLOT0_BASE || address >= (MBK_SLOT0_BASE + MBK_SLOT_SIZE)) &&
        (address < MBK_SLOT1_BASE || address >= (MBK_SLOT1_BASE + MBK_SLOT_SIZE))) {
        fprintf(stderr, "Mock Flash: Write out of bounds at 0x%p\n", (void*)address);
        return -1;
    }
    if (data == NULL) {
        return -1;
    }

    uint8_t* dst = (uint8_t*)address;
    /* Flash特性をシミュレート: 1→0への変更のみ可能 */
    const uint8_t* src = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        /* ビットが1→0にしか変更できないことをチェック */
        uint8_t current = dst[i];
        uint8_t new_val = src[i];

        /* 0→1への変更があればエラー（消去が必要） */
        /* currentが0のビットをnew_valで1にしようとしているかチェック */
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

    if ((address < MBK_DATA_BASE || address >= (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2)) &&
        (address < TANEUE_PROGRESS_BASE || address >= (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE)) &&
        (address < MBK_SLOT0_BASE || address >= (MBK_SLOT0_BASE + MBK_SLOT_SIZE)) &&
        (address < MBK_SLOT1_BASE || address >= (MBK_SLOT1_BASE + MBK_SLOT_SIZE))) {
        fprintf(stderr, "Mock Flash: Erase out of bounds at 0x%p\n", (void*)address);
        return -1;
    }
    /* セクター境界にアラインメント */
    uintptr_t sector_addr = (address / MOCK_FLASH_SECTOR_SIZE) * MOCK_FLASH_SECTOR_SIZE;
    uintptr_t sector_end = sector_addr + MOCK_FLASH_SECTOR_SIZE;
    if ((sector_addr < (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2) && (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2) < sector_end) &&
        (sector_addr < (MBK_SLOT0_BASE + MBK_SLOT_SIZE) && (MBK_SLOT0_BASE + MBK_SLOT_SIZE) < sector_end) &&
        (sector_addr < (MBK_SLOT1_BASE + MBK_SLOT_SIZE) && (MBK_SLOT1_BASE + MBK_SLOT_SIZE) < sector_end)) {
        fprintf(stderr, "Mock Flash: Erase out of bounds (addr=0x%p)\n", (void*)address);
        return -1;
    }

    /* セクターを0xFFで埋める */
    memset((void*)sector_addr, 0xFF, MOCK_FLASH_SECTOR_SIZE);

    erase_count++;
    return 0;
}

int hal_flash_erase_all(void)
{
    if (error_injection_enabled) {
        error_injection_enabled = false;
        return -1;
    }

    memset((void*)MBK_DATA_BASE, 0xFF, MBK_BLOCK_SIZE * 2);
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
        if (map_memory(MOCK_META_REGION_START, MOCK_META_REGION_SIZE) != (void*)MOCK_META_REGION_START) {
            fprintf(stderr, "Mock Flash: Failed to map meta region at 0x%p\n",
                    (void*)MOCK_META_REGION_START);
            return false;
        }
        if (map_memory(MBK_SLOT0_BASE, MBK_SLOT_SIZE) != (void *)MBK_SLOT0_BASE) {
            fprintf(stderr, "Mock Flash: Failed to map slot 0 at 0x%p\n", (void*)MBK_SLOT0_BASE);
            return false;
        }
        if (map_memory(MBK_SLOT1_BASE, MBK_SLOT_SIZE) != (void *)MBK_SLOT1_BASE) {
            fprintf(stderr, "Mock Flash: Failed to map slot 1 at 0x%p\n", (void*)MBK_SLOT1_BASE);
            return false;
        }
        initialized = true;
    }
    memset((void*)MBK_DATA_BASE, 0xFF, MBK_BLOCK_SIZE * 2);
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
    if (!((MBK_DATA_BASE <= address && address < (MBK_DATA_BASE + MBK_BLOCK_SIZE * 2)) ||
          (TANEUE_PROGRESS_BASE <= address && address < (TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE)) ||
          (MBK_SLOT0_BASE <= address && address < (MBK_SLOT0_BASE + MBK_SLOT_SIZE)) ||
          (MBK_SLOT1_BASE <= address && address < (MBK_SLOT1_BASE + MBK_SLOT_SIZE)))) {
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
}
