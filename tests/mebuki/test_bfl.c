/**
 * @file test_bfl.c
 * @brief BFL (Boot Flash Layer) Unit Tests
 *
 * 現行 BFL 実装の白箱テスト
 */

#include "unity.h"
#include "mock_hal_flash.h"
#include "mebuki.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef MBK_BFL_INVALID
#  define MBK_BFL_INVALID UINT32_MAX
#endif

/* 内部型定義 */
struct mbk_boot_history {
    uint16_t max_booted_security_version;
    uint16_t second_max_booted_security_version;
    uint8_t max_key_generation;
    uint8_t second_max_key_generation;
    uint16_t reserved;
};

typedef uint8_t mbk_image_hash[MBK_HASH_SIZE];

struct mbk_boot_record {
    struct mbk_boot_history history;
    mbk_image_hash last_booted_image_hash;
};

struct mbk_bfl_entry {
    uint32_t remaining_stores;
    struct mbk_boot_record record;
    uint32_t integrity;
};

enum mbk_bfl_result {
    MBK_BFL_SUCCESS = 0,
    MBK_BFL_ERROR_NO_REMAINING_STORES,
    MBK_BFL_ERROR_INTEGRITY_MISMATCH,
    MBK_BFL_ERROR_ERASE_FAILED,
    MBK_BFL_ERROR_WRITE_FAILED
};

/* 内部関数プロトタイプ */
extern void bfl_load_entry(struct mbk_bfl_entry* out);
extern enum mbk_bfl_result bfl_store_entry(struct mbk_bfl_entry* inout);

static uintptr_t get_sector_address(int sector_id)
{
    return MBK_DATA_BASE + ((uintptr_t)sector_id * (uintptr_t)MBK_BLOCK_SIZE);
}

static uint32_t crc32_update(uint32_t crc, const uint8_t* data, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        crc ^= (uint32_t)data[i];
        for (int bit = 0; bit < 8; ++bit) {
            const uint32_t lsb = crc & 1U;
            crc >>= 1U;
            if (lsb != 0U) {
                crc ^= 0xEDB88320U;
            }
        }
    }

    return crc;
}

static uint32_t compute_integrity(const struct mbk_bfl_entry* entry)
{
    uint32_t crc = 0xFFFFFFFFU;
    crc = crc32_update(crc, (const uint8_t*)entry, offsetof(struct mbk_bfl_entry, integrity));
    return crc ^ 0xFFFFFFFFU;
}

static void prepare_entry(struct mbk_bfl_entry* entry,
                          uint32_t remaining_stores,
                          uint8_t fill_byte,
                          bool corrupt_integrity)
{
    entry->remaining_stores = remaining_stores;
    memset(&entry->record, fill_byte, sizeof(entry->record));
    entry->integrity = compute_integrity(entry);

    if (corrupt_integrity) {
        entry->integrity ^= 0xA5A5A5A5U;
    }
}

static void write_entry_direct(int sector_id,
                               uint32_t remaining_stores,
                               uint8_t fill_byte,
                               bool corrupt_integrity)
{
    struct mbk_bfl_entry entry;

    prepare_entry(&entry, remaining_stores, fill_byte, corrupt_integrity);
    mock_flash_set_memory(get_sector_address(sector_id), &entry, sizeof(entry));
}

static void read_entry_direct(int sector_id, struct mbk_bfl_entry* out)
{
    memcpy(out, (const void*)get_sector_address(sector_id), sizeof(*out));
}

static void assert_all_bytes_equal(const void* data, size_t size, uint8_t expected)
{
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < size; ++i) {
        TEST_ASSERT_EQUAL_HEX8(expected, bytes[i]);
    }
}

static void assert_entry_matches_fill(const struct mbk_bfl_entry* entry,
                                      uint32_t expected_remaining_stores,
                                      uint8_t expected_fill)
{
    TEST_ASSERT_EQUAL_UINT32(expected_remaining_stores, entry->remaining_stores);
    TEST_ASSERT_EQUAL_UINT32(compute_integrity(entry), entry->integrity);
    assert_all_bytes_equal(&entry->record, sizeof(entry->record), expected_fill);
}

