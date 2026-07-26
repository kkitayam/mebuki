/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef MOCK_SVL_H
#define MOCK_SVL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SVL configuration (matching mebuki_svl.h / libmebuki_config.h) */
#define MOCK_SVL_PUBKEY_SIZE          32U
#define MOCK_SVL_SIGNATURE_SIZE       64U
#define MOCK_SVL_HASH_SIZE            32U
#define MOCK_SVL_NUM_KEY_GENERATIONS  8U

enum mbk_svl_result {
    MBK_SVL_SUCCESS = 0,
    MBK_SVL_ERROR_NULL_POINTER = -1,
    MBK_SVL_ERROR_INVALID_KEY_GEN = -2,
    MBK_SVL_ERROR_SIGNATURE_INVALID = -3,
    MBK_SVL_ERROR_HARDWARE_FAILURE = -4,
};

enum mbk_svl_result mbk_svl_init(void);
enum mbk_svl_result mbk_svl_verify_signature(const uint8_t* data, size_t data_len,
                                       const uint8_t* signature,
                                       uint8_t key_generation);
enum mbk_svl_result mbk_svl_compute_hash(const uint8_t* data, size_t data_len,
                                   uint8_t digest_out[MOCK_SVL_HASH_SIZE]);
bool mbk_svl_compare_hash(const uint8_t digest1[MOCK_SVL_HASH_SIZE],
                          const uint8_t digest2[MOCK_SVL_HASH_SIZE]);

void mock_svl_set_verification_result(bool result);
void mock_svl_inject_init_error(bool enable);
uint32_t mock_svl_get_verify_count(void);
uint32_t mock_svl_get_hash_count(void);
void mock_svl_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_SVL_H */
