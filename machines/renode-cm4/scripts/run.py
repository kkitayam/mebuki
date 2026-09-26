#!/usr/bin/env python3
"""
Run target implementation for Renode Cortex-M4F use cases.

This script generates a temporary .resc at run time, starts Renode, captures
UART output from a TCP socket terminal, then stops Renode.
"""

from __future__ import annotations

import argparse
import logging
import os
import shutil
import socket
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path
from typing import IO, Optional

from ymodem_sender import ymodem_send

logger = logging.getLogger(__name__)

SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_REPL = SCRIPT_DIR.parent / "renode" / "platform.repl"
DEFAULT_DURATION_S = 5
SOCKET_WAIT_TIMEOUT_S = 30
SOCKET_RETRY_INTERVAL_S = 0.1
RENODE_QUIT_TIMEOUT_S = 5
UART_RECV_TIMEOUT_S = 0.2
OTA_COMPLETION_TIMEOUT_S = 120


def setup_logging(level: int = logging.INFO) -> None:
    """Configure logging."""
    logging.basicConfig(
        level=level,
        format="%(asctime)s - %(levelname)s - %(message)s",
    )


def resc_path(path: Path) -> str:
    """Return a Renode @-path with forward slashes."""
    return "@" + path.resolve().as_posix()


def find_free_tcp_port() -> int:
    """Select an available localhost TCP port."""
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.bind(("127.0.0.1", 0))
            sock.listen(1)
            return int(sock.getsockname()[1])
    except OSError as exc:
        raise RuntimeError(f"Failed to allocate a localhost TCP port: {exc}") from exc


def resolve_renode(renode: Optional[str]) -> Path:
    """Resolve the Renode executable path."""
    if renode:
        candidate = Path(renode)
        if candidate.is_file():
            return candidate.resolve()
        found = shutil.which(renode)
        if found:
            return Path(found).resolve()
        raise FileNotFoundError(f"Renode executable not found: {renode}")

    found = shutil.which("renode")
    if found:
        return Path(found).resolve()
    raise FileNotFoundError("Renode executable not found")


def require_file(path: Path, description: str) -> Path:
    """Return path if it exists as a file."""
    if not path.is_file():
        raise FileNotFoundError(f"{description} not found: {path}")
    return path.resolve()


def generate_resc(
    resc_path_out: Path,
    repl: Path,
    firmware: list[Path],
    socket_port: int,
) -> None:
    """Write a temporary Renode script. Do not start the CPU."""
    lines = [
        "emulation SetGlobalAdvanceImmediately true",
        "mach create",
        f"machine LoadPlatformDescription {resc_path(repl)}",
        "cpu FpuEnabled true",
        *[f"sysbus LoadSRecord {resc_path(image)}" for image in firmware],
    ]

    lines.extend(
        [
            f'emulation CreateServerSocketTerminal {socket_port} "uart" false',
            "connector Connect sysbus.uart0 uart",
        ]
    )

    try:
        resc_path_out.write_text("\n".join(lines) + "\n", encoding="utf-8")
    except OSError as exc:
        raise RuntimeError(f"Failed to generate .resc file: {exc}") from exc

    logger.info("Generated Renode script: %s", resc_path_out)


def wait_for_uart_socket(port: int, timeout_s: float = SOCKET_WAIT_TIMEOUT_S) -> socket.socket:
    """Retry connecting to the UART TCP socket until it is available."""
    deadline = time.monotonic() + timeout_s
    last_error: Optional[OSError] = None

    while time.monotonic() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1)
            sock.settimeout(UART_RECV_TIMEOUT_S)
            logger.info("Connected to UART socket on port %s", port)
            return sock
        except OSError as exc:
            last_error = exc
            time.sleep(SOCKET_RETRY_INTERVAL_S)

    raise ConnectionError(
        f"Failed to connect to UART socket on port {port}: {last_error}"
    )


