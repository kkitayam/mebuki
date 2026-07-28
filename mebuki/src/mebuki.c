/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <mebuki.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
/* C23 provides static_assert as a keyword. */
#if __STDC_VERSION__ < 202311L
#  include <assert.h>
#endif

/* Expose internal functions when building unit tests. */
#ifndef STATIC
#  ifdef UNIT_TEST
#    define STATIC
#  else
#    define STATIC static
#  endif
#endif

#if defined(__has_c_attribute)
#  if __has_c_attribute(fallthrough)
#    define FALLTHROUGH [[fallthrough]]
#  endif
#endif

#ifndef FALLTHROUGH
#  if defined(__GNUC__)
#    define FALLTHROUGH __attribute__((fallthrough))
#  else
#    define FALLTHROUGH ((void)0)
#  endif
#endif


// === Constants and Type Definitions ===

/* Maximum firmware image size in bytes. */
#ifndef MBK_SOFTWARE_SIZE_MAX
#  define MBK_SOFTWARE_SIZE_MAX  (MBK_SLOT_SIZE - MBK_HEADER_SIZE - MBK_SIGNATURE_SIZE)
#endif

/* BFL constants */
#define MBK_BFL_NUM_BLOCKS 2U
#define MBK_BFL_INVALID UINT32_MAX

/* Maximum supported security version. */
#define MBK_SECURITY_VERSION_MAX            0xFFFEU

/* Uninitialized security version. */
#define MBK_SECURITY_VERSION_UNINITIALIZED  0xFFFFU

/* Uninitialized key generation. */
#define MBK_KEY_GENERATION_UNINITIALIZED    0xFFU

/* Valid firmware image. */
#define MBK_INVALIDATION_FLAG_VALID         0xFFU

#define MBK_NUM_SLOTS 2U

/* === Config Validation === */

/* Validate configuration limits. */
static_assert(MBK_NUM_KEY_GENERATIONS >= 1 && MBK_NUM_KEY_GENERATIONS <= 255,
               "MBK_NUM_KEY_GENERATIONS must be between 1 and 255");

static_assert(MBK_SLOT_SIZE > MBK_HEADER_SIZE + MBK_SIGNATURE_SIZE,
               "MBK_SLOT_SIZE must be larger than MBK_HEADER_SIZE + MBK_SIGNATURE_SIZE");

/*
 * Enforce the address and size requirements defined by
 * mebuki_config.h.
 *
 * Block alignment guarantees the minimum alignment required by the
 * implementation for direct structure accesses.
 */
static_assert((MBK_BLOCK_SIZE % 16U) == 0U,
              "MBK_BLOCK_SIZE must be a multiple of 16 (min 16-byte alignment)");
static_assert((MBK_BLOCK_SIZE % MBK_FLASH_PAGE_SIZE) == 0U,
              "MBK_BLOCK_SIZE must be a multiple of MBK_FLASH_PAGE_SIZE");
static_assert((MBK_DATA0_BASE % MBK_BLOCK_SIZE) == 0U,
              "MBK_DATA0_BASE must be MBK_BLOCK_SIZE aligned");
static_assert((MBK_DATA1_BASE % MBK_BLOCK_SIZE) == 0U,
              "MBK_DATA1_BASE must be MBK_BLOCK_SIZE aligned");
static_assert((MBK_SLOT0_BASE % MBK_BLOCK_SIZE) == 0U,
              "MBK_SLOT0_BASE must be MBK_BLOCK_SIZE aligned");
static_assert((MBK_SLOT1_BASE % MBK_BLOCK_SIZE) == 0U,
              "MBK_SLOT1_BASE must be MBK_BLOCK_SIZE aligned");
static_assert((MBK_SLOT_SIZE % MBK_BLOCK_SIZE) == 0U,
              "MBK_SLOT_SIZE must be multiple of MBK_BLOCK_SIZE");
static_assert((MBK_DATA0_BASE % 16U) == 0U,
              "MBK_DATA0_BASE must be at least 16-byte aligned");
static_assert((MBK_DATA1_BASE % 16U) == 0U,
              "MBK_DATA1_BASE must be at least 16-byte aligned");
static_assert((MBK_SLOT0_BASE % 16U) == 0U,
              "MBK_SLOT0_BASE must be at least 16-byte aligned");
