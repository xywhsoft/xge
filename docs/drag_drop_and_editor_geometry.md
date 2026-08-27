# Drag/drop and editor geometry

## Coordinate and thread contract

- XGE pointer and native drag coordinates are framebuffer pixels and match
  `xgeGetWidth()` / `xgeGetHeight()`.
- The XGE-to-XUI proxy converts those coordinates once into XUI context pixel
  coordinates. Widgets must not apply DPI or parent-offset compensation.
- XUI drag events and CodeEdit geometry use XUI context coordinates.
- Rendering into an externally positioned surface does not change that
  contract. The application owns the surface-to-screen transform; XUI does not
  infer or compensate for external window, viewport, or parent offsets.
- Native window, drag/drop, and widget APIs are UI-thread APIs.
- `xgePlatformNativeHandle()` returns a borrowed opaque handle. It is `NULL`
  before initialization, after shutdown, and for backends without a window.

## Drag/drop

`xge_data_object` and `xui_data_object` store ordered MIME-like formats and
support lazy providers. A session negotiates one of COPY, MOVE, or LINK.
Targets receive ENTER, OVER, LEAVE, and DROP synchronously so native backends
can return the selected effect before the OS call returns.

Windows uses OLE `IDataObject`, `IDropTarget`, and `IDropSource`. Other Sokol
backends expose file drops when the platform supplies them. XUI internal
drag/drop is independent of native window support and remains available in
headless contexts.

Drop targets opt in with `xuiWidgetSetDropEnabled()`. During ENTER/OVER/DROP,
call `xuiDragAccept()` with a single allowed effect. Payload pointers are
borrowed; retain a data object with AddRef when it must outlive the callback.

## Word wrap and exact CodeEdit geometry

TextEdit, CodeEdit, and RichEdit expose symmetric runtime WordWrap Set/Get
APIs. TextEdit uses the shared shaped text layout, RichEdit uses xLayout Flow,
and CodeEdit uses a sparse visual-line index. CodeEdit shapes only viewport
neighbors; unmeasured lines keep a one-row estimate until visited.

`xuiCodeEditHitTestText()` and `xuiCodeEditGetTextOffsetRect()` consume the
same visual-line boundaries, shaped caret stops, fold state, tabs, scrolling,
and soft wrapping used by rendering. UTF-8 offsets are byte offsets. Padding
returns the nearest insertion position with `bInsideText == 0`.
