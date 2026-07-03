# XUI ScrollBar

ScrollBar is a cache-first value control for viewport scrolling, list panes, and compact app chrome.

## Goals

- keep XUI1 behavior: range/page/value, full mode buttons, compact mode, pointer drag, track paging, wheel, keyboard navigation, change callback
- render through XUI2 widget cache and proxy shape APIs
- provide a cleaner light flat default style without requiring atlas assets
- expose geometry queries for scroll frame/list integrations
- keep XSON deferred

## Modes

Full mode:

- draws decrease/increase buttons at both ends
- draws the track between buttons
- supports button click for small-step movement
- supports track click for large-step/page movement

Compact mode:

- hides buttons
- draws a centered thin track and thumb
- is intended for embedded list views, side panels, and low-emphasis scroll areas

Both modes support horizontal and vertical orientation.

## Behavior

Default range is `0..1`, value starts at the minimum, and page defaults to `0.2`.

`xuiScrollBarSetRange` normalizes reversed ranges and clamps negative page size to `0`. `xuiScrollBarSetValue` clamps values to the current range and does not notify the change callback. User interaction does notify.

Input support:

- left-click thumb: capture and drag
- left-click full-mode buttons: small step
- left-click track: large step/page
- mouse wheel: large step/page, positive wheel moves toward the minimum
- keyboard: Up/Left, Down/Right, PageUp, PageDown, Home, End, Escape

## Public API

```c
xuiScrollBarGetType
xuiScrollBarCreate
xuiScrollBarSetChange
xuiScrollBarSetRange
xuiScrollBarGetRange
xuiScrollBarSetPage
xuiScrollBarGetPage
xuiScrollBarSetValue
xuiScrollBarGetValue
xuiScrollBarSetSteps
xuiScrollBarGetSteps
xuiScrollBarSetOrientation
xuiScrollBarGetOrientation
xuiScrollBarSetMode
xuiScrollBarGetMode
xuiScrollBarSetButtonMode
xuiScrollBarGetButtonMode
xuiScrollBarSetMetrics
xuiScrollBarGetMetrics
xuiScrollBarSetColors
xuiScrollBarSetButtonColors
xuiScrollBarGetTrackRect
xuiScrollBarGetThumbRect
xuiScrollBarGetDecreaseRect
xuiScrollBarGetIncreaseRect
xuiScrollBarGetHoverPart
xuiScrollBarGetActivePart
xuiScrollBarGetState
xuiScrollBarGetChangeCount
```

## Constants

```c
XUI_SCROLLBAR_MODE_FULL
XUI_SCROLLBAR_MODE_COMPACT
XUI_SCROLLBAR_BUTTONS_AUTO
XUI_SCROLLBAR_BUTTONS_ON
XUI_SCROLLBAR_BUTTONS_OFF
XUI_SCROLLBAR_PART_NONE
XUI_SCROLLBAR_PART_TRACK
XUI_SCROLLBAR_PART_THUMB
XUI_SCROLLBAR_PART_DECREASE
XUI_SCROLLBAR_PART_INCREASE
```

The control also uses shared orientation constants:

```c
XUI_ORIENTATION_HORIZONTAL
XUI_ORIENTATION_VERTICAL
```

## Style Properties

```text
scrollbar.track.color
scrollbar.thumb.color
scrollbar.thumb.hover_color
scrollbar.thumb.active_color
scrollbar.focus.color
scrollbar.disabled.color
scrollbar.button.color
scrollbar.button.icon_color
scrollbar.thickness
scrollbar.min_thumb_size
scrollbar.thumb_radius
scrollbar.button_size
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scrollbar_test.bat
examples\xui_scrollbar\build.bat
build\xui_scrollbar.exe --frames 360
```

The example summary should include `create=1`, `layout=1`, `full=1`, `compact=1`, and `input=1`.
