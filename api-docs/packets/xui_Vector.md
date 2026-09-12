# 草稿包：xui.h / Vector（2 条 API）

> 生成 2026-09-10 02:52 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiVectorIconGetCount
- 位置: xui.h:6071  已注释: 否
- 签名: `XUI_API int xuiVectorIconGetCount(void);`
- 实现: src/xui_core.c:2572（体 4 行）
- 用法: test_xui/xui_vector_smoke_test.c:78

## xuiVectorIconGetName
- 位置: xui.h:6072  已注释: 否
- 签名: `XUI_API const char* xuiVectorIconGetName(int iIndex);`
- 实现: src/xui_core.c:2577（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_vector_smoke_test.c:78

