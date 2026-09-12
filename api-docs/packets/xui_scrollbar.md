# 草稿包：xui.h / scrollbar（31 条 API）

> 生成 2026-09-10 03:04 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiScrollBarGetType
- 位置: xui.h:8337  已注释: 否
- 签名: `XUI_API xui_widget_type xuiScrollBarGetType(xui_context pContext);`
- 实现: src/xui_scrollbar.c:1406（体 35 行）
- 返回码: NULL

## xuiScrollBarCreate
- 位置: xui.h:8338  已注释: 否
- 签名: `XUI_API int xuiScrollBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_scrollbar_desc_t* pDesc);`
- 实现: src/xui_scrollbar.c:1442（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_scrollbar/main.c:164; test_xui/xui_accessibility_test.c:248; test_xui/xui_scrollbar_test.c:133

## xuiScrollBarSetChange
- 位置: xui.h:8339  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetChange(xui_widget pWidget, xui_scrollbar_change_proc onChange, void* pUser);`
- 实现: src/xui_scrollbar.c:1457（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollbar/main.c:166; test_xui/xui_accessibility_test.c:252; test_xui/xui_scrollbar_test.c:138

## xuiScrollBarSetRange
- 位置: xui.h:8340  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetRange(xui_widget pWidget, float fMin, float fMax, float fPage);`
- 实现: src/xui_scrollbar.c:1466（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:249; test_xui/xui_scrollbar_test.c:335; test_xui/xui_scroll_pixel_test.c:39

## xuiScrollBarGetRange
- 位置: xui.h:8341  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetRange(xui_widget pWidget, float* pMin, float* pMax, float* pPage);`
- 实现: src/xui_scrollbar.c:1482（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scrollbar_test.c:339

## xuiScrollBarSetPage
- 位置: xui.h:8342  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetPage(xui_widget pWidget, float fPage);`
- 实现: src/xui_scrollbar.c:1492（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollBarGetPage
- 位置: xui.h:8343  已注释: 否
- 签名: `XUI_API float xuiScrollBarGetPage(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1502（体 5 行）

## xuiScrollBarSetValue
- 位置: xui.h:8344  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetValue(xui_widget pWidget, float fValue);`
- 实现: src/xui_scrollbar.c:1508（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:251; test_xui/xui_scrollbar_test.c:194; test_xui/xui_scrollbar_test.c:219

## xuiScrollBarGetValue
- 位置: xui.h:8345  已注释: 否
- 签名: `XUI_API float xuiScrollBarGetValue(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1515（体 5 行）
- 用法: test_xui/xui_accessibility_test.c:470; test_xui/xui_code_edit_test.c:1263; test_xui/xui_code_edit_test.c:1263

## xuiScrollBarSetSteps
- 位置: xui.h:8346  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetSteps(xui_widget pWidget, float fSmallStep, float fLargeStep);`
- 实现: src/xui_scrollbar.c:1521（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:250

## xuiScrollBarGetSteps
- 位置: xui.h:8347  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetSteps(xui_widget pWidget, float* pSmallStep, float* pLargeStep);`
- 实现: src/xui_scrollbar.c:1532（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollBarSetOrientation
- 位置: xui.h:8348  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_scrollbar.c:1541（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:304; test_xui/xui_scrollbar_test.c:305; test_xui/xui_scroll_pixel_test.c:36

## xuiScrollBarGetOrientation
- 位置: xui.h:8349  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetOrientation(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1551（体 5 行）
- 用法: test_xui/xui_scrollbar_test.c:144

## xuiScrollBarSetMode
- 位置: xui.h:8350  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetMode(xui_widget pWidget, int iMode);`
- 实现: src/xui_scrollbar.c:1557（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scrollbar_test.c:301

