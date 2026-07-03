# XUI TimeLineView

TimeLineView is a Flash-like timeline editor control for animation and sequence authoring. It owns timeline presentation and interaction: layer list, frame ruler, frame grid, current-frame playhead, sparse key frames, spans, selection, scrolling, and zooming.

The XUI2 version keeps the XUI1 user-facing mechanisms while replacing the old XUI1 control infrastructure with a typed widget, fixed internal model arrays, cache rendering, and a `ScrollFrame` viewport.

## Goals

- preserve the XUI1 four-region timeline layout
- keep current frame separate from selected frames
- support sparse frames and spans without storing every empty cell
- keep layer visible/locked state as direct row controls
- support horizontal and vertical scrolling through `ScrollFrame`
- support Ctrl+wheel frame-width zoom anchored near the mouse
- expose hit testing, callbacks, renderer hooks, and diagnostic counters
- keep business animation playback, undo/redo, clipboard, audio waveform, and data serialization outside this widget

## Structure

```text
corner          frame ruler
layer list      timeline grid
```

Internally:

```text
TimeLineView root widget
  ScrollFrame
    viewport cache renders timeline grid
    compact/full scrollbars
```

The root cache renders the corner, ruler, layer list, focus border, and the playhead in the ruler. The viewport cache renders rows, grid lines, spans, frame markers, selected cells, and the playhead over the grid.

Horizontal scroll affects ruler and grid. Vertical scroll affects layer list and grid. The playhead overlays both ruler and grid.

## Model

The model is intentionally compact:

```c
xui_timeline_layer_t     layers[XUI_TIMELINE_LAYER_CAPACITY]
xui_timeline_frame_t     sparse frames[XUI_TIMELINE_FRAME_CAPACITY]
xui_timeline_span_t      spans[XUI_TIMELINE_SPAN_CAPACITY]
xui_timeline_selection_t selected frames[XUI_TIMELINE_SELECTION_CAPACITY]
```

Default capacities:

```c
XUI_TIMELINE_LAYER_CAPACITY      64
XUI_TIMELINE_FRAME_CAPACITY      512
XUI_TIMELINE_SPAN_CAPACITY       256
XUI_TIMELINE_SELECTION_CAPACITY  1024
```

Empty frames are not stored. `xuiTimeLineViewGetFrame` returns an empty frame record when no sparse record exists.

## Frame And Span Types

Frame types:

```c
XUI_TIMELINE_FRAME_EMPTY
XUI_TIMELINE_FRAME_NORMAL
XUI_TIMELINE_FRAME_KEY
XUI_TIMELINE_FRAME_BLANK_KEY
```

Span types:

```c
XUI_TIMELINE_SPAN_CUSTOM
XUI_TIMELINE_SPAN_MOTION
XUI_TIMELINE_SPAN_SHAPE
XUI_TIMELINE_SPAN_EVENT
XUI_TIMELINE_SPAN_AUDIO
XUI_TIMELINE_SPAN_HOLD
```

Spans are inclusive: `iStartFrame` through `iEndFrame`.

## Creating A Timeline

```c
xui_timeline_view_desc_t desc;
xui_widget timeline;
int layer;
int span;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.pFont = font;
desc.iFrameCount = 180;
desc.fFrameRate = 24.0f;
desc.fLayerHeaderWidth = 168.0f;
desc.fFrameWidth = 12.0f;
desc.fRowHeight = 25.0f;
desc.fRulerHeight = 26.0f;
desc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
xuiTimeLineViewCreate(ctx, &timeline, &desc);

xuiTimeLineViewAddLayer(timeline, "Character pose", &layer);
xuiTimeLineViewSetFrame(timeline, layer, 8, XUI_TIMELINE_FRAME_KEY, NULL);
xuiTimeLineViewSetFrame(timeline, layer, 36, XUI_TIMELINE_FRAME_KEY, NULL);
xuiTimeLineViewAddSpan(timeline, layer, 8, 36, XUI_TIMELINE_SPAN_MOTION, "walk cycle", &span);
xuiTimeLineViewSetCurrentFrame(timeline, 12);
xuiTimeLineViewSelectRange(timeline, layer, 8, layer, 12, 1);
```

## Interaction

Mouse:

