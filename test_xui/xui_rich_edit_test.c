#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define CHECK(e, m) do { if (!(e)) { printf("xui_rich_edit_test failed: %s\n", m); failed = 1; goto cleanup; } } while (0)

static int g_link_clicks;
static int g_selection_events;
static int g_scroll_events;
static int g_object_events;
static xui_document_node_id_t g_object_node_id;
static int g_zoom_events;
static int g_accessibility_events;
static int g_accessibility_last_type;
static uint32_t g_accessibility_last_revision;

static void on_link_click(xui_widget widget, xui_document_node_id_t nodeId, const char* url, void* user)
{
	(void)widget; (void)nodeId; (void)user;
	if ( url != NULL && strcmp(url, "https://xge.local/rich") == 0 ) g_link_clicks++;
}

static void on_rich_event(xui_widget widget, const xui_rich_edit_event_t* event, void* user)
{
	(void)widget; (void)user;
	if ( event == NULL ) return;
	if ( event->iType == XUI_RICH_EDIT_EVENT_SELECTION_CHANGED ) g_selection_events++;
	else if ( event->iType == XUI_RICH_EDIT_EVENT_SCROLL_CHANGED ) g_scroll_events++;
	else if ( event->iType == XUI_RICH_EDIT_EVENT_OBJECT_ACTIVATE ) {
		g_object_events++;
		g_object_node_id = event->iNodeId;
	}
	else if ( event->iType == XUI_RICH_EDIT_EVENT_ZOOM_CHANGED ) g_zoom_events++;
}

static void on_accessibility_event(xui_context context, xui_widget widget,
	const xui_accessibility_event_t* event, void* user)
{
	(void)context; (void)widget; (void)user;
	if ( event == NULL ) return;
	g_accessibility_events++;
	g_accessibility_last_type = event->iType;
	g_accessibility_last_revision = event->iRevision;
}

static int block_info_at(xui_rich_document document, int offset, xui_rich_node_info_t* info)
{
	xui_document_position_t position;
	xui_rich_node node;
	xui_rich_node_info_t nodeInfo;
	if ( xuiRichDocumentOffsetToPosition(document, offset, &position) != XUI_OK ) return 0;
	node = xuiRichDocumentFindNode(document, position.iNodeId);
	memset(&nodeInfo, 0, sizeof(nodeInfo)); nodeInfo.iSize = sizeof(nodeInfo);
	if ( node == NULL || xuiRichNodeGetInfo(node, &nodeInfo) != XUI_OK ) return 0;
	if ( nodeInfo.iType == XUI_RICH_NODE_TEXT || nodeInfo.iType == XUI_RICH_NODE_LINK ||
	     nodeInfo.iType == XUI_RICH_NODE_INLINE_WIDGET || nodeInfo.iType == XUI_RICH_NODE_INLINE_IMAGE )
		node = xuiRichDocumentFindNode(document, nodeInfo.iParentId);
	memset(info, 0, sizeof(*info)); info->iSize = sizeof(*info);
	return node != NULL && xuiRichNodeGetInfo(node, info) == XUI_OK;
}

