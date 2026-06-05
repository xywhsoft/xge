#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_terminal_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_terminal_test_state_t {
	char sInput[512];
	int iInputSize;
	int iResizeCount;
	int iLastCols;
	int iLastRows;
	int iSessionResizeCount;
	int iSessionLastCols;
	int iSessionLastRows;
	int iLinkCount;
	char sTitle[128];
	char sLink[256];
} xui_terminal_test_state_t;

static void __xuiTerminalTestInput(xui_widget pWidget, const uint8_t* pData, int iSize, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;
	int iCopy;

	(void)pWidget;
	if ( pState == NULL || pData == NULL || iSize <= 0 ) return;
	iCopy = iSize;
	if ( pState->iInputSize + iCopy >= (int)sizeof(pState->sInput) ) {
		iCopy = (int)sizeof(pState->sInput) - 1 - pState->iInputSize;
	}
	if ( iCopy > 0 ) {
		memcpy(pState->sInput + pState->iInputSize, pData, (size_t)iCopy);
		pState->iInputSize += iCopy;
		pState->sInput[pState->iInputSize] = '\0';
	}
}

static void __xuiTerminalTestResize(xui_widget pWidget, int iColumns, int iRows, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;

	(void)pWidget;
	if ( pState != NULL ) {
		pState->iResizeCount++;
		pState->iLastCols = iColumns;
		pState->iLastRows = iRows;
	}
}

static void __xuiTerminalTestSessionResize(xui_terminal_session_t* pSession, int iColumns, int iRows, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;

	(void)pSession;
	if ( pState != NULL ) {
		pState->iSessionResizeCount++;
		pState->iSessionLastCols = iColumns;
		pState->iSessionLastRows = iRows;
	}
}

static void __xuiTerminalTestTitle(xui_widget pWidget, const char* sTitle, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;

	(void)pWidget;
	if ( pState != NULL && sTitle != NULL ) {
		strncpy(pState->sTitle, sTitle, sizeof(pState->sTitle) - 1u);
		pState->sTitle[sizeof(pState->sTitle) - 1u] = '\0';
	}
}

static void __xuiTerminalTestLink(xui_widget pWidget, const char* sUrl, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;

	(void)pWidget;
	if ( pState != NULL && sUrl != NULL ) {
		pState->iLinkCount++;
		strncpy(pState->sLink, sUrl, sizeof(pState->sLink) - 1u);
		pState->sLink[sizeof(pState->sLink) - 1u] = '\0';
	}
}

static int __xuiTerminalTestClick(xui_context pContext, float fX, float fY)
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

static int __xuiTerminalTestDoubleClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiTerminalTestClick(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTerminalTestClick(pContext, fX, fY);
}

static int __xuiTerminalTestRightClick(xui_context pContext, float fX, float fY)
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

static int __xuiTerminalTestKey(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static xui_style_property_t __xuiTerminalTestColorProp(const char* sName, uint32_t iColor)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.tValue.iSize = sizeof(tProperty.tValue);
	tProperty.tValue.iType = XUI_STYLE_VALUE_COLOR;
	tProperty.tValue.iColor = iColor;
	return tProperty;
}

static xui_style_property_t __xuiTerminalTestFloatProp(const char* sName, float fValue)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.tValue.iSize = sizeof(tProperty.tValue);
	tProperty.tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tProperty.tValue.fFloat = fValue;
	return tProperty;
}

static int __xuiTerminalTestDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
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

