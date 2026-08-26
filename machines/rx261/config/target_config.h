/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef TARGET_CONFIG_H
#define TARGET_CONFIG_H

#define FLASH_BASE          0xFFF80000U
#define FLASH_SIZE          0x00080000U

#define BFL_BASE            0x00100000U
#define BFL_SIZE            0x00000100U

#define PROGRESS_BASE       0x00100400U
#define PROGRESS_SIZE       0x00000400U

#define SLOT0_BASE          0xFFF80000U
#define SLOT0_SIZE          0x00020000U

#define SLOT1_BASE          0xFFFA0000U
#define SLOT1_SIZE          0x00020000U

#endif /* TARGET_CONFIG_H */
