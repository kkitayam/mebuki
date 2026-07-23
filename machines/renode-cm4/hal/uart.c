#include "uart.h"
#include "target_config.h"
#include <string.h>

/* ============================================================================
 * PL011 UART レジスタ定義
 * ========================================================================== */

/** @brief UART データレジスタ (offset 0x00) */
#define UART_DR         (*(volatile unsigned char *)(UART0_BASE + 0x00))

/** @brief UART フラグレジスタ (offset 0x18) */
#define UART_FR         (*(volatile unsigned int *)(UART0_BASE + 0x18))

/** @brief UART ライン制御レジスタ (offset 0x2C) */
#define UART_LCR_H      (*(volatile unsigned int *)(UART0_BASE + 0x2C))

/** @brief UART コントロールレジスタ (offset 0x30) */
#define UART_CR         (*(volatile unsigned int *)(UART0_BASE + 0x30))

/** @brief UART ボーレートレジスタ (IBRD: offset 0x24, FBRD: offset 0x28) */
#define UART_IBRD       (*(volatile unsigned int *)(UART0_BASE + 0x24))
#define UART_FBRD       (*(volatile unsigned int *)(UART0_BASE + 0x28))

/* ============================================================================
 * PL011 フラグレジスタ (FR) ビット
 * ========================================================================== */

/** @brief TX FIFO Empty フラグ */
#define UART_FR_TXFE    (1U << 7)

/** @brief RX FIFO Full フラグ */
#define UART_FR_RXFF    (1U << 6)

/** @brief UART Busy フラグ */
#define UART_FR_BUSY    (1U << 3)

/** @brief RX FIFO Empty フラグ */
#define UART_FR_RXFE    (1U << 4)

/* ============================================================================
 * PL011 ライン制御レジスタ (LCR_H) ビット
 * ========================================================================== */

/** @brief FIFO Enable */
#define UART_LCR_H_FEN  (1U << 4)

/** @brief Word Length: 8-bit */
#define UART_LCR_H_WLEN_8BIT (0x3U << 5)

/* ============================================================================
 * PL011 コントロールレジスタ (CR) ビット
 * ========================================================================== */

/** @brief UART Enable */
#define UART_CR_UARTEN  (1U << 0)

/** @brief TX Enable */
#define UART_CR_TXE     (1U << 8)

/** @brief RX Enable */
#define UART_CR_RXE     (1U << 9)

/* ============================================================================
 * 実装
 * ========================================================================== */

void uart_init(void)
{
    /*
     * PL011 初期化手順:
     * 1. UART を無効化 (CR = 0)
     * 2. ボーレート設定 (IBRD, FBRD)
     * 3. ライン制御設定 (LCR_H): 8-bit, FIFO有効化
     * 4. UART を有効化 (CR: UARTEN | TXE | RXE)
     *
     * ボーレート計算 (Renode, 32MHz, 115200 bps):
     *   UART_BRD = UARTCLK / (16 * Baud)
     *           = 32000000 / (16 * 115200)
     *           = 17.36...
     *   IBRD = 17 (整数部)
     *   FBRD = 0.36... * 64 ≈ 23 (小数部)
     */

    /* UART 無効化 */
    UART_CR = 0U;

    /* ボーレート設定 (115200 bps, 32MHz) */
    UART_IBRD = 17U;
    UART_FBRD = 23U;

    /* ライン制御: 8-bit, FIFO有効化 */
    UART_LCR_H = UART_LCR_H_WLEN_8BIT | UART_LCR_H_FEN;

    /* UART 有効化: UARTEN | TXE | RXE */
    UART_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void uart_putc(char c)
{
    /*
     * TX FIFO に空きが出るまで待機
     * PL011 のポーリング送信
     */
    while ((UART_FR & UART_FR_TXFE) == 0) {
        /* TX FIFO がいっぱい */
    }

    /* データレジスタに書き込み */
    UART_DR = (unsigned char)c;
}

void uart_puts(const char* str)
{
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        uart_putc(*str);
        str++;
    }
}
