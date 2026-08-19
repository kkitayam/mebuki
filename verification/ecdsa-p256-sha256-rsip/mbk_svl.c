/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include "mebuki_svl.h"

#include "rsip_ecdsa_p256_sha256.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#ifndef MBK_SVL_PUBLIC_KEYS_HEADER
#define MBK_SVL_PUBLIC_KEYS_HEADER "public_keys.h"
#endif

#include MBK_SVL_PUBLIC_KEYS_HEADER

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
static_assert(sizeof(public_keys[0]) == MBK_SVL_PUBKEY_SIZE,
              "public_keys row size must equal MBK_SVL_PUBKEY_SIZE");
static_assert(sizeof(public_keys) / sizeof(public_keys[0]) == MBK_SVL_NUM_KEY_GENERATIONS,
              "public_keys row count must equal MBK_SVL_NUM_KEY_GENERATIONS");
#endif

static const uint8_t* get_public_key(uint8_t key_generation)
{
    if (key_generation >= MBK_SVL_NUM_KEY_GENERATIONS) {
        return NULL;
    }
    return public_keys[key_generation];
}

int mbk_svl_init(void)
{
    if (rsip_hw_init() != 0) {
        return MBK_SVL_ERROR_HARDWARE_FAILURE;
    }
    return MBK_SVL_SUCCESS;
}

int mbk_svl_verify_signature(const void* data, size_t data_len,
                             const void* signature,
                             uint8_t key_generation)
{
    uint8_t hash[MBK_SVL_HASH_SIZE];
    int err;

    if (data == NULL || signature == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

    const uint8_t* public_key = get_public_key(key_generation);
    if (public_key == NULL) {
        return MBK_SVL_ERROR_INVALID_KEY_GEN;
    }

    err = mbk_svl_compute_hash(data, data_len, hash);
    if (err != MBK_SVL_SUCCESS) {
        return err;
    }

    err = rsip_ecdsa_p256_verify_hash(hash, signature, public_key);
    if (err == 0) {
        return MBK_SVL_SUCCESS;
    }
    if (err == -1) {
        return MBK_SVL_ERROR_SIGNATURE_INVALID;
    }
    return MBK_SVL_ERROR_HARDWARE_FAILURE;
}

int mbk_svl_compute_hash(const void* data, size_t data_len,
                         uint8_t digest_out[MBK_SVL_HASH_SIZE])
{
    if (data == NULL || digest_out == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

    if (rsip_sha256_compute(data, data_len, digest_out) != 0) {
        return MBK_SVL_ERROR_HARDWARE_FAILURE;
    }
    return MBK_SVL_SUCCESS;
}

bool mbk_svl_compare_hash(const uint8_t digest1[MBK_SVL_HASH_SIZE],
                          const uint8_t digest2[MBK_SVL_HASH_SIZE])
{
    uint8_t diff = 0;
    size_t i;

    if (digest1 == NULL || digest2 == NULL) {
        return false;
    }

    for (i = 0; i < MBK_SVL_HASH_SIZE; i++) {
        diff |= (uint8_t)(digest1[i] ^ digest2[i]);
    }

    return diff == 0;
}
