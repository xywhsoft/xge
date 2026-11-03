# Native XUI owning payloads

`xuiContextTraceOwnership` enumerates actual owning references in a quiescent
context, including unattached widgets. It combines the existing owned event
slots with native type payload tracers. The caller holds whole-domain ownership
Freeze on the UI thread; a visitor or type tracer must not allocate, mutate,
acquire/release references, lock, or invoke language code. A false result
invalidates the whole partial enumeration.

A custom native type configures `xuiWidgetTypeSetOwnershipTrace` while it has no
live instances, including derived instances. The context calls each type's
tracer once along the instance's parent chain. Each tracer visits only the
owning slots introduced by that type. Its C code must remain resident with the
context; the hook does not own arbitrary `pUser` data. Independently shared
native owners need separate graph nodes, not recursive flattening.

The built-in model slots are:

| Native owner | Actual owning slots |
| --- | --- |
| FlowGraph | Current node configs; old/new config snapshots in undo and redo; remove-node snapshots inside delete-selection groups |
| Workflow | Its uniquely owned graph; registered type config schemas; variable defaults |
| Owning FlowGraph widget | Its unique model's slots |
| Owning Workflow widget | Its unique workflow's slots, including that workflow's graph |
| Borrowing graph/workflow widget | No ownership of the borrowed model |

`xuiFlowGraphTraceOwnedValues` and `xuiWorkflowTraceOwnedValues` expose the same
read-only enumeration for native owners outside a context. They visit every
physical `xvalue` owning slot, even when several slots point to the same value.
They do not deep-clone values, infer ownership from raw userdata, or call graph
operations supplied by an unknown handle. Graph admission belongs to the
collector that receives these references.

The native command model currently has one group level: delete-selection owns
remove-node/remove-edge records. Enumeration checks that closed shape and
refuses deeper unknown payloads rather than recursing on an unbounded C stack.
Array traversal uses `size_t` and zero-based bounds. The existing native model
APIs still perform their documented config/schema copies; tracing adds none.

The normal render/event path does not execute the payload tracers. Each
registered native type has one additional function pointer; widget instances
have no additional payload field or allocation for this mechanism. Tracing is
linear in the live widgets, type chains, and model slots. This is ownership
bookkeeping, not a claim that managed bindings have the cost of raw C pointers.

`tools/test/check_xui_lifetime.py` runs model payload, inheritance, lifetime,
observer, owned event, and drag/drop checks at O0/O2. The new payload test checks
undo/redo/group slots, schema/defaults, early refusal, existing diagnostics, and
200 enumerations with allocation failure enabled. Linux supports
`--sanitize` for ASan/UBSan and leak checking.

The companion xlang hosted-engine fixture also closes cycles through actual
FlowGraph/Workflow widgets, captured event closures, boxed views, and multiple
borrowed canvases. This does not introduce managed xlang declarations or finish
the remaining borrowed-callback, resource lifetime, SDK, or package migration.
