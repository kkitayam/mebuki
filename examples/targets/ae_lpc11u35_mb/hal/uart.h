#ifndef UART_H
#define UART_H

#include <stddef.h>

/**
 * @file uart.h
 * @brief AE LPC11U35-MB の UART0 HAL
 */

/**
 * @brief UART0 を初期化する。
 *
 * ボーレートは 115200 bps、8N1、ポーリング送信を前提とする。
 */
void uart_init(void);

/**
 * @brief 1 文字を送信する。
 *
 * @param c 送信する文字
 */
void uart_putc(char c);

/**
 * @brief NUL 終端文字列を送信する。
 *
 * @param str 送信する文字列
 */
void uart_puts(const char *str);

#endif /* UART_H */