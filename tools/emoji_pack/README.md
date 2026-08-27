# XGE built-in Emoji package

XGE embeds 1,923 Unicode Emoji 17.0 SVG graphics directly in the engine
binary. Applications do not load a sidecar resource and do not need to enable
Emoji support. The same text shaping path is used by ordinary XGE text, XUI
Input, TextEdit, and CodeEdit.

The production package uses:

- Twemoji 17.0.3 graphics from the pinned commit recorded in the dataset;
- one independently compressed Zstandard frame per SVG;
- Zstandard level 19 and a shared 32 KiB trained SVG dictionary;
- a private, official Zstandard 1.5.7 decompressor-only build;
- a versioned, bounds-checked package index;
- package-level CRC32 validation and Zstandard frame validation;
- lazy SVG decompression and parsing with a 256-entry / 16 MiB LRU cache.

Skin-tone modifiers fall back to the matching base Emoji while remaining one
text cluster. Exact custom pack entries still take priority. A corrupt package
or failed SVG is logged and reported through `xgeSetErrorCallback`. These
errors are recoverable: they do not abort a frame, and drawing falls back to
the font glyph where one exists.

## Production package

`build_builtin_pack.ps1` trains the dictionary deterministically, compresses
and verifies every source SVG, then writes:

- `artifacts/xge_builtin_emoji.xep`: binary inspection artifact, not shipped;
- `src/xge_builtin_emoji.inc`: embedded production data compiled into XGE.

Run from the repository root:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/emoji_pack/build_builtin_pack.ps1
```

If Python is not on `PATH`, pass its executable with
`-PythonPath C:\path\to\python.exe`. The decoder generation script accepts the
same option.

Python and the pinned `zstandard` package are build-maintenance dependencies
only. Normal XGE builds compile the checked-in generated include without
Python, network access, or the Zstandard compressor.

## Decoder generation

The private decoder is generated from the official Zstandard 1.5.7 source.
The script checks the release archive SHA-256 before running the upstream
single-file combiner:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/emoji_pack/generate_zstd_decoder.ps1
```

The generated decoder removes compression, legacy formats, error strings, ASM,
the second Huffman decoder, and the long sequence decoder. It retains compiler
inlining because the more aggressive `ZSTD_NO_INLINE` build was materially
slower in the native benchmark.

## Source dataset

`prepare_core_svg.ps1` recreates the versioned corpus. It selects all
fully-qualified Unicode 17.0 Emoji without skin-tone sequences and retains the
nine Unicode Emoji components. The often quoted 1,932 count belongs to an
earlier snapshot; this pinned dataset contains 1,923 records.

The source and license files are stored under
`res/emoji/twemoji_core/17.0`. Regenerate them with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/emoji_pack/prepare_core_svg.ps1
```

## Compression benchmark

The benchmark compares Deflate, Zstandard, trained dictionaries, and XZ for
whole-corpus, blocked, and independently addressable storage:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File tools/emoji_pack/run_codec_benchmark.ps1
```

Results are written under `artifacts/emoji_compression_benchmark`. See
`docs/EMOJI.md` for the runtime behavior exposed to applications.
