# 草稿包：xui.h / Clear（3 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiClearDamage
- 位置: xui.h:5866  已注释: 否
- 签名: `XUI_API void xuiClearDamage(xui_context pContext);`
- 实现: src/xui/core/xui_core.c:2082（体 7 行）
- 用法: tests/xui/xui_context_test.c:89; tests/xui/xui_context_test.c:98; tests/xui/xui_context_test.c:108

## xuiClearFonts
- 位置: xui.h:5918  已注释: 否
- 签名: `XUI_API void xuiClearFonts(xui_context pContext);`
- 实现: src/xui/core/xui_core.c:2030（体 8 行）
- 用法: tests/xui/xui_style_test.c:321

## xuiClearEvents
- 位置: xui.h:6131  已注释: 否
- 签名: `XUI_API void xuiClearEvents(xui_context pContext);`
- 实现: src/xui/core/xui_input.c:2442（体 8 行）
- 用法: examples/xui/xui_input_layer/main.c:384; tests/xui/xui_drag_drop_reentry_test.c:240; tests/xui/xui_drag_drop_test.c:109

