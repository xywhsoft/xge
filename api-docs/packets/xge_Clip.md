# 草稿包：xge.h / Clip（5 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeClipSet
- 位置: xge.h:2337  已注释: 否
- 签名: `XGE_API void xgeClipSet(xge_rect_t tRect);`
- 实现: src/xge_render.c:617（体 75 行）
- 用法: examples/xge_particles/common.c:263; test/test_native_input_coordinates.c:304

## xgeClipGet
- 位置: xge.h:2338  已注释: 否
- 签名: `XGE_API xge_rect_t xgeClipGet(void);`
- 实现: src/xge_render.c:701（体 10 行）
- 用法: test/test_native_input_coordinates.c:305

## xgeClipClear
- 位置: xge.h:2339  已注释: 否
- 签名: `XGE_API void xgeClipClear(void);`
- 实现: src/xge_render.c:720（体 10 行）
- 用法: examples/xge_particles/common.c:265; test/test_native_input_coordinates.c:309

## xgeClipSetPixels
- 位置: xge.h:2345  已注释: 否
- 签名: `XGE_API void xgeClipSetPixels(xge_rect_i_t tRect);`
- 实现: src/xge_render.c:693（体 7 行）

## xgeClipGetPixels
- 位置: xge.h:2346  已注释: 否
- 签名: `XGE_API xge_rect_i_t xgeClipGetPixels(void);`
- 实现: src/xge_render.c:712（体 7 行）

