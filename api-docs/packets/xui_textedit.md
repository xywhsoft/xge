# 草稿包：xui.h / textedit（61 条 API）

> 生成 2026-09-10 03:07 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTextEditGetType
- 位置: xui.h:7805  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTextEditGetType(xui_context pContext);`
- 实现: src/xui_text_edit.c:3158（体 36 行）
- 返回码: NULL

## xuiTextEditCreate
- 位置: xui.h:7806  已注释: 否
- 签名: `XUI_API int xuiTextEditCreate(xui_context pContext, xui_widget* ppWidget, const xui_text_edit_desc_t* pDesc);`
- 实现: src/xui_text_edit.c:3195（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/audit_xui_text/main.c:24; examples/audit_xui_text/main.c:26; examples/tutorial_capture/ch140_main1.c:16

## xuiTextEditSetChange
- 位置: xui.h:7807  已注释: 否
- 签名: `XUI_API int xuiTextEditSetChange(xui_widget pWidget, xui_text_edit_change_proc onChange, void* pUser);`
- 实现: src/xui_text_edit.c:3210（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_textedit/main.c:202; examples/xui_textedit/main.c:225; test_xui/xui_accessibility_test.c:243

## xuiTextEditSetText
- 位置: xui.h:7808  已注释: 否
- 签名: `XUI_API int xuiTextEditSetText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_text_edit.c:3219（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_accessibility_test.c:242; test_xui/xui_grid_focus_test.c:355; test_xui/xui_text_edit_lifetime_test.c:127

