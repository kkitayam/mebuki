#!/usr/bin/env python3
"""
Run target implementation for FPB-RX261.

This script performs the following operations:
1. Opens a serial port to capture UART logs
2. Starts the FPB-RX261 application using rfp-cli
3. Captures UART logs for a specified duration
4. Closes the serial port and saves the log file
"""

import argparse
import logging
import subprocess
import sys
import time
from pathlib import Path
from typing import Optional

import serial

from ymodem_sender import ymodem_send


class SerialYmodemTransport:
    """Adapt SerialLogCapture to the YMODEM sender byte-stream interface."""

    def __init__(self, capture, log_file):
        self.capture = capture
        self.log_file = log_file
        self.buffer = bytearray()

    def read(self, size=1, timeout=0.0):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if self.buffer:
                data = bytes(self.buffer[:size])
                del self.buffer[:size]
                return data
            data = self.capture.read_uart(self.log_file)
            if data:
                self.capture.captured_uart.extend(data)
                self.buffer.extend(data)
            time.sleep(0.001)
        return b""

    def write(self, data):
        self.capture.serial.write(data)

    def flush(self):
        self.capture.serial.flush()


class SerialLogCapture:
    """Manages UART log capture and application execution."""

    def __init__(self, serial_port: str, build_dir: str, duration: int = 5, display: bool = False):
        """
        Initialize the serial log capture.

        Args:
            serial_port: Serial port name (e.g., COM3, /dev/ttyUSB0)
            build_dir: Build directory path for log file output
            duration: Log capture duration in seconds
            display: If True, also print logs to stdout
        """
        self.serial_port = serial_port
        self.build_dir = Path(build_dir)
        self.duration = duration
        self.log_file = self.build_dir / "uart.log"
        self.serial = None
        self.rfp_process = None
        self.display = display

        logging.basicConfig(
            level=logging.INFO,
            format="%(asctime)s - %(levelname)s - %(message)s"
        )
        self.logger = logging.getLogger(__name__)

    def open_serial_port(self) -> bool:
        """
        Open the serial port.

        Returns:
            True if successful, False otherwise
        """
        try:
            self.logger.info(f"Opening serial port: {self.serial_port}")
            self.serial = serial.Serial(
                port=self.serial_port,
                baudrate=115200,
                timeout=1
            )
            self.logger.info(f"Serial port opened successfully")
            return True
        except serial.SerialException as e:
            self.logger.error(f"Failed to open serial port {self.serial_port}: {e}")
            return False

    def close_serial_port(self) -> None:
        """Close the serial port."""
        if self.serial is not None:
            try:
                self.serial.close()
                self.logger.info("Serial port closed")
            except Exception as e:
                self.logger.error(f"Error closing serial port: {e}")

    def start_application(self, rfp_cli: str, rfp_args: list) -> bool:
        """
        Start the FPB-RX261 application using rfp-cli.

        Args:
            rfp_cli: Path to rfp-cli executable
            rfp_args: Arguments to pass to rfp-cli

        Returns:
            True if process started successfully, False otherwise
        """
        try:
            self.logger.info(f"Starting application with rfp-cli: {rfp_cli}")
            self.rfp_process = subprocess.Popen(
                [rfp_cli] + rfp_args,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE
            )
            self.logger.info(f"Application started (PID: {self.rfp_process.pid})")
            return True
        except Exception as e:
            self.logger.error(f"Failed to start application: {e}")
            return False

    def capture_logs(self) -> bool:
        """
        Capture UART logs for the specified duration.

        Returns:
            True if successful, False otherwise
        """
        try:
            self.logger.info(f"Starting log capture for {self.duration} seconds")
            self.log_file.parent.mkdir(parents=True, exist_ok=True)

            with open(self.log_file, "wb") as f:
                start_time = time.time()

                if self.ymodem_image is not None:
                    if not self.wait_for_marker(f, b"APP_OTA: waiting for YMODEM image...", 20):
                        self.logger.error("OTA receiver banner was not observed")
                        return False
                    image_path = Path(self.ymodem_image)
                    self.logger.info("Sending signed OTA image: %s", image_path)
                    if not ymodem_send(SerialYmodemTransport(self, f), image_path.read_bytes(), image_path.name):
                        self.logger.error("YMODEM transfer failed")
                        return False
                    for marker in (b"APP_OTA: receive OK", b"Slot ID: 1", b"Scheduling slot swap", b"hello world"):
                        if not self.wait_for_marker(f, marker, 20):
                            self.logger.error("OTA completion marker was not observed: %s", marker.decode())
                            return False

                while time.time() - start_time < self.duration:
                    if not self.read_uart(f):
                        time.sleep(0.01)

            self.logger.info(f"Log capture completed. Logs saved to: {self.log_file}")
            return True
        except Exception as e:
            self.logger.error(f"Failed to capture logs: {e}")
            return False

    def read_uart(self, log_file) -> bytes:
        """Read immediately available UART bytes and record them."""
        if self.serial is None or self.serial.in_waiting == 0:
            return b""
        data = self.serial.read(self.serial.in_waiting)
        log_file.write(data)
        log_file.flush()
        if self.display:
            sys.stdout.write(data.decode("utf-8", errors="replace"))
            sys.stdout.flush()
        return data

    def wait_for_marker(self, log_file, marker: bytes, timeout: float) -> bool:
        """Record UART output until marker is observed or timeout expires."""
        if not hasattr(self, "captured_uart"):
            self.captured_uart = bytearray()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            data = self.read_uart(log_file)
            if data:
                self.captured_uart.extend(data)
            if marker in self.captured_uart:
                return True
            time.sleep(0.01)
        return False

    def wait_for_application(self) -> int:
        """
        Wait for the application process to complete.

        Returns:
            Return code of the process
        """
        if self.rfp_process is None:
            return 0

        try:
            return_code = self.rfp_process.wait()
            self.logger.info(f"Application exited with return code: {return_code}")
            return return_code
        except Exception as e:
            self.logger.error(f"Error waiting for application: {e}")
            return 1

    def terminate_application(self) -> None:
        """Terminate the application process if it is still running."""
        if self.rfp_process is not None:
            try:
                if self.rfp_process.poll() is None:
                    self.logger.info("Terminating application process")
                    self.rfp_process.terminate()
                    self.rfp_process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.logger.warning("Application process did not terminate; killing it")
                self.rfp_process.kill()
            except Exception as e:
                self.logger.error(f"Error terminating application: {e}")

    def run(self, rfp_cli: str, rfp_args: list, ymodem_image: Optional[str] = None) -> int:
        """
        Execute the full log capture and application run sequence.

        Args:
            rfp_cli: Path to rfp-cli executable
            rfp_args: Arguments to pass to rfp-cli

        Returns:
            Exit code (0 for success, 1 for failure)
        """
        try:
            self.ymodem_image = ymodem_image
            if not self.open_serial_port():
                return 1

            if not self.start_application(rfp_cli, rfp_args):
                self.close_serial_port()
                return 1

            if not self.capture_logs():
                self.close_serial_port()
                self.terminate_application()
                return 1

            self.terminate_application()
            self.close_serial_port()
            return 0

        except Exception as e:
            self.logger.error(f"Unexpected error: {e}")
            self.close_serial_port()
            self.terminate_application()
            return 1


