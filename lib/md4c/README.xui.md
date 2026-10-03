# MD4C in XUI Document

Source: https://github.com/mity/md4c

Pinned revision: `b3c6223903c1df483cef926ba347e531248f0b92`.
See `LICENSE.md` for the MIT license. `md4c.c` has a guarded XUI integration
patch (`MD_XUI_SOURCE_BLOCK`): record complete block byte ranges while grouping
lines, report them beside block callbacks, and retain paragraph callbacks in
tight lists. Table rows and explicitly parsed cells also report their source
ranges, including a zero-width anchor for a cell containing only whitespace;
cells synthesized to fill a short row have no source anchor. With that macro
undefined the upstream behavior is unchanged.
`MD_XUI_SOURCE_REFERENCE` reports complete link definitions and their raw label,
destination and optional title ranges, including duplicate definitions. The
callback runs only after successful definition parsing and propagates errors.
`MD_XUI_SOURCE_SPAN` reports each opening/closing delimiter at the actual inline
callback, including split emphasis runs, padded code ticks, images, references,
math and autolinks. Resolved link destination ranges connect uses to the exact
chosen definition; XUI does not duplicate the parser's Unicode label matching.
`MD_XUI_SOURCE_BREAK` reports a parser-confirmed inline line break after its
text callback. It distinguishes soft breaks, backslash hard breaks, trailing
space hard breaks and parser-forced hard breaks, and returns the trailing
horizontal whitespace, optional hard marker and LF/CR/CRLF byte boundaries.
Document packs relative offsets on the SoftBreak/HardBreak node so a persistent
source shift leaves earlier snapshots intact. Image alt text has no semantic
break node and does not receive this Document metadata.
`MD_XUI_SOURCE_MARKERS` reports parser-confirmed ATX/Setext heading, fenced
code, thematic-break, list-item and quote-opening marker spans after the block
source callback. The opening/closing spans are captured when MD4C classifies
each line, including fences inside list/quote containers and a thematic break
revealed after link definitions are consumed. List-item openers also carry the
optional task checkbox; CommonMark leaves that range absent. A fenced block
also carries the exact end of its opening source line, so XUI can expose the
raw info-string tail including spaces and tabs without rescanning a long line.
`MD_XUI_SOURCE_FENCE_INFO` reports the same fenced-code parser's significant
info bounds after its ASCII-space trimming and the first language-token end
after its whitespace split. It runs after the fenced block enters Document;
empty info/language ranges are valid. The Document node reuses its
kind-specific syntax auxiliary blob for these three relative offsets, while
tables use that blob for their packed delimiter records. The raw tail still
carries spaces, tabs and unparsed metadata without normalization.
`MD_XUI_SOURCE_HEADING_CONTENT` reports the finalized ATX/Setext inline-content
extent after block entry and marker callbacks, before inline callbacks. Link
definitions have already been consumed and a Setext underline has already
been removed from the retained line set. Empty ATX content is valid; multiline
Setext content spans its raw intervening line endings and container prefixes.
Document keeps these two relative offsets in the heading's syntax auxiliary
blob and resolves persistent source shifts when queried. The hook has no
effect when undefined and does not change heading recognition.
For list items, the same guarded marker hook carries the end of the raw
horizontal-whitespace gap after the bullet/number delimiter and, when a task
checkbox is recognized, the end of the gap after `]`. The parser captures
these endpoints when it consumes the opening line, including empty gaps and
tabs. Document stores one relative tail endpoint per marked node: a task
item's first gap ends at the checkbox's already stored opening offset; the
same field serves a fenced block's info tail, since the node kinds are
exclusive. This records raw bytes, not the logical column at which a tab stops.
The XUI node stores offsets relative to its block syntax start, so persistent
suffix shifts do not copy or rewrite marker ranges. The guarded
`MD_XUI_SOURCE_QUOTE_PREFIXES` hook records every explicit `>` accepted by an
existing quote container and emits the complete per-quote sequence after its
block opener. Lazy continuation lines carry no marker. XUI stores only later
prefixes in a compact relative array; a one-line quote needs no extra array.
`MD_XUI_REFERENCE_CANDIDATE` reports valid link/image label lookups that found
no definition and valid footnote references that found no definition. For a
full link, the reported source range is the queried label bracket, not the
entire link text. XUI sorts and deduplicates the resulting source records;
failed callbacks abort parsing without publishing a partial index.
All hooks are guarded; the uninstrumented parser also compiles with -Werror.
`MD_XUI_CANCEL` polls a retained prepare's atomic cancellation reason at parser
checkpoints and allocation boundaries. The default hook is constant zero.
Cancellation unwinds through normal parser cleanup, and the wrapper restores
its thread-local token along with the allocator and source callbacks. This is
cooperative cancellation, not a hard deadline for individual scans or sorting.
Footnote block extents are retained even when definitions are emitted in reference order.
`MD_XUI_FOOTNOTE_DEFINED` reports the full source and label ranges of every
definition, including unused ones, after the definition table takes ownership
of its content lines. The span hook passes the parser-selected definition's
source range for resolved footnote references. Callback failures unwind through
the parser's normal cleanup path.
Footnote emission keeps the definition array stable because label hash buckets
point into it. A separate index-to-definition pointer array preserves reference
order and accepts definitions first referenced from another footnote body;
sorting the definitions in place had broken those lookups.
For `MD_FLAG_FOOTNOTES`, a blank separator followed by a four-column indented
continuation (relative to an enclosing list or quote) stays in the definition block.
The body is reanalyzed by MD4C's block parser after stripping the footnote
continuation prefix. Paragraphs, lists, fenced/indented code and quotes keep
their block structure and source ranges, including a definition with an empty
first line. This works at document root and in list/quote nesting. Blank list
separators may omit list indentation, but quote prefixes must remain explicit.
The definition source callback covers all continuation lines. An indented
continuation that begins with `[^label]:` remains body text rather than
becoming a second definition.
Incremental parsing must not infer their absence from the rendered node tree:
an edit can introduce the first use of an otherwise invisible definition.
The wrapper restores the callback and presence output pointer with its TLS state.
Keep the patch when updating the pinned parser and run the Document corpus tests.