- ruler click or playhead drag changes current frame
- grid click changes current frame, active layer, and selection
- Ctrl-click toggles selected frame
- Shift-click selects from the anchor frame
- drag across the grid previews and selects a frame range
- Ctrl-drag adds or removes a dragged frame range against the selection that existed when the drag started
- layer name or row click selects the active layer
- visibility and lock icons toggle the layer state
- context-menu events open the built-in layer or frame menu and store the latest hit

Keyboard:

- Left/Right changes the current frame
- Up/Down changes the active layer
- Home/End jumps to first or last frame
- Shift with frame navigation extends selection from the anchor frame
- Context Menu key opens the frame menu at the active layer/current frame

Zoom:

- Ctrl+wheel changes frame width
- zoom is clamped by `xuiTimeLineViewSetFrameWidthRange`
- zoom keeps the frame near the mouse anchored by adjusting horizontal scroll

## Hit Testing

`xuiTimeLineViewHitTest` takes local widget coordinates.

Hit types:

```c
XUI_TIMELINE_HIT_NONE
XUI_TIMELINE_HIT_CORNER
XUI_TIMELINE_HIT_RULER
XUI_TIMELINE_HIT_PLAYHEAD
XUI_TIMELINE_HIT_LAYER_ROW
XUI_TIMELINE_HIT_LAYER_NAME
XUI_TIMELINE_HIT_LAYER_VISIBLE
XUI_TIMELINE_HIT_LAYER_LOCK
XUI_TIMELINE_HIT_FRAME
XUI_TIMELINE_HIT_SPAN
XUI_TIMELINE_HIT_SELECTION
XUI_TIMELINE_HIT_HSCROLLBAR
XUI_TIMELINE_HIT_VSCROLLBAR
```

The returned `xui_timeline_hit_t` contains layer index, layer id, frame index, span id, and the local hit rect where applicable.

## Context Menus

TimeLineView owns two built-in `xuiMenu` instances:

- layer menu for layer row/name/visible/lock hits
- frame menu for frame/selection/span hits

The layer menu follows the XUI1 command set:

```text
Rename
Visible
Locked
Add Layer
Delete Layer
Move Up
Move Down
```

The frame menu follows the XUI1 timeline command set:

```text
Insert Frame
Insert Keyframe
Insert Blank Keyframe
Clear Keyframe
Create Span / Create Span From Selection
Clear Span
```

`xuiTimeLineViewSetContextMenu` is still the integration hook. The opening callback can return `0` to veto the menu. After a default menu command runs, the command callback is invoked with the stored hit. `xuiTimeLineViewRunContextCommand` executes the same default command path, so external menus and the built-in menu stay consistent.

Layer rename currently dispatches `XUI_TIMELINE_MENU_LAYER_RENAME` only. XUI2 does not yet own an inline layer rename editor, so hosts can handle that command and call `xuiTimeLineViewSetLayerName`.

## Callbacks

Changing callbacks return `1` to allow the change and `0` to veto it.

```c
xuiTimeLineViewSetCurrentFrameCallbacks
xuiTimeLineViewSetLayerCallbacks
xuiTimeLineViewSetFrameCallbacks
xuiTimeLineViewSetSpanCallbacks
xuiTimeLineViewSetLayerSelected
xuiTimeLineViewSetContextMenu
xuiTimeLineViewRunContextCommand
xuiTimeLineViewSetFrameClick
xuiTimeLineViewSetSelectionChange
```

Renderer hooks return non-zero to fully handle painting for that item, or `0` to let the default renderer continue:

```c
xuiTimeLineViewSetRenderers(timeline, onLayer, onRuler, onFrame, onSpan, user);
```

## Scroll And Visibility

```c
xuiTimeLineViewSetOffset(timeline, x, y);
xuiTimeLineViewGetOffset(timeline, &x, &y);
xuiTimeLineViewEnsureFrameVisible(timeline, layer, frame);
xuiTimeLineViewGetFrameWidget(timeline);
xuiTimeLineViewGetViewportWidget(timeline);
xuiTimeLineViewGetModel(timeline);
```

The internal `ScrollFrame` uses automatic scrollbars. The default mode is compact; full mode can be enabled through `xuiTimeLineViewSetScrollbarMode`.

## Public API

