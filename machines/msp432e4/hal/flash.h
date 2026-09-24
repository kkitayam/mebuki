#ifndef MSP432E4_FLASH_H
#define MSP432E4_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void hal_flash_init(void);
int hal_flash_read(uint32_t address, void *buffer, size_t size);
int hal_flash_write(uint32_t address, const void *data, size_t size);
int hal_flash_erase_sector(uint32_t address);
bool hal_flash_is_blank(uintptr_t address);

#endif
