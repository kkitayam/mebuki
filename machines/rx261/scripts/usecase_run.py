#!/usr/bin/env python3
"""
Run target implementation for FPB-RX261 use cases.

This script deploys boot and use-case-specific app images, then runs the application.
"""

import argparse
import logging
import subprocess
import sys
from pathlib import Path
from typing import Optional, List

logger = logging.getLogger(__name__)


def setup_logging(level=logging.INFO):
    """Configure logging."""
    logging.basicConfig(
        level=level,
        format="%(asctime)s - %(levelname)s - %(message)s"
    )


def deploy_with_rfp(rfp_cli: str, image_file: str, address: Optional[str] = None) -> bool:
    """
    Deploy an image using rfp-cli.

    Args:
        rfp_cli: Path to rfp-cli executable
        image_file: Path to image file
        address: Memory address (optional, only for binary files)

    Returns:
        True if successful, False otherwise
    """
    try:
        cmd = [rfp_cli, "-d", "RX200", "-t", "e2l", "-if", "fine", "-a", image_file]
        if address:
            cmd.extend(["-r", address])

        logger.info(f"Deploying: {image_file}" + (f" at {address}" if address else ""))
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        logger.info(f"Deploy successful: {image_file}")
        return True
    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to deploy {image_file}: {e}")
        logger.error(f"stdout: {e.stdout}")
        logger.error(f"stderr: {e.stderr}")
        return False
    except Exception as e:
        logger.error(f"Unexpected error deploying {image_file}: {e}")
        return False


def run_application(
    python_exe: str,
    run_script: str,
    serial_port: str,
    build_dir: str,
    rfp_cli: str,
    display: bool = False,
) -> int:
    """
    Run the application using the run.py script.

    Args:
        python_exe: Path to python executable
        run_script: Path to run.py script
        serial_port: Serial port for UART logging
        build_dir: Build directory
        rfp_cli: Path to rfp-cli executable
        display: If True, display UART logs to stdout

    Returns:
        Exit code
    """
    try:
        cmd = [
            python_exe,
            run_script,
            serial_port,
            "--build-dir", build_dir,
            "--rfp-cli", rfp_cli,
        ]
        if display:
            cmd.append("--display")

        logger.info(f"Starting application")
        result = subprocess.run(cmd)
        return result.returncode
    except Exception as e:
        logger.error(f"Failed to run application: {e}")
        return 1


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Run FPB-RX261 use case with image deployment"
    )
    parser.add_argument("serial_port", help="Serial port for UART logging")
    parser.add_argument("--boot-image", required=True, help="Path to boot SREC image")
    parser.add_argument("--slot0-image", help="Path to slot0 image (optional)")
    parser.add_argument("--slot1-image", help="Path to slot1 image (optional)")
    parser.add_argument("--slot0-address", default="0xFFF80000", help="Slot0 memory address")
    parser.add_argument("--slot1-address", default="0xFFFA0000", help="Slot1 memory address")
    parser.add_argument("--build-dir", required=True, help="Build directory path")
    parser.add_argument("--rfp-cli", required=True, help="Path to rfp-cli executable")
    parser.add_argument("--python-exe", required=True, help="Path to python executable")
    parser.add_argument("--run-script", required=True, help="Path to run.py script")
    parser.add_argument("--display", action="store_true", help="Display UART logs to stdout")

    args = parser.parse_args()
    setup_logging()

    # Deploy boot image
    if not deploy_with_rfp(args.rfp_cli, args.boot_image):
        logger.error("Failed to deploy boot image")
        return 1

    # Deploy slot0 image if provided
    if args.slot0_image:
        if not deploy_with_rfp(args.rfp_cli, args.slot0_image, args.slot0_address):
            logger.error("Failed to deploy slot0 image")
            return 1

    # Deploy slot1 image if provided
    if args.slot1_image:
        if not deploy_with_rfp(args.rfp_cli, args.slot1_image, args.slot1_address):
            logger.error("Failed to deploy slot1 image")
            return 1

    # Run application
    return run_application(
        args.python_exe,
        args.run_script,
        args.serial_port,
        args.build_dir,
        args.rfp_cli,
        args.display,
    )


if __name__ == "__main__":
    sys.exit(main())