static_assert((MBK_SLOT1_BASE % 16U) == 0U,
              "MBK_SLOT1_BASE must be at least 16-byte aligned");
static_assert(MBK_DATA0_BASE != MBK_DATA1_BASE,
              "MBK_DATA0_BASE and MBK_DATA1_BASE must be different sectors");

/* --- BEL Types --- */

/* Boot history used for rollback protection. */
struct mbk_boot_history {
    uint16_t max_booted_security_version;
    uint16_t second_max_booted_security_version;
    uint8_t  max_key_generation;
    uint8_t  second_max_key_generation;
    uint16_t reserved;
};

/* Hash of the previously booted image. */
typedef uint8_t mbk_image_hash[MBK_HASH_SIZE];

/* BFL record stored in flash. */
struct mbk_boot_record {
    struct mbk_boot_history history;
    mbk_image_hash last_booted_image_hash;
};

/* --- BFL Types --- */

struct mbk_bfl_entry {
    uint32_t remaining_stores;     /* Remaining write cycles. Decrements on each successful update. */
    struct mbk_boot_record record;
    uint32_t integrity;            /* CRC32 of remaining_stores and record. */
};

struct mbk_bfl_sector {
    struct mbk_bfl_entry entry;                                      /* Active record. */
    uint8_t reserved[MBK_BLOCK_SIZE - sizeof(struct mbk_bfl_entry)]; /* Pad to one erase block. */
};

enum mbk_bfl_result {
    MBK_BFL_SUCCESS = 0,
    MBK_BFL_ERROR_NO_REMAINING_STORES,
    MBK_BFL_ERROR_INTEGRITY_MISMATCH,
    MBK_BFL_ERROR_ERASE_FAILED,
    MBK_BFL_ERROR_WRITE_FAILED
};

// --- BSL Local Result Codes ---

#define MBK_BSL_SUCCESS                              0
#define MBK_BSL_SUCCESS_HASH_MATCHED                 0  /* It matched the hash of the image from the previous boot */
#define MBK_BSL_SUCCESS_VERIFIED                     1  /* The slot was successfully verified and deemed bootable */
#define MBK_BSL_ERROR_INIT_FAILED                   -1
#define MBK_BSL_ERROR_INVALID_HEADER                -2
#define MBK_BSL_ERROR_SECURITY_VERSION_NOT_ELIGIBLE -3
#define MBK_BSL_ERROR_KEY_GENERATION_NOT_ELIGIBLE   -4
#define MBK_BSL_ERROR_HASH_COMPUTATION_FAILED       -5
#define MBK_BSL_ERROR_SIGNATURE_VERIFICATION_FAILED -6

struct mbk_bsl_boot_info {
    int slot_id;
    const struct mbk_header* header;
    uint32_t entry_point;
};

struct mbk_record_update {
    uint16_t security_version;
    uint8_t key_generation;
    mbk_image_hash computed_hash;
};

/* Size validation */
static_assert(sizeof(struct mbk_boot_history) == 8, "struct mbk_boot_history must be 8 bytes");
static_assert(sizeof(mbk_image_hash) == MBK_HASH_SIZE, "mbk_image_hash must be 32 bytes");
static_assert(sizeof(struct mbk_header) == MBK_HEADER_SIZE, "struct mbk_header must be 8 bytes");
static_assert(sizeof(struct mbk_bfl_entry) <= MBK_BLOCK_SIZE,
               "struct mbk_bfl_entry must fit in one flash sector");
static_assert(sizeof(struct mbk_context) >= sizeof(struct mbk_bfl_entry),
               "struct mbk_context must be large enough to hold struct mbk_bfl_entry");

/* Device-specific Flash HAL. */
extern int hal_flash_write(uintptr_t address, const void* data, size_t size);
extern int hal_flash_erase_sector(uintptr_t address);

/* Signature verification layer (mebuki_svl.h or external implementation) */
extern int mbk_svl_init(void);
extern int mbk_svl_verify_signature(const void* data, size_t data_len,
                                    const void* signature,
                                    uint8_t key_generation);
extern int mbk_svl_compute_hash(const void* data, size_t data_len,
                                uint8_t digest_out[MBK_HASH_SIZE]);
extern bool mbk_svl_compare_hash(const uint8_t digest1[MBK_HASH_SIZE],
                                  const uint8_t digest2[MBK_HASH_SIZE]);

// === Function Implementations ===

// === BEL Functions ===

