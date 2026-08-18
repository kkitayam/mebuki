/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include "mebuki_svl.h"

#include "bearssl_ec.h"
#include "bearssl_hash.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

#if defined(MBK_SVL_RSIP_AVAILABLE) && MBK_SVL_RSIP_AVAILABLE
#include "rsip_ecdsa_p256_sha256.h"
#define MBK_SVL_HAS_RSIP 1
#else
#define MBK_SVL_HAS_RSIP 0
#endif

#ifndef MBK_SVL_ACCEL_MODE
#define MBK_SVL_ACCEL_MODE 1
#endif

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

static bool g_use_rsip;

static const uint8_t* get_public_key(uint8_t key_generation)
{
    if (key_generation >= MBK_SVL_NUM_KEY_GENERATIONS) {
        return NULL;
    }
    return public_keys[key_generation];
}

int mbk_svl_init(void)
{
#if MBK_SVL_ACCEL_MODE == 0
    g_use_rsip = false;
    return MBK_SVL_SUCCESS;
#elif MBK_SVL_ACCEL_MODE == 2
#if MBK_SVL_HAS_RSIP
    if (rsip_hw_init() == 0) {
        g_use_rsip = true;
        return MBK_SVL_SUCCESS;
    }
    return MBK_SVL_ERROR_HARDWARE_FAILURE;
#else
    return MBK_SVL_ERROR_HARDWARE_FAILURE;
#endif
#else
#if MBK_SVL_HAS_RSIP
    if (rsip_hw_init() == 0) {
        g_use_rsip = true;
        return MBK_SVL_SUCCESS;
    }
#endif
    g_use_rsip = false;
    return MBK_SVL_SUCCESS;
#endif
}

int mbk_svl_verify_signature(const void* data, size_t data_len,
                                              const void* signature,
                                              uint8_t key_generation)
{
    const uint8_t* public_key;
    uint8_t hash[MBK_SVL_HASH_SIZE];
    int err;

    if (data == NULL || signature == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

    public_key = get_public_key(key_generation);
    if (public_key == NULL) {
        return MBK_SVL_ERROR_INVALID_KEY_GEN;
    }

    err = mbk_svl_compute_hash(data, data_len, hash);
    if (err != MBK_SVL_SUCCESS) {
        return err;
    }

#if MBK_SVL_HAS_RSIP
    if (g_use_rsip) {
        err = rsip_ecdsa_p256_verify_hash(hash, signature, public_key);
        if (err == 0) {
            return MBK_SVL_SUCCESS;
        }
        if (err == -1) {
            return MBK_SVL_ERROR_SIGNATURE_INVALID;
        }
        return MBK_SVL_ERROR_HARDWARE_FAILURE;
    }
#endif

    br_ec_public_key pk;
    pk.curve = BR_EC_secp256r1;
    pk.q = (unsigned char*)(uintptr_t)public_key;
    pk.qlen = MBK_SVL_PUBKEY_SIZE;

    const uint32_t ok = br_ecdsa_i31_vrfy_raw(&br_ec_p256_m31,
                                              hash,
                                              MBK_SVL_HASH_SIZE,
                                              &pk,
                                              signature,
                                              MBK_SVL_SIGNATURE_SIZE);
    if (ok != 1U) {
        return MBK_SVL_ERROR_SIGNATURE_INVALID;
    }

    return MBK_SVL_SUCCESS;
}

int mbk_svl_compute_hash(const void* data, size_t data_len,
                                          uint8_t digest_out[MBK_SVL_HASH_SIZE])
{
    if (data == NULL || digest_out == NULL) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }

#if MBK_SVL_HAS_RSIP
    if (g_use_rsip) {
        if (rsip_sha256_compute(data, data_len, digest_out) != 0) {
            return MBK_SVL_ERROR_HARDWARE_FAILURE;
        }
        return MBK_SVL_SUCCESS;
    }
#endif

    br_sha256_context sha_ctx;
    br_sha256_init(&sha_ctx);
    br_sha256_update(&sha_ctx, data, data_len);
    br_sha256_out(&sha_ctx, digest_out);

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
