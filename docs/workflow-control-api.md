# XUI Workflow / FlowGraph API Notes

This document records the implemented V1 API surface for the XUI2 workflow
control. It is intentionally implementation-facing: host applications should be
able to build a custom workflow editor without reading the private source.

## Layers

The workflow system has two layers:

- `FlowGraph`: generic node, port, edge, viewport, selection, diagnostics, and
  canvas rendering.
- `Workflow`: semantic layer for node type registration, config schema,
  variables, validation, XSON persistence, runtime state, and the composite
  workflow widget.

`Workflow` owns a `FlowGraph` instance. Hosts can still access the graph through
`xuiWorkflowGetGraph()` when they need low-level canvas operations.

## FlowGraph Model API

Create and destroy:

```c
xui_flow_graph graph = NULL;
xuiFlowGraphCreate(&graph);
xuiFlowGraphDestroy(graph);
```

Core editing APIs:

- `xuiFlowGraphAddNode`
- `xuiFlowGraphAddPort`
- `xuiFlowGraphAddEdge`
- `xuiFlowGraphAddEdgePreserveInvalid`
- `xuiFlowGraphRemoveNode`
- `xuiFlowGraphRemoveEdge`
- `xuiFlowGraphSetNodePosition`
- `xuiFlowGraphSetNodeSummary`
- `xuiFlowGraphSetNodeConfig`
- `xuiFlowGraphRebuildNodeDynamicPorts`

`xuiFlowGraphAddEdge()` requires both endpoint nodes and ports to exist.
`xuiFlowGraphAddEdgePreserveInvalid()` is for persistence and repair workflows:
it stores the edge even when an endpoint is missing, then marks it invalid.

Selection and inspection:

- `xuiFlowGraphSelectNode`
- `xuiFlowGraphSelectEdge`
- `xuiFlowGraphClearSelection`
- `xuiFlowGraphGetNode`
- `xuiFlowGraphGetPort`
- `xuiFlowGraphGetEdge`
- `xuiFlowGraphFindNode`
- `xuiFlowGraphFindEdge`

Viewport:

- `xuiFlowGraphSetViewport`
- `xuiFlowGraphGetViewport`
- `xuiFlowGraphWorldToScreen`
- `xuiFlowGraphScreenToWorld`

Diagnostics:

- `xuiFlowGraphAddDiagnostic`
- `xuiFlowGraphClearDiagnostics`
- `xuiFlowGraphGetDiagnosticCount`
- `xuiFlowGraphGetNodeDiagnosticCount`
- `xuiFlowGraphGetEdgeDiagnosticCount`

Runtime visual state can also be mirrored directly into FlowGraph:

- `xuiFlowGraphSetNodeRunState`
- `xuiFlowGraphSetEdgeRunState`
- `xuiFlowGraphSetEdgeRoute`

Edge route hints are stored on each edge and round-trip through XSON:

- `XUI_FLOW_ROUTE_AUTO`
- `XUI_FLOW_ROUTE_STRAIGHT`
- `XUI_FLOW_ROUTE_ORTHOGONAL`
- `XUI_FLOW_ROUTE_BEZIER`

The graph widget draws nodes and edges directly in one canvas. Edges are not
represented as child widgets.

## FlowGraph Widget API

Create a graph widget with `xuiFlowGraphWidgetCreate()`.

Important APIs:

- `xuiFlowGraphWidgetGetGraph`
- `xuiFlowGraphWidgetSetGraph`
- `xuiFlowGraphWidgetHitTest`
- `xuiFlowGraphWidgetGetHoverHit`
- `xuiFlowGraphWidgetSelectAt`

The widget supports node selection, node dragging, connection preview, edge
selection, marquee selection, hover feedback for nodes, ports, edges, and the
background, diagnostic/result badges on nodes, deletion through input events,
visible node culling, and edge segment bounds culling.

## Workflow Widget API

Create a composite workflow widget with `xuiWorkflowWidgetCreate()`.

Important APIs:

- `xuiWorkflowWidgetGetWorkflow`
- `xuiWorkflowWidgetSetWorkflow`
- `xuiWorkflowWidgetGetCanvas`

The widget wraps an inner `FlowGraph` canvas and keeps the graph bound to the
workflow model.

## Node Type Registration

Hosts register node types before creating workflow nodes:

```c
xui_workflow_node_type_desc_t type;
memset(&type, 0, sizeof(type));
type.iSize = sizeof(type);
type.sId = "llm.mock";
type.iVersion = 1;
type.sTitle = "LLM Mock";
type.sCategory = "Action";
type.pInputs = inputs;
type.iInputCount = input_count;
type.pOutputs = outputs;
type.iOutputCount = output_count;
xuiWorkflowRegisterNodeType(workflow, &type, NULL);
```

Each node type can define:

- stable type id
- title, category, description, icon
- static input and output ports
- config schema
- dynamic port callback
- host validation callback

Port ids must be stable because edges reference ports by id.

## Config Schema Field Types

Schemas are `xvalue` tables created through:

- `xuiWorkflowConfigSchemaCreate`
- `xuiWorkflowConfigSchemaAddField`

Field kinds:

