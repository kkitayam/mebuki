/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef MOCK_HAL_FLASH_H
#define MOCK_HAL_FLASH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "mebuki_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mock Flash configuration */
#define MOCK_FLASH_SIZE         (1024U * 1024U)
#define MOCK_FLASH_SECTOR_SIZE  4096U
#define MOCK_FLASH_PAGE_SIZE    256U

int hal_flash_init(void);
int hal_flash_write(uintptr_t address, const void* data, size_t size);
int hal_flash_erase_sector(uintptr_t address);
int hal_flash_erase_all(void);

/* Mock control functions for testing */

/* Reset mock Flash memory (all 0xFF) */
bool mock_flash_reset(void);

/**
 * @brief Set the value of a specific address in the mock Flash memory
 * @param[in] address The address in the mock Flash memory
 * @param[in] data The data buffer
 * @param[in] size The size of the data
 */
void mock_flash_set_memory(uintptr_t address, const void* data, size_t size);

/**
 * @brief Inject error (next operation will fail)
 * @param[in] enable true to enable error injection
 */
void mock_flash_inject_error(bool enable);

/**
 * @brief Get the write count
 * @return The number of writes
 */
uint32_t mock_flash_get_write_count(void);

/**
 * @brief Get the erase count
 * @return The number of erases
 */
uint32_t mock_flash_get_erase_count(void);

/**
 * @brief Reset the statistics
 */
void mock_flash_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_HAL_FLASH_H */
