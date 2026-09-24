#!/usr/bin/env python3
"""Deploy an MSP432E401Y image with dslite and observe its UART output."""

import argparse
import logging
import subprocess
import sys
import time
from pathlib import Path
from typing import BinaryIO

import serial

from ymodem_sender import ymodem_send


class SerialTransport:
    """Adapt pyserial to the timeout-aware YMODEM transport interface."""

    def __init__(self, serial_port: serial.Serial) -> None:
        self.serial_port = serial_port

    def read(self, size: int, timeout: float) -> bytes:
        previous_timeout = self.serial_port.timeout
        self.serial_port.timeout = max(0.0, timeout)
        try:
            return self.serial_port.read(size)
        finally:
            self.serial_port.timeout = previous_timeout

    def write(self, data: bytes) -> int:
        return self.serial_port.write(data)

    def flush(self) -> None:
        self.serial_port.flush()


class SerialLogCapture:
    """Deploy an image and collect UART output."""

    def __init__(
        self,
        serial_port: str,
        dslite: str,
        ccxml: str,
        firmware: list[str],
        duration: int,
        log_file: Path | None,
        expected: str | None,
        ymodem_image: Path | None,
    ) -> None:
        self.serial_port = serial_port
        self.dslite = dslite
        self.ccxml = ccxml
        self.firmware = firmware
        self.duration = duration
        self.log_file = log_file
        self.expected = expected
        self.ymodem_image = ymodem_image
        self.ota_ready = False
        self.captured = bytearray()
        self.serial_port_handle: serial.Serial | None = None
        self.logger = logging.getLogger(__name__)

    def open_serial_port(self) -> None:
        """Open the configured UART port."""
        self.logger.info("Opening serial port: %s", self.serial_port)
        self.serial_port_handle = serial.Serial(
            port=self.serial_port,
            baudrate=115200,
            timeout=0,
        )

    def close_serial_port(self) -> None:
        """Close the UART port if it is open."""
        if self.serial_port_handle is not None:
            self.serial_port_handle.close()
            self.serial_port_handle = None

    def capture_available(self, log_file: BinaryIO | None) -> None:
        """Display and optionally save all UART bytes currently available."""
        if self.serial_port_handle is None:
            return

        byte_count = self.serial_port_handle.in_waiting
        if byte_count == 0:
            return

        read_count = 1 if self.ymodem_image is not None and not self.ota_ready else byte_count
        data = self.serial_port_handle.read(read_count)
        self.captured.extend(data)
        if log_file is not None:
            log_file.write(data)
            log_file.flush()

        sys.stdout.write(data.decode("utf-8", errors="replace"))
        sys.stdout.flush()
        if self.ymodem_image is not None and b"APP_OTA: waiting for YMODEM image" in self.captured:
            self.ota_ready = True

    def deploy(self, log_file: BinaryIO | None) -> int:
        """Run dslite while observing UART output."""
        command = [self.dslite, "-c", self.ccxml, "-u", *self.firmware]
        self.logger.info("Deploying firmware: %s", " ".join(command))
        try:
            process = subprocess.Popen(command)
        except OSError as error:
            self.logger.error("Failed to start dslite: %s", error)
            return 1

        while process.poll() is None:
            self.capture_available(log_file)
            time.sleep(0.01)

        if self.ymodem_image is not None and not self.ota_ready:
            deadline = time.monotonic() + 10.0
            while time.monotonic() < deadline and not self.ota_ready:
                self.capture_available(log_file)
                time.sleep(0.01)

        if self.ymodem_image is not None and self.ota_ready:
            if self.send_ymodem():
                self.ymodem_image = None
            else:
                return 1
        else:
            self.capture_available(log_file)
        if process.returncode != 0:
            self.logger.error("dslite exited with return code: %d", process.returncode)
            return process.returncode

        return 0

    def send_ymodem(self) -> bool:
        """Send the configured OTA image after the receiver banner."""
        if self.ymodem_image is None or self.serial_port_handle is None:
            return False
        image = self.ymodem_image.read_bytes()
        self.logger.info("Sending YMODEM image: %s", self.ymodem_image)
        if not ymodem_send(
            SerialTransport(self.serial_port_handle),
            image,
            self.ymodem_image.name,
        ):
            self.logger.error("YMODEM transfer failed")
            return False
        return True

    def capture_for_duration(self, log_file: BinaryIO | None) -> None:
        """Capture UART output for the configured duration."""
        deadline = time.monotonic() + self.duration
        while time.monotonic() < deadline:
            self.capture_available(log_file)
            time.sleep(0.01)
        self.capture_available(log_file)

    def run(self) -> int:
        """Open UART, deploy firmware, and optionally write a UART log."""
        try:
            self.open_serial_port()
        except serial.SerialException as error:
            self.logger.error(
                "Failed to open serial port %s: %s", self.serial_port, error
            )
            return 1

        try:
            if self.log_file is None:
                return self.deploy(None)

            self.log_file.parent.mkdir(parents=True, exist_ok=True)
            with self.log_file.open("wb") as file:
                deploy_status = self.deploy(file)
                if deploy_status != 0:
                    return deploy_status

                self.capture_for_duration(file)
                if self.expected is not None and self.expected.encode() not in self.captured:
                    self.logger.error("Expected UART text was not received: %s", self.expected)
                    return 1
                self.logger.info("UART log saved to: %s", self.log_file)
                return 0
        except (OSError, serial.SerialException) as error:
            self.logger.error("UART capture failed: %s", error)
            return 1
        finally:
            self.close_serial_port()


def main() -> int:
    """Parse command-line arguments and deploy firmware."""
    parser = argparse.ArgumentParser(
        description="Deploy MSP432E401Y firmware with dslite and observe UART output"
    )
    parser.add_argument(
        "serial_port",
        help="Serial port for UART output (for example, COM5)",
    )
    parser.add_argument(
        "--dslite",
        required=True,
        help="Path to the dslite executable",
    )
    parser.add_argument(
        "--ccxml",
        required=True,
        help="Path to the target configuration file",
    )
    parser.add_argument(
        "--firmware",
        action="append",
        required=True,
        help="Path to a firmware file; may be specified multiple times",
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=5,
        help="UART capture duration after deployment in seconds (default: 5)",
    )
    parser.add_argument(
        "--log-file",
        type=Path,
        help="Write UART output to this file",
    )
    parser.add_argument(
        "--expect",
        help="Require this text in the captured UART output",
    )
    parser.add_argument(
        "--ymodem-image",
        type=Path,
        help="Signed image to send after the OTA receiver starts",
    )
    args = parser.parse_args()

    if args.duration < 0:
        parser.error("--duration must not be negative")

    if hasattr(sys.stdout, "reconfigure"):
        sys.stdout.reconfigure(errors="replace")
    logging.basicConfig(level=logging.INFO, format="%(levelname)s: %(message)s")
    capture = SerialLogCapture(
        args.serial_port,
        args.dslite,
        args.ccxml,
        args.firmware,
        args.duration,
        args.log_file,
        args.expect,
        args.ymodem_image,
    )
    return capture.run()


if __name__ == "__main__":
    sys.exit(main())
