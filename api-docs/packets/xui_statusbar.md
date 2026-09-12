# 草稿包：xui.h / statusbar（37 条 API）

> 生成 2026-09-10 03:05 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiStatusBarGetType
- 位置: xui.h:10342  已注释: 否
- 签名: `XUI_API xui_widget_type xuiStatusBarGetType(xui_context pContext);`
- 实现: src/xui_statusbar.c:855（体 26 行）
- 返回码: NULL

## xuiStatusBarCreate
- 位置: xui.h:10343  已注释: 否
- 签名: `XUI_API int xuiStatusBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_statusbar_desc_t* pDesc);`
- 实现: src/xui_statusbar.c:882（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch162_main1.c:13; examples/tutorial_capture/ch202_main1.c:38; examples/tutorial_capture/ch204_main1.c:41

## xuiStatusBarSetSelect
- 位置: xui.h:10344  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetSelect(xui_widget pWidget, xui_statusbar_select_proc onSelect, void* pUser);`
- 实现: src/xui_statusbar.c:893（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_statusbar/main.c:197; test_xui/xui_statusbar_test.c:192

## xuiStatusBarSetContextMenu
- 位置: xui.h:10345  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetContextMenu(xui_widget pWidget, xui_statusbar_context_proc onContext, void* pUser);`
- 实现: src/xui_statusbar.c:1016（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiStatusBarSetItems
- 位置: xui.h:10346  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItems(xui_widget pWidget, const xui_statusbar_item_t* pItems, int iCount);`
- 实现: src/xui_statusbar.c:902（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:231

## xuiStatusBarAddText
- 位置: xui.h:10347  已注释: 否
- 签名: `XUI_API int xuiStatusBarAddText(xui_widget pWidget, int iSection, const char* sText, float fWidth, int bClickable, int iValue);`
- 实现: src/xui_statusbar.c:954（体 6 行）
- 用法: examples/tutorial_capture/ch162_main1.c:20; examples/tutorial_capture/ch162_main1.c:21; examples/tutorial_capture/ch162_main1.c:23

## xuiStatusBarAddProgress
- 位置: xui.h:10348  已注释: 否
- 签名: `XUI_API int xuiStatusBarAddProgress(xui_widget pWidget, int iSection, float fMin, float fMax, float fValue, float fWidth);`
- 实现: src/xui_statusbar.c:961（体 17 行）
- 用法: examples/tutorial_capture/ch162_main1.c:22; examples/xui_statusbar/main.c:201

## xuiStatusBarAddSpacer
- 位置: xui.h:10349  已注释: 否
- 签名: `XUI_API int xuiStatusBarAddSpacer(xui_widget pWidget, int iSection, float fWidth);`
- 实现: src/xui_statusbar.c:979（体 4 行）
- 用法: examples/xui_statusbar/main.c:202

## xuiStatusBarAddFlexibleSpacer
- 位置: xui.h:10350  已注释: 否
- 签名: `XUI_API int xuiStatusBarAddFlexibleSpacer(xui_widget pWidget, int iSection, float fWeight);`
- 实现: src/xui_statusbar.c:984（体 13 行）
- 用法: examples/xui_split_layout_dock_repro/main.c:313; examples/xui_statusbar/main.c:203

## xuiStatusBarClear
- 位置: xui.h:10351  已注释: 否
- 签名: `XUI_API int xuiStatusBarClear(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:998（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStatusBarGetItemCount
- 位置: xui.h:10352  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetItemCount(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1010（体 5 行）
- 用法: examples/xui_statusbar/main.c:248; test_xui/xui_statusbar_test.c:199

## xuiStatusBarGetItem
- 位置: xui.h:10353  已注释: 否
- 签名: `XUI_API const xui_statusbar_item_t* xuiStatusBarGetItem(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1025（体 6 行）
- 返回码: NULL
- 用法: test_xui/xui_statusbar_test.c:235

## xuiStatusBarGetItemRect
- 位置: xui.h:10354  已注释: 否
- 签名: `XUI_API xui_rect_t xuiStatusBarGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1032（体 12 行）
- 用法: examples/xui_statusbar/main.c:236; examples/xui_statusbar/main.c:249; examples/xui_statusbar/main.c:250

## xuiStatusBarGetItemAt
- 位置: xui.h:10355  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetItemAt(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_statusbar.c:1045（体 9 行）
- 用法: test_xui/xui_statusbar_test.c:212; test_xui/xui_statusbar_test.c:213

## xuiStatusBarSetItemEnabled
- 位置: xui.h:10356  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemEnabled(xui_widget pWidget, int iIndex, int bEnabled);`
- 实现: src/xui_statusbar.c:1055（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_statusbar/main.c:207; test_xui/xui_statusbar_test.c:230

## xuiStatusBarIsItemEnabled
- 位置: xui.h:10357  已注释: 否
- 签名: `XUI_API int xuiStatusBarIsItemEnabled(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1070（体 6 行）
- 用法: test_xui/xui_statusbar_test.c:231

## xuiStatusBarSetItemClickable
- 位置: xui.h:10358  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemClickable(xui_widget pWidget, int iIndex, int bClickable);`
- 实现: src/xui_statusbar.c:1077（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_statusbar_test.c:232

## xuiStatusBarIsItemClickable
- 位置: xui.h:10359  已注释: 否
- 签名: `XUI_API int xuiStatusBarIsItemClickable(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1092（体 6 行）
- 用法: test_xui/xui_statusbar_test.c:233

## xuiStatusBarSetItemText
- 位置: xui.h:10360  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemText(xui_widget pWidget, int iIndex, const char* sText);`
- 实现: src/xui_statusbar.c:1099（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_split_layout_dock_repro/main.c:204; examples/xui_split_layout_dock_repro/main.c:215; test_xui/xui_statusbar_test.c:234

## xuiStatusBarSetItemValue
- 位置: xui.h:10361  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemValue(xui_widget pWidget, int iIndex, int iValue);`
- 实现: src/xui_statusbar.c:1108（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_statusbar_test.c:236

## xuiStatusBarGetItemValue
- 位置: xui.h:10362  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetItemValue(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1117（体 6 行）
- 用法: test_xui/xui_statusbar_test.c:237

## xuiStatusBarSetItemWidth
- 位置: xui.h:10363  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemWidth(xui_widget pWidget, int iIndex, float fWidth);`
- 实现: src/xui_statusbar.c:1124（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_statusbar_test.c:238

## xuiStatusBarSetItemFlex
- 位置: xui.h:10364  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetItemFlex(xui_widget pWidget, int iIndex, float fFlex);`
- 实现: src/xui_statusbar.c:1133（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_statusbar_test.c:240

## xuiStatusBarSetProgress
- 位置: xui.h:10365  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetProgress(xui_widget pWidget, int iIndex, float fValue);`
- 实现: src/xui_statusbar.c:1143（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_statusbar/main.c:255; test_xui/xui_statusbar_test.c:228

## xuiStatusBarGetProgress
- 位置: xui.h:10366  已注释: 否
- 签名: `XUI_API float xuiStatusBarGetProgress(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1157（体 6 行）
- 用法: examples/xui_statusbar/main.c:256; test_xui/xui_statusbar_test.c:201; test_xui/xui_statusbar_test.c:229

## xuiStatusBarGetHoverIndex
- 位置: xui.h:10367  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1164（体 5 行）
- 用法: test_xui/xui_statusbar_test.c:224

## xuiStatusBarSetHoverIndex
- 位置: xui.h:10368  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetHoverIndex(xui_widget pWidget, int iIndex);`
- 实现: src/xui_statusbar.c:1170（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_statusbar/main.c:266; test_xui/xui_statusbar_test.c:223

## xuiStatusBarGetActiveIndex
- 位置: xui.h:10369  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1177（体 5 行）

## xuiStatusBarSetFont
- 位置: xui.h:10370  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_statusbar.c:1183（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStatusBarGetFont
- 位置: xui.h:10371  已注释: 否
- 签名: `XUI_API xui_font xuiStatusBarGetFont(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1192（体 5 行）

## xuiStatusBarSetMetrics
- 位置: xui.h:10372  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetMetrics(xui_widget pWidget, const xui_statusbar_metrics_t* pMetrics);`
- 实现: src/xui_statusbar.c:1198（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStatusBarGetMetrics
- 位置: xui.h:10373  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetMetrics(xui_widget pWidget, xui_statusbar_metrics_t* pMetrics);`
- 实现: src/xui_statusbar.c:1208（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_statusbar_test.c:242

## xuiStatusBarSetColors
- 位置: xui.h:10374  已注释: 否
- 签名: `XUI_API int xuiStatusBarSetColors(xui_widget pWidget, const xui_statusbar_colors_t* pColors);`
- 实现: src/xui_statusbar.c:1217（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStatusBarGetColors
- 位置: xui.h:10375  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetColors(xui_widget pWidget, xui_statusbar_colors_t* pColors);`
- 实现: src/xui_statusbar.c:1227（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_statusbar_test.c:244

## xuiStatusBarGetState
- 位置: xui.h:10376  已注释: 否
- 签名: `XUI_API uint32_t xuiStatusBarGetState(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1236（体 11 行）
- 返回码: XUI_WIDGET_STATE_DISABLED
- 用法: test_xui/xui_statusbar_test.c:246

## xuiStatusBarGetSelectCount
- 位置: xui.h:10377  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetSelectCount(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1248（体 5 行）
- 用法: test_xui/xui_statusbar_test.c:247

## xuiStatusBarGetChangeCount
- 位置: xui.h:10378  已注释: 否
- 签名: `XUI_API int xuiStatusBarGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_statusbar.c:1254（体 5 行）
- 用法: test_xui/xui_statusbar_test.c:248

