# libunibreak for XUI text layout

Source: https://github.com/adah1972/libunibreak

- Release: `libunibreak_8_0` (8.0), released 2026-09-15.
- Commit: `28a2756b864c343f438cd22537d49d394d4666a5`.
- Release archive SHA256:
  `35f1008184c13de55793fa292b62a0c10739f1294f401a3b6a772edc145a4b3b`.
- Line breaking: Unicode **17.0.0**, UAX #14 revision 55, with upstream
  LB25 numeric handling and `-strict` CJ/NS handling.
- Vendored reference grapheme and supporting EAW/emoji data: Unicode **17.0.0**.
  Runtime extended graphemes now use XGE/XUI's shared Unicode **17.0.0** kernel.
- License: zlib/libpng-style license, preserved in `LICENCE` and sources.
  Earlier Unicode terms are preserved in `UNICODE-LICENSE.txt`; current
  Unicode License V3 is preserved in `lib/unicode/LICENSE`.

Only line/grapheme breaking and their dependencies are vendored. Runtime
sources and generated property tables are unchanged from the pinned release
(line endings/final newline normalized). This includes `linebreakauxdata.c`,
which supplies the General_Category information required by the new rules.
Official current property/test data is in `test_xui/data/unicode-17/`;
`test_xui/data/libunibreak/` retains the earlier 15.0/15.1 evidence.
No hand-authored Unicode property table is used.

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
and Linux GCC are verified here; other toolchains are not claimed.

## Conformance and policy bounds

The 8.0 line engine passes all **19,338** official Unicode 17 sequences,
including the formerly exceptional `U+1F02C U+1F3FF` sequence (LB30b).
No test is skipped and no deviation is accepted. The index test also expands
the official Line_Break property file independently and checks all 1,114,112
codepoints, every UTF-8 interior byte, and the actual XUI line/grapheme
intersection. Upstream's removed incremental public API is not restored;
the private adapter uses the new default/strict LB1 class resolver for both
map construction and hard-line scanning.

The runtime grapheme algorithm remains src/xge_unicode_grapheme.h, shared with
editing, font fallback and GDEF carets. Its generated Unicode 17 GCB/InCB/EP
table is covered by lib/unicode/LICENSE. All 766 official Unicode 17 grapheme
cases and full property expansion are tested; no hand-maintained ranges remain
in the editor's grapheme classification. Vendored upstream sources are unchanged.

The line algorithm includes AK/AP/AS/VF/VI/HH and LB28a orthographic-syllable
rules. WORD emergency wrapping is a separate, documented XUI policy;
CHAR wrapping deliberately relaxes UAX #14 restrictions. Dictionary-based
Thai/Lao/Khmer segmentation is not supplied. Bidi visual ordering and font
shaping remain the proxy's responsibility; XUI boundaries stay in logical
UTF-8 source order.

See `test_xui/TEXT_LINEBREAK.md` for the explicit XUI wrapping/display policy,
complexity evidence, tests, and integration requirements.