/*
 * Eligibility rule shared by security_version and key_generation:
 * - Accept the first observed value. (M is uninitialized)
 * - Accept the second observed value. (S is uninitialized)
 * - Thereafter, accept values greater than or equal to the
 *   smaller of the two highest accepted values. (v >= min(M, S))
 */
STATIC bool bel_is_security_version_eligible(uint16_t security_version,
                                        const uint16_t max_booted_security_version,
                                        const uint16_t second_max_booted_security_version)
{
    if (security_version > MBK_SECURITY_VERSION_MAX) return false;

    const uint16_t M = max_booted_security_version;
    const uint16_t S = second_max_booted_security_version;

    if (M == MBK_SECURITY_VERSION_UNINITIALIZED) return true;
    if (S == MBK_SECURITY_VERSION_UNINITIALIZED) return true;
    const uint16_t T = (M < S) ? M : S;
    return security_version >= T;
}

STATIC bool bel_is_key_generation_eligible(uint8_t key_gen,
                                        const uint8_t max_key_generation,
                                        const uint8_t second_max_key_generation)
{
    if (key_gen >= MBK_NUM_KEY_GENERATIONS) return false;

    const uint8_t M = max_key_generation;
    const uint8_t S = second_max_key_generation;

    if (M == MBK_KEY_GENERATION_UNINITIALIZED) return true;
    if (S == MBK_KEY_GENERATION_UNINITIALIZED) return true;
    const uint8_t T = (M < S) ? M : S;
    return key_gen >= T;
}

/*
 * Update the two highest accepted values:
 *   v > M -> (M', S') = (v, M)
 *   M > v > S -> S' = v
 *   otherwise -> unchanged
 */
STATIC bool bel_accept_security_version(uint16_t security_version,
                                        struct mbk_boot_history* history)
{
    const uint16_t M = history->max_booted_security_version;
    const uint16_t S = history->second_max_booted_security_version;

    if (M == MBK_SECURITY_VERSION_UNINITIALIZED) {
        history->max_booted_security_version = security_version;
        return true;
    }

    if (S == MBK_SECURITY_VERSION_UNINITIALIZED) {
        if (security_version > M) {
            history->second_max_booted_security_version = M;
            history->max_booted_security_version = security_version;
            return true;
        } else if (security_version < M) {
            history->second_max_booted_security_version = security_version;
            return true;
        }
        return false;
    }

    if (security_version > M) {
        history->second_max_booted_security_version = M;
        history->max_booted_security_version = security_version;
        return true;
    } else if (security_version > S && security_version < M) {
        history->second_max_booted_security_version = security_version;
        return true;
    }
    return false;
}

STATIC bool bel_accept_key_generation(uint8_t key_gen,
                                      struct mbk_boot_history* history)
{
    const uint8_t M = history->max_key_generation;
    const uint8_t S = history->second_max_key_generation;

    if (M == MBK_KEY_GENERATION_UNINITIALIZED) {
        history->max_key_generation = key_gen;
        return true;
    }

    if (S == MBK_KEY_GENERATION_UNINITIALIZED) {
        if (key_gen > M) {
            history->second_max_key_generation = M;
            history->max_key_generation = key_gen;
            return true;
        } else if (key_gen < M) {
            history->second_max_key_generation = key_gen;
            return true;
        }
        return false;
    }

    if (key_gen > M) {
        history->second_max_key_generation = M;
        history->max_key_generation = key_gen;
        return true;
    } else if (key_gen > S && key_gen < M) {
        history->second_max_key_generation = key_gen;
        return true;
    }
    return false;
}

// === SML Functions ===

STATIC bool sml_prefer_slot1(const struct mbk_header* slot0, const struct mbk_header* slot1)
{
    const uint16_t sv0 = slot0->security_version;
    const uint16_t sv1 = slot1->security_version;
    return (sv1 != MBK_SECURITY_VERSION_UNINITIALIZED) &&
           ((sv0 == MBK_SECURITY_VERSION_UNINITIALIZED) || (sv1 > sv0));
}

STATIC bool sml_is_slot_header_valid(const struct mbk_header* header)
{
    /* BEL validates security_version and key_generation, so SML only checks other fields */
    if (header->invalidation_flag != MBK_INVALIDATION_FLAG_VALID) return false;
    if (header->software_size == 0) return false;
    if (header->software_size > MBK_SOFTWARE_SIZE_MAX) return false;
    return true;
}

