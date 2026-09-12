# 草稿包：xui.h / radio（28 条 API）

> 生成 2026-09-10 03:03 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiRadioGetType
- 位置: xui.h:8128  已注释: 否
- 签名: `XUI_API xui_widget_type xuiRadioGetType(xui_context pContext);`
- 实现: src/xui_radio.c:926（体 34 行）
- 返回码: NULL

## xuiRadioCreate
- 位置: xui.h:8129  已注释: 否
- 签名: `XUI_API int xuiRadioCreate(xui_context pContext, xui_widget* ppWidget, const xui_radio_desc_t* pDesc);`
- 实现: src/xui_radio.c:961（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:43; examples/audit_xui_basic/main.c:45; examples/audit_xui_basic/main.c:47

## xuiRadioSetChange
- 位置: xui.h:8130  已注释: 否
- 签名: `XUI_API int xuiRadioSetChange(xui_widget pWidget, xui_radio_change_proc onChange, void* pUser);`
- 实现: src/xui_radio.c:976（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_radio/main.c:227; test_xui/xui_radio_test.c:135; test_xui/xui_radio_test.c:139

## xuiRadioSetText
- 位置: xui.h:8131  已注释: 否
- 签名: `XUI_API int xuiRadioSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_radio.c:985（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:232

## xuiRadioGetText
- 位置: xui.h:8132  已注释: 否
- 签名: `XUI_API const char* xuiRadioGetText(xui_widget pWidget);`
- 实现: src/xui_radio.c:998（体 5 行）
- 用法: test_xui/xui_radio_test.c:145

## xuiRadioSetFont
- 位置: xui.h:8133  已注释: 否
- 签名: `XUI_API int xuiRadioSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_radio.c:1004（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRadioGetFont
- 位置: xui.h:8134  已注释: 否
- 签名: `XUI_API xui_font xuiRadioGetFont(xui_widget pWidget);`
- 实现: src/xui_radio.c:1012（体 5 行）

## xuiRadioSetTextFlags
- 位置: xui.h:8135  已注释: 否
- 签名: `XUI_API int xuiRadioSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_radio.c:1018（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRadioGetTextFlags
- 位置: xui.h:8136  已注释: 否
- 签名: `XUI_API uint32_t xuiRadioGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_radio.c:1026（体 5 行）

## xuiRadioSetChecked
- 位置: xui.h:8137  已注释: 否
- 签名: `XUI_API int xuiRadioSetChecked(xui_widget pWidget, int bChecked);`
- 实现: src/xui_radio.c:1032（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:233

## xuiRadioGetChecked
- 位置: xui.h:8138  已注释: 否
- 签名: `XUI_API int xuiRadioGetChecked(xui_widget pWidget);`
- 实现: src/xui_radio.c:1039（体 5 行）
- 用法: examples/xui_radio/main.c:337; examples/xui_radio/main.c:338; test_xui/xui_radio_test.c:147