static int __xuiTerminalTestClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	return __xuiTerminalTestClick(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_terminal_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTerminal;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pCache;
	xui_font pFont;
	const xui_menu_item_t* pItem;
	xui_terminal_desc_t tDesc;
	xui_terminal_cell_t tCell;
	xui_terminal_session_desc_t tSessionDesc;
	xui_terminal_process_desc_t tProcessDesc;
	xui_terminal_session_t* pSession;
	xui_style_property_t arrStyle[5];
	xui_rect_t tWorld;
	uint32_t iStyledForeground;
	uint32_t iStyledPaletteRed;
	char sBuffer[2048];
	int iFailed;
	int iRet;
	int iLine;
	int iColumn;
	int iLength;
	int iStartColumn;
	int iCursorX;
	int iCursorY;
	int iFullTextDraws;
	int iPartialTextDraws;
	int iSessionResizeBefore;
	int iPoll;
	int iPollBytes;

	memset(&tProxyState, 0, sizeof(tProxyState));
	memset(&tState, 0, sizeof(tState));
	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tProcessDesc, 0, sizeof(tProcessDesc));
	pContext = NULL;
	pRoot = NULL;
	pTerminal = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pCache = NULL;
	pFont = NULL;
	pSession = NULL;
	iFailed = 0;
	iStyledForeground = XUI_COLOR_RGBA(210, 211, 212, 255);
	iStyledPaletteRed = XUI_COLOR_RGBA(101, 102, 103, 255);

	xuiTestProxyInit(&tProxyState);
	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "terminal", 8, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "font set");
	iRet = xuiTestSurfaceCreate(&tProxyState, &pTarget, 360, 240, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 240.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iColumns = 10;
	tDesc.iRows = 4;
	tDesc.iScrollbackLimit = 8;
	tDesc.iParseBudgetBytes = 4;
	tDesc.fCellWidth = 8.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 4.0f;
	iRet = xuiTerminalCreate(pContext, &pTerminal, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTerminal != NULL, "terminal create");
	(void)xuiWidgetSetRect(pTerminal, (xui_rect_t){10.0f, 10.0f, 120.0f, 80.0f});
	(void)xuiWidgetAddChild(pRoot, pTerminal);
	(void)xuiTerminalSetInputCallback(pTerminal, __xuiTerminalTestInput, &tState);
	(void)xuiTerminalSetResizeCallback(pTerminal, __xuiTerminalTestResize, &tState);
	(void)xuiTerminalSetTitleCallback(pTerminal, __xuiTerminalTestTitle, &tState);
	(void)xuiTerminalSetLinkCallback(pTerminal, __xuiTerminalTestLink, &tState);

	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "terminal.background.color") != 0u, "terminal background style registered");
	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "terminal.palette.1") != 0u, "terminal palette style registered");
	arrStyle[0] = __xuiTerminalTestColorProp("terminal.foreground.color", iStyledForeground);
	arrStyle[1] = __xuiTerminalTestColorProp("terminal.palette.1", iStyledPaletteRed);
	arrStyle[2] = __xuiTerminalTestFloatProp("terminal.cell.width", 10.0f);
	arrStyle[3] = __xuiTerminalTestFloatProp("terminal.cell.height", 16.0f);
	arrStyle[4] = __xuiTerminalTestFloatProp("terminal.padding", 2.0f);
	iRet = xuiWidgetSetInlineStyle(pTerminal, arrStyle, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal inline style");
	iRet = xuiTerminalFit(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTerminalGetColumns(pTerminal) == 11 && xuiTerminalGetRows(pTerminal) == 4, "terminal style fit");
	XUI_TEST_CHECK(xuiTerminalGetPalette(pTerminal, 1) == iStyledPaletteRed, "terminal style palette getter");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style clear");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[31mR\x1b[0mZ");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style flush");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'R' && tCell.iFgColor == iStyledPaletteRed, "terminal style ansi palette");
	iRet = xuiTerminalGetCell(pTerminal, 1, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Z' && tCell.iFgColor == iStyledForeground, "terminal style foreground reset");
	iRet = xuiWidgetSetInlineStyle(pTerminal, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal inline style clear");
	iRet = xuiTerminalResize(pTerminal, 10, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style restore size");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style restore clear");
	XUI_TEST_CHECK(xuiTerminalGetPalette(pTerminal, 1) == XUI_COLOR_RGBA(205, 49, 49, 255), "terminal palette restores base");

	iRet = xuiTerminalWriteText(pTerminal, "abc\r\n\x1b[31mR\x1b[0m\r\nline3\r\nline4\r\nline5");
	XUI_TEST_CHECK(iRet == XUI_OK, "write text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'l', "scroll visible row");
	iRet = xuiTerminalFindText(pTerminal, "abc", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 0, "find scrollback text");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[2;3Hxy\x1b[1D!\x1b[2K\rz");
	XUI_TEST_CHECK(iRet == XUI_OK, "write cursor sequence");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush cursor sequence");
	iRet = xuiTerminalGetCursor(pTerminal, &iCursorX, &iCursorY);
	XUI_TEST_CHECK(iRet == XUI_OK && iCursorY == 1, "cursor query");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'z', "erase line sequence");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear default tab stop");
	iRet = xuiTerminalWriteText(pTerminal, "a\tb");
	XUI_TEST_CHECK(iRet == XUI_OK, "write default tab stop");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush default tab stop");
	iRet = xuiTerminalGetCell(pTerminal, 8, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'b', "default tab stop column");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear custom tab stop");
	iRet = xuiTerminalWriteText(pTerminal, "ab\x1bH\r\tX");
	XUI_TEST_CHECK(iRet == XUI_OK, "write custom tab stop");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush custom tab stop");
	iRet = xuiTerminalGetCell(pTerminal, 2, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "custom tab stop column");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear current tab stop screen");
	iRet = xuiTerminalWriteText(pTerminal, "ab\x1bH\x1b[0g\r\tY");
	XUI_TEST_CHECK(iRet == XUI_OK, "write clear current tab stop");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush clear current tab stop");
	iRet = xuiTerminalGetCell(pTerminal, 8, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Y', "clear current tab stop falls back");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear all tab stops screen");
	iRet = xuiTerminalWriteText(pTerminal, "ab\x1bH\x1b[3g\r\tZ");
	XUI_TEST_CHECK(iRet == XUI_OK, "write clear all tab stops");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush clear all tab stops");
	iRet = xuiTerminalGetCell(pTerminal, 9, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Z', "clear all tab stops falls to last column");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear dirty row cache screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear dirty row cache scrollback");
	iRet = xuiTerminalWriteText(pTerminal, "row0\r\nrow1\r\nrow2");
	XUI_TEST_CHECK(iRet == XUI_OK, "write dirty row cache seed");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush dirty row cache seed");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	if ( pCache != NULL ) xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare dirty row full cache");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL, "dirty row cache surface");
	iFullTextDraws = xuiTestSurfaceGetTextDrawCount(pCache);
	XUI_TEST_CHECK(iFullTextDraws >= 3, "dirty row full render text count");
	xuiTestSurfaceReset(pCache);
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[2;1HROW1");
	XUI_TEST_CHECK(iRet == XUI_OK, "write dirty row update");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush dirty row update");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare dirty row partial cache");
	iPartialTextDraws = xuiTestSurfaceGetTextDrawCount(pCache);
	XUI_TEST_CHECK(iPartialTextDraws > 0 && iPartialTextDraws < iFullTextDraws, "dirty row partial render text count");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'R', "dirty row content updated");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear color");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[38;5;196mX\x1b[0m");
	XUI_TEST_CHECK(iRet == XUI_OK, "write color");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush color");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X' && tCell.iFgColor == xuiTerminalGetPalette(pTerminal, 196), "256 color cell");

	iRet = xuiTerminalWriteText(pTerminal, "\x1b]0;XUI title\a");
	XUI_TEST_CHECK(iRet == XUI_OK, "write title");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sTitle, "XUI title") == 0, "osc title");

	iRet = xuiTerminalWriteText(pTerminal, "\x1b[?1049hALT\x1b[?1049l");
	XUI_TEST_CHECK(iRet == XUI_OK, "write alt");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush alt");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "main screen restored");

	iRet = xuiTerminalInputText(pTerminal, "go");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "go") == 0, "input callback");
	iRet = xuiTerminalSetBracketedPaste(pTerminal, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set bracketed paste");
	iRet = xuiTerminalPasteText(pTerminal, "paste");
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[200~paste\x1b[201~") != NULL, "bracketed paste");

	iRet = xuiTerminalSelectAll(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "select all");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "X") != NULL, "selection text");
	iRet = xuiTerminalCopySelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(xuiTestProxyGetClipboardText(&tProxyState), "X") != NULL, "copy selection");
	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer)) == 0, "clear selection");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear search screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear search scrollback");
	iRet = xuiTerminalWriteText(pTerminal, "one beta\r\nrow beta\r\nwide \xe4\xb8\xad");
	XUI_TEST_CHECK(iRet == XUI_OK, "write search text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush search text");
	iRet = xuiTerminalFindText(pTerminal, "beta", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 4, "find first beta");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 4 && iLength == 4, "find match state");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	if ( pCache != NULL ) xuiTestSurfaceReset(pCache);
	iRet = xuiWidgetInvalidate(pTerminal, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate search cache");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render search cache");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(255, 197, 66, 110)) > 0, "search highlight rendered");
	iRet = xuiTerminalFindNext(pTerminal, "beta", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 1 && iColumn == 4, "find next beta");
	iRet = xuiTerminalFindNext(pTerminal, "beta", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 4, "find next wraps");
	iRet = xuiTerminalFindPrev(pTerminal, "beta", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 1 && iColumn == 4, "find prev wraps");
	iRet = xuiTerminalFindText(pTerminal, "BETA", XUI_TERMINAL_SEARCH_CASE_SENSITIVE, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 0 && xuiTerminalGetFindMatch(pTerminal, NULL, NULL, NULL) == 0, "case sensitive miss clears match");
	iRet = xuiTerminalFindText(pTerminal, "\xe4\xb8\xad", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 2 && iColumn == 5, "find cjk column");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLength == 2, "find cjk width");
	iRet = xuiTerminalClearFind(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTerminalGetFindMatch(pTerminal, NULL, NULL, NULL) == 0, "clear find");

	iRet = xuiWidgetSetRect(pTerminal, (xui_rect_t){10.0f, 10.0f, 330.0f, 80.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "resize terminal for link test");
	iRet = xuiTerminalResize(pTerminal, 40, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal link columns");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear link screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear link scrollback");
	(void)xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 0.0f);
	iRet = xuiTerminalWriteText(pTerminal, "open https://example.com/path, done");
	XUI_TEST_CHECK(iRet == XUI_OK, "write link text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush link text");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 8, sBuffer, (int)sizeof(sBuffer), &iStartColumn, &iLength);
	XUI_TEST_CHECK(iRet == 24 && strcmp(sBuffer, "https://example.com/path") == 0 && iStartColumn == 5 && iLength == 24, "terminal link hit");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 30, sBuffer, (int)sizeof(sBuffer), NULL, NULL);
	XUI_TEST_CHECK(iRet == 0 && sBuffer[0] == '\0', "terminal link miss after trim");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "link layout prepare");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	if ( pCache != NULL ) xuiTestSurfaceReset(pCache);
	iRet = xuiInputPointerMove(pContext, tWorld.fX + 4.0f + 8.0f * 8.0f + 0.5f, tWorld.fY + 4.0f + 8.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal link hover move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal link hover dispatch");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render link hover cache");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(86, 156, 214, 255)) > 0, "link hover underline rendered");
	iRet = __xuiTerminalTestClick(pContext, tWorld.fX + 4.0f + 8.0f * 8.0f + 0.5f, tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLinkCount == 1 && strcmp(tState.sLink, "https://example.com/path") == 0, "terminal link click callback");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear osc link screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear osc link scrollback");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b]8;;https://example.com/osc\aOpen\x1b]8;;\a plain");
	XUI_TEST_CHECK(iRet == XUI_OK, "write osc8 bel link");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush osc8 bel link");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 1, sBuffer, (int)sizeof(sBuffer), &iStartColumn, &iLength);
	XUI_TEST_CHECK(iRet == 23 && strcmp(sBuffer, "https://example.com/osc") == 0 && iStartColumn == 0 && iLength == 4, "terminal osc8 bel link hit");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iLinkId != 0, "terminal osc8 cell link id");
	iRet = xuiTerminalGetCell(pTerminal, 5, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iLinkId == 0, "terminal osc8 close clears cell link id");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 5, sBuffer, (int)sizeof(sBuffer), NULL, NULL);
	XUI_TEST_CHECK(iRet == 0 && sBuffer[0] == '\0', "terminal osc8 miss after close");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal osc8 layout prepare");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	tState.iLinkCount = 0;
	tState.sLink[0] = '\0';
	iRet = __xuiTerminalTestClick(pContext, tWorld.fX + 4.0f + 1.0f * 8.0f + 0.5f, tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLinkCount == 1 && strcmp(tState.sLink, "https://example.com/osc") == 0, "terminal osc8 click callback");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear osc st link screen");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b]8;;mailto:test@example.com\x1b\\Mail\x1b]8;;\x1b\\");
	XUI_TEST_CHECK(iRet == XUI_OK, "write osc8 st link");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush osc8 st link");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 2, sBuffer, (int)sizeof(sBuffer), &iStartColumn, &iLength);
	XUI_TEST_CHECK(iRet == 23 && strcmp(sBuffer, "mailto:test@example.com") == 0 && iStartColumn == 0 && iLength == 4, "terminal osc8 st link hit");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear osc scrollback link screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear osc scrollback link buffer");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b]8;;https://example.com/scroll\aScroll\x1b]8;;\a\r\nline1\r\nline2\r\nline3\r\nline4");
	XUI_TEST_CHECK(iRet == XUI_OK, "write osc8 scrollback link");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush osc8 scrollback link");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 2, sBuffer, (int)sizeof(sBuffer), &iStartColumn, &iLength);
	XUI_TEST_CHECK(iRet == 26 && strcmp(sBuffer, "https://example.com/scroll") == 0 && iStartColumn == 0 && iLength == 6, "terminal osc8 scrollback link hit");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 7, sBuffer, (int)sizeof(sBuffer), NULL, NULL);
	XUI_TEST_CHECK(iRet == 0 && sBuffer[0] == '\0', "terminal osc8 scrollback link miss outside range");
	(void)xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 0.0f);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal osc8 scrollback layout prepare");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	tState.iLinkCount = 0;
	tState.sLink[0] = '\0';
	iRet = __xuiTerminalTestClick(pContext, tWorld.fX + 4.0f + 2.0f * 8.0f + 0.5f, tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iLinkCount == 1 && strcmp(tState.sLink, "https://example.com/scroll") == 0, "terminal osc8 scrollback click callback");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear selection screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear selection scrollback");
	iRet = xuiTerminalWriteText(pTerminal, "alph beta\r\nsecond line");
	XUI_TEST_CHECK(iRet == XUI_OK, "write selection text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush selection text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection layout prepare");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = __xuiTerminalTestDrag(pContext,
		tWorld.fX + 4.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f,
		tWorld.fX + 4.0f + 4.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal drag select");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 4 && strcmp(sBuffer, "alph") == 0, "drag selection text");
	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear drag selection");
	iRet = __xuiTerminalTestDoubleClick(pContext,
		tWorld.fX + 4.0f + 7.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal double click");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 4 && strcmp(sBuffer, "beta") == 0, "double click selects word");
	iRet = __xuiTerminalTestClick(pContext,
		tWorld.fX + 4.0f + 7.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal third click");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 9 && strcmp(sBuffer, "alph beta") == 0, "triple click selects row");
	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear row selection");
	iRet = __xuiTerminalTestDrag(pContext,
		tWorld.fX + 4.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f,
		tWorld.fX + 4.0f + 4.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal base range drag");
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "set shift for terminal");
	iRet = __xuiTerminalTestClick(pContext,
		tWorld.fX + 4.0f + 6.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 1.0f * 16.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal shift click");
	iRet = xuiInputSetModifiers(pContext, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear shift for terminal");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 16 && strcmp(sBuffer, "alph beta\nsecond") == 0, "shift click extends selection");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	if ( pCache != NULL ) xuiTestSurfaceReset(pCache);
	iRet = xuiWidgetInvalidate(pTerminal, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate selection cache");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render selection cache");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(64, 130, 220, 95)) > 0, "selection overlay rendered");

	pMenu = xuiTerminalGetMenuWidget(pTerminal);
	XUI_TEST_CHECK(pMenu != NULL, "terminal menu widget");
	iRet = xuiTestProxySetClipboardText(&tProxyState, "menu-paste");
	XUI_TEST_CHECK(iRet == XUI_OK, "set terminal clipboard");
	iRet = xuiTerminalSetBracketedPaste(pTerminal, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable bracketed paste for menu");
	iRet = xuiTerminalOpenMenu(pTerminal, 44.0f, 44.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "open terminal menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal menu layout");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 8, "terminal menu item count");
	pItem = xuiMenuGetItem(pMenu, 0);
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_COPY && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "terminal copy menu enabled");
	pItem = xuiMenuGetItem(pMenu, 1);
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_PASTE && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "terminal paste menu enabled");
	pItem = xuiMenuGetItem(pMenu, 3);
	XUI_TEST_CHECK(pItem != NULL && pItem->iType == XUI_MENU_ITEM_SEPARATOR, "terminal menu separator");
	pItem = xuiMenuGetItem(pMenu, 7);
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_FIND && (pItem->iState & XUI_MENU_ITEM_ENABLED) == 0u, "terminal find menu disabled placeholder");
	iRet = xuiMenuSetHoverIndex(pMenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal copy menu hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK((iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP) && !xuiMenuIsOpen(pMenu), "terminal menu copy click");
	XUI_TEST_CHECK(strcmp(xuiTestProxyGetClipboardText(&tProxyState), "alph beta\nsecond") == 0, "terminal menu copy text");

	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiTerminalOpenMenu(pTerminal, 44.0f, 44.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen terminal paste menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal paste menu layout");
	iRet = xuiMenuSetHoverIndex(pMenu, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal paste menu hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK((iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP) && strstr(tState.sInput, "alph beta\nsecond") != NULL, "terminal menu paste");

	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before terminal find menu");
	iRet = xuiTerminalClearFind(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear stale terminal find menu state");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear terminal find menu screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear terminal find menu scrollback");
	iRet = xuiTerminalWriteText(pTerminal, "alpha beta alpha beta");
	XUI_TEST_CHECK(iRet == XUI_OK, "write terminal find menu text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush terminal find menu text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu layout prepare");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = __xuiTerminalTestDrag(pContext,
		tWorld.fX + 4.0f + 6.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f,
		tWorld.fX + 4.0f + 10.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu drag selection");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 4 && strcmp(sBuffer, "beta") == 0, "terminal find menu selection text");
	iRet = xuiTerminalOpenMenu(pTerminal, 44.0f, 44.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "open terminal find menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu layout");
	pItem = xuiMenuGetItem(pMenu, 7);
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_FIND && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "terminal find menu enabled by selection");
	iRet = xuiMenuSetHoverIndex(pMenu, 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "terminal find menu commit selection");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 6 && iLength == 4, "terminal find menu first match");
	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu clear selection");
	iRet = xuiTerminalOpenMenu(pTerminal, 44.0f, 44.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "open terminal find next menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find next menu layout");
	pItem = xuiMenuGetItem(pMenu, 7);
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_FIND && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "terminal find menu enabled by existing query");
	iRet = xuiMenuSetHoverIndex(pMenu, 7);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find next menu hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "terminal find menu commit next");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 17 && iLength == 4, "terminal find menu next match");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before menu clear");
	iRet = xuiTerminalWriteText(pTerminal, "hotcopy");
	XUI_TEST_CHECK(iRet == XUI_OK, "write hotcopy");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush hotcopy");
	iRet = xuiTerminalSelectAll(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "select hotcopy");
	iRet = __xuiTerminalTestKey(pContext, 'C', XUI_MOD_CTRL | XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(xuiTestProxyGetClipboardText(&tProxyState), "hotcopy") != NULL, "terminal ctrl shift copy");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiTestProxySetClipboardText(&tProxyState, "hotpaste");
	XUI_TEST_CHECK(iRet == XUI_OK, "set hotpaste clipboard");
	iRet = __xuiTerminalTestKey(pContext, 'V', XUI_MOD_CTRL | XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "hotpaste") != NULL, "terminal ctrl shift paste");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = __xuiTerminalTestKey(pContext, 'A', XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "\x1b" "a") == 0, "terminal alt printable");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = __xuiTerminalTestKey(pContext, 'C', XUI_MOD_CTRL | XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 2 && (uint8_t)tState.sInput[0] == 0x1bu && (uint8_t)tState.sInput[1] == 0x03u, "terminal ctrl alt control");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = __xuiTerminalTestKey(pContext, XUI_KEY_LEFT, XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "\x1b[1;3D") == 0, "terminal alt left");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = __xuiTerminalTestKey(pContext, XUI_KEY_PAGE_DOWN, XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "\x1b[6;3~") == 0, "terminal alt page down");
	iRet = xuiTerminalOpenMenu(pTerminal, 44.0f, 44.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "open terminal clear menu");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal clear menu layout");
	iRet = xuiMenuSetHoverIndex(pMenu, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal clear menu hover");
	iRet = xuiMenuCommitHover(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK || iRet == XUI_EVENT_DISPATCH_STOP, "terminal menu clear screen");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && (tCell.iCodepoint == 0u || tCell.iCodepoint == ' '), "terminal menu clear screen cell");
	iRet = __xuiTerminalTestRightClick(pContext, tWorld.fX + 12.0f, tWorld.fY + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu), "terminal right click opens menu");
	iRet = xuiMenuClose(pMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close terminal right menu");

	memset(&tSessionDesc, 0, sizeof(tSessionDesc));
	tSessionDesc.iSize = sizeof(tSessionDesc);
	tSessionDesc.bEcho = 1;
	tSessionDesc.sPrompt = "$ ";
	tSessionDesc.onResize = __xuiTerminalTestSessionResize;
	tSessionDesc.pResizeUser = &tState;
	pSession = xuiTerminalCreateFakeSession(&tSessionDesc);
	XUI_TEST_CHECK(pSession != NULL, "fake session create");
	iRet = xuiTerminalAttachSession(pTerminal, pSession);
	XUI_TEST_CHECK(iRet == XUI_OK, "attach session");
	XUI_TEST_CHECK(tState.iSessionResizeCount > 0 && tState.iSessionLastCols == xuiTerminalGetColumns(pTerminal) && tState.iSessionLastRows == xuiTerminalGetRows(pTerminal), "session initial resize callback");
	iSessionResizeBefore = tState.iSessionResizeCount;
	iRet = xuiTerminalSessionSetResizeCallback(pSession, __xuiTerminalTestSessionResize, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "session resize callback set");
	XUI_TEST_CHECK(tState.iSessionResizeCount > iSessionResizeBefore && tState.iSessionLastCols == xuiTerminalGetColumns(pTerminal) && tState.iSessionLastRows == xuiTerminalGetRows(pTerminal), "session resize callback set notify");
	iRet = xuiTerminalInputText(pTerminal, "echo");
	XUI_TEST_CHECK(iRet == XUI_OK, "session input");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet > 0 && strstr(sBuffer, "$ echo") != NULL, "fake session echo");

	iSessionResizeBefore = tState.iSessionResizeCount;
	iRet = xuiTerminalResize(pTerminal, 12, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iResizeCount > 0 && tState.iLastCols == 12 && tState.iLastRows == 5, "resize callback");
	XUI_TEST_CHECK(tState.iSessionResizeCount > iSessionResizeBefore && tState.iSessionLastCols == 12 && tState.iSessionLastRows == 5, "session resize callback");
	iRet = xuiTerminalSessionPoll(pSession);
	XUI_TEST_CHECK(iRet == XUI_OK, "fake session poll");
	XUI_TEST_CHECK(xuiTerminalSessionIsRunning(pSession), "fake session running");
	iRet = xuiTerminalSessionTerminate(pSession);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiTerminalSessionIsRunning(pSession), "fake session terminate");
	xuiTerminalSessionDestroy(pSession);
	pSession = NULL;

