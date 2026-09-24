#ifndef MEBUKI_CONFIG_H
#define MEBUKI_CONFIG_H

#include "target_config.h"

#define MBK_PUBKEY_SIZE         65U
#define MBK_SIGNATURE_SIZE      64U
#define MBK_HASH_SIZE           32U
#define MBK_NUM_KEY_GENERATIONS 8U
#define MBK_FLASH_PAGE_SIZE     128U
#define MBK_BLOCK_SIZE_BFL      BFL_SIZE
#define MBK_BLOCK_SIZE_SLOT     FLASH_ERASE_SIZE
#define MBK_BLOCK_SIZE_PROGRESS FLASH_ERASE_SIZE
#define MBK_DATA0_BASE          BFL_DATA0_BASE
#define MBK_DATA1_BASE          BFL_DATA1_BASE
#define TANEUE_PROGRESS_BASE    PROGRESS_BASE
#define TANEUE_PROGRESS_SIZE    PROGRESS_SIZE
#define MBK_SLOT0_BASE          SLOT0_BASE
#define MBK_SLOT1_BASE          SLOT1_BASE
#define MBK_SLOT_SIZE           SLOT0_SIZE
#define MBK_HEADER_SIZE         8U

#ifdef MBK_ENABLE_LOG
extern void uart_puts(const char *);
#define MBK_LOG(message) uart_puts(message)
#else
#define MBK_LOG(message) ((void)0)
#endif

#endif