```c
xuiTimeLineViewGetType
xuiTimeLineViewCreate
xuiTimeLineViewGetFrameWidget
xuiTimeLineViewGetViewportWidget
xuiTimeLineViewGetModel
xuiTimeLineViewClear
xuiTimeLineViewSetFont
xuiTimeLineViewGetFont
xuiTimeLineViewSetFrameCount
xuiTimeLineViewGetFrameCount
xuiTimeLineViewSetFrameRate
xuiTimeLineViewGetFrameRate
xuiTimeLineViewSetCurrentFrame
xuiTimeLineViewGetCurrentFrame
xuiTimeLineViewSetMetrics
xuiTimeLineViewGetMetrics
xuiTimeLineViewSetFrameWidthRange
xuiTimeLineViewSetFrameWidth
xuiTimeLineViewGetFrameWidth
xuiTimeLineViewSetFeatureFlags
xuiTimeLineViewSetScrollbarMode
xuiTimeLineViewGetScrollbarMode
xuiTimeLineViewAddLayer
xuiTimeLineViewRemoveLayer
xuiTimeLineViewMoveLayer
xuiTimeLineViewGetLayerCount
xuiTimeLineViewGetLayer
xuiTimeLineViewSetLayerName
xuiTimeLineViewGetLayerName
xuiTimeLineViewSetLayerVisible
xuiTimeLineViewGetLayerVisible
xuiTimeLineViewSetLayerLocked
xuiTimeLineViewGetLayerLocked
xuiTimeLineViewSetLayerHeight
xuiTimeLineViewGetLayerHeight
xuiTimeLineViewSetLayerColor
xuiTimeLineViewGetLayerColor
xuiTimeLineViewSetLayerUserData
xuiTimeLineViewGetLayerUserData
xuiTimeLineViewSetFrame
xuiTimeLineViewGetFrame
xuiTimeLineViewClearFrame
xuiTimeLineViewSetFrameUserData
xuiTimeLineViewGetFrameUserData
xuiTimeLineViewAddSpan
xuiTimeLineViewRemoveSpan
xuiTimeLineViewSetSpan
xuiTimeLineViewGetSpan
xuiTimeLineViewSetSpanColor
xuiTimeLineViewSetSpanCustomType
xuiTimeLineViewSetSpanUserData
xuiTimeLineViewGetSpanUserData
xuiTimeLineViewClearSelection
xuiTimeLineViewSelectFrame
xuiTimeLineViewSelectRange
xuiTimeLineViewIsFrameSelected
xuiTimeLineViewGetSelectionCount
xuiTimeLineViewGetSelection
xuiTimeLineViewSetCurrentFrameCallbacks
xuiTimeLineViewSetLayerCallbacks
xuiTimeLineViewSetFrameCallbacks
xuiTimeLineViewSetSpanCallbacks
xuiTimeLineViewSetLayerSelected
xuiTimeLineViewSetContextMenu
xuiTimeLineViewRunContextCommand
xuiTimeLineViewSetFrameClick
xuiTimeLineViewSetSelectionChange
xuiTimeLineViewSetRenderers
xuiTimeLineViewSetColors
xuiTimeLineViewGetColors
xuiTimeLineViewEnsureFrameVisible
xuiTimeLineViewHitTest
xuiTimeLineViewSetOffset
xuiTimeLineViewGetOffset
xuiTimeLineViewGetChangeCount
xuiTimeLineViewGetCurrentFrameChangeCount
xuiTimeLineViewGetLayerChangeCount
xuiTimeLineViewGetFrameChangeCount
xuiTimeLineViewGetSpanChangeCount
xuiTimeLineViewGetSelectionChangeCount
xuiTimeLineViewGetClickCount
```

## Current Boundaries

The XUI2 migration includes the timeline body, model, selection, scrolling, zoom, hit testing, built-in layer/frame context menus, callbacks, render hooks, tests, and an example.

Not included in this slice:

- inline layer rename editor
- undo/redo and clipboard commands
- playback engine
- audio waveform painting
- XSON persistence

Applications can still customize or replace menu behavior by using `xuiTimeLineViewSetContextMenu`, `xuiTimeLineViewHitTest`, and `xuiTimeLineViewRunContextCommand`.

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_timeline_view_test.bat
examples\xui_timelineview\build.bat
build\xui_timelineview.exe --frames 5
```

The example prints a final summary with `create=1 layout=1 interaction=1 scroll=1 paint=1` when the control passes its automated smoke path.
