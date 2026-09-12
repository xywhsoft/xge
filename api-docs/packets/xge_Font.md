# 草稿包：xge.h / Font（37 条 API）

> 生成 2026-09-10 02:37 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeFontFaceLoad
- 位置: xge.h:1797  已注释: 否
- 签名: `XGE_API int xgeFontFaceLoad(xge_font_face* ppFace, const char* sPath, const xge_font_face_desc_t* pDesc);`
- 实现: src/xge_font.c:827（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch84_main1.c:22; examples/tutorial_capture/ch84_main1.c:23

## xgeFontFaceLoadMemory
- 位置: xge.h:1798  已注释: 否
- 签名: `XGE_API int xgeFontFaceLoadMemory(xge_font_face* ppFace, const void* pData, int iSize, const xge_font_face_desc_t* pDesc);`
- 实现: src/xge_font.c:785（体 41 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: test/test_text_foundation.c:106

## xgeFontFaceAddRef
- 位置: xge.h:1799  已注释: 否
- 签名: `XGE_API int xgeFontFaceAddRef(xge_font_face pFace);`
- 实现: src/xge_font.c:844（体 6 行）

## xgeFontFaceFree
- 位置: xge.h:1800  已注释: 否
- 签名: `XGE_API void xgeFontFaceFree(xge_font_face pFace);`
- 实现: src/xge_font.c:851（体 25 行）
- 用法: test/test_text_foundation.c:115; test/test_text_foundation.c:123; test/test_text_foundation.c:125

## xgeFontFaceGetInfo
- 位置: xge.h:1801  已注释: 否
- 签名: `XGE_API int xgeFontFaceGetInfo(xge_font_face pFace, xge_font_face_info_t* pInfo);`
- 实现: src/xge_font.c:877（体 13 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_text_foundation.c:113

## xgeFontFamilyCreate
- 位置: xge.h:1802  已注释: 否
- 签名: `XGE_API int xgeFontFamilyCreate(xge_font_family* ppFamily);`
- 实现: src/xge_font.c:891（体 12 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch84_main1.c:24; test/test_text_foundation.c:120

## xgeFontFamilyFree
- 位置: xge.h:1803  已注释: 否
- 签名: `XGE_API void xgeFontFamilyFree(xge_font_family pFamily);`
- 实现: src/xge_font.c:904（体 14 行）
- 用法: test/test_text_foundation.c:124; test/test_text_foundation.c:135; test/test_text_foundation.c:143

## xgeFontFamilyAddFace
- 位置: xge.h:1804  已注释: 否
- 签名: `XGE_API int xgeFontFamilyAddFace(xge_font_family pFamily, xge_font_face pFace);`
- 实现: src/xge_font.c:919（体 21 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/tutorial_capture/ch84_main1.c:25; examples/tutorial_capture/ch84_main1.c:26; test/test_text_foundation.c:120

## xgeFontFamilyResolve
- 位置: xge.h:1805  已注释: 否
- 签名: `XGE_API int xgeFontFamilyResolve(xge_font_family pFamily, int iWeight, int iSlant, xge_font_face* ppFace);`
- 实现: src/xge_font.c:969（体 4 行）
- 用法: examples/tutorial_capture/ch84_main1.c:27; examples/tutorial_capture/ch84_main1.c:30; test/test_text_foundation.c:121

## xgeFontFamilyResolveEx
- 位置: xge.h:1806  已注释: 否
- 签名: `XGE_API int xgeFontFamilyResolveEx(xge_font_family pFamily, int iWeight, int iStretch, int iSlant, xge_font_face* ppFace);`
- 实现: src/xge_font.c:941（体 27 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeFontCreate
- 位置: xge.h:1807  已注释: 否
- 签名: `XGE_API int xgeFontCreate(xge_font pFont, xge_font_face pFace, const xge_font_instance_desc_t* pDesc);`
- 实现: src/xge_font.c:974（体 48 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: examples/tutorial_capture/ch84_main1.c:28; examples/tutorial_capture/ch84_main1.c:31; test/test_text_foundation.c:133

## xgeFontCreateSized
- 位置: xge.h:1808  已注释: 否
- 签名: `XGE_API int xgeFontCreateSized(xge_font pFont, xge_font pSource, float fPixelSize);`
- 实现: src/xge_font.c:1023（体 15 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_UNSUPPORTED

