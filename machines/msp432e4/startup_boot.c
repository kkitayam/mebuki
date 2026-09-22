#include <stddef.h>
#include <stdint.h>

#include "msp432e401y.h"

extern uint8_t __text[];
extern uint8_t __etext[];
extern const uint8_t __text_load[];
extern uint8_t __data[];
extern uint8_t __edata[];
extern const uint8_t __data_load[];
extern uint8_t __bss[];
extern uint8_t __ebss[];

extern void system_init(void);
extern int main(void);

static void copy_words(uint8_t *destination, const uint8_t *source, size_t size)
{
    uint32_t *destination_words = (uint32_t *)destination;
    const uint32_t *source_words = (const uint32_t *)source;

    for (size_t index = 0U; index < size / sizeof(uint32_t); ++index) {
        destination_words[index] = source_words[index];
    }
}

static void clear_words(uint8_t *destination, size_t size)
{
    uint32_t *destination_words = (uint32_t *)destination;

    for (size_t index = 0U; index < size / sizeof(uint32_t); ++index) {
        destination_words[index] = 0U;
    }
}

void Default_Handler(void)
{
    for (;;) {
        __WFI();
    }
}

void Reset_Handler(void) __attribute__((noreturn, noinline, section(".text.reset")));
void Reset_Handler(void)
{
    system_init();
    copy_words(__text, __text_load, (size_t)(__etext - __text));
    copy_words(__data, __data_load, (size_t)(__edata - __data));
    clear_words(__bss, (size_t)(__ebss - __bss));
    (void)main();

    for (;;) {
        __WFI();
    }
}

__attribute__((section(".isr_vector"), used))
const uintptr_t vector_table[] = {
    (uintptr_t)0x20040000U,
    (uintptr_t)Reset_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
    (uintptr_t)Default_Handler,
};