- `XUI_WORKFLOW_CONFIG_FIELD_STRING`
- `XUI_WORKFLOW_CONFIG_FIELD_NUMBER`
- `XUI_WORKFLOW_CONFIG_FIELD_BOOL`
- `XUI_WORKFLOW_CONFIG_FIELD_ENUM`
- `XUI_WORKFLOW_CONFIG_FIELD_VARIABLE_REF`
- `XUI_WORKFLOW_CONFIG_FIELD_EXPRESSION`
- `XUI_WORKFLOW_CONFIG_FIELD_OBJECT`
- `XUI_WORKFLOW_CONFIG_FIELD_ARRAY`

Fields can be required and can carry default `xvalue` data. Default configs are
created with `xuiWorkflowCreateDefaultConfig()`. Validation is available through
`xuiWorkflowValidateConfig()` and `xuiWorkflowValidateConfigEx()`.

## Dynamic Port Rules

Dynamic ports are rebuilt through a node type callback:

```c
typedef int (*xui_workflow_dynamic_ports_proc)(
    xui_workflow workflow,
    const char* node_id,
    const char* type_id,
    xvalue config,
    xui_flow_port_desc_t* ports,
    int port_capacity,
    int* port_count,
    void* user);
```

The workflow layer calls this callback after config changes and merges static
and dynamic ports on the graph node. Existing valid edges are preserved. Edges
that point at removed dynamic ports are retained as invalid references so the
user can repair them.

## Workflow XSON Format

Workflow persistence uses XRT XSON APIs:

- `xrtStringifyXSON_File`
- `xrtParseXSON_File`
- `xuiWorkflowToXValue`
- `xuiWorkflowLoadXValue`
- `xuiWorkflowSaveXSONFile`
- `xuiWorkflowLoadXSONFile`

The V1 object shape is:

```xson
{
  kind: "xui.workflow",
  version: 1,
  metadata: {
    format: "xui.workflow.v1",
    schemaVersion: 1
  },
  nodeTypes: ["start", "llm.mock"],
  variables: [
    { id: "var_prompt", title: "Prompt", type: "string", scope: "workflow", default: "user_prompt" }
  ],
  nodes: [
    {
      id: "llm",
      type: "llm.mock",
      title: "LLM Mock",
      summary: "model: glm",
      x: 290,
      y: 130,
      w: 220,
      h: 120,
      runState: 2,
      runPreview: "drafting",
      ports: [
        { id: "in", direction: 0, kind: 0, required: false, multi: false, dynamic: false }
      ],
      config: { prompt: "hello" }
    }
  ],
  edges: [
    {
      id: "e_llm_condition",
      kind: 0,
      fromNode: "llm",
      fromPort: "out",
      toNode: "condition",
      toPort: "in",
      route: { style: 2, bias: 0.65, sourceOffset: 12, targetOffset: -8 },
      invalid: false,
      runState: 2,
      runPreview: "true"
    }
  ],
  viewport: { panX: 20, panY: 120, zoom: 1, width: 760, height: 560 }
}
```

Unknown node types are allowed during load so the graph can be shown and fixed.
`xuiWorkflowValidateGraph()` reports missing node types after load.

## Validation Diagnostic Codes

Implemented graph validation reports diagnostics into the FlowGraph diagnostic
store. Current codes include:

- `workflow.node_type.missing`
- `workflow.config.required`
- `workflow.config.variable_ref`
- `workflow.config.expression`
- `workflow.start.missing`
- `workflow.end.missing`
- `workflow.edge.direction`
- `workflow.edge.invalid_ref`
- `workflow.control.cycle`
- `workflow.node.unreachable`

Hosts can add type-specific diagnostics through `xui_workflow_validate_proc`.

## Runtime State

Node run states:

- `XUI_WORKFLOW_NODE_RUN_IDLE`
- `XUI_WORKFLOW_NODE_RUN_QUEUED`
- `XUI_WORKFLOW_NODE_RUN_RUNNING`
- `XUI_WORKFLOW_NODE_RUN_SUCCESS`
- `XUI_WORKFLOW_NODE_RUN_FAILED`
- `XUI_WORKFLOW_NODE_RUN_SKIPPED`
- `XUI_WORKFLOW_NODE_RUN_WARNING`

Edge run states:

- `XUI_WORKFLOW_EDGE_RUN_IDLE`
- `XUI_WORKFLOW_EDGE_RUN_ACTIVE`
- `XUI_WORKFLOW_EDGE_RUN_TAKEN`
- `XUI_WORKFLOW_EDGE_RUN_SKIPPED`
- `XUI_WORKFLOW_EDGE_RUN_INVALID`

Use `xuiWorkflowSetNodeRunState()` and `xuiWorkflowSetEdgeRunState()` to apply
state and compact preview text. The FlowGraph widget renders these states as
node/edge visual feedback.

## Host Integration Boundaries

XUI owns:

- graph editing model
- canvas interactions
- config schema defaults and validation
- structural graph validation
- XSON import/export
- run-state visualization

Host applications own:

- actual workflow execution
- external API calls
- node-specific config editors
- secret management
- durable project storage policy
- advanced node-specific validation

The reference example is `examples/xui_workflow_editor`.
