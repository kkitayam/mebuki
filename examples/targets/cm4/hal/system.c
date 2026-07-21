#include "system.h"

/**
 * @file system.c
 * @brief システム初期化実装 (Cortex-M4)
 *
 * - Renode 環境では CPU クロックはデフォルト 32MHz（設定不要）
 * - NVIC は platform.repl で初期化済み（設定不要）
 * - prepare_handoff: ユーザアプリへの制御移譲前処理
 */

/* ============================================================================
 * Cortex-M4 レジスタ定義
 * ========================================================================== */

/**
 * @brief PRIMASK レジスタ (割り込みマスクレジスタ)
 *
 * ARM Cortex-M 割り込み制御用：
 * - bit 0 = 1: 割り込み禁止
 * - bit 0 = 0: 割り込み有効
 */
static inline void __disable_irq(void)
{
    __asm__ volatile("cpsid i" : : : "memory");
}

static inline void __enable_irq(void)
{
    __asm__ volatile("cpsie i" : : : "memory");
}

/* ============================================================================
 * 実装
 * ========================================================================== */

void _init(void) {
    /* C ランタイム初期化用ダミー関数 */
}

void system_init(void)
{
    /*
     * Renode Cortex-M4 環境での初期化:
     * - CPU クロック: Renode デフォルト 32MHz（設定不要）
     * - NVIC: platform.repl で自動初期化（設定不要）
     * - スタックポインタ: startup.s で設定済み
     *
     * よって、ここでは特に処理なし
     * （実ボード対応時に必要に応じて拡張）
     */
}

void system_reset(void)
{
    /*
     * Cortex-M4 のシステムリセット
     * NVIC_SystemReset() は CMSIS 依存のため、直接レジスタ操作で実装
     */
    const uint32_t AIRCR_VECTKEY = 0x5FAU;
    const uint32_t AIRCR_SYSRESETREQ = (1U << 2);
    volatile uint32_t* AIRCR = (volatile uint32_t*)0xE000ED0C;

    /* VECTKEY を設定して SYSRESETREQ ビットをセット */
    *AIRCR = (AIRCR_VECTKEY << 16) | AIRCR_SYSRESETREQ;

    /* リセットが発生するまで無限ループ */
    while (1) {
        __asm__ volatile("nop");
    }
}

void prepare_handoff(void)
{
    /*
     * ユーザアプリケーションへ制御移譲前の準備
     * - グローバル割り込み禁止
     *   （要求事項: 割り込みは使用しない）
     */
    __disable_irq();
}

__attribute__((noreturn))
void jump_to_firmware(uint32_t entry_point)
{
    const uint32_t* vector_table = (const uint32_t*)entry_point;
    __asm__ volatile ("MSR msp, %0" : : "r" (vector_table[0]) : );
    typedef void (*app_entry_t)(void);
    app_entry_t app_entry = (app_entry_t)vector_table[1];
    app_entry();
    halt();
}

void halt(void)
{
    /*
     * システム停止: 割り込みを禁止して無限ループ
     * WFI (Wait For Interrupt) で電力消費削減
     */
    __disable_irq();

    while (1) {
        __asm__ volatile("wfi");
    }
}
