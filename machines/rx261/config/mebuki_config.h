/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

#include "target_config.h"

#define MBK_PUBKEY_SIZE         65U
#define MBK_SIGNATURE_SIZE      64U
#define MBK_HASH_SIZE           32U
#define MBK_NUM_KEY_GENERATIONS 8U

#define MBK_FLASH_PAGE_SIZE     8U
#define MBK_BLOCK_SIZE_BFL      2048U
#define MBK_BLOCK_SIZE_SLOT     2048U
#define MBK_BLOCK_SIZE_PROGRESS 2048U

#define MBK_DATA0_BASE          BFL_BASE
#define MBK_DATA1_BASE          (BFL_BASE + MBK_BLOCK_SIZE_BFL)

#define TANEUE_PROGRESS_BASE    PROGRESS_BASE
#define TANEUE_PROGRESS_SIZE    PROGRESS_SIZE

#define MBK_SLOT0_BASE          SLOT0_BASE
#define MBK_SLOT1_BASE          SLOT1_BASE
#define MBK_SLOT_SIZE           SLOT0_SIZE
#define MBK_HEADER_SIZE         8U

#define MBK_ENABLE_LOG

#ifdef MBK_ENABLE_LOG
extern void uart_printf(const char* fmt, ...);
#  define MBK_LOG(...)  uart_printf(__VA_ARGS__)
#else
#  define MBK_LOG(msg)  ((void)0)
#endif

#endif /* MEBUKI_CONFIG_H */
