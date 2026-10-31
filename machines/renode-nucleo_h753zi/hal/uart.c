/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "uart.h"

#include <stdint.h>

#define USART3_BASE 0x40004800U
#define REG(offset) (*(volatile uint32_t *)(USART3_BASE + (offset)))
#define USART_CR1 REG(0x00U)
#define USART_BRR REG(0x0CU)
#define USART_ISR REG(0x1CU)
#define USART_RDR REG(0x24U)
#define USART_TDR REG(0x28U)
#define ISR_RXNE (1U << 5)
#define ISR_TXE (1U << 7)

void uart_init(void)
{
    USART_CR1 = 0U;
    USART_BRR = 1085U;
    USART_CR1 = (1U << 0) | (1U << 2) | (1U << 3);
}

void uart_putc(char c)
{
    while ((USART_ISR & ISR_TXE) == 0U) {}
    USART_TDR = (uint8_t)c;
}

int uart_getc_timeout(uint32_t timeout_ms)
{
    uint32_t remaining = timeout_ms * 8000U;
    while (remaining-- != 0U) {
        if ((USART_ISR & ISR_RXNE) != 0U) {
            return (int)(USART_RDR & 0xFFU);
        }
    }
    return -1;
}

void uart_puts(const char *text)
{
    if (text == NULL) {
        return;
    }
    while (*text != '\0') {
        uart_putc(*text++);
    }
}
