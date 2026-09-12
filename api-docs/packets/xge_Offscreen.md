# 草稿包：xge.h / Offscreen（4 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeOffscreenInit
- 位置: xge.h:1726  已注释: 否
- 签名: `XGE_API int xgeOffscreenInit(xge_offscreen pOffscreen, int iWidth, int iHeight);`
- 实现: src/xge_egl.c:308（体 34 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch99_main1.c:13

## xgeOffscreenUnit
- 位置: xge.h:1727  已注释: 否
- 签名: `XGE_API void xgeOffscreenUnit(xge_offscreen pOffscreen);`
- 实现: src/xge_egl.c:343（体 9 行）
- 用法: examples/tutorial_capture/ch99_main1.c:52

## xgeOffscreenRenderTarget
- 位置: xge.h:1728  已注释: 否
- 签名: `XGE_API xge_render_target xgeOffscreenRenderTarget(xge_offscreen pOffscreen);`
- 实现: src/xge_egl.c:353（体 7 行）
- 返回码: NULL
- 用法: examples/tutorial_capture/ch99_main1.c:16

## xgeOffscreenReadPixels
- 位置: xge.h:1729  已注释: 否
- 签名: `XGE_API int xgeOffscreenReadPixels(xge_offscreen pOffscreen, void* pPixels, int iStride);`
- 实现: src/xge_egl.c:361（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch99_main1.c:31

