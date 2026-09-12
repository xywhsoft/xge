# 草稿包：xge.h / Shader（11 条 API）

> 生成 2026-09-10 02:41 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeShaderCreate
- 位置: xge.h:2096  已注释: 否
- 签名: `XGE_API int xgeShaderCreate(xge_shader pShader, const char* sVertexSource, const char* sFragmentSource);`
- 实现: src/xge_material.c:1（体 48 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_GPU_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch102_main1.c:30; test/test_particle_render.c:102

## xgeShaderAddRef
- 位置: xge.h:2097  已注释: 否
- 签名: `XGE_API int xgeShaderAddRef(xge_shader pShader);`
- 实现: src/xge_material.c:50（体 10 行）

## xgeShaderFree
- 位置: xge.h:2098  已注释: 否
- 签名: `XGE_API void xgeShaderFree(xge_shader pShader);`
- 实现: src/xge_material.c:61（体 14 行）
- 用法: examples/tutorial_capture/ch102_main1.c:57; examples/xge_particles/common.c:438; test/test_particle_render.c:186

## xgeShaderUniform1i
- 位置: xge.h:2099  已注释: 否
- 签名: `XGE_API int xgeShaderUniform1i(xge_shader pShader, const char* sName, int iX);`
- 实现: src/xge_material.c:92（体 13 行）
- 返回码: XGE_OK

## xgeShaderUniform1f
- 位置: xge.h:2100  已注释: 否
- 签名: `XGE_API int xgeShaderUniform1f(xge_shader pShader, const char* sName, float fX);`
- 实现: src/xge_material.c:106（体 13 行）
- 返回码: XGE_OK
- 用法: examples/xge_particles/common.c:254

## xgeShaderUniform2f
- 位置: xge.h:2101  已注释: 否
- 签名: `XGE_API int xgeShaderUniform2f(xge_shader pShader, const char* sName, float fX, float fY);`
- 实现: src/xge_material.c:120（体 13 行）
- 返回码: XGE_OK

## xgeShaderUniform3f
- 位置: xge.h:2102  已注释: 否
- 签名: `XGE_API int xgeShaderUniform3f(xge_shader pShader, const char* sName, float fX, float fY, float fZ);`
- 实现: src/xge_material.c:134（体 13 行）
- 返回码: XGE_OK

## xgeShaderUniform4f
- 位置: xge.h:2103  已注释: 否
- 签名: `XGE_API int xgeShaderUniform4f(xge_shader pShader, const char* sName, float fX, float fY, float fZ, float fW);`
- 实现: src/xge_material.c:148（体 13 行）
- 返回码: XGE_OK
- 用法: examples/tutorial_capture/ch102_main1.c:34; examples/tutorial_capture/ch102_main1.c:47

## xgeShaderVariantSetInit
- 位置: xge.h:2104  已注释: 否
- 签名: `XGE_API int xgeShaderVariantSetInit(xge_shader_variant_set pSet, const char* sVertexSource, const char* sFragmentSource);`
- 实现: src/xge_material.c:266（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch103_main1.c:39

## xgeShaderVariantSetFree
- 位置: xge.h:2105  已注释: 否
- 签名: `XGE_API void xgeShaderVariantSetFree(xge_shader_variant_set pSet);`
- 实现: src/xge_material.c:281（体 23 行）
- 用法: examples/tutorial_capture/ch103_main1.c:57

## xgeShaderVariantGet
- 位置: xge.h:2106  已注释: 否
- 签名: `XGE_API int xgeShaderVariantGet(xge_shader_variant_set pSet, uint32_t iKey, const xge_shader_define_t* pDefines, int iDefineCount, xge_shader* ppShader);`
- 实现: src/xge_material.c:305（体 62 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch103_main1.c:43; examples/tutorial_capture/ch103_main1.c:52

