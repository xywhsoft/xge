# Owned global callbacks

Error, quit-request and drag-event callbacks have `Owned` setters accepting
`xge_callback_owner_t`. Its ownership reference and Hold/Drop functions follow
the same physical ABI as the XUI callback owner. Failed Hold acquires nothing;
Drop consumes one reference even when cleanup reports failure.

The setter obtains a real owner reference and allocates one registration node
before publication. Admission and allocation failures preserve the old slot.
Publication occurs under a short publisher lock, followed by old-registration
cleanup outside the lock. A cleanup failure after publication is checked and
does not roll the publication back. Reentrant owner code may replace or clear
the same slot. An empty owner preserves the raw C caller-managed lifetime
contract and allocates no registration node.

A dispatcher pins the native registration node under the publisher lock,
then holds an independent callback owner outside the lock. It releases the
registration reader and invokes the exact typed native function directly.
The snapshot owner is released only after the callback's C return. A callback
can clear its own slot without releasing its code or userdata prematurely.
No owner hook, callback, allocation or final destruction runs under a
publisher spin lock. Dispatch adds no heap allocation.

These are external native registrations. They remain roots until explicitly
cleared and are not represented as automatically collectible language module
fields. A language host must independently keep the native publisher library
resident through its own physical return tail. Retiring a language callback
does not itself prove that another native thread has stopped executing DLL
instructions. Stop/join publishers before acknowledging quiescence to a
checked module retirement API.

`xgeQuitRequestDispatch(int*)`, `xgeRequestQuitChecked(int*)` and
`xgeDragEventDispatchChecked(event, effect*)` distinguish native status from
veto/effect. They write outputs only on success. The first quit function only
evaluates a close request for a custom platform backend; it does not initialize
or stop an engine. Reentrant quit evaluation vetoes without invoking another
callback. The old boolean/effect entry points remain available to native C.

`xgeUnitChecked()` clears quit and drag registrations, including registrations
made before engine initialization. It returns a checked failure when an Owned
callback or owner admission is active, and rejects reentrant shutdown. A short
shutdown gate rejects initialization and new quit/drag registrations during
teardown. Every cleanup step runs; the first failure stays primary even when
its diagnostic is NULL and a later cleanup creates an error. The old void
`xgeUnit()` delegates to this operation and preserves a diagnostic when one is
available; language declarations should use the checked entry.

The Error callback remains installed across engine shutdown so it can report
shutdown diagnostics. Clear it explicitly with
`xgeSetErrorCallbackOwned(NULL, NULL, NULL)`. Error reporting retains the
existing recursion/concurrent-report suppression behavior. Error callbacks can
run on reporting threads; quit and drag engine operations keep their existing
UI-thread requirements. Thread-safe registration is not permission to invoke
arbitrary engine methods from worker callbacks.

`tests/xge/xge_owned_global_test.c` tests the actual native APIs without a
window, renderer or device: registration OOM and Hold refusal, self-clear
snapshots, checked output preservation, pre-init shutdown, reentry refusal,
NULL-first cleanup failures and concurrent Error callback replacement. Run it
with `tools/test/check_xge_global_owned.py` against a matching hosted engine
and explicit XRT runtime. The resulting report does not certify GPU behavior
or language-package delivery.
