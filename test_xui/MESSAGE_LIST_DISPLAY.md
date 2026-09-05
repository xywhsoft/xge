# MessageList Display/Source Regression

This integration fix follows shared-text display-line commit `0b09fb9` and
filtered-shaping/source-projection correction `2027b68`.
MessageList prerequisites `e905f7e`, `7c4de78`, and DPI base `c4d3332`
are already ancestors. It changes no public ABI or other widget source.

## Mapping Contract

- Body lines and auxiliary title lines are drawn with the private
  `xuiInternalTextLayoutGetDisplayLine` interface. Titles retain the measured
  wrapping and are clipped to the available header width.
- A cold caret cache shapes that same display text. A forward scalar merge
  maps display advances to original UTF-8 offsets without a second Unicode
  classification table or a persistent byte-map allocation.
- Only common source/display grapheme and display shaping-cluster boundaries
  admit carets. Removing a format may join a displayed grapheme or ligature;
  a caret must not split that displayed unit.
- An inserted line-end hyphen maps to the ending offset of the original SHY.
  Selecting it copies SHY, never a synthetic ASCII hyphen. Removed controls
  keep zero-width source positions; selecting only those bytes paints no ink.
- Coincident caret positions retain the existing binary-hit tie policy:
  choose the earlier source boundary. Selection APIs, cross-message selection,
  clipboard copying, and stored node text remain in original byte coordinates.
- The cache still holds one line per node and is invalidated by existing
  text/width/font/DPI changes. A failed display, shape, or caret allocation
  cannot publish a partially rebuilt cache.

## Verification

Run `test_xui\build_message_list_audit_test.bat` for Display, P2-4, P2-5,
and original MessageList interaction tests. The `Display` argument selects
the new cases plus original interactions. The runner compiles `xui_text.c`
alongside the white-box MessageList translation unit, without importing the
private display-line symbol from a DLL. `run_all_tests.bat` discovers this
runner automatically. `build_message_list_test.bat` checks DLL linkage.

- 14 fixtures each with scalar shaping, fallback shaping, and DPI-sensitive
  shaping: SHY taken/not taken, real hyphens, space trimming, ZWSP, WJ, FEFF,
  NBSP overflow, shared UTF-8 prefixes, combining marks, emoji ZWJ, logical
  mixed RTL, CJK punctuation, and CRLF. Draw strings have literal oracles.
- Display-joined SHY/WJ ligatures and combining graphemes assert actual
  display-shape, line, measured-text and bubble width equality, including
  exact-width wrap thresholds; original-byte hit offsets remain unchanged.
- Partial SHY and zero-width
  selection, forward/reverse cross-message copying, self/system/auxiliary
  rendering, collapsed/narrow auxiliary titles, mutation, and DPI reflow.
- Display/shape/caret-allocation failure injection and successful retry.
- 512..65536 repetitions of a visible scalar plus four invisible formats:
  6144..786432 raw bytes. Cold mapping work is exactly 17 operations per
  repetition (8704..1114112). Thirty-two warm hits do no shaping, measuring,
  display conversion, or map rebuilding; binary comparisons are 368..592.
- 513/4097/32769-line messages: a deep cold hit decodes only 3 source scalars
  and shapes 2 display bytes; painting visits 24 visible lines at each size.
- Existing P2-4 warm-hit and P2-5 node/line indexing counts remain unchanged.

All of these cases pass after the shared projection correction, along with
the original DLL-linked MessageList test and 13 related shared-text/control
runners. The rebuilt DLL still does not export the private display-line API.

## Reusable Internal Entry Points

`src/xui_text_internal.h` declares `xuiInternalTextLayoutGetDisplayLine`.
Its NUL-terminated result is borrowed until the next display-line request,
Draw, Reset, or layout destruction; do not retain it in widget caches.
`xuiTextLayoutGetLine` and `xuiTextLayoutGetText` still provide original byte
ranges and text. There is no new generic display-offset-to-source API.
`__xuiMessageEnsureLineCarets` is the local reference for a linear scalar
merge, common source/display grapheme boundaries, and synthetic SHY mapping.
The shared `__xuiTextShapeProjection` implementation instead remaps shaping
clusters in one pass and discards its transient displayed-byte source index.

## Other Consumers (Read Only)

Searching `TextLayoutGetLine` at this base also finds the same raw-line
rendering risk in `src/xui_label.c`, `src/xui_hyperlink.c`,
`src/xui_msgbox.c`, `src/xui_msgtip.c`, and `src/xui_toast.c`.
`src/xui_text_edit.c` copies shared logical line offsets, drops break type,
then measures and paints original text; it also needs its own display/source
integration. Shared `xuiTextLayoutDraw` already uses display lines.
RichEdit does not call GetLine here and its separate second phase is not
included. Sender metadata and application-owned custom node renderers do
not consume shared layouts in MessageList and are unchanged.

## Boundaries

This is logical caret mapping, not a visual bidi or shaping-engine redesign.
The existing backend supplies ordered logical shaping clusters. Shared layout
now shapes the same filtered projection before mapping to source coordinates;
MessageList does not override shared line widths. Contextual shaping across
line breaks remains a backend limitation, not an adapter width adjustment.
Tests use deterministic proxies on Windows/MinGW; native font/GDI visual
integration and the complete suite are coordinated by the parent task.
