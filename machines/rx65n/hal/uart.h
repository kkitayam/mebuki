/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(void);
void uart_putc(char c);
int uart_getc_timeout(uint32_t timeout_ms);
void uart_puts(const char* str);

#endif /* UART_H */