class UartReader:
    """Read UART bytes on a background thread and write them to a file."""

    def __init__(self, sock: socket.socket, log_file: IO[bytes]) -> None:
        self._sock = sock
        self._log_file = log_file
        self._stop = threading.Event()
        self._error: Optional[BaseException] = None
        self._received = bytearray()
        self._captured = bytearray()
        self._received_condition = threading.Condition()
        self._thread = threading.Thread(target=self._run, name="uart-reader", daemon=True)

    def start(self) -> None:
        """Start the UART reader thread."""
        self._thread.start()

    def stop(self) -> None:
        """Signal the reader to stop and wait for it to finish."""
        self._stop.set()
        self._thread.join(timeout=2)

    def read(self, size: int = 1, timeout: float = 0.0) -> bytes:
        """Read captured UART bytes, waiting up to timeout seconds."""
        deadline = time.monotonic() + timeout
        with self._received_condition:
            while not self._received and not self._stop.is_set():
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    return b""
                self._received_condition.wait(remaining)
            data = bytes(self._received[:size])
            del self._received[:size]
            return data

    def write(self, data: bytes) -> None:
        """Write bytes to the UART socket."""
        self._sock.sendall(data)

    def flush(self) -> None:
        """Provide the serial transport flush interface."""

    def wait_for(self, marker: bytes, timeout: float) -> bool:
        """Wait until marker has appeared in UART output."""
        deadline = time.monotonic() + timeout
        with self._received_condition:
            while marker not in self._received and not self._stop.is_set():
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    return False
                self._received_condition.wait(remaining)
            return marker in self._received

    @property
    def failed(self) -> bool:
        """True if the reader stopped due to an error."""
        return self._error is not None

    def contains(self, text: str) -> bool:
        """Return whether the captured UART output contains text."""
        return text.encode() in self._captured

    @property
    def error(self) -> Optional[BaseException]:
        """Reader exception, if any."""
        return self._error

    def _run(self) -> None:
        try:
            while not self._stop.is_set():
                try:
                    data = self._sock.recv(4096)
                except TimeoutError:
                    continue
                except socket.timeout:
                    continue
                except OSError as exc:
                    if self._stop.is_set():
                        return
                    self._error = exc
                    logger.error("UART reader failed: %s", exc)
                    return

                if not data:
                    return

                self._log_file.write(data)
                self._log_file.flush()
                with self._received_condition:
                    self._received.extend(data)
                    self._captured.extend(data)
                    self._received_condition.notify_all()

                try:
                    text = data.decode("utf-8", errors="replace")
                    sys.stdout.write(text)
                    sys.stdout.flush()
                except OSError as exc:
                    self._error = exc
                    logger.error("Failed to write UART output to stdout: %s", exc)
                    return
        except BaseException as exc:
            self._error = exc
            logger.error("UART reader failed: %s", exc)


def drain_process_output(proc: subprocess.Popen[bytes], log_file: IO[bytes], stop: threading.Event) -> None:
    """Copy Renode stdout/stderr so the process cannot block on a full pipe."""
    try:
        while not stop.is_set():
            if proc.stdout is None:
                return
            data = proc.stdout.read(4096)
            if not data:
                return
            log_file.write(data)
            log_file.flush()
    except OSError:
        return


def send_monitor_command(proc: subprocess.Popen[bytes], command: str) -> None:
    """Send a command to the Renode Monitor on stdin."""
    if proc.stdin is None:
        raise RuntimeError("Renode stdin is not available")
    proc.stdin.write((command + "\n").encode("utf-8"))
    proc.stdin.flush()
    logger.info("Sent Renode Monitor command: %s", command)


