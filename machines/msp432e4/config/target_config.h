#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define MBK_FLASH_BASE      0x00000000U
#define MBK_FLASH_SIZE      0x00100000U
#define MBK_SRAM_BASE       0x20000000U
#define MBK_SRAM_SIZE       0x00040000U
#define FLASH_ERASE_SIZE    0x00004000U

#define BFL_DATA0_BASE      0x00008000U
#define BFL_DATA1_BASE      0x0000C000U
#define BFL_SIZE            FLASH_ERASE_SIZE

#define PROGRESS_BASE       0x00010000U
#define PROGRESS_SIZE       FLASH_ERASE_SIZE

#define SLOT0_BASE          0x00020000U
#define SLOT0_SIZE          0x00040000U
#define SLOT1_BASE          0x000A0000U
#define SLOT1_SIZE          0x00040000U

#endif
