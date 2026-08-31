/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "uart.h"

#include "iodefine.h"

void uart_init(void)
{
    SYSTEM.PRCR.WORD = 0xA502;
    MSTP(SCI12) = 0;
    SYSTEM.PRCR.WORD = 0xA500;  

    /* 1) Set to general-purpose I/O temporarily */
    PORTE.PMR.BIT.B1 = 0;
    PORTE.PMR.BIT.B2 = 0;

    /* 2) Direction settings */
    PORTE.PDR.BIT.B1 = 1;   /* PE1 TX output */
    PORTE.PDR.BIT.B2 = 0;   /* PE2 RX input */

    /* 3) PFS settings (write while PMR=0) */
    MPC.PWPR.BIT.B0WI  = 0;
    MPC.PWPR.BIT.PFSWE = 1;

    MPC.PE1PFS.BYTE = 0x0C; /* TXD12 */
    MPC.PE2PFS.BYTE = 0x0C; /* RXD12 */

    MPC.PWPR.BIT.PFSWE = 0;
    MPC.PWPR.BIT.B0WI  = 1;

    /* 4) Switch to peripheral function */
    PORTE.PMR.BIT.B1 = 1;
    PORTE.PMR.BIT.B2 = 1;

    /* SCI12 */
    SCI12.SCR.BYTE  = 0x00;
    SCI12.SMR.BYTE  = 0x00;
    SCI12.SCMR.BYTE = 0xF2;
    SCI12.SEMR.BYTE = 0x54;  /* ABCS | BGDM | BRME */
    if (SYSTEM.SCKCR3.BIT.CKSEL != 1) {
        /* LOCO @ 4MHz  => PCLK 500kHz */
        SCI12.BRR       = 0;     /* At PCLK=500kHz, 62500 bps */
        SCI12.MDDR      = 0xEC;  /* 0xEC=236 -> 62500 * 236 / 256 = 57617.2 bps */
    } else {
        /* At HOCO @ 64MHz => PCLKB 32MHz */
#if 0
        SCI12.BRR       = 3;     /* At PCLK=32MHz, 1000000 bps */
        SCI12.MDDR      = 0xFF;  /* MDDR=0xFF -> BRR=3 -> 1000000 * 255 / 256 = 921875 bps */
#else
        SCI12.BRR       = 33;    /* At PCLK=32MHz, 117647.059 bps */
        SCI12.MDDR      = 0xFB;  /* MDDR=0xFF -> BRR=3 -> 117646.059 * 251 / 256 = 115349.264 bps */
#endif
    }

    SCI12.SCR.BYTE  = 0x30;  // TE/RE
}

void uart_putc(char c)
{
    while (SCI12.SSR.BIT.TDRE == 0) ;
    SCI12.TDR = c;
}

int uart_getc_timeout(uint32_t timeout_ms)
{
    uint32_t remaining = timeout_ms * 16000U;

    while (remaining > 0U) {
        if (SCI12.SSR.BIT.RDRF != 0) {
            return (int)SCI12.RDR;
        }
        remaining--;
    }

    return -1;
}

void uart_puts(const char *str)
{
    if (str == NULL) return;
    char c;
    while ((c = *str)) {
        if ('\n' == c) {
            uart_putc('\r');       /* CR+LF */
        }
        uart_putc(c);
        str++;
    }
    /* Wait for transmission completion (optional) */
    while (SCI12.SSR.BIT.TEND == 0) ;
}

void _putchar(char character)
{
    if ('\n' == character) {
        uart_putc('\r');       /* CR+LF */
    }
    uart_putc(character);
}
