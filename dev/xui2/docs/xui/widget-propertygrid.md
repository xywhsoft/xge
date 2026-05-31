# XUI PropertyGrid

PropertyGrid is an inspector-style editable property table. It is built on `xuiTableGrid`, which in turn reuses `xuiTableView` for row painting, selection, merged category rows, scrolling, and editor placement.

The XUI2 version keeps the useful XUI1 behavior while removing the old table/control infrastructure: data is stored in a compact property model, category rows are merged across the two columns, the header is hidden, the value column owns editing, and picker-like values delegate to application callbacks.

## Goals

- preserve XUI1 category grouping, collapse/expand, hidden properties, readonly and disabled states
- keep quick edit as the default interaction
- support tooltip descriptions and an optional bottom description panel
- mark dirty values from an explicit flag or by comparing value and default value
- expose validation, change, selection, action, renderer, scroll, and diagnostic counters
- keep XSON deferred

## Structure

```text
PropertyGrid root widget
  TableGrid
    TableView
      merged category rows
      two property columns
      vertical scrollbar
    inline editors and popup editors from TableGrid

Optional description panel
  rendered by the PropertyGrid root below the TableGrid area
```

The two visible columns are fixed by the control:

```text
column 0: property name, readonly
column 1: property value, editable when allowed
```

Horizontal scrolling is hidden by default. The name column uses `fNameWidth`; the value column fills the remaining viewport width.

## Categories And Properties

```c
xui_property_grid_desc_t desc;
xui_property_desc_t prop;
xui_widget grid;
int cat;
int title;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.fNameWidth = 156.0f;
desc.fRowHeight = 26.0f;
desc.fCategoryHeight = 28.0f;
desc.iDescriptionMode = XUI_PROPERTY_GRID_DESCRIPTION_BOTH;
desc.fDescriptionPanelHeight = 56.0f;
desc.iEditMode = XUI_TABLE_GRID_EDIT_QUICK;
xuiPropertyGridCreate(ctx, &grid, &desc);

cat = xuiPropertyGridAddCategory(grid, "appearance", "Appearance", 1);

memset(&prop, 0, sizeof(prop));
prop.sId = "title";
prop.sName = "Title";
prop.sDescription = "Displayed name in the inspector.";
prop.iType = XUI_TABLE_CELL_TYPE_TEXT;
prop.sValue = "Player Controller";
prop.sDefaultValue = "Player Controller";
title = xuiPropertyGridAddProperty(grid, cat, &prop);
```

Only one category level is supported. Category rows are visible even when collapsed; hidden properties stay in the model but are excluded from visible rows.

## Editing

PropertyGrid uses TableGrid editors for value cells:

```text
TEXT       Input
INT        NumericInput integer mode
FLOAT      NumericInput
BOOL       direct toggle
ENUM       ComboBox
COLOR      ColorPicker
DATE       DatePicker date mode
TIME       DatePicker time mode
DATETIME   DatePicker datetime mode
TEXTAREA   Popup + TextEdit
PICKER     external action callback
FILE       external action callback
IMAGE      external action callback
CUSTOM     external action callback
```

Programmatic editing uses property indexes:

```c
if ( xuiPropertyGridBeginEdit(grid, title) ) {
	xuiPropertyGridEndEdit(grid, 1);
}
```

Bool values toggle immediately. Picker, file, image, and custom values call the property-specific action callback first, then the global action callback if no property action is installed.

## Editor Config

PropertyGrid stores per-property editor config. Enum item pointers are copied into an internal fixed array so callers do not need to keep a temporary config struct alive.

```c
static const char* themes[] = {"Default", "Focused", "High contrast"};
xui_table_grid_editor_config_t config;

memset(&config, 0, sizeof(config));
config.arrEnumItems = themes;
config.iEnumItemCount = 3;
config.iEnumSelected = 1;
xuiPropertyGridSetEditorConfig(grid, theme_property, &config);
```

The same config structure supports numeric range/step/precision, date mode and limits, color palette/alpha, and action metadata.

## Validation And Change

```c
static int validate_property(
	xui_widget grid,
	int property,
	const char* id,
	const char* value,
	int type,
	void* user)
{
	if ( strcmp(id, "maxPlayers") == 0 ) {
		return value_is_integer_1_to_64(value);
	}
	return 1;
}

xuiPropertyGridSetValidate(grid, validate_property, user);
```

