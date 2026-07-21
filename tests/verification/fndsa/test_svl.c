/**
 * @file test_svl.c
 * @brief Unit tests for SVL FN-DSA implementation
 */

#include "unity.h"
#include "mebuki_svl.h"
#include "fndsa.h"

#include <string.h>

#define FNDSA_LOGN FNDSA_LOGN_512
#define FNDSA_SIGN_KEY_SIZE_512 FNDSA_SIGN_KEY_SIZE(FNDSA_LOGN)

static const uint8_t test_seed_key[] = "mebuki-fndsa-test-key-seed";
static const uint8_t test_seed_sig[] = "mebuki-fndsa-test-sig-seed";

static uint8_t test_signing_key[FNDSA_SIGN_KEY_SIZE_512];

uint8_t public_keys[MBK_SVL_NUM_KEY_GENERATIONS][MBK_SVL_PUBKEY_SIZE];

void setUp(void)
{
    memset(public_keys, 0, sizeof(public_keys));

    mbk_svl_init();

    fndsa_keygen_seeded(
        FNDSA_LOGN,
        test_seed_key,
        sizeof(test_seed_key) - 1,
        test_signing_key,
        public_keys[0]);
}

void tearDown(void)
{
}

void test_svl_init_success(void)
{
    enum mbk_svl_result result = mbk_svl_init();
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_signature_valid(void)
{
    const uint8_t test_data[] = "Hello, Mebuki FN-DSA!";
    const size_t data_len = sizeof(test_data) - 1;
    uint8_t signature[MBK_SVL_SIGNATURE_SIZE];
    size_t sig_len;
    enum mbk_svl_result result;

    sig_len = fndsa_sign_seeded(
        test_signing_key,
        sizeof(test_signing_key),
        NULL,
        0,
        FNDSA_HASH_ID_RAW,
        test_data,
        data_len,
        test_seed_sig,
        sizeof(test_seed_sig) - 1,
        signature,
        sizeof(signature));

    TEST_ASSERT_EQUAL_UINT32((uint32_t)MBK_SVL_SIGNATURE_SIZE, (uint32_t)sig_len);

    result = mbk_svl_verify_signature(test_data, data_len, signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_signature_invalid(void)
{
    const uint8_t test_data[] = "Hello, Mebuki FN-DSA!";
    const size_t data_len = sizeof(test_data) - 1;
    uint8_t signature[MBK_SVL_SIGNATURE_SIZE];
    size_t sig_len;
    enum mbk_svl_result result;

    sig_len = fndsa_sign_seeded(
        test_signing_key,
        sizeof(test_signing_key),
        NULL,
        0,
        FNDSA_HASH_ID_RAW,
        test_data,
        data_len,
        test_seed_sig,
        sizeof(test_seed_sig) - 1,
        signature,
        sizeof(signature));

    TEST_ASSERT_EQUAL_UINT32((uint32_t)MBK_SVL_SIGNATURE_SIZE, (uint32_t)sig_len);

    signature[0] ^= 0xFF;
    result = mbk_svl_verify_signature(test_data, data_len, signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_verify_signature_null_data(void)
{
    uint8_t signature[MBK_SVL_SIGNATURE_SIZE] = {0};
    enum mbk_svl_result result = mbk_svl_verify_signature(NULL, 100, signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_null_signature(void)
{
    const uint8_t test_data[] = "Test";
    enum mbk_svl_result result = mbk_svl_verify_signature(test_data, sizeof(test_data) - 1, NULL, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_invalid_key_gen(void)
{
    const uint8_t test_data[] = "Test";
    uint8_t signature[MBK_SVL_SIGNATURE_SIZE] = {0};
    enum mbk_svl_result result = mbk_svl_verify_signature(
        test_data,
        sizeof(test_data) - 1,
        signature,
        MBK_SVL_NUM_KEY_GENERATIONS);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_INVALID_KEY_GEN, result);
}

void test_svl_compute_hash_known_vector(void)
{
    const uint8_t test_data[] = "abc";
    const uint8_t expected_hash[MBK_SVL_HASH_SIZE] = {
        0x50, 0x8c, 0x5e, 0x8c, 0x32, 0x7c, 0x14, 0xe2,
        0xe1, 0xa7, 0x2b, 0xa3, 0x4e, 0xeb, 0x45, 0x2f,
        0x37, 0x45, 0x8b, 0x20, 0x9e, 0xd6, 0x3a, 0x29,
        0x4d, 0x99, 0x9b, 0x4c, 0x86, 0x67, 0x59, 0x82
    };
    uint8_t computed_hash[MBK_SVL_HASH_SIZE];
    enum mbk_svl_result result = mbk_svl_compute_hash(test_data, 3, computed_hash);

    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected_hash, computed_hash, MBK_SVL_HASH_SIZE);
}

void test_svl_compute_hash_consistency(void)
{
    const uint8_t test_data[] = "Consistent hash test";
    uint8_t hash1[MBK_SVL_HASH_SIZE];
    uint8_t hash2[MBK_SVL_HASH_SIZE];

    mbk_svl_compute_hash(test_data, sizeof(test_data) - 1, hash1);
    mbk_svl_compute_hash(test_data, sizeof(test_data) - 1, hash2);

    TEST_ASSERT_EQUAL_HEX8_ARRAY(hash1, hash2, MBK_SVL_HASH_SIZE);
}

void test_svl_compute_hash_null_data(void)
{
    uint8_t digest[MBK_SVL_HASH_SIZE];
    enum mbk_svl_result result = mbk_svl_compute_hash(NULL, 100, digest);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_compute_hash_null_output(void)
{
    const uint8_t test_data[] = "Test";
    enum mbk_svl_result result = mbk_svl_compute_hash(test_data, sizeof(test_data) - 1, NULL);
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
    RUN_TEST(test_svl_verify_signature_invalid);
    RUN_TEST(test_svl_verify_signature_null_data);
    RUN_TEST(test_svl_verify_signature_null_signature);
    RUN_TEST(test_svl_verify_signature_invalid_key_gen);

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
