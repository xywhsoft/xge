# 草稿包：xge.h / Touch（3 条 API）

> 生成 2026-09-10 02:39 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeTouchGetCount
- 位置: xge.h:2628  已注释: 否
- 签名: `XGE_API int xgeTouchGetCount(void);`
- 实现: src/xge_input.c:578（体 4 行）

## xgeTouchGet
- 位置: xge.h:2629  已注释: 否
- 签名: `XGE_API int xgeTouchGet(int iIndex, xge_touch_point_t* pPoint);`
- 实现: src/xge_input.c:583（体 8 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeTouchFind
- 位置: xge.h:2630  已注释: 否
- 签名: `XGE_API int xgeTouchFind(uint64_t iId, xge_touch_point_t* pPoint);`
- 实现: src/xge_input.c:592（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_FILE_NOT_FOUND, XGE_OK

