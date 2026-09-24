#include <string.h>

#include "flash.h"
#include "msp432e401y_flash.h"
#include "target_config.h"

void hal_flash_init(void)
{
    msp432_flash_init();
}

int hal_flash_read(uint32_t address, void *buffer, size_t size)
{
    if (buffer == NULL || address < MBK_FLASH_BASE ||
        address - MBK_FLASH_BASE > MBK_FLASH_SIZE ||
        size > MBK_FLASH_SIZE - (address - MBK_FLASH_BASE)) {
        return -1;
    }
    memcpy(buffer, (const void *)(uintptr_t)address, size);
    return 0;
}

int hal_flash_write(uint32_t address, const void *data, size_t size)
{
    return msp432_flash_write((uintptr_t)address, data, size);
}

int hal_flash_erase_sector(uint32_t address)
{
    return msp432_flash_erase((uintptr_t)address, FLASH_ERASE_SIZE);
}

bool hal_flash_is_blank(uintptr_t address)
{
    return msp432_flash_is_blank(address, FLASH_ERASE_SIZE);
}
