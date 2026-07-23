#ifndef UART_H
#define UART_H

#include <stddef.h>

/**
 * @file uart.h
 * @brief UART (PL011) ハードウェア抽象化層
 *
 * PL011 UART の送信機能を提供
 * - ポーリング方式のみ（割り込み禁止）
 * - 受信機能は未実装
 */

/**
 * @brief UART0 初期化
 *
 * PL011 UART0 を初期化し、送信可能な状態にする
 * - ボーレート: 115200 bps
 * - データビット: 8
 * - ストップビット: 1
 * - パリティ: なし
 */
void uart_init(void);

/**
 * @brief 1文字送信 (ポーリング)
 *
 * @param c 送信する文字
 *
 * TX FIFO に空きが出るまで待機してから送信
 */
void uart_putc(char c);

/**
 * @brief 文字列送信
 *
 * @param str 送信する NUL 終端文字列
 *
 * 文字列を最後の NUL まで送信
 */
void uart_puts(const char* str);

#endif /* UART_H */