int main(void)
{
	xui_test_proxy_state_t proxy;
	xui_context context = NULL;
	xui_surface target = NULL;
	xui_surface editCache = NULL;
	xui_surface buttonCache = NULL;
	xui_surface imageSurface = NULL;
	xui_widget root = NULL;
	xui_widget edit = NULL;
	xui_widget menu = NULL;
	xui_widget button = NULL;
	xui_widget cellEditor = NULL;
	xui_widget findEdit = NULL;
	xui_widget toolbar = NULL;
	xui_font font = NULL;
	xui_rich_document doc = NULL;
	xui_rich_document cellDocument = NULL;
	xui_rich_document findDocument = NULL;
	xui_rich_node paragraph;
	xui_rich_node tableNode;
	xui_rich_node_info_t tableInfo;
	xui_rich_text_style_t red;
	xui_rich_text_style_t accent;
	xui_rich_edit_desc_t desc;
	xui_rich_image_desc_t imageDesc;
	xui_rich_table_desc_t tableDesc;
	xui_rich_font_set_t fontSet;
	xui_find_options_t findOptions;
	xui_find_result_t findResult;
	xui_rich_paragraph_style_t paragraphStyle;
	xui_button_desc_t buttonDesc;
	xui_toolbar_desc_t toolbarDesc;
	xui_rect_i_t damage = {0,0,520,320};
	xui_rich_fragment_t fragment;
	xui_rich_fragment_t dragFragment;
	xui_rich_fragment_t tableFragment;
	xui_accessible_node_t accessible;
	xui_accessible_selection_t accessibleSelection;
	xui_event_t event;
	xui_rect_t editWorld;
	xui_rect_t buttonRect;
	uint32_t selectionColor = XUI_COLOR_RGBA(210,60,170,110);
	uint32_t cursorColor = XUI_COLOR_RGBA(20,180,120,255);
	int start;
	int end;
	int widgetStart = -1;
	int widgetEnd = -1;
	int inlineImageStart = -1;
	int inlineImageEnd = -1;
	int linkFragment = -1;
	int linkLine = -1;
	int linkLineEnd = -1;
	float linkLineY = 0.0f;
	float lastBottom = 0.0f;
	float scrollX = 0.0f;
	float scrollY = 0.0f;
	float maxScrollX = 0.0f;
	float maxScrollY = 0.0f;
	xui_scroll_model_t* scrollModel;
	int i;
	int failed = 0;
	int ret;
	int commandState;
	int secondParagraphStart;
	int thirdParagraphStart;
	int firstParagraphEnd;
	int followingFragment = -1;
	xui_document_node_id_t followingNodeId = 0;
	xui_document_node_id_t tableId = 0;
	uint64_t accessibleLinkId = 0;
	uint64_t accessibleImageId = 0;
	uint64_t accessibleTableId = 0;
	uint64_t accessibleCellId = 0;
	uint64_t accessibleCheckId = 0;
	uint64_t accessibleWidgetId = 0;
	int accessibleCount;
	int followingStart;
	int beforePlainPasteLength;
	int replaceCount;
	float followingY;

	xuiTestProxyInit(&proxy);
	CHECK(xuiCreate(&context) == XUI_OK, "context");
	CHECK(xuiSetProxy(context, &proxy.tProxy) == XUI_OK, "proxy");
	CHECK(xuiSetAccessibilityEventCallback(context, on_accessibility_event, NULL) == XUI_OK,
		"accessibility event callback");
	CHECK(proxy.tProxy.fontLoadFile(&proxy.tProxy, &font, "test.ttf", 14.0f, 0) == XUI_OK, "font");
	CHECK(xuiSetDefaultFont(context, font) == XUI_OK, "default font");
	CHECK(xuiInputViewport(context, 520.0f, 320.0f) == XUI_OK, "viewport");
	CHECK(xuiTestSurfaceCreate(&proxy, &target, 520, 320, XUI_SURFACE_USAGE_TARGET) == XUI_OK, "target");
	CHECK(xuiWidgetCreate(context, &root) == XUI_OK, "root");
	CHECK(xuiSetRootWidget(context, root) == XUI_OK, "set root");
	CHECK(xuiWidgetSetRect(root, (xui_rect_t){0,0,520,320}) == XUI_OK, "root rect");
	memset(&buttonDesc, 0, sizeof(buttonDesc)); buttonDesc.iSize = sizeof(buttonDesc); buttonDesc.sText = "Action";
	CHECK(xuiButtonCreate(context, &button, &buttonDesc) == XUI_OK, "inline button");
	CHECK(xuiRichDocumentCreate(&doc) == XUI_OK, "document");
	paragraph = xuiRichNodeGetFirstChild(xuiRichDocumentGetRoot(doc));
	memset(&red, 0, sizeof(red)); red.iSize = sizeof(red); red.iTextColor = XUI_COLOR_RGBA(190,40,40,255);
	memset(&accent, 0, sizeof(accent)); accent.iSize = sizeof(accent); accent.iTextColor = XUI_COLOR_RGBA(30,90,210,255); accent.iFlags = XUI_RICH_STYLE_UNDERLINE;
	CHECK(xuiRichDocumentAppendText(doc, paragraph, "Rich text ", &red) != NULL, "first run");
	CHECK(xuiRichDocumentAppendWidget(doc, paragraph, button, 72.0f, 24.0f, 18.0f) != NULL, "widget run");
	CHECK(xuiRichDocumentAppendText(doc, paragraph, " mixes controls and styled UTF-8: \xe4\xbd\xa0\xe5\xa5\xbd", &accent) != NULL, "second run");
	paragraph = xuiRichDocumentAppendParagraph(doc);
	CHECK(xuiRichDocumentAppendText(doc, paragraph, "A short line with a ", &red) != NULL, "short line");
	CHECK(xuiRichDocumentAppendLink(doc, paragraph, "link", "https://xge.local/rich", &accent) != NULL, "link run");
	paragraph = xuiRichDocumentAppendParagraph(doc);
	CHECK(xuiRichDocumentAppendText(doc, paragraph, "Following line", &red) != NULL, "following line");
	memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc); desc.pDocument = doc; desc.pFont = font; desc.bOwnDocument = 1; desc.bWordWrap = 1;
	desc.iSelectionColor = selectionColor; desc.iCursorColor = cursorColor;
	ret = xuiRichEditCreate(context, &edit, &desc);
	if ( ret != XUI_OK ) printf("xuiRichEditCreate returned %d\n", ret);
	CHECK(ret == XUI_OK, "editor"); doc = NULL;
	CHECK(xuiWidgetSetRect(edit, (xui_rect_t){20,20,260,180}) == XUI_OK, "editor rect");
	CHECK(xuiWidgetAddChild(root, edit) == XUI_OK, "attach editor");
	CHECK(xuiRichEditGetWordWrap(edit) == 1, "rich editor initial word wrap");
	CHECK(xuiRichEditSetWordWrap(edit, 0) == XUI_OK && xuiRichEditGetWordWrap(edit) == 0,
		"rich editor disables word wrap at runtime");
	CHECK(xuiRichEditSetWordWrap(edit, 1) == XUI_OK && xuiRichEditGetWordWrap(edit) == 1,
		"rich editor restores word wrap at runtime");
	CHECK(xuiRichEditSetEvent(edit, on_rich_event, NULL) == XUI_OK, "rich event callback");
	menu = xuiRichEditGetMenuWidget(edit);
	CHECK(menu != NULL && xuiMenuGetItemCount(menu) == 12, "built-in rich edit menu includes find and replace");
	CHECK(xuiLayout(context) == XUI_OK, "layout");
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK && g_selection_events == 1,
		"initial rich selection event is reported once");
	CHECK(xuiRichEditGetFragmentCount(edit) > 10, "flow fragments");
	CHECK(xuiRichEditGetFragment(edit, 0, &fragment) == XUI_OK && fragment.iDocumentEnd > fragment.iDocumentStart, "fragment info");
	{
		xui_rect_t beforeZoom = fragment.tRect;
		CHECK(xuiRichEditSetZoom(edit, 2.0f) == XUI_OK && xuiRichEditGetZoom(edit) == 2.0f,
			"rich editor accepts real view zoom");
		CHECK(xuiLayout(context) == XUI_OK && xuiRichEditGetFragment(edit, 0, &fragment) == XUI_OK &&
			fragment.tRect.fH > beforeZoom.fH * 1.8f, "zoom changes shaped text geometry");
		CHECK(xuiUpdate(context, 0.016f) == XUI_OK && g_zoom_events > 0,
			"zoom changes use the unified rich event contract");
		CHECK(xuiRichEditSetZoom(edit, 1.0f) == XUI_OK && xuiLayout(context) == XUI_OK,
			"rich editor zoom reset");
	}
	CHECK(xuiRichEditGetFragment(edit, 4, &dragFragment) == XUI_OK, "drag fragment info");
	CHECK(xuiWidgetGetParent(button) == edit, "widget hosted");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "initial fragment scan");
		if ( fragment.iNodeType == XUI_RICH_NODE_INLINE_WIDGET ) {
			buttonRect = xuiWidgetGetRect(button);
			CHECK(buttonRect.fX >= fragment.tRect.fX + 8.0f && buttonRect.fX <= fragment.tRect.fX + 10.0f,
				"inline widget uses editor-local content offset");
		}
		if ( fragment.iNodeType == XUI_RICH_NODE_LINK ) {
			linkFragment = i;
			linkLine = fragment.iLine;
			linkLineY = fragment.tRect.fY + fragment.tRect.fH * 0.5f;
		}
		if ( fragment.tRect.fY + fragment.tRect.fH > lastBottom ) lastBottom = fragment.tRect.fY + fragment.tRect.fH;
	}
	CHECK(linkFragment >= 0, "link fragment found");
	secondParagraphStart = (int)(strstr(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "A short line") -
		xuiRichDocumentGetText(xuiRichEditGetDocument(edit)));
	thirdParagraphStart = (int)(strstr(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "Following line") -
		xuiRichDocumentGetText(xuiRichEditGetDocument(edit)));
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "find following fragment");
		if ( fragment.iDocumentStart == thirdParagraphStart ) {
			followingFragment = i; followingNodeId = fragment.iNodeId;
			followingStart = fragment.iDocumentStart; followingY = fragment.tRect.fY; break;
		}
	}
	CHECK(followingFragment >= 0, "following paragraph fragment found");
	CHECK(xuiRichEditSetSelection(edit, secondParagraphStart + 2, secondParagraphStart + 2) == XUI_OK,
		"incremental caret");
	CHECK(xuiRichEditInsertText(edit, "X") == XUI_OK, "incremental paragraph insert");
	followingFragment = -1;
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "find following fragment after insert");
		if ( fragment.iNodeId == followingNodeId ) { followingFragment = i; break; }
	}
	CHECK(followingFragment >= 0 && fragment.iDocumentStart == followingStart + 1 && fragment.tRect.fY == followingY,
		"incremental layout shifts following document offsets");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "incremental insert undo");
	followingFragment = -1;
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "find following fragment after undo");
		if ( fragment.iNodeId == followingNodeId ) { followingFragment = i; break; }
	}
	CHECK(followingFragment >= 0 && fragment.iDocumentStart == followingStart && fragment.tRect.fY == followingY,
		"incremental undo restores following geometry");
	CHECK(xuiRichEditSetSelection(edit, secondParagraphStart + 5, secondParagraphStart + 5) == XUI_OK,
		"structural incremental caret");
	CHECK(xuiRichEditInsertText(edit, "\nnew") == XUI_OK, "structural incremental enter");
	followingFragment = -1;
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "find following fragment after enter");
		if ( fragment.iNodeId == followingNodeId ) { followingFragment = i; break; }
	}
	CHECK(followingFragment >= 0 && fragment.iDocumentStart == followingStart + 4 && fragment.tRect.fY > followingY,
		"structural incremental layout shifts following geometry");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "structural incremental undo");
	followingFragment = -1;
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "find following fragment after structural undo");
		if ( fragment.iNodeId == followingNodeId ) { followingFragment = i; break; }
	}
	CHECK(followingFragment >= 0 && fragment.iDocumentStart == followingStart && fragment.tRect.fY == followingY,
		"structural incremental undo restores geometry");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOUBLE_CLICK;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + fragment.tRect.fX + 1.0f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + fragment.tRect.fY + fragment.tRect.fH * 0.5f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "double click word dispatch");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start == followingStart && end == followingStart + 9,
		"double click selects a complete word");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "link line scan");
		if ( fragment.iLine == linkLine && fragment.iDocumentEnd > linkLineEnd ) linkLineEnd = fragment.iDocumentEnd;
	}
	CHECK(xuiRender(context, target, &damage, 1) == XUI_OK, "render");
	editCache = xuiWidgetGetCacheSurface(edit, xuiWidgetGetStateId(edit));
	CHECK(editCache != NULL && xuiTestSurfaceGetTextDrawCount(editCache) > 0, "text rendered");
	CHECK(xuiTestSurfaceGetTextDrawCount(editCache) < xuiRichEditGetFragmentCount(edit),
		"visual text runs are batched instead of drawing every cluster");
	buttonCache = xuiWidgetGetCacheSurface(button, xuiWidgetGetStateId(button));
	CHECK(buttonCache != NULL && xuiTestSurfaceGetTextDrawCount(buttonCache) > 0, "inline widget rendered");
	editWorld = xuiWidgetGetWorldRect(edit);
	CHECK(xuiInputPointerDown(context, editWorld.fX + 250.0f, editWorld.fY + 10.0f + linkLineY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "line blank down");
	CHECK(xuiInputPointerUp(context, editWorld.fX + 250.0f, editWorld.fY + 10.0f + linkLineY,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "line blank up");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK, "line blank dispatch");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && end == linkLineEnd,
		"horizontal blank clamps to selected visual line");
	CHECK(xuiInputPointerDown(context, editWorld.fX + 12.0f, editWorld.fY + 10.0f + lastBottom + 24.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "below document down");
	CHECK(xuiInputPointerUp(context, editWorld.fX + 12.0f, editWorld.fY + 10.0f + lastBottom + 24.0f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "below document up");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK, "below document dispatch");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && end == xuiRichDocumentGetLength(xuiRichEditGetDocument(edit)),
		"blank below document moves caret to document end");
	CHECK(xuiRichEditSetLinkClick(edit, on_link_click, NULL) == XUI_OK, "link callback");
	CHECK(xuiRichEditGetFragment(edit, linkFragment, &fragment) == XUI_OK, "link click fragment");
	CHECK(xuiInputPointerDown(context, editWorld.fX + 10.0f + fragment.tRect.fX + fragment.tRect.fW * 0.5f,
		editWorld.fY + 10.0f + fragment.tRect.fY + fragment.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "link down");
	CHECK(xuiInputPointerUp(context, editWorld.fX + 10.0f + fragment.tRect.fX + fragment.tRect.fW * 0.5f,
		editWorld.fY + 10.0f + fragment.tRect.fY + fragment.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "link up");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK && g_link_clicks == 1, "link click dispatched");
	CHECK(xuiInputPointerDown(context, editWorld.fX + 10.0f + fragment.tRect.fX + 1.0f,
		editWorld.fY + 10.0f + fragment.tRect.fY + fragment.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "pointer down");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK && xuiGetFocusWidget(context) == edit, "pointer focuses editor");
	CHECK(xuiInputPointerMove(context, editWorld.fX + 10.0f + dragFragment.tRect.fX + dragFragment.tRect.fW,
		editWorld.fY + 10.0f + dragFragment.tRect.fY + dragFragment.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT) == XUI_OK, "pointer drag");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK, "pointer drag dispatch");
	CHECK(xuiInputPointerUp(context, editWorld.fX + 10.0f + dragFragment.tRect.fX + dragFragment.tRect.fW,
		editWorld.fY + 10.0f + dragFragment.tRect.fY + dragFragment.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "pointer up");
	CHECK(xuiDispatchPendingEvents(context) == XUI_OK, "pointer up dispatch");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start != end, "mouse drag creates selection");
	CHECK(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_DELETE, &commandState) == XUI_OK &&
		commandState == XUI_RICH_COMMAND_STATE_ON, "delete command enabled for selection");
	CHECK(xuiRender(context, target, &damage, 1) == XUI_OK, "focused render");
	editCache = xuiWidgetGetCacheSurface(edit, xuiWidgetGetStateId(edit));
	CHECK(xuiTestSurfaceGetRectFillColorCount(editCache, selectionColor) > 0, "selection highlight rendered");
	CHECK(xuiTestSurfaceGetRectFillColorCount(editCache, cursorColor) > 0, "caret rendered");
	CHECK(xuiRichEditSetSelection(edit, 0, 4) == XUI_OK, "selection");
	CHECK(xuiRichEditApplyStyle(edit, &accent) == XUI_OK, "style command");
	CHECK(xuiRichEditSetSelection(edit, 0, 0) == XUI_OK, "caret");
	CHECK(xuiRichEditInsertText(edit, "Start ") == XUI_OK, "insert");
	CHECK(strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "Start ", 6) == 0, "insert result");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "undo");
	CHECK(strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "Rich", 4) == 0, "undo result");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start == 0 && end == 0,
		"undo restores insertion point at changed range");
	CHECK(xuiRichEditRedo(edit) == XUI_OK, "redo");
	CHECK(strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "Start ", 6) == 0 &&
		xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start == 6 && end == 6,
		"redo restores insertion point after changed range");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "second undo");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
	event.pTarget = edit; event.iKey = 'Z'; event.iModifiers = XUI_MOD_CTRL | XUI_MOD_SHIFT;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK &&
		strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "Start ", 6) == 0,
		"Ctrl+Shift+Z performs redo");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "undo keyboard redo");
	CHECK(xuiSetFocusWidget(context, edit) == XUI_OK, "focus");
	ret = xuiInputText(context, '!'); if (ret == XUI_OK) ret = xuiDispatchPendingEvents(context);
	CHECK(ret == XUI_OK, "input dispatch");
	CHECK(strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "!Rich", 5) == 0, "input at undo caret");
	CHECK(xuiRichEditSetSelection(edit, 0, 4) == XUI_OK, "IME selection");
	ret = xuiInputImeComposition(context, "\xe4\xbd\xa0\xe5\xa5\xbd", -1, 0, 6); if (ret == XUI_OK) ret = xuiDispatchPendingEvents(context);
	CHECK(ret == XUI_OK, "IME preedit");
	ret = xuiInputImeComposition(context, "\xe4\xbd\xa0\xe5\xa5\xbd", -1, 0, 0); if (ret == XUI_OK) ret = xuiDispatchPendingEvents(context);
	CHECK(ret == XUI_OK && strncmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), "\xe4\xbd\xa0\xe5\xa5\xbd", 6) == 0, "IME replaces selection");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start == end, "IME collapses selection");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "widget fragment scan");
		if ( fragment.iNodeType == XUI_RICH_NODE_INLINE_WIDGET ) {
			widgetStart = fragment.iDocumentStart;
			widgetEnd = fragment.iDocumentEnd;
			break;
		}
	}
	CHECK(widgetStart >= 0 && widgetEnd > widgetStart, "widget fragment found");
	CHECK(xuiRichDocumentReplace(xuiRichEditGetDocument(edit), widgetStart, widgetEnd, "", NULL) == XUI_OK, "remove widget node");
	CHECK(xuiLayout(context) == XUI_OK && xuiWidgetGetParent(button) == NULL, "removed widget detached");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "restore widget node");
	CHECK(xuiLayout(context) == XUI_OK && xuiWidgetGetParent(button) == edit, "restored widget attached");
	memset(&fontSet, 0, sizeof(fontSet)); fontSet.iSize = sizeof(fontSet); fontSet.pNormal = font; fontSet.pBold = font; fontSet.pItalic = font; fontSet.pBoldItalic = font;
	CHECK(xuiRichEditSetFontSet(edit, &fontSet) == XUI_OK, "font set");
	memset(&fontSet, 0, sizeof(fontSet)); CHECK(xuiRichEditGetFontSet(edit, &fontSet) == XUI_OK && fontSet.pNormal == font, "get font set");
	CHECK(xuiRichEditSetSelection(edit, 0, 3) == XUI_OK, "command selection");
	CHECK(xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_BOLD, NULL) == XUI_OK, "bold command");
	CHECK(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_BOLD, &commandState) == XUI_OK && commandState == XUI_RICH_COMMAND_STATE_ON, "bold state");
	CHECK(xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_ALIGN_CENTER, NULL) == XUI_OK, "align command");
	CHECK(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_ALIGN_CENTER, &commandState) == XUI_OK && commandState == XUI_RICH_COMMAND_STATE_ON, "align state");
	memset(&toolbarDesc, 0, sizeof(toolbarDesc)); toolbarDesc.iSize = sizeof(toolbarDesc); toolbarDesc.pFont = font;
	CHECK(xuiToolbarCreate(context, &toolbar, &toolbarDesc) == XUI_OK &&
		xuiRichEditSetupToolbar(edit, toolbar, XUI_RICH_TOOLBAR_HISTORY | XUI_RICH_TOOLBAR_INLINE_FORMAT | XUI_RICH_TOOLBAR_ALIGNMENT) == XUI_OK,
		"standard rich toolbar setup");
	CHECK(xuiToolbarGetItemCount(toolbar) == 12, "rich toolbar groups and separators");
	for ( i = 0; i < xuiToolbarGetItemCount(toolbar); i++ ) {
		const xui_toolbar_item_t* item = xuiToolbarGetItem(toolbar, i);
		if ( item != NULL && item->iValue == XUI_RICH_COMMAND_BOLD ) break;
	}
	CHECK(i < xuiToolbarGetItemCount(toolbar) && xuiToolbarGetItemChecked(toolbar, i),
		"rich toolbar reflects active inline format");
	CHECK(xuiRichEditExecuteToolbarItem(edit, toolbar, i) == XUI_OK &&
		xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_BOLD, &commandState) == XUI_OK && commandState == XUI_RICH_COMMAND_STATE_OFF &&
		!xuiToolbarGetItemChecked(toolbar, i), "rich toolbar executes and resynchronizes command");
	CHECK(xuiRichEditExecuteToolbarItem(edit, toolbar, i) == XUI_OK, "restore bold from toolbar");
	CHECK(xuiRichEditSetReadonly(edit, 1) == XUI_OK &&
		xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_UNDO, &commandState) == XUI_OK &&
		commandState == XUI_RICH_COMMAND_STATE_DISABLED &&
		xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_UNDO, NULL) == XUI_ERROR_UNSUPPORTED,
		"readonly blocks undo through query and execution");
	CHECK(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_REPLACE, &commandState) == XUI_OK &&
		commandState == XUI_RICH_COMMAND_STATE_DISABLED &&
		xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_REPLACE, NULL) == XUI_ERROR_UNSUPPORTED,
		"readonly blocks replace through query and execution");
	CHECK(xuiRichEditSyncToolbar(edit, toolbar) == XUI_OK && !xuiToolbarIsItemEnabled(toolbar, i),
		"readonly state reaches standard toolbar");
	CHECK(xuiRichEditSetReadonly(edit, 0) == XUI_OK && xuiRichEditSyncToolbar(edit, toolbar) == XUI_OK &&
		xuiToolbarIsItemEnabled(toolbar, i), "leaving readonly restores toolbar commands");
	CHECK(xuiSetLanguage(context, XUI_LANGUAGE_ZH) == XUI_OK && xuiRichEditSyncToolbar(edit, toolbar) == XUI_OK &&
		strstr(xuiToolbarGetItemTooltip(toolbar, i), "\xE7\xB2\x97\xE4\xBD\x93") != NULL,
		"standard toolbar tooltip follows the global language table");
	CHECK(xuiSetLanguage(context, XUI_LANGUAGE_EN) == XUI_OK, "restore English language");
	firstParagraphEnd = (int)(strchr(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)), '\n') -
		xuiRichDocumentGetText(xuiRichEditGetDocument(edit)));
	CHECK(xuiRichEditSetSelection(edit, 0, firstParagraphEnd) == XUI_OK &&
		xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_BULLET_LIST, NULL) == XUI_OK,
		"convert paragraph to bullet list");
	memset(&tableInfo, 0, sizeof(tableInfo)); tableInfo.iSize = sizeof(tableInfo);
	CHECK(block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.iType == XUI_RICH_NODE_LIST_ITEM && tableInfo.tParagraphStyle.iListType == XUI_RICH_LIST_BULLET,
		"bullet list metadata");
	CHECK(xuiRichEditSetSelection(edit, 1, 1) == XUI_OK, "list caret");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
	event.pTarget = edit; event.iKey = XUI_KEY_TAB;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.tParagraphStyle.iListLevel == 1, "Tab indents list item");
	event.iModifiers = XUI_MOD_SHIFT;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.tParagraphStyle.iListLevel == 0, "Shift+Tab outdents list item");
	CHECK(xuiRichEditSetSelection(edit, firstParagraphEnd, firstParagraphEnd) == XUI_OK, "list end caret");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
	event.pTarget = edit; event.iKey = XUI_KEY_ENTER;
	ret = xuiDispatchEvent(context, &event);
	CHECK(ret == XUI_OK &&
		block_info_at(xuiRichEditGetDocument(edit), firstParagraphEnd + 1, &tableInfo) &&
		tableInfo.iType == XUI_RICH_NODE_LIST_ITEM && tableInfo.tParagraphStyle.iListType == XUI_RICH_LIST_BULLET,
		"Enter creates a peer list item");
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK &&
		block_info_at(xuiRichEditGetDocument(edit), firstParagraphEnd + 1, &tableInfo) &&
		tableInfo.iType == XUI_RICH_NODE_PARAGRAPH && tableInfo.tParagraphStyle.iListType == XUI_RICH_LIST_NONE,
		"Enter on an empty list item exits the list");
	CHECK(xuiRichEditSetSelection(edit, 1, 1) == XUI_OK &&
		xuiRichEditExecuteCommand(edit, XUI_RICH_COMMAND_CHECK_LIST, NULL) == XUI_OK,
		"convert paragraph to check list");
	CHECK(xuiLayout(context) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.tParagraphStyle.iListType == XUI_RICH_LIST_CHECK && !tableInfo.tParagraphStyle.bListChecked,
		"check list starts unchecked");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "check list fragment scan");
		if ( fragment.iDocumentStart <= 1 && fragment.iDocumentEnd >= 1 ) break;
	}
	CHECK(i < xuiRichEditGetFragmentCount(edit), "check list fragment found");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOWN;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT; event.iButtons = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + 8.0f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + fragment.tRect.fY + fragment.tRect.fH * 0.5f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.tParagraphStyle.bListChecked, "clicking check marker toggles state");
	CHECK(xuiRichEditUndo(edit) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		!tableInfo.tParagraphStyle.bListChecked, "check state participates in undo");
	CHECK(xuiRichEditRedo(edit) == XUI_OK && block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) &&
		tableInfo.tParagraphStyle.bListChecked, "check state participates in redo");
	accessibleCount = xuiWidgetGetAccessibleNodeCount(edit);
	for ( i = 0; i < accessibleCount; i++ ) {
		memset(&accessible, 0, sizeof(accessible)); accessible.iSize = sizeof(accessible);
		CHECK(xuiWidgetGetAccessibleNode(edit, i, &accessible) == XUI_OK, "accessible check lookup");
		if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX ) { accessibleCheckId = accessible.iId; break; }
	}
	CHECK(accessibleCheckId != 0 && xuiWidgetPerformAccessibleAction(edit, accessibleCheckId,
		XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK &&
		block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) && !tableInfo.tParagraphStyle.bListChecked,
		"accessible check action toggles list item");
	CHECK(xuiWidgetPerformAccessibleAction(edit, accessibleCheckId,
		XUI_ACCESSIBLE_ACTION_TOGGLE, NULL) == XUI_OK &&
		block_info_at(xuiRichEditGetDocument(edit), 1, &tableInfo) && tableInfo.tParagraphStyle.bListChecked,
		"accessible check action toggles back");
	memset(&paragraphStyle, 0, sizeof(paragraphStyle)); paragraphStyle.iSize = sizeof(paragraphStyle); paragraphStyle.iAlign = XUI_RICH_ALIGN_RIGHT; paragraphStyle.fSpaceBefore = 4;
	CHECK(xuiRichEditApplyParagraphStyle(edit, &paragraphStyle) == XUI_OK, "paragraph API");
	CHECK(xuiRichEditSetSelection(edit, 0, 3) == XUI_OK && xuiRichEditCopy(edit) == XUI_OK, "rich copy");
	CHECK(xuiRichEditSetSelection(edit, 3, 3) == XUI_OK && xuiRichEditPaste(edit) == XUI_OK, "rich paste");
	CHECK(xuiRichEditQueryCommand(edit, XUI_RICH_COMMAND_BOLD, &commandState) == XUI_OK && commandState == XUI_RICH_COMMAND_STATE_ON, "rich paste preserves style");
	beforePlainPasteLength = xuiRichDocumentGetLength(xuiRichEditGetDocument(edit));
	CHECK(xuiTestProxySetClipboardText(&proxy, "external") == XUI_OK &&
		xuiRichEditSetSelection(edit, beforePlainPasteLength, beforePlainPasteLength) == XUI_OK &&
		xuiRichEditPaste(edit) == XUI_OK, "external plain clipboard paste");
	CHECK(xuiRichDocumentGetLength(xuiRichEditGetDocument(edit)) == beforePlainPasteLength + 8 &&
		strcmp(xuiRichDocumentGetText(xuiRichEditGetDocument(edit)) + beforePlainPasteLength, "external") == 0,
		"plain clipboard cannot reuse stale rich content");
	CHECK(xuiTestSurfaceCreate(&proxy, &imageSurface, 64, 32, 0) == XUI_OK, "image surface");
	memset(&imageDesc, 0, sizeof(imageDesc)); imageDesc.iSize = sizeof(imageDesc); imageDesc.pSurface = imageSurface;
	imageDesc.sAltText = "inline image"; imageDesc.fWidth = 20; imageDesc.fHeight = 16; imageDesc.fBaseline = 13;
	CHECK(xuiRichEditSetSelection(edit, 0, 0) == XUI_OK && xuiRichEditInsertInlineImage(edit, &imageDesc) == XUI_OK,
		"insert inline image");
	CHECK(xuiLayout(context) == XUI_OK, "inline image layout");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "inline image fragment scan");
		if ( fragment.iNodeType == XUI_RICH_NODE_INLINE_IMAGE ) {
			inlineImageStart = fragment.iDocumentStart; inlineImageEnd = fragment.iDocumentEnd;
			break;
		}
	}
	CHECK(i < xuiRichEditGetFragmentCount(edit) && inlineImageStart == 0 && inlineImageEnd == 3 &&
		fragment.tRect.fW == 20 && fragment.tRect.fH == 16, "inline image is an atomic measured fragment");
	CHECK(xuiRichEditUndo(edit) == XUI_OK, "undo inline image");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK && start == 0 && end == 0,
		"inline image undo restores caret");
	CHECK(xuiRichEditRedo(edit) == XUI_OK, "redo inline image");
	imageDesc.sAltText = "test image"; imageDesc.fWidth = 96; imageDesc.fHeight = 48; imageDesc.fBaseline = 0;
	CHECK(xuiRichEditSetSelection(edit, xuiRichDocumentGetLength(xuiRichEditGetDocument(edit)), xuiRichDocumentGetLength(xuiRichEditGetDocument(edit))) == XUI_OK, "object caret");
	CHECK(xuiRichEditInsertImage(edit, &imageDesc) == XUI_OK, "insert image");
	memset(&tableDesc, 0, sizeof(tableDesc)); tableDesc.iSize = sizeof(tableDesc); tableDesc.iRows = 2; tableDesc.iColumns = 3; tableDesc.fWidth = 220;
	CHECK(xuiRichEditInsertTable(edit, &tableDesc) == XUI_OK, "insert table");
	CHECK(xuiRichEditInsertHorizontalRule(edit) == XUI_OK, "insert rule");
	CHECK(xuiLayout(context) == XUI_OK, "object layout");
	CHECK(xuiWidgetSetAccessibleName(edit, "Rich document test") == XUI_OK,
		"accessible document name");
	accessibleCount = xuiWidgetGetAccessibleNodeCount(edit);
	CHECK(accessibleCount >= 12, "rich accessibility tree has semantic nodes");
	for ( i = 0; i < accessibleCount; i++ ) {
		memset(&accessible, 0, sizeof(accessible)); accessible.iSize = sizeof(accessible);
		CHECK(xuiWidgetGetAccessibleNode(edit, i, &accessible) == XUI_OK,
			"accessible node enumeration");
		if ( i == 0 ) {
			CHECK(accessible.iRole == XUI_ACCESSIBLE_ROLE_DOCUMENT &&
				strcmp(accessible.sName, "Rich document test") == 0 &&
				(accessible.iState & XUI_ACCESSIBLE_STATE_EDITABLE) != 0,
				"accessible root semantics");
		}
		if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_LINK ) accessibleLinkId = accessible.iId;
		else if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_IMAGE && accessible.sName != NULL &&
			strcmp(accessible.sName, "test image") == 0 ) accessibleImageId = accessible.iId;
		else if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_TABLE ) accessibleTableId = accessible.iId;
		else if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_CELL ) accessibleCellId = accessible.iId;
		else if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX ) accessibleCheckId = accessible.iId;
		else if ( accessible.iRole == XUI_ACCESSIBLE_ROLE_EMBEDDED_OBJECT ) accessibleWidgetId = accessible.iId;
		if ( accessible.iRole != XUI_ACCESSIBLE_ROLE_DOCUMENT )
			CHECK(accessible.iId != 0 && accessible.iParentId != 0, "accessible node stable identity");
	}
	CHECK(accessibleLinkId != 0 && accessibleImageId != 0 && accessibleTableId != 0 &&
		accessibleCellId != 0 && accessibleCheckId != 0 && accessibleWidgetId != 0,
		"rich accessibility roles cover links, objects, tables, cells and widgets");
	tableId = accessibleTableId;
	memset(&accessible, 0, sizeof(accessible)); accessible.iSize = sizeof(accessible);
	for ( i = 0; i < accessibleCount; i++ ) {
		CHECK(xuiWidgetGetAccessibleNode(edit, i, &accessible) == XUI_OK, "accessible table lookup");
		if ( accessible.iId == accessibleTableId ) break;
	}
	CHECK(i < accessibleCount && accessible.iRowCount == 2 && accessible.iColumnCount == 3 &&
		accessible.tBounds.fW > 0.0f && accessible.tBounds.fH > 0.0f,
		"accessible table exposes dimensions and world bounds");
	CHECK(xuiWidgetPerformAccessibleAction(edit, accessibleLinkId,
		XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK && g_link_clicks == 2,
		"accessible link activation uses normal callback");
	CHECK(xuiWidgetPerformAccessibleAction(edit, accessibleImageId,
		XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK && g_object_events == 1,
		"accessible object activation uses rich event contract");
	memset(&accessibleSelection, 0, sizeof(accessibleSelection));
	accessibleSelection.iSize = sizeof(accessibleSelection); accessibleSelection.iAnchor = 0; accessibleSelection.iCaret = 3;
	CHECK(xuiWidgetPerformAccessibleAction(edit, 0, XUI_ACCESSIBLE_ACTION_SET_SELECTION,
		&accessibleSelection) == XUI_OK && xuiRichEditGetSelection(edit, &start, &end) == XUI_OK &&
		start == 0 && end == 3, "accessible text selection action");
	CHECK(xuiWidgetPerformAccessibleAction(edit, accessibleTableId,
		XUI_ACCESSIBLE_ACTION_SCROLL_INTO_VIEW, NULL) == XUI_OK,
		"accessible scroll-into-view action");
	CHECK(g_accessibility_events > 0 && g_accessibility_last_revision ==
		xuiWidgetGetAccessibilityRevision(edit) && g_accessibility_last_type >= XUI_ACCESSIBLE_EVENT_TREE_CHANGED,
		"accessibility notifications expose current revision");
	scrollModel = xuiRichEditGetScrollModel(edit);
	CHECK(scrollModel != NULL && xuiRichEditGetHScrollBarWidget(edit) != NULL &&
		xuiRichEditGetVScrollBarWidget(edit) != NULL, "rich editor exposes standard scroll model and bars");
	CHECK(xuiWidgetGetParent(xuiRichEditGetHScrollBarWidget(edit)) == edit &&
		xuiWidgetGetParent(xuiRichEditGetVScrollBarWidget(edit)) == edit,
		"scroll bars survive embedded widget synchronization");
	CHECK(xuiWidgetGetVisible(xuiRichEditGetVScrollBarWidget(edit)) != 0,
		"long rich document shows vertical scroll bar");
	CHECK(xuiScrollModelGetMaxOffset(scrollModel, &maxScrollX, &maxScrollY) == XUI_OK && maxScrollY > 0.0f,
		"scroll model reports vertical range");
	CHECK(xuiRichEditSetScroll(edit, 100000.0f, 100000.0f) == XUI_OK &&
		xuiRichEditGetScroll(edit, &scrollX, &scrollY) == XUI_OK &&
		scrollX <= maxScrollX + 0.01f && scrollY <= maxScrollY + 0.01f,
		"scroll offsets clamp through the shared model");
	CHECK(xuiUpdate(context, 0.016f) == XUI_OK && g_scroll_events > 0,
		"scroll changes use the unified rich event contract");
	CHECK(xuiRichEditSetScroll(edit, 0.0f, 0.0f) == XUI_OK, "reset rich editor scroll");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &tableFragment) == XUI_OK, "table fragment scan");
		if ( tableFragment.iNodeType == XUI_RICH_NODE_TABLE ) { tableId = tableFragment.iNodeId; break; }
	}
	CHECK(i < xuiRichEditGetFragmentCount(edit) && tableId != 0, "table fragment found");
	CHECK(xuiRichEditGetScroll(edit, &scrollX, &scrollY) == XUI_OK, "table edit scroll");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOUBLE_CLICK;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT; event.iButtons = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + tableFragment.tRect.fX - scrollX + tableFragment.tRect.fW / 6.0f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + tableFragment.tRect.fY - scrollY + tableFragment.tRect.fH / 4.0f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "double click enters table cell edit");
	cellEditor = xuiRichEditGetTableCellEditor(edit, NULL, &start, &end);
	CHECK(cellEditor != NULL && start == 0 && end == 0 && xuiGetFocusWidget(context) == cellEditor,
		"table cell editor receives focus");
	CHECK(xuiInputText(context, 'Q') == XUI_OK && xuiDispatchPendingEvents(context) == XUI_OK,
		"type through table cell editor");
	CHECK(strcmp(xuiRichTableGetCellText(xuiRichDocumentFindNode(xuiRichEditGetDocument(edit), tableId), 0, 0), "Q") == 0,
		"table cell input updates structured document");
	CHECK(xuiRichEditEndTableCellEdit(edit) == XUI_OK && xuiRichEditGetTableCellEditor(edit, NULL, NULL, NULL) == NULL,
		"table cell editor closes cleanly");
	tableNode = xuiRichDocumentFindNode(xuiRichEditGetDocument(edit), tableId);
	memset(&tableInfo, 0, sizeof(tableInfo)); tableInfo.iSize = sizeof(tableInfo);
	CHECK(tableNode != NULL && xuiRichNodeGetInfo(tableNode, &tableInfo) == XUI_OK &&
		tableInfo.iType == XUI_RICH_NODE_TABLE, "table node remains addressable");
	CHECK(xuiRichTableGetCellDocument(xuiRichEditGetDocument(edit), tableNode, 0, 0, &cellDocument) == XUI_OK &&
		cellDocument != NULL, "table cell document is addressable");
	CHECK(xuiRichEditBeginTableCellEdit(edit, tableId, 0, 0) == XUI_OK &&
		xuiRichEditGetTableCellEditor(edit, NULL, NULL, NULL) != NULL, "programmatic table cell edit");
	CHECK(xuiRichEditEndTableCellEdit(edit) == XUI_OK, "programmatic table cell edit closes");
	for ( i = 0; i < xuiRichEditGetFragmentCount(edit); i++ ) {
		CHECK(xuiRichEditGetFragment(edit, i, &fragment) == XUI_OK, "image fragment scan");
		if ( fragment.iNodeType == XUI_RICH_NODE_IMAGE ) break;
	}
	CHECK(i < xuiRichEditGetFragmentCount(edit), "image fragment found");
	CHECK(xuiRichEditGetScroll(edit, &scrollX, &scrollY) == XUI_OK, "object scroll");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOWN;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT; event.iButtons = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + fragment.tRect.fX - scrollX + fragment.tRect.fW * 0.5f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + fragment.tRect.fY - scrollY + fragment.tRect.fH * 0.5f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "image selection dispatch");
	CHECK(xuiRichEditGetSelection(edit, &start, &end) == XUI_OK &&
		start == fragment.iDocumentStart && end == fragment.iDocumentEnd,
		"pointer selects a non-text object atomically");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_UP;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + fragment.tRect.fX - scrollX + fragment.tRect.fW * 0.5f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + fragment.tRect.fY - scrollY + fragment.tRect.fH * 0.5f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && g_object_events == 2 &&
		g_object_node_id == fragment.iNodeId, "object click emits structured activation event");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_DOWN;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT; event.iButtons = XUI_POINTER_BUTTON_LEFT;
	event.fX = xuiWidgetGetWorldRect(edit).fX + 10.0f + fragment.tRect.fX - scrollX + fragment.tRect.fW * 0.5f;
	event.fY = xuiWidgetGetWorldRect(edit).fY + 10.0f + fragment.tRect.fY - scrollY + fragment.tRect.fH * 0.5f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "restart object drag after activation");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_MOVE;
	event.pTarget = edit; event.iButtons = XUI_POINTER_BUTTON_LEFT;
	event.fX = editWorld.fX + 80.0f; event.fY = editWorld.fY + 260.0f;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "drag auto-scroll dispatch");
	CHECK(xuiRichEditGetScroll(edit, &scrollX, &scrollY) == XUI_OK && scrollY > 0.0f,
		"selection drag beyond viewport auto-scrolls");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_POINTER_UP;
	event.pTarget = edit; event.iButton = XUI_POINTER_BUTTON_LEFT;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK, "finish auto-scroll drag");
	CHECK(xuiRender(context, target, &damage, 1) == XUI_OK, "object render");
	editCache = xuiWidgetGetCacheSurface(edit, xuiWidgetGetStateId(edit));
	CHECK(editCache != NULL && xuiTestSurfaceGetDrawCount(editCache) > 0, "image rendered");

	memset(&desc, 0, sizeof(desc)); desc.iSize = sizeof(desc); desc.pFont = font;
	desc.sText = "Alpha alpha beta alpha"; desc.bWordWrap = 1;
	CHECK(xuiRichEditCreate(context, &findEdit, &desc) == XUI_OK &&
		xuiWidgetAddChild(root, findEdit) == XUI_OK, "find editor create");
	CHECK(xuiWidgetSetRect(findEdit, (xui_rect_t){0, 0, 420, 100}) == XUI_OK, "find editor rect");
	findDocument = xuiRichEditGetDocument(findEdit);
	memset(&red, 0, sizeof(red)); red.iSize = sizeof(red); red.iFlags = XUI_RICH_STYLE_BOLD;
	CHECK(xuiRichDocumentApplyStyle(findDocument, 0, 5, &red) == XUI_OK, "find source style");
	memset(&findOptions, 0, sizeof(findOptions)); findOptions.iSize = sizeof(findOptions);
	findOptions.sPattern = "alpha"; findOptions.sReplacement = "ONE";
	CHECK(xuiRichEditFindNext(findEdit, &findOptions) == XUI_OK &&
		xuiRichEditGetFindResultCount(findEdit) == 3, "rich find all highlights");
	CHECK(xuiRichEditGetFindResult(findEdit, 0, &findResult) == XUI_OK &&
		findResult.iStart == 0 && findResult.iEnd == 5, "rich find result offsets");
	CHECK(xuiRichEditReplaceCurrent(findEdit, &findOptions) == XUI_OK &&
		strncmp(xuiRichDocumentGetText(findDocument), "ONE alpha", 9) == 0,
		"replace current keeps document structure");
	CHECK(xuiRichEditSetSelection(findEdit, 1, 1) == XUI_OK &&
		xuiRichEditQueryCommand(findEdit, XUI_RICH_COMMAND_BOLD, &commandState) == XUI_OK &&
		commandState == XUI_RICH_COMMAND_STATE_ON, "replacement inherits match style");
	findOptions.sPattern = "(alpha)"; findOptions.sReplacement = "[$1]";
	findOptions.iFlags = XUI_FIND_REGEX;
	replaceCount = 0;
	CHECK(xuiRichEditReplaceAll(findEdit, &findOptions, &replaceCount) == XUI_OK && replaceCount == 2 &&
		strcmp(xuiRichDocumentGetText(findDocument), "ONE [alpha] beta [alpha]") == 0,
		"regex capture replace all");
	CHECK(xuiRichEditUndo(findEdit) == XUI_OK &&
		strcmp(xuiRichDocumentGetText(findDocument), "ONE alpha beta alpha") == 0,
		"replace all is one undo transaction");
	CHECK(xuiSetFocusWidget(context, findEdit) == XUI_OK, "find editor focus");
	memset(&event, 0, sizeof(event)); event.iSize = sizeof(event); event.iType = XUI_EVENT_KEY_DOWN;
	event.pTarget = findEdit; event.iKey = 'F'; event.iModifiers = XUI_MOD_CTRL;
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && xuiRichEditGetFindWindow(findEdit) != NULL,
		"Ctrl+F opens built-in find window");
	event.iKey = 'H';
	CHECK(xuiDispatchEvent(context, &event) == XUI_OK && xuiRichEditGetFindWindow(findEdit) != NULL,
		"Ctrl+H opens built-in replace window");

cleanup:
	if ( toolbar != NULL ) xuiWidgetDestroy(toolbar);
	if ( context != NULL ) xuiDestroy(context);
	if ( target != NULL ) proxy.tProxy.surfaceDestroy(&proxy.tProxy, target);
	if ( imageSurface != NULL ) proxy.tProxy.surfaceDestroy(&proxy.tProxy, imageSurface);
	if ( doc != NULL ) xuiRichDocumentDestroy(doc);
	if (!failed) printf("xui_rich_edit_test: all tests passed\n");
	return failed ? 1 : 0;
}
