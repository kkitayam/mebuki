/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <msp432e401y.h>

#include <target_config.h>

#if __STDC_VERSION__ < 202311L
#  include <assert.h>
#endif

static_assert((BFL_DATA0_BASE % FLASH_ERASE_SIZE) == 0U, "BFL_DATA0 must be sector aligned");
static_assert((BFL_DATA1_BASE % FLASH_ERASE_SIZE) == 0U, "BFL_DATA1 must be sector aligned");
static_assert((PROGRESS_BASE % FLASH_ERASE_SIZE) == 0U, "progress must be sector aligned");
static_assert((SLOT0_BASE % FLASH_ERASE_SIZE) == 0U, "slot0 must be sector aligned");
static_assert((SLOT1_BASE % FLASH_ERASE_SIZE) == 0U, "slot1 must be sector aligned");
static_assert((SLOT0_BASE + SLOT0_SIZE) <= 0x00080000U,
               "slot0 must fit below the bank 0 boot mirror");
static_assert(SLOT1_BASE >= 0x00088000U,
               "slot1 must not overlap the bank 1 boot mirror");
static_assert((SLOT1_BASE + SLOT1_SIZE) <= (MBK_FLASH_BASE + MBK_FLASH_SIZE), "slot1 exceeds flash");

extern const uint8_t __text_load[];
extern const uint8_t __data_load[];
extern uint8_t __text[];
extern uint8_t __etext[];
extern uint8_t __data[];
extern uint8_t __edata[];
extern uint8_t __bss[];
extern uint8_t __ebss[];

extern void system_init(void);
extern int main(void);

static inline void wordcpy(void *__restrict dest, const void *__restrict src, size_t n)
{
    const uint32_t *s = src;
    uint32_t *d = dest;
    n /= 4;
    while (n--) {
        *d++ = *s++;
    }
}

static inline void wordset(void *s, uint32_t v, size_t n)
{
    uint32_t *p = s;
    n /= 4;
    while (n--) {
        *p++ = v;
    }
}

void Reset_Handler(void)
{
    SCB->CPACR |= (3UL << 10*2) | (3UL << 11*2); /* CP10 and CP11 */
    system_init();
    wordcpy(__text, __text_load, __etext - __text);
    wordcpy(__data, __data_load, __edata - __data);
    wordset(__bss, 0, __ebss - __bss);
    (void)main();

    for (;;) __WFI();
}

void Default_Handler(void)
{
    for (;;) __WFI();
}
