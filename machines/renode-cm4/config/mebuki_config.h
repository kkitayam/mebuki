#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

/**
 * @file mebuki_config.h
 * @brief libmebuki ライブラリ用設定
 *
 * target_config.h で定義されたメモリマップを利用して、
 * BFL, BEL, SML, SVL, BSL の動作パラメータを設定
 */

#include "target_config.h"

/* ============================================================================
 * 暗号エンジン (SVL) 設定
 * ========================================================================== */

/** @brief Ed25519 公開鍵サイズ (bytes) */
#define MBK_PUBKEY_SIZE         32U

/** @brief Ed25519 署名サイズ (bytes) */
#define MBK_SIGNATURE_SIZE      64U

/** @brief Blake2b-256 ハッシュサイズ (bytes) */
#define MBK_HASH_SIZE           32U

/** @brief 鍵世代数 */
#define MBK_NUM_KEY_GENERATIONS 8U

/* ============================================================================
 * データ永続化 (BFL) 設定
 * ========================================================================== */

/** @brief BFL 書き込み単位 (bytes) */
#define MBK_FLASH_PAGE_SIZE     256U

/** @brief BFL 領域開始アドレス */
#define MBK_DATA_BASE           BFL_BASE

/** @brief Flash 消去単位 (bytes) */
#define MBK_BLOCK_SIZE          4096U

/* ============================================================================
 * スロット管理 (SML) 設定
 * ========================================================================== */

/** @brief Slot 0 開始アドレス */
#define MBK_SLOT0_BASE          SLOT0_BASE

/** @brief Slot 1 開始アドレス */
#define MBK_SLOT1_BASE          SLOT1_BASE

/** @brief 各スロットサイズ (bytes) */
#define MBK_SLOT_SIZE           SLOT0_SIZE

/** @brief ソフトウェアヘッダサイズ (bytes) */
#define MBK_HEADER_SIZE         8U

/* ============================================================================
 * デバッグ/ログ設定
 * ========================================================================== */

/** @brief ログ出力を有効化 */
#define MBK_ENABLE_LOG

/** @brief ログマクロ定義（外部 uart_puts() に委譲） */
#ifdef MBK_ENABLE_LOG
extern void uart_printf(const char* fmt, ...);
#  define MBK_LOG(...)  uart_printf(__VA_ARGS__)
#else
#  define MBK_LOG(msg)  ((void)0)
#endif

#endif /* MEBUKI_CONFIG_H */
