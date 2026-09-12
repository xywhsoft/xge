# 草稿包：xge.h / Text（6 条 API）

> 生成 2026-09-10 02:37 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xgeTextUTF8Next
- 位置: xge.h:1796  已注释: 否
- 签名: `XGE_API int xgeTextUTF8Next(const char** psText, uint32_t* pCodepoint);`
- 实现: src/xge_font.c:621（体 55 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_ERROR, XGE_ERROR_UNSUPPORTED, XGE_OK
- 用法: test/test_text_foundation.c:90; test_xui/xui_proxy_xge_test.c:87

## xgeTextShape
- 位置: xge.h:1843  已注释: 否
- 签名: `XGE_API int xgeTextShape(const xge_text_shape_desc_t* pDesc, xge_glyph_run_t* pRun);`
- 实现: src/xge_text_run.c:41（体 195 行）
- 返回码: XGE_ERROR_INVALID_ARGUMENT, XGE_OK, XGE_ERROR_OUT_OF_MEMORY
- 用法: examples/audit_text_render/main.c:81; examples/tutorial_capture/ch88_main1.c:27; examples/tutorial_capture/ch89_main1.c:25

## xgeTextMeasure
- 位置: xge.h:1852  已注释: 否
- 签名: `XGE_API xge_vec2_t xgeTextMeasure(xge_font pFont, const char* sText);`
- 实现: src/xge_font.c:2077（体 9 行）
- 用法: examples/audit_text_render/main.c:67; examples/tutorial_capture/ch90_main1.c:20; test/test_emoji.c:238

## xgeTextDraw
- 位置: xge.h:1853  已注释: 否
- 签名: `XGE_API void xgeTextDraw(xge_font pFont, const char* sText, float fX, float fY, uint32_t iColor);`
- 实现: src/xge_font.c:2126（体 22 行）
- 用法: examples/audit_text_render/main.c:42; examples/audit_text_render/main.c:57; examples/audit_text_render/main.c:60

## xgeTextDrawRect
- 位置: xge.h:1854  已注释: 否
- 签名: `XGE_API void xgeTextDrawRect(xge_font pFont, const char* sText, xge_rect_t tRect, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xge_font.c:2149（体 102 行）
- 用法: examples/audit_text_render/main.c:47; examples/audit_text_render/main.c:49; examples/audit_text_render/main.c:51

## xgeTextGet
- 位置: xge.h:2448  已注释: 否
- 签名: `XGE_API uint32_t xgeTextGet(void);`
- 实现: src/xge_input.c:284（体 4 行）
- 用法: test_xui/xui_proxy_xge_test.c:685; test_xui/xui_proxy_xge_test.c:689; test_xui/xui_proxy_xge_test.c:690

