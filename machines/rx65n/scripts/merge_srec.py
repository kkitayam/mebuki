#!/usr/bin/env python3
"""Merge S-record data records while preserving their load addresses."""

import argparse
from pathlib import Path


def checksum(record: bytes) -> int:
    return (~(sum(record) & 0xFF)) & 0xFF


def parse_record(line: str) -> tuple[int, bytes] | None:
    line = line.strip()
    if not line.startswith("S") or len(line) < 4:
        return None
    record_type = int(line[1], 16)
    raw = bytes.fromhex(line[2:])
    count = raw[0]
    if count != len(raw) - 1 or checksum(raw[:-1]) != raw[-1]:
        raise ValueError(f"invalid S-record: {line}")
    return record_type, raw


def merge(inputs: list[Path], output: Path) -> None:
    data_records: list[bytes] = []
    termination: tuple[int, bytes] | None = None

    for input_path in inputs:
        for line in input_path.read_text(encoding="ascii").splitlines():
            parsed = parse_record(line)
            if parsed is None:
                continue
            record_type, raw = parsed
            if record_type == 3:
                data_records.append(raw)
            elif record_type in (7, 8, 9) and termination is None:
                termination = (record_type, raw)

    if termination is None:
        raise ValueError("no S-record termination record found")

    lines = ["S3" + raw.hex().upper() for raw in data_records]
    termination_type, termination_raw = termination
    lines.append(f"S{termination_type}{termination_raw.hex().upper()}")
    output.write_text("\n".join(lines) + "\n", encoding="ascii")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("inputs", nargs=2, type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    merge(args.inputs, args.output)


if __name__ == "__main__":
    main()
