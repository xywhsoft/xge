# Shared proxy, Font and Surface ownership

This implementation is in development. Component, matching-engine CPU and
actual resident-plan tests cover different boundaries. The current common
resource component has passed its CPU and actual planner gates. Earlier
matching-backend CPU proofs precede the new affinity/paint ABI and RGBA
hardening. Those changes still need matching-engine integration, together
with the complete Context/Widget/cache owning-edge trace, checked parent
cleanup and final language package delivery.

`xuiProxyXge()` and `xuiSetProxy()` retain their caller-managed C contract.
Their by-value tables contain no owned capability. Safe language declarations
use `xuiProxyXgeCreate()` and `xuiSetProxyOwned()` instead. A custom owned
provider uses `xuiProxyCreateOwned()` with an actual backend owner; a bare
userdata pointer is insufficient.

An owned proxy allocates one immutable function table with its native RC
capability. It holds the backend owner before publishing the result. Input
metadata is copied before invoking its Hold hook, and registration failure
does not publish an output. Ordinary operations continue to call function
pointer fields directly.

A resource allocates one common header and its backend payload together.
Font and Surface carry a real reference to the same proxy capability, rather
than a copy of the whole table. On the tested x64 ABIs the common header is
80 bytes, the proxy table is 584 bytes, and the callback owner is 32 bytes.
The complete capability allocation is 704 bytes, including alignment. These
are native ownership costs; direct language declarations do not make them
zero.

| Owner-release condition | Owner consumed | Result |
| --- | --- | --- |
| Invalid object, no such owner, or mutation preflight refusal | No | Checked failure; the owner is unchanged |
| Last release on the wrong cleanup thread | No | BUSY; retain the owner/pin and retry on its creator thread |
| Valid owner, including a claimed object | Yes | Exactly one owning reference is consumed |
| Other real references or physical pins remain | Yes | Physical storage remains protected |
| Last physical reference | Yes | One payload finalization, owning-edge cleanup and physical release |
| Payload or backend cleanup reports failure | Yes | Status is independent from consumption, including a NULL diagnostic |

The payload finalizer consumes its semantic payload on every return. A
nonzero status reports a cleanup failure and never authorizes a second call.
It must not free the common record. Providers whose cleanup can refuse
consumption need a different retryable carrier contract; they cannot use this
callback while silently retaining ownership.

Reference/owner/pin transitions use the XRT mutation gate followed by a short
object lock. The mutation gate admits concurrent mutators; it is not a mutex.
No allocator or payload/backend-owner hook executes under the object lock.
Readiness and busy queries use an atomic state word and immutable affinity
metadata. Native Count/Trace read ordinary counts and edges only under graph
freeze. Four concurrent threads exercise shared references/pins and last
cleanup; an in-flight finalizer is BUSY to another finalization attempt.

Owned providers default to creator-thread cleanup. A C provider may explicitly
select `XUI_PROXY_OWNED_ANYTHREAD_FINALIZE` in `xuiProxyCreateOwnedEx()` only
when all its resource and backend-owner cleanup supports that promise.
This flag does not make ordinary UI operations thread-safe. Font and Surface
copy the immutable creator/flag metadata into their common headers, so a
readiness query never follows an owning edge that Finish may already release.
Once Finalize and Finish have consumed every semantic duty, only the resident
XRT header remains and its final snapshot release may run on another thread.
The existing ForeignView creator-thread queue keeps ordinary language cleanup
and its real code owner until creator-thread drain.

Count reports actual references. Trace preserves the independent resource,
capability and backend-owner nodes. Claim blocks new admission; it does not
block an existing owner from releasing its reference. Finalize executes
outside graph freeze, while capability and userdata owners still exist.
Clear only quarantines owning slots and calls no user code. Finish consumes
those quarantined slots outside freeze. Native operations reject access to
a payload once its semantic finalization has begun. Retained glyph-shape
measurement and drawing apply that same readiness rule before using their
borrowed native Font addresses, including when another graph finalizer runs
after the Font payload has already been consumed.

Retained glyph paint owns one Font reference for each actual paint record.
Its physical carrier also keeps the native glyph run's borrowed Font record
addresses valid. A persistent cache must trace that exact owning edge once;
an alias to the same paint record is not another Font reference. A long-lived
cache reference is not represented as a busy call pin. Short active call pins
and their quiescence checks have a separate purpose.

The optional typed `paintFreeChecked` hook reports status and consumption
separately. The legacy void hook remains available to C callers. Checked
parent cleanup must propagate a failure even when diagnostic allocation fails;
it must not discard an unconsumed owning slot.

A retained paint admitted to managed caches additionally supplies the exact
40-byte `xui_text_paint_ownership_v1_t` contract. Its pure Ready checks the
paint's own immutable creator thread, Trace reports its actual owning edges,
and Drop consumes the admitted unique paint on every semantic result. The
default paint keeps one real Font reference and adds an 8-byte creator field;
the public shape adds one descriptor pointer. A record copy is an ownership
move, not a second paint owner. The legacy retry-capable hook remains for C
interoperation and does not by itself establish managed admission. Finalize,
Finish and paint Drop require calls outside ownership freeze; a nested XRT
mutation scope does not prove that precondition.

The V1 ownership adapter's void Drop cannot express a diagnostic-free cleanup
failure when an uncommitted snapshot holds the last physical reference.
The component provides the authoritative XRT FinishV2 descriptor with its
typed checked-Drop extension; its explicit native CPU test covers that final
snapshot failure under OOM. Independent actual resident-plan tests also cover
the native Font/capability/backend-owner cycle, the last uncommitted snapshot
release, first-NULL failure preservation, checked outer scopes and deferred
mailbox delivery. Earlier private language policy/domain tests exercise the
real Font/capability/native-callback/closure/ForeignView cycle; the new 48-byte
contract requires its matching rerun. Planner tests reuse the verified
six-unit FinishV2 runtime overlay and explicitly do not claim a new full
compiler build. No replacement error stack or per-method language
wrapper conceals that boundary. The unpublished V2 descriptor now also has a
required pure `DropReady` preflight. Before consuming any physical snapshot,
the planner must validate every node under the same freeze; one BUSY result
retains the entire plan for a later creator-thread retry. Earlier 40-byte
experimental descriptors and reports remain historical evidence, not
qualification of this 48-byte contract.
