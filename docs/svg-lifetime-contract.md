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
