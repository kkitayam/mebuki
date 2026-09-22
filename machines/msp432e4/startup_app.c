#include <stdint.h>

#include "msp432e401y.h"

extern void system_init(void);
extern int main(void);

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
