# 草稿包：xge.h / Image（9 条 API）

> 生成 2026-09-10 02:38 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeImageLoad
- 位置: xge.h:1905  已注释: 否
- 签名: `XGE_API int xgeImageLoad(xge_image pImage, const char* sPath);`
- 实现: src/xge_texture.c:454（体 4 行）
- 用法: examples/tutorial_capture/ch91_main1.c:28

## xgeImageLoadEx
- 位置: xge.h:1906  已注释: 否
- 签名: `XGE_API int xgeImageLoadEx(xge_image pImage, const char* sPath, uint32_t iFlags);`
- 实现: src/xge_texture.c:459（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: test/test_main.c:116

## xgeImageLoadMemory
- 位置: xge.h:1907  已注释: 否
- 签名: `XGE_API int xgeImageLoadMemory(xge_image pImage, const void* pData, int iSize);`
- 实现: src/xge_texture.c:477（体 4 行）

## xgeImageLoadMemoryEx
- 位置: xge.h:1908  已注释: 否
- 签名: `XGE_API int xgeImageLoadMemoryEx(xge_image pImage, const void* pData, int iSize, uint32_t iFlags);`
- 实现: src/xge_texture.c:482（体 27 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeImageGetPixels
- 位置: xge.h:1909  已注释: 否
- 签名: `XGE_API void* xgeImageGetPixels(xge_image pImage);`
- 实现: src/xge_texture.c:510（体 7 行）
- 返回码: NULL

## xgeImagePremultiply
- 位置: xge.h:1910  已注释: 否
- 签名: `XGE_API void xgeImagePremultiply(xge_image pImage);`
- 实现: src/xge_texture.c:518（体 21 行）

## xgeImageSavePNG
- 位置: xge.h:1911  已注释: 否
- 签名: `XGE_API int xgeImageSavePNG(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride);`
- 实现: src/xge_texture.c:625（体 4 行）
- 用法: examples/audit_camera_viewport/main.c:15; examples/audit_ninepatch_sprite/main.c:14; examples/audit_shape_basic/main.c:75

## xgeImageSavePNGEx
- 位置: xge.h:1914  已注释: 是
- 签名: `XGE_API int xgeImageSavePNGEx(const char* sPath, int iWidth, int iHeight, const void* pPixels, int iStride, uint32_t iFlags);`
- 既有注释: /* pPixels are straight RGBA by default. Pass XGE_IMAGE_PREMULTIPLIED when * saving premultiplied RGBA, such as an external render-target capture. */
- 实现: src/xge_texture.c:550（体 74 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_FILE_NOT_FOUND
- 用法: test/test_main.c:114

## xgeImageFree
- 位置: xge.h:1915  已注释: 否
- 签名: `XGE_API void xgeImageFree(xge_image pImage);`
- 实现: src/xge_texture.c:630（体 10 行）
- 用法: examples/tutorial_capture/ch91_main1.c:49; test/test_main.c:56; test/test_main.c:60

