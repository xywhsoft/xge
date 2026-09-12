# 草稿包：xui.h / splitlayout（35 条 API）

> 生成 2026-09-10 03:05 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiSplitLayoutGetType
- 位置: xui.h:8849  已注释: 否
- 签名: `XUI_API xui_widget_type xuiSplitLayoutGetType(xui_context pContext);`
- 实现: src/xui_split_layout.c:1329（体 34 行）
- 返回码: NULL

## xuiSplitLayoutCreate
- 位置: xui.h:8850  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutCreate(xui_context pContext, xui_widget* ppWidget, const xui_split_layout_desc_t* pDesc);`
- 实现: src/xui_split_layout.c:1364（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_split_layout/main.c:207; examples/xui_split_layout/main.c:248; examples/xui_split_layout_dock_repro/main.c:337

## xuiSplitLayoutSetChange
- 位置: xui.h:8851  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetChange(xui_widget pWidget, xui_split_layout_change_proc onChange, void* pUser);`
- 实现: src/xui_split_layout.c:1379（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_split_layout/main.c:215; examples/xui_split_layout/main.c:256; examples/xui_split_layout_dock_repro/main.c:341

## xuiSplitLayoutSetOrientation
- 位置: xui.h:8852  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_split_layout.c:1390（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_split_layout_test.c:309; test_xui/xui_split_layout_test.c:423

