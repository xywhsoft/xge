# 草稿包：xui.h / virtual_joystick（24 条 API）

> 生成 2026-09-10 03:09 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiVirtualJoystickGetType
- 位置: xui.h:8816  已注释: 否
- 签名: `XUI_API xui_widget_type xuiVirtualJoystickGetType(xui_context pContext);`
- 实现: src/xui_virtual_joystick.c:940（体 33 行）
- 返回码: NULL

## xuiVirtualJoystickCreate
- 位置: xui.h:8817  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickCreate(xui_context pContext, xui_widget* ppWidget, const xui_virtual_joystick_desc_t* pDesc);`
- 实现: src/xui_virtual_joystick.c:974（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch188_main1.c:16; examples/tutorial_capture/ch206_main1.c:49; examples/xui_virtual_joystick/main.c:195

## xuiVirtualJoystickSetChange
- 位置: xui.h:8818  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetChange(xui_widget pWidget, xui_virtual_joystick_change_proc onChange, void* pUser);`
- 实现: src/xui_virtual_joystick.c:989（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiVirtualJoystickSetValue
- 位置: xui.h:8819  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetValue(xui_widget pWidget, float fX, float fY, int bNotify);`
- 实现: src/xui_virtual_joystick.c:1000（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_virtual_joystick_test.c:135

## xuiVirtualJoystickReset
- 位置: xui.h:8820  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickReset(xui_widget pWidget, int bNotify);`
- 实现: src/xui_virtual_joystick.c:1019（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_virtual_joystick_test.c:142

## xuiVirtualJoystickGetState
- 位置: xui.h:8821  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetState(xui_widget pWidget, xui_virtual_joystick_state_t* pState);`
- 实现: src/xui_virtual_joystick.c:1043（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_virtual_joystick/main.c:318; examples/xui_virtual_joystick/main.c:327; examples/xui_virtual_joystick/main.c:361

## xuiVirtualJoystickGetX
- 位置: xui.h:8822  已注释: 否
- 签名: `XUI_API float xuiVirtualJoystickGetX(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1055（体 7 行）
- 用法: test_xui/xui_virtual_joystick_test.c:137

## xuiVirtualJoystickGetY
- 位置: xui.h:8823  已注释: 否
- 签名: `XUI_API float xuiVirtualJoystickGetY(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1063（体 7 行）

## xuiVirtualJoystickGetMagnitude
- 位置: xui.h:8824  已注释: 否
- 签名: `XUI_API float xuiVirtualJoystickGetMagnitude(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1071（体 7 行）
- 用法: test_xui/xui_virtual_joystick_test.c:138

## xuiVirtualJoystickGetAngle
- 位置: xui.h:8825  已注释: 否
- 签名: `XUI_API float xuiVirtualJoystickGetAngle(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1079（体 7 行）

## xuiVirtualJoystickSetChannel
- 位置: xui.h:8826  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetChannel(xui_widget pWidget, int iChannel, int bPressed, float fValue, int bNotify);`
- 实现: src/xui_virtual_joystick.c:1087（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_virtual_joystick/main.c:316; examples/xui_virtual_joystick/main.c:317; test_xui/xui_virtual_joystick_test.c:120

## xuiVirtualJoystickGetChannel
- 位置: xui.h:8827  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetChannel(xui_widget pWidget, int iChannel, int* pPressed, float* pValue);`
- 实现: src/xui_virtual_joystick.c:1104（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_virtual_joystick_test.c:127

## xuiVirtualJoystickClearChannels
- 位置: xui.h:8828  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickClearChannels(xui_widget pWidget, int bNotify);`
- 实现: src/xui_virtual_joystick.c:1117（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_virtual_joystick/main.c:320; test_xui/xui_virtual_joystick_test.c:140

## xuiVirtualJoystickSetMetrics
- 位置: xui.h:8829  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetMetrics(xui_widget pWidget, float fRadius, float fKnobSize, float fDeadZone);`
- 实现: src/xui_virtual_joystick.c:1134（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_virtual_joystick_test.c:169

## xuiVirtualJoystickGetMetrics
- 位置: xui.h:8830  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetMetrics(xui_widget pWidget, float* pRadius, float* pKnobSize, float* pDeadZone);`
- 实现: src/xui_virtual_joystick.c:1149（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_virtual_joystick_test.c:103

## xuiVirtualJoystickUseBuiltinAtlas
- 位置: xui.h:8831  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickUseBuiltinAtlas(xui_widget pWidget, int bEnable);`
- 实现: src/xui_virtual_joystick.c:1161（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_virtual_joystick_test.c:171

## xuiVirtualJoystickGetUseBuiltinAtlas
- 位置: xui.h:8832  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetUseBuiltinAtlas(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1171（体 7 行）
- 用法: examples/xui_virtual_joystick/main.c:200; test_xui/xui_virtual_joystick_test.c:101; test_xui/xui_virtual_joystick_test.c:172

## xuiVirtualJoystickSetSurface
- 位置: xui.h:8833  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetSurface(xui_widget pWidget, int iPart, xui_surface pSurface, xui_rect_t tSrc);`
- 实现: src/xui_virtual_joystick.c:1179（体 32 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiVirtualJoystickGetSurface
- 位置: xui.h:8834  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetSurface(xui_widget pWidget, int iPart, xui_surface* ppSurface, xui_rect_t* pSrc);`
- 实现: src/xui_virtual_joystick.c:1212（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiVirtualJoystickSetColors
- 位置: xui.h:8835  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickSetColors(xui_widget pWidget, uint32_t iBase, uint32_t iBaseActive, uint32_t iKnob, uint32_t iKnobActive, uint32_t iRipple, uint32_t iFocus, uint32_t iDisabled);`
- 实现: src/xui_virtual_joystick.c:1251（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_virtual_joystick_test.c:173

## xuiVirtualJoystickGetColors
- 位置: xui.h:8836  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetColors(xui_widget pWidget, uint32_t* pBase, uint32_t* pBaseActive, uint32_t* pKnob, uint32_t* pKnobActive, uint32_t* pRipple, uint32_t* pFocus, uint32_t* pDisabled);`
- 实现: src/xui_virtual_joystick.c:1267（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiVirtualJoystickGetChangeCount
- 位置: xui.h:8837  已注释: 否
- 签名: `XUI_API int xuiVirtualJoystickGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1283（体 7 行）
- 用法: examples/xui_virtual_joystick/main.c:385; test_xui/xui_virtual_joystick_test.c:126

## xuiVirtualJoystickGetBaseRect
- 位置: xui.h:8838  已注释: 否
- 签名: `XUI_API xui_rect_t xuiVirtualJoystickGetBaseRect(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1291（体 7 行）
- 用法: examples/xui_virtual_joystick/main.c:308; examples/xui_virtual_joystick/main.c:321; test_xui/xui_virtual_joystick_test.c:114

## xuiVirtualJoystickGetKnobRect
- 位置: xui.h:8839  已注释: 否
- 签名: `XUI_API xui_rect_t xuiVirtualJoystickGetKnobRect(xui_widget pWidget);`
- 实现: src/xui_virtual_joystick.c:1299（体 7 行）
- 用法: test_xui/xui_virtual_joystick_test.c:115

