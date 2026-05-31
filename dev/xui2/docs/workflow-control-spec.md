# XUI Workflow / FlowGraph Control SPEC

This SPEC tracks the implementation of a complex workflow and node-graph
system for XUI2.

Related design document:

- `dev/xui2/docs/workflow-control-design.md`

Status markers:

- `[ ]` Not started.
- `[~]` In progress.
- `[X]` Complete and verified.

Maintenance rules:

1. Before starting a task, change its status from `[ ]` to `[~]`.
2. If the task scope changes, update the task text before implementation.
3. Mark a task `[X]` only after code, tests, examples, and docs required by
   that task are complete.
4. If validation exposes a blocker, record it in the blocker section and keep
   the related task `[~]`.
5. Do not remove historical tasks when replacing a design. Add a note instead.

## Overall Acceptance

- [X] Applications can register custom workflow node types.
- [X] Registered node types can define static ports and rich config schemas.
- [X] Node instances can be created, moved, selected, connected, and deleted on
  a graph canvas.
- [X] Workflow data can be saved to and loaded from XSON.
- [X] Dynamic ports can be rebuilt from config or host callbacks.
- [X] Validation reports structural, graph, type, config, and host diagnostics.
- [X] Runtime state can be applied to nodes and edges for visual feedback.
- [X] A workflow editor example demonstrates Start -> LLM -> Condition -> End.
- [X] All first-slice tests and examples build on Windows.

## Phase 0: Design And Tracking

- [X] Add workflow/flowgraph design document.
- [X] Add workflow/flowgraph tracked SPEC.
- [X] Add docs index entries for the design and SPEC.
- [X] Inspect `dev/xui2/lib/xrt/xrt.h` and add reusable XRT foundation index.
- [X] Update workflow design to require XRT reuse for foundational services.
- [X] Add `docs/xui/widget-flowgraph.md`.
- [X] Add `docs/xui/widget-workflow.md`.

## Phase 1: Core Model

- [X] Use XRT memory APIs for all owned model allocations.
- [X] Use XRT containers for model lists and id maps unless a graph-specific
  structure is explicitly justified.
- [X] Define internal graph model structs for graph, node, port, edge,
  selection, viewport, and diagnostics.
- [X] Define workflow model structs for node type, node instance, config
  schema, variables, runtime state, and validation result.
- [X] Add stable id generation helpers for nodes, edges, ports, and variables.
- [X] Add model ownership and cleanup tests.
- [X] Add duplicate id detection tests.
- [X] Add node type registration and lookup.
- [X] Add node instance creation from registered node type.
- [X] Add static port resolution from node type declarations.
- [X] Add graph mutation helpers for add/remove node and add/remove edge.
- [X] Add selection model for nodes and edges.

Acceptance:

- [X] `test_xui/xui_flow_graph_test.c` can build and pass core model tests.
- [X] `test_xui/xui_workflow_schema_test.c` can build and pass registry tests.

## Phase 2: Graph Canvas Widget

- [X] Add `src/xui_flow_graph.c`.
- [X] Add `src/xui_flow_graph_widget.c`.
- [X] Register a `flowgraph` widget type.
- [X] Implement viewport pan and zoom state.
- [X] Implement world-to-screen and screen-to-world transforms.
- [X] Implement grid drawing.
- [X] Implement node card drawing.
- [X] Implement port drawing.
- [X] Implement edge drawing with bezier or polyline routing.
- [X] Implement selected, hover, invalid, and disabled visual states.
- [X] Implement graph dirty flags and cache invalidation boundaries.
- [X] Implement visible-node clipping.
- [X] Implement basic edge bounds clipping.

Acceptance:

- [X] `test_xui/xui_flow_graph_widget_test.c` can build and pass static render
  tests.
- [X] `examples/xui_flowgraph` displays a static graph.
- [X] The example supports `--frames N` and `--seconds N`.
- [X] Graph render does not require new proxy capabilities.

## Phase 3: Hit Testing And Editing

