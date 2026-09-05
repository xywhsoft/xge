# P2-7 Unicode line layout contract

## Scope and modes

The previous WORD wrapper saved only ASCII space/tab break opportunities.
On overflow it split arbitrary shaping clusters, bypassing CJK kinsoku and
NBSP/WJ, and scalar-only proxies could split combining/ZWJ sequences.

- WORD selects libunibreak's Unicode 15.0 default/strict line opportunities,
  intersected with Unicode 15.1 extended grapheme and shaping boundaries.
- When no fitting normal opportunity exists, WORD may break alphabetic/Hebrew/
  numeric runs at the same common boundaries. This is explicit emergency
  tailoring, not an additional UAX #14 rule. It does not override punctuation,
  GL/WJ, ZWJ, or grapheme prohibitions. UAX #14's own exceptions still apply.
- A protected unit wider than the width is kept intact and may overflow.
  An oversized final WORD unit ends with END, without a spurious empty line.
  Parentheses do not protect an entire expression: only the actual UAX #14
  prohibited boundaries are protected. CJK can wrap inside a longer pair.
- CHAR deliberately ignores WORD punctuation/glue restrictions and breaks at
  common grapheme/shaping boundaries. It preserves ASCII trimming and its
  historical trailing empty line after an oversized final character.
- NONE disables width wrapping. All modes recognize CRLF as one hard break,
  and CR/LF/NL/BK (including VT/FF/NEL/LS/PS) as paragraph breaks.
- Existing ASCII space/tab trimming and post-WORD-wrap skipping remain.
  NBSP, narrow NBSP and other Unicode spaces are not trimmed as ASCII spaces.

## Source and display

`GetText` and `GetLine` still expose the owned, original UTF-8 and logical byte
offsets. No bidi visual-order offsets or rewritten source are published.
SHY/ZWSP/WJ/FEFF are hidden in display lines; NBSP remains a visible-width
space, and emoji ZWJ is retained for shaping. Layout shapes this same filtered
projection once, then maps its cluster ranges back to original byte offsets.
The original UAX14 map continues to own logical break opportunities. Zero-width
source clusters preserve removed formats between displayed clusters; formats
inside a newly joined shaping cluster remain indivisible. Display graphemes
are also intersected with the original boundaries, including for scalar proxies.
Thus removal can join a ligature or combining sequence without leaving stale
source-shape advances in line widths or wrap decisions.

A WORD break exactly after SHY displays ASCII `-`; its shaped advance is
included in break selection and line width. SHY otherwise remains invisible,
including CHAR and NONE, and a break after spaces following SHY adds no hyphen.
This is discretionary hyphenation only, not dictionary or language-specific
automatic hyphenation. Cross-line kerning/shaping remains a backend limitation.

`xuiInternalTextLayoutGetDisplayLine` in `src/xui_text_internal.h` returns a
borrowed display string. Shared Draw uses it. Controls that bypass shared Draw
(notably MessageList) must adopt this helper for equivalent discretionary-hyphen
rendering; source selection/copy/hit offsets must continue to use raw text.
This commit intentionally does not modify control source or public ABI.

## Bounds and tests

For B source bytes and C ordered, finite-advance shaping clusters, added
boundary construction is O(B + C), retaining B+1 bytes and using 2B transient
bytes. With D displayed bytes and R removed scalars, filtered shaping adds
6(D+1) transient bytes for text/graphemes/source indices, followed by a cluster
merge allocation of at most C+R entries. These buffers are not retained or
shared with the display scratch buffer, preserving transactional DPI refresh.
Upstream table lookup is bounded by fixed Unicode data, not text length.
The existing 8(C+1)-byte double prefix and binary range searches remain;
wrapping is O(B + C log C). Malformed/unordered custom shapes retain the prior
range-scan fallback and are outside that fast-path guarantee.

Run from the private worktree with MinGW GCC on PATH:

```bat
call test_xui\build_text_linebreak_test.bat
call test_xui\build_text_projection_test.bat
call test_xui\build_text_break_index_test.bat
call test_xui\build_text_scale_test.bat
call test_xui\build_text_layout_test.bat
call test_xui\build_text_dpi_test.bat
```

The index test uses both pinned official conformance suites, checks the sole
known line exception explicitly, and exercises malformed bytes/embedded NUL.
Allocation failure is injected at every construction allocation, geometry is
cleared and retried, and tracked outstanding allocations must reach zero.
Twelve scale patterns grow from 256 to 65,536 repetitions, up to 1,048,576
clusters. They include long combining/RI/ZWJ runs, protected glue, OP plus long
space runs, CJK, RTL, SHY, zero-width and mandatory-break inputs. Assertions
bound actual decoder calls, byte visits, wrap iterations and binary probes;
there is no wall-clock pass threshold. The earlier 256-case differential test
still compares prefix lookups against legacy scans under the same new policy.

Verified for the original Unicode commit on Windows/MinGW: 58 fixtures pass with scalar
proxy shaping, grapheme fallback and a DPI refresh of each retained layout.
The same fixtures linked against `d1fb691`'s text implementation pass only
15/58. All 14 related runners pass: linebreak, break_index, scale, layout, dpi,
text, unicode, text_edit, label, rich_edit, message_list, render_schedule,
context and the independently rebuilt message_list_audit. GCC `-fanalyzer`
reports no diagnostics for the text translation unit. No full UI suite or
complete bidi/shaping engine conformance is claimed.

For the largest OP/space case (B=C=1,048,576): decoder calls 3,145,730,
byte-map visits 2,097,152, wrap iterations 1,048,576, range+next probes
63,963,195. Pure 65,536-mark input has 196,610 decoder calls and one wrap
iteration, demonstrating no repeated scan of a long indivisible grapheme.

## Filtered Shaping Regression

Before the projection fix, deterministic joined-shape fixtures reproduce
`Af<SHY>fiB` and `Af<WJ>fiB` line width 40 versus displayed width 30, and
`Ae<SHY><combining acute>B` width 32 versus 24. Exact displayed-width wrapping
also incorrectly produces multiple lines. The projection test now asserts
actual display-shape/line/layout width equality and raw-text identity in
NONE/WORD/CHAR, not just hit offsets. It also checks all displayed lines of
five patterns at 256..65,536 repetitions, including SHY/ZWSP gaps and pure
formats, with no invisible scalars passed to the shaping proxy.

The new test bounds preprocessing visits by 16B, wrap steps by 3B, shaped bytes
by B+1, and binary probes by 20(C+1)ceil(log2(C+1)). At B=589,824 for SHY/ZWSP,
the measured preprocessing visits are 5,373,955 and probes 39,955,139; shaping
receives only 262,145 bytes (including the discretionary hyphen). Thirty-two
warm size queries cause no shaping. Eleven construction allocation failures
and nine retained-layout DPI refresh failures each retry without leaks or
loss of original text/geometry. Existing conformance cases and complexity
assertions are unchanged: UAX14 7,653/7,654 with its one exact documented
upstream exception, and graphemes 1,187/1,187.

After the correction, all 15 related runners pass against the rebuilt private
Windows/MinGW DLL or their standalone source composition: projection, linebreak,
break_index, scale, layout, dpi, text, unicode, text_edit, label, rich_edit,
message_list, render_schedule, context, and message_list_audit. The text
translation unit again passes GCC `-fanalyzer` without diagnostics. The private
display-line helper is absent from DLL exports; no public ABI has been added.
Parent-managed native font and full-suite integration are not claimed here.
