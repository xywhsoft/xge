# 草稿包：xge.h / Viewport（5 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeViewportSet
- 位置: xge.h:2456  已注释: 否
- 签名: `XGE_API void xgeViewportSet(xge_rect_t tRect);`
- 实现: src/xge_render.c:473（体 29 行）
- 用法: examples/audit_camera_viewport/main.c:53; examples/audit_camera_viewport/main.c:63; examples/audit_camera_viewport/main.c:73

## xgeViewportGet
- 位置: xge.h:2457  已注释: 否
- 签名: `XGE_API xge_rect_t xgeViewportGet(void);`
- 实现: src/xge_render.c:503（体 13 行）
- 用法: test/test_native_input_coordinates.c:301

## xgeViewportClear
- 位置: xge.h:2458  已注释: 否
- 签名: `XGE_API void xgeViewportClear(void);`
- 实现: src/xge_render.c:517（体 13 行）
- 用法: examples/audit_camera_viewport/main.c:55; examples/audit_camera_viewport/main.c:65; examples/audit_camera_viewport/main.c:75

## xgeViewportSetPixels
- 位置: xge.h:2468  已注释: 否
- 签名: `XGE_API void xgeViewportSetPixels(xge_rect_i_t tRect);`
- 实现: src/xge_render.c:604（体 7 行）

## xgeViewportGetPixels
- 位置: xge.h:2469  已注释: 否
- 签名: `XGE_API xge_rect_i_t xgeViewportGetPixels(void);`
- 实现: src/xge_render.c:612（体 4 行）

