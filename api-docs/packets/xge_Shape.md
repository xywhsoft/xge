# 草稿包：xge.h / Shape（58 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeShapePoint
- 位置: xge.h:1939  已注释: 否
- 签名: `XGE_API void xgeShapePoint(float fX, float fY, float fSize, uint32_t iColor);`
- 实现: src/xge_shape.c:1281（体 4 行）
- 用法: examples/tutorial_capture/ch21_main1.c:14

## xgeShapePointPx
- 位置: xge.h:1940  已注释: 否
- 签名: `XGE_API void xgeShapePointPx(float fX, float fY, float fSize, uint32_t iColor);`
- 实现: src/xge_shape.c:1286（体 4 行）

## xgeShapeLine
- 位置: xge.h:1941  已注释: 否
- 签名: `XGE_API void xgeShapeLine(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1343（体 4 行）
- 用法: examples/audit_camera_viewport/main.c:26; examples/audit_camera_viewport/main.c:27; examples/audit_camera_viewport/main.c:112

## xgeShapeLinePx
- 位置: xge.h:1942  已注释: 否
- 签名: `XGE_API void xgeShapeLinePx(float fX0, float fY0, float fX1, float fY1, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1348（体 4 行）
- 用法: examples/xge_shape/main.c:153; examples/xge_shape_ex/main.c:361

## xgeShapeRectFill
- 位置: xge.h:1943  已注释: 否
- 签名: `XGE_API void xgeShapeRectFill(xge_rect_t tRect, uint32_t iColor);`
- 实现: src/xge_shape.c:1353（体 4 行）
- 用法: examples/audit_camera_viewport/main.c:17; examples/audit_camera_viewport/main.c:30; examples/audit_camera_viewport/main.c:31

## xgeShapeRectFillPx
- 位置: xge.h:1944  已注释: 否
- 签名: `XGE_API void xgeShapeRectFillPx(xge_rect_t tRect, uint32_t iColor);`
- 实现: src/xge_shape.c:1358（体 4 行）
- 用法: examples/audit_shape_basic/main.c:240; examples/xge_particles/common.c:113; examples/xge_particles/common.c:119

## xgeShapeRectStroke
- 位置: xge.h:1945  已注释: 否
- 签名: `XGE_API void xgeShapeRectStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1441（体 4 行）
- 用法: examples/audit_shape_basic/main.c:237; examples/audit_text_render/main.c:46; examples/audit_text_render/main.c:48

## xgeShapeRectStrokePx
- 位置: xge.h:1946  已注释: 否
- 签名: `XGE_API void xgeShapeRectStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1446（体 4 行）
- 用法: examples/audit_shape_basic/main.c:241; examples/xge_svg/main.c:929

## xgeShapeRectFillPixels
- 位置: xge.h:1947  已注释: 否
- 签名: `XGE_API void xgeShapeRectFillPixels(xge_rect_i_t tRect, uint32_t iColor);`
- 实现: src/xge_shape.c:1363（体 11 行）

## xgeShapeRectBorderPixels
- 位置: xge.h:1948  已注释: 否
- 签名: `XGE_API void xgeShapeRectBorderPixels(xge_rect_i_t tRect, xge_edges_i_t tBorder, uint32_t iColor);`
- 实现: src/xge_shape.c:1375（体 38 行）

## xgeShapeCircleFill
- 位置: xge.h:1949  已注释: 否
- 签名: `XGE_API void xgeShapeCircleFill(float fX, float fY, float fRadius, uint32_t iColor);`
- 实现: src/xge_shape.c:1456（体 4 行）
- 用法: examples/audit_camera_viewport/main.c:33; examples/audit_camera_viewport/main.c:97; examples/audit_camera_viewport/main.c:110

## xgeShapeCircleFillPx
- 位置: xge.h:1950  已注释: 否
- 签名: `XGE_API void xgeShapeCircleFillPx(float fX, float fY, float fRadius, uint32_t iColor);`
- 实现: src/xge_shape.c:1461（体 4 行）
- 用法: examples/audit_shape_basic/main.c:245

## xgeShapeCircleStroke
- 位置: xge.h:1951  已注释: 否
- 签名: `XGE_API void xgeShapeCircleStroke(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1490（体 4 行）
- 用法: examples/audit_shape_basic/main.c:129; examples/tutorial_capture/ch02_main1.c:25; examples/tutorial_capture/ch03_main1.c:31

## xgeShapeCircleStrokePx
- 位置: xge.h:1952  已注释: 否
- 签名: `XGE_API void xgeShapeCircleStrokePx(float fX, float fY, float fRadius, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1495（体 4 行）
- 用法: examples/xge_shape/main.c:163

## xgeShapeArc
- 位置: xge.h:1953  已注释: 否
- 签名: `XGE_API void xgeShapeArc(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1480（体 4 行）
- 用法: examples/audit_shape_basic/main.c:160; examples/tutorial_capture/ch24_main1.c:13; examples/tutorial_capture/ch24_main1.c:17

## xgeShapeArcPx
- 位置: xge.h:1954  已注释: 否
- 签名: `XGE_API void xgeShapeArcPx(float fX, float fY, float fRadius, float fStartRadians, float fEndRadians, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1485（体 4 行）
- 用法: examples/xge_shape/main.c:162

## xgeShapeTriangleFill
- 位置: xge.h:1955  已注释: 否
- 签名: `XGE_API void xgeShapeTriangleFill(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);`
- 实现: src/xge_shape.c:1510（体 4 行）
- 用法: examples/tutorial_capture/ch115_main1.c:43; examples/tutorial_capture/ch120_main1.c:68; examples/tutorial_capture/ch25_main1.c:13

## xgeShapeTriangleFillPx
- 位置: xge.h:1956  已注释: 否
- 签名: `XGE_API void xgeShapeTriangleFillPx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor);`
- 实现: src/xge_shape.c:1515（体 4 行）

## xgeShapeTriangleStroke
- 位置: xge.h:1957  已注释: 否
- 签名: `XGE_API void xgeShapeTriangleStroke(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1527（体 4 行）
- 用法: examples/tutorial_capture/ch25_main1.c:14

## xgeShapeTriangleStrokePx
- 位置: xge.h:1958  已注释: 否
- 签名: `XGE_API void xgeShapeTriangleStrokePx(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1532（体 4 行）
- 用法: examples/xge_shape/main.c:157

## xgeShapeEllipseFill
- 位置: xge.h:1959  已注释: 否
- 签名: `XGE_API void xgeShapeEllipseFill(float fX, float fY, float fRadiusX, float fRadiusY, uint32_t iColor);`
- 实现: src/xge_shape.c:1542（体 4 行）
- 用法: examples/audit_shape_basic/main.c:176; examples/tutorial_capture/ch26_main1.c:11; examples/tutorial_capture/ch26_main1.c:14

## xgeShapeEllipseFillPx
- 位置: xge.h:1960  已注释: 否
- 签名: `XGE_API void xgeShapeEllipseFillPx(float fX, float fY, float fRadiusX, float fRadiusY, uint32_t iColor);`
- 实现: src/xge_shape.c:1547（体 4 行）
- 用法: examples/xge_shape/main.c:159

## xgeShapeEllipseStroke
- 位置: xge.h:1961  已注释: 否
- 签名: `XGE_API void xgeShapeEllipseStroke(float fX, float fY, float fRadiusX, float fRadiusY, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1557（体 4 行）
- 用法: examples/audit_shape_basic/main.c:177; examples/tutorial_capture/ch26_main1.c:17

## xgeShapeEllipseStrokePx
- 位置: xge.h:1962  已注释: 否
- 签名: `XGE_API void xgeShapeEllipseStrokePx(float fX, float fY, float fRadiusX, float fRadiusY, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1562（体 4 行）
- 用法: examples/xge_shape/main.c:160

## xgeShapePieFill
- 位置: xge.h:1963  已注释: 否
- 签名: `XGE_API void xgeShapePieFill(float fX, float fY, float fRadiusX, float fRadiusY, float fStartRadians, float fEndRadians, uint32_t iColor);`
- 实现: src/xge_shape.c:1572（体 4 行）
- 用法: examples/tutorial_capture/ch27_main1.c:23; examples/tutorial_capture/ch27_main1.c:29

## xgeShapePieFillPx
- 位置: xge.h:1964  已注释: 否
- 签名: `XGE_API void xgeShapePieFillPx(float fX, float fY, float fRadiusX, float fRadiusY, float fStartRadians, float fEndRadians, uint32_t iColor);`
- 实现: src/xge_shape.c:1577（体 4 行）
- 用法: examples/xge_shape/main.c:165

## xgeShapeChordFill
- 位置: xge.h:1965  已注释: 否
- 签名: `XGE_API void xgeShapeChordFill(float fX, float fY, float fRadiusX, float fRadiusY, float fStartRadians, float fEndRadians, uint32_t iColor);`
- 实现: src/xge_shape.c:1587（体 4 行）
- 用法: examples/tutorial_capture/ch27_main1.c:34

## xgeShapeChordFillPx
- 位置: xge.h:1966  已注释: 否
- 签名: `XGE_API void xgeShapeChordFillPx(float fX, float fY, float fRadiusX, float fRadiusY, float fStartRadians, float fEndRadians, uint32_t iColor);`
- 实现: src/xge_shape.c:1592（体 4 行）
- 用法: examples/xge_shape/main.c:166

## xgeShapeCapsuleFill
- 位置: xge.h:1967  已注释: 否
- 签名: `XGE_API void xgeShapeCapsuleFill(xge_rect_t tRect, uint32_t iColor);`
- 实现: src/xge_shape.c:1708（体 4 行）
- 用法: examples/audit_shape_basic/main.c:224; examples/audit_shape_basic/main.c:226; examples/tutorial_capture/ch28_main1.c:10

## xgeShapeCapsuleFillPx
- 位置: xge.h:1968  已注释: 否
- 签名: `XGE_API void xgeShapeCapsuleFillPx(xge_rect_t tRect, uint32_t iColor);`
- 实现: src/xge_shape.c:1713（体 4 行）
- 用法: examples/xge_shape/main.c:169

## xgeShapeCapsuleStroke
- 位置: xge.h:1969  已注释: 否
- 签名: `XGE_API void xgeShapeCapsuleStroke(xge_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1726（体 4 行）
- 用法: examples/audit_shape_basic/main.c:225; examples/audit_shape_basic/main.c:227; examples/tutorial_capture/ch28_main1.c:14

## xgeShapeCapsuleStrokePx
- 位置: xge.h:1970  已注释: 否
- 签名: `XGE_API void xgeShapeCapsuleStrokePx(xge_rect_t tRect, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1731（体 4 行）
- 用法: examples/xge_shape/main.c:172

## xgeShapeRenderModeSet
- 位置: xge.h:1971  已注释: 否
- 签名: `XGE_API void xgeShapeRenderModeSet(int iMode);`
- 实现: src/xge_shape.c:1616（体 6 行）
- 用法: examples/tutorial_capture/ch32_main1.c:9; examples/tutorial_capture/ch32_main1.c:18; examples/tutorial_capture/ch32_main1.c:24

## xgeShapeRenderModeGet
- 位置: xge.h:1972  已注释: 否
- 签名: `XGE_API int xgeShapeRenderModeGet(void);`
- 实现: src/xge_shape.c:1623（体 4 行）

## xgeShapeRoundRectModeSet
- 位置: xge.h:1973  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectModeSet(int iMode);`
- 实现: src/xge_shape.c:1628（体 6 行）
- 用法: examples/xge_shape/main.c:272

## xgeShapeRoundRectModeGet
- 位置: xge.h:1974  已注释: 否
- 签名: `XGE_API int xgeShapeRoundRectModeGet(void);`
- 实现: src/xge_shape.c:1635（体 4 行）

## xgeShapeRoundRectFill
- 位置: xge.h:1975  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectFill(xge_rect_t tRect, float fRadius, uint32_t iColor);`
- 实现: src/xge_shape.c:1640（体 4 行）
- 用法: examples/audit_shape_basic/main.c:144; examples/tutorial_capture/ch29_main1.c:12; examples/tutorial_capture/ch29_main1.c:23

## xgeShapeRoundRectFillPx
- 位置: xge.h:1976  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectFillPx(xge_rect_t tRect, float fRadius, uint32_t iColor);`
- 实现: src/xge_shape.c:1645（体 4 行）
- 用法: examples/xge_particles/common.c:138; examples/xge_particles/common.c:144; examples/xge_particles/common.c:147

## xgeShapeRoundRectFillEx
- 位置: xge.h:1977  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectFillEx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, uint32_t iColor);`
- 实现: src/xge_shape.c:1650（体 4 行）
- 用法: examples/tutorial_capture/ch30_main1.c:13; examples/tutorial_capture/ch30_main1.c:23

## xgeShapeRoundRectFillExPx
- 位置: xge.h:1978  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectFillExPx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, uint32_t iColor);`
- 实现: src/xge_shape.c:1655（体 4 行）

## xgeShapeRoundRectStroke
- 位置: xge.h:1979  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectStroke(xge_rect_t tRect, float fRadius, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1660（体 4 行）
- 用法: examples/audit_shape_basic/main.c:145; examples/tutorial_capture/ch29_main1.c:13; examples/tutorial_capture/ch30_main1.c:29

## xgeShapeRoundRectStrokePx
- 位置: xge.h:1980  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectStrokePx(xge_rect_t tRect, float fRadius, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1665（体 4 行）
- 用法: examples/xge_shape_ex/main.c:353

## xgeShapeRoundRectStrokeEx
- 位置: xge.h:1981  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectStrokeEx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1670（体 4 行）

## xgeShapeRoundRectStrokeExPx
- 位置: xge.h:1982  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectStrokeExPx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, float fWidth, uint32_t iColor);`
- 实现: src/xge_shape.c:1675（体 4 行）

## xgeShapeRoundRectDraw
- 位置: xge.h:1983  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectDraw(xge_rect_t tRect, float fRadius, uint32_t iFillColor, float fStrokeWidth, uint32_t iStrokeColor);`
- 实现: src/xge_shape.c:1680（体 4 行）
- 用法: examples/tutorial_capture/ch29_main1.c:17

## xgeShapeRoundRectDrawPx
- 位置: xge.h:1984  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectDrawPx(xge_rect_t tRect, float fRadius, uint32_t iFillColor, float fStrokeWidth, uint32_t iStrokeColor);`
- 实现: src/xge_shape.c:1685（体 4 行）
- 用法: examples/xge_shape/main.c:131; examples/xge_shape/main.c:175; examples/xge_shape_ex/main.c:88

## xgeShapeRoundRectDrawEx
- 位置: xge.h:1985  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectDrawEx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, uint32_t iFillColor, float fStrokeWidth, uint32_t iStrokeColor);`
- 实现: src/xge_shape.c:1690（体 4 行）

## xgeShapeRoundRectDrawExPx
- 位置: xge.h:1986  已注释: 否
- 签名: `XGE_API void xgeShapeRoundRectDrawExPx(xge_rect_t tRect, xge_shape_round_rect_t tRadii, int iMode, uint32_t iFillColor, float fStrokeWidth, uint32_t iStrokeColor);`
- 实现: src/xge_shape.c:1695（体 4 行）
- 用法: examples/xge_shape/main.c:181; examples/xge_shape/main.c:182

## xgeShapePolygonFill
- 位置: xge.h:1987  已注释: 否
- 签名: `XGE_API void xgeShapePolygonFill(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);`
- 实现: src/xge_shape.c:1736（体 4 行）
- 用法: examples/audit_shape_basic/main.c:198; examples/audit_shape_basic/main.c:206; examples/audit_shape_basic/main.c:215

## xgeShapePolygonFillPx
- 位置: xge.h:1988  已注释: 否
- 签名: `XGE_API void xgeShapePolygonFillPx(const xge_vec2_t* pPoints, int iCount, uint32_t iColor);`
- 实现: src/xge_shape.c:1741（体 4 行）

## xgeShapeMeshFill
- 位置: xge.h:1989  已注释: 否
- 签名: `XGE_API int xgeShapeMeshFill(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount);`
- 实现: src/xge_shape.c:1791（体 4 行）
- 用法: examples/tutorial_capture/ch31_main1.c:38

## xgeShapeMeshFillPx
- 位置: xge.h:1990  已注释: 否
- 签名: `XGE_API int xgeShapeMeshFillPx(const xge_shape_vertex_t* pVertices, int iVertexCount, const uint32_t* pIndices, int iIndexCount);`
- 实现: src/xge_shape.c:1796（体 4 行）

## xgeShapeBatchInit
- 位置: xge.h:2328  已注释: 否
- 签名: `XGE_API int xgeShapeBatchInit(xge_shape_batch pBatch, uint32_t iColor, int iTriangleCapacity, uint32_t iFlags);`
- 实现: src/xge_shape.c:1801（体 21 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch106_main1.c:10; examples/tutorial_capture/ch106_main1.c:27

## xgeShapeBatchFree
- 位置: xge.h:2329  已注释: 否
- 签名: `XGE_API void xgeShapeBatchFree(xge_shape_batch pBatch);`
- 实现: src/xge_shape.c:1823（体 10 行）

## xgeShapeBatchClear
- 位置: xge.h:2330  已注释: 否
- 签名: `XGE_API void xgeShapeBatchClear(xge_shape_batch pBatch);`
- 实现: src/xge_shape.c:1834（体 6 行）

## xgeShapeBatchTriangleFill
- 位置: xge.h:2331  已注释: 否
- 签名: `XGE_API int xgeShapeBatchTriangleFill(xge_shape_batch pBatch, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC);`
- 实现: src/xge_shape.c:1851（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK

## xgeShapeBatchRectFill
- 位置: xge.h:2332  已注释: 否
- 签名: `XGE_API int xgeShapeBatchRectFill(xge_shape_batch pBatch, xge_rect_t tRect);`
- 实现: src/xge_shape.c:1871（体 31 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch106_main1.c:20; examples/tutorial_capture/ch106_main1.c:34

## xgeShapeBatchFlush
- 位置: xge.h:2333  已注释: 否
- 签名: `XGE_API int xgeShapeBatchFlush(xge_shape_batch pBatch);`
- 实现: src/xge_shape.c:1903（体 44 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_GPU_FAILED
- 用法: examples/tutorial_capture/ch106_main1.c:24; examples/tutorial_capture/ch106_main1.c:36

