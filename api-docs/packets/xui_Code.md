# 草稿包：xui.h / Code（191 条 API）

> 生成 2026-09-10 02:49 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiCodeDocumentCreate
- 位置: xui.h:6374  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentCreate(xui_code_document* ppDocument);`
- 实现: src/xui_code_document.c:773（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:196; test_xui/xui_code_document_test.c:102; test_xui/xui_code_editing_test.c:36

## xuiCodeDocumentDestroy
- 位置: xui.h:6375  已注释: 否
- 签名: `XUI_API void xuiCodeDocumentDestroy(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:800（体 13 行）
- 用法: test_xui/xui_code_command_test.c:294; test_xui/xui_code_document_test.c:268; test_xui/xui_code_editing_test.c:140

## xuiCodeDocumentSetText
- 位置: xui.h:6376  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentSetText(xui_code_document pDocument, const char* sText);`
- 实现: src/xui_code_document.c:814（体 4 行）
- 用法: test_xui/xui_code_command_test.c:198; test_xui/xui_code_command_test.c:224; test_xui/xui_code_command_test.c:240

## xuiCodeDocumentSetTextLength
- 位置: xui.h:6377  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentSetTextLength(xui_code_document pDocument, const char* sText, int iLength);`
- 实现: src/xui_code_document.c:819（体 40 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_code_document_test.c:124; test_xui/xui_code_large_perf_test.c:129; test_xui/xui_code_search_test.c:95

## xuiCodeDocumentGetText
- 位置: xui.h:6378  已注释: 否
- 签名: `XUI_API const char* xuiCodeDocumentGetText(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:860（体 6 行）
- 用法: test_xui/xui_code_command_test.c:249; test_xui/xui_code_command_test.c:251; test_xui/xui_code_command_test.c:257

## xuiCodeDocumentGetByte
- 位置: xui.h:6379  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetByte(xui_code_document pDocument, int iOffset, char* pByte);`
- 实现: src/xui_code_document.c:867（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:222

## xuiCodeDocumentCopyRange
- 位置: xui.h:6380  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentCopyRange(xui_code_document pDocument, int iStart, int iEnd, char* sOutput, int iCapacity, int* pLength);`
- 实现: src/xui_code_document.c:877（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_document_test.c:219

## xuiCodeDocumentLoadTextFile
- 位置: xui.h:6381  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentLoadTextFile(xui_code_document pDocument, const char* sPath, int iCharset);`
- 实现: src/xui_code_document.c:897（体 46 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_code_document_test.c:263; test_xui/xui_code_large_perf_test.c:136

## xuiCodeDocumentSaveTextFile
- 位置: xui.h:6382  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentSaveTextFile(xui_code_document pDocument, const char* sPath, int iCharset);`
- 实现: src/xui_code_document.c:944（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_RESOURCE_FAILED, XUI_OK
- 用法: test_xui/xui_code_document_test.c:259

## xuiCodeDocumentGetLength
- 位置: xui.h:6383  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetLength(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:961（体 5 行）
- 用法: test_xui/xui_code_document_test.c:107; test_xui/xui_code_document_test.c:173; test_xui/xui_code_document_test.c:191

## xuiCodeDocumentGetLineCount
- 位置: xui.h:6384  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetLineCount(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:967（体 5 行）
- 用法: test_xui/xui_code_document_test.c:62; test_xui/xui_code_document_test.c:64; test_xui/xui_code_document_test.c:108

## xuiCodeDocumentGetLineRange
- 位置: xui.h:6385  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetLineRange(xui_code_document pDocument, int iLine, int* pStart, int* pEnd);`
- 实现: src/xui_code_document.c:973（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:53; test_xui/xui_code_document_test.c:114; test_xui/xui_code_edit_large_perf_test.c:249

## xuiCodeDocumentOffsetToLineColumn
- 位置: xui.h:6386  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentOffsetToLineColumn(xui_code_document pDocument, int iOffset, int* pLine, int* pColumn);`
- 实现: src/xui_code_document.c:983（体 40 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:232; test_xui/xui_code_command_test.c:238; test_xui/xui_code_document_test.c:116

## xuiCodeDocumentLineColumnToOffset
- 位置: xui.h:6387  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentLineColumnToOffset(xui_code_document pDocument, int iLine, int iColumn, int* pOffset);`
- 实现: src/xui_code_document.c:1024（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:120; test_xui/xui_code_document_test.c:135; test_xui/xui_code_large_perf_test.c:204

## xuiCodeDocumentInsert
- 位置: xui.h:6388  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentInsert(xui_code_document pDocument, int iOffset, const char* sText);`
- 实现: src/xui_code_document.c:1126（体 4 行）
- 用法: test_xui/xui_code_document_test.c:143; test_xui/xui_code_document_test.c:173; test_xui/xui_code_document_test.c:180

## xuiCodeDocumentDelete
- 位置: xui.h:6389  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentDelete(xui_code_document pDocument, int iStart, int iEnd);`
- 实现: src/xui_code_document.c:1131（体 4 行）
- 用法: test_xui/xui_code_document_test.c:153; test_xui/xui_code_document_test.c:208; test_xui/xui_code_document_test.c:209

## xuiCodeDocumentReplace
- 位置: xui.h:6390  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentReplace(xui_code_document pDocument, int iStart, int iEnd, const char* sText);`
- 实现: src/xui_code_document.c:1043（体 82 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_document_test.c:158; test_xui/xui_code_document_test.c:245

## xuiCodeDocumentBeginEdit
- 位置: xui.h:6391  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentBeginEdit(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1136（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_document_test.c:178

## xuiCodeDocumentEndEdit
- 位置: xui.h:6392  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentEndEdit(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1154（体 16 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:184

## xuiCodeDocumentSetChangeCallback
- 位置: xui.h:6393  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentSetChangeCallback(xui_code_document pDocument, xui_code_document_change_proc onChange, void* pUser);`
- 实现: src/xui_code_document.c:1171（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:104

## xuiCodeDocumentUndo
- 位置: xui.h:6394  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentUndo(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1179（体 39 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_document_test.c:164; test_xui/xui_code_document_test.c:166; test_xui/xui_code_document_test.c:189

## xuiCodeDocumentRedo
- 位置: xui.h:6395  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentRedo(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1219（体 39 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_document_test.c:168; test_xui/xui_code_document_test.c:205

## xuiCodeDocumentCanUndo
- 位置: xui.h:6396  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentCanUndo(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1259（体 4 行）
- 用法: test_xui/xui_code_document_test.c:151

## xuiCodeDocumentCanRedo
- 位置: xui.h:6397  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentCanRedo(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1264（体 4 行）

## xuiCodeDocumentGetVersion
- 位置: xui.h:6398  已注释: 否
- 签名: `XUI_API uint32_t xuiCodeDocumentGetVersion(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1269（体 4 行）
- 用法: test_xui/xui_code_document_test.c:138; test_xui/xui_code_document_test.c:141; test_xui/xui_code_document_test.c:145

## xuiCodeDocumentGetChangeVersion
- 位置: xui.h:6399  已注释: 否
- 签名: `XUI_API uint32_t xuiCodeDocumentGetChangeVersion(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1274（体 4 行）

## xuiCodeDocumentGetLastEditRange
- 位置: xui.h:6400  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetLastEditRange(xui_code_document pDocument, xui_code_range_t* pRange);`
- 实现: src/xui_code_document.c:1279（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_document_test.c:147; test_xui/xui_code_document_test.c:155; test_xui/xui_code_document_test.c:160

## xuiCodeDocumentGetDirty
- 位置: xui.h:6401  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentGetDirty(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1288（体 4 行）
- 用法: test_xui/xui_code_document_test.c:122; test_xui/xui_code_document_test.c:149; test_xui/xui_code_document_test.c:172

## xuiCodeDocumentSetDirty
- 位置: xui.h:6402  已注释: 否
- 签名: `XUI_API int xuiCodeDocumentSetDirty(xui_code_document pDocument, int bDirty);`
- 实现: src/xui_code_document.c:1293（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_document_test.c:171

## xuiCodeDocumentGetLastError
- 位置: xui.h:6403  已注释: 否
- 签名: `XUI_API const char* xuiCodeDocumentGetLastError(xui_code_document pDocument);`
- 实现: src/xui_code_document.c:1304（体 5 行）
- 用法: test_xui/xui_code_document_test.c:194; test_xui/xui_code_large_perf_test.c:144

## xuiCodeLexerCTokenize
- 位置: xui.h:6404  已注释: 否
- 签名: `XUI_API int xuiCodeLexerCTokenize(const char* sText, int iTextSize, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount);`
- 实现: src/xui_code_lexer_c.c:178（体 4 行）
- 用法: test_xui/xui_code_lexer_test.c:46; test_xui/xui_code_lexer_test.c:58; test_xui/xui_code_lexer_test.c:67

## xuiCodeLexerCTokenizeRange
- 位置: xui.h:6405  已注释: 否
- 签名: `XUI_API int xuiCodeLexerCTokenizeRange(const char* sText, int iTextSize, int iStartOffset, int iEndOffset, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount);`
- 实现: src/xui_code_lexer_c.c:86（体 91 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_lexer_test.c:62

## xuiCodeLexerCTokenizeDocumentRange
- 位置: xui.h:6406  已注释: 否
- 签名: `XUI_API int xuiCodeLexerCTokenizeDocumentRange(xui_code_document pDocument, int iStartOffset, int iEndOffset, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount);`
- 实现: src/xui_code_lexer_c.c:183（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY

## xuiCodeLexerRegexTokenize
- 位置: xui.h:6407  已注释: 否
- 签名: `XUI_API int xuiCodeLexerRegexTokenize(const char* sText, int iTextSize, const xui_code_regex_rule_t* pRules, int iRuleCount, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_lexer.c:33（体 75 行）
- 返回码: XUI_ERROR_UNSUPPORTED, XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_lexer_test.c:89; test_xui/xui_code_lexer_test.c:99

## xuiCodeFoldCBuildRanges
- 位置: xui.h:6408  已注释: 否
- 签名: `XUI_API int xuiCodeFoldCBuildRanges(const char* sText, int iTextSize, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount);`
- 实现: src/xui_code_fold.c:178（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: examples/xui_codeedit/main.c:297; test_xui/xui_code_fold_test.c:49

## xuiCodeFoldCBuildDocumentRanges
- 位置: xui.h:6409  已注释: 否
- 签名: `XUI_API int xuiCodeFoldCBuildDocumentRanges(xui_code_document pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount);`
- 实现: src/xui_code_fold.c:189（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_fold_test.c:67

## xuiCodeFoldBuildVisibleLines
- 位置: xui.h:6410  已注释: 否
- 签名: `XUI_API int xuiCodeFoldBuildVisibleLines(int iLineCount, const xui_code_fold_range_t* pRanges, int iRangeCount, int* pVisibleLines, int iVisibleCapacity, int* pVisibleCount);`
- 实现: src/xui_code_fold.c:200（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_test.c:73

## xuiCodeLayoutBuildVisibleLines
- 位置: xui.h:6411  已注释: 否
- 签名: `XUI_API int xuiCodeLayoutBuildVisibleLines(const xui_code_layout_desc_t* pDesc, xui_code_layout_line_t* pLines, int iLineCapacity, int* pLineCount, xui_vec2_t* pContentSize, xui_rect_t* pTextRect);`
- 实现: src/xui_code_layout.c:253（体 77 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_layout_test.c:45; test_xui/xui_code_layout_test.c:64; test_xui/xui_code_layout_test.c:80

## xuiCodeLayoutHitTest
- 位置: xui.h:6412  已注释: 否
- 签名: `XUI_API int xuiCodeLayoutHitTest(const xui_code_layout_desc_t* pDesc, float fX, float fY, xui_code_hit_t* pHit);`
- 实现: src/xui_code_layout.c:331（体 74 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_layout_test.c:52; test_xui/xui_code_layout_test.c:71; test_xui/xui_code_layout_test.c:84

## xuiCodeLayoutGetCaretRect
- 位置: xui.h:6413  已注释: 否
- 签名: `XUI_API int xuiCodeLayoutGetCaretRect(const xui_code_layout_desc_t* pDesc, int iLine, int iColumn, xui_rect_t* pRect);`
- 实现: src/xui_code_layout.c:406（体 66 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_layout_test.c:54; test_xui/xui_code_layout_test.c:67; test_xui/xui_code_layout_test.c:82

## xuiCodeFindScopeCreate
- 位置: xui.h:6421  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeCreate(xui_code_find_scope* ppScope);`
- 实现: src/xui_code_edit.c:7643（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1085

## xuiCodeFindScopeDestroy
- 位置: xui.h:6422  已注释: 否
- 签名: `XUI_API void xuiCodeFindScopeDestroy(xui_code_find_scope pScope);`
- 实现: src/xui_code_edit.c:7657（体 15 行）
- 用法: test_xui/xui_code_edit_test.c:1131; test_xui/xui_code_edit_test.c:1968

## xuiCodeFindScopeClearEditors
- 位置: xui.h:6423  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeClearEditors(xui_code_find_scope pScope);`
- 实现: src/xui_code_edit.c:7673（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeFindScopeAddEditor
- 位置: xui.h:6424  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeAddEditor(xui_code_find_scope pScope, xui_widget pEditor);`
- 实现: src/xui_code_edit.c:7692（体 4 行）

## xuiCodeFindScopeAddEditorNamed
- 位置: xui.h:6425  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeAddEditorNamed(xui_code_find_scope pScope, xui_widget pEditor, const char* sName);`
- 实现: src/xui_code_edit.c:7697（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1087; test_xui/xui_code_edit_test.c:1089

## xuiCodeFindScopeSetEditorName
- 位置: xui.h:6426  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeSetEditorName(xui_code_find_scope pScope, xui_widget pEditor, const char* sName);`
- 实现: src/xui_code_edit.c:7722（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED

## xuiCodeFindScopeRemoveEditor
- 位置: xui.h:6427  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeRemoveEditor(xui_code_find_scope pScope, xui_widget pEditor);`
- 实现: src/xui_code_edit.c:7735（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED

## xuiCodeFindScopeGetEditorCount
- 位置: xui.h:6428  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeGetEditorCount(xui_code_find_scope pScope);`
- 实现: src/xui_code_edit.c:7755（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:1090

## xuiCodeFindScopeGetEditor
- 位置: xui.h:6429  已注释: 否
- 签名: `XUI_API xui_widget xuiCodeFindScopeGetEditor(xui_code_find_scope pScope, int iIndex);`
- 实现: src/xui_code_edit.c:7760（体 5 行）
- 返回码: NULL

## xuiCodeFindScopeGetEditorName
- 位置: xui.h:6430  已注释: 否
- 签名: `XUI_API const char* xuiCodeFindScopeGetEditorName(xui_code_find_scope pScope, int iIndex);`
- 实现: src/xui_code_edit.c:7766（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1091; test_xui/xui_code_edit_test.c:1091

## xuiCodeFindScopeSetActivate
- 位置: xui.h:6431  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeSetActivate(xui_code_find_scope pScope, xui_code_find_activate_proc onActivate, void* pUser);`
- 实现: src/xui_code_edit.c:7772（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1092

## xuiCodeFindScopeFindAll
- 位置: xui.h:6432  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeFindAll(xui_code_find_scope pScope, const xui_find_options_t* pOptions, int* pResultCount);`
- 实现: src/xui_code_edit.c:7780（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1098

## xuiCodeFindScopeGetResultCount
- 位置: xui.h:6433  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeGetResultCount(xui_code_find_scope pScope);`
- 实现: src/xui_code_edit.c:7812（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:1099

## xuiCodeFindScopeGetResult
- 位置: xui.h:6434  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeGetResult(xui_code_find_scope pScope, int iIndex, xui_code_find_result_t* pResult);`
- 实现: src/xui_code_edit.c:7817（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1100

## xuiCodeFindScopeGetResultEditorName
- 位置: xui.h:6435  已注释: 否
- 签名: `XUI_API const char* xuiCodeFindScopeGetResultEditorName(xui_code_find_scope pScope, int iIndex);`
- 实现: src/xui_code_edit.c:7825（体 5 行）
- 用法: test_xui/xui_code_edit_test.c:1102

## xuiCodeFindScopeActivateResult
- 位置: xui.h:6436  已注释: 否
- 签名: `XUI_API int xuiCodeFindScopeActivateResult(xui_code_find_scope pScope, int iIndex);`
- 实现: src/xui_code_edit.c:7831（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1127

## xuiCodeSearchFindPlain
- 位置: xui.h:6437  已注释: 否
- 签名: `XUI_API int xuiCodeSearchFindPlain(xui_code_document pDocument, const char* sPattern, int iStartOffset, uint32_t iFlags, xui_code_range_t* pRange);`
- 实现: src/xui_code_search.c:155（体 21 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_large_perf_test.c:215; test_xui/xui_code_search_test.c:35; test_xui/xui_code_search_test.c:37

## xuiCodeSearchFindPlainRange
- 位置: xui.h:6438  已注释: 否
- 签名: `XUI_API int xuiCodeSearchFindPlainRange(xui_code_document pDocument, const char* sPattern, int iStartOffset, int iRangeStart, int iRangeEnd, uint32_t iFlags, xui_code_range_t* pRange);`
- 实现: src/xui_code_search.c:177（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_search_test.c:47; test_xui/xui_code_search_test.c:49; test_xui/xui_code_search_test.c:51

## xuiCodeSearchFindRegex
- 位置: xui.h:6439  已注释: 否
- 签名: `XUI_API int xuiCodeSearchFindRegex(xui_code_document pDocument, const char* sPattern, int iStartOffset, uint32_t iFlags, xui_code_search_result_t* pResult, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_search.c:285（体 5 行）
- 用法: test_xui/xui_code_search_test.c:68; test_xui/xui_code_search_test.c:74; test_xui/xui_code_search_test.c:85

## xuiCodeSearchFindRegexRange
- 位置: xui.h:6440  已注释: 否
- 签名: `XUI_API int xuiCodeSearchFindRegexRange(xui_code_document pDocument, const char* sPattern, int iStartOffset, int iRangeStart, int iRangeEnd, uint32_t iFlags, xui_code_search_result_t* pResult, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_search.c:291（体 75 行）
- 返回码: XUI_ERROR_UNSUPPORTED, XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_code_search_test.c:80

## xuiCodeSearchReplaceAllPlain
- 位置: xui.h:6441  已注释: 否
- 签名: `XUI_API int xuiCodeSearchReplaceAllPlain(xui_code_document pDocument, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount);`
- 实现: src/xui_code_search.c:367（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_search_test.c:108

## xuiCodeSearchReplaceAllRegex
- 位置: xui.h:6442  已注释: 否
- 签名: `XUI_API int xuiCodeSearchReplaceAllRegex(xui_code_document pDocument, const char* sPattern, const char* sReplacement, uint32_t iFlags, int* pReplaceCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_search.c:447（体 61 行）
- 返回码: XUI_ERROR_UNSUPPORTED, XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_search_test.c:114

## xuiCodeSearchClearResultIndicators
- 位置: xui.h:6443  已注释: 否
- 签名: `XUI_API int xuiCodeSearchClearResultIndicators(xui_code_annotation_store pStore, int iIndicator);`
- 实现: src/xui_code_search.c:514（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_search_indicator_test.c:50; test_xui/xui_code_search_indicator_test.c:54

## xuiCodeSearchMarkAllPlain
- 位置: xui.h:6444  已注释: 否
- 签名: `XUI_API int xuiCodeSearchMarkAllPlain(xui_code_document pDocument, xui_code_annotation_store pStore, const char* sPattern, uint32_t iFlags, int iIndicator, int* pMatchCount);`
- 实现: src/xui_code_search.c:521（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_search_indicator_test.c:35

## xuiCodeSearchMarkAllRegex
- 位置: xui.h:6445  已注释: 否
- 签名: `XUI_API int xuiCodeSearchMarkAllRegex(xui_code_document pDocument, xui_code_annotation_store pStore, const char* sPattern, uint32_t iFlags, int iIndicator, int* pMatchCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_search.c:548（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_search_indicator_test.c:44

## xuiCodeAnnotationStoreCreate
- 位置: xui.h:6446  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationStoreCreate(xui_code_annotation_store* ppStore);`
- 实现: src/xui_code_annotation.c:187（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:31; test_xui/xui_code_search_indicator_test.c:30

## xuiCodeAnnotationStoreDestroy
- 位置: xui.h:6447  已注释: 否
- 签名: `XUI_API void xuiCodeAnnotationStoreDestroy(xui_code_annotation_store pStore);`
- 实现: src/xui_code_annotation.c:200（体 10 行）
- 用法: test_xui/xui_code_annotation_test.c:115; test_xui/xui_code_search_indicator_test.c:60

## xuiCodeAnnotationStoreClear
- 位置: xui.h:6448  已注释: 否
- 签名: `XUI_API void xuiCodeAnnotationStoreClear(xui_code_annotation_store pStore);`
- 实现: src/xui_code_annotation.c:211（体 12 行）
- 用法: test_xui/xui_code_annotation_test.c:110

## xuiCodeAnnotationSetMarker
- 位置: xui.h:6449  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationSetMarker(xui_code_annotation_store pStore, int iLine, int iMarker, uint32_t iFlags, const char* sTooltip, uintptr_t iUserData);`
- 实现: src/xui_code_annotation.c:224（体 34 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_codeedit/main.c:283; test_xui/xui_code_annotation_test.c:34; test_xui/xui_code_annotation_test.c:36

## xuiCodeAnnotationClearMarker
- 位置: xui.h:6450  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationClearMarker(xui_code_annotation_store pStore, int iLine, int iMarker);`
- 实现: src/xui_code_annotation.c:259（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_annotation_test.c:44

## xuiCodeAnnotationClearMarkers
- 位置: xui.h:6451  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationClearMarkers(xui_code_annotation_store pStore, int iLine);`
- 实现: src/xui_code_annotation.c:275（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeAnnotationGetMarkers
- 位置: xui.h:6452  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetMarkers(xui_code_annotation_store pStore, int iLine, xui_code_marker_t* pMarkers, int iMarkerCapacity, int* pMarkerCount);`
- 实现: src/xui_code_annotation.c:293（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:40; test_xui/xui_code_annotation_test.c:46; test_xui/xui_code_annotation_test.c:92

## xuiCodeAnnotationSetIndicator
- 位置: xui.h:6453  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationSetIndicator(xui_code_annotation_store pStore, int iIndicator, int iStyle, int iStart, int iEnd, uint32_t iFlags, uintptr_t iUserData);`
- 实现: src/xui_code_annotation.c:309（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:49; test_xui/xui_code_annotation_test.c:51

## xuiCodeAnnotationClearIndicator
- 位置: xui.h:6454  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationClearIndicator(xui_code_annotation_store pStore, int iIndicator, int iStart, int iEnd);`
- 实现: src/xui_code_annotation.c:329（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_annotation_test.c:56

## xuiCodeAnnotationClearIndicators
- 位置: xui.h:6455  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationClearIndicators(xui_code_annotation_store pStore, int iIndicator);`
- 实现: src/xui_code_annotation.c:350（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT

## xuiCodeAnnotationGetIndicatorsAt
- 位置: xui.h:6456  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetIndicatorsAt(xui_code_annotation_store pStore, int iOffset, xui_code_indicator_t* pIndicators, int iIndicatorCapacity, int* pIndicatorCount);`
- 实现: src/xui_code_annotation.c:371（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:53; test_xui/xui_code_annotation_test.c:58; test_xui/xui_code_annotation_test.c:94

## xuiCodeAnnotationSetDiagnostics
- 位置: xui.h:6457  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationSetDiagnostics(xui_code_annotation_store pStore, const xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCount);`
- 实现: src/xui_code_annotation.c:387（体 43 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_codeedit/main.c:294; test_xui/xui_code_annotation_test.c:76; test_xui/xui_code_edit_test.c:594

## xuiCodeAnnotationClearDiagnostics
- 位置: xui.h:6458  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationClearDiagnostics(xui_code_annotation_store pStore);`
- 实现: src/xui_code_annotation.c:431（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:107

## xuiCodeAnnotationGetDiagnosticCount
- 位置: xui.h:6459  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetDiagnosticCount(xui_code_annotation_store pStore);`
- 实现: src/xui_code_annotation.c:442（体 4 行）
- 用法: test_xui/xui_code_annotation_test.c:77; test_xui/xui_code_annotation_test.c:108; test_xui/xui_code_edit_test.c:1570

## xuiCodeAnnotationGetDiagnostic
- 位置: xui.h:6460  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetDiagnostic(xui_code_annotation_store pStore, int iIndex, xui_code_diagnostic_t* pDiagnostic);`
- 实现: src/xui_code_annotation.c:447（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:81

## xuiCodeAnnotationGetDiagnosticsAt
- 位置: xui.h:6461  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetDiagnosticsAt(xui_code_annotation_store pStore, int iOffset, xui_code_diagnostic_t* pDiagnostics, int iDiagnosticCapacity, int* pDiagnosticCount);`
- 实现: src/xui_code_annotation.c:454（体 4 行）
- 用法: test_xui/xui_code_annotation_test.c:87; test_xui/xui_code_annotation_test.c:96; test_xui/xui_code_annotation_test.c:104

## xuiCodeAnnotationGetDiagnosticsInRange
- 位置: xui.h:6462  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationGetDiagnosticsInRange(xui_code_annotation_store pStore, int iStart, int iEnd, xui_code_diagnostic_t* pDiagnostics, int* pIndices, int iDiagnosticCapacity, int* pDiagnosticCount);`
- 实现: src/xui_code_annotation.c:459（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:84

## xuiCodeAnnotationGetDiagnosticVersion
- 位置: xui.h:6463  已注释: 否
- 签名: `XUI_API uint32_t xuiCodeAnnotationGetDiagnosticVersion(xui_code_annotation_store pStore);`
- 实现: src/xui_code_annotation.c:484（体 4 行）
- 用法: test_xui/xui_code_annotation_test.c:78; test_xui/xui_code_annotation_test.c:109

## xuiCodeAnnotationTrackEdit
- 位置: xui.h:6464  已注释: 否
- 签名: `XUI_API int xuiCodeAnnotationTrackEdit(xui_code_annotation_store pStore, int iStartOffset, int iEndOffset, int iNewEndOffset, int iStartLine, int iEndLine, int iNewEndLine);`
- 实现: src/xui_code_annotation.c:505（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_annotation_test.c:90; test_xui/xui_code_annotation_test.c:98

## xuiCodeLanguageRegistryCreate
- 位置: xui.h:6465  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryCreate(xui_code_language_registry* ppRegistry);`
- 实现: src/xui_code_language.c:245（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_language_test.c:51

## xuiCodeLanguageRegistryDestroy
- 位置: xui.h:6466  已注释: 否
- 签名: `XUI_API void xuiCodeLanguageRegistryDestroy(xui_code_language_registry pRegistry);`
- 实现: src/xui_code_language.c:258（体 7 行）
- 用法: test_xui/xui_code_language_test.c:119

## xuiCodeLanguageRegistryClear
- 位置: xui.h:6467  已注释: 否
- 签名: `XUI_API void xuiCodeLanguageRegistryClear(xui_code_language_registry pRegistry);`
- 实现: src/xui_code_language.c:266（体 8 行）
- 用法: test_xui/xui_code_language_test.c:114

## xuiCodeLanguageRegistryLoadDefaults
- 位置: xui.h:6468  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryLoadDefaults(xui_code_language_registry pRegistry);`
- 实现: src/xui_code_language.c:275（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_language_test.c:53

## xuiCodeLanguageRegistryRegister
- 位置: xui.h:6469  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryRegister(xui_code_language_registry pRegistry, const xui_code_language_t* pLanguage);`
- 实现: src/xui_code_language.c:296（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1613; test_xui/xui_code_language_test.c:87; test_xui/xui_code_language_test.c:107

## xuiCodeLanguageRegistryGetCount
- 位置: xui.h:6470  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryGetCount(xui_code_language_registry pRegistry);`
- 实现: src/xui_code_language.c:323（体 4 行）
- 用法: test_xui/xui_code_language_test.c:54; test_xui/xui_code_language_test.c:88; test_xui/xui_code_language_test.c:115

## xuiCodeLanguageRegistryGet
- 位置: xui.h:6471  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryGet(xui_code_language_registry pRegistry, int iIndex, xui_code_language_t* pLanguage);`
- 实现: src/xui_code_language.c:328（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeLanguageRegistryFind
- 位置: xui.h:6472  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryFind(xui_code_language_registry pRegistry, const char* sId, xui_code_language_t* pLanguage);`
- 实现: src/xui_code_language.c:336（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_language_test.c:55; test_xui/xui_code_language_test.c:109

## xuiCodeLanguageRegistryFindByExtension
- 位置: xui.h:6473  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageRegistryFindByExtension(xui_code_language_registry pRegistry, const char* sExtension, xui_code_language_t* pLanguage);`
- 实现: src/xui_code_language.c:351（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_language_test.c:57; test_xui/xui_code_language_test.c:89

## xuiCodeLanguageLex
- 位置: xui.h:6474  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageLex(const xui_code_language_t* pLanguage, xui_code_document pDocument, int iStartLine, int iEndLine, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount, char* sError, int iErrorCapacity);`
- 实现: src/xui_code_language.c:371（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_language_test.c:68; test_xui/xui_code_language_test.c:72; test_xui/xui_code_language_test.c:91

## xuiCodeLanguageFold
- 位置: xui.h:6475  已注释: 否
- 签名: `XUI_API int xuiCodeLanguageFold(const xui_code_language_t* pLanguage, xui_code_document pDocument, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount);`
- 实现: src/xui_code_language.c:386（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_language_test.c:75

## xuiCodeTokenBufferCreate
- 位置: xui.h:6476  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferCreate(xui_code_token_buffer* ppBuffer);`
- 实现: src/xui_code_token_buffer.c:32（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:29

## xuiCodeTokenBufferDestroy
- 位置: xui.h:6477  已注释: 否
- 签名: `XUI_API void xuiCodeTokenBufferDestroy(xui_code_token_buffer pBuffer);`
- 实现: src/xui_code_token_buffer.c:45（体 6 行）
- 用法: test_xui/xui_code_token_buffer_test.c:68

## xuiCodeTokenBufferClear
- 位置: xui.h:6478  已注释: 否
- 签名: `XUI_API void xuiCodeTokenBufferClear(xui_code_token_buffer pBuffer);`
- 实现: src/xui_code_token_buffer.c:52（体 8 行）
- 用法: test_xui/xui_code_token_buffer_test.c:64

## xuiCodeTokenBufferSet
- 位置: xui.h:6479  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferSet(xui_code_token_buffer pBuffer, const xui_code_token_t* pTokens, int iTokenCount, uint32_t iTextVersion);`
- 实现: src/xui_code_token_buffer.c:61（体 4 行）
- 用法: test_xui/xui_code_token_buffer_test.c:43

## xuiCodeTokenBufferSetRange
- 位置: xui.h:6480  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferSetRange(xui_code_token_buffer pBuffer, const xui_code_token_t* pTokens, int iTokenCount, uint32_t iTextVersion, int iRangeStart, int iRangeEnd);`
- 实现: src/xui_code_token_buffer.c:66（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:56

## xuiCodeTokenBufferGetVersion
- 位置: xui.h:6481  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferGetVersion(xui_code_token_buffer pBuffer, uint32_t* pTextVersion);`
- 实现: src/xui_code_token_buffer.c:85（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:46

## xuiCodeTokenBufferGetRange
- 位置: xui.h:6482  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferGetRange(xui_code_token_buffer pBuffer, uint32_t* pTextVersion, int* pRangeStart, int* pRangeEnd);`
- 实现: src/xui_code_token_buffer.c:92（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:58

## xuiCodeTokenBufferGetCount
- 位置: xui.h:6483  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferGetCount(xui_code_token_buffer pBuffer);`
- 实现: src/xui_code_token_buffer.c:101（体 4 行）
- 用法: examples/xui_codeedit/main.c:354; test_xui/xui_code_edit_large_perf_test.c:219; test_xui/xui_code_edit_large_perf_test.c:235

## xuiCodeTokenBufferGetTokens
- 位置: xui.h:6484  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferGetTokens(xui_code_token_buffer pBuffer, uint32_t iTextVersion, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount);`
- 实现: src/xui_code_token_buffer.c:106（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:48; test_xui/xui_code_token_buffer_test.c:50

## xuiCodeTokenBufferGetTokensInRange
- 位置: xui.h:6485  已注释: 否
- 签名: `XUI_API int xuiCodeTokenBufferGetTokensInRange(xui_code_token_buffer pBuffer, uint32_t iTextVersion, int iStart, int iEnd, xui_code_token_t* pTokens, int iTokenCapacity, int* pTokenCount);`
- 实现: src/xui_code_token_buffer.c:122（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_token_buffer_test.c:53; test_xui/xui_code_token_buffer_test.c:60; test_xui/xui_code_token_buffer_test.c:62

## xuiCodeFoldStateCreate
- 位置: xui.h:6486  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateCreate(xui_code_fold_state* ppState);`
- 实现: src/xui_code_fold_state.c:133（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:202; test_xui/xui_code_fold_state_test.c:50

## xuiCodeFoldStateDestroy
- 位置: xui.h:6487  已注释: 否
- 签名: `XUI_API void xuiCodeFoldStateDestroy(xui_code_fold_state pState);`
- 实现: src/xui_code_fold_state.c:146（体 7 行）
- 用法: test_xui/xui_code_command_test.c:292; test_xui/xui_code_fold_state_test.c:132

## xuiCodeFoldStateClear
- 位置: xui.h:6488  已注释: 否
- 签名: `XUI_API void xuiCodeFoldStateClear(xui_code_fold_state pState);`
- 实现: src/xui_code_fold_state.c:154（体 8 行）

## xuiCodeFoldStateSetRanges
- 位置: xui.h:6489  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateSetRanges(xui_code_fold_state pState, const xui_code_fold_range_t* pRanges, int iRangeCount);`
- 实现: src/xui_code_fold_state.c:163（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:298; test_xui/xui_code_edit_large_perf_test.c:290; test_xui/xui_code_edit_test.c:601

## xuiCodeFoldStateTrackEdit
- 位置: xui.h:6490  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateTrackEdit(xui_code_fold_state pState, int iStartLine, int iEndLine, int iNewEndLine);`
- 实现: src/xui_code_fold_state.c:184（体 33 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:105; test_xui/xui_code_fold_state_test.c:109

## xuiCodeFoldStateBuildFromProvider
- 位置: xui.h:6491  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateBuildFromProvider(xui_code_fold_state pState, xui_code_document pDocument, xui_code_fold_proc onFold, void* pUser);`
- 实现: src/xui_code_fold_state.c:218（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_code_command_test.c:204; test_xui/xui_code_fold_state_test.c:101

## xuiCodeFoldStateGetCount
- 位置: xui.h:6492  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateGetCount(xui_code_fold_state pState);`
- 实现: src/xui_code_fold_state.c:242（体 4 行）
- 用法: test_xui/xui_code_fold_state_test.c:64; test_xui/xui_code_fold_state_test.c:102; test_xui/xui_code_fold_state_test.c:110

## xuiCodeFoldStateGetRange
- 位置: xui.h:6493  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateGetRange(xui_code_fold_state pState, int iIndex, xui_code_fold_range_t* pRange);`
- 实现: src/xui_code_fold_state.c:247（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:264; test_xui/xui_code_command_test.c:268; test_xui/xui_code_edit_test.c:706

## xuiCodeFoldStateGetRanges
- 位置: xui.h:6494  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateGetRanges(xui_code_fold_state pState, xui_code_fold_range_t* pRanges, int iRangeCapacity, int* pRangeCount);`
- 实现: src/xui_code_fold_state.c:254（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeFoldStateToggleLine
- 位置: xui.h:6495  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateToggleLine(xui_code_fold_state pState, int iLine);`
- 实现: src/xui_code_fold_state.c:266（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:68; test_xui/xui_code_fold_state_test.c:94

## xuiCodeFoldStateFoldAll
- 位置: xui.h:6496  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateFoldAll(xui_code_fold_state pState);`
- 实现: src/xui_code_fold_state.c:278（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:84

## xuiCodeFoldStateUnfoldAll
- 位置: xui.h:6497  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateUnfoldAll(xui_code_fold_state pState);`
- 实现: src/xui_code_fold_state.c:288（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:90

## xuiCodeFoldStateIsLineVisible
- 位置: xui.h:6498  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateIsLineVisible(xui_code_fold_state pState, int iLine, int* pVisible);`
- 实现: src/xui_code_fold_state.c:298（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:70; test_xui/xui_code_fold_state_test.c:72

## xuiCodeFoldStateGetVisibleLineCount
- 位置: xui.h:6499  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateGetVisibleLineCount(xui_code_fold_state pState, int iLineCount, int* pVisibleCount);`
- 实现: src/xui_code_fold_state.c:325（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_large_perf_test.c:295; test_xui/xui_code_fold_state_test.c:77; test_xui/xui_code_fold_state_test.c:123

## xuiCodeFoldStateLineToVisibleRow
- 位置: xui.h:6500  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateLineToVisibleRow(xui_code_fold_state pState, int iLine, int* pRow);`
- 实现: src/xui_code_fold_state.c:349（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:79; test_xui/xui_code_fold_state_test.c:127

## xuiCodeFoldStateVisibleRowToLine
- 位置: xui.h:6501  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateVisibleRowToLine(xui_code_fold_state pState, int iLineCount, int iRow, int* pLine);`
- 实现: src/xui_code_fold_state.c:373（体 38 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:81; test_xui/xui_code_fold_state_test.c:88; test_xui/xui_code_fold_state_test.c:125

## xuiCodeFoldStateBuildVisibleLines
- 位置: xui.h:6502  已注释: 否
- 签名: `XUI_API int xuiCodeFoldStateBuildVisibleLines(xui_code_fold_state pState, int iLineCount, int* pVisibleLines, int iVisibleCapacity, int* pVisibleCount);`
- 实现: src/xui_code_fold_state.c:412（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_fold_state_test.c:74; test_xui/xui_code_fold_state_test.c:86; test_xui/xui_code_fold_state_test.c:92

## xuiCodeProviderSetCreate
- 位置: xui.h:6503  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetCreate(xui_code_provider_set* ppProviders);`
- 实现: src/xui_code_provider.c:18（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:206; test_xui/xui_code_provider_test.c:105

## xuiCodeProviderSetDestroy
- 位置: xui.h:6504  已注释: 否
- 签名: `XUI_API void xuiCodeProviderSetDestroy(xui_code_provider_set pProviders);`
- 实现: src/xui_code_provider.c:31（体 4 行）
- 用法: test_xui/xui_code_command_test.c:291; test_xui/xui_code_provider_test.c:146

## xuiCodeProviderSetClear
- 位置: xui.h:6505  已注释: 否
- 签名: `XUI_API void xuiCodeProviderSetClear(xui_code_provider_set pProviders);`
- 实现: src/xui_code_provider.c:36（体 5 行）
- 用法: test_xui/xui_code_provider_test.c:139

## xuiCodeProviderSetCompletion
- 位置: xui.h:6506  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetCompletion(xui_code_provider_set pProviders, xui_code_completion_proc onComplete, void* pUser);`
- 实现: src/xui_code_provider.c:42（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:274; test_xui/xui_code_edit_test.c:1699; test_xui/xui_code_provider_test.c:109

## xuiCodeProviderSetHover
- 位置: xui.h:6507  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetHover(xui_code_provider_set pProviders, xui_code_hover_proc onHover, void* pUser);`
- 实现: src/xui_code_provider.c:50（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_provider_test.c:116

## xuiCodeProviderSetSignature
- 位置: xui.h:6508  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetSignature(xui_code_provider_set pProviders, xui_code_signature_proc onSignature, void* pUser);`
- 实现: src/xui_code_provider.c:58（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:276; test_xui/xui_code_edit_test.c:1905; test_xui/xui_code_provider_test.c:122

## xuiCodeProviderSetCommand
- 位置: xui.h:6509  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetCommand(xui_code_provider_set pProviders, xui_code_command_proc onCommand, void* pUser);`
- 实现: src/xui_code_provider.c:66（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:208; test_xui/xui_code_edit_test.c:1572; test_xui/xui_code_provider_test.c:128

## xuiCodeProviderSetCommandEnabled
- 位置: xui.h:6510  已注释: 否
- 签名: `XUI_API int xuiCodeProviderSetCommandEnabled(xui_code_provider_set pProviders, xui_code_command_enabled_proc onEnabled, void* pUser);`
- 实现: src/xui_code_provider.c:74（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1574; test_xui/xui_code_provider_test.c:132

## xuiCodeProviderRequestCompletion
- 位置: xui.h:6511  已注释: 否
- 签名: `XUI_API int xuiCodeProviderRequestCompletion(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, const char* sPrefix, xui_code_completion_item_t* pItems, int iItemCapacity, int* pItemCount);`
- 实现: src/xui_code_provider.c:82（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_provider_test.c:107; test_xui/xui_code_provider_test.c:112

## xuiCodeProviderRequestHover
- 位置: xui.h:6512  已注释: 否
- 签名: `XUI_API int xuiCodeProviderRequestHover(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, xui_code_hover_t* pHover);`
- 实现: src/xui_code_provider.c:90（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_provider_test.c:118

## xuiCodeProviderRequestSignature
- 位置: xui.h:6513  已注释: 否
- 签名: `XUI_API int xuiCodeProviderRequestSignature(xui_code_provider_set pProviders, xui_widget pWidget, int iOffset, xui_code_signature_help_t* pHelp);`
- 实现: src/xui_code_provider.c:99（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_provider_test.c:124

## xuiCodeProviderCanExecuteCommand
- 位置: xui.h:6514  已注释: 否
- 签名: `XUI_API int xuiCodeProviderCanExecuteCommand(xui_code_provider_set pProviders, xui_widget pWidget, int iCommand, int* pEnabled);`
- 实现: src/xui_code_provider.c:116（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_provider_test.c:130; test_xui/xui_code_provider_test.c:134; test_xui/xui_code_provider_test.c:140

## xuiCodeProviderExecuteCommand
- 位置: xui.h:6515  已注释: 否
- 签名: `XUI_API int xuiCodeProviderExecuteCommand(xui_code_provider_set pProviders, xui_widget pWidget, int iCommand, const void* pCommandData, int* pHandled);`
- 实现: src/xui_code_provider.c:108（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_provider_test.c:136; test_xui/xui_code_provider_test.c:142

## xuiCodeMarginModelCreate
- 位置: xui.h:6516  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelCreate(xui_code_margin_model* ppModel);`
- 实现: src/xui_code_margin.c:67（体 12 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:44

## xuiCodeMarginModelDestroy
- 位置: xui.h:6517  已注释: 否
- 签名: `XUI_API void xuiCodeMarginModelDestroy(xui_code_margin_model pModel);`
- 实现: src/xui_code_margin.c:80（体 6 行）
- 用法: test_xui/xui_code_margin_test.c:91

## xuiCodeMarginModelClear
- 位置: xui.h:6518  已注释: 否
- 签名: `XUI_API void xuiCodeMarginModelClear(xui_code_margin_model pModel);`
- 实现: src/xui_code_margin.c:87（体 5 行）
- 用法: test_xui/xui_code_margin_test.c:87

## xuiCodeMarginModelLoadDefaults
- 位置: xui.h:6519  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelLoadDefaults(xui_code_margin_model pModel, int bLineNumber, int bMarker, int bFold, int bDiagnostic);`
- 实现: src/xui_code_margin.c:93（体 32 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:46

## xuiCodeMarginModelAdd
- 位置: xui.h:6520  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelAdd(xui_code_margin_model pModel, const xui_code_margin_desc_t* pDesc);`
- 实现: src/xui_code_margin.c:126（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:612; test_xui/xui_code_margin_test.c:79

## xuiCodeMarginModelRemove
- 位置: xui.h:6521  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelRemove(xui_code_margin_model pModel, int iId);`
- 实现: src/xui_code_margin.c:146（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:83; test_xui/xui_code_margin_test.c:85

## xuiCodeMarginModelSetWidth
- 位置: xui.h:6522  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelSetWidth(xui_code_margin_model pModel, int iId, float fWidth);`
- 实现: src/xui_code_margin.c:158（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:62

## xuiCodeMarginModelSetVisible
- 位置: xui.h:6523  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelSetVisible(xui_code_margin_model pModel, int iId, int bVisible);`
- 实现: src/xui_code_margin.c:169（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:66

## xuiCodeMarginModelGetCount
- 位置: xui.h:6524  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelGetCount(xui_code_margin_model pModel);`
- 实现: src/xui_code_margin.c:181（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:688; test_xui/xui_code_margin_test.c:47; test_xui/xui_code_margin_test.c:81

## xuiCodeMarginModelGet
- 位置: xui.h:6525  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelGet(xui_code_margin_model pModel, int iIndex, xui_code_margin_info_t* pInfo);`
- 实现: src/xui_code_margin.c:186（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:690; test_xui/xui_code_margin_test.c:81

## xuiCodeMarginModelGetTotalWidth
- 位置: xui.h:6526  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelGetTotalWidth(xui_code_margin_model pModel, float* pWidth);`
- 实现: src/xui_code_margin.c:196（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:627; test_xui/xui_code_edit_test.c:692; test_xui/xui_code_edit_test.c:1731

## xuiCodeMarginModelGetRect
- 位置: xui.h:6527  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelGetRect(xui_code_margin_model pModel, int iId, xui_rect_t tViewport, xui_rect_t* pRect);`
- 实现: src/xui_code_margin.c:210（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:59

## xuiCodeMarginModelLayout
- 位置: xui.h:6528  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelLayout(xui_code_margin_model pModel, xui_rect_t tViewport, xui_code_margin_info_t* pInfos, int iInfoCapacity, int* pInfoCount, xui_rect_t* pTextRect);`
- 实现: src/xui_code_margin.c:236（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:50

## xuiCodeMarginModelHitTest
- 位置: xui.h:6529  已注释: 否
- 签名: `XUI_API int xuiCodeMarginModelHitTest(xui_code_margin_model pModel, xui_rect_t tViewport, float fX, float fY, float fLineHeight, float fScrollY, xui_code_margin_hit_t* pHit);`
- 实现: src/xui_code_margin.c:263（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_UNSUPPORTED, XUI_OK
- 用法: test_xui/xui_code_margin_test.c:56; test_xui/xui_code_margin_test.c:68

## xuiCodeSelectionCreate
- 位置: xui.h:6530  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionCreate(xui_code_selection_model* ppSelection);`
- 实现: src/xui_code_selection.c:122（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:200; test_xui/xui_code_editing_test.c:38; test_xui/xui_code_selection_test.c:34

## xuiCodeSelectionDestroy
- 位置: xui.h:6531  已注释: 否
- 签名: `XUI_API void xuiCodeSelectionDestroy(xui_code_selection_model pSelection);`
- 实现: src/xui_code_selection.c:136（体 6 行）
- 用法: test_xui/xui_code_command_test.c:293; test_xui/xui_code_editing_test.c:139; test_xui/xui_code_selection_test.c:187

## xuiCodeSelectionClear
- 位置: xui.h:6532  已注释: 否
- 签名: `XUI_API void xuiCodeSelectionClear(xui_code_selection_model pSelection);`
- 实现: src/xui_code_selection.c:143（体 7 行）
- 用法: test_xui/xui_code_selection_test.c:182

## xuiCodeSelectionSetRange
- 位置: xui.h:6533  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionSetRange(xui_code_selection_model pSelection, xui_code_document pDocument, int iAnchorOffset, int iCaretOffset);`
- 实现: src/xui_code_selection.c:151（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:254; test_xui/xui_code_command_test.c:270; test_xui/xui_code_command_test.c:277

## xuiCodeSelectionSetState
- 位置: xui.h:6534  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionSetState(xui_code_selection_model pSelection, xui_code_document pDocument, const xui_code_selection_t* pState);`
- 实现: src/xui_code_selection.c:166（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeSelectionGetRange
- 位置: xui.h:6535  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGetRange(xui_code_selection_model pSelection, int* pStart, int* pEnd);`
- 实现: src/xui_code_selection.c:186（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:371; test_xui/xui_code_editing_test.c:122; test_xui/xui_code_edit_test.c:945

## xuiCodeSelectionGetState
- 位置: xui.h:6536  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGetState(xui_code_selection_model pSelection, xui_code_selection_t* pState);`
- 实现: src/xui_code_selection.c:204（体 7 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:230; test_xui/xui_code_command_test.c:236; test_xui/xui_code_command_test.c:246

## xuiCodeSelectionHasSelection
- 位置: xui.h:6537  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionHasSelection(xui_code_selection_model pSelection);`
- 实现: src/xui_code_selection.c:212（体 11 行）
- 用法: test_xui/xui_code_selection_test.c:43; test_xui/xui_code_selection_test.c:97; test_xui/xui_code_selection_test.c:101

## xuiCodeSelectionGetCount
- 位置: xui.h:6538  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGetCount(xui_code_selection_model pSelection);`
- 实现: src/xui_code_selection.c:224（体 4 行）
- 用法: test_xui/xui_code_selection_test.c:44; test_xui/xui_code_selection_test.c:47; test_xui/xui_code_selection_test.c:57

## xuiCodeSelectionGetAt
- 位置: xui.h:6539  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGetAt(xui_code_selection_model pSelection, int iIndex, xui_code_selection_t* pState);`
- 实现: src/xui_code_selection.c:229（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_selection_test.c:49; test_xui/xui_code_selection_test.c:52

## xuiCodeSelectionAdd
- 位置: xui.h:6540  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionAdd(xui_code_selection_model pSelection, const xui_code_selection_t* pState);`
- 实现: src/xui_code_selection.c:239（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_selection_test.c:45

## xuiCodeSelectionSelectAll
- 位置: xui.h:6541  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionSelectAll(xui_code_selection_model pSelection, xui_code_document pDocument);`
- 实现: src/xui_code_selection.c:255（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_selection_test.c:107

## xuiCodeSelectionSelectWord
- 位置: xui.h:6542  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionSelectWord(xui_code_selection_model pSelection, xui_code_document pDocument, int iOffset);`
- 实现: src/xui_code_selection.c:261（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_selection_test.c:111; test_xui/xui_code_selection_test.c:115; test_xui/xui_code_selection_test.c:119

## xuiCodeSelectionSelectLine
- 位置: xui.h:6543  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionSelectLine(xui_code_selection_model pSelection, xui_code_document pDocument, int iLine, int bIncludeLineBreak);`
- 实现: src/xui_code_selection.c:276（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_selection_test.c:121; test_xui/xui_code_selection_test.c:125; test_xui/xui_code_selection_test.c:129

## xuiCodeSelectionGotoOffset
- 位置: xui.h:6544  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGotoOffset(xui_code_selection_model pSelection, xui_code_document pDocument, int iOffset, int bExtend);`
- 实现: src/xui_code_selection.c:300（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_codeedit/main.c:380; test_xui/xui_code_command_test.c:242; test_xui/xui_code_editing_test.c:43

## xuiCodeSelectionGotoLineColumn
- 位置: xui.h:6545  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionGotoLineColumn(xui_code_selection_model pSelection, xui_code_document pDocument, int iLine, int iColumn, int bExtend);`
- 实现: src/xui_code_selection.c:311（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:226; test_xui/xui_code_command_test.c:260; test_xui/xui_code_edit_test.c:921

## xuiCodeSelectionMove
- 位置: xui.h:6546  已注释: 否
- 签名: `XUI_API int xuiCodeSelectionMove(xui_code_selection_model pSelection, xui_code_document pDocument, int iCommand);`
- 实现: src/xui_code_selection.c:323（体 73 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_selection_test.c:58; test_xui/xui_code_selection_test.c:62; test_xui/xui_code_selection_test.c:66

## xuiCodeEditingInsertText
- 位置: xui.h:6547  已注释: 否
- 签名: `XUI_API int xuiCodeEditingInsertText(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sText, int bReadonly);`
- 实现: src/xui_code_editing.c:120（体 17 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_editing_test.c:45; test_xui/xui_code_editing_test.c:51; test_xui/xui_code_editing_test.c:53

## xuiCodeEditingDeleteBackward
- 位置: xui.h:6548  已注释: 否
- 签名: `XUI_API int xuiCodeEditingDeleteBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly);`
- 实现: src/xui_code_editing.c:138（体 18 行）
- 返回码: XUI_OK
- 用法: test_xui/xui_code_editing_test.c:58; test_xui/xui_code_editing_test.c:89; test_xui/xui_code_editing_test.c:101

## xuiCodeEditingDeleteForward
- 位置: xui.h:6549  已注释: 否
- 签名: `XUI_API int xuiCodeEditingDeleteForward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly);`
- 实现: src/xui_code_editing.c:157（体 20 行）
- 返回码: XUI_OK
- 用法: test_xui/xui_code_editing_test.c:60; test_xui/xui_code_editing_test.c:64; test_xui/xui_code_editing_test.c:95

## xuiCodeEditingDeleteWordBackward
- 位置: xui.h:6550  已注释: 否
- 签名: `XUI_API int xuiCodeEditingDeleteWordBackward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly);`
- 实现: src/xui_code_editing.c:178（体 24 行）
- 返回码: XUI_OK
- 用法: test_xui/xui_code_editing_test.c:70; test_xui/xui_code_editing_test.c:81

## xuiCodeEditingDeleteWordForward
- 位置: xui.h:6551  已注释: 否
- 签名: `XUI_API int xuiCodeEditingDeleteWordForward(xui_code_document pDocument, xui_code_selection_model pSelection, int bReadonly);`
- 实现: src/xui_code_editing.c:203（体 24 行）
- 返回码: XUI_OK
- 用法: test_xui/xui_code_editing_test.c:74

## xuiCodeEditingIndentSelection
- 位置: xui.h:6552  已注释: 否
- 签名: `XUI_API int xuiCodeEditingIndentSelection(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sIndent, int bReadonly);`
- 实现: src/xui_code_editing.c:228（体 36 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_editing_test.c:120

## xuiCodeEditingOutdentSelection
- 位置: xui.h:6553  已注释: 否
- 签名: `XUI_API int xuiCodeEditingOutdentSelection(xui_code_document pDocument, xui_code_selection_model pSelection, int iIndentColumns, int bReadonly);`
- 实现: src/xui_code_editing.c:265（体 50 行）
- 用法: test_xui/xui_code_editing_test.c:124

## xuiCodeEditingToggleLineComment
- 位置: xui.h:6554  已注释: 否
- 签名: `XUI_API int xuiCodeEditingToggleLineComment(xui_code_document pDocument, xui_code_selection_model pSelection, const char* sLineComment, int bReadonly);`
- 实现: src/xui_code_editing.c:339（体 54 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_editing_test.c:131; test_xui/xui_code_editing_test.c:133; test_xui/xui_code_editing_test.c:135

## xuiCodeThemeCreate
- 位置: xui.h:6555  已注释: 否
- 签名: `XUI_API int xuiCodeThemeCreate(xui_code_theme* ppTheme);`
- 实现: src/xui_code_theme.c:23（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1429; test_xui/xui_code_theme_test.c:26

## xuiCodeThemeDestroy
- 位置: xui.h:6556  已注释: 否
- 签名: `XUI_API void xuiCodeThemeDestroy(xui_code_theme pTheme);`
- 实现: src/xui_code_theme.c:42（体 4 行）
- 用法: test_xui/xui_code_edit_test.c:1972; test_xui/xui_code_theme_test.c:63

## xuiCodeThemeSetDefault
- 位置: xui.h:6557  已注释: 否
- 签名: `XUI_API int xuiCodeThemeSetDefault(xui_code_theme pTheme);`
- 实现: src/xui_code_theme.c:47（体 50 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeThemeSetStyle
- 位置: xui.h:6558  已注释: 否
- 签名: `XUI_API int xuiCodeThemeSetStyle(xui_code_theme pTheme, int iStyleId, const xui_code_style_t* pStyle);`
- 实现: src/xui_code_theme.c:98（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_edit_test.c:1397; test_xui/xui_code_edit_test.c:1434; test_xui/xui_code_theme_test.c:46

## xuiCodeThemeGetStyle
- 位置: xui.h:6559  已注释: 否
- 签名: `XUI_API int xuiCodeThemeGetStyle(xui_code_theme pTheme, int iStyleId, xui_code_style_t* pStyle);`
- 实现: src/xui_code_theme.c:108（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_theme_test.c:30

## xuiCodeThemeMapTokenKind
- 位置: xui.h:6560  已注释: 否
- 签名: `XUI_API int xuiCodeThemeMapTokenKind(xui_code_theme pTheme, int iTokenKind, int iStyleId);`
- 实现: src/xui_code_theme.c:118（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_theme_test.c:48; test_xui/xui_code_theme_test.c:59

## xuiCodeThemeGetTokenStyle
- 位置: xui.h:6561  已注释: 否
- 签名: `XUI_API int xuiCodeThemeGetTokenStyle(xui_code_theme pTheme, int iTokenKind, xui_code_style_t* pStyle);`
- 实现: src/xui_code_theme.c:127（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_code_theme_test.c:37; test_xui/xui_code_theme_test.c:51

## xuiCodeThemeCopy
- 位置: xui.h:6562  已注释: 否
- 签名: `XUI_API int xuiCodeThemeCopy(xui_code_theme pDst, xui_code_theme pSrc);`
- 实现: src/xui_code_theme.c:138（体 6 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiCodeCommandMapCreate
- 位置: xui.h:6563  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapCreate(xui_code_command_map* ppMap);`
- 实现: src/xui_code_command.c:26（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:140

## xuiCodeCommandMapDestroy
- 位置: xui.h:6564  已注释: 否
- 签名: `XUI_API void xuiCodeCommandMapDestroy(xui_code_command_map pMap);`
- 实现: src/xui_code_command.c:45（体 6 行）
- 用法: test_xui/xui_code_command_test.c:295

## xuiCodeCommandMapLoadDefaults
- 位置: xui.h:6565  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapLoadDefaults(xui_code_command_map pMap);`
- 实现: src/xui_code_command.c:120（体 49 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_code_command_test.c:193

## xuiCodeCommandMapBind
- 位置: xui.h:6566  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapBind(xui_code_command_map pMap, int iKey, uint32_t iModifiers, int iCommand);`
- 实现: src/xui_code_command.c:52（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:169; test_xui/xui_code_command_test.c:174; test_xui/xui_code_edit_test.c:1588

## xuiCodeCommandMapUnbind
- 位置: xui.h:6567  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapUnbind(xui_code_command_map pMap, int iKey, uint32_t iModifiers);`
- 实现: src/xui_code_command.c:76（体 14 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_command_test.c:185; test_xui/xui_code_command_test.c:190

## xuiCodeCommandMapFind
- 位置: xui.h:6568  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapFind(xui_code_command_map pMap, int iKey, uint32_t iModifiers, int* pCommand);`
- 实现: src/xui_code_command.c:91（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_command_test.c:146; test_xui/xui_code_command_test.c:148; test_xui/xui_code_command_test.c:150

## xuiCodeCommandMapGetCount
- 位置: xui.h:6569  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapGetCount(xui_code_command_map pMap);`
- 实现: src/xui_code_command.c:105（体 4 行）
- 用法: test_xui/xui_code_command_test.c:142; test_xui/xui_code_command_test.c:176; test_xui/xui_code_command_test.c:181

## xuiCodeCommandMapGetBinding
- 位置: xui.h:6570  已注释: 否
- 签名: `XUI_API int xuiCodeCommandMapGetBinding(xui_code_command_map pMap, int iIndex, xui_code_key_binding_t* pBinding);`
- 实现: src/xui_code_command.c:110（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_code_command_test.c:181

## xuiCodeCommandExecute
- 位置: xui.h:6571  已注释: 否
- 签名: `XUI_API int xuiCodeCommandExecute(const xui_code_command_context_t* pContext, int iCommand, int* pHandled);`
- 实现: src/xui_code_command.c:369（体 133 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK, XUI_ERROR_UNSUPPORTED
- 用法: test_xui/xui_code_command_test.c:222; test_xui/xui_code_command_test.c:228; test_xui/xui_code_command_test.c:234

