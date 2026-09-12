# 草稿包：xge.h / Key（4 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeKeyDown
- 位置: xge.h:2590  已注释: 否
- 签名: `XGE_API int xgeKeyDown(int iKey);`
- 实现: src/xge_input.c:189（体 10 行）

## xgeKeyPressed
- 位置: xge.h:2591  已注释: 否
- 签名: `XGE_API int xgeKeyPressed(int iKey);`
- 实现: src/xge_input.c:200（体 10 行）
- 用法: examples/xge_particles/common.c:166; examples/xge_particles/common.c:168; examples/xge_particles/common.c:174

## xgeKeyRepeated
- 位置: xge.h:2592  已注释: 否
- 签名: `XGE_API int xgeKeyRepeated(int iKey);`
- 实现: src/xge_input.c:211（体 10 行）

## xgeKeyReleased
- 位置: xge.h:2593  已注释: 否
- 签名: `XGE_API int xgeKeyReleased(int iKey);`
- 实现: src/xge_input.c:222（体 10 行）

