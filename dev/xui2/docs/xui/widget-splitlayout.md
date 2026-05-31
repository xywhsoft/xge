# XUI SplitLayout

SplitLayout is a cache-first layout widget for resizable workbench panes. It preserves the XUI1 design while moving the implementation to XUI2 widgets, events, cache rendering, and style properties.

## Goals

- keep the XUI1 pane model: fixed panes, grow panes, weights, min/max sizes, and proportional fallback when space is oversubscribed
- keep dividers internal to the control rather than exposing them as standalone application widgets
- preserve the three divider rectangles: layout size, visual size, and hit size
- support shadow drag and live drag
- keep pane widgets accessible so callers can attach normal XUI content
- keep XSON deferred

## Structure

`xuiSplitLayoutCreate` returns the SplitLayout root widget. Internally it owns pane widgets and divider widgets:

```text
SplitLayout root widget
  pane 0
    user children
  pane 1
    user children
  ...
  divider 0
  divider 1
  ...
```

Use `xuiSplitLayoutGetPaneWidget` or `xuiSplitLayoutAddPaneChild` to attach application content. Dividers are internal and are only exposed through geometry and state query APIs.

## Orientation

`XUI_ORIENTATION_VERTICAL` means vertical dividers and left-to-right panes.

`XUI_ORIENTATION_HORIZONTAL` means horizontal dividers and top-to-bottom panes.

## Pane Layout

Each pane can be `XUI_SPLIT_PANE_FIXED` or `XUI_SPLIT_PANE_GROW`.

Fixed panes reserve their fixed pixel size, clamped by min/max. Grow panes divide the remaining space by weight, then clamp by min/max. If fixed and minimum sizes exceed the available space, SplitLayout proportionally degrades all panes so no pane becomes negative and no chained scrollbar effect is introduced by impossible size requirements.

The default pane settings are:

```text
mode: grow
weight: 1
fixed size: 160
min size: 64
max size: unlimited
```

## Dividers

Each divider has three rectangles:

- layout rect: the reserved gap between panes
- visual rect: the painted divider strip
- hit rect: the wider pointer target used for hover and drag

This keeps the default divider visually slim while still easy to grab.

Default metrics:

```text
layout size: 8
visual size: 4
hit size: 12
```

## Drag Behavior

Shadow drag (`xuiSplitLayoutSetShadowDrag(widget, 1)`) shows a preview line while the pointer is captured, then commits on pointer up.

Live drag (`xuiSplitLayoutSetShadowDrag(widget, 0)`) commits while the pointer moves.

Dragging a divider updates only the adjacent panes:

- fixed + grow: the fixed pane changes size and the grow pane receives the remaining space
- grow + fixed: the fixed pane changes size and the grow pane receives the remaining space
- fixed + fixed: both fixed sizes change while preserving the pair total
- grow + grow: the pair total weight is preserved and redistributed by the new sizes

Dragging is clamped by the adjacent panes' min/max constraints. Escape or capture loss cancels an active drag and hides the shadow line.

## Public API

```c
xuiSplitLayoutGetType
xuiSplitLayoutCreate
xuiSplitLayoutSetChange
xuiSplitLayoutSetOrientation
xuiSplitLayoutGetOrientation
xuiSplitLayoutSetPaneCount
xuiSplitLayoutGetPaneCount
xuiSplitLayoutGetPaneWidget
xuiSplitLayoutAddPaneChild
xuiSplitLayoutSetPaneWeight
xuiSplitLayoutGetPaneWeight
xuiSplitLayoutSetPaneMode
xuiSplitLayoutGetPaneMode
xuiSplitLayoutSetPaneFixedSize
xuiSplitLayoutGetPaneFixedSize
xuiSplitLayoutSetPaneMinSize
xuiSplitLayoutGetPaneMinSize
xuiSplitLayoutSetPaneMaxSize
xuiSplitLayoutGetPaneMaxSize
xuiSplitLayoutGetPaneSize
xuiSplitLayoutSetDividerMetrics
xuiSplitLayoutGetDividerMetrics
xuiSplitLayoutSetDividerSize
xuiSplitLayoutSetDividerVisualSize
xuiSplitLayoutSetDividerHitSize
xuiSplitLayoutSetShadowDrag
xuiSplitLayoutGetShadowDrag
xuiSplitLayoutSetColors
xuiSplitLayoutGetColors
xuiSplitLayoutGetPaneRect
xuiSplitLayoutGetDividerLayoutRect
xuiSplitLayoutGetDividerVisualRect
xuiSplitLayoutGetDividerHitRect
xuiSplitLayoutGetShadowRect
xuiSplitLayoutGetHoverDivider
xuiSplitLayoutGetActiveDivider
xuiSplitLayoutGetChangeCount
```

## Constants

```c
XUI_SPLIT_LAYOUT_MAX_PANES
XUI_SPLIT_PANE_GROW
XUI_SPLIT_PANE_FIXED
XUI_SPLIT_LAYOUT_STATE_DRAGGING
```

The widget also uses shared orientation constants:

```c
XUI_ORIENTATION_HORIZONTAL
XUI_ORIENTATION_VERTICAL
```

## Style Properties

```text
splitlayout.divider.color
splitlayout.divider.hover_color
splitlayout.divider.active_color
splitlayout.shadow.color
splitlayout.divider.size
splitlayout.divider.visual_size
splitlayout.divider.hit_size
```

Color properties affect divider and shadow rendering. Size properties participate in layout and invalidate pane geometry.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_split_layout_test.bat
examples\xui_split_layout\build.bat
build\xui_split_layout.exe --frames 3
```

The example summary should include `create=1`, `layout=1`, `divider=1`, `shadow=1`, and `live=1`.
