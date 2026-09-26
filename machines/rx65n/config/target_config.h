#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define FLASH_BASE          0xFFE00000U
#define FLASH_SIZE          0x00200000U

#if defined(MEBUKI_RX65N_DATA_IN_CODE_FLASH)
#define BFL_DATA0_BASE      0xFFEF0000U
#define BFL_DATA1_BASE      0xFFFF0000U
#define BFL_SIZE            128U
#else
#define MBK_FLASH_BLANK_VALUE_UNDEFINED 1
#define BFL_DATA0_BASE      0x00100000U
#define BFL_DATA1_BASE      0x00100040U
#define BFL_SIZE            64U
#endif

#define PROGRESS_BASE       0x00100400U
#define PROGRESS_SIZE       0x00000400U

#define SLOT0_BASE          0xFFE00000U
#define SLOT0_SIZE          0x00020000U

#define SLOT1_BASE          0xFFF00000U
#define SLOT1_SIZE          0x00020000U

#define HAS_BANK_SWAP
#define BANK_SWAP_HEADER    "flash_type4.h"
#define BANK_SWAP()         flash_type4_swap_bank()

#endif