// === BFL Functions ===

STATIC uint32_t bfl_crc32_update(uint32_t crc, const void* data, size_t size)
{
    const uint8_t* p = (const uint8_t*)data;
    const uint8_t* const end = p + size;

    while (p < end) {
        crc ^= *p++;
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

STATIC uint32_t bfl_compute_record_integrity(const struct mbk_bfl_entry* entry)
{
    uint32_t crc = 0xFFFFFFFFU;
    crc = bfl_crc32_update(crc, entry, offsetof(struct mbk_bfl_entry, integrity));
    return crc ^ 0xFFFFFFFFU;
}

STATIC void bfl_load_entry(struct mbk_bfl_entry* out)
{
    const struct mbk_bfl_sector* sec0 = (const struct mbk_bfl_sector*)MBK_DATA0_BASE;
    const struct mbk_bfl_sector* sec1 = (const struct mbk_bfl_sector*)MBK_DATA1_BASE;
    const struct mbk_bfl_sector* secs[MBK_BFL_NUM_BLOCKS] = {sec0, sec1};
    const uint32_t rem0 = sec0->entry.remaining_stores;
    const uint32_t rem1 = sec1->entry.remaining_stores;
    /* Prefer the entry with the smaller remaining_stores value. */
    bool prefer_sec1 = rem1 != MBK_BFL_INVALID && (rem0 == MBK_BFL_INVALID || rem1 < rem0);

    int n = prefer_sec1 ? 1 : 0;
    for (unsigned int i = 0; i < MBK_BFL_NUM_BLOCKS; ++i, n ^= 1) {
        *out = secs[n]->entry;
        if (out->integrity == bfl_compute_record_integrity(out)) {
            return;
        }
    }
    memset(out, 0xFF, sizeof(*out));
}

STATIC int bfl_store_entry(struct mbk_bfl_entry* inout)
{
    const uint32_t rem = inout->remaining_stores;
    const struct mbk_bfl_sector* sec0 = (const struct mbk_bfl_sector*)MBK_DATA0_BASE;
    const struct mbk_bfl_sector* sec1 = (const struct mbk_bfl_sector*)MBK_DATA1_BASE;
    int err;

    if (0 == rem) {
        MBK_LOG("BFL store failed: no remaining stores\n");
        return MBK_BFL_ERROR_NO_REMAINING_STORES;
    }

    /* Write to the alternate configured sector (ping-pong update). */
    uintptr_t target_addr;
    const uint32_t c = inout->integrity;
    if (c == sec0->entry.integrity) {
        target_addr = MBK_DATA1_BASE;
    } else if (c == sec1->entry.integrity) {
        target_addr = MBK_DATA0_BASE;
    } else {
        MBK_LOG("BFL store failed: integrity mismatch\n");
        return MBK_BFL_ERROR_INTEGRITY_MISMATCH;
    }

    /* Keep caller side RAM state until flash operation succeeds */
    struct mbk_bfl_entry next = *inout;
    next.remaining_stores = rem - 1U;
    next.integrity = bfl_compute_record_integrity(&next);

    err = hal_flash_erase_sector(target_addr);
    if (err != 0) {
        MBK_LOG("BFL store failed: erase sector failed\n");
        return MBK_BFL_ERROR_ERASE_FAILED;
    }
    err = hal_flash_write(target_addr, &next, sizeof(next));
    if (err != 0) {
        MBK_LOG("BFL store failed: write failed\n");
        return MBK_BFL_ERROR_WRITE_FAILED;
    }

    *inout = next;
    return MBK_BFL_SUCCESS;
}

// === BSL Functions ===

STATIC int bsl_validate_slot(const struct mbk_boot_record* record,
                             const void* slot_adr,
                             struct mbk_record_update* update)
{
    const struct mbk_header* hdr = (const struct mbk_header*)slot_adr;
    int err;

    if (!sml_is_slot_header_valid(hdr)) {
        MBK_LOG("Slot header invalid\n");
        return MBK_BSL_ERROR_INVALID_HEADER;
    }

    const uint16_t sv = hdr->security_version;
    update->security_version = sv;
    const struct mbk_boot_history* h = &record->history;
    if (!bel_is_security_version_eligible(sv, h->max_booted_security_version, h->second_max_booted_security_version)) {
        MBK_LOG("Security version not eligible\n");
        return MBK_BSL_ERROR_SECURITY_VERSION_NOT_ELIGIBLE;
    }

    const uint8_t key_gen = hdr->key_generation;
    update->key_generation = key_gen;
    if (!bel_is_key_generation_eligible(key_gen, h->max_key_generation, h->second_max_key_generation)) {
        MBK_LOG("Key generation not eligible\n");
        return MBK_BSL_ERROR_KEY_GENERATION_NOT_ELIGIBLE;
    }

    const size_t sig_sz = (size_t)MBK_HEADER_SIZE + (size_t)hdr->software_size;
    err = mbk_svl_compute_hash(slot_adr, sig_sz, update->computed_hash);
    if (err != 0) {
        MBK_LOG("Hash computation failed\n");
        return MBK_BSL_ERROR_HASH_COMPUTATION_FAILED;
    }
    /* Skip signature verification if the image matches the last booted image. */
    if (mbk_svl_compare_hash(update->computed_hash, record->last_booted_image_hash)) {
        return MBK_BSL_SUCCESS_HASH_MATCHED;
    }

    const uint8_t* sig = (const uint8_t*)slot_adr + sig_sz;
    err = mbk_svl_verify_signature(slot_adr, sig_sz, sig, key_gen);
    if (err != 0) {
        MBK_LOG("Signature verification failed\n");
        return MBK_BSL_ERROR_SIGNATURE_VERIFICATION_FAILED;
    }
    return MBK_BSL_SUCCESS_VERIFIED;
}

STATIC int bsl_init(struct mbk_context* ctx)
{
    int err = mbk_svl_init();
    if (err != 0) {
        MBK_LOG("SVL init failed\n");
        return MBK_BSL_ERROR_INIT_FAILED;
    }

    bfl_load_entry((struct mbk_bfl_entry*)ctx);
    return MBK_BSL_SUCCESS;
}

STATIC void bsl_update_record(const struct mbk_record_update* update, struct mbk_boot_record* record)
{
    memcpy(record->last_booted_image_hash, update->computed_hash, sizeof(record->last_booted_image_hash));
    bel_accept_security_version(update->security_version, &record->history);
    bel_accept_key_generation(update->key_generation, &record->history);
}

/* === Public API Functions === */

int mbk_init(struct mbk_context* ctx)
{
    if (ctx == NULL) return MBK_ERROR_INVALID_PARAM;
    return bsl_init(ctx);
}

int mbk_find_bootable_slot(struct mbk_context* ctx, struct mbk_boot_info* boot_info)
{
    if (ctx == NULL || boot_info == NULL) return MBK_ERROR_INVALID_PARAM;

    const struct mbk_header* slot_adrs[MBK_NUM_SLOTS] = {
        (const struct mbk_header*)MBK_SLOT0_BASE,
        (const struct mbk_header*)MBK_SLOT1_BASE
    };
    const bool prefer_slot1 = sml_prefer_slot1(slot_adrs[0], slot_adrs[1]);

    int n = prefer_slot1 ? 1 : 0;
    struct mbk_record_update upd;
    struct mbk_bfl_entry* e = (struct mbk_bfl_entry*)ctx;
    for (unsigned int i = 0; i < MBK_NUM_SLOTS; ++i, n ^= 1) {
        const struct mbk_header* slot_adr = slot_adrs[n];
        switch (bsl_validate_slot(&e->record, slot_adr, &upd)) {
        default:
            break;
        case MBK_BSL_SUCCESS_VERIFIED:
            bsl_update_record(&upd, &e->record);
            /* Failure to update the BFL is non-fatal. Boot continues. */
            (void)bfl_store_entry(e);
            FALLTHROUGH;
        case MBK_BSL_SUCCESS_HASH_MATCHED:
            boot_info->slot_id = n;
            boot_info->header = slot_adr;
            boot_info->entry_point = (uint32_t)((uintptr_t)slot_adr + MBK_HEADER_SIZE);
            return MBK_SUCCESS;
        case MBK_BSL_ERROR_HASH_COMPUTATION_FAILED:
            MBK_LOG("Hash computation failed\n");
            return MBK_ERROR_HASH_COMPUTATION_FAILED;
        }
    }
    MBK_LOG("No bootable slot found\n");
    return MBK_ERROR_NO_BOOTABLE_SLOT;
}
