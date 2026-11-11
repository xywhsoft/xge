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