void setUp(void)
{
    TEST_ASSERT_TRUE(mock_flash_reset());
    TEST_ASSERT_EQUAL(0, hal_flash_init());
}

void tearDown(void)
{
}

/* ============================================================
 * 初期化テスト
 * ============================================================ */

void test_bfl_init_empty_flash_returns_uninitialized_data(void)
{
    struct mbk_bfl_entry e;

    bfl_load_entry(&e);

    TEST_ASSERT_EQUAL_UINT32(MBK_BFL_INVALID, e.remaining_stores);
    TEST_ASSERT_EQUAL_UINT32(MBK_BFL_INVALID, e.integrity);
    assert_all_bytes_equal(&e.record, sizeof(e.record), 0xFF);
}

void test_bfl_load_prefers_lower_remaining_stores(void)
{
    write_entry_direct(0, 10U, 0x11, false);
    write_entry_direct(1, 9U, 0x22, false);

    struct mbk_bfl_entry e;
    bfl_load_entry(&e);

    assert_entry_matches_fill(&e, 9U, 0x22);
}

void test_bfl_load_falls_back_when_preferred_integrity_is_invalid(void)
{
    write_entry_direct(0, 20U, 0x44, false);
    write_entry_direct(1, 19U, 0x55, true);

    struct mbk_bfl_entry e;
    bfl_load_entry(&e);

    assert_entry_matches_fill(&e, 20U, 0x44);
}

void test_bfl_load_uses_other_sector_when_first_candidate_is_invalid(void)
{
    write_entry_direct(0, MBK_BFL_INVALID, 0x33, true);
    write_entry_direct(1, 5U, 0x77, false);

    struct mbk_bfl_entry e;
    bfl_load_entry(&e);

    assert_entry_matches_fill(&e, 5U, 0x77);
}

void test_bfl_load_both_integrity_invalid_returns_default(void)
{
    write_entry_direct(0, 30U, 0x66, true);
    write_entry_direct(1, 31U, 0x77, true);

    struct mbk_bfl_entry e;
    bfl_load_entry(&e);

    TEST_ASSERT_EQUAL_UINT32(MBK_BFL_INVALID, e.remaining_stores);
    TEST_ASSERT_EQUAL_UINT32(MBK_BFL_INVALID, e.integrity);
    assert_all_bytes_equal(&e.record, sizeof(e.record), 0xFF);
}

/* ============================================================
 * 書き込みテスト
 * ============================================================ */

void test_bfl_store_entry_writes_to_opposite_sector(void)
{
    struct mbk_bfl_entry e;
    struct mbk_bfl_entry sec0_before;
    struct mbk_bfl_entry sec0_after;
    struct mbk_bfl_entry sec1_after;
    struct mbk_bfl_entry reloaded;

    write_entry_direct(0, 1U, 0xAA, false);

    bfl_load_entry(&e);
    read_entry_direct(0, &sec0_before);
    mock_flash_reset_stats();

    enum mbk_bfl_result result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL(MBK_BFL_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());

    read_entry_direct(0, &sec0_after);
    read_entry_direct(1, &sec1_after);

    TEST_ASSERT_EQUAL_MEMORY(&sec0_before, &sec0_after, sizeof(sec0_after));
    TEST_ASSERT_EQUAL_MEMORY(&e, &sec1_after, sizeof(sec1_after));

    bfl_load_entry(&reloaded);
    TEST_ASSERT_EQUAL_MEMORY(&e, &reloaded, sizeof(reloaded));
    TEST_ASSERT_EQUAL_UINT32(compute_integrity(&sec1_after), sec1_after.integrity);
}

