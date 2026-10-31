#!/usr/bin/env python3
"""Exercise bank mapping, reset persistence, and bank-specific sector erase."""

import re
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path


PLATFORM = Path(__file__).resolve().parents[1] / "renode" / "platform.repl"


class FlashModelTest(unittest.TestCase):
    def test_swap_reset_and_erase(self):
        renode = shutil.which("renode")
        self.assertIsNotNone(renode, "Renode executable is required")
        script = f"""
mach create
machine LoadPlatformDescription @{PLATFORM.as_posix()}
sysbus WriteDoubleWord 0x08000000 0x11111111
sysbus WriteDoubleWord 0x08100000 0x22222222
sysbus WriteDoubleWord 0x08020000 0x33333333
sysbus WriteDoubleWord 0x08120000 0x44444444
sysbus WriteDoubleWord 0x5200200C 0x184
sysbus ReadDoubleWord 0x08020000
sysbus WriteDoubleWord 0x52002020 0x8406AAF0
sysbus ReadDoubleWord 0x52002020
sysbus WriteDoubleWord 0x52002008 0x08192A3B
sysbus WriteDoubleWord 0x52002008 0x4C5D6E7F
sysbus WriteDoubleWord 0x52002020 0x80000000
sysbus.swappableFlash Reset
sysbus ReadDoubleWord 0x5200201C
sysbus ReadDoubleWord 0x00000000
sysbus WriteDoubleWord 0x52002008 0x08192A3B
sysbus WriteDoubleWord 0x52002008 0x4C5D6E7F
sysbus WriteDoubleWord 0x52002020 0x8406AAF0
sysbus WriteDoubleWord 0x52002018 0x2
sysbus ReadDoubleWord 0x08000000
sysbus.swappableFlash Reset
sysbus ReadDoubleWord 0x00000000
sysbus ReadDoubleWord 0x08000000
sysbus ReadDoubleWord 0x08100000
sysbus ReadDoubleWord 0x08020000
sysbus ReadDoubleWord 0x08120000
sysbus.swappableFlash Reset
sysbus ReadDoubleWord 0x00000000
sysbus WriteDoubleWord 0x52002004 0x45670123
sysbus WriteDoubleWord 0x52002004 0xCDEF89AB
sysbus WriteDoubleWord 0x5200200C 0x184
sysbus ReadDoubleWord 0x08020000
sysbus ReadDoubleWord 0x08120000
sysbus WriteDoubleWord 0x52002008 0x08192A3B
sysbus WriteDoubleWord 0x52002008 0x4C5D6E7F
sysbus WriteDoubleWord 0x52002020 0x0406AAF0
sysbus WriteDoubleWord 0x52002018 0x2
sysbus.swappableFlash Reset
sysbus ReadDoubleWord 0x00000000
sysbus ReadDoubleWord 0x08020000
sysbus ReadDoubleWord 0x08120000
quit
"""
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "flash-test.resc"
            path.write_text(script, encoding="utf-8")
            result = subprocess.run(
                [renode, "--plain", "--disable-xwt", "--console", str(path)],
                capture_output=True, text=True, timeout=60, check=False,
            )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertNotIn("There was an error", result.stdout)
        values = re.findall(r"(?m)^0x[0-9A-Fa-f]{8}", result.stdout)
        self.assertEqual(values, [
            "0x33333333",
            "0x0406AAF0", "0x0406AAF0", "0x11111111",
            "0x11111111",  # A programmed option does not swap until reset.
            "0x22222222", "0x22222222", "0x11111111",
            "0x44444444", "0x33333333",
            "0x22222222",  # A second reset retains the selected bank.
            "0xFFFFFFFF", "0x33333333",  # Erase only the selected logical bank.
            "0x11111111", "0x33333333", "0xFFFFFFFF",
        ], result.stdout)


if __name__ == "__main__":
    unittest.main()
