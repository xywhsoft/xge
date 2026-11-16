# FlowGraph shared native ownership — 2026-11-16

`xuiFlowGraphCreateOwned` creates a creator-thread-affine, reference-counted
model. `ShareOwned` acquires one actual independent reference to the same model;
it changes its output only on success. `ReleaseOwnerChecked` reports consumption
separately from cleanup success. A completed Drop callback may report failure
after consuming the last owner, so that owner must not be retried. The legacy
unique creator remains available and is not silently promoted.

The ownership origin has exact Count/Trace, adapter, preparation, quiescence,
resumable finalization and checked completion descriptors. It traces the actual
configuration/history references once. A canvas or Workflow owning a promoted
graph traces its one graph reference, rather than flattening the graph's values
again. The native owner's address survives semantic finalization and Finish
until the last real view or collector reference is dropped.

All 62 ordinary public integer operations, plus internal configuration, revision
and data-swap operations, protect managed activity. Configuration Clone/Drop
still rejects same-model ordinary reentry with Busy, preserving outputs; peer
models remain usable. The immutable owner tail is excluded from data swaps and
content clearing. Another thread is rejected before reading mutable model data,
including while the creator is actively swapping or modifying it.

There is one additional allocation for shared-owner/refcount metadata and one
owner pointer in the graph. Managed operations perform native activity/reference
and ownership-domain bookkeeping; they are not claimed to have the cost of the
legacy raw model. Nested native operations reuse the outer activity. The warmed
1000-count-call test observes zero XRT allocation/reallocation, not a CPU timing
or all-allocator zero-overhead result. There is no per-method forwarding layer,
configuration copy on Share, or array copy on model sharing.

Both platforms passed the source-unit gate at O0 and O2, with the sole matching
latest-XRT compiler runtime. Each report contains 33 successful build/run/header
steps: new ownership tests, existing configuration Busy/model/schema/XSON tests,
owned and raw DataObject regressions, canvas compilation, and C11/C++11/C++17
public headers. The ownership test covers create OOM prefixes, first-error
preservation, consumed cleanup failures, history edges, aliases after swaps,
collector claims/finalization/Finish with outstanding references, and concurrent
wrong-thread rejection during 1000 creator mutations/swaps. Memory-debug live
counts and bytes balance; invalid/double frees and use-after-free counters are
zero. The DataObject lifetime-template opt-ins retain their default behavior.

Evidence (external payloads, not repository artifacts):

- `F:/xlang-validation/xge-full-20261110/flow-shared-native-win-r5/results.json`
- `F:/xlang-validation/xge-full-20261110/flow-shared-native-linux-r1/results.json`

Run `tools/test/check_flow_graph_reentry.py --help` for the exact frozen-baseline,
compiler/runtime/config and new-output-directory arguments. Earlier r1/r2 gate
failures were test-host setup issues (a full-engine helper in a direct-XRT CPU
fixture); their reports and logs remain preserved.

This is a native CPU foundation, not a rebuilt SDK/host, complete Workflow
sharing, shared canvas handoff, safe xlang surface, or final IDE/runtime delivery.
No graphics device, desktop or network test was run in this gate.