XUI's guarded table-source hook runs only after GFM table recognition. It
reports the separator line and each parser-confirmed unescaped row boundary
pipe; it also classifies the separator's hyphen/colon runs and optional pipes.
The Document adapter stores relative offsets on the table node so immutable
snapshots retain their own byte coordinates after incremental source shifts.
The uninstrumented upstream parser has no XUI table callback.

The guarded list-indent hook attaches raw horizontal-whitespace ranges to
list-item blocks only for lines that explicitly satisfy their indentation.
It waits for the parser's final blank-line, lazy-continuation and sibling
decisions before recording a line. The first item line is not a continuation;
an implicit lazy line has no indent token. A compact linked arena survives
until the corresponding list-item block is emitted, then Document stores
relative offsets and each nested item's parser-consumed logical columns on
its immutable node. Tabs use MD4C's four-column stops, so a nested item's
content boundary can fall inside one raw tab byte. Keep this hook guarded in
plain MD4C.

The guarded indented-code hook runs after MD4C has trimmed leading and
trailing blank lines from the code block. Each retained line carries its raw
horizontal-whitespace range and the parser's logical start, four-column
content boundary and full indentation end. Internal blank lines have no
required content column. The analysis metadata travels with verbatim lines
until block emission; fenced code uses its separate marker and info hook.
Document stores the records relative to the CodeBlock syntax start, including
boundaries inside a tab and offsets shared with enclosing list indentation.
The plain parser remains free of this hook.

`src/xui_document_md4c.c` compiles the parser with allocation functions routed
to Document's allocator. It restores thread-local state after each parse.
The adapter also exposes private footnote-label comparison and grammar checks
using this pinned parser's own Unicode fold and label scanner. Commands use
these for explicit labels, unused source definitions and dependency cleanup;
do not replace them with ASCII-only or equal-byte-length comparisons.
The Markdown writer also calls `doc_md4c_unicode_punct` and
`doc_md4c_unicode_whitespace` from this pinned parser's delimiter-flanking
tables when spelling adjacent emphasis runs. Keep those classifiers
synchronized with MD4C when updating the parser; ASCII-only boundary tests
cannot cover CJK punctuation or nonbreaking spaces.
The Document allocation-failure tests include parser allocation failures.
`MD_XUI_REFERENCE_VALUES` reports every link definition's parser-merged raw
label, destination, title, and title-presence flag, including unused duplicate
definitions. Regular Document parsing stores these values as immutable derived
metadata, shared by snapshots and unchanged incremental edits. A private source
validator compares these cached complete bytes when container prefixes change
inside multiline fields, never hashes or only rendered links. A separate
two-parser observer remains a test oracle; production validation needs no
additional source parse. The callback's TLS slot and dialect flags are shared
with the regular adapter, with allocator/cancellation restoration.
Reference line lookup rejects an empty line set or a missing lookup result
before dereferencing it. This defensive check follows the strict analyzer's
reported path; it is not evidence that valid input previously reached it.
Do not also compile `md4c.c` directly into the XUI source target.

`entity.c` supplies the named-entity table. `md4c-html.c` is retained as an
upstream reference and is not part of the XUI target; Document exports HTML
from its shared semantic tree. Parsing math or Mermaid source does not itself
provide mathematical typesetting or diagram rendering.
