# Emoji native lifetime and cache prerequisites

This describes the qualified native fixes and their qualification boundary.
It does not declare the full safe xlang Emoji interface complete. Mutable pack
publication semantics and language/runtime engine retirement remain separate
decisions; no source, SDK or IDE delivery is installed by these tests.

## References and edits

Pack references use the canonical XRT atomic type and operations. Retain
rejects zero/saturated counts without wraparound; the final release observes
prior writes before destroying the owned fields. Callers still need an existing
strong reference. Atomic reference operations do not serialize mutable trie,
entry or SVG-cache state.

`AddSvgMemory` validates the entire UTF-8 sequence before allocating or changing
logical entries. It copies the input sequence and SVG, reserves entry/node
capacity with checked growth arithmetic, then publishes without allocation,
callbacks or further failure. Previous payload/cache owners are released only
after the new mapping is complete. Replacement keeps the ID; a failed insertion
does not leave an orphan entry or consume an ID. Failure can retain newly
reserved metadata capacity until pack destruction, but cannot retain the failed
payload or replace the old logical mapping. Invalid UTF-8 rejects without any
allocation. The C `strlen` length query is performed once; UTF-8 validation,
lookup and publication still traverse the sequence.

The trie registration helper used by the built-in loader uses the same
validate/reserve/publish phases. Its immutable input and reserved upper bound
make publication infallible. Entry/node capacity doubling and byte-size bounds
are checked before allocation, including near `INT32_MAX`/`SIZE_MAX` limits.

## CPU SVG cache and built-in publication

SVG-cache OOM preserves the source and leaves the entry retryable. Other parse
failure caching retains the prior native behavior; this is not a new promise
that every resource/format error is transient. Built-in compressed-data integrity
errors remain permanent errors for that immutable payload.

Built-in package construction and error reporting happen outside the global
slot spin. A warm lookup retains the selected pack under the slot lock. Cold
callers may build competing private candidates, publish one winner, then destroy
their unused candidate outside the lock. Error callbacks may clear the default
slot without deadlocking. Slot references and independently returned references
remain distinct owners.

Concurrent cold construction can therefore perform duplicate CRC/trie/context
work and allocations. Warm built-in acquisition still has a short slot lock and
atomic reference operations; matching still performs the native UTF-8/trie
lookup. The 1,000-iteration tests measure zero **XRT** heap operations for warm
match/acquire/release, not zero CPU cost, all-allocator activity or hardware
throughput. No language forwarding layer is introduced by these native fixes.

## Evidence and remaining boundaries

The old-code CPU controls reproduce an OOM that permanently disables a valid
SVG cache entry, rejected UTF-8 that leaves an entry and three live allocations,
and a real plain-reference read racing with an atomic write. A separate bounded
control enters an OOM error callback, then times out clearing the default slot
because the old loader holds its global spin. The timeout kills only that test
child; no graphical, audio or network test is involved.

The aggregate tests exercise transactional insert/replacement
allocation prefixes, retained previous SVGs, custom/built-in cache retry,
exact cleaned-up XRT heap/error counters, capacity arithmetic, reference-count
saturation, concurrent references, competing cold builders and callback reentry.
Linux O0/O2, TSan and UBSan preflight passes. TSan instruments both canonical
XRT and the aggregate; remaining archive objects are uninstrumented. UBSan
instruments the aggregate only. A separate public C11 O0/O2 and C++11/C++17
preflight passes against the candidate provider. The complete r65 host/static
products and canonical `tools/test/check_emoji_ownership.py` gates subsequently
pass too. `docs/validation/emoji-lifetime-20261112.json` records 109 passing
command checks: 52 Windows and 57 Linux, including the existing actual SVG GPU
regressions. Six O2 products contain 876 translation-unit occurrences: 875 fresh
compilations and one dependency-checked HarfBuzz reuse in the EGL host build.
All frozen/gate/log inputs were reverified. Typed Linux DWARF records both old
and new pack size as 112 requested bytes. These are not RSS measurements or
complete Emoji GPU-rendering qualification.

The independent default-publication control also reproduces mutation racing
with a public matcher while strong references remain valid. This batch
does not change that contract. The proposed safe language model freezes a pack
when it is published as the default; edits then build a replacement pack. The
alternative adds native serialization to both matching and mutation. That
decision is pending, so these fixes do not qualify concurrent mutable sharing.

The existing cold `Unit` branch does not clear the built-in Emoji singleton if
the engine has never been initialized. The public-client lifecycle tests use
manual Init/Unit without starting a rendering backend. Global pre-init cleanup,
private queued GPU references and module/code residency remain in the separate
engine-retirement work; this batch does not silently select a new shutdown rule.
The language pack interface, real Emoji rendering and matched final delivery
remain open.
