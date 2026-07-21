/**
 * @file test_svl.c
 * @brief SVL (Signature Verification Layer) Unit Tests
 * 
 * 署名検証・ハッシュ計算レイヤのテスト
 * SVLは外部実装として提供されるため、mock_svlを使用
 */

#include "unity.h"
#include "mock_svl.h"
#include "mebuki.h"
#include <string.h>

/* 定数は mock_svl.h の値を直接使用 */

void setUp(void)
{
    mock_svl_reset();
    mbk_svl_init();
}

void tearDown(void)
{
}

/* ============================================================
 * 初期化テスト
 * ============================================================ */

void test_svl_init_success(void)
{
    enum mbk_svl_result result = mbk_svl_init();
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_init_hardware_failure(void)
{
    mock_svl_reset();
    mock_svl_inject_init_error(true);
    enum mbk_svl_result result = mbk_svl_init();
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_HARDWARE_FAILURE, result);
}

/* ============================================================
 * 署名検証テスト
 * ============================================================ */

void test_svl_verify_signature_success(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    mock_svl_set_verification_result(true);
    
    enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), 
                                                    signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_signature_invalid(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    mock_svl_set_verification_result(false);
    
    enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), 
                                                    signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_SIGNATURE_INVALID, result);
}

void test_svl_verify_signature_null_data(void)
{
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    memset(signature, 0x55, sizeof(signature));
    
    enum mbk_svl_result result = mbk_svl_verify_signature(NULL, 256, signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_null_signature(void)
{
    uint8_t data[256];
    memset(data, 0xAA, sizeof(data));
    
    enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), NULL, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_verify_signature_invalid_key_gen(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    // key_generation = MBK_NUM_KEY_GENERATIONS (out of range)
    enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), 
                                                    signature, MOCK_SVL_NUM_KEY_GENERATIONS);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_INVALID_KEY_GEN, result);
}

void test_svl_verify_signature_key_gen_max_valid(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    mock_svl_set_verification_result(true);
    
    // key_generation = MBK_NUM_KEY_GENERATIONS - 1 (最大有効値)
    enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), 
                                                    signature, MOCK_SVL_NUM_KEY_GENERATIONS - 1);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_verify_signature_all_key_generations(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    mock_svl_set_verification_result(true);
    
    // 全ての有効な鍵世代で検証成功
    for (uint8_t kg = 0; kg < MOCK_SVL_NUM_KEY_GENERATIONS; kg++) {
        enum mbk_svl_result result = mbk_svl_verify_signature(data, sizeof(data), 
                                                        signature, kg);
        TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
    }
}

/* ============================================================
 * ハッシュ計算テスト
 * ============================================================ */

