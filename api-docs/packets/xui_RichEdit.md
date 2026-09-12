# 草稿包：xui.h / RichEdit（60 条 API）

> 生成 2026-09-10 02:55 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiRichEditGetType
- 位置: xui.h:7499  已注释: 否
- 签名: `XUI_API xui_widget_type xuiRichEditGetType(xui_context pContext);`
- 实现: src/xui_rich_edit.c:3744（体 6 行）
- 用法: test_xui/xui_style_rich_edit_test.c:116; test_xui/xui_style_rich_edit_test.c:119

## xuiRichEditCreate
- 位置: xui.h:7500  已注释: 否
- 签名: `XUI_API int xuiRichEditCreate(xui_context pContext, xui_widget* ppWidget, const xui_rich_edit_desc_t* pDesc);`
- 实现: src/xui_rich_edit.c:3751（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:256; test_xui/xui_edit_contract_test.c:142; test_xui/xui_rich_edit_large_perf_test.c:89

## xuiRichEditGetDocument
- 位置: xui.h:7501  已注释: 否
- 签名: `XUI_API xui_rich_document xuiRichEditGetDocument(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:3761（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:221; test_xui/xui_rich_edit_test.c:222; test_xui/xui_rich_edit_test.c:223

## xuiRichEditSetDocument
- 位置: xui.h:7502  已注释: 否
- 签名: `XUI_API int xuiRichEditSetDocument(xui_widget pWidget, xui_rich_document pDocument, int bOwnDocument);`
- 实现: src/xui_rich_edit.c:3763（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichEditSetChange
- 位置: xui.h:7503  已注释: 否
- 签名: `XUI_API int xuiRichEditSetChange(xui_widget pWidget, xui_rich_edit_change_proc onChange, void* pUser);`
- 实现: src/xui_rich_edit.c:3780（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED

## xuiRichEditSetLinkClick
- 位置: xui.h:7504  已注释: 否
- 签名: `XUI_API int xuiRichEditSetLinkClick(xui_widget pWidget, xui_rich_edit_link_proc onClick, void* pUser);`
- 实现: src/xui_rich_edit.c:3781（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:261; test_xui/xui_rich_edit_test.c:302

## xuiRichEditSetEvent
- 位置: xui.h:7505  已注释: 否
- 签名: `XUI_API int xuiRichEditSetEvent(xui_widget pWidget, xui_rich_edit_event_proc onEvent, void* pUser);`
- 实现: src/xui_rich_edit.c:3782（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:262; test_xui/xui_rich_edit_test.c:185

## xuiRichEditSetSelection
- 位置: xui.h:7506  已注释: 否
- 签名: `XUI_API int xuiRichEditSetSelection(xui_widget pWidget, int iStart, int iEnd);`
- 实现: src/xui_rich_edit.c:3783（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_large_perf_test.c:101; test_xui/xui_rich_edit_large_perf_test.c:105; test_xui/xui_rich_edit_large_perf_test.c:111

## xuiRichEditGetSelection
- 位置: xui.h:7507  已注释: 否
- 签名: `XUI_API int xuiRichEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd);`
- 实现: src/xui_rich_edit.c:3784（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_test.c:274; test_xui/xui_rich_edit_test.c:293; test_xui/xui_rich_edit_test.c:300

## xuiRichEditSelectAll
- 位置: xui.h:7508  已注释: 否
- 签名: `XUI_API int xuiRichEditSelectAll(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:3785（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED

## xuiRichEditInsertText
- 位置: xui.h:7509  已注释: 否
- 签名: `XUI_API int xuiRichEditInsertText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_rich_edit.c:3786（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_large_perf_test.c:103; test_xui/xui_rich_edit_large_perf_test.c:107; test_xui/xui_rich_edit_large_perf_test.c:114

## xuiRichEditApplyStyle
- 位置: xui.h:7510  已注释: 否
- 签名: `XUI_API int xuiRichEditApplyStyle(xui_widget pWidget, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_edit.c:3787（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:74; test_xui/xui_rich_edit_test.c:331

## xuiRichEditApplyLink
- 位置: xui.h:7511  已注释: 否
- 签名: `XUI_API int xuiRichEditApplyLink(xui_widget pWidget, const char* sUrl);`
- 实现: src/xui_rich_edit.c:3788（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:120

## xuiRichEditApplyParagraphStyle
- 位置: xui.h:7512  已注释: 否
- 签名: `XUI_API int xuiRichEditApplyParagraphStyle(xui_widget pWidget, const xui_rich_paragraph_style_t* pStyle);`
- 实现: src/xui_rich_edit.c:3797（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_test.c:478

## xuiRichEditInsertImage
- 位置: xui.h:7513  已注释: 否
- 签名: `XUI_API int xuiRichEditInsertImage(xui_widget pWidget, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_edit.c:3817（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:126; test_xui/xui_rich_edit_test.c:510

## xuiRichEditInsertInlineImage
- 位置: xui.h:7514  已注释: 否
- 签名: `XUI_API int xuiRichEditInsertInlineImage(xui_widget pWidget, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_edit.c:3826（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_test.c:492

## xuiRichEditInsertTable
- 位置: xui.h:7515  已注释: 否
- 签名: `XUI_API int xuiRichEditInsertTable(xui_widget pWidget, const xui_rich_table_desc_t* pDesc);`
- 实现: src/xui_rich_edit.c:3836（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:133; test_xui/xui_rich_edit_test.c:512

## xuiRichEditInsertHorizontalRule
- 位置: xui.h:7516  已注释: 否
- 签名: `XUI_API int xuiRichEditInsertHorizontalRule(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:3845（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: examples/xui_richedit/main.c:135; test_xui/xui_rich_edit_test.c:513

## xuiRichEditBeginTableCellEdit
- 位置: xui.h:7517  已注释: 否
- 签名: `XUI_API int xuiRichEditBeginTableCellEdit(xui_widget pWidget, xui_document_node_id_t iTableId, int iRow, int iColumn);`
- 实现: src/xui_rich_edit.c:3854（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_edit_test.c:611

## xuiRichEditEndTableCellEdit
- 位置: xui.h:7519  已注释: 否
- 签名: `XUI_API int xuiRichEditEndTableCellEdit(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:3902（体 5 行）
- 用法: test_xui/xui_rich_edit_test.c:603; test_xui/xui_rich_edit_test.c:613

## xuiRichEditGetTableCellEditor
- 位置: xui.h:7520  已注释: 否
- 签名: `XUI_API xui_widget xuiRichEditGetTableCellEditor(xui_widget pWidget, xui_document_node_id_t* pTableId, int* pRow, int* pColumn);`
- 实现: src/xui_rich_edit.c:3908（体 10 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_edit_test.c:596; test_xui/xui_rich_edit_test.c:603; test_xui/xui_rich_edit_test.c:612

## xuiRichEditSetFontSet
- 位置: xui.h:7522  已注释: 否
- 签名: `XUI_API int xuiRichEditSetFontSet(xui_widget pWidget, const xui_rich_font_set_t* pFonts);`
- 实现: src/xui_rich_edit.c:3919（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:260; test_xui/xui_rich_edit_test.c:374

## xuiRichEditGetFontSet
- 位置: xui.h:7523  已注释: 否
- 签名: `XUI_API int xuiRichEditGetFontSet(xui_widget pWidget, xui_rich_font_set_t* pFonts);`
- 实现: src/xui_rich_edit.c:3930（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:375

## xuiRichEditExecuteCommand
- 位置: xui.h:7524  已注释: 否
- 签名: `XUI_API int xuiRichEditExecuteCommand(xui_widget pWidget, int iCommand, const void* pData);`
- 实现: src/xui_rich_edit.c:4033（体 66 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: examples/xui_richedit/main.c:137; test_xui/xui_rich_edit_test.c:377; test_xui/xui_rich_edit_test.c:379

## xuiRichEditQueryCommand
- 位置: xui.h:7525  已注释: 否
- 签名: `XUI_API int xuiRichEditQueryCommand(xui_widget pWidget, int iCommand, int* pState);`
- 实现: src/xui_rich_edit.c:3948（体 84 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:324; test_xui/xui_rich_edit_test.c:378; test_xui/xui_rich_edit_test.c:380

## xuiRichEditSetupToolbar
- 位置: xui.h:7526  已注释: 否
- 签名: `XUI_API int xuiRichEditSetupToolbar(xui_widget pWidget, xui_widget pToolbar, uint32_t iGroups);`
- 实现: src/xui_rich_edit.c:4133（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:158; test_xui/xui_rich_edit_test.c:383

## xuiRichEditSyncToolbar
- 位置: xui.h:7527  已注释: 否
- 签名: `XUI_API int xuiRichEditSyncToolbar(xui_widget pWidget, xui_widget pToolbar);`
- 实现: src/xui_rich_edit.c:4162（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_richedit/main.c:91; test_xui/xui_rich_edit_test.c:405; test_xui/xui_rich_edit_test.c:407

## xuiRichEditExecuteToolbarItem
- 位置: xui.h:7528  已注释: 否
- 签名: `XUI_API int xuiRichEditExecuteToolbarItem(xui_widget pWidget, xui_widget pToolbar, int iIndex);`
- 实现: src/xui_rich_edit.c:4189（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_richedit/main.c:110; test_xui/xui_rich_edit_test.c:392; test_xui/xui_rich_edit_test.c:395

## xuiRichEditCopy
- 位置: xui.h:7529  已注释: 否
- 签名: `XUI_API int xuiRichEditCopy(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4200（体 46 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_rich_edit_scale_test.c:232; test_xui/xui_rich_edit_test.c:479

## xuiRichEditCut
- 位置: xui.h:7530  已注释: 否
- 签名: `XUI_API int xuiRichEditCut(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4201（体 45 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK

## xuiRichEditPaste
- 位置: xui.h:7531  已注释: 否
- 签名: `XUI_API int xuiRichEditPaste(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4336（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_test.c:480; test_xui/xui_rich_edit_test.c:485

## xuiRichEditOpenFind
- 位置: xui.h:7532  已注释: 否
- 签名: `XUI_API int xuiRichEditOpenFind(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4772（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRichEditOpenReplace
- 位置: xui.h:7533  已注释: 否
- 签名: `XUI_API int xuiRichEditOpenReplace(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4773（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRichEditGetFindWindow
- 位置: xui.h:7534  已注释: 否
- 签名: `XUI_API xui_widget xuiRichEditGetFindWindow(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4774（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_edit_test.c:684; test_xui/xui_rich_edit_test.c:687

## xuiRichEditFindNext
- 位置: xui.h:7535  已注释: 否
- 签名: `XUI_API int xuiRichEditFindNext(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_rich_edit.c:4368（体 4 行）
- 用法: test_xui/xui_rich_edit_test.c:662; test_xui/xui_style_rich_edit_test.c:106

## xuiRichEditFindPrevious
- 位置: xui.h:7536  已注释: 否
- 签名: `XUI_API int xuiRichEditFindPrevious(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_rich_edit.c:4373（体 4 行）

## xuiRichEditReplaceCurrent
- 位置: xui.h:7537  已注释: 否
- 签名: `XUI_API int xuiRichEditReplaceCurrent(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_rich_edit.c:4428（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_rich_edit_test.c:666

## xuiRichEditReplaceAll
- 位置: xui.h:7538  已注释: 否
- 签名: `XUI_API int xuiRichEditReplaceAll(xui_widget pWidget, const xui_find_options_t* pOptions, int* pReplaceCount);`
- 实现: src/xui_rich_edit.c:4467（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_edit_test.c:675

## xuiRichEditClearFind
- 位置: xui.h:7539  已注释: 否
- 签名: `XUI_API int xuiRichEditClearFind(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4496（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRichEditGetFindResultCount
- 位置: xui.h:7540  已注释: 否
- 签名: `XUI_API int xuiRichEditGetFindResultCount(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4504（体 5 行）
- 用法: test_xui/xui_rich_edit_test.c:663

## xuiRichEditGetFindResult
- 位置: xui.h:7541  已注释: 否
- 签名: `XUI_API int xuiRichEditGetFindResult(xui_widget pWidget, int iIndex, xui_find_result_t* pResult);`
- 实现: src/xui_rich_edit.c:4510（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:664

## xuiRichEditOpenMenu
- 位置: xui.h:7542  已注释: 否
- 签名: `XUI_API int xuiRichEditOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_rich_edit.c:4776（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiRichEditGetMenuWidget
- 位置: xui.h:7543  已注释: 否
- 签名: `XUI_API xui_widget xuiRichEditGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4786（体 5 行）
- 用法: test_xui/xui_rich_edit_test.c:186

## xuiRichEditUndo
- 位置: xui.h:7544  已注释: 否
- 签名: `XUI_API int xuiRichEditUndo(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4812（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_large_perf_test.c:109; test_xui/xui_rich_edit_scale_test.c:246; test_xui/xui_rich_edit_test.c:243

## xuiRichEditRedo
- 位置: xui.h:7545  已注释: 否
- 签名: `XUI_API int xuiRichEditRedo(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4813（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_large_perf_test.c:110; test_xui/xui_rich_edit_scale_test.c:247; test_xui/xui_rich_edit_test.c:339

## xuiRichEditSetReadonly
- 位置: xui.h:7546  已注释: 否
- 签名: `XUI_API int xuiRichEditSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_rich_edit.c:4814（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:396; test_xui/xui_rich_edit_test.c:407

## xuiRichEditIsReadonly
- 位置: xui.h:7547  已注释: 否
- 签名: `XUI_API int xuiRichEditIsReadonly(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4825（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichEditSetWordWrap
- 位置: xui.h:7548  已注释: 否
- 签名: `XUI_API int xuiRichEditSetWordWrap(xui_widget pWidget, int bWordWrap);`
- 实现: src/xui_rich_edit.c:4826（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:181; test_xui/xui_rich_edit_test.c:183

## xuiRichEditGetWordWrap
- 位置: xui.h:7549  已注释: 否
- 签名: `XUI_API int xuiRichEditGetWordWrap(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4838（体 5 行）
- 用法: test_xui/xui_rich_edit_test.c:180; test_xui/xui_rich_edit_test.c:181; test_xui/xui_rich_edit_test.c:183

## xuiRichEditSetScroll
- 位置: xui.h:7550  已注释: 否
- 签名: `XUI_API int xuiRichEditSetScroll(xui_widget pWidget, float fScrollX, float fScrollY);`
- 实现: src/xui_rich_edit.c:4843（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, NULL
- 用法: test_xui/xui_rich_edit_scale_test.c:201; test_xui/xui_rich_edit_scale_test.c:263; test_xui/xui_rich_edit_scale_test.c:265

## xuiRichEditGetScroll
- 位置: xui.h:7551  已注释: 否
- 签名: `XUI_API int xuiRichEditGetScroll(xui_widget pWidget, float* pScrollX, float* pScrollY);`
- 实现: src/xui_rich_edit.c:4844（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, NULL
- 用法: test_xui/xui_rich_edit_test.c:579; test_xui/xui_rich_edit_test.c:590; test_xui/xui_rich_edit_test.c:619

## xuiRichEditScrollBy
- 位置: xui.h:7552  已注释: 否
- 签名: `XUI_API int xuiRichEditScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY);`
- 实现: src/xui_rich_edit.c:4845（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, NULL

## xuiRichEditSetZoom
- 位置: xui.h:7553  已注释: 否
- 签名: `XUI_API int xuiRichEditSetZoom(xui_widget pWidget, float fZoom);`
- 实现: src/xui_rich_edit.c:4846（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, NULL
- 用法: test_xui/xui_rich_edit_test.c:195; test_xui/xui_rich_edit_test.c:201

## xuiRichEditGetZoom
- 位置: xui.h:7554  已注释: 否
- 签名: `XUI_API float xuiRichEditGetZoom(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4847（体 7 行）
- 返回码: NULL, XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:195

## xuiRichEditGetScrollModel
- 位置: xui.h:7555  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiRichEditGetScrollModel(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4848（体 6 行）
- 返回码: NULL, XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:568

## xuiRichEditGetHScrollBarWidget
- 位置: xui.h:7556  已注释: 否
- 签名: `XUI_API xui_widget xuiRichEditGetHScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4849（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:569; test_xui/xui_rich_edit_test.c:571

## xuiRichEditGetVScrollBarWidget
- 位置: xui.h:7557  已注释: 否
- 签名: `XUI_API xui_widget xuiRichEditGetVScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4850（体 4 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:570; test_xui/xui_rich_edit_test.c:572; test_xui/xui_rich_edit_test.c:574

## xuiRichEditGetFragmentCount
- 位置: xui.h:7558  已注释: 否
- 签名: `XUI_API int xuiRichEditGetFragmentCount(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4851（体 3 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_scale_test.c:196; test_xui/xui_rich_edit_test.c:191; test_xui/xui_rich_edit_test.c:206

## xuiRichEditGetFragment
- 位置: xui.h:7559  已注释: 否
- 签名: `XUI_API int xuiRichEditGetFragment(xui_widget pWidget, int iIndex, xui_rich_fragment_t* pFragment);`
- 实现: src/xui_rich_edit.c:4852（体 2 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_test.c:192; test_xui/xui_rich_edit_test.c:197; test_xui/xui_rich_edit_test.c:204

## xuiRichEditGetCursorRect
- 位置: xui.h:7560  已注释: 否
- 签名: `XUI_API xui_rect_t xuiRichEditGetCursorRect(xui_widget pWidget);`
- 实现: src/xui_rich_edit.c:4853（体 1 行）

