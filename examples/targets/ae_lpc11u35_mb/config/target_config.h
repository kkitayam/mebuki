#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

/**
 * @file target_config.h
 * @brief AE-LPC11U35-MB ターゲット用メモリマップ定義
 */

#define FLASH_BASE          0x00000000U
#define FLASH_SIZE          0x00010000U    /* 64KB */

#define BOOT_BASE           0x00000000U
#define BOOT_SIZE           0x00009000U    /* 36KB */

#define BFL_BASE            0x00009000U
#define BFL_SIZE            0x00002000U    /* 8KB, 2 x 4KB sectors */

#define SLOT0_BASE          0x0000B000U
#define SLOT0_SIZE          0x00002000U    /* 8KB */

#define SLOT1_BASE          0x0000D000U
#define SLOT1_SIZE          0x00002000U    /* 8KB */

#define SRAM_BASE           0x10000000U
#define SRAM_SIZE           0x00002000U    /* 8KB */

/* Flash erase/program parameters for LPC11U35 */
#define FLASH_SECTOR_SIZE   0x00001000U    /* 4KB */
#define FLASH_PAGE_SIZE     0x00000100U    /* 256 bytes */

#endif /* TARGET_CONFIG_H */