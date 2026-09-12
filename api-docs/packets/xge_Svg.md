# 草稿包：xge.h / Svg（66 条 API）

> 生成 2026-09-10 02:29 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeSvgCreate
- 位置: xge.h:2196  已注释: 否
- 签名: `XGE_API int xgeSvgCreate(xge_svg* ppSvg);`
- 实现: src/xge_svg.c:16032（体 32 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_svg_render/main.c:57; examples/tutorial_capture/ch73_main1.c:16; examples/tutorial_capture/ch73_main1.c:36

## xgeSvgDestroy
- 位置: xge.h:2197  已注释: 否
- 签名: `XGE_API void xgeSvgDestroy(xge_svg pSvg);`
- 实现: src/xge_svg.c:16129（体 31 行）
- 用法: examples/audit_svg_render/main.c:58; examples/audit_svg_render/main.c:60; examples/tutorial_capture/ch73_main1.c:30

## xgeSvgClear
- 位置: xge.h:2198  已注释: 否
- 签名: `XGE_API int xgeSvgClear(xge_svg pSvg);`
- 实现: src/xge_svg.c:16179（体 45 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:7862

## xgeSvgClone
- 位置: xge.h:2199  已注释: 否
- 签名: `XGE_API int xgeSvgClone(xge_svg pSvg, xge_svg* ppClone);`
- 实现: src/xge_svg.c:16065（体 63 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch73_main1.c:38; examples/xge_svg/main.c:505; examples/xge_svg/main.c:722

## xgeSvgLoad
- 位置: xge.h:2200  已注释: 否
- 签名: `XGE_API int xgeSvgLoad(xge_svg pSvg, const char* sURI);`
- 实现: src/xge_svg.c:17293（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/xge_svg/main.c:497; examples/xge_svg/main.c:704; test/test_main.c:7947

## xgeSvgLoadCached
- 位置: xge.h:2201  已注释: 否
- 签名: `XGE_API int xgeSvgLoadCached(const char* sURI, xge_svg* ppSvg);`
- 实现: src/xge_svg.c:17317（体 43 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/xge_svg/main.c:936; examples/xge_svg/main.c:940; test/test_main.c:8011

## xgeSvgLoadMemory
- 位置: xge.h:2202  已注释: 否
- 签名: `XGE_API int xgeSvgLoadMemory(xge_svg pSvg, const void* pData, int iSize);`
- 实现: src/xge_svg.c:17288（体 4 行）
- 用法: examples/audit_svg_render/main.c:58; examples/tutorial_capture/ch73_main1.c:17; examples/tutorial_capture/ch73_main1.c:37

## xgeSvgAddRef
- 位置: xge.h:2203  已注释: 否
- 签名: `XGE_API int xgeSvgAddRef(xge_svg pSvg);`
- 实现: src/xge_svg.c:16161（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgRefCountGet
- 位置: xge.h:2204  已注释: 否
- 签名: `XGE_API int xgeSvgRefCountGet(xge_svg pSvg, int* pRefCount);`
- 实现: src/xge_svg.c:16170（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgCacheInvalidate
- 位置: xge.h:2205  已注释: 否
- 签名: `XGE_API int xgeSvgCacheInvalidate(const char* sURI);`
- 实现: src/xge_svg.c:17361（体 21 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_NOT_FOUND
- 用法: test/test_main.c:8015

## xgeSvgCacheClear
- 位置: xge.h:2206  已注释: 否
- 签名: `XGE_API void xgeSvgCacheClear(void);`
- 实现: src/xge_svg.c:17383（体 13 行）
- 用法: examples/xge_svg/main.c:533; examples/xge_svg/main.c:1094; examples/xge_svg/main.c:1107

## xgeSvgSetSize
- 位置: xge.h:2207  已注释: 否
- 签名: `XGE_API int xgeSvgSetSize(xge_svg pSvg, float fWidth, float fHeight);`
- 实现: src/xge_svg.c:18973（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch74_main1.c:38; examples/xge_svg/main.c:706

## xgeSvgGetSize
- 位置: xge.h:2208  已注释: 否
- 签名: `XGE_API int xgeSvgGetSize(xge_svg pSvg, float* pWidth, float* pHeight);`
- 实现: src/xge_svg.c:18984（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/tutorial_capture/ch74_main1.c:20; examples/xge_svg/main.c:732

## xgeSvgSetOrigin
- 位置: xge.h:2209  已注释: 否
- 签名: `XGE_API int xgeSvgSetOrigin(xge_svg pSvg, float fX, float fY);`
- 实现: src/xge_svg.c:18995（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch74_main1.c:43

## xgeSvgGetOrigin
- 位置: xge.h:2210  已注释: 否
- 签名: `XGE_API int xgeSvgGetOrigin(xge_svg pSvg, float* pX, float* pY);`
- 实现: src/xge_svg.c:19005（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgGetViewBox
- 位置: xge.h:2211  已注释: 否
- 签名: `XGE_API int xgeSvgGetViewBox(xge_svg pSvg, xge_rect_t* pViewBox);`
- 实现: src/xge_svg.c:19015（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch74_main1.c:23; test/test_main.c:6662; test/test_main.c:6757

## xgeSvgPaintGetById
- 位置: xge.h:2212  已注释: 否
- 签名: `XGE_API int xgeSvgPaintGetById(xge_svg pSvg, uint32_t iId, xge_svg_paint* ppPaint);`
- 实现: src/xge_svg.c:17501（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:44

## xgeSvgPaintGetByName
- 位置: xge.h:2213  已注释: 否
- 签名: `XGE_API int xgeSvgPaintGetByName(xge_svg pSvg, const char* sName, xge_svg_paint* ppPaint);`
- 实现: src/xge_svg.c:17515（体 5 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch77_main1.c:26; examples/tutorial_capture/ch77_main1.c:32; examples/tutorial_capture/ch77_main1.c:62

## xgeSvgPaintGetPicture
- 位置: xge.h:2214  已注释: 否
- 签名: `XGE_API int xgeSvgPaintGetPicture(xge_svg pSvg, xge_svg_paint* ppPaint);`
- 实现: src/xge_svg.c:17521（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:54; examples/tutorial_capture/ch79_main1.c:37; examples/xge_svg/main.c:582

## xgeSvgPaintIdGet
- 位置: xge.h:2215  已注释: 否
- 签名: `XGE_API int xgeSvgPaintIdGet(xge_svg_paint pPaint, uint32_t* pId);`
- 实现: src/xge_svg.c:17528（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintNameGet
- 位置: xge.h:2216  已注释: 否
- 签名: `XGE_API int xgeSvgPaintNameGet(xge_svg_paint pPaint, const char** ppName);`
- 实现: src/xge_svg.c:17535（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: examples/tutorial_capture/ch79_main1.c:51

## xgeSvgPaintTypeGet
- 位置: xge.h:2217  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTypeGet(xge_svg_paint pPaint, int* pType);`
- 实现: src/xge_svg.c:17558（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintTransformSet
- 位置: xge.h:2218  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformSet(xge_svg_paint pPaint, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_svg.c:17566（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintTransformIdentity
- 位置: xge.h:2219  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformIdentity(xge_svg_paint pPaint);`
- 实现: src/xge_svg.c:17581（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch78_main1.c:27; examples/tutorial_capture/ch78_main1.c:36; examples/tutorial_capture/ch78_main1.c:47

