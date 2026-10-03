# SheenBidi in XGE / XUI

This is the `Headers/`, `Source/` and `LICENSE` subset of
[SheenBidi v3.0.0](https://github.com/Tehreer/SheenBidi/releases/tag/v3.0.0),
commit `cfe430e7375a7845b679adae9d51dac6deaa8858`, licensed under Apache-2.0.
The release uses Unicode 17.0.0 data.

Upstream archive:
`https://codeload.github.com/Tehreer/SheenBidi/zip/refs/tags/v3.0.0`

Archive SHA256:
`4c3ebd5dcc3424a20a47e0337db65e19058ff2c7448e66b11fe082c18ee5462e`

## Local changes

Project includes use paths relative to each source/header instead of requiring
vendor include-directory flags in every XUI build. Modified files are marked.
There are two additional fixes found by our independent tests:

1. `Source/API/SBParagraph.c`: initialize `_algorithm` to NULL at allocation.
   An allocation failure during paragraph processing releases the object before
   the retained algorithm is assigned. Previously its finalizer accessed an
   uninitialized pointer; our allocation-failure sweep crashes the original.
2. `Source/API/SBLine.c`: terminate the pending BN chain at a non-trailing space
   or isolate in L1. UTF-8 continuation bytes are represented as BN. Carrying the
   chain across an intervening character could reset only part of that character
   at a preceding S/B, creating visual runs within a UTF-8 scalar. The original
   fails Unicode 17 BidiTest record 208322 (`S RLE PDI R`, explicit LTR).

`test/verify_sheenbidi_vendor.py` reconstructs exactly these changes from an
extracted release and compares all 99 vendored files. It does not permit other
algorithm/data changes. The two defects and corrections are recorded locally;
they have not been submitted upstream.

## Integration and verification scope

`src/xui_text_bidi.c` compiles the upstream unity source as C. It owns copied UTF-8
and immutable paragraphs, retains logical byte offsets, and returns independently
owned per-line visual runs and mirror records. Line creation applies L1/L2
without mutating paragraph levels. XGE/XUI allocations use the engine allocator.
Mirror records describe L4 substitutions. HarfBuzz already mirrors an RTL item;
its input must remain logical text rather than being mirrored a second time.
`SB_CONFIG_DISABLE_SCRATCH_MEMORY` disables the default retained TLS scratch pool;
atomic object references remain enabled. Experimental text editing is disabled:
Document continues to own storage, transactions, positions and undo history.

Windows and Linux C builds pass both full Unicode 17 corpora: 91,707
`BidiCharacterTest` records and 770,241 `BidiTest` class/direction cases. The latter
uses fixed representative scalar values for all 23 bidi classes, including
multi-byte B/R/AL/isolates. Local tests additionally cover UTF-8 validation,
CRLF/P1, embedded NUL, copied lifetime, line ownership, L1 immutability, mirroring,
deep isolate/bracket limits and every allocation failure in two fixtures.
Linux also runs ASan/UBSan with leak detection. Upstream code units are never
treated as editable UTF-8 byte boundaries in the bridge.

The corpora are downloaded test inputs under ignored `artifacts/`; obtain them
from `https://www.unicode.org/Public/17.0.0/ucd/BidiCharacterTest.txt` and
`https://www.unicode.org/Public/17.0.0/ucd/BidiTest.txt`, retaining their headers
and Unicode terms. Run `test_xui/build_text_bidi_test.bat [character-file]
[class-file]` or `test_xui/build_text_bidi_sanitizer.sh [character-file]
[class-file]`. Without the downloaded files the Windows script runs local
contracts only (and, if available, the character corpus).

Document now consumes this bridge for paragraph, source/live and code layout.
Logical UTF-8 fragments stay source-sorted; line-specific L1 levels feed shaping
and a separate visual grapheme permutation. The bridge also exposes Unicode
Script for directional item boundaries. Native Document pixel/geometry probes
and Linux failure/reflow tests accompany this integration. Basic visual arrows
are separate from logical deletion. Safe-cut paragraph continuations retain
the complete paragraph analysis and its accounted owners; only the unfinished
tail is laid out again. Allocation failures restore the published geometry,
visual order and paint strings. Joining paragraphs without safe cuts and
non-ASCII code/source rows retain complete shaping context. Full script
extensions, font fallback and five-platform native graphics/IME acceptance
remain unfinished. UBA conformance alone does not establish complete Document
complex-script rendering.
