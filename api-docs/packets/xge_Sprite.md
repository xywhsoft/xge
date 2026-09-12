# 草稿包：xge.h / Sprite（7 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeSpriteBatchInit
- 位置: xge.h:2091  已注释: 否
- 签名: `XGE_API int xgeSpriteBatchInit(xge_sprite_batch pBatch, xge_texture pTexture, int iCapacity, uint32_t iFlags);`
- 实现: src/xge_sprite.c:1（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_ninepatch_sprite/main.c:105; examples/tutorial_capture/ch105_main1.c:28

## xgeSpriteBatchFree
- 位置: xge.h:2092  已注释: 否
- 签名: `XGE_API void xgeSpriteBatchFree(xge_sprite_batch pBatch);`
- 实现: src/xge_sprite.c:24（体 13 行）
- 用法: examples/audit_ninepatch_sprite/main.c:126; examples/tutorial_capture/ch105_main1.c:48

## xgeSpriteBatchClear
- 位置: xge.h:2093  已注释: 否
- 签名: `XGE_API void xgeSpriteBatchClear(xge_sprite_batch pBatch);`
- 实现: src/xge_sprite.c:38（体 6 行）

## xgeSpriteBatchAdd
- 位置: xge.h:2094  已注释: 否
- 签名: `XGE_API int xgeSpriteBatchAdd(xge_sprite_batch pBatch, const xge_draw_t* pDraw);`
- 实现: src/xge_sprite.c:70（体 101 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/audit_ninepatch_sprite/main.c:112; examples/audit_ninepatch_sprite/main.c:123; examples/tutorial_capture/ch105_main1.c:43

## xgeSpriteBatchFlush
- 位置: xge.h:2095  已注释: 否
- 签名: `XGE_API int xgeSpriteBatchFlush(xge_sprite_batch pBatch);`
- 实现: src/xge_sprite.c:172（体 41 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_GPU_FAILED
- 用法: examples/audit_ninepatch_sprite/main.c:125; examples/tutorial_capture/ch105_main1.c:47

## xgeSpriteBatchSetTexture
- 位置: xge.h:2097  已注释: 是
- 签名: `XGE_API int xgeSpriteBatchSetTexture(xge_sprite_batch pBatch, xge_texture pTexture);`
- 既有注释: /* Retarget only an empty batch; retained vertex storage is reused. */
- 实现: src/xge_sprite.c:45（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK

## xgeSpriteBatchFlushMaterial
- 位置: xge.h:2099  已注释: 是
- 签名: `XGE_API int xgeSpriteBatchFlushMaterial(xge_sprite_batch pBatch, const xge_material_t* pMaterial);`
- 既有注释: /* NULL material selects the default shader. Custom material tint is uColor. */
- 实现: src/xge_material.c:656（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

