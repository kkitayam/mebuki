/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#ifndef MEBUKI_SVL_H
#define MEBUKI_SVL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <mebuki_svl_limits.h>

#ifndef MBK_SVL_NUM_KEY_GENERATIONS
# define MBK_SVL_NUM_KEY_GENERATIONS     8U
#endif

#define MBK_SVL_SUCCESS                  0
#define MBK_SVL_ERROR_NULL_POINTER      -1
#define MBK_SVL_ERROR_INVALID_KEY_GEN   -2
#define MBK_SVL_ERROR_SIGNATURE_INVALID -3
#define MBK_SVL_ERROR_HARDWARE_FAILURE  -4

#ifdef __cplusplus
extern "C" {
#endif

int mbk_svl_init(void);
int mbk_svl_verify_signature(const void* data, size_t data_len,
                                              const void* signature,
                                              uint8_t key_generation);
int mbk_svl_compute_hash(const void* data, size_t data_len,
                                          uint8_t digest_out[MBK_SVL_HASH_SIZE]);
bool mbk_svl_compare_hash(const uint8_t digest1[MBK_SVL_HASH_SIZE],
                          const uint8_t digest2[MBK_SVL_HASH_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* MEBUKI_SVL_H */