#if defined(_WIN32) || defined(_WIN64)
	tProcessDesc.iSize = sizeof(tProcessDesc);
	tProcessDesc.sCommandLine = "cmd.exe /d /c echo xuiok";
	tProcessDesc.onResize = __xuiTerminalTestSessionResize;
	tProcessDesc.pResizeUser = &tState;
	pSession = xuiTerminalCreateProcessSession(&tProcessDesc);
	XUI_TEST_CHECK(pSession != NULL, "process session create");
	iRet = xuiTerminalAttachSession(pTerminal, pSession);
	XUI_TEST_CHECK(iRet == XUI_OK, "process session attach");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "process session clear");
	iPollBytes = 0;
	for ( iPoll = 0; iPoll < 200; iPoll++ ) {
		iRet = xuiTerminalSessionPoll(pSession);
		XUI_TEST_CHECK(iRet >= 0, "process session poll");
		iPollBytes += iRet;
		iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
		XUI_TEST_CHECK(iRet >= 0, "process session serialize");
		if ( strstr(sBuffer, "xuiok") != NULL ) break;
		Sleep(10);
	}
	XUI_TEST_CHECK(iPollBytes > 0 && strstr(sBuffer, "xuiok") != NULL, "process session output");
	(void)xuiTerminalSessionTerminate(pSession);
	xuiTerminalSessionDestroy(pSession);
	pSession = NULL;

	memset(&tProcessDesc, 0, sizeof(tProcessDesc));
	tProcessDesc.iSize = sizeof(tProcessDesc);
	tProcessDesc.sCommandLine = "cmd.exe /d /c ping -n 2 127.0.0.1 >nul";
	tProcessDesc.iFlags = XUI_TERMINAL_PROCESS_CONPTY;
	tProcessDesc.iColumns = 20;
	tProcessDesc.iRows = 5;
	tProcessDesc.onResize = __xuiTerminalTestSessionResize;
	tProcessDesc.pResizeUser = &tState;
	pSession = xuiTerminalCreateProcessSession(&tProcessDesc);
	XUI_TEST_CHECK(pSession != NULL, "conpty session create");
	iRet = xuiTerminalAttachSession(pTerminal, pSession);
	XUI_TEST_CHECK(iRet == XUI_OK, "conpty session attach");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "conpty session clear");
	iSessionResizeBefore = tState.iSessionResizeCount;
	iRet = xuiTerminalResize(pTerminal, 40, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iSessionResizeCount > iSessionResizeBefore && tState.iSessionLastCols == 40 && tState.iSessionLastRows == 6, "conpty resize propagation");
	iRet = xuiTerminalSessionPoll(pSession);
	XUI_TEST_CHECK(iRet >= 0, "conpty lifecycle poll");
	iRet = xuiTerminalSessionTerminate(pSession);
	XUI_TEST_CHECK(iRet == XUI_OK, "conpty session terminate");
	xuiTerminalSessionDestroy(pSession);
	pSession = NULL;
#endif

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetTextDrawCount(pCache) > 0, "terminal render text");

cleanup:
	if ( pSession != NULL ) xuiTerminalSessionDestroy(pSession);
	if ( pTarget != NULL ) tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_terminal_test passed\n");
	return 0;
}
