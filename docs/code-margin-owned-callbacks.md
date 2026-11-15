# Owned CodeMargin callbacks

`SetRenderCallbackOwned` and `SetEventCallbackOwned` publish independent typed
slots. Each non-NULL slot allocates one immutable node and acquires its declared
owner once. A NULL callback clears that role. The old descriptor remains a
borrowed C input; `Add` replacement, `Remove`, `ClearChecked`, defaults and
destruction detach the affected owned slots before dropping their references.
Drop may reenter, and never runs while the graph mutation gate is held.

Hold is arbitrary user code. Publication rereads model admission, Context close
state and the requested id after Hold. A true Hold with a fresh diagnostic is
rolled back with exactly one Drop; a false Hold acquired nothing. Drop consumes
the reference even when it fails. Replacement can therefore commit before a
consuming Drop reports an error; retrying the old Drop is incorrect. Cleanup
retains the first failed status even without a diagnostic. Standalone scopes
isolate ambient diagnostics just as Context scopes do, restoring them only on
success rather than mistaking them for a fresh Hold failure.

CodeEdit captures a private layout snapshot for its whole cache render. The
snapshot pins the exact render slots, independent of later model replacement or
clear. Each actual invocation also keeps a private pin and protects its Widget
operation. Snapshot end/reuse is rejected while invoking or ending, and copied
live snapshots cannot end the original. Frame storage is reused by CodeEdit;
no frame or mutable callback-bearing array is a public language result.
Events capture their own typed event slot and user data, independent of render
user data. The native functions are called directly once, without boxing or
language-specific forwarding functions.

Closing the Widget stops subsequent callback invocations. The legacy public
`xuiRender` walks a best-effort void preparation tree and can return OK after
retiring its root; the recorded callback failure reaches an enclosing native
checked call scope. The tests verify both the stopped invocation sequence and
that checked outcome. A Context close returning DEFERRED still needs the
existing explicit retry after its protecting operation ends.

`GetState` copies only numeric metadata. Legacy `Get` refuses an entry with
owned slots, and legacy `Layout` refuses a model with owned slots, before
exporting pointers or modifying outputs. Their shared single `pUser` cannot
represent the two independent owned users. Purely borrowed descriptors retain
their old behavior. GetCount retains its count-or-zero legacy form; checked
clients use an admitted model scope.

The unique model traces one graph edge per published owner, including two
separate edges when both roles use the same owner. Active scopes/frames make
quiescent inspection busy. Logical owner consumption remains distinct from
physical storage release and from cleanup failure.

Measured x64 requested storage is 64 bytes per model (56 in the preceding
lifetime-only foundation), 72 per non-NULL callback slot and one allocation
each. Numeric descriptors still use the existing capacity array. Private
layout entries use 72 bytes (64 previously) and the frame uses 96 C-stack bytes;
the model call scope remains 72. Prepared 1000-frame/invocation loops perform
zero heap operations and zero external owner Hold/Drop calls. Private refcount
changes, thread/admission checks, graph gates, Context scopes and cleanup still
cost work. Built-in margins retain direct drawing, with no per-line callback
consumer call. No CPU-equivalence or zero-cost safety claim is made.

`tools/test/check_code_margin_owned.py` qualifies O0/O2 hosted/static callers,
C11/C++11/C++17 headers, publication OOM, independent/duplicate owner edges,
true/false Hold failure, consuming Drop failure, reentrant clear/close, actual
CodeEdit frame/event use, copied/busy/capacity snapshots, heap balance and the
prior numeric, lifetime, constructor-retirement and full CodeEdit regressions.
Build inputs exclude unrelated pixel geometry changes. See the dated receipt
under `docs/validation/`. Language aliases, output-array projection, devices
and whole IDE/release qualification remain separate work.
