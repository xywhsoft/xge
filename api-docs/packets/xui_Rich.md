# 草稿包：xui.h / Rich（58 条 API）

> 生成 2026-09-10 02:50 | revision 81f1054c3882
> 取证规则见 SPEC 第 5 节：错误码来自实现体；用法来自 examples/test；平台限制/桩状态见 SPEC 第 9 节。注释规范见 SPEC 第 4 节。

## xuiRichDocumentCreate
- 位置: xui.h:7105  已注释: 否
- 签名: `XUI_API int xuiRichDocumentCreate(xui_rich_document* ppDocument);`
- 实现: src/xui_rich_document.c:999（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: examples/xui_richedit/main.c:233; test_xui/xui_rich_document_test.c:73; test_xui/xui_rich_document_test.c:282

## xuiRichDocumentDestroy
- 位置: xui.h:7106  已注释: 否
- 签名: `XUI_API void xuiRichDocumentDestroy(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1018（体 17 行）
- 用法: examples/xui_richedit/main.c:257; test_xui/xui_rich_document_test.c:208; test_xui/xui_rich_document_test.c:226

## xuiRichDocumentBeginTransaction
- 位置: xui.h:7107  已注释: 否
- 签名: `XUI_API int xuiRichDocumentBeginTransaction(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1036（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:292

## xuiRichDocumentEndTransaction
- 位置: xui.h:7108  已注释: 否
- 签名: `XUI_API int xuiRichDocumentEndTransaction(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1047（体 20 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:295

## xuiRichDocumentClear
- 位置: xui.h:7109  已注释: 否
- 签名: `XUI_API int xuiRichDocumentClear(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1068（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:106

## xuiRichDocumentGetRoot
- 位置: xui.h:7110  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentGetRoot(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1088（体 4 行）
- 用法: examples/xui_richedit/main.c:234; test_xui/xui_rich_document_test.c:26; test_xui/xui_rich_document_test.c:74

## xuiRichDocumentAppendParagraph
- 位置: xui.h:7111  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendParagraph(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1093（体 4 行）
- 用法: examples/xui_richedit/main.c:240; examples/xui_richedit/main.c:250; test_xui/xui_rich_document_test.c:330

## xuiRichDocumentAppendBlock
- 位置: xui.h:7112  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendBlock(xui_rich_document pDocument, int iNodeType, const xui_rich_paragraph_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1098（体 11 行）
- 返回码: NULL
- 用法: test_xui/xui_style_rich_edit_test.c:78

## xuiRichDocumentAppendText
- 位置: xui.h:7113  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendText(xui_rich_document pDocument, xui_rich_node pParagraph, const char* sText, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1110（体 18 行）
- 返回码: NULL
- 用法: examples/xui_richedit/main.c:239; examples/xui_richedit/main.c:241; examples/xui_richedit/main.c:243

## xuiRichDocumentAppendLink
- 位置: xui.h:7114  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendLink(xui_rich_document pDocument, xui_rich_node pParagraph, const char* sText, const char* sUrl, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1129（体 21 行）
- 返回码: NULL
- 用法: examples/xui_richedit/main.c:244; test_xui/xui_rich_document_test.c:108; test_xui/xui_rich_document_test.c:159

## xuiRichDocumentAppendWidget
- 位置: xui.h:7115  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendWidget(xui_rich_document pDocument, xui_rich_node pParagraph, xui_widget pWidget, float fWidth, float fHeight, float fBaseline);`
- 实现: src/xui_rich_document.c:1151（体 14 行）
- 返回码: NULL
- 用法: examples/xui_richedit/main.c:242; test_xui/xui_rich_edit_scale_test.c:256; test_xui/xui_rich_edit_scale_test.c:286

## xuiRichDocumentAppendInlineImage
- 位置: xui.h:7116  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendInlineImage(xui_rich_document pDocument, xui_rich_node pParagraph, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1166（体 23 行）
- 返回码: NULL

## xuiRichDocumentAppendImage
- 位置: xui.h:7117  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendImage(xui_rich_document pDocument, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1190（体 17 行）
- 返回码: NULL
- 用法: examples/xui_richedit/main.c:249; test_xui/xui_style_rich_edit_test.c:81

## xuiRichDocumentAppendTable
- 位置: xui.h:7118  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendTable(xui_rich_document pDocument, const xui_rich_table_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1208（体 23 行）
- 返回码: NULL
- 用法: test_xui/xui_style_rich_edit_test.c:83

## xuiRichDocumentAppendHorizontalRule
- 位置: xui.h:7119  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentAppendHorizontalRule(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1232（体 10 行）
- 返回码: NULL
- 用法: test_xui/xui_style_rich_edit_test.c:85

## xuiRichDocumentInsertImage
- 位置: xui.h:7120  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentInsertImage(xui_rich_document pDocument, int iStart, int iEnd, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1297（体 18 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:143

## xuiRichDocumentInsertInlineImage
- 位置: xui.h:7121  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentInsertInlineImage(xui_rich_document pDocument, int iStart, int iEnd, const xui_rich_image_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1333（体 56 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:130

## xuiRichDocumentInsertTable
- 位置: xui.h:7122  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentInsertTable(xui_rich_document pDocument, int iStart, int iEnd, const xui_rich_table_desc_t* pDesc);`
- 实现: src/xui_rich_document.c:1390（体 20 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:146

## xuiRichDocumentInsertHorizontalRule
- 位置: xui.h:7123  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentInsertHorizontalRule(xui_rich_document pDocument, int iStart, int iEnd);`
- 实现: src/xui_rich_document.c:1411（体 11 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:182

## xuiRichDocumentFindNode
- 位置: xui.h:7124  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichDocumentFindNode(xui_rich_document pDocument, xui_document_node_id_t iNodeId);`
- 实现: src/xui_rich_document.c:1435（体 4 行）
- 用法: test_xui/xui_rich_document_test.c:140; test_xui/xui_rich_document_test.c:171; test_xui/xui_rich_document_test.c:176

## xuiRichNodeGetFirstChild
- 位置: xui.h:7125  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichNodeGetFirstChild(xui_rich_node pNode);`
- 实现: src/xui_rich_document.c:1440（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: examples/xui_richedit/main.c:234; test_xui/xui_rich_document_test.c:18; test_xui/xui_rich_document_test.c:26

## xuiRichNodeGetNextSibling
- 位置: xui.h:7126  已注释: 否
- 签名: `XUI_API xui_rich_node xuiRichNodeGetNextSibling(xui_rich_node pNode);`
- 实现: src/xui_rich_document.c:1441（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:18; test_xui/xui_rich_document_test.c:27; test_xui/xui_rich_document_test.c:91

## xuiRichNodeGetInfo
- 位置: xui.h:7127  已注释: 否
- 签名: `XUI_API int xuiRichNodeGetInfo(xui_rich_node pNode, xui_rich_node_info_t* pInfo);`
- 实现: src/xui_rich_document.c:1443（体 28 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:15; test_xui/xui_rich_document_test.c:30; test_xui/xui_rich_document_test.c:93

## xuiRichNodeSetText
- 位置: xui.h:7128  已注释: 否
- 签名: `XUI_API int xuiRichNodeSetText(xui_rich_document pDocument, xui_rich_node pNode, const char* sText);`
- 实现: src/xui_rich_document.c:1472（体 19 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiRichNodeSetStyle
- 位置: xui.h:7129  已注释: 否
- 签名: `XUI_API int xuiRichNodeSetStyle(xui_rich_document pDocument, xui_rich_node pNode, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1492（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichNodeSetParagraphStyle
- 位置: xui.h:7130  已注释: 否
- 签名: `XUI_API int xuiRichNodeSetParagraphStyle(xui_rich_document pDocument, xui_rich_node pNode, const xui_rich_paragraph_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1503（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_edit_scale_test.c:260; test_xui/xui_rich_edit_scale_test.c:279; test_xui/xui_rich_edit_scale_test.c:284

## xuiRichNodeSetResource
- 位置: xui.h:7131  已注释: 否
- 签名: `XUI_API int xuiRichNodeSetResource(xui_rich_document pDocument, xui_rich_node pNode, const char* sResource, const char* sAltText);`
- 实现: src/xui_rich_document.c:1515（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK

## xuiRichImageSetSurface
- 位置: xui.h:7132  已注释: 否
- 签名: `XUI_API int xuiRichImageSetSurface(xui_rich_document pDocument, xui_rich_node pImage, xui_surface pSurface, float fWidth, float fHeight);`
- 实现: src/xui_rich_document.c:1534（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichTableSetCellText
- 位置: xui.h:7133  已注释: 否
- 签名: `XUI_API int xuiRichTableSetCellText(xui_rich_document pDocument, xui_rich_node pTable, int iRow, int iColumn, const char* sText, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1548（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:151

## xuiRichTableGetCellText
- 位置: xui.h:7134  已注释: 否
- 签名: `XUI_API const char* xuiRichTableGetCellText(xui_rich_node pTable, int iRow, int iColumn);`
- 实现: src/xui_rich_document.c:1558（体 7 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:153; test_xui/xui_rich_document_test.c:161; test_xui/xui_rich_document_test.c:167

## xuiRichTableGetCellStyle
- 位置: xui.h:7135  已注释: 否
- 签名: `XUI_API int xuiRichTableGetCellStyle(xui_rich_node pTable, int iRow, int iColumn, xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1566（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:153

## xuiRichTableGetCellDocument
- 位置: xui.h:7136  已注释: 否
- 签名: `XUI_API int xuiRichTableGetCellDocument(xui_rich_document pDocument, xui_rich_node pTable, int iRow, int iColumn, xui_rich_document* ppCellDocument);`
- 实现: src/xui_rich_document.c:1585（体 24 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:155; test_xui/xui_rich_document_test.c:199; test_xui/xui_rich_edit_test.c:609

## xuiRichTableResize
- 位置: xui.h:7138  已注释: 否
- 签名: `XUI_API int xuiRichTableResize(xui_rich_document pDocument, xui_rich_node pTable, int iRows, int iColumns);`
- 实现: src/xui_rich_document.c:1610（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:167

## xuiRichDocumentGetLength
- 位置: xui.h:7139  已注释: 否
- 签名: `XUI_API int xuiRichDocumentGetLength(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1641（体 5 行）
- 用法: test_xui/xui_rich_document_test.c:131; test_xui/xui_rich_document_test.c:137; test_xui/xui_rich_document_test.c:139

## xuiRichDocumentGetText
- 位置: xui.h:7140  已注释: 否
- 签名: `XUI_API const char* xuiRichDocumentGetText(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:1647（体 5 行）
- 返回码: NULL
- 用法: test_xui/xui_rich_document_test.c:78; test_xui/xui_rich_document_test.c:84; test_xui/xui_rich_document_test.c:99

## xuiRichDocumentReplace
- 位置: xui.h:7141  已注释: 否
- 签名: `XUI_API int xuiRichDocumentReplace(xui_rich_document pDocument, int iStart, int iEnd, const char* sText, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1653（体 37 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:79; test_xui/xui_rich_document_test.c:102; test_xui/xui_rich_document_test.c:104

## xuiRichDocumentApplyStyle
- 位置: xui.h:7142  已注释: 否
- 签名: `XUI_API int xuiRichDocumentApplyStyle(xui_rich_document pDocument, int iStart, int iEnd, const xui_rich_text_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1691（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:85; test_xui/xui_rich_edit_test.c:659

## xuiRichDocumentUpdateStyleFlags
- 位置: xui.h:7143  已注释: 否
- 签名: `XUI_API int xuiRichDocumentUpdateStyleFlags(xui_rich_document pDocument, int iStart, int iEnd, uint32_t iSetFlags, uint32_t iClearFlags);`
- 实现: src/xui_rich_document.c:1718（体 23 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:112

## xuiRichDocumentApplyLink
- 位置: xui.h:7144  已注释: 否
- 签名: `XUI_API int xuiRichDocumentApplyLink(xui_rich_document pDocument, int iStart, int iEnd, const char* sUrl);`
- 实现: src/xui_rich_document.c:1742（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:113

## xuiRichDocumentApplyParagraphStyle
- 位置: xui.h:7145  已注释: 否
- 签名: `XUI_API int xuiRichDocumentApplyParagraphStyle(xui_rich_document pDocument, int iStart, int iEnd, const xui_rich_paragraph_style_t* pStyle);`
- 实现: src/xui_rich_document.c:1765（体 29 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:121

## xuiRichDocumentSetBlockType
- 位置: xui.h:7146  已注释: 否
- 签名: `XUI_API int xuiRichDocumentSetBlockType(xui_rich_document pDocument, int iStart, int iEnd, int iNodeType);`
- 实现: src/xui_rich_document.c:1795（体 31 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:122

## xuiRichDocumentCloneRange
- 位置: xui.h:7147  已注释: 否
- 签名: `XUI_API int xuiRichDocumentCloneRange(xui_rich_document pDocument, int iStart, int iEnd, xui_rich_document* ppFragment);`
- 实现: src/xui_rich_document.c:1827（体 26 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:289

## xuiRichDocumentInsertDocument
- 位置: xui.h:7148  已注释: 否
- 签名: `XUI_API int xuiRichDocumentInsertDocument(xui_rich_document pDocument, int iStart, int iEnd, xui_rich_document pFragment);`
- 实现: src/xui_rich_document.c:1854（体 30 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:290

## xuiRichDocumentOffsetToPosition
- 位置: xui.h:7149  已注释: 否
- 签名: `XUI_API int xuiRichDocumentOffsetToPosition(xui_rich_document pDocument, int iOffset, xui_document_position_t* pPosition);`
- 实现: src/xui_rich_document.c:1907（体 47 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:97; test_xui/xui_rich_edit_test.c:54

## xuiRichDocumentPositionToOffset
- 位置: xui.h:7150  已注释: 否
- 签名: `XUI_API int xuiRichDocumentPositionToOffset(xui_rich_document pDocument, const xui_document_position_t* pPosition, int* pOffset);`
- 实现: src/xui_rich_document.c:1955（体 15 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_FILE_NOT_FOUND, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:98

## xuiRichDocumentUndo
- 位置: xui.h:7151  已注释: 否
- 签名: `XUI_API int xuiRichDocumentUndo(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:2086（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_document_test.c:99; test_xui/xui_rich_document_test.c:100; test_xui/xui_rich_document_test.c:137

## xuiRichDocumentRedo
- 位置: xui.h:7152  已注释: 否
- 签名: `XUI_API int xuiRichDocumentRedo(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:2092（体 5 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_document_test.c:101; test_xui/xui_rich_document_test.c:139; test_xui/xui_rich_document_test.c:175

## xuiRichDocumentCanUndo
- 位置: xui.h:7153  已注释: 否
- 签名: `XUI_API int xuiRichDocumentCanUndo(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:2098（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichDocumentCanRedo
- 位置: xui.h:7154  已注释: 否
- 签名: `XUI_API int xuiRichDocumentCanRedo(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:2099（体 10 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK

## xuiRichDocumentGetVersion
- 位置: xui.h:7155  已注释: 否
- 签名: `XUI_API uint32_t xuiRichDocumentGetVersion(xui_rich_document pDocument);`
- 实现: src/xui_rich_document.c:2100（体 9 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_style_rich_edit_test.c:93; test_xui/xui_style_rich_edit_test.c:99; test_xui/xui_style_rich_edit_test.c:124

## xuiRichDocumentGetLastChange
- 位置: xui.h:7156  已注释: 否
- 签名: `XUI_API int xuiRichDocumentGetLastChange(xui_rich_document pDocument, xui_rich_change_t* pChange);`
- 实现: src/xui_rich_document.c:2101（体 8 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:81; test_xui/xui_rich_document_test.c:87; test_xui/xui_rich_document_test.c:164

## xuiRichDocumentSerialize
- 位置: xui.h:7157  已注释: 否
- 签名: `XUI_API int xuiRichDocumentSerialize(xui_rich_document pDocument, int bPretty, char** ppText, size_t* pSize);`
- 实现: src/xui_rich_document.c:2321（体 25 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY
- 用法: test_xui/xui_rich_document_test.c:187; test_xui/xui_rich_document_test.c:286

## xuiRichDocumentDeserialize
- 位置: xui.h:7158  已注释: 否
- 签名: `XUI_API int xuiRichDocumentDeserialize(const char* sText, size_t iSize, xui_rich_document* ppDocument);`
- 实现: src/xui_rich_document.c:2586（体 18 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:189; test_xui/xui_rich_document_test.c:270; test_xui/xui_rich_document_test.c:272

## xuiRichDocumentExportHtml
- 位置: xui.h:7159  已注释: 否
- 签名: `XUI_API int xuiRichDocumentExportHtml(xui_rich_document pDocument, char** ppText, size_t* pSize);`
- 实现: src/xui_rich_document.c:2852（体 13 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_document_test.c:209

## xuiRichDocumentImportHtml
- 位置: xui.h:7160  已注释: 否
- 签名: `XUI_API int xuiRichDocumentImportHtml(const char* sText, size_t iSize, xui_rich_document* ppDocument);`
- 实现: src/xui_rich_document.c:3561（体 22 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:213; test_xui/xui_rich_document_test.c:228; test_xui/xui_rich_document_test.c:232

## xuiRichDocumentExportMarkdown
- 位置: xui.h:7161  已注释: 否
- 签名: `XUI_API int xuiRichDocumentExportMarkdown(xui_rich_document pDocument, char** ppText, size_t* pSize);`
- 实现: src/xui_rich_document.c:3705（体 11 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT
- 用法: test_xui/xui_rich_document_test.c:235

## xuiRichDocumentImportMarkdown
- 位置: xui.h:7162  已注释: 否
- 签名: `XUI_API int xuiRichDocumentImportMarkdown(const char* sText, size_t iSize, xui_rich_document* ppDocument);`
- 实现: src/xui_rich_document.c:3977（体 93 行）
- 返回码: XUI_ERROR_INVALID_ARGUMENT, XUI_ERROR_OUT_OF_MEMORY, XUI_OK
- 用法: test_xui/xui_rich_document_test.c:242

## xuiRichDocumentFreeSerialized
- 位置: xui.h:7163  已注释: 否
- 签名: `XUI_API void xuiRichDocumentFreeSerialized(char* sText);`
- 实现: src/xui_rich_document.c:2347（体 4 行）
- 用法: test_xui/xui_rich_document_test.c:207; test_xui/xui_rich_document_test.c:227; test_xui/xui_rich_document_test.c:241