## xgeFontLoad
- 位置: xge.h:1809  已注释: 否
- 签名: `XGE_API int xgeFontLoad(xge_font pFont, const char* sPath, float fSize);`
- 实现: src/xge_font.c:1039（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/audit_text_render/main.c:39; examples/audit_text_render/main.c:56; examples/audit_text_render/main.c:59

## xgeFontLoadMemory
- 位置: xge.h:1810  已注释: 否
- 签名: `XGE_API int xgeFontLoadMemory(xge_font pFont, const void* pData, int iSize, float fSize);`
- 实现: src/xge_font.c:1058（体 17 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT

## xgeFontLoadXRF
- 位置: xge.h:1811  已注释: 否
- 签名: `XGE_API int xgeFontLoadXRF(xge_font pFont, const char* sPath);`
- 实现: src/xge_font.c:1076（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/tutorial_capture/ch82_main1.c:18

## xgeFontLoadXRFMemory
- 位置: xge.h:1812  已注释: 否
- 签名: `XGE_API int xgeFontLoadXRFMemory(xge_font pFont, const void* pData, int iSize);`
- 实现: src/xge_font.c:1093（体 141 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_ERROR_UNSUPPORTED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test/test_text_foundation.c:227

## xgeFontBuildXRFMemory
- 位置: xge.h:1813  已注释: 否
- 签名: `XGE_API int xgeFontBuildXRFMemory(xge_font pFont, uint32_t iFirstCodepoint, uint32_t iCount, void** ppData, int* pSize);`
- 实现: src/xge_font.c:1235（体 145 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK
- 用法: test/test_text_foundation.c:210

## xgeFontSaveXRF
- 位置: xge.h:1814  已注释: 否
- 签名: `XGE_API int xgeFontSaveXRF(xge_font pFont, const char* sPath, uint32_t iFirstCodepoint, uint32_t iCount);`
- 实现: src/xge_font.c:1381（体 37 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK
- 用法: examples/tutorial_capture/ch82_main1.c:14; examples/xui_proxy_font/main.c:135; test/test_text_foundation.c:253

## xgeFontLoadCached
- 位置: xge.h:1815  已注释: 否
- 签名: `XGE_API int xgeFontLoadCached(xge_font pFont, const char* sTTFPath, const char* sXRFPath, float fSize, uint32_t iFirstCodepoint, uint32_t iCount);`
- 实现: src/xge_font.c:1419（体 43 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch83_main1.c:13; test/test_text_foundation.c:261

## xgeFontAddRef
- 位置: xge.h:1816  已注释: 否
- 签名: `XGE_API int xgeFontAddRef(xge_font pFont);`
- 实现: src/xge_font.c:1463（体 10 行）

## xgeFontFree
- 位置: xge.h:1817  已注释: 否
- 签名: `XGE_API void xgeFontFree(xge_font pFont);`
- 实现: src/xge_font.c:1474（体 26 行）
- 用法: examples/audit_text_render/main.c:55; examples/audit_text_render/main.c:58; examples/audit_text_render/main.c:61

## xgeFontSetFallback
- 位置: xge.h:1818  已注释: 否
- 签名: `XGE_API void xgeFontSetFallback(xge_font pFont, xge_font pFallback);`
- 实现: src/xge_font.c:1501（体 17 行）
- 用法: examples/tutorial_capture/ch85_main1.c:18

## xgeFontFallbackSet
- 位置: xge.h:1819  已注释: 否
- 签名: `XGE_API int xgeFontFallbackSet(const char* sPath, float fSize);`
- 实现: src/xge_font.c:1519（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT
- 用法: examples/xge_svg/main.c:1114; examples/xui_codeedit/main.c:432

## xgeFontFallbackSetMemory
- 位置: xge.h:1820  已注释: 否
- 签名: `XGE_API int xgeFontFallbackSetMemory(const void* pData, int iSize, float fSize);`
- 实现: src/xge_font.c:1536（体 16 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK

## xgeFontFallbackGet
- 位置: xge.h:1821  已注释: 否
- 签名: `XGE_API int xgeFontFallbackGet(xge_font pFont, float fSize);`
- 实现: src/xge_font.c:1579（体 4 行）

