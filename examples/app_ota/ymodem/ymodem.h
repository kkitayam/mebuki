/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#ifndef YMODEM_H
#define YMODEM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YMODEM_FILENAME_MAX 32U

/*
 * Receive one YMODEM file in CRC-16 mode.
 *
 * buf must designate writable storage for buf_size bytes. filename may be
 * NULL; otherwise it must provide YMODEM_FILENAME_MAX bytes. The function
 * returns the declared file size on success and a negative error code on
 * timeout, cancellation, malformed input, or insufficient storage.
 */
int32_t ymodem_receive(uint8_t *buf, size_t buf_size, char *filename);

#ifdef __cplusplus
}
#endif

#endif /* YMODEM_H */
