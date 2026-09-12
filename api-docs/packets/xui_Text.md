# 草稿包：xui.h / Text（12 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTextLayoutCreate
- 位置: xui.h:6045  已注释: 否
- 签名: `XUI_API int xuiTextLayoutCreate(xui_context pContext, xui_text_layout* ppLayout, const xui_text_layout_desc_t* pDesc);`
- 实现: src/xui_text.c:729（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_message_list_audit_test.c:26; test_xui/xui_text_consumer_display_test.c:25; test_xui/xui_text_dpi_test.c:109

## xuiTextLayoutDestroy
- 位置: xui.h:6046  已注释: 否
- 签名: `XUI_API void xuiTextLayoutDestroy(xui_text_layout pLayout);`
- 实现: src/xui_text.c:756（体 13 行）
- 用法: test_xui/xui_message_list_audit_test.c:38; test_xui/xui_text_consumer_display_test.c:52; test_xui/xui_text_dpi_test.c:185

## xuiTextLayoutReset
- 位置: xui.h:6047  已注释: 否
- 签名: `XUI_API int xuiTextLayoutReset(xui_text_layout pLayout, const xui_text_layout_desc_t* pDesc);`
- 实现: src/xui_text.c:810（体 36 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_INVALID_STATE, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_message_list_audit_test.c:32; test_xui/xui_text_dpi_test.c:32; test_xui/xui_text_layout_test.c:103

## xuiTextLayoutGetSize
- 位置: xui.h:6048  已注释: 否
- 签名: `XUI_API xui_vec2_t xuiTextLayoutGetSize(xui_text_layout pLayout);`
- 实现: src/xui_text.c:882（体 11 行）
- 用法: test_xui/xui_text_dpi_test.c:30; test_xui/xui_text_dpi_test.c:128; test_xui/xui_text_dpi_test.c:142

## xuiTextLayoutGetLineCount
- 位置: xui.h:6049  已注释: 否
- 签名: `XUI_API int xuiTextLayoutGetLineCount(xui_text_layout pLayout);`
- 实现: src/xui_text.c:894（体 4 行）
- 用法: test_xui/xui_message_list_audit_test.c:176; test_xui/xui_message_list_audit_test.c:383; test_xui/xui_message_list_audit_test.c:471

## xuiTextLayoutGetLine
- 位置: xui.h:6050  已注释: 否
- 签名: `XUI_API int xuiTextLayoutGetLine(xui_text_layout pLayout, int iIndex, xui_text_line_t* pLine);`
- 实现: src/xui_text.c:899（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_message_list_audit_test.c:110; test_xui/xui_message_list_audit_test.c:353; test_xui/xui_message_list_audit_test.c:389

## xuiTextLayoutGetText
- 位置: xui.h:6051  已注释: 否
- 签名: `XUI_API const char* xuiTextLayoutGetText(xui_text_layout pLayout);`
- 实现: src/xui_text.c:912（体 8 行）
- 返回码: NULL
- 用法: test_xui/xui_text_consumer_display_test.c:30; test_xui/xui_text_consumer_display_test.c:45; test_xui/xui_text_consumer_display_test.c:46

## xuiTextLayoutGetTruncated
- 位置: xui.h:6052  已注释: 否
- 签名: `XUI_API int xuiTextLayoutGetTruncated(xui_text_layout pLayout);`
- 实现: src/xui_text.c:921（体 4 行）
- 用法: test_xui/xui_text_dpi_test.c:132; test_xui/xui_text_layout_test.c:105; test_xui/xui_text_layout_test.c:127

## xuiTextLayoutDraw
- 位置: xui.h:6053  已注释: 否
- 签名: `XUI_API int xuiTextLayoutDraw(xui_text_layout pLayout, xui_surface pTarget, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags);`
- 实现: src/xui_text.c:988（体 62 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_OK
- 用法: test_xui/xui_text_dpi_test.c:133; test_xui/xui_text_dpi_test.c:164; test_xui/xui_text_dpi_test.c:203

## xuiTextMeasureLayout
- 位置: xui.h:6054  已注释: 否
- 签名: `XUI_API int xuiTextMeasureLayout(xui_context pContext, const xui_text_layout_desc_t* pDesc, xui_vec2_t* pSize);`
- 实现: src/xui_text.c:1051（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_layout_test.c:174; test_xui/xui_text_test.c:187

## xuiTextShape
- 位置: xui.h:6055  已注释: 否
- 签名: `XUI_API int xuiTextShape(xui_context pContext, xui_font pFont, const char* sText, int iTextSize, uint32_t iFlags, xui_text_shape_t* pShape);`
- 实现: src/xui_text.c:667（体 61 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_message_list_audit_test.c:463; test_xui/xui_text_projection_test.c:127; test_xui/xui_text_projection_test.c:185

## xuiTextShapeFree
- 位置: xui.h:6057  已注释: 否
- 签名: `XUI_API void xuiTextShapeFree(xui_text_shape_t* pShape);`
- 实现: src/xui_text.c:660（体 6 行）
- 用法: test_xui/xui_message_list_audit_test.c:465; test_xui/xui_text_projection_test.c:136; test_xui/xui_text_projection_test.c:187