void test_svl_compute_hash_success(void)
{
    uint8_t data[256];
    uint8_t digest[MOCK_SVL_HASH_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    
    enum mbk_svl_result result = mbk_svl_compute_hash(data, sizeof(data), digest);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

void test_svl_compute_hash_null_data(void)
{
    uint8_t digest[MOCK_SVL_HASH_SIZE];
    
    enum mbk_svl_result result = mbk_svl_compute_hash(NULL, 256, digest);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_compute_hash_null_output(void)
{
    uint8_t data[256];
    memset(data, 0xAA, sizeof(data));
    
    enum mbk_svl_result result = mbk_svl_compute_hash(data, sizeof(data), NULL);
    TEST_ASSERT_EQUAL(MBK_SVL_ERROR_NULL_POINTER, result);
}

void test_svl_compute_hash_same_data_same_result(void)
{
    uint8_t data[256];
    uint8_t digest1[MOCK_SVL_HASH_SIZE];
    uint8_t digest2[MOCK_SVL_HASH_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    
    mbk_svl_compute_hash(data, sizeof(data), digest1);
    mbk_svl_compute_hash(data, sizeof(data), digest2);
    
    TEST_ASSERT_EQUAL_MEMORY(digest1, digest2, MOCK_SVL_HASH_SIZE);
}

void test_svl_compute_hash_different_data_different_result(void)
{
    uint8_t data1[256];
    uint8_t data2[257];  /* 異なる長さを使用 */
    uint8_t digest1[MOCK_SVL_HASH_SIZE];
    uint8_t digest2[MOCK_SVL_HASH_SIZE];
    
    memset(data1, 0xAA, sizeof(data1));
    memset(data2, 0xAA, sizeof(data2));  /* 同じパターンだが長さが異なる */
    
    mbk_svl_compute_hash(data1, sizeof(data1), digest1);
    mbk_svl_compute_hash(data2, sizeof(data2), digest2);
    
    /* 異なる長さのデータは異なるハッシュを生成 */
    bool different = (memcmp(digest1, digest2, MOCK_SVL_HASH_SIZE) != 0);
    TEST_ASSERT_TRUE(different);
}

void test_svl_compute_hash_hardware_error(void)
{
    uint8_t data[256];
    uint8_t digest[MOCK_SVL_HASH_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    
    enum mbk_svl_result result = mbk_svl_compute_hash(data, sizeof(data), digest);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, result);
}

/* ============================================================
 * ハッシュ比較テスト
 * ============================================================ */

void test_svl_compare_hash_equal(void)
{
    uint8_t digest1[MOCK_SVL_HASH_SIZE];
    uint8_t digest2[MOCK_SVL_HASH_SIZE];
    
    memset(digest1, 0xAA, MBK_HASH_SIZE);
    memset(digest2, 0xAA, MBK_HASH_SIZE);
    
    bool result = mbk_svl_compare_hash(digest1, digest2);
    TEST_ASSERT_TRUE(result);
}

void test_svl_compare_hash_not_equal(void)
{
    uint8_t digest1[MBK_HASH_SIZE];
    uint8_t digest2[MBK_HASH_SIZE];
    
    memset(digest1, 0xAA, MBK_HASH_SIZE);
    memset(digest2, 0xBB, MBK_HASH_SIZE);
    
    bool result = mbk_svl_compare_hash(digest1, digest2);
    TEST_ASSERT_FALSE(result);
}

void test_svl_compare_hash_single_bit_difference(void)
{
    uint8_t digest1[MBK_HASH_SIZE];
    uint8_t digest2[MBK_HASH_SIZE];
    
    memset(digest1, 0xAA, MBK_HASH_SIZE);
    memcpy(digest2, digest1, MBK_HASH_SIZE);
    digest2[15] ^= 0x01;  // 1ビットだけ違う
    
    bool result = mbk_svl_compare_hash(digest1, digest2);
    TEST_ASSERT_FALSE(result);
}

void test_svl_compare_hash_null_digest1(void)
{
    uint8_t digest2[MBK_HASH_SIZE];
    memset(digest2, 0xAA, MBK_HASH_SIZE);
    
    bool result = mbk_svl_compare_hash(NULL, digest2);
    TEST_ASSERT_FALSE(result);
}

void test_svl_compare_hash_null_digest2(void)
{
    uint8_t digest1[MBK_HASH_SIZE];
    memset(digest1, 0xAA, MBK_HASH_SIZE);
    
    bool result = mbk_svl_compare_hash(digest1, NULL);
    TEST_ASSERT_FALSE(result);
}

void test_svl_compare_hash_both_null(void)
{
    bool result = mbk_svl_compare_hash(NULL, NULL);
    TEST_ASSERT_FALSE(result);
}

/* ============================================================
 * 統合テスト
 * ============================================================ */

void test_svl_hash_then_verify_workflow(void)
{
    uint8_t data[256];
    uint8_t signature[MBK_SIGNATURE_SIZE];
    uint8_t digest1[MBK_HASH_SIZE];
    uint8_t digest2[MBK_HASH_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    // ハッシュ計算
    enum mbk_svl_result hash_result = mbk_svl_compute_hash(data, sizeof(data), digest1);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, hash_result);
    
    // 再度ハッシュ計算
    hash_result = mbk_svl_compute_hash(data, sizeof(data), digest2);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, hash_result);
    
    // ハッシュ比較
    bool match = mbk_svl_compare_hash(digest1, digest2);
    TEST_ASSERT_TRUE(match);
    
    // 署名検証
    mock_svl_set_verification_result(true);
    enum mbk_svl_result verify_result = mbk_svl_verify_signature(data, sizeof(data), 
                                                          signature, 0);
    TEST_ASSERT_EQUAL(MBK_SVL_SUCCESS, verify_result);
}

void test_svl_verification_count(void)
{
    uint8_t data[256];
    uint8_t signature[MOCK_SVL_SIGNATURE_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    memset(signature, 0x55, sizeof(signature));
    
    mock_svl_set_verification_result(true);
    
    TEST_ASSERT_EQUAL(0, mock_svl_get_verify_count());
    
    // 3回検証
    mbk_svl_verify_signature(data, sizeof(data), signature, 0);
    mbk_svl_verify_signature(data, sizeof(data), signature, 1);
    mbk_svl_verify_signature(data, sizeof(data), signature, 2);
    
    TEST_ASSERT_EQUAL(3, mock_svl_get_verify_count());
}

void test_svl_hash_count(void)
{
    uint8_t data[256];
    uint8_t digest[MOCK_SVL_HASH_SIZE];
    
    memset(data, 0xAA, sizeof(data));
    
    TEST_ASSERT_EQUAL(0, mock_svl_get_hash_count());
    
    // 5回ハッシュ計算
    for (int i = 0; i < 5; i++) {
        mbk_svl_compute_hash(data, sizeof(data), digest);
    }
    
    TEST_ASSERT_EQUAL(5, mock_svl_get_hash_count());
}

int main(void)
{
    UNITY_BEGIN();
    
    /* 初期化テスト */
    RUN_TEST(test_svl_init_success);
    RUN_TEST(test_svl_init_hardware_failure);
    
    /* 署名検証テスト */
    RUN_TEST(test_svl_verify_signature_success);
    RUN_TEST(test_svl_verify_signature_invalid);
    RUN_TEST(test_svl_verify_signature_null_data);
    RUN_TEST(test_svl_verify_signature_null_signature);
    RUN_TEST(test_svl_verify_signature_invalid_key_gen);
    RUN_TEST(test_svl_verify_signature_key_gen_max_valid);
    RUN_TEST(test_svl_verify_signature_all_key_generations);
    
    /* ハッシュ計算テスト */
    RUN_TEST(test_svl_compute_hash_success);
    RUN_TEST(test_svl_compute_hash_null_data);
    RUN_TEST(test_svl_compute_hash_null_output);
    RUN_TEST(test_svl_compute_hash_same_data_same_result);
    RUN_TEST(test_svl_compute_hash_different_data_different_result);
    RUN_TEST(test_svl_compute_hash_hardware_error);
    
    /* ハッシュ比較テスト */
    RUN_TEST(test_svl_compare_hash_equal);
    RUN_TEST(test_svl_compare_hash_not_equal);
    RUN_TEST(test_svl_compare_hash_single_bit_difference);
    RUN_TEST(test_svl_compare_hash_null_digest1);
    RUN_TEST(test_svl_compare_hash_null_digest2);
    RUN_TEST(test_svl_compare_hash_both_null);
    
    /* 統合テスト */
    RUN_TEST(test_svl_hash_then_verify_workflow);
    RUN_TEST(test_svl_verification_count);
    RUN_TEST(test_svl_hash_count);
    
    return UNITY_END();
}
