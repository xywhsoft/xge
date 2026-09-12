# 草稿包：xge.h / Nine（5 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeNinePatchInitSimple
- 位置: xge.h:1925  已注释: 否
- 签名: `XGE_API void xgeNinePatchInitSimple(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc);`
- 实现: src/xge_render.c:81（体 16 行）
- 用法: examples/audit_ninepatch_sprite/main.c:81

## xgeNinePatchInit
- 位置: xge.h:1926  已注释: 否
- 签名: `XGE_API void xgeNinePatchInit(xge_nine_patch pPatch, xge_texture pTexture, xge_rect_t tSrc, float fX1, float fY1, float fX2, float fY2);`
- 实现: src/xge_render.c:98（体 16 行）
- 用法: examples/audit_ninepatch_sprite/main.c:56; examples/tutorial_capture/ch107_main1.c:30

## xgeNinePatchSetMode
- 位置: xge.h:1927  已注释: 否
- 签名: `XGE_API void xgeNinePatchSetMode(xge_nine_patch pPatch, int iMode);`
- 实现: src/xge_render.c:115（体 8 行）
- 用法: examples/audit_ninepatch_sprite/main.c:57; examples/audit_ninepatch_sprite/main.c:72; examples/audit_ninepatch_sprite/main.c:82

## xgeNinePatchSetColor
- 位置: xge.h:1928  已注释: 否
- 签名: `XGE_API void xgeNinePatchSetColor(xge_nine_patch pPatch, uint32_t iColor);`
- 实现: src/xge_render.c:124（体 7 行）
- 用法: examples/audit_ninepatch_sprite/main.c:58; examples/audit_ninepatch_sprite/main.c:83; examples/audit_ninepatch_sprite/main.c:91

## xgeNinePatchDraw
- 位置: xge.h:1929  已注释: 否
- 签名: `XGE_API void xgeNinePatchDraw(const xge_nine_patch_t* pPatch, xge_rect_t tDst, uint32_t iFlags);`
- 实现: src/xge_render.c:324（体 4 行）
- 用法: examples/audit_ninepatch_sprite/main.c:60; examples/audit_ninepatch_sprite/main.c:62; examples/audit_ninepatch_sprite/main.c:64

