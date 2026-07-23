#include "flash.h"
#include "target_config.h"
#include <string.h>

/**
 * @file flash.c
 * @brief Flash メモリ操作実装 (Renode MappedMemory)
 *
 * Renode の MappedMemory は書き込み可能で、Flash 特性をシミュレート
 * - 読み出し: memcpy で実装 (XiP)
 * - 書き込み: メモリ直接操作 + ベリファイ
 * - 消去: セクター全体を 0xFF で埋める
 */

/* ============================================================================
 * パラメータ検証用マクロ
 * ========================================================================== */

/** @brief アドレスが Flash 領域内か確認 */
#if FLASH_BASE == 0
# define IS_FLASH_ADDR(addr) ((addr) < FLASH_SIZE)
#else
# define IS_FLASH_ADDR(addr) ((addr) >= FLASH_BASE && (addr) < (FLASH_BASE + FLASH_SIZE))
#endif

/** @brief 長さが Flash 領域内に収まるか確認 */
#define IS_FLASH_RANGE(addr, len) \
    (IS_FLASH_ADDR(addr) && IS_FLASH_ADDR((addr) + (len) - 1))

/** @brief セクター開始アドレスか確認 */
#define IS_SECTOR_ALIGNED(addr) \
    (((addr) & (MBK_BLOCK_SIZE - 1)) == 0)

/* ============================================================================
 * 実装
 * ========================================================================== */

int hal_flash_read(uint32_t addr, void* buf, size_t len)
{
    /* パラメータ検証 */
    if (buf == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (!IS_FLASH_RANGE(addr, len)) {
        return -1;
    }

    /* XiP (メモリマッピング): memcpy で読み出し */
    memcpy(buf, (const void*)addr, len);

    return 0;
}

int hal_flash_write(uint32_t addr, const void* data, size_t len)
{
    /* パラメータ検証 */
    if (data == NULL) {
        return -1;
    }

    if (len == 0) {
        return 0;
    }

    if (!IS_FLASH_RANGE(addr, len)) {
        return -1;
    }

    /*
     * Flash 特性チェック: 0→1 の変更が不可
     * ただし Renode MappedMemory は制約がないため、事前チェック省略
     * (実ボード対応時に追加)
     */

    /* メモリ書き込み */
    memcpy((void*)addr, data, len);

    /* ベリファイ: 読み戻してチェック */
    if (memcmp((const void*)addr, data, len) != 0) {
        return -1;
    }

    return 0;
}

int hal_flash_erase_sector(uint32_t addr)
{
    /* パラメータ検証 */
    if (!IS_FLASH_ADDR(addr)) {
        return -1;
    }

    if (!IS_SECTOR_ALIGNED(addr)) {
        return -1;
    }

    /* セクター全体を 0xFF で埋める */
    memset((void*)addr, 0xFF, MBK_BLOCK_SIZE);

    /* ベリファイ */
    for (size_t i = 0; i < MBK_BLOCK_SIZE; i++) {
        if (((unsigned char*)addr)[i] != 0xFF) {
            return -1;
        }
    }

    return 0;
}
