# XUI FlowGraph Widget

FlowGraph is the generic graph model and canvas foundation for workflow-like
editors. It owns graph nodes, ports, edges, viewport state, selection state,
and a first static canvas rendering surface.

V1 now includes the model layer and a cache-rendered `flowgraph` widget.
Pointer interaction is tracked in the Workflow / FlowGraph SPEC as later
phases.

## Scope

FlowGraph is intentionally workflow-neutral:

- It does not know LLM, HTTP, prompt, condition, or plugin semantics.
- It stores nodes, ports, edges, positions, and graph selection.
- It uses XRT memory and containers for owned model data.
- Edges are graph render items, not normal child widgets in the first design.

Workflow-specific node type registration, config schema, XSON, validation, and
runtime state belong to the Workflow layer.

## Model Objects

- `xui_flow_graph`: graph model handle.
- `xui_flow_node_desc_t`: node creation descriptor.
- `xui_flow_port_desc_t`: port creation descriptor.
- `xui_flow_edge_desc_t`: edge creation descriptor.
- `xui_flow_node_info_t`: read-only node snapshot for rendering/inspection.
- `xui_flow_port_info_t`: read-only port snapshot for rendering/inspection.
- `xui_flow_edge_info_t`: read-only edge snapshot for rendering/inspection.
- `xui_flow_graph_desc_t`: `flowgraph` widget descriptor.
- `xui_flow_hit_t`: graph canvas hit-test result.
- `xuiFlowGraphGetRevision`: monotonically increasing graph mutation revision.

Ports have a direction and a kind:

```c
XUI_FLOW_PORT_INPUT
XUI_FLOW_PORT_OUTPUT

XUI_FLOW_PORT_CONTROL
XUI_FLOW_PORT_DATA
```

## Minimal Usage

```c
xui_flow_graph graph;
xui_flow_node_desc_t node;
xui_flow_port_desc_t port;
xui_flow_edge_desc_t edge;
int start;
int end;

xuiFlowGraphCreate(&graph);

memset(&node, 0, sizeof(node));
node.iSize = sizeof(node);
node.sId = "n_start";
node.sType = "start";
node.sTitle = "Start";
node.fX = 80.0f;
node.fY = 120.0f;
xuiFlowGraphAddNode(graph, &node, &start);

node.sId = "n_end";
node.sType = "end";
node.sTitle = "End";
node.fX = 360.0f;
xuiFlowGraphAddNode(graph, &node, &end);

memset(&port, 0, sizeof(port));
port.iSize = sizeof(port);
port.sId = "out";
port.iDirection = XUI_FLOW_PORT_OUTPUT;
port.iKind = XUI_FLOW_PORT_CONTROL;
xuiFlowGraphAddPort(graph, start, &port, NULL);

port.sId = "in";
port.iDirection = XUI_FLOW_PORT_INPUT;
xuiFlowGraphAddPort(graph, end, &port, NULL);

memset(&edge, 0, sizeof(edge));
edge.iSize = sizeof(edge);
edge.sId = "e_start_end";
edge.iKind = XUI_FLOW_PORT_CONTROL;
edge.sFromNode = "n_start";
edge.sFromPort = "out";
edge.sToNode = "n_end";
edge.sToPort = "in";
xuiFlowGraphAddEdge(graph, &edge, NULL);

xuiFlowGraphDestroy(graph);
```

## Widget Usage

```c
xui_flow_graph_desc_t desc;
xui_widget canvas;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pGraph = graph;
desc.bOwnGraph = 0;

xuiFlowGraphWidgetCreate(context, &canvas, &desc);
xuiWidgetSetRect(canvas, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
xuiWidgetAddChild(root, canvas);
```

When `pGraph` is null, the widget creates and owns an empty graph. When
`bOwnGraph` is non-zero, the widget destroys the graph with the widget.

The runnable static-canvas example is under `examples/xui_flowgraph`:

```bat
examples\xui_flowgraph\build.bat
build\xui_flowgraph.exe --frames 2
```

## Hit Testing

`xuiFlowGraphWidgetHitTest` takes widget-local coordinates and returns one of:

```c
XUI_FLOW_HIT_NONE
XUI_FLOW_HIT_BACKGROUND
XUI_FLOW_HIT_NODE
XUI_FLOW_HIT_PORT
XUI_FLOW_HIT_EDGE
```

Ports have priority over node cards, node cards have priority over edges, and
background is returned for points inside the canvas but outside graph items.
`xuiFlowGraphWidgetSelectAt` uses the same hit result to perform single
selection: node/port hits select the node, edge hits select the edge, and
background hits clear selection.

## Current Constraints

- Node, port, and edge ids are workflow-local strings.
- Node and edge id lookup uses XRT dictionaries.
- Ordered node/edge/port storage uses XRT arrays.
- Index values returned by the model are zero-based public indexes.
- Internal XRT arrays remain one-based and are hidden from callers.
- `xuiFlowGraphGetNode`, `xuiFlowGraphGetPort`, and `xuiFlowGraphGetEdge`
  return borrowed string pointers valid until the graph is mutated or destroyed.
- The widget renders background, grid, node cards, ports, selected state, and
  straight-line edges into the existing XUI cache path.
- External model mutations update the graph revision. The widget checks this
  revision during `xuiUpdate` and invalidates its cache when it changes.
- Completely off-canvas edges are skipped before draw submission.
- Hit testing currently uses node rectangles, 14 px port hit boxes, and a 6 px
  edge distance tolerance around the straight-line route.
- Removing nodes and edges is currently immediate; command/undo wrapping is
  deferred until the command phase.

## Deferred

- Hit testing.
- Pan/zoom interaction.
- Connection drag preview.
- Edge rerouting.
- Hover, invalid, and runtime execution visual states.
- Spatial buckets.
