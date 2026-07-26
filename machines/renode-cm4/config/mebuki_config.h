/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

/* Use the memory map defined in `target_config.h` to configure the operational parameters */
#include "target_config.h"

/* ============================================================================
 * Cryptography
 * ========================================================================== */

/* Public key size in bytes. */
#define MBK_PUBKEY_SIZE         32U

/* Signature size in bytes. */
#define MBK_SIGNATURE_SIZE      64U

/* Hash digest size in bytes. */
#define MBK_HASH_SIZE           32U

/* Number of key generations (default: 8, maximum: 255). */
#define MBK_NUM_KEY_GENERATIONS 8U

/* ============================================================================
 * Persistent storage
 * ========================================================================== */

/* Flash programming unit in bytes.
 *
 * Requirements:
 *   - Must be a multiple of 16.
 *   - Used as the erase and block copy unit.
 */
#define MBK_FLASH_PAGE_SIZE     256U

/* Flash erase block size in bytes.
 *
 * Requirements:
 *   - Must be a multiple of 16.
 *   - Used as the erase and block copy unit.
 */
#define MBK_BLOCK_SIZE          4096U

/* Base address of the persistent data area. */
#define MBK_DATA_BASE           BFL_BASE

/* ============================================================================
 * Firmware slots
 * ========================================================================== */

/*
 * Base address of Slot 0.
 *
 * Requirements:
 *   - Must be aligned to MBK_BLOCK_SIZE.
 *   - The implementation performs direct header loads and sector operations.
 */
#define MBK_SLOT0_BASE          SLOT0_BASE

/* Base address of Slot 1.
 *
 * Requirements:
 *   - Must be aligned to MBK_BLOCK_SIZE.
 *   - The implementation performs direct header loads and sector operations.
 */
#define MBK_SLOT1_BASE          SLOT1_BASE

/* Size of each firmware slot in bytes. */
#define MBK_SLOT_SIZE           SLOT0_SIZE

/* Size of the software header in bytes. */
#define MBK_HEADER_SIZE         8U

/* ============================================================================
 * Logging
 * ========================================================================== */

/* Enable logging */
#define MBK_ENABLE_LOG

#ifdef MBK_ENABLE_LOG
/* Log macro definition (external uart_puts() delegation) */
extern void uart_printf(const char* fmt, ...);
#  define MBK_LOG(...)  uart_printf(__VA_ARGS__)
#else
#  define MBK_LOG(msg)  ((void)0)
#endif

#endif /* MEBUKI_CONFIG_H */
