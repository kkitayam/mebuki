/**
 * @file test_bsl.c
 * @brief BSL (Boot Sequence Layer) Integration Tests
 *
 * 起動シーケンス統合テスト
 */

#include "unity.h"
#include "mock_hal_flash.h"
#include "mock_svl.h"
#include "mebuki.h"
#include <stdint.h>
#include <string.h>

void setUp(void)
{
    mock_flash_reset();
    mock_svl_reset();
    mbk_svl_init();
}

void tearDown(void)
{
}

/* ヘッダ設定ヘルパー */
static void setup_slot_header(uintptr_t slot_addr, uint16_t sec_ver, uint8_t key_gen, uint32_t sw_size)
{
    uint8_t header[MBK_HEADER_SIZE];
    header[0] = (sec_ver >> 0) & 0xFF;
    header[1] = (sec_ver >> 8) & 0xFF;
    header[2] = key_gen;
    header[3] = 0xFF;  // invalidation_flag=valid
    header[4] = (sw_size >>  0) & 0xFF;
    header[5] = (sw_size >>  8) & 0xFF;
    header[6] = (sw_size >> 16) & 0xFF;
    header[7] = (sw_size >> 24) & 0xFF;

    hal_flash_write(slot_addr, header, MBK_HEADER_SIZE);
}

static void erase_slot_region(uintptr_t slot_addr)
{
    for (uint32_t offset = 0; offset < MBK_SLOT_SIZE; offset += MOCK_FLASH_SECTOR_SIZE) {
        TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(slot_addr + offset));
    }
}

static void write_slot_body(uintptr_t slot_addr, uint8_t fill_byte, uint32_t sw_size)
{
    uint8_t body_chunk[256];
    memset(body_chunk, fill_byte, sizeof(body_chunk));

    uintptr_t body_addr = slot_addr + MBK_HEADER_SIZE;
    uint32_t remaining = sw_size;

    while (remaining > 0) {
        const size_t chunk_size = (remaining < sizeof(body_chunk)) ? remaining : sizeof(body_chunk);
        TEST_ASSERT_EQUAL(0, hal_flash_write(body_addr, body_chunk, chunk_size));
        body_addr += (uintptr_t)chunk_size;
        remaining -= (uint32_t)chunk_size;
    }
}

static void setup_slot_image(uintptr_t slot_addr, uint16_t sec_ver, uint8_t key_gen,
                             uint8_t body_fill, uint32_t sw_size)
{
    erase_slot_region(slot_addr);
    setup_slot_header(slot_addr, sec_ver, key_gen, sw_size);
    write_slot_body(slot_addr, body_fill, sw_size);
}

/* ============================================================
 * 初期化テスト
 * ============================================================ */

void test_bsl_init_success(void)
{
    struct mbk_context ctx;
    enum mbk_result result = mbk_init(&ctx);
    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
}

void test_bsl_init_empty_flash(void)
{
    struct mbk_context ctx;
    // 空のFlashでも初期化成功（初回起動）
    hal_flash_erase_all();
    enum mbk_result result = mbk_init(&ctx);
    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
}

/* ============================================================
 * スロット検索テスト
 * ============================================================ */

void test_bsl_find_bootable_slot_null_param(void)
{
    struct mbk_context ctx;
    mbk_init(&ctx);
    enum mbk_result result = mbk_find_bootable_slot(NULL, NULL);
    TEST_ASSERT_EQUAL(MBK_ERROR_INVALID_PARAM, result);
}

void test_bsl_find_bootable_slot_both_invalid(void)
{
    struct mbk_context ctx;
    // 両スロット無効（初期化前に設定）
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 0);  // software_size=0で無効
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);

    mbk_init(&ctx);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);
    TEST_ASSERT_EQUAL(MBK_ERROR_NO_BOOTABLE_SLOT, result);
}

