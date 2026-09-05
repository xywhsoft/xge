# libunibreak for XUI text layout

Source: https://github.com/adah1972/libunibreak

- Release: `libunibreak_7_0` (7.0).
- Commit: `3ce4bfa3129ff3738046a44a6db533d2ce25af2b`.
- Line breaking: Unicode **15.0.0**, UAX #14 revision 49, with upstream
  Example 7 LB25 numeric tailoring and `-strict` CJ/NS handling.
- Grapheme breaking and supporting EAW/emoji data: Unicode **15.1.0**.
- License: zlib/libpng-style license, preserved in `LICENCE` and sources.
  Unicode data/test terms are preserved in `UNICODE-LICENSE.txt`.

Only line/grapheme breaking and their dependencies are included. Runtime
sources and generated property tables are unchanged from the pinned release
(line endings/final newline normalized). Official upstream test data is in
`test_xui/data/libunibreak/`. No hand-authored Unicode property table is used.

`src/xui_text_break.inl` includes these C files in the text translation unit,
renaming external symbols with the private `__xuiUb` prefix. The names are not
part of XUI's public API or DLL export list. This avoids adding link requirements
to every existing standalone text/MessageList build. Do not compile these
vendored `.c` files a second time into the same target.

The adapter uses XRT's checked UTF-8 decoder, replacing each malformed byte by
U+FFFD for boundary analysis while retaining the original source bytes. It
passes this reader to the upstream streaming algorithms. Neither global
initialization nor a mutable singleton is needed. The runtime uses C99 and
the existing XRT allocator/decoder; it has no ICU, Windows text service,
locale, C++, network, or additional runtime-DLL dependency. Windows MinGW
is verified here; other toolchains have not been exercised by this change.

## Conformance limitation

The 7.0 line engine passes 7,653/7,654 Unicode 15.0 test sequences. Upstream
documents one XX-to-AL exception: `U+1F02C U+1F3FF` is allowed to break by the
engine, whereas the official test expects no break (LB30b). Our regression
executes this case and asserts the exact known deviation rather than silently
skipping it. The grapheme engine passes all 1,187 Unicode 15.1 sequences.

This is **not** a Unicode 15.1/16/17 UAX #14 implementation. In particular,
newer LB28a orthographic-syllable rules are not claimed. Dictionary-based
Thai/Lao/Khmer segmentation is not supplied. Bidi visual ordering and font
shaping remain the proxy's responsibility; XUI boundaries stay in logical
UTF-8 source order.

See `test_xui/TEXT_LINEBREAK.md` for the explicit XUI wrapping/display policy,
complexity evidence, tests, and integration requirements.
