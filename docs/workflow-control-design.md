# XUI Workflow / FlowGraph Control Design

This document defines the first implementation design for a complex workflow
and node-graph system in XUI2. The target is a schema-driven workflow editor:
applications can define custom node types, users arrange node instances on a
canvas, and the complete workflow can be saved as XSON.

The public API prefix remains `xui`. The directory name `xui2` is only the
development workspace name.

## Goals

- Provide a reusable node graph editor for workflows, visual scripting,
  behavior trees, material graphs, automation pipelines, and AI agent flows.
- Let applications define rich custom node types without rewriting the canvas.
- Persist the full workflow as XSON, not only the visual layout.
- Separate control flow from data binding so large workflows remain readable.
- Support Coze-like workflow authoring: node library, canvas, connection
  editing, node configuration, validation, and run-state visualization.
- Fit XUI2's cache-first DirectUI architecture and avoid per-edge widget
  explosion.
- Keep execution outside the UI control. XUI owns editing, validation, XSON
  import/export, and run-state display. The host owns real execution.

## Non-Goals

- Do not implement a full workflow execution engine in XUI2 V1.
- Do not bind the model to one external product's backend schema.
- Do not make every data dependency a visible edge by default.
- Do not add platform or proxy capabilities unless a later implementation
  proves the current proxy contract cannot render or input the editor.
- Do not introduce `xui2` public symbols.

## Design Summary

The system is split into two layers:

1. `FlowGraph`: a generic node graph canvas and model.
2. `Workflow`: a workflow-specific layer built on FlowGraph with custom node
   definitions, config schema, validation rules, XSON persistence, and
   run-state display.

This split keeps the graph editor reusable. Workflow is a preset and semantic
layer, not the only use case.

## XRT Foundation Reuse

Workflow implementation must use XRT for foundational runtime services before
adding new local primitives. The detailed index is maintained in:

- `dev/xui2/docs/xrt-workflow-foundation-index.md`

Required reuse decisions:

- Use `xrtMalloc`/`xrtCalloc`/`xrtRealloc`/`xrtFree` for owned allocations.
- Use `xrtArray` for ordered graph/model lists where practical.
- Use `xrtDict` for id lookup maps.
- Use `xvalue` for node config, config schema data, workflow variables,
  runtime result preview data, and XSON tree data.
- Use XRT XSON APIs for workflow persistence:
  `xrtParseXSON`, `xrtParseXSONEx`, `xrtParseXSON_File`,
  `xrtStringifyXSON`, and `xrtStringifyXSON_File`.
- Use `xrtFile*` and `xrtPath*` helpers for file loading/saving and paths.
- Use `xrtTimer`/`xtime` helpers for profiling, run durations, and persisted
  timestamps.
- Use `xlog*`/`xlogger*` for development logs.
- Use XRT random/XID helpers for generated ids when deterministic ids are not
  required by tests.

Workflow-local code may still define graph-specific structures such as nodes,
ports, edges, commands, diagnostics, and spatial buckets. It should not define
generic replacements for dynamic arrays, dictionaries, typed values, XSON,
file helpers, or logging.

## Main Controls

### xuiFlowGraph

`xuiFlowGraph` is the canvas widget.

Responsibilities:

- Infinite 2D canvas with pan and zoom.
- Grid, optional snap, view transform, minimap hooks.
- Node placement, node selection, multi-selection, marquee selection.
- Edge rendering and edge hit testing.
- Port hit testing and connection preview.
- Keyboard command routing.
- Dirty/cache invalidation for graph layers.
- Optional spatial index for large graphs.

`xuiFlowGraph` does not understand LLM nodes, HTTP nodes, prompts, or workflow
execution. It only understands nodes, ports, edges, groups, and graph-level
editing operations.

### xuiFlowNode

`xuiFlowNode` is the visual node card model used by the graph.

The implementation may represent nodes as internal graph items rather than
normal child widgets in V1. If later XUI2 widget composition makes nested node
widgets cheap enough, selected node chrome and node content can be promoted
into widgets. The initial design should avoid thousands of node children in
the normal widget tree.

