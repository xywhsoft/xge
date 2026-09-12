# 草稿包：xui.h / breadcrumb（33 条 API）

> 生成 2026-09-10 02:55 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiBreadcrumbGetType
- 位置: xui.h:6361  已注释: 否
- 签名: `XUI_API xui_widget_type xuiBreadcrumbGetType(xui_context pContext);`
- 实现: src/xui_breadcrumb.c:790（体 29 行）
- 返回码: NULL

## xuiBreadcrumbCreate
- 位置: xui.h:6362  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbCreate(xui_context pContext, xui_widget* ppWidget, const xui_breadcrumb_desc_t* pDesc);`
- 实现: src/xui_breadcrumb.c:820（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch153_main1.c:13; examples/xui_breadcrumb/main.c:197; test_xui/xui_breadcrumb_test.c:112

## xuiBreadcrumbSetClick
- 位置: xui.h:6363  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetClick(xui_widget pWidget, xui_breadcrumb_click_proc onClick, void* pUser);`
- 实现: src/xui_breadcrumb.c:831（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_breadcrumb/main.c:200; test_xui/xui_breadcrumb_test.c:117; test_xui/xui_chart_breadcrumb_keyboard_test.c:188

## xuiBreadcrumbSetContextMenu
- 位置: xui.h:6364  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetContextMenu(xui_widget pWidget, xui_breadcrumb_context_proc onContext, void* pUser);`
- 实现: src/xui_breadcrumb.c:842（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:189; test_xui/xui_chart_breadcrumb_keyboard_test.c:260; test_xui/xui_chart_breadcrumb_keyboard_test.c:300

## xuiBreadcrumbSetItems
- 位置: xui.h:6365  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetItems(xui_widget pWidget, const xui_breadcrumb_item_t* pItems, int iItemCount);`
- 实现: src/xui_breadcrumb.c:851（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:187; test_xui/xui_chart_breadcrumb_keyboard_test.c:273

