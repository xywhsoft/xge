# Canvas Widget

`Canvas` is a scrollable drawing surface. It owns a persistent target surface, renders that surface through an internal `ScrollFrame` viewport, and exposes the proxy draw operations as widget APIs.

Use it when a control needs durable free-form drawing content rather than child widgets.

## Creation

```c
xui_canvas_desc_t desc;
xui_widget canvas;

memset(&desc, 0, sizeof(desc));
desc.iSize = sizeof(desc);
desc.fCanvasWidth = 1200.0f;
desc.fCanvasHeight = 800.0f;
desc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
desc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
desc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
desc.bPenEnabled = 1;
desc.fPenWidth = 4.0f;
desc.iPenColor = XUI_COLOR_RGBA(30, 105, 220, 255);

xuiCanvasCreate(context, &canvas, &desc);
```

## Drawing

The drawing APIs paint into the canvas surface and invalidate the viewport cache:

- `xuiCanvasClear` / `xuiCanvasClearRect`
- `xuiCanvasDrawSurface` / `xuiCanvasDrawSurfaceQuad`
- `xuiCanvasDrawMeshTriangles`
- `xuiCanvasDrawPoint` / `xuiCanvasDrawLine`
- `xuiCanvasDrawTriangleFill` / `xuiCanvasDrawTriangleStroke`
- `xuiCanvasDrawRectFill` / `xuiCanvasDrawRectStroke`
- `xuiCanvasDrawCircleFill` / `xuiCanvasDrawCircleStroke`
- `xuiCanvasDrawRoundRectFill` / `xuiCanvasDrawRoundRectStroke`
- `xuiCanvasDrawText`

Coordinates are canvas-content coordinates. Scrolling only changes the viewport into the persistent surface.

## Scrolling

Canvas uses an internal `ScrollFrame`.

- `xuiCanvasSetCanvasSize` recreates the target surface and updates scroll content size.
- `xuiCanvasSetOffset`, `xuiCanvasScrollBy`, and `xuiCanvasEnsureRectVisible` forward to the internal frame.
- `xuiCanvasGetFrameWidget`, `xuiCanvasGetViewportWidget`, and `xuiCanvasGetModel` expose integration points for tests and advanced layout code.

## Pen Input

`xuiCanvasSetPen` enables the built-in left-button pen. Pointer movement is translated into content coordinates and persisted with `xuiCanvasDrawLine`.

## Style Properties

- `canvas.background.color`
- `canvas.border.color`
- `canvas.pen.color`
