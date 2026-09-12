# 草稿包：xui.h / canvas（36 条 API）

> 生成 2026-09-10 02:55 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCanvasGetType
- 位置: xui.h:8584  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCanvasGetType(xui_context pContext);`
- 实现: src/xui_canvas.c:473（体 30 行）
- 返回码: NULL

## xuiCanvasCreate
- 位置: xui.h:8585  已注释: 否
- 签名: `XUI_API int xuiCanvasCreate(xui_context pContext, xui_widget* ppWidget, const xui_canvas_desc_t* pDesc);`
- 实现: src/xui_canvas.c:504（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch190_main1.c:15; examples/tutorial_capture/ch203_main1.c:31; examples/xui_canvas/main.c:177

## xuiCanvasGetFrameWidget
- 位置: xui.h:8586  已注释: 否
- 签名: `XUI_API xui_widget xuiCanvasGetFrameWidget(xui_widget pWidget);`
- 实现: src/xui_canvas.c:515（体 5 行）
- 用法: test_xui/xui_canvas_test.c:110

## xuiCanvasGetViewportWidget
- 位置: xui.h:8587  已注释: 否
- 签名: `XUI_API xui_widget xuiCanvasGetViewportWidget(xui_widget pWidget);`
- 实现: src/xui_canvas.c:521（体 5 行）
- 用法: examples/xui_canvas/main.c:299; examples/xui_canvas/main.c:317; test_xui/xui_canvas_test.c:110

## xuiCanvasGetModel
- 位置: xui.h:8588  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiCanvasGetModel(xui_widget pWidget);`
- 实现: src/xui_canvas.c:527（体 5 行）

## xuiCanvasGetSurface
- 位置: xui.h:8589  已注释: 否
- 签名: `XUI_API xui_surface xuiCanvasGetSurface(xui_widget pWidget);`
- 实现: src/xui_canvas.c:533（体 7 行）
- 返回码: NULL
- 用法: examples/xui_canvas/main.c:298; examples/xui_canvas/main.c:298; test_xui/xui_canvas_test.c:111

## xuiCanvasSetCanvasSize
- 位置: xui.h:8590  已注释: 否
- 签名: `XUI_API int xuiCanvasSetCanvasSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_canvas.c:541（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_canvas_test.c:141

## xuiCanvasGetCanvasSize
- 位置: xui.h:8591  已注释: 否
- 签名: `XUI_API int xuiCanvasGetCanvasSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_canvas.c:563（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_canvas_test.c:108; test_xui/xui_canvas_test.c:143

