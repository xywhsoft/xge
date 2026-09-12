# 草稿包：xui.h / Hot（4 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiHotKeyRegister
- 位置: xui.h:6156  已注释: 否
- 签名: `XUI_API int xuiHotKeyRegister(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers, xui_widget_event_proc onEvent, void* pUser);`
- 实现: src/xui_input.c:3004（体 4 行）
- 用法: test_xui/xui_callback_lifetime_test.c:223; test_xui/xui_callback_lifetime_test.c:225; test_xui/xui_input_test.c:814

## xuiHotKeyRegisterCommand
- 位置: xui.h:6157  已注释: 否
- 签名: `XUI_API int xuiHotKeyRegisterCommand(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers, int iCommand, const char* sCommand, void* pData);`
- 实现: src/xui_input.c:3009（体 4 行）
- 用法: test_xui/xui_input_test.c:812

## xuiHotKeyUnregister
- 位置: xui.h:6158  已注释: 否
- 签名: `XUI_API int xuiHotKeyUnregister(xui_context pContext, xui_widget pWidget, int iKey, uint32_t iModifiers);`
- 实现: src/xui_input.c:3014（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_callback_lifetime_test.c:82; test_xui/xui_input_test.c:850

## xuiHotKeyClearWidget
- 位置: xui.h:6159  已注释: 否
- 签名: `XUI_API int xuiHotKeyClearWidget(xui_widget pWidget);`
- 实现: src/xui_input.c:3038（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

