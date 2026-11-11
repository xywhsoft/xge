# SVG operation lifetime

SVG documents and paints are caller-serialized CPU objects. This change does not
make their mutation or the global SVG cache thread-safe. Borrowed paints remain
invalid after content replacement, Clear or destruction; it does not turn raw
paint pointers into independently owning handles.

Loading and traversal hold an independent document reference through their full
return, including resource-provider callbacks. A callback may drop its own last
document reference; the active operation releases its reference only after it
has finished using document storage. Reference increments reject zero/negative
counts and INT_MAX instead of reviving invalid counts or overflowing. Cache and
embedded-image acquisition paths propagate these failures and do not release a
reference they failed to acquire.

Clear and reload return Busy during loading or traversal, before modifying
content. Recursive URI loads reject admission before path allocation or invoking
another provider. Clone and new traversal reject an in-progress load. Nested
read traversal remains supported. Traversal also refuses mask/clip composition
replacement, which can change the internal child storage. Nonstructural opacity
changes continue to work. Callers may stop a traversal normally by returning zero;
all operation counters and references still unwind.

Picture exposes its Root paint as one child. The Root/Content/Document chain and
its actual SVG descendants can consequently be reached through public child
queries and traversal; child and parent relations agree. Previously the public
Picture traversal silently stopped at the Picture itself.

Destroy and Clear now share the same content cleanup, including unused embedded
`@font-face` source data and decoded faces. The Base64 bit accumulator is unsigned
so consumed high bits wrap with defined semantics. Its allocation bound avoids
overflowing the length-times-three intermediate, while preserving the old bound
and whitespace/padding behavior. No new decoder, per-method forwarding function
or backing-array copy is introduced.

The private document grows from 9,408 to 9,416 requested bytes on the tested x64
ABIs. Operation guards add reference/count/state checks and a private parsing
body call; they do not allocate another owner record or acquire a lock. This is
a source/storage description, not a timing or total-zero-cost claim. Existing
parser, path/provider, font and clone allocation/copy work remains.

`docs/validation/svg-lifetime-20261111.json` pins source, original full build
parents, matched incremental O2 host/static images and exact reports. Windows and
Linux O0/O2 tests cover real project-owned embedded fonts, unused/decoded face
cleanup, owner drops, nested/early-stop traversal, admission, reference bounds,
96 clone and 128 load allocation prefixes against live heap baselines. Actual
DLL/shared/static products are separately exercised through strict C11,
C++11 and C++17 clients with the matched host-XRT contract.

Seven old behaviors reproduce in bounded assertion controls before invalidated
storage is used. A further valid embedded-font input reproduces the old signed
Base64 shift under UBSan. Final ASan/UBSan covers the authoritative aggregate;
remaining archive objects and the host XRT are not instrumented. No GPU, window,
audio device, network or performance benchmark runs in this batch.

These are native prerequisites. Safe language paint observation, complete SVG
declarations, output storage contracts, actual rendering and final matched
delivery remain separate work. No release/IDE installation is qualified here.

## Paint content-generation observation

`xgeSvgPaintLifetimeWatch` and `xgeSvgPaintLifetimeUnwatch` provide native
invalidation for borrowed paints. The caller supplies one zero-initialized,
address-stable 32-byte link on the tested x64 ABIs. It does not retain the
document or make an expired raw paint usable. Keep the document alive and
serialize access, as for the existing SVG API; never copy an attached link.

Content replacement, Clear and destruction detach observers before notifying
them and before resetting/freeing any paint storage. Each callback may free its
own detached link or unwatch/free peers, must return normally and must not use
the invalidated paint. Cloning does not copy observers. Invalid arguments do
not start replacement or invalidate old paints. Once replacement starts, an
allocation failure still invalidates the previous generation; it does not
restore the old content or its paints.

Clear now pins the document through notification and cleanup, so an invalidator
may drop the last external reference safely. Reentrant Clear/load/clone/
traversal and new observation reject the clearing/loading state. Observation
can be registered during an ordinary traversal, whose independent document
reference already protects that operation. Duplicate attachment is rejected
without changing the original subscription; detached unwatch is idempotent.

The private document grows from 9,416 to 9,424 requested bytes on x64. Register
and unregister are O(1), allocate nothing and introduce no lock; invalidation
is O(number of observers). Clear adds a reference acquire/release pair.
Existing parser, native resource, clone and normal language-view allocation/
bookkeeping costs remain. There is no timing or total-zero-overhead claim.

`docs/validation/svg-watches-20261111.json` indexes full Windows/Linux O2 host
and static builds (146 native translation units each), separate authoritative
aggregate O0/O2 tests, actual host/static C11/C++11/C++17 clients and Linux
aggregate ASan/UBSan: 149 command rows. The remaining archive and host XRT are
uninstrumented. Tests cover self/peer removal, last-owner release, normal and
failed replacement, clone independence, reentry, allocation-free observation
under a zero allocation budget, existing embedded-font allocation sweeps and
exact heap baselines. No window, GPU, audio device, network or security test ran.

This raises the SVG public function count from 68 to 70. Safe language paint
declarations and exact status/output borrowed-pointer admission still need
their own compiler and SDK gates. The xlang fixed-array output gate was closed
separately in `26701e7f4`. Complete rendering and matched final delivery remain
open; this native batch does not install release/IDE binaries.

## Explicit document caches

