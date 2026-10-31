#!/usr/bin/env python3
"""Check shared Renode runner vector parsing and ordered UART expectations."""

import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import Mock


SCRIPT_DIR = Path(__file__).resolve().parents[2] / "renode-cm4" / "scripts"
sys.path.insert(0, str(SCRIPT_DIR))
spec = importlib.util.spec_from_file_location("renode_runner", SCRIPT_DIR / "run.py")
runner = importlib.util.module_from_spec(spec)
spec.loader.exec_module(runner)


class RunnerTest(unittest.TestCase):
    def test_vector_table_at_nonzero_address(self):
        with tempfile.TemporaryDirectory() as directory:
            image = Path(directory) / "boot.srec"
            image.write_text("S30D08000000000002206502000859\n", encoding="ascii")
            self.assertEqual(runner.read_vector_table(image, 0x08000000), (0x20020000, 0x08000265))
            with self.assertRaises(RuntimeError):
                runner.read_vector_table(image)

    def test_expected_markers_are_ordered(self):
        reader = runner.UartReader(Mock(), Mock())
        reader._captured.extend(b"boot 1 swap boot 0 hello")
        self.assertTrue(reader.contains_in_order(["boot 1", "swap", "boot 0", "hello"]))
        self.assertFalse(reader.contains_in_order(["boot 0", "swap"]))
        self.assertFalse(reader.contains_in_order(["missing"]))

    def test_discard_banner_preserves_ymodem_handshake(self):
        reader = runner.UartReader(Mock(), Mock())
        marker = b"APP_OTA: waiting for YMODEM image..."
        reader._received.extend(b"Software Size: 0xA6C\r\n" + marker + b"\r\nC")
        reader.discard_through(marker)
        self.assertEqual(bytes(reader._received), b"\r\nC")
        with self.assertRaises(RuntimeError):
            reader.discard_through(marker)


if __name__ == "__main__":
    unittest.main()
