# 草稿包：xui.h / page（26 条 API）

> 生成 2026-09-10 03:02 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiPageGetType
- 位置: xui.h:8262  已注释: 否
- 签名: `XUI_API xui_widget_type xuiPageGetType(xui_context pContext);`
- 实现: src/xui_page.c:858（体 29 行）
- 返回码: NULL

## xuiPageCreate
- 位置: xui.h:8263  已注释: 否
- 签名: `XUI_API int xuiPageCreate(xui_context pContext, xui_widget* ppWidget, const xui_page_desc_t* pDesc);`
- 实现: src/xui_page.c:888（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_page/main.c:173; test_xui/xui_page_test.c:126

## xuiPageSetChange
- 位置: xui.h:8264  已注释: 否
- 签名: `XUI_API int xuiPageSetChange(xui_widget pWidget, xui_page_change_proc onChange, void* pUser);`
- 实现: src/xui_page.c:899（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_page/main.c:176; test_xui/xui_page_test.c:131

## xuiPageSetPageCount
- 位置: xui.h:8265  已注释: 否
- 签名: `XUI_API int xuiPageSetPageCount(xui_widget pWidget, int iPageCount);`
- 实现: src/xui_page.c:908（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPageGetPageCount
- 位置: xui.h:8266  已注释: 否
- 签名: `XUI_API int xuiPageGetPageCount(xui_widget pWidget);`
- 实现: src/xui_page.c:920（体 5 行）
- 用法: examples/xui_page/main.c:301; examples/xui_page/main.c:317; test_xui/xui_page_test.c:134

## xuiPageSetCurrent
- 位置: xui.h:8267  已注释: 否
- 签名: `XUI_API int xuiPageSetCurrent(xui_widget pWidget, int iPage, int bNotify);`
- 实现: src/xui_page.c:926（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_page_test.c:220

## xuiPageGetCurrent
- 位置: xui.h:8268  已注释: 否
- 签名: `XUI_API int xuiPageGetCurrent(xui_widget pWidget);`
- 实现: src/xui_page.c:933（体 5 行）
- 用法: examples/xui_page/main.c:268; examples/xui_page/main.c:269; examples/xui_page/main.c:270

## xuiPageSetTotal
- 位置: xui.h:8269  已注释: 否
- 签名: `XUI_API int xuiPageSetTotal(xui_widget pWidget, int iTotalCount, int iPageSize);`
- 实现: src/xui_page.c:939（体 10 行）
- 用法: examples/xui_page/main.c:212; test_xui/xui_page_test.c:218

## xuiPageSetWindowSize
- 位置: xui.h:8270  已注释: 否
- 签名: `XUI_API int xuiPageSetWindowSize(xui_widget pWidget, int iWindowSize);`
- 实现: src/xui_page.c:950（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPageGetWindowSize
- 位置: xui.h:8271  已注释: 否
- 签名: `XUI_API int xuiPageGetWindowSize(xui_widget pWidget);`
- 实现: src/xui_page.c:958（体 5 行）
- 用法: test_xui/xui_page_test.c:135

## xuiPageSetText
- 位置: xui.h:8272  已注释: 否
- 签名: `XUI_API int xuiPageSetText(xui_widget pWidget, const char* sFirst, const char* sLast, const char* sPrev, const char* sNext);`
- 实现: src/xui_page.c:964（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_page/main.c:210; test_xui/xui_page_test.c:162