## xgeSvgPaintTransformGet
- 位置: xge.h:2220  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformGet(xge_svg_paint pPaint, xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_svg.c:17598（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintTransformTranslate
- 位置: xge.h:2221  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformTranslate(xge_svg_paint pPaint, float fTX, float fTY);`
- 实现: src/xge_svg.c:17622（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch78_main1.c:28; examples/tutorial_capture/ch78_main1.c:37; examples/tutorial_capture/ch78_main1.c:39

## xgeSvgPaintTransformScale
- 位置: xge.h:2222  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformScale(xge_svg_paint pPaint, float fSX, float fSY);`
- 实现: src/xge_svg.c:17639（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch78_main1.c:62; examples/xge_svg/main.c:576; examples/xge_svg/main.c:584

## xgeSvgPaintTransformRotate
- 位置: xge.h:2223  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTransformRotate(xge_svg_paint pPaint, float fRadians);`
- 实现: src/xge_svg.c:17656（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch78_main1.c:38; examples/xge_svg/main.c:577; examples/xge_svg/main.c:585

## xgeSvgPaintOpacitySet
- 位置: xge.h:2224  已注释: 否
- 签名: `XGE_API int xgeSvgPaintOpacitySet(xge_svg_paint pPaint, float fOpacity);`
- 实现: src/xge_svg.c:17672（体 30 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:29; examples/tutorial_capture/ch77_main1.c:63; examples/tutorial_capture/ch78_main1.c:48

## xgeSvgPaintOpacityGet
- 位置: xge.h:2225  已注释: 否
- 签名: `XGE_API int xgeSvgPaintOpacityGet(xge_svg_paint pPaint, float* pOpacity);`
- 实现: src/xge_svg.c:17703（体 41 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintVisibleSet
- 位置: xge.h:2226  已注释: 否
- 签名: `XGE_API int xgeSvgPaintVisibleSet(xge_svg_paint pPaint, int bVisible);`
- 实现: src/xge_svg.c:17745（体 29 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:34; examples/tutorial_capture/ch77_main1.c:65

## xgeSvgPaintVisibleGet
- 位置: xge.h:2227  已注释: 否
- 签名: `XGE_API int xgeSvgPaintVisibleGet(xge_svg_paint pPaint, int* pVisible);`
- 实现: src/xge_svg.c:17775（体 33 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintBlendSet
- 位置: xge.h:2228  已注释: 否
- 签名: `XGE_API int xgeSvgPaintBlendSet(xge_svg_paint pPaint, int iBlend);`
- 实现: src/xge_svg.c:17809（体 33 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_svg/main.c:581

## xgeSvgPaintBlendGet
- 位置: xge.h:2229  已注释: 否
- 签名: `XGE_API int xgeSvgPaintBlendGet(xge_svg_paint pPaint, int* pBlend);`
- 实现: src/xge_svg.c:17843（体 27 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintMaskShapeSet
- 位置: xge.h:2230  已注释: 否
- 签名: `XGE_API int xgeSvgPaintMaskShapeSet(xge_svg_paint pPaint, xge_shape_ex pTarget, int iMethod);`
- 实现: src/xge_svg.c:18146（体 26 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch79_main1.c:63; examples/xge_svg/main.c:663; examples/xge_svg/main.c:667

## xgeSvgPaintMaskSceneSet
- 位置: xge.h:2231  已注释: 否
- 签名: `XGE_API int xgeSvgPaintMaskSceneSet(xge_svg_paint pPaint, xge_shape_ex_scene pTarget, int iMethod);`
- 实现: src/xge_svg.c:18173（体 26 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintMaskClear
- 位置: xge.h:2232  已注释: 否
- 签名: `XGE_API int xgeSvgPaintMaskClear(xge_svg_paint pPaint);`
- 实现: src/xge_svg.c:18200（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch79_main1.c:74

## xgeSvgPaintMaskGet
- 位置: xge.h:2233  已注释: 否
- 签名: `XGE_API int xgeSvgPaintMaskGet(xge_svg_paint pPaint, int* pMethod, int* pTargetType, xge_shape_ex* ppTargetShape, xge_shape_ex_scene* ppTargetScene);`
- 实现: src/xge_svg.c:18219（体 34 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintClipShapeSet
- 位置: xge.h:2234  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipShapeSet(xge_svg_paint pPaint, xge_shape_ex pClipShape);`
- 实现: src/xge_svg.c:18458（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch79_main1.c:83; examples/xge_svg/main.c:661; examples/xge_svg/main.c:665

## xgeSvgPaintClipClear
- 位置: xge.h:2235  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipClear(xge_svg_paint pPaint);`
- 实现: src/xge_svg.c:18481（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintClipRectGet
- 位置: xge.h:2236  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipRectGet(xge_svg_paint pPaint, xge_rect_t* pRect, int* pEnabled);`
- 实现: src/xge_svg.c:18497（体 27 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintClipShapeGetCount
- 位置: xge.h:2237  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipShapeGetCount(xge_svg_paint pPaint, int* pCount);`
- 实现: src/xge_svg.c:18525（体 20 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintClipShapeGetAt
- 位置: xge.h:2238  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipShapeGetAt(xge_svg_paint pPaint, int iIndex, xge_shape_ex* ppClipShape);`
- 实现: src/xge_svg.c:18546（体 4 行）

## xgeSvgPaintClipShapeGetAtEx
- 位置: xge.h:2239  已注释: 否
- 签名: `XGE_API int xgeSvgPaintClipShapeGetAtEx(xge_svg_paint pPaint, int iIndex, xge_shape_ex* ppClipShape, int* pMode);`
- 实现: src/xge_svg.c:18551（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK

## xgeSvgPaintShapeGet
- 位置: xge.h:2240  已注释: 否
- 签名: `XGE_API int xgeSvgPaintShapeGet(xge_svg_paint pPaint, xge_shape_ex* ppShape);`
- 实现: src/xge_svg.c:18575（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK

## xgeSvgPaintOwnerGet
- 位置: xge.h:2241  已注释: 否
- 签名: `XGE_API int xgeSvgPaintOwnerGet(xge_svg_paint pPaint, xge_svg* ppSvg);`
- 实现: src/xge_svg.c:18584（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:47

## xgeSvgPaintParentGet
- 位置: xge.h:2242  已注释: 否
- 签名: `XGE_API int xgeSvgPaintParentGet(xge_svg_paint pPaint, xge_svg_paint* ppParent);`
- 实现: src/xge_svg.c:18593（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintChildGetCount
- 位置: xge.h:2243  已注释: 否
- 签名: `XGE_API int xgeSvgPaintChildGetCount(xge_svg_paint pPaint, int* pCount);`
- 实现: src/xge_svg.c:18635（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch77_main1.c:57; examples/tutorial_capture/ch79_main1.c:45

## xgeSvgPaintChildGetAt
- 位置: xge.h:2244  已注释: 否
- 签名: `XGE_API int xgeSvgPaintChildGetAt(xge_svg_paint pPaint, int iIndex, xge_svg_paint* ppChild);`
- 实现: src/xge_svg.c:18646（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: examples/tutorial_capture/ch79_main1.c:48

## xgeSvgPaintTraverse
- 位置: xge.h:2245  已注释: 否
- 签名: `XGE_API int xgeSvgPaintTraverse(xge_svg_paint pPaint, xge_svg_paint_visit_proc onPaint, void* pUser);`
- 实现: src/xge_svg.c:18683（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch79_main1.c:39

## xgeSvgPaintGetBounds
- 位置: xge.h:2246  已注释: 否
- 签名: `XGE_API int xgeSvgPaintGetBounds(xge_svg_paint pPaint, float fTolerance, xge_rect_t* pBounds);`
- 实现: src/xge_svg.c:18763（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

## xgeSvgPaintGetOBB
- 位置: xge.h:2247  已注释: 否
- 签名: `XGE_API int xgeSvgPaintGetOBB(xge_svg_paint pPaint, float fTolerance, xge_vec2_t* pPoints4);`
- 实现: src/xge_svg.c:18897（体 31 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgPaintIntersects
- 位置: xge.h:2248  已注释: 否
- 签名: `XGE_API int xgeSvgPaintIntersects(xge_svg_paint pPaint, xge_rect_t tRect, float fTolerance, int* pIntersects);`
- 实现: src/xge_svg.c:18929（体 43 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgSetPreserveAspectRatio
- 位置: xge.h:2249  已注释: 否
- 签名: `XGE_API int xgeSvgSetPreserveAspectRatio(xge_svg pSvg, const char* sValue);`
- 实现: src/xge_svg.c:19024（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch76_main1.c:19; examples/tutorial_capture/ch76_main1.c:34; examples/tutorial_capture/ch76_main1.c:48

## xgeSvgGetPreserveAspectRatio
- 位置: xge.h:2250  已注释: 否
- 签名: `XGE_API int xgeSvgGetPreserveAspectRatio(xge_svg pSvg, int* pAlignX, int* pAlignY, int* pMeetOrSlice);`
- 实现: src/xge_svg.c:19033（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeSvgGetDrawViewport
- 位置: xge.h:2251  已注释: 否
- 签名: `XGE_API int xgeSvgGetDrawViewport(xge_svg pSvg, xge_rect_t tDst, xge_rect_t* pViewport);`
- 实现: src/xge_svg.c:19045（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:6664; test/test_main.c:6732; test/test_main.c:6737

## xgeSvgGetBounds
- 位置: xge.h:2252  已注释: 否
- 签名: `XGE_API int xgeSvgGetBounds(xge_svg pSvg, float fTolerance, xge_rect_t* pBounds);`
- 实现: src/xge_svg.c:22000（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_svg/main.c:510; test/test_main.c:6668; test/test_main.c:6694

## xgeSvgGetDrawBounds
- 位置: xge.h:2253  已注释: 否
- 签名: `XGE_API int xgeSvgGetDrawBounds(xge_svg pSvg, xge_rect_t tDst, float fTolerance, xge_rect_t* pBounds);`
- 实现: src/xge_svg.c:22008（体 4 行）
- 用法: examples/tutorial_capture/ch75_main1.c:40; examples/xge_svg/main.c:517; test/test_main.c:6670

## xgeSvgContainsPoint
- 位置: xge.h:2254  已注释: 否
- 签名: `XGE_API int xgeSvgContainsPoint(xge_svg pSvg, float fX, float fY, float fTolerance, int* pContains);`
- 实现: src/xge_svg.c:22013（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:6674; test/test_main.c:6676; test/test_main.c:6678

## xgeSvgDrawContainsPoint
- 位置: xge.h:2255  已注释: 否
- 签名: `XGE_API int xgeSvgDrawContainsPoint(xge_svg pSvg, xge_rect_t tDst, float fX, float fY, float fTolerance, int* pContains);`
- 实现: src/xge_svg.c:22025（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:6698; test/test_main.c:6700; test/test_main.c:6702

## xgeSvgDraw
- 位置: xge.h:2256  已注释: 否
- 签名: `XGE_API int xgeSvgDraw(xge_svg pSvg, xge_rect_t tDst, float fTolerance);`
- 实现: src/xge_svg.c:22039（体 4 行）
- 用法: examples/audit_svg_render/main.c:59; examples/tutorial_capture/ch73_main1.c:21; examples/tutorial_capture/ch73_main1.c:25

## xgeSvgDrawPx
- 位置: xge.h:2257  已注释: 否
- 签名: `XGE_API int xgeSvgDrawPx(xge_svg pSvg, xge_rect_t tDst, float fTolerance);`
- 实现: src/xge_svg.c:22044（体 4 行）
- 用法: examples/tutorial_capture/ch75_main1.c:26; examples/xge_shape_ex/main.c:914; examples/xge_svg/main.c:741

## xgeSvgRasterize
- 位置: xge.h:2258  已注释: 否
- 签名: `XGE_API int xgeSvgRasterize(const char* sURI, int iWidth, int iHeight, void* pPixels, int iStride);`
- 实现: src/xge_svg.c:22049（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/xge_svg/main.c:808

## xgeSvgRasterizeMemory
- 位置: xge.h:2259  已注释: 否
- 签名: `XGE_API int xgeSvgRasterizeMemory(const void* pData, int iSize, int iWidth, int iHeight, void* pPixels, int iStride);`
- 实现: src/xge_svg.c:22152（体 4 行）
- 用法: examples/tutorial_capture/ch80_main1.c:31; examples/tutorial_capture/ch80_main1.c:56; examples/tutorial_capture/ch80_main1.c:57

## xgeSvgTextureLoad
- 位置: xge.h:2260  已注释: 否
- 签名: `XGE_API int xgeSvgTextureLoad(xge_texture pTexture, const char* sURI, int iWidth, int iHeight);`
- 实现: src/xge_svg.c:22157（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/xge_svg/main.c:814; examples/xge_svg/main.c:947; examples/xge_svg/main.c:953

## xgeSvgTextureLoadMemory
- 位置: xge.h:2261  已注释: 否
- 签名: `XGE_API int xgeSvgTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize, int iWidth, int iHeight);`
- 实现: src/xge_svg.c:22206（体 4 行）
- 用法: examples/xge_svg/main.c:820; test/test_main.c:6650; test/test_main.c:6654

