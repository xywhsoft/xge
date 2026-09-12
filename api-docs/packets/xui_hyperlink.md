# 草稿包：xui.h / hyperlink（30 条 API）

> 生成 2026-09-10 03:00 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiHyperlinkGetType
- 位置: xui.h:6330  已注释: 否
- 签名: `XUI_API xui_widget_type xuiHyperlinkGetType(xui_context pContext);`
- 实现: src/xui_hyperlink.c:712（体 34 行）
- 返回码: NULL

## xuiHyperlinkCreate
- 位置: xui.h:6331  已注释: 否
- 签名: `XUI_API int xuiHyperlinkCreate(xui_context pContext, xui_widget* ppWidget, const xui_hyperlink_desc_t* pDesc);`
- 实现: src/xui_hyperlink.c:747（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch132_main1.c:19; examples/tutorial_capture/ch132_main1.c:27; examples/xui_hyperlink/main.c:172

## xuiHyperlinkSetClick
- 位置: xui.h:6332  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetClick(xui_widget pWidget, xui_hyperlink_click_proc onClick, void* pUser);`
- 实现: src/xui_hyperlink.c:762（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_hyperlink/main.c:178; test_xui/xui_hyperlink_test.c:124

## xuiHyperlinkSetText
- 位置: xui.h:6333  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_hyperlink.c:775（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_consumer_display_test.c:156; test_xui/xui_text_consumer_display_test.c:165

## xuiHyperlinkGetText
- 位置: xui.h:6334  已注释: 否
- 签名: `XUI_API const char* xuiHyperlinkGetText(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:797（体 7 行）
- 用法: test_xui/xui_hyperlink_test.c:117; test_xui/xui_text_consumer_display_test.c:161

## xuiHyperlinkSetFont
- 位置: xui.h:6335  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_hyperlink.c:805（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiHyperlinkGetFont
- 位置: xui.h:6336  已注释: 否
- 签名: `XUI_API xui_font xuiHyperlinkGetFont(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:820（体 7 行）

## xuiHyperlinkSetTextColor
- 位置: xui.h:6337  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_hyperlink.c:840（体 7 行）

## xuiHyperlinkGetTextColor
- 位置: xui.h:6338  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetTextColor(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:848（体 7 行）
- 用法: examples/xui_hyperlink/main.c:246; test_xui/xui_hyperlink_test.c:118; test_xui/xui_style_basic_content_test.c:30

## xuiHyperlinkSetHoverTextColor
- 位置: xui.h:6339  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetHoverTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_hyperlink.c:856（体 7 行）

## xuiHyperlinkGetHoverTextColor
- 位置: xui.h:6340  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetHoverTextColor(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:864（体 7 行）

## xuiHyperlinkSetActiveTextColor
- 位置: xui.h:6341  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetActiveTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_hyperlink.c:872（体 7 行）

## xuiHyperlinkGetActiveTextColor
- 位置: xui.h:6342  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetActiveTextColor(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:880（体 7 行）

## xuiHyperlinkSetDisabledTextColor
- 位置: xui.h:6343  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_hyperlink.c:888（体 7 行）

## xuiHyperlinkGetDisabledTextColor
- 位置: xui.h:6344  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:896（体 7 行）

## xuiHyperlinkSetTextColors
- 位置: xui.h:6345  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetTextColors(xui_widget pWidget, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iDisabled);`
- 实现: src/xui_hyperlink.c:904（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_hyperlink/main.c:212

## xuiHyperlinkSetTextFlags
- 位置: xui.h:6346  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_hyperlink.c:925（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_hyperlink/main.c:220

## xuiHyperlinkGetTextFlags
- 位置: xui.h:6347  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:941（体 7 行）

## xuiHyperlinkSetWrapMode
- 位置: xui.h:6348  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetWrapMode(xui_widget pWidget, int iWrapMode);`
- 实现: src/xui_hyperlink.c:949（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_hyperlink/main.c:219; test_xui/xui_text_consumer_display_test.c:157; test_xui/xui_text_consumer_display_test.c:192

## xuiHyperlinkGetWrapMode
- 位置: xui.h:6349  已注释: 否
- 签名: `XUI_API int xuiHyperlinkGetWrapMode(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:967（体 7 行）
- 用法: examples/xui_hyperlink/main.c:247

## xuiHyperlinkSetUnderline
- 位置: xui.h:6350  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetUnderline(xui_widget pWidget, int bNormal, int bHover, int bActive);`
- 实现: src/xui_hyperlink.c:975（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_hyperlink/main.c:218

## xuiHyperlinkGetUnderline
- 位置: xui.h:6351  已注释: 否
- 签名: `XUI_API int xuiHyperlinkGetUnderline(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:997（体 7 行）

## xuiHyperlinkGetHoverUnderline
- 位置: xui.h:6352  已注释: 否
- 签名: `XUI_API int xuiHyperlinkGetHoverUnderline(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1005（体 7 行）
- 用法: test_xui/xui_hyperlink_test.c:119

## xuiHyperlinkGetActiveUnderline
- 位置: xui.h:6353  已注释: 否
- 签名: `XUI_API int xuiHyperlinkGetActiveUnderline(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1013（体 7 行）
- 用法: test_xui/xui_hyperlink_test.c:119

## xuiHyperlinkSetLineGap
- 位置: xui.h:6354  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetLineGap(xui_widget pWidget, float fLineGap);`
- 实现: src/xui_hyperlink.c:1021（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiHyperlinkGetLineGap
- 位置: xui.h:6355  已注释: 否
- 签名: `XUI_API float xuiHyperlinkGetLineGap(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1039（体 7 行）

## xuiHyperlinkSetParagraphGap
- 位置: xui.h:6356  已注释: 否
- 签名: `XUI_API int xuiHyperlinkSetParagraphGap(xui_widget pWidget, float fParagraphGap);`
- 实现: src/xui_hyperlink.c:1047（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiHyperlinkGetParagraphGap
- 位置: xui.h:6357  已注释: 否
- 签名: `XUI_API float xuiHyperlinkGetParagraphGap(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1065（体 7 行）

## xuiHyperlinkGetState
- 位置: xui.h:6358  已注释: 否
- 签名: `XUI_API uint32_t xuiHyperlinkGetState(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1073（体 11 行）
- 用法: examples/xui_hyperlink/main.c:154; test_xui/xui_hyperlink_test.c:149; test_xui/xui_hyperlink_test.c:155

## xuiHyperlinkGetClickCount
- 位置: xui.h:6359  已注释: 否
- 签名: `XUI_API int xuiHyperlinkGetClickCount(xui_widget pWidget);`
- 实现: src/xui_hyperlink.c:1085（体 7 行）
- 用法: examples/xui_hyperlink/main.c:260; test_xui/xui_hyperlink_test.c:161; test_xui/xui_hyperlink_test.c:174

