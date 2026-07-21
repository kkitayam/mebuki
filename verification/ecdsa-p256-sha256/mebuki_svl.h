/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#ifndef MEBUKI_SVL_H
#define MEBUKI_SVL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* ECDSA P-256 sizes */
#define MBK_SVL_PUBKEY_SIZE      65   /* uncompressed P-256 public key: 0x04 || 32-byte x || 32-byte y */
#define MBK_SVL_SIGNATURE_SIZE   64   /* raw ECDSA P-256 signature: 32-byte r || 32-byte s */
#define MBK_SVL_HASH_SIZE        32   /* SHA-256 digest size */

#ifndef MBK_SVL_NUM_KEY_GENERATIONS
#define MBK_SVL_NUM_KEY_GENERATIONS  8
#endif

enum mbk_svl_result {
    MBK_SVL_SUCCESS = 0,
    MBK_SVL_ERROR_NULL_POINTER = -1,
    MBK_SVL_ERROR_INVALID_KEY_GEN = -2,
    MBK_SVL_ERROR_SIGNATURE_INVALID = -3,
    MBK_SVL_ERROR_HARDWARE_FAILURE = -4,
};

#ifdef __cplusplus
extern "C" {
#endif

enum mbk_svl_result mbk_svl_init(void);

enum mbk_svl_result mbk_svl_verify_signature(const void* data, size_t data_len,
                                              const void* signature,
                                              uint8_t key_generation);

enum mbk_svl_result mbk_svl_compute_hash(const void* data, size_t data_len,
                                          uint8_t digest_out[MBK_SVL_HASH_SIZE]);

bool mbk_svl_compare_hash(const uint8_t digest1[MBK_SVL_HASH_SIZE],
                          const uint8_t digest2[MBK_SVL_HASH_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* MEBUKI_SVL_H */