Responsibilities:

- Card bounds, title, subtitle, icon, category color.
- Input and output port layout.
- Compact summary content.
- Node state decoration: selected, hover, disabled, invalid, running, success,
  failed, warning, skipped.
- Collapse/expand state.
- Optional badges: breakpoints, comments, dirty config, validation count.

### xuiFlowPort

`xuiFlowPort` is a model entity owned by a node. It is not a separate widget in
V1.

Port responsibilities:

- Direction: input or output.
- Kind: control or data.
- Data type: string, number, bool, object, array, image, file, custom, any.
- Multiplicity: single edge, multiple edges, or no visible edge.
- Required/optional flag.
- Dynamic/static origin.
- Display title and compact name.
- Hit rect calculated during node layout.

### xuiFlowEdge

`xuiFlowEdge` is a graph-owned connection model and render item.

Responsibilities:

- Connect output port to input port.
- Render as bezier, orthogonal, or straight style.
- Show invalid, selected, hover, running, success, skipped, and failed states.
- Carry kind: control or data.
- Carry route hints and optional anchors.
- Support hit testing without becoming a widget.

### xuiWorkflow

`xuiWorkflow` is the workflow editor widget built on `xuiFlowGraph`.

Responsibilities:

- Node type registry.
- Node instance creation.
- Config schema binding.
- Right-side property editing integration.
- Node library integration.
- Workflow validation.
- XSON save/load.
- Runtime state update and visualization.
- Host callback hooks for preview, validation, and execution integration.

### xuiWorkflowEditor

`xuiWorkflowEditor` is an optional composite example/control, not the only API.

It composes:

- Left node library tree/list.
- Center workflow graph.
- Right config/property panel.
- Top toolbar.
- Bottom log/result panel.

It should reuse existing XUI2 controls such as `treeview`, `listview`,
`propertygrid`, `toolbar`, `statusbar`, `dockpanel`, `popup`, and `menu`.

## Data Model

### Type Registry

Applications register node types before or during workflow loading.

```c
typedef struct xui_workflow_node_type_t xui_workflow_node_type_t;
typedef struct xui_workflow_node_schema_t xui_workflow_node_schema_t;
typedef struct xui_workflow_config_schema_t xui_workflow_config_schema_t;
```

A node type describes what a node can do. A node instance describes one node on
the canvas.

Node type fields:

- `id`: stable string, for example `llm.chat`.
- `version`: integer schema version.
- `title`: display name.
- `category`: node library group.
- `description`: optional help text.
- `icon`: icon key or asset key.
- `accent`: optional theme color.
- `inputs`: static port declarations.
- `outputs`: static port declarations.
- `configSchema`: rich form schema.
- `flags`: start node, end node, branch node, side-effect node, deprecated.
- `dynamicPortPolicy`: optional callback or schema-driven rules.
- `validateProc`: optional host validation callback.

### Node Instance

Node instance fields:

- `id`: workflow-local stable id.
- `type`: node type id.
- `typeVersion`: node type version used when saved.
- `title`: instance title override.
- `position`: canvas x/y.
- `size`: optional saved size.
- `collapsed`: visual state.
- `disabled`: execution/editor flag.
- `config`: XSON value object.
- `ports`: resolved static + dynamic port snapshot.
- `ui`: editor-only visual metadata.
- `runtime`: optional runtime state snapshot.

### Ports

Port fields:

- `id`: stable port key within node.
- `title`: display title.
- `direction`: `input` or `output`.
- `kind`: `control` or `data`.
- `dataType`: simple or custom type id.
- `required`: required for validation.
- `multi`: allows multiple incoming/outgoing edges.
- `visible`: show in node chrome.
- `dynamic`: generated from config or host callback.
- `group`: optional visual grouping.
- `order`: stable sort order.

### Edges

Edge fields:

