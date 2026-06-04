# XUI Cascader

Cascader is a single-select hierarchical picker. It uses a compact owner control plus an XUI Popup that renders adjacent columns, matching the common Cascader interaction model while staying inside the XUI2 widget/popup stack.

## Goals

- preserve the XUI1/XUI2 popup behavior model: focus, outside close, Escape close, and owner state sync
- support flat item data with parent indices, disabled rows, leaf markers, and user payloads
- support full-path display and last-level-only display
- support click-to-expand and hover-to-expand interaction
- expose stable geometry getters for tests and future integration
- keep lazy loading, search, and multi-select out of the first slice

## Structure

`xuiCascaderCreate` returns the owner widget. The control creates an internal Popup and a cache-rendered panel widget.

```text
Cascader owner widget
  internal Popup
    Cascader panel widget
      column 0: root items
      column 1: active root children
      column N: active path children
```

Use `xuiCascaderGetPopupWidget` and `xuiCascaderGetPanelWidget` for tests, diagnostics, or advanced integration only. Normal application code should use the Cascader open/close and selection APIs.

## Items

The item list is a flat array. `iParent` is the parent item index, or `-1` for a root item.

```c
xui_cascader_item_t items[] = {
	{"Guide", 1, -1, 0u, NULL},
	{"Navigation", 12, 0, 0u, NULL},
	{"Side nav", 121, 1, XUI_CASCADER_ITEM_LEAF, NULL},
	{"Top nav", 122, 1, XUI_CASCADER_ITEM_LEAF, NULL}
};
```

Fields:

- `sText`: displayed text
- `iValue`: public value used by selected paths and callbacks
- `iParent`: parent item index, or `-1`
- `iFlags`: `XUI_CASCADER_ITEM_DISABLED` and `XUI_CASCADER_ITEM_LEAF`
- `pUser`: application payload

Text pointers are not copied. The caller must keep strings alive while the Cascader uses them.

## Selection

The selected path is value-based:

```c
int path[] = {1, 12, 121};
xuiCascaderSetSelectedPath(cascader, path, 3);
```

`xuiCascaderSetSelectedPath` updates selection without notifying the callback. User commits through mouse or keyboard notify `xui_cascader_change_proc` only after a valid item is committed.

By default, only leaf rows commit. `xuiCascaderSetSelectAnyLevel(widget, 1)` allows committing parent rows too.

Display modes:

- `xuiCascaderSetShowAllLevels(widget, 1)`: `Guide / Navigation / Side nav`
- `xuiCascaderSetShowAllLevels(widget, 0)`: `Side nav`

The default empty-selection placeholder is `Select`. Use `xuiCascaderSetPlaceholder` or `xui_cascader_desc_t.sPlaceholder` to override it.

## Popup Behavior

Open paths:

- left mouse down on the owner toggles the popup
- Enter and Space toggle from keyboard focus
- Down opens the popup

Close paths:

- selecting a valid item commits and closes
- Escape closes through the panel
- outside press closes through Popup
- disabling, hiding, or explicitly closing the owner closes the popup

Placement modes:

- `XUI_CASCADER_POPUP_AUTO`
- `XUI_CASCADER_POPUP_BOTTOM`
- `XUI_CASCADER_POPUP_TOP`

The popup width is `column count * column width`. The panel draws compact per-column scroll thumbs when a column has more rows than fit in the popup height.

## Keyboard

When the panel has focus:

- Up/Down moves row hover inside the current column
- Right expands the hovered item if it has children
- Left moves hover back to the parent column
- Enter/Space commits the hovered item if allowed
- Escape closes the popup

## Public API

```c
xuiCascaderGetType
xuiCascaderCreate
xuiCascaderSetChange
xuiCascaderSetItems
xuiCascaderGetItemCount
xuiCascaderGetItem
xuiCascaderSetSelectedPath
xuiCascaderGetSelectedPath
xuiCascaderGetSelectedDepth
xuiCascaderGetSelectedLeaf
xuiCascaderGetSelectedText
xuiCascaderClear
xuiCascaderOpen
xuiCascaderClose
xuiCascaderToggle
xuiCascaderIsOpen
xuiCascaderSetShowAllLevels
xuiCascaderGetShowAllLevels
xuiCascaderSetClearable
xuiCascaderGetClearable
xuiCascaderSetSelectAnyLevel
xuiCascaderGetSelectAnyLevel
xuiCascaderSetExpandTrigger
xuiCascaderGetExpandTrigger
xuiCascaderSetPopupSize
xuiCascaderGetPopupSize
xuiCascaderSetPopupPlacement
xuiCascaderGetPopupPlacement
xuiCascaderSetMetrics
xuiCascaderGetMetrics
xuiCascaderSetColors
xuiCascaderSetBorderColors
xuiCascaderSetArrowColors
xuiCascaderSetPopupColors
xuiCascaderSetFont
xuiCascaderGetFont
xuiCascaderSetPlaceholder
xuiCascaderGetPlaceholder
xuiCascaderSetSeparator
xuiCascaderGetSeparator
xuiCascaderGetPopupWidget
xuiCascaderGetPanelWidget
xuiCascaderGetColumnCount
xuiCascaderGetItemRect
xuiCascaderGetButtonRect
xuiCascaderGetClearRect
xuiCascaderGetTextRect
xuiCascaderGetState
xuiCascaderGetChangeCount
```

## Style Properties

```text
cascader.text.color
cascader.text.placeholder_color
cascader.text.disabled_color
cascader.background.color
cascader.background.hover_color
cascader.background.open_color
cascader.background.disabled_color
cascader.border.color
cascader.border.hover_color
cascader.border.focus_color
cascader.arrow.color
cascader.arrow.disabled_color
cascader.radius
cascader.border.width
font.name
```

Popup row colors are configured through `xuiCascaderSetPopupColors`.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_cascader_test.bat
examples\xui_cascader\build.bat
build\xui_cascader.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `open=1`, `select=1`, `clear=1`, `key=1`, and `placement=1`.
