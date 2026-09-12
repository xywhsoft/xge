# 草稿包：xge.h / Mesh（4 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeMeshCreate
- 位置: xge.h:2116  已注释: 否
- 签名: `XGE_API int xgeMeshCreate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount, uint32_t iFlags);`
- 实现: src/xge_mesh.c:161（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch104_main1.c:39

## xgeMeshUpdate
- 位置: xge.h:2117  已注释: 否
- 签名: `XGE_API int xgeMeshUpdate(xge_mesh pMesh, const xge_vertex_t* pVertices, int iVertexCount, const uint16_t* pIndices, int iIndexCount);`
- 实现: src/xge_mesh.c:178（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeMeshFree
- 位置: xge.h:2118  已注释: 否
- 签名: `XGE_API void xgeMeshFree(xge_mesh pMesh);`
- 实现: src/xge_mesh.c:201（体 18 行）
- 用法: examples/tutorial_capture/ch104_main1.c:42

## xgeMeshDraw
- 位置: xge.h:2119  已注释: 否
- 签名: `XGE_API void xgeMeshDraw(xge_mesh pMesh, xge_texture pTexture, uint32_t iFlags);`
- 实现: src/xge_mesh.c:220（体 36 行）
- 用法: examples/tutorial_capture/ch104_main1.c:41

