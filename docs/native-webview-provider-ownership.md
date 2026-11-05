# Native WebView and Document web provider lifetime

The WebView is still a native Widget. `SetEventOwned` publishes the exact
`xui_webview_event_proc` signature through the shared Widget callback directory.
Registration and invocation snapshots retain the actual userdata/code owner.
Self-clear, replacement, requested Context close and cleanup with a NULL
diagnostic use the same checked callback/Context receipts as other Widgets.
The ordinary C setter also releases an old owned registration correctly.

Both Event and the private Document Message consumer hold a native Context
activity scope through their complete return tail. Message source/JSON buffers
remain borrowed until callback and snapshot Drop have finished. The private
Message channel remains an internal C implementation detail; this change does
not claim that a new public scripting/IPC API was added.

## Delegate and module lifetime

Every existing WebView2 delegate retains the real WebView data record. Merely
clearing `data.widget` would prevent a userdata dereference but would not keep
the delegate's own vtable code resident after Context/module retirement.

The native Widget therefore has true destruction preparation. Its first
Prepare marks the data retiring/closed and clears callbacks before stopping the
backend, removing registrations and cancelling requests. It then waits for all
delegate references to return to the Widget's sole data owner. Waiting keeps
the actual Widget and Context in their existing retirement carriers; the host
retains their code lease. Initial pure readiness permits this revocation step;
subsequent readiness rejects outstanding delegate references.

Delegate Release only decreases its reference. It does not destroy the Widget
or unload code from its own COM return frame. A later creator-thread checked
host boundary retries physical destruction. Native callback activity scopes
independently protect currently executing callbacks. The existing WebView2
STA/UI-thread contract still applies; this is not a thread-safe general UI API.

## Context-owned Document provider

The Context uniquely owns the provider, as it does the other native composite
managers. The previous contract already required release before Parent/Context
destruction; the handle is not independently usable after Context destruction.
Create installs one intrusive manager node in the same provider allocation.
Explicit Release requests logical close and invalidates watches immediately;
the Context retains physical storage until its worker has been destroyed and
all native provider call tails have returned. Parent destruction requests the
same close. The worker's internal native owner identity is present before Init,
so public destroy/detach/reparent cannot invalidate the provider's worker slot.

The provider retains the Context's canonical owned Proxy. It no longer copies
the complete vtable or treats a copy's address as the native resource owner.
Each cached Surface and each of two bounded producer/crop slots is a real
owning reference, included once in native tracing and checked preparation.
Partial non-NULL outputs stay in those actual slots. An unconsumed release
keeps its carrier; status and consumption are independent. The source/capture
request is also retained through decoding if reentrant close releases the
provider's request reference. Existing eviction, cropping, byte limits and
palette keys remain in use.

An owned invalidate callback is a real manager slot and trace edge. Whole
native operations reserve the provider and hold its scope through their tail;
recursive mutation returns BUSY. Pure stats/context getters do not dispatch or
drain a Context. Error-card measurement/drawing retains the resolved Font
through callbacks, even if the default Font is replaced. Explicit first-error
prefixes preserve both primary status and a NULL diagnostic across subsequent
consuming cleanup. Ordinary returned failures remain local; actual cleanup
failure reaches the Context's existing receipt.

## Cost and CPU acceptance

On the measured x64 ABI the provider shrinks from 33,672 to 33,288 bytes by
replacing its by-value vtable with the canonical pointer. Its 64 entries remain
512 bytes each. Portable WebView data grows from 56 to 64 bytes for physical
reference/retiring state. No per-operation manager, queue or wrapper allocation
is added. Native scopes, real resource references and callback snapshots do
have bookkeeping cost; no claim of zero bookkeeping is made.

The Windows/Linux O0/O2 tests link actual production Provider/WebView code into
the CPU fixture and omit their old object files. A test-only replacement for
`InitializeAsync` prevents starting a browser; Surface I/O is a native CPU
backend. Tests cover actual Proxy/Surface graph counts, crop ownership, partial
output, first NULL failure and separate sole Drop failure, Font replacement at
the measurement tail, 32 construction allocation-failure budgets, same-provider
reentry, Parent close, typed Event self-clear/destroy, owned creator-thread
admission, and pending delegate/module-carrier retirement. The Windows WebView2
backend is separately compiled with its SDK as a system include; it is not
executed. No window, engine, GPU, browser, network or security probe is run.
These are matched native CPU results, not final shipping-DLL or public package
acceptance.