## xgeFontFallbackGetEx
- 位置: xge.h:1822  已注释: 否
- 签名: `XGE_API int xgeFontFallbackGetEx(xge_font pFont, float fSize, uint32_t iFlags);`
- 实现: src/xge_font.c:1553（体 25 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK

## xgeFontFallbackClear
- 位置: xge.h:1823  已注释: 否
- 签名: `XGE_API void xgeFontFallbackClear(void);`
- 实现: src/xge_font.c:1584（体 7 行）
- 用法: examples/xui_codeedit/main.c:435

## xgeFontGetMetrics
- 位置: xge.h:1824  已注释: 否
- 签名: `XGE_API int xgeFontGetMetrics(xge_font pFont, xge_font_metrics_t* pMetrics);`
- 实现: src/xge_font.c:1592（体 21 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch86_main1.c:23; test/test_emoji.c:209; test/test_text_foundation.c:140

## xgeFontCacheGetStats
- 位置: xge.h:1825  已注释: 否
- 签名: `XGE_API int xgeFontCacheGetStats(xge_font pFont, xge_font_cache_stats_t* pStats);`
- 实现: src/xge_font.c:1614（体 25 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: examples/tutorial_capture/ch86_main1.c:34; test/test_text_foundation.c:200; test/test_text_foundation.c:271

## xgeFontCacheClear
- 位置: xge.h:1826  已注释: 否
- 签名: `XGE_API void xgeFontCacheClear(xge_font pFont);`
- 实现: src/xge_font.c:1640（体 4 行）

## xgeFontGlyphGet
- 位置: xge.h:1827  已注释: 否
- 签名: `XGE_API int xgeFontGlyphGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_metrics_t* pMetrics);`
- 实现: src/xge_font.c:1645（体 36 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_RESOURCE_FAILED
- 用法: examples/tutorial_capture/ch87_main1.c:19; test_xui/xui_proxy_xge_test.c:93

## xgeFontGlyphGetByIndex
- 位置: xge.h:1828  已注释: 否
- 签名: `XGE_API int xgeFontGlyphGetByIndex(xge_font pFont, int iGlyph, xge_glyph_metrics_t* pMetrics);`
- 实现: src/xge_font.c:1682（体 40 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_NOT_INITIALIZED

## xgeFontGlyphRasterize
- 位置: xge.h:1829  已注释: 否
- 签名: `XGE_API int xgeFontGlyphRasterize(xge_font pFont, uint32_t iCodepoint, xge_glyph_bitmap_t* pBitmap);`
- 实现: src/xge_font.c:1750（体 18 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch87_main1.c:28

## xgeFontGlyphRasterizeByIndex
- 位置: xge.h:1830  已注释: 否
- 签名: `XGE_API int xgeFontGlyphRasterizeByIndex(xge_font pFont, int iGlyph, xge_glyph_bitmap_t* pBitmap);`
- 实现: src/xge_font.c:1723（体 26 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_NOT_INITIALIZED, XGE_ERROR_OUT_OF_MEMORY, XGE_OK

## xgeFontGlyphAtlasGet
- 位置: xge.h:1831  已注释: 否
- 签名: `XGE_API int xgeFontGlyphAtlasGet(xge_font pFont, uint32_t iCodepoint, xge_glyph_t* pGlyph);`
- 实现: src/xge_font.c:1832（体 72 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR_RESOURCE_FAILED, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: test/test_text_foundation.c:192; test/test_text_foundation.c:272

## xgeFontGlyphAtlasGetByIndex
- 位置: xge.h:1832  已注释: 否
- 签名: `XGE_API int xgeFontGlyphAtlasGetByIndex(xge_font pFont, int iGlyph, xge_glyph_t* pGlyph);`
- 实现: src/xge_font.c:1769（体 62 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY

## xgeFontGlyphOutlineAppendShapeEx
- 位置: xge.h:1850  已注释: 否
- 签名: `XGE_API int xgeFontGlyphOutlineAppendShapeEx(xge_font pFont, int iGlyph, xge_shape_ex pShape, float fPenX, float fBaselineY);`
- 实现: src/xge_text_vector.c:13（体 70 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_RESOURCE_FAILED

