#include <stddef.h>

#include "msp432e401y.h"
#include "uart.h"

void uart_init(void)
{
    const uint32_t gpio_mask = 1U;

    SYSCTL->RCGCGPIO |= gpio_mask;
    while ((SYSCTL->RCGCGPIO & gpio_mask) != gpio_mask) {
    }
    GPIOA->AFSEL = 3U;
    GPIOA->PCTL = 0x11U;
    GPIOA->DEN = 3U;

    SYSCTL->RCGCUART |= 1U;
    while ((SYSCTL->PRUART & 1U) == 0U) {
    }
    UART0->CTL = 0U;
    UART0->IBRD = 8U;
    UART0->FBRD = 44U;
    UART0->LCRH = UART_LCRH_WLEN_8 | UART_LCRH_FEN;
    UART0->CC = UART_CC_CS_PIOSC;
    UART0->CTL = UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;
}

void uart_putc(char character)
{
    while ((UART0->FR & UART_FR_TXFF) != 0U) {
    }
    UART0->DR = (uint32_t)(unsigned char)character;
}

void uart_puts(const char *string)
{
    if (string == NULL) {
        return;
    }
    while (*string != '\0') {
        if (*string == '\n') {
            uart_putc('\r');
        }
        uart_putc(*string++);
    }
    while ((UART0->FR & UART_FR_BUSY) != 0U) {
    }
}

int uart_getc_timeout(uint32_t timeout_ms)
{
    uint32_t cycles = timeout_ms * 120000U;
    while (cycles-- > 0U) {
        if ((UART0->FR & UART_FR_RXFE) == 0U) {
            return (int)(UART0->DR & 0xFFU);
        }
    }
    return -1;
}
