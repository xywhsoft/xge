# 草稿包：xui.h / Proxy（4 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiProxyXge
- 位置: xui.h:9862  已注释: 否
- 签名: `XUI_API xui_proxy_t xuiProxyXge(void);`
- 实现: src/xui_proxy_xge.c:2975（体 76 行）
- 用法: examples/xui_accordion/main.c:305; examples/xui_breadcrumb/main.c:284; examples/xui_button/main.c:541

## xuiProxyXgePumpKeyboard
- 位置: xui.h:9864  已注释: 是
- 签名: `XUI_API int xuiProxyXgePumpKeyboard(xui_context pContext);`
- 既有注释: /* Pumps keyboard, text and IME events without changing pointer state. */
- 实现: src/xui_proxy_xge.c:2940（体 4 行）

## xuiProxyXgePumpInput
- 位置: xui.h:9865  已注释: 否
- 签名: `XUI_API int xuiProxyXgePumpInput(xui_context pContext);`
- 实现: src/xui_proxy_xge.c:2962（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/xui_split_layout_dock_repro/main.c:529; test_xui/xui_split_layout_test.c:87

## xuiProxyXgePumpInputRect
- 位置: xui.h:9866  已注释: 否
- 签名: `XUI_API int xuiProxyXgePumpInputRect(xui_context pContext, xui_rect_t tWindowRect);`
- 实现: src/xui_proxy_xge.c:2945（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_accordion/main.c:360; examples/xui_breadcrumb/main.c:346; examples/xui_button/main.c:609