void test_bfl_store_entry_alternates_target_sector_on_consecutive_stores(void)
{
    struct mbk_bfl_entry e;
    struct mbk_bfl_entry sec0_before;
    struct mbk_bfl_entry sec1_before;
    struct mbk_bfl_entry sec0_after;
    struct mbk_bfl_entry sec1_after;

    write_entry_direct(0, 5U, 0xA1, false);

    /* 1回目: セクター0を読み込み、セクター1へ書き込むことを確認 */
    bfl_load_entry(&e);
    read_entry_direct(0, &sec0_before);
    read_entry_direct(1, &sec1_before);
    mock_flash_reset_stats();

    enum mbk_bfl_result first_result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL(MBK_BFL_SUCCESS, first_result);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());

    read_entry_direct(0, &sec0_after);
    read_entry_direct(1, &sec1_after);

    TEST_ASSERT_EQUAL_MEMORY(&sec0_before, &sec0_after, sizeof(sec0_after));
    TEST_ASSERT_EQUAL_MEMORY(&e, &sec1_after, sizeof(sec1_after));

    /* 2回目: セクター1を読み込み、セクター0へ書き込むことを確認 */
    bfl_load_entry(&e);
    read_entry_direct(0, &sec0_before);
    read_entry_direct(1, &sec1_before);
    mock_flash_reset_stats();

    enum mbk_bfl_result second_result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL(MBK_BFL_SUCCESS, second_result);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());

    read_entry_direct(0, &sec0_after);
    read_entry_direct(1, &sec1_after);

    TEST_ASSERT_EQUAL_MEMORY(&e, &sec0_after, sizeof(sec0_after));
    TEST_ASSERT_EQUAL_MEMORY(&sec1_before, &sec1_after, sizeof(sec1_after));
}

void test_bfl_store_entry_returns_erase_failed_when_erase_fails(void)
{
    struct mbk_bfl_entry e;
    struct mbk_bfl_entry before;

    write_entry_direct(0, 30U, 0x5A, false);
    read_entry_direct(0, &before);
    bfl_load_entry(&e);
    mock_flash_reset_stats();
    mock_flash_inject_error(true);

    enum mbk_bfl_result result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL(MBK_BFL_ERROR_ERASE_FAILED, result);
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_write_count());

    read_entry_direct(0, &e);
    TEST_ASSERT_EQUAL_MEMORY(&before, &e, sizeof(e));
}

void test_bfl_store_entry_rejects_zero_remaining_stores(void)
{
    struct mbk_bfl_entry e;
    struct mbk_bfl_entry before;

    write_entry_direct(0, 0, 0x3C, false);
    read_entry_direct(0, &before);
    bfl_load_entry(&e);
    mock_flash_reset_stats();

    enum mbk_bfl_result result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL(MBK_BFL_ERROR_NO_REMAINING_STORES, result);
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_write_count());

    read_entry_direct(0, &e);
    TEST_ASSERT_EQUAL_MEMORY(&before, &e, sizeof(e));
}

/* ============================================================
 * 異常系テスト
 * ============================================================ */

void test_bfl_store_entry_rejects_unknown_integrity_source(void)
{
    struct mbk_bfl_entry e;
    struct mbk_bfl_entry before;

    write_entry_direct(0, 2U, 0x42, false);
    read_entry_direct(0, &before);
    bfl_load_entry(&e);
    e.integrity ^= 0x12345678U;
    mock_flash_reset_stats();

    enum mbk_bfl_result result = bfl_store_entry(&e);

    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_erase_count());
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_write_count());
    TEST_ASSERT_EQUAL(MBK_BFL_ERROR_INTEGRITY_MISMATCH, result);

    read_entry_direct(0, &e);
    TEST_ASSERT_EQUAL_MEMORY(&before, &e, sizeof(e));
}

int main(void)
{
    UNITY_BEGIN();

    /* 初期化テスト */
    RUN_TEST(test_bfl_init_empty_flash_returns_uninitialized_data);
    RUN_TEST(test_bfl_load_prefers_lower_remaining_stores);
    RUN_TEST(test_bfl_load_falls_back_when_preferred_integrity_is_invalid);
    RUN_TEST(test_bfl_load_uses_other_sector_when_first_candidate_is_invalid);
    RUN_TEST(test_bfl_load_both_integrity_invalid_returns_default);

    /* 書き込みテスト */
    RUN_TEST(test_bfl_store_entry_writes_to_opposite_sector);
    RUN_TEST(test_bfl_store_entry_alternates_target_sector_on_consecutive_stores);
    RUN_TEST(test_bfl_store_entry_returns_erase_failed_when_erase_fails);
    RUN_TEST(test_bfl_store_entry_rejects_zero_remaining_stores);

    /* 異常系テスト */
    RUN_TEST(test_bfl_store_entry_rejects_unknown_integrity_source);

    return UNITY_END();
}
