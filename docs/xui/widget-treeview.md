# XUI TreeView

TreeView is a hierarchical row widget. The XUI2 version preserves the mature XUI1 expand, checkbox, disabled-node, keyboard navigation, ensure-visible, adapter, and custom-row-rendering behavior, while implementing clipping and scrolling through ScrollFrame.

## Goals

- keep XUI1 node id and parent id model
- rebuild a visible row list from expanded nodes
- keep disabled nodes visible but non-selectable
- preserve expander, checkbox, selection, hover, focus, and keyboard behavior
- reuse ScrollFrame and ScrollModel for wheel scrolling, clipping, and compact/full scrollbar visuals
- expose frame, viewport, model, row rects, hover index, focus index, visible ids, and counters for tests and integration
- keep XSON deferred

## Structure

```text
TreeView root widget
  ScrollFrame
    viewport widget
      cache-rendered visible tree rows
    vertical ScrollBar
```

Nodes are not child widgets. TreeView stores an internal node array and a visible-index array. Rendering walks the visible array and draws rows into the viewport cache.

The horizontal scrollbar policy is hidden by default. The vertical scrollbar policy is automatic. The default scrollbar mode is compact.

## Nodes

Nodes use stable integer ids:

```c
xuiTreeViewAddNode(tree, 10, -1, "Project");
xuiTreeViewAddNode(tree, 20, 10, "src");
xuiTreeViewAddNode(tree, 30, 20, "xui_tree_view.c");

xuiTreeViewSetNodeExpanded(tree, 10, 1);
xuiTreeViewSetNodeExpanded(tree, 20, 1);
xuiTreeViewSetSelected(tree, 30);
```

`iParent < 0` marks a root node. `xuiTreeViewAddNode` requires the parent to already exist, matching XUI1's deterministic insertion order.

Node state APIs:

```c
xuiTreeViewSetNodeExpanded
xuiTreeViewSetNodeEnabled
xuiTreeViewSetNodeChecked
xuiTreeViewSetNodeDecorations
```

`xuiTreeViewSetNodeChecked` reserves the checkbox slot automatically. `xuiTreeViewSetNodeDecorations` can hide the icon or checkbox space for a node.

## Adapter

Adapter mode mirrors XUI1 for generated or external data:

```c
static int count_nodes(xui_widget tree, void* user);
static int read_node(xui_widget tree, int index, xui_tree_view_node_t* node, void* user);

xuiTreeViewSetAdapter(tree, count_nodes, read_node, user);
xuiTreeViewRefreshAdapter(tree);
```

`read_node` should fill `iId`, `iParent`, `sText`, and optional state fields. During refresh TreeView preserves the previous selected id and scroll offset when possible.

## Interaction

Mouse behavior:

- clicking the expander toggles expanded/collapsed state
- clicking the checkbox toggles the node checked state
- clicking the row selects the enabled node and fires the select callback
- disabled nodes consume the click but do not change selection

Keyboard behavior:

- Up/Down select previous or next enabled visible node
- PageUp/PageDown move by the current viewport row count
- Home/End select the first or last enabled visible node
- Right expands the selected node, or moves into the next visible child when already expanded
- Left collapses the selected node, or moves to the parent when already collapsed
- Enter/Space notify selection for the current node

When collapsing a branch hides the selected descendant, TreeView falls back to the collapsed node if it is visible and enabled.

## Rendering

Default rendering draws:

- a rounded border using the focus color while focused
- a clipped viewport background
- rounded hover and selected row fills
- selected row text in white
- a visible expander hotspot; selected expandable rows draw a white triangle over a darker hotspot so the click target remains clear
- muted disabled text and decoration colors
- expander triangles for nodes with children
- optional checkbox and icon decorations
- compact ScrollFrame scrollbar visuals

Custom row rendering can replace the default row draw by returning non-zero:

```c
static int render_tree_row(
	xui_widget tree,
	int node_id,
	int visible_index,
	const xui_tree_view_node_t* node,
	xui_draw_context draw,
	xui_rect_t row,
	int state,
	void* user)
{
	if ( (state & XUI_TREE_ITEM_SELECTED) != 0 ) {
		/* custom paint */
	}
	return 0; /* return non-zero to skip default rendering */
}

xuiTreeViewSetItemRenderer(tree, render_tree_row, user);
```

The row rect passed to the renderer is viewport-local.

## Public API

```c
xuiTreeViewGetType
xuiTreeViewCreate
xuiTreeViewSetSelect
xuiTreeViewSetItemRenderer
xuiTreeViewSetAdapter
xuiTreeViewRefreshAdapter
xuiTreeViewClear
xuiTreeViewSetNodes
xuiTreeViewAddNode
xuiTreeViewGetNodeCount
xuiTreeViewGetNode
xuiTreeViewGetNodeById
xuiTreeViewFindNode
xuiTreeViewSetNodeExpanded
xuiTreeViewGetNodeExpanded
xuiTreeViewToggleNode
xuiTreeViewSetNodeEnabled
xuiTreeViewGetNodeEnabled
xuiTreeViewSetNodeChecked
xuiTreeViewGetNodeChecked
xuiTreeViewSetNodeDecorations
xuiTreeViewSetSelected
xuiTreeViewGetSelected
xuiTreeViewGetVisibleCount
xuiTreeViewGetVisibleNodeId
xuiTreeViewGetVisibleNode
xuiTreeViewGetVisibleIndexOfId
xuiTreeViewSetFont
xuiTreeViewGetFont
xuiTreeViewSetMetrics
xuiTreeViewGetMetrics
xuiTreeViewSetScroll
xuiTreeViewGetScroll
xuiTreeViewEnsureVisible
xuiTreeViewSetScrollbarMode
xuiTreeViewGetScrollbarMode
xuiTreeViewSetColors
xuiTreeViewSetDecorationColors
xuiTreeViewSetScrollbarColors
xuiTreeViewGetFrameWidget
xuiTreeViewGetViewportWidget
xuiTreeViewGetModel
xuiTreeViewGetViewportRect
xuiTreeViewGetItemRect
xuiTreeViewGetItemAt
xuiTreeViewGetHoverIndex
xuiTreeViewGetFocusIndex
xuiTreeViewGetSelectCount
xuiTreeViewGetChangeCount
```

## Style Properties

```text
treeview.background.color
treeview.border.color
treeview.focus.color
treeview.row.color
treeview.row.hover_color
treeview.row.selected_color
treeview.text.color
treeview.text.disabled_color
treeview.expander.color
treeview.icon.color
treeview.check.color
treeview.item.height
treeview.indent
treeview.padding
treeview.radius
treeview.border.width
font.name
```

Scrollbar colors are configured with `xuiTreeViewSetScrollbarColors`, which forwards to the internal ScrollFrame.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_tree_view_test.bat
examples\xui_treeview\build.bat
build\xui_treeview.exe --frames 5
```

The example summary should include `create=1`, `layout=1`, `state=1`, `interaction=1`, `scroll=1`, and `custom=1`.