def stop_renode(proc: subprocess.Popen[bytes]) -> None:
    """Stop Renode, first via Monitor quit, then terminate/kill if needed."""
    if proc.poll() is not None:
        return

    try:
        send_monitor_command(proc, "quit")
        if proc.stdin is not None:
            proc.stdin.close()
    except OSError as exc:
        logger.warning("Failed to send quit to Renode: %s", exc)

    try:
        proc.wait(timeout=RENODE_QUIT_TIMEOUT_S)
        return
    except subprocess.TimeoutExpired:
        logger.warning("Renode did not exit after quit; terminating")

    proc.terminate()
    try:
        proc.wait(timeout=RENODE_QUIT_TIMEOUT_S)
        return
    except subprocess.TimeoutExpired:
        logger.warning("Renode did not terminate; killing")
        proc.kill()
        proc.wait(timeout=RENODE_QUIT_TIMEOUT_S)


def run_renode(
    renode: Path,
    resc_file: Path,
    socket_port: int,
    log_file_path: Path,
    duration_s: int,
    ymodem_image: Optional[Path] = None,
    expected: Optional[str] = None,
) -> int:
    """Start Renode, capture UART, then stop and clean up."""
    uart_log_path = log_file_path
    renode_log_path = log_file_path.with_name(log_file_path.stem + "_renode.log")
    log_file_path.parent.mkdir(parents=True, exist_ok=True)

    proc: Optional[subprocess.Popen[bytes]] = None
    uart_sock: Optional[socket.socket] = None
    uart_log: Optional[IO[bytes]] = None
    renode_log: Optional[IO[bytes]] = None
    reader: Optional[UartReader] = None
    drain_stop = threading.Event()
    drain_thread: Optional[threading.Thread] = None

    try:
        uart_log = open(uart_log_path, "wb")
        renode_log = open(renode_log_path, "wb")

        try:
            command = [str(renode), "--console", str(resc_file)]
            if renode.suffix.lower() == ".py":
                command = [sys.executable, str(renode), "--console", str(resc_file)]
            proc = subprocess.Popen(
                command,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
            )
        except OSError as exc:
            logger.error("Failed to start Renode: %s", exc)
            return 1

        logger.info("Started Renode (PID: %s)", proc.pid)

        drain_thread = threading.Thread(
            target=drain_process_output,
            args=(proc, renode_log, drain_stop),
            name="renode-output",
            daemon=True,
        )
        drain_thread.start()

        if proc.poll() is not None:
            logger.error("Renode exited immediately with code %s", proc.returncode)
            return 1

        try:
            uart_sock = wait_for_uart_socket(socket_port)
        except ConnectionError as exc:
            logger.error("%s", exc)
            return 1

        reader = UartReader(uart_sock, uart_log)
        reader.start()

        try:
            send_monitor_command(proc, "start")
        except (OSError, RuntimeError) as exc:
            logger.error("Failed to start the CPU: %s", exc)
            return 1

        if ymodem_image is not None:
            if not reader.wait_for(b"APP_OTA: waiting for YMODEM image...", 20.0):
                logger.error("OTA receiver banner was not observed")
                return 1
            try:
                image_data = ymodem_image.read_bytes()
            except OSError as exc:
                logger.error("Failed to read OTA image: %s", exc)
                return 1
            logger.info("Sending signed OTA image: %s", ymodem_image)
            if not ymodem_send(reader, image_data, ymodem_image.name):
                logger.error("YMODEM transfer failed")
                return 1
            for marker in (b"APP_OTA: receive OK", b"Slot ID: 1", b"Scheduling slot swap", b"hello world"):
                if not reader.wait_for(marker, OTA_COMPLETION_TIMEOUT_S):
                    logger.error("OTA completion marker was not observed: %s", marker.decode())
                    return 1

        deadline = time.monotonic() + duration_s
        while time.monotonic() < deadline:
            if proc.poll() is not None:
                logger.error(
                    "Renode terminated abnormally with code %s", proc.returncode
                )
                return 1
            if reader.failed:
                logger.error("UART reader failed: %s", reader.error)
                return 1
            time.sleep(0.05)

        logger.info("Run duration elapsed (%s s)", duration_s)
        if reader.failed:
            logger.error("UART reader failed: %s", reader.error)
            return 1
        if expected is not None and not reader.contains(expected):
            logger.error("Expected UART text was not received: %s", expected)
            return 1
        return 0

    except Exception as exc:
        logger.error("Unexpected error while running Renode: %s", exc)
        return 1

    finally:
        if reader is not None:
            reader.stop()
            if reader.failed:
                logger.error("UART reader failed during cleanup: %s", reader.error)

        if uart_sock is not None:
            try:
                uart_sock.close()
            except OSError:
                pass

        if proc is not None:
            stop_renode(proc)

        drain_stop.set()
        if drain_thread is not None:
            drain_thread.join(timeout=2)

        if uart_log is not None:
            uart_log.close()
        if renode_log is not None:
            renode_log.close()

        logger.info("UART log saved to: %s", uart_log_path)


