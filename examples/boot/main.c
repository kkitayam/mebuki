/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "uart.h"
#include "system.h"
#include "flash.h"

#include "mebuki.h"
#include "taneue.h"

void uart_printf(const char* fmt, ...)
{
    uart_puts(fmt);
}

static void put_hex(uint32_t value, bool put_leading_zero)
{
    int i = 7;
    /* skip leading zeros */
    if (!put_leading_zero) {
        do {
            uint8_t nibble = (value >> (i * 4)) & 0x0F;
            if (nibble) { break;}
            --i;
        } while (i > 0);
    }
    for (; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0x0F;
        uart_putc(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
    }
}

static void put_dec(uint32_t value)
{
    char buffer[10];
    size_t index = 0;

    if (value == 0U) {
        uart_putc('0');
        return;
    }

    while (value > 0U) {
        buffer[index++] = (char)('0' + (value % 10U));
        value /= 10U;
    }
    while (index > 0U) {
        uart_putc(buffer[--index]);
    }
}

static void put_error_code(const char* message, int code)
{
    uart_puts(message);
    if (code < 0) {
        uart_putc('-');
        put_dec((uint32_t)(-code));
    } else {
        put_dec((uint32_t)code);
    }
    uart_putc(')');
    uart_puts("\r\n");
}

#if 0
/* for debug */
static void dump(const void* beg, const void* end)
{
    const uint8_t* p = (const uint8_t*)beg;
    const uint8_t* e = (const uint8_t*)end;

    for (; p < e; ++p) {
        uintptr_t addr = (uintptr_t)p;
        if ((addr % 16) == 0) {
            uart_puts("\r\n");
            uart_printf("%08X: ", (unsigned int)addr);
        }
        uint8_t byte = *p;
        uint8_t nibble = (byte >> 4) & 0x0F;
        uart_putc(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
        nibble = byte & 0x0F;
        uart_putc(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
        uart_puts(" ");
    }
    uart_puts("\r\n");
}
#endif

int main(void)
{
    system_init();
    uart_init();

    uart_puts("\r\n");
    uart_puts("==================================================\r\n");
    uart_puts("Boot Software (mebuki)\r\n");
    uart_puts("==================================================\r\n");

    uart_puts("swap slots if needed...\r\n");
    enum taneue_result err = taneue_swap_if_scheduled();
    if (err != TANEUE_SUCCESS) {
        put_error_code("ERROR: Failed to perform scheduled slot swap (code: ", (int)err);
        halt();
    }

    uart_puts("Initializing mebuki...\r\n");
    hal_flash_init();

    struct mbk_context ctx;
    enum mbk_result result = mbk_init(&ctx);

    if (result != MBK_SUCCESS) {
        put_error_code("ERROR: mbk_init failed (code: ", (int)result);
        halt();
    }

    uart_puts("Finding bootable slot...\r\n");

    struct mbk_boot_info boot_info;
    result = mbk_find_bootable_slot(&ctx, &boot_info);

    if (result != MBK_SUCCESS) {
        put_error_code("ERROR: No bootable slot found (code: ", (int)result);
        halt();
    }

    /* print boot information */

    uart_puts("Bootable slot found!\r\n");
    uart_puts("  Slot ID: ");
    uart_putc('0' + boot_info.slot_id);
    uart_puts("\r\n");

    uart_puts("  Security Version: 0x");
    uint16_t sv = boot_info.header->security_version;
    put_hex(sv, false);
    uart_puts("\r\n");

    uart_puts("  Key Generation: 0x");
    put_hex(boot_info.header->key_generation, false);
    uart_puts("\r\n");

    uart_puts("  Software Size: 0x");
    uint32_t size = boot_info.header->software_size;
    put_hex(size, false);
    uart_puts("\r\n");

    uart_puts("  Entry Point: 0x");
    uint32_t entry = boot_info.entry_point;
    put_hex(entry, true);
    uart_puts("\r\n");

    uart_puts("Booting slot ");
    uart_putc('0' + boot_info.slot_id);
    uart_puts("...\r\n");
    uart_puts("\r\n");

#if 0
    dump((const uint8_t*)MBK_DATA0_BASE, (const uint8_t*)MBK_DATA0_BASE + 64);
    dump((const uint8_t*)MBK_DATA1_BASE, (const uint8_t*)MBK_DATA1_BASE + 64);
#endif
    if (boot_info.slot_id == 1) {
        /* The application software is built to run from slot0, so a swap is necessary when booting from slot1 */
        uart_puts("Scheduling slot swap...\r\n");
        enum taneue_result result = taneue_schedule_swap();
        if (result != TANEUE_SUCCESS) {
            put_error_code("ERROR: Failed to schedule slot swap (code: ", (int)result);
            halt();
        }
        uart_puts("Slot swap scheduled\r\n");
        system_reset();  /* swap operation is deferred until the next boot */
    }

    prepare_handoff();
    jump_to_firmware(boot_info.entry_point);

    /* unreachable */
    halt();
    return 0;
}
