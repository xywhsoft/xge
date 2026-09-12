# 草稿包：xui.h / codeedit（102 条 API）

> 生成 2026-09-10 02:58 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCodeEditGetType
- 位置: xui.h:7123  已注释: 否
- 签名: `XUI_API xui_widget_type xuiCodeEditGetType(xui_context pContext);`
- 实现: src/xui_code_edit.c:5940（体 31 行）
- 返回码: NULL
- 用法: test_xui/xui_code_edit_test.c:553; test_xui/xui_code_edit_test.c:555

## xuiCodeEditCreate
- 位置: xui.h:7124  已注释: 否
- 签名: `XUI_API int xuiCodeEditCreate(xui_context pContext, xui_widget* ppWidget, const xui_code_edit_desc_t* pDesc);`
- 实现: src/xui_code_edit.c:5972（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch173_main1.c:18; examples/tutorial_capture/ch174_main1.c:17; examples/tutorial_capture/ch175_main1.c:18

## xuiCodeEditGetDocument
- 位置: xui.h:7125  已注释: 否
- 签名: `XUI_API xui_code_document xuiCodeEditGetDocument(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:5985（体 5 行）
- 用法: examples/xui_codeedit/main.c:350; examples/xui_codeedit/main.c:381; test_xui/xui_code_edit_large_perf_test.c:204

## xuiCodeEditGetSelection
- 位置: xui.h:7126  已注释: 否
- 签名: `XUI_API xui_code_selection_model xuiCodeEditGetSelection(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:5991（体 5 行）
- 用法: examples/xui_codeedit/main.c:371; examples/xui_codeedit/main.c:380; test_xui/xui_code_edit_large_perf_test.c:253

## xuiCodeEditGetTheme
- 位置: xui.h:7127  已注释: 否
- 签名: `XUI_API xui_code_theme xuiCodeEditGetTheme(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:5997（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:675; test_xui/xui_code_edit_test.c:1397

## xuiCodeEditSetTheme
- 位置: xui.h:7128  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetTheme(xui_widget pWidget, xui_code_theme pTheme);`
- 实现: src/xui_code_edit.c:6003（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1436

## xuiCodeEditSetStyle
- 位置: xui.h:7129  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetStyle(xui_widget pWidget, int iStyleId, const xui_code_style_t* pStyle);`
- 实现: src/xui_code_edit.c:6015（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1444

## xuiCodeEditGetFoldState
- 位置: xui.h:7130  已注释: 否
- 签名: `XUI_API xui_code_fold_state xuiCodeEditGetFoldState(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6027（体 5 行）
- 用法: examples/xui_codeedit/main.c:298; test_xui/xui_code_edit_large_perf_test.c:290; test_xui/xui_code_edit_large_perf_test.c:295

## xuiCodeEditGetAnnotations
- 位置: xui.h:7131  已注释: 否
- 签名: `XUI_API xui_code_annotation_store xuiCodeEditGetAnnotations(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6033（体 5 行）
- 用法: examples/xui_codeedit/main.c:283; examples/xui_codeedit/main.c:294; test_xui/xui_code_edit_test.c:586

## xuiCodeEditHitTestDiagnostic
- 位置: xui.h:7132  已注释: 否
- 签名: `XUI_API int xuiCodeEditHitTestDiagnostic(xui_widget pWidget, float fX, float fY, xui_code_diagnostic_hit_t* pHit);`
- 实现: src/xui_code_edit.c:6039（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:632; test_xui/xui_code_edit_test.c:634

## xuiCodeEditSetDiagnosticHover
- 位置: xui.h:7133  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetDiagnosticHover(xui_widget pWidget, xui_code_diagnostic_hover_proc onHover, void* pUser);`
- 实现: src/xui_code_edit.c:6046（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:636

## xuiCodeEditGetTokenBuffer
- 位置: xui.h:7134  已注释: 否
- 签名: `XUI_API xui_code_token_buffer xuiCodeEditGetTokenBuffer(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6060（体 5 行）
- 用法: examples/xui_codeedit/main.c:354; test_xui/xui_code_edit_large_perf_test.c:219; test_xui/xui_code_edit_large_perf_test.c:235

## xuiCodeEditGetProviders
- 位置: xui.h:7135  已注释: 否
- 签名: `XUI_API xui_code_provider_set xuiCodeEditGetProviders(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6066（体 5 行）
- 用法: examples/xui_codeedit/main.c:274; examples/xui_codeedit/main.c:276; test_xui/xui_code_edit_test.c:679

## xuiCodeEditGetMargins
- 位置: xui.h:7136  已注释: 否
- 签名: `XUI_API xui_code_margin_model xuiCodeEditGetMargins(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6072（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:612; test_xui/xui_code_edit_test.c:627; test_xui/xui_code_edit_test.c:680

## xuiCodeEditGetCommandMap
- 位置: xui.h:7137  已注释: 否
- 签名: `XUI_API xui_code_command_map xuiCodeEditGetCommandMap(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6120（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:681; test_xui/xui_code_edit_test.c:699; test_xui/xui_code_edit_test.c:1588

## xuiCodeEditGetLanguageRegistry
- 位置: xui.h:7138  已注释: 否
- 签名: `XUI_API xui_code_language_registry xuiCodeEditGetLanguageRegistry(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6126（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:682; test_xui/xui_code_edit_test.c:1613

## xuiCodeEditSetLanguage
- 位置: xui.h:7139  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetLanguage(xui_widget pWidget, const char* sLanguage);`
- 实现: src/xui_code_edit.c:6132（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1615; test_xui/xui_code_edit_test.c:1617

## xuiCodeEditGetLanguage
- 位置: xui.h:7140  已注释: 否
- 签名: `XUI_API const char* xuiCodeEditGetLanguage(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6152（体 5 行）
- 用法: examples/xui_codeedit/main.c:351; test_xui/xui_code_edit_test.c:1604; test_xui/xui_code_edit_test.c:1618

## xuiCodeEditGetMenuWidget
- 位置: xui.h:7141  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeEditGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6158（体 5 行）
- 用法: examples/xui_codeedit/main.c:355; test_xui/xui_code_edit_test.c:683

## xuiCodeEditSetMenuTitle
- 位置: xui.h:7142  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_code_edit.c:6164（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY

## xuiCodeEditGetMenuTitle
- 位置: xui.h:7143  已注释: 否
- 签名: `XUI_API const char* xuiCodeEditGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_code_edit.c:6188（体 7 行）

## xuiCodeEditSetFont
- 位置: xui.h:7144  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_code_edit.c:6196（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditGetFont
- 位置: xui.h:7145  已注释: 否
- 签名: `XUI_API xui_font xuiCodeEditGetFont(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6228（体 5 行）

## xuiCodeEditSetText
- 位置: xui.h:7146  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_code_edit.c:6668（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:727; test_xui/xui_code_edit_test.c:731; test_xui/xui_code_edit_test.c:733

## xuiCodeEditSetTextLength
- 位置: xui.h:7147  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetTextLength(xui_widget pWidget, const char* sText, int iLength);`
- 实现: src/xui_code_edit.c:6673（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_large_perf_test.c:198

## xuiCodeEditLoadTextFile
- 位置: xui.h:7148  已注释: 否
- 签名: `XUI_API int xuiCodeEditLoadTextFile(xui_widget pWidget, const char* sPath, int iCharset);`
- 实现: src/xui_code_edit.c:6689（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_large_perf_test.c:195

## xuiCodeEditGetText
- 位置: xui.h:7149  已注释: 否
- 签名: `XUI_API const char* xuiCodeEditGetText(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6705（体 5 行）
- 用法: examples/xui_codeedit/main.c:377; examples/xui_codeedit/main.c:379; examples/xui_codeedit/main.c:395

## xuiCodeEditSetInputHandler
- 位置: xui.h:7150  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetInputHandler(xui_widget pWidget, xui_code_input_proc onInput, void* pUser);`
- 实现: src/xui_code_edit.c:6711（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: examples/xui_codeedit/main.c:278; test_xui/xui_code_edit_test.c:1836; test_xui/xui_code_edit_test.c:1862

## xuiCodeEditApplyTextEdits
- 位置: xui.h:7151  已注释: 否
- 签名: `XUI_API int xuiCodeEditApplyTextEdits(xui_widget pWidget, const xui_code_text_edit_t* pEdits, int iEditCount, int iSelectionAnchor, int iSelectionCaret);`
- 实现: src/xui_code_edit.c:6722（体 23 行）
- 返回码: XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1874; test_xui/xui_code_edit_test.c:1894

## xuiCodeEditSetInlineCompletion
- 位置: xui.h:7152  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetInlineCompletion(xui_widget pWidget, int iOffset, const char* sText);`
- 实现: src/xui_code_edit.c:6746（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_codeedit/main.c:296

## xuiCodeEditClearInlineCompletion
- 位置: xui.h:7153  已注释: 否
- 签名: `XUI_API int xuiCodeEditClearInlineCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6767（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditHasInlineCompletion
- 位置: xui.h:7154  已注释: 否
- 签名: `XUI_API int xuiCodeEditHasInlineCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6778（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1640; test_xui/xui_code_edit_test.c:1657; test_xui/xui_code_edit_test.c:1662

## xuiCodeEditGetInlineCompletion
- 位置: xui.h:7155  已注释: 否
- 签名: `XUI_API const char* xuiCodeEditGetInlineCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6784（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1641; test_xui/xui_code_edit_test.c:1658; test_xui/xui_code_edit_test.c:1663

## xuiCodeEditAcceptInlineCompletion
- 位置: xui.h:7156  已注释: 否
- 签名: `XUI_API int xuiCodeEditAcceptInlineCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6790（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED

## xuiCodeEditSetVirtualText
- 位置: xui.h:7157  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetVirtualText(xui_widget pWidget, const xui_code_virtual_text_t* pVirtualText);`
- 实现: src/xui_code_edit.c:6820（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1639; test_xui/xui_code_edit_test.c:1674

## xuiCodeEditGetVirtualText
- 位置: xui.h:7158  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetVirtualText(xui_widget pWidget, xui_code_virtual_text_t* pVirtualText);`
- 实现: src/xui_code_edit.c:6830（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1644

## xuiCodeEditClearVirtualText
- 位置: xui.h:7159  已注释: 否
- 签名: `XUI_API int xuiCodeEditClearVirtualText(xui_widget pWidget, int iKind);`
- 实现: src/xui_code_edit.c:6846（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditShowCompletion
- 位置: xui.h:7160  已注释: 否
- 签名: `XUI_API int xuiCodeEditShowCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6852（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:382; test_xui/xui_code_edit_test.c:1711; test_xui/xui_code_edit_test.c:1746

## xuiCodeEditCancelCompletion
- 位置: xui.h:7161  已注释: 否
- 签名: `XUI_API int xuiCodeEditCancelCompletion(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6859（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:385; test_xui/xui_code_edit_test.c:1821; test_xui/xui_code_edit_test.c:1850

## xuiCodeEditIsCompletionOpen
- 位置: xui.h:7162  已注释: 否
- 签名: `XUI_API int xuiCodeEditIsCompletionOpen(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6866（体 6 行）
- 用法: examples/xui_codeedit/main.c:383; test_xui/xui_code_edit_test.c:1712; test_xui/xui_code_edit_test.c:1720

## xuiCodeEditGetCompletionCount
- 位置: xui.h:7163  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetCompletionCount(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6873（体 5 行）
- 用法: examples/xui_codeedit/main.c:384; test_xui/xui_code_edit_test.c:1713; test_xui/xui_code_edit_test.c:1721

## xuiCodeEditGetCompletionSelected
- 位置: xui.h:7164  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetCompletionSelected(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6879（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1714; test_xui/xui_code_edit_test.c:1749; test_xui/xui_code_edit_test.c:1752

## xuiCodeEditGetCompletionPopupWidget
- 位置: xui.h:7165  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeEditGetCompletionPopupWidget(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6885（体 5 行）

## xuiCodeEditApplyCompletionItems
- 位置: xui.h:7166  已注释: 否
- 签名: `XUI_API int xuiCodeEditApplyCompletionItems(xui_widget pWidget, uint32_t iDocumentVersion, int iOffset, const xui_code_completion_item_t* pItems, int iItemCount);`
- 实现: src/xui_code_edit.c:6891（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1816; test_xui/xui_code_edit_test.c:1824

## xuiCodeEditSetCompletionOptions
- 位置: xui.h:7168  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetCompletionOptions(xui_widget pWidget, int bAutoShow, int iMinPrefix, int iMaxItems, float fDelay);`
- 实现: src/xui_code_edit.c:6917（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:280; test_xui/xui_code_edit_test.c:1785; test_xui/xui_code_edit_test.c:1832

## xuiCodeEditGetCompletionOptions
- 位置: xui.h:7169  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetCompletionOptions(xui_widget pWidget, int* pAutoShow, int* pMinPrefix, int* pMaxItems, float* pDelay);`
- 实现: src/xui_code_edit.c:6935（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditShowSignatureHelp
- 位置: xui.h:7170  已注释: 否
- 签名: `XUI_API int xuiCodeEditShowSignatureHelp(xui_widget pWidget, const xui_code_signature_help_t* pHelp);`
- 实现: src/xui_code_edit.c:7190（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED

## xuiCodeEditRequestSignatureHelp
- 位置: xui.h:7171  已注释: 否
- 签名: `XUI_API int xuiCodeEditRequestSignatureHelp(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7254（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1908; test_xui/xui_style_code_edit_test.c:73

## xuiCodeEditCloseSignatureHelp
- 位置: xui.h:7172  已注释: 否
- 签名: `XUI_API int xuiCodeEditCloseSignatureHelp(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7260（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditIsSignatureHelpOpen
- 位置: xui.h:7173  已注释: 否
- 签名: `XUI_API int xuiCodeEditIsSignatureHelpOpen(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7268（体 6 行）
- 用法: test_xui/xui_code_edit_test.c:1909; test_xui/xui_code_edit_test.c:1912; test_xui/xui_style_code_edit_test.c:73

## xuiCodeEditShowHint
- 位置: xui.h:7174  已注释: 否
- 签名: `XUI_API int xuiCodeEditShowHint(xui_widget pWidget, const xui_code_hover_t* pHint);`
- 实现: src/xui_code_edit.c:7275（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1919

## xuiCodeEditCloseHint
- 位置: xui.h:7175  已注释: 否
- 签名: `XUI_API int xuiCodeEditCloseHint(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7307（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1923

## xuiCodeEditIsHintOpen
- 位置: xui.h:7176  已注释: 否
- 签名: `XUI_API int xuiCodeEditIsHintOpen(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7315（体 6 行）
- 用法: test_xui/xui_code_edit_test.c:1920; test_xui/xui_code_edit_test.c:1923

## xuiCodeEditGetOffsetRect
- 位置: xui.h:7177  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetOffsetRect(xui_widget pWidget, int iOffset, xui_rect_t* pRect);`
- 实现: src/xui_code_edit.c:6999（体 60 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:775; test_xui/xui_code_edit_test.c:787; test_xui/xui_code_edit_test.c:802

## xuiCodeEditHitTestText
- 位置: xui.h:7182  已注释: 是
- 签名: `XUI_API int xuiCodeEditHitTestText(xui_widget pWidget, float fContextX, float fContextY, xui_code_edit_text_hit_t* pHit);`
- 既有注释: /* Exact text geometry uses UTF-8 byte offsets and XUI context pixel coordinates. * Hit testing and offset geometry share rendering's fold, tab, scroll, and * soft-wrap model. Padding clamps to the nearest insertion point and reports * bInsideText == 0. */
- 实现: src/xui_code_edit.c:7066（体 123 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_word_wrap_test.c:74; test_xui/xui_code_word_wrap_test.c:86; test_xui/xui_code_word_wrap_test.c:96

## xuiCodeEditGetTextOffsetRect
- 位置: xui.h:7184  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetTextOffsetRect(xui_widget pWidget, int iByteOffset, xui_rect_t* pRect);`
- 实现: src/xui_code_edit.c:7060（体 5 行）
- 用法: test_xui/xui_code_word_wrap_test.c:66; test_xui/xui_code_word_wrap_test.c:68; test_xui/xui_code_word_wrap_test.c:80

## xuiCodeEditSetPlaceholders
- 位置: xui.h:7186  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetPlaceholders(xui_widget pWidget, const xui_code_placeholder_t* pPlaceholders, int iCount, int iActiveIndex);`
- 实现: src/xui_code_edit.c:7330（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_code_edit_test.c:1888

## xuiCodeEditClearPlaceholders
- 位置: xui.h:7187  已注释: 否
- 签名: `XUI_API int xuiCodeEditClearPlaceholders(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7362（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1903

## xuiCodeEditMovePlaceholder
- 位置: xui.h:7188  已注释: 否
- 签名: `XUI_API int xuiCodeEditMovePlaceholder(xui_widget pWidget, int iDirection);`
- 实现: src/xui_code_edit.c:7373（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1896

## xuiCodeEditGetActivePlaceholder
- 位置: xui.h:7189  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetActivePlaceholder(xui_widget pWidget, xui_code_placeholder_t* pPlaceholder);`
- 实现: src/xui_code_edit.c:7388（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1899

## xuiCodeEditSetFindScope
- 位置: xui.h:7190  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetFindScope(xui_widget pWidget, xui_code_find_scope pScope);`
- 实现: src/xui_code_edit.c:6654（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditGetFindScope
- 位置: xui.h:7191  已注释: 否
- 签名: `XUI_API xui_code_find_scope xuiCodeEditGetFindScope(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:6662（体 5 行）

## xuiCodeEditOpenFind
- 位置: xui.h:7192  已注释: 否
- 签名: `XUI_API int xuiCodeEditOpenFind(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8388（体 4 行）
- 用法: examples/xui_codeedit/main.c:373; test_xui/xui_code_edit_test.c:1064; test_xui/xui_code_edit_test.c:1103

## xuiCodeEditOpenReplace
- 位置: xui.h:7193  已注释: 否
- 签名: `XUI_API int xuiCodeEditOpenReplace(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8393（体 4 行）

## xuiCodeEditGetFindWindow
- 位置: xui.h:7194  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeEditGetFindWindow(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8398（体 5 行）
- 用法: examples/xui_codeedit/main.c:374; test_xui/xui_code_edit_test.c:1038; test_xui/xui_code_edit_test.c:1055

## xuiCodeEditFindAll
- 位置: xui.h:7195  已注释: 否
- 签名: `XUI_API int xuiCodeEditFindAll(xui_widget pWidget, const xui_find_options_t* pOptions, int* pResultCount);`
- 实现: src/xui_code_edit.c:7398（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:305; examples/xui_codeedit/main.c:367; test_xui/xui_code_edit_test.c:965

## xuiCodeEditFindNext
- 位置: xui.h:7196  已注释: 否
- 签名: `XUI_API int xuiCodeEditFindNext(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_code_edit.c:7421（体 4 行）
- 用法: examples/xui_codeedit/main.c:368; test_xui/xui_code_edit_test.c:969; test_xui/xui_code_edit_test.c:973

## xuiCodeEditFindPrevious
- 位置: xui.h:7197  已注释: 否
- 签名: `XUI_API int xuiCodeEditFindPrevious(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_code_edit.c:7426（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:977

## xuiCodeEditReplaceCurrent
- 位置: xui.h:7198  已注释: 否
- 签名: `XUI_API int xuiCodeEditReplaceCurrent(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_code_edit.c:7431（体 73 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1007

## xuiCodeEditReplaceAll
- 位置: xui.h:7199  已注释: 否
- 签名: `XUI_API int xuiCodeEditReplaceAll(xui_widget pWidget, const xui_find_options_t* pOptions, int* pReplaceCount);`
- 实现: src/xui_code_edit.c:7505（体 52 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:1009; test_xui/xui_code_edit_test.c:1019

## xuiCodeEditClearFind
- 位置: xui.h:7200  已注释: 否
- 签名: `XUI_API int xuiCodeEditClearFind(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7558（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditGetFindResultCount
- 位置: xui.h:7201  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetFindResultCount(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:7566（体 5 行）
- 用法: examples/xui_codeedit/main.c:401; test_xui/xui_code_edit_test.c:966

## xuiCodeEditGetFindResult
- 位置: xui.h:7202  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetFindResult(xui_widget pWidget, int iIndex, xui_code_find_result_t* pResult);`
- 实现: src/xui_code_edit.c:7572（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:981

## xuiCodeEditReplaceAllPlain
- 位置: xui.h:7203  已注释: 否
- 签名: `XUI_API int xuiCodeEditReplaceAllPlain(xui_widget pWidget, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount);`
- 实现: src/xui_code_edit.c:8431（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:943

## xuiCodeEditReplaceAllRegex
- 位置: xui.h:7204  已注释: 否
- 签名: `XUI_API int xuiCodeEditReplaceAllRegex(xui_widget pWidget, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount);`
- 实现: src/xui_code_edit.c:8449（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_edit_test.c:951

## xuiCodeEditSetReadonly
- 位置: xui.h:7205  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_code_edit.c:8470（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_large_perf_test.c:251; test_xui/xui_code_edit_test.c:725; test_xui/xui_code_edit_test.c:1143

## xuiCodeEditIsReadonly
- 位置: xui.h:7206  已注释: 否
- 签名: `XUI_API int xuiCodeEditIsReadonly(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8478（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:686; test_xui/xui_code_edit_test.c:726

## xuiCodeEditSetDefaultMargins
- 位置: xui.h:7207  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetDefaultMargins(xui_widget pWidget, int bShowLineNumbers, int bShowMarkerMargin, int bShowFoldMargin, int bShowDiagnosticMargin);`
- 实现: src/xui_code_edit.c:6093（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditGetDefaultMargins
- 位置: xui.h:7208  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetDefaultMargins(xui_widget pWidget, int* pShowLineNumbers, int* pShowMarkerMargin, int* pShowFoldMargin, int* pShowDiagnosticMargin);`
- 实现: src/xui_code_edit.c:6108（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditGetScrollModel
- 位置: xui.h:7209  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiCodeEditGetScrollModel(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8484（体 7 行）
- 返回码: NULL
- 用法: examples/xui_codeedit/main.c:390; test_xui/xui_code_edit_test.c:1254; test_xui/xui_code_edit_test.c:1629

## xuiCodeEditGetHScrollBarWidget
- 位置: xui.h:7210  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeEditGetHScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8492（体 7 行）
- 返回码: NULL
- 用法: examples/xui_codeedit/main.c:391; test_xui/xui_code_edit_test.c:694; test_xui/xui_code_edit_test.c:1258

## xuiCodeEditGetVScrollBarWidget
- 位置: xui.h:7211  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeEditGetVScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8500（体 7 行）
- 返回码: NULL
- 用法: examples/xui_codeedit/main.c:392; test_xui/xui_code_edit_test.c:695; test_xui/xui_code_edit_test.c:1259

## xuiCodeEditSetScroll
- 位置: xui.h:7212  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetScroll(xui_widget pWidget, float fScrollX, float fScrollY);`
- 实现: src/xui_code_edit.c:8508（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:358; test_xui/xui_code_edit_large_perf_test.c:225; test_xui/xui_code_edit_large_perf_test.c:296

## xuiCodeEditGetScroll
- 位置: xui.h:7213  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetScroll(xui_widget pWidget, float* pScrollX, float* pScrollY);`
- 实现: src/xui_code_edit.c:8524（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:389; test_xui/xui_code_edit_test.c:1252; test_xui/xui_code_edit_test.c:1280

## xuiCodeEditEnsureCaretVisible
- 位置: xui.h:7214  已注释: 否
- 签名: `XUI_API int xuiCodeEditEnsureCaretVisible(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8535（体 73 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1352

## xuiCodeEditSetDisplayOptions
- 位置: xui.h:7215  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetDisplayOptions(xui_widget pWidget, uint32_t iOptions);`
- 实现: src/xui_code_edit.c:8609（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1320; test_xui/xui_code_edit_test.c:1344

## xuiCodeEditGetDisplayOptions
- 位置: xui.h:7216  已注释: 否
- 签名: `XUI_API uint32_t xuiCodeEditGetDisplayOptions(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8623（体 5 行）
- 用法: examples/xui_codeedit/main.c:398; test_xui/xui_code_edit_test.c:687; test_xui/xui_code_edit_test.c:1321

## xuiCodeEditSetMinimap
- 位置: xui.h:7217  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetMinimap(xui_widget pWidget, int bVisible, float fWidth);`
- 实现: src/xui_code_edit.c:8629（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:281; test_xui/xui_code_edit_test.c:1931

## xuiCodeEditGetMinimap
- 位置: xui.h:7218  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetMinimap(xui_widget pWidget, int* pVisible, float* pWidth);`
- 实现: src/xui_code_edit.c:8646（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditGetMinimapRect
- 位置: xui.h:7219  已注释: 否
- 签名: `XUI_API xui_rect_t xuiCodeEditGetMinimapRect(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8656（体 7 行）
- 用法: examples/xui_codeedit/main.c:399; test_xui/xui_code_edit_test.c:1935

## xuiCodeEditSetWordWrap
- 位置: xui.h:7222  已注释: 是
- 签名: `XUI_API int xuiCodeEditSetWordWrap(xui_widget pWidget, int bWordWrap);`
- 既有注释: /* TextEdit, CodeEdit, and RichEdit use the same runtime WordWrap convention: * nonzero enables soft visual lines without modifying document text. */
- 实现: src/xui_code_edit.c:8664（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeEditGetWordWrap
- 位置: xui.h:7223  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetWordWrap(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8676（体 5 行）
- 用法: test_xui/xui_code_word_wrap_test.c:63

## xuiCodeEditSetEolMode
- 位置: xui.h:7224  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetEolMode(xui_widget pWidget, int iEolMode);`
- 实现: src/xui_code_edit.c:8682（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeEditGetEolMode
- 位置: xui.h:7225  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetEolMode(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8692（体 5 行）

## xuiCodeEditSetTabColumns
- 位置: xui.h:7226  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetTabColumns(xui_widget pWidget, int iTabColumns);`
- 实现: src/xui_code_edit.c:8698（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:801; test_xui/xui_code_edit_test.c:804; test_xui/xui_code_edit_test.c:1366

## xuiCodeEditGetTabColumns
- 位置: xui.h:7227  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetTabColumns(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8711（体 7 行）
- 用法: test_xui/xui_code_edit_test.c:1367; test_xui/xui_code_edit_test.c:1381

## xuiCodeEditSetIndentColumns
- 位置: xui.h:7228  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetIndentColumns(xui_widget pWidget, int iIndentColumns);`
- 实现: src/xui_code_edit.c:8719（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:901

## xuiCodeEditGetIndentColumns
- 位置: xui.h:7229  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetIndentColumns(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8729（体 7 行）
- 用法: test_xui/xui_code_edit_test.c:902

## xuiCodeEditSetExpandTabs
- 位置: xui.h:7230  已注释: 否
- 签名: `XUI_API int xuiCodeEditSetExpandTabs(xui_widget pWidget, int bExpandTabs);`
- 实现: src/xui_code_edit.c:8737（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:903; test_xui/xui_code_edit_test.c:913

## xuiCodeEditGetExpandTabs
- 位置: xui.h:7231  已注释: 否
- 签名: `XUI_API int xuiCodeEditGetExpandTabs(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8752（体 9 行）
- 用法: test_xui/xui_code_edit_test.c:904; test_xui/xui_code_edit_test.c:914

## xuiCodeEditOpenMenu
- 位置: xui.h:7232  已注释: 否
- 签名: `XUI_API int xuiCodeEditOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_code_edit.c:8762（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_edit_test.c:1218; test_xui/xui_code_edit_test.c:1235; test_xui/xui_code_edit_test.c:1576

## xuiCodeEditGetLastError
- 位置: xui.h:7233  已注释: 否
- 签名: `XUI_API const char* xuiCodeEditGetLastError(xui_widget pWidget);`
- 实现: src/xui_code_edit.c:8780（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:732; test_xui/xui_code_edit_test.c:734; test_xui/xui_code_edit_test.c:1616

