#include "system.h"

#include "chip.h"

const uint32_t OscRateIn = 12000000U;
const uint32_t ExtRateIn = 0U;
uint32_t SystemCoreClock = 12000000U;

void _init(void)
{
}

void SystemCoreClockUpdate(void)
{
  SystemCoreClock = Chip_Clock_GetSystemClockRate();
}

void system_init(void)
{
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_FLASHREG);
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GPIO);
  Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

  LPC_SYSCTL->SYSAHBCLKDIV = 1U;
  SystemCoreClockUpdate();
}

void prepare_handoff(void)
{
  __disable_irq();
}

void jump_to_firmware(uint32_t entry_point)
{
  if ((entry_point & 0x3U) != 0U) {
    halt();
  }

  const uint32_t *vector_table = (const uint32_t *)(uintptr_t)entry_point;
  const uint32_t initial_msp = vector_table[0];
  void (*reset_vector)(void) = (void (*)(void))(uintptr_t)vector_table[1];

  if ((initial_msp & 0x3U) != 0U || reset_vector == 0U) {
    halt();
  }

  __disable_irq();
  __set_MSP(initial_msp);
  __DSB();
  __ISB();

  reset_vector();

  halt();
}

void halt(void)
{
  __disable_irq();

  for (;;) {
    __asm__ volatile("wfi");
  }
}