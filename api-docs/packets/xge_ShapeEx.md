# 草稿包：xge.h / ShapeEx（205 条 API）

> 生成 2026-09-10 02:42 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeShapeExMatrixIdentity
- 位置: xge.h:2352  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixIdentity(xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:814（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:22; examples/tutorial_capture/ch35_main1.c:43; examples/tutorial_capture/ch45_main1.c:27

## xgeShapeExMatrixMultiply
- 位置: xge.h:2353  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixMultiply(xge_shape_ex_matrix_t* pOut, const xge_shape_ex_matrix_t* pParent, const xge_shape_ex_matrix_t* pLocal);`
- 实现: src/xge_shape_ex.c:823（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:351; test/test_main.c:449; test/test_main.c:451

## xgeShapeExMatrixTranslate
- 位置: xge.h:2354  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixTranslate(xge_shape_ex_matrix_t* pMatrix, float fTX, float fTY);`
- 实现: src/xge_shape_ex.c:832（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:23; examples/tutorial_capture/ch35_main1.c:44; examples/tutorial_capture/ch45_main1.c:28

## xgeShapeExMatrixScale
- 位置: xge.h:2355  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixScale(xge_shape_ex_matrix_t* pMatrix, float fSX, float fSY);`
- 实现: src/xge_shape_ex.c:846（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:46; examples/tutorial_capture/ch45_main1.c:42; examples/tutorial_capture/ch45_main1.c:55

## xgeShapeExMatrixRotate
- 位置: xge.h:2356  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixRotate(xge_shape_ex_matrix_t* pMatrix, float fRadians);`
- 实现: src/xge_shape_ex.c:860（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:24; examples/tutorial_capture/ch35_main1.c:45; examples/tutorial_capture/ch45_main1.c:29

## xgeShapeExMatrixSkew
- 位置: xge.h:2357  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixSkew(xge_shape_ex_matrix_t* pMatrix, float fXRadians, float fYRadians);`
- 实现: src/xge_shape_ex.c:879（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch61_main1.c:53; test/test_main.c:470; test/test_main.c:475

## xgeShapeExMatrixInvert
- 位置: xge.h:2358  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixInvert(xge_shape_ex_matrix_t* pOut, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:945（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:25; test/test_main.c:462; test/test_main.c:466

## xgeShapeExMatrixPoint
- 位置: xge.h:2359  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixPoint(xge_vec2_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_vec2_t tPoint);`
- 实现: src/xge_shape_ex.c:956（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch35_main1.c:54; examples/xge_shape_ex/main.c:355; test/test_main.c:452

## xgeShapeExMatrixVector
- 位置: xge.h:2360  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixVector(xge_vec2_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_vec2_t tVector);`
- 实现: src/xge_shape_ex.c:965（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:356; test/test_main.c:454; test/test_main.c:456

## xgeShapeExMatrixRectBounds
- 位置: xge.h:2361  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixRectBounds(xge_rect_t* pOut, const xge_shape_ex_matrix_t* pMatrix, xge_rect_t tRect);`
- 实现: src/xge_shape_ex.c:975（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:352; test/test_main.c:460

## xgeShapeExMatrixStrokeScale
- 位置: xge.h:2362  已注释: 否
- 签名: `XGE_API int xgeShapeExMatrixStrokeScale(float* pScale, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:999（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:358; test/test_main.c:457; test/test_main.c:459

## xgeShapeExCreate
- 位置: xge.h:2363  已注释: 否
- 签名: `XGE_API int xgeShapeExCreate(xge_shape_ex* ppShape);`
- 实现: src/xge_shape_ex.c:9668（体 37 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_shape_ex_blend/main.c:21; examples/audit_shape_ex_blend/main.c:27; examples/audit_shape_ex_clip_mask/main.c:22

## xgeShapeExAddRef
- 位置: xge.h:2364  已注释: 否
- 签名: `XGE_API int xgeShapeExAddRef(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:9706（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2721

## xgeShapeExRefCountGet
- 位置: xge.h:2365  已注释: 否
- 签名: `XGE_API int xgeShapeExRefCountGet(xge_shape_ex pShape, int* pRefCount);`
- 实现: src/xge_shape_ex.c:9715（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeShapeExIdFromName
- 位置: xge.h:2366  已注释: 否
- 签名: `XGE_API uint32_t xgeShapeExIdFromName(const char* sName);`
- 实现: src/xge_shape_ex.c:9724（体 12 行）
- 用法: examples/tutorial_capture/ch65_main1.c:16; examples/tutorial_capture/ch65_main1.c:23; examples/tutorial_capture/ch77_main1.c:43

## xgeShapeExId
- 位置: xge.h:2367  已注释: 否
- 签名: `XGE_API int xgeShapeExId(xge_shape_ex pShape, uint32_t iId);`
- 实现: src/xge_shape_ex.c:9737（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch65_main1.c:16; examples/tutorial_capture/ch65_main1.c:23; test/test_main.c:256

## xgeShapeExIdGet
- 位置: xge.h:2368  已注释: 否
- 签名: `XGE_API int xgeShapeExIdGet(xge_shape_ex pShape, uint32_t* pId);`
- 实现: src/xge_shape_ex.c:9746（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:211

## xgeShapeExParentGet
- 位置: xge.h:2369  已注释: 否
- 签名: `XGE_API int xgeShapeExParentGet(xge_shape_ex pShape, xge_shape_ex_scene* ppParentScene);`
- 实现: src/xge_shape_ex.c:9755（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2555; test/test_main.c:2562; test/test_main.c:2571

## xgeShapeExClone
- 位置: xge.h:2370  已注释: 否
- 签名: `XGE_API int xgeShapeExClone(xge_shape_ex pShape, xge_shape_ex* ppClone);`
- 实现: src/xge_shape_ex.c:9944（体 6 行）
- 用法: test/test_main.c:389; test/test_main.c:1402; test/test_main.c:1820

## xgeShapeExDestroy
- 位置: xge.h:2371  已注释: 否
- 签名: `XGE_API void xgeShapeExDestroy(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:9951（体 23 行）
- 用法: examples/audit_shape_ex_blend/main.c:25; examples/audit_shape_ex_blend/main.c:32; examples/audit_shape_ex_clip_mask/main.c:27

## xgeShapeExReset
- 位置: xge.h:2372  已注释: 否
- 签名: `XGE_API int xgeShapeExReset(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:9975（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:392; test/test_main.c:523; test/test_main.c:746

## xgeShapeExMoveTo
- 位置: xge.h:2373  已注释: 否
- 签名: `XGE_API int xgeShapeExMoveTo(xge_shape_ex pShape, float fX, float fY);`
- 实现: src/xge_shape_ex.c:9991（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_clip_mask/main.c:54; examples/audit_shape_ex_fill/main.c:73; examples/audit_shape_ex_stroke/main.c:55

## xgeShapeExLineTo
- 位置: xge.h:2374  已注释: 否
- 签名: `XGE_API int xgeShapeExLineTo(xge_shape_ex pShape, float fX, float fY);`
- 实现: src/xge_shape_ex.c:9999（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_clip_mask/main.c:55; examples/audit_shape_ex_clip_mask/main.c:56; examples/audit_shape_ex_clip_mask/main.c:57

## xgeShapeExQuadTo
- 位置: xge.h:2375  已注释: 否
- 签名: `XGE_API int xgeShapeExQuadTo(xge_shape_ex pShape, float fCX, float fCY, float fX, float fY);`
- 实现: src/xge_shape_ex.c:10007（体 27 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_stroke/main.c:184; examples/audit_shape_ex_stroke/main.c:185; examples/tutorial_capture/ch37_main1.c:11

## xgeShapeExCubicTo
- 位置: xge.h:2376  已注释: 否
- 签名: `XGE_API int xgeShapeExCubicTo(xge_shape_ex pShape, float fC1X, float fC1Y, float fC2X, float fC2Y, float fX, float fY);`
- 实现: src/xge_shape_ex.c:10035（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch37_main1.c:21; examples/tutorial_capture/ch37_main1.c:31; examples/tutorial_capture/ch37_main1.c:32

## xgeShapeExArcTo
- 位置: xge.h:2377  已注释: 否
- 签名: `XGE_API int xgeShapeExArcTo(xge_shape_ex pShape, float fRX, float fRY, float fAxisDegrees, int bLargeArc, int bSweep, float fX, float fY);`
- 实现: src/xge_shape_ex.c:10043（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch38_main1.c:12; examples/tutorial_capture/ch38_main1.c:22; examples/tutorial_capture/ch38_main1.c:32

## xgeShapeExClose
- 位置: xge.h:2378  已注释: 否
- 签名: `XGE_API int xgeShapeExClose(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:10061（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_clip_mask/main.c:64; examples/audit_shape_ex_fill/main.c:75; examples/tutorial_capture/ch36_main1.c:13

## xgeShapeExAppendTriangle
- 位置: xge.h:2379  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendTriangle(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, float fX3, float fY3, int bClockwise);`
- 实现: src/xge_shape_ex.c:10081（体 30 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch100_main1.c:32; examples/tutorial_capture/ch100_main1.c:49; examples/tutorial_capture/ch39_main1.c:57

## xgeShapeExAppendLine
- 位置: xge.h:2380  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendLine(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2);`
- 实现: src/xge_shape_ex.c:10112（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch46_main1.c:10; examples/tutorial_capture/ch46_main1.c:17; examples/tutorial_capture/ch46_main1.c:24

## xgeShapeExAppendPolyline
- 位置: xge.h:2381  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendPolyline(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iPointCount);`
- 实现: src/xge_shape_ex.c:10162（体 4 行）
- 用法: examples/tutorial_capture/ch39_main1.c:78; examples/xge_shape_ex/main.c:263; test/test_main.c:710

## xgeShapeExAppendPolygon
- 位置: xge.h:2382  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendPolygon(xge_shape_ex pShape, const xge_vec2_t* pPoints, int iPointCount);`
- 实现: src/xge_shape_ex.c:10167（体 4 行）
- 用法: examples/tutorial_capture/ch39_main1.c:69; examples/tutorial_capture/ch59_main1.c:33; examples/tutorial_capture/ch60_main1.c:44

## xgeShapeExAppendRect
- 位置: xge.h:2383  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendRect(xge_shape_ex pShape, float fX, float fY, float fW, float fH, float fRX, float fRY, int bClockwise);`
- 实现: src/xge_shape_ex.c:10307（体 4 行）
- 用法: examples/audit_shape_ex_blend/main.c:22; examples/audit_shape_ex_blend/main.c:28; examples/audit_shape_ex_clip_mask/main.c:23

## xgeShapeExAppendCapsule
- 位置: xge.h:2384  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendCapsule(xge_shape_ex pShape, float fX, float fY, float fW, float fH, int bClockwise);`
- 实现: src/xge_shape_ex.c:10317（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch39_main1.c:34; examples/xge_shape_ex/main.c:336; test/test_main.c:715

## xgeShapeExAppendCircle
- 位置: xge.h:2385  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendCircle(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, int bClockwise);`
- 实现: src/xge_shape_ex.c:10395（体 4 行）
- 用法: examples/audit_shape_ex_clip_mask/main.c:30; examples/audit_shape_ex_clip_mask/main.c:43; examples/audit_shape_ex_clip_mask/main.c:80

## xgeShapeExAppendEllipse
- 位置: xge.h:2386  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendEllipse(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, int bClockwise);`
- 实现: src/xge_shape_ex.c:10400（体 4 行）
- 用法: examples/tutorial_capture/ch39_main1.c:27; examples/xge_shape_ex/main.c:444; examples/xge_shape_ex/main.c:467

## xgeShapeExAppendArc
- 位置: xge.h:2387  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendArc(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians);`
- 实现: src/xge_shape_ex.c:10410（体 29 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch39_main1.c:41; examples/xge_shape_ex/main.c:303; test/test_main.c:4102

## xgeShapeExAppendPie
- 位置: xge.h:2388  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendPie(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians);`
- 实现: src/xge_shape_ex.c:10440（体 36 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch39_main1.c:50; examples/xge_shape_ex/main.c:312; test/test_main.c:4110

## xgeShapeExAppendChord
- 位置: xge.h:2389  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendChord(xge_shape_ex pShape, float fCX, float fCY, float fRX, float fRY, float fStartRadians, float fEndRadians);`
- 实现: src/xge_shape_ex.c:10477（体 30 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:320; test/test_main.c:4118; test/test_main.c:4424

## xgeShapeExAppendPath
- 位置: xge.h:2390  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendPath(xge_shape_ex pShape, const uint8_t* pCommands, int iCommandCount, const xge_vec2_t* pPoints, int iPointCount);`
- 实现: src/xge_shape_ex.c:10508（体 190 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:178; test/test_main.c:855; test/test_main.c:864

## xgeShapeExAppendSvgPath
- 位置: xge.h:2391  已注释: 否
- 签名: `XGE_API int xgeShapeExAppendSvgPath(xge_shape_ex pShape, const char* sPath);`
- 实现: src/xge_shape_ex.c:11040（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch40_main1.c:12; examples/tutorial_capture/ch40_main1.c:21; examples/tutorial_capture/ch40_main1.c:41

## xgeShapeExGetPath
- 位置: xge.h:2392  已注释: 否
- 签名: `XGE_API int xgeShapeExGetPath(xge_shape_ex pShape, const uint8_t** ppCommands, int* pCommandCount, const xge_vec2_t** ppPoints, int* pPointCount);`
- 实现: src/xge_shape_ex.c:11053（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:529; test/test_main.c:595; test/test_main.c:621

## xgeShapeExGetSvgPathData
- 位置: xge.h:2393  已注释: 否
- 签名: `XGE_API int xgeShapeExGetSvgPathData(xge_shape_ex pShape, char* sBuffer, int iBufferSize, int* pRequiredSize);`
- 实现: src/xge_shape_ex.c:11098（体 70 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_BUFFER_TOO_SMALL, XGE_OK
- 用法: examples/tutorial_capture/ch40_main1.c:36; examples/xge_shape_ex/main.c:227; test/test_main.c:874

## xgeShapeExFillColor
- 位置: xge.h:2394  已注释: 否
- 签名: `XGE_API int xgeShapeExFillColor(xge_shape_ex pShape, uint32_t iColor);`
- 实现: src/xge_shape_ex.c:11169（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_blend/main.c:23; examples/audit_shape_ex_blend/main.c:29; examples/audit_shape_ex_clip_mask/main.c:24

## xgeShapeExFillTypeGet
- 位置: xge.h:2395  已注释: 否
- 签名: `XGE_API int xgeShapeExFillTypeGet(xge_shape_ex pShape, int* pType);`
- 实现: src/xge_shape_ex.c:11177（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1367; test/test_main.c:1435; test/test_main.c:1460

## xgeShapeExFillColorGet
- 位置: xge.h:2396  已注释: 否
- 签名: `XGE_API int xgeShapeExFillColorGet(xge_shape_ex pShape, uint32_t* pColor);`
- 实现: src/xge_shape_ex.c:11186（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:366; test/test_main.c:383; test/test_main.c:393

## xgeShapeExFillLinearGradient
- 位置: xge.h:2397  已注释: 否
- 签名: `XGE_API int xgeShapeExFillLinearGradient(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11195（体 34 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_clip_mask/main.c:139; examples/audit_shape_ex_fill/main.c:137; examples/audit_shape_ex_fill/main.c:144

## xgeShapeExFillLinearGradientGet
- 位置: xge.h:2398  已注释: 否
- 签名: `XGE_API int xgeShapeExFillLinearGradientGet(xge_shape_ex pShape, float* pX1, float* pY1, float* pX2, float* pY2, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11230（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:1369; test/test_main.c:1391; test/test_main.c:1403

## xgeShapeExFillRadialGradient
- 位置: xge.h:2399  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRadialGradient(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11248（体 4 行）
- 用法: examples/audit_shape_ex_fill/main.c:168; examples/audit_shape_ex_fill/main.c:176; examples/tutorial_capture/ch43_main1.c:15

## xgeShapeExFillRadialGradientEx
- 位置: xge.h:2400  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRadialGradientEx(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11253（体 39 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:184; examples/tutorial_capture/ch43_main1.c:47; test/test_main.c:1448

## xgeShapeExFillRadialGradientGet
- 位置: xge.h:2401  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11293（体 4 行）
- 用法: test/test_main.c:1376; test/test_main.c:1424; test/test_main.c:1437

## xgeShapeExFillRadialGradientGetEx
- 位置: xge.h:2402  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRadialGradientGetEx(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, float* pFocalRadius, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11298（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:1439; test/test_main.c:1452

## xgeShapeExFillGradientSpread
- 位置: xge.h:2403  已注释: 否
- 签名: `XGE_API int xgeShapeExFillGradientSpread(xge_shape_ex pShape, int iSpread);`
- 实现: src/xge_shape_ex.c:11318（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:203; examples/audit_shape_ex_fill/main.c:211; examples/audit_shape_ex_fill/main.c:219

## xgeShapeExFillGradientSpreadGet
- 位置: xge.h:2404  已注释: 否
- 签名: `XGE_API int xgeShapeExFillGradientSpreadGet(xge_shape_ex pShape, int* pSpread);`
- 实现: src/xge_shape_ex.c:11332（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1372; test/test_main.c:1443

## xgeShapeExFillGradientTransformSet
- 位置: xge.h:2405  已注释: 否
- 签名: `XGE_API int xgeShapeExFillGradientTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:11341（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch45_main1.c:31; examples/tutorial_capture/ch45_main1.c:44; examples/tutorial_capture/ch45_main1.c:57

## xgeShapeExFillGradientTransformIdentity
- 位置: xge.h:2406  已注释: 否
- 签名: `XGE_API int xgeShapeExFillGradientTransformIdentity(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:11350（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeShapeExFillGradientTransformGet
- 位置: xge.h:2407  已注释: 否
- 签名: `XGE_API int xgeShapeExFillGradientTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:11359（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1374; test/test_main.c:1445

## xgeShapeExStrokeColor
- 位置: xge.h:2408  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeColor(xge_shape_ex pShape, uint32_t iColor);`
- 实现: src/xge_shape_ex.c:11368（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:235; examples/audit_shape_ex_fill/main.c:245; examples/audit_shape_ex_fill/main.c:261

## xgeShapeExStrokeTypeGet
- 位置: xge.h:2409  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeTypeGet(xge_shape_ex pShape, int* pType);`
- 实现: src/xge_shape_ex.c:11377（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1486; test/test_main.c:1525; test/test_main.c:1546

## xgeShapeExStrokeColorGet
- 位置: xge.h:2410  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeColorGet(xge_shape_ex pShape, uint32_t* pColor);`
- 实现: src/xge_shape_ex.c:11386（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: test/test_main.c:368; test/test_main.c:377; test/test_main.c:387

## xgeShapeExStrokeLinearGradient
- 位置: xge.h:2411  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeLinearGradient(xge_shape_ex pShape, float fX1, float fY1, float fX2, float fY2, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11396（体 40 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:186; examples/tutorial_capture/ch46_main1.c:68; examples/tutorial_capture/ch50_main1.c:15

## xgeShapeExStrokeLinearGradientGet
- 位置: xge.h:2412  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeLinearGradientGet(xge_shape_ex pShape, float* pX1, float* pY1, float* pX2, float* pY2, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11437（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:1488; test/test_main.c:1501; test/test_main.c:1532

## xgeShapeExStrokeRadialGradient
- 位置: xge.h:2413  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeRadialGradient(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11455（体 4 行）
- 用法: examples/tutorial_capture/ch50_main1.c:26; examples/xge_shape_ex_gradient_blend/main.c:116; test/test_main.c:1508

## xgeShapeExStrokeRadialGradientEx
- 位置: xge.h:2414  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeRadialGradientEx(xge_shape_ex pShape, float fCX, float fCY, float fRadius, float fFX, float fFY, float fFocalRadius, int iUnits, const xge_shape_ex_color_stop_t* pStops, int iStopCount);`
- 实现: src/xge_shape_ex.c:11460（体 45 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:491; test/test_main.c:1534; test/test_main.c:1540

## xgeShapeExStrokeRadialGradientGet
- 位置: xge.h:2415  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeRadialGradientGet(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11506（体 4 行）
- 用法: test/test_main.c:1515; test/test_main.c:1527; test/test_main.c:1548

## xgeShapeExStrokeRadialGradientGetEx
- 位置: xge.h:2416  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeRadialGradientGetEx(xge_shape_ex pShape, float* pCX, float* pCY, float* pRadius, float* pFX, float* pFY, float* pFocalRadius, int* pUnits, const xge_shape_ex_color_stop_t** ppStops, int* pStopCount);`
- 实现: src/xge_shape_ex.c:11511（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:1529; test/test_main.c:1538

## xgeShapeExStrokeGradientSpread
- 位置: xge.h:2417  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeGradientSpread(xge_shape_ex pShape, int iSpread);`
- 实现: src/xge_shape_ex.c:11531（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch50_main1.c:55; test/test_main.c:1469; test/test_main.c:1470

## xgeShapeExStrokeGradientSpreadGet
- 位置: xge.h:2418  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeGradientSpreadGet(xge_shape_ex pShape, int* pSpread);`
- 实现: src/xge_shape_ex.c:11545（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1491

## xgeShapeExStrokeGradientTransformSet
- 位置: xge.h:2419  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeGradientTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:11554（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1472; test/test_main.c:1477

## xgeShapeExStrokeGradientTransformIdentity
- 位置: xge.h:2420  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeGradientTransformIdentity(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:11563（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeShapeExStrokeGradientTransformGet
- 位置: xge.h:2421  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeGradientTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:11572（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1493

## xgeShapeExStrokeWidth
- 位置: xge.h:2422  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeWidth(xge_shape_ex pShape, float fWidth);`
- 实现: src/xge_shape_ex.c:11581（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:236; examples/audit_shape_ex_fill/main.c:246; examples/audit_shape_ex_fill/main.c:262

## xgeShapeExStrokeWidthGet
- 位置: xge.h:2423  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeWidthGet(xge_shape_ex pShape, float* pWidth);`
- 实现: src/xge_shape_ex.c:11590（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:540; test/test_main.c:1559; test/test_main.c:1571

## xgeShapeExStrokeCap
- 位置: xge.h:2424  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeCap(xge_shape_ex pShape, int iCap);`
- 实现: src/xge_shape_ex.c:11599（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:112; examples/audit_shape_ex_stroke/main.c:137; examples/audit_shape_ex_stroke/main.c:166

## xgeShapeExStrokeCapGet
- 位置: xge.h:2425  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeCapGet(xge_shape_ex pShape, int* pCap);`
- 实现: src/xge_shape_ex.c:11608（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:542; test/test_main.c:1561; test/test_main.c:1625

## xgeShapeExStrokeJoin
- 位置: xge.h:2426  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeJoin(xge_shape_ex pShape, int iJoin);`
- 实现: src/xge_shape_ex.c:11617（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:82; examples/audit_shape_ex_stroke/main.c:93; examples/tutorial_capture/ch36_main1.c:28

## xgeShapeExStrokeJoinGet
- 位置: xge.h:2427  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeJoinGet(xge_shape_ex pShape, int* pJoin);`
- 实现: src/xge_shape_ex.c:11626（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:544; test/test_main.c:1563; test/test_main.c:1627

## xgeShapeExStrokeMiterLimit
- 位置: xge.h:2428  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeMiterLimit(xge_shape_ex pShape, float fLimit);`
- 实现: src/xge_shape_ex.c:11635（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:94; examples/tutorial_capture/ch48_main1.c:17; examples/tutorial_capture/ch48_main1.c:29

## xgeShapeExStrokeMiterLimitGet
- 位置: xge.h:2429  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeMiterLimitGet(xge_shape_ex pShape, float* pLimit);`
- 实现: src/xge_shape_ex.c:11643（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1565; test/test_main.c:1574; test/test_main.c:1577

## xgeShapeExStrokeNonScaling
- 位置: xge.h:2430  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeNonScaling(xge_shape_ex pShape, int bNonScaling);`
- 实现: src/xge_shape_ex.c:11652（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:216; examples/tutorial_capture/ch48_main1.c:38; examples/tutorial_capture/ch48_main1.c:50

## xgeShapeExStrokeNonScalingGet
- 位置: xge.h:2431  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeNonScalingGet(xge_shape_ex pShape, int* pNonScaling);`
- 实现: src/xge_shape_ex.c:11660（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1631

## xgeShapeExStrokeDash
- 位置: xge.h:2432  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeDash(xge_shape_ex pShape, const float* pDashPattern, int iDashCount, float fDashOffset);`
- 实现: src/xge_shape_ex.c:11669（体 36 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/audit_shape_ex_stroke/main.c:138; examples/audit_shape_ex_stroke/main.c:148; examples/tutorial_capture/ch49_main1.c:17

## xgeShapeExStrokeDashGet
- 位置: xge.h:2433  已注释: 否
- 签名: `XGE_API int xgeShapeExStrokeDashGet(xge_shape_ex pShape, const float** ppDashPattern, int* pDashCount, float* pDashOffset);`
- 实现: src/xge_shape_ex.c:11706（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:369; test/test_main.c:1633; test/test_main.c:1636

## xgeShapeExTrimPath
- 位置: xge.h:2434  已注释: 否
- 签名: `XGE_API int xgeShapeExTrimPath(xge_shape_ex pShape, float fBegin, float fEnd, int bSimultaneous);`
- 实现: src/xge_shape_ex.c:11723（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch51_main1.c:23; examples/tutorial_capture/ch51_main1.c:33; examples/tutorial_capture/ch51_main1.c:43

## xgeShapeExTrimPathGet
- 位置: xge.h:2435  已注释: 否
- 签名: `XGE_API int xgeShapeExTrimPathGet(xge_shape_ex pShape, float* pBegin, float* pEnd, int* pSimultaneous, int* pEnabled);`
- 实现: src/xge_shape_ex.c:11743（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:371; test/test_main.c:379; test/test_main.c:402

## xgeShapeExTrimClear
- 位置: xge.h:2436  已注释: 否
- 签名: `XGE_API int xgeShapeExTrimClear(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:11763（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:404; test/test_main.c:4233

## xgeShapeExFillRule
- 位置: xge.h:2437  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRule(xge_shape_ex pShape, int iRule);`
- 实现: src/xge_shape_ex.c:11775（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:94; examples/audit_shape_ex_fill/main.c:102; examples/audit_shape_ex_fill/main.c:110

## xgeShapeExFillRuleGet
- 位置: xge.h:2438  已注释: 否
- 签名: `XGE_API int xgeShapeExFillRuleGet(xge_shape_ex pShape, int* pRule);`
- 实现: src/xge_shape_ex.c:11783（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:546; test/test_main.c:1641

## xgeShapeExPaintOrder
- 位置: xge.h:2439  已注释: 否
- 签名: `XGE_API int xgeShapeExPaintOrder(xge_shape_ex pShape, int bStrokeFirst);`
- 实现: src/xge_shape_ex.c:11792（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_fill/main.c:237; examples/audit_shape_ex_fill/main.c:247; examples/tutorial_capture/ch52_main1.c:50

## xgeShapeExPaintOrderGet
- 位置: xge.h:2440  已注释: 否
- 签名: `XGE_API int xgeShapeExPaintOrderGet(xge_shape_ex pShape, int* pStrokeFirst);`
- 实现: src/xge_shape_ex.c:11800（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1643

## xgeShapeExOpacity
- 位置: xge.h:2441  已注释: 否
- 签名: `XGE_API int xgeShapeExOpacity(xge_shape_ex pShape, float fOpacity);`
- 实现: src/xge_shape_ex.c:11809（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:130; examples/audit_shape_ex_transform/main.c:147; examples/audit_shape_ex_transform/main.c:154

## xgeShapeExOpacityGet
- 位置: xge.h:2442  已注释: 否
- 签名: `XGE_API int xgeShapeExOpacityGet(xge_shape_ex pShape, float* pOpacity);`
- 实现: src/xge_shape_ex.c:11818（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:548; test/test_main.c:1645; test/test_main.c:1648

## xgeShapeExVisible
- 位置: xge.h:2443  已注释: 否
- 签名: `XGE_API int xgeShapeExVisible(xge_shape_ex pShape, int bVisible);`
- 实现: src/xge_shape_ex.c:11827（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch53_main1.c:63; examples/tutorial_capture/ch53_main1.c:70; test/test_main.c:514

## xgeShapeExVisibleGet
- 位置: xge.h:2444  已注释: 否
- 签名: `XGE_API int xgeShapeExVisibleGet(xge_shape_ex pShape, int* pVisible);`
- 实现: src/xge_shape_ex.c:11834（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:550; test/test_main.c:1657; test/test_main.c:1660

## xgeShapeExBlend
- 位置: xge.h:2445  已注释: 否
- 签名: `XGE_API int xgeShapeExBlend(xge_shape_ex pShape, int iBlend);`
- 实现: src/xge_shape_ex.c:11843（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_blend/main.c:30; examples/tutorial_capture/ch54_main1.c:20; examples/tutorial_capture/ch54_main1.c:34

## xgeShapeExBlendClear
- 位置: xge.h:2446  已注释: 否
- 签名: `XGE_API int xgeShapeExBlendClear(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:11853（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1675; test/test_main.c:1679

## xgeShapeExBlendGet
- 位置: xge.h:2447  已注释: 否
- 签名: `XGE_API int xgeShapeExBlendGet(xge_shape_ex pShape, int* pBlend, int* pBlendSet);`
- 实现: src/xge_shape_ex.c:11863（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1663; test/test_main.c:1666; test/test_main.c:1670

## xgeShapeExMaskShapeSet
- 位置: xge.h:2448  已注释: 否
- 签名: `XGE_API int xgeShapeExMaskShapeSet(xge_shape_ex pShape, xge_shape_ex pTarget, int iMethod);`
- 实现: src/xge_shape_ex.c:11877（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_clip_mask/main.c:112; examples/audit_shape_ex_clip_mask/main.c:123; examples/audit_shape_ex_clip_mask/main.c:143

## xgeShapeExMaskSceneSet
- 位置: xge.h:2449  已注释: 否
- 签名: `XGE_API int xgeShapeExMaskSceneSet(xge_shape_ex pShape, xge_shape_ex_scene pTarget, int iMethod);`
- 实现: src/xge_shape_ex.c:11901（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:717; test/test_main.c:2654; test/test_main.c:4612

## xgeShapeExMaskClear
- 位置: xge.h:2450  已注释: 否
- 签名: `XGE_API int xgeShapeExMaskClear(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:11925（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch57_main1.c:39; test/test_main.c:2658; test/test_main.c:4616

## xgeShapeExMaskGet
- 位置: xge.h:2451  已注释: 否
- 签名: `XGE_API int xgeShapeExMaskGet(xge_shape_ex pShape, int* pMethod, int* pTargetType, xge_shape_ex* ppTargetShape, xge_shape_ex_scene* ppTargetScene);`
- 实现: src/xge_shape_ex.c:11932（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch57_main1.c:23; test/test_main.c:556; test/test_main.c:2649

## xgeShapeExMaskCompositeScene
- 位置: xge.h:2452  已注释: 否
- 签名: `XGE_API int xgeShapeExMaskCompositeScene(xge_shape_ex_scene pMaskScene, int iMethod, int iOutputBlend, xge_rect_t tSourceBounds, xge_shape_ex_draw_proc pSourceDraw, void* pSourceUser, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int bScreenSpace);`
- 实现: src/xge_shape_ex.c:15592（体 48 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:4601; test/test_main.c:4602; test/test_main.c:4603

## xgeShapeExGetBounds
- 位置: xge.h:2453  已注释: 否
- 签名: `XGE_API int xgeShapeExGetBounds(xge_shape_ex pShape, float fTolerance, xge_rect_t* pBounds);`
- 实现: src/xge_shape_ex.c:13064（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE
- 用法: test/test_main.c:331; test/test_main.c:339; test/test_main.c:604

## xgeShapeExGetOBB
- 位置: xge.h:2454  已注释: 否
- 签名: `XGE_API int xgeShapeExGetOBB(xge_shape_ex pShape, float fTolerance, xge_vec2_t* pPoints4);`
- 实现: src/xge_shape_ex.c:13078（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE
- 用法: test/test_main.c:334; test/test_main.c:2779; test/test_main.c:2926

## xgeShapeExBoundsIntersects
- 位置: xge.h:2455  已注释: 否
- 签名: `XGE_API int xgeShapeExBoundsIntersects(xge_shape_ex pShape, xge_rect_t tRect, float fTolerance, int* pIntersects);`
- 实现: src/xge_shape_ex.c:13090（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2793; test/test_main.c:2795; test/test_main.c:2797

## xgeShapeExIntersects
- 位置: xge.h:2456  已注释: 否
- 签名: `XGE_API int xgeShapeExIntersects(xge_shape_ex pShape, xge_rect_t tRect, float fTolerance, int* pIntersects);`
- 实现: src/xge_shape_ex.c:13110（体 4 行）
- 用法: test/test_main.c:2859; test/test_main.c:2860; test/test_main.c:2861

## xgeShapeExIntersectsEx
- 位置: xge.h:2457  已注释: 否
- 签名: `XGE_API int xgeShapeExIntersectsEx(xge_shape_ex pShape, xge_rect_t tRect, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pIntersects);`
- 实现: src/xge_shape_ex.c:13115（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2871; test/test_main.c:2873

## xgeShapeExContainsPoint
- 位置: xge.h:2458  已注释: 否
- 签名: `XGE_API int xgeShapeExContainsPoint(xge_shape_ex pShape, float fX, float fY, float fTolerance, int* pContains);`
- 实现: src/xge_shape_ex.c:13140（体 4 行）
- 用法: test/test_main.c:642; test/test_main.c:951; test/test_main.c:955

## xgeShapeExContainsPointEx
- 位置: xge.h:2459  已注释: 否
- 签名: `XGE_API int xgeShapeExContainsPointEx(xge_shape_ex pShape, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pContains);`
- 实现: src/xge_shape_ex.c:13145（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:1032; test/test_main.c:1033

## xgeShapeExGetLength
- 位置: xge.h:2460  已注释: 否
- 签名: `XGE_API int xgeShapeExGetLength(xge_shape_ex pShape, float fTolerance, float* pLength);`
- 实现: src/xge_shape_ex.c:13160（体 14 行）
- 用法: test/test_main.c:637; test/test_main.c:1684; test/test_main.c:4149

## xgeShapeExGetPointAtLength
- 位置: xge.h:2461  已注释: 否
- 签名: `XGE_API int xgeShapeExGetPointAtLength(xge_shape_ex pShape, float fDistance, float fTolerance, xge_vec2_t* pPoint, xge_vec2_t* pTangent);`
- 实现: src/xge_shape_ex.c:13175（体 14 行）
- 用法: test/test_main.c:4154; test/test_main.c:4163

## xgeShapeExPathMeasureCreate
- 位置: xge.h:2462  已注释: 否
- 签名: `XGE_API int xgeShapeExPathMeasureCreate(xge_shape_ex_path_measure* ppMeasure, xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix, float fTolerance);`
- 实现: src/xge_shape_ex.c:13190（体 28 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/xge_shape_ex/main.c:121; test/test_main.c:1692; test/test_main.c:1707

## xgeShapeExPathMeasureDestroy
- 位置: xge.h:2463  已注释: 否
- 签名: `XGE_API void xgeShapeExPathMeasureDestroy(xge_shape_ex_path_measure pMeasure);`
- 实现: src/xge_shape_ex.c:13219（体 10 行）
- 用法: examples/xge_shape_ex/main.c:125; examples/xge_shape_ex/main.c:136; test/test_main.c:1695

## xgeShapeExPathMeasureGetLength
- 位置: xge.h:2464  已注释: 否
- 签名: `XGE_API int xgeShapeExPathMeasureGetLength(xge_shape_ex_path_measure pMeasure, float* pLength);`
- 实现: src/xge_shape_ex.c:13230（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/xge_shape_ex/main.c:124; test/test_main.c:1694; test/test_main.c:1708

## xgeShapeExPathMeasureGetPointAtLength
- 位置: xge.h:2465  已注释: 否
- 签名: `XGE_API int xgeShapeExPathMeasureGetPointAtLength(xge_shape_ex_path_measure pMeasure, float fDistance, xge_vec2_t* pPoint, xge_vec2_t* pTangent);`
- 实现: src/xge_shape_ex.c:13239（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_shape_ex/main.c:132; test/test_main.c:1702; test/test_main.c:1709

## xgeShapeExClipRectSet
- 位置: xge.h:2466  已注释: 否
- 签名: `XGE_API int xgeShapeExClipRectSet(xge_shape_ex pShape, xge_rect_t tRect);`
- 实现: src/xge_shape_ex.c:13247（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_clip_mask/main.c:25; examples/audit_shape_ex_clip_mask/main.c:32; examples/tutorial_capture/ch58_main1.c:14

## xgeShapeExClipRectGet
- 位置: xge.h:2467  已注释: 否
- 签名: `XGE_API int xgeShapeExClipRectGet(xge_shape_ex pShape, xge_rect_t* pRect, int* pEnabled);`
- 实现: src/xge_shape_ex.c:13257（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1729; test/test_main.c:1731; test/test_main.c:1732

## xgeShapeExClipShapeAdd
- 位置: xge.h:2468  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeAdd(xge_shape_ex pShape, xge_shape_ex pClipShape);`
- 实现: src/xge_shape_ex.c:13275（体 4 行）
- 用法: examples/audit_shape_ex_clip_mask/main.c:48; examples/audit_shape_ex_clip_mask/main.c:68; examples/tutorial_capture/ch59_main1.c:17

## xgeShapeExClipShapeAddEx
- 位置: xge.h:2469  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeAddEx(xge_shape_ex pShape, xge_shape_ex pClipShape, int iMode);`
- 实现: src/xge_shape_ex.c:13280（体 4 行）
- 用法: examples/audit_shape_ex_clip_mask/main.c:84; examples/audit_shape_ex_clip_mask/main.c:94; examples/tutorial_capture/ch59_main1.c:69

## xgeShapeExClipShapeGetCount
- 位置: xge.h:2470  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeGetCount(xge_shape_ex pShape, int* pCount);`
- 实现: src/xge_shape_ex.c:13285（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:554; test/test_main.c:1773; test/test_main.c:1807

## xgeShapeExClipShapeGetAt
- 位置: xge.h:2471  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeGetAt(xge_shape_ex pShape, int iIndex, xge_shape_ex* ppClipShape);`
- 实现: src/xge_shape_ex.c:13294（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:555; test/test_main.c:1781; test/test_main.c:1794

## xgeShapeExClipShapeGetAtEx
- 位置: xge.h:2472  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeGetAtEx(xge_shape_ex pShape, int iIndex, xge_shape_ex* ppClipShape, int* pMode);`
- 实现: src/xge_shape_ex.c:13307（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:1789; test/test_main.c:1802; test/test_main.c:1815

## xgeShapeExClipShapeClear
- 位置: xge.h:2473  已注释: 否
- 签名: `XGE_API int xgeShapeExClipShapeClear(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:13330（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1845; test/test_main.c:1859; test/test_main.c:1889

## xgeShapeExClipClear
- 位置: xge.h:2474  已注释: 否
- 签名: `XGE_API int xgeShapeExClipClear(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:13337（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:1050; test/test_main.c:1074; test/test_main.c:1091

## xgeShapeExStencilClipBegin
- 位置: xge.h:2475  已注释: 否
- 签名: `XGE_API int xgeShapeExStencilClipBegin(xge_shape_ex pClipShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pApplied);`
- 实现: src/xge_shape_ex.c:13346（体 4 行）
- 用法: examples/tutorial_capture/ch60_main1.c:15; examples/tutorial_capture/ch60_main1.c:47; examples/tutorial_capture/ch60_main1.c:70

## xgeShapeExStencilClipBeginPx
- 位置: xge.h:2476  已注释: 否
- 签名: `XGE_API int xgeShapeExStencilClipBeginPx(xge_shape_ex pClipShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pApplied);`
- 实现: src/xge_shape_ex.c:13351（体 4 行）
- 用法: examples/xge_shape_ex/main.c:668; examples/xge_shape_ex/main.c:733; test/test_main.c:1748

## xgeShapeExStencilClipEnd
- 位置: xge.h:2477  已注释: 否
- 签名: `XGE_API int xgeShapeExStencilClipEnd(int bApplied, int iRet);`
- 实现: src/xge_shape_ex.c:13356（体 4 行）
- 用法: examples/tutorial_capture/ch60_main1.c:30; examples/tutorial_capture/ch60_main1.c:62; examples/tutorial_capture/ch60_main1.c:83

## xgeShapeExTransformSet
- 位置: xge.h:2478  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformSet(xge_shape_ex pShape, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:13361（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_stroke/main.c:207; examples/audit_shape_ex_stroke/main.c:219; examples/tutorial_capture/ch35_main1.c:30

## xgeShapeExTransformIdentity
- 位置: xge.h:2479  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformIdentity(xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:13369（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch34_main1.c:14; examples/tutorial_capture/ch34_main1.c:23; examples/tutorial_capture/ch34_main1.c:33

## xgeShapeExTransformGet
- 位置: xge.h:2480  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformGet(xge_shape_ex pShape, xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:13377（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:552; test/test_main.c:1928; test/test_main.c:1930

## xgeShapeExTransformTranslate
- 位置: xge.h:2481  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformTranslate(xge_shape_ex pShape, float fTX, float fTY);`
- 实现: src/xge_shape_ex.c:13386（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:32; examples/audit_shape_ex_transform/main.c:39; examples/audit_shape_ex_transform/main.c:47

## xgeShapeExTransformScale
- 位置: xge.h:2482  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformScale(xge_shape_ex pShape, float fSX, float fSY);`
- 实现: src/xge_shape_ex.c:13396（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:40; examples/tutorial_capture/ch34_main1.c:35; examples/tutorial_capture/ch48_main1.c:41

## xgeShapeExTransformRotate
- 位置: xge.h:2483  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformRotate(xge_shape_ex pShape, float fRadians);`
- 实现: src/xge_shape_ex.c:13406（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:48; examples/tutorial_capture/ch34_main1.c:25; examples/tutorial_capture/ch61_main1.c:25

## xgeShapeExTransformSkew
- 位置: xge.h:2484  已注释: 否
- 签名: `XGE_API int xgeShapeExTransformSkew(xge_shape_ex pShape, float fXRadians, float fYRadians);`
- 实现: src/xge_shape_ex.c:13415（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:56; examples/tutorial_capture/ch35_main1.c:17; test/test_main.c:1974

## xgeShapeExDraw
- 位置: xge.h:2485  已注释: 否
- 签名: `XGE_API int xgeShapeExDraw(xge_shape_ex pShape, float fTolerance);`
- 实现: src/xge_shape_ex.c:16009（体 4 行）
- 用法: examples/audit_shape_ex_blend/main.c:24; examples/audit_shape_ex_blend/main.c:31; examples/audit_shape_ex_clip_mask/main.c:26

## xgeShapeExDrawPx
- 位置: xge.h:2486  已注释: 否
- 签名: `XGE_API int xgeShapeExDrawPx(xge_shape_ex pShape, float fTolerance);`
- 实现: src/xge_shape_ex.c:16014（体 4 行）
- 用法: examples/tutorial_capture/ch62_main1.c:21; examples/xge_shape_ex/main.c:101; examples/xge_shape_ex/main.c:109

## xgeShapeExDrawEx
- 位置: xge.h:2487  已注释: 否
- 签名: `XGE_API int xgeShapeExDrawEx(xge_shape_ex pShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity);`
- 实现: src/xge_shape_ex.c:16019（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch62_main1.c:31; test/test_main.c:2074; test/test_main.c:2081

## xgeShapeExDrawPxEx
- 位置: xge.h:2488  已注释: 否
- 签名: `XGE_API int xgeShapeExDrawPxEx(xge_shape_ex pShape, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity);`
- 实现: src/xge_shape_ex.c:16032（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_shape_ex/main.c:363; test/test_main.c:2075; test/test_main.c:2082

## xgeShapeExSceneCreate
- 位置: xge.h:2489  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneCreate(xge_shape_ex_scene* ppScene);`
- 实现: src/xge_shape_ex.c:16181（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_shape_ex_effect/main.c:23; examples/audit_shape_ex_effect/main.c:74; examples/audit_shape_ex_transform/main.c:67

## xgeShapeExSceneAddRef
- 位置: xge.h:2490  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneAddRef(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16204（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2723

## xgeShapeExSceneRefCountGet
- 位置: xge.h:2491  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneRefCountGet(xge_shape_ex_scene pScene, int* pRefCount);`
- 实现: src/xge_shape_ex.c:16211（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeShapeExSceneId
- 位置: xge.h:2492  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneId(xge_shape_ex_scene pScene, uint32_t iId);`
- 实现: src/xge_shape_ex.c:16220（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:255; test/test_main.c:257; test/test_main.c:259

## xgeShapeExSceneIdGet
- 位置: xge.h:2493  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneIdGet(xge_shape_ex_scene pScene, uint32_t* pId);`
- 实现: src/xge_shape_ex.c:16229（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:209

## xgeShapeExSceneParentGet
- 位置: xge.h:2494  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneParentGet(xge_shape_ex_scene pScene, xge_shape_ex_scene* ppParentScene);`
- 实现: src/xge_shape_ex.c:16238（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2558; test/test_main.c:2564; test/test_main.c:2575

## xgeShapeExSceneClone
- 位置: xge.h:2495  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClone(xge_shape_ex_scene pScene, xge_shape_ex_scene* ppClone);`
- 实现: src/xge_shape_ex.c:16247（体 101 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch63_main1.c:41; examples/xge_shape_ex_effects/main.c:155; test/test_main.c:2063

## xgeShapeExSceneDestroy
- 位置: xge.h:2496  已注释: 否
- 签名: `XGE_API void xgeShapeExSceneDestroy(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16349（体 23 行）
- 用法: examples/audit_shape_ex_effect/main.c:52; examples/audit_shape_ex_effect/main.c:85; examples/audit_shape_ex_transform/main.c:87

## xgeShapeExSceneClear
- 位置: xge.h:2497  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16373（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch63_main1.c:50; test/test_main.c:2276; test/test_main.c:2373

## xgeShapeExSceneAdd
- 位置: xge.h:2498  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneAdd(xge_shape_ex_scene pScene, xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:16517（体 5 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:27; examples/audit_shape_ex_effect/main.c:78; examples/audit_shape_ex_transform/main.c:74

## xgeShapeExSceneAddScene
- 位置: xge.h:2499  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneAddScene(xge_shape_ex_scene pScene, xge_shape_ex_scene pChildScene);`
- 实现: src/xge_shape_ex.c:16523（体 5 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_transform/main.c:84; examples/audit_shape_ex_transform/main.c:104; examples/audit_shape_ex_transform/main.c:106

## xgeShapeExSceneInsert
- 位置: xge.h:2500  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneInsert(xge_shape_ex_scene pScene, xge_shape_ex pShape, xge_shape_ex pBefore);`
- 实现: src/xge_shape_ex.c:16529（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND
- 用法: examples/tutorial_capture/ch64_main1.c:26; test/test_main.c:3239; test/test_main.c:3240

## xgeShapeExSceneInsertShapeAt
- 位置: xge.h:2501  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneInsertShapeAt(xge_shape_ex_scene pScene, xge_shape_ex pShape, int iIndex);`
- 实现: src/xge_shape_ex.c:16497（体 9 行）
- 用法: examples/tutorial_capture/ch64_main1.c:32; test/test_main.c:2452; test/test_main.c:2468

## xgeShapeExSceneInsertSceneAt
- 位置: xge.h:2502  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneInsertSceneAt(xge_shape_ex_scene pScene, xge_shape_ex_scene pChildScene, int iIndex);`
- 实现: src/xge_shape_ex.c:16507（体 9 行）
- 用法: test/test_main.c:2469

## xgeShapeExSceneRemove
- 位置: xge.h:2503  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneRemove(xge_shape_ex_scene pScene, xge_shape_ex pShape);`
- 实现: src/xge_shape_ex.c:16548（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND
- 用法: examples/tutorial_capture/ch64_main1.c:37; test/test_main.c:2467; test/test_main.c:2570

## xgeShapeExSceneRemoveScene
- 位置: xge.h:2504  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneRemoveScene(xge_shape_ex_scene pScene, xge_shape_ex_scene pChildScene);`
- 实现: src/xge_shape_ex.c:16566（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND
- 用法: test/test_main.c:2525; test/test_main.c:2526; test/test_main.c:2574

## xgeShapeExSceneGetCount
- 位置: xge.h:2505  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneGetCount(xge_shape_ex_scene pScene, int* pCount);`
- 实现: src/xge_shape_ex.c:16584（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch63_main1.c:33; examples/tutorial_capture/ch65_main1.c:34; test/test_main.c:2106

## xgeShapeExSceneGetAt
- 位置: xge.h:2506  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppShape);`
- 实现: src/xge_shape_ex.c:16593（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: examples/tutorial_capture/ch65_main1.c:39; test/test_main.c:2108; test/test_main.c:2110

## xgeShapeExSceneChildGetAt
- 位置: xge.h:2507  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneChildGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex_scene_child_t* pChild);`
- 实现: src/xge_shape_ex.c:16607（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: examples/tutorial_capture/ch65_main1.c:57; test/test_main.c:2459; test/test_main.c:2461

## xgeShapeExSceneTraverse
- 位置: xge.h:2508  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTraverse(xge_shape_ex_scene pScene, xge_shape_ex_scene_visit_proc onPaint, void* pUser);`
- 实现: src/xge_shape_ex.c:16646（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:246; test/test_main.c:254; test/test_main.c:271

## xgeShapeExSceneTransformSet
- 位置: xge.h:2509  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformSet(xge_shape_ex_scene pScene, const xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:16661（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_effect/main.c:50; examples/audit_shape_ex_effect/main.c:83; examples/audit_shape_ex_transform/main.c:162

## xgeShapeExSceneTransformIdentity
- 位置: xge.h:2510  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformIdentity(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16669（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch62_main1.c:57; examples/tutorial_capture/ch63_main1.c:42; examples/tutorial_capture/ch64_main1.c:39

## xgeShapeExSceneTransformGet
- 位置: xge.h:2511  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformGet(xge_shape_ex_scene pScene, xge_shape_ex_matrix_t* pMatrix);`
- 实现: src/xge_shape_ex.c:16677（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2055; test/test_main.c:2124; test/test_main.c:2129

## xgeShapeExSceneTransformTranslate
- 位置: xge.h:2512  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformTranslate(xge_shape_ex_scene pScene, float fTX, float fTY);`
- 实现: src/xge_shape_ex.c:16686（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:78; examples/audit_shape_ex_transform/main.c:82; examples/audit_shape_ex_transform/main.c:101

## xgeShapeExSceneTransformScale
- 位置: xge.h:2513  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformScale(xge_shape_ex_scene pScene, float fSX, float fSY);`
- 实现: src/xge_shape_ex.c:16696（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:83; examples/tutorial_capture/ch66_main1.c:37; examples/tutorial_capture/ch66_main1.c:50

## xgeShapeExSceneTransformRotate
- 位置: xge.h:2514  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformRotate(xge_shape_ex_scene pScene, float fRadians);`
- 实现: src/xge_shape_ex.c:16706（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:79; examples/audit_shape_ex_transform/main.c:102; examples/tutorial_capture/ch66_main1.c:31

## xgeShapeExSceneTransformSkew
- 位置: xge.h:2515  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneTransformSkew(xge_shape_ex_scene pScene, float fXRadians, float fYRadians);`
- 实现: src/xge_shape_ex.c:16715（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/tutorial_capture/ch66_main1.c:43; test/test_main.c:2136; test/test_main.c:2149

## xgeShapeExSceneOpacity
- 位置: xge.h:2516  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneOpacity(xge_shape_ex_scene pScene, float fOpacity);`
- 实现: src/xge_shape_ex.c:16725（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_shape_ex_transform/main.c:159; examples/tutorial_capture/ch62_main1.c:59; examples/tutorial_capture/ch63_main1.c:44

## xgeShapeExSceneOpacityGet
- 位置: xge.h:2517  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneOpacityGet(xge_shape_ex_scene pScene, float* pOpacity);`
- 实现: src/xge_shape_ex.c:16734（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2112; test/test_main.c:2115; test/test_main.c:2118

## xgeShapeExSceneVisible
- 位置: xge.h:2518  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneVisible(xge_shape_ex_scene pScene, int bVisible);`
- 实现: src/xge_shape_ex.c:16743（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch67_main1.c:40; examples/tutorial_capture/ch67_main1.c:44; examples/tutorial_capture/ch67_main1.c:48

## xgeShapeExSceneVisibleGet
- 位置: xge.h:2519  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneVisibleGet(xge_shape_ex_scene pScene, int* pVisible);`
- 实现: src/xge_shape_ex.c:16752（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2151; test/test_main.c:2285

## xgeShapeExSceneBlend
- 位置: xge.h:2520  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneBlend(xge_shape_ex_scene pScene, int iBlend);`
- 实现: src/xge_shape_ex.c:16761（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch67_main1.c:54; test/test_main.c:2156; test/test_main.c:2159

## xgeShapeExSceneBlendClear
- 位置: xge.h:2521  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneBlendClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16771（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch67_main1.c:58; test/test_main.c:2162; test/test_main.c:2166

## xgeShapeExSceneBlendGet
- 位置: xge.h:2522  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneBlendGet(xge_shape_ex_scene pScene, int* pBlend, int* pBlendSet);`
- 实现: src/xge_shape_ex.c:16781（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2154; test/test_main.c:2157; test/test_main.c:2163

## xgeShapeExSceneMaskShapeSet
- 位置: xge.h:2523  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneMaskShapeSet(xge_shape_ex_scene pScene, xge_shape_ex pTarget, int iMethod);`
- 实现: src/xge_shape_ex.c:16795（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch57_main1.c:64; examples/tutorial_capture/ch68_main1.c:27; examples/tutorial_capture/ch68_main1.c:72

## xgeShapeExSceneMaskSceneSet
- 位置: xge.h:2524  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneMaskSceneSet(xge_shape_ex_scene pScene, xge_shape_ex_scene pTarget, int iMethod);`
- 实现: src/xge_shape_ex.c:16819（体 23 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch68_main1.c:53; examples/xge_shape_ex/main.c:726; test/test_main.c:2712

## xgeShapeExSceneMaskClear
- 位置: xge.h:2525  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneMaskClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16843（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch68_main1.c:32; test/test_main.c:2729; test/test_main.c:2733

## xgeShapeExSceneMaskGet
- 位置: xge.h:2526  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneMaskGet(xge_shape_ex_scene pScene, int* pMethod, int* pTargetType, xge_shape_ex* ppTargetShape, xge_shape_ex_scene* ppTargetScene);`
- 实现: src/xge_shape_ex.c:16850（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2722; test/test_main.c:4631; test/test_main.c:4634

## xgeShapeExSceneClipRectSet
- 位置: xge.h:2527  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipRectSet(xge_shape_ex_scene pScene, xge_rect_t tRect);`
- 实现: src/xge_shape_ex.c:16863（体 9 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch69_main1.c:26; examples/xge_shape_ex/main.c:463; test/test_main.c:2177

## xgeShapeExSceneClipRectGet
- 位置: xge.h:2528  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipRectGet(xge_shape_ex_scene pScene, xge_rect_t* pRect, int* pEnabled);`
- 实现: src/xge_shape_ex.c:16873（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2178; test/test_main.c:2180; test/test_main.c:2223

## xgeShapeExSceneClipShapeAdd
- 位置: xge.h:2529  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeAdd(xge_shape_ex_scene pScene, xge_shape_ex pClipShape);`
- 实现: src/xge_shape_ex.c:16891（体 4 行）
- 用法: examples/tutorial_capture/ch69_main1.c:37; examples/xge_shape_ex/main.c:657; test/test_main.c:2188

## xgeShapeExSceneClipShapeAddEx
- 位置: xge.h:2530  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeAddEx(xge_shape_ex_scene pScene, xge_shape_ex pClipShape, int iMode);`
- 实现: src/xge_shape_ex.c:16896（体 4 行）
- 用法: examples/tutorial_capture/ch69_main1.c:55; examples/xge_shape_ex/main.c:469; test/test_main.c:2214

## xgeShapeExSceneClipShapeGetCount
- 位置: xge.h:2531  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeGetCount(xge_shape_ex_scene pScene, int* pCount);`
- 实现: src/xge_shape_ex.c:16901（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2192; test/test_main.c:2206; test/test_main.c:2225

## xgeShapeExSceneClipShapeGetAt
- 位置: xge.h:2532  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppClipShape);`
- 实现: src/xge_shape_ex.c:16910（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:2194; test/test_main.c:2204; test/test_main.c:2207

## xgeShapeExSceneClipShapeGetAtEx
- 位置: xge.h:2533  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeGetAtEx(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex* ppClipShape, int* pMode);`
- 实现: src/xge_shape_ex.c:16923（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:2199; test/test_main.c:2211; test/test_main.c:2213

## xgeShapeExSceneClipShapeClear
- 位置: xge.h:2534  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipShapeClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16946（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2246; test/test_main.c:2735; test/test_main.c:3737

## xgeShapeExSceneClipClear
- 位置: xge.h:2535  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneClipClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16953（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch69_main1.c:32; examples/tutorial_capture/ch69_main1.c:43; test/test_main.c:2252

## xgeShapeExSceneEffectClear
- 位置: xge.h:2536  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectClear(xge_shape_ex_scene pScene);`
- 实现: src/xge_shape_ex.c:16962（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch70_main1.c:28; examples/tutorial_capture/ch70_main1.c:34; examples/tutorial_capture/ch70_main1.c:40

## xgeShapeExSceneEffectGaussianBlur
- 位置: xge.h:2537  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectGaussianBlur(xge_shape_ex_scene pScene, float fSigma, int iDirection, int iBorder, int iQuality);`
- 实现: src/xge_shape_ex.c:16969（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:32; examples/audit_shape_ex_effect/main.c:80; examples/tutorial_capture/ch70_main1.c:29

## xgeShapeExSceneEffectDropShadow
- 位置: xge.h:2538  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectDropShadow(xge_shape_ex_scene pScene, uint32_t iColor, float fAngleDegrees, float fDistance, float fSigma, int iQuality);`
- 实现: src/xge_shape_ex.c:16987（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:35; examples/tutorial_capture/ch71_main1.c:19; examples/tutorial_capture/ch71_main1.c:26

## xgeShapeExSceneEffectFill
- 位置: xge.h:2539  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectFill(xge_shape_ex_scene pScene, uint32_t iColor);`
- 实现: src/xge_shape_ex.c:17005（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:38; examples/tutorial_capture/ch71_main1.c:32; examples/xge_shape_ex/main.c:780

## xgeShapeExSceneEffectTint
- 位置: xge.h:2540  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectTint(xge_shape_ex_scene pScene, uint32_t iBlackColor, uint32_t iWhiteColor, float fIntensity);`
- 实现: src/xge_shape_ex.c:17016（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:41; examples/tutorial_capture/ch71_main1.c:38; examples/xge_shape_ex/main.c:782

## xgeShapeExSceneEffectTritone
- 位置: xge.h:2541  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectTritone(xge_shape_ex_scene pScene, uint32_t iShadowColor, uint32_t iMidtoneColor, uint32_t iHighlightColor, int iBlend);`
- 实现: src/xge_shape_ex.c:17032（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/audit_shape_ex_effect/main.c:44; examples/tutorial_capture/ch71_main1.c:45; examples/xge_shape_ex/main.c:784

## xgeShapeExSceneEffectGetCount
- 位置: xge.h:2542  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectGetCount(xge_shape_ex_scene pScene, int* pCount);`
- 实现: src/xge_shape_ex.c:17048（体 6 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2313; test/test_main.c:2349; test/test_main.c:2371

## xgeShapeExSceneEffectGetAt
- 位置: xge.h:2543  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneEffectGetAt(xge_shape_ex_scene pScene, int iIndex, xge_shape_ex_scene_effect_t* pEffect);`
- 实现: src/xge_shape_ex.c:17055（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_main.c:2356; test/test_main.c:2360; test/test_main.c:2362

## xgeShapeExSceneGetBounds
- 位置: xge.h:2544  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneGetBounds(xge_shape_ex_scene pScene, float fTolerance, xge_rect_t* pBounds);`
- 实现: src/xge_shape_ex.c:17286（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE
- 用法: examples/tutorial_capture/ch72_main1.c:27; test/test_main.c:2316; test/test_main.c:2323

## xgeShapeExSceneGetOBB
- 位置: xge.h:2545  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneGetOBB(xge_shape_ex_scene pScene, float fTolerance, xge_vec2_t* pPoints4);`
- 实现: src/xge_shape_ex.c:17311（体 49 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/tutorial_capture/ch72_main1.c:74; test/test_main.c:2478; test/test_main.c:2513

## xgeShapeExSceneBoundsIntersects
- 位置: xge.h:2546  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneBoundsIntersects(xge_shape_ex_scene pScene, xge_rect_t tRect, float fTolerance, int* pIntersects);`
- 实现: src/xge_shape_ex.c:17467（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:2832; test/test_main.c:2834

## xgeShapeExSceneIntersects
- 位置: xge.h:2547  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneIntersects(xge_shape_ex_scene pScene, xge_rect_t tRect, float fTolerance, int* pIntersects);`
- 实现: src/xge_shape_ex.c:17487（体 4 行）
- 用法: examples/tutorial_capture/ch72_main1.c:39; test/test_main.c:2899; test/test_main.c:2900

## xgeShapeExSceneIntersectsEx
- 位置: xge.h:2548  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneIntersectsEx(xge_shape_ex_scene pScene, xge_rect_t tRect, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pIntersects);`
- 实现: src/xge_shape_ex.c:17492（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeShapeExSceneContainsPoint
- 位置: xge.h:2549  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneContainsPoint(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, int* pContains);`
- 实现: src/xge_shape_ex.c:17517（体 4 行）
- 用法: examples/tutorial_capture/ch72_main1.c:50; examples/tutorial_capture/ch72_main1.c:54; test/test_main.c:3394

## xgeShapeExSceneContainsPointEx
- 位置: xge.h:2550  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneContainsPointEx(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, int* pContains);`
- 实现: src/xge_shape_ex.c:17522（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_main.c:3456

## xgeShapeExSceneHitTest
- 位置: xge.h:2551  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneHitTest(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, xge_shape_ex* ppShape);`
- 实现: src/xge_shape_ex.c:17539（体 4 行）
- 用法: examples/tutorial_capture/ch72_main1.c:61; test/test_main.c:2481; test/test_main.c:3409

## xgeShapeExSceneHitTestEx
- 位置: xge.h:2552  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneHitTestEx(xge_shape_ex_scene pScene, float fX, float fY, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, xge_shape_ex* ppShape);`
- 实现: src/xge_shape_ex.c:17544（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:3455; test/test_main.c:3457; test/test_main.c:3461

## xgeShapeExSceneDraw
- 位置: xge.h:2553  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneDraw(xge_shape_ex_scene pScene, float fTolerance);`
- 实现: src/xge_shape_ex.c:18330（体 4 行）
- 用法: examples/audit_shape_ex_effect/main.c:51; examples/audit_shape_ex_effect/main.c:84; examples/audit_shape_ex_transform/main.c:86

## xgeShapeExSceneDrawPx
- 位置: xge.h:2554  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneDrawPx(xge_shape_ex_scene pScene, float fTolerance);`
- 实现: src/xge_shape_ex.c:18335（体 4 行）
- 用法: examples/xge_shape_ex/main.c:473; examples/xge_shape_ex/main.c:658; examples/xge_shape_ex/main.c:727

## xgeShapeExSceneDrawEx
- 位置: xge.h:2555  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneDrawEx(xge_shape_ex_scene pScene, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity);`
- 实现: src/xge_shape_ex.c:18340（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch62_main1.c:84; test/test_main.c:2265; test/test_main.c:2272

## xgeShapeExSceneDrawPxEx
- 位置: xge.h:2556  已注释: 否
- 签名: `XGE_API int xgeShapeExSceneDrawPxEx(xge_shape_ex_scene pScene, float fTolerance, const xge_shape_ex_matrix_t* pParentMatrix, float fParentOpacity);`
- 实现: src/xge_shape_ex.c:18353（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:2266; test/test_main.c:2273