## xuiPageGetText
- 位置: xui.h:8273  已注释: 否
- 签名: `XUI_API int xuiPageGetText(xui_widget pWidget, const char** ppFirst, const char** ppLast, const char** ppPrev, const char** ppNext);`
- 实现: src/xui_page.c:975（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPageSetFont
- 位置: xui.h:8274  已注释: 否
- 签名: `XUI_API int xuiPageSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_page.c:986（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiPageGetFont
- 位置: xui.h:8275  已注释: 否
- 签名: `XUI_API xui_font xuiPageGetFont(xui_widget pWidget);`
- 实现: src/xui_page.c:994（体 5 行）

## xuiPageSetMetrics
- 位置: xui.h:8276  已注释: 否
- 签名: `XUI_API int xuiPageSetMetrics(xui_widget pWidget, float fItemHeight, float fPageWidth, float fTextWidth, float fNavWidth, float fEllipsisWidth);`
- 实现: src/xui_page.c:1000（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_page/main.c:211; test_xui/xui_page_test.c:156

## xuiPageGetMetrics
- 位置: xui.h:8277  已注释: 否
- 签名: `XUI_API int xuiPageGetMetrics(xui_widget pWidget, float* pItemHeight, float* pPageWidth, float* pTextWidth, float* pNavWidth, float* pEllipsisWidth);`
- 实现: src/xui_page.c:1012（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_page/main.c:294; test_xui/xui_page_test.c:158

## xuiPageSetColors
- 位置: xui.h:8278  已注释: 否
- 签名: `XUI_API int xuiPageSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iHover, uint32_t iActive, uint32_t iCurrent, uint32_t iCurrentText, uint32_t iDisabledText);`
- 实现: src/xui_page.c:1024（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_page/main.c:213; test_xui/xui_page_test.c:164

## xuiPageGetColors
- 位置: xui.h:8279  已注释: 否
- 签名: `XUI_API int xuiPageGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pText, uint32_t* pHover, uint32_t* pActive, uint32_t* pCurrent, uint32_t* pCurrentText, uint32_t* pDisabledText);`
- 实现: src/xui_page.c:1039（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiPageSetFocusColor
- 位置: xui.h:8280  已注释: 否
- 签名: `XUI_API int xuiPageSetFocusColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_page.c:1054（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_page/main.c:222; test_xui/xui_page_test.c:166

## xuiPageGetFocusColor
- 位置: xui.h:8281  已注释: 否
- 签名: `XUI_API uint32_t xuiPageGetFocusColor(xui_widget pWidget);`
- 实现: src/xui_page.c:1062（体 5 行）

## xuiPageGetItemCount
- 位置: xui.h:8282  已注释: 否
- 签名: `XUI_API int xuiPageGetItemCount(xui_widget pWidget);`
- 实现: src/xui_page.c:1068（体 13 行）
- 用法: examples/xui_page/main.c:237; examples/xui_page/main.c:291; examples/xui_page/main.c:292

## xuiPageGetItemInfo
- 位置: xui.h:8283  已注释: 否
- 签名: `XUI_API int xuiPageGetItemInfo(xui_widget pWidget, int iIndex, xui_page_item_info_t* pInfo);`
- 实现: src/xui_page.c:1082（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_page/main.c:242; test_xui/xui_page_test.c:147; test_xui/xui_page_test.c:149

## xuiPageGetHoverItem
- 位置: xui.h:8284  已注释: 否
- 签名: `XUI_API int xuiPageGetHoverItem(xui_widget pWidget);`
- 实现: src/xui_page.c:1101（体 5 行）
- 用法: test_xui/xui_page_test.c:193

## xuiPageGetActiveItem
- 位置: xui.h:8285  已注释: 否
- 签名: `XUI_API int xuiPageGetActiveItem(xui_widget pWidget);`
- 实现: src/xui_page.c:1107（体 5 行）
- 用法: test_xui/xui_page_test.c:197

## xuiPageGetState
- 位置: xui.h:8286  已注释: 否
- 签名: `XUI_API uint32_t xuiPageGetState(xui_widget pWidget);`
- 实现: src/xui_page.c:1113（体 8 行）
- 用法: examples/xui_page/main.c:302; test_xui/xui_page_test.c:223

## xuiPageGetChangeCount
- 位置: xui.h:8287  已注释: 否
- 签名: `XUI_API int xuiPageGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_page.c:1122（体 5 行）

