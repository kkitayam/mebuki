#include "uart.h"

#include "chip.h"

void uart_init(void)
{
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_UART0);

  Chip_IOCON_PinMuxSet(LPC_IOCON, 0U, 18U, IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN);
  Chip_IOCON_PinMuxSet(LPC_IOCON, 0U, 19U, IOCON_FUNC1 | IOCON_MODE_INACT | IOCON_DIGMODE_EN);

  Chip_UART_Init(LPC_USART);
  Chip_UART_SetBaud(LPC_USART, 115200U);
  Chip_UART_TXEnable(LPC_USART);
}

void uart_putc(char c)
{
  const char value = c;

  (void)Chip_UART_Send(LPC_USART, &value, 1);
}

void uart_puts(const char *str)
{
  if (str == NULL) {
    return;
  }

  while (*str != '\0') {
    uart_putc(*str);
    ++str;
  }
}