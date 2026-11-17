# Workflow schema/default value callback exclusion

Workflow now brackets every model-owned value Clone/Drop boundary with the
existing associated Graph callback depth. This makes the same Workflow and
its Graph reject reentrant operations with `XUI_ERROR_BUSY`, before reading
other arguments or writing outputs. Independent Workflow/Graph objects remain
usable. There is no global lock, new model field or extra allocation.

The boundaries include type-schema registration, variable defaults, default
configuration construction, export/rollback, and table destruction. Repeated
schema field IDs can replace a default: the underlying ObjectSetTake is also
bracketed because replacing its prior value can execute Drop. The standalone
schema builder has no owning model identity and retains its existing rules.

Import's temporary variable Workflow gives its Graph the existing target
callback identity. Its Clone and replaced-table Drop therefore protect the
target as well. The owned finalizer keeps the Graph reference until all schema
and default-value Drops finish; independently shared Graph aliases continue to
observe Busy during those Drops. Empty tables remain safe to revisit if graph
retirement needs a retry. The existing first-error cleanup behavior is kept.
The old finalizer regression now expects Busy during Drop instead of the
previous invalid-state code, matching the approved callback rule.

The focused test checks every existing integer Workflow/Graph entry, preserved
outputs, independent model mutation, legacy and shared identities, imported
variables, repeated defaults, failed custom Clone and 90 allocation budgets
per identity mode. Heap accounting checks for leaks and invalid/double frees.
The preceding native sources fail the same deterministic callback assertion
on both systems. The baseline stops there before returning to the invalidated
table.

`tools/test/check_workflow_value_reentry.py` exercises O0/O2 CPU source units,
shared ownership, the original Graph callback tests, schema/XSON regressions,
widget compilation and C11/C++11/C++17 headers. The matching xlang gate
`tools/check_xui_workflow_value_sdk.py` then links C clients to actual fresh
hosted/static SDKs, with one compiler-owned XRT and no production-unit
replacement. Source tests and actual SDK clients are separately reported.

The four private production files change; the public function/type ABI and
model layouts do not. Counter entry/exit and the synchronous helper calls have
CPU cost, so this is not a zero-CPU-overhead claim. Public language model
promotion, the approved default data-snapshot policy and final matched runtime
delivery remain separate work. Large logs and products stay outside Git; the
small JSON receipt indexes their exact hashes.
