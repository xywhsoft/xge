# 草稿包：xge.h / Glyph（8 条 API）

> 生成 2026-09-10 02:37 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeGlyphBitmapFree
- 位置: xge.h:1833  已注释: 否
- 签名: `XGE_API void xgeGlyphBitmapFree(xge_glyph_bitmap_t* pBitmap);`
- 实现: src/xge_font.c:1905（体 10 行）
- 用法: examples/tutorial_capture/ch87_main1.c:34

## xgeGlyphRunFree
- 位置: xge.h:1844  已注释: 否
- 签名: `XGE_API void xgeGlyphRunFree(xge_glyph_run_t* pRun);`
- 实现: src/xge_text_run.c:237（体 16 行）
- 用法: examples/audit_text_render/main.c:88; examples/tutorial_capture/ch88_main1.c:36; examples/tutorial_capture/ch89_main1.c:48

## xgeGlyphRunMeasure
- 位置: xge.h:1845  已注释: 否
- 签名: `XGE_API xge_vec2_t xgeGlyphRunMeasure(const xge_glyph_run_t* pRun);`
- 实现: src/xge_text_run.c:254（体 8 行）
- 用法: examples/tutorial_capture/ch88_main1.c:29; examples/xge_emoji/main.c:56

## xgeGlyphRunHitTest
- 位置: xge.h:1846  已注释: 否
- 签名: `XGE_API int xgeGlyphRunHitTest(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t* pCluster, int* pTrailing);`
- 实现: src/xge_text_run.c:263（体 42 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_emoji.c:248; test/test_text_foundation.c:164

## xgeGlyphRunDraw
- 位置: xge.h:1847  已注释: 否
- 签名: `XGE_API void xgeGlyphRunDraw(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xge_text_run.c:424（体 4 行）
- 用法: examples/tutorial_capture/ch88_main1.c:30; examples/xge_emoji/main.c:59; test/test_emoji_render_all.c:132

## xgeGlyphRunDrawSpans
- 位置: xge.h:1848  已注释: 否
- 签名: `XGE_API void xgeGlyphRunDrawSpans(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags, const xge_text_paint_span_t* pSpans, int iSpanCount);`
- 实现: src/xge_text_run.c:429（体 5 行）

## xgeGlyphRunDrawDecorated
- 位置: xge.h:1849  已注释: 否
- 签名: `XGE_API void xgeGlyphRunDrawDecorated(const xge_glyph_run_t* pRun, float fX, float fY, uint32_t iColor, uint32_t iFlags, const xge_text_decoration_t* pDecorations, int iDecorationCount);`
- 实现: src/xge_text_run.c:481（体 83 行）
- 用法: examples/audit_text_render/main.c:87; examples/tutorial_capture/ch89_main1.c:45

## xgeGlyphRunAppendShapeEx
- 位置: xge.h:1851  已注释: 否
- 签名: `XGE_API int xgeGlyphRunAppendShapeEx(const xge_glyph_run_t* pRun, xge_shape_ex pShape, float fX, float fY);`
- 实现: src/xge_text_vector.c:84（体 32 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK
- 用法: test/test_text_foundation.c:178

