# Checked native XUI lifecycle

This contract describes the owned-Proxy path. Legacy raw backend handles do
not acquire an ownership protocol by being cast to an owned resource.

## Context and resources

`xuiCreateOwned(proxy, &context)` binds and retains one canonical owned Proxy
before publishing the Context. The backend is immutable. A Font or Surface
stored in that Context must belong to the same backend. Retention completes
before a slot is published; replacement publishes the complete new state
before releasing old owners. A failed hold leaves the previous state intact.

The Context traces actual resource, widget and manager slots. A child language
view does not introduce another native owning edge for those same slots.
Widget cache Draw and Surface owners are separate real references, including
partially produced results and pending cleanup. Trace never reconstructs an
owner from a stale descriptor, a hash key or a borrowed pointer.

## Closing and consumption

`xuiRequestClose` immediately stops business operations and transactions.
Physical destruction is a separate step. `xuiDestroyOwnerChecked` always
initializes `consumed` to false. It consumes that owner only after actual
preparation completes. A positive `XUI_STATUS_DEFERRED` is a normal wait:
the owner and its physical lifetime remain intact, and no diagnostic is
required. Negative failures are independent of the consumption result and
may have a NULL diagnostic, including under allocation failure.

Idle preparation can release independent ready resources even when some
other graph edge is busy. It preserves every unconsumed slot for retry.
Nested Draw sessions must actually End in native order; releasing an extra
Draw reference is not a substitute for ending the session.

The legacy void close transfers its existing destruction responsibility to
the Context while an active native call/callback unwinds. It completes at
that tail when preparation is possible. Code which may have independently
busy resources must use the checked interface and retain its retry owner;
the void API has no consumption result or independent scheduling guarantee.

## Receipts and errors

The caller owns a zero-initialized, address-stable `xui_call_scope_t`. Begin
and End allocate no storage. Owned Context admission and exit require the
Proxy creator thread. Failed admission does not change the token, the
operation depth, or the caller's existing diagnostic.

The receipt is 48 bytes on the accepted 64-bit targets. Its state distinguishes:

| State | Meaning |
| --- | --- |
| Success | The error slot owns the entry ambient diagnostic. It is hidden during the operation and restored on success. |
| Local failure | A checked operation returns its own status and diagnostic. It is not automatically a consuming-cleanup failure of its caller. |
| Propagated failure | Consuming cleanup, or an unhandled boundary diagnostic, has populated an enclosing receipt. Its exact first status and nullable diagnostic are authoritative. |
| Inherited primary | A nested consuming cleanup must deliver an earlier propagated receipt or explicit cleanup prefix instead of a later Drop error. |
| Ending | Reentry on this token is rejected while its native tail executes. |

The last three states use flag bits in the existing receipt word. An ordinary
local error cannot become an inherited cleanup primary merely because both
have nonzero statuses. An unrelated old ambient diagnostic cannot become a
new failure merely because it is non-NULL. A failed-with-NULL primary cannot
be replaced by a later allocated diagnostic.

Balanced tokens may End out of order. An ending outer receipt transfers the
inherited ambient owner to the remaining inner receipt; it does not create a
new failure. Tokens remain linked through the physical cleanup tail, so an
outer checked boundary observes deferred destructor failures before its
storage is zeroed. A live token must never be copied or moved.

## Qualification and cost

The development acceptance uses real production native objects with CPU
fake backends on Windows and Linux, at O0 and O2. It includes callback Hold
reentry, logical close, transaction rejection, resource graphs, failed outputs,
partial cleanup, NULL errors, forced allocation failure, mixed widget/native
ownership, arbitrary balanced receipt order and wrong-thread admission.
This does not certify GPU/device behavior or a separately shipped SDK.

No forwarding C wrapper is required by these contracts. They still perform
real ownership work: reference holds/releases, slot validation, scope entry
and exit, graph visits, and preallocated language result carriers. Nested
receipts retain diagnostics without allocating; resource publication and
language result carriers can allocate. These costs must be reported against
an equivalent handwritten C workload before claiming zero total overhead.