- `id`: stable edge id.
- `kind`: `control` or `data`.
- `from.node`: source node id.
- `from.port`: source port id.
- `to.node`: target node id.
- `to.port`: target port id.
- `enabled`: disabled edges stay visible but do not execute.
- `label`: optional display label.
- `route`: optional route hints.
- `ui`: selected route style and debug flags.

### Variables

Workflow variables are first-class because complex orchestration needs state.

Variable fields:

- `id`
- `title`
- `type`
- `scope`: `workflow`, `node`, `loop`, `temporary`
- `defaultValue`
- `description`

### Runtime State

Runtime state is separate from workflow definition. A workflow file may include
last-run state for editor convenience, but this should be optional.

State levels:

- workflow state: idle, running, paused, success, failed, canceled.
- node state: idle, queued, running, success, failed, skipped, warning.
- edge state: idle, active, taken, skipped, invalid.
- diagnostics: validation error, runtime error, warning, info.
- result preview: compact JSON/XSON/text payload metadata.

## XSON Persistence

The workflow XSON must save enough information to reconstruct the editor and
pass the model to a host engine.

The initial file shape:

```xson
workflow {
  version: 1
  id: "wf_customer_service"
  title: "Customer service workflow"
  description: "Intent recognition and response routing"

  registry {
    required_node_types: [
      { id: "start", version: 1 }
      { id: "llm.chat", version: 1 }
      { id: "condition", version: 1 }
      { id: "end", version: 1 }
    ]
  }

  variables: [
    { id: "user_input", type: "string", scope: "workflow" }
    { id: "intent", type: "string", scope: "workflow" }
  ]

  nodes: [
    {
      id: "n_start"
      type: "start"
      type_version: 1
      title: "Start"
      position: { x: 80, y: 120 }
      config: {
        outputs: [
          { id: "user_input", type: "string" }
        ]
      }
    }

    {
      id: "n_llm"
      type: "llm.chat"
      type_version: 1
      title: "Intent classifier"
      position: { x: 360, y: 120 }
      config: {
        model: "glm"
        temperature: 0.2
        system_prompt: "Classify user intent."
        prompt: "${n_start.user_input}"
      }
    }
  ]

  edges: [
    {
      id: "e_start_llm"
      kind: "control"
      from: { node: "n_start", port: "out" }
      to: { node: "n_llm", port: "in" }
    }
  ]

  view {
    zoom: 1.0
    pan_x: 0
    pan_y: 0
    selected: ["n_llm"]
  }
}
```

Node type definitions may be saved separately:

```xson
workflow_node_type {
  id: "llm.chat"
  version: 1
  title: "LLM Chat"
  category: "AI"
  icon: "bot"

  inputs: [
    { id: "in", title: "In", kind: "control", direction: "input" }
    { id: "prompt", title: "Prompt", kind: "data", direction: "input", data_type: "string", required: true }
  ]

  outputs: [
    { id: "out", title: "Done", kind: "control", direction: "output" }
    { id: "text", title: "Text", kind: "data", direction: "output", data_type: "string" }
    { id: "error", title: "Error", kind: "control", direction: "output", optional: true }
  ]

  config_schema: {
    fields: [
      { id: "model", title: "Model", type: "select", options: ["glm", "qwen", "openai"], default: "glm" }
      { id: "temperature", title: "Temperature", type: "float", min: 0, max: 2, default: 0.7 }
      { id: "system_prompt", title: "System prompt", type: "textarea" }
      { id: "prompt", title: "Prompt", type: "expression", accepts: ["string"] }
    ]
  }
}
```

### XSON Strategy

Workflow XSON must be implemented through XRT's XSON and `xvalue` layer. The
workflow layer should provide typed adapters around `xvalue`, but it must not
write a separate parser/printer for the workflow format.

Implementation order:

1. Build in-memory model and editor operations with XRT memory/container
   ownership.
2. Store node config and schema values as `xvalue` tables/arrays/scalars.
3. Implement workflow import/export through `xrtParseXSON*` and
   `xrtStringifyXSON*`.