When validation rejects a value, TableGrid keeps the active editor open, increments its reject counter, and does not update the property value. Accepted edits call `xuiPropertyGridSetValue`, then the change callback.

## Flags

```c
XUI_PROPERTY_FLAG_READONLY
XUI_PROPERTY_FLAG_DISABLED
XUI_PROPERTY_FLAG_DIRTY
XUI_PROPERTY_FLAG_INVALID
XUI_PROPERTY_FLAG_HIDDEN
```

Readonly disables value editing but keeps the row visible. Disabled also uses disabled visual state. Hidden removes the property from visible rows. Dirty can be explicit through `XUI_PROPERTY_FLAG_DIRTY`; otherwise the control marks a property dirty when `sDefaultValue` is non-empty and differs from the current value.

## Description

Description modes:

```c
xuiPropertyGridSetDescriptionMode(grid, XUI_PROPERTY_GRID_DESCRIPTION_NONE, 0.0f);
xuiPropertyGridSetDescriptionMode(grid, XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP, 0.0f);
xuiPropertyGridSetDescriptionMode(grid, XUI_PROPERTY_GRID_DESCRIPTION_PANEL, 56.0f);
xuiPropertyGridSetDescriptionMode(grid, XUI_PROPERTY_GRID_DESCRIPTION_BOTH, 56.0f);
```

The bottom panel displays the selected property's name and description. It reserves `fDescriptionPanelHeight` from the root content rect and keeps the internal TableGrid above it.

## Public API

```c
xuiPropertyGridGetType
xuiPropertyGridCreate
xuiPropertyGridGetTableGrid
xuiPropertyGridGetTableView
xuiPropertyGridClear
xuiPropertyGridAddCategory
xuiPropertyGridAddProperty
xuiPropertyGridFindCategory
xuiPropertyGridFindProperty
xuiPropertyGridSetCategoryExpanded
xuiPropertyGridGetCategoryExpanded
xuiPropertyGridSetSelected
xuiPropertyGridGetSelected
xuiPropertyGridGetCategoryCount
xuiPropertyGridGetPropertyCount
xuiPropertyGridGetVisibleCount
xuiPropertyGridGetVisibleProperty
xuiPropertyGridSetValue
xuiPropertyGridGetValue
xuiPropertyGridGetBool
xuiPropertyGridSetBool
xuiPropertyGridGetInt
xuiPropertyGridSetInt
xuiPropertyGridGetFloat
xuiPropertyGridSetFloat
xuiPropertyGridGetColor
xuiPropertyGridSetColor
xuiPropertyGridSetPropertyFlags
xuiPropertyGridGetPropertyFlags
xuiPropertyGridSetEditorConfig
xuiPropertyGridSetRenderer
xuiPropertyGridSetAction
xuiPropertyGridIsEditing
xuiPropertyGridBeginEdit
xuiPropertyGridEndEdit
xuiPropertyGridSetFont
xuiPropertyGridGetFont
xuiPropertyGridSetMetrics
xuiPropertyGridGetMetrics
xuiPropertyGridSetDescriptionMode
xuiPropertyGridGetDescriptionMode
xuiPropertyGridSetEditMode
xuiPropertyGridGetEditMode
xuiPropertyGridSetScroll
xuiPropertyGridGetScroll
xuiPropertyGridSetScrollbarMode
xuiPropertyGridGetScrollbarMode
xuiPropertyGridSetSelect
xuiPropertyGridSetValidate
xuiPropertyGridSetChange
xuiPropertyGridSetGlobalAction
xuiPropertyGridSetGlobalRenderer
xuiPropertyGridSetStyle
xuiPropertyGridGetStyle
xuiPropertyGridSetColors
xuiPropertyGridGetSelectCount
xuiPropertyGridGetToggleCount
```

Use `xuiPropertyGridGetTableGrid` and `xuiPropertyGridGetTableView` for lower-level TableGrid/TableView diagnostics or advanced integration.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_property_grid_test.bat
examples\xui_propertygrid\build.bat
build\xui_propertygrid.exe --frames 5
```

The example summary should include `create=1`, `layout=1`, `edit=1`, `validate=1`, `action=1`, `toggle=1`, `scroll=1`, and `render=1`. The run may still print the existing bundled-resource `libpng iCCP` warning.
