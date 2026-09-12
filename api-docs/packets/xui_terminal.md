# 草稿包：xui.h / terminal（71 条 API）

> 生成 2026-09-10 03:07 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiTerminalGetType
- 位置: xui.h:8836  已注释: 否
- 签名: `XUI_API xui_widget_type xuiTerminalGetType(xui_context pContext);`
- 实现: src/xui_terminal.c:6003（体 34 行）
- 返回码: NULL

## xuiTerminalCreate
- 位置: xui.h:8837  已注释: 否
- 签名: `XUI_API int xuiTerminalCreate(xui_context pContext, xui_widget* ppWidget, const xui_terminal_desc_t* pDesc);`
- 实现: src/xui_terminal.c:6038（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_NOT_INITIALIZED
- 用法: examples/tutorial_capture/ch178_main1.c:16; examples/tutorial_capture/ch179_main1.c:16; examples/xui_terminal/main.c:195

## xuiTerminalWrite
- 位置: xui.h:8838  已注释: 否
- 签名: `XUI_API int xuiTerminalWrite(xui_widget pWidget, const void* pData, int iSize);`
- 实现: src/xui_terminal.c:6070（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal_cmd_lab/main.c:198; test_xui/xui_terminal_history_memory_test.c:323; test_xui/xui_terminal_large_perf_test.c:108

