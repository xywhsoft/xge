#!/usr/bin/env python3
"""Benchmark whole-corpus and random-access compression for the Emoji SVG set."""

from __future__ import annotations

import argparse
import hashlib
import json
import lzma
import os
import platform
import random
import statistics
import sys
import time
import zlib
from pathlib import Path

import zstandard as zstd


BLOCK_HEADER_BYTES = 8
DEFAULT_ITERATIONS = 25
DEFAULT_RANDOM_PASSES = 40
ZSTD_LEVEL = 19
XZ_PRESET = 9 | lzma.PRESET_EXTREME
DICT_SIZES = (32768, 65536)
BLOCK_SIZES = (65536, 262144)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dataset", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--iterations", type=int, default=DEFAULT_ITERATIONS)
    parser.add_argument("--random-passes", type=int, default=DEFAULT_RANDOM_PASSES)
    return parser.parse_args()


def percentile(values: list[float], fraction: float) -> float:
    ordered = sorted(values)
    index = min(len(ordered) - 1, int((len(ordered) - 1) * fraction))
    return ordered[index]


def timing_summary(samples: list[float], byte_count: int) -> dict[str, float]:
    median = statistics.median(samples)
    return {
        "median_ms": median * 1000.0,
        "p95_ms": percentile(samples, 0.95) * 1000.0,
        "min_ms": min(samples) * 1000.0,
        "median_mib_s": (byte_count / (1024.0 * 1024.0)) / median,
    }


def measure(action, iterations: int) -> tuple[list[float], object]:
    action()
    samples: list[float] = []
    result = None
    for _ in range(iterations):
        start = time.perf_counter()
        result = action()
        samples.append(time.perf_counter() - start)
    return samples, result


def load_dataset(root: Path) -> tuple[list[tuple[str, bytes]], bytes, dict]:
    metadata = json.loads((root / "metadata.json").read_text(encoding="utf-8-sig"))
    records: list[tuple[str, bytes]] = []
    for line in (root / "manifest.txt").read_text(encoding="utf-8").splitlines():
        if not line:
            continue
        fields = line.split("|", 4)
        path = root / "svg" / fields[1]
        records.append((fields[1], path.read_bytes()))
    corpus = b"".join(data for _, data in records)
    expected_hash = metadata["files"]["corpus_sha256"]
    actual_hash = hashlib.sha256(corpus).hexdigest()
    if actual_hash != expected_hash:
        raise RuntimeError(f"corpus hash mismatch: {actual_hash} != {expected_hash}")
    return records, corpus, metadata


def train_dictionary(samples: list[bytes], size: int) -> bytes:
    return zstd.train_dictionary(size, samples).as_bytes()


def benchmark_whole(corpus: bytes, dictionaries: dict[int, bytes], iterations: int) -> list[dict]:
    zstd_compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL)
    zstd_decompressor = zstd.ZstdDecompressor()
    codecs = [
        (
            "deflate-9",
            lambda: zlib.compress(corpus, level=9),
            lambda data: zlib.decompress(data),
            0,
        ),
        (
            "zstd-19",
            lambda: zstd_compressor.compress(corpus),
            lambda data: zstd_decompressor.decompress(data),
            0,
        ),
        (
            "xz-9e",
            lambda: lzma.compress(corpus, format=lzma.FORMAT_XZ, preset=XZ_PRESET),
            lambda data: lzma.decompress(data, format=lzma.FORMAT_XZ),
            0,
        ),
    ]
    for size, dictionary in dictionaries.items():
        compression_dict = zstd.ZstdCompressionDict(dictionary)
        compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL, dict_data=compression_dict)
        decompressor = zstd.ZstdDecompressor(dict_data=compression_dict)
        codecs.append(
            (
                f"zstd-19-dict-{size // 1024}k",
                lambda compressor=compressor: compressor.compress(corpus),
                lambda data, decompressor=decompressor: decompressor.decompress(data),
                len(dictionary),
            )
        )

    results = []
    for name, compress, decompress, overhead in codecs:
        compression_samples, compressed = measure(compress, max(3, min(iterations, 5)))
        decompression_samples, restored = measure(lambda: decompress(compressed), iterations)
        if restored != corpus:
            raise RuntimeError(f"{name} whole-corpus round trip failed")
        payload = len(compressed)
        total = payload + overhead
        results.append(
            {
                "name": name,
                "payload_bytes": payload,
                "overhead_bytes": overhead,
                "total_bytes": total,
                "ratio": total / len(corpus),
                "compression": timing_summary(compression_samples, len(corpus)),
                "decompression": timing_summary(decompression_samples, len(corpus)),
            }
        )
    return results


