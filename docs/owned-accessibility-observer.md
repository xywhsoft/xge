# Owned accessibility event observers

`xuiSetAccessibilityEventCallbackOwned` retains a supplied shared callback
owner before publishing the typed observer and user pointer. Allocation/Hold
failure preserves the preceding observer. The raw setter delegates with no
owner and remains available to native callers.

The Context stores one registration-record pointer and traces its owner.
Notification pins that record, not the arbitrary owner Hold hook, through the
selected callback. Self-removal/replacement cannot release its active payload.
Both observers are disabled before Context teardown invokes consuming owner
Drops. Context operation scopes defer physical Widget/Context destruction until
notification work and snapshot cleanup have finished. Publication/detachment
uses the ownership mutation gate; arbitrary callbacks and Drops run outside it.
Owned Contexts use the creating Proxy's thread checks; normal UI-thread rules
remain in effect.

Explicit `xuiWidgetNotifyAccessibility` is synchronous. Existing coalescing and
same-kind recursion suppression remain intact. Notifications carry the full
unsigned 64-bit provider-local node ID and nonzero revision. An observer or
cleanup diagnostic produces a checked status/first-error outcome; notification
does not roll back the change that preceded it. Logical close during backend
notification prevents later observer admission. With no observer, the snapshot
mutation gate is skipped.

The record is the existing 48-byte generic callback record. The Context adds
one pointer; registration allocates/holds once and dispatch retains/releases
the private record. Receiver/operation, graph, TLS/error and lifecycle work is
still required. No CPU equivalence or whole-API zero-cost claim is made.

`tools/test/check_owned_accessibility_observer.py` checks the actual hosted and
static O2 SDKs with C O0/O2 clients. It exercises owner graph references,
self-unregister/replace, raw handoff, Widget/Context close, same-kind recursion,
Drop-register reentry, allocation prefixes, failed Hold/callback/Drop, real
record counter overflow and 1000 allocation-free notifications with no extra
owner Hold. It also runs the unchanged public accessibility suite. The existing
owned-input checker separately regresses hotkey/error ownership and legacy
input routing. The Native SDK is built separately from language integration
and final distribution.
