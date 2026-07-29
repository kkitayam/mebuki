/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "unity.h"
#include "mock_hal_flash.h"
#include "taneue.h"

#include <stdint.h>
#include <string.h>

#define TANEUE_SECTOR_COUNT (MBK_SLOT_SIZE / MBK_BLOCK_SIZE)
#define TANEUE_RESERVED_INDEX (TANEUE_SECTOR_COUNT - 1U)
#define TANEUE_PROGRESS_HEADER_SIZE (sizeof(uint16_t) * 2U)
#define TANEUE_MAX_PROGRESS_STEPS ((3U * (TANEUE_SECTOR_COUNT - 1U)) + 1U)

static_assert(TANEUE_MAX_PROGRESS_STEPS > 0U,
              "TANEUE_MAX_PROGRESS_STEPS must be greater than 0");
static_assert(TANEUE_PROGRESS_SIZE >= (TANEUE_PROGRESS_HEADER_SIZE + TANEUE_MAX_PROGRESS_STEPS),
              "TANEUE_PROGRESS_SIZE is too small for taneue progress layout");

enum taneue_result taneue_swap_phase_step1(size_t i);
enum taneue_result taneue_swap_phase_step2(size_t i);
enum taneue_result taneue_mark_step_done(size_t step_index);

static uintptr_t slot0_sector(size_t i)
{
    return MBK_SLOT0_BASE + ((uintptr_t)i * (uintptr_t)MBK_BLOCK_SIZE);
}

static uintptr_t slot1_sector(size_t i)
{
    return MBK_SLOT1_BASE + ((uintptr_t)i * (uintptr_t)MBK_BLOCK_SIZE);
}

static void fill_sector(uintptr_t addr, uint8_t value)
{
    uint8_t page[MBK_FLASH_PAGE_SIZE];

    memset(page, value, sizeof(page));
    TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(addr));

    for (size_t offset = 0; offset < MBK_BLOCK_SIZE; offset += MBK_FLASH_PAGE_SIZE) {
        TEST_ASSERT_EQUAL(0, hal_flash_write(addr + (uintptr_t)offset, page, sizeof(page)));
    }
}

static uint16_t progress_read_u16(size_t offset)
{
    uint16_t value = 0U;

    memcpy(&value, (const void*)(TANEUE_PROGRESS_BASE + (uintptr_t)offset), sizeof(value));
    return value;
}

static size_t total_steps_for_endpoint(size_t endpoint)
{
    return (3U * (endpoint + 1U)) + 1U;
}

static void seed_scheduled_progress(uint16_t endpoint, size_t completed_steps)
{
    const uint16_t endpoint_inv = (uint16_t)~endpoint;
    const uint8_t done = 0x00U;

    TEST_ASSERT_TRUE((size_t)endpoint < TANEUE_RESERVED_INDEX);
    TEST_ASSERT_TRUE(completed_steps <= (TANEUE_PROGRESS_SIZE - TANEUE_PROGRESS_HEADER_SIZE));
    TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(TANEUE_PROGRESS_BASE));
    TEST_ASSERT_EQUAL(0, hal_flash_write(TANEUE_PROGRESS_BASE, &endpoint, sizeof(endpoint)));
    TEST_ASSERT_EQUAL(0,
                      hal_flash_write(TANEUE_PROGRESS_BASE + sizeof(endpoint),
                                      &endpoint_inv,
                                      sizeof(endpoint_inv)));

    for (size_t i = 0; i < completed_steps; ++i) {
        TEST_ASSERT_EQUAL(0,
                          hal_flash_write(TANEUE_PROGRESS_BASE +
                                              (uintptr_t)TANEUE_PROGRESS_HEADER_SIZE +
                                              (uintptr_t)i,
                                          &done,
                                          sizeof(done)));
    }
}

static void setup_slots_for_endpoint(size_t endpoint)
{
    TEST_ASSERT_TRUE(endpoint < TANEUE_RESERVED_INDEX);

    for (size_t i = 0; i < TANEUE_SECTOR_COUNT; ++i) {
        if (i <= endpoint) {
            fill_sector(slot0_sector(i), (uint8_t)(0x10U + i));
            fill_sector(slot1_sector(i), (uint8_t)(0x80U + i));
        } else {
            TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(slot0_sector(i)));
            TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(slot1_sector(i)));
        }
    }
}

static void apply_e3_first_five_steps(void)
{
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_phase_step1(3U));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_mark_step_done(0U));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_phase_step2(3U));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_mark_step_done(1U));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_phase_step1(2U));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_mark_step_done(2U));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_phase_step2(2U));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_mark_step_done(3U));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_phase_step1(1U));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_mark_step_done(4U));
}

static void snapshot_slots(uint8_t out0[TANEUE_SECTOR_COUNT], uint8_t out1[TANEUE_SECTOR_COUNT])
{
    for (size_t i = 0; i < TANEUE_SECTOR_COUNT; ++i) {
        out0[i] = *(const uint8_t*)slot0_sector(i);
        out1[i] = *(const uint8_t*)slot1_sector(i);
    }
}

