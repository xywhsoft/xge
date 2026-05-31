# XUI StatusBar

StatusBar is a cache-first bottom chrome widget for compact application state. It is used for short status text, progress display, fixed spacing, flexible spacing, and clickable state items.

## Goals

- preserve XUI1 StatusBar behavior: text/progress/spacer items, left/center/right sections, fixed and flexible spacers, clickable items, progress clamping, and disabled click suppression
- keep the widget cache-first and styleable
- use the current XUI2 flat light-blue chrome style
- keep pointer capture semantics: press an item, release on the same item to commit
- keep XSON deferred

## Structure

`xuiStatusBarCreate` returns one widget. Items are stored in widget-private data and laid out into cached `tRect` fields.

```text
StatusBar widget
  left section
    Ready text
    progress
    fixed spacer
    flexible spacer
  center section
    Scene text
  right section
    UTF-8 text
    Read only text
```

Each section is laid out independently across the StatusBar width:

- left starts from the left padding
- center is centered in the available width
- right ends at the right padding

This matches the XUI1 behavior and keeps common app chrome simple.

## Items

```c
typedef struct xui_statusbar_item_t {
	const char* sText;
	int iType;
	int iSection;
	uint32_t iState;
	int iValue;
	float fWidth;
	float fFlex;
	float fMin;
	float fMax;
	float fValue;
	xui_rect_t tRect;
	void* pUser;
} xui_statusbar_item_t;
```

Item types:

```c
XUI_STATUSBAR_ITEM_TEXT
XUI_STATUSBAR_ITEM_PROGRESS
XUI_STATUSBAR_ITEM_SPACER
```

Sections:

```c
XUI_STATUSBAR_SECTION_LEFT
XUI_STATUSBAR_SECTION_CENTER
XUI_STATUSBAR_SECTION_RIGHT
```

Item state:

```c
XUI_STATUSBAR_ITEM_ENABLED
XUI_STATUSBAR_ITEM_CLICKABLE
```

Enabled and clickable are explicit for `xuiStatusBarSetItems`. An item with `iState == 0` is disabled. Spacers never participate in hit testing.

Text pointers are not copied; keep strings alive while the StatusBar uses them.

## Layout

`fWidth` is a fixed item width. If `fWidth <= 0`, the StatusBar uses a natural width:

- text uses measured text width plus horizontal item padding
- progress defaults to 92px
- fixed spacer defaults to 12px

For spacer items, `fFlex > 0` turns the spacer into a flexible spacer. Flexible spacers share the extra space left in their section by weight.

Progress items use:

- `fMin`
- `fMax`
- `fValue`

`fValue` is clamped to `[fMin, fMax]` both when items are set and when `xuiStatusBarSetProgress` is called.

## Interaction

Pointer:

- moving over an enabled clickable non-spacer item updates hover
- pressing a clickable item captures the pointer and marks it active
- releasing on the same active item calls the select callback
- releasing outside clears active without committing
- disabled items, non-clickable items, and spacers are ignored by hit testing

Keyboard:

- when the StatusBar has focus, Space/Enter commits the hovered interactive item
- if no item is hovered, Space/Enter commits the first interactive item

The callback receives both the item index and `iValue`, matching the command-style pattern used by Toolbar and MenuBar.

## Metrics And Colors

Defaults use a compact chrome bar: 26px height, subtle top border/highlight, transparent idle items, blue hover/active items, and blue progress fill.

Customize with:

```c
xui_statusbar_metrics_t metrics;
xui_statusbar_colors_t colors;
xuiStatusBarGetMetrics(statusbar, &metrics);
xuiStatusBarGetColors(statusbar, &colors);
xuiStatusBarSetMetrics(statusbar, &metrics);
xuiStatusBarSetColors(statusbar, &colors);
```

## Public API

```c
xuiStatusBarGetType
xuiStatusBarCreate
xuiStatusBarSetSelect
xuiStatusBarSetItems
xuiStatusBarAddText
xuiStatusBarAddProgress
xuiStatusBarAddSpacer
xuiStatusBarAddFlexibleSpacer
xuiStatusBarClear
xuiStatusBarGetItemCount
xuiStatusBarGetItem
xuiStatusBarGetItemRect
xuiStatusBarGetItemAt
xuiStatusBarSetItemEnabled
xuiStatusBarIsItemEnabled
xuiStatusBarSetItemClickable
xuiStatusBarIsItemClickable
xuiStatusBarSetItemText
xuiStatusBarSetItemValue
xuiStatusBarGetItemValue
xuiStatusBarSetItemWidth
xuiStatusBarSetItemFlex
xuiStatusBarSetProgress
xuiStatusBarGetProgress
xuiStatusBarGetHoverIndex
xuiStatusBarSetHoverIndex
xuiStatusBarGetActiveIndex
xuiStatusBarSetFont
xuiStatusBarGetFont
xuiStatusBarSetMetrics
xuiStatusBarGetMetrics
xuiStatusBarSetColors
xuiStatusBarGetColors
xuiStatusBarGetState
xuiStatusBarGetSelectCount
xuiStatusBarGetChangeCount
```

## Constants

```c
XUI_STATUSBAR_ITEM_CAPACITY
XUI_STATUSBAR_ITEM_TEXT
XUI_STATUSBAR_ITEM_PROGRESS
XUI_STATUSBAR_ITEM_SPACER
XUI_STATUSBAR_SECTION_LEFT
XUI_STATUSBAR_SECTION_CENTER
XUI_STATUSBAR_SECTION_RIGHT
XUI_STATUSBAR_ITEM_ENABLED
XUI_STATUSBAR_ITEM_CLICKABLE
```

## Style Properties

```text
font.name
statusbar.background.color
statusbar.border.color
statusbar.highlight.color
statusbar.item.color
statusbar.item.hover_color
statusbar.item.active_color
statusbar.focus.color
statusbar.text.color
statusbar.text.disabled_color
statusbar.progress.track_color
statusbar.progress.fill_color
statusbar.height
statusbar.padding.x
statusbar.padding.y
statusbar.gap
statusbar.item.padding.x
statusbar.item.padding.y
statusbar.progress.height
statusbar.radius
statusbar.border.width
statusbar.top_border.width
```

Color properties affect cache rendering. Metrics affect content measurement, layout, hit testing, and cached item rects.

## Example

```c
xui_widget statusbar;

xuiStatusBarCreate(context, &statusbar, NULL);
xuiStatusBarAddText(statusbar, XUI_STATUSBAR_SECTION_LEFT, "Ready", 78.0f, 1, 10);
xuiStatusBarAddProgress(statusbar, XUI_STATUSBAR_SECTION_LEFT, 0.0f, 100.0f, 64.0f, 112.0f);
xuiStatusBarAddFlexibleSpacer(statusbar, XUI_STATUSBAR_SECTION_LEFT, 1.0f);
xuiStatusBarAddText(statusbar, XUI_STATUSBAR_SECTION_CENTER, "Scene: sample.xge", 150.0f, 1, 20);
xuiStatusBarAddText(statusbar, XUI_STATUSBAR_SECTION_RIGHT, "UTF-8", 62.0f, 1, 30);
xuiStatusBarSetSelect(statusbar, onStatusSelect, user);
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_statusbar_test.bat
examples\xui_statusbar\build.bat
build\xui_statusbar.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `progress=1`, `click=1`, `disabled=1`, and `keyboard=1`.
