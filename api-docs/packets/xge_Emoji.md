# 草稿包：xge.h / Emoji（9 条 API）

> 生成 2026-09-10 02:40 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeEmojiPackCreate
- 位置: xge.h:1933  已注释: 否
- 签名: `XGE_API int xgeEmojiPackCreate(xge_emoji_pack* ppPack);`
- 实现: src/xge_emoji.c:284（体 26 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test/test_emoji.c:325

## xgeEmojiPackLoadBuiltin
- 位置: xge.h:1934  已注释: 否
- 签名: `XGE_API int xgeEmojiPackLoadBuiltin(xge_emoji_pack* ppPack);`
- 实现: src/xge_emoji.c:628（体 25 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_emoji.c:170

## xgeEmojiPackAddRef
- 位置: xge.h:1935  已注释: 否
- 签名: `XGE_API int xgeEmojiPackAddRef(xge_emoji_pack pPack);`
- 实现: src/xge_emoji.c:311（体 5 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

## xgeEmojiPackFree
- 位置: xge.h:1936  已注释: 否
- 签名: `XGE_API void xgeEmojiPackFree(xge_emoji_pack pPack);`
- 实现: src/xge_emoji.c:317（体 20 行）
- 用法: test/test_emoji.c:172; test/test_emoji.c:179; test/test_emoji.c:186

## xgeEmojiPackAddSvgMemory
- 位置: xge.h:1937  已注释: 否
- 签名: `XGE_API int xgeEmojiPackAddSvgMemory(xge_emoji_pack pPack, const char* sSequence, const void* pSvgData, int iSvgSize, const xge_emoji_metrics_t* pMetrics, uint32_t* pEmojiId);`
- 实现: src/xge_emoji.c:338（体 78 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: test/test_emoji.c:326

## xgeEmojiPackMatch
- 位置: xge.h:1938  已注释: 否
- 签名: `XGE_API int xgeEmojiPackMatch(xge_emoji_pack pPack, const char* sText, int iTextSize, xge_emoji_match_t* pMatch);`
- 实现: src/xge_emoji.c:457（体 47 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_FOUND, XGE_OK
- 用法: test/test_emoji.c:110; test/test_emoji.c:177; test/test_emoji.c:184

## xgeEmojiPackSetDefault
- 位置: xge.h:1939  已注释: 否
- 签名: `XGE_API int xgeEmojiPackSetDefault(xge_emoji_pack pPack);`
- 实现: src/xge_emoji.c:654（体 14 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeEmojiPackGetDefault
- 位置: xge.h:1940  已注释: 否
- 签名: `XGE_API int xgeEmojiPackGetDefault(xge_emoji_pack* ppPack);`
- 实现: src/xge_emoji.c:693（体 11 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeEmojiPackClearDefault
- 位置: xge.h:1941  已注释: 否
- 签名: `XGE_API void xgeEmojiPackClearDefault(void);`
- 实现: src/xge_emoji.c:705（体 10 行）

