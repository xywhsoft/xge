# Owning Draw sessions

`xuiProxyXgeCreate` returns an owning provider. Its `drawBegin` produces an
owning `xui_draw_context`: one allocation contains a 96-byte shared Draw
prefix, the backend payload and its weak session-list link. Font and Surface
prefixes remain 80 bytes. The Draw owns one actual Proxy reference and, when
present, one actual Surface reference; its graph Trace visits those exact
edges. There is no separate language-facing rendering object or per-method
forwarder.

The default backend admits one manual owning session per backend and creator
thread. A second begin returns `XUI_ERROR_BUSY` before allocating or entering
a render target. All capability tables for that backend share this check.
The list contains only weak pointers embedded in live Draw records; it does
not retain a Draw, provider or module. Existing synchronous internal target
helpers retain their LIFO behavior. The old by-value `xuiProxyXge` and its raw
sessions retain their caller-managed C contract.

`xuiDrawEndChecked` consumes semantic End at most once, even if the backend
reports failure without an allocated diagnostic. It restores the target and
then releases the Draw's owning Proxy/Surface edges. A retained ownership-plan
snapshot may keep the terminal header alive; it does not prolong rendering
or target ownership. Repeating End performs no backend work. New AddRef and
ordinary rendering operations reject an ended or ending Draw.

An active `xuiDrawCallScopeBegin` protects the physical record. End requested
inside a call is performed by the final `xuiDrawCallScopeEnd`, even when
snapshot references still exist. Releasing the last native owning reference
also requests End. Reentrant End/close cannot free an executing finalizer:
the Native tail retains a temporary physical reference until End returns.
The release result separates owner consumption from cleanup status.

The preparation descriptor runs End outside graph Freeze, before any
dependency's semantic Finalize. Its pure Ready becomes true only after End
and edge cleanup complete. The planner refreshes the whole graph afterwards.
This avoids relying on traversal order when a Surface appears before its
Draw. The independent pure V2 DropReady permits snapshots of an open Draw
only on a suitable cleanup thread. Once End/Finish consumed all semantic
duties, a terminal header can be dropped on another thread.

The default backend is creator-affine. A custom provider may explicitly use
`XUI_PROXY_OWNED_ANYTHREAD_FINALIZE` only when its payload and owner cleanup
support that promise. This does not make its rendering methods thread-safe.
Backend End, checked owner release and semantic cleanup must be invoked
outside inherited graph Freeze. XRT MutationBegin is not an exclusion test
for a caller already holding Freeze.

The safe language binding uses `xl6Xui3DrawPolicy`: its Native Scope pins,
Trace and checked owner consumption are common lifecycle work. `.end` maps
to `xuiDrawEndChecked`; `.close` releases the managed owning reference.
It must not expose the old provider `drawEnd(proxy, draw)` as an owning class
method, because that C function consumes an actual native reference.
Callback Draw views remain borrowed for the callback epoch and are not
copied or allocated. Ordinary Proxy rendering fields still call their exact
native function pointer. Lifecycle synchronization is real work shared with
equivalent owning Native C; a claim of zero total execution overhead would
be inaccurate.

`xui_owned_draw_test.c` exercises the common protocol with a CPU provider.
The language repository's `foreign_xge3_package_draw_plan.c` exercises the
actual resident planner, including Surface-first discovery and failed End.
These tests initialize no engine, window, device or network. Default drawing
and target-restoration GPU behavior require a separate device qualification;
strict compilation is not a runtime rendering claim.
