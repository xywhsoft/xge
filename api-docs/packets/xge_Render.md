# 草稿包：xge.h / Render（14 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeRenderRequest
- 位置: xge.h:1588  已注释: 否
- 签名: `XGE_API void xgeRenderRequest(void);`
- 实现: src/xge_core.c:198（体 4 行）

## xgeRenderRequestAfter
- 位置: xge.h:1590  已注释: 是
- 签名: `XGE_API void xgeRenderRequestAfter(float fDelaySeconds);`
- 既有注释: /* Requests an on-demand frame after at most fDelaySeconds. Main-thread only. */
- 实现: src/xge_core.c:203（体 15 行）

## xgeRender
- 位置: xge.h:1592  已注释: 否
- 签名: `XGE_API int xgeRender(void);`
- 实现: src/xge_core.c:1073（体 13 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_UNSUPPORTED

## xgeRenderThreadCapsGet
- 位置: xge.h:1672  已注释: 否
- 签名: `XGE_API int xgeRenderThreadCapsGet(xge_render_thread_caps_t* pCaps);`
- 实现: src/xge_core.c:1304（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeRenderThreadEGLSet
- 位置: xge.h:1673  已注释: 否
- 签名: `XGE_API int xgeRenderThreadEGLSet(const xge_egl_desc_t* pDesc);`
- 实现: src/xge_core.c:1285（体 18 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_ALREADY_INITIALIZED, XGE_OK, XGE_ERROR_UNSUPPORTED

## xgeRenderThreadSet
- 位置: xge.h:1674  已注释: 否
- 签名: `XGE_API int xgeRenderThreadSet(int bEnabled);`
- 实现: src/xge_core.c:1261（体 18 行）
- 返回码: XGE_ERROR_NOT_INITIALIZED, XGE_OK, XGE_ERROR_UNSUPPORTED

## xgeRenderThreadGet
- 位置: xge.h:1675  已注释: 否
- 签名: `XGE_API int xgeRenderThreadGet(void);`
- 实现: src/xge_core.c:1280（体 4 行）

## xgeRenderTargetWindow
- 位置: xge.h:1938  已注释: 否
- 签名: `XGE_API int xgeRenderTargetWindow(xge_render_target pTarget);`
- 实现: src/xge_render_target.c:72（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeRenderTargetCreate
- 位置: xge.h:1939  已注释: 否
- 签名: `XGE_API int xgeRenderTargetCreate(xge_render_target pTarget, int iWidth, int iHeight);`
- 实现: src/xge_render_target.c:84（体 24 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_camera_viewport/main.c:137; examples/audit_ninepatch_sprite/main.c:148; examples/audit_shape_basic/main.c:368

## xgeRenderTargetResize
- 位置: xge.h:1940  已注释: 否
- 签名: `XGE_API int xgeRenderTargetResize(xge_render_target pTarget, int iWidth, int iHeight);`
- 实现: src/xge_render_target.c:109（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeRenderTargetCaptureCurrent
- 位置: xge.h:1941  已注释: 否
- 签名: `XGE_API int xgeRenderTargetCaptureCurrent(xge_render_target pTarget, int iSrcX, int iSrcY);`
- 实现: src/xge_render_target.c:129（体 34 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_OK

## xgeRenderTargetReadPixels
- 位置: xge.h:1942  已注释: 否
- 签名: `XGE_API int xgeRenderTargetReadPixels(xge_render_target pTarget, void* pPixels, int iStride);`
- 实现: src/xge_render_target.c:164（体 60 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_UNSUPPORTED, XGE_ERROR_GPU_FAILED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_camera_viewport/main.c:15; examples/audit_ninepatch_sprite/main.c:14; examples/audit_shape_basic/main.c:73

## xgeRenderTargetTexture
- 位置: xge.h:1943  已注释: 否
- 签名: `XGE_API xge_texture xgeRenderTargetTexture(xge_render_target pTarget);`
- 实现: src/xge_render_target.c:225（体 7 行）
- 返回码: NULL
- 用法: examples/xge_particles/common.c:296; examples/xge_shape/main.c:220; examples/xge_shape_ex/main.c:995

## xgeRenderTargetFree
- 位置: xge.h:1944  已注释: 否
- 签名: `XGE_API void xgeRenderTargetFree(xge_render_target pTarget);`
- 实现: src/xge_render_target.c:233（体 23 行）
- 用法: examples/audit_camera_viewport/main.c:139; examples/audit_ninepatch_sprite/main.c:150; examples/audit_shape_basic/main.c:376

