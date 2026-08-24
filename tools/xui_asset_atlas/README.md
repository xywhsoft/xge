# XUI built-in atlas generator

`generate.bat` builds the C generator and converts
`res/xui_builtin_atlas.png` into `src/xui_builtin_atlas.c`.

The generated source contains premultiplied RGBA8 pixels compressed as a
Zstandard frame at compression level 19. The generator performs a decompression
round trip before replacing the output file.

The full Zstandard compressor in this directory is used by the offline tool
only. XGE/XUI runtime builds continue to use `lib/zstd/zstddeclib.c`, the
official compact decoder. The vendored Zstandard files are version 1.5.7 and
are covered by `lib/zstd/LICENSE`.

Run from the repository root:

```bat
tools\xui_asset_atlas\generate.bat
```
