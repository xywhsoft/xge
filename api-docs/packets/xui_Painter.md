# 草稿包：xui.h / Painter（15 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPainterBegin
- 位置: xui.h:6074  已注释: 否
- 签名: `XUI_API int xuiPainterBegin(xui_context pContext, xui_surface pTarget, xui_painter* ppPainter);`
- 实现: src/xui_core.c:2355（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_icon/main.c:215; test_xui/xui_builtin_asset_test.c:88; test_xui/xui_chart_test.c:241

## xuiPainterEnd
- 位置: xui.h:6075  已注释: 否
- 签名: `XUI_API int xuiPainterEnd(xui_painter pPainter);`
- 实现: src/xui_core.c:2386（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_icon/main.c:252; test_xui/xui_builtin_asset_test.c:92; test_xui/xui_builtin_asset_test.c:101

## xuiPainterGetDrawContext
- 位置: xui.h:6076  已注释: 否
- 签名: `XUI_API xui_draw_context xuiPainterGetDrawContext(xui_painter pPainter);`
- 实现: src/xui_core.c:2405（体 4 行）
- 用法: test_xui/xui_render_schedule_test.c:1475

## xuiPainterClearRect
- 位置: xui.h:6077  已注释: 否
- 签名: `XUI_API int xuiPainterClearRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor);`
- 实现: src/xui_core.c:2410（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_render_schedule_test.c:1478

## xuiPainterDrawSurface
- 位置: xui.h:6078  已注释: 否
- 签名: `XUI_API int xuiPainterDrawSurface(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_core.c:2422（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_builtin_asset_test.c:90

## xuiPainterDrawSurfaceQuad
- 位置: xui.h:6079  已注释: 否
- 签名: `XUI_API int xuiPainterDrawSurfaceQuad(xui_painter pPainter, xui_surface pSurface, const xui_surface_vertex_t* pVertices, uint32_t iFlags);`
- 实现: src/xui_core.c:2437（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPainterDrawMeshTriangles
- 位置: xui.h:6080  已注释: 否
- 签名: `XUI_API int xuiPainterDrawMeshTriangles(xui_painter pPainter, const xui_mesh_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount, uint32_t iFlags);`
- 实现: src/xui_core.c:2451（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_chart_test.c:243

## xuiPainterFillPath
- 位置: xui.h:6081  已注释: 否
- 签名: `XUI_API int xuiPainterFillPath(xui_painter pPainter, xui_path pPath, uint32_t iColor, float fTolerance);`
- 实现: src/xui_core.c:2481（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_chart_test.c:269; test_xui/xui_vector_smoke_test.c:58

## xuiPainterDrawPath
- 位置: xui.h:6082  已注释: 否
- 签名: `XUI_API int xuiPainterDrawPath(xui_painter pPainter, xui_path pPath, const xui_path_style_t* pStyle, float fTolerance);`
- 实现: src/xui_core.c:2498（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_test.c:277; test_xui/xui_chart_test.c:282; test_xui/xui_chart_test.c:286

## xuiPainterDrawSvgPath
- 位置: xui.h:6083  已注释: 否
- 签名: `XUI_API int xuiPainterDrawSvgPath(xui_painter pPainter, const char* sPath, xui_rect_t tViewBox, xui_rect_t tTarget, const xui_path_style_t* pStyle, float fTolerance);`
- 实现: src/xui_core.c:2523（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_vector_smoke_test.c:74; test_xui/xui_vector_xge_test.c:58

## xuiPainterFillRect
- 位置: xui.h:6084  已注释: 否
- 签名: `XUI_API int xuiPainterFillRect(xui_painter pPainter, xui_rect_t tRect, uint32_t iColor);`
- 实现: src/xui_core.c:2541（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:75; examples/xui_icon/main.c:220; examples/xui_icon/main.c:231

## xuiPainterStrokeRect
- 位置: xui.h:6085  已注释: 否
- 签名: `XUI_API int xuiPainterStrokeRect(xui_painter pPainter, xui_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xui_core.c:2556（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:221; examples/xui_icon/main.c:232

## xuiPainterDrawVectorIcon
- 位置: xui.h:6086  已注释: 否
- 签名: `XUI_API int xuiPainterDrawVectorIcon(xui_painter pPainter, const char* sName, xui_rect_t tRect, uint32_t iColor);`
- 实现: src/xui_core.c:2600（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_vector_smoke_test.c:79; test_xui/xui_vector_smoke_test.c:81; test_xui/xui_vector_xge_test.c:63

## xuiPainterDrawText
- 位置: xui.h:6087  已注释: 否
- 签名: `XUI_API int xuiPainterDrawText(xui_painter pPainter, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_core.c:2625（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:195

## xuiPainterDrawNinePatch
- 位置: xui.h:6088  已注释: 否
- 签名: `XUI_API int xuiPainterDrawNinePatch(xui_painter pPainter, xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst, xui_thickness_t tSlice, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_core.c:2640（体 106 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_render_schedule_test.c:1480

