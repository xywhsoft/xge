# Unicode 17 boundary data and corpora

Source: https://www.unicode.org/Public/17.0.0/ucd/auxiliary/GraphemeBreakTest.txt

- Official raw SHA256: `e2d134d2c52919bace503ebb6a551c1855fe1a1faec18478c78fff254a1793ec`.
- Repository SHA256: `88ead7e1c192b5d9767eb4c54cb62e177ab82afd972efc247e04599fab66b719`.
- Only trailing whitespace on comment lines was removed. All 766 test records,
  codepoints, boundary markers, comments and license attribution remain intact.
- Unicode License V3 is preserved at repository path `extlibs/unicode/LICENSE`.

`tests/xui/build_unicode_grapheme_test.bat` checks every UTF-8 byte against the
official boundaries using both contiguous and callback-based navigation. The
actual layout integration is covered by `build_text_break_index_test.bat`.

`WordBreakTest.txt` is the official Unicode 17.0.0 corpus from
https://www.unicode.org/Public/17.0.0/ucd/auxiliary/WordBreakTest.txt.
Official raw SHA256: `1de23a75f37904abc7d206239ee8d34f8fdf0fb4ab32a7174dfbabbde25419b2`.
Repository SHA256: `712cd8b8189944ad86cd81ebc3a9f6e9bb036cde6c27c7cafed3cfb68f8efe60`.
Only trailing whitespace on two comment lines was removed.
`tests/xui/build_unicode_word_corpus_test.bat` checks all 1,944 records and
8,487 boundaries through the pointer, 32-bit callback and 64-bit callback APIs.

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
Unicode License V3 applies, as preserved at `extlibs/unicode/LICENSE`.
