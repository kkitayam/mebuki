/**
 * @file boot/main.c
 * @brief Boot Software メインロジック
 *
 * 責務（ハードウェア非依存):
 * - HAL インターフェース経由でシステム初期化
 * - libmebuki を使用してスロット検索・起動
 * - ユーザアプリケーションへ制御移譲
 *
 * 処理フロー:
 * 1. system_init()          - ハードウェア初期化 (Renode 環境では NOP)
 * 2. uart_init()            - UART 初期化 (115200 bps)
 * 3. uart_puts()            - ログ出力
 * 4. mbk_init()             - libmebuki 初期化
 * 5. mbk_find_bootable_slot() - 起動可能スロット検索
 * 6. prepare_handoff()      - 制御移譲準備 (割り込み禁止)
 * 7. エントリポイント→ジャンプ
 * 8. エラー時: halt()
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>

/* HAL インターフェース */
#include "uart.h"
#include "system.h"

/* libmebuki インターフェース */
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

/* ============================================================================
 * エントリポイント
 * ========================================================================== */

int main(void)
{
    /* ========================================================
     * ハードウェア初期化
     * ======================================================== */

    system_init();
    uart_init();

    uart_puts("\r\n");
    uart_puts("==================================================\r\n");
    uart_puts("Boot Software (libmebuki)\r\n");
    uart_puts("==================================================\r\n");

    /* ========================================================
     * taneue 進捗管理領域の初期化
     * ======================================================== */
    uart_puts("swap slots if needed...\r\n");
    enum taneue_result err = taneue_swap_if_scheduled();
    if (err != TANEUE_SUCCESS) {
        uart_printf("ERROR: Failed to perform scheduled slot swap (code: %d)\r\n", (int)err);
        halt();
    }

    /* ========================================================
     * mebuki 初期化
     * ======================================================== */

    uart_puts("Initializing mebuki...\r\n");

    struct mbk_context ctx;
    enum mbk_result result = mbk_init(&ctx);

    if (result != MBK_SUCCESS) {
        uart_printf("ERROR: mbk_init failed (code: %d)\r\n", (int)result);
        halt();
    }

    uart_puts("mebuki initialized\r\n");

    /* ========================================================
     * 起動可能スロット検索
     * ======================================================== */

    uart_puts("Finding bootable slot...\r\n");

    struct mbk_boot_info boot_info;
    result = mbk_find_bootable_slot(&ctx, &boot_info);

    if (result != MBK_SUCCESS) {
        uart_printf("ERROR: No bootable slot found (code: %d)\r\n", (int)result);
        halt();
    }

    /* ========================================================
     * ログ出力
     * ======================================================== */

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
    dump((const uint8_t*)MBK_DATA_BASE, (const uint8_t*)MBK_DATA_BASE + 64);
    dump((const uint8_t*)MBK_DATA_BASE + MBK_BLOCK_SIZE, (const uint8_t*)MBK_DATA_BASE + MBK_BLOCK_SIZE + 64);
#endif
    /* ========================================================
     * slot1 の場合、内容の交換を計画
     * ======================================================== */
    if (boot_info.slot_id == 1) {
        uart_puts("Scheduling slot swap...\r\n");
        enum taneue_result result = taneue_schedule_swap();
        if (result != TANEUE_SUCCESS) {
            uart_printf("ERROR: Failed to schedule slot swap (code: %d)\r\n", (int)result);
            halt();
        }
        uart_puts("Slot swap scheduled\r\n");
        system_reset();  /* 再起動して slot0 を起動 */
    }

    /* ========================================================
     * 制御移譲準備と ジャンプ
     * ======================================================== */

    prepare_handoff();

    /*
     * エントリポイントへジャンプ
     * entry_point は Slot 0 または Slot 1 のコード開始位置
     */
    jump_to_firmware(boot_info.entry_point);

    /* 到達しない */
    halt();

    return 0;  /* 警告回避 */
}
