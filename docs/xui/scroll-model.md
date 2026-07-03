# XUI ScrollModel

ScrollModel is the pure scrolling state used by ScrollFrame and ScrollView. It has no widget tree, drawing, input handling, or child ownership.

## Goals

- preserve XUI1 behavior for viewport size, content size, clamped offsets, coordinate transforms, and ensure-rect-visible
- keep scroll math testable without a rendering proxy
- make ScrollFrame and ScrollView share one offset/clamp contract
- keep XSON deferred

## Behavior

The model stores:

- viewport rect in screen/world coordinates
- content width and height
- current scroll offset

Offsets are always clamped to:

```text
0 <= offsetX <= max(contentWidth - viewportWidth, 0)
0 <= offsetY <= max(contentHeight - viewportHeight, 0)
```

Changing viewport size or content size reclamps the current offset.

Coordinate helpers map between screen, viewport, and content spaces:

- screen to viewport subtracts the viewport world origin
- viewport to content adds the current scroll offset
- content to viewport subtracts the current scroll offset
- content to screen adds the viewport world origin after content-to-viewport conversion

## Public API

```c
xuiScrollModelInit
xuiScrollModelSetViewport
xuiScrollModelGetViewport
xuiScrollModelSetContentSize
xuiScrollModelGetContentSize
xuiScrollModelSetOffset
xuiScrollModelScrollBy
xuiScrollModelGetOffset
xuiScrollModelGetMaxOffset
xuiScrollModelEnsureRectVisible
xuiScrollModelScreenToViewport
xuiScrollModelViewportToContent
xuiScrollModelScreenToContent
xuiScrollModelContentToViewport
xuiScrollModelContentToScreen
```

All mutating functions return `XUI_OK` or a negative XUI error code. To know whether a mutation changed the offset, compare `xuiScrollModelGetOffset` before and after the call.

## Usage

```c
xui_scroll_model_t model;
xuiScrollModelInit(&model);
xuiScrollModelSetViewport(&model, (xui_rect_t){20.0f, 20.0f, 160.0f, 120.0f});
xuiScrollModelSetContentSize(&model, 480.0f, 320.0f);
xuiScrollModelSetOffset(&model, 64.0f, 32.0f);
xuiScrollModelEnsureRectVisible(&model, (xui_rect_t){300.0f, 180.0f, 40.0f, 30.0f});
```

## Verification

```bat
cd /d D:\git\xge\dev\xui2
test_xui\build_scroll_model_test.bat
```

The test covers initialization, offset clamp, max offset, ensure-rect-visible, and coordinate conversion.