void setUp(void)
{
    TEST_ASSERT_TRUE(mock_flash_reset());
    TEST_ASSERT_EQUAL(0, hal_flash_init());
}

void tearDown(void)
{
}

void test_taneue_schedule_swap_writes_endpoint_header(void)
{
    setup_slots_for_endpoint(3U);

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());
    TEST_ASSERT_EQUAL_HEX16(3U, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16((uint16_t)~(uint16_t)3U, progress_read_u16(sizeof(uint16_t)));
    TEST_ASSERT_EQUAL_HEX8(0xFF,
                           *((const uint8_t*)(TANEUE_PROGRESS_BASE +
                                              (uintptr_t)TANEUE_PROGRESS_HEADER_SIZE)));
}

void test_taneue_schedule_swap_erases_dirty_progress_then_schedules(void)
{
    const uint8_t byte = 0x00U;

    setup_slots_for_endpoint(3U);

    TEST_ASSERT_EQUAL(0,
                      hal_flash_write(TANEUE_PROGRESS_BASE +
                                          (uintptr_t)TANEUE_PROGRESS_HEADER_SIZE +
                                          2U,
                                      &byte,
                                      sizeof(byte)));
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());

    TEST_ASSERT_EQUAL_HEX16(3U, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16((uint16_t)~(uint16_t)3U, progress_read_u16(sizeof(uint16_t)));
    TEST_ASSERT_EQUAL_HEX8(0xFF,
                           *((const uint8_t*)(TANEUE_PROGRESS_BASE +
                                              (uintptr_t)TANEUE_PROGRESS_HEADER_SIZE +
                                              2U)));
}

void test_taneue_schedule_swap_returns_precondition_when_no_endpoint(void)
{
    TEST_ASSERT_EQUAL(TANEUE_ERROR_PRECONDITION, taneue_schedule_swap());
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_schedule_swap_returns_precondition_when_reserved_sector_used(void)
{
    setup_slots_for_endpoint(3U);
    fill_sector(slot0_sector(TANEUE_RESERVED_INDEX), 0x33U);

    TEST_ASSERT_EQUAL(TANEUE_ERROR_PRECONDITION, taneue_schedule_swap());
}

void test_taneue_schedule_swap_erases_dirty_progress_before_precondition_error(void)
{
    const uint8_t done = 0x00U;

    TEST_ASSERT_EQUAL(0, hal_flash_write(TANEUE_PROGRESS_BASE + 7U, &done, sizeof(done)));
    TEST_ASSERT_EQUAL(TANEUE_ERROR_PRECONDITION, taneue_schedule_swap());

    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_swap_if_scheduled_noop_when_not_scheduled(void)
{
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());
}

void test_taneue_swap_if_scheduled_repairs_corrupt_header(void)
{
    const uint16_t endpoint = 3U;

    TEST_ASSERT_EQUAL(0, hal_flash_erase_sector(TANEUE_PROGRESS_BASE));
    TEST_ASSERT_EQUAL(0, hal_flash_write(TANEUE_PROGRESS_BASE, &endpoint, sizeof(endpoint)));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());

    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_swap_if_scheduled_repairs_non_monotonic_progress(void)
{
    const uint8_t done = 0x00U;

    seed_scheduled_progress(3U, 1U);
    TEST_ASSERT_EQUAL(0,
                      hal_flash_write(TANEUE_PROGRESS_BASE +
                                          (uintptr_t)TANEUE_PROGRESS_HEADER_SIZE +
                                          2U,
                                      &done,
                                      sizeof(done)));

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_swap_if_scheduled_returns_invalid_state_for_oversized_progress(void)
{
    seed_scheduled_progress(3U, total_steps_for_endpoint(3U) + 1U);

    TEST_ASSERT_EQUAL(TANEUE_ERROR_INVALID_STATE, taneue_swap_if_scheduled());
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_swap_if_scheduled_completes_swap_for_e3(void)
{
    uint8_t before0[TANEUE_SECTOR_COUNT];
    uint8_t before1[TANEUE_SECTOR_COUNT];

    setup_slots_for_endpoint(3U);
    snapshot_slots(before0, before1);

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());

    for (size_t i = 0; i <= 3U; ++i) {
        TEST_ASSERT_EQUAL_HEX8(before1[i], *(const uint8_t*)slot0_sector(i));
        TEST_ASSERT_EQUAL_HEX8(before0[i], *(const uint8_t*)slot1_sector(i));
    }

    for (size_t i = 4U; i < TANEUE_SECTOR_COUNT; ++i) {
        TEST_ASSERT_EQUAL_HEX8(0xFF, *(const uint8_t*)slot0_sector(i));
        TEST_ASSERT_EQUAL_HEX8(0xFF, *(const uint8_t*)slot1_sector(i));
    }

    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(0U));
    TEST_ASSERT_EQUAL_HEX16(UINT16_MAX, progress_read_u16(sizeof(uint16_t)));
}

void test_taneue_swap_if_scheduled_resumes_from_mid_progress_prefix(void)
{
    uint8_t before0[TANEUE_SECTOR_COUNT];
    uint8_t before1[TANEUE_SECTOR_COUNT];

    setup_slots_for_endpoint(3U);
    snapshot_slots(before0, before1);

    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());
    apply_e3_first_five_steps();
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());

    for (size_t i = 0; i <= 3U; ++i) {
        TEST_ASSERT_EQUAL_HEX8(before1[i], *(const uint8_t*)slot0_sector(i));
        TEST_ASSERT_EQUAL_HEX8(before0[i], *(const uint8_t*)slot1_sector(i));
    }
}