## xuiBreadcrumbClearItems
- 位置: xui.h:6366  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbClearItems(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:863（体 4 行）
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:62; test_xui/xui_chart_breadcrumb_keyboard_test.c:262; test_xui/xui_chart_breadcrumb_keyboard_test.c:295

## xuiBreadcrumbAddItem
- 位置: xui.h:6367  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbAddItem(xui_widget pWidget, const char* sText, int bClickable, int iValue);`
- 实现: src/xui_breadcrumb.c:868（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch153_main1.c:19; examples/tutorial_capture/ch153_main1.c:20; examples/tutorial_capture/ch153_main1.c:21

## xuiBreadcrumbSetItem
- 位置: xui.h:6368  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetItem(xui_widget pWidget, int iIndex, const char* sText, int bClickable, int iValue);`
- 实现: src/xui_breadcrumb.c:885（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_breadcrumb_test.c:158; test_xui/xui_chart_breadcrumb_keyboard_test.c:256; test_xui/xui_chart_breadcrumb_keyboard_test.c:259

## xuiBreadcrumbGetItemCount
- 位置: xui.h:6369  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetItemCount(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:899（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:120; test_xui/xui_breadcrumb_test.c:157; test_xui/xui_chart_breadcrumb_keyboard_test.c:270

## xuiBreadcrumbGetItemText
- 位置: xui.h:6370  已注释: 否
- 签名: `XUI_API const char* xuiBreadcrumbGetItemText(xui_widget pWidget, int iIndex);`
- 实现: src/xui_breadcrumb.c:907（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:121; test_xui/xui_breadcrumb_test.c:159

## xuiBreadcrumbGetItemClickable
- 位置: xui.h:6371  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetItemClickable(xui_widget pWidget, int iIndex);`
- 实现: src/xui_breadcrumb.c:915（体 8 行）
- 用法: test_xui/xui_breadcrumb_test.c:122

## xuiBreadcrumbGetItemValue
- 位置: xui.h:6372  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetItemValue(xui_widget pWidget, int iIndex);`
- 实现: src/xui_breadcrumb.c:924（体 8 行）
- 用法: test_xui/xui_breadcrumb_test.c:123; test_xui/xui_breadcrumb_test.c:159

## xuiBreadcrumbSetSeparator
- 位置: xui.h:6373  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetSeparator(xui_widget pWidget, const char* sSeparator);`
- 实现: src/xui_breadcrumb.c:933（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_breadcrumb_test.c:149

## xuiBreadcrumbGetSeparator
- 位置: xui.h:6374  已注释: 否
- 签名: `XUI_API const char* xuiBreadcrumbGetSeparator(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:946（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:124; test_xui/xui_breadcrumb_test.c:150

## xuiBreadcrumbSetSeparatorIcon
- 位置: xui.h:6375  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetSeparatorIcon(xui_widget pWidget, xui_surface pSurface, xui_rect_t tSrc, float fIconSize);`
- 实现: src/xui_breadcrumb.c:954（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_breadcrumb_test.c:153

## xuiBreadcrumbGetSeparatorIcon
- 位置: xui.h:6376  已注释: 否
- 签名: `XUI_API xui_surface xuiBreadcrumbGetSeparatorIcon(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:966（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:154

## xuiBreadcrumbGetSeparatorIconSource
- 位置: xui.h:6377  已注释: 否
- 签名: `XUI_API xui_rect_t xuiBreadcrumbGetSeparatorIconSource(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:974（体 7 行）

## xuiBreadcrumbGetSeparatorIconSize
- 位置: xui.h:6378  已注释: 否
- 签名: `XUI_API float xuiBreadcrumbGetSeparatorIconSize(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:982（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:155

## xuiBreadcrumbSetFont
- 位置: xui.h:6379  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_breadcrumb.c:990（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiBreadcrumbGetFont
- 位置: xui.h:6380  已注释: 否
- 签名: `XUI_API xui_font xuiBreadcrumbGetFont(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1000（体 7 行）

## xuiBreadcrumbSetTextColors
- 位置: xui.h:6381  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetTextColors(xui_widget pWidget, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iDisabled);`
- 实现: src/xui_breadcrumb.c:1008（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_breadcrumb_test.c:160

## xuiBreadcrumbGetTextColors
- 位置: xui.h:6382  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetTextColors(xui_widget pWidget, uint32_t* pNormal, uint32_t* pHover, uint32_t* pActive, uint32_t* pDisabled);`
- 实现: src/xui_breadcrumb.c:1021（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiBreadcrumbSetSeparatorColor
- 位置: xui.h:6383  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetSeparatorColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_breadcrumb.c:1034（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiBreadcrumbGetSeparatorColor
- 位置: xui.h:6384  已注释: 否
- 签名: `XUI_API uint32_t xuiBreadcrumbGetSeparatorColor(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1044（体 7 行）

## xuiBreadcrumbSetBackgroundColor
- 位置: xui.h:6385  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_breadcrumb.c:1052（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiBreadcrumbGetBackgroundColor
- 位置: xui.h:6386  已注释: 否
- 签名: `XUI_API uint32_t xuiBreadcrumbGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1062（体 7 行）

## xuiBreadcrumbSetMetrics
- 位置: xui.h:6387  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbSetMetrics(xui_widget pWidget, float fGap, float fPaddingX, float fPaddingY);`
- 实现: src/xui_breadcrumb.c:1070（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_breadcrumb_test.c:162

## xuiBreadcrumbGetMetrics
- 位置: xui.h:6388  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetMetrics(xui_widget pWidget, float* pGap, float* pPaddingX, float* pPaddingY);`
- 实现: src/xui_breadcrumb.c:1082（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiBreadcrumbGetItemRect
- 位置: xui.h:6389  已注释: 否
- 签名: `XUI_API xui_rect_t xuiBreadcrumbGetItemRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_breadcrumb.c:1094（体 8 行）
- 用法: examples/xui_breadcrumb/main.c:256; examples/xui_breadcrumb/main.c:369; test_xui/xui_breadcrumb_test.c:134

## xuiBreadcrumbGetSeparatorRect
- 位置: xui.h:6390  已注释: 否
- 签名: `XUI_API xui_rect_t xuiBreadcrumbGetSeparatorRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_breadcrumb.c:1103（体 8 行）
- 用法: examples/xui_breadcrumb/main.c:370; test_xui/xui_breadcrumb_test.c:136; test_xui/xui_breadcrumb_test.c:168

## xuiBreadcrumbGetHoverIndex
- 位置: xui.h:6391  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetHoverIndex(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1112（体 7 行）

## xuiBreadcrumbGetActiveIndex
- 位置: xui.h:6392  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetActiveIndex(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1120（体 7 行）
- 用法: test_xui/xui_chart_breadcrumb_keyboard_test.c:207; test_xui/xui_chart_breadcrumb_keyboard_test.c:218

## xuiBreadcrumbGetClickCount
- 位置: xui.h:6393  已注释: 否
- 签名: `XUI_API int xuiBreadcrumbGetClickCount(xui_widget pWidget);`
- 实现: src/xui_breadcrumb.c:1128（体 7 行）
- 用法: test_xui/xui_breadcrumb_test.c:142

