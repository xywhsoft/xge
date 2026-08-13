#!/usr/bin/env python3
"""Compress a premultiplied RGBA atlas for XUI's embedded texture."""

from __future__ import annotations

import argparse
from pathlib import Path

import zstandard as zstd


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--level", type=int, default=19)
    args = parser.parse_args()

    source = args.input.read_bytes()
    compressed = zstd.ZstdCompressor(
        level=args.level,
        write_checksum=False,
        write_content_size=True,
        write_dict_id=False,
    ).compress(source)
    decoded = zstd.ZstdDecompressor().decompress(compressed)
    if decoded != source:
        raise RuntimeError("Zstandard round trip failed")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(compressed)
    print(f"RGBA={len(source)} Zstd={len(compressed)} level={args.level}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
