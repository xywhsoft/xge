# 草稿包：xge.h / Depth（2 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeDepthTestSet
- 位置: xge.h:1690  已注释: 否
- 签名: `XGE_API void xgeDepthTestSet(int bEnabled);`
- 实现: src/xge_core.c:1415（体 5 行）

## xgeDepthTestGet
- 位置: xge.h:1691  已注释: 否
- 签名: `XGE_API int xgeDepthTestGet(void);`
- 实现: src/xge_core.c:1421（体 4 行）

