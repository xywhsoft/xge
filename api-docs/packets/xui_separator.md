# 草稿包：xui.h / separator（13 条 API）

> 生成 2026-09-10 03:05 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiSeparatorGetType
- 位置: xui.h:6641  已注释: 否
- 签名: `XUI_API xui_widget_type xuiSeparatorGetType(xui_context pContext);`
- 实现: src/xui_separator.c:416（体 33 行）
- 返回码: NULL

## xuiSeparatorCreate
- 位置: xui.h:6642  已注释: 否
- 签名: `XUI_API int xuiSeparatorCreate(xui_context pContext, xui_widget* ppWidget, const xui_separator_desc_t* pDesc);`
- 实现: src/xui_separator.c:450（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch142_main1.c:29; examples/tutorial_capture/ch142_main1.c:44; examples/xui_separator/main.c:249

## xuiSeparatorSetColor
- 位置: xui.h:6643  已注释: 否
- 签名: `XUI_API int xuiSeparatorSetColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_separator.c:465（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_separator_test.c:118

## xuiSeparatorGetColor
- 位置: xui.h:6644  已注释: 否
- 签名: `XUI_API uint32_t xuiSeparatorGetColor(xui_widget pWidget);`
- 实现: src/xui_separator.c:480（体 7 行）

## xuiSeparatorSetThickness
- 位置: xui.h:6645  已注释: 否
- 签名: `XUI_API int xuiSeparatorSetThickness(xui_widget pWidget, float fThickness);`
- 实现: src/xui_separator.c:488（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_separator_test.c:116; test_xui/xui_separator_test.c:142; test_xui/xui_separator_test.c:170

## xuiSeparatorGetThickness
- 位置: xui.h:6646  已注释: 否
- 签名: `XUI_API float xuiSeparatorGetThickness(xui_widget pWidget);`
- 实现: src/xui_separator.c:506（体 7 行）

## xuiSeparatorSetOrientation
- 位置: xui.h:6647  已注释: 否
- 签名: `XUI_API int xuiSeparatorSetOrientation(xui_widget pWidget, int iOrientation);`
- 实现: src/xui_separator.c:514（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_separator_test.c:136

## xuiSeparatorGetOrientation
- 位置: xui.h:6648  已注释: 否
- 签名: `XUI_API int xuiSeparatorGetOrientation(xui_widget pWidget);`
- 实现: src/xui_separator.c:532（体 7 行）
- 用法: examples/xui_separator/main.c:394; test_xui/xui_separator_test.c:110

## xuiSeparatorSetAlign
- 位置: xui.h:6649  已注释: 否
- 签名: `XUI_API int xuiSeparatorSetAlign(xui_widget pWidget, int iAlign);`
- 实现: src/xui_separator.c:540（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_separator/main.c:253; test_xui/xui_separator_test.c:138

## xuiSeparatorGetAlign
- 位置: xui.h:6650  已注释: 否
- 签名: `XUI_API int xuiSeparatorGetAlign(xui_widget pWidget);`
- 实现: src/xui_separator.c:558（体 7 行）
- 用法: test_xui/xui_separator_test.c:111

## xuiSeparatorSetLineStyle
- 位置: xui.h:6651  已注释: 否
- 签名: `XUI_API int xuiSeparatorSetLineStyle(xui_widget pWidget, int iLineStyle);`
- 实现: src/xui_separator.c:566（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_separator_test.c:128; test_xui/xui_separator_test.c:140; test_xui/xui_separator_test.c:172

## xuiSeparatorGetLineStyle
- 位置: xui.h:6652  已注释: 否
- 签名: `XUI_API int xuiSeparatorGetLineStyle(xui_widget pWidget);`
- 实现: src/xui_separator.c:584（体 7 行）
- 用法: examples/xui_separator/main.c:390; examples/xui_separator/main.c:391; examples/xui_separator/main.c:392

## xuiSeparatorGetLineRect
- 位置: xui.h:6653  已注释: 否
- 签名: `XUI_API xui_rect_t xuiSeparatorGetLineRect(xui_widget pWidget);`
- 实现: src/xui_separator.c:592（体 14 行）
- 用法: examples/xui_separator/main.c:386; test_xui/xui_separator_test.c:113; test_xui/xui_separator_test.c:148

