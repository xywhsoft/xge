# XUI Workflow Widget

Workflow is the semantic layer built on FlowGraph. It lets applications define
custom node types, create workflow node instances, bind rich config data, and
later save/load the whole workflow as XSON.

V1 starts with the node type registry and instance creation. Config schema,
dynamic ports, validation, XSON persistence, and runtime state are tracked in
later SPEC phases.

## Scope

Workflow owns:

- node type registry.
- node instance creation by type id.
- static port projection from node type to node instance.
- config values on top of XRT `xvalue`.
- later XSON import/export using XRT XSON APIs.

Workflow does not execute the graph. Execution is host-owned.

## Node Types

A node type defines reusable node shape and semantics:

```c
xui_workflow_node_type_desc_t type;
xui_flow_port_desc_t input;
xui_flow_port_desc_t output;

memset(&input, 0, sizeof(input));
input.iSize = sizeof(input);
input.sId = "in";
input.iDirection = XUI_FLOW_PORT_INPUT;
input.iKind = XUI_FLOW_PORT_CONTROL;

memset(&output, 0, sizeof(output));
output.iSize = sizeof(output);
output.sId = "out";
output.iDirection = XUI_FLOW_PORT_OUTPUT;
output.iKind = XUI_FLOW_PORT_CONTROL;

memset(&type, 0, sizeof(type));
type.iSize = sizeof(type);
type.sId = "llm.chat";
type.iVersion = 1;
type.sTitle = "LLM Chat";
type.sCategory = "AI";
type.pInputs = &input;
type.iInputCount = 1;
type.pOutputs = &output;
type.iOutputCount = 1;
```

## Minimal Usage

```c
xui_workflow workflow;
int typeIndex;
int nodeIndex;

xuiWorkflowCreate(&workflow);
xuiWorkflowRegisterNodeType(workflow, &type, &typeIndex);
xuiWorkflowAddNode(workflow, "llm.chat", "n_llm", "Ask model", 320.0f, 120.0f, &nodeIndex);
xuiWorkflowDestroy(workflow);
```

The node instance receives static ports copied from the registered node type.

## Config Values

Workflow config is designed around XRT `xvalue`:

- node config is stored as an `xvalue` table.
- schema-only node definitions can later be loaded from XSON into `xvalue`.
- duplicate/paste will use `xvoDeepCopy`.
- persistence will use `xrtParseXSON*` and `xrtStringifyXSON*`.

The first model slice accepts optional config values but does not yet validate
schema fields.

## Deferred

- Config schema builder.
- Dynamic port callbacks.
- Validation diagnostics.
- XSON save/load.
- Runtime state visualization.
- Composite editor panels.
- Host execution bridge.