## xuiSplitLayoutGetOrientation
- 位置: xui.h:8853  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetOrientation(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1401（体 5 行）
- 用法: test_xui/xui_split_layout_test.c:202

## xuiSplitLayoutSetPaneCount
- 位置: xui.h:8854  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneCount(xui_widget pWidget, int iPaneCount);`
- 实现: src/xui_split_layout.c:1407（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiSplitLayoutGetPaneCount
- 位置: xui.h:8855  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetPaneCount(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1417（体 5 行）
- 用法: test_xui/xui_split_layout_test.c:198

## xuiSplitLayoutGetPaneWidget
- 位置: xui.h:8856  已注释: 否
- 签名: `XUI_API xui_widget xuiSplitLayoutGetPaneWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1423（体 6 行）
- 返回码: NULL
- 用法: examples/xui_split_layout/main.c:222; examples/xui_split_layout/main.c:223; examples/xui_split_layout/main.c:224

## xuiSplitLayoutAddPaneChild
- 位置: xui.h:8857  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutAddPaneChild(xui_widget pWidget, int iPane, xui_widget pChild);`
- 实现: src/xui_split_layout.c:1430（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutSetPaneWeight
- 位置: xui.h:8858  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneWeight(xui_widget pWidget, int iIndex, float fWeight);`
- 实现: src/xui_split_layout.c:1439（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_split_layout/main.c:219

## xuiSplitLayoutGetPaneWeight
- 位置: xui.h:8859  已注释: 否
- 签名: `XUI_API float xuiSplitLayoutGetPaneWeight(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1447（体 6 行）

## xuiSplitLayoutSetPaneMode
- 位置: xui.h:8860  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneMode(xui_widget pWidget, int iIndex, int iMode);`
- 实现: src/xui_split_layout.c:1454（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_split_layout/main.c:216; examples/xui_split_layout/main.c:220; examples/xui_split_layout/main.c:257

## xuiSplitLayoutGetPaneMode
- 位置: xui.h:8861  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetPaneMode(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1462（体 6 行）
- 返回码: XUI_SPLIT_PANE_GROW

## xuiSplitLayoutSetPaneFixedSize
- 位置: xui.h:8862  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneFixedSize(xui_widget pWidget, int iIndex, float fSize);`
- 实现: src/xui_split_layout.c:1469（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_split_layout/main.c:217; examples/xui_split_layout/main.c:221; examples/xui_split_layout/main.c:258

## xuiSplitLayoutGetPaneFixedSize
- 位置: xui.h:8863  已注释: 否
- 签名: `XUI_API float xuiSplitLayoutGetPaneFixedSize(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1477（体 6 行）
- 用法: examples/xui_split_layout/main.c:335; test_xui/xui_split_layout_pixel_test.c:108; test_xui/xui_split_layout_test.c:274

## xuiSplitLayoutSetPaneMinSize
- 位置: xui.h:8864  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneMinSize(xui_widget pWidget, int iIndex, float fSize);`
- 实现: src/xui_split_layout.c:1484（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_split_layout/main.c:218; examples/xui_split_layout/main.c:259; examples/xui_split_layout/main.c:260

## xuiSplitLayoutGetPaneMinSize
- 位置: xui.h:8865  已注释: 否
- 签名: `XUI_API float xuiSplitLayoutGetPaneMinSize(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1492（体 6 行）

## xuiSplitLayoutSetPaneMaxSize
- 位置: xui.h:8866  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetPaneMaxSize(xui_widget pWidget, int iIndex, float fSize);`
- 实现: src/xui_split_layout.c:1499（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutGetPaneMaxSize
- 位置: xui.h:8867  已注释: 否
- 签名: `XUI_API float xuiSplitLayoutGetPaneMaxSize(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1507（体 6 行）

## xuiSplitLayoutGetPaneSize
- 位置: xui.h:8868  已注释: 否
- 签名: `XUI_API float xuiSplitLayoutGetPaneSize(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1514（体 7 行）
- 用法: test_xui/xui_split_layout_test.c:332; test_xui/xui_split_layout_test.c:333; test_xui/xui_split_layout_test.c:334

## xuiSplitLayoutSetDividerMetrics
- 位置: xui.h:8869  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetDividerMetrics(xui_widget pWidget, float fLayoutSize, float fVisualSize, float fHitSize);`
- 实现: src/xui_split_layout.c:1522（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_split_layout_test.c:335

## xuiSplitLayoutGetDividerMetrics
- 位置: xui.h:8870  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetDividerMetrics(xui_widget pWidget, float* pLayoutSize, float* pVisualSize, float* pHitSize);`
- 实现: src/xui_split_layout.c:1532（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_split_layout_test.c:203; test_xui/xui_split_layout_test.c:336

## xuiSplitLayoutSetDividerSize
- 位置: xui.h:8871  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetDividerSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_split_layout.c:1542（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutSetDividerVisualSize
- 位置: xui.h:8872  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetDividerVisualSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_split_layout.c:1550（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutSetDividerHitSize
- 位置: xui.h:8873  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetDividerHitSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_split_layout.c:1558（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutSetColors
- 位置: xui.h:8874  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutSetColors(xui_widget pWidget, uint32_t iDivider, uint32_t iHover, uint32_t iActive, uint32_t iShadow);`
- 实现: src/xui_split_layout.c:1566（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiSplitLayoutGetColors
- 位置: xui.h:8875  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetColors(xui_widget pWidget, uint32_t* pDivider, uint32_t* pHover, uint32_t* pActive, uint32_t* pShadow);`
- 实现: src/xui_split_layout.c:1577（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiSplitLayoutGetPaneRect
- 位置: xui.h:8876  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSplitLayoutGetPaneRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1588（体 5 行）
- 用法: examples/xui_split_layout/main.c:327; examples/xui_split_layout/main.c:328; test_xui/xui_split_layout_test.c:206

## xuiSplitLayoutGetDividerLayoutRect
- 位置: xui.h:8877  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSplitLayoutGetDividerLayoutRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1594（体 7 行）
- 用法: test_xui/xui_split_layout_pixel_test.c:70; test_xui/xui_split_layout_pixel_test.c:100; test_xui/xui_split_layout_test.c:209

## xuiSplitLayoutGetDividerVisualRect
- 位置: xui.h:8878  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSplitLayoutGetDividerVisualRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1602（体 7 行）
- 用法: examples/xui_split_layout/main.c:330; examples/xui_split_layout_dock_repro/main.c:506; test_xui/xui_split_layout_pixel_test.c:73

## xuiSplitLayoutGetDividerHitRect
- 位置: xui.h:8879  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSplitLayoutGetDividerHitRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_split_layout.c:1610（体 7 行）
- 用法: examples/xui_split_layout/main.c:306; examples/xui_split_layout/main.c:329; test_xui/xui_split_layout_pixel_test.c:74

## xuiSplitLayoutGetShadowRect
- 位置: xui.h:8880  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSplitLayoutGetShadowRect(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1618（体 6 行）
- 用法: test_xui/xui_split_layout_pixel_test.c:99; test_xui/xui_split_layout_test.c:246

## xuiSplitLayoutGetHoverDivider
- 位置: xui.h:8881  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetHoverDivider(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1625（体 5 行）
- 用法: test_xui/xui_split_layout_test.c:225; test_xui/xui_split_layout_test.c:283

## xuiSplitLayoutGetActiveDivider
- 位置: xui.h:8882  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetActiveDivider(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1631（体 5 行）
- 用法: test_xui/xui_split_layout_pixel_test.c:96; test_xui/xui_split_layout_test.c:229; test_xui/xui_split_layout_test.c:270

## xuiSplitLayoutGetChangeCount
- 位置: xui.h:8883  已注释: 否
- 签名: `XUI_API int xuiSplitLayoutGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_split_layout.c:1637（体 5 行）
- 用法: examples/xui_split_layout/main.c:339; test_xui/xui_split_layout_test.c:301

