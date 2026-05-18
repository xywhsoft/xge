# XUI ScrollView

ScrollView is built on a three-layer scrolling design:

## ScrollModel

`ScrollModel` is not a user-facing control. It is the shared scrolling model used by scrollable controls.

Responsibilities:

- outer viewport rectangle
- content viewport rectangle
- content width and height
- scroll offset
- offset clamp
- nested scroll policy
- wheel axis
- scrollbar mode and colors
- pointer drag state used by scroll interactions
- coordinate conversion

The coordinate contract is the most important part of this layer:

- screen coordinate: window/global coordinate from the original event
- viewport coordinate: coordinate relative to the content viewport
- content coordinate: viewport coordinate plus scroll offset

`ScrollModel` keeps two viewport rectangles:

- outer viewport: the widget content box before scrollbars reserve space
- content viewport: the actual visible content area after scrollbar space is removed

Scrollbars belong to the outer viewport. Child layout, clipping, coordinate conversion, max offset, and `ensure visible` all use the content viewport. This means content must stop at the left edge of a vertical scrollbar and at the top edge of a horizontal scrollbar; scrollbar space is never counted as usable content space.

`event.fX` and `event.fY` stay as screen coordinates. Scrollable controls must not rewrite the original event coordinate, because existing widgets compare the event against their current widget rectangles. Controls that need logical content coordinates should call the ScrollModel conversion APIs.

Core APIs:

```c
void xgeXuiScrollModelSetViewport(xge_xui_scroll_model pModel, xge_rect_t tViewport);
void xgeXuiScrollModelSetContentSize(xge_xui_scroll_model pModel, float fWidth, float fHeight);
void xgeXuiScrollModelSetOffset(xge_xui_scroll_model pModel, float fX, float fY);
void xgeXuiScrollModelScreenToViewport(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pViewportX, float* pViewportY);
void xgeXuiScrollModelViewportToContent(xge_xui_scroll_model pModel, float fViewportX, float fViewportY, float* pContentX, float* pContentY);
void xgeXuiScrollModelScreenToContent(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pContentX, float* pContentY);
void xgeXuiScrollModelContentToScreen(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pScreenX, float* pScreenY);
```

When a ScrollModel is bound to a widget, layout synchronizes the outer viewport from the widget content rectangle, resolves the content viewport, and then applies that content viewport to child layout and clipping. This keeps the coordinate model stable after layout changes.

Example:

If the viewport starts at screen `(100, 100)` and the scroll offset is `(100, 100)`, a mouse click at screen `(100, 100)` maps to viewport `(0, 0)` and content `(100, 100)`.

## ScrollView

`ScrollView` is the normal entity-content scroll control.

Use it when a large content surface is shown through a smaller viewport:

- canvas
- image surface
- map
- form with real child widgets
- custom drawn large content

ScrollView owns a `ScrollModel` and applies the visual content offset to child widgets. For ordinary XUI child widgets, pointer events remain screen-based and work against the shifted widget rectangles. For custom rendering or hit testing, use `xgeXuiScrollModelScreenToContent` to get stable logical content coordinates.

When scrollbars are visible, ScrollView reserves their area before laying out children. The reserved area is still part of the ScrollView widget for hit testing and scrollbar interaction, but it is not part of the content viewport.

## VirtualViewBase

`VirtualViewBase` is the base for data views with many rows/items.

Use it when the content is data-driven and only visible items should exist:

- virtual list
- tree view
- table view
- property grid

VirtualViewBase also owns a `ScrollModel`, but its hit testing usually starts from viewport coordinates and then maps to item indexes. It should not be treated as a generic child-widget container.

## Boundary

- ScrollModel: shared scrolling state and coordinate conversion.
- ScrollView: real child/content viewport.
- VirtualViewBase: virtualized data viewport.

The layers must not duplicate independent scroll coordinate systems. If a control scrolls, its authoritative viewport/content mapping must be represented by `ScrollModel`.

## XSON

`scrollView` and `scroll` are aliases for a real-content ScrollView.

Common fields:

- `contentSize`: `[width, height]`, or object values accepted by the vector parser.
- `contentWidth`, `contentHeight`: independent content-size fields.
- `offset`, `scrollOffset`, `contentOffset`: initial `[x, y]` scroll offset.
- `scrollX`, `scrollY`: independent initial offset fields.
- `wheelAxis`: `vertical`, `horizontal`, or `both`.
- `contentDrag`: enables dragging the content surface to pan.
- `scrollbarDrag`: enables dragging scrollbar thumbs.
- `nestedScroll`: `consume` or `passEdge`.
- `background` / `backgroundColor`, `barColor`, `thumbColor`: visual styling.

Children inside a ScrollView are real child widgets positioned in content coordinates. The layout system applies the current scroll offset to their screen rectangles; events remain screen-based and continue to hit the shifted widgets correctly.

## Examples

- `examples/xui_scrollview`: programmatic real-content ScrollView example. It draws a large grid/canvas, displays live viewport/content coordinate conversion, and includes child labels/buttons inside the scrolled content.
- `examples/xui_scrollview_xson`: XSON version with the same content-size, offset, wheel-axis, content-drag, and child-widget coverage.
- `examples/xui_scroll_standard_lab`: focused ScrollModel/ScrollView behavior smoke lab.
- `examples/xui_scroll_view_proc_lab`: lower-level proc and paint behavior lab.
