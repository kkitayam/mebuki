/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright (c) 2026 Koji KITAYAMA */

#include "ymodem.h"

#include <stdbool.h>
#include <string.h>

#include "flash.h"
#include "mebuki_config.h"
#include "uart.h"

#define YM_SOH 0x01
#define YM_STX 0x02
#define YM_EOT 0x04
#define YM_ACK 0x06
#define YM_NAK 0x15
#define YM_CAN 0x18
#define YM_CRC 'C'

#define YM_PACKET_SIZE 128U
#define YM_PACKET_1K_SIZE 1024U
#define YM_MAX_RETRIES 10U
#define YM_PACKET_TIMEOUT_MS 1000U
#define YM_INITIAL_TIMEOUT_MS 3000U

static uint16_t crc16_ccitt(const uint8_t *data, size_t length)
{
    uint16_t crc = 0U;

    while (length > 0U) {
        crc ^= (uint16_t)*data++ << 8;
        for (uint32_t bit = 0U; bit < 8U; bit++) {
            crc = (crc & 0x8000U) != 0U ? (uint16_t)((crc << 1) ^ 0x1021U) :
                                           (uint16_t)(crc << 1);
        }
        length--;
    }

    return crc;
}

static int receive_packet(uint8_t *payload, uint8_t *sequence, uint32_t timeout_ms)
{
    int character = uart_getc_timeout(timeout_ms);
    if (character == YM_EOT) {
        return 0;
    }
    if (character == YM_CAN && uart_getc_timeout(YM_PACKET_TIMEOUT_MS) == YM_CAN) {
        return -1;
    }
    if (character != YM_SOH && character != YM_STX) {
        return -2;
    }

    const size_t packet_size = character == YM_SOH ? YM_PACKET_SIZE : YM_PACKET_1K_SIZE;
    const int sequence_number = uart_getc_timeout(YM_PACKET_TIMEOUT_MS);
    const int inverse_sequence_number = uart_getc_timeout(YM_PACKET_TIMEOUT_MS);
    if (sequence_number < 0 || inverse_sequence_number < 0 ||
        (sequence_number + inverse_sequence_number) != 0xFF) {
        return -2;
    }

    for (size_t index = 0U; index < packet_size; index++) {
        character = uart_getc_timeout(YM_PACKET_TIMEOUT_MS);
        if (character < 0) {
            return -2;
        }
        payload[index] = (uint8_t)character;
    }

    const int crc_high = uart_getc_timeout(YM_PACKET_TIMEOUT_MS);
    const int crc_low = uart_getc_timeout(YM_PACKET_TIMEOUT_MS);
    if (crc_high < 0 || crc_low < 0) {
        return -2;
    }

    const uint16_t received_crc = ((uint16_t)crc_high << 8) | (uint16_t)crc_low;
    if (received_crc != crc16_ccitt(payload, packet_size)) {
        return -2;
    }

    *sequence = (uint8_t)sequence_number;
    return (int)packet_size;
}

static bool parse_size(const uint8_t *packet, size_t packet_size, size_t *size)
{
    size_t name_length = 0U;
    while (name_length < packet_size && packet[name_length] != '\0') {
        name_length++;
    }
    if (name_length == packet_size || name_length + 1U == packet_size) {
        return false;
    }

    size_t value = 0U;
    size_t index = name_length + 1U;
    if (packet[index] == '\0') {
        return false;
    }
    while (index < packet_size && packet[index] >= '0' && packet[index] <= '9') {
        const size_t digit = (size_t)(packet[index] - '0');
        if (value > (SIZE_MAX - digit) / 10U) {
            return false;
        }
        value = value * 10U + digit;
        index++;
    }

    if (index == name_length + 1U || value == 0U) {
        return false;
    }

    *size = value;
    return true;
}

static int write_packet(uint8_t *destination, const uint8_t *data, size_t length)
{
    const size_t aligned_length = length - (length % MBK_FLASH_PAGE_SIZE);
    if (aligned_length > 0U &&
        hal_flash_write((uintptr_t)destination, data, aligned_length) != 0) {
        return -1;
    }

    if (aligned_length == length) {
        return 0;
    }

    uint8_t final_page[MBK_FLASH_PAGE_SIZE];
    memset(final_page, 0xFF, sizeof(final_page));
    memcpy(final_page, data + aligned_length, length - aligned_length);
    return hal_flash_write(
        (uintptr_t)(destination + aligned_length), final_page, sizeof(final_page));
}

int32_t ymodem_receive(uint8_t *buf, size_t buf_size, char *filename)
{
    if (buf == NULL || buf_size == 0U) {
        return -1;
    }
    if (filename != NULL) {
        filename[0] = '\0';
    }

    uint8_t packet[YM_PACKET_1K_SIZE];
    uint8_t expected_sequence = 0U;
    size_t file_size = 0U;
    size_t received = 0U;

    uint32_t retries = 0U;
    uart_putc(YM_CRC);
    for (;;) {
        uint8_t sequence = 0U;
        const int length = receive_packet(
            packet, &sequence, expected_sequence == 0U ? YM_INITIAL_TIMEOUT_MS : YM_PACKET_TIMEOUT_MS);

        if (length == -1) {
            return -2;
        }
        if (length == -2) {
            uart_putc(YM_NAK);
            retries++;
            if (retries == YM_MAX_RETRIES) {
                break;
            }
            continue;
        }
        if (length == 0) {
            if (expected_sequence == 0U || received != file_size) {
                uart_putc(YM_CAN);
                uart_putc(YM_CAN);
                return -3;
            }
            uart_putc(YM_ACK);
            uart_putc(YM_CRC);
            const int final_packet = receive_packet(packet, &sequence, YM_PACKET_TIMEOUT_MS);
            if (final_packet > 0 && sequence == 0U && packet[0] == '\0') {
                uart_putc(YM_ACK);
            }
            return (int32_t)received;
        }

        if (sequence != expected_sequence) {
            if (expected_sequence != 0U && sequence == (uint8_t)(expected_sequence - 1U)) {
                uart_putc(YM_ACK);
            } else {
                uart_putc(YM_NAK);
            }
            retries++;
            if (retries == YM_MAX_RETRIES) {
                break;
            }
            continue;
        }

        if (expected_sequence == 0U) {
            if (packet[0] == '\0' || !parse_size(packet, (size_t)length, &file_size) ||
                file_size > buf_size || file_size > INT32_MAX) {
                uart_putc(YM_CAN);
                uart_putc(YM_CAN);
                return -3;
            }

            if (filename != NULL) {
                size_t index = 0U;
                while (index + 1U < YMODEM_FILENAME_MAX && packet[index] != '\0') {
                    filename[index] = (char)packet[index];
                    index++;
                }
                filename[index] = '\0';
            }

            expected_sequence = 1U;
            retries = 0U;
            uart_putc(YM_ACK);
            uart_putc(YM_CRC);
            continue;
        }

        const size_t remaining = file_size - received;
        const size_t to_copy = remaining < (size_t)length ? remaining : (size_t)length;
        if (write_packet(buf + received, packet, to_copy) != 0) {
            uart_putc(YM_CAN);
            uart_putc(YM_CAN);
            return -5;
        }
        received += to_copy;
        expected_sequence++;
        retries = 0U;
        uart_putc(YM_ACK);
    }

    uart_putc(YM_CAN);
    uart_putc(YM_CAN);
    return -4;
}
