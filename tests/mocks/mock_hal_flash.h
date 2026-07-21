/**
 * @file mock_hal_flash.h
 * @brief Flash HAL Mock for Testing
 * 
 * テスト用のFlash HALモック実装
 * ホストPC上での単体テスト実行を可能にする
 */

#ifndef MOCK_HAL_FLASH_H
#define MOCK_HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mebuki_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mock Flash configuration */
#define MOCK_FLASH_SIZE         (1024 * 1024)  // 1MB
#define MOCK_FLASH_SECTOR_SIZE  4096           // 4KB sector
#define MOCK_FLASH_PAGE_SIZE    256            // 256 bytes page

/**
 * @brief Flash HAL初期化（モック）
 * @return 常に成功
 */
int hal_flash_init(void);

/**
 * @brief Flash書き込み
 * @param[in] address Flash内のアドレス
 * @param[in] data 書き込みデータ
 * @param[in] size 書き込みサイズ
 * @return 成功時0、失敗時-1
 */
int hal_flash_write(uintptr_t address, const void* data, size_t size);

/**
 * @brief Flashセクター消去
 * @param[in] address セクター先頭アドレス
 * @return 成功時0、失敗時-1
 */
int hal_flash_erase_sector(uintptr_t address);

/**
 * @brief Flash全消去
 * @return 成功時0、失敗時-1
 */
int hal_flash_erase_all(void);

/* Mock control functions for testing */

/**
 * @brief モックFlashメモリをリセット（全0xFF）
 */
bool mock_flash_reset(void);

/**
 * @brief モックFlashの特定アドレスの値を設定
 * @param[in] address アドレス
 * @param[in] data データバッファ
 * @param[in] size データサイズ
 */
void mock_flash_set_memory(uintptr_t address, const void* data, size_t size);

/**
 * @brief エラー注入（次の操作を失敗させる）
 * @param[in] enable trueでエラー注入有効
 */
void mock_flash_inject_error(bool enable);

/**
 * @brief 書き込み回数カウンタを取得
 * @return 書き込み回数
 */
uint32_t mock_flash_get_write_count(void);

/**
 * @brief 消去回数カウンタを取得
 * @return 消去回数
 */
uint32_t mock_flash_get_erase_count(void);

/**
 * @brief 統計情報をリセット
 */
void mock_flash_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_HAL_FLASH_H */