4. Convert parsed `xvalue` trees into typed workflow model data before repeated
   validation or rendering.

The in-memory model must not depend on string-only parsing tricks. All parsed
values should become typed model values before validation or rendering.

## Config Schema

The config schema drives the node property panel and validation.

Minimum field properties:

- `id`
- `title`
- `type`
- `default`
- `required`
- `description`
- `visible_if`
- `enabled_if`
- `validation`
- `group`
- `order`

Field types for V1:

- `string`
- `textarea`
- `int`
- `float`
- `bool`
- `select`
- `multi_select`
- `enum`
- `color`
- `date`
- `time`
- `json`
- `xson`
- `expression`
- `variable_ref`
- `node_output_ref`
- `file`
- `image`
- `object`
- `array`
- `group`
- `tabs`
- `condition_builder`
- `mapping_builder`

Implementation can start with a smaller rendering subset, but the schema model
must reserve these field kinds so node definitions do not need a format break.

### Expression Values

Expression fields support references such as:

```text
${n_start.user_input}
${vars.intent}
${n_llm.text}
${vars.count + 1}
```

V1 does not need a complete expression engine in XUI. It needs:

- storage as typed expression text.
- reference extraction for dependency inspection.
- validation that referenced node ids, port ids, and variables exist.
- optional host callback for expression validation.

### Mapping Builder

Mapping fields describe how one object maps to another:

```xson
mapping: [
  { from: "${n_http.body.user.name}", to: "profile.name" }
  { from: "${n_http.body.user.id}", to: "profile.id" }
]
```

This is essential for HTTP, plugin, parser, variable, and data-transform
nodes.

### Condition Builder

Condition fields describe structured branch rules:

```xson
branches: [
  {
    id: "vip"
    title: "VIP user"
    when: { left: "${n_parse.level}", op: "eq", right: "vip" }
  }
  {
    id: "normal"
    title: "Normal user"
    when: { default: true }
  }
]
```

Condition and switch nodes use this config to create dynamic output ports.

## Dynamic Ports

Complex workflows require ports to change with config.

Examples:

- Condition branches create one control output per branch.
- Switch cases create one output per case.
- Plugin nodes generate inputs/outputs from tool metadata.
- Subflow nodes generate ports from the referenced sub-workflow signature.
- Mapping nodes generate output fields from mapping config.

V1 should support C callbacks:

```c
typedef int (*xuiWorkflowResolvePortsProc)(
    xui_workflow_t* workflow,
    const xui_workflow_node_t* node,
    xui_workflow_port_list_t* outPorts,
    void* userData);
```

Later, schema-only dynamic port rules can be added:

```xson
dynamic_ports {
  outputs_from: "config.branches"
  template: {
    direction: "output"
    kind: "control"
    id: "${item.id}"
    title: "${item.title}"
  }
}
```

Port rebuild rules:

- Rebuild ports after relevant config changes.
- Preserve existing edges when matching port ids still exist.
- Mark edges invalid when a referenced port disappeared.
- Provide a repair action for missing dynamic ports.
- Dirty node chrome, edge layer, and validation state after rebuild.

## Control Flow And Data Flow

Control flow and data flow must be separate.

Control flow:

- Usually represented by visible edges.
- Determines execution order.
- Must be DAG-validated unless a node type explicitly allows loop semantics.
- Has branch/taken/skipped runtime visualization.

Data flow:

- Usually represented by config references, not always visible edges.
- Can be inspected by selecting a field or enabling dependency overlay.
- Must validate references and types.
- May optionally be shown as faint temporary links.

This prevents large workflows from becoming unreadable when every data binding
is displayed as a permanent edge.

## Built-In Node Types

The first workflow example should register these built-in node types:

- `start`: workflow input definition.
- `end`: workflow output definition.
- `condition`: structured branching with dynamic outputs.
- `llm.chat`: mock LLM call config.
- `http.request`: HTTP/API config.
- `code`: mock code or expression node.
- `variable.set`: variable assignment.
- `variable.get`: variable read.
- `json.parse`: parse JSON into structured output.
- `subflow`: reference another workflow, initially disabled or mock-only.

