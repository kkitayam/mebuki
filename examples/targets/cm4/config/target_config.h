#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/**
 * @file target_config.h
 * @brief Renode Cortex-M4 ターゲット用メモリマップ定義
 *
 * platform.repl の定義に基づく：
 * - CPU: Cortex-M4F
 * - Flash: 1MB @ 0x00000000
 * - SRAM: 128KB @ 0x20000000
 * - UART0: PL011 @ 0x4001C000
 */

/* ============================================================================
 * Flash メモリレイアウト（0x00000000 - 0x00100000, 1MB）
 * ========================================================================== */

#define FLASH_BASE          0x00000000U
#define FLASH_SIZE          0x00100000U    /* 1MB */

/* Boot Software セクション */
#define BOOT_BASE           0x00000000U
#define BOOT_SIZE           0x00010000U    /* 64KB */

/* Boot Flash Layer (BFL) データ領域 */
#define BFL_BASE            0x00010000U
#define BFL_SIZE            0x00004000U    /* 16KB */

/* 予約領域 */
#define RESERVED_BASE       0x00014000U
#define RESERVED_SIZE       0x0000C000U    /* 48KB */

/* Slot 0 (User Software) */
#define SLOT0_BASE          0x00020000U
#define SLOT0_SIZE          0x00020000U    /* 128KB */

/* Slot 1 (User Software) */
#define SLOT1_BASE          0x00040000U
#define SLOT1_SIZE          0x00020000U    /* 128KB */

/* ユーザデータ領域（将来用） */
#define USER_DATA_BASE      0x00060000U
#define USER_DATA_SIZE      0x000A0000U    /* 640KB */

/* ============================================================================
 * SRAM メモリレイアウト（0x20000000 - 0x20020000, 128KB）
 * ========================================================================== */

#define SRAM_BASE           0x20000000U
#define SRAM_SIZE           0x00020000U    /* 128KB */

/* ============================================================================
 * ペリフェラル
 * ========================================================================== */

/* UART0 (PL011) */
#define UART0_BASE          0x4001C000U
#define UART0_BAUDRATE      115200U

/* NVIC */
#define NVIC_BASE           0xE000E000U

/* ============================================================================
 * パラメータ定義
 * ========================================================================== */

/* Flash エラーハンドリング用 */
#define MBK_BLOCK_SIZE      4096U          /* Flash 消去単位 */

#endif /* TARGET_CONFIG_H */
