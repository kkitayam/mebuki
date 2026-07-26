/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "unity.h"
#include "mock_hal_flash.h"
#include "mebuki.h"
#include <string.h>

 struct mbk_boot_history {
    uint16_t max_booted_security_version;
    uint16_t second_max_booted_security_version;
    uint8_t  max_key_generation;
    uint8_t  second_max_key_generation;
    uint16_t reserved;
};

extern bool bel_is_security_version_eligible(uint16_t security_version,
                                            const uint16_t max_booted_security_version,
                                            const uint16_t second_max_booted_security_version);
extern bool bel_is_key_generation_eligible(uint8_t key_gen,
                                            const uint8_t max_key_generation,
                                            const uint8_t second_max_key_generation);
extern bool bel_accept_security_version(uint16_t security_version,
                                        struct mbk_boot_history* history);
extern bool bel_accept_key_generation(uint8_t key_gen,
                                      struct mbk_boot_history* history);

void setUp(void)
{
    mock_flash_reset();
}

void tearDown(void)
{
}

/* ============================================================
 * Security Version Tests
 * ============================================================ */

void test_bel_security_version_first_boot_accepts_any(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 0xFFFF,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    /* The 1st boot is unconditionally accepted */
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(0, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(100, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(0xFFFE, history.max_booted_security_version, history.second_max_booted_security_version));
}

void test_bel_security_version_second_boot_accepts_any(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 5,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0,
        .second_max_key_generation = 0xFF
    };

    /* The 2nd boot is unconditionally accepted */
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(0, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(3, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(5, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(10, history.max_booted_security_version, history.second_max_booted_security_version));
}

void test_bel_security_version_monotonic_sequence(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 2,
        .second_max_booted_security_version = 1,
        .max_key_generation = 0,
        .second_max_key_generation = 0
    };

    /* T = min(2, 1) = 1 -> v >= 1 is accepted */
    TEST_ASSERT_FALSE(bel_is_security_version_eligible(0, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(1, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(2, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(3, history.max_booted_security_version, history.second_max_booted_security_version));
}

void test_bel_security_version_mixed_sequence(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 5,
        .second_max_booted_security_version = 3,
        .max_key_generation = 0,
        .second_max_key_generation = 0
    };

    /* T = min(5, 3) = 3 -> v >= 3 is accepted */
    TEST_ASSERT_FALSE(bel_is_security_version_eligible(0, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_FALSE(bel_is_security_version_eligible(2, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(3, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(4, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(5, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(10, history.max_booted_security_version, history.second_max_booted_security_version));
}

void test_bel_security_version_rejects_uninitialized(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 5,
        .second_max_booted_security_version = 3,
        .max_key_generation = 0,
        .second_max_key_generation = 0
    };

    /* 0xFFFF (uninitialized) is always rejected */
    TEST_ASSERT_FALSE(bel_is_security_version_eligible(0xFFFF, history.max_booted_security_version, history.second_max_booted_security_version));
}

void test_bel_security_version_boundary_values(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 0xFFFF,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    /* Boundary value tests */
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(0, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_TRUE(bel_is_security_version_eligible(0xFFFE, history.max_booted_security_version, history.second_max_booted_security_version));
    TEST_ASSERT_FALSE(bel_is_security_version_eligible(0xFFFF, history.max_booted_security_version, history.second_max_booted_security_version));
}

/* ============================================================
 * Security Version Accept Tests
 * ============================================================ */

void test_bel_security_version_accept_first(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 0xFFFF,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    TEST_ASSERT_TRUE(bel_accept_security_version(5, &history));

    TEST_ASSERT_EQUAL_UINT16(5, history.max_booted_security_version);
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, history.second_max_booted_security_version);
}

void test_bel_security_version_accept_second_higher(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 5,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0,
        .second_max_key_generation = 0xFF
    };

    TEST_ASSERT_TRUE(bel_accept_security_version(10, &history));

    TEST_ASSERT_EQUAL_UINT16(10, history.max_booted_security_version);
    TEST_ASSERT_EQUAL_UINT16(5, history.second_max_booted_security_version);
}

void test_bel_security_version_accept_second_lower(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 5,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0,
        .second_max_key_generation = 0xFF
    };

    TEST_ASSERT_TRUE(bel_accept_security_version(3, &history));

    TEST_ASSERT_EQUAL_UINT16(5, history.max_booted_security_version);
    TEST_ASSERT_EQUAL_UINT16(3, history.second_max_booted_security_version);
}

void test_bel_security_version_accept_updates_second(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 10,
        .second_max_booted_security_version = 5,
        .max_key_generation = 0,
        .second_max_key_generation = 0
    };

    /* v=7 -> M > v > S so S' = 7 */
    TEST_ASSERT_TRUE(bel_accept_security_version(7, &history));

    TEST_ASSERT_EQUAL_UINT16(10, history.max_booted_security_version);
    TEST_ASSERT_EQUAL_UINT16(7, history.second_max_booted_security_version);
}

/* ============================================================
 * Key Generation Tests
 * ============================================================ */

void test_bel_key_generation_first_boot_accepts_any(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 0xFFFF,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    /* The 1st boot is unconditionally accepted (within range) */
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(0, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(5, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(7, history.max_key_generation, history.second_max_key_generation));
}

void test_bel_key_generation_rejects_out_of_range(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 0xFFFF,
        .second_max_booted_security_version = 0xFFFF,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    /* NUM_KEY_GENERATIONS=8 so 8 or higher is rejected */
    TEST_ASSERT_FALSE(bel_is_key_generation_eligible(8, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_FALSE(bel_is_key_generation_eligible(0xFE, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_FALSE(bel_is_key_generation_eligible(0xFF, history.max_key_generation, history.second_max_key_generation));
}

void test_bel_key_generation_independent_from_security_version(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 10,
        .second_max_booted_security_version = 5,
        .max_key_generation = 2,
        .second_max_key_generation = 1
    };

    /* T = min(2, 1) = 1 -> key_gen >= 1 is accepted */
    TEST_ASSERT_FALSE(bel_is_key_generation_eligible(0, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(1, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(2, history.max_key_generation, history.second_max_key_generation));
    TEST_ASSERT_TRUE(bel_is_key_generation_eligible(3, history.max_key_generation, history.second_max_key_generation));
}

/* ============================================================
 * Slot Bootable Tests
 * ============================================================ */

void test_bel_security_version_accept_same_value_returns_false(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 10,
        .second_max_booted_security_version = 5,
        .max_key_generation = 0,
        .second_max_key_generation = 0
    };

    TEST_ASSERT_FALSE(bel_accept_security_version(10, &history));
    TEST_ASSERT_EQUAL_UINT16(10, history.max_booted_security_version);
    TEST_ASSERT_EQUAL_UINT16(5, history.second_max_booted_security_version);
}

void test_bel_key_generation_accept_updates_and_reports_change(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 10,
        .second_max_booted_security_version = 5,
        .max_key_generation = 0xFF,
        .second_max_key_generation = 0xFF
    };

    TEST_ASSERT_TRUE(bel_accept_key_generation(2, &history));
    TEST_ASSERT_EQUAL_UINT8(2, history.max_key_generation);
    TEST_ASSERT_EQUAL_UINT8(0xFF, history.second_max_key_generation);
}

void test_bel_key_generation_accept_same_value_returns_false(void)
{
    struct mbk_boot_history history = {
        .max_booted_security_version = 10,
        .second_max_booted_security_version = 5,
        .max_key_generation = 2,
        .second_max_key_generation = 1
    };

    TEST_ASSERT_FALSE(bel_accept_key_generation(2, &history));
    TEST_ASSERT_EQUAL_UINT8(2, history.max_key_generation);
    TEST_ASSERT_EQUAL_UINT8(1, history.second_max_key_generation);
}

/* ============================================================
 * HAL Flash Mock Tests
 * ============================================================ */

void test_hal_flash_read_write(void)
{
    uint8_t write_data[4] = {0x11, 0x22, 0x33, 0x44};

    TEST_ASSERT_EQUAL(0, hal_flash_init());
    TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(MBK_DATA_BASE));
    TEST_ASSERT_EQUAL(0, hal_flash_write(MBK_DATA_BASE, write_data, sizeof(write_data)));
    const uint8_t* read_view = (const uint8_t*)MBK_DATA_BASE;
    TEST_ASSERT_EQUAL_MEMORY(write_data, read_view, sizeof(write_data));
}

void test_hal_flash_erase(void)
{
    uint8_t data[16];

    hal_flash_erase_sector(MBK_DATA_BASE);
    const uint8_t* read_view = (const uint8_t*)MBK_DATA_BASE;
    TEST_ASSERT_NOT_NULL(read_view);
    memcpy(data, read_view, sizeof(data));

    for (size_t i = 0; i < sizeof(data); i++) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, data[i]);
    }
}

int main(void)
{
    UNITY_BEGIN();

    /* Security Version Tests */
    RUN_TEST(test_bel_security_version_first_boot_accepts_any);
    RUN_TEST(test_bel_security_version_second_boot_accepts_any);
    RUN_TEST(test_bel_security_version_monotonic_sequence);
    RUN_TEST(test_bel_security_version_mixed_sequence);
    RUN_TEST(test_bel_security_version_rejects_uninitialized);
    RUN_TEST(test_bel_security_version_boundary_values);

    /* Security Version Accept Tests */
    RUN_TEST(test_bel_security_version_accept_first);
    RUN_TEST(test_bel_security_version_accept_second_higher);
    RUN_TEST(test_bel_security_version_accept_second_lower);
    RUN_TEST(test_bel_security_version_accept_updates_second);
    RUN_TEST(test_bel_security_version_accept_same_value_returns_false);

    /* Key Generation Tests */
    RUN_TEST(test_bel_key_generation_first_boot_accepts_any);
    RUN_TEST(test_bel_key_generation_rejects_out_of_range);
    RUN_TEST(test_bel_key_generation_independent_from_security_version);
    RUN_TEST(test_bel_key_generation_accept_updates_and_reports_change);
    RUN_TEST(test_bel_key_generation_accept_same_value_returns_false);

    /* HAL Flash Mock Tests */
    RUN_TEST(test_hal_flash_read_write);
    RUN_TEST(test_hal_flash_erase);

    return UNITY_END();
}