## xuiScrollBarGetMode
- 位置: xui.h:8351  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetMode(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1566（体 5 行）
- 用法: examples/xui_scrollbar/main.c:251; examples/xui_scrollbar/main.c:252; examples/xui_scrollview/main.c:317

## xuiScrollBarSetButtonMode
- 位置: xui.h:8352  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetButtonMode(xui_widget pWidget, int iButtonMode);`
- 实现: src/xui_scrollbar.c:1572（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_scrollbar_test.c:303; test_xui/xui_scroll_pixel_test.c:37

## xuiScrollBarGetButtonMode
- 位置: xui.h:8353  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetButtonMode(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1581（体 5 行）
- 用法: test_xui/xui_scrollbar_test.c:143

## xuiScrollBarSetMetrics
- 位置: xui.h:8354  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetMetrics(xui_widget pWidget, float fThickness, float fMinThumbSize, float fButtonSize);`
- 实现: src/xui_scrollbar.c:1587（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollbar/main.c:168; test_xui/xui_scrollbar_test.c:307; test_xui/xui_scroll_pixel_test.c:38

## xuiScrollBarGetMetrics
- 位置: xui.h:8355  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetMetrics(xui_widget pWidget, float* pThickness, float* pMinThumbSize, float* pButtonSize);`
- 实现: src/xui_scrollbar.c:1601（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollview/main.c:305; examples/xui_scrollview/main.c:306; examples/xui_scrollview/main.c:307

## xuiScrollBarSetColors
- 位置: xui.h:8356  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetColors(xui_widget pWidget, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_scrollbar.c:1611（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollbar/main.c:203

## xuiScrollBarGetColors
- 位置: xui.h:8357  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetColors(xui_widget pWidget, uint32_t* pTrack, uint32_t* pThumb, uint32_t* pHover, uint32_t* pActive, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_scrollbar.c:1627（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollBarSetButtonColors
- 位置: xui.h:8358  已注释: 否
- 签名: `XUI_API int xuiScrollBarSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iIcon);`
- 实现: src/xui_scrollbar.c:1640（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_scrollbar/main.c:210

## xuiScrollBarGetButtonColors
- 位置: xui.h:8359  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pIcon);`
- 实现: src/xui_scrollbar.c:1650（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiScrollBarGetTrackRect
- 位置: xui.h:8360  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollBarGetTrackRect(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1677（体 4 行）
- 用法: test_xui/xui_scrollbar_test.c:156; test_xui/xui_scrollbar_test.c:314; test_xui/xui_scroll_pixel_test.c:47

## xuiScrollBarGetThumbRect
- 位置: xui.h:8361  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollBarGetThumbRect(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1682（体 4 行）
- 用法: examples/xui_scrollbar/main.c:249; test_xui/xui_code_edit_test.c:1271; test_xui/xui_scrollbar_test.c:157

## xuiScrollBarGetDecreaseRect
- 位置: xui.h:8362  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollBarGetDecreaseRect(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1687（体 4 行）
- 用法: examples/xui_scrollbar/main.c:247; examples/xui_scrollbar/main.c:248; test_xui/xui_scrollbar_test.c:158

## xuiScrollBarGetIncreaseRect
- 位置: xui.h:8363  已注释: 否
- 签名: `XUI_API xui_rect_t xuiScrollBarGetIncreaseRect(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1692（体 4 行）
- 用法: test_xui/xui_scrollbar_test.c:159; test_xui/xui_scrollbar_test.c:317; test_xui/xui_scroll_pixel_test.c:46

## xuiScrollBarGetHoverPart
- 位置: xui.h:8364  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetHoverPart(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1697（体 5 行）
- 用法: test_xui/xui_scrollbar_test.c:168

## xuiScrollBarGetActivePart
- 位置: xui.h:8365  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetActivePart(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1703（体 5 行）
- 用法: test_xui/xui_scrollbar_test.c:229; test_xui/xui_scrollbar_test.c:275; test_xui/xui_scrollbar_test.c:283

## xuiScrollBarGetState
- 位置: xui.h:8366  已注释: 否
- 签名: `XUI_API uint32_t xuiScrollBarGetState(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1709（体 7 行）
- 用法: test_xui/xui_scrollbar_test.c:343

## xuiScrollBarGetChangeCount
- 位置: xui.h:8367  已注释: 否
- 签名: `XUI_API int xuiScrollBarGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_scrollbar.c:1717（体 5 行）

