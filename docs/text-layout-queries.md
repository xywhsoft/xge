# Checked TextLayout reads

TextLayout remains an independent unique native owner. Context observation is
weak; expiration rejects reads but does not consume the layout's Font/paint
cleanup obligation. The existing checked destructor and consumed flag remain
authoritative. No reference-count layer is added.

`xuiTextLayoutGetState(layout,size,lines,truncated)` refreshes DPI once and
returns the actual status. Any output may be NULL. Failure leaves all state
outputs unchanged. This distinguishes an empty/zero-size layout from a failed
read; the original scalar getters keep their existing ABI and zero fallback.

`xuiTextLayoutGetTextView(layout,view)` returns the complete counted source,
including embedded NUL. Empty text has a non-NULL pointer and zero size.
Failure clears the output and returns a checked status. Its borrowed pointer
requires the creating thread, a live layout and no intervening reset, DPI
rebuild or destruction. Language adapters must copy while their scope pins the
layout. The native function itself does not allocate a snapshot or scan for NUL.

Both functions reuse the existing DPI rebuild and cleanup path. A cold DPI
refresh may allocate or call the backend; hot reads are direct state access.
Only selected output fields are written, avoiding a mandatory combined record
for callers needing one scalar. These are public native safety interfaces,
not language-specific forwarding functions. They introduce no per-frame work
unless the caller invokes them.

`xui_text_layout_queries_test.c` uses the actual owned Font/paint CPU backend,
original reset/close/draw lifecycle regressions, NUL/empty source, failed DPI
allocation, expired Context and warm allocation-budget checks. Its `legacy`
mode reproduces the old read limitations without treating them as acceptance
of the new checked interfaces. No window or network/security test is required.

Windows and Linux acceptance on 2026-11-14 each passed the actual legacy
reproduction and the fixed O0/O2 build/run gate (four commands per gate).
All eight fixed commands passed. The fixed test also executes the prior
owned-layout reset/OOM and draw/close reentry regressions, verifies complete
heap balance, and observes no allocation/free/reallocation in 1000 hot pairs
of state/text queries under a zero-allocation budget. This is not an equal
CPU-time claim. Full O2 host and static SDKs were rebuilt from frozen sources;
the xlang integration and graphical lesson have their own acceptance.
See `validation/text-layout-queries-20261114.json` for exact local evidence.
