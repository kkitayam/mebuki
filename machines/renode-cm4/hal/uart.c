#include "uart.h"
#include "target_config.h"
#include <string.h>

/* ============================================================================
 * PL011 UART register definitions
 * ========================================================================== */

/** @brief UART data register (offset 0x00) */
#define UART_DR         (*(volatile unsigned char *)(UART0_BASE + 0x00))

/** @brief UART flag register (offset 0x18) */
#define UART_FR         (*(volatile unsigned int *)(UART0_BASE + 0x18))

/** @brief UART line control register (offset 0x2C) */
#define UART_LCR_H      (*(volatile unsigned int *)(UART0_BASE + 0x2C))

/** @brief UART control register (offset 0x30) */
#define UART_CR         (*(volatile unsigned int *)(UART0_BASE + 0x30))

/** @brief UART baud rate registers (IBRD: offset 0x24, FBRD: offset 0x28) */
#define UART_IBRD       (*(volatile unsigned int *)(UART0_BASE + 0x24))
#define UART_FBRD       (*(volatile unsigned int *)(UART0_BASE + 0x28))

/* ============================================================================
 * PL011 flag register (FR) bits
 * ========================================================================== */

/** @brief TX FIFO Empty flag */
#define UART_FR_TXFE    (1U << 7)

/** @brief RX FIFO Full flag */
#define UART_FR_RXFF    (1U << 6)

/** @brief UART Busy flag */
#define UART_FR_BUSY    (1U << 3)

/** @brief RX FIFO Empty flag */
#define UART_FR_RXFE    (1U << 4)

/* ============================================================================
 * PL011 line control register (LCR_H) bits
 * ========================================================================== */

/** @brief FIFO Enable */
#define UART_LCR_H_FEN  (1U << 4)

/** @brief Word Length: 8-bit */
#define UART_LCR_H_WLEN_8BIT (0x3U << 5)

/* ============================================================================
 * PL011 control register (CR) bits
 * ========================================================================== */

/** @brief UART Enable */
#define UART_CR_UARTEN  (1U << 0)

/** @brief TX Enable */
#define UART_CR_TXE     (1U << 8)

/** @brief RX Enable */
#define UART_CR_RXE     (1U << 9)

/* ============================================================================
 * Functions
 * ========================================================================== */

void uart_init(void)
{
    /*
     * PL011 initialization procedure:
     * 1. Disable UART (CR = 0)
     * 2. Set baud rate (IBRD, FBRD)
     * 3. Configure line control (LCR_H): 8-bit, enable FIFO
     * 4. Enable UART (CR: UARTEN | TXE | RXE)
     *
     * Baud rate calculation (Renode, 32MHz, 115200 bps):
     *   UART_BRD = UARTCLK / (16 * Baud)
     *           = 32000000 / (16 * 115200)
     *           = 17.36...
     *   IBRD = 17 (integer part)
     *   FBRD = 0.36... * 64 ≈ 23 (fractional part)
     */

    /* Disable UART */
    UART_CR = 0U;

    /* Set baud rate (115200 bps, 32MHz) */
    UART_IBRD = 17U;
    UART_FBRD = 23U;

    /* Line control: 8-bit, enable FIFO */
    UART_LCR_H = UART_LCR_H_WLEN_8BIT | UART_LCR_H_FEN;

    /* Enable UART: UARTEN | TXE | RXE */
    UART_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

void uart_putc(char c)
{
    /*
     * Wait until there is space in the TX FIFO
     * PL011 polling transmission
     */
    while ((UART_FR & UART_FR_TXFE) == 0) {
        /* TX FIFO is full */
    }

    /* Write to the data register */
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
