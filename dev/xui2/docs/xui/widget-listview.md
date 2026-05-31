# XUI ListView

ListView is a fixed-height row list widget. The XUI2 version keeps the mature XUI1 selection, keyboard, disabled-row, hover, and ensure-visible behavior, but implements scrolling through the shared ScrollFrame.

## Goals

- preserve XUI1 single, multi, and range selection behavior
- keep disabled rows visible but non-selectable
- retain keyboard navigation: Up, Down, PageUp, PageDown, Home, End, Enter, and Space
- reuse ScrollFrame and ScrollModel for clipping, wheel scroll, and compact/full scrollbar visuals
- expose the frame, viewport, model, row rects, hover index, and focus index for tests and advanced integration
- keep XSON deferred

## Structure

```text
ListView root widget
  ScrollFrame
    viewport widget
      cache-rendered list rows
    vertical ScrollBar
```

The horizontal scrollbar policy is hidden by default. The vertical scrollbar policy is automatic. The default mode is compact, matching the newer XUI2 scrollbar design.

Rows are not child widgets. They are rendered in the viewport cache from the item array, selection state, hover state, and scroll offset.

## Items

Simple string items are copied into the ListView:

```c
const char* items[] = {"Scene", "Camera", "Player"};

xui_list_view_desc_t desc;
memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.arrItems = items;
desc.iItemCount = 3;
desc.iSelected = 0;
xuiListViewCreate(context, &list, &desc);
```

Enabled flags are copied into an internal array. Missing enabled entries default to enabled.

```c
int enabled[] = {1, 0, 1};
xuiListViewSetEnabledItems(list, enabled, 3);
```

Disabled rows cannot be selected by API, mouse, or keyboard. If a row becomes disabled while selected, selection moves to the first remaining selected row or clears.

## Selection

Selection modes:

```c
XUI_SELECTION_SINGLE
XUI_SELECTION_MULTI
XUI_SELECTION_RANGE
```

Single selection uses `xuiListViewSetSelected` and `xuiListViewGetSelected`.

Multi and range selection use an internal selection array by default. Applications can provide their own buffer with `xuiListViewSetSelectionBuffer`; ListView writes one integer per row, using non-zero for selected.

Mouse behavior:

- single: click selects one enabled row
- multi: Ctrl-click toggles a row; normal click clears the previous selection and selects one row
- range: Shift-click selects the range from the anchor to the clicked row; Ctrl-click toggles

Keyboard behavior:

- Up/Down move to the previous or next enabled row
- PageUp/PageDown move by the current viewport row count
- Home/End move to the first or last enabled row
- Shift with movement extends a range in range-selection mode
- Enter/Space notify selection for the current row

`xuiListViewSetNotifyRepeatSelect` enables callback notification when the same row is selected again.

## Rendering

Default rendering draws:

- a rounded list border using the focus color while focused
- a clipped viewport background
- rounded selected and hover row fills
- selected row text in white
- disabled row text in muted color
- compact scrollbar visuals inherited from ScrollFrame

Custom row rendering can replace the default row draw by returning non-zero:

```c
static int render_item(
	xui_widget list,
	int index,
	xui_draw_context draw,
	xui_rect_t row,
	int state,
	void* user)
{
	if ( (state & XUI_LIST_ITEM_SELECTED) != 0 ) {
		/* custom paint */
	}
	return 0; /* return non-zero to skip default rendering */
}

xuiListViewSetItemRenderer(list, render_item, user);
```

The row rect passed to the renderer is viewport-local.

## Public API

```c
xuiListViewGetType
xuiListViewCreate
xuiListViewSetSelect
xuiListViewSetItemRenderer
xuiListViewSetItems
xuiListViewSetEnabledItems
xuiListViewGetItemCount
xuiListViewGetItemText
xuiListViewIsItemEnabled
xuiListViewSetFont
xuiListViewGetFont
xuiListViewSetItemHeight
xuiListViewGetItemHeight
xuiListViewSetMetrics
xuiListViewGetMetrics
xuiListViewSetSelected
xuiListViewGetSelected
xuiListViewSetSelectionMode
xuiListViewGetSelectionMode
xuiListViewSetSelectionBuffer
xuiListViewClearSelection
xuiListViewSetItemSelected
xuiListViewIsItemSelected
xuiListViewSetNotifyRepeatSelect
xuiListViewGetNotifyRepeatSelect
xuiListViewSetScroll
xuiListViewGetScroll
xuiListViewEnsureVisible
xuiListViewSetScrollbarMode
xuiListViewGetScrollbarMode
xuiListViewSetColors
xuiListViewSetScrollbarColors
xuiListViewGetFrameWidget
xuiListViewGetViewportWidget
xuiListViewGetModel
xuiListViewGetViewportRect
xuiListViewGetItemRect
xuiListViewGetItemAt
xuiListViewGetHoverIndex
xuiListViewGetFocusIndex
xuiListViewGetSelectCount
xuiListViewGetChangeCount
```

## Style Properties

```text
listview.background.color
listview.border.color
listview.focus.color
listview.row.color
listview.row.hover_color
listview.row.selected_color
listview.text.color
listview.text.disabled_color
listview.item.height
listview.padding
listview.radius
listview.border.width
font.name
```

Scrollbar colors are configured with `xuiListViewSetScrollbarColors`, which forwards to the internal ScrollFrame.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_list_view_test.bat
examples\xui_listview\build.bat
build\xui_listview.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `select=1`, `range=1`, and `scroll=1`.
