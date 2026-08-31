/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef UART_H
#define UART_H

#include <stddef.h>
#include <stdint.h>

/*
 * SCI12 UART Hardware Abstraction Layer
 *
 * Provides transmission functionality for the PL011 UART
 * - Polling only (interrupts disabled)
 * - Reception functionality is not implemented
 */

/*
 * UART0 initialization
 *
 * Initialize PL011 UART0 and make it ready for transmission
 * - Baud rate: 115200 bps
 * - Data bits: 8
 * - Stop bits: 1
 * - Parity: none
 */
void uart_init(void);

/*
 * Send a single character (polling)
 *
 * Wait until there is space in the TX FIFO before sending
 */
void uart_putc(char c);

/*
 * Receive one character (polling).
 *
 * Returns a byte in the range 0-255, or -1 when no byte is received before
 * timeout_ms elapses.
 */
int uart_getc_timeout(uint32_t timeout_ms);

/*
 * Send a string (polling)
 *
  * Sends the string up to the terminating NUL character
 */
void uart_puts(const char* str);

#endif /* UART_H */
