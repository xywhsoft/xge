# 草稿包：xge.h / Mouse（4 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeMouseGet
- 位置: xge.h:2596  已注释: 否
- 签名: `XGE_API void xgeMouseGet(float* pX, float* pY);`
- 实现: src/xge_input.c:249（体 9 行）
- 用法: test/test_native_input_coordinates.c:117; test/test_native_input_coordinates.c:136

## xgeMouseGetDelta
- 位置: xge.h:2597  已注释: 否
- 签名: `XGE_API void xgeMouseGetDelta(float* pDX, float* pDY);`
- 实现: src/xge_input.c:259（体 9 行）

## xgeMouseGetWheel
- 位置: xge.h:2598  已注释: 否
- 签名: `XGE_API void xgeMouseGetWheel(float* pX, float* pY);`
- 实现: src/xge_input.c:269（体 9 行）

## xgeMouseDown
- 位置: xge.h:2599  已注释: 否
- 签名: `XGE_API int xgeMouseDown(int iButton);`
- 实现: src/xge_input.c:279（体 4 行）