def parse_args(argv: Optional[list[str]] = None) -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Run a Renode Cortex-M4F use case with SREC firmware"
    )
    parser.add_argument(
        "--socket-port",
        type=int,
        help="TCP port for the UART socket (default: an available localhost port)",
    )
    parser.add_argument(
        "--firmware",
        action="append",
        help="Path to an SREC firmware image; may be specified multiple times",
    )
    parser.add_argument(
        "--ymodem-image",
        help="Signed image to send after the app_ota UART banner is observed",
    )
    parser.add_argument("--log-file", required=True, type=Path, help="Path for UART log output"    )
    parser.add_argument("--renode", help="Path to the Renode executable")
    parser.add_argument(
        "--duration",
        type=int,
        default=DEFAULT_DURATION_S,
        help=f"Run duration in seconds (default: {DEFAULT_DURATION_S})",
    )
    parser.add_argument("--expect", help="Require this text in the captured UART output")
    parser.add_argument(
        "--repl",
        default=str(DEFAULT_REPL),
        help="Path to the Renode platform REPL",
    )
    return parser.parse_args(argv)


def main(argv: Optional[list[str]] = None) -> int:
    """Main entry point."""
    args = parse_args(argv)
    setup_logging()

    resc_file: Optional[Path] = None

    try:
        if args.duration < 0:
            logger.error("Duration must be non-negative")
            return 1

        if not args.firmware:
            logger.error("At least one --firmware image is required")
            return 1

        try:
            renode = resolve_renode(args.renode)
            repl = require_file(Path(args.repl), "REPL")
            firmware = [
                require_file(Path(image), "Firmware image")
                for image in args.firmware
            ]
            ymodem_image = (
                require_file(Path(args.ymodem_image), "YMODEM image")
                if args.ymodem_image
                else None
            )
        except FileNotFoundError as exc:
            logger.error("%s", exc)
            return 1

        try:
            socket_port = args.socket_port if args.socket_port is not None else find_free_tcp_port()
        except RuntimeError as exc:
            logger.error("%s", exc)
            return 1

        if socket_port <= 0 or socket_port > 65535:
            logger.error("Invalid socket port: %s", socket_port)
            return 1

        logger.info("UART socket port: %s", socket_port)

        try:
            args.log_file.parent.mkdir(parents=True, exist_ok=True)
            fd, resc_name = tempfile.mkstemp(
                prefix="usecase_",
                suffix=".resc",
                dir=args.log_file.parent,
            )
            os.close(fd)
            resc_file = Path(resc_name)
            generate_resc(
                resc_file,
                repl,
                firmware,
                socket_port,
            )
        except RuntimeError as exc:
            logger.error("%s", exc)
            return 1

        return run_renode(
            renode,
            resc_file,
            socket_port,
            args.log_file,
            args.duration,
            ymodem_image=ymodem_image,
            expected=args.expect,
        )

    finally:
        if resc_file is not None:
            try:
                resc_file.unlink(missing_ok=True)
            except OSError as exc:
                logger.warning("Failed to remove temporary .resc file: %s", exc)


if __name__ == "__main__":
    sys.exit(main())