## xuiCanvasSetViewportHint
- 位置: xui.h:8592  已注释: 否
- 签名: `XUI_API int xuiCanvasSetViewportHint(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_canvas.c:572（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCanvasGetViewportHint
- 位置: xui.h:8593  已注释: 否
- 签名: `XUI_API int xuiCanvasGetViewportHint(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_canvas.c:589（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCanvasSetOffset
- 位置: xui.h:8594  已注释: 否
- 签名: `XUI_API int xuiCanvasSetOffset(xui_widget pWidget, float fOffsetX, float fOffsetY);`
- 实现: src/xui_canvas.c:598（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_canvas_test.c:133

## xuiCanvasScrollBy
- 位置: xui.h:8595  已注释: 否
- 签名: `XUI_API int xuiCanvasScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY);`
- 实现: src/xui_canvas.c:609（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCanvasGetOffset
- 位置: xui.h:8596  已注释: 否
- 签名: `XUI_API int xuiCanvasGetOffset(xui_widget pWidget, float* pOffsetX, float* pOffsetY);`
- 实现: src/xui_canvas.c:620（体 5 行）
- 用法: examples/xui_canvas/main.c:303; examples/xui_canvas/main.c:313; test_xui/xui_canvas_test.c:135

## xuiCanvasEnsureRectVisible
- 位置: xui.h:8597  已注释: 否
- 签名: `XUI_API int xuiCanvasEnsureRectVisible(xui_widget pWidget, xui_rect_t tContentRect);`
- 实现: src/xui_canvas.c:626（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_canvas/main.c:312

## xuiCanvasClear
- 位置: xui.h:8598  已注释: 否
- 签名: `XUI_API int xuiCanvasClear(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_canvas.c:637（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_canvas/main.c:214; examples/xui_canvas/main.c:243; test_xui/xui_canvas_test.c:114

## xuiCanvasClearRect
- 位置: xui.h:8599  已注释: 否
- 签名: `XUI_API int xuiCanvasClearRect(xui_widget pWidget, xui_rect_t tRect, uint32_t iColor);`
- 实现: src/xui_canvas.c:668（体 7 行）

## xuiCanvasDrawSurface
- 位置: xui.h:8600  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawSurface(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_canvas.c:685（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCanvasDrawSurfaceQuad
- 位置: xui.h:8601  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawSurfaceQuad(xui_widget pWidget, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);`
- 实现: src/xui_canvas.c:702（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCanvasDrawMeshTriangles
- 位置: xui.h:8602  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawMeshTriangles(xui_widget pWidget, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags);`
- 实现: src/xui_canvas.c:726（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_canvas/main.c:233; test_xui/xui_canvas_test.c:130

## xuiCanvasDrawPoint
- 位置: xui.h:8603  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawPoint(xui_widget pWidget, float fX, float fY, float fSize, uint32_t iColor);`
- 实现: src/xui_canvas.c:751（体 5 行）

## xuiCanvasDrawLine
- 位置: xui.h:8604  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawLine(xui_widget pWidget, float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);`
- 实现: src/xui_canvas.c:756（体 5 行）
- 用法: examples/xui_canvas/main.c:196; examples/xui_canvas/main.c:200; examples/xui_canvas/main.c:225

## xuiCanvasDrawTriangleFill
- 位置: xui.h:8605  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawTriangleFill(xui_widget pWidget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, uint32_t iColor);`
- 实现: src/xui_canvas.c:779（体 5 行）
- 用法: examples/xui_canvas/main.c:223

## xuiCanvasDrawTriangleStroke
- 位置: xui.h:8606  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawTriangleStroke(xui_widget pWidget, xui_vec2_t tA, xui_vec2_t tB, xui_vec2_t tC, float fWidth, uint32_t iColor);`
- 实现: src/xui_canvas.c:784（体 5 行）
- 用法: examples/xui_canvas/main.c:224

## xuiCanvasDrawRectFill
- 位置: xui.h:8607  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawRectFill(xui_widget pWidget, xui_rect_t tRect, uint32_t iColor);`
- 实现: src/xui_canvas.c:807（体 5 行）
- 用法: examples/xui_canvas/main.c:217; examples/xui_canvas/main.c:246; test_xui/xui_canvas_test.c:116

## xuiCanvasDrawRectStroke
- 位置: xui.h:8608  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawRectStroke(xui_widget pWidget, xui_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xui_canvas.c:812（体 5 行）
- 用法: examples/xui_canvas/main.c:218; examples/xui_canvas/main.c:247

## xuiCanvasDrawCircleFill
- 位置: xui.h:8609  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawCircleFill(xui_widget pWidget, float fX, float fY, float fRadius, uint32_t iColor);`
- 实现: src/xui_canvas.c:834（体 5 行）
- 用法: examples/xui_canvas/main.c:221; examples/xui_canvas/main.c:248

## xuiCanvasDrawCircleStroke
- 位置: xui.h:8610  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawCircleStroke(xui_widget pWidget, float fX, float fY, float fRadius, float fWidth, uint32_t iColor);`
- 实现: src/xui_canvas.c:839（体 5 行）
- 用法: examples/xui_canvas/main.c:222; test_xui/xui_canvas_test.c:121

## xuiCanvasDrawText
- 位置: xui.h:8611  已注释: 否
- 签名: `XUI_API int xuiCanvasDrawText(xui_widget pWidget, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_canvas.c:854（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_canvas/main.c:216; examples/xui_canvas/main.c:219; examples/xui_canvas/main.c:220

## xuiCanvasSetPen
- 位置: xui.h:8612  已注释: 否
- 签名: `XUI_API int xuiCanvasSetPen(xui_widget pWidget, int bEnabled, float fWidth, uint32_t iColor);`
- 实现: src/xui_canvas.c:862（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_canvas_test.c:148; test_xui/xui_style_basic_canvas_test.c:81; test_xui/xui_style_basic_canvas_test.c:83

## xuiCanvasGetPen
- 位置: xui.h:8613  已注释: 否
- 签名: `XUI_API int xuiCanvasGetPen(xui_widget pWidget, int* pEnabled, float* pWidth, uint32_t* pColor);`
- 实现: src/xui_canvas.c:873（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_canvas_test.c:150; test_xui/xui_style_basic_canvas_test.c:51

## xuiCanvasSetColors
- 位置: xui.h:8614  已注释: 否
- 签名: `XUI_API int xuiCanvasSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iPen);`
- 实现: src/xui_canvas.c:883（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCanvasGetColors
- 位置: xui.h:8615  已注释: 否
- 签名: `XUI_API int xuiCanvasGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pPen);`
- 实现: src/xui_canvas.c:894（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCanvasSetScrollbarPolicy
- 位置: xui.h:8616  已注释: 否
- 签名: `XUI_API int xuiCanvasSetScrollbarPolicy(xui_widget pWidget, int iPolicyX, int iPolicyY);`
- 实现: src/xui_canvas.c:904（体 5 行）

## xuiCanvasSetScrollbarMode
- 位置: xui.h:8617  已注释: 否
- 签名: `XUI_API int xuiCanvasSetScrollbarMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_canvas.c:910（体 5 行）

## xuiCanvasGetChangeCount
- 位置: xui.h:8618  已注释: 否
- 签名: `XUI_API int xuiCanvasGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_canvas.c:916（体 5 行）

## xuiCanvasGetDrawCount
- 位置: xui.h:8619  已注释: 否
- 签名: `XUI_API int xuiCanvasGetDrawCount(xui_widget pWidget);`
- 实现: src/xui_canvas.c:922（体 5 行）
- 用法: examples/xui_canvas/main.c:302; examples/xui_canvas/main.c:302; examples/xui_canvas/main.c:326

