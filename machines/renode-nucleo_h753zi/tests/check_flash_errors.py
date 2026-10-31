#!/usr/bin/env python3
"""Verify that a previous flash-controller error does not block later operations."""

import argparse
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "machines" / "renode-cm4" / "scripts"))
from run import read_vector_table  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", type=Path)
    parser.add_argument("other_bank", type=Path)
    args = parser.parse_args()
    renode = shutil.which("renode")
    if renode is None:
        parser.error("Renode executable not found")
    sp, pc = read_vector_table(args.image, 0x08000000)
    platform = ROOT / "machines" / "renode-nucleo_h753zi" / "renode" / "platform.repl"
    script = f"""
emulation SetGlobalAdvanceImmediately true
mach create
machine LoadPlatformDescription @{platform.as_posix()}
cpu FpuEnabled true
sysbus LoadSRecord @{args.image.resolve().as_posix()}
sysbus LoadSRecord @{args.other_bank.resolve().as_posix()}
cpu VectorTableOffset 0x08000000
cpu SP 0x{sp:X}
cpu PC 0x{pc:X}
sysbus.swappableFlash TriggerOperationError 1
sysbus.swappableFlash TriggerOperationError 2
emulation RunFor "00:00:02"
sysbus ReadDoubleWord 0x5200201C
sysbus ReadDoubleWord 0x08020000
sysbus ReadDoubleWord 0x08120000
quit
"""
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "flash-errors.resc"
        path.write_text(script, encoding="utf-8")
        result = subprocess.run(
            [renode, "--plain", "--disable-xwt", "--console", str(path)],
            capture_output=True, text=True, timeout=60, check=False,
        )
    values = re.findall(r"(?m)^0x[0-9A-Fa-f]{8}", result.stdout)
    expected = ["0x8406AAF0", "0x78563412", "0xFFFFFFFF"]
    if result.returncode != 0 or "There was an error" in result.stdout or values != expected:
        print(result.stdout, file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        print(f"Expected {expected}, got {values}", file=sys.stderr)
        return 1
    print("Both banks recover from latched errors and complete flash operations.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
