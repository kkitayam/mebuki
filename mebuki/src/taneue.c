/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <taneue.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Expose internal functions for unit testing */
#ifndef STATIC
#  ifdef UNIT_TEST
#    define STATIC
#  else
#    define STATIC static
#  endif
#endif

/* Device-specific Flash HAL. */
extern int hal_flash_write(uintptr_t address, const void* data, size_t size);
extern int hal_flash_erase_sector(uintptr_t address);

enum taneue_progress_state {
    TANEUE_PROGRESS_CLEAN = 0,
    TANEUE_PROGRESS_SCHEDULED,
    TANEUE_PROGRESS_CORRUPT,
};

struct taneue_progress_scan {
    enum taneue_progress_state state;
    uint32_t endpoint;
    uint32_t completed_steps;
};

/*
 * Default progress area.
 *
 * See mebuki_config.h for the required alignment and placement
 * constraints when overriding this definition.
 */
#ifndef TANEUE_PROGRESS_BASE
#  define TANEUE_PROGRESS_BASE \
      (((MBK_DATA0_BASE > MBK_DATA1_BASE) ? MBK_DATA0_BASE : MBK_DATA1_BASE) + MBK_BLOCK_SIZE)
#endif

#ifndef TANEUE_PROGRESS_SIZE
#  define TANEUE_PROGRESS_SIZE MBK_BLOCK_SIZE
#endif

#define TANEUE_SECTOR_COUNT (MBK_SLOT_SIZE / MBK_BLOCK_SIZE)
#define TANEUE_PROGRESS_HEADER_SIZE (sizeof(uint16_t) * 2U)
#define TANEUE_PROGRESS_STEPS_OFFSET TANEUE_PROGRESS_HEADER_SIZE
#define TANEUE_PROGRESS_STEP_COUNT_MAX ((3U * (TANEUE_SECTOR_COUNT - 1U)) + 1U)

/* Enforce the address and size requirements defined by mebuki_config.h. */
static_assert(TANEUE_SECTOR_COUNT >= 5U,
              "taneue requires at least 5 sectors per slot");
static_assert((TANEUE_SECTOR_COUNT - 1U) <= UINT16_MAX,
              "taneue endpoint must fit in uint16_t");
static_assert((TANEUE_PROGRESS_SIZE % MBK_BLOCK_SIZE) == 0U,
              "TANEUE_PROGRESS_SIZE must be a multiple of MBK_BLOCK_SIZE");
static_assert(TANEUE_PROGRESS_SIZE >=
                  (TANEUE_PROGRESS_STEPS_OFFSET + TANEUE_PROGRESS_STEP_COUNT_MAX),
              "TANEUE_PROGRESS_SIZE must fit taneue progress header and steps");
static_assert((TANEUE_PROGRESS_BASE % MBK_BLOCK_SIZE) == 0U,
              "TANEUE_PROGRESS_BASE must be MBK_BLOCK_SIZE (sector) aligned");
static_assert((TANEUE_PROGRESS_BASE % 16U) == 0U,
              "TANEUE_PROGRESS_BASE must be at least 16-byte aligned");

static_assert((TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE) <= MBK_SLOT0_BASE ||
              TANEUE_PROGRESS_BASE >= (MBK_SLOT0_BASE + MBK_SLOT_SIZE),
              "TANEUE_PROGRESS region must not overlap Slot0");
static_assert((TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE) <= MBK_SLOT1_BASE ||
              TANEUE_PROGRESS_BASE >= (MBK_SLOT1_BASE + MBK_SLOT_SIZE),
              "TANEUE_PROGRESS region must not overlap Slot1");

STATIC bool taneue_sector_is_erased(uintptr_t address)
{
    const uint32_t* p = (const uint32_t*)address;
    const uint32_t* const end = p + (MBK_BLOCK_SIZE / sizeof(*p));

    while (p < end) {
        if (*p++ != 0xFFFFFFFFU) {
            return false;
        }
    }

    return true;
}

STATIC int taneue_erase_progress_area(void)
{
    uintptr_t address = TANEUE_PROGRESS_BASE;
    const uintptr_t end = TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE;

    while (address < end) {
        if (hal_flash_erase_sector(address) != 0) {
            return TANEUE_ERROR_FLASH;
        }
        address += MBK_BLOCK_SIZE;
    }

    return TANEUE_SUCCESS;
}

STATIC int taneue_progress_write_u16(uintptr_t offset, uint16_t value)
{
    if (hal_flash_write(TANEUE_PROGRESS_BASE + offset, &value, sizeof(value)) != 0) {
        return TANEUE_ERROR_FLASH;
    }

    return TANEUE_SUCCESS;
}

