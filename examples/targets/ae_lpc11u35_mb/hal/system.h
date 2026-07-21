#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

/**
 * @file system.h
 * @brief AE LPC11U35-MB のシステム初期化 HAL
 */

/**
 * @brief システム初期化を行う。
 *
 * クロックと周辺回路の最低限の初期化を行う。
 */
void system_init(void);

/**
 * @brief システムリセットを行う。
 *
 * システムリセットを行い、リセット後はブートローダーから再起動する。
 */
void system_reset(void);

/**
 * @brief 割り込みを禁止し、ハンドオフ前の状態にする。
 */
void prepare_handoff(void);

/**
 * @brief ベクタテーブル先頭のアプリケーションへジャンプする。
 *
 * @param entry_point アプリケーションのベクタテーブル先頭アドレス
 */
void jump_to_firmware(uint32_t entry_point) __attribute__((noreturn));

/**
 * @brief エラー時の無限待機ループへ入る。
 */
void halt(void) __attribute__((noreturn));

#endif /* SYSTEM_H */