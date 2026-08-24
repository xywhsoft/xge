# XUI Pixel Coordinate Contract

XUI layout, input, hit testing, clipping, damage, and widget geometry use integer
context pixels. XUI rectangles use half-open bounds:

```text
[x, x + width) x [y, y + height)
```

The left and top edges are included. The right and bottom edges are excluded.
Widths and heights are non-negative integer pixel counts.

xLayout continues to calculate ideal geometry with floating-point values. XUI
quantizes the global left, top, right, and bottom edges once when layout results
are committed. Child-local positions are obtained by subtracting the quantized
parent global origin. Rendering and input must consume these committed integer
rectangles and must not apply another layout snap.

XGE keeps its floating-point world and screen-space APIs. APIs whose names end
in `Pixels` consume integer framebuffer geometry. `xgeShapeRectBorderPixels()`
draws an inside, area-based border; it is not a center-line geometric stroke.

Clipping and damage may use outward conversion when accepting floating-point
geometry. Content containment may use inward conversion. Normal widget layout
uses nearest edge conversion. Callers must use the conversion helpers instead
of casting each coordinate or size independently.
