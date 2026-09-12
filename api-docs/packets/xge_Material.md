# 草稿包：xge.h / Material（9 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeMaterialInit
- 位置: xge.h:2107  已注释: 否
- 签名: `XGE_API void xgeMaterialInit(xge_material pMaterial);`
- 实现: src/xge_material.c:368（体 9 行）
- 用法: examples/tutorial_capture/ch104_main1.c:28; test/test_particle_render.c:47

## xgeMaterialFree
- 位置: xge.h:2108  已注释: 否
- 签名: `XGE_API void xgeMaterialFree(xge_material pMaterial);`
- 实现: src/xge_material.c:378（体 16 行）
- 用法: examples/tutorial_capture/ch104_main1.c:57; examples/xge_particles/common.c:437; test/test_particle_render.c:185

## xgeMaterialSetShader
- 位置: xge.h:2109  已注释: 否
- 签名: `XGE_API void xgeMaterialSetShader(xge_material pMaterial, xge_shader pShader);`
- 实现: src/xge_material.c:395（体 6 行）
- 用法: test/test_particle_render.c:103

## xgeMaterialSetTexture
- 位置: xge.h:2110  已注释: 否
- 签名: `XGE_API void xgeMaterialSetTexture(xge_material pMaterial, xge_texture pTexture);`
- 实现: src/xge_material.c:402（体 16 行）
- 用法: examples/tutorial_capture/ch104_main1.c:29

## xgeMaterialSetTexture2
- 位置: xge.h:2111  已注释: 否
- 签名: `XGE_API void xgeMaterialSetTexture2(xge_material pMaterial, xge_texture pTexture);`
- 实现: src/xge_material.c:419（体 16 行）

## xgeMaterialSetTexture3
- 位置: xge.h:2112  已注释: 否
- 签名: `XGE_API void xgeMaterialSetTexture3(xge_material pMaterial, xge_texture pTexture);`
- 实现: src/xge_material.c:436（体 16 行）

## xgeMaterialSetColor
- 位置: xge.h:2113  已注释: 否
- 签名: `XGE_API void xgeMaterialSetColor(xge_material pMaterial, uint32_t iColor);`
- 实现: src/xge_material.c:453（体 6 行）
- 用法: examples/tutorial_capture/ch104_main1.c:30

## xgeMaterialSetBlend
- 位置: xge.h:2114  已注释: 否
- 签名: `XGE_API void xgeMaterialSetBlend(xge_material pMaterial, int iBlend);`
- 实现: src/xge_material.c:460（体 6 行）

## xgeMaterialDraw
- 位置: xge.h:2115  已注释: 否
- 签名: `XGE_API void xgeMaterialDraw(xge_material pMaterial, const xge_draw_t* pDraw);`
- 实现: src/xge_material.c:645（体 10 行）
- 用法: examples/tutorial_capture/ch104_main1.c:54; test/test_particle_render.c:137

