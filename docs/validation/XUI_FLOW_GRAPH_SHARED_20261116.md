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

This foundation does not qualify a rebuilt SDK/host, complete Workflow sharing,
safe xlang surface, or final IDE/runtime delivery. No graphics device, desktop
or network test was run in this gate.

## Canvas sharing follow-up

`xuiFlowGraphWidgetGetGraphOwned` returns an independent native reference and
promotes a uniquely owned native slot once. An unowned legacy borrowed slot is
rejected; an already shared borrowed model can supply a new reference without
making that slot an owner. The output is unchanged on failure. Promotion and
output delivery use the existing Context call scope, and a failed output tail
releases its temporary reference.

`xuiFlowGraphWidgetSetGraphShared` acquires the canvas's own reference and never
consumes the caller's. Repeating the same owning assignment does not add another
reference. The internal handoff records installation before callbacks or scope
retirement can release it; a failed uninstalled prefix drops only its temporary
hold. Replacement preserves an old-model cleanup error independently from whether
that model reference was consumed. Cleanup failure can therefore accompany an
installed new model, as documented on the native declaration.

The CPU tests link the exact latest-XRT static archive with three replacement
production translation units and the frozen test proxy. They do not rebuild or
claim a new SDK. Both Windows and Linux pass eight O0/O2 build/run steps covering
new sharing behavior and the original full canvas interaction fixture:

- `F:/xlang-validation/xge-full-20261110/flow-shared-canvas-win-r2/results.json`
- `F:/xlang-validation/xge-full-20261110/flow-shared-canvas-linux-r1/results.json`

The new cases cover promotion OOM, unchanged outputs, one real model edge per
owning canvas, borrowed slots, multiple aliases/canvases, independent close,
ambient diagnostic preservation, consumed Drop failure and a Drop callback that
logically closes the canvas. Its physical retirement tail consumes the installed
reference exactly once while the caller's model remains alive. Live bytes/counts
balance and invalid/double free/use-after-free counters remain zero. Earlier
canvas r1 asserted physical Busy for a logical close, contrary to XUI's existing
deferred-retirement contract; that failed fixture and log are preserved.
