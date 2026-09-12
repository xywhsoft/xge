# 草稿包：xge.h / Draw（4 条 API）

> 生成 2026-09-10 02:33 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeDraw
- 位置: xge.h:1921  已注释: 否
- 签名: `XGE_API void xgeDraw(xge_texture pTexture, float fX, float fY);`
- 实现: src/xge_render.c:1（体 16 行）
- 用法: examples/tutorial_capture/ch108_main1.c:20; examples/tutorial_capture/ch87_main1.c:32; examples/tutorial_capture/ch91_main1.c:34

## xgeDrawEx
- 位置: xge.h:1922  已注释: 否
- 签名: `XGE_API void xgeDrawEx(const xge_draw_t* pDraw);`
- 实现: src/xge_render.c:36（体 4 行）
- 用法: examples/tutorial_capture/ch108_main1.c:24; examples/tutorial_capture/ch108_main1.c:25; examples/tutorial_capture/ch108_main1.c:26

## xgeDrawQuad3D
- 位置: xge.h:1923  已注释: 否
- 签名: `XGE_API void xgeDrawQuad3D(xge_texture pTexture, const xge_vertex_t* pVertices, uint32_t iFlags);`
- 实现: src/xge_mesh.c:1（体 62 行）

## xgeDrawPx
- 位置: xge.h:1924  已注释: 否
- 签名: `XGE_API void xgeDrawPx(xge_texture pTexture, int iX, int iY);`
- 实现: src/xge_render.c:18（体 17 行）
- 用法: examples/tutorial_capture/ch108_main1.c:21