`xgeSvgCacheCreate`/`Destroy` manage an explicit cache. `Load` returns one
independent document reference; repeated loads of the same URI in one cache
share the mutable document. Separate caches have independent document identity.
`Remove` drops one entry, returning NotFound when absent; `Reset` drops all cache
references. Existing external document references survive Remove/Reset/Destroy.
The new cache is caller-serialized, including ownership operations; independent
caches may run concurrently. The safe xlang design binds cache/document views
to their creator thread, with independent caches across threads.

Load/Remove/Reset pin the cache through provider and invalidation return tails.
Callbacks may drop its external owner. Recursive operations on the same active
cache return Busy before allocation or provider invocation. Failures do not
publish an entry or transfer an output reference. Cache creation leaves the
output untouched on failure. The cache adds no callback slot; provider and
invalidation callbacks retain their existing native contracts. Full language
resource/ownership policies remain a separate gate.

The old global entry points and the explicit cache share the same internal
lookup/load/remove/reset algorithms. Legacy global Reset now detaches its old
list before destruction callbacks. A reentrant Clear sees none of the dying
entries, and new entries published during notification remain in the new list.
This fixes reentrant destruction; it does not make the legacy global API
thread-safe or qualify cross-thread language views of that global cache.

A cache requests 24 bytes on tested x64 ABIs and one creation allocation.
Hits allocate nothing; misses retain the existing document/parser, entry and URI
allocations/copies. Lookup remains O(entries), including URI comparison; active
operations add reference/state checks without a mutex or per-call heap adapter.
Language-view allocation, creator-thread, error and code-residency costs have
not been measured for a complete language API. No total-zero-overhead or timing
claim is made.

`docs/validation/svg-cache-20261111.json` indexes 198 passing command rows across
full Windows/Linux O2 host/static products, aggregate O0/O2 tests, actual
C11/C++11/C++17 clients, independent cache threads and Linux aggregate
ASan/UBSan. The remaining native archive and host XRT are uninstrumented. Each
96-prefix allocation sweep has 26 failed and 70 successful prefixes and returns
exactly to a live-cache heap baseline. Old O0/O2 global-reset controls stop at an
assertion before unsafe reentry. No GPU, device, window, network or security test
ran. The whole-tree pre-build inventory mistakenly included two generated
HarfBuzz dependency/cache metadata files: original failed inventory reports and
freezes are preserved, and separate proofs classify only those exact generated
outputs while verifying every other byte unchanged. Both builds had succeeded;
no native rebuild or source mutation was needed for that inventory correction.

The native SVG family now has 75 public functions. Complete safe language
declarations, borrowed status/output admission, GPU/renderer lifetime and final
matched delivery remain open; this native cache batch installs no release/IDE.

## Stable font records and cached GPU owners

SVG font cache entries now hold separately allocated, address-stable Font
records. Cache growth cannot move a record retained by a GlyphRun. Clear and
destruction release the cache's native reference; independently retained Fonts,
Faces and owned GlyphRuns remain valid. The private Font storage enum separates
caller storage, graphics-owned storage and CPU-owned storage. CPU cache fonts
need no initialized engine and do not reserve one merely for metrics or layout.
Their lazily materialized atlas textures acquire independent graphics owners.

Raster images also hold ordinary owned Texture pointers instead of embedded
records. SVG clones and queued drawing consequently retain real native
references. Engine shutdown reports Busy while those GPU obligations remain;
releasing the final references permits shutdown. Native draw-state restoration
can flush SVG output before return, so a caller must not assume every SVG call
leaves pending commands.

The Font statistic is an atomic counter outside resettable engine state. It
continues to count live CPU records through Init/Unit cycles. This does not make
Font or Face mutation thread-safe: independent cache tests load independent
embedded Faces. The process-global fallback/shared mutable Face contract still
requires caller serialization. Safe language fallback/thread admission remains
an integration gate.

Embedded-face OOM preserves its source bytes and retry state. Allocation errors
from cache creation, glyph metrics and shaping propagate through text loading,
measurement and drawing; they no longer silently select another font or report
zero advance. ResourceFailed/Unsupported preserve the existing optional-font
behavior. Failed loads retain the existing destructive replacement semantics.

On tested x64 ABIs a cache entry shrinks from 232 to 144 requested bytes and
adds one 96-byte Font allocation/free. Full occupied capacity therefore requests
240 rather than 232 bytes per font; unused entry capacity is smaller. A raster
shrinks from 88 to 48 bytes and adds a 48-byte Texture only on GPU materialization.
These sizes exclude allocator headers/slack and are not RSS measurements.
Graphics admission/release and atomic statistics have explicit native costs.
The 1,000-hit cache loop has no XRT heap operations; this is not a timing or
complete language zero-overhead claim.

`docs/validation/svg-font-cache-20261112.json` indexes 331 passing command rows:
complete Windows/Linux O2 host/static builds, a matched Linux EGL cohort,
aggregate O0/O2 and debug checks, strict C11/C++11/C++17 clients, retained
GlyphRuns, independent caches, ownership regressions, allocation-prefix sweeps
and actual GPU pixels. Windows OpenGL and headless Linux software EGL both
read back 64 red and 33 white pixels. Aggregate tests additionally keep the
actual cached raster alive in a queued public draw after the last SVG is gone.
Four old-code assertion stops reproduce moving records and swallowed OOM before
invalid storage is accessed (six build/run command rows). Linux ASan/UBSan
instruments the aggregate only; remaining archive objects and host XRT are
uninstrumented. No network, audio-device or hardware performance test ran.

This qualifies native prerequisites, not complete safe SVG language bindings.
Clamped byte-prefix inputs, creator-thread admission, fallback/thread policy,
complete Svg/SvgPaint/SvgCache declarations and final matched delivery remain
open. No release/IDE files are installed by this batch.
