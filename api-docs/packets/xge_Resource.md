# 草稿包：xge.h / Resource（6 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeResourceProviderAdd
- 位置: xge.h:1709  已注释: 否
- 签名: `XGE_API int xgeResourceProviderAdd(const xge_resource_provider_t* pProvider);`
- 实现: src/xge_resource.c:142（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: test/test_main.c:7916

## xgeResourceProviderClear
- 位置: xge.h:1710  已注释: 否
- 签名: `XGE_API void xgeResourceProviderClear(void);`
- 实现: src/xge_resource.c:155（体 7 行）
- 用法: test/test_main.c:7915; test/test_main.c:7930

## xgeResourceXPackProviderAdd
- 位置: xge.h:1711  已注释: 否
- 签名: `XGE_API int xgeResourceXPackProviderAdd(const xge_xpack_provider_t* pProvider);`
- 实现: src/xge_resource.c:163（体 25 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeResourceLoad
- 位置: xge.h:1712  已注释: 否
- 签名: `XGE_API int xgeResourceLoad(const char* sURI, xge_resource_t* pResource);`
- 实现: src/xge_resource.c:189（体 69 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK, XGE_ERROR_UNSUPPORTED, XGE_ERROR_FILE_NOT_FOUND, XGE_ERROR_RESOURCE_FAILED
- 用法: examples/xge_svg/main.c:802; examples/xui_proxy_font/main.c:271; examples/xui_proxy_surface/main.c:257

## xgeResourceLoadMemory
- 位置: xge.h:1713  已注释: 否
- 签名: `XGE_API int xgeResourceLoadMemory(const void* pData, int iSize, xge_resource_t* pResource);`
- 实现: src/xge_resource.c:259（体 21 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY

## xgeResourceFree
- 位置: xge.h:1714  已注释: 否
- 签名: `XGE_API void xgeResourceFree(xge_resource_t* pResource);`
- 实现: src/xge_resource.c:281（体 12 行）
- 用法: examples/xge_svg/main.c:832; examples/xui_proxy_font/main.c:277; examples/xui_proxy_surface/main.c:263

