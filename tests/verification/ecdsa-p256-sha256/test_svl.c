/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "unity.h"
#include "mebuki_svl.h"

#include <string.h>

/*
 * Pre-computed test key pair for ECDSA P-256.
 * Derived deterministically from seed "mebuki-ecdsa-test-key-seed".
 *
 * Private key d (for reference, not used in verification tests):
 *   sha256("mebuki-ecdsa-test-key-seed") mod n
 */

/* Uncompressed P-256 public key (65 bytes: 0x04 || x || y) */
static const uint8_t test_pubkey[MBK_SVL_PUBKEY_SIZE] = {
    0x04, 0x2e, 0x2a, 0xd1, 0x1c, 0xb4, 0x6b, 0xb9,
    0x1b, 0x94, 0xf3, 0x52, 0xe3, 0x4c, 0xcd, 0xde,
    0x07, 0x65, 0xb8, 0x2b, 0xb2, 0x9b, 0x21, 0x5e,
    0x9d, 0xd6, 0x42, 0x9c, 0x2b, 0xbc, 0x1e, 0xd1,
    0x96, 0xd7, 0x0c, 0xad, 0xa7, 0xb7, 0xb1, 0x0d,
    0x11, 0x28, 0x07, 0xe7, 0xbd, 0x0f, 0x77, 0x26,
    0xbd, 0x21, 0x8d, 0xb9, 0xc5, 0x5c, 0x75, 0x90,
    0x6d, 0x38, 0xfb, 0xfd, 0x5f, 0xaa, 0x97, 0xab,
    0xeb
};

/*
 * Valid ECDSA P-256 signature (raw r||s, 64 bytes) over
 * "Hello, Mebuki ECDSA!" using SHA-256 and the test private key.
 */
static const uint8_t test_signature[MBK_SVL_SIGNATURE_SIZE] = {
    0x15, 0x58, 0xde, 0x2a, 0x77, 0x68, 0xdc, 0x7c,
    0xa2, 0xab, 0x5f, 0x9f, 0x77, 0xec, 0x7d, 0x14,
    0xf4, 0x44, 0x23, 0x87, 0xd8, 0xb1, 0x4c, 0x3e,
    0x8d, 0x70, 0x35, 0x22, 0x7e, 0x32, 0x0e, 0x39,
    0x6d, 0x69, 0x30, 0x73, 0x7d, 0x17, 0xd6, 0xca,
    0xb5, 0xd2, 0xf8, 0x55, 0x81, 0xb8, 0xa6, 0x0d,
    0x2f, 0x78, 0x1e, 0x38, 0x7a, 0xfb, 0x1e, 0x14,
    0x1a, 0x59, 0x4c, 0x9b, 0x74, 0x83, 0x01, 0x15
};

static const uint8_t test_data[] = "Hello, Mebuki ECDSA!";
static const size_t test_data_len = sizeof(test_data) - 1;

uint8_t public_keys[MBK_SVL_NUM_KEY_GENERATIONS][MBK_SVL_PUBKEY_SIZE];

void setUp(void)
{
    memset(public_keys, 0, sizeof(public_keys));
    mbk_svl_init();
    memcpy(public_keys[0], test_pubkey, MBK_SVL_PUBKEY_SIZE);
}

void tearDown(void)
{
}

