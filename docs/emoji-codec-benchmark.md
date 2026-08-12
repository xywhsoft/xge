# Emoji codec benchmark

Dataset: XGE Twemoji core, Unicode Emoji 17.0, 1,923 SVG files.

- Source size: 3,669,397 bytes.
- Corpus SHA-256:
  `7fdcce4daab14572cb32d3704912260b723824afd9b7eb2f2599f9cfbe28c1f3`.
- Platform: Windows x64.
- Zstd: 1.5.7 through Python zstandard 0.23.0.
- Deflate: zlib 1.3.1 and the miniz implementation embedded in XRT.
- XZ: Python 3.12 LZMA2, preset 9 extreme.

Reported package sizes include a 32/64 KiB trained dictionary and an 8-byte
index entry per independently addressable item or block. Compression time is
an offline build cost. Decode timings use warm process state and validate the
uncompressed bytes after compression.

## Whole corpus

| Codec | Package bytes | Source ratio | Decode MiB/s |
| --- | ---: | ---: | ---: |
| XRT Deflate 9 | 1,080,598 | 29.45% | 498.0 |
| Zstd 19 | 924,435 | 25.19% | 795.4 |
| XZ 9 extreme | 890,900 | 24.28% | 111.8 |

Trained dictionaries do not help the whole corpus because the corpus itself
already supplies a much larger history window. XZ saves only 33,535 bytes over
Zstd while decoding about seven times slower.

## Independently compressed SVG files

| Codec | Package bytes | Source ratio | Random decodes/s | Mean decode |
| --- | ---: | ---: | ---: | ---: |
| XRT Deflate 9 | 1,631,698 | 44.47% | 120,665 | 8.29 us |
| Zstd 19 | 1,595,162 | 43.47% | 191,623 | 5.22 us |
| XZ 9 extreme | 1,747,264 | 47.62% | 22,868 | 43.73 us |
| Zstd 19 + 32 KiB dictionary | 1,261,121 | 34.37% | 251,792 | 3.97 us |
| Zstd 19 + 64 KiB dictionary | 1,247,050 | 33.99% | 250,728 | 3.99 us |

The 64 KiB dictionary saves 14,071 bytes over the 32 KiB dictionary after the
larger dictionary itself is counted. The saving is too small to justify the
extra 32 KiB resident resource. The 32 KiB dictionary is the balanced choice.
Zstd level 22 was also checked and saved only 9 bytes over level 19 in the
32 KiB dictionary layout, so level 19 is retained.

## Resource blocks

| Block size | Codec | Package bytes | Source ratio | Decode MiB/s |
| ---: | --- | ---: | ---: | ---: |
| 64 KiB | Zstd 19 | 1,033,215 | 28.16% | 689.2 |
| 64 KiB | Zstd 19 + 32 KiB dictionary | 1,015,099 | 27.66% | 668.7 |
| 64 KiB | XZ 9 extreme | 1,013,528 | 27.62% | 84.5 |
| 256 KiB | Zstd 19 | 977,357 | 26.64% | 715.1 |
| 256 KiB | XZ 9 extreme | 942,364 | 25.68% | 96.8 |

For block storage, ordinary Zstd is preferred. A dictionary contributes little
once blocks are large enough to establish their own history.

## Decision

Use Zstd with a 32 KiB trained SVG dictionary when each Emoji must be directly
addressable. It reduces the package by about 326 KiB versus ordinary Zstd and
by about 362 KiB versus XRT Deflate while also producing the fastest random
decode result.

Use ordinary Zstd with 64 KiB resource blocks if one block decompression and a
small block cache are acceptable. It saves about 228 KiB compared with the
per-file dictionary layout, at the cost of decoding and caching unrelated SVGs
with each miss.

Do not use XZ for runtime Emoji resources. It wins the whole-corpus size test by
less than 1% of source size, but has much lower decode throughput and expensive
per-stream initialization. Keep XZ for offline archives only.

The official minified decoder configuration selected for XGE adds approximately
73 KiB in the current GCC build. It keeps compiler inlining, forces the X1
Huffman decoder and short sequence decoder, and removes compression, legacy
formats, error strings, and ASM. The more aggressive `ZSTD_NO_INLINE` build was
smaller but retained only about 59% of the normal decoder throughput, so it was
rejected.

The final production package has 1,923 independently addressable entries and is
1,294,188 bytes, including the real 24-byte entry index, UTF-8 sequence table,
32 KiB dictionary, compressed payload, and package header. The package is
embedded into XGE. Normal application builds and deployments do not require a
Zstandard compressor or an external Emoji file.

Re-run with:

```powershell
powershell -ExecutionPolicy Bypass -File tools/emoji_pack/run_codec_benchmark.ps1
```
