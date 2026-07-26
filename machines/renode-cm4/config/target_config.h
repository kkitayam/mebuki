/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/**
  * @brief Memory map definition for the Renode Cortex-M4 target
 *
 * Based on the definitions in platform.repl:
 * - CPU: Cortex-M4F
 * - Flash: 1MB @ 0x00000000
 * - SRAM: 128KB @ 0x20000000
 * - UART0: PL011 @ 0x4001C000
 */

/* ============================================================================
 * Flash memory layout (0x00000000 - 0x00100000, 1MB)
 * ========================================================================== */

#define FLASH_BASE          0x00000000U
#define FLASH_SIZE          0x00100000U    /* 1MB */

/* Boot Software section */
#define BOOT_BASE           0x00000000U
#define BOOT_SIZE           0x00010000U    /* 64KB */

/* Boot Flash Layer (BFL) data section */
#define BFL_BASE            0x00010000U
#define BFL_SIZE            0x00004000U    /* 16KB */

/* Reserved section */
#define RESERVED_BASE       0x00014000U
#define RESERVED_SIZE       0x0000C000U    /* 48KB */

/* Slot 0 (User Software) */
#define SLOT0_BASE          0x00020000U
#define SLOT0_SIZE          0x00020000U    /* 128KB */

/* Slot 1 (User Software) */
#define SLOT1_BASE          0x00040000U
#define SLOT1_SIZE          0x00020000U    /* 128KB */

/* User data section (for future use) */
#define USER_DATA_BASE      0x00060000U
#define USER_DATA_SIZE      0x000A0000U    /* 640KB */

/* ============================================================================
 * SRAM memory layout (0x20000000 - 0x20020000, 128KB)
 * ========================================================================== */

#define SRAM_BASE           0x20000000U
#define SRAM_SIZE           0x00020000U    /* 128KB */

/* ============================================================================
 * Peripherals
 * ========================================================================== */

/* UART0 (PL011) */
#define UART0_BASE          0x4001C000U
#define UART0_BAUDRATE      115200U

/* NVIC */
#define NVIC_BASE           0xE000E000U

/* ============================================================================
 * Parameters
 * ========================================================================== */

/* Flash error handling */
#define MBK_BLOCK_SIZE      4096U          /* Flash erase unit */

#endif /* TARGET_CONFIG_H */
