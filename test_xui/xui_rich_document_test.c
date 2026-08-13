#include "xui.h"

#include <stdio.h>
#include <string.h>

#define CHECK(e, m) do { if (!(e)) { printf("xui_rich_document_test failed: %s\n", m); failed = 1; goto cleanup; } } while (0)

static int collect_unique_ids(xui_rich_node node, xui_document_node_id_t* ids, int* count, int capacity)
{
	xui_rich_node child;
	xui_rich_node_info_t info;
	int i;
	if ( node == NULL || *count >= capacity ) return 0;
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	if ( xuiRichNodeGetInfo(node, &info) != XUI_OK ) return 0;
	for ( i = 0; i < *count; i++ ) if ( ids[i] == info.iId ) return 0;
	ids[(*count)++] = info.iId;
	for ( child = xuiRichNodeGetFirstChild(node); child != NULL; child = xuiRichNodeGetNextSibling(child) )
		if ( !collect_unique_ids(child, ids, count, capacity) ) return 0;
	return 1;
}

static xui_rich_node find_root_node_type(xui_rich_document document, int type)
{
	xui_rich_node node;
	for ( node = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(document)); node != NULL;
	      node = xuiRichNodeGetNextSibling(node) ) {
		xui_rich_node_info_t info;
		memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
		if ( xuiRichNodeGetInfo(node, &info) == XUI_OK && info.iType == type ) return node;
	}
	return NULL;
}

