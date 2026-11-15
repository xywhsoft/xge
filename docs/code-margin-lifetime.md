# Unique CodeMarginModel lifetime foundation

A standalone model has one native destroy right. CodeEdit claims the model it
creates; `xuiCodeEditGetMargins` returns a borrow. Calling the public checked
destroy on this internal model returns INVALID_STATE and leaves `consumed`
false. CodeEdit alone consumes that owner during its own cleanup. This does
not introduce sharing, attachment or a long-lived language alias contract.

`xuiCodeMarginModelDestroyOwnerChecked(model,&consumed)` separates failure
from consumption. When consumption succeeds, reads and mutations reject the
logically closed model. Physical storage stays pinned until its last native
scope ends. A lifetime invalidator can fail after the owner was consumed;
check both outputs and never retry a consumed destroy right. The old void
destroy delegates to checked destruction but cannot report an error.

`xuiCodeMarginModelCallBegin/End` use a caller-owned, zero-initialized,
address-stable scope. The scope pins the unique model and, for an internal
model, its Context operation. Nested scopes are supported. Copies, double
ends, reused live scopes and wrong-thread ends are rejected without releasing
the original pin. Native business methods validate their creating thread and
logical state; `ClearChecked` reports failures that the legacy void Clear
cannot. GetCount retains its count-or-zero C form; use a scope for checked
read admission. Ownership enumeration is read-only and refuses active models.

CodeEdit can be logically destroyed while a model scope is active. Context
tail cleanup then consumes its model owner, but the child storage survives
until the final scope end. End reports INVALID_STATE instead of silently
committing a result after that tail consumption. A Context close returning
DEFERRED is different: its owner is not consumed. End the protecting scope,
then explicitly retry Context close after quiescence. The lifetime test checks
both paths and full heap/backend balance.

Lifetime watches use the existing native intrusive link protocol. Invalidation
detaches each link before invoking it, and permits reentrant inspection or
unwatching. Closing state prevents resurrection and duplicate destruction.
Failure receipts retain the first outcome, including failures without a
diagnostic. XRT permits balanced same-thread mutation scopes during a frozen
inspection; no arbitrary callback or cross-thread wait runs under that freeze.

The x64 model occupies 56 requested heap bytes, compared with the former 16.
The first candidate used 64; putting its two closing flags in the magic field's
alignment gap removes 8 bytes. Creation still allocates one object. A public
scope occupies 72 caller-owned bytes. Thread checks, mutation admission,
Context call bookkeeping and lifetime invalidation remain costs. Prepared
1000-scope/width-query loops perform no heap operation; this is not a CPU
equivalence or zero-cost safety claim.

`tools/test/check_code_margin_lifetime.py` tests actual hosted/static libraries
at O0/O2, C11/C++11/C++17 prototypes, standalone and internal owner handling,
nested/copy/thread rejection, consumed cleanup failure, deferred Context retry,
and 130 calibrated CodeEdit constructor budgets per mode. The original margin
geometry/default/overflow/OOM regression now calls the public native library;
it no longer includes the implementation C file or replaces its allocator.
Its two failure controls use the real XRT allocation budget instead.

Windows/Linux initially build 164 hosted and 166 static translation units in
full O2 cohorts. The packing-only successor rebuilds one hosted unit and
reuses 163; static rebuilds that unit and its generated contract and reuses
164. Final QA sources are pinned separately from the immutable build freeze;
earlier incorrect freeze/Context expectations and a missing test include are
retained as diagnostics. See `validation/code-margin-lifetime-20261115.json`.

This foundation does **not** own the existing borrowed `onRender`, `onEvent`
or `pUser` payloads. Owned callback publication, independent invocation/layout
snapshots and their graph edges remain the next phase. It must not be exposed
as a callback-free leaf object in a language binding. Writable output arrays,
internal aliases and full xlang/IDE delivery retain their separate contracts.
No language, GPU, window, network or complete CodeMargin integration claim is
made by this native-only receipt.
