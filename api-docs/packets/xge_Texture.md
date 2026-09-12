# 草稿包：xge.h / Texture（19 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeTextureCreateRGBA
- 位置: xge.h:1918  已注释: 是
- 签名: `XGE_API int xgeTextureCreateRGBA(xge_texture pTexture, int iWidth, int iHeight, const void* pPixels);`
- 既有注释: /* pTexture must be zero-initialized before first use and freed before reuse. * Creating over a live texture returns XGE_ERROR_INVALID_STATE. */
- 实现: src/xge_texture.c:381（体 36 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/audit_ninepatch_sprite/main.c:33; examples/tutorial_capture/ch104_main1.c:25; examples/tutorial_capture/ch105_main1.c:25

## xgeTextureCreateYUV420P
- 位置: xge.h:1919  已注释: 否
- 签名: `XGE_API int xgeTextureCreateYUV420P(xge_texture pTexture, int iWidth, int iHeight);`
- 实现: src/xge_texture.c:418（体 35 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_INVALID_STATE, XGE_OK
- 用法: examples/tutorial_capture/ch94_main1.c:27; examples/yuv420p_texture_lab/main.c:36

## xgeTextureCreateFromImage
- 位置: xge.h:1920  已注释: 否
- 签名: `XGE_API int xgeTextureCreateFromImage(xge_texture pTexture, const xge_image_t* pImage);`
- 实现: src/xge_texture.c:641（体 7 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch91_main1.c:33

## xgeTextureLoad
- 位置: xge.h:1921  已注释: 否
- 签名: `XGE_API int xgeTextureLoad(xge_texture pTexture, const char* sPath);`
- 实现: src/xge_texture.c:649（体 4 行）
- 用法: examples/tutorial_capture/ch93_main1.c:34

## xgeTextureLoadEx
- 位置: xge.h:1922  已注释: 否
- 签名: `XGE_API int xgeTextureLoadEx(xge_texture pTexture, const char* sPath, uint32_t iFlags);`
- 实现: src/xge_texture.c:654（体 16 行）

## xgeTextureLoadMemory
- 位置: xge.h:1923  已注释: 否
- 签名: `XGE_API int xgeTextureLoadMemory(xge_texture pTexture, const void* pData, int iSize);`
- 实现: src/xge_texture.c:671（体 4 行）

## xgeTextureLoadMemoryEx
- 位置: xge.h:1924  已注释: 否
- 签名: `XGE_API int xgeTextureLoadMemoryEx(xge_texture pTexture, const void* pData, int iSize, uint32_t iFlags);`
- 实现: src/xge_texture.c:676（体 16 行）

## xgeTextureAddRef
- 位置: xge.h:1925  已注释: 否
- 签名: `XGE_API int xgeTextureAddRef(xge_texture pTexture);`
- 实现: src/xge_texture.c:693（体 10 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

## xgeTextureUpdateRGBA
- 位置: xge.h:1926  已注释: 否
- 签名: `XGE_API int xgeTextureUpdateRGBA(xge_texture pTexture, int iX, int iY, int iWidth, int iHeight, const void* pPixels, int iStride);`
- 实现: src/xge_texture.c:704（体 55 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_GPU_FAILED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch95_main1.c:46

## xgeTextureUpdateYUV420P
- 位置: xge.h:1927  已注释: 否
- 签名: `XGE_API int xgeTextureUpdateYUV420P(xge_texture pTexture, const void* pY, int iStrideY, const void* pU, int iStrideU, const void* pV, int iStrideV);`
- 实现: src/xge_texture.c:781（体 40 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_GPU_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch94_main1.c:28; examples/yuv420p_texture_lab/main.c:43

## xgeTextureSetSampler
- 位置: xge.h:1929  已注释: 否
- 签名: `XGE_API int xgeTextureSetSampler(xge_texture pTexture, const xge_sampler_t* pSampler);`
- 实现: src/xge_texture.c:836（体 19 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch95_main1.c:27; test/test_particle_render.c:154

## xgeTextureGetSampler
- 位置: xge.h:1930  已注释: 否
- 签名: `XGE_API xge_sampler_t xgeTextureGetSampler(xge_texture pTexture);`
- 实现: src/xge_texture.c:822（体 13 行）

## xgeTextureFallbackSetRGBA
- 位置: xge.h:1931  已注释: 否
- 签名: `XGE_API int xgeTextureFallbackSetRGBA(int iWidth, int iHeight, const void* pPixels);`
- 实现: src/xge_texture.c:856（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch96_main1.c:16

## xgeTextureFallbackGet
- 位置: xge.h:1932  已注释: 否
- 签名: `XGE_API int xgeTextureFallbackGet(xge_texture pTexture);`
- 实现: src/xge_texture.c:871（体 22 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch96_main1.c:20

## xgeTextureFallbackClear
- 位置: xge.h:1933  已注释: 否
- 签名: `XGE_API void xgeTextureFallbackClear(void);`
- 实现: src/xge_texture.c:894（体 7 行）
- 用法: examples/tutorial_capture/ch96_main1.c:41

## xgeTextureUploadQueue
- 位置: xge.h:1934  已注释: 否
- 签名: `XGE_API int xgeTextureUploadQueue(xge_texture pTexture);`
- 实现: src/xge_texture.c:919（体 40 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_RESOURCE_FAILED, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch96_main1.c:34

## xgeTextureUploadFlush
- 位置: xge.h:1935  已注释: 否
- 签名: `XGE_API int xgeTextureUploadFlush(void);`
- 实现: src/xge_texture.c:960（体 26 行）
- 用法: examples/tutorial_capture/ch96_main1.c:35

## xgeTextureReadPixels
- 位置: xge.h:1936  已注释: 否
- 签名: `XGE_API int xgeTextureReadPixels(xge_texture pTexture, void* pPixels, int iStride);`
- 实现: src/xge_texture.c:1037（体 35 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: examples/xge_svg/main.c:815; examples/xge_svg/main.c:821

## xgeTextureFree
- 位置: xge.h:1937  已注释: 否
- 签名: `XGE_API void xgeTextureFree(xge_texture pTexture);`
- 实现: src/xge_texture.c:1073（体 36 行）
- 用法: examples/audit_ninepatch_sprite/main.c:130; examples/tutorial_capture/ch104_main1.c:58; examples/tutorial_capture/ch105_main1.c:29

