# 草稿包：xui.h / stepbar（23 条 API）

> 生成 2026-09-10 03:05 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiStepBarGetType
- 位置: xui.h:6726  已注释: 否
- 签名: `XUI_API xui_widget_type xuiStepBarGetType(xui_context pContext);`
- 实现: src/xui_step_bar.c:714（体 33 行）
- 返回码: NULL

## xuiStepBarCreate
- 位置: xui.h:6727  已注释: 否
- 签名: `XUI_API int xuiStepBarCreate(xui_context pContext, xui_widget* ppWidget, const xui_step_bar_desc_t* pDesc);`
- 实现: src/xui_step_bar.c:748（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch172_main1.c:19; examples/xui_stepbar/main.c:144; test_xui/xui_step_bar_test.c:70

## xuiStepBarSetSteps
- 位置: xui.h:6728  已注释: 否
- 签名: `XUI_API int xuiStepBarSetSteps(xui_widget pWidget, const char* const* ppTitles, int iStepCount);`
- 实现: src/xui_step_bar.c:763（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:424

## xuiStepBarSetTitle
- 位置: xui.h:6729  已注释: 否
- 签名: `XUI_API int xuiStepBarSetTitle(xui_widget pWidget, int iIndex, const char* sTitle);`
- 实现: src/xui_step_bar.c:776（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_stepbar/main.c:184; test_xui/xui_step_bar_test.c:99

## xuiStepBarGetStepCount
- 位置: xui.h:6730  已注释: 否
- 签名: `XUI_API int xuiStepBarGetStepCount(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:789（体 7 行）
- 用法: test_xui/xui_step_bar_test.c:75

## xuiStepBarGetTitle
- 位置: xui.h:6731  已注释: 否
- 签名: `XUI_API const char* xuiStepBarGetTitle(xui_widget pWidget, int iIndex);`
- 实现: src/xui_step_bar.c:797（体 7 行）
- 用法: test_xui/xui_step_bar_test.c:76; test_xui/xui_step_bar_test.c:100

## xuiStepBarSetCurrent
- 位置: xui.h:6732  已注释: 否
- 签名: `XUI_API int xuiStepBarSetCurrent(xui_widget pWidget, int iCurrent);`
- 实现: src/xui_step_bar.c:805（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_stepbar/main.c:181; examples/xui_stepbar/main.c:182; examples/xui_stepbar/main.c:183

## xuiStepBarGetCurrent
- 位置: xui.h:6733  已注释: 否
- 签名: `XUI_API int xuiStepBarGetCurrent(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:817（体 7 行）
- 用法: examples/xui_stepbar/main.c:290; test_xui/xui_step_bar_test.c:77; test_xui/xui_step_bar_test.c:94

## xuiStepBarSetStyle
- 位置: xui.h:6734  已注释: 否
- 签名: `XUI_API int xuiStepBarSetStyle(xui_widget pWidget, int iStyle);`
- 实现: src/xui_step_bar.c:825（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_step_bar_test.c:97; test_xui/xui_style_chrome_test.c:427; test_xui/xui_style_chrome_test.c:429

## xuiStepBarGetStyle
- 位置: xui.h:6735  已注释: 否
- 签名: `XUI_API int xuiStepBarGetStyle(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:837（体 7 行）
- 用法: test_xui/xui_step_bar_test.c:98

## xuiStepBarSetFont
- 位置: xui.h:6736  已注释: 否
- 签名: `XUI_API int xuiStepBarSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_step_bar.c:845（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStepBarGetFont
- 位置: xui.h:6737  已注释: 否
- 签名: `XUI_API xui_font xuiStepBarGetFont(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:855（体 7 行）

## xuiStepBarSetColors
- 位置: xui.h:6738  已注释: 否
- 签名: `XUI_API int xuiStepBarSetColors(xui_widget pWidget, uint32_t iDone, uint32_t iActive, uint32_t iPending, uint32_t iLine);`
- 实现: src/xui_step_bar.c:863（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_step_bar_test.c:101; test_xui/xui_style_chrome_test.c:431

## xuiStepBarGetColors
- 位置: xui.h:6739  已注释: 否
- 签名: `XUI_API int xuiStepBarGetColors(xui_widget pWidget, uint32_t* pDone, uint32_t* pActive, uint32_t* pPending, uint32_t* pLine);`
- 实现: src/xui_step_bar.c:876（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiStepBarSetTextColors
- 位置: xui.h:6740  已注释: 否
- 签名: `XUI_API int xuiStepBarSetTextColors(xui_widget pWidget, uint32_t iText, uint32_t iActiveText, uint32_t iPendingText);`
- 实现: src/xui_step_bar.c:889（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStepBarGetTextColors
- 位置: xui.h:6741  已注释: 否
- 签名: `XUI_API int xuiStepBarGetTextColors(xui_widget pWidget, uint32_t* pText, uint32_t* pActiveText, uint32_t* pPendingText);`
- 实现: src/xui_step_bar.c:901（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiStepBarSetBackgroundColor
- 位置: xui.h:6742  已注释: 否
- 签名: `XUI_API int xuiStepBarSetBackgroundColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_step_bar.c:913（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiStepBarGetBackgroundColor
- 位置: xui.h:6743  已注释: 否
- 签名: `XUI_API uint32_t xuiStepBarGetBackgroundColor(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:923（体 7 行）

## xuiStepBarSetMetrics
- 位置: xui.h:6744  已注释: 否
- 签名: `XUI_API int xuiStepBarSetMetrics(xui_widget pWidget, float fBarHeight, float fDotRadius, float fLineWidth);`
- 实现: src/xui_step_bar.c:931（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_step_bar_test.c:103

## xuiStepBarGetMetrics
- 位置: xui.h:6745  已注释: 否
- 签名: `XUI_API int xuiStepBarGetMetrics(xui_widget pWidget, float* pBarHeight, float* pDotRadius, float* pLineWidth);`
- 实现: src/xui_step_bar.c:943（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiStepBarGetStepRect
- 位置: xui.h:6746  已注释: 否
- 签名: `XUI_API xui_rect_t xuiStepBarGetStepRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_step_bar.c:955（体 8 行）
- 用法: examples/xui_stepbar/main.c:271; test_xui/xui_step_bar_test.c:117; test_xui/xui_step_bar_test.c:119

## xuiStepBarGetIndicatorRect
- 位置: xui.h:6747  已注释: 否
- 签名: `XUI_API xui_rect_t xuiStepBarGetIndicatorRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_step_bar.c:964（体 8 行）
- 用法: examples/xui_stepbar/main.c:270; test_xui/xui_step_bar_test.c:123

## xuiStepBarGetChangeCount
- 位置: xui.h:6748  已注释: 否
- 签名: `XUI_API int xuiStepBarGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_step_bar.c:973（体 7 行）
- 用法: examples/xui_stepbar/main.c:185; test_xui/xui_step_bar_test.c:105