void test_svl_init_success(void)
{
    int result = mbk_svl_init();
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_signature_valid(void)
{
    int result = mbk_svl_verify_signature(
        test_data, test_data_len, test_signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_data_tampered(void)
{
    uint8_t tampered_data[sizeof(test_data)];
    memcpy(tampered_data, test_data, sizeof(test_data));
    tampered_data[0] ^= 0xFF;

    int result = mbk_svl_verify_signature(
        tampered_data, test_data_len, test_signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_verify_signature_tampered(void)
{
    uint8_t tampered_sig[MBK_SVL_SIGNATURE_SIZE];
    memcpy(tampered_sig, test_signature, MBK_SVL_SIGNATURE_SIZE);
    tampered_sig[0] ^= 0xFF;

    int result = mbk_svl_verify_signature(
        test_data, test_data_len, tampered_sig, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_verify_pubkey_tampered(void)
{
    public_keys[0][1] ^= 0xFF;

    int result = mbk_svl_verify_signature(
        test_data, test_data_len, test_signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_verify_signature_null_data(void)
{
    int result = mbk_svl_verify_signature(
        NULL, test_data_len, test_signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_null_signature(void)
{
    int result = mbk_svl_verify_signature(
        test_data, test_data_len, NULL, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_invalid_key_gen(void)
{
    int result = mbk_svl_verify_signature(
        test_data,
        test_data_len,
        test_signature,
        MBK_SVL_NUM_KEY_GENERATIONS);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_INVALID_KEY_GEN, result);
}

void test_svl_verify_zero_length_data(void)
{
    uint8_t tampered_sig[MBK_SVL_SIGNATURE_SIZE] = {0};
    int result = mbk_svl_verify_signature(
        test_data, 0, tampered_sig, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_compute_hash_known_vector(void)
{
    const uint8_t input[] = "abc";
    /* SHA-256("abc") */
    const uint8_t expected[MBK_SVL_HASH_SIZE] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
    };
    uint8_t computed[MBK_SVL_HASH_SIZE];
    int result = mbk_svl_compute_hash(input, 3, computed);

    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected, computed, MBK_SVL_HASH_SIZE);
}

void test_svl_compute_hash_consistency(void)
{
    const uint8_t input[] = "Consistent hash test";
    uint8_t hash1[MBK_SVL_HASH_SIZE];
    uint8_t hash2[MBK_SVL_HASH_SIZE];

    mbk_svl_compute_hash(input, sizeof(input) - 1, hash1);
    mbk_svl_compute_hash(input, sizeof(input) - 1, hash2);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(hash1, hash2, MBK_SVL_HASH_SIZE);
}

void test_svl_compute_hash_null_data(void)
{
    uint8_t digest[MBK_SVL_HASH_SIZE];
    int result = mbk_svl_compute_hash(NULL, 100, digest);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_compute_hash_null_output(void)
{
    const uint8_t input[] = "Test";
    int result = mbk_svl_compute_hash(input, sizeof(input) - 1, NULL);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_compare_hash_equal(void)
{
    const uint8_t hash1[MBK_SVL_HASH_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    const uint8_t hash2[MBK_SVL_HASH_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    TEST_ASSERT_TRUE(mbk_svl_compare_hash(hash1, hash2));
}

void test_svl_compare_hash_different(void)
{
    const uint8_t hash1[MBK_SVL_HASH_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    const uint8_t hash2[MBK_SVL_HASH_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0xFF
    };
    TEST_ASSERT_FALSE(mbk_svl_compare_hash(hash1, hash2));
}

void test_svl_compare_hash_null_first(void)
{
    const uint8_t hash[MBK_SVL_HASH_SIZE] = {0};
    TEST_ASSERT_FALSE(mbk_svl_compare_hash(NULL, hash));
}

void test_svl_compare_hash_null_second(void)
{
    const uint8_t hash[MBK_SVL_HASH_SIZE] = {0};
    TEST_ASSERT_FALSE(mbk_svl_compare_hash(hash, NULL));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_svl_init_success);

    RUN_TEST(test_svl_verify_signature_valid);
    RUN_TEST(test_svl_verify_data_tampered);
    RUN_TEST(test_svl_verify_signature_tampered);
    RUN_TEST(test_svl_verify_pubkey_tampered);
    RUN_TEST(test_svl_verify_signature_null_data);
    RUN_TEST(test_svl_verify_signature_null_signature);
    RUN_TEST(test_svl_verify_signature_invalid_key_gen);
    RUN_TEST(test_svl_verify_zero_length_data);

    RUN_TEST(test_svl_compute_hash_known_vector);
    RUN_TEST(test_svl_compute_hash_consistency);
    RUN_TEST(test_svl_compute_hash_null_data);
    RUN_TEST(test_svl_compute_hash_null_output);

    RUN_TEST(test_svl_compare_hash_equal);
    RUN_TEST(test_svl_compare_hash_different);
    RUN_TEST(test_svl_compare_hash_null_first);
    RUN_TEST(test_svl_compare_hash_null_second);

    return UNITY_END();
}
