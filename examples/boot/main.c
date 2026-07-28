/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

#include "uart.h"
#include "system.h"

#include "mebuki.h"
#include "taneue.h"

void uart_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    uart_puts(buffer);
    va_end(args);
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
        uart_printf("ERROR: Failed to perform scheduled slot swap (code: %d)\r\n", (int)err);
        halt();
    }

    uart_puts("Initializing mebuki...\r\n");

    struct mbk_context ctx;
    enum mbk_result result = mbk_init(&ctx);

    if (result != MBK_SUCCESS) {
        uart_printf("ERROR: mbk_init failed (code: %d)\r\n", (int)result);
        halt();
    }

    uart_puts("Finding bootable slot...\r\n");

    struct mbk_boot_info boot_info;
    result = mbk_find_bootable_slot(&ctx, &boot_info);

    if (result != MBK_SUCCESS) {
        uart_printf("ERROR: No bootable slot found (code: %d)\r\n", (int)result);
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
            uart_printf("ERROR: Failed to schedule slot swap (code: %d)\r\n", (int)result);
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
