# XUI TableView

TableView is a cached, scrollable grid for tabular data. The XUI2 version keeps the XUI1 user-facing behavior while replacing the old widget plumbing with typed widgets, ScrollFrame, and cache rendering.

## Goals

- keep the XUI1 column, row, cell, adapter, formatter, renderer, merge-provider, and tooltip model
- keep fixed headers that scroll horizontally with the content
- support automatic horizontal and vertical scrollbars through ScrollFrame
- support cell-selection and row-selection modes
- preserve disabled rows/cells, hover, focus, active cell, keyboard navigation, sorting, column resize, and ensure-visible behavior
- keep cell types for text, bool, color, picker/file/image, textarea, and custom renderers
- expose frame, viewport, ScrollModel, rect queries, counters, and callback hooks for integration tests
- keep XSON deferred

## Structure

```text
TableView root widget
  cache-rendered fixed header
  ScrollFrame
    viewport widget
      cache-rendered table cells
    horizontal ScrollBar
    vertical ScrollBar
```

Cells are not child widgets. TableView asks the adapter for visible cells during viewport painting. The root widget paints the background, border, and header.

The default scrollbar mode is compact. Both scrollbar policies are automatic.

## Columns And Rows

Columns are copied into the widget:

```c
xui_table_view_column_t columns[3] = {0};

columns[0].sTitle = "Name";
columns[0].fWidth = 120.0f;
columns[1].sTitle = "Enabled";
columns[1].fWidth = 72.0f;
columns[1].iType = XUI_TABLE_CELL_TYPE_BOOL;
columns[2].sTitle = "Color";
columns[2].fWidth = 96.0f;
columns[2].iType = XUI_TABLE_CELL_TYPE_COLOR;

xuiTableViewSetColumns(table, columns, 3);
```

If `bVisibleSet` is not set, the column is visible. If `bResizableSet` is not set, the column is resizable. Width falls back to `fDefaultColumnWidth`.

Rows are copied as row-state records. The row count comes from `xuiTableViewSetRows` unless an adapter count callback is installed.

## Adapter

Adapter mode mirrors XUI1:

```c
static int row_count(xui_widget table, void* user);
static int read_cell(
	xui_widget table,
	int row,
	int column,
	xui_table_view_cell_t* cell,
	void* user);

xuiTableViewSetAdapter(table, row_count, read_cell, user);
xuiTableViewRefreshAdapter(table);
```

`read_cell` should fill `sText`, optional `sTooltip`, cell type, disabled/editing/invalid/dirty flags, spans, and optional renderer or formatter callbacks. TableView normalizes `iRowSpan` and `iColSpan` to at least 1 and clamps spans to the current row and column range.

## Selection And Navigation

Selection modes:

```c
xuiTableViewSetSelectionMode(table, XUI_TABLE_VIEW_SELECTION_CELL);
xuiTableViewSetSelectionMode(table, XUI_TABLE_VIEW_SELECTION_ROW);
```

Mouse behavior:

- clicking an enabled cell selects it in cell-selection mode
- clicking an enabled row selects the row in row-selection mode
- disabled rows and disabled cells consume the click but do not change selection
- clicking a header sorts that column and toggles descending on repeated clicks
- dragging near the right edge of a resizable header column changes column width

Keyboard behavior:

- Up/Down move by enabled row
- Left/Right move between visible columns in cell mode
- PageUp/PageDown move by the current viewport row count
- Home/End move to row/column boundaries
- Ctrl+Home/Ctrl+End move to the first or last cell
- Enter/Space notify the current selection again

## Merged Cells

Merged cells are provided either by cell spans or by a merge provider:

```c
static int merge_cell(
	xui_widget table,
	int row,
	int column,
	int* row_span,
	int* column_span,
	void* user)
{
	if ( row == 2 && column == 1 ) {
		*row_span = 2;
		*column_span = 2;
		return 1;
	}
	return 0;
}

xuiTableViewSetMergeProvider(table, merge_cell, user);
```

Hit testing and rect queries return the merge owner. `xuiTableViewGetCellContentRect` returns unscrolled content coordinates. `xuiTableViewGetCellRect` returns table-local viewport coordinates after scroll offset.

