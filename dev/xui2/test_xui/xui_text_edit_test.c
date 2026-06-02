#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_KEY_BACKSPACE	8
#define XUI_TEST_KEY_DELETE	46

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_text_edit_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_text_edit_change_t {
	int iCount;
	char sText[256];
} xui_text_edit_change_t;

static void __xuiTextEditTestChanged(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_text_edit_change_t* pChange;

	(void)pWidget;
	pChange = (xui_text_edit_change_t*)pUser;
	if ( pChange == NULL ) return;
	pChange->iCount++;
	if ( sText == NULL ) sText = "";
	strncpy(pChange->sText, sText, sizeof(pChange->sText) - 1u);
	pChange->sText[sizeof(pChange->sText) - 1u] = '\0';
}

static int __xuiTextEditTestRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTextEditTestDispatchText(xui_context pContext, uint32_t iCodepoint)
{
	int iRet;

	iRet = xuiInputText(pContext, iCodepoint);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiTextEditTestDispatchKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiTextEditTestDispatchIme(xui_context pContext, const char* sText)
{
	int iRet;

	iRet = xuiInputImeComposition(pContext, sText, -1, 0, (int)strlen(sText));
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiTextEditTestRightClick(xui_context pContext, float fX, float fY)
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

static int __xuiTextEditTestClick(xui_context pContext, float fX, float fY)
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

static int __xuiTextEditTestDoubleClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiTextEditTestClick(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTextEditTestClick(pContext, fX, fY);
}

static int __xuiTextEditTestClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
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
	xui_text_edit_change_t tChange;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTextEdit;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pTextEditCache;
	xui_font pFont;
	xui_text_edit_desc_t tDesc;
	const xui_menu_item_t* pItem;
	const char* sNiHao;
	xui_rect_t tTextRect;
	xui_rect_t tWorldRect;
	float fScrollX;
	float fScrollY;
	int iStart;
	int iEnd;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTextEdit = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pTextEditCache = NULL;
	pFont = NULL;
	iFailed = 0;
	sNiHao = "\xE4\xBD\xA0\xE5\xA5\xBD";
	memset(&tChange, 0, sizeof(tChange));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "test.ttf", 14.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "one\ntwo";
	tDesc.sPlaceholder = "请输入多行文本";
	tDesc.pFont = pFont;
	tDesc.iMaxLength = 128;
	tDesc.bWordWrap = 1;
	tDesc.bLineNumbers = 1;
	tDesc.fLineNumberWidth = 44.0f;
	iRet = xuiTextEditCreate(pContext, &pTextEdit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTextEdit != NULL, "textedit create");
	xuiWidgetSetRect(pTextEdit, (xui_rect_t){24.0f, 24.0f, 320.0f, 150.0f});
	iRet = xuiWidgetAddChild(pRoot, pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "add textedit");
	iRet = xuiTextEditSetChange(pTextEdit, __xuiTextEditTestChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiTextEditTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo") == 0, "initial text");
	XUI_TEST_CHECK(strcmp(xuiTextEditGetPlaceholder(pTextEdit), "请输入多行文本") == 0, "placeholder");
	XUI_TEST_CHECK(xuiTextEditGetMaxLength(pTextEdit) == 128, "max length");
	XUI_TEST_CHECK(xuiTextEditGetLineCount(pTextEdit) >= 2, "line count");
	XUI_TEST_CHECK(xuiTextEditGetWordWrap(pTextEdit) == 1, "word wrap");
	XUI_TEST_CHECK(xuiTextEditGetLineNumbers(pTextEdit) == 1, "line numbers enabled");
	XUI_TEST_CHECK(xuiTextEditGetLineNumberWidth(pTextEdit) == 44.0f, "line number width");
	tTextRect = xuiTextEditGetTextRect(pTextEdit);
	XUI_TEST_CHECK(tTextRect.fX >= 50.0f, "line number gutter shifts text rect");

	iRet = xuiSetFocusWidget(pContext, pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pTextEdit, "focus textedit");
	iRet = __xuiTextEditTestDispatchText(pContext, '!');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo!") == 0, "type exclaim");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo!\n") == 0, "enter newline");
	iRet = __xuiTextEditTestDispatchText(pContext, 'A');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo!\nA") == 0, "type after newline");
	XUI_TEST_CHECK(tChange.iCount >= 3 && strcmp(tChange.sText, "one\ntwo!\nA") == 0, "change callback text");
	iRet = __xuiTextEditTestDispatchKey(pContext, 'A', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTextEditHasSelection(pTextEdit), "ctrl a select all");
	iRet = __xuiTextEditTestDispatchText(pContext, 'a');
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo!\nA") == 0, "ctrl text suppressed");
	iRet = xuiInputSetModifiers(pContext, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear modifiers after ctrl text");

	iRet = xuiTextEditSetText(pTextEdit, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "set word text");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "word layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "word update");
	iRet = __xuiTextEditTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "word render");
	tWorldRect = xuiWidgetGetWorldRect(pTextEdit);
	tTextRect = xuiTextEditGetTextRect(pTextEdit);
	iRet = __xuiTextEditTestDoubleClick(pContext, tWorldRect.fX + tTextRect.fX + 25.0f, tWorldRect.fY + tTextRect.fY + tTextRect.fH * 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK, "double click word");
	iRet = xuiTextEditGetSelection(pTextEdit, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 5, "double click selects word");
	iRet = xuiTextEditSetSelection(pTextEdit, 5, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "cursor after double click");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_KEY_LEFT, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "ctrl left");
	iRet = xuiTextEditGetSelection(pTextEdit, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 3, "ctrl left word cursor");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_KEY_RIGHT, XUI_MOD_CTRL | XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift ctrl right");
	iRet = xuiTextEditGetSelection(pTextEdit, &iStart, &iEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iStart == 3 && iEnd == 5, "shift ctrl right selection");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_TEST_KEY_DELETE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "ab ") == 0, "delete selection by key");
	iRet = xuiTextEditSetText(pTextEdit, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset word text for backspace");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_TEST_KEY_BACKSPACE, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "ab ") == 0, "ctrl backspace word delete");
	iRet = xuiTextEditSetText(pTextEdit, "ab cd");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset word text for delete");
	iRet = xuiTextEditSetSelection(pTextEdit, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "cursor to beginning");
	iRet = __xuiTextEditTestDispatchKey(pContext, XUI_TEST_KEY_DELETE, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "cd") == 0, "ctrl delete word delete");
	iRet = xuiTextEditSetText(pTextEdit, "one\ntwo!\nA");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore edit text");

	iRet = xuiTextEditSelectAll(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTextEditHasSelection(pTextEdit), "select all");
	iRet = xuiTextEditCopy(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTestProxyGetClipboardText(&tState), "one\ntwo!\nA") == 0, "copy");
	iRet = xuiTextEditCut(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "") == 0, "cut");
	XUI_TEST_CHECK(xuiTextEditCanUndo(pTextEdit), "can undo cut");
	iRet = xuiTextEditUndo(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "one\ntwo!\nA") == 0, "undo cut");
	XUI_TEST_CHECK(xuiTextEditCanRedo(pTextEdit), "can redo cut");
	iRet = xuiTextEditRedo(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "") == 0, "redo cut");
	iRet = __xuiTextEditTestDispatchIme(pContext, sNiHao);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), sNiHao) == 0, "IME committed text");
	iRet = xuiTextEditSetText(pTextEdit, "");
	XUI_TEST_CHECK(iRet == XUI_OK, "clear IME text");

	iRet = xuiTestProxySetClipboardText(&tState, "x\r\ny");
	XUI_TEST_CHECK(iRet == XUI_OK, "set clipboard");
	iRet = xuiTextEditPaste(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "x\ny") == 0, "paste multiline");
	iRet = xuiTextEditSetText(pTextEdit, "line01\nline02\nline03\nline04\nline05\nline06\nline07\nline08\nline09\nline10\nline11\nline12");
	XUI_TEST_CHECK(iRet == XUI_OK, "set scroll text");
	iRet = xuiTextEditSetScroll(pTextEdit, 0.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set scroll");
	iRet = xuiTextEditGetScroll(pTextEdit, &fScrollX, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY > 0.0f, "get scroll");

	XUI_TEST_CHECK(strcmp(xuiTextEditGetMenuTitle(pTextEdit, XUI_INPUT_MENU_COPY), "复制") == 0, "default menu title");
	iRet = xuiTextEditSetMenuTitle(pTextEdit, XUI_INPUT_MENU_COPY, "复制文本");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetMenuTitle(pTextEdit, XUI_INPUT_MENU_COPY), "复制文本") == 0, "custom menu title");
	iRet = xuiTextEditSetMenuTitle(pTextEdit, XUI_INPUT_MENU_COPY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetMenuTitle(pTextEdit, XUI_INPUT_MENU_COPY), "复制") == 0, "reset menu title");

	iRet = xuiTextEditSetLineNumbers(pTextEdit, 0, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTextEditGetLineNumbers(pTextEdit) == 0, "line numbers disabled for menu visual check");
	iRet = xuiTextEditSetText(pTextEdit, "menu\ntext");
	XUI_TEST_CHECK(iRet == XUI_OK, "set menu text");
	iRet = xuiTextEditSelectAll(pTextEdit);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all menu text");
	pMenu = xuiTextEditGetMenuWidget(pTextEdit);
	XUI_TEST_CHECK(pMenu != NULL, "menu widget");
	iRet = xuiTextEditOpenMenu(pTextEdit, 70.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open menu");
	pTextEditCache = xuiWidgetGetCacheSurface(pTextEdit, xuiWidgetGetStateId(pTextEdit));
	if ( pTextEditCache != NULL ) {
		xuiTestSurfaceReset(pTextEditCache);
	}
	iRet = xuiWidgetInvalidate(pTextEdit, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate menu selection cache");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu update");
	iRet = __xuiTextEditTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "menu render");
	pTextEditCache = xuiWidgetGetCacheSurface(pTextEdit, xuiWidgetGetStateId(pTextEdit));
	XUI_TEST_CHECK(pTextEditCache != NULL && xuiTestSurfaceGetRectFillColorCount(pTextEditCache, XUI_COLOR_RGBA(47, 128, 237, 78)) > 0, "selection visible while menu is focused");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 8, "menu item count");
	pItem = xuiMenuGetItem(pMenu, 1);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_SEPARATOR, "first separator");
	pItem = xuiMenuGetItem(pMenu, 6);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_SEPARATOR, "second separator");
	pItem = xuiMenuGetItem(pMenu, 3);
	XUI_TEST_CHECK(pItem != NULL && pItem->sText != NULL && strcmp(pItem->sText, "复制") == 0 && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "copy item");
	pItem = xuiMenuGetItem(pMenu, 7);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "select all disabled when all selected");

	iRet = xuiTextEditSetReadonly(pTextEdit, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTextEditIsReadonly(pTextEdit), "readonly set");
	XUI_TEST_CHECK((xuiTextEditGetState(pTextEdit) & XUI_TEXT_EDIT_STATE_READONLY) != 0u, "readonly state");
	iRet = xuiTextEditOpenMenu(pTextEdit, 72.0f, 66.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "open readonly menu");
	pItem = xuiMenuGetItem(pMenu, 2);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly cut disabled");
	pItem = xuiMenuGetItem(pMenu, 3);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "readonly copy enabled");
	pItem = xuiMenuGetItem(pMenu, 4);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly paste disabled");
	pItem = xuiMenuGetItem(pMenu, 5);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "readonly delete disabled");
	iRet = xuiTextEditSetReadonly(pTextEdit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "readonly clear");

	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close menu");
	iRet = xuiTextEditSetText(pTextEdit, "menu");
	XUI_TEST_CHECK(iRet == XUI_OK, "reset menu text");
	iRet = xuiTextEditSetSelection(pTextEdit, 0, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection for right menu");
	iRet = __xuiTextEditTestRightClick(pContext, 40.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "right click opens menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "right menu layout");
	pItem = xuiMenuGetItem(pMenu, 5);
	XUI_TEST_CHECK(pItem != NULL && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "delete enabled");
	iRet = __xuiTextEditTestRightClick(pContext, 30.0f, 34.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pMenu), "right click owner closes menu");
	iRet = __xuiTextEditTestRightClick(pContext, 40.0f, 42.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "right click opens menu again");
	iRet = __xuiTextEditTestClickMenuItem(pContext, pMenu, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTextEditGetText(pTextEdit), "") == 0, "menu delete");
	iRet = xuiTextEditGetSelection(pTextEdit, &iStart, &iEnd);
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
		printf("xui_text_edit_test passed\n");
	}
	return iFailed ? 1 : 0;
}
