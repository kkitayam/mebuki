#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define FLASH_BASE          0xFFE00000U
#define FLASH_SIZE          0x00200000U

#if defined(MEBUKI_RX65N_DATA_IN_CODE_FLASH)
#define BFL_BASE            0xFFFF0000U
#define BFL_BASE_SWAP       0xFFEF0000U
#else
#define BFL_BASE            0x00100000U
#endif
#define BFL_SIZE            0x00000100U

#define PROGRESS_BASE       0x00100400U
#define PROGRESS_SIZE       0x00000400U

#define SLOT0_BASE          0xFFE00000U
#define SLOT0_SIZE          0x00020000U

#define SLOT1_BASE          0xFFF00000U
#define SLOT1_SIZE          0x00020000U

#endif
