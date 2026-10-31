/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"
#include "system.h"
#include "target_config.h"
#include "uart.h"

#include <stdint.h>
#include <string.h>

#define OPTSR_CUR (*(volatile uint32_t *)0x5200201CU)
#define SWAP_BIT (1U << 31)
#define WORD_SIZE 32U

static void require(bool condition)
{
    if (!condition) {
        uart_puts("FLASH_HAL_TEST: FAIL\r\n");
        halt();
    }
}

int main(void)
{
    const uint8_t data[33] = {0x12, 0x34, 0x56, 0x78, 0x9A};
    uint8_t actual[64];

    uart_init();
    hal_flash_init();
    if ((OPTSR_CUR & SWAP_BIT) == 0U) {
        require(hal_flash_erase_sector(FLASH_BASE) != 0);
        require(hal_flash_erase_sector(BFL_DATA0_BASE + 1U) != 0);
        require(hal_flash_erase_sector(BFL_DATA0_BASE) == 0);
        require(hal_flash_erase_sector(BFL_DATA1_BASE) == 0);
        memset(actual, 0xFF, sizeof(actual));
        require(hal_flash_write(BFL_DATA0_BASE + FLASH_ERASE_SIZE - WORD_SIZE,
                                actual, sizeof(actual)) != 0);
        require(hal_flash_is_blank(BFL_DATA0_BASE));
        require(hal_flash_write(BFL_DATA0_BASE, data, sizeof(data)) == 0);
        require(hal_flash_read(BFL_DATA0_BASE, actual, sizeof(actual)) == 0);
        require(memcmp(actual, data, sizeof(data)) == 0);
        for (size_t i = sizeof(data); i < sizeof(actual); ++i) {
            require(actual[i] == 0xFFU);
        }
        memset(actual, 0xFF, sizeof(actual));
        require(hal_flash_write(BFL_DATA0_BASE, actual, sizeof(actual)) != 0);
        require(hal_flash_read(BFL_DATA0_BASE, actual, sizeof(data)) == 0);
        require(memcmp(actual, data, sizeof(data)) == 0);
        require(hal_flash_swap_bank() == 0);
        uart_puts("FLASH_HAL_TEST: swap scheduled\r\n");
        system_reset();
    }

    require(hal_flash_read(BFL_DATA1_BASE, actual, sizeof(data)) == 0);
    require(memcmp(actual, data, sizeof(data)) == 0);
    require(hal_flash_erase_sector(BFL_DATA0_BASE) == 0);
    require(hal_flash_write(BFL_DATA0_BASE, data, sizeof(data)) == 0);
    require(hal_flash_read(BFL_DATA0_BASE, actual, sizeof(data)) == 0);
    require(memcmp(actual, data, sizeof(data)) == 0);
    require(hal_flash_erase_sector(BFL_DATA1_BASE) == 0);
    require(hal_flash_is_blank(BFL_DATA1_BASE));
    require(hal_flash_read(BFL_DATA0_BASE, actual, sizeof(data)) == 0);
    require(memcmp(actual, data, sizeof(data)) == 0);
    uart_puts("FLASH_HAL_TEST: PASS\r\n");
    halt();
}
