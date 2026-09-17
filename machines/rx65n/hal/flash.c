#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "flash.h"
#include "flash_type4.h"
#include "mebuki_config.h"

static size_t flash_erase_size(uintptr_t address)
{
#if defined(MEBUKI_RX65N_DATA_IN_CODE_FLASH)
    if ((address >= MBK_DATA0_BASE &&
         address < MBK_DATA0_BASE + MBK_BLOCK_SIZE_BFL) ||
        (address >= MBK_DATA1_BASE &&
         address < MBK_DATA1_BASE + MBK_BLOCK_SIZE_BFL)) {
        return MBK_BLOCK_SIZE_SLOT;
    }
#else
    if ((address >= MBK_DATA0_BASE &&
         address < MBK_DATA0_BASE + MBK_BLOCK_SIZE_BFL * 2U)) {
        return MBK_BLOCK_SIZE_BFL;
    }
#endif
    if (address >= TANEUE_PROGRESS_BASE &&
        address < TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE) {
        return MBK_BLOCK_SIZE_PROGRESS;
    }
    return MBK_BLOCK_SIZE_SLOT;
}

void hal_flash_init(void)
{
    flash_type4_init();
}

int hal_flash_read(uintptr_t address, void *buffer, size_t length)
{
    if (buffer == NULL) {
        return -1;
    }
    if (length != 0U) {
        memcpy(buffer, (const void *)address, length);
    }
    return 0;
}

int hal_flash_write(uintptr_t address, const void *data, size_t length)
{
    return flash_type4_write(address, data, length);
}

int hal_flash_erase_sector(uintptr_t address)
{
    const size_t erase_size = flash_erase_size(address);
    const int result = flash_type4_erase(address, erase_size);
    return result == (int)erase_size ? 0 : result;
}

bool hal_flash_is_blank(uintptr_t address)
{
    return flash_type4_is_blank(address, flash_erase_size(address));
}
