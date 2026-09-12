# 草稿包：xui.h / checkbox（28 条 API）

> 生成 2026-09-10 02:57 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCheckBoxGetType
- 位置: xui.h:7748  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCheckBoxGetType(xui_context pContext);`
- 实现: src/xui_checkbox.c:856（体 34 行）
- 返回码: NULL
- 用法: test_xui/xui_code_edit_test.c:1109

## xuiCheckBoxCreate
- 位置: xui.h:7749  已注释: 否
- 签名: `XUI_API int xuiCheckBoxCreate(xui_context pContext, xui_widget* ppWidget, const xui_checkbox_desc_t* pDesc);`
- 实现: src/xui_checkbox.c:891（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:32; examples/audit_xui_basic/main.c:34; examples/audit_xui_basic/main.c:36

## xuiCheckBoxSetChange
- 位置: xui.h:7750  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetChange(xui_widget pWidget, xui_checkbox_change_proc onChange, void* pUser);`
- 实现: src/xui_checkbox.c:906（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_checkbox/main.c:179; test_xui/xui_checkbox_test.c:107

## xuiCheckBoxSetText
- 位置: xui.h:7751  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_checkbox.c:919（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:228; test_xui/xui_checkbox_test.c:105

## xuiCheckBoxGetText
- 位置: xui.h:7752  已注释: 否
- 签名: `XUI_API const char* xuiCheckBoxGetText(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:936（体 7 行）
- 用法: test_xui/xui_checkbox_test.c:77; test_xui/xui_checkbox_test.c:106; test_xui/xui_code_edit_test.c:248

## xuiCheckBoxSetFont
- 位置: xui.h:7753  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_checkbox.c:944（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCheckBoxGetFont
- 位置: xui.h:7754  已注释: 否
- 签名: `XUI_API xui_font xuiCheckBoxGetFont(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:956（体 7 行）

## xuiCheckBoxSetTextFlags
- 位置: xui.h:7755  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_checkbox.c:964（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCheckBoxGetTextFlags
- 位置: xui.h:7756  已注释: 否
- 签名: `XUI_API uint32_t xuiCheckBoxGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:974（体 7 行）

## xuiCheckBoxSetChecked
- 位置: xui.h:7757  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetChecked(xui_widget pWidget, int bChecked);`
- 实现: src/xui_checkbox.c:982（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:229; test_xui/xui_checkbox_test.c:98; test_xui/xui_checkbox_test.c:122

## xuiCheckBoxGetChecked
- 位置: xui.h:7758  已注释: 否
- 签名: `XUI_API int xuiCheckBoxGetChecked(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:993（体 7 行）
- 用法: examples/xui_checkbox/main.c:246; examples/xui_checkbox/main.c:262; test_xui/xui_checkbox_test.c:78

## xuiCheckBoxSetTextColor
- 位置: xui.h:7759  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_checkbox.c:1001（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCheckBoxGetTextColor
- 位置: xui.h:7760  已注释: 否
- 签名: `XUI_API uint32_t xuiCheckBoxGetTextColor(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1009（体 5 行）

## xuiCheckBoxSetDisabledTextColor
- 位置: xui.h:7761  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_checkbox.c:1015（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCheckBoxGetDisabledTextColor
- 位置: xui.h:7762  已注释: 否
- 签名: `XUI_API uint32_t xuiCheckBoxGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1023（体 5 行）

## xuiCheckBoxSetIndicatorSize
- 位置: xui.h:7763  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetIndicatorSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_checkbox.c:1029（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_checkbox/main.c:216; test_xui/xui_checkbox_test.c:101; test_xui/xui_indicator_pixel_test.c:45

## xuiCheckBoxGetIndicatorSize
- 位置: xui.h:7764  已注释: 否
- 签名: `XUI_API float xuiCheckBoxGetIndicatorSize(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1037（体 5 行）
- 用法: examples/xui_checkbox/main.c:248; test_xui/xui_checkbox_test.c:79; test_xui/xui_checkbox_test.c:102

## xuiCheckBoxSetGap
- 位置: xui.h:7765  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_checkbox.c:1043（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_checkbox/main.c:217; test_xui/xui_checkbox_test.c:103; test_xui/xui_indicator_pixel_test.c:29

## xuiCheckBoxGetGap
- 位置: xui.h:7766  已注释: 否
- 签名: `XUI_API float xuiCheckBoxGetGap(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1051（体 5 行）
- 用法: test_xui/xui_checkbox_test.c:80; test_xui/xui_checkbox_test.c:104

## xuiCheckBoxSetColors
- 位置: xui.h:7767  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetColors(xui_widget pWidget, uint32_t iAccent, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocus);`
- 实现: src/xui_checkbox.c:1057（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_checkbox/main.c:218

## xuiCheckBoxGetColors
- 位置: xui.h:7768  已注释: 否
- 签名: `XUI_API int xuiCheckBoxGetColors(xui_widget pWidget, uint32_t* pAccent, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocus);`
- 实现: src/xui_checkbox.c:1070（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCheckBoxSetIndicatorSurface
- 位置: xui.h:7769  已注释: 否
- 签名: `XUI_API int xuiCheckBoxSetIndicatorSurface(xui_widget pWidget, xui_surface pUncheckedSurface, xui_rect_t tUncheckedSrc, xui_surface pCheckedSurface, xui_rect_t tCheckedSrc);`
- 实现: src/xui_checkbox.c:1081（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_checkbox_test.c:143

## xuiCheckBoxGetIndicatorSurface
- 位置: xui.h:7770  已注释: 否
- 签名: `XUI_API int xuiCheckBoxGetIndicatorSurface(xui_widget pWidget, xui_surface* ppUncheckedSurface, xui_rect_t* pUncheckedSrc, xui_surface* ppCheckedSurface, xui_rect_t* pCheckedSrc);`
- 实现: src/xui_checkbox.c:1092（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCheckBoxUseBuiltinAtlas
- 位置: xui.h:7771  已注释: 否
- 签名: `XUI_API int xuiCheckBoxUseBuiltinAtlas(xui_widget pWidget, int bEnable);`
- 实现: src/xui_checkbox.c:1103（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_checkbox_test.c:140

## xuiCheckBoxGetUseBuiltinAtlas
- 位置: xui.h:7772  已注释: 否
- 签名: `XUI_API int xuiCheckBoxGetUseBuiltinAtlas(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1111（体 5 行）
- 用法: examples/xui_checkbox/main.c:249; test_xui/xui_checkbox_test.c:141; test_xui/xui_checkbox_test.c:144

## xuiCheckBoxGetIndicatorRect
- 位置: xui.h:7773  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCheckBoxGetIndicatorRect(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1117（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:49

## xuiCheckBoxGetTextRect
- 位置: xui.h:7774  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCheckBoxGetTextRect(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1123（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:51

## xuiCheckBoxGetState
- 位置: xui.h:7775  已注释: 否
- 签名: `XUI_API uint32_t xuiCheckBoxGetState(xui_widget pWidget);`
- 实现: src/xui_checkbox.c:1129（体 5 行）
- 用法: test_xui/xui_checkbox_test.c:100