int main(void)
{
	xui_rich_document doc = NULL;
	xui_rich_node root;
	xui_rich_node paragraph;
	xui_rich_node run;
	xui_rich_node_info_t info;
	xui_rich_text_style_t red;
	xui_rich_text_style_t blue;
	xui_document_position_t pos;
	xui_rich_document fragmentDoc = NULL;
	xui_rich_document localDoc = NULL;
	xui_rich_document cellDoc = NULL;
	xui_rich_document loadedDoc = NULL;
	xui_rich_document unsupportedDoc = NULL;
	xui_rich_paragraph_style_t paragraphStyle;
	xui_rich_image_desc_t imageDesc;
	xui_rich_table_desc_t tableDesc;
	xui_rich_change_t change;
	xui_rich_node link;
	xui_rich_node table;
	xui_rich_node inlineImage;
	xui_document_node_id_t localNodeId;
	xui_document_node_id_t inlineImageId;
	xui_document_node_id_t tableId;
	xui_rich_node stableParagraph;
	xui_rich_node cellParagraph;
	xui_rich_text_style_t cellStyle;
	xui_document_node_id_t ids[512];
	int idCount = 0;
	char* serialized = NULL;
	size_t serializedSize = 0;
	int offset;
	int ret;
	int failed = 0;

	memset(&red, 0, sizeof(red)); red.iSize = sizeof(red); red.iTextColor = XUI_COLOR_RGBA(220, 40, 40, 255);
	memset(&blue, 0, sizeof(blue)); blue.iSize = sizeof(blue); blue.iTextColor = XUI_COLOR_RGBA(40, 90, 220, 255); blue.iFlags = XUI_RICH_STYLE_UNDERLINE;
	CHECK(xuiRichDocumentCreate(&doc) == XUI_OK && doc != NULL, "create");
	root = xuiRichDocumentGetRoot(doc);
	paragraph = xuiRichNodeGetFirstChild(root);
	CHECK(paragraph != NULL, "default paragraph");
	run = xuiRichDocumentAppendText(doc, paragraph, "hello", &red);
	CHECK(run != NULL && strcmp(xuiRichDocumentGetText(doc), "hello") == 0, "append text");
	CHECK(xuiRichDocumentReplace(doc, 5, 5, " world\nnext", &red) == XUI_OK, "multiline insert");
	memset(&change, 0, sizeof(change)); change.iSize = sizeof(change);
	CHECK(xuiRichDocumentGetLastChange(doc, &change) == XUI_OK && change.iKind == XUI_RICH_CHANGE_REPLACE &&
		change.iOldStart == 5 && change.iOldEnd == 5 && change.iNewStart == 5 && change.iNewEnd == 16 &&
		(change.iFlags & XUI_RICH_CHANGE_TEXT) != 0, "replace change descriptor");
	CHECK(strcmp(xuiRichDocumentGetText(doc), "hello world\nnext") == 0, "flatten multiline");
	CHECK(xuiRichDocumentApplyStyle(doc, 6, 11, &blue) == XUI_OK, "apply style");
	memset(&change, 0, sizeof(change)); change.iSize = sizeof(change);
	CHECK(xuiRichDocumentGetLastChange(doc, &change) == XUI_OK && change.iKind == XUI_RICH_CHANGE_STYLE &&
		change.iOldStart == 6 && change.iOldEnd == 11 && (change.iFlags & XUI_RICH_CHANGE_FORMAT) != 0,
		"style change descriptor");
	paragraph = xuiRichNodeGetFirstChild(root = xuiRichDocumentGetRoot(doc));
	for ( run = xuiRichNodeGetFirstChild(paragraph); run != NULL; run = xuiRichNodeGetNextSibling(run) ) {
		memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
		CHECK(xuiRichNodeGetInfo(run, &info) == XUI_OK, "node info");
		if ( info.sText != NULL && strcmp(info.sText, "world") == 0 ) break;
	}
	CHECK(run != NULL && info.tStyle.iTextColor == blue.iTextColor && (info.tStyle.iFlags & XUI_RICH_STYLE_UNDERLINE) != 0, "styled run");
	CHECK(xuiRichDocumentOffsetToPosition(doc, 8, &pos) == XUI_OK, "offset to position");
	CHECK(xuiRichDocumentPositionToOffset(doc, &pos, &offset) == XUI_OK && offset == 8, "position roundtrip");
	CHECK(xuiRichDocumentUndo(doc) == XUI_OK && strcmp(xuiRichDocumentGetText(doc), "hello world\nnext") == 0, "undo style preserves text");
	CHECK(xuiRichDocumentUndo(doc) == XUI_OK && strcmp(xuiRichDocumentGetText(doc), "hello") == 0, "undo insert");
	CHECK(xuiRichDocumentRedo(doc) == XUI_OK && strcmp(xuiRichDocumentGetText(doc), "hello world\nnext") == 0, "redo insert");
	CHECK(xuiRichDocumentReplace(doc, 0, 5, "\xe4\xbd\xa0\xe5\xa5\xbd", &blue) == XUI_OK, "unicode replace");
	CHECK(strncmp(xuiRichDocumentGetText(doc), "\xe4\xbd\xa0\xe5\xa5\xbd", 6) == 0, "unicode text");
	CHECK(xuiRichDocumentReplace(doc, 1, 1, "x", &blue) == XUI_OK, "clamped UTF-8 edit");
	CHECK(strncmp(xuiRichDocumentGetText(doc), "x\xe4\xbd\xa0\xe5\xa5\xbd", 7) == 0, "UTF-8 boundary clamp");
	CHECK(xuiRichDocumentClear(doc) == XUI_OK, "clear advanced document");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
	link = xuiRichDocumentAppendLink(doc, paragraph, "XUI", "https://xge.local/xui", &blue);
	CHECK(link != NULL, "append link");
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(link, &info) == XUI_OK && info.iType == XUI_RICH_NODE_LINK, "link node type");
	CHECK(xuiRichDocumentUpdateStyleFlags(doc, 0, 3, XUI_RICH_STYLE_BOLD, 0) == XUI_OK, "patch style flags");
	CHECK(xuiRichDocumentApplyLink(doc, 0, 2, "https://xge.local/partial") == XUI_OK, "apply partial link");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
	link = xuiRichNodeGetFirstChild(paragraph);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(link, &info) == XUI_OK && info.iType == XUI_RICH_NODE_LINK &&
		strcmp(info.sResource, "https://xge.local/partial") == 0 && (info.tStyle.iFlags & XUI_RICH_STYLE_BOLD), "format preserves link");
	memset(&paragraphStyle, 0, sizeof(paragraphStyle)); paragraphStyle.iSize = sizeof(paragraphStyle);
	paragraphStyle.iAlign = XUI_RICH_ALIGN_CENTER; paragraphStyle.iHeadingLevel = 2;
	CHECK(xuiRichDocumentApplyParagraphStyle(doc, 0, 3, &paragraphStyle) == XUI_OK, "paragraph style");
	CHECK(xuiRichDocumentSetBlockType(doc, 0, 3, XUI_RICH_NODE_HEADING) == XUI_OK, "heading block");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(paragraph, &info) == XUI_OK && info.iType == XUI_RICH_NODE_HEADING &&
		info.tParagraphStyle.iAlign == XUI_RICH_ALIGN_CENTER, "heading metadata");
	memset(&imageDesc, 0, sizeof(imageDesc)); imageDesc.iSize = sizeof(imageDesc);
	imageDesc.sSource = "asset://inline.svg"; imageDesc.sAltText = "inline";
	imageDesc.fWidth = 18; imageDesc.fHeight = 14; imageDesc.fBaseline = 11;
	inlineImage = xuiRichDocumentInsertInlineImage(doc, 3, 3, &imageDesc);
	CHECK(inlineImage != NULL && xuiRichDocumentGetLength(doc) == 6, "insert inline image");
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(inlineImage, &info) == XUI_OK && info.iType == XUI_RICH_NODE_INLINE_IMAGE &&
		info.fWidth == 18 && info.fHeight == 14 && info.fBaseline == 11 &&
		strcmp(info.sResource, "asset://inline.svg") == 0, "inline image metadata");
	inlineImageId = info.iId;
	CHECK(xuiRichDocumentUndo(doc) == XUI_OK && xuiRichDocumentGetLength(doc) == 3,
		"undo inline image insertion");
	CHECK(xuiRichDocumentRedo(doc) == XUI_OK && xuiRichDocumentGetLength(doc) == 6 &&
		xuiRichDocumentFindNode(doc, inlineImageId) != NULL, "redo inline image insertion");
	imageDesc.sSource = "asset://cover.svg"; imageDesc.sAltText = "cover";
	imageDesc.fWidth = 120; imageDesc.fHeight = 60; imageDesc.fBaseline = 0;
	CHECK(xuiRichDocumentInsertImage(doc, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc), &imageDesc) != NULL,
		"insert image");
	memset(&tableDesc, 0, sizeof(tableDesc)); tableDesc.iSize = sizeof(tableDesc); tableDesc.iRows = 2; tableDesc.iColumns = 2; tableDesc.fWidth = 240;
	table = xuiRichDocumentInsertTable(doc, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc), &tableDesc);
	CHECK(table != NULL, "insert table");
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(table, &info) == XUI_OK, "table node info"); tableId = info.iId;
	memset(&cellStyle, 0, sizeof(cellStyle)); cellStyle.iSize = sizeof(cellStyle); cellStyle.iFlags = XUI_RICH_STYLE_BOLD; cellStyle.iTextColor = red.iTextColor;
	CHECK(xuiRichTableSetCellText(doc, table, 0, 0, "Name", &cellStyle) == XUI_OK, "table cell text");
	memset(&cellStyle, 0, sizeof(cellStyle)); cellStyle.iSize = sizeof(cellStyle);
	CHECK(strcmp(xuiRichTableGetCellText(table, 0, 0), "Name") == 0 && xuiRichTableGetCellStyle(table, 0, 0, &cellStyle) == XUI_OK &&
		(cellStyle.iFlags & XUI_RICH_STYLE_BOLD), "table cell style");
	CHECK(xuiRichTableGetCellDocument(doc, table, 0, 1, &cellDoc) == XUI_OK && cellDoc != NULL,
		"table exposes a structured cell document");
	cellParagraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(cellDoc));
	CHECK(cellParagraph != NULL && xuiRichDocumentAppendText(cellDoc, cellParagraph, "Rich ", &red) != NULL &&
		xuiRichDocumentAppendLink(cellDoc, cellParagraph, "cell", "https://xge.local/cell", &blue) != NULL,
		"table cell accepts mixed rich nodes");
	CHECK(strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0,
		"table cell convenience text reflects structured content");
	memset(&change, 0, sizeof(change)); change.iSize = sizeof(change);
	CHECK(xuiRichDocumentGetLastChange(doc, &change) == XUI_OK &&
		change.iKind == XUI_RICH_CHANGE_RESOURCE && change.iNodeId != 0 &&
		(change.iFlags & XUI_RICH_CHANGE_GEOMETRY) != 0, "cell edits invalidate parent table geometry");
	CHECK(xuiRichTableResize(doc, table, 3, 4) == XUI_OK && strcmp(xuiRichTableGetCellText(table, 0, 0), "Name") == 0, "table resize preserves cells");
	CHECK(strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0,
		"table resize preserves structured cell documents");
	CHECK(xuiRichDocumentUndo(doc) == XUI_OK, "undo table resize");
	table = xuiRichDocumentFindNode(doc, tableId);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK && info.iRows == 2 && info.iColumns == 2 &&
		strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0, "undo restores deep-cloned cell documents");
	CHECK(xuiRichDocumentRedo(doc) == XUI_OK, "redo table resize");
	table = xuiRichDocumentFindNode(doc, tableId);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK && info.iRows == 3 && info.iColumns == 4 &&
		strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0, "redo restores resized structured table");
	CHECK(collect_unique_ids(xuiRichDocumentGetRoot(doc), ids, &idCount, 512),
		"document node ids remain unique after history and object insertion");
	CHECK(xuiRichDocumentInsertHorizontalRule(doc, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc)) != NULL, "insert rule");
	table = find_root_node_type(doc, XUI_RICH_NODE_TABLE);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK, "find current table before serialization");
	tableId = info.iId;
	CHECK(xuiRichDocumentSerialize(doc, 1, &serialized, &serializedSize) == XUI_OK && serialized != NULL &&
		serializedSize > 0 && strstr(serialized, "xui-rich-document") != NULL, "serialize native rich document");
	CHECK(xuiRichDocumentDeserialize(serialized, serializedSize, &loadedDoc) == XUI_OK && loadedDoc != NULL,
		"deserialize native rich document");
	CHECK(strcmp(xuiRichDocumentGetText(loadedDoc), xuiRichDocumentGetText(doc)) == 0,
		"native format preserves flat text and object positions");
	table = xuiRichDocumentFindNode(loadedDoc, tableId);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK && info.iRows == 3 && info.iColumns == 4 &&
		strcmp(xuiRichTableGetCellText(table, 0, 0), "Name") == 0 &&
		strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0,
		"native format preserves table metadata and rich cell documents");
	CHECK(xuiRichTableGetCellDocument(loadedDoc, table, 0, 1, &cellDoc) == XUI_OK && cellDoc != NULL,
		"loaded table exposes structured cell document");
	cellParagraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(cellDoc));
	link = xuiRichNodeGetNextSibling(xuiRichNodeGetFirstChild(cellParagraph));
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(link != NULL && xuiRichNodeGetInfo(link, &info) == XUI_OK && info.iType == XUI_RICH_NODE_LINK &&
		strcmp(info.sResource, "https://xge.local/cell") == 0 && info.tStyle.iTextColor == blue.iTextColor,
		"native format preserves nested link and text style");
	xuiRichDocumentFreeSerialized(serialized); serialized = NULL; serializedSize = 0;
	xuiRichDocumentDestroy(loadedDoc); loadedDoc = NULL;
	CHECK(xuiRichDocumentExportHtml(doc, &serialized, &serializedSize) == XUI_OK && serialized != NULL &&
		strstr(serialized, "<table><tbody>") != NULL && strstr(serialized, "https://xge.local/cell") != NULL &&
		strstr(serialized, "<img src=\"asset://cover.svg\"") != NULL && strstr(serialized, "<hr>") != NULL,
		"HTML export preserves semantic objects and nested rich cells");
	CHECK(xuiRichDocumentImportHtml(serialized, serializedSize, &loadedDoc) == XUI_OK && loadedDoc != NULL,
		"import exported HTML");
	CHECK(strcmp(xuiRichDocumentGetText(loadedDoc), xuiRichDocumentGetText(doc)) == 0,
		"HTML roundtrip preserves visible text and object positions");
	table = find_root_node_type(loadedDoc, XUI_RICH_NODE_TABLE);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK && info.iRows == 3 && info.iColumns == 4 &&
		strcmp(xuiRichTableGetCellText(table, 0, 0), "Name") == 0 &&
		strcmp(xuiRichTableGetCellText(table, 0, 1), "Rich cell") == 0,
		"HTML roundtrip preserves table and rich cell content");
	CHECK(find_root_node_type(loadedDoc, XUI_RICH_NODE_IMAGE) != NULL &&
		find_root_node_type(loadedDoc, XUI_RICH_NODE_HORIZONTAL_RULE) != NULL,
		"HTML roundtrip preserves block image and rule");
	xuiRichDocumentDestroy(loadedDoc); loadedDoc = NULL;
	xuiRichDocumentFreeSerialized(serialized); serialized = NULL; serializedSize = 0;
	CHECK(xuiRichDocumentImportHtml("<p>A &amp; B <strong>bold</strong></p><script>bad()</script>", 0,
		&loadedDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(loadedDoc), "A & B bold") == 0,
		"HTML import decodes entities, applies inline tags and skips scripts");
	xuiRichDocumentDestroy(loadedDoc); loadedDoc = NULL;
	CHECK(xuiRichDocumentImportHtml("<table><tr><td>A</td><td>B</td></tr><tr><td>C</td></tr></table>", 0,
		&loadedDoc) == XUI_ERROR_INVALID_ARGUMENT && loadedDoc == NULL,
		"HTML import rejects ragged tables without leaking partial documents");
	CHECK(xuiRichDocumentExportMarkdown(doc, &serialized, &serializedSize) == XUI_OK && serialized != NULL,
		"export Markdown");
	CHECK(
		strstr(serialized, "[cell](https://xge.local/cell)") != NULL &&
		strstr(serialized, "![cover](asset://cover.svg)") != NULL && strstr(serialized, "| --- |") != NULL,
		"Markdown export preserves links, images and GFM table structure");
	xuiRichDocumentFreeSerialized(serialized); serialized = NULL; serializedSize = 0;
	ret = xuiRichDocumentImportMarkdown(
		"# Heading\n\nParagraph **bold** *italic* ~~strike~~ `code` [link](https://example.test).\n\n"
		"- [x] done\n  - [ ] nested\n1. numbered\n\n![alt](asset://photo.svg)\n\n---\n\n"
		"| Name | Value |\n| --- | --- |\n| Alpha | [Beta](https://beta.test) |\n", 0, &loadedDoc);
	CHECK(ret == XUI_OK && loadedDoc != NULL, "import structured Markdown");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(loadedDoc));
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(paragraph != NULL && xuiRichNodeGetInfo(paragraph, &info) == XUI_OK && info.iType == XUI_RICH_NODE_HEADING &&
		info.tParagraphStyle.iHeadingLevel == 1, "Markdown heading semantics");
	paragraph = xuiRichNodeGetNextSibling(xuiRichNodeGetNextSibling(paragraph));
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(paragraph != NULL && xuiRichNodeGetInfo(paragraph, &info) == XUI_OK && info.iType == XUI_RICH_NODE_LIST_ITEM &&
		info.tParagraphStyle.iListType == XUI_RICH_LIST_CHECK && info.tParagraphStyle.bListChecked,
		"Markdown checked task semantics");
	paragraph = xuiRichNodeGetNextSibling(paragraph);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(paragraph != NULL && xuiRichNodeGetInfo(paragraph, &info) == XUI_OK && info.iType == XUI_RICH_NODE_LIST_ITEM &&
		info.tParagraphStyle.iListLevel == 1 && !info.tParagraphStyle.bListChecked,
		"Markdown nested unchecked task semantics");
	table = find_root_node_type(loadedDoc, XUI_RICH_NODE_TABLE);
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(table != NULL && xuiRichNodeGetInfo(table, &info) == XUI_OK && info.iRows == 2 && info.iColumns == 2 &&
		strcmp(xuiRichTableGetCellText(table, 1, 1), "Beta") == 0,
		"Markdown GFM table imports rich cell text");
	CHECK(find_root_node_type(loadedDoc, XUI_RICH_NODE_IMAGE) != NULL &&
		find_root_node_type(loadedDoc, XUI_RICH_NODE_HORIZONTAL_RULE) != NULL,
		"Markdown imports block image and rule");
	xuiRichDocumentDestroy(loadedDoc); loadedDoc = NULL;
	CHECK(xuiRichDocumentDeserialize("{broken", 7, &loadedDoc) == XUI_ERROR_INVALID_ARGUMENT && loadedDoc == NULL,
		"invalid JSON does not produce a partial document");
	CHECK(xuiRichDocumentDeserialize(
		"{\"format\":\"xui-rich-document\",\"version\":2,\"rootId\":1,\"nodes\":[]}", 0, &loadedDoc) ==
		XUI_ERROR_INVALID_ARGUMENT && loadedDoc == NULL, "unsupported native format version is rejected");
	CHECK(xuiRichDocumentDeserialize(
		"{\"format\":\"xui-rich-document\",\"version\":1,\"rootId\":1,\"nodes\":["
		"{\"id\":1,\"type\":\"paragraph\",\"paragraphStyle\":{\"align\":0,\"direction\":0,"
		"\"listType\":0,\"listLevel\":0,\"listChecked\":false,\"headingLevel\":0,\"indentLeft\":0,\"indentRight\":0,"
		"\"firstLineIndent\":0,\"lineHeight\":0,\"spaceBefore\":0,\"spaceAfter\":0,"
		"\"backgroundColor\":0,\"borderColor\":0},\"children\":[]}]}", 0, &loadedDoc) ==
		XUI_ERROR_INVALID_ARGUMENT && loadedDoc == NULL, "duplicate root and node ids are rejected");
	CHECK(xuiRichDocumentCreate(&unsupportedDoc) == XUI_OK, "unsupported serialization document create");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(unsupportedDoc));
	red.pFont = (xui_font)(uintptr_t)1;
	CHECK(xuiRichDocumentAppendText(unsupportedDoc, paragraph, "font", &red) != NULL &&
		xuiRichDocumentSerialize(unsupportedDoc, 0, &serialized, &serializedSize) == XUI_ERROR_UNSUPPORTED && serialized == NULL,
		"runtime font pointers are never serialized as addresses");
	red.pFont = NULL;
	CHECK(xuiRichDocumentCloneRange(doc, 0, xuiRichDocumentGetLength(doc), &fragmentDoc) == XUI_OK, "clone rich range");
	CHECK(xuiRichDocumentInsertDocument(doc, xuiRichDocumentGetLength(doc), xuiRichDocumentGetLength(doc), fragmentDoc) == XUI_OK, "paste rich range");
	CHECK(strstr(xuiRichDocumentGetText(doc), "XUI") != NULL, "rich paste text");
	CHECK(xuiRichDocumentBeginTransaction(doc) == XUI_OK, "begin transaction");
	CHECK(xuiRichDocumentReplace(doc, 0, 0, "A", &red) == XUI_OK, "transaction edit one");
	CHECK(xuiRichDocumentReplace(doc, 1, 1, "B", &red) == XUI_OK, "transaction edit two");
	CHECK(xuiRichDocumentEndTransaction(doc) == XUI_OK, "end transaction");
	memset(&change, 0, sizeof(change)); change.iSize = sizeof(change);
	CHECK(xuiRichDocumentGetLastChange(doc, &change) == XUI_OK && change.iOldStart == 0 && change.iNewStart == 0 &&
		change.iNewEnd >= 2 && (change.iFlags & XUI_RICH_CHANGE_TEXT) != 0, "transaction merged change");
	CHECK(strncmp(xuiRichDocumentGetText(doc), "AB", 2) == 0, "transaction result");
	CHECK(xuiRichDocumentUndo(doc) == XUI_OK && strncmp(xuiRichDocumentGetText(doc), "AB", 2) != 0, "transaction single undo");

	CHECK(xuiRichDocumentCreate(&localDoc) == XUI_OK, "local document create");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(localDoc));
	run = xuiRichDocumentAppendText(localDoc, paragraph, "abcdef", &red);
	CHECK(run != NULL, "local text run");
	memset(&info, 0, sizeof(info)); info.iSize = sizeof(info);
	CHECK(xuiRichNodeGetInfo(run, &info) == XUI_OK, "local node info");
	localNodeId = info.iId;
	CHECK(xuiRichDocumentReplace(localDoc, 3, 3, "X", &red) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "abcXdef") == 0, "local insert");
	CHECK(xuiRichDocumentFindNode(localDoc, localNodeId) == run, "local insert preserves node");
	CHECK(xuiRichDocumentReplace(localDoc, 1, 4, "YZ", &red) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "aYZdef") == 0, "local replace");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcXdef") == 0,
		"undo local replace");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcdef") == 0,
		"undo local insert");
	CHECK(xuiRichDocumentRedo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcXdef") == 0,
		"redo local insert");
	CHECK(xuiRichDocumentReplace(localDoc, 4, 4, "\nline", &red) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "abcX\nlinedef") == 0, "structural replace after local delta");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcXdef") == 0,
		"undo structural after local delta");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcdef") == 0,
		"undo delta after structural snapshot");
	CHECK(xuiRichDocumentRedo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcXdef") == 0,
		"redo delta before structural snapshot");
	CHECK(xuiRichDocumentRedo(localDoc) == XUI_OK && strcmp(xuiRichDocumentGetText(localDoc), "abcX\nlinedef") == 0,
		"redo structural after delta");
	stableParagraph = xuiRichDocumentAppendParagraph(localDoc);
	CHECK(stableParagraph != NULL && xuiRichDocumentAppendText(localDoc, stableParagraph, "stable", &red) != NULL,
		"stable trailing paragraph");
	CHECK(xuiRichDocumentReplace(localDoc, 2, 2, "\nnew", &red) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "ab\nnewcX\nlinedef\nstable") == 0, "block delta enter");
	CHECK(xuiRichNodeGetNextSibling(xuiRichNodeGetNextSibling(xuiRichNodeGetNextSibling(
		xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(localDoc))))) == stableParagraph,
		"block delta preserves trailing node address");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "abcX\nlinedef\nstable") == 0, "undo block delta enter");
	CHECK(xuiRichDocumentRedo(localDoc) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "ab\nnewcX\nlinedef\nstable") == 0, "redo block delta enter");
	CHECK(xuiRichDocumentReplace(localDoc, 1, 7, "Q", &red) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "aQX\nlinedef\nstable") == 0, "block delta cross paragraph replace");
	CHECK(xuiRichDocumentUndo(localDoc) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "ab\nnewcX\nlinedef\nstable") == 0, "undo cross paragraph replace");
	CHECK(xuiRichDocumentRedo(localDoc) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(localDoc), "aQX\nlinedef\nstable") == 0, "redo cross paragraph replace");

cleanup:
	xuiRichDocumentFreeSerialized(serialized);
	xuiRichDocumentDestroy(unsupportedDoc);
	xuiRichDocumentDestroy(loadedDoc);
	xuiRichDocumentDestroy(localDoc);
	xuiRichDocumentDestroy(fragmentDoc);
	xuiRichDocumentDestroy(doc);
	if (!failed) printf("xui_rich_document_test: all tests passed\n");
	return failed ? 1 : 0;
}
