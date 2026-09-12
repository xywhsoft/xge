# 草稿包：xge.h / Pass（3 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgePassInit
- 位置: xge.h:1949  已注释: 否
- 签名: `XGE_API void xgePassInit(xge_pass pPass, xge_render_target pTarget, uint32_t iClearFlags, uint32_t iClearColor);`
- 实现: src/xge_render_target.c:257（体 10 行）
- 用法: examples/audit_camera_viewport/main.c:46; examples/audit_ninepatch_sprite/main.c:47; examples/audit_shape_basic/main.c:278

## xgePassBegin
- 位置: xge.h:1950  已注释: 否
- 签名: `XGE_API int xgePassBegin(xge_pass pPass);`
- 实现: src/xge_render_target.c:268（体 47 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_ALREADY_INITIALIZED, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_BACKEND_FAILED, XGE_ERROR_GPU_FAILED, XGE_OK
- 用法: examples/audit_camera_viewport/main.c:47; examples/audit_ninepatch_sprite/main.c:48; examples/audit_shape_basic/main.c:279

## xgePassEnd
- 位置: xge.h:1951  已注释: 否
- 签名: `XGE_API int xgePassEnd(xge_pass pPass);`
- 实现: src/xge_render_target.c:316（体 31 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED
- 用法: examples/audit_camera_viewport/main.c:120; examples/audit_ninepatch_sprite/main.c:52; examples/audit_ninepatch_sprite/main.c:131

