# XUI Incremental Rendering Plan and Implementation

## Objective

Opening or closing a small overlay must cost approximately the size of the overlay and its
ancestor path, not the size of the application widget tree. The work covers message boxes,
file dialogs, find/replace windows, menus, and other retained overlays.

Dialog reuse and the incremental rendering pipeline are both implemented. `xuiMsgBoxShow` and
`xuiFileDialogShow` retain a closed instance and update it before reopening. Render preparation,
dirty/event aggregation, update scheduling, active-window lookup, and render-tree maintenance
now scale with the changed overlay and its ancestor paths instead of the application tree.

## Implementation Status (2026-11-08)

All five phases are complete:

- `xui_render_stats_t` exposes prepare visits/skips, propagation steps, stable-node operations,
  and independent root/Overlay structure revisions.
- Event-mask additions use bitwise ancestor propagation. Removal and detach recompute exact
  aggregates from the widget and immediate children only.
- Dirty flags maintain exact subtree aggregates incrementally. Clearing performs immediate
  bottom-up repair; this is simpler than the originally proposed deferred queue and already
  coalesces work by stopping at the first unchanged ancestor.
- RenderPrepare skips clean subtrees conservatively. Widget types can opt into
  `XUI_WIDGET_TYPE_PREPARE_FRAME_DEPENDENT` when a prepare callback must run every frame.
- Each Widget owns a stable render record. Paint order is a linked sequence of spliceable
  subtree ranges, while `xuiGetRenderNode()` materializes the indexed compatibility view lazily.
- Root and Overlay ranges have independent structure revisions. Overlay-only open/close keeps
  the main-root records intact.
- Update callbacks and inactive updates have exact subtree-presence aggregates, and active
  Window lookup is indexed, removing two additional full-tree scans from dialog opening.
- `XUI_ENABLE_INCREMENTAL_VERIFY=1` enables legacy-oracle checks for dirty flags, event masks,
  update aggregates, active Window selection, stable render order, geometry, and subtree ranges.

The implementation also removes stable render ranges immediately on detach. This is required
for Overlay controls that are detached and destroyed before another RenderTree pass; deferring
the removal would leave an old ancestor range endpoint pointing at retired storage.

### Benchmark Result

`tests/xui/xui_dialog_large_tree_perf_test.c` uses actual shallow-wide and balanced Widget
trees. Values below are warm medians in microseconds from the 2026-11-08 Windows baseline:

| Shape | Widgets | MsgBox show | RenderPrepare | RenderTree |
| --- | ---: | ---: | ---: | ---: |
| wide | 1,000 | 31.2 | 25.0 | 0.9 |
| balanced | 1,000 | 34.8 | 31.2 | 1.4 |
| wide | 10,000 | 18.1 | 15.8 | 0.6 |
| balanced | 10,000 | 19.2 | 17.0 | 0.6 |
| wide | 50,000 | 29.6 | 21.3 | 1.0 |
| balanced | 50,000 | 56.6 | 32.4 | 2.0 |

At 50,000 Widgets the warm frame visits 7 prepare nodes, skips the clean main-root subtree,
reuses all 50,000 main-root render records, updates 6 overlay records, and inserts 7 overlay
records. Cold construction and the first full frame intentionally remain O(N).

Run the benchmark with:

```bat
tests\xui\build_dialog_large_tree_perf_test.bat
```

Run the incremental correctness suite with:

```bat
tests\xui\build_incremental_render_test.bat
```

## Original Cost Centers

1. Render preparation recursively visited the visible root and overlay trees, then recursively
   recomputes subtree dirty flags.
2. The render tree was flattened from scratch after each render request.
3. Event-mask changes recomputed subtree masks recursively for every ancestor.
4. Attaching an overlay correctly marked structure and rendering dirty, but the invalidation
   was consumed as a global rebuild.

The optimization must preserve layout, clipping, stacking order, modal hit testing, focus,
accessibility, cache invalidation, and callback reentrancy semantics.

## Phase 0: Measurement and Correctness Oracle

Add `tests/xui/xui_dialog_large_tree_perf_test.c` with actual Widget trees of 1,000, 10,000,
and 50,000 nodes. TreeView data-only nodes are not a substitute for Widgets.

For each size, measure separately:

- cold `xuiMsgBoxCreate` plus first open;
- warm `xuiMsgBoxShow` on a retained, closed instance;
- the first `xuiUpdate` after opening;
- RenderPrepare time and visited/skipped widget counts;
- render-tree update time and reused/rebuilt node counts;
- close plus the first frame after close;
- allocations and peak temporary bytes when allocator statistics are available.

Use a shallow-wide tree and a balanced tree. Run warm-up iterations, then report median, P95,
and maximum. The benchmark should print machine-readable rows for CI artifacts and should not
use a pass/fail wall-clock threshold until stable baselines exist.

Add debug counters to `xui_render_stats_t`:

