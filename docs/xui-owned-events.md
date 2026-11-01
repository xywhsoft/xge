# Owned XUI event userdata

`xuiWidgetSetEventCallbackOwned` and `xuiWidgetSetEventHandlerOwned` add actual
owned userdata to the existing typed C event callbacks. They borrow an immutable
`xui_callback_owner_t` and acquire one reference before publication. A failed
admission leaves the old callback untouched. Replacement is committed before
consuming old cleanup; a cleanup error does not undo that replacement.

An invocation acquires a separate snapshot reference, calls the exact typed
function, and drops the snapshot **after that function returns**. Thus a
callback can replace itself, recursively dispatch, or close its context without
freeing the running userdata or unmapping its adapter return instructions.
The host must provide resident Hold/Drop hooks and a complete ownership reference.
Hold is a reference transition only. Drop always consumes, even on false.

Ordinary C setters keep borrowed userdata. When replacing an owned slot they
perform its checked cleanup; previously borrowed-only widgets allocate no owner
storage. Owned event storage uses one lazy allocation per widget and one extra
pointer in the private widget record. Stable event invocation allocates nothing,
but performs the required snapshot RC transitions. These are explicit lifetime
costs, not a claim of universally zero-cost language binding.

`xuiCallScopeEnd` keeps a checked receipt linked through native deferred cleanup.
It returns `XUI_ERROR_CALLBACK_FAILED` for a failed owned destructor even if no
diagnostic was allocated. Empty tokens are reusable; ending tokens reject
reentry. Logical invalidation is immediate, physical destruction is deferred.
The scope record grew; regenerate consumers from the matching authoritative
header and do not mix old records with this engine binary.

`xuiContextTraceEventOwners` enumerates each actual event owner once per slot.
It refuses active native operations, callbacks and destruction. It covers only
these event slots: it is **not** a complete graph policy for other XUI callbacks,
shared resources, workflow/flowgraph values or custom widget data. Those need
their own exact contracts before an entire XUI context can be admitted.

Offline checks: `tools/test/check_xui_lifetime.py` batches the existing widget
lifetime, weak observation and 47 drag/drop reentry cases with owned event
replacement, recursive dispatch, context close, no-diagnostic cleanup errors,
OOM/admission rollback, actual graph edges and stable-path allocation counters.
Windows O0/O2 and Linux O0/O2 with ASan/UBSan/leak detection passed on 2026-11-02.
No desktop input, rendering window or network test is used by these CPU gates.