## xuiRadioSetTextColor
- 位置: xui.h:8139  已注释: 否
- 签名: `XUI_API int xuiRadioSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_radio.c:1045（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRadioGetTextColor
- 位置: xui.h:8140  已注释: 否
- 签名: `XUI_API uint32_t xuiRadioGetTextColor(xui_widget pWidget);`
- 实现: src/xui_radio.c:1053（体 5 行）

## xuiRadioSetDisabledTextColor
- 位置: xui.h:8141  已注释: 否
- 签名: `XUI_API int xuiRadioSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_radio.c:1059（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRadioGetDisabledTextColor
- 位置: xui.h:8142  已注释: 否
- 签名: `XUI_API uint32_t xuiRadioGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_radio.c:1067（体 5 行）

## xuiRadioSetIndicatorSize
- 位置: xui.h:8143  已注释: 否
- 签名: `XUI_API int xuiRadioSetIndicatorSize(xui_widget pWidget, float fSize);`
- 实现: src/xui_radio.c:1073（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_radio/main.c:234; test_xui/xui_indicator_pixel_test.c:46; test_xui/xui_radio_test.c:184

## xuiRadioGetIndicatorSize
- 位置: xui.h:8144  已注释: 否
- 签名: `XUI_API float xuiRadioGetIndicatorSize(xui_widget pWidget);`
- 实现: src/xui_radio.c:1081（体 5 行）
- 用法: examples/xui_radio/main.c:327; test_xui/xui_radio_test.c:148; test_xui/xui_radio_test.c:185

## xuiRadioSetGap
- 位置: xui.h:8145  已注释: 否
- 签名: `XUI_API int xuiRadioSetGap(xui_widget pWidget, float fGap);`
- 实现: src/xui_radio.c:1087（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_radio/main.c:235; test_xui/xui_indicator_pixel_test.c:30; test_xui/xui_radio_test.c:186

## xuiRadioGetGap
- 位置: xui.h:8146  已注释: 否
- 签名: `XUI_API float xuiRadioGetGap(xui_widget pWidget);`
- 实现: src/xui_radio.c:1095（体 5 行）
- 用法: test_xui/xui_radio_test.c:149; test_xui/xui_radio_test.c:187

## xuiRadioSetColors
- 位置: xui.h:8147  已注释: 否
- 签名: `XUI_API int xuiRadioSetColors(xui_widget pWidget, uint32_t iAccent, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocus);`
- 实现: src/xui_radio.c:1101（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_radio/main.c:236; test_xui/xui_radio_test.c:188

## xuiRadioGetColors
- 位置: xui.h:8148  已注释: 否
- 签名: `XUI_API int xuiRadioGetColors(xui_widget pWidget, uint32_t* pAccent, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocus);`
- 实现: src/xui_radio.c:1114（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRadioSetIndicatorSurface
- 位置: xui.h:8149  已注释: 否
- 签名: `XUI_API int xuiRadioSetIndicatorSurface(xui_widget pWidget, xui_surface pUncheckedSurface, xui_rect_t tUncheckedSrc, xui_surface pCheckedSurface, xui_rect_t tCheckedSrc);`
- 实现: src/xui_radio.c:1125（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_radio_test.c:198

## xuiRadioGetIndicatorSurface
- 位置: xui.h:8150  已注释: 否
- 签名: `XUI_API int xuiRadioGetIndicatorSurface(xui_widget pWidget, xui_surface* ppUncheckedSurface, xui_rect_t* pUncheckedSrc, xui_surface* ppCheckedSurface, xui_rect_t* pCheckedSrc);`
- 实现: src/xui_radio.c:1136（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRadioUseBuiltinAtlas
- 位置: xui.h:8151  已注释: 否
- 签名: `XUI_API int xuiRadioUseBuiltinAtlas(xui_widget pWidget, int bEnable);`
- 实现: src/xui_radio.c:1147（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_radio_test.c:195

## xuiRadioGetUseBuiltinAtlas
- 位置: xui.h:8152  已注释: 否
- 签名: `XUI_API int xuiRadioGetUseBuiltinAtlas(xui_widget pWidget);`
- 实现: src/xui_radio.c:1155（体 5 行）
- 用法: examples/xui_radio/main.c:328; test_xui/xui_radio_test.c:196

## xuiRadioGetIndicatorRect
- 位置: xui.h:8153  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRadioGetIndicatorRect(xui_widget pWidget);`
- 实现: src/xui_radio.c:1161（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:50

## xuiRadioGetTextRect
- 位置: xui.h:8154  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRadioGetTextRect(xui_widget pWidget);`
- 实现: src/xui_radio.c:1167（体 5 行）
- 用法: test_xui/xui_indicator_pixel_test.c:52

## xuiRadioGetState
- 位置: xui.h:8155  已注释: 否
- 签名: `XUI_API uint32_t xuiRadioGetState(xui_widget pWidget);`
- 实现: src/xui_radio.c:1173（体 5 行）