void test_taneue_swap_if_scheduled_returns_flash_error_on_progress_write_failure(void)
{
    setup_slots_for_endpoint(3U);
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());

    mock_flash_inject_error(true);
    TEST_ASSERT_EQUAL(TANEUE_ERROR_FLASH, taneue_swap_if_scheduled());
}

/*
 * Under TANEUE_WEAR_OPT_PRIMARY the alignment and finalize phases only
 * erase sectors in the Secondary Slot (SLOT_A = Slot1).  The Primary Slot
 * (SLOT_B = Slot0) receives exactly one erase per swapped sector from
 * swap step2 and nothing more.
 *
 * Under TANEUE_WEAR_OPT_BALANCED the roles are reversed: alignment and
 * finalize erases fall on the Primary Slot (SLOT_A = Slot0), and the
 * Secondary Slot (SLOT_B = Slot1) receives only the step2 erases.
 *
 * Both tests use endpoint=3.  Step2 runs for sectors 0..3, producing
 * exactly 4 erases on SLOT_B and none on SLOT_B from any other phase.
 */
#if TANEUE_WEAR_OPT == TANEUE_WEAR_OPT_PRIMARY
void test_taneue_wear_primary_alignment_erases_secondary(void)
{
    const size_t endpoint = 3U;

    setup_slots_for_endpoint(endpoint);
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());

    mock_flash_reset_stats();
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());

    /* Slot0 (SLOT_B) receives only step2 erases: one per sector in [0..endpoint]. */
    TEST_ASSERT_EQUAL(endpoint + 1U,
                      mock_flash_count_erases_in_range(MBK_SLOT0_BASE, MBK_SLOT_SIZE));
}
#endif

#if TANEUE_WEAR_OPT == TANEUE_WEAR_OPT_BALANCED
void test_taneue_wear_balanced_alignment_erases_primary(void)
{
    const size_t endpoint = 3U;

    setup_slots_for_endpoint(endpoint);
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_schedule_swap());

    mock_flash_reset_stats();
    TEST_ASSERT_EQUAL(TANEUE_SUCCESS, taneue_swap_if_scheduled());

    /* Slot1 (SLOT_B) receives only step2 erases: one per sector in [0..endpoint]. */
    TEST_ASSERT_EQUAL(endpoint + 1U,
                      mock_flash_count_erases_in_range(MBK_SLOT1_BASE, MBK_SLOT_SIZE));
}
#endif

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_taneue_schedule_swap_writes_endpoint_header);
    RUN_TEST(test_taneue_schedule_swap_erases_dirty_progress_then_schedules);
    RUN_TEST(test_taneue_schedule_swap_returns_precondition_when_no_endpoint);
    RUN_TEST(test_taneue_schedule_swap_returns_precondition_when_reserved_sector_used);
    RUN_TEST(test_taneue_schedule_swap_erases_dirty_progress_before_precondition_error);
    RUN_TEST(test_taneue_swap_if_scheduled_noop_when_not_scheduled);
    RUN_TEST(test_taneue_swap_if_scheduled_repairs_corrupt_header);
    RUN_TEST(test_taneue_swap_if_scheduled_repairs_non_monotonic_progress);
    RUN_TEST(test_taneue_swap_if_scheduled_returns_invalid_state_for_oversized_progress);
    RUN_TEST(test_taneue_swap_if_scheduled_completes_swap_for_e3);
    RUN_TEST(test_taneue_swap_if_scheduled_resumes_from_mid_progress_prefix);
    RUN_TEST(test_taneue_swap_if_scheduled_returns_flash_error_on_progress_write_failure);

#if TANEUE_WEAR_OPT == TANEUE_WEAR_OPT_PRIMARY
    RUN_TEST(test_taneue_wear_primary_alignment_erases_secondary);
#endif
#if TANEUE_WEAR_OPT == TANEUE_WEAR_OPT_BALANCED
    RUN_TEST(test_taneue_wear_balanced_alignment_erases_primary);
#endif

    return UNITY_END();
}