## Rendering

Default rendering draws:

- a single focus/border stroke around the root
- fixed header background and separators
- row stripe background
- hover and selected fills
- disabled text and disabled row background
- bool checkbox, color swatch, and picker button visuals
- invalid left marker, dirty corner marker, editing stroke, and focus stroke
- compact/full ScrollFrame scrollbars

Custom renderers can replace header or cell drawing by returning non-zero:

```c
static int render_cell(
	xui_widget table,
	int row,
	int column,
	const xui_table_view_cell_t* cell,
	xui_draw_context draw,
	xui_rect_t rect,
	int state,
	void* user)
{
	if ( column == 5 ) {
		/* custom paint */
		return 1;
	}
	return 0;
}

xuiTableViewSetCellRenderer(table, render_cell, user);
```

Renderer rects are viewport-local for cells and table-local for headers.

## Public API

```c
xuiTableViewGetType
xuiTableViewCreate
xuiTableViewSetColumns
xuiTableViewSetRows
xuiTableViewSetAdapter
xuiTableViewRefreshAdapter
xuiTableViewSetSort
xuiTableViewSetSelect
xuiTableViewSetColumnResize
xuiTableViewSetHover
xuiTableViewSetMergeProvider
xuiTableViewSetHeaderRenderer
xuiTableViewSetCellRenderer
xuiTableViewSetColumnFormatter
xuiTableViewSetFont
xuiTableViewGetFont
xuiTableViewSetDefaultMetrics
xuiTableViewGetDefaultMetrics
xuiTableViewSetSelectionMode
xuiTableViewGetSelectionMode
xuiTableViewSetSelectedRow
xuiTableViewGetSelectedRow
xuiTableViewSetSelectedCell
xuiTableViewGetSelectedCell
xuiTableViewSetOffset
xuiTableViewGetOffset
xuiTableViewSetScrollbarMode
xuiTableViewGetScrollbarMode
xuiTableViewGetColumnCount
xuiTableViewGetColumn
xuiTableViewGetColumnWidth
xuiTableViewSetColumnWidth
xuiTableViewGetRowCount
xuiTableViewGetFirstVisible
xuiTableViewGetPaintVisibleCount
xuiTableViewGetActiveCell
xuiTableViewGetHoverCell
xuiTableViewGetCellContentRect
xuiTableViewGetCellRect
xuiTableViewGetFrameWidget
xuiTableViewGetViewportWidget
xuiTableViewGetBodyWidget
xuiTableViewGetModel
xuiTableViewGetViewportRect
xuiTableViewGetItemAt
xuiTableViewSetColors
xuiTableViewGetColors
xuiTableViewSetDisabledTextColor
xuiTableViewSetScrollbarColors
xuiTableViewRefresh
xuiTableViewEnsureCellVisible
xuiTableViewGetSortColumn
xuiTableViewGetSelectCount
xuiTableViewGetSortCount
xuiTableViewGetHoverCount
xuiTableViewGetColumnResizeCount
xuiTableViewGetChangeCount
```

Zero-initialized `xui_table_view_desc_t` does not select a cell by default. Use `xuiTableViewSetSelectedRow` or `xuiTableViewSetSelectedCell` when row 0 / column 0 should be selected initially.

## Style Properties

```text
tableview.background.color
tableview.header.color
tableview.header.text_color
tableview.row.color
tableview.row.alt_color
tableview.row.hover_color
tableview.row.selected_color
tableview.row.disabled_color
tableview.grid.color
tableview.text.color
tableview.text.disabled_color
tableview.focus.color
tableview.default.column_width
tableview.default.row_height
tableview.header.height
tableview.border.width
font.name
```

Scrollbar colors are configured with `xuiTableViewSetScrollbarColors`, which forwards to the internal ScrollFrame.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_table_view_test.bat
examples\xui_tableview\build.bat
build\xui_tableview.exe --frames 5
```

The example summary should include `create=1`, `layout=1`, `select=1`, `sort=1`, `scroll=1`, and `custom=1`.
