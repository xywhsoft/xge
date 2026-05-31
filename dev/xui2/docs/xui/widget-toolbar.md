# XUI Toolbar

Toolbar is a cache-first command strip widget for compact editor and application commands. It preserves the useful XUI1 behavior while using the XUI2 widget, input, style, and cache infrastructure.

## Goals

- support button, toggle, and separator items
- support horizontal and vertical orientation
- preserve group gaps, tooltip metadata, disabled state, checked state, keyboard activation, and overflow reporting
- keep pointer capture semantics: press an item, release on the same item to commit
- keep rendering flat, compact, and styleable for the current XUI2 light-blue design
- keep XSON deferred

## Structure

`xuiToolbarCreate` returns one widget. Items are stored in widget-private data and laid out into cached `tRect` fields whenever size, metrics, items, or style change.

```text
Toolbar widget
  item 0: button
  item 1: toggle
  item 2: separator
  item 3: button
  overflow button
```

The toolbar does not own icon surfaces. If an item has `pIcon`, the caller owns the surface lifetime.

## Items

```c
typedef struct xui_toolbar_item_t {
	const char* sText;
	const char* sTooltip;
	int iType;
	uint32_t iState;
	int iValue;
	int iGroup;
	struct xui_surface_t* pIcon;
	xui_rect_t tIconSrc;
	xui_rect_t tRect;
	void* pUser;
} xui_toolbar_item_t;
```

Item types:

```c
XUI_TOOLBAR_ITEM_BUTTON
XUI_TOOLBAR_ITEM_TOGGLE
XUI_TOOLBAR_ITEM_SEPARATOR
```

Item state:

```c
XUI_TOOLBAR_ITEM_ENABLED
XUI_TOOLBAR_ITEM_CHECKED
```

Enabled is explicit for `xuiToolbarSetItems`: an item with `iState == 0` is disabled unless it is a separator. `xuiToolbarAddItem` creates enabled button/toggle items by default.

Text and tooltip pointers are not copied; keep those strings alive while the Toolbar uses them.

## Layout

Metrics control item width, item height, separator size, padding, group gap, overflow button size, corner radius, border width, and icon layout.

`iGroup` inserts `fGroupGap` when the group id changes between adjacent visible items. This keeps related command groups visually separated without requiring multiple toolbar widgets.

When overflow is enabled, layout reserves an overflow button at the far edge. Items that no longer fit are hidden from hit testing, and the toolbar records:

- `xuiToolbarGetOverflowFirst`
- `xuiToolbarGetOverflowCount`
- `xuiToolbarGetOverflowRect`

The overflow callback receives `first` and `count`, so a higher-level control can open a menu or popup for hidden commands.

## Interaction

Pointer:

- moving over an enabled item updates hover
- pressing an enabled item captures the pointer and marks it active
- releasing on the same active item commits the command
- releasing outside clears active without committing
- pressing and releasing the overflow button calls the overflow callback
- disabled items and separators are ignored by hit testing

Keyboard:

- Left/Right move hover in horizontal toolbars
- Up/Down move hover in vertical toolbars
- Space/Enter commits the hovered item
- if nothing is hovered, Space/Enter commits the first enabled item

Toggle items flip `XUI_TOOLBAR_ITEM_CHECKED` before the select callback is invoked.

## Tooltip

Each item may carry `sTooltip`. The control registers a tooltip resolver with the core tooltip infrastructure and returns the tooltip for the currently hovered interactive item.

Use:

```c
xuiToolbarSetItemTooltip(toolbar, index, "Run command");
xuiToolbarGetHoverTooltip(toolbar);
```

## Metrics And Colors

Defaults use the XUI2 flat light-blue style: compact 26px items, subtle idle background, blue hover/active/checked state, muted disabled text, and single-pixel borders.

Customize with:

```c
xui_toolbar_metrics_t metrics;
xui_toolbar_colors_t colors;
xuiToolbarGetMetrics(toolbar, &metrics);
xuiToolbarGetColors(toolbar, &colors);
xuiToolbarSetMetrics(toolbar, &metrics);
xuiToolbarSetColors(toolbar, &colors);
```