- [X] Implement node hit testing.
- [X] Implement port hit testing.
- [X] Implement edge hit testing.
- [X] Implement background hit testing.
- [X] Implement single selection.
- [X] Implement Ctrl/Shift selection toggle.
- [X] Implement marquee selection.
- [X] Implement node drag.
- [X] Implement multi-node drag.
- [X] Implement port-to-port connection drag.
- [X] Implement connection validity preview.
- [X] Implement delete selected nodes and edges.
- [X] Implement cancel drag with Escape.

Acceptance:

- [X] `test_xui/xui_workflow_interaction_test.c` covers hit testing, Ctrl/Shift selection toggle, marquee selection, node drag, multi-node drag, port-to-port connection drag, connection validity preview, delete selected nodes/edges, and Escape drag cancellation
  state transitions.
- [X] `examples/xui_flowgraph` can add, move, connect, select, and delete
  nodes.

## Phase 4: Commands And Undo Foundation

- [X] Define graph command type.
- [X] Add apply/undo data for add node.
- [X] Add apply/undo data for remove node.
- [X] Add apply/undo data for move node.
- [X] Add apply/undo data for add edge.
- [X] Add apply/undo data for remove edge.
- [X] Add apply/undo data for set node config.
- [X] Add command coalescing for continuous node drag.
- [X] Add command history capacity limits.
- [X] Add dirty revision tracking.

Acceptance:

- [X] Unit tests prove basic undo/redo works for node and edge edits.
- [X] Continuous dragging creates one undo step.

## Phase 5: Config Schema

- [X] Store node config and schema data on top of XRT `xvalue`.
- [X] Add config schema model.
- [X] Add primitive fields: string, textarea, int, float, bool.
- [X] Add select and multi-select field metadata.
- [X] Add expression field metadata.
- [X] Add variable reference field metadata.
- [X] Add node output reference field metadata.
- [X] Add object and array field metadata.
- [X] Add group and tabs field metadata.
- [X] Add condition builder field metadata.
- [X] Add mapping builder field metadata.
- [X] Add config default value materialization.
- [X] Add config field path diagnostics.
- [X] Add config schema validation tests.

Acceptance:

- [X] A registered node type can create a default node config.
- [X] Required fields and numeric ranges are validated.
- [X] Expression references are stored without evaluating them in XUI.

## Phase 6: Workflow Control Layer

- [X] Add `src/xui_workflow.c`.
- [X] Register a `workflow` widget type.
- [X] Embed or own a FlowGraph model.
- [X] Add node type registry API.
- [X] Add workflow add-node API by node type id.
- [X] Add workflow connect API.
- [X] Add workflow remove-node API with connected edge cleanup.
- [X] Add workflow node config get/set API.
- [X] Rebuild node visual summary after config change.
- [X] Expose selected node to host/editor panels.
- [X] Add node library data extraction from registry.

Acceptance:

- [X] Host code can register Start, End, Condition, and LLM mock node types.
- [X] Host code can create a workflow through API without direct struct access.

## Phase 7: Dynamic Ports

- [X] Add dynamic port callback type.
- [X] Add node type dynamic port callback registration.
- [X] Add static + dynamic port merge.
- [X] Add dynamic port rebuild after config changes.
- [X] Preserve edges when dynamic port ids remain stable.
- [X] Mark edges invalid when dynamic port ids disappear.
- [X] Add Condition node dynamic output ports from `config.branches`.
- [X] Add tests for dynamic port add/remove/rename behavior.

Acceptance:

- [X] Editing Condition branches updates visible output ports.
- [X] Existing valid branch edges survive branch title changes.
- [X] Missing branch ids produce validation diagnostics instead of crashes.

## Phase 8: XSON Persistence

- [X] Implement workflow persistence through XRT XSON APIs, not a custom
  parser/printer.
- [X] Define workflow XSON writer.
- [X] Define workflow XSON reader.
- [X] Save workflow metadata.
- [X] Save required node type references.
- [X] Save variables.
- [X] Save nodes, positions, config, and visual state.
- [X] Save edges and route hints.
- [X] Save viewport state.
- [X] Load and validate missing node type references.
- [X] Load and preserve invalid edges for repair.
- [X] Add round-trip tests.

Acceptance:

