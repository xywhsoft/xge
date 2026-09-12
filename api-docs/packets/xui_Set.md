# 草稿包：xui.h / Set（16 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiSetErrorCallback
- 位置: xui.h:5789  已注释: 否
- 签名: `XUI_API int xuiSetErrorCallback(xui_context pContext, xui_error_proc onError, void* pUser);`
- 实现: src/xui_core.c:952（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_callback_lifetime_test.c:257; test_xui/xui_layout_callback_test.c:168; test_xui/xui_render_lifetime_test.c:140

## xuiSetAccessibilityEventCallback
- 位置: xui.h:5790  已注释: 否
- 签名: `XUI_API int xuiSetAccessibilityEventCallback(xui_context pContext, xui_accessibility_event_proc onEvent, void* pUser);`
- 实现: src/xui_core.c:962（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_accessibility_test.c:287; test_xui/xui_rich_edit_test.c:150

## xuiSetDefaultLanguage
- 位置: xui.h:5792  已注释: 否
- 签名: `XUI_API int xuiSetDefaultLanguage(int iLanguageId);`
- 实现: src/xui_core.c:485（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_context_test.c:69; test_xui/xui_context_test.c:71

## xuiSetLanguage
- 位置: xui.h:5794  已注释: 否
- 签名: `XUI_API int xuiSetLanguage(xui_context pContext, int iLanguageId);`
- 实现: src/xui_core.c:499（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_language/main.c:274; examples/xui_language/main.c:415; test_xui/xui_context_test.c:50

## xuiSetProxy
- 位置: xui.h:5806  已注释: 否
- 签名: `XUI_API int xuiSetProxy(xui_context pContext, const xui_proxy_t* pProxy);`
- 实现: src/xui_core.c:1684（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_ALREADY_INITIALIZED, XUI_OK
- 用法: examples/xui_accordion/main.c:308; examples/xui_breadcrumb/main.c:287; examples/xui_button/main.c:547

## xuiSetViewportSize
- 位置: xui.h:5832  已注释: 否
- 签名: `XUI_API int xuiSetViewportSize(xui_context pContext, int iWidth, int iHeight);`
- 实现: src/xui_core.c:1730（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_layout/main.c:39; test_xui/xui_accessibility_test.c:218; test_xui/xui_cache_transition_test.c:24

## xuiSetVirtualDpi
- 位置: xui.h:5835  已注释: 否
- 签名: `XUI_API int xuiSetVirtualDpi(xui_context pContext, float fDpiScale);`
- 实现: src/xui_core.c:1767（体 27 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_icon/main.c:333; test_xui/xui_context_test.c:104; test_xui/xui_icon_test.c:356

## xuiSetInteractionPolicy
- 位置: xui.h:5838  已注释: 否
- 签名: `XUI_API int xuiSetInteractionPolicy(xui_context pContext, const xui_interaction_policy_t* pPolicy);`
- 实现: src/xui_core.c:1522（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:785; test_xui/xui_code_edit_test.c:1205

## xuiSetCacheBudget
- 位置: xui.h:5850  已注释: 否
- 签名: `XUI_API int xuiSetCacheBudget(xui_context pContext, size_t iBudgetBytes);`
- 实现: src/xui_widget.c:7219（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_render_schedule_test.c:1412

## xuiSetTheme
- 位置: xui.h:5877  已注释: 否
- 签名: `XUI_API int xuiSetTheme(xui_context pContext, const xui_theme_t* pTheme);`
- 实现: src/xui_core.c:1848（体 37 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_test.c:293

## xuiSetChromeStyle
- 位置: xui.h:5879  已注释: 否
- 签名: `XUI_API int xuiSetChromeStyle(xui_context pContext, const xui_chrome_style_t* pChrome);`
- 实现: src/xui_core.c:1896（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_test.c:306; test_xui/xui_style_tooltip_test.c:70

## xuiSetDefaultFont
- 位置: xui.h:5881  已注释: 否
- 签名: `XUI_API int xuiSetDefaultFont(xui_context pContext, xui_font pFont);`
- 实现: src/xui_core.c:1941（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:324; examples/xui_breadcrumb/main.c:303; examples/xui_button/main.c:569

## xuiSetFocusWidget
- 位置: xui.h:6031  已注释: 否
- 签名: `XUI_API int xuiSetFocusWidget(xui_context pContext, xui_widget pWidget);`
- 实现: src/xui_input.c:2892（体 8 行）
- 用法: examples/xui_cascader/main.c:335; examples/xui_codeedit/main.c:273; examples/xui_codeedit/main.c:357

## xuiSetPointerCapture
- 位置: xui.h:6032  已注释: 否
- 签名: `XUI_API int xuiSetPointerCapture(xui_context pContext, xui_widget pWidget);`
- 实现: src/xui_input.c:2740（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_multitouch/main.c:235; test_xui/xui_input_test.c:226; test_xui/xui_input_test.c:635

## xuiSetPointerCaptureEx
- 位置: xui.h:6035  已注释: 否
- 签名: `XUI_API int xuiSetPointerCaptureEx(xui_context pContext, uint64_t iPointerId, int iPointerType, xui_widget pWidget);`
- 实现: src/xui_input.c:2731（体 8 行）

## xuiSetRootWidget
- 位置: xui.h:9147  已注释: 否
- 签名: `XUI_API int xuiSetRootWidget(xui_context pContext, xui_widget pWidget);`
- 实现: src/xui_widget.c:2775（体 9 行）
- 用法: examples/xui_accordion/main.c:189; examples/xui_breadcrumb/main.c:231; examples/xui_button/main.c:400