def main() -> int:
    """
    Main entry point.

    Returns:
        Exit code
    """
    parser = argparse.ArgumentParser(
        description="Run FPB-RX261 application with UART log capture"
    )
    parser.add_argument(
        "serial_port",
        help="Serial port for UART logging (e.g., COM3, /dev/ttyUSB0)"
    )
    parser.add_argument("--ymodem-image", help="Signed image to send after the app_ota banner")
    parser.add_argument(
        "--build-dir",
        required=True,
        help="Build directory path"
    )
    parser.add_argument(
        "--rfp-cli",
        required=True,
        help="Path to rfp-cli executable"
    )
    parser.add_argument(
        "--duration",
        type=int,
        default=5,
        help="Log capture duration in seconds (default: 5)"
    )
    parser.add_argument(
        "--display",
        action="store_true",
        help="Display UART logs to stdout in addition to saving to file"
    )

    args = parser.parse_args()

    if not args.serial_port:
        print("Error: serial_port option is required", file=sys.stderr)
        return 1

    capture = SerialLogCapture(args.serial_port, args.build_dir, args.duration, args.display)

    rfp_args = [
        "-d", "RX200",
        "-t", "e2l",
        "-if", "fine",
        "-run"
    ]

    return capture.run(args.rfp_cli, rfp_args, args.ymodem_image)


if __name__ == "__main__":
    sys.exit(main())
