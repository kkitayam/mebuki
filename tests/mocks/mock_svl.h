/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef MOCK_SVL_H
#define MOCK_SVL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void mock_svl_set_verification_result(bool result);
void mock_svl_inject_init_error(bool enable);
uint32_t mock_svl_get_verify_count(void);
uint32_t mock_svl_get_hash_count(void);
void mock_svl_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_SVL_H */