Only `start`, `end`, `condition`, and one mock action node are required for the
first editor smoke. The rest provide the design target.

## Validation

Validation runs on the in-memory workflow model and reports diagnostics with
stable paths.

Validation categories:

- Structural validation:
  - duplicate ids.
  - missing node type.
  - missing node id.
  - missing port id.
  - invalid edge direction.
  - invalid control/data edge kind.
  - orphan dynamic port edges.

- Graph validation:
  - no start node.
  - no end node.
  - unreachable node.
  - cycle in control flow, unless allowed.
  - multiple incoming control edges where not allowed.
  - missing required control input.

- Type validation:
  - data edge type mismatch.
  - expression reference type mismatch.
  - variable type mismatch.
  - mapping target type mismatch.

- Config validation:
  - missing required field.
  - invalid select option.
  - number out of range.
  - invalid expression syntax.
  - invalid condition builder rule.
  - invalid dynamic port source.

- Host validation:
  - node-specific rules supplied by the application.
  - external resource availability warnings.

Diagnostics should include:

- severity: error, warning, info.
- code: stable string.
- message.
- node id, port id, edge id, or config path.
- optional quick-fix id.

## Editing Commands

Graph mutations should be represented as commands to support undo/redo.

Command types:

- add node.
- remove node.
- move node.
- resize node.
- add edge.
- remove edge.
- set node config.
- set node title.
- set node disabled.
- rebuild dynamic ports.
- paste subgraph.
- group nodes.
- ungroup nodes.
- align/distribute nodes.

Each command needs:

- apply.
- undo.
- merge policy for continuous drag/edit.
- dirty flags.

V1 can implement command recording before a full public undo stack. The data
shape should not block undo/redo later.

## Interaction Model

Mouse:

- Left click node: select.
- Shift/Ctrl click: toggle selection.
- Drag node: move selected nodes.
- Drag from output port to input port: create edge.
- Drag from input port to output port: create edge if allowed.
- Drag on background: pan if space/middle button is active.
- Drag marquee: select nodes.
- Right click: context menu.
- Wheel: zoom around cursor, or scroll when a nested panel has focus.

Keyboard:

- Delete: delete selection.
- Ctrl+C: copy selected subgraph.
- Ctrl+V: paste subgraph.
- Ctrl+Z/Ctrl+Y: undo/redo.
- Ctrl+A: select all.
- F: zoom to selection.
- Home: zoom to all.
- Escape: cancel drag or clear transient state.

Menus:

- Background menu: add node, paste, zoom to all.
- Node menu: rename, duplicate, delete, disable, run node, inspect output.
- Edge menu: delete, disable, inspect.
- Port menu: copy reference, inspect connected nodes.

Drag states:

- idle.
- node drag.
- port connect drag.
- marquee drag.
- pan drag.
- edge reroute drag, later phase.

## Rendering And Cache Strategy

The editor should use layered rendering:

1. Background layer: canvas fill and grid.
2. Edge layer: normal edges.
3. Node layer: node cards and ports.
4. Selection layer: marquee, selected outlines, handles.
5. Overlay layer: connection preview, tooltips, menus, debug overlays.

Cache guidance:

- Grid can be cached per zoom bucket.
- Node cards can have state caches: normal, hover, selected, invalid, running.
- Edge layer should be cached separately from node layer.
- Moving nodes invalidates affected node bounds and connected edge bounds.
- Panning can reuse cached content where possible.
- Zoom changes can invalidate all graph visual caches.
- Large graphs must only render visible nodes and visible edge segments.

Edges should not be normal child widgets in V1. They are graph render items,
otherwise the widget tree becomes too heavy for large workflows.

## Hit Testing

Hit test order:

1. Active drag target.
2. Overlay items.
3. Ports on visible nodes.
4. Node chrome.
5. Edges.
6. Background.

