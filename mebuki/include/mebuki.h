/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef MEBUKI_H
#define MEBUKI_H

/* Include a project-specific configuration file if provided. */
#if defined(MEBUKI_CONFIG_FILE)
# include MEBUKI_CONFIG_FILE
#else
# include <mebuki_config.h>
#endif

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Public data types
 * ========================================================================= */

/*
 * Library context.
 *
 * Allocate one context for each independent boot operation.
 * The library initializes all internal state; the context does not need to
 * be zero-initialized.
 *
 * Do not access the internal fields directly.
 */
struct mbk_context {
    _Alignas(4) uint8_t _internal[MBK_HASH_SIZE + sizeof(uint32_t) * 4];
};

/*
 * Result codes returned by public APIs.
 */
enum mbk_result {
    MBK_SUCCESS = 0,
    MBK_ERROR_INIT_FAILED = -1,
    MBK_ERROR_NO_BOOTABLE_SLOT = -2,
    MBK_ERROR_INVALID_PARAM = -3,
    MBK_ERROR_HASH_COMPUTATION_FAILED = -4,
};

/*
 * Firmware image header.
 */
struct mbk_header {
    uint16_t security_version;  /* 0..MBK_SECURITY_VERSION_MAX */
    uint8_t  key_generation;    /* 0..MBK_NUM_KEY_GENERATIONS-1 */
    uint8_t  invalidation_flag; /* 0xFF = valid, 0x00 = invalid */
    uint32_t software_size;     /* Size of the software image in bytes (excluding header and signature) */
};

/*
 * Boot information returned by mbk_find_bootable_slot().
 */
struct mbk_boot_info {
    int slot_id;                      /* 0 for Slot 0, 1 for Slot 1 */
    const struct mbk_header* header;  /* Pointer to the firmware header in the selected slot */
    uint32_t entry_point;             /* Entry point address of the selected firmware image */
};

/* =========================================================================
 * Public API
 * ========================================================================= */

/*
 * Initialize the library.
 *
 * Call this once before calling any other mebuki API.
 *
 * Returns:
 *   MBK_SUCCESS
 *   MBK_ERROR_INIT_FAILED
 *   MBK_ERROR_INVALID_PARAM
 */
int mbk_init(struct mbk_context* ctx);

/*
 * Find a bootable firmware image.
 *
 * On success, boot_info is populated with the selected image.
 *
 * Returns:
 *   MBK_SUCCESS
 *   MBK_ERROR_NO_BOOTABLE_SLOT
 *   MBK_ERROR_INVALID_PARAM
 */
int mbk_find_bootable_slot(struct mbk_context* ctx,
                           struct mbk_boot_info* boot_info);

#ifdef __cplusplus
}
#endif

#endif /* MEBUKI_H */
