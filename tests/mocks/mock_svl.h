/**
 * @file mock_svl.h
 * @brief SVL (Signature Verification Layer) Mock for Testing
 * 
 * テスト用のSVLモック実装
 * 署名検証とハッシュ計算をシミュレート
 */

#ifndef MOCK_SVL_H
#define MOCK_SVL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SVL configuration (matching mebuki_svl.h / libmebuki_config.h) */
#define MOCK_SVL_PUBKEY_SIZE          32    // 公開鍵サイズ（バイト）
#define MOCK_SVL_SIGNATURE_SIZE       64    // 署名サイズ（バイト、Ed25519想定）
#define MOCK_SVL_HASH_SIZE            32    // ハッシュダイジェストサイズ（バイト、Blake2b-256）
#define MOCK_SVL_NUM_KEY_GENERATIONS  8     // 鍵世代数

/**
 * @brief SVL戻り値型
 */
enum mbk_svl_result {
    MBK_SVL_SUCCESS = 0,
    MBK_SVL_ERROR_NULL_POINTER = -1,
    MBK_SVL_ERROR_INVALID_KEY_GEN = -2,
    MBK_SVL_ERROR_SIGNATURE_INVALID = -3,
    MBK_SVL_ERROR_HARDWARE_FAILURE = -4,
};

/**
 * @brief SVL初期化
 * @return MBK_SVL_SUCCESS=成功、MBK_SVL_ERROR_HARDWARE_FAILURE=初期化失敗
 */
enum mbk_svl_result mbk_svl_init(void);

/**
 * @brief 署名検証
 * @param[in] data 署名対象データ（Flash上のアドレス）
 * @param[in] data_len データ長（バイト）
 * @param[in] signature 署名データ（Flash上のアドレス）
 * @param[in] key_generation 鍵世代（0 to NUM_KEY_GENERATIONS-1）
 * @return MBK_SVL_SUCCESS=成功、その他=失敗
 */
enum mbk_svl_result mbk_svl_verify_signature(const uint8_t* data, size_t data_len,
                                       const uint8_t* signature,
                                       uint8_t key_generation);

/**
 * @brief ハッシュ計算
 * @param[in] data データポインタ
 * @param[in] data_len データ長
 * @param[out] digest_out ハッシュ出力バッファ（MOCK_SVL_HASH_SIZE バイト）
 * @return MBK_SVL_SUCCESS=成功、MBK_SVL_ERROR_NULL_POINTER=失敗
 */
enum mbk_svl_result mbk_svl_compute_hash(const uint8_t* data, size_t data_len,
                                   uint8_t digest_out[MOCK_SVL_HASH_SIZE]);

/**
 * @brief ハッシュ比較（定数時間）
 * @param[in] digest1 第1ハッシュ
 * @param[in] digest2 第2ハッシュ
 * @return true=一致、false=不一致またはNULL
 */
bool mbk_svl_compare_hash(const uint8_t digest1[MOCK_SVL_HASH_SIZE],
                          const uint8_t digest2[MOCK_SVL_HASH_SIZE]);

/**
 * @brief テスト用：検証結果を設定
 */
void mock_svl_set_verification_result(bool result);

/**
 * @brief テスト用：初期化エラーを注入
 */
void mock_svl_inject_init_error(bool enable);

/**
 * @brief テスト用：検証呼び出し回数を取得
 */
uint32_t mock_svl_get_verify_count(void);

/**
 * @brief テスト用：ハッシュ呼び出し回数を取得
 */
uint32_t mock_svl_get_hash_count(void);

/**
 * @brief テスト用：モック状態をリセット
 */
void mock_svl_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* MOCK_SVL_H */
