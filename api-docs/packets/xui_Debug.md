# 草稿包：xui.h / Debug（5 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiDebugWidgetInspect
- 位置: xui.h:9741  已注释: 否
- 签名: `XUI_API int xuiDebugWidgetInspect(xui_widget pWidget, xui_debug_widget_info_t* pInfo);`
- 实现: src/xui_widget.c:7450（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:970; test_xui/xui_label_test.c:1004

## xuiDebugWidgetInspectAt
- 位置: xui.h:9742  已注释: 否
- 签名: `XUI_API xui_widget xuiDebugWidgetInspectAt(xui_context pContext, float fX, float fY, xui_debug_widget_info_t* pInfo);`
- 实现: src/xui_input.c:3088（体 10 行）
- 用法: test_xui/xui_input_test.c:974

## xuiDebugWidgetTreeDump
- 位置: xui.h:9743  已注释: 否
- 签名: `XUI_API int xuiDebugWidgetTreeDump(xui_context pContext, xui_widget pRoot, char* sBuffer, int iCapacity);`
- 实现: src/xui_widget.c:7540（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_test.c:975

## xuiDebugLayoutSnapshot
- 位置: xui.h:9744  已注释: 否
- 签名: `XUI_API int xuiDebugLayoutSnapshot(xui_context pContext, char* sBuffer, int iCapacity);`
- 实现: src/xui_widget.c:7569（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_input_test.c:977

## xuiDebugEventTrace
- 位置: xui.h:9745  已注释: 否
- 签名: `XUI_API int xuiDebugEventTrace(xui_context pContext, const xui_event_t* pEvent, char* sBuffer, int iCapacity);`
- 实现: src/xui_widget.c:7589（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_input_test.c:983

