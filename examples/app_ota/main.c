/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include <stdint.h>

#include "flash.h"
#include "mebuki_config.h"
#include "system.h"
#include "uart.h"
#include "ymodem.h"

static void put_dec(uint32_t value)
{
    char digits[10];
    uint32_t count = 0U;

    if (value == 0U) {
        uart_putc('0');
        return;
    }

    while (value > 0U) {
        digits[count++] = (char)('0' + value % 10U);
        value /= 10U;
    }
    while (count > 0U) {
        uart_putc(digits[--count]);
    }
}

static int erase_slot1(void)
{
    for (uint32_t offset = 0U; offset < MBK_SLOT_SIZE; offset += MBK_BLOCK_SIZE_SLOT) {
        if (hal_flash_erase_sector(MBK_SLOT1_BASE + offset) != 0) {
            return -1;
        }
    }
    return 0;
}

int main(void)
{
#ifndef MSP432E4_STARTUP_INITIALIZES_SYSTEM
    system_init();
#endif
    uart_init();
    hal_flash_init();

    if (erase_slot1() != 0) {
        uart_puts("APP_OTA: slot1 erase failed\r\n");
        halt();
    }

    uart_puts("\r\nAPP_OTA: waiting for YMODEM image...\r\n");

    const int32_t received = ymodem_receive((uint8_t *)MBK_SLOT1_BASE, MBK_SLOT_SIZE, NULL);
    if (received <= 0) {
        uart_puts("APP_OTA: receive failed\r\n");
        halt();
    }

    uart_puts("APP_OTA: receive OK (");
    put_dec((uint32_t)received);
    uart_puts(" bytes)\r\n");
    system_reset();
}
