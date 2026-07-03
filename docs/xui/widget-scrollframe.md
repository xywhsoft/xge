# XUI ScrollFrame

ScrollFrame is the reusable viewport shell for scrollable widgets. It owns a ScrollModel, a clipped viewport widget, horizontal and vertical ScrollBar widgets, and the optional corner block.

## Goals

- preserve XUI1 scroll-frame behavior: automatic scrollbar policy, full/compact scrollbar modes, wheel scrolling, content drag, corner grip, and model/bar synchronization
- keep business content outside the frame implementation; content belongs to ScrollView or another higher-level owner
- prevent content drawing and hit testing from leaking into scrollbar regions by clipping at the viewport widget
- avoid chained scrollbar creation when a size-content parent can expand by the scrollbar reserve
- keep XSON deferred

## Scrollbar Policy

ScrollFrame supports three policies per axis:

```c
XUI_SCROLLBAR_POLICY_AUTO
XUI_SCROLLBAR_POLICY_ALWAYS
XUI_SCROLLBAR_POLICY_HIDDEN
```

`AUTO` compares content size against the computed viewport size. `ALWAYS` reserves the scrollbar area even when content fits. `HIDDEN` disables the visual scrollbar for that axis, while the model still clamps offsets.

## Automatic Layout

The frame uses a fixed 2x2 arrangement:

```text
+----------------------+----+
| viewport             | V  |
|                      |    |
+----------------------+----+
| H                    | C  |
+----------------------+----+
```

The need for horizontal and vertical bars is solved iteratively because one scrollbar can reduce the opposite viewport dimension.

To avoid the XUI1 "chained scrollbar" problem:

- during measure, the preferred outer size includes the needed scrollbar reserve, so size-content parents can grow instead of shrinking the opposite axis
- during arrange into a fixed rect, the frame reserves the scrollbar area and only shows the second axis when content still overflows after reservation
- when a parent imposes a hard maximum, showing both scrollbars is allowed

## Modes

ScrollFrame reuses the ScrollBar widget:

```c
XUI_SCROLLBAR_MODE_FULL
XUI_SCROLLBAR_MODE_COMPACT
```

Full mode reserves the configured scrollbar size and allows ScrollBar button auto mode. Compact mode reserves a thin strip capped at 8px and disables buttons.

## Input

- mouse wheel inside the viewport scrolls the configured axis
- `XUI_WHEEL_AXIS_VERTICAL` maps wheel Y to vertical offset
- `XUI_WHEEL_AXIS_HORIZONTAL` maps wheel Y to horizontal offset when wheel X is absent
- `XUI_WHEEL_AXIS_BOTH` uses wheel X and wheel Y independently
- optional content drag captures the pointer and scrolls opposite to pointer movement
- scrollbars keep their own pointer, keyboard, wheel, and focus behavior

ScrollFrame only stops wheel bubbling when the offset actually changes, so outer scroll containers can still respond at an edge.

## Public API

```c
xuiScrollFrameGetType
xuiScrollFrameCreate
xuiScrollFrameSetChange
xuiScrollFrameGetModel
xuiScrollFrameGetViewportWidget
xuiScrollFrameGetHScrollBarWidget
xuiScrollFrameGetVScrollBarWidget
xuiScrollFrameGetCornerWidget
xuiScrollFrameLayout
xuiScrollFrameSetContentSize
xuiScrollFrameGetContentSize
xuiScrollFrameSetOffset
xuiScrollFrameScrollBy
xuiScrollFrameGetOffset
xuiScrollFrameEnsureRectVisible
xuiScrollFrameSetScrollbarPolicy
xuiScrollFrameGetScrollbarPolicy
xuiScrollFrameSetScrollbarMode
xuiScrollFrameGetScrollbarMode
xuiScrollFrameSetWheelAxis
xuiScrollFrameGetWheelAxis
xuiScrollFrameSetWheelStep
xuiScrollFrameGetWheelStep
xuiScrollFrameSetContentDragEnabled
xuiScrollFrameIsContentDragEnabled
xuiScrollFrameSetCornerMode
xuiScrollFrameGetCornerMode
xuiScrollFrameSetMetrics
xuiScrollFrameSetColors
xuiScrollFrameSetButtonColors
xuiScrollFrameSetCornerColors
xuiScrollFrameGetViewportRect
xuiScrollFrameGetHScrollBarRect
xuiScrollFrameGetVScrollBarRect
xuiScrollFrameGetCornerRect
xuiScrollFrameIsHScrollBarVisible
xuiScrollFrameIsVScrollBarVisible
xuiScrollFrameIsCornerVisible
xuiScrollFrameGetChangeCount
```

## Ownership

Use `xuiScrollFrameGetViewportWidget` when building a higher-level control. Add scrollable content under that viewport, not under the frame itself.

ScrollFrame exposes child scrollbar widgets for advanced integration, but normal code should prefer ScrollFrame APIs so model and bar state stay synchronized.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scroll_frame_test.bat
```

The test covers auto/full layout, viewport reservation, corner visibility, model/bar sync, wheel scrolling, content drag, hidden policy, and always policy.
