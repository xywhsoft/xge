# Immutable shaping and raster resources

`xgeTextShapeOwned` consumes the descriptor and its UTF-8/context/language data
synchronously and returns one heap-owned immutable GlyphRun reference. Use
`xgeGlyphRunAddRef` / `xgeGlyphRunFree` for independent native owners. Raw
`xgeTextShape` and caller-owned run records retain their existing storage ABI.
Do not reshape, memcpy or mutate an owned published run. The resource and its
Font references require single-threaded/creator-thread use; const inputs do not
make reference counts atomic.

Every retained Font must own its outer native record. The owned constructor
rejects a run retaining caller/stack Font storage instead of pretending that a
reference count keeps that storage alive. Empty runs own no Font. Line-break
positions retain the Font they publish, just like drawable positions; the old
implementation published that pointer without acquiring a reference.

`GetGlyphs` and `GetCarets` expose immutable contiguous arrays for an independently
retained run's lifetime. They reject raw caller-owned runs and reset failed
pointer/count outputs. `GetFont` acquires a separate owned Font reference, or
returns NULL for an item without a Font. Its failed output is reset; an earlier
caller-owned reference remains the caller's obligation. Existing measure, hit,
draw, span/decorated draw and outline APIs operate on the same native run.
`RetainedBytes` includes the outer record for owned runs and excludes shared
Fonts/atlas resources. Missing required glyph metrics fail shaping rather than
silently removing an item; Font-reference vector growth checks overflow.

`xgeFontGlyphRasterizeOwned` / `ByIndexOwned` produce immutable raster resources.
The resource owns its alpha pixels independently of the Font and engine; `Info`
reports dimensions/stride/format/byte extent. `GetPixels` exposes a stable
read-only byte span. AddRef/Free preserve the same bytes until the final owner
releases them. Empty images have zero byte extent. Failure and OOM never publish
a partially constructed resource. Rollback preserves the first diagnostic,
including a cleanup diagnostic when none existed earlier, and retains an
unreplaced caller diagnostic.

On x64 the run's private backend grows by eight bytes for its owning reference
count; a heap run adds its 72-byte outer record. An owned bitmap adds one 48-byte
record around the ordinary raster output. No text-array copy or additional
Font-data copy is introduced by native owning references. These are storage
costs, not a claim of zero end-to-end language or hardware overhead.

`tools/test/check_text_resources.py` covers raw newline lifetime, independent
run/font references, NUL input, full paragraph context, empty runs, span identity,
hit/outline APIs, raster lifetime, 128 allocation budgets and final heap/resource
balance in matched shared/static O0/O2 programs. Enabled software EGL draws
ordinary, span-colored and decorated text, drops Run/Font before submission and
reads 696 colored pixels back. Font, context and Sokol resource regressions are
separate evidence in `docs/validation/text-resources-20261111.json`.

Generated valid fonts are used throughout. No network, audio, malformed-font
fuzzing or hardware-performance test runs. The language projection and final
all-mainline compiler/SDK/IDE delivery remain separately qualified work; this
native batch does not install release artifacts.
