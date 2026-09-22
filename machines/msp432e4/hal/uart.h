#ifndef MSP432E4_UART_H
#define MSP432E4_UART_H

void uart_init(void);
void uart_putc(char character);
void uart_puts(const char *string);

#endif
