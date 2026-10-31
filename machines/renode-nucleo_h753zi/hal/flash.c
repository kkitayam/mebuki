/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "flash.h"
#include "target_config.h"

#include <string.h>

#define FLASH_REG_BASE 0x52002000U
#define REG(offset) (*(volatile uint32_t *)(FLASH_REG_BASE + (offset)))
#define BANK_OFFSET 0x100U
#define KEYR(bank) REG(0x04U + (bank) * BANK_OFFSET)
#define CR(bank) REG(0x0CU + (bank) * BANK_OFFSET)
#define SR(bank) REG(0x10U + (bank) * BANK_OFFSET)
#define CCR(bank) REG(0x14U + (bank) * BANK_OFFSET)
#define OPTKEYR REG(0x08U)
#define OPTCR REG(0x18U)
#define OPTSR_CUR REG(0x1CU)
#define OPTSR_PRG REG(0x20U)

#define FLASH_KEY1 0x45670123U
#define FLASH_KEY2 0xCDEF89ABU
#define FLASH_OPTKEY1 0x08192A3BU
#define FLASH_OPTKEY2 0x4C5D6E7FU
#define FLASH_LOCK (1U << 0)
#define FLASH_PG (1U << 1)
#define FLASH_SER (1U << 2)
#define FLASH_START (1U << 7)
#define FLASH_BUSY_MASK 0x5U
#define FLASH_ERRORS 0x07EE0000U
#define FLASH_CLEAR 0x07EF0000U
#define FLASH_SWAP (1U << 31)
#define FLASH_WORD_SIZE 32U
#define FLASH_TIMEOUT 10000000U

static bool valid_range(uint32_t address, size_t size)
{
    return address >= FLASH_BASE && address - FLASH_BASE <= FLASH_SIZE &&
           size <= FLASH_SIZE - (address - FLASH_BASE);
}

static bool writable_range(uint32_t address, size_t size)
{
    const uint32_t bases[] = {BFL_DATA0_BASE, BFL_DATA1_BASE, SLOT0_BASE, SLOT1_BASE};
    const uint32_t sizes[] = {FLASH_ERASE_SIZE, FLASH_ERASE_SIZE, SLOT0_SIZE, SLOT1_SIZE};
    for (size_t i = 0; i < sizeof(bases) / sizeof(bases[0]); ++i) {
        if (address >= bases[i] && address - bases[i] <= sizes[i] &&
            size <= sizes[i] - (address - bases[i])) {
            return true;
        }
    }
    return false;
}

static int wait_idle(unsigned bank)
{
    for (uint32_t count = 0; count < FLASH_TIMEOUT; ++count) {
        if ((SR(bank) & FLASH_BUSY_MASK) == 0U) {
            return 0;
        }
    }
    return -1;
}

static int wait_ready(unsigned bank)
{
    return wait_idle(bank) == 0 && (SR(bank) & FLASH_ERRORS) == 0U ? 0 : -1;
}

static int unlock(unsigned bank)
{
    if ((CR(bank) & FLASH_LOCK) != 0U) {
        KEYR(bank) = FLASH_KEY1;
        KEYR(bank) = FLASH_KEY2;
    }
    return (CR(bank) & FLASH_LOCK) == 0U ? 0 : -1;
}

void hal_flash_init(void)
{
}

int hal_flash_read(uint32_t address, void *buffer, size_t size)
{
    if (buffer == NULL || !valid_range(address, size)) {
        return -1;
    }
    memcpy(buffer, (const void *)(uintptr_t)address, size);
    return 0;
}

