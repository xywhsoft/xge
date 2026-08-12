# SVG Emoji Rendering

XGE treats an emoji as a text-run item rather than a post-layout image. This
keeps measurement, hit testing, caret movement, selection, decoration, and
drawing on one cluster-aware path.

## Default behavior

- `XGE_TEXT_SHAPE_DEFAULT` enables the built-in emoji pack.
- `XGE_EMOJI_PRESENTATION_AUTO` respects text-default entries and VS15/VS16.
- `XGE_EMOJI_LINE_STABLE` fits SVG artwork into the font ascent/descent box and
  does not change the normal line height.
- Emoji advance, baseline, and ink size come from pack metrics. SVG `viewBox`
  only controls artwork fitting and never controls text measurement.
- SVG documents are parsed lazily on first draw and retained by the pack.
  Measuring a run does not parse SVG data.

`xgeTextMeasure`, `xgeTextShape`, `xgeGlyphRunMeasure`, and
`xgeGlyphRunHitTest` use the same advance values. A ZWJ sequence, flag,
variation sequence, or modifier sequence in the pack is one text item with one
UTF-8 cluster range.

## Line-height policies

`XGE_EMOJI_LINE_STABLE` is the default for XUI and code editors. The emoji is
scaled down when necessary so its normalized above-baseline and below-baseline
extents fit the current font.

`XGE_EMOJI_LINE_EXPAND` allows the emoji extents to increase run ascent,
descent, and line height. It is intended for document and rich-text layouts.

## Custom packs

Create a pack with `xgeEmojiPackCreate`, then register UTF-8 sequences with
`xgeEmojiPackAddSvgMemory`. Entries are indexed in a longest-match codepoint
trie. A caller can attach a pack to `xge_text_shape_desc_t` or install it as the
process default with `xgeEmojiPackSetDefault`.

Pack metrics are expressed in em units:

- `fAdvanceEm`: layout advance.
- `fWidthEm` and `fHeightEm`: normalized SVG ink box.
- `fBaselineRatio`: fraction of the ink box above the baseline.
- `XGE_EMOJI_ENTRY_TEXT_DEFAULT`: use the font in AUTO mode unless VS16 or
  COLOR presentation requests the SVG entry.

## Built-in assets

The built-in pack is generated from the pinned manifest under
`res/emoji/twemoji_core/17.0`. Run
`tools/emoji_pack/build_builtin_pack.ps1` after changing its SVG sources or
sequences.

The bundled graphics contain 1,923 Unicode Emoji 17.0 core entries derived from
Twemoji 17.0.3. The complete graphics license and attribution are retained in
`res/emoji/twemoji_core/17.0/LICENSE-GRAPHICS` and `NOTICE.md`.