- prepare visited and skipped subtrees;
- dirty aggregate propagation steps;
- render nodes reused, updated, inserted, and removed;
- event-mask propagation steps;
- root and overlay structural revisions.

During migration, keep a debug verification mode that recomputes the legacy subtree dirty
flags, event masks, and flattened render order and compares them with the incremental result.

## Phase 1: Incremental Event Masks

Keep `iSubtreeEventMask` authoritative at all times.

- AddChild: OR the attached child's current subtree mask into each ancestor and stop when an
  ancestor does not change.
- Adding an event handler or event interest: propagate only newly added bits upward.
- RemoveChild or clearing bits: recompute only the affected bits from the widget's own mask and
  its immediate children's already-valid subtree masks, then propagate only if the aggregate
  changed.
- A detached subtree keeps its own valid aggregate and can be attached without recursive work.

This avoids a 64-counter array on every Widget and changes the common path from recursive
subtree scans to ancestor propagation. If profiling later shows very high sibling counts,
introduce an optional sidecar bit-count table only for high-fanout parents.

Required tests cover add, detach, reparent, handler replacement, clearing the last provider of
an event bit, and callback-driven tree mutation.

## Phase 2: Incremental Dirty Aggregates

`iDirtyFlags` remains local and `iSubtreeDirtyFlags` remains the exact aggregate.

- Marking dirty ORs only newly introduced bits up the ancestor chain and stops at the first
  unchanged ancestor.
- Clearing local dirty bits queues the widget for aggregate repair.
- Aggregate repair runs bottom-up. Each queued node combines its local flags with the already
  valid aggregates of immediate children; only a changed result is propagated to its parent.
- Batch mutations coalesce duplicate queue entries by generation so one ancestor is repaired at
  most once per drain.
- Attach, detach, visibility, layer, and style inheritance changes mark explicit structural or
  inherited-state bits rather than relying on a later global scan.

Delete the unconditional recursive dirty recomputation only after the debug oracle has passed
all widget, overlay, reparenting, and destruction tests.

## Phase 3: Skip Clean RenderPrepare Subtrees

RenderPrepare receives a required-mask and inherited-state revision. A subtree can be skipped
when all of the following are true:

- its aggregate has none of the required prepare bits;
- its structure revision is unchanged;
- inherited style, theme, DPI, viewport, proxy, clip, opacity, and transform revisions match;
- it has no prepare callback explicitly registered as frame-dependent;
- no active cache job or deferred resource completion targets the subtree.

Store the last prepared revisions on the subtree root. Do not infer frame dependence merely
from `onUpdate`; update scheduling and render preparation remain separate contracts.

The first implementation should skip only conservatively proven clean subtrees. Broader skips
can be enabled after counters show which reasons still force visits.

## Phase 4: Stable Incremental Render Tree

Do not mutate one global flat array in place for every subtree insertion. Give each Widget a
stable render-node record and maintain paint order as spliceable subtree ranges:

- each render node stores stable identity, parent identity, geometry, clip, layer, z-order, and
  a node revision;
- each subtree stores first/last paint-order links and a structure revision;
- geometry or paint dirtiness updates existing records;
- AddChild, RemoveChild, reparent, visibility, or stacking changes rebuild only the nearest
  structurally dirty subtree and splice its range;
- root and Overlay use independent top-level ranges, so opening a dialog patches only Overlay;
- the composer iterates the stable paint-order links or segmented ranges directly;
- `xuiGetRenderNode(index)` uses a lazily refreshed index table so the public inspection API
  does not force every frame back to a flat rebuild.

Keep the current full builder behind a runtime/debug fallback until stable-tree output matches
the legacy builder for node order, clips, layers, and damage rectangles.

## Phase 5: Integration and Performance Gates

Land phases independently in this order: instrumentation, event masks, dirty aggregates,
RenderPrepare skipping, then stable RenderTree. Each phase must pass the full XUI test suite
with the legacy verification mode enabled before the next phase starts.

After stabilization, CI gates should assert structural work rather than fragile absolute time:

- warm MsgBox open at 50,000 Widgets visits only the overlay, changed ancestors, and bounded
  housekeeping nodes;
- warm open reuses the MsgBox widget identities and existing render records;
- root render-node rebuild count is zero for overlay-only open/close;
- 50,000-Widget warm-open P95 stays within a small factor of the 1,000-Widget result;
- no event-mask or dirty-aggregate oracle mismatch occurs under randomized add/remove/reparent
  operations.

Absolute latency budgets can be added after collecting release and debug baselines on CI and a
representative Windows desktop.

## Non-goals

- Virtualizing application data models is separate from Widget-tree rendering work.
- Reusing dialogs does not make one retained instance safe for simultaneous nested requests;
  the reusable show APIs return `XUI_ERROR_BUSY` while an instance is open.
- File-dialog mode and font are construction properties for reuse. Applications that need
  Open, Save, and Select Folder concurrently should retain one instance per mode.