def build_blocks(records: list[tuple[str, bytes]], limit: int) -> list[bytes]:
    blocks: list[bytes] = []
    current = bytearray()
    for _, data in records:
        if current and len(current) + len(data) > limit:
            blocks.append(bytes(current))
            current.clear()
        current.extend(data)
    if current:
        blocks.append(bytes(current))
    return blocks


def benchmark_blocks(
    records: list[tuple[str, bytes]],
    dictionaries: dict[int, bytes],
    iterations: int,
) -> list[dict]:
    results = []
    for block_size in BLOCK_SIZES:
        blocks = build_blocks(records, block_size)
        zstd_compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL)
        zstd_decompressor = zstd.ZstdDecompressor()
        codecs = [
            (
                "deflate-9",
                lambda data: zlib.compress(data, level=9),
                lambda data: zlib.decompress(data),
                0,
            ),
            (
                "zstd-19",
                lambda data: zstd_compressor.compress(data),
                lambda data: zstd_decompressor.decompress(data),
                0,
            ),
            (
                "xz-9e",
                lambda data: lzma.compress(data, format=lzma.FORMAT_XZ, preset=XZ_PRESET),
                lambda data: lzma.decompress(data, format=lzma.FORMAT_XZ),
                0,
            ),
        ]
        for size, dictionary in dictionaries.items():
            compression_dict = zstd.ZstdCompressionDict(dictionary)
            compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL, dict_data=compression_dict)
            decompressor = zstd.ZstdDecompressor(dict_data=compression_dict)
            codecs.append(
                (
                    f"zstd-19-dict-{size // 1024}k",
                    lambda data, compressor=compressor: compressor.compress(data),
                    lambda data, decompressor=decompressor: decompressor.decompress(data),
                    len(dictionary),
                )
            )

        for name, compress, decompress, overhead in codecs:
            start = time.perf_counter()
            compressed = [compress(block) for block in blocks]
            compression_seconds = time.perf_counter() - start

            def decompress_all():
                return [decompress(block) for block in compressed]

            samples, restored = measure(decompress_all, iterations)
            if restored != blocks:
                raise RuntimeError(f"{name} {block_size} block round trip failed")
            payload = sum(len(block) for block in compressed)
            index_bytes = len(blocks) * BLOCK_HEADER_BYTES
            total = payload + overhead + index_bytes
            source_bytes = sum(len(block) for block in blocks)
            results.append(
                {
                    "name": name,
                    "block_limit": block_size,
                    "block_count": len(blocks),
                    "payload_bytes": payload,
                    "dictionary_bytes": overhead,
                    "index_bytes": index_bytes,
                    "total_bytes": total,
                    "ratio": total / source_bytes,
                    "compression": timing_summary([compression_seconds], source_bytes),
                    "decompression": timing_summary(samples, source_bytes),
                }
            )
    return results


