#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

/**
 * @file system.h
 * @brief システムハードウェア抽象化層
 *
 * Cortex-M4 CPU と周辺回路の基本初期化
 */

/**
 * @brief システム初期化
 *
 * - CPU クロック設定 (Renode は デフォルト 32MHz)
 * - NVIC (割り込みコントローラ) 初期化
 * - スタックポインタ初期化 (startup.s で実施)
 */
void system_init(void);

/**
 * @brief システムリセット
 *
 * Cortex-M4 のシステムリセットを実行する。
 * NVIC_SystemReset() は CMSIS 依存のため、直接レジスタ操作で実装。
 */
void system_reset(void);

/**
 * @brief 割り込み禁止、ユーザソフトウェアへ制御移譲準備
 *
 * - グローバル割り込みを禁止 (__disable_irq)
 * - その他のハンドオフ処理
 *
 * この関数の直後、ユーザアプリケーション エントリポイントへジャンプ
 */
void prepare_handoff(void);

__attribute__((noreturn))
void jump_to_firmware(uint32_t entry_point);

/**
 * @brief システム停止 (無限ループ)
 *
 * エラー時の最後の手段。WFI ナノループで待機
 */
void halt(void) __attribute__((noreturn));

#endif /* SYSTEM_H */
