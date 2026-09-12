# 草稿包：xui.h / Get（30 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiGetDefaultLanguage
- 位置: xui.h:5793  已注释: 否
- 签名: `XUI_API int xuiGetDefaultLanguage(void);`
- 实现: src/xui_core.c:494（体 4 行）
- 用法: test_xui/xui_context_test.c:47; test_xui/xui_context_test.c:70

## xuiGetLanguage
- 位置: xui.h:5795  已注释: 否
- 签名: `XUI_API int xuiGetLanguage(xui_context pContext);`
- 实现: src/xui_core.c:518（体 4 行）
- 用法: examples/xui_language/main.c:244; examples/xui_language/main.c:511; test_xui/xui_context_test.c:48

## xuiGetLanguageRevision
- 位置: xui.h:5796  已注释: 否
- 签名: `XUI_API uint32_t xuiGetLanguageRevision(xui_context pContext);`
- 实现: src/xui_core.c:523（体 4 行）
- 用法: examples/xui_language/main.c:247; test_xui/xui_context_test.c:52; test_xui/xui_context_test.c:68

## xuiGetLanguagePack
- 位置: xui.h:5797  已注释: 否
- 签名: `XUI_API xui_language xuiGetLanguagePack(xui_context pContext, int iLanguageId);`
- 实现: src/xui_core.c:528（体 4 行）
- 用法: examples/xui_language/main.c:245; examples/xui_language/main.c:411; examples/xui_language/main.c:412

## xuiGetLanguageId
- 位置: xui.h:5799  已注释: 否
- 签名: `XUI_API int xuiGetLanguageId(xui_language pLanguage);`
- 实现: src/xui_core.c:555（体 4 行）
- 用法: examples/xui_language/main.c:228; examples/xui_language/main.c:410; examples/xui_language/main.c:415

## xuiGetLanguageTextArray
- 位置: xui.h:5802  已注释: 否
- 签名: `XUI_API xarray* xuiGetLanguageTextArray(xui_language pLanguage);`
- 实现: src/xui_core.c:596（体 4 行）
- 用法: examples/xui_language/main.c:205; examples/xui_language/main.c:413; test_xui/xui_context_test.c:60

## xuiGetLanguageCode
- 位置: xui.h:5803  已注释: 否
- 签名: `XUI_API const char* xuiGetLanguageCode(xui_language pLanguage);`
- 实现: src/xui_core.c:601（体 4 行）
- 用法: examples/xui_language/main.c:247

## xuiGetLanguageName
- 位置: xui.h:5804  已注释: 否
- 签名: `XUI_API const char* xuiGetLanguageName(xui_language pLanguage);`
- 实现: src/xui_core.c:606（体 4 行）
- 用法: examples/xui_language/main.c:247

## xuiGetProxy
- 位置: xui.h:5807  已注释: 否
- 签名: `XUI_API int xuiGetProxy(xui_context pContext, xui_proxy_t* pProxy);`
- 实现: src/xui_core.c:1706（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: examples/xui_input/main.c:338; test_xui/xui_context_test.c:46

## xuiGetProxyCaps
- 位置: xui.h:5808  已注释: 否
- 签名: `XUI_API int xuiGetProxyCaps(xui_context pContext, xui_proxy_caps_t* pCaps);`
- 实现: src/xui_core.c:1718（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: test_xui/xui_context_test.c:80

## xuiGetViewportSize
- 位置: xui.h:5833  已注释: 否
- 签名: `XUI_API xui_size_t xuiGetViewportSize(xui_context pContext);`
- 实现: src/xui_core.c:1749（体 12 行）
- 用法: test_xui/xui_context_test.c:44

## xuiGetVirtualDpi
- 位置: xui.h:5836  已注释: 否
- 签名: `XUI_API float xuiGetVirtualDpi(xui_context pContext);`
- 实现: src/xui_core.c:1795（体 7 行）
- 用法: test_xui/xui_context_test.c:43; test_xui/xui_render_schedule_test.c:1125; test_xui/xui_render_schedule_test.c:1136

## xuiGetInteractionPolicy
- 位置: xui.h:5839  已注释: 否
- 签名: `XUI_API int xuiGetInteractionPolicy(xui_context pContext, xui_interaction_policy_t* pPolicy);`
- 实现: src/xui_core.c:1527（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:781

## xuiGetDamageRects
- 位置: xui.h:5843  已注释: 否
- 签名: `XUI_API int xuiGetDamageRects(xui_context pContext, xui_rect_i_t* pRects, int iCapacity);`
- 实现: src/xui_core.c:2064（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_context_test.c:86; test_xui/xui_context_test.c:95; test_xui/xui_context_test.c:101

## xuiGetRenderStats
- 位置: xui.h:5846  已注释: 否
- 签名: `XUI_API int xuiGetRenderStats(xui_context pContext, xui_render_stats_t* pStats);`
- 实现: src/xui_widget.c:7209（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:386; examples/xui_breadcrumb/main.c:385; examples/xui_button/main.c:637

## xuiGetLayoutStats
- 位置: xui.h:5849  已注释: 否
- 签名: `XUI_API int xuiGetLayoutStats(xui_context pContext, xui_layout_stats_t* pStats);`
- 实现: src/xui_widget.c:2734（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_layout/main.c:83; examples/xui_layout/main.c:98; test_xui/xui_layout_callback_test.c:259