- [X] Start -> LLM -> Condition -> End workflow round-trips through XSON.
- [X] Loaded graph keeps node positions and viewport.
- [X] Invalid references are reported as diagnostics with paths.

## Phase 9: Validation

- [X] Add structural validation.
- [X] Add missing node type validation.
- [X] Add missing node/port validation.
- [X] Add edge direction validation.
- [X] Add control-flow DAG validation.
- [X] Add start/end node validation.
- [X] Add unreachable node validation.
- [X] Add required config field validation.
- [X] Add expression reference validation.
- [X] Add variable reference validation.
- [X] Add host validation callback.
- [X] Add validation diagnostic rendering on nodes and edges.

Acceptance:

- [X] The validator catches missing Start or End.
- [X] The validator catches invalid edges after dynamic port removal.
- [X] Node cards show validation errors and warnings.

## Phase 10: Runtime State Visualization

- [X] Define workflow runtime state model.
- [X] Add node run states: idle, queued, running, success, failed, skipped,
  warning.
- [X] Add edge run states: idle, active, taken, skipped, invalid.
- [X] Add API to update node run state.
- [X] Add API to update edge run state.
- [X] Add compact result preview metadata.
- [X] Add diagnostic/result badges on nodes.
- [X] Add mock run playback in workflow editor example.

Acceptance:

- [X] Example can animate mock execution across the workflow.
- [X] Failed node state remains visible after playback completes.

## Phase 11: Composite Workflow Editor Example

- [X] Add `examples/xui_workflow_editor`.
- [X] Add node library panel.
- [X] Add central workflow canvas.
- [X] Add property/config panel.
- [X] Add toolbar actions.
- [X] Add bottom diagnostics/log panel.
- [X] Add context menus for canvas, node, edge, and port.
- [X] Add Start, End, Condition, LLM mock, HTTP mock, and Variable mock node
  types.
- [X] Add sample workflow creation.
- [X] Add save/load demonstration.
- [X] Add mock run-state demonstration.

Acceptance:

- [X] The example supports `--frames N` and `--seconds N`.
- [X] The example does not auto-exit without duration arguments.
- [X] The example demonstrates custom node registration and XSON workflow
  persistence.

## Phase 12: Performance And Scale

- [X] Add visible node culling.
- [X] Add edge route bounds culling.
- [X] Add node spatial buckets.
- [X] Add edge spatial buckets.
- [X] Add grid cache by zoom bucket.
- [X] Add node card state cache.
- [X] Add edge layer cache.
- [X] Add benchmark or smoke graph with 500 nodes.
- [X] Add benchmark or smoke graph with 2000 edges.

Acceptance:

- [X] Large graph smoke remains interactive enough for editor use.
- [X] Large graph render avoids creating one widget per edge.

## Phase 13: Documentation

- [X] Document FlowGraph widget API.
- [X] Document Workflow widget API.
- [X] Document node type registration.
- [X] Document config schema field types.
- [X] Document dynamic port rules.
- [X] Document workflow XSON format.
- [X] Document validation diagnostic codes.
- [X] Document host integration boundaries.

Acceptance:

- [X] A host application can implement a custom node type by following docs
  without reading example internals.

## Blockers And Decisions

- [X] Decide whether the first XSON implementation uses existing XUI2 parser
  helpers or a workflow-local adapter.
- [X] Decide the first public API surface after internal tests stabilize.
- [X] Decide whether FlowGraph nodes are internal render items only or promoted
  to child widgets after performance validation.

## Verification Commands

Expected future commands:

```bat
test_xui\build_flow_graph_test.bat
test_xui\build_flow_graph_widget_test.bat
test_xui\build_workflow_schema_test.bat
test_xui\build_workflow_widget_test.bat
test_xui\build_workflow_validation_test.bat
test_xui\build_workflow_xson_test.bat
test_xui\build_workflow_interaction_test.bat
examples\xui_flowgraph\build.bat
build\xui_flowgraph.exe --frames 2
build\xui_flowgraph.exe --frames 2 --scripted-edit
examples\xui_workflow_editor\build.bat
```

Before handoff, run:

```bat
git diff --check
```