int hal_flash_write(uint32_t address, const void *data, size_t size)
{
    if (data == NULL || address % FLASH_WORD_SIZE != 0U ||
        size > SIZE_MAX - (FLASH_WORD_SIZE - 1U)) {
        return -1;
    }
    const size_t padded = (size + FLASH_WORD_SIZE - 1U) & ~(size_t)(FLASH_WORD_SIZE - 1U);
    if (!writable_range(address, padded)) {
        return -1;
    }
    if (size == 0U) {
        return 0;
    }
    const uint8_t *current = (const uint8_t *)(uintptr_t)address;
    for (size_t i = 0; i < padded; ++i) {
        if (current[i] != 0xFFU) {
            return -1;
        }
    }

    const unsigned bank = (address - FLASH_BASE) / 0x100000U;
    if (wait_idle(bank) != 0 || unlock(bank) != 0) {
        return -1;
    }
    CCR(bank) = FLASH_CLEAR;
    const uint8_t *source = data;
    int result = 0;
    for (size_t offset = 0; offset < padded; offset += FLASH_WORD_SIZE) {
        uint32_t words[FLASH_WORD_SIZE / sizeof(uint32_t)];
        memset(words, 0xFF, sizeof(words));
        size_t count = size - offset;
        if (count > FLASH_WORD_SIZE) {
            count = FLASH_WORD_SIZE;
        }
        memcpy(words, source + offset, count);
        CR(bank) |= FLASH_PG;
        volatile uint32_t *destination = (volatile uint32_t *)(uintptr_t)(address + offset);
        for (size_t i = 0; i < sizeof(words) / sizeof(words[0]); ++i) {
            destination[i] = words[i];
        }
        __asm__ volatile("dsb" ::: "memory");
        if (wait_ready(bank) != 0) {
            result = -1;
            break;
        }
        CR(bank) &= ~FLASH_PG;
        if (memcmp((const void *)(uintptr_t)(address + offset), words, sizeof(words)) != 0) {
            result = -1;
            break;
        }
    }
    CR(bank) = (CR(bank) & ~FLASH_PG) | FLASH_LOCK;
    return result;
}

int hal_flash_erase_sector(uint32_t address)
{
    if (address % FLASH_ERASE_SIZE != 0U ||
        !writable_range(address, FLASH_ERASE_SIZE)) {
        return -1;
    }
    const unsigned bank = (address - FLASH_BASE) / 0x100000U;
    const unsigned sector = ((address - FLASH_BASE) % 0x100000U) / FLASH_ERASE_SIZE;
    if (wait_idle(bank) != 0 || unlock(bank) != 0) {
        return -1;
    }
    CCR(bank) = FLASH_CLEAR;
    CR(bank) = (CR(bank) & ~(0x7U << 8)) | FLASH_SER | (sector << 8);
    CR(bank) |= FLASH_START;
    int result = wait_ready(bank);
    CR(bank) = (CR(bank) & ~(FLASH_SER | (0x7U << 8))) | FLASH_LOCK;
    if (result == 0 && !hal_flash_is_blank(address)) {
        result = -1;
    }
    return result;
}

bool hal_flash_is_blank(uintptr_t address)
{
    if (address > UINT32_MAX || address % FLASH_ERASE_SIZE != 0U ||
        !writable_range((uint32_t)address, FLASH_ERASE_SIZE)) {
        return false;
    }
    const uint8_t *memory = (const uint8_t *)address;
    for (size_t i = 0; i < FLASH_ERASE_SIZE; ++i) {
        if (memory[i] != 0xFFU) {
            return false;
        }
    }
    return true;
}

int hal_flash_swap_bank(void)
{
    if ((OPTCR & FLASH_LOCK) != 0U) {
        OPTKEYR = FLASH_OPTKEY1;
        OPTKEYR = FLASH_OPTKEY2;
    }
    if ((OPTCR & FLASH_LOCK) != 0U) {
        return -1;
    }
    const uint32_t desired = OPTSR_CUR ^ FLASH_SWAP;
    OPTSR_PRG = desired;
    OPTCR |= (1U << 1);
    for (uint32_t count = 0; count < FLASH_TIMEOUT; ++count) {
        if ((OPTSR_CUR & FLASH_SWAP) == (desired & FLASH_SWAP)) {
            OPTCR |= FLASH_LOCK;
            return 0;
        }
    }
    OPTCR |= FLASH_LOCK;
    return -1;
}
