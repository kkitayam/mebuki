#include "flash.h"

#include <string.h>

#include "chip.h"
#include "target_config.h"

static int flash_range_is_valid(uint32_t addr, size_t len)
{
  if (len == 0U) {
    return 1;
  }

#if FLASH_BASE != 0U
  if (addr < FLASH_BASE) {
    return 0;
  }
#endif

  if (addr >= FLASH_BASE + FLASH_SIZE) {
    return 0;
  }

  if (len > (size_t)(FLASH_BASE + FLASH_SIZE - addr)) {
    return 0;
  }

  return 1;
}

static int flash_is_sector_aligned(uint32_t addr)
{
  return ((addr - FLASH_BASE) & (FLASH_SECTOR_SIZE - 1U)) == 0U;
}

static int flash_is_page_aligned(uint32_t addr, size_t len)
{
  return (((addr - FLASH_BASE) & (FLASH_PAGE_SIZE - 1U)) == 0U) && ((len & (FLASH_PAGE_SIZE - 1U)) == 0U);
}

static uint32_t flash_sector_index(uint32_t addr)
{
  return (addr - FLASH_BASE) / FLASH_SECTOR_SIZE;
}

enum
{
  IAP_CMD_PREPARE_SECTOR = 50U,
  IAP_CMD_COPY_RAM_TO_FLASH = 51U,
  IAP_CMD_ERASE_SECTOR = 52U,
};

static unsigned int flash_cclk_khz(void)
{
  return (SystemCoreClock != 0U) ? (unsigned int)(SystemCoreClock / 1000U) : 0U;
}

static uint8_t flash_iap_call(const unsigned int command[5])
{
  unsigned int result[5] = {0U, 0U, 0U, 0U, 0U};

  iap_entry((unsigned int *)command, result);
  return (uint8_t)result[0];
}

int hal_flash_read(uint32_t addr, void *buf, size_t len)
{
  if (buf == NULL) {
    return -1;
  }

  if (len == 0U) {
    return 0;
  }

  if (!flash_range_is_valid(addr, len)) {
    return -1;
  }

  memcpy(buf, (const void *)(uintptr_t)addr, len);
  return 0;
}

int hal_flash_write(uint32_t addr, const void *data, size_t len)
{
  if (data == NULL) {
    return -1;
  }

  if (len == 0U) {
    return 0;
  }

  if (!flash_range_is_valid(addr, len)) {
    return -1;
  }

  if (!flash_is_page_aligned(addr, len)) {
    return -1;
  }

  const uint32_t start_sector = flash_sector_index(addr);
  const uint32_t end_sector = flash_sector_index(addr + (uint32_t)len - 1U);

  {
    const unsigned int prepare_command[5] = {
      IAP_CMD_PREPARE_SECTOR,
      start_sector,
      end_sector,
      0U,
      0U,
    };

    if (flash_iap_call(prepare_command) != 0U) {
      return -1;
    }
  }

  const uint8_t *src = (const uint8_t *)data;
  for (size_t offset = 0U; offset < len; offset += FLASH_PAGE_SIZE) {
    unsigned int page_words[FLASH_PAGE_SIZE / sizeof(unsigned int)];
    memcpy(page_words, src + offset, FLASH_PAGE_SIZE);

    const uint32_t page_addr = addr + (uint32_t)offset;
    {
      const unsigned int write_command[5] = {
        IAP_CMD_COPY_RAM_TO_FLASH,
        page_addr,
        (unsigned int)(uintptr_t)page_words,
        FLASH_PAGE_SIZE,
        flash_cclk_khz(),
      };

      if (flash_iap_call(write_command) != 0U) {
        return -1;
      }
    }

    if (memcmp((const void *)(uintptr_t)page_addr, src + offset, FLASH_PAGE_SIZE) != 0) {
      return -1;
    }
  }

  return 0;
}

int hal_flash_erase_sector(uint32_t addr)
{
  if (!flash_range_is_valid(addr, FLASH_SECTOR_SIZE)) {
    return -1;
  }

  if (!flash_is_sector_aligned(addr)) {
    return -1;
  }

  const uint32_t sector = flash_sector_index(addr);

  {
    const unsigned int prepare_command[5] = {
      IAP_CMD_PREPARE_SECTOR,
      sector,
      sector,
      0U,
      0U,
    };

    const unsigned int erase_command[5] = {
      IAP_CMD_ERASE_SECTOR,
      sector,
      sector,
      flash_cclk_khz(),
      0U,
    };

    if (flash_iap_call(prepare_command) != 0U) {
      return -1;
    }

    if (flash_iap_call(erase_command) != 0U) {
      return -1;
    }
  }

  for (size_t i = 0U; i < FLASH_SECTOR_SIZE; ++i) {
    if (((const uint8_t *)(uintptr_t)addr)[i] != 0xFFU) {
      return -1;
    }
  }

  return 0;
}