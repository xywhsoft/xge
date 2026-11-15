# Owned CodeProvider callbacks

The five `xuiCodeProviderSet*Owned` registrations accept their original typed
callback, userdata and an `xui_callback_owner_t`. Completion, Hover, Signature,
Command and CommandEnabled each have a separate immutable registration node.
Publication holds its owner once. Replacement, clear and destruction detach
the registration before dropping it. Active native scopes retain the node,
without calling the owner's Hold for every request. Raw registration functions
remain available with their original external ownership obligations.
A NULL/empty owner remains usable for externally managed C callbacks; it adds
no graph edge or owner Hold. A language closure must supply its real owner.

For owned Completion, Hover and Signature, open an address-stable, zeroed
`xui_code_provider_scope_t` with `xuiCodeProviderSetCallBegin`, issue the request,
copy or use its borrowed results, then call `xuiCodeProviderSetCallEnd`. A request
without that enclosing scope returns INVALID_STATE before calling the provider.
The selected node is retained by the caller scope until its next request of the
same role or its end. Clearing/replacing registrations cannot free that node
prematurely. The provider still controls its string/array storage: ownership of
the callback is not a deep copy of arbitrary returned pointers. Command and
CommandEnabled return scalar values and require no additional caller scope.

Do not move/copy an active scope, reuse its storage before End, or access its
results after End. Nested scopes can end out of order. A monotonic generation
also detects a caller scope ended and reopened by an owner Drop during a
request. Recheck it before invocation, before the request scope ends and after
its cleanup. If protection is lost, do not publish the result. Physical model
destruction waits for all scopes; logical close rejects new publication.

CodeEdit completion and signature consumers now retain this provider scope
through copying the returned data, including calls originating inside editor
event handling. Embedded providers also protect the enclosing Context. The
existing raw callback close/error regressions continue to exercise this path.

The graph reports one owner edge per published owned registration and refuses
quiescent tracing while a call is active. Owner Hold/Drop, callback errors,
allocation failures and post-commit cleanup failures use checked outcomes;
preserve the first ambient diagnostic. Failure after replacement has committed
does not restore the prior registration. Owner cleanup must support the calling
thread. Standalone sets still require external synchronization, and active
cross-thread use returns BUSY.

On the qualified x64 platforms the model is one 176-byte allocation; the prior
lifetime-only model was 120 bytes. An owned node adds one 56-byte allocation and
one owner Hold. The caller scope is 136 stack bytes, previously 72. A warmed
5000-request loop across all five owned roles has zero heap operations under a
zero allocation budget and no per-request owner Hold. Node reference counts,
five-slot scope snapshots, generation/state/thread checks, ownership mutation
admission and error/Context bookkeeping remain real work. This is not a CPU
parity or zero-cost claim.

`tools/test/check_code_provider_lifetime.py --owned` qualifies O0/O2 clients
against full O2 hosted and static SDKs, public C11/C++11/C++17 headers, original
raw providers, five-role self-clear/self-close/error, Hold/Drop reentry and
failure, allocation failure, graph edges, generation reuse, complete-heap
balance and CodeEdit copying after self-removal. The existing constructor
retirement/OOM and owned Margin tests run in the same gate. Production source
freezes exclude unrelated user pixel geometry and generated HarfBuzz caches.
Consumer allocation-prefix tests also retain the original memory diagnostic
when nested checked Context cleanup reports CALLBACK_FAILED; that status must
not discard the XERR_MEMORY cause. All complete heaps balance after cleanup.

This changes the public scope layout; consumers must rebuild with the matching
header and library. No existing installed runtime is updated by this source
qualification. Safe language callbacks with owning strings/arrays, aliases of
an editor's internal model and generic output marshalling remain separate work.
See `docs/validation/code-provider-owned-20261115.json` for the pinned evidence.
