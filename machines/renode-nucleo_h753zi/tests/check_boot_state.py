#!/usr/bin/env python3
"""Verify the security record after the higher-version bank swap."""

import argparse
import hashlib
import json
import re
import shutil
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "machines" / "renode-cm4" / "scripts"))
from run import read_vector_table  # noqa: E402


def run_monitor(renode: str, script: str) -> subprocess.CompletedProcess[str]:
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "boot-state.resc"
        path.write_text(script, encoding="utf-8")
        return subprocess.run(
            [renode, "--plain", "--disable-xwt", "--console", str(path)],
            capture_output=True, text=True, timeout=60, check=False,
        )


def decode_record(values: list[str]) -> bytes:
    return b"".join(int(word, 16).to_bytes(4, "little") for word in values)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", type=Path)
    args = parser.parse_args()
    examples = args.build_dir.resolve() / "examples"
    images = [
        examples / "boot" / "boot.srec",
        examples / "boot" / "boot_bank1.srec",
        examples / "higher_version_slot0.srec",
        examples / "higher_version_slot1.srec",
    ]
    for image in images:
        if not image.is_file():
            parser.error(f"Required image not found: {image}; run run_higher_version first")
    renode = shutil.which("renode")
    if renode is None:
        parser.error("Renode executable not found")
    sp, pc = read_vector_table(images[0], 0x08000000)
    platform = ROOT / "machines" / "renode-nucleo_h753zi" / "renode" / "platform.repl"
    load = "\n".join(f"sysbus LoadSRecord @{image.as_posix()}" for image in images)
    record_reads = "\n".join(
        f"sysbus ReadDoubleWord 0x{0x08020000 + offset:X}"
        for offset in range(0, 48, 4)
    )
    script = f"""
emulation SetGlobalAdvanceImmediately true
mach create
machine LoadPlatformDescription @{platform.as_posix()}
cpu FpuEnabled true
{load}
cpu VectorTableOffset 0x08000000
cpu SP 0x{sp:X}
cpu PC 0x{pc:X}
emulation RunFor "00:00:02"
{record_reads}
sysbus ReadDoubleWord 0x08120000
sysbus ReadDoubleWord 0x08040000
sysbus WriteDoubleWord 0xE000ED0C 0x05FA0004
emulation RunFor "00:00:02"
{record_reads}
sysbus ReadDoubleWord 0x08120000
sysbus ReadDoubleWord 0x08040000
quit
"""
    result = run_monitor(renode, script)
    values = re.findall(r"(?m)^0x[0-9A-Fa-f]{8}", result.stdout)
    if result.returncode != 0 or "There was an error" in result.stdout or len(values) != 28:
        print(result.stdout, file=sys.stderr)
        print(result.stderr, file=sys.stderr)
        print(f"Expected 28 state words, got {values}", file=sys.stderr)
        return 1

    records = [
        decode_record(values[start:start + 12])
        for start in (0, 14)
    ]
    options = json.loads((args.build_dir / "meson-info" / "intro-buildoptions.json").read_text())
    algorithm = next(option["value"] for option in options if option["name"] == "svl")
    signed_image = (examples / "app" / "app.v2.k0.img").read_bytes()
    software_size = int.from_bytes(signed_image[4:8], "little")
    image = signed_image[:8 + software_size]
    digest = hashlib.blake2s(image).digest() if algorithm == "fndsa" else hashlib.sha256(image).digest()
    for index, record in enumerate(records):
        if (record[:8] != bytes.fromhex("feffffff0200ffff") or
                record[12:44] != digest or
                int.from_bytes(record[44:48], "little") != zlib.crc32(record[:44])):
            print(f"Invalid BFL record after reset {index}: {record.hex()}", file=sys.stderr)
            return 1
    if records[0] != records[1] or values[12:14] != ["0xFFFFFFFF", "0xFF000002"] or \
            values[26:28] != ["0xFFFFFFFF", "0xFF000002"]:
        print(f"BFL or selected slot changed after reset: {values}", file=sys.stderr)
        return 1

    other_record_reads = record_reads.replace("0x802", "0x812")
    initial_load = "\n".join(f"sysbus LoadSRecord @{image.as_posix()}" for image in images[:3])
    recovery_script = f"""
emulation SetGlobalAdvanceImmediately true
mach create
machine LoadPlatformDescription @{platform.as_posix()}
cpu FpuEnabled true
{initial_load}
cpu VectorTableOffset 0x08000000
cpu SP 0x{sp:X}
cpu PC 0x{pc:X}
emulation RunFor "00:00:02"
sysbus LoadSRecord @{images[3].as_posix()}
sysbus WriteDoubleWord 0xE000ED0C 0x05FA0004
emulation RunFor "00:00:02"
{record_reads}
{other_record_reads}
sysbus WriteDoubleWord 0x0812002C 0
sysbus WriteDoubleWord 0xE000ED0C 0x05FA0004
emulation RunFor "00:00:02"
{record_reads}
{other_record_reads}
quit
"""
    recovery = run_monitor(renode, recovery_script)
    words = re.findall(r"(?m)^0x[0-9A-Fa-f]{8}", recovery.stdout)
    if recovery.returncode != 0 or "There was an error" in recovery.stdout or len(words) != 48:
        print(recovery.stdout, file=sys.stderr)
        print(recovery.stderr, file=sys.stderr)
        print(f"Expected 48 recovery words, got {words}", file=sys.stderr)
        return 1
    old, newest, old_after, newest_after = [
        decode_record(words[start:start + 12]) for start in (0, 12, 24, 36)
    ]
    for record in (old, newest, old_after, newest_after):
        if int.from_bytes(record[44:48], "little") != zlib.crc32(record[:44]):
            print(f"Invalid recovery record: {record.hex()}", file=sys.stderr)
            return 1
    if (old[:8] != bytes.fromhex("feffffff0100ffff") or
            newest[:8] != bytes.fromhex("fdffffff02000100") or
            newest[12:44] != digest or int.from_bytes(newest[44:48], "little") == 0 or
            old != old_after or newest != newest_after):
        print("The valid older BFL record did not restore the corrupted newer record.", file=sys.stderr)
        return 1
    print("The complete BFL record, image hash, CRC, and selected slot persist across resets.")
    print("A corrupted newer BFL record is recovered using the older valid record.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