STATIC struct taneue_progress_scan taneue_scan_progress(void)
{
    struct taneue_progress_scan scan = {TANEUE_PROGRESS_CLEAN, UINT32_MAX, 0U};
    /* The progress area is at least 16-byte aligned. The header
     * consists of two uint16_t values at the beginning of the region. */
    const uint16_t* const hdr = (const uint16_t*)TANEUE_PROGRESS_BASE;
    const uint16_t ep = hdr[0];
    const uint16_t ep_inv = hdr[1];
    const uint8_t* p = (const uint8_t*)(TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_STEPS_OFFSET);
    const uint8_t* const end = (const uint8_t*)(TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_SIZE);
    const bool header_is_clean = (ep == UINT16_MAX) && (ep_inv == UINT16_MAX);
    const bool header_is_valid = (ep != UINT16_MAX) &&
                                 ((uint32_t)ep < (TANEUE_SECTOR_COUNT - 1U)) &&
                                 (ep_inv == (uint16_t)~ep);
    bool seen_ff = false;

    if (!header_is_clean && !header_is_valid) {
        scan.state = TANEUE_PROGRESS_CORRUPT;
        return scan;
    }

    while (p < end) {
        const uint8_t value = *p++;

        if (value == 0x00U) {
            if (seen_ff) {
                scan.state = TANEUE_PROGRESS_CORRUPT;
                return scan;
            }
            scan.completed_steps++;
            continue;
        }

        if (value == 0xFFU) {
            seen_ff = true;
            continue;
        }

        scan.state = TANEUE_PROGRESS_CORRUPT;
        return scan;
    }

    if (header_is_clean) {
        if (scan.completed_steps != 0U) {
            scan.state = TANEUE_PROGRESS_CORRUPT;
        }
        return scan;
    }

    scan.state = TANEUE_PROGRESS_SCHEDULED;
    scan.endpoint = (uint32_t)ep;
    return scan;
}

STATIC int taneue_mark_step_done(uint32_t step_index)
{
    /* Flash write source must reside in RAM (no static storage). */
    const uint8_t done = 0x00U;

    if (step_index >= TANEUE_PROGRESS_STEP_COUNT_MAX) {
        return TANEUE_ERROR_INVALID_STATE;
    }

    if (hal_flash_write(TANEUE_PROGRESS_BASE + TANEUE_PROGRESS_STEPS_OFFSET + step_index,
                        &done,
                        sizeof(done)) != 0) {
        return TANEUE_ERROR_FLASH;
    }

    return TANEUE_SUCCESS;
}

STATIC int taneue_ensure_erased(uintptr_t address)
{
    if (taneue_sector_is_erased(address)) {
        return TANEUE_SUCCESS;
    }

    if (hal_flash_erase_sector(address) != 0) {
        return TANEUE_ERROR_FLASH;
    }

    return TANEUE_SUCCESS;
}

STATIC int taneue_copy_sector(uintptr_t dst, uintptr_t src)
{
    const uint8_t* src_bytes = (const uint8_t*)src;
    const uint8_t* const end = src_bytes + MBK_BLOCK_SIZE;
    uint8_t page[MBK_FLASH_PAGE_SIZE];
    uintptr_t dst_address = dst;

    while (src_bytes < end) {
        memcpy(page, src_bytes, MBK_FLASH_PAGE_SIZE);
        if (hal_flash_write(dst_address, page, MBK_FLASH_PAGE_SIZE) != 0) {
            return TANEUE_ERROR_FLASH;
        }
        src_bytes += MBK_FLASH_PAGE_SIZE;
        dst_address += MBK_FLASH_PAGE_SIZE;
    }

    return TANEUE_SUCCESS;
}

STATIC uint32_t taneue_detect_endpoint(void)
{
    uintptr_t slot0_address = MBK_SLOT0_BASE + TANEUE_SECTOR_COUNT * MBK_BLOCK_SIZE;
    uintptr_t slot1_address = MBK_SLOT1_BASE + TANEUE_SECTOR_COUNT * MBK_BLOCK_SIZE;

    for (uint32_t i = TANEUE_SECTOR_COUNT; i > 0U; --i) {
        slot0_address -= MBK_BLOCK_SIZE;
        slot1_address -= MBK_BLOCK_SIZE;
        if (!taneue_sector_is_erased(slot0_address) ||
            !taneue_sector_is_erased(slot1_address)) {
            return i - 1U;
        }
    }

    return UINT32_MAX;
}

STATIC int taneue_find_schedule_endpoint(uint32_t* endpoint_out)
{
    const uint32_t reserved_index = TANEUE_SECTOR_COUNT - 1U;
    const uint32_t endpoint = taneue_detect_endpoint();
    const uintptr_t slot0_reserved = MBK_SLOT0_BASE + reserved_index * MBK_BLOCK_SIZE;
    const uintptr_t slot1_reserved = MBK_SLOT1_BASE + reserved_index * MBK_BLOCK_SIZE;

    if (!taneue_sector_is_erased(slot0_reserved) ||
        !taneue_sector_is_erased(slot1_reserved)) {
        return TANEUE_ERROR_PRECONDITION;
    }

    if (endpoint == UINT32_MAX || endpoint >= reserved_index) {
        return TANEUE_ERROR_PRECONDITION;
    }

    *endpoint_out = endpoint;
    return TANEUE_SUCCESS;
}