## xuiGetCacheBudget
- 位置: xui.h:5851  已注释: 否
- 签名: `XUI_API size_t xuiGetCacheBudget(xui_context pContext);`
- 实现: src/xui_widget.c:7235（体 4 行）

## xuiGetCacheStats
- 位置: xui.h:5852  已注释: 否
- 签名: `XUI_API int xuiGetCacheStats(xui_context pContext, xui_cache_stats_t* pStats);`
- 实现: src/xui_widget.c:7240（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_accordion/main.c:387; examples/xui_breadcrumb/main.c:386; examples/xui_canvas/main.c:428

## xuiGetTheme
- 位置: xui.h:5878  已注释: 否
- 签名: `XUI_API int xuiGetTheme(xui_context pContext, xui_theme_t* pTheme);`
- 实现: src/xui_core.c:1886（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_test.c:296

## xuiGetChromeStyle
- 位置: xui.h:5880  已注释: 否
- 签名: `XUI_API int xuiGetChromeStyle(xui_context pContext, xui_chrome_style_t* pChrome);`
- 实现: src/xui_core.c:1931（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_test.c:309; test_xui/xui_style_tooltip_test.c:49; test_xui/xui_style_tooltip_test.c:60

## xuiGetDefaultFont
- 位置: xui.h:5882  已注释: 否
- 签名: `XUI_API xui_font xuiGetDefaultFont(xui_context pContext);`
- 实现: src/xui_core.c:1952（体 4 行）
- 用法: test_xui/xui_style_test.c:320; test_xui/xui_style_test.c:322

## xuiGetRenderNodeCount
- 位置: xui.h:5995  已注释: 否
- 签名: `XUI_API int xuiGetRenderNodeCount(xui_context pContext);`
- 实现: src/xui_widget.c:7339（体 7 行）
- 用法: test_xui/xui_dock_panel_test.c:1385; test_xui/xui_input_test.c:28; test_xui/xui_property_grid_test.c:172

## xuiGetRenderNode
- 位置: xui.h:5996  已注释: 否
- 签名: `XUI_API int xuiGetRenderNode(xui_context pContext, int iIndex, xui_render_node_t* pNode);`
- 实现: src/xui_widget.c:7347（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_dock_panel_test.c:1389; test_xui/xui_input_test.c:31; test_xui/xui_property_grid_test.c:174

## xuiGetHoverWidget
- 位置: xui.h:6028  已注释: 否
- 签名: `XUI_API xui_widget xuiGetHoverWidget(xui_context pContext);`
- 实现: src/xui_input.c:2659（体 4 行）
- 用法: examples/xui_input_layer/main.c:290; test_xui/xui_input_test.c:469; test_xui/xui_input_test.c:539

## xuiGetActiveWidget
- 位置: xui.h:6029  已注释: 否
- 签名: `XUI_API xui_widget xuiGetActiveWidget(xui_context pContext);`
- 实现: src/xui_input.c:2664（体 4 行）
- 用法: test_xui/xui_input_test.c:476; test_xui/xui_input_test.c:533; test_xui/xui_state_event_test.c:223

## xuiGetFocusWidget
- 位置: xui.h:6030  已注释: 否
- 签名: `XUI_API xui_widget xuiGetFocusWidget(xui_context pContext);`
- 实现: src/xui_input.c:2669（体 4 行）
- 用法: examples/xui_input_layer/main.c:296; test_xui/xui_accessibility_test.c:546; test_xui/xui_cascader_test.c:182

## xuiGetPointerCapture
- 位置: xui.h:6034  已注释: 否
- 签名: `XUI_API xui_widget xuiGetPointerCapture(xui_context pContext);`
- 实现: src/xui_input.c:2801（体 4 行）
- 用法: test_xui/xui_button_test.c:849; test_xui/xui_button_test.c:859; test_xui/xui_canvas_test.c:157

## xuiGetPointerCaptureEx
- 位置: xui.h:6037  已注释: 否
- 签名: `XUI_API xui_widget xuiGetPointerCaptureEx(xui_context pContext, uint64_t iPointerId, int iPointerType);`
- 实现: src/xui_input.c:2790（体 10 行）
- 返回码: NULL
- 用法: examples/xui_multitouch/main.c:236; examples/xui_multitouch/main.c:490; examples/xui_multitouch/main.c:496

## xuiGetRootWidget
- 位置: xui.h:9148  已注释: 否
- 签名: `XUI_API xui_widget xuiGetRootWidget(xui_context pContext);`
- 实现: src/xui_widget.c:2785（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_callback_lifetime_test.c:146; test_xui/xui_callback_lifetime_test.c:187; test_xui/xui_callback_lifetime_test.c:204

## xuiGetImeCandidateRect
- 位置: xui.h:9409  已注释: 否
- 签名: `XUI_API xui_rect_t xuiGetImeCandidateRect(xui_context pContext);`
- 实现: src/xui_widget.c:5153（体 10 行）
- 用法: test_xui/xui_code_edit_test.c:738; test_xui/xui_code_edit_test.c:745; test_xui/xui_code_edit_test.c:1153

