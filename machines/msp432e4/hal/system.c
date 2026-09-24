#include "msp432e401y.h"
#include "system.h"

void system_init(void)
{
    SYSCTL->PCCAN = 0U;
    SYSCTL->PCEMAC = 0U;
    SYSCTL->PCEPHY = 0U;
    SYSCTL->PCCCM = 0U;

    SYSCTL->MOSCCTL = SYSCTL_MOSCCTL_OSCRNG;
    while ((SYSCTL->RIS & SYSCTL_RIS_MOSCPUPRIS) == 0U) {
    }
    SYSCTL->MISC = SYSCTL_MISC_MOSCPUPMIS;
    SYSCTL->RSCLKCFG = SYSCTL_RSCLKCFG_PLLSRC_MOSC;
    SYSCTL->PLLFREQ1 = (4U << SYSCTL_PLLFREQ1_N_S) | (1U << SYSCTL_PLLFREQ1_Q_S);
    SYSCTL->PLLFREQ0 = (96U << SYSCTL_PLLFREQ0_MINT_S) | SYSCTL_PLLFREQ0_PLLPWR;
    SYSCTL->MEMTIM0 = SYSCTL_MEMTIM0_EBCHT_3_5 | (5U << SYSCTL_MEMTIM0_EWS_S) |
                      SYSCTL_MEMTIM0_FBCHT_3_5 | (5U << SYSCTL_MEMTIM0_FWS_S) |
                      SYSCTL_MEMTIM0_MB1;
    while ((SYSCTL->RIS & SYSCTL_RIS_PLLLRIS) == 0U) {
    }
    SYSCTL->MISC = SYSCTL_MISC_PLLLMIS;
    SYSCTL->RSCLKCFG = SYSCTL_RSCLKCFG_MEMTIMU | SYSCTL_RSCLKCFG_ACG |
                       SYSCTL_RSCLKCFG_USEPLL | SYSCTL_RSCLKCFG_PLLSRC_MOSC |
                       (1U << SYSCTL_RSCLKCFG_PSYSDIV_S);
}

void system_reset(void)
{
    SCB->AIRCR = (0x5FAU << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk;
    for (;;) {
        __NOP();
    }
}

void prepare_handoff(void)
{
    __disable_irq();
    __DSB();
    __ISB();
}

void jump_to_firmware(uint32_t entry_point)
{
    const uint32_t *vector_table = (const uint32_t *)(uintptr_t)entry_point;
    __set_MSP(vector_table[0]);
    ((void (*)(void))(uintptr_t)vector_table[1])();
    halt();
}

void halt(void)
{
    __disable_irq();
    for (;;) {
        __WFI();
    }
}

uint32_t get_cycle_count(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    return DWT->CYCCNT;
}
