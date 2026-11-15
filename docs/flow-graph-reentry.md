# FlowGraph configuration callback reentry

FlowGraph operations require creator-thread use or external serialization.
This rule does not add concurrent access to a model.

Deep-copying or releasing a graph-owned configuration can invoke custom XRT
value callbacks. Until that copy/release returns, every ordinary operation on
the same model returns `XUI_ERROR_BUSY` before inspecting other arguments or
writing outputs. This includes reads, counts, revision queries, mutation,
command snapshots, undo/redo, history cleanup and destruction. Independent
models remain usable, including nested configuration operations.

Use `xuiFlowGraphDestroyChecked`, `xuiFlowGraphGetRevisionChecked` and
`xuiFlowGraphGetCleanRevisionChecked` when status is required. Rejected destroy
does not consume the graph; rejected queries leave their output unchanged.
Busy rejection itself allocates nothing. Existing integer count/find/boolean
queries can also return a negative Busy status; do not interpret it as a count
or a true boolean.

The legacy void destroy and unsigned queries cannot return a status. They
refuse the operation, return zero where applicable, and set an XRT state error
with domain `xui.flow_graph` and code `XUI_ERROR_BUSY` when no earlier error is
pending. An existing error remains intact. Ownership tracing requires a frozen,
quiescent model; a trace attempted during a configuration callback fails without
calling the visitor.

Workflow entrypoints targeting this model obey the same rule. Staged graph
replacement also marks its logical destination busy while copying new configs
or dropping old configs. Legacy workflow destruction leaves the owner alive,
and its graph getter returns NULL with the same error-reporting rule. This
does not establish a general reentry policy for every workflow validation,
dynamic-port, schema or variable callback.

No configuration contents are copied for this guard. Each model keeps a
callback depth and a borrowed internal replacement-owner pointer. Copy/Drop
brackets increment/decrement the corresponding depth; ordinary entrypoints
make a busy check. There is no new lock or global exclusion. This bookkeeping
has a cost and is not a CPU zero-overhead claim.

Run `tools/test/check_flow_graph_reentry.py --help` for the exact-cohort source
gate. It tests all 63 status-bearing existing-model entrypoints and all 30
existing-workflow integer entrypoints, output canaries, independent and nested
models, old APIs, command history, destruction, replacement and 176 allocation
failure prefixes at both O0 and O2. It also runs existing model, schema and XSON
tests and compiles the widget consumer. The report explicitly distinguishes
source-unit validation from a rebuilt SDK or a shipping IDE.
