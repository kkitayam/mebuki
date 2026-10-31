#!/usr/bin/env python3
"""Run Nucleo H753ZI firmware with the shared Renode runner."""

import sys
from pathlib import Path


MACHINE_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(MACHINE_DIR.parent / "renode-cm4" / "scripts"))
from run import main  # noqa: E402


if __name__ == "__main__":
    sys.exit(main([
        "--repl", str(MACHINE_DIR / "renode" / "platform.repl"),
        "--vector-base", "0x08000000",
        "--uart", "usart3",
        *sys.argv[1:],
    ]))
