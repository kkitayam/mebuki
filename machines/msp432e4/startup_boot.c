#include <stddef.h>
#include <stdint.h>
#include <string.h>

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

void Reset_Handler(void)
{
    system_init();
    memcpy(__text, __text_load, (size_t)(__etext - __text));
    memcpy(__data, __data_load, (size_t)(__edata - __data));
    memset(__bss, 0, (size_t)(__ebss - __bss));
    (void)main();

    for (;;) {
        __asm__ volatile ("wfi");
    }
}

void Default_Handler(void)
{
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
