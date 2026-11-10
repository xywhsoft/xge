# Font resources and queued atlas lifetime

The owned Font factories create one independently allocated native Font record.
`FontCreateOwned` retains its Face. XRF memory/file, cached TTF/XRF and fallback
factories use the same checked graphics admission and rollback as the existing
owned resources. The caller releases them with `xgeFontFree`. FontFamily owns
one reference to each distinct Face; resolving returns another Face reference.
The normal stretch is a percentage (`100`), not the OS/2 width class (`5`).

Font sizes reject nonfinite values. Family scoring uses 64-bit differences and
scores; capacity growth checks multiplication. Glyph indices and scaled bitmap
coordinates are checked before the stb integer conversions. These checks cover
public numeric inputs; they do not qualify the complete font-file parser for
untrusted data, CFF or variable fonts.

Fallback replacement is transactional. Optional cache/file probes isolate their
diagnostics: ordinary unavailable caches may fall back, but OOM, callback,
state and unexpected diagnostic failures propagate. A successful optional probe
restores the caller's prior diagnostic. XRF baking propagates required glyph
preparation failures and never delivers partial output; saving checks `fclose`.
Failed cache insertion frees an unpublished glyph entry; after publication the
Font owns it, even when the subsequent codepoint mapping fails.

Atlas textures now have stable, independent native records. A delayed render
command retains the texture record, so growing the page array, clearing the
cache or freeing the Font cannot invalidate queued texture pointers. Each
uploaded page adds one cold allocation for its record and a pointer read when
accessing its texture. CPU-only pages do not allocate a texture record. The
engine-private storage tag is preserved as an integer through final release;
these textures are owned by the Font/private render queue and do not create a
separate public graphics reservation. Existing public textures still reserve
the engine and enforce checked Busy at shutdown. Private queues are drained
before renderer teardown.

`tools/test/check_font_resources.py` tests real matched host and static SDKs at
O0/O2 with the generated valid OpenType fixture. It covers extreme scoring,
nonfinite sizes, invalid glyph indices, scaled geometry, owned references,
fallback OOM preservation, XRF round trips, callback diagnostics, 32 allocation
budgets and heap/invalid-free/double-free balance. An explicitly enabled EGL SDK
also renders queued text after cache clear and Font destruction and reads back
actual pixels. Default SDKs require Unsupported from the optional EGL factory.
The earlier matched renderer crashed in that queued-text test; the immutable
failure evidence is retained alongside the final reports.

These checks run no network, audio or malformed-font fuzzing. Software EGL
results are correctness evidence, not hardware-performance measurements. Source
and delivery hashes, before/after reports and the exact frozen SDK cohort are in
`docs/validation/font-resources-20261111.json`. The cohort intentionally retains
the previously qualified UI sources; it is not a final all-mainline delivery.
