# 草稿包：xui.h / toggle（38 条 API）

> 生成 2026-09-10 03:08 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiToggleGetType
- 位置: xui.h:8443  已注释: 否
- 签名: `XUI_API xui_widget_type xuiToggleGetType(xui_context pContext);`
- 实现: src/xui_toggle.c:1003（体 34 行）
- 返回码: NULL

## xuiToggleCreate
- 位置: xui.h:8444  已注释: 否
- 签名: `XUI_API int xuiToggleCreate(xui_context pContext, xui_widget* ppWidget, const xui_toggle_desc_t* pDesc);`
- 实现: src/xui_toggle.c:1038（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:54; examples/audit_xui_basic/main.c:56; examples/audit_xui_basic/main.c:58

## xuiToggleSetChange
- 位置: xui.h:8445  已注释: 否
- 签名: `XUI_API int xuiToggleSetChange(xui_widget pWidget, xui_toggle_change_proc onChange, void* pUser);`
- 实现: src/xui_toggle.c:1053（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_toggle/main.c:181; test_xui/xui_toggle_test.c:166

## xuiToggleSetText
- 位置: xui.h:8446  已注释: 否
- 签名: `XUI_API int xuiToggleSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_toggle.c:1062（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:224; test_xui/xui_toggle_test.c:164

## xuiToggleGetText
- 位置: xui.h:8447  已注释: 否
- 签名: `XUI_API const char* xuiToggleGetText(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1075（体 5 行）
- 用法: test_xui/xui_toggle_test.c:113; test_xui/xui_toggle_test.c:165

## xuiToggleSetInnerText
- 位置: xui.h:8448  已注释: 否
- 签名: `XUI_API int xuiToggleSetInnerText(xui_widget pWidget, const char* sUncheckedText, const char* sCheckedText);`
- 实现: src/xui_toggle.c:1081（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_indicator_pixel_test.c:34; test_xui/xui_style_basic_choices_test.c:57; test_xui/xui_toggle_test.c:135

## xuiToggleGetUncheckedText
- 位置: xui.h:8449  已注释: 否
- 签名: `XUI_API const char* xuiToggleGetUncheckedText(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1093（体 5 行）
- 用法: examples/xui_toggle/main.c:327; test_xui/xui_toggle_test.c:137; test_xui/xui_toggle_test.c:153

## xuiToggleGetCheckedText
- 位置: xui.h:8450  已注释: 否
- 签名: `XUI_API const char* xuiToggleGetCheckedText(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1099（体 5 行）
- 用法: examples/xui_toggle/main.c:328; test_xui/xui_toggle_test.c:138; test_xui/xui_toggle_test.c:153

## xuiToggleSetInnerTextColor
- 位置: xui.h:8451  已注释: 否
- 签名: `XUI_API int xuiToggleSetInnerTextColor(xui_widget pWidget, uint32_t iUncheckedColor, uint32_t iCheckedColor);`
- 实现: src/xui_toggle.c:1105（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toggle_test.c:139

## xuiToggleGetInnerTextColor
- 位置: xui.h:8452  已注释: 否
- 签名: `XUI_API int xuiToggleGetInnerTextColor(xui_widget pWidget, uint32_t* pUncheckedColor, uint32_t* pCheckedColor);`
- 实现: src/xui_toggle.c:1116（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToggleSetInnerTextMetrics
- 位置: xui.h:8453  已注释: 否
- 签名: `XUI_API int xuiToggleSetInnerTextMetrics(xui_widget pWidget, float fPadding, float fGap);`
- 实现: src/xui_toggle.c:1125（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_indicator_pixel_test.c:35; test_xui/xui_toggle_test.c:141

## xuiToggleGetInnerTextMetrics
- 位置: xui.h:8454  已注释: 否
- 签名: `XUI_API int xuiToggleGetInnerTextMetrics(xui_widget pWidget, float* pPadding, float* pGap);`
- 实现: src/xui_toggle.c:1136（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToggleSetFont
- 位置: xui.h:8455  已注释: 否
- 签名: `XUI_API int xuiToggleSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_toggle.c:1145（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToggleGetFont
- 位置: xui.h:8456  已注释: 否
- 签名: `XUI_API xui_font xuiToggleGetFont(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1153（体 5 行）

## xuiToggleSetChecked
- 位置: xui.h:8457  已注释: 否
- 签名: `XUI_API int xuiToggleSetChecked(xui_widget pWidget, int bChecked);`
- 实现: src/xui_toggle.c:1159（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:225; test_xui/xui_indicator_pixel_test.c:62; test_xui/xui_toggle_test.c:155

## xuiToggleGetChecked
- 位置: xui.h:8458  已注释: 否
- 签名: `XUI_API int xuiToggleGetChecked(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1166（体 5 行）
- 用法: examples/xui_toggle/main.c:325; examples/xui_toggle/main.c:343; test_xui/xui_toggle_test.c:114

## xuiToggleSetTextColor
- 位置: xui.h:8459  已注释: 否
- 签名: `XUI_API int xuiToggleSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_toggle.c:1172（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToggleGetTextColor
- 位置: xui.h:8460  已注释: 否
- 签名: `XUI_API uint32_t xuiToggleGetTextColor(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1180（体 5 行）

## xuiToggleSetDisabledTextColor
- 位置: xui.h:8461  已注释: 否
- 签名: `XUI_API int xuiToggleSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_toggle.c:1186（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiToggleGetDisabledTextColor
- 位置: xui.h:8462  已注释: 否
- 签名: `XUI_API uint32_t xuiToggleGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1194（体 5 行）

## xuiToggleSetTrackSize
- 位置: xui.h:8463  已注释: 否
- 签名: `XUI_API int xuiToggleSetTrackSize(xui_widget pWidget, float fWidth, float fHeight);`
- 实现: src/xui_toggle.c:1200（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toggle/main.c:288; examples/xui_toggle/main.c:296; examples/xui_toggle/main.c:297

## xuiToggleGetTrackWidth
- 位置: xui.h:8464  已注释: 否
- 签名: `XUI_API float xuiToggleGetTrackWidth(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1209（体 5 行）
- 用法: test_xui/xui_toggle_test.c:115; test_xui/xui_toggle_test.c:159

## xuiToggleGetTrackHeight
- 位置: xui.h:8465  已注释: 否
- 签名: `XUI_API float xuiToggleGetTrackHeight(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1215（体 5 行）
- 用法: test_xui/xui_toggle_test.c:116; test_xui/xui_toggle_test.c:159

## xuiToggleSetThumbSize
- 位置: xui.h:8466  已注释: 否
- 签名: `XUI_API int xuiToggleSetThumbSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_toggle.c:1221（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toggle/main.c:289; test_xui/xui_indicator_pixel_test.c:33; test_xui/xui_toggle_test.c:160

## xuiToggleGetThumbSize
- 位置: xui.h:8467  已注释: 否
- 签名: `XUI_API float xuiToggleGetThumbSize(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1229（体 5 行）
- 用法: examples/xui_toggle/main.c:329; test_xui/xui_toggle_test.c:117; test_xui/xui_toggle_test.c:161

## xuiToggleSetGap
- 位置: xui.h:8468  已注释: 否
- 签名: `XUI_API int xuiToggleSetGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_toggle.c:1235（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_indicator_pixel_test.c:31; test_xui/xui_toggle_test.c:162

## xuiToggleGetGap
- 位置: xui.h:8469  已注释: 否
- 签名: `XUI_API float xuiToggleGetGap(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1243（体 5 行）
- 用法: test_xui/xui_toggle_test.c:118; test_xui/xui_toggle_test.c:163

## xuiToggleSetColors
- 位置: xui.h:8470  已注释: 否
- 签名: `XUI_API int xuiToggleSetColors(xui_widget pWidget, uint32_t iAccent, uint32_t iTrack, uint32_t iHoverTrack, uint32_t iFocus);`
- 实现: src/xui_toggle.c:1249（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toggle/main.c:290; test_xui/xui_toggle_test.c:185

## xuiToggleGetColors
- 位置: xui.h:8471  已注释: 否
- 签名: `XUI_API int xuiToggleGetColors(xui_widget pWidget, uint32_t* pAccent, uint32_t* pTrack, uint32_t* pHoverTrack, uint32_t* pFocus);`
- 实现: src/xui_toggle.c:1262（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToggleSetIndicatorSurface
- 位置: xui.h:8472  已注释: 否
- 签名: `XUI_API int xuiToggleSetIndicatorSurface(xui_widget pWidget, xui_surface pUncheckedSurface, xui_rect_t tUncheckedSrc, xui_surface pCheckedSurface, xui_rect_t tCheckedSrc);`
- 实现: src/xui_toggle.c:1273（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_toggle/main.c:298; examples/xui_toggle/main.c:299; test_xui/xui_toggle_test.c:193

## xuiToggleGetIndicatorSurface
- 位置: xui.h:8473  已注释: 否
- 签名: `XUI_API int xuiToggleGetIndicatorSurface(xui_widget pWidget, xui_surface* ppUncheckedSurface, xui_rect_t* pUncheckedSrc, xui_surface* ppCheckedSurface, xui_rect_t* pCheckedSrc);`
- 实现: src/xui_toggle.c:1284（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiToggleUseBuiltinAtlas
- 位置: xui.h:8474  已注释: 否
- 签名: `XUI_API int xuiToggleUseBuiltinAtlas(xui_widget pWidget, int bEnable);`
- 实现: src/xui_toggle.c:1295（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_toggle_test.c:191

## xuiToggleGetUseBuiltinAtlas
- 位置: xui.h:8475  已注释: 否
- 签名: `XUI_API int xuiToggleGetUseBuiltinAtlas(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1303（体 5 行）
- 用法: test_xui/xui_toggle_test.c:192

## xuiToggleGetTrackRect
- 位置: xui.h:8476  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToggleGetTrackRect(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1309（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:64; test_xui/xui_toggle_test.c:131; test_xui/xui_toggle_test.c:147

## xuiToggleGetThumbRect
- 位置: xui.h:8477  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToggleGetThumbRect(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1315（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:65; test_xui/xui_toggle_test.c:132; test_xui/xui_toggle_test.c:148

## xuiToggleGetTextRect
- 位置: xui.h:8478  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToggleGetTextRect(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1321（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:67

## xuiToggleGetInnerTextRect
- 位置: xui.h:8479  已注释: 否
- 签名: `XUI_API xui_rect_t xuiToggleGetInnerTextRect(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1327（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:66; test_xui/xui_toggle_test.c:149

## xuiToggleGetState
- 位置: xui.h:8480  已注释: 否
- 签名: `XUI_API uint32_t xuiToggleGetState(xui_widget pWidget);`
- 实现: src/xui_toggle.c:1333（体 5 行）
- 用法: test_xui/xui_toggle_test.c:157

