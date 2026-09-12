# 草稿包：xge.h / Async（10 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeAsyncRequestInit
- 位置: xge.h:1740  已注释: 否
- 签名: `XGE_API void xgeAsyncRequestInit(xge_async_request pRequest);`
- 实现: src/xge_async.c:135（体 8 行）
- 用法: examples/tutorial_capture/ch114_main1.c:5; test/test_main.c:40; test/test_main.c:75

## xgeAsyncRequestFree
- 位置: xge.h:1741  已注释: 否
- 签名: `XGE_API void xgeAsyncRequestFree(xge_async_request pRequest);`
- 实现: src/xge_async.c:144（体 17 行）
- 用法: examples/tutorial_capture/ch114_main1.c:5; test/test_main.c:27; test/test_main.c:57

## xgeAsyncRequestCancel
- 位置: xge.h:1742  已注释: 否
- 签名: `XGE_API int xgeAsyncRequestCancel(xge_async_request pRequest);`
- 实现: src/xge_async.c:162（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED, XGE_OK

## xgeAsyncThreadingSet
- 位置: xge.h:1747  已注释: 是
- 签名: `XGE_API int xgeAsyncThreadingSet(int bEnabled);`
- 既有注释: /* Enables worker decoding for file-backed IMAGE and TEXTURE requests only. * Requests handled by a registered resource provider, and FONT/SOUND requests, * are completed synchronously on the caller thread. Completion callbacks may * release their request; no request field may be read after the callback. */
- 实现: src/xge_async.c:176（体 5 行）
- 返回码: XGE_OK
- 用法: examples/tutorial_capture/ch114_main1.c:5; test/test_main.c:41; test/test_main.c:62

## xgeAsyncThreadingGet
- 位置: xge.h:1748  已注释: 否
- 签名: `XGE_API int xgeAsyncThreadingGet(void);`
- 实现: src/xge_async.c:182（体 4 行）

## xgeAsyncPoll
- 位置: xge.h:1750  已注释: 是
- 签名: `XGE_API int xgeAsyncPoll(xge_async_request pRequest);`
- 既有注释: /* Returns an XGE_ASYNC_* status. A completion callback may release pRequest. */
- 实现: src/xge_async.c:187（体 34 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ASYNC_LOADING
- 用法: examples/tutorial_capture/ch114_main1.c:5; test/test_main.c:46; test/test_main.c:93

## xgeAsyncImageLoad
- 位置: xge.h:1751  已注释: 否
- 签名: `XGE_API int xgeAsyncImageLoad(xge_async_request pRequest, xge_image pImage, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);`
- 实现: src/xge_async.c:222（体 18 行）
- 返回码: XGE_OK
- 用法: test/test_main.c:42; test/test_main.c:77; test/test_main.c:89

## xgeAsyncTextureLoad
- 位置: xge.h:1752  已注释: 否
- 签名: `XGE_API int xgeAsyncTextureLoad(xge_async_request pRequest, xge_texture pTexture, const char* sPath, uint32_t iFlags, xge_async_proc onComplete, void* pUser);`
- 实现: src/xge_async.c:241（体 18 行）
- 返回码: XGE_OK

## xgeAsyncFontLoad
- 位置: xge.h:1753  已注释: 否
- 签名: `XGE_API int xgeAsyncFontLoad(xge_async_request pRequest, xge_font pFont, const char* sPath, float fSize, xge_async_proc onComplete, void* pUser);`
- 实现: src/xge_async.c:260（体 18 行）
- 返回码: XGE_OK

## xgeAsyncSoundLoad
- 位置: xge.h:1754  已注释: 否
- 签名: `XGE_API int xgeAsyncSoundLoad(xge_async_request pRequest, xge_sound pSound, const char* sPath, xge_async_proc onComplete, void* pUser);`
- 实现: src/xge_async.c:279（体 17 行）
- 返回码: XGE_OK
- 用法: examples/tutorial_capture/ch114_main1.c:5

