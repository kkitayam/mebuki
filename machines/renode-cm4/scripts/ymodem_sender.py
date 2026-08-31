#!/usr/bin/env python3
"""Minimal YMODEM-1K sender for a byte-stream serial transport."""

from __future__ import annotations

import struct
import time
from typing import Protocol

SOH = 0x01
STX = 0x02
EOT = 0x04
ACK = 0x06
NAK = 0x15
CAN = 0x18
CRC = ord("C")


class SerialTransport(Protocol):
    """Byte-stream transport used by the YMODEM sender."""

    def read(self, size: int = 1, timeout: float = 0.0) -> bytes: ...

    def write(self, data: bytes) -> None: ...

    def flush(self) -> None: ...


def crc16_ccitt(data: bytes) -> int:
    """Return the CRC-16-CCITT checksum used by YMODEM."""
    crc = 0
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def wait_for_byte(transport: SerialTransport, expected: int, timeout: float) -> bool:
    """Wait for one expected control byte."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        received = transport.read(1, max(0.0, deadline - time.monotonic()))
        if received and received[0] == expected:
            return True
    return False


def wait_response(transport: SerialTransport, timeout: float) -> int:
    """Return a YMODEM response control byte or -1 after a timeout."""
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        received = transport.read(1, max(0.0, deadline - time.monotonic()))
        if received and received[0] in (ACK, NAK, CAN, CRC):
            return received[0]
    return -1


def send_packet(transport: SerialTransport, sequence: int, payload: bytes, use_1k: bool) -> None:
    """Send a CRC-protected YMODEM packet."""
    packet_size = 1024 if use_1k else 128
    header = STX if use_1k else SOH
    padded = payload[:packet_size].ljust(packet_size, b"\0")
    packet = bytes((header, sequence & 0xFF, 0xFF - (sequence & 0xFF))) + padded
    transport.write(packet + struct.pack(">H", crc16_ccitt(padded)))
    transport.flush()


def ymodem_send(transport: SerialTransport, data: bytes, filename: str) -> bool:
    """Send one file and return whether the receiver completed the transfer."""
    if not wait_for_byte(transport, CRC, timeout=20.0):
        return False

    metadata = filename.encode("ascii", errors="replace") + b"\0" + str(len(data)).encode() + b"\0"
    send_packet(transport, 0, metadata, use_1k=False)
    if wait_response(transport, timeout=5.0) != ACK:
        return False
    if not wait_for_byte(transport, CRC, timeout=5.0):
        return False

    sequence = 1
    for offset in range(0, len(data), 1024):
        payload = data[offset:offset + 1024]
        for _ in range(10):
            send_packet(transport, sequence, payload, use_1k=len(payload) > 128)
            response = wait_response(transport, timeout=5.0)
            if response == ACK:
                break
            if response == CAN:
                return False
        else:
            return False
        sequence = (sequence + 1) & 0xFF

    for _ in range(3):
        transport.write(bytes((EOT,)))
        transport.flush()
        response = wait_response(transport, timeout=5.0)
        if response == ACK:
            break
        if response == CAN:
            return False
    else:
        return False

    if wait_for_byte(transport, CRC, timeout=2.0):
        send_packet(transport, 0, b"", use_1k=False)
        return wait_response(transport, timeout=2.0) == ACK
    return True
