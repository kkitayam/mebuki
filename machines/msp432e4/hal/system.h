#ifndef MSP432E4_SYSTEM_H
#define MSP432E4_SYSTEM_H

#include <stdint.h>

void system_init(void);
void system_reset(void);
void prepare_handoff(void);
void jump_to_firmware(uint32_t entry_point) __attribute__((noreturn));
void halt(void) __attribute__((noreturn));
uint32_t get_cycle_count(void);

#endif
