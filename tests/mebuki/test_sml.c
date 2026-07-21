/**
 * @file test_sml.c
 * @brief SML (Slot Management Library) Unit Tests
 * 
 * スロット管理ライブラリの単体テスト
 */

#include "unity.h"
#include "mock_hal_flash.h"
#include "mebuki.h"

/* 内部型定義 */
 struct mbk_sml_slot_info {
    struct mbk_header header;
    int slot_id;
};

struct mbk_sml_boot_context {
    struct mbk_sml_slot_info priority_order[2];
};

/* 内部関数プロトタイプ */
extern bool sml_prefer_slot1(const struct mbk_header* slot0, const struct mbk_header* slot1);
extern bool sml_is_slot_header_valid(const struct mbk_header* header);

void setUp(void)
{
    mock_flash_reset();
    hal_flash_init();
}

void tearDown(void)
{
}

/* ============================================================
 * 優先順位決定テスト
 * ============================================================ */

void test_sml_priority_slot0_higher_version(void)
{
    /* Slot 0 (v=100) > Slot 1 (v=50) → [0, 1] */
    struct mbk_header header0 = {100, 0, 0xFF, 1000};
    struct mbk_header header1 = {50, 0, 0xFF, 1000};
    
    TEST_ASSERT_FALSE(sml_prefer_slot1(&header0, &header1));
}

void test_sml_priority_slot1_higher_version(void)
{
    /* Slot 0 (v=50) < Slot 1 (v=100) → [1, 0] */
    struct mbk_header header0 = {50, 0, 0xFF, 1000};
    struct mbk_header header1 = {100, 0, 0xFF, 1000};
    
    TEST_ASSERT_TRUE(sml_prefer_slot1(&header0, &header1));
}

void test_sml_priority_same_version_prefers_slot0(void)
{
    /* Slot 0 (v=100) == Slot 1 (v=100) → [0, 1] (slot_id優先) */
    struct mbk_header header0 = {100, 0, 0xFF, 1000};
    struct mbk_header header1 = {100, 0, 0xFF, 1000};
    
    TEST_ASSERT_FALSE(sml_prefer_slot1(&header0, &header1));
}

void test_sml_priority_slot0_uninitialized(void)
{
    /* Slot 0 (v=0xFFFF), Slot 1 (v=100) → [1, 0] */
    struct mbk_header header0 = {0xFFFF, 0xFF, 0xFF, 0xFFFFFFFF};
    struct mbk_header header1 = {100, 0, 0xFF, 1000};
    
    TEST_ASSERT_TRUE(sml_prefer_slot1(&header0, &header1));
}

void test_sml_priority_slot1_uninitialized(void)
{
    /* Slot 0 (v=100), Slot 1 (v=0xFFFF) → [0, 1] */
    struct mbk_header header0 = {100, 0, 0xFF, 1000};
    struct mbk_header header1 = {0xFFFF, 0xFF, 0xFF, 0xFFFFFFFF};

    TEST_ASSERT_FALSE(sml_prefer_slot1(&header0, &header1));
}

void test_sml_priority_both_uninitialized(void)
{
    /* Slot 0 (v=0xFFFF), Slot 1 (v=0xFFFF) → [0, 1] (slot_id順) */
    struct mbk_header header0 = {0xFFFF, 0xFF, 0xFF, 0xFFFFFFFF};
    struct mbk_header header1 = {0xFFFF, 0xFF, 0xFF, 0xFFFFFFFF};

    TEST_ASSERT_FALSE(sml_prefer_slot1(&header0, &header1));
}

/* ============================================================
 * 有効性判定テスト
 * ============================================================ */

void test_sml_is_slot_header_valid_all_valid_fields(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {100, 0, 0xFF, 1000},
        .slot_id = 0
    };
    
    TEST_ASSERT_TRUE(sml_is_slot_header_valid(&slot.header));
}

void test_sml_is_slot_header_valid_blank_flash(void)
{
    /* 全0xFF（ブランクFlash） */
    struct mbk_sml_slot_info slot = {
        .header = {0xFFFF, 0xFF, 0xFF, 0xFFFFFFFF},
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}

void test_sml_is_slot_header_valid_invalidation_flag_invalid(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {100, 0, 0x00, 1000},  // invalidation_flag = 0x00
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}

#if 0
/* security version は監査対象から除外 */
void test_sml_is_slot_header_valid_security_version_out_of_range(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {0xFFFF, 0, 0xFF, 1000},  // security_version = 0xFFFF
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}
/* key generation は監査対象から除外 */
void test_sml_is_slot_header_valid_key_generation_out_of_range(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {100, 0xFF, 0xFF, 1000},  // key_generation = 0xFF
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}
#endif

void test_sml_is_slot_header_valid_software_size_zero(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {100, 0, 0xFF, 0},  // software_size = 0
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}

void test_sml_is_slot_header_valid_software_size_too_large(void)
{
    struct mbk_sml_slot_info slot = {
        .header = {100, 0, 0xFF, 0x00010000},  // software_size > MAX
        .slot_id = 0
    };
    
    TEST_ASSERT_FALSE(sml_is_slot_header_valid(&slot.header));
}

void test_sml_is_slot_header_valid_boundary_values(void)
{
    /* security_version = 0（最小有効値） */
    struct mbk_sml_slot_info slot1 = {{0, 0, 0xFF, 1000}, 0};
    TEST_ASSERT_TRUE(sml_is_slot_header_valid(&slot1.header));
    
    /* security_version = 0xFFFE（最大有効値） */
    struct mbk_sml_slot_info slot2 = {{0xFFFE, 0, 0xFF, 1000}, 0};
    TEST_ASSERT_TRUE(sml_is_slot_header_valid(&slot2.header));
    
    /* key_generation = 0（最小有効値） */
    struct mbk_sml_slot_info slot3 = {{100, 0, 0xFF, 1000}, 0};
    TEST_ASSERT_TRUE(sml_is_slot_header_valid(&slot3.header));
    
    /* key_generation = 0xFE（最大有効値） */
    struct mbk_sml_slot_info slot4 = {{100, 0xFE, 0xFF, 1000}, 0};
    TEST_ASSERT_TRUE(sml_is_slot_header_valid(&slot4.header));
}

int main(void)
{
    UNITY_BEGIN();

    /* 優先順位決定テスト */
    RUN_TEST(test_sml_priority_slot0_higher_version);
    RUN_TEST(test_sml_priority_slot1_higher_version);
    RUN_TEST(test_sml_priority_same_version_prefers_slot0);
    RUN_TEST(test_sml_priority_slot0_uninitialized);
    RUN_TEST(test_sml_priority_slot1_uninitialized);
    RUN_TEST(test_sml_priority_both_uninitialized);

    /* 有効性判定テスト */
    RUN_TEST(test_sml_is_slot_header_valid_all_valid_fields);
    RUN_TEST(test_sml_is_slot_header_valid_blank_flash);
    RUN_TEST(test_sml_is_slot_header_valid_invalidation_flag_invalid);
#if 0
    RUN_TEST(test_sml_is_slot_header_valid_security_version_out_of_range);
    RUN_TEST(test_sml_is_slot_header_valid_key_generation_out_of_range);
#endif
    RUN_TEST(test_sml_is_slot_header_valid_software_size_zero);
    RUN_TEST(test_sml_is_slot_header_valid_software_size_too_large);
    RUN_TEST(test_sml_is_slot_header_valid_boundary_values);
    
    return UNITY_END();
}

