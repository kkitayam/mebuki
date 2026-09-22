"""YMODEM-1K sender for the MSP432E4 OTA runner."""

import struct
import time


SOH, STX, EOT, ACK, NAK, CAN, CRC = 1, 2, 4, 6, 21, 24, ord("C")


def crc16_ccitt(data: bytes) -> int:
    crc = 0
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc


def _response(transport, accepted: tuple[int, ...], timeout: float) -> int:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        data = transport.read(1, min(1.0, deadline - time.monotonic()))
        if data and data[0] in accepted:
            return data[0]
    return -1


def _packet(transport, sequence: int, payload: bytes, use_1k: bool) -> None:
    size = 1024 if use_1k else 128
    payload = payload[:size].ljust(size, b"\0")
    transport.write(
        bytes((STX if use_1k else SOH, sequence, 0xFF - sequence))
        + payload
        + struct.pack(">H", crc16_ccitt(payload))
    )
    transport.flush()


def ymodem_send(transport, data: bytes, filename: str) -> bool:
    """Send one file and return whether the receiver accepted it."""
    if _response(transport, (CRC,), 90) != CRC:
        return False
    _packet(transport, 0, filename.encode("ascii", "replace") + b"\0" +
            str(len(data)).encode() + b"\0", False)
    if _response(transport, (ACK, CAN), 5) != ACK or _response(transport, (CRC, CAN), 5) != CRC:
        return False
    for sequence, offset in enumerate(range(0, len(data), 1024), 1):
        payload = data[offset:offset + 1024]
        for _ in range(10):
            _packet(transport, sequence & 0xFF, payload, len(payload) > 128)
            response = _response(transport, (ACK, NAK, CAN), 5)
            if response == ACK:
                break
            if response == CAN:
                return False
        else:
            return False
    for _ in range(3):
        transport.write(bytes((EOT,)))
        transport.flush()
        response = _response(transport, (ACK, NAK, CAN), 5)
        if response == ACK:
            break
        if response == CAN:
            return False
    else:
        return False
    if _response(transport, (CRC,), 2) == CRC:
        _packet(transport, 0, b"", False)
        return _response(transport, (ACK,), 2) == ACK
    return True
