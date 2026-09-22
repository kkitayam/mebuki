#include <string.h>

#include "flash.h"
#include "msp432e401y.h"
#include "target_config.h"

#define FLASH_WRITE_KEY 0xA4420000U
#define FLASH_BANK_SIZE 0x00080000U
#define FLASH_WRITE_BUFFER_WORDS 32U

static bool valid_range(uintptr_t address, size_t size)
{
    if (size == 0U) {
        return true;
    }
    if (address < MBK_FLASH_BASE || address >= MBK_FLASH_BASE + MBK_FLASH_SIZE) {
        return false;
    }
    return size <= (MBK_FLASH_BASE + MBK_FLASH_SIZE) - address;
}

static uintptr_t physical_address(uintptr_t address)
{
    if ((FLASH_CTRL->CONF & FLASH_CONF_FMME) == 0U) {
        return address;
    }
    return address < FLASH_BANK_SIZE ? address + FLASH_BANK_SIZE : address - FLASH_BANK_SIZE;
}

static bool controller_error(uint32_t mask)
{
    return (FLASH_CTRL->FCRIS & mask) != 0U;
}

static int erase_one(uintptr_t address) __attribute__((section(".ramfunc"), noinline));
static int erase_one(uintptr_t address)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC | FLASH_FCMISC_ERMISC;
    FLASH_CTRL->FMA = (uint32_t)address;
    FLASH_CTRL->FMC = FLASH_WRITE_KEY | FLASH_FMC_ERASE;
    while ((FLASH_CTRL->FMC & FLASH_FMC_ERASE) != 0U) {
    }
    return controller_error(FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS | FLASH_FCRIS_ERRIS)
               ? -1 : 0;
}

static int program_buffer(uintptr_t address, const uint32_t *source, size_t words)
    __attribute__((section(".ramfunc"), noinline));
static int program_buffer(uintptr_t address, const uint32_t *source, size_t words)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC |
                         FLASH_FCMISC_INVDMISC | FLASH_FCMISC_PROGMISC;
    FLASH_CTRL->FMA = (uint32_t)(address & ~0x7FU);
    for (size_t index = 0U; index < words; ++index) {
        FLASH_CTRL->FWBN[((address + index * sizeof(uint32_t)) & 0x7CU) / sizeof(uint32_t)] =
            source[index];
    }
    FLASH_CTRL->FMC2 = FLASH_WRITE_KEY | FLASH_FMC2_WRBUF;
    while ((FLASH_CTRL->FMC2 & FLASH_FMC2_WRBUF) != 0U) {
    }
    return controller_error(FLASH_FCRIS_ARIS | FLASH_FCRIS_VOLTRIS |
                            FLASH_FCRIS_INVDRIS | FLASH_FCRIS_PROGRIS) ? -1 : 0;
}

void hal_flash_init(void)
{
    FLASH_CTRL->FCMISC = FLASH_FCMISC_AMISC | FLASH_FCMISC_VOLTMISC |
                         FLASH_FCMISC_ERMISC | FLASH_FCMISC_INVDMISC |
                         FLASH_FCMISC_PROGMISC;
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
    if (data == NULL || (address & 3U) != 0U || (size & 3U) != 0U ||
        !valid_range(address, size)) {
        return -1;
    }
    const uint32_t *source = (const uint32_t *)data;
    uintptr_t current = address;
    while (size != 0U) {
        const uintptr_t physical = physical_address(current);
        size_t words = FLASH_WRITE_BUFFER_WORDS - ((physical >> 2) & 31U);
        if (words > size / sizeof(uint32_t)) {
            words = size / sizeof(uint32_t);
        }
        if (program_buffer(physical, source, words) != 0) {
            return -1;
        }
        current += words * sizeof(uint32_t);
        source += words;
        size -= words * sizeof(uint32_t);
    }
    return 0;
}

int hal_flash_erase_sector(uint32_t address)
{
    if ((address & (FLASH_ERASE_SIZE - 1U)) != 0U ||
        !valid_range(address, FLASH_ERASE_SIZE)) {
        return -1;
    }
    return erase_one(physical_address(address));
}

bool hal_flash_is_blank(uintptr_t address)
{
    if (!valid_range(address, FLASH_ERASE_SIZE)) {
        return false;
    }
    for (size_t index = 0U; index < FLASH_ERASE_SIZE; ++index) {
        if (((const uint8_t *)address)[index] != 0xFFU) {
            return false;
        }
    }
    return true;
}
