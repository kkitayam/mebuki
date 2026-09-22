#ifndef MSP432E4_UART_H
#define MSP432E4_UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char character);
void uart_puts(const char *string);
int uart_getc_timeout(uint32_t timeout_ms);

#endif
