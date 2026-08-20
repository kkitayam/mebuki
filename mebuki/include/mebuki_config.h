/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
/*
 * Default configuration for mebuki.
 *
 * To override these defaults, either:
 *   - Provide your own configuration file and compile with:
 *       -DMEBUKI_CONFIG_FILE=\"my_config.h\"
 *   - Or place your own mebuki_config.h earlier in the include search path.
 */
#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

/* =========================================================================
 * Cryptography
 * ========================================================================= */

/* Public key size in bytes. */
#define MBK_PUBKEY_SIZE             32U

/* Signature size in bytes. */
#define MBK_SIGNATURE_SIZE          7856U

/* Hash digest size in bytes. */
#define MBK_HASH_SIZE               32U

/* Number of key generations (default: 8, maximum: 255). */
#define MBK_NUM_KEY_GENERATIONS     8U


/* =========================================================================
 * Persistent storage
 * ========================================================================= */

/* Flash programming unit in bytes. */
#ifndef MBK_FLASH_PAGE_SIZE
#  define MBK_FLASH_PAGE_SIZE       256U
#endif

/* Flash erase block size for BFL sectors in bytes. */
#define MBK_BLOCK_SIZE_BFL          4096U

/* Flash erase block size for slot sectors in bytes. */
#define MBK_BLOCK_SIZE_SLOT         4096U

/* Flash erase block size for the Taneue progress area in bytes. */
#define MBK_BLOCK_SIZE_PROGRESS     4096U

/*
 * Base address of BFL sector 0.
 *
 * Requirements:
 *   - Must be aligned to MBK_BLOCK_SIZE_BFL.
 *   - The implementation performs direct loads from aligned structures.
 */
#define MBK_DATA0_BASE              0x08000000U

/*
 * Base address of BFL sector 1.
 *
 * Requirements are identical to MBK_DATA0_BASE.
 */
#define MBK_DATA1_BASE              0x08001000U


/* =========================================================================
 * Firmware slots
 * ========================================================================= */

/*
 * Base address of Slot 0.
 *
 * Requirements:
 *   - Must be aligned to MBK_BLOCK_SIZE_SLOT.
 *   - The implementation performs direct header loads and sector operations.
 */
#define MBK_SLOT0_BASE              0x08010000U

/*
 * Base address of Slot 1.
 *
 * Requirements are identical to MBK_SLOT0_BASE.
 */
#define MBK_SLOT1_BASE              0x08020000U

/*
 * Slot size in bytes.
 *
 * Must be a multiple of MBK_BLOCK_SIZE_SLOT.
 */
#define MBK_SLOT_SIZE               0x00010000U  /* 64 KiB */

/* Firmware header size in bytes. */
#define MBK_HEADER_SIZE             8U


/* =========================================================================
 * Taneue progress storage
 * ========================================================================= */

/*
 * Base address of the Taneue progress area.
 *
 * Requirements:
 *   - Must be aligned to MBK_BLOCK_SIZE_PROGRESS.
 *   - The implementation directly loads uint16_t progress fields.
 */
#define TANEUE_PROGRESS_BASE        \
    (((MBK_DATA0_BASE > MBK_DATA1_BASE) ? MBK_DATA0_BASE : MBK_DATA1_BASE) + MBK_BLOCK_SIZE_BFL)

/*
 * Size of the Taneue progress area in bytes, including the header.
 *
 * Requirements:
 *   - Minimum size: 3N + 2 bytes.
 *   - Must be a multiple of MBK_BLOCK_SIZE_PROGRESS.
 */
#define TANEUE_PROGRESS_SIZE        MBK_BLOCK_SIZE_PROGRESS


/* =========================================================================
 * Logging
 * ========================================================================= */

#ifdef MBK_ENABLE_LOG
#  define MBK_LOG(msg)              uart_puts(msg)
#else
#  define MBK_LOG(msg)              ((void)0)
#endif

#endif /* MEBUKI_CONFIG_H */
