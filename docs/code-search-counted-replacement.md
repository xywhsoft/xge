# Counted whole-document search replacement

Two public native entry points retain their signatures and existing status,
creator-thread, undo and callback contracts. No public structure layout changes.

* Regex replacement now commits with an explicit output length rather than a
  C-string edit; an embedded document NUL no longer truncates the result.
* Plain replacement advances by the length of the committed normalized insert,
  rather than the raw input length. Replacing both `x` bytes in `xx` with CRLF
  produces two LF bytes and count two.
* Plain matches are exact UTF-8 byte ranges. The private counted edit validates
  code-point boundaries without expanding to whole graphemes. Replacing `e` in
  decomposed e/acute retains the combining mark and does not rescan inserted text.
  The public C-string editor replacement keeps its grapheme-clamped behavior.

The internal counted kernel reuses document validation, observers and history.
Both whole operations keep a real document call scope through EndEdit and final
cleanup. They preserve an earlier body outcome and its diagnostic (including
NULL), then release native pins. A callback may close the owner or fail after
committed text; failure is not atomic rollback. The legacy first-outcome control
already passed and is not classified as a fourth original defect.

`tools/test/check_code_search_outcome.py` links independent O0/O2 hosted/static
clients to the exact SDK and one complete compiler XRT on Windows and Linux.
It covers NUL/undo/redo, normalized advancement, partial graphemes, owned callback
failure/close, forty-eight earlier-error budgets and ninety-six regex allocation
budgets. It checks final live/invalid/double-free counters. Two old defect modes
are replayed against the original SDK. The partial-grapheme negative control
uses the bounded intermediate SDK only; the original unbounded path is not run.

Final acceptance contains 72 search steps and 56 existing document/compound-edit
regression steps across both platforms. Eighty baseline/control steps are
separate; some are successful regressions rather than expected rejections. A
frozen older editing fixture lacked the new fifth mode; its failed runner
attempt is preserved, and the final run pins the current test-only overlay.

The O2 host relinks compile three units/reuse 161. Static builds compile four
(including the generated contract)/reuse 162. Frozen source manifests exclude
unrelated worktree edits. Versus the prior complete collision SDK, hosted text
increases by 1664 bytes on Windows and 1600 on Linux; file deltas are 2048 and
32 bytes and BSS is unchanged. File deltas include linker/export/layout data.
There is no CPU-equivalence or complete source-rebuild claim. Native regex
compilation, text copying, history, checked pins and cleanup retain their costs.

`docs/validation/code-search-counted-replacement-20261114.json` pins exact
sources, snapshots, builds, controls and qualified reports. No runtime/IDE
installation, interactive graphics, network test or full-ecosystem completion
is claimed by this repair.
