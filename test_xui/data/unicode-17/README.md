# Unicode 17 boundary data and corpora

Source: https://www.unicode.org/Public/17.0.0/ucd/auxiliary/GraphemeBreakTest.txt

- Official raw SHA256: `e2d134d2c52919bace503ebb6a551c1855fe1a1faec18478c78fff254a1793ec`.
- Repository SHA256: `88ead7e1c192b5d9767eb4c54cb62e177ab82afd972efc247e04599fab66b719`.
- Only trailing whitespace on comment lines was removed. All 766 test records,
  codepoints, boundary markers, comments and license attribution remain intact.
- Unicode License V3 is preserved at repository path `lib/unicode/LICENSE`.

`test_xui/build_unicode_grapheme_test.bat` checks every UTF-8 byte against the
official boundaries using both contiguous and callback-based navigation. The
actual layout integration is covered by `build_text_break_index_test.bat`.

## Line breaking

`LineBreakTest.txt` is the unmodified official corpus from
https://www.unicode.org/Public/17.0.0/ucd/auxiliary/LineBreakTest.txt.
SHA256: `e69884e0dde6a8724873f885d68c52dc14518abf9ae4ca9e2283b8773db3b752`.
All 19,338 records are tested without exceptions, including every UTF-8
interior byte and the actual XUI line/grapheme intersection.

`LineBreak.txt` is the unmodified normative property file from
https://www.unicode.org/Public/17.0.0/ucd/LineBreak.txt.
SHA256: `e6a18fa91f8f6a6f8e534b1d3f128c21ada45bfe152eb6b1bcc5e15fd8ac92e6`.
`build_text_break_index_test.bat` expands its explicit ranges and XX default
independently and compares all 1,114,112 codepoints to the vendored lookup.
Unicode License V3 applies, as preserved at `lib/unicode/LICENSE`.