## Public API

```c
xuiToolbarGetType
xuiToolbarCreate
xuiToolbarSetSelect
xuiToolbarSetOverflow
xuiToolbarSetItems
xuiToolbarAddItem
xuiToolbarAddSeparator
xuiToolbarClear
xuiToolbarGetItemCount
xuiToolbarGetItem
xuiToolbarGetItemRect
xuiToolbarGetItemAt
xuiToolbarSetOrientation
xuiToolbarGetOrientation
xuiToolbarSetItemSize
xuiToolbarSetItemGroup
xuiToolbarGetItemGroup
xuiToolbarSetItemTooltip
xuiToolbarGetItemTooltip
xuiToolbarGetHoverTooltip
xuiToolbarSetItemEnabled
xuiToolbarIsItemEnabled
xuiToolbarSetItemChecked
xuiToolbarGetItemChecked
xuiToolbarSetItemIcon
xuiToolbarGetHoverIndex
xuiToolbarSetHoverIndex
xuiToolbarGetActiveIndex
xuiToolbarGetOverflowFirst
xuiToolbarGetOverflowCount
xuiToolbarGetOverflowRect
xuiToolbarSetFont
xuiToolbarGetFont
xuiToolbarSetMetrics
xuiToolbarGetMetrics
xuiToolbarSetColors
xuiToolbarGetColors
xuiToolbarGetState
xuiToolbarGetSelectCount
xuiToolbarGetOverflowSelectCount
xuiToolbarGetChangeCount
```

## Constants

```c
XUI_TOOLBAR_ITEM_CAPACITY
XUI_TOOLBAR_ITEM_BUTTON
XUI_TOOLBAR_ITEM_TOGGLE
XUI_TOOLBAR_ITEM_SEPARATOR
XUI_TOOLBAR_ITEM_ENABLED
XUI_TOOLBAR_ITEM_CHECKED
XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE
```

## Style Properties

```text
font.name
toolbar.background.color
toolbar.border.color
toolbar.item.color
toolbar.item.hover_color
toolbar.item.active_color
toolbar.item.checked_color
toolbar.item.disabled_color
toolbar.focus.color
toolbar.separator.color
toolbar.text.color
toolbar.text.disabled_color
toolbar.icon.color
toolbar.orientation
toolbar.item.width
toolbar.item.height
toolbar.separator.size
toolbar.group_gap
toolbar.padding.x
toolbar.padding.y
toolbar.overflow.size
toolbar.radius
toolbar.border.width
toolbar.icon.size
toolbar.icon.gap
```

Color properties affect cache rendering. Metrics affect layout, hit testing, overflow calculation, and cached item rects.

## Example

```c
xui_toolbar_item_t items[4];

memset(items, 0, sizeof(items));
items[0].sText = "New";
items[0].sTooltip = "Create scene";
items[0].iType = XUI_TOOLBAR_ITEM_BUTTON;
items[0].iState = XUI_TOOLBAR_ITEM_ENABLED;
items[0].iValue = 10;
items[1].sText = "Pin";
items[1].sTooltip = "Toggle pin";
items[1].iType = XUI_TOOLBAR_ITEM_TOGGLE;
items[1].iState = XUI_TOOLBAR_ITEM_ENABLED;
items[1].iValue = 11;
items[2].iType = XUI_TOOLBAR_ITEM_SEPARATOR;
items[3].sText = "Run";
items[3].iType = XUI_TOOLBAR_ITEM_BUTTON;
items[3].iState = XUI_TOOLBAR_ITEM_ENABLED;
items[3].iValue = 12;

xuiToolbarSetItems(toolbar, items, 4);
xuiToolbarSetSelect(toolbar, onToolbarSelect, user);
xuiToolbarSetOverflow(toolbar, 1, 28.0f, onToolbarOverflow, user);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_toolbar_test.bat
examples\xui_toolbar\build.bat
build\xui_toolbar.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `toggle=1`, `disabled=1`, `keyboard=1`, `overflow=1`, and `vertical=1`.
