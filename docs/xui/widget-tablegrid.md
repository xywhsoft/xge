# XUI TableGrid

TableGrid is the editable layer above TableView. It keeps the TableView rendering, scrolling, selection, sorting, column resize, merge, formatter, and custom renderer behavior, and only adds cell editing and commit/validation hooks.

The XUI2 version follows the XUI1 design rule: TableGrid does not own table data. The application or model remains the source of truth. TableGrid reads cells through the adapter and writes accepted edits through `onSet`.

## Goals

- reuse TableView instead of duplicating table rendering
- keep cell data owned by the adapter/model
- preserve XUI1 edit modes: display and quick; immediate is reserved
- support text, int, float, bool, enum, color, date, time, datetime, textarea, picker, file, image, and custom cell types
- keep validation failure inside the active editor and mark input error
- commit on Enter or outside pointer/wheel actions
- cancel on Escape or explicit textarea cancel
- expose counters and editing-cell queries for tests and diagnostics
- keep XSON deferred

## Structure

```text
TableGrid root widget
  TableView
    ScrollFrame
      viewport/table body
      scrollbars
  inline Input editor
  inline NumericInput editor
  inline ComboBox editor
  inline ColorPicker editor
  inline DatePicker editor

Textarea editor
  Popup owned by TableGrid
    TextEdit
    Cancel button
    OK button
```

Inline editors are placed over the active TableView cell and clipped to the TableView viewport. The textarea editor uses Popup because it needs more room than the source cell.

## Adapter Contract

TableGrid uses the same read callbacks as TableView:

```c
static int row_count(xui_widget grid, void* user);
static int read_cell(
	xui_widget grid,
	int row,
	int column,
	xui_table_view_cell_t* cell,
	void* user);
static void set_cell(
	xui_widget grid,
	int row,
	int column,
	const char* value,
	int type,
	void* user);

xuiTableGridSetAdapter(grid, row_count, read_cell, set_cell, user);
```

`read_cell` fills `sText`, optional `pValue`, cell type, disabled flag, invalid/dirty state, spans, and render/format hooks. During active editing, TableGrid marks the current cell with `bEditing` before TableView paints it.

`set_cell` is called only when the committed value differs from the original value and validation passes.

## Editing

Edit modes:

```c
xuiTableGridSetEditMode(grid, XUI_TABLE_GRID_EDIT_DISPLAY);
xuiTableGridSetEditMode(grid, XUI_TABLE_GRID_EDIT_QUICK);
```

Display mode starts editing with double click or keyboard confirmation. Quick mode starts editing with a single click release. `XUI_TABLE_GRID_EDIT_IMMEDIATE` is accepted by the API as a reserved mode but is not expanded into continuous-cell editing yet.

Programmatic editing:

```c
if ( xuiTableGridBeginEdit(grid, row, column) ) {
	/* user edits through the active editor */
	xuiTableGridEndEdit(grid, 1); /* commit */
}
```

`xuiTableGridEndEdit(grid, 0)` cancels the edit. Bool cells toggle directly without keeping an editor open.

## Editor Mapping

```text
TEXT       Input
INT        NumericInput with integer mode
FLOAT      NumericInput
BOOL       direct toggle
ENUM       ComboBox, configured by editor-config callback
COLOR      ColorPicker
DATE       DatePicker date mode
TIME       DatePicker time mode
DATETIME   DatePicker datetime mode
TEXTAREA   Popup + TextEdit + OK/Cancel
PICKER     external editor callback
FILE       external editor callback
IMAGE      external editor callback
CUSTOM     external editor callback
```

Picker/file/image/custom cells call `xui_table_grid_editor_proc` with the grid, row, column, current cell, and world-space cell rect. TableGrid does not assume a native file dialog or asset picker.

## Validation And Change

Validation runs before writing:

```c
static int validate_cell(
	xui_widget grid,
	int row,
	int column,
	const char* value,
	int type,
	void* user)
{
	return value_is_valid ? 1 : 0;
}

xuiTableGridSetValidate(grid, validate_cell, user);
```

If validation returns zero:

- the editor remains open
- reject count increments
- Input or NumericInput gets error state where applicable
- `onSet` and `onChange` are not called

After a successful value change, TableGrid calls `onSet`, then `onChange`, increments commit/change counters, hides the editor, and refreshes TableView.

## Editor Config

The editor-config callback supplies per-cell options:

```c
static int config_editor(
	xui_widget grid,
	int row,
	int column,
	int type,
	xui_table_grid_editor_config_t* config,
	void* user)
{
	static const char* status[] = {"Draft", "Ready", "Done"};

	if ( type == XUI_TABLE_CELL_TYPE_ENUM ) {
		config->arrEnumItems = status;
		config->iEnumItemCount = 3;
		config->iEnumSelected = 1;
		return 1;
	}
	return 0;
}

xuiTableGridSetEditorConfig(grid, config_editor, user);
```

The same config structure also covers numeric range/step/precision, date mode/format/limits, color alpha/palette, and range separators.

## Public API

```c
xuiTableGridGetType
xuiTableGridCreate
xuiTableGridGetTableView
xuiTableGridSetColumns
xuiTableGridSetRows
xuiTableGridSetAdapter
xuiTableGridSetValidate
xuiTableGridSetChange
xuiTableGridSetEditor
xuiTableGridSetEditorConfig
xuiTableGridSetEditMode
xuiTableGridGetEditMode
xuiTableGridSetFont
xuiTableGridGetFont
xuiTableGridSetDefaultMetrics
xuiTableGridSetSelectionMode
xuiTableGridSetScrollbarMode
xuiTableGridSetColors
xuiTableGridBeginEdit
xuiTableGridEndEdit
xuiTableGridIsEditing
xuiTableGridGetEditingCell
xuiTableGridGetCommitCount
xuiTableGridGetRejectCount
xuiTableGridGetCancelCount
xuiTableGridGetPickerCount
xuiTableGridGetChangeCount
```

Use `xuiTableGridGetTableView` when selection, scrolling, merge, rendering, formatter, sort, resize, or rect APIs are needed. Those remain TableView responsibilities.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_table_grid_test.bat
examples\xui_tablegrid\build.bat
build\xui_tablegrid.exe --frames 5
```

The example summary should include `create=1`, `layout=1`, `edit=1`, `validate=1`, `picker=1`, `scroll=1`, and `quick=1`.
