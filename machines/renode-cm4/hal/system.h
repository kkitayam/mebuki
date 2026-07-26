/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdint.h>

void system_init(void);
void system_reset(void);
void prepare_handoff(void);

__attribute__((noreturn))
void jump_to_firmware(uint32_t entry_point);

void halt(void) __attribute__((noreturn));

#endif /* SYSTEM_H */