## xuiTextEditGetText
- 位置: xui.h:7809  已注释: 否
- 签名: `XUI_API const char* xuiTextEditGetText(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3228（体 5 行）
- 用法: examples/xui_textedit/main.c:322; examples/xui_textedit/main.c:335; examples/xui_textedit/main.c:336

## xuiTextEditSetPlaceholder
- 位置: xui.h:7810  已注释: 否
- 签名: `XUI_API int xuiTextEditSetPlaceholder(xui_widget pWidget, const char* sText);`
- 实现: src/xui_text_edit.c:3234（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditGetPlaceholder
- 位置: xui.h:7811  已注释: 否
- 签名: `XUI_API const char* xuiTextEditGetPlaceholder(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3245（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:296

## xuiTextEditSetFont
- 位置: xui.h:7812  已注释: 否
- 签名: `XUI_API int xuiTextEditSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_text_edit.c:3251（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_display_test.c:191; test_xui/xui_text_edit_display_test.c:197

## xuiTextEditGetFont
- 位置: xui.h:7813  已注释: 否
- 签名: `XUI_API xui_font xuiTextEditGetFont(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3262（体 5 行）

## xuiTextEditSetMaxLength
- 位置: xui.h:7814  已注释: 否
- 签名: `XUI_API int xuiTextEditSetMaxLength(xui_widget pWidget, int iMaxLength);`
- 实现: src/xui_text_edit.c:3268（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:482; test_xui/xui_text_edit_test.c:492

## xuiTextEditGetMaxLength
- 位置: xui.h:7815  已注释: 否
- 签名: `XUI_API int xuiTextEditGetMaxLength(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3280（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:297

## xuiTextEditSetReadonly
- 位置: xui.h:7816  已注释: 否
- 签名: `XUI_API int xuiTextEditSetReadonly(xui_widget pWidget, int bReadonly);`
- 实现: src/xui_text_edit.c:3286（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:760; test_xui/xui_text_edit_test.c:773

## xuiTextEditIsReadonly
- 位置: xui.h:7817  已注释: 否
- 签名: `XUI_API int xuiTextEditIsReadonly(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3297（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:761

## xuiTextEditSetWordWrap
- 位置: xui.h:7818  已注释: 否
- 签名: `XUI_API int xuiTextEditSetWordWrap(xui_widget pWidget, int bWordWrap);`
- 实现: src/xui_text_edit.c:3303（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_lifetime_test.c:126; test_xui/xui_text_edit_test.c:517; test_xui/xui_text_edit_test.c:552

## xuiTextEditGetWordWrap
- 位置: xui.h:7819  已注释: 否
- 签名: `XUI_API int xuiTextEditGetWordWrap(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3313（体 5 行）
- 用法: examples/xui_textedit/main.c:292; examples/xui_textedit/main.c:296; test_xui/xui_text_edit_test.c:299

## xuiTextEditSetLineNumbers
- 位置: xui.h:7820  已注释: 否
- 签名: `XUI_API int xuiTextEditSetLineNumbers(xui_widget pWidget, int bVisible, float fWidth);`
- 实现: src/xui_text_edit.c:3319（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:709

## xuiTextEditGetLineNumbers
- 位置: xui.h:7821  已注释: 否
- 签名: `XUI_API int xuiTextEditGetLineNumbers(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3332（体 5 行）
- 用法: examples/xui_textedit/main.c:293; examples/xui_textedit/main.c:297; test_xui/xui_text_edit_test.c:300

## xuiTextEditGetLineNumberWidth
- 位置: xui.h:7822  已注释: 否
- 签名: `XUI_API float xuiTextEditGetLineNumberWidth(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3338（体 5 行）
- 用法: examples/xui_textedit/main.c:294; test_xui/xui_text_edit_test.c:301

## xuiTextEditSetLineNumberColors
- 位置: xui.h:7823  已注释: 否
- 签名: `XUI_API int xuiTextEditSetLineNumberColors(xui_widget pWidget, uint32_t iText, uint32_t iBackground, uint32_t iBorder);`
- 实现: src/xui_text_edit.c:3344（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditSetSelection
- 位置: xui.h:7824  已注释: 否
- 签名: `XUI_API int xuiTextEditSetSelection(xui_widget pWidget, int iStart, int iEnd);`
- 实现: src/xui_text_edit.c:3525（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_textedit/main.c:322; test_xui/xui_text_edit_display_test.c:119; test_xui/xui_text_edit_display_test.c:123

## xuiTextEditGetSelection
- 位置: xui.h:7825  已注释: 否
- 签名: `XUI_API int xuiTextEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd);`
- 实现: src/xui_text_edit.c:3543（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_textedit/main.c:316; test_xui/xui_text_edit_display_test.c:112; test_xui/xui_text_edit_display_test.c:181

## xuiTextEditSelectAll
- 位置: xui.h:7826  已注释: 否
- 签名: `XUI_API int xuiTextEditSelectAll(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3551（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:426; test_xui/xui_text_edit_test.c:486; test_xui/xui_text_edit_test.c:713

## xuiTextEditHasSelection
- 位置: xui.h:7827  已注释: 否
- 签名: `XUI_API int xuiTextEditHasSelection(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3558（体 4 行）
- 用法: test_xui/xui_text_edit_test.c:315; test_xui/xui_text_edit_test.c:427

## xuiTextEditCopy
- 位置: xui.h:7828  已注释: 否
- 签名: `XUI_API int xuiTextEditCopy(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3563（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_display_test.c:120; test_xui/xui_text_edit_display_test.c:124; test_xui/xui_text_edit_display_test.c:182

## xuiTextEditCut
- 位置: xui.h:7829  已注释: 否
- 签名: `XUI_API int xuiTextEditCut(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3570（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_edit_test.c:430; test_xui/xui_text_edit_test.c:715

## xuiTextEditPaste
- 位置: xui.h:7830  已注释: 否
- 签名: `XUI_API int xuiTextEditPaste(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3579（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:476; test_xui/xui_text_edit_test.c:488

## xuiTextEditDeleteSelection
- 位置: xui.h:7831  已注释: 否
- 签名: `XUI_API int xuiTextEditDeleteSelection(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:3586（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTextEditOpenFind
- 位置: xui.h:7832  已注释: 否
- 签名: `XUI_API int xuiTextEditOpenFind(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4030（体 4 行）
- 用法: examples/xui_language/main.c:266; examples/xui_language/main.c:417; examples/xui_textedit/main.c:318

## xuiTextEditOpenReplace
- 位置: xui.h:7833  已注释: 否
- 签名: `XUI_API int xuiTextEditOpenReplace(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4035（体 4 行）
- 用法: examples/xui_language/main.c:270

## xuiTextEditGetFindWindow
- 位置: xui.h:7834  已注释: 否
- 签名: `XUI_API xui_widget xuiTextEditGetFindWindow(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4040（体 6 行）
- 返回码: NULL
- 用法: examples/xui_language/main.c:418; examples/xui_textedit/main.c:319; test_xui/xui_text_edit_test.c:659

## xuiTextEditFindNext
- 位置: xui.h:7835  已注释: 否
- 签名: `XUI_API int xuiTextEditFindNext(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_text_edit.c:4047（体 4 行）
- 用法: examples/xui_textedit/main.c:313; test_xui/xui_text_edit_test.c:585; test_xui/xui_text_edit_test.c:589

## xuiTextEditFindPrevious
- 位置: xui.h:7836  已注释: 否
- 签名: `XUI_API int xuiTextEditFindPrevious(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_text_edit.c:4052（体 4 行）
- 用法: test_xui/xui_text_edit_test.c:593

## xuiTextEditReplaceCurrent
- 位置: xui.h:7837  已注释: 否
- 签名: `XUI_API int xuiTextEditReplaceCurrent(xui_widget pWidget, const xui_find_options_t* pOptions);`
- 实现: src/xui_text_edit.c:4057（体 56 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_text_edit_test.c:623

## xuiTextEditReplaceAll
- 位置: xui.h:7838  已注释: 否
- 签名: `XUI_API int xuiTextEditReplaceAll(xui_widget pWidget, const xui_find_options_t* pOptions, int* pReplaceCount);`
- 实现: src/xui_text_edit.c:4114（体 39 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_text_edit_test.c:627; test_xui/xui_text_edit_test.c:637

## xuiTextEditClearFind
- 位置: xui.h:7839  已注释: 否
- 签名: `XUI_API int xuiTextEditClearFind(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4154（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:609

## xuiTextEditUndo
- 位置: xui.h:7840  已注释: 否
- 签名: `XUI_API int xuiTextEditUndo(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4184（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_edit_display_test.c:128; test_xui/xui_text_edit_test.c:433; test_xui/xui_text_edit_test.c:460

## xuiTextEditRedo
- 位置: xui.h:7841  已注释: 否
- 签名: `XUI_API int xuiTextEditRedo(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4208（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_edit_test.c:436

## xuiTextEditCanUndo
- 位置: xui.h:7842  已注释: 否
- 签名: `XUI_API int xuiTextEditCanUndo(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4232（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:432

## xuiTextEditCanRedo
- 位置: xui.h:7843  已注释: 否
- 签名: `XUI_API int xuiTextEditCanRedo(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4238（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:435; test_xui/xui_text_edit_test.c:718

## xuiTextEditGetScrollModel
- 位置: xui.h:7844  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiTextEditGetScrollModel(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4244（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_text_edit_test.c:500

## xuiTextEditGetHScrollBarWidget
- 位置: xui.h:7845  已注释: 否
- 签名: `XUI_API xui_widget xuiTextEditGetHScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4252（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_text_edit_lifetime_test.c:53; test_xui/xui_text_edit_test.c:504

## xuiTextEditGetVScrollBarWidget
- 位置: xui.h:7846  已注释: 否
- 签名: `XUI_API xui_widget xuiTextEditGetVScrollBarWidget(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4260（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_text_edit_lifetime_test.c:54; test_xui/xui_text_edit_test.c:505

## xuiTextEditSetScroll
- 位置: xui.h:7847  已注释: 否
- 签名: `XUI_API int xuiTextEditSetScroll(xui_widget pWidget, float fScrollX, float fScrollY);`
- 实现: src/xui_text_edit.c:4268（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_textedit/main.c:324; test_xui/xui_text_edit_lifetime_test.c:129; test_xui/xui_text_edit_test.c:496

## xuiTextEditGetScroll
- 位置: xui.h:7848  已注释: 否
- 签名: `XUI_API int xuiTextEditGetScroll(xui_widget pWidget, float* pScrollX, float* pScrollY);`
- 实现: src/xui_text_edit.c:4283（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_text_edit_test.c:498; test_xui/xui_text_edit_test.c:523; test_xui/xui_text_edit_test.c:546

## xuiTextEditScrollBy
- 位置: xui.h:7849  已注释: 否
- 签名: `XUI_API int xuiTextEditScrollBy(xui_widget pWidget, float fDeltaX, float fDeltaY);`
- 实现: src/xui_text_edit.c:4292（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditSetColors
- 位置: xui.h:7850  已注释: 否
- 签名: `XUI_API int xuiTextEditSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iText, uint32_t iBorder, uint32_t iFocus);`
- 实现: src/xui_text_edit.c:4299（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditGetColors
- 位置: xui.h:7851  已注释: 否
- 签名: `XUI_API int xuiTextEditGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pText, uint32_t* pBorder, uint32_t* pFocus);`
- 实现: src/xui_text_edit.c:4312（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTextEditSetExtendedColors
- 位置: xui.h:7852  已注释: 否
- 签名: `XUI_API int xuiTextEditSetExtendedColors(xui_widget pWidget, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iHoverBackground, uint32_t iDisabledBackground, uint32_t iHoverBorder, uint32_t iSelection, uint32_t iCursor, uint32_t iFindResult, uint32_t iFindActive);`
- 实现: src/xui_text_edit.c:4323（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditGetExtendedColors
- 位置: xui.h:7853  已注释: 否
- 签名: `XUI_API int xuiTextEditGetExtendedColors(xui_widget pWidget, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pHoverBackground, uint32_t* pDisabledBackground, uint32_t* pHoverBorder, uint32_t* pSelection, uint32_t* pCursor, uint32_t* pFindResult, uint32_t* pFindActive);`
- 实现: src/xui_text_edit.c:4339（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTextEditSetVisualMetrics
- 位置: xui.h:7854  已注释: 否
- 签名: `XUI_API int xuiTextEditSetVisualMetrics(xui_widget pWidget, float fBorderWidth, float fLineGap);`
- 实现: src/xui_text_edit.c:4355（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTextEditGetVisualMetrics
- 位置: xui.h:7855  已注释: 否
- 签名: `XUI_API int xuiTextEditGetVisualMetrics(xui_widget pWidget, float* pBorderWidth, float* pLineGap);`
- 实现: src/xui_text_edit.c:4364（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTextEditGetLineNumberColors
- 位置: xui.h:7856  已注释: 否
- 签名: `XUI_API int xuiTextEditGetLineNumberColors(xui_widget pWidget, uint32_t* pText, uint32_t* pBackground, uint32_t* pBorder);`
- 实现: src/xui_text_edit.c:3354（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTextEditSetMenuTitle
- 位置: xui.h:7857  已注释: 否
- 签名: `XUI_API int xuiTextEditSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_text_edit.c:4373（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_textedit/main.c:203; test_xui/xui_text_edit_test.c:704; test_xui/xui_text_edit_test.c:706

## xuiTextEditGetMenuTitle
- 位置: xui.h:7858  已注释: 否
- 签名: `XUI_API const char* xuiTextEditGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_text_edit.c:4394（体 8 行）
- 用法: examples/xui_textedit/main.c:295; test_xui/xui_text_edit_test.c:699; test_xui/xui_text_edit_test.c:701

## xuiTextEditOpenMenu
- 位置: xui.h:7859  已注释: 否
- 签名: `XUI_API int xuiTextEditOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_text_edit.c:4403（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_text_edit_test.c:723; test_xui/xui_text_edit_test.c:763

## xuiTextEditGetMenuWidget
- 位置: xui.h:7860  已注释: 否
- 签名: `XUI_API xui_widget xuiTextEditGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4414（体 5 行）
- 用法: examples/xui_textedit/main.c:327; test_xui/xui_text_edit_test.c:721

## xuiTextEditGetTextRect
- 位置: xui.h:7861  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTextEditGetTextRect(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4420（体 5 行）
- 用法: test_xui/xui_text_edit_test.c:302; test_xui/xui_text_edit_test.c:330; test_xui/xui_text_edit_test.c:364

## xuiTextEditGetCursorRect
- 位置: xui.h:7862  已注释: 否
- 签名: `XUI_API xui_rect_t xuiTextEditGetCursorRect(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4426（体 9 行）

## xuiTextEditGetLineCount
- 位置: xui.h:7863  已注释: 否
- 签名: `XUI_API int xuiTextEditGetLineCount(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4436（体 7 行）
- 用法: examples/xui_textedit/main.c:164; examples/xui_textedit/main.c:290; examples/xui_textedit/main.c:291

## xuiTextEditGetState
- 位置: xui.h:7864  已注释: 否
- 签名: `XUI_API uint32_t xuiTextEditGetState(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4444（体 14 行）
- 用法: test_xui/xui_text_edit_test.c:762

## xuiTextEditGetChangeCount
- 位置: xui.h:7865  已注释: 否
- 签名: `XUI_API int xuiTextEditGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_text_edit.c:4459（体 5 行）