## xuiTerminalWriteText
- 位置: xui.h:8839  已注释: 否
- 签名: `XUI_API int xuiTerminalWriteText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_terminal.c:6077（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal/main.c:203; examples/xui_terminal/main.c:204; examples/xui_terminal/main.c:205

## xuiTerminalFlush
- 位置: xui.h:8840  已注释: 否
- 签名: `XUI_API int xuiTerminalFlush(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6083（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal/main.c:212; examples/xui_terminal_cmd_lab/main.c:266; examples/xui_terminal_cmd_lab/main.c:380

## xuiTerminalClear
- 位置: xui.h:8841  已注释: 否
- 签名: `XUI_API int xuiTerminalClear(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6091（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_history_memory_test.c:254; test_xui/xui_terminal_history_memory_test.c:265; test_xui/xui_terminal_parser_test.c:109

## xuiTerminalClearScrollback
- 位置: xui.h:8842  已注释: 否
- 签名: `XUI_API int xuiTerminalClearScrollback(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6107（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:634; examples/xui_terminal_cmd_lab/main.c:739; test_xui/xui_terminal_history_memory_test.c:253

## xuiTerminalSetScrollbackLimit
- 位置: xui.h:8843  已注释: 否
- 签名: `XUI_API int xuiTerminalSetScrollbackLimit(xui_widget pWidget, int iLimit);`
- 实现: src/xui_terminal.c:6121（体 58 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY

## xuiTerminalGetScrollbackLimit
- 位置: xui.h:8844  已注释: 否
- 签名: `XUI_API int xuiTerminalGetScrollbackLimit(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6180（体 5 行）

## xuiTerminalSetParseBudget
- 位置: xui.h:8845  已注释: 否
- 签名: `XUI_API int xuiTerminalSetParseBudget(xui_widget pWidget, int iBytesPerUpdate);`
- 实现: src/xui_terminal.c:6186（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:826

## xuiTerminalGetParseBudget
- 位置: xui.h:8846  已注释: 否
- 签名: `XUI_API int xuiTerminalGetParseBudget(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6194（体 5 行）

## xuiTerminalSetFont
- 位置: xui.h:8847  已注释: 否
- 签名: `XUI_API int xuiTerminalSetFont(xui_widget pWidget, xui_font pFont);`
- 实现: src/xui_terminal.c:6200（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTerminalGetFont
- 位置: xui.h:8848  已注释: 否
- 签名: `XUI_API xui_font xuiTerminalGetFont(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6212（体 5 行）

## xuiTerminalSetMetrics
- 位置: xui.h:8849  已注释: 否
- 签名: `XUI_API int xuiTerminalSetMetrics(xui_widget pWidget, float fCellWidth, float fCellHeight, float fPadding);`
- 实现: src/xui_terminal.c:6218（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:335; test_xui/xui_terminal_test.c:340

## xuiTerminalGetMetrics
- 位置: xui.h:8850  已注释: 否
- 签名: `XUI_API int xuiTerminalGetMetrics(xui_widget pWidget, float* pCellWidth, float* pCellHeight, float* pPadding);`
- 实现: src/xui_terminal.c:6232（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:337

## xuiTerminalSetColors
- 位置: xui.h:8851  已注释: 否
- 签名: `XUI_API int xuiTerminalSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iForeground, uint32_t iCursor, uint32_t iSelection, uint32_t iSelectionText, uint32_t iSearchHighlight, uint32_t iFocus, uint32_t iLinkHover);`
- 实现: src/xui_terminal.c:6244（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTerminalGetColors
- 位置: xui.h:8852  已注释: 否
- 签名: `XUI_API int xuiTerminalGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pForeground, uint32_t* pCursor, uint32_t* pSelection, uint32_t* pSelectionText, uint32_t* pSearchHighlight, uint32_t* pFocus, uint32_t* pLinkHover);`
- 实现: src/xui_terminal.c:6262（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTerminalSetLigaturesEnabled
- 位置: xui.h:8853  已注释: 否
- 签名: `XUI_API int xuiTerminalSetLigaturesEnabled(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_terminal.c:6279（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTerminalGetLigaturesEnabled
- 位置: xui.h:8854  已注释: 否
- 签名: `XUI_API int xuiTerminalGetLigaturesEnabled(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6289（体 5 行）

## xuiTerminalFit
- 位置: xui.h:8855  已注释: 否
- 签名: `XUI_API int xuiTerminalFit(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6295（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:309

## xuiTerminalResize
- 位置: xui.h:8856  已注释: 否
- 签名: `XUI_API int xuiTerminalResize(xui_widget pWidget, int iColumns, int iRows);`
- 实现: src/xui_terminal.c:6320（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_history_memory_test.c:245; test_xui/xui_terminal_history_memory_test.c:246; test_xui/xui_terminal_history_memory_test.c:252

## xuiTerminalGetColumns
- 位置: xui.h:8857  已注释: 否
- 签名: `XUI_API int xuiTerminalGetColumns(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6334（体 5 行）
- 用法: examples/xui_terminal/main.c:300; examples/xui_terminal/main.c:335; examples/xui_terminal/main.c:359

## xuiTerminalGetRows
- 位置: xui.h:8858  已注释: 否
- 签名: `XUI_API int xuiTerminalGetRows(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6340（体 5 行）
- 用法: examples/xui_terminal/main.c:301; examples/xui_terminal/main.c:335; examples/xui_terminal/main.c:359

## xuiTerminalGetCursor
- 位置: xui.h:8859  已注释: 否
- 签名: `XUI_API int xuiTerminalGetCursor(xui_widget pWidget, int* pColumn, int* pRow);`
- 实现: src/xui_terminal.c:6346（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_parser_test.c:106; test_xui/xui_terminal_test.c:360; test_xui/xui_terminal_test.c:480

## xuiTerminalSetInputCallback
- 位置: xui.h:8860  已注释: 否
- 签名: `XUI_API int xuiTerminalSetInputCallback(xui_widget pWidget, xui_terminal_data_proc onData, void* pUser);`
- 实现: src/xui_terminal.c:6355（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal/main.c:198; examples/xui_terminal_cmd_lab/main.c:824; test_xui/xui_terminal_parser_test.c:98

## xuiTerminalSetResizeCallback
- 位置: xui.h:8861  已注释: 否
- 签名: `XUI_API int xuiTerminalSetResizeCallback(xui_widget pWidget, xui_terminal_resize_proc onResize, void* pUser);`
- 实现: src/xui_terminal.c:6364（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal/main.c:199; examples/xui_terminal_cmd_lab/main.c:825; test_xui/xui_terminal_test.c:294

## xuiTerminalSetTitleCallback
- 位置: xui.h:8862  已注释: 否
- 签名: `XUI_API int xuiTerminalSetTitleCallback(xui_widget pWidget, xui_terminal_title_proc onTitle, void* pUser);`
- 实现: src/xui_terminal.c:6373（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_parser_test.c:97; test_xui/xui_terminal_test.c:295

## xuiTerminalSetLinkCallback
- 位置: xui.h:8863  已注释: 否
- 签名: `XUI_API int xuiTerminalSetLinkCallback(xui_widget pWidget, xui_terminal_link_proc onLink, void* pUser);`
- 实现: src/xui_terminal.c:6382（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:296

## xuiTerminalSetImageCallback
- 位置: xui.h:8864  已注释: 否
- 签名: `XUI_API int xuiTerminalSetImageCallback(xui_widget pWidget, xui_terminal_image_proc onImage, void* pUser);`
- 实现: 未定位（可能在条件编译块或别名定义，需人工确认）

## xuiTerminalSetPalette
- 位置: xui.h:8865  已注释: 否
- 签名: `XUI_API int xuiTerminalSetPalette(xui_widget pWidget, int iIndex, uint32_t iColor);`
- 实现: src/xui_terminal.c:6391（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiTerminalGetPalette
- 位置: xui.h:8866  已注释: 否
- 签名: `XUI_API uint32_t xuiTerminalGetPalette(xui_widget pWidget, int iIndex);`
- 实现: src/xui_terminal.c:6401（体 7 行）
- 用法: test_xui/xui_terminal_test.c:311; test_xui/xui_terminal_test.c:344; test_xui/xui_terminal_test.c:576

## xuiTerminalGetScrollModel
- 位置: xui.h:8867  已注释: 否
- 签名: `XUI_API xui_scroll_model_t* xuiTerminalGetScrollModel(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6409（体 5 行）
- 用法: test_xui/xui_terminal_test.c:412; test_xui/xui_terminal_test.c:418; test_xui/xui_terminal_test.c:420

## xuiTerminalGetCell
- 位置: xui.h:8868  已注释: 否
- 签名: `XUI_API int xuiTerminalGetCell(xui_widget pWidget, int iColumn, int iRow, xui_terminal_cell_t* pCell);`
- 实现: src/xui_terminal.c:6415（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_history_memory_test.c:236; test_xui/xui_terminal_history_memory_test.c:248; test_xui/xui_terminal_parser_test.c:104

## xuiTerminalSetBracketedPaste
- 位置: xui.h:8869  已注释: 否
- 签名: `XUI_API int xuiTerminalSetBracketedPaste(xui_widget pWidget, int bEnabled);`
- 实现: src/xui_terminal.c:6426（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal/main.c:283; test_xui/xui_terminal_test.c:592; test_xui/xui_terminal_test.c:894

## xuiTerminalGetBracketedPaste
- 位置: xui.h:8870  已注释: 否
- 签名: `XUI_API int xuiTerminalGetBracketedPaste(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6434（体 5 行）

## xuiTerminalInputText
- 位置: xui.h:8871  已注释: 否
- 签名: `XUI_API int xuiTerminalInputText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_terminal.c:6440（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal/main.c:281; examples/xui_terminal/main.c:282; examples/xui_terminal_cmd_lab/main.c:236

## xuiTerminalPasteText
- 位置: xui.h:8872  已注释: 否
- 签名: `XUI_API int xuiTerminalPasteText(xui_widget pWidget, const char* sText);`
- 实现: src/xui_terminal.c:6447（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal/main.c:284; test_xui/xui_terminal_test.c:594

## xuiTerminalSelectAll
- 位置: xui.h:8873  已注释: 否
- 签名: `XUI_API int xuiTerminalSelectAll(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6463（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal/main.c:285; test_xui/xui_terminal_test.c:667; test_xui/xui_terminal_test.c:976

## xuiTerminalClearSelection
- 位置: xui.h:8874  已注释: 否
- 签名: `XUI_API int xuiTerminalClearSelection(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6478（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:673; test_xui/xui_terminal_test.c:849; test_xui/xui_terminal_test.c:863

## xuiTerminalSetSelectionRange
- 位置: xui.h:8875  已注释: 否
- 签名: `XUI_API int xuiTerminalSetSelectionRange(xui_widget pWidget, int iAnchorLine, int iAnchorColumn, int iEndLine, int iEndColumn);`
- 实现: src/xui_terminal.c:6488（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_history_memory_test.c:270

## xuiTerminalGetSelectionRange
- 位置: xui.h:8876  已注释: 否
- 签名: `XUI_API int xuiTerminalGetSelectionRange(xui_widget pWidget, int* pAnchorLine, int* pAnchorColumn, int* pEndLine, int* pEndColumn);`
- 实现: src/xui_terminal.c:6498（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:817

## xuiTerminalGetSelectionText
- 位置: xui.h:8877  已注释: 否
- 签名: `XUI_API int xuiTerminalGetSelectionText(xui_widget pWidget, char* sBuffer, int iCapacity);`
- 实现: src/xui_terminal.c:6520（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_terminal/main.c:286; test_xui/xui_terminal_history_memory_test.c:271; test_xui/xui_terminal_test.c:470

## xuiTerminalCopySelection
- 位置: xui.h:8878  已注释: 否
- 签名: `XUI_API int xuiTerminalCopySelection(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6527（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY
- 用法: examples/xui_terminal/main.c:287; test_xui/xui_terminal_history_memory_test.c:273; test_xui/xui_terminal_test.c:671

## xuiTerminalSerializeText
- 位置: xui.h:8879  已注释: 否
- 签名: `XUI_API int xuiTerminalSerializeText(xui_widget pWidget, char* sBuffer, int iCapacity);`
- 实现: src/xui_terminal.c:6547（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_history_memory_test.c:261; test_xui/xui_terminal_history_memory_test.c:337; test_xui/xui_terminal_history_memory_test.c:341

## xuiTerminalFindText
- 位置: xui.h:8880  已注释: 否
- 签名: `XUI_API int xuiTerminalFindText(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn);`
- 实现: src/xui_terminal.c:6554（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_history_memory_test.c:268; test_xui/xui_terminal_history_memory_test.c:336; test_xui/xui_terminal_large_perf_test.c:122

## xuiTerminalFindNext
- 位置: xui.h:8881  已注释: 否
- 签名: `XUI_API int xuiTerminalFindNext(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn);`
- 实现: src/xui_terminal.c:6561（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_test.c:696; test_xui/xui_terminal_test.c:698

## xuiTerminalFindPrev
- 位置: xui.h:8882  已注释: 否
- 签名: `XUI_API int xuiTerminalFindPrev(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn);`
- 实现: src/xui_terminal.c:6568（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_test.c:700

## xuiTerminalClearFind
- 位置: xui.h:8883  已注释: 否
- 签名: `XUI_API int xuiTerminalClearFind(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6575（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:708; test_xui/xui_terminal_test.c:928

## xuiTerminalGetFindMatch
- 位置: xui.h:8884  已注释: 否
- 签名: `XUI_API int xuiTerminalGetFindMatch(xui_widget pWidget, int* pLine, int* pColumn, int* pLength);`
- 实现: src/xui_terminal.c:6589（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_parser_test.c:223; test_xui/xui_terminal_parser_test.c:281; test_xui/xui_terminal_parser_test.c:295

## xuiTerminalOpenFind
- 位置: xui.h:8885  已注释: 否
- 签名: `XUI_API int xuiTerminalOpenFind(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6599（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiTerminalGetFindWindow
- 位置: xui.h:8886  已注释: 否
- 签名: `XUI_API xui_widget xuiTerminalGetFindWindow(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6606（体 5 行）
- 用法: test_xui/xui_terminal_test.c:604; test_xui/xui_terminal_test.c:605; test_xui/xui_terminal_test.c:614

## xuiTerminalGetLinkAt
- 位置: xui.h:8887  已注释: 否
- 签名: `XUI_API int xuiTerminalGetLinkAt(xui_widget pWidget, int iLine, int iColumn, char* sBuffer, int iCapacity, int* pStartColumn, int* pLength);`
- 实现: src/xui_terminal.c:6612（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_test.c:724; test_xui/xui_terminal_test.c:726; test_xui/xui_terminal_test.c:752

## xuiTerminalOpenMenu
- 位置: xui.h:8888  已注释: 否
- 签名: `XUI_API int xuiTerminalOpenMenu(xui_widget pWidget, float fX, float fY);`
- 实现: src/xui_terminal.c:6633（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_terminal_test.c:896; test_xui/xui_terminal_test.c:917; test_xui/xui_terminal_test.c:949

## xuiTerminalGetMenuWidget
- 位置: xui.h:8889  已注释: 否
- 签名: `XUI_API xui_widget xuiTerminalGetMenuWidget(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6645（体 5 行）
- 用法: test_xui/xui_terminal_test.c:890

## xuiTerminalSetMenuTitle
- 位置: xui.h:8890  已注释: 否
- 签名: `XUI_API int xuiTerminalSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle);`
- 实现: src/xui_terminal.c:6651（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_OUT_OF_MEMORY

## xuiTerminalGetMenuTitle
- 位置: xui.h:8891  已注释: 否
- 签名: `XUI_API const char* xuiTerminalGetMenuTitle(xui_widget pWidget, int iCommand);`
- 实现: src/xui_terminal.c:6675（体 7 行）

## xuiTerminalAttachSession
- 位置: xui.h:8892  已注释: 否
- 签名: `XUI_API int xuiTerminalAttachSession(xui_widget pWidget, xui_terminal_session_t* pSession);`
- 实现: src/xui_terminal.c:6683（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal/main.c:220; examples/xui_terminal_cmd_lab/main.c:581; test_xui/xui_terminal_test.c:1025

## xuiTerminalDetachSession
- 位置: xui.h:8893  已注释: 否
- 签名: `XUI_API int xuiTerminalDetachSession(xui_widget pWidget);`
- 实现: src/xui_terminal.c:6705（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:599; examples/xui_terminal_cmd_lab/main.c:876

## xuiTerminalCreateFakeSession
- 位置: xui.h:8894  已注释: 否
- 签名: `XUI_API xui_terminal_session_t* xuiTerminalCreateFakeSession(const xui_terminal_session_desc_t* pDesc);`
- 实现: src/xui_terminal.c:6808（体 20 行）
- 返回码: NULL
- 用法: examples/xui_terminal/main.c:218; test_xui/xui_terminal_test.c:1023

## xuiTerminalCreateProcessSession
- 位置: xui.h:8895  已注释: 否
- 签名: `XUI_API xui_terminal_session_t* xuiTerminalCreateProcessSession(const xui_terminal_process_desc_t* pDesc);`
- 实现: src/xui_terminal.c:6979（体 44 行）
- 返回码: NULL
- 用法: examples/xui_terminal_cmd_lab/main.c:575; test_xui/xui_terminal_test.c:1054; test_xui/xui_terminal_test.c:1083

## xuiTerminalBuildSshCommand
- 位置: xui.h:8896  已注释: 否
- 签名: `XUI_API int xuiTerminalBuildSshCommand(const xui_terminal_ssh_desc_t* pDesc, char* sBuffer, int iCapacity);`
- 实现: 未定位（可能在条件编译块或别名定义，需人工确认）

## xuiTerminalCreateSshSession
- 位置: xui.h:8897  已注释: 否
- 签名: `XUI_API xui_terminal_session_t* xuiTerminalCreateSshSession(const xui_terminal_ssh_desc_t* pDesc);`
- 实现: 未定位（可能在条件编译块或别名定义，需人工确认）

## xuiTerminalSessionDestroy
- 位置: xui.h:8898  已注释: 否
- 签名: `XUI_API void xuiTerminalSessionDestroy(xui_terminal_session_t* pSession);`
- 实现: src/xui_terminal.c:7024（体 16 行）
- 用法: examples/xui_terminal/main.c:258; examples/xui_terminal_cmd_lab/main.c:582; examples/xui_terminal_cmd_lab/main.c:600

## xuiTerminalSessionWrite
- 位置: xui.h:8899  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionWrite(xui_terminal_session_t* pSession, const void* pData, int iSize);`
- 实现: src/xui_terminal.c:7041（体 35 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_BACKEND_FAILED, XUI_ERROR_UNSUPPORTED

## xuiTerminalSessionPoll
- 位置: xui.h:8900  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionPoll(xui_terminal_session_t* pSession);`
- 实现: src/xui_terminal.c:7077（体 57 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:596; examples/xui_terminal_cmd_lab/main.c:598; test_xui/xui_terminal_test.c:1041

## xuiTerminalSessionIsRunning
- 位置: xui.h:8901  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionIsRunning(xui_terminal_session_t* pSession);`
- 实现: src/xui_terminal.c:7135（体 8 行）
- 用法: examples/xui_terminal_cmd_lab/main.c:597; test_xui/xui_terminal_test.c:1043; test_xui/xui_terminal_test.c:1045

## xuiTerminalSessionTerminate
- 位置: xui.h:8902  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionTerminate(xui_terminal_session_t* pSession);`
- 实现: src/xui_terminal.c:7144（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:1037; test_xui/xui_terminal_test.c:1044; test_xui/xui_terminal_test.c:1071

## xuiTerminalSessionResize
- 位置: xui.h:8903  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionResize(xui_terminal_session_t* pSession, int iColumns, int iRows);`
- 实现: src/xui_terminal.c:7154（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_terminal_cmd_lab/main.c:788

## xuiTerminalSessionSetResizeCallback
- 位置: xui.h:8904  已注释: 否
- 签名: `XUI_API int xuiTerminalSessionSetResizeCallback(xui_terminal_session_t* pSession, xui_terminal_session_resize_proc onResize, void* pUser);`
- 实现: src/xui_terminal.c:7175（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_terminal_test.c:1029

## xuiTerminalGetChangeCount
- 位置: xui.h:8905  已注释: 否
- 签名: `XUI_API int xuiTerminalGetChangeCount(xui_widget pWidget);`
- 实现: src/xui_terminal.c:7191（体 5 行）
- 用法: examples/xui_terminal/main.c:302; examples/xui_terminal/main.c:360; examples/xui_terminal_cmd_lab/main.c:1029

## xuiTerminalGetStats
- 位置: xui.h:8906  已注释: 否
- 签名: `XUI_API int xuiTerminalGetStats(xui_widget pWidget, xui_terminal_stats_t* pStats);`
- 实现: src/xui_terminal.c:7197（体 49 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_terminal_history_memory_test.c:325; test_xui/xui_terminal_history_memory_test.c:345; test_xui/xui_terminal_large_perf_test.c:125

