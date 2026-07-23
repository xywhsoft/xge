#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_KEY_BACKSPACE	8
#define XUI_TEST_KEY_DELETE	46

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_input_widget_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_input_widget_change_t {
	int iCount;
	char sText[128];
} xui_input_widget_change_t;

typedef struct xui_input_widget_decoration_t {
	int iClickCount;
} xui_input_widget_decoration_t;

static void __xuiInputWidgetChanged(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_input_widget_change_t* pChange;

	(void)pWidget;
	pChange = (xui_input_widget_change_t*)pUser;
	if ( pChange == NULL ) return;
	pChange->iCount++;
	if ( sText == NULL ) sText = "";
	strncpy(pChange->sText, sText, sizeof(pChange->sText) - 1u);
	pChange->sText[sizeof(pChange->sText) - 1u] = '\0';
}

static void __xuiInputWidgetDecorationClick(xui_widget pWidget, xui_input_decoration pDecoration, void* pUser)
{
	xui_input_widget_decoration_t* pData;

	(void)pWidget;
	(void)pDecoration;
	pData = (xui_input_widget_decoration_t*)pUser;
	if ( pData != NULL ) {
		pData->iClickCount++;
	}
}

static int __xuiInputWidgetNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.01f;
}

static int __xuiInputWidgetRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 480, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiInputWidgetDispatchText(xui_context pContext, uint32_t iCodepoint)
{
	int iRet;

	iRet = xuiInputText(pContext, iCodepoint);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetDispatchKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetDispatchIme(xui_context pContext, const char* sText)
{
	int iRet;

	iRet = xuiInputImeComposition(pContext, sText, -1, 0, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetDispatchImePreedit(xui_context pContext, const char* sText)
{
	int iRet;

	iRet = xuiInputImeComposition(pContext, sText, -1, 0, (int)strlen(sText));
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetRightClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetLongPress(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.60f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiInputWidgetDoubleClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiInputWidgetClick(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiInputWidgetClick(pContext, fX, fY);
}

static int __xuiInputWidgetClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_input_widget_change_t tChange;
	xui_input_widget_decoration_t tDecorationState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pInput;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pInputCache;
	xui_font pFont;
	xui_input_desc_t tDesc;
	xui_input_decoration_desc_t tDecorationDesc;
	xui_input_decoration pSearchDecoration;
	xui_input_decoration pClearDecoration;
	xui_input_decoration pGoDecoration;
	const xui_menu_item_t* pItem;
	const char* sNiHao;
	xui_rect_t tSelectionRect;
	xui_rect_t tTextRect;
	xui_rect_t tWorldRect;
	xui_rect_t tSearchRect;
	xui_rect_t tClearRect;
	xui_rect_t tGoRect;
	char sLongPaste[12001];
	int iStart;
	int iEnd;
	int iVectorDrawBase;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pInput = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pInputCache = NULL;
	pFont = NULL;
	pSearchDecoration = NULL;
	pClearDecoration = NULL;
	pGoDecoration = NULL;
	iVectorDrawBase = 0;
	iFailed = 0;
	sNiHao = "\xE4\xBD\xA0\xE5\xA5\xBD";
	memset(&tChange, 0, sizeof(tChange));
	memset(&tDecorationState, 0, sizeof(tDecorationState));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "test.ttf", 14.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 480.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 480.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "abc";
	tDesc.sPlaceholder = "Type text";
	tDesc.pFont = pFont;
	tDesc.iMaxLength = 5;
	tDesc.iTextAlign = XUI_INPUT_ALIGN_LEFT;
	iRet = xuiInputCreate(pContext, &pInput, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pInput != NULL, "input create");
	xuiWidgetSetRect(pInput, (xui_rect_t){24.0f, 28.0f, 220.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "add input");
	iRet = xuiInputSetChange(pInput, __xuiInputWidgetChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 480, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiInputWidgetRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(strcmp(xuiInputGetText(pInput), "abc") == 0, "initial text");
	XUI_TEST_CHECK(strcmp(xuiInputGetPlaceholder(pInput), "Type text") == 0, "placeholder");
	XUI_TEST_CHECK(xuiInputGetMaxLength(pInput) == 5, "max length");
	XUI_TEST_CHECK(xuiInputGetTextAlign(pInput) == XUI_INPUT_ALIGN_LEFT, "align");
	XUI_TEST_CHECK((xuiInputGetState(pInput) & XUI_INPUT_STATE_ERROR) == 0u, "initial state");
	iRet = xuiInputSetError(pInput, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiInputGetState(pInput) & XUI_INPUT_STATE_ERROR) != 0u, "error state");
	iRet = xuiInputSetError(pInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear error state");

	memset(&tDecorationDesc, 0, sizeof(tDecorationDesc));
	tDecorationDesc.iSize = sizeof(tDecorationDesc);
	tDecorationDesc.iKind = XUI_INPUT_DECORATION_ICON;
	tDecorationDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDecorationDesc.iIcon = XUI_INPUT_ICON_SEARCH;
	iRet = xuiInputDecorationAdd(pInput, XUI_INPUT_DECORATION_SIDE_LEADING, &pSearchDecoration, &tDecorationDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSearchDecoration != NULL, "add leading decoration");
	memset(&tDecorationDesc, 0, sizeof(tDecorationDesc));
	tDecorationDesc.iSize = sizeof(tDecorationDesc);
	tDecorationDesc.iKind = XUI_INPUT_DECORATION_CLEAR;
	tDecorationDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_NOT_EMPTY;
	iRet = xuiInputDecorationAdd(pInput, XUI_INPUT_DECORATION_SIDE_TRAILING, &pClearDecoration, &tDecorationDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pClearDecoration != NULL, "add clear decoration");
	memset(&tDecorationDesc, 0, sizeof(tDecorationDesc));
	tDecorationDesc.iSize = sizeof(tDecorationDesc);
	tDecorationDesc.iKind = XUI_INPUT_DECORATION_TEXT;
	tDecorationDesc.iVisibleMode = XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDecorationDesc.sText = "GO";
	tDecorationDesc.fPadding = 8.0f;
	tDecorationDesc.onClick = __xuiInputWidgetDecorationClick;
	tDecorationDesc.pUser = &tDecorationState;
	iRet = xuiInputDecorationAdd(pInput, XUI_INPUT_DECORATION_SIDE_TRAILING, &pGoDecoration, &tDecorationDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGoDecoration != NULL, "add text decoration");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "decoration layout");
	iVectorDrawBase = xuiTestProxyGetSvgPathDrawCount(&tState);
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "decoration update");
	iRet = __xuiInputWidgetRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "decoration render");
	XUI_TEST_CHECK(xuiTestProxyGetSvgPathDrawCount(&tState) >= iVectorDrawBase + 2, "decoration vector icon path draw");
	tTextRect = xuiInputGetTextRect(pInput);
	tSearchRect = xuiInputDecorationGetRect(pInput, pSearchDecoration);
	tClearRect = xuiInputDecorationGetRect(pInput, pClearDecoration);
	tGoRect = xuiInputDecorationGetRect(pInput, pGoDecoration);
	XUI_TEST_CHECK(__xuiInputWidgetNear(tSearchRect.fW, 22.0f) && tSearchRect.fH > 0.0f, "search decoration rect");
	XUI_TEST_CHECK(tTextRect.fX >= tSearchRect.fX + tSearchRect.fW, "decoration text padding");
	XUI_TEST_CHECK(tClearRect.fW > 0.0f && tGoRect.fW > 0.0f && tGoRect.fX < tClearRect.fX, "trailing decoration order");
	tWorldRect = xuiWidgetGetWorldRect(pInput);
	iRet = __xuiInputWidgetClick(pContext, tWorldRect.fX + tGoRect.fX + tGoRect.fW * 0.5f, tWorldRect.fY + tGoRect.fY + tGoRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tDecorationState.iClickCount == 1, "text decoration click");
	iRet = __xuiInputWidgetClick(pContext, tWorldRect.fX + tClearRect.fX + tClearRect.fW * 0.5f, tWorldRect.fY + tClearRect.fY + tClearRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "clear decoration click");
	tClearRect = xuiInputDecorationGetRect(pInput, pClearDecoration);
	XUI_TEST_CHECK(tClearRect.fW == 0.0f, "clear decoration hidden when empty");
	iRet = xuiInputSetText(pInput, "abc");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after decoration");

	iRet = xuiSetFocusWidget(pContext, pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pInput, "focus input");
	iRet = __xuiInputWidgetDispatchText(pContext, 'd');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcd") == 0, "type d");
	iRet = __xuiInputWidgetDispatchText(pContext, 'e');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "type e");
	iRet = __xuiInputWidgetDispatchText(pContext, 'f');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "max length clamp");
	XUI_TEST_CHECK(tChange.iCount >= 2 && strcmp(tChange.sText, "abcde") == 0, "change callback text");
	iRet = __xuiInputWidgetDispatchKey(pContext, 'A', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputHasSelection(pInput), "ctrl a select all");
	iRet = __xuiInputWidgetDispatchText(pContext, 'a');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "ctrl text suppressed");
	iRet = xuiInputSetModifiers(pContext, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear modifiers after ctrl text");

	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputHasSelection(pInput), "select all");
	iRet = xuiInputCopy(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTestProxyGetClipboardText(&tState), "abcde") == 0, "copy");
	iRet = xuiInputCut(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "cut");
	XUI_TEST_CHECK(xuiInputCanUndo(pInput), "can undo cut");
	iRet = xuiInputUndo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "undo cut");
	XUI_TEST_CHECK(xuiInputCanRedo(pInput), "can redo cut");
	iRet = xuiInputRedo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "redo cut");
	iRet = xuiInputUndo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "undo redo cut");
	iRet = xuiTestProxySetClipboardText(&tState, "xy");
	XUI_TEST_CHECK(iRet == XUI_OK, "set clipboard");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all before paste");
	iRet = xuiInputPaste(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "xy") == 0, "paste replaces selection");
	XUI_TEST_CHECK(!xuiInputCanRedo(pInput), "edit clears redo");
	iRet = xuiInputUndo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "undo paste");
	iRet = __xuiInputWidgetDispatchKey(pContext, 'Y', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "xy") == 0, "ctrl y redo paste");
	iRet = __xuiInputWidgetDispatchKey(pContext, 'Z', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "abcde") == 0, "ctrl z undo redo paste");
	memset(sLongPaste, 'p', sizeof(sLongPaste) - 1u);
	sLongPaste[sizeof(sLongPaste) - 1u] = '\0';
	iRet = xuiInputSetMaxLength(pInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "remove max length before long paste");
	iRet = xuiTestProxySetClipboardText(&tState, sLongPaste);
	XUI_TEST_CHECK(iRet == XUI_OK, "set long clipboard");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all before long paste");
	iRet = xuiInputPaste(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(xuiInputGetText(pInput)) == sizeof(sLongPaste) - 1u &&
		xuiInputGetText(pInput)[sizeof(sLongPaste) - 2u] == 'p', "paste long single-line text");
	iRet = xuiInputSetMaxLength(pInput, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore max length after long paste");
	iRet = xuiInputSetText(pInput, "abcde");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after long paste");

	iRet = xuiInputSetText(pInput, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "set word text");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "word layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "word update");
	iRet = __xuiInputWidgetRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "word render");
	tWorldRect = xuiWidgetGetWorldRect(pInput);
	tTextRect = xuiInputGetTextRect(pInput);
	iRet = __xuiInputWidgetDoubleClick(pContext, tWorldRect.fX + tTextRect.fX + 25.0f, tWorldRect.fY + tTextRect.fY + tTextRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click word");
	iRet = xuiInputGetSelection(pInput, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 5, "double click selects word");
	iRet = xuiInputSetSelection(pInput, 5, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "cursor after double click");
	iRet = __xuiInputWidgetDispatchKey(pContext, XUI_KEY_LEFT, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl left");
	iRet = xuiInputGetSelection(pInput, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 3, "ctrl left word cursor");
	iRet = __xuiInputWidgetDispatchKey(pContext, XUI_KEY_RIGHT, XUI_MOD_CTRL | XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift ctrl right");
	iRet = xuiInputGetSelection(pInput, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 5, "shift ctrl right selection");
	iRet = __xuiInputWidgetDispatchKey(pContext, XUI_TEST_KEY_DELETE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "ab ") == 0, "delete selection by key");
	iRet = xuiInputSetText(pInput, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset word text for backspace");
	iRet = __xuiInputWidgetDispatchKey(pContext, XUI_TEST_KEY_BACKSPACE, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "ab ") == 0, "ctrl backspace word delete");
	iRet = xuiInputSetText(pInput, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset word text for delete");
	iRet = xuiInputSetSelection(pInput, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "cursor to beginning");
	iRet = __xuiInputWidgetDispatchKey(pContext, XUI_TEST_KEY_DELETE, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "cd") == 0, "ctrl delete word delete");
	iRet = xuiInputSetText(pInput, "abcd");
	XUI_TEST_CHECK(iRet == XUI_OK, "set drag move text");
	iRet = xuiInputSetSelection(pInput, 1, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "select drag move range");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move update");
	tWorldRect = xuiWidgetGetWorldRect(pInput);
	tTextRect = xuiInputGetTextRect(pInput);
	iRet = __xuiInputWidgetDrag(pContext,
		tWorldRect.fX + tTextRect.fX + 12.0f,
		tWorldRect.fY + tTextRect.fY + tTextRect.fH * 0.5f,
		tWorldRect.fX + tTextRect.fX + 60.0f,
		tWorldRect.fY + tTextRect.fY + tTextRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "adbc") == 0, "selection drag moves text");
	iRet = xuiInputGetSelection(pInput, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 2 && iEnd == 4, "selection follows moved text");
	iRet = xuiInputSetMaxLength(pInput, 32);
	XUI_TEST_CHECK(iRet == XUI_OK, "raise max length for IME");
	iRet = xuiInputSetText(pInput, "");
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before IME");
	iRet = __xuiInputWidgetDispatchImePreedit(pContext, "nihao");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "IME preedit does not commit text");
	iRet = __xuiInputWidgetDispatchIme(pContext, sNiHao);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), sNiHao) == 0, "IME committed text");

	iRet = xuiInputSetText(pInput, "hide");
	XUI_TEST_CHECK(iRet == XUI_OK, "set password text");
	iRet = xuiInputSetPassword(pInput, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputIsPassword(pInput) && xuiWidgetGetImeMode(pInput) == XUI_IME_DISABLED, "password ime disabled");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "password select all");
	iRet = xuiTestProxySetClipboardText(&tState, "keep");
	XUI_TEST_CHECK(iRet == XUI_OK, "set password clipboard");
	iRet = xuiInputCopy(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTestProxyGetClipboardText(&tState), "keep") == 0, "password direct copy blocked");
	iRet = __xuiInputWidgetDispatchKey(pContext, 'C', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTestProxyGetClipboardText(&tState), "keep") == 0, "password hotkey copy blocked");
	iRet = __xuiInputWidgetDispatchKey(pContext, 'X', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "hide") == 0 && strcmp(xuiTestProxyGetClipboardText(&tState), "keep") == 0, "password hotkey cut blocked");
	pMenu = xuiInputGetMenuWidget(pInput);
	XUI_TEST_CHECK(pMenu != NULL, "password menu widget");
	iRet = xuiInputOpenMenu(pInput, 70.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "password menu opens");
	pItem = xuiMenuGetItem(pMenu, 3);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "password menu cut disabled");
	pItem = xuiMenuGetItem(pMenu, 4);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "password menu copy disabled");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close password menu");
	iRet = xuiInputSetPassword(pInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiInputIsPassword(pInput) && xuiWidgetGetImeMode(pInput) == XUI_IME_AUTO, "password clear ime auto");
	iRet = xuiInputSetText(pInput, "abcde");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text after password");

	iRet = xuiInputSetTextAlign(pInput, XUI_INPUT_ALIGN_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputGetTextAlign(pInput) == XUI_INPUT_ALIGN_RIGHT, "right align");
	iRet = xuiInputSetSelection(pInput, 0, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "right align selection");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "right align layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "right align update");
	iRet = __xuiInputWidgetRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "right align render");
	pInputCache = xuiWidgetGetCacheSurface(pInput, xuiWidgetGetStateId(pInput));
	XUI_TEST_CHECK(pInputCache != NULL, "right align cache");
	tTextRect = xuiInputGetTextRect(pInput);
	tSelectionRect = xuiTestSurfaceGetLastRect(pInputCache);
	XUI_TEST_CHECK(__xuiInputWidgetNear(tSelectionRect.fX, tTextRect.fX + tTextRect.fW - 35.0f), "right align selection x");
	XUI_TEST_CHECK(__xuiInputWidgetNear(tSelectionRect.fW, 14.0f), "right align selection width");

	XUI_TEST_CHECK(strcmp(xuiInputGetMenuTitle(pInput, XUI_INPUT_MENU_COPY), "Copy") == 0, "default menu title");
	iRet = xuiSetLanguage(pContext, XUI_LANGUAGE_ZH);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetMenuTitle(pInput, XUI_INPUT_MENU_COPY), "复制") == 0, "localized menu title");
	iRet = xuiSetLanguage(pContext, XUI_LANGUAGE_EN);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetMenuTitle(pInput, XUI_INPUT_MENU_COPY), "Copy") == 0, "restore English menu title");
	iRet = xuiInputSetMenuTitle(pInput, XUI_INPUT_MENU_COPY, "Copy Text");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetMenuTitle(pInput, XUI_INPUT_MENU_COPY), "Copy Text") == 0, "custom menu title");
	iRet = xuiInputSetMenuTitle(pInput, XUI_INPUT_MENU_COPY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetMenuTitle(pInput, XUI_INPUT_MENU_COPY), "Copy") == 0, "reset menu title");

	iRet = xuiInputSetText(pInput, "menu");
	XUI_TEST_CHECK(iRet == XUI_OK, "set menu text");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all menu text");
	iRet = xuiInputCut(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "cut menu text");
	iRet = xuiInputUndo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "menu") == 0 && xuiInputCanRedo(pInput), "undo menu cut leaves redo");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all menu text after undo");
	pMenu = xuiInputGetMenuWidget(pInput);
	XUI_TEST_CHECK(pMenu != NULL, "menu widget");
	iRet = xuiInputOpenMenu(pInput, 70.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu update");
	iRet = __xuiInputWidgetRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu render");
	pInputCache = xuiWidgetGetCacheSurface(pInput, xuiWidgetGetStateId(pInput));
	XUI_TEST_CHECK(pInputCache != NULL && xuiTestSurfaceGetRectFillColorCount(pInputCache, XUI_COLOR_RGBA(47, 128, 237, 78)) > 0, "selection visible while menu is focused");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 9, "menu item count");
	pItem = xuiMenuGetItem(pMenu, 1);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_NORMAL && pItem->iValue == XUI_INPUT_MENU_REDO &&
		pItem->sText != NULL && strcmp(pItem->sText, "Redo") == 0 &&
		pItem->sShortcut != NULL && strcmp(pItem->sShortcut, "Ctrl+Y") == 0 &&
		(pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "redo item");
	pItem = xuiMenuGetItem(pMenu, 2);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_SEPARATOR, "first separator");
	pItem = xuiMenuGetItem(pMenu, 7);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_SEPARATOR, "second separator");
	pItem = xuiMenuGetItem(pMenu, 4);
	XUI_TEST_CHECK(pItem != NULL && pItem->sText != NULL && strcmp(pItem->sText, "Copy") == 0 && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "copy item");
	pItem = xuiMenuGetItem(pMenu, 8);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "select all disabled when all selected");
	iRet = __xuiInputWidgetClickMenuItem(pContext, pMenu, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "menu redo");
	iRet = xuiInputUndo(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "menu") == 0, "restore after menu redo");
	iRet = xuiInputSelectAll(pInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore selection after menu redo");

	iRet = xuiInputSetReadonly(pInput, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiInputIsReadonly(pInput) && xuiWidgetGetImeMode(pInput) == XUI_IME_DISABLED, "readonly set");
	iRet = xuiInputOpenMenu(pInput, 72.0f, 66.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "open readonly menu");
	pItem = xuiMenuGetItem(pMenu, 3);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly cut disabled");
	pItem = xuiMenuGetItem(pMenu, 4);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "readonly copy enabled");
	pItem = xuiMenuGetItem(pMenu, 5);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly paste disabled");
	pItem = xuiMenuGetItem(pMenu, 6);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly delete disabled");
	iRet = xuiInputSetReadonly(pInput, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetImeMode(pInput) == XUI_IME_AUTO, "readonly clear");

	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close menu");
	iRet = xuiInputSetText(pInput, "menu");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset menu text");
	iRet = xuiInputSetSelection(pInput, 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection for right menu");
	iRet = __xuiInputWidgetRightClick(pContext, 40.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "right click opens menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "right menu layout");
	pItem = xuiMenuGetItem(pMenu, 6);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "delete enabled");
	iRet = __xuiInputWidgetRightClick(pContext, 30.0f, 34.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "right click owner closes menu");
	iRet = xuiInputSetSelection(pInput, 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection for long press menu");
	tWorldRect = xuiWidgetGetWorldRect(pInput);
	tTextRect = xuiInputGetTextRect(pInput);
	iRet = __xuiInputWidgetLongPress(pContext,
		tWorldRect.fX + tTextRect.fX + tTextRect.fW * 0.5f,
		tWorldRect.fY + tTextRect.fY + tTextRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "long press opens menu");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close long press menu");
	iRet = __xuiInputWidgetRightClick(pContext, 40.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "right click opens menu again");
	iRet = __xuiInputWidgetClickMenuItem(pContext, pMenu, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiInputGetText(pInput), "") == 0, "menu delete");
	iRet = xuiInputGetSelection(pInput, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == iEnd, "selection cleared");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed == 0 ) {
		printf("xui_input_widget_test passed\n");
	}
	return iFailed ? 1 : 0;
}