def benchmark_random_access(
    records: list[tuple[str, bytes]],
    dictionaries: dict[int, bytes],
    random_passes: int,
) -> list[dict]:
    rng = random.Random(0x584745)
    order = list(range(len(records))) * random_passes
    rng.shuffle(order)
    source_bytes = sum(len(records[index][1]) for index in order)
    source_total = sum(len(data) for _, data in records)
    zstd_compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL)
    zstd_decompressor = zstd.ZstdDecompressor()
    codecs = [
        (
            "deflate-9",
            lambda data: zlib.compress(data, level=9),
            lambda data: zlib.decompress(data),
            0,
        ),
        (
            "zstd-19",
            lambda data: zstd_compressor.compress(data),
            lambda data: zstd_decompressor.decompress(data),
            0,
        ),
        (
            "xz-9e",
            lambda data: lzma.compress(data, format=lzma.FORMAT_XZ, preset=XZ_PRESET),
            lambda data: lzma.decompress(data, format=lzma.FORMAT_XZ),
            0,
        ),
    ]
    for size, dictionary in dictionaries.items():
        compression_dict = zstd.ZstdCompressionDict(dictionary)
        compressor = zstd.ZstdCompressor(level=ZSTD_LEVEL, dict_data=compression_dict)
        decompressor = zstd.ZstdDecompressor(dict_data=compression_dict)
        codecs.append(
            (
                f"zstd-19-dict-{size // 1024}k",
                lambda data, compressor=compressor: compressor.compress(data),
                lambda data, decompressor=decompressor: decompressor.decompress(data),
                len(dictionary),
            )
        )

    results = []
    for name, compress, decompress, overhead in codecs:
        start = time.perf_counter()
        compressed = [compress(data) for _, data in records]
        compression_seconds = time.perf_counter() - start

        start = time.perf_counter()
        checksum = 0
        for index in order:
            restored = decompress(compressed[index])
            checksum ^= restored[0] if restored else 0
        random_seconds = time.perf_counter() - start
        if checksum < 0:
            raise AssertionError("unreachable")

        for index, (_, source) in enumerate(records):
            if decompress(compressed[index]) != source:
                raise RuntimeError(f"{name} per-file round trip failed at {index}")

        payload = sum(len(item) for item in compressed)
        index_bytes = len(records) * BLOCK_HEADER_BYTES
        total = payload + overhead + index_bytes
        results.append(
            {
                "name": name,
                "file_count": len(records),
                "payload_bytes": payload,
                "dictionary_bytes": overhead,
                "index_bytes": index_bytes,
                "total_bytes": total,
                "ratio": total / source_total,
                "compression": timing_summary([compression_seconds], source_total),
                "random_access": {
                    "operations": len(order),
                    "total_ms": random_seconds * 1000.0,
                    "operations_per_second": len(order) / random_seconds,
                    "mib_s": (source_bytes / (1024.0 * 1024.0)) / random_seconds,
                    "average_us": random_seconds * 1_000_000.0 / len(order),
                },
            }
        )
    return results


def main() -> int:
    args = parse_args()
    records, corpus, metadata = load_dataset(args.dataset)
    samples = [data for _, data in records]
    dictionaries = {size: train_dictionary(samples, size) for size in DICT_SIZES}
    args.output.mkdir(parents=True, exist_ok=True)
    for size, dictionary in dictionaries.items():
        (args.output / f"svg-{size // 1024}k.dict").write_bytes(dictionary)

    result = {
        "format_version": 1,
        "dataset": {
            "path": str(args.dataset.resolve()),
            "records": len(records),
            "bytes": len(corpus),
            "sha256": hashlib.sha256(corpus).hexdigest(),
            "selection": metadata["selection"]["description"],
        },
        "environment": {
            "platform": platform.platform(),
            "python": sys.version,
            "zlib": zlib.ZLIB_VERSION,
            "zstandard": zstd.__version__,
            "cpu_count": os.cpu_count(),
        },
        "parameters": {
            "iterations": args.iterations,
            "random_passes": args.random_passes,
            "zstd_level": ZSTD_LEVEL,
            "xz_preset": "9e",
            "dictionary_sizes": list(DICT_SIZES),
            "block_sizes": list(BLOCK_SIZES),
            "index_entry_bytes": BLOCK_HEADER_BYTES,
        },
        "dictionary_sha256": {
            str(size): hashlib.sha256(data).hexdigest() for size, data in dictionaries.items()
        },
        "whole_corpus": benchmark_whole(corpus, dictionaries, args.iterations),
        "blocked": benchmark_blocks(records, dictionaries, args.iterations),
        "per_file": benchmark_random_access(records, dictionaries, args.random_passes),
    }
    result_path = args.output / "results.json"
    result_path.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(result_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