STATIC int taneue_swap_phase_step1(uint32_t i)
{
    const uintptr_t dst_slot1 = MBK_SLOT1_BASE + (i + 1U) * MBK_BLOCK_SIZE;
    int err;

    err = taneue_ensure_erased(dst_slot1);
    if (err) { return err; }

    return taneue_copy_sector(dst_slot1, MBK_SLOT0_BASE + i * MBK_BLOCK_SIZE);
}

STATIC int taneue_swap_phase_step2(uint32_t i)
{
    const uintptr_t dst_slot0 = MBK_SLOT0_BASE + i * MBK_BLOCK_SIZE;
    const uintptr_t src_slot1 = MBK_SLOT1_BASE + i * MBK_BLOCK_SIZE;
    int err;

    if (memcmp((const void*)dst_slot0, (const void*)src_slot1, MBK_BLOCK_SIZE) == 0) {
        return TANEUE_SUCCESS;
    }

    err = taneue_ensure_erased(dst_slot0);
    if (err) { return err; }

    return taneue_copy_sector(dst_slot0, src_slot1);
}

STATIC int taneue_align_phase_step(uint32_t i)
{
    const uintptr_t dst_slot1 = MBK_SLOT1_BASE + i * MBK_BLOCK_SIZE;
    const uintptr_t src_slot1 = MBK_SLOT1_BASE + (i + 1U) * MBK_BLOCK_SIZE;
    int err;

    err = taneue_ensure_erased(dst_slot1);
    if (err) { return err; }

    return taneue_copy_sector(dst_slot1, src_slot1);
}

STATIC int taneue_finalize_phase(uint32_t endpoint)
{
    return taneue_ensure_erased(MBK_SLOT1_BASE + (endpoint + 1U) * MBK_BLOCK_SIZE);
}

STATIC int taneue_execute_from_step(uint32_t endpoint, uint32_t completed_steps)
{
    const uint32_t total_steps = (3U * (endpoint + 1U)) + 1U;
    int err;
    uint32_t step = 0U;

    for (uint32_t i = endpoint + 1U; i-- > 0U;) {
        if (step >= completed_steps) {
            err = taneue_swap_phase_step1(i);
            if (err) { return err; }
            err = taneue_mark_step_done(step);
            if (err) { return err; }
        }
        step++;

        if (step >= completed_steps) {
            err = taneue_swap_phase_step2(i);
            if (err) { return err; }
            err = taneue_mark_step_done(step);
            if (err) { return err; }
        }
        step++;
    }

    for (uint32_t i = 0; i <= endpoint; ++i) {
        if (step >= completed_steps) {
            err = taneue_align_phase_step(i);
            if (err) { return err; }
            err = taneue_mark_step_done(step);
            if (err) { return err; }
        }
        step++;
    }

    if (step >= completed_steps) {
        err = taneue_finalize_phase(endpoint);
        if (err) { return err; }
        err = taneue_mark_step_done(step);
        if (err) { return err; }
    }
    step++;

    if (step != total_steps) {
        return TANEUE_ERROR_INVALID_STATE;
    }

    return TANEUE_SUCCESS;
}

int taneue_schedule_swap(void)
{
    const struct taneue_progress_scan scan = taneue_scan_progress();
    int err;
    uint32_t endpoint = UINT32_MAX;

    if (scan.state != TANEUE_PROGRESS_CLEAN) {
        err = taneue_erase_progress_area();
        if (err) { return err; }
    }

    err = taneue_find_schedule_endpoint(&endpoint);
    if (err) { return err; }

    err = taneue_progress_write_u16(0U, (uint16_t)endpoint);
    if (err) { return err; }

    err = taneue_progress_write_u16(sizeof(uint16_t), (uint16_t)~(uint16_t)endpoint);
    if (err) { return err; }

    return TANEUE_SUCCESS;
}

int taneue_swap_if_scheduled(void)
{
    const struct taneue_progress_scan scan = taneue_scan_progress();
    int err;

    if (scan.state == TANEUE_PROGRESS_CORRUPT) {
        return taneue_erase_progress_area();
    }

    if (scan.state == TANEUE_PROGRESS_CLEAN) {
        return TANEUE_SUCCESS;
    }

    {
        const uint32_t total_steps = (3U * (scan.endpoint + 1U)) + 1U;

        if (scan.completed_steps > total_steps) {
            err = taneue_erase_progress_area();
            if (err) { return err; }
            return TANEUE_ERROR_INVALID_STATE;
        }

        if (scan.completed_steps < total_steps) {
            err = taneue_execute_from_step(scan.endpoint, scan.completed_steps);
            if (err) { return err; }
        }
    }

    return taneue_erase_progress_area();
}
