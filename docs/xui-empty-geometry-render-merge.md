# XUI Empty Geometry Render Merge Note

## Scope

- Source file: `src/xui_proxy_xge.c`
- Change type: rendering contract correction

## Problem

XUI layout intentionally permits a child content rectangle to collapse to zero
width or height when a dock, split, row, or column has insufficient space.
The render proxy previously treated those empty rectangles as invalid arguments.
That error propagated through cache preparation and `xuiRender()`, so a normal
layout overflow could stop an application's frame loop.

## Required behavior

An empty destination rectangle with exactly zero width or height is a
clipped/no-op draw operation. It must return `XUI_OK` after the drawing context
and resource handles have been validated. Negative geometry remains invalid.
Invalid handles, invalid fonts, invalid paths, and backend failures must still be
reported as errors.

## Changes

The XGE proxy now returns success without issuing a draw for empty rectangles in:

- local clear-rectangle operations
- surface draw and draw-to-target operations
- target rectangle fill and stroke operations
- target text operations
- draw-context rectangle fill and stroke operations
- draw-context text operations
- zero-radius circles, zero-size points, and zero-width lines/strokes
- SVG paths whose destination rectangle has been clipped to empty

This matches browser layout behavior: constrained content may be clipped or not
painted, but it cannot turn a layout shortage into a renderer failure.

## Verification

The final source was rebuilt with `build_dll.bat` and passed the layout/render
regression suite:

```
test_xui\build_proxy_xge_test.bat
test_xui\build_layout_test.bat
test_xui\build_render_schedule_test.bat
test_xui\build_dock_panel_test.bat
test_xui\build_split_layout_test.bat
```

## Merge guidance

When merging this change into a branch that changed the proxy implementation,
preserve the ordering below in each affected function:

1. validate proxy/draw context/resource handles;
2. return success for an empty destination rectangle;
3. validate draw-specific parameters such as stroke width;
4. submit the actual draw.

Do not restore the old combined validation expression that reports empty geometry
as `XUI_ERROR_INVALID_ARGUMENT`.
