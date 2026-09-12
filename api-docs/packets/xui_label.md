# 草稿包：xui.h / label（20 条 API）

> 生成 2026-09-10 03:01 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiLabelGetType
- 位置: xui.h:6309  已注释: 否
- 签名: `XUI_API xui_widget_type xuiLabelGetType(xui_context pContext);`
- 实现: src/xui_label.c:463（体 34 行）
- 返回码: NULL
- 用法: examples/xui_label/main.c:251; test_xui/xui_label_test.c:895

## xuiLabelCreate
- 位置: xui.h:6310  已注释: 否
- 签名: `XUI_API int xuiLabelCreate(xui_context pContext, xui_widget* ppWidget, const xui_label_desc_t* pDesc);`
- 实现: src/xui_label.c:498（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_basic/main.c:18; examples/audit_xui_basic/main.c:29; examples/audit_xui_basic/main.c:40

## xuiLabelSetText
- 位置: xui.h:6311  已注释: 否
- 签名: `XUI_API int xuiLabelSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_label.c:513（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/tutorial_capture/ch124_main1.c:21; examples/xui_breadcrumb/main.c:179; examples/xui_carousel/main.c:262

## xuiLabelGetText
- 位置: xui.h:6312  已注释: 否
- 签名: `XUI_API const char* xuiLabelGetText(xui_widget pWidget);`
- 实现: src/xui_label.c:536（体 7 行）
- 用法: examples/xui_panel/main.c:319; test_xui/xui_label_test.c:917; test_xui/xui_text_consumer_display_test.c:161

## xuiLabelSetFont
- 位置: xui.h:6313  已注释: 否
- 签名: `XUI_API int xuiLabelSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_label.c:544（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetFont
- 位置: xui.h:6314  已注释: 否
- 签名: `XUI_API xui_font xuiLabelGetFont(xui_widget pWidget);`
- 实现: src/xui_label.c:559（体 7 行）

## xuiLabelSetTextColor
- 位置: xui.h:6315  已注释: 否
- 签名: `XUI_API int xuiLabelSetTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_label.c:567（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetTextColor
- 位置: xui.h:6316  已注释: 否
- 签名: `XUI_API uint32_t xuiLabelGetTextColor(xui_widget pWidget);`
- 实现: src/xui_label.c:582（体 7 行）
- 用法: test_xui/xui_label_test.c:918; test_xui/xui_style_basic_content_test.c:30

## xuiLabelSetDisabledTextColor
- 位置: xui.h:6317  已注释: 否
- 签名: `XUI_API int xuiLabelSetDisabledTextColor(xui_widget pWidget, uint32_t iColor);`
- 实现: src/xui_label.c:590（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetDisabledTextColor
- 位置: xui.h:6318  已注释: 否
- 签名: `XUI_API uint32_t xuiLabelGetDisabledTextColor(xui_widget pWidget);`
- 实现: src/xui_label.c:605（体 7 行）

## xuiLabelSetTextFlags
- 位置: xui.h:6319  已注释: 否
- 签名: `XUI_API int xuiLabelSetTextFlags(xui_widget pWidget, uint32_t iTextFlags);`
- 实现: src/xui_label.c:613（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetTextFlags
- 位置: xui.h:6320  已注释: 否
- 签名: `XUI_API uint32_t xuiLabelGetTextFlags(xui_widget pWidget);`
- 实现: src/xui_label.c:629（体 7 行）
- 用法: test_xui/xui_label_test.c:919

## xuiLabelSetWrapMode
- 位置: xui.h:6321  已注释: 否
- 签名: `XUI_API int xuiLabelSetWrapMode(xui_widget pWidget, int iWrapMode);`
- 实现: src/xui_label.c:637（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_label_test.c:927; test_xui/xui_text_consumer_display_test.c:157; test_xui/xui_text_consumer_display_test.c:192

## xuiLabelGetWrapMode
- 位置: xui.h:6322  已注释: 否
- 签名: `XUI_API int xuiLabelGetWrapMode(xui_widget pWidget);`
- 实现: src/xui_label.c:655（体 7 行）

## xuiLabelSetUnderline
- 位置: xui.h:6323  已注释: 否
- 签名: `XUI_API int xuiLabelSetUnderline(xui_widget pWidget, int bUnderline);`
- 实现: src/xui_label.c:663（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_label/main.c:421

## xuiLabelGetUnderline
- 位置: xui.h:6324  已注释: 否
- 签名: `XUI_API int xuiLabelGetUnderline(xui_widget pWidget);`
- 实现: src/xui_label.c:679（体 7 行）
- 用法: examples/xui_label/main.c:486

## xuiLabelSetLineGap
- 位置: xui.h:6325  已注释: 否
- 签名: `XUI_API int xuiLabelSetLineGap(xui_widget pWidget, float fLineGap);`
- 实现: src/xui_label.c:687（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetLineGap
- 位置: xui.h:6326  已注释: 否
- 签名: `XUI_API float xuiLabelGetLineGap(xui_widget pWidget);`
- 实现: src/xui_label.c:705（体 7 行）

## xuiLabelSetParagraphGap
- 位置: xui.h:6327  已注释: 否
- 签名: `XUI_API int xuiLabelSetParagraphGap(xui_widget pWidget, float fParagraphGap);`
- 实现: src/xui_label.c:713（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiLabelGetParagraphGap
- 位置: xui.h:6328  已注释: 否
- 签名: `XUI_API float xuiLabelGetParagraphGap(xui_widget pWidget);`
- 实现: src/xui_label.c:731（体 7 行）

