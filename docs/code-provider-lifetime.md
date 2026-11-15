# CodeProviderSet lifetime foundation

This records the lifetime-only baseline. The successor
`code-provider-owned-callbacks.md` adds owned registrations, graph edges,
borrow-result pinning and a larger public scope; use its matched SDK sizes.

`xuiCodeProviderSetCreate` grants one destroy right. CodeEdit claims its own
set; `xuiCodeEditGetProviders` returns a borrow, and public checked destruction
of that borrow returns INVALID_STATE with `consumed=false`. This adds neither
shared ownership nor an owning language alias of the editor's internal model.

`DestroyOwnerChecked` separates status from consumption. After consumption,
new operations fail; active native scopes delay physical free. Lifetime
invalidators run only after detachment, cannot resurrect the set, and can fail
after consumption. A consumed owner must never be retried. Legacy void destroy
and clear delegate to checked versions but cannot report their status.

Call scopes are zero-initialized, address-stable C-stack records. Nested scopes
pin the set; embedded sets also pin the editor's Context operation. A copied
scope cannot release the original pin. A live scope cannot be reused or ended
twice. A standalone set remains externally synchronized, as before: quiescent
transfer between threads is allowed, callbacks execute on the requesting
thread, and another thread cannot use/end an already active call chain.
This is not a new concurrent container or a cross-thread callback scheduler.

All five request paths snapshot their raw function/user pair before invoking
it, isolate the ambient diagnostic, validate state and returned descriptors,
and clear failed outputs. Closing the editor or requesting Context close in
the callback rejects result publication. Clearing/replacing callbacks in a
callback does not corrupt the in-flight raw function/user pair. Their backing
storage is still the C provider's responsibility.

Completion, signature help and menu-title updates protect the entire public
consumer operation, including result consumption and local cleanup. Otherwise
the inner provider scope could flush editor destruction while its caller was
still using editor data. Menu-state callback failures are now returned instead
of silently being converted to a disabled item. A title assignment can already
be committed when its subsequent menu update fails; failure is not rollback.

Destroying an editor during a request consumes it logically, then frees it at
the outer operation tail. Context close returning DEFERRED has not consumed its
owner: end the operation, then explicitly retry Context destruction. These are
different outcomes; native scopes do not invent automatic Context retries.
Quiescent ownership inspection still works after a deferred Context close:
refusing new calls must not hide resources that the Context still owns.

The x64 set requests 120 heap bytes, versus 80 before, in one allocation. Its
public scope occupies 72 caller-owned bytes. A warmed 1000-scope/request loop
requires no heap operations, including with allocation budget zero. Thread
queries, mutation admission, error reference handling, state checks and Context
bookkeeping are real costs. No CPU-equivalence claim is made.

`tools/test/check_code_provider_lifetime.py` exercises hosted/static O0/O2
clients, C11/C++11/C++17 headers, all five raw request families, reentry, close,
scope/address/thread checks, lifetime invalidator failures, whole-heap balance,
constructor allocation prefixes and existing owned Margin consumers. Fixtures
are pinned separately from the immutable production source cohort. Generated
HarfBuzz caches belong outside the production-input freeze; earlier failed
freeze/probe records remain in external validation materials.

This is a native prerequisite. Raw callbacks, userdata, returned strings and
parameter arrays are still borrowed under the existing public contract.
There are no owning language callbacks or safe language result publication
for ProviderSet yet. Ownership trace has no edges only because these callbacks
remain raw C inputs; do not expose future language closures as an edge-free
leaf. Internal aliases and owning callback outputs need their own contracts.
No complete xlang/XGE/IDE or final shipping qualification is implied.
