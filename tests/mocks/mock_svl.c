/**
 * @file mock_svl.c
 * @brief SVL (Signature Verification Layer) Mock Implementation
 * 
 * テスト用のSVLモック実装
 */

#include "mock_svl.h"
#include <string.h>
#include <stdio.h>

/* Internal mock state */
static uint8_t mock_public_keys[MOCK_SVL_NUM_KEY_GENERATIONS][MOCK_SVL_PUBKEY_SIZE];
static bool verification_result = true;
static bool hash_error_enabled = false;
static bool verify_error_enabled = false;
static bool init_error_enabled = false;
static uint32_t verify_count = 0;
static uint32_t hash_count = 0;
static bool initialized = false;

/* シンプルなハッシュ計算（Blake2b-256の簡易シミュレーション） */
static void simple_hash(const uint8_t* data, size_t len, uint8_t output[MOCK_SVL_HASH_SIZE])
{
    /* テスト用の決定論的なハッシュ計算 */
    memset(output, 0, MOCK_SVL_HASH_SIZE);
    
    /* 簡易ハッシュ: データの各バイトをハッシュに混ぜる */
    for (size_t i = 0; i < len; i++) {
        output[i % MOCK_SVL_HASH_SIZE] ^= data[i];
        output[(i + 1) % MOCK_SVL_HASH_SIZE] ^= (data[i] >> 4);
    }
    
    /* 長さ情報を混ぜる */
    for (size_t i = 0; i < 4; i++) {
        output[i] ^= (len >> (i * 8)) & 0xFF;
    }
}

enum mbk_svl_result mbk_svl_init(void)
{
    if (init_error_enabled) {
        init_error_enabled = false;
        return MBK_SVL_ERROR_HARDWARE_FAILURE;
    }
    
    initialized = true;
    verify_count = 0;
    hash_count = 0;
    memset(mock_public_keys, 0, sizeof(mock_public_keys));
    return MBK_SVL_SUCCESS;
}

enum mbk_svl_result mbk_svl_verify_signature(const uint8_t* data, size_t data_len,
                                       const uint8_t* signature,
                                       uint8_t key_generation)
{
    (void)data_len; /* Unused parameter in mock */
    
    if (verify_error_enabled) {
        return MBK_SVL_ERROR_SIGNATURE_INVALID;
    }
    
    if (!data || !signature) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }
    
    if (key_generation >= MOCK_SVL_NUM_KEY_GENERATIONS) {
        return MBK_SVL_ERROR_INVALID_KEY_GEN;
    }
    
    verify_count++;
    return verification_result ? MBK_SVL_SUCCESS : MBK_SVL_ERROR_SIGNATURE_INVALID;
}

enum mbk_svl_result mbk_svl_compute_hash(const uint8_t* data, size_t data_len,
                                   uint8_t digest_out[MOCK_SVL_HASH_SIZE])
{
    if (hash_error_enabled) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }
    
    if (!data || !digest_out) {
        return MBK_SVL_ERROR_NULL_POINTER;
    }
    
    hash_count++;
    simple_hash(data, data_len, digest_out);
    return MBK_SVL_SUCCESS;
}

bool mbk_svl_compare_hash(const uint8_t digest1[MOCK_SVL_HASH_SIZE],
                          const uint8_t digest2[MOCK_SVL_HASH_SIZE])
{
    if (!digest1 || !digest2) {
        return false;
    }
    
    return memcmp(digest1, digest2, MOCK_SVL_HASH_SIZE) == 0;
}

/* ============================================================
 * Mock control functions
 * ============================================================ */

void mock_svl_set_public_key(uint8_t key_gen, const uint8_t pubkey[MOCK_SVL_PUBKEY_SIZE])
{
    if (key_gen < MOCK_SVL_NUM_KEY_GENERATIONS && pubkey != NULL) {
        memcpy(mock_public_keys[key_gen], pubkey, MOCK_SVL_PUBKEY_SIZE);
    }
}

void mock_svl_set_verification_result(bool should_pass)
{
    verification_result = should_pass;
}

void mock_svl_inject_hash_error(bool enable)
{
    hash_error_enabled = enable;
}

void mock_svl_inject_verify_error(bool enable)
{
    verify_error_enabled = enable;
}

void mock_svl_inject_init_error(bool enable)
{
    init_error_enabled = enable;
}

void mock_svl_reset(void)
{
    initialized = false;
    verification_result = true;
    hash_error_enabled = false;
    verify_error_enabled = false;
    init_error_enabled = false;
    verify_count = 0;
    hash_count = 0;
    memset(mock_public_keys, 0, sizeof(mock_public_keys));
}

uint32_t mock_svl_get_verify_count(void)
{
    return verify_count;
}

uint32_t mock_svl_get_hash_count(void)
{
    return hash_count;
}