void test_bsl_find_bootable_slot_single_valid(void)
{
    struct mbk_context ctx;
    // Slot 0のみ有効（初期化前に設定）
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);  // 無効

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
    TEST_ASSERT_EQUAL(MBK_SLOT0_BASE, boot_info.header);
    TEST_ASSERT_EQUAL(MBK_SLOT0_BASE + MBK_HEADER_SIZE, boot_info.entry_point);
}

void test_bsl_find_bootable_slot_priority_slot0(void)
{
    struct mbk_context ctx;
    // Slot 0の方がバージョン高い（初期化前に設定）
    setup_slot_header(MBK_SLOT0_BASE, 200, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 1024);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
}

void test_bsl_find_bootable_slot_priority_slot1(void)
{
    struct mbk_context ctx;
    // Slot 1の方がバージョン高い（初期化前に設定）
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 200, 0, 1024);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, boot_info.slot_id);
}

void test_bsl_find_bootable_slot_signature_verification_failed(void)
{
    struct mbk_context ctx;
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);  // 無効

    mbk_init(&ctx);
    mock_svl_set_verification_result(false);  // 署名検証失敗

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_ERROR_NO_BOOTABLE_SLOT, result);
}

void test_bsl_find_bootable_slot_fallback_to_slot1(void)
{
    struct mbk_context ctx;
    // Slot 0無効、Slot 1のみ有効
    setup_slot_header(MBK_SLOT0_BASE, 200, 0, 0);  // 無効
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 1024);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, boot_info.slot_id);
}

/* ============================================================
 * ロールバック対策テスト
 * ============================================================ */

void test_bsl_rollback_protection(void)
{
    struct mbk_context ctx;
    // ロールバック対策は複雑なBFL永続化が必要なため、基本テストのみ
    // TODO: BFL永続化を考慮したテストケース追加
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);
    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
}

void test_bsl_key_generation_rollback(void)
{
    struct mbk_context ctx;
    // 鍵世代ロールバック対策もBFL永続化が必要
    // TODO: BFL永続化を考慮したテストケース追加
    setup_slot_header(MBK_SLOT0_BASE, 100, 1, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);
    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
}

/* ============================================================
 * 統合テスト
 * ============================================================ */

void test_bsl_full_boot_sequence(void)
{
    struct mbk_context ctx;
    // 完全な起動シーケンス（初期化前に設定）
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 2048);
    setup_slot_header(MBK_SLOT1_BASE, 50, 0, 2048);

    enum mbk_result result = mbk_init(&ctx);
    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);

    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
    TEST_ASSERT_EQUAL(100, boot_info.header->security_version);
    TEST_ASSERT_EQUAL(0, boot_info.header->key_generation);
    TEST_ASSERT_EQUAL(2048, boot_info.header->software_size);
}

