# 草稿包：xui.h / tabs（52 条 API）

> 生成 2026-09-10 03:06 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTabsGetType
- 位置: xui.h:8979  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTabsGetType(xui_context pContext);`
- 实现: src/xui_tabs.c:1937（体 37 行）
- 返回码: NULL

## xuiTabsCreate
- 位置: xui.h:8980  已注释: 否
- 签名: `XUI_API int xuiTabsCreate(xui_context pContext, xui_widget* ppWidget, const xui_tabs_desc_t* pDesc);`
- 实现: src/xui_tabs.c:1975（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: examples/tutorial_capture/ch151_main1.c:14; examples/xui_tabs/main.c:198; test_xui/xui_code_edit_test.c:1481

## xuiTabsSetSelect
- 位置: xui.h:8981  已注释: 否
- 签名: `XUI_API int xuiTabsSetSelect(xui_widget pWidget, xui_tabs_select_proc onSelect, void* pUser);`
- 实现: src/xui_tabs.c:2001（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_tabs/main.c:200; test_xui/xui_tabs_test.c:197

## xuiTabsSetClose
- 位置: xui.h:8982  已注释: 否
- 签名: `XUI_API int xuiTabsSetClose(xui_widget pWidget, xui_tabs_close_proc onClose, int bCloseButtons, void* pUser);`
- 实现: src/xui_tabs.c:2012（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tabs/main.c:201; test_xui/xui_style_chrome_test.c:301; test_xui/xui_tabs_test.c:199

## xuiTabsSetContextMenu
- 位置: xui.h:8983  已注释: 否
- 签名: `XUI_API int xuiTabsSetContextMenu(xui_widget pWidget, xui_tabs_context_proc onContext, void* pUser);`
- 实现: src/xui_tabs.c:2334（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_tabs_test.c:201

## xuiTabsAddPage
- 位置: xui.h:8984  已注释: 否
- 签名: `XUI_API int xuiTabsAddPage(xui_widget pWidget, const char* sTitle, xui_widget* ppPage);`
- 实现: src/xui_tabs.c:2024（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch151_main1.c:21; examples/tutorial_capture/ch151_main1.c:22; examples/tutorial_capture/ch151_main1.c:23

## xuiTabsSetItems
- 位置: xui.h:8985  已注释: 否
- 签名: `XUI_API int xuiTabsSetItems(xui_widget pWidget, const char** arrItems, int iItemCount);`
- 实现: src/xui_tabs.c:2046（体 7 行）

## xuiTabsGetItemCount
- 位置: xui.h:8986  已注释: 否
- 签名: `XUI_API int xuiTabsGetItemCount(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2054（体 7 行）
- 用法: examples/xui_tabs/main.c:249; test_xui/xui_tabs_test.c:209

## xuiTabsGetItemText
- 位置: xui.h:8987  已注释: 否
- 签名: `XUI_API const char* xuiTabsGetItemText(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2062（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_tabs_test.c:210

## xuiTabsGetTabBarWidget
- 位置: xui.h:8988  已注释: 否
- 签名: `XUI_API xui_widget xuiTabsGetTabBarWidget(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2071（体 7 行）
- 用法: test_xui/xui_tabs_test.c:212

## xuiTabsGetClientWidget
- 位置: xui.h:8989  已注释: 否
- 签名: `XUI_API xui_widget xuiTabsGetClientWidget(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2079（体 7 行）
- 用法: test_xui/xui_tabs_test.c:213

## xuiTabsGetPageWidget
- 位置: xui.h:8990  已注释: 否
- 签名: `XUI_API xui_widget xuiTabsGetPageWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2087（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_tabs_test.c:215

## xuiTabsGetButtonWidget
- 位置: xui.h:8991  已注释: 否
- 签名: `XUI_API xui_widget xuiTabsGetButtonWidget(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2096（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_tabs_test.c:216; test_xui/xui_tabs_test.c:247

## xuiTabsAddPageChild
- 位置: xui.h:8992  已注释: 否
- 签名: `XUI_API int xuiTabsAddPageChild(xui_widget pWidget, int iIndex, xui_widget pChild);`
- 实现: src/xui_tabs.c:2105（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_tabs/main.c:155; test_xui/xui_code_edit_test.c:1492; test_xui/xui_code_edit_test.c:1494

## xuiTabsSetEnabledItems
- 位置: xui.h:8993  已注释: 否
- 签名: `XUI_API int xuiTabsSetEnabledItems(xui_widget pWidget, const int* arrEnabled, int iItemCount);`
- 实现: src/xui_tabs.c:2115（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsSetDirtyItems
- 位置: xui.h:8994  已注释: 否
- 签名: `XUI_API int xuiTabsSetDirtyItems(xui_widget pWidget, const int* arrDirty, int iItemCount);`
- 实现: src/xui_tabs.c:2135（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsSetIcons
- 位置: xui.h:8995  已注释: 否
- 签名: `XUI_API int xuiTabsSetIcons(xui_widget pWidget, xui_surface* arrIcons, const xui_rect_t* arrSrc, int iItemCount);`
- 实现: src/xui_tabs.c:2148（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_tabs_test.c:270

## xuiTabsSetFont
- 位置: xui.h:8996  已注释: 否
- 签名: `XUI_API int xuiTabsSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_tabs.c:2162（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsGetFont
- 位置: xui.h:8997  已注释: 否
- 签名: `XUI_API xui_font xuiTabsGetFont(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2175（体 7 行）

## xuiTabsSetSelected
- 位置: xui.h:8998  已注释: 否
- 签名: `XUI_API int xuiTabsSetSelected(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2183（体 7 行）
- 用法: examples/xui_tabs/main.c:254; examples/xui_tabs/main.c:268; test_xui/xui_tabs_test.c:222

## xuiTabsGetSelected
- 位置: xui.h:8999  已注释: 否
- 签名: `XUI_API int xuiTabsGetSelected(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2191（体 7 行）
- 用法: examples/xui_tabs/main.c:255; examples/xui_tabs/main.c:259; examples/xui_tabs/main.c:263

## xuiTabsSetTabSize
- 位置: xui.h:9000  已注释: 否
- 签名: `XUI_API int xuiTabsSetTabSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_tabs.c:2199（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsGetTabSize
- 位置: xui.h:9001  已注释: 否
- 签名: `XUI_API int xuiTabsGetTabSize(xui_widget pWidget, float* pWidth, float* pHeight);`
- 实现: src/xui_tabs.c:2210（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTabsSetPlacement
- 位置: xui.h:9002  已注释: 否
- 签名: `XUI_API int xuiTabsSetPlacement(xui_widget pWidget, int iPlacement);`
- 实现: src/xui_tabs.c:2221（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_tabs_test.c:283

## xuiTabsGetPlacement
- 位置: xui.h:9003  已注释: 否
- 签名: `XUI_API int xuiTabsGetPlacement(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2233（体 7 行）
- 用法: test_xui/xui_tabs_test.c:284

## xuiTabsSetScrollable
- 位置: xui.h:9004  已注释: 否
- 签名: `XUI_API int xuiTabsSetScrollable(xui_widget pWidget, int bScrollable);`
- 实现: src/xui_tabs.c:2241（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsIsScrollable
- 位置: xui.h:9005  已注释: 否
- 签名: `XUI_API int xuiTabsIsScrollable(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2252（体 7 行）

## xuiTabsSetScroll
- 位置: xui.h:9006  已注释: 否
- 签名: `XUI_API int xuiTabsSetScroll(xui_widget pWidget, float fScrollX);`
- 实现: src/xui_tabs.c:2260（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_tabs_test.c:285

## xuiTabsGetScroll
- 位置: xui.h:9007  已注释: 否
- 签名: `XUI_API float xuiTabsGetScroll(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2272（体 7 行）
- 用法: examples/xui_tabs/main.c:275; examples/xui_tabs/main.c:380

## xuiTabsGetMaxScroll
- 位置: xui.h:9008  已注释: 否
- 签名: `XUI_API float xuiTabsGetMaxScroll(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2280（体 9 行）

## xuiTabsEnsureVisible
- 位置: xui.h:9009  已注释: 否
- 签名: `XUI_API int xuiTabsEnsureVisible(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2290（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsSetColors
- 位置: xui.h:9010  已注释: 否
- 签名: `XUI_API int xuiTabsSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText);`
- 实现: src/xui_tabs.c:2300（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsGetColors
- 位置: xui.h:9011  已注释: 否
- 签名: `XUI_API int xuiTabsGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pTab, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled, uint32_t* pText, uint32_t* pActiveText);`
- 实现: src/xui_tabs.c:2317（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTabsSetFrameColors
- 位置: xui.h:9012  已注释: 否
- 签名: `XUI_API int xuiTabsSetFrameColors(xui_widget pWidget, uint32_t iBorder, uint32_t iClient);`
- 实现: src/xui_tabs.c:2343（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTabsGetFrameColors
- 位置: xui.h:9013  已注释: 否
- 签名: `XUI_API int xuiTabsGetFrameColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pClient);`
- 实现: src/xui_tabs.c:2354（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTabsGetTabBarRect
- 位置: xui.h:9014  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetTabBarRect(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2365（体 7 行）
- 用法: examples/xui_tabs/main.c:265; test_xui/xui_tabs_test.c:243; test_xui/xui_tabs_test.c:263

## xuiTabsGetClientRect
- 位置: xui.h:9015  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetClientRect(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2373（体 7 行）
- 用法: examples/xui_tabs/main.c:250; test_xui/xui_tabs_test.c:217

## xuiTabsGetTabRect
- 位置: xui.h:9016  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetTabRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2381（体 8 行）
- 用法: examples/xui_tabs/main.c:250; examples/xui_tabs/main.c:270; test_xui/xui_tabs_test.c:218

## xuiTabsGetTextRect
- 位置: xui.h:9017  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetTextRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2390（体 8 行）

## xuiTabsGetIconRect
- 位置: xui.h:9018  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetIconRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2399（体 8 行）
- 用法: test_xui/xui_tabs_test.c:277

## xuiTabsGetDirtyRect
- 位置: xui.h:9019  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetDirtyRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2408（体 8 行）
- 用法: examples/xui_tabs/main.c:251; examples/xui_tabs/main.c:271; test_xui/xui_tabs_test.c:276

## xuiTabsGetCloseRect
- 位置: xui.h:9020  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetCloseRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tabs.c:2417（体 8 行）
- 用法: examples/xui_tabs/main.c:261; test_xui/xui_tabs_test.c:238

## xuiTabsIsOverflow
- 位置: xui.h:9021  已注释: 否
- 签名: `XUI_API int xuiTabsIsOverflow(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2426（体 7 行）
- 用法: examples/xui_tabs/main.c:273; test_xui/xui_tabs_test.c:245

## xuiTabsGetOverflowRect
- 位置: xui.h:9022  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTabsGetOverflowRect(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2434（体 8 行）
- 用法: examples/xui_tabs/main.c:272; test_xui/xui_tabs_test.c:244; test_xui/xui_tabs_test.c:264

## xuiTabsGetOverflowMenu
- 位置: xui.h:9023  已注释: 否
- 签名: `XUI_API xui_widget xuiTabsGetOverflowMenu(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2443（体 7 行）
- 用法: test_xui/xui_tabs_test.c:214; test_xui/xui_tabs_test.c:250

## xuiTabsGetHoverIndex
- 位置: xui.h:9024  已注释: 否
- 签名: `XUI_API int xuiTabsGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2451（体 7 行）

## xuiTabsGetActiveIndex
- 位置: xui.h:9025  已注释: 否
- 签名: `XUI_API int xuiTabsGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2459（体 7 行）

## xuiTabsGetCloseHoverIndex
- 位置: xui.h:9026  已注释: 否
- 签名: `XUI_API int xuiTabsGetCloseHoverIndex(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2467（体 7 行）

## xuiTabsGetCloseActiveIndex
- 位置: xui.h:9027  已注释: 否
- 签名: `XUI_API int xuiTabsGetCloseActiveIndex(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2475（体 7 行）

## xuiTabsGetState
- 位置: xui.h:9028  已注释: 否
- 签名: `XUI_API uint32_t xuiTabsGetState(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2483（体 15 行）
- 用法: test_xui/xui_tabs_test.c:251

## xuiTabsGetChangeCount
- 位置: xui.h:9029  已注释: 否
- 签名: `XUI_API int xuiTabsGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2499（体 7 行）

## xuiTabsGetCloseCount
- 位置: xui.h:9030  已注释: 否
- 签名: `XUI_API int xuiTabsGetCloseCount(xui_widget pWidget);`
- 实现: src/xui_tabs.c:2507（体 7 行）

