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

An empty destination rectangle with zero or negative width or height is a
clipped/no-op draw operation. It must return `XUI_OK` after the drawing context
and resource handles have been validated. Invalid handles, invalid fonts,
invalid paths, and backend failures remain errors.

Negative geometry can occur transiently while a parent layout is being
constrained. It is not useful to submit it to the GPU, and it must not abort the
rest of the UI frame.

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

## Error isolation

Applications can install a context-level diagnostic callback:

```c
static void onXuiError(xui_context context, const xui_error_info_t* error, void* user)
{
	(void)context;
	(void)user;
	fprintf(stderr, "XUI %s failed: %d (%s)\n",
		error->sOperation, error->iCode, error->sMessage);
}

xuiSetErrorCallback(context, onXuiError, NULL);
```

The callback receives the stage, error code, recoverability flag, widget, world
rectangle, operation, and message. It is synchronous and intended for logging;
it must not mutate or destroy the context while a render is in progress.

Widget cache and render-node failures are recoverable. XUI reports the error,
discards a partially rendered cache, skips the failed widget, and continues with
its siblings and overlays. `xuiRender()` still returns `XUI_OK` for these cases.
`xui_render_stats_t.iRecoveredErrors` records the number recovered during the
current render generation.

The same isolation rule applies to the context pumps. `xuiUpdate()` reports a
failed widget update or tooltip update and continues the remaining update tree.
`xuiDispatchPendingEvents()` reports a failed event handler and continues with
the next queued event. A direct `xuiDispatchEvent()` call still returns the
handler error to its caller when explicit per-event control is required.

Widget layout callbacks are also isolated. Failed measure callbacks use zero
content size, failed prepare callbacks use the current properties, and failed
custom arrange or complete callbacks preserve the results already computed.
When layout reaches its pass limit, XUI reports `XUI_ERROR_LAYOUT_UNSTABLE` and
uses the last completed pass instead of aborting rendering.

Errors that invalidate the whole frame contract remain fatal and are returned
after being reported. These include an invalid target surface, a failed target
description query, memory exhaustion in the layout/render-tree infrastructure,
and render-tree construction failures. XUI cannot safely submit a new frame when
those prerequisites are unavailable.

Custom controls and host integrations can send structured diagnostics through
`xuiReportError()` using the same callback path. Recursive reports from inside
the callback are ignored to prevent callback recursion.

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
2. return success for an empty or negative destination rectangle;
3. treat non-positive drawing sizes such as stroke width or radius as no-op;
4. submit the actual draw.

Do not restore the old combined validation expression that reports non-positive
geometry as `XUI_ERROR_INVALID_ARGUMENT`.