void test_bsl_verification_count(void)
{
    struct mbk_context ctx;
    setup_slot_header(MBK_SLOT0_BASE, 100, 0, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    TEST_ASSERT_EQUAL(0, mock_svl_get_verify_count());

    struct mbk_boot_info boot_info;
    mbk_find_bootable_slot(&ctx, &boot_info);

    // 署名検証が1回呼ばれる
    TEST_ASSERT_EQUAL(1, mock_svl_get_verify_count());
}

void test_bsl_verification_skip_on_reboot(void)
{
    struct mbk_context ctx;

    setup_slot_image(MBK_SLOT0_BASE, 200, 0, 0x11, 1024);
    setup_slot_image(MBK_SLOT1_BASE, 100, 0, 0x22, 1024);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());
    TEST_ASSERT_EQUAL(1, mock_svl_get_verify_count());
    TEST_ASSERT_EQUAL(1, mock_svl_get_hash_count());

    struct mbk_context reboot_ctx;
    mbk_init(&reboot_ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    result = mbk_find_bootable_slot(&reboot_ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_write_count());
    TEST_ASSERT_EQUAL(0, mock_svl_get_verify_count());
    TEST_ASSERT_EQUAL(1, mock_svl_get_hash_count());
}

void test_bsl_verification_skip_after_slot_swap(void)
{
    struct mbk_context ctx;

    setup_slot_image(MBK_SLOT0_BASE, 200, 0, 0x11, 1024);
    setup_slot_image(MBK_SLOT1_BASE, 100, 0, 0x22, 1024);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(0, boot_info.slot_id);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());
    TEST_ASSERT_EQUAL(1, mock_svl_get_verify_count());

    setup_slot_image(MBK_SLOT0_BASE, 100, 0, 0x22, 1024);
    setup_slot_image(MBK_SLOT1_BASE, 200, 0, 0x11, 1024);

    struct mbk_context reboot_ctx;
    mbk_init(&reboot_ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    result = mbk_find_bootable_slot(&reboot_ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL(1, boot_info.slot_id);
    TEST_ASSERT_EQUAL_UINT32(0, mock_flash_get_write_count());
    TEST_ASSERT_EQUAL(0, mock_svl_get_verify_count());
    TEST_ASSERT_EQUAL(1, mock_svl_get_hash_count());
}

void test_bsl_writeback_when_boot_data_changes(void)
{
    struct mbk_context ctx;

    setup_slot_image(MBK_SLOT0_BASE, 200, 0, 0x11, 1024);
    setup_slot_header(MBK_SLOT1_BASE, 100, 0, 0);

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());

    setup_slot_image(MBK_SLOT0_BASE, 200, 0, 0x11, 2048);

    struct mbk_context reboot_ctx;
    mbk_init(&reboot_ctx);
    mock_svl_set_verification_result(true);
    mock_flash_reset_stats();

    result = mbk_find_bootable_slot(&reboot_ctx, &boot_info);

    TEST_ASSERT_EQUAL(MBK_SUCCESS, result);
    TEST_ASSERT_EQUAL_UINT32(1, mock_flash_get_write_count());
}

void test_bsl_both_slots_uninitialized(void)
{
    struct mbk_context ctx;
    // 両スロット未初期化の場合は起動不可
    // 実機では工場出荷時に少なくとも1スロットは書き込まれる
    hal_flash_erase_all();  // 完全に空のFlash

    mbk_init(&ctx);
    mock_svl_set_verification_result(true);

    struct mbk_boot_info boot_info;
    enum mbk_result result = mbk_find_bootable_slot(&ctx, &boot_info);

    // 両スロット空なので起動不可
    TEST_ASSERT_EQUAL(MBK_ERROR_NO_BOOTABLE_SLOT, result);
}

int main(void)
{
    UNITY_BEGIN();

    /* 初期化テスト */
    RUN_TEST(test_bsl_init_success);
    RUN_TEST(test_bsl_init_empty_flash);

    /* スロット検索テスト */
    RUN_TEST(test_bsl_find_bootable_slot_null_param);
    RUN_TEST(test_bsl_find_bootable_slot_both_invalid);
    RUN_TEST(test_bsl_find_bootable_slot_single_valid);
    RUN_TEST(test_bsl_find_bootable_slot_priority_slot0);
    RUN_TEST(test_bsl_find_bootable_slot_priority_slot1);
    RUN_TEST(test_bsl_find_bootable_slot_signature_verification_failed);
    RUN_TEST(test_bsl_find_bootable_slot_fallback_to_slot1);

    /* ロールバック対策テスト */
    RUN_TEST(test_bsl_rollback_protection);
    RUN_TEST(test_bsl_key_generation_rollback);

    /* 統合テスト */
    RUN_TEST(test_bsl_full_boot_sequence);
    RUN_TEST(test_bsl_verification_count);
    RUN_TEST(test_bsl_verification_skip_on_reboot);
    RUN_TEST(test_bsl_verification_skip_after_slot_swap);
    RUN_TEST(test_bsl_writeback_when_boot_data_changes);
    RUN_TEST(test_bsl_both_slots_uninitialized);

    return UNITY_END();
}
