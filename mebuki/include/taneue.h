/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef TANEUE_H
#define TANEUE_H

/* Include a project-specific configuration file if provided. */
#if defined(MEBUKI_CONFIG_FILE)
# include MEBUKI_CONFIG_FILE
#else
# include <mebuki_config.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Public result codes
 * ========================================================================= */

/*
 * Result codes returned by the taneue API.
 */
enum taneue_result {
    TANEUE_SUCCESS = 0,
    TANEUE_ERROR_FLASH = -1,
    TANEUE_ERROR_INVALID_STATE = -2,
    TANEUE_ERROR_PRECONDITION = -3,
};

/* =========================================================================
 * Public API
 * ========================================================================= */

/*
 * Schedule a slot swap.
 *
 * Call this after downloading a new firmware image and before rebooting.
 *
 * The library prepares the metadata required to perform the swap. If a swap
 * is already scheduled, the existing schedule is replaced.
 *
 * Returns:
 *   TANEUE_SUCCESS
 *   TANEUE_ERROR_FLASH
 *   TANEUE_ERROR_PRECONDITION
 */
int taneue_schedule_swap(void);

/*
 * Perform a scheduled slot swap.
 *
 * Call this during boot before selecting the firmware image to boot.
 *
 * If no swap is scheduled, this function returns successfully without making
 * any changes. If a previous swap was interrupted by a reset or power loss,
 * the operation is resumed automatically.
 *
 * Returns:
 *   TANEUE_SUCCESS
 *   TANEUE_ERROR_FLASH
 *   TANEUE_ERROR_INVALID_STATE
 */
int taneue_swap_if_scheduled(void);

#ifdef __cplusplus
}
#endif

#endif /* TANEUE_H */
