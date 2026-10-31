/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define FLASH_BASE          0x08000000U
#define FLASH_SIZE          0x00200000U
#define FLASH_ERASE_SIZE    0x00020000U
#define BFL_DATA0_BASE      0x08020000U
#define BFL_DATA1_BASE      0x08120000U
#define SLOT0_BASE          0x08040000U
#define SLOT1_BASE          0x08140000U
#define SLOT0_SIZE          0x000C0000U
#define SLOT1_SIZE          SLOT0_SIZE

#define HAS_BANK_SWAP
#define BANK_SWAP_HEADER "flash.h"
#define BANK_SWAP() hal_flash_swap_bank()

#endif
