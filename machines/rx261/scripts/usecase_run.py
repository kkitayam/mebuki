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

def deploy_with_rfp(
    rfp_cli: str,
    *images: tuple[str, Optional[str]],
) -> bool:
    """
    Deploy images using rfp-cli.

    Args:
        rfp_cli: Path to rfp-cli executable
        images: Tuples of image file path and optional memory address.
            If address is None, the image is specified with -file.
            Otherwise, the image is specified with -bin.

    Returns:
        True if successful, False otherwise
    """
    try:
        cmd = [rfp_cli, "-d", "RX200", "-t", "e2l", "-if", "fine", "-a"]

        for image_file, address in images:
            if address is None:
                cmd.extend(["-file", image_file])
            else:
                cmd.extend(["-bin", address, image_file])

        logger.info(f"Deploying: {images}")
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
#        logger.info(result.stdout)
        logger.info("Deploy successful")
        return True

    except subprocess.CalledProcessError as e:
        logger.error(f"Failed to deploy: {images}")
        logger.error(f"stdout: {e.stdout}")
        logger.error(f"stderr: {e.stderr}")
        return False
    except Exception as e:
        logger.error(f"Unexpected error deploying {images}: {e}")
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
    images_to_deploy = [(args.boot_image, None)]
    if args.slot0_image:
        images_to_deploy.append((args.slot0_image, args.slot0_address))
    if args.slot1_image:
        images_to_deploy.append((args.slot1_image, args.slot1_address))
    
    if not deploy_with_rfp(args.rfp_cli, *images_to_deploy):
        logger.error("Failed to deploy images")
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
