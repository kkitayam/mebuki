/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include "mebuki_svl.h"

#include "blake2.h"
#include "fndsa.h"

#include <assert.h>
#include <stddef.h>

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

enum mbk_svl_result mbk_svl_init(void)
{
    return MBK_SVL_SUCCESS;
}

enum mbk_svl_result mbk_svl_verify_signature(const void* data, size_t data_len,
                                              const void* signature,
                                              uint8_t key_generation)
{
    const uint8_t* data_bytes = (const uint8_t*)data;
    const uint8_t* signature_bytes = (const uint8_t*)signature;
    const uint8_t* public_key;
    int ok;

    if (data == NULL || signature == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

    public_key = get_public_key(key_generation);
    if (public_key == NULL) {
        return MBK_SVL_ERROR_INVALID_KEY_GEN;
    }

    ok = fndsa_verify(
        signature_bytes,
        MBK_SVL_SIGNATURE_SIZE,
        public_key,
        MBK_SVL_PUBKEY_SIZE,
        NULL,
        0,
        FNDSA_HASH_ID_RAW,
        data_bytes,
        data_len);

    if (ok != 1) {
        return MBK_SVL_ERROR_SIGNATURE_INVALID;
    }

    return MBK_SVL_SUCCESS;
}

enum mbk_svl_result mbk_svl_compute_hash(const void* data, size_t data_len,
                                          uint8_t digest_out[MBK_SVL_HASH_SIZE])
{
    if (data == NULL || digest_out == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

    const uint8_t* data_bytes = (const uint8_t*)data;
    if (blake2s(digest_out, MBK_SVL_HASH_SIZE, data_bytes, data_len, NULL, 0) != 0) {
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
