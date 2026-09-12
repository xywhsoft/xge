# 草稿包：xui.h / taginput（33 条 API）

> 生成 2026-09-10 03:07 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTagInputGetType
- 位置: xui.h:7641  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTagInputGetType(xui_context pContext);`
- 实现: src/xui_tag_input.c:1345（体 37 行）
- 返回码: NULL

## xuiTagInputCreate
- 位置: xui.h:7642  已注释: 否
- 签名: `XUI_API int xuiTagInputCreate(xui_context pContext, xui_widget* ppWidget, const xui_tag_input_desc_t* pDesc);`
- 实现: src/xui_tag_input.c:1383（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch168_main1.c:19; examples/xui_taginput/main.c:184; test_xui/xui_edit_contract_test.c:179

## xuiTagInputSetChange
- 位置: xui.h:7643  已注释: 否
- 签名: `XUI_API int xuiTagInputSetChange(xui_widget pWidget, xui_tag_input_change_proc onChange, void* pUser);`
- 实现: src/xui_tag_input.c:1398（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_taginput/main.c:186; test_xui/xui_tag_input_test.c:90

## xuiTagInputSetContextMenu
- 位置: xui.h:7644  已注释: 否
- 签名: `XUI_API int xuiTagInputSetContextMenu(xui_widget pWidget, xui_tag_input_context_proc onContext, void* pUser);`
- 实现: src/xui_tag_input.c:1409（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputAddTag
- 位置: xui.h:7645  已注释: 否
- 签名: `XUI_API int xuiTagInputAddTag(xui_widget pWidget, const char* sText);`
- 实现: src/xui_tag_input.c:1418（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:305; examples/xui_taginput/main.c:314; examples/xui_taginput/main.c:316

## xuiTagInputRemoveTag
- 位置: xui.h:7646  已注释: 否
- 签名: `XUI_API int xuiTagInputRemoveTag(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tag_input.c:1427（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:307

## xuiTagInputClearTags
- 位置: xui.h:7647  已注释: 否
- 签名: `XUI_API int xuiTagInputClearTags(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1436（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_taginput/main.c:312

## xuiTagInputSetTags
- 位置: xui.h:7648  已注释: 否
- 签名: `XUI_API int xuiTagInputSetTags(xui_widget pWidget, const char* const* ppTags, int iCount);`
- 实现: src/xui_tag_input.c:1457（体 37 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:309

## xuiTagInputGetTagCount
- 位置: xui.h:7649  已注释: 否
- 签名: `XUI_API int xuiTagInputGetTagCount(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1495（体 7 行）
- 用法: examples/xui_taginput/main.c:160; examples/xui_taginput/main.c:161; examples/xui_taginput/main.c:162

## xuiTagInputGetTag
- 位置: xui.h:7650  已注释: 否
- 签名: `XUI_API const char* xuiTagInputGetTag(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tag_input.c:1503（体 8 行）
- 返回码: NULL
- 用法: examples/xui_taginput/main.c:298; examples/xui_taginput/main.c:304; examples/xui_taginput/main.c:311

## xuiTagInputSetText
- 位置: xui.h:7651  已注释: 否
- 签名: `XUI_API int xuiTagInputSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_tag_input.c:1512（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:301; test_xui/xui_tag_input_test.c:110

## xuiTagInputGetText
- 位置: xui.h:7652  已注释: 否
- 签名: `XUI_API const char* xuiTagInputGetText(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1521（体 7 行）
- 用法: test_xui/xui_tag_input_test.c:111; test_xui/xui_tag_input_test.c:115; test_xui/xui_tag_input_test.c:136

## xuiTagInputCommit
- 位置: xui.h:7653  已注释: 否
- 签名: `XUI_API int xuiTagInputCommit(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1529（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:302; test_xui/xui_tag_input_test.c:112

## xuiTagInputSetPlaceholder
- 位置: xui.h:7654  已注释: 否
- 签名: `XUI_API int xuiTagInputSetPlaceholder(xui_widget pWidget, const char* sText);`
- 实现: src/xui_tag_input.c:1543（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_style_chrome_test.c:472

## xuiTagInputGetPlaceholder
- 位置: xui.h:7655  已注释: 否
- 签名: `XUI_API const char* xuiTagInputGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1558（体 7 行）
- 用法: test_xui/xui_tag_input_test.c:86

## xuiTagInputSetFont
- 位置: xui.h:7656  已注释: 否
- 签名: `XUI_API int xuiTagInputSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_tag_input.c:1566（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputGetFont
- 位置: xui.h:7657  已注释: 否
- 签名: `XUI_API xui_font xuiTagInputGetFont(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1578（体 7 行）

## xuiTagInputSetMaxTags
- 位置: xui.h:7658  已注释: 否
- 签名: `XUI_API int xuiTagInputSetMaxTags(xui_widget pWidget, int iMaxTags);`
- 实现: src/xui_tag_input.c:1586（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputGetMaxTags
- 位置: xui.h:7659  已注释: 否
- 签名: `XUI_API int xuiTagInputGetMaxTags(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1598（体 7 行）
- 用法: examples/xui_taginput/main.c:287

## xuiTagInputSetMaxLength
- 位置: xui.h:7660  已注释: 否
- 签名: `XUI_API int xuiTagInputSetMaxLength(xui_widget pWidget, int iMaxLength);`
- 实现: src/xui_tag_input.c:1606（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputGetMaxLength
- 位置: xui.h:7661  已注释: 否
- 签名: `XUI_API int xuiTagInputGetMaxLength(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1617（体 7 行）

## xuiTagInputSetColors
- 位置: xui.h:7662  已注释: 否
- 签名: `XUI_API int xuiTagInputSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iBorder, uint32_t iFocusBorder, uint32_t iTagBackground, uint32_t iTagText);`
- 实现: src/xui_tag_input.c:1625（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_taginput/main.c:223

## xuiTagInputGetColors
- 位置: xui.h:7663  已注释: 否
- 签名: `XUI_API int xuiTagInputGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pBorder, uint32_t* pFocusBorder, uint32_t* pTagBackground, uint32_t* pTagText);`
- 实现: src/xui_tag_input.c:1643（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputSetExtendedColors
- 位置: xui.h:7664  已注释: 否
- 签名: `XUI_API int xuiTagInputSetExtendedColors(xui_widget pWidget, uint32_t iText, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iFocusBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iTagHoverBackground, uint32_t iTagClose, uint32_t iTagCloseHover);`
- 实现: src/xui_tag_input.c:1657（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTagInputGetExtendedColors
- 位置: xui.h:7665  已注释: 否
- 签名: `XUI_API int xuiTagInputGetExtendedColors(xui_widget pWidget, uint32_t* pText, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pFocusBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pTagHoverBackground, uint32_t* pTagClose, uint32_t* pTagCloseHover);`
- 实现: src/xui_tag_input.c:1677（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputSetVisualMetrics
- 位置: xui.h:7666  已注释: 否
- 签名: `XUI_API int xuiTagInputSetVisualMetrics(xui_widget pWidget, float fBorderWidth, float fTagHeight);`
- 实现: src/xui_tag_input.c:1696（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTagInputGetVisualMetrics
- 位置: xui.h:7667  已注释: 否
- 签名: `XUI_API int xuiTagInputGetVisualMetrics(xui_widget pWidget, float* pBorderWidth, float* pTagHeight);`
- 实现: src/xui_tag_input.c:1709（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTagInputGetTagRect
- 位置: xui.h:7668  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTagInputGetTagRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tag_input.c:1720（体 8 行）

## xuiTagInputGetCloseRect
- 位置: xui.h:7669  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTagInputGetCloseRect(xui_widget pWidget, int iIndex);`
- 实现: src/xui_tag_input.c:1729（体 8 行）
- 用法: examples/xui_taginput/main.c:283; test_xui/xui_tag_input_test.c:103; test_xui/xui_tag_input_test.c:146

## xuiTagInputGetInputRect
- 位置: xui.h:7670  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTagInputGetInputRect(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1738（体 7 行）
- 用法: examples/xui_taginput/main.c:282; test_xui/xui_tag_input_test.c:101

## xuiTagInputGetInputWidget
- 位置: xui.h:7671  已注释: 否
- 签名: `XUI_API xui_widget xuiTagInputGetInputWidget(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1746（体 7 行）
- 用法: examples/xui_taginput/main.c:291; test_xui/xui_style_chrome_test.c:444; test_xui/xui_tag_input_test.c:94

## xuiTagInputGetState
- 位置: xui.h:7672  已注释: 否
- 签名: `XUI_API uint32_t xuiTagInputGetState(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1754（体 9 行）
- 用法: examples/xui_taginput/main.c:286; test_xui/xui_tag_input_test.c:165

## xuiTagInputGetChangeCount
- 位置: xui.h:7673  已注释: 否
- 签名: `XUI_API int xuiTagInputGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_tag_input.c:1764（体 7 行）