Ports should win over node body when hit rects overlap.

Edge hit testing can use a widened path distance test. V1 may approximate
bezier hit testing with sampled line segments.

Large graph hit testing should use buckets:

- Node bucket by world-space bounds.
- Edge bucket by route bounds.
- Query bucket set intersecting cursor or marquee rect.

## Layout And Auto Layout

Manual layout is the default. V1 must preserve exact node positions from XSON.

Optional automatic layout phases:

- align selected nodes.
- distribute selected nodes.
- arrange from start node by control-flow levels.
- avoid node overlap.
- route orthogonal edges.

Auto layout should be a tool command, not an automatic mutation after load.

## API Shape

Initial public functions should stay small and stable:

```c
xuiWorkflowCreate
xuiWorkflowDestroy

xuiWorkflowRegisterNodeType
xuiWorkflowUnregisterNodeType

xuiWorkflowAddNode
xuiWorkflowRemoveNode
xuiWorkflowMoveNode

xuiWorkflowConnect
xuiWorkflowDisconnect

xuiWorkflowSetNodeConfig
xuiWorkflowGetNodeConfig

xuiWorkflowValidate
xuiWorkflowSaveXson
xuiWorkflowLoadXson

xuiWorkflowSetNodeRunState
xuiWorkflowSetEdgeRunState
```

Config schema builder:

```c
xuiConfigSchemaCreate
xuiConfigSchemaDestroy
xuiConfigSchemaAddString
xuiConfigSchemaAddTextArea
xuiConfigSchemaAddInt
xuiConfigSchemaAddFloat
xuiConfigSchemaAddBool
xuiConfigSchemaAddSelect
xuiConfigSchemaAddExpression
xuiConfigSchemaAddObject
xuiConfigSchemaAddArray
xuiConfigSchemaAddConditionBuilder
xuiConfigSchemaAddMappingBuilder
```

The actual implementation can start internal-only and expose API after tests
stabilize.

## Suggested Files

Implementation:

```text
src/xui_flow_graph.c
src/xui_flow_node.c
src/xui_flow_edge.c
src/xui_workflow.c
src/xui_workflow_schema.c
src/xui_workflow_xson.c
src/xui_workflow_xrt.c
```

Tests:

```text
test_xui/xui_flow_graph_test.c
test_xui/xui_workflow_schema_test.c
test_xui/xui_workflow_validation_test.c
test_xui/xui_workflow_xson_test.c
test_xui/xui_workflow_interaction_test.c
```

Examples:

```text
examples/xui_flowgraph/main.c
examples/xui_workflow_editor/main.c
```

Docs:

```text
docs/xui/widget-flowgraph.md
docs/xui/widget-workflow.md
docs/workflow-control-design.md
docs/workflow-control-spec.md
```

## Example Acceptance Workflow

The first visual example should allow this graph:

```text
Start -> LLM Chat -> Condition -> End
                         |
                         -> End
```

Capabilities:

- User can add these nodes.
- User can move nodes.
- User can connect control ports.
- User can configure LLM prompt and condition branches.
- Condition config creates dynamic output ports.
- User can save and reload the workflow as XSON.
- Validator catches missing required fields and invalid edges.
- Mock run state marks nodes and edges as running, success, skipped, or failed.

## Implementation Risks

- XSON infrastructure may not be ready when the graph model is ready. Mitigate
  by isolating serialization behind `xui_workflow_xson.c`.
- Config schema rendering may become too broad. Mitigate by starting with
  propertygrid-backed primitive fields and reserving advanced field kinds.
- Dynamic ports can invalidate existing edges. Mitigate with stable port ids
  and invalid-edge diagnostics instead of silent deletion.
- Large graph performance can degrade if nodes/edges become normal widgets.
  Mitigate with graph-owned render items and spatial buckets.
- Control flow cycles may be needed for loops. Mitigate with explicit loop node
  semantics rather than allowing arbitrary cycles.
