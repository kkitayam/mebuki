#!/usr/bin/env python3
"""Deploy RX65N boot and application images, then capture UART output."""

import argparse
import logging
import subprocess
import sys


LOGGER = logging.getLogger(__name__)


def deploy(rfp_cli: str, boot_image: str, images: list[tuple[str, str]]) -> None:
    command = [
        rfp_cli, "-d", "RX65x", "-t", "e2l", "-if", "fine",
        "-auth", "id", "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", "-a",
        "-file", boot_image,
    ]
    for image, address in images:
        command += ["-bin", address, image]
    LOGGER.info("Deploying boot and application images")
    subprocess.run(command, check=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("serial_port")
    parser.add_argument("--boot-image", required=True)
    parser.add_argument("--slot0-image")
    parser.add_argument("--slot1-image")
    parser.add_argument("--slot0-address", default="0xFFE00000")
    parser.add_argument("--slot1-address", default="0xFFF00000")
    parser.add_argument("--build-dir", required=True)
    parser.add_argument("--rfp-cli", required=True)
    parser.add_argument("--python-exe", required=True)
    parser.add_argument("--run-script", required=True)
    parser.add_argument("--display", action="store_true")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="%(asctime)s - %(levelname)s - %(message)s")
    images = []
    if args.slot0_image:
        images.append((args.slot0_image, args.slot0_address))
    if args.slot1_image:
        images.append((args.slot1_image, args.slot1_address))
    try:
        deploy(args.rfp_cli, args.boot_image, images)
        command = [
            args.python_exe, args.run_script, args.serial_port,
            "--build-dir", args.build_dir, "--rfp-cli", args.rfp_cli,
        ]
        if args.display:
            command.append("--display")
        return subprocess.run(command, check=False).returncode
    except subprocess.CalledProcessError as error:
        LOGGER.error("Deployment failed with exit code %s", error.returncode)
        return error.returncode or 1


if __name__ == "__main__":
    sys.exit(main())
