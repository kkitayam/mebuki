#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

/**
 * @file mebuki_config.h
 * @brief AE-LPC11U35-MB 向け libmebuki 設定
 */

#include "target_config.h"

#define MBK_PUBKEY_SIZE         32U
#define MBK_SIGNATURE_SIZE      64U
#define MBK_HASH_SIZE           32U
#define MBK_NUM_KEY_GENERATIONS 8U

#define MBK_DATA_SIZE           72U
#define MBK_FLASH_PAGE_SIZE     FLASH_PAGE_SIZE
#define MBK_DATA_BASE           BFL_BASE
#define MBK_BLOCK_SIZE          FLASH_SECTOR_SIZE
#define MBK_MAGIC               0x4D42454BU

#define MBK_SLOT0_BASE          SLOT0_BASE
#define MBK_SLOT1_BASE          SLOT1_BASE
#define MBK_SLOT_SIZE           SLOT0_SIZE
#define MBK_HEADER_SIZE         8U

#define MBK_UNINITIALIZED_SECURITY_VERSION  0xFFFFU
#define MBK_UNINITIALIZED_KEY_GEN           0xFFU
#define MBK_SECURITY_VERSION_MAX            0xFFFEU
#define MBK_SECURITY_VERSION_UNINITIALIZED  0xFFFFU
#define MBK_KEY_GENERATION_MAX              0xFEU
#define MBK_KEY_GENERATION_UNINITIALIZED    0xFFU

#define MBK_INVALIDATION_FLAG_VALID         0xFFU
#define MBK_INVALIDATION_FLAG_INVALID       0x00U

#define MBK_ENABLE_LOG

#ifdef MBK_ENABLE_LOG
extern void uart_puts(const char* str);
#  define MBK_LOG(msg)  uart_puts(msg)
#else
#  define MBK_LOG(msg)  ((void)0)
#endif

#endif /* MEBUKI_CONFIG_H */