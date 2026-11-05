# Document UI Owned callbacks

The legacy C descriptor callbacks keep their original caller-managed lifetime.
Safe bindings use these typed Native registrations:

| Registration | Callback inputs |
| --- | --- |
| `xuiDocumentViewSetActivateOwned` | View, node ID, callback-period `const xstrview*`, independent user |
| `xuiDocumentEditorSetErrorOwned` | Editor, native checked error code, independent user |
| `xuiMessageListSetNodeDocumentActivateOwned` | List, node ID, callback-period `const xstrview*`, independent user |
| `xuiMessageListSetNodeDocumentTaskToggleOwned` | List, item ID, checked state, independent user |

Function signatures remain typed throughout publication and dispatch. Each
registration owns its own callback owner and user, even when the other Message
binding callback uses a different closure. Clearing a registration releases its
one owning reference. Captured owners enter the actual Context ownership trace;
the implementation does not invent an RC count for a Context or flatten a real
independent RC node.

The View's pointer and accessibility activation consumers use the same snapshot.
Its retained Document Snapshot keeps the resource bytes alive through callback
return and final snapshot Drop. The native URL view lives on the C stack through
that entire interval. A binding may explicitly copy the view for later use;
keeping the view itself after the callback is invalid. No URL copy or `strlen`
is performed by dispatch.

Editor errors dispatch the selected callback while preserving the prior business
diagnostic. A pre-existing error does not reject a successful owner Hold. Failure
to acquire the callback is still a checked error, including false with NULL.

Message bindings are real heap bodies, not stable addresses inside the movable
message array. A stack callback scope pins the binding body through Hold, the
callback, output consumption and Drop. Replacing/removing an active binding
retains its retiring body in the existing work slot until that tail ends. It
does not free the binding from inside its callback or allocate a new retirement
queue. The active and retiring binding traces contain their exact callback
owners. Cleanup drops them outside the ownership mutation gate and preserves
the first nullable failure.

All operations follow the existing Context creator-thread contract. Callback
registration and same-slot replacement may reenter: candidate Hold runs before
publication, and the resulting current slot is read afterward. A failed outer
candidate cannot overwrite a valid nested replacement. Holding a callback while
requesting Context closure rejects new dispatch/publication and rolls back only
the newly acquired reference. Callback code remains alive until the physical
snapshot Drop, including self-clear and closure from inside the callback.

Costs on x64: View TypeData grows by two pointer fields (16 bytes; inherited by
Editor). Each bound Message document adds two function/user pointers, two
32-byte owner records and an additional independent task user (88 bytes after
reuse of existing padding). The callback activity counter reuses existing
padding. No additional per-callback allocation is added to a Message binding.
View/Editor use the existing sparse Widget callback-owner directory, allocated
only for registered owned callbacks. Dispatch adds normal native scope,
ownership mutation and Hold/Drop bookkeeping; it does not claim those are free.

CPU-only commands, using a matched Native source/engine and host XRT runtime:

```
python -B tools/test/check_xui_owned_cache.py --source SOURCE --engine ENGINE --runtime XL_RUNTIME --out NEW_REPORT --case doc_ui_callbacks --case doc_editor_regression
```

The focused fixture uses the exact production View, Editor, accessibility
adapter and MessageList TUs, excluding only their duplicate objects at link.
It covers replace/self-clear/close, binding replacement while active, OOM,
Hold and Drop reentry, true Hold plus fresh error, false/NULL, secondary cleanup,
resource-view lifetime through Drop and independent real edges in two Contexts.
The second case runs the pre-existing offscreen Editor behavior suite including
View accessibility and async editing. No engine/window/GPU/browser/network is
initialized. Native C graph tests establish real counts and edges; final language
collector and module-unload qualification is a separate integration gate.
