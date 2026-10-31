#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
	int iInputCalls;
	int iInputBytes;
	int iResizeCount;
	int iLastCols;
	int iLastRows;
	int iSessionResizeCount;
	int iSessionLastCols;
	int iSessionLastRows;
	int iLinkCount;
	char sTitle[1024];
	char sLink[1024];
} xui_terminal_test_state_t;

typedef struct xui_terminal_destroy_test_state_t {
	xui_widget* ppWidget;
	int iCalls;
} xui_terminal_destroy_test_state_t;

typedef struct xui_terminal_session_destroy_test_state_t {
	xui_terminal_session_t** ppSession;
	int iCalls;
} xui_terminal_session_destroy_test_state_t;

static xui_terminal_destroy_test_state_t* g_pTerminalClipboardDestroyState;

static void __xuiTerminalTestDestroyWidget(xui_widget pWidget, xui_terminal_destroy_test_state_t* pState)
{
	if ( pState == NULL ) return;
	pState->iCalls++;
	if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	xuiWidgetDestroy(pWidget);
}

static void __xuiTerminalTestDestroyOnInput(xui_widget pWidget, const uint8_t* pData, int iSize, void* pUser)
{
	(void)pData;
	(void)iSize;
	__xuiTerminalTestDestroyWidget(pWidget, (xui_terminal_destroy_test_state_t*)pUser);
}

static void __xuiTerminalTestDestroyOnResize(xui_widget pWidget, int iColumns, int iRows, void* pUser)
{
	(void)iColumns;
	(void)iRows;
	__xuiTerminalTestDestroyWidget(pWidget, (xui_terminal_destroy_test_state_t*)pUser);
}

static void __xuiTerminalTestDestroyOnTitle(xui_widget pWidget, const char* sTitle, void* pUser)
{
	(void)sTitle;
	__xuiTerminalTestDestroyWidget(pWidget, (xui_terminal_destroy_test_state_t*)pUser);
}

static void __xuiTerminalTestDestroySessionOnResize(xui_terminal_session_t* pSession, int iColumns, int iRows, void* pUser)
{
	xui_terminal_session_destroy_test_state_t* pState = (xui_terminal_session_destroy_test_state_t*)pUser;

	(void)iColumns;
	(void)iRows;
	if ( pState == NULL ) return;
	pState->iCalls++;
	if ( pState->ppSession != NULL ) *pState->ppSession = NULL;
	xuiTerminalSessionDestroy(pSession);
}

static int __xuiTerminalTestDestroyOnClipboardRead(xui_proxy pProxy, char* sText, int iCapacity)
{
	xui_test_proxy_state_t* pProxyState;
	xui_widget pWidget;
	int iLength;

	if ( pProxy == NULL || pProxy->pUser == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxyState = (xui_test_proxy_state_t*)pProxy->pUser;
	iLength = (int)strlen(pProxyState->sClipboard);
	if ( sText != NULL && iCapacity > 0 ) {
		int iCopy = (iLength < iCapacity - 1) ? iLength : iCapacity - 1;
		if ( iCopy > 0 ) memcpy(sText, pProxyState->sClipboard, (size_t)iCopy);
		sText[iCopy] = '\0';
	}
	if ( g_pTerminalClipboardDestroyState != NULL &&
	     g_pTerminalClipboardDestroyState->ppWidget != NULL ) {
		pWidget = *g_pTerminalClipboardDestroyState->ppWidget;
		if ( pWidget != NULL ) {
			__xuiTerminalTestDestroyWidget(pWidget, g_pTerminalClipboardDestroyState);
		}
	}
	return iLength;
}

static void __xuiTerminalTestInput(xui_widget pWidget, const uint8_t* pData, int iSize, void* pUser)
{
	xui_terminal_test_state_t* pState = (xui_terminal_test_state_t*)pUser;
	int iCopy;

	(void)pWidget;
	if ( pState == NULL || pData == NULL || iSize <= 0 ) return;
	pState->iInputCalls++;
	pState->iInputBytes += iSize;
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

static int __xuiTerminalTestFailSelection(xui_widget pWidget,
	const xui_edit_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	return pEvent != NULL && pEvent->iType == XUI_EDIT_EVENT_SELECTION_CHANGED ?
		XUI_ERROR_BACKEND_FAILED : XUI_OK;
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

static int __xuiTerminalTestCreateDisposable(xui_context pContext, xui_widget pRoot,
	xui_font pFont, xui_widget* ppWidget)
{
	xui_terminal_desc_t tDesc;
	int iRet;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iColumns = 10;
	tDesc.iRows = 4;
	tDesc.iScrollbackLimit = 8;
	tDesc.iParseBudgetBytes = 64;
	tDesc.fCellWidth = 8.0f;
	tDesc.fCellHeight = 16.0f;
	tDesc.fPadding = 4.0f;
	iRet = xuiTerminalCreate(pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pRoot, *ppWidget);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(*ppWidget);
		*ppWidget = NULL;
		return iRet;
	}
	return xuiWidgetSetRect(*ppWidget, (xui_rect_t){150.0f, 10.0f, 120.0f, 80.0f});
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_terminal_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTerminal;
	xui_widget pDisposable;
	xui_widget pBadTerminal;
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pCache;
	xui_font pFont;
	const xui_menu_item_t* pItem;
	xui_terminal_desc_t tDesc;
	xui_terminal_desc_t tBadDesc;
	xui_terminal_cell_t tCell;
	xui_terminal_stats_t tStats;
	xui_terminal_session_desc_t tSessionDesc;
	xui_terminal_process_desc_t tProcessDesc;
	xui_terminal_ssh_desc_t tSshDesc;
	xui_terminal_session_t* pSession;
	xui_terminal_destroy_test_state_t tDestroyState;
	xui_terminal_session_destroy_test_state_t tSessionDestroyState;
	xui_style_property_t arrStyle[6];
	xui_event_t tPointer;
	xui_rect_t tCursorRect;
	xui_rect_t tTextRect;
	xui_rect_t tContent;
	xui_rect_t tWorld;
	xui_thickness_t tWidgetPadding;
	xui_vec2_t tNatural;
	xui_vec2_t tMeasured;
	uint32_t iStyledForeground;
	uint32_t iStyledPaletteRed;
	char sBuffer[2048];
	char sSmallBuffer[8];
	char sLongTitle[512];
	char sLongUrl[512];
	char sLongOsc[1152];
	char* sOversizedOsc;
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
	int iInputCallsBefore;
	int iInputBytesBefore;
	int iPoll;
	int iPollBytes;
	int i;
	int iOscSize;
	float fScrollY;
	float fMaxY;
	float fCellWidth;
	float fCellHeight;
	float fPadding;

	memset(&tProxyState, 0, sizeof(tProxyState));
	memset(&tState, 0, sizeof(tState));
	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tBadDesc, 0, sizeof(tBadDesc));
	memset(&tStats, 0, sizeof(tStats));
	sOversizedOsc = NULL;
	memset(&tProcessDesc, 0, sizeof(tProcessDesc));
	memset(&tSshDesc, 0, sizeof(tSshDesc));
	pContext = NULL;
	pRoot = NULL;
	pTerminal = NULL;
	pDisposable = NULL;
	pBadTerminal = NULL;
	pMenu = NULL;
	pTarget = NULL;
	pCache = NULL;
	pFont = NULL;
	pSession = NULL;
	memset(&tDestroyState, 0, sizeof(tDestroyState));
	memset(&tSessionDestroyState, 0, sizeof(tSessionDestroyState));
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
	tBadDesc = tDesc;
	tBadDesc.iSize = sizeof(uint32_t);
	iRet = xuiTerminalCreate(pContext, &pBadTerminal, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pBadTerminal == NULL,
		"terminal rejects undersized descriptor");
	tBadDesc = tDesc;
	tBadDesc.iColumns = 1001;
	iRet = xuiTerminalCreate(pContext, &pBadTerminal, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pBadTerminal == NULL,
		"terminal rejects oversized dimensions");
	tBadDesc = tDesc;
	tBadDesc.fCellWidth = NAN;
	iRet = xuiTerminalCreate(pContext, &pBadTerminal, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pBadTerminal == NULL,
		"terminal rejects non-finite metrics");
	iRet = xuiTerminalCreate(pContext, &pTerminal, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTerminal != NULL, "terminal create");
	iRet = xuiWidgetMeasure(pTerminal,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tNatural);
	XUI_TEST_CHECK(iRet == XUI_OK && tNatural.fX > 60.0f && tNatural.fY > 40.0f,
		"terminal intrinsic grid exceeds finite constraint");
	iRet = xuiWidgetMeasure(pTerminal, (xui_vec2_t){60.0f, 40.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 60.0f && tMeasured.fY == 40.0f &&
		xuiTerminalGetColumns(pTerminal) == 10 && xuiTerminalGetRows(pTerminal) == 4,
		"terminal finite measure constraint preserves grid");
	iRet = xuiWidgetMeasure(pTerminal,
		(xui_vec2_t){60.0f, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 60.0f &&
		tMeasured.fY == tNatural.fY, "terminal single-axis measure constraint");
	pBadTerminal = pTerminal;
	iRet = xuiTerminalCreate(NULL, &pBadTerminal, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pBadTerminal == NULL,
		"terminal clears output on invalid context");
	(void)xuiWidgetSetRect(pTerminal, (xui_rect_t){10.0f, 10.0f, 120.0f, 80.0f});
	(void)xuiWidgetAddChild(pRoot, pTerminal);
	(void)xuiTerminalSetInputCallback(pTerminal, __xuiTerminalTestInput, &tState);
	(void)xuiTerminalSetResizeCallback(pTerminal, __xuiTerminalTestResize, &tState);
	(void)xuiTerminalSetTitleCallback(pTerminal, __xuiTerminalTestTitle, &tState);
	(void)xuiTerminalSetLinkCallback(pTerminal, __xuiTerminalTestLink, &tState);
	iRet = xuiTerminalSetImageCallback(pTerminal, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "terminal image protocol is explicitly unsupported");

	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "terminal.background.color") != 0u, "terminal background style registered");
	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "terminal.palette.1") != 0u, "terminal palette style registered");
	XUI_TEST_CHECK(xuiStyleFindProperty(pContext, "terminal.focus.disabled_color") != 0u, "terminal disabled focus style registered");
	arrStyle[0] = __xuiTerminalTestColorProp("terminal.foreground.color", iStyledForeground);
	arrStyle[1] = __xuiTerminalTestColorProp("terminal.palette.1", iStyledPaletteRed);
	arrStyle[2] = __xuiTerminalTestFloatProp("terminal.cell.width", 10.0f);
	arrStyle[3] = __xuiTerminalTestFloatProp("terminal.cell.height", 16.0f);
	arrStyle[4] = __xuiTerminalTestFloatProp("terminal.padding", 2.0f);
	arrStyle[5] = __xuiTerminalTestColorProp("terminal.focus.color", XUI_COLOR_RGBA(0, 0, 0, 0));
	iRet = xuiWidgetSetInlineStyle(pTerminal, arrStyle, 6);
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
	iRet = xuiSetFocusWidget(pContext, pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal focus for cursor metrics");
	iRet = xuiWidgetInvalidate(pTerminal, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal invalidate cursor metrics");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal render cursor metrics");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	tCursorRect = xuiTestSurfaceGetLastRect(pCache);
	XUI_TEST_CHECK(tCursorRect.fH == 13.0f, "terminal cursor follows font height instead of cell height");
	iRet = xuiWidgetSetInlineStyle(pTerminal, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal inline style clear");
	iRet = xuiTerminalResize(pTerminal, 10, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style restore size");
	iRet = xuiTerminalSetMetrics(pTerminal, 7.4f, 15.6f, 2.4f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal fractional metrics set");
	iRet = xuiTerminalGetMetrics(pTerminal, &fCellWidth, &fCellHeight, &fPadding);
	XUI_TEST_CHECK(iRet == XUI_OK && fCellWidth == 7.0f && fCellHeight == 16.0f && fPadding == 2.0f,
		"terminal metrics resolve to stable pixel grid");
	iRet = xuiTerminalSetMetrics(pTerminal, 8.0f, 16.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal metrics restore");
	iRet = xuiTerminalSetMetrics(pTerminal, NAN, 16.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects NaN metric");
	iRet = xuiTerminalGetMetrics(pTerminal, &fCellWidth, &fCellHeight, &fPadding);
	XUI_TEST_CHECK(iRet == XUI_OK && fCellWidth == 8.0f && fCellHeight == 16.0f && fPadding == 4.0f,
		"invalid metric leaves state unchanged");
	iRet = xuiTerminalWrite(pTerminal, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal accepts empty write");
	iRet = xuiTerminalSerializeText(pTerminal, NULL, -1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects negative serialize capacity");
	iRet = xuiTerminalGetSelectionText(pTerminal, NULL, -1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects negative selection capacity");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 0, NULL, -1, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects negative link capacity");
	iRet = xuiTerminalFindNext(pTerminal, "x", 0x80000000u, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects unknown search flags");
	iRet = xuiTerminalOpenMenu(pTerminal, NAN, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "terminal rejects non-finite menu position");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal cursor query layout");
	XUI_TEST_CHECK(xuiQueryCursor(pContext, 20, 20) == XUI_CURSOR_IBEAM,
		"enabled terminal uses text cursor");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiQueryCursor(pContext, 20, 20) == XUI_CURSOR_ARROW,
		"disabled terminal subtree is skipped by cursor hit testing");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch disabled terminal state");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.pTarget = pTerminal;
	tPointer.iType = XUI_EVENT_KEY_DOWN;
	tPointer.iKey = XUI_KEY_ENTER;
	iRet = xuiDispatchEvent(pContext, &tPointer);
	tPointer.iType = XUI_EVENT_TEXT;
	tPointer.iKey = 0;
	tPointer.iTextSize = 1;
	tPointer.sText[0] = 'd';
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointer);
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iTextSize = 0;
	tPointer.sText[0] = '\0';
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = 20;
	tPointer.fY = 20;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointer);
	pMenu = xuiTerminalGetMenuWidget(pTerminal);
	tPointer.iType = XUI_EVENT_CONTEXT_MENU;
	tPointer.iButton = 0;
	tPointer.iButtons = 0;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 0 &&
		xuiGetPointerCapture(pContext) == NULL && pMenu != NULL && !xuiMenuIsOpen(pMenu),
		"disabled terminal rejects direct input and context menu events");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore terminal ancestor enabled state");
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.pTarget = pTerminal;
	tPointer.iType = XUI_EVENT_IME_COMPOSITION;
	tPointer.bCompositionActive = 1;
	tPointer.iTextSize = 1;
	tPointer.sText[0] = 'i';
	iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK, "start terminal IME composition before hide");
	iRet = xuiWidgetSetVisible(pRoot, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide terminal ancestor during IME composition");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	tPointer.iType = XUI_EVENT_KEY_DOWN;
	tPointer.iKey = XUI_KEY_ENTER;
	tPointer.iTextSize = 0;
	tPointer.sText[0] = '\0';
	iRet = xuiDispatchEvent(pContext, &tPointer);
	tPointer.iType = XUI_EVENT_TEXT;
	tPointer.iKey = 0;
	tPointer.iTextSize = 1;
	tPointer.sText[0] = 'h';
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 0,
		"hidden terminal rejects direct key and text events");
	iRet = xuiWidgetSetVisible(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	tPointer.sText[0] = 'x';
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 1 && tState.sInput[0] == 'x',
		"hiding terminal clears stale IME composition state");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal style restore clear");
	XUI_TEST_CHECK(xuiTerminalGetPalette(pTerminal, 1) == XUI_COLOR_RGBA(205, 49, 49, 255), "terminal palette restores base");

	iRet = xuiTerminalResize(pTerminal, 4, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap resize");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap clear history");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap clear");
	iRet = xuiTerminalWriteText(pTerminal, "ABCD\r\nE");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap CRLF write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap CRLF flush");
	iRet = xuiTerminalGetCell(pTerminal, 3, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'D', "terminal last column retained before CRLF");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'E', "terminal CRLF after full row advances once");
	iRet = xuiTerminalGetCursor(pTerminal, &iCursorX, &iCursorY);
	XUI_TEST_CHECK(iRet == XUI_OK && iCursorX == 1 && iCursorY == 1, "terminal delayed wrap cursor after CRLF");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap printable clear");
	iRet = xuiTerminalWriteText(pTerminal, "ABCDX");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap printable write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap printable flush");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "terminal next printable triggers delayed wrap");

	iRet = xuiTerminalResize(pTerminal, 4, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize history setup");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize history clear");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize screen clear");
	iRet = xuiTerminalWriteText(pTerminal, "1111\r\n2222\r\n3333");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize history write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize history flush");
	iRet = xuiTerminalResize(pTerminal, 4, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal grow restores history tail");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet >= 0 && strcmp(sBuffer, "1111\n2222\n3333") == 0, "terminal grow does not duplicate restored rows");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[2J\x1b[H1111\r\n2222\r\n3333");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize repaint write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal resize repaint flush");
	iRet = xuiTerminalSerializeText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet >= 0 && strcmp(sBuffer, "1111\n2222\n3333") == 0, "terminal ConPTY-style repaint stays replacement-only");
	iRet = xuiTerminalResize(pTerminal, 10, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap restore size");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap restore history");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal delayed wrap restore screen");

	iRet = xuiTerminalWriteText(pTerminal, "abc\r\n\x1b[31mR\x1b[0m\r\nline3\r\nline4\r\nline5");
	XUI_TEST_CHECK(iRet == XUI_OK, "write text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush");
	iRet = xuiTerminalGetCell(pTerminal, 0, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'l', "scroll visible row");
	iRet = xuiTerminalFindText(pTerminal, "abc", 0, &iLine, &iColumn);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 0, "find scrollback text");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before terminal tail follow");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear scrollback before terminal tail follow");
	iRet = xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset terminal tail follow offset");
	iRet = xuiTerminalWriteText(pTerminal, "tail0\r\ntail1\r\ntail2\r\ntail3\r\ntail4\r\ntail5\r\ntail6\r\ntail7");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal tail follow write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal tail follow flush");
	iRet = xuiScrollModelGetOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal scroll offset query");
	iRet = xuiScrollModelGetMaxOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fMaxY);
	XUI_TEST_CHECK(iRet == XUI_OK && fMaxY == 64.0f && fScrollY == fMaxY, "terminal follows scrollback row tail");
	iRet = xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal manual scroll up");
	iRet = xuiTerminalWriteText(pTerminal, "\r\nheld");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal write while scrolled up");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal flush while scrolled up");
	iRet = xuiScrollModelGetOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY == 0.0f, "terminal preserves manual scrollback view");
	iRet = xuiTerminalInputText(pTerminal, "x");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "x") == 0, "terminal input returns to tail");
	iRet = xuiScrollModelGetOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal input scroll offset query");
	iRet = xuiScrollModelGetMaxOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fMaxY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY == fMaxY, "terminal input follows tail");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal wheel layout");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = xuiInputPointerWheel(pContext, (int)(tWorld.fX + 20.0f), (int)(tWorld.fY + 20.0f), 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal wheel dispatch");
	iRet = xuiScrollModelGetOffset(xuiTerminalGetScrollModel(pTerminal), NULL, &fScrollY);
	XUI_TEST_CHECK(iRet == XUI_OK && fScrollY == fMaxY - 48.0f, "terminal wheel scrolls three text rows");

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear fractional scroll screen");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear fractional scroll history");
	iRet = xuiTerminalWriteText(pTerminal, "zero\r\none\r\ntwo\r\nthree\r\nfour");
	XUI_TEST_CHECK(iRet == XUI_OK, "write fractional scroll rows");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush fractional scroll rows");
	iRet = xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set fractional terminal scroll offset");
	iRet = xuiWidgetInvalidate(pTerminal, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate fractional terminal scroll offset");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render fractional terminal scroll offset");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	tTextRect = xuiTestSurfaceGetLastTextRect(pCache);
	XUI_TEST_CHECK(tTextRect.fY == 63.0f, "terminal renders partial bottom row at exact pixel offset");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = __xuiTerminalTestDoubleClick(pContext,
		tWorld.fX + 4.0f + 1.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal fractional scroll hit test");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 3 && strcmp(sBuffer, "one") == 0,
		"terminal hit testing uses exact pixel scroll offset");

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
	XUI_TEST_CHECK(iRet == XUI_OK, "clear combining character screen");
	iRet = xuiTerminalWriteText(pTerminal, "e\xcc\x81X");
	XUI_TEST_CHECK(iRet == XUI_OK, "write combining character");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush combining character");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'e' && tCell.iCombiningCount == 1 &&
		tCell.arrCombining[0] == 0x0301u, "terminal combining mark attaches to base cell");
	iRet = xuiTerminalGetCell(pTerminal, 1, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "terminal combining mark consumes no column");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear emoji width screen");
	iRet = xuiTerminalWriteText(pTerminal, "\xf0\x9f\x98\x80X");
	XUI_TEST_CHECK(iRet == XUI_OK, "write emoji width text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush emoji width text");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 0x1f600u && tCell.iWidth == 2,
		"terminal emoji occupies two cells");
	iRet = xuiTerminalGetCell(pTerminal, 2, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "terminal text follows emoji cell width");

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

	iOscSize = 0;
	memcpy(sLongOsc + iOscSize, "\x1b[", 2u);
	iOscSize += 2;
	for ( i = 0; i < 15; ++i ) {
		memcpy(sLongOsc + iOscSize, "0;", 2u);
		iOscSize += 2;
	}
	memcpy(sLongOsc + iOscSize, "31;1mX", 6u);
	iOscSize += 6;
	sLongOsc[iOscSize] = '\0';
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear long csi screen");
	iRet = xuiTerminalWriteText(pTerminal, sLongOsc);
	XUI_TEST_CHECK(iRet == XUI_OK, "write long csi sgr");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush long csi sgr");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X' &&
		tCell.iFgColor == xuiTerminalGetPalette(pTerminal, 1) &&
		(tCell.iFlags & XUI_TERMINAL_CELL_BOLD) != 0u,
		"long csi preserves parameters after inline capacity");

	iOscSize = 0;
	memcpy(sLongOsc + iOscSize, "\x1b[0m\x1b[", 6u);
	iOscSize += 6;
	for ( i = 0; i < 300; ++i ) {
		memcpy(sLongOsc + iOscSize, "1;", 2u);
		iOscSize += 2;
	}
	memcpy(sLongOsc + iOscSize, "31mX\x1b[32mY\x1b[0m", 14u);
	iOscSize += 14;
	sLongOsc[iOscSize] = '\0';
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear oversized csi screen");
	iRet = xuiTerminalWriteText(pTerminal, sLongOsc);
	XUI_TEST_CHECK(iRet == XUI_OK, "write oversized csi sgr");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush oversized csi sgr");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X' &&
		(tCell.iFlags & XUI_TERMINAL_CELL_BOLD) == 0u,
		"oversized csi is discarded");
	iRet = xuiTerminalGetCell(pTerminal, 1, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'Y' &&
		tCell.iFgColor == xuiTerminalGetPalette(pTerminal, 2),
		"terminal recovers after oversized csi");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[0m");
	if ( iRet == XUI_OK ) iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset style after csi capacity tests");

	iRet = xuiTerminalWriteText(pTerminal, "\x1b]0;XUI title\a");
	XUI_TEST_CHECK(iRet == XUI_OK, "write title");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sTitle, "XUI title") == 0, "osc title");

	memcpy(sLongTitle, "long-title-", 11u);
	for ( i = 11; i < 411; ++i ) sLongTitle[i] = (char)('a' + (i % 26));
	sLongTitle[411] = '\0';
	iOscSize = snprintf(sLongOsc, sizeof(sLongOsc), "\x1b]0;%s\a", sLongTitle);
	XUI_TEST_CHECK(iOscSize > 255 && iOscSize < (int)sizeof(sLongOsc), "build long osc title");
	iRet = xuiTerminalWriteText(pTerminal, sLongOsc);
	XUI_TEST_CHECK(iRet == XUI_OK, "write long osc title");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sTitle, sLongTitle) == 0,
		"long osc title is not truncated");

	iRet = xuiTerminalWriteText(pTerminal, "\x1b[?1049hALT\x1b[?1049l");
	XUI_TEST_CHECK(iRet == XUI_OK, "write alt");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush alt");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'X', "main screen restored");

	sOversizedOsc = (char*)malloc(70016u);
	XUI_TEST_CHECK(sOversizedOsc != NULL, "allocate oversized osc test");
	memcpy(sOversizedOsc, "\x1b]0;", 4u);
	memset(sOversizedOsc + 4, 'Q', 70000u);
	sOversizedOsc[70004] = '\a';
	sOversizedOsc[70005] = 'R';
	sOversizedOsc[70006] = '\0';
	strcpy(tState.sTitle, "unchanged");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before oversized osc");
	iRet = xuiTerminalWriteText(pTerminal, sOversizedOsc);
	XUI_TEST_CHECK(iRet == XUI_OK, "write oversized osc");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sTitle, "unchanged") == 0,
		"oversized osc is discarded");
	iRet = xuiTerminalGetCell(pTerminal, 0, 0, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK && tCell.iCodepoint == 'R',
		"terminal recovers after oversized osc");
	free(sOversizedOsc);
	sOversizedOsc = NULL;
	iRet = xuiTerminalClear(pTerminal);
	if ( iRet == XUI_OK ) iRet = xuiTerminalWriteText(pTerminal, "X");
	if ( iRet == XUI_OK ) iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore screen after oversized osc test");

	iRet = xuiTerminalInputText(pTerminal, "go");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tState.sInput, "go") == 0, "input callback");
	iRet = xuiTerminalSetBracketedPaste(pTerminal, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set bracketed paste");
	iRet = xuiTerminalPasteText(pTerminal, "paste");
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[200~paste\x1b[201~") != NULL, "bracketed paste");

	iRet = xuiTerminalGetStats(pTerminal, &tStats);
	XUI_TEST_CHECK(iRet == XUI_OK && tStats.iSize == sizeof(tStats) &&
		tStats.iQueuedBytes == 0 && tStats.iQueueCapacity >= tStats.iQueuePeakBytes &&
		tStats.iOutputBytesReceived == tStats.iOutputBytesParsed &&
		tStats.iScreenMemoryBytes > 0 && tStats.iHistoryMemoryBytes > 0,
		"terminal runtime statistics");

	if ( xuiTerminalGetFindWindow(pTerminal) != NULL ) {
		(void)xuiWindowSetOpen(xuiTerminalGetFindWindow(pTerminal), 0);
	}
	(void)xuiSetFocusWidget(pContext, pTerminal);
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = __xuiTerminalTestKey(pContext, 'F', XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 1 &&
		(unsigned char)tState.sInput[0] == 0x06u, "terminal ctrl f reaches TUI");
	iRet = __xuiTerminalTestKey(pContext, 'F', XUI_MOD_CTRL | XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTerminalGetFindWindow(pTerminal) != NULL &&
		xuiWindowIsOpen(xuiTerminalGetFindWindow(pTerminal)), "terminal ctrl shift f opens find window");
	iRet = xuiInputKeyUp(pContext, 'F', 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find shortcut key release");
	(void)xuiWindowSetOpen(xuiTerminalGetFindWindow(pTerminal), 0);
	(void)xuiSetFocusWidget(pContext, pTerminal);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiTerminalWriteText(pTerminal, "\x1b[?1002h\x1b[?1006h\x1b[?1004h");
	XUI_TEST_CHECK(iRet == XUI_OK, "enable terminal application input modes");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush terminal application input modes");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	tContent = xuiWidgetGetContentRect(pTerminal);
	iRet = xuiInputPointerDown(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal application mouse down");
	iRet = xuiInputPointerMove(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 8.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal application mouse drag");
	iRet = xuiInputPointerUp(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 8.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[<0;1;1M") != NULL &&
		strstr(tState.sInput, "\x1b[<32;2;1M") != NULL &&
		strstr(tState.sInput, "\x1b[<0;2;1m") != NULL, "terminal SGR mouse protocol");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiInputPointerDown(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pTerminal,
		"terminal application mouse reacquires capture");
	iRet = xuiInputPointerMove(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 8.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f), 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		strstr(tState.sInput, "\x1b[<0;2;1m") != NULL,
		"terminal application mouse lost button releases remote and local capture");
	iRet = xuiInputPointerWheel(pContext,
		(int)(tWorld.fX + tContent.fX + 4.0f + 8.0f + 1.0f),
		(int)(tWorld.fY + tContent.fY + 4.0f + 1.0f), 0.0f, 1.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[<64;2;1M") != NULL,
		"terminal SGR wheel protocol");
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.iType = XUI_EVENT_POINTER_WHEEL;
	tPointer.pTarget = pTerminal;
	tPointer.fX = (int)(tWorld.fX + tContent.fX + 4.0f + 8.0f + 1.0f);
	tPointer.fY = (int)(tWorld.fY + tContent.fY + 4.0f + 1.0f);
	tPointer.fWheelY = 1000000.0f;
	iInputCallsBefore = tState.iInputCalls;
	iInputBytesBefore = tState.iInputBytes;
	iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputCalls - iInputCallsBefore == 32 &&
		tState.iInputBytes > iInputBytesBefore,
		"terminal bounds application mouse wheel expansion");
	tPointer.fWheelY = NAN;
	iInputCallsBefore = tState.iInputCalls;
	iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && tState.iInputCalls == iInputCallsBefore,
		"terminal rejects non-finite direct wheel input");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	iRet = xuiSetFocusWidget(pContext, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pTerminal);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strstr(tState.sInput, "\x1b[O") != NULL &&
		strstr(tState.sInput, "\x1b[I") != NULL, "terminal focus reporting");
	iRet = xuiTerminalWriteText(pTerminal, "\x1b[?1002l\x1b[?1006l\x1b[?1004l");
	if ( iRet == XUI_OK ) iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable terminal application input modes");
	iRet = xuiWidgetCreate(pContext, &pBadTerminal);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pTerminal, pBadTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && pBadTerminal != NULL, "terminal descendant create");
	memset(tState.sInput, 0, sizeof(tState.sInput));
	tState.iInputSize = 0;
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.iType = XUI_EVENT_TEXT;
	tPointer.pTarget = pBadTerminal;
	tPointer.iTextSize = 1;
	tPointer.sText[0] = 'z';
	iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_OK && tState.iInputSize == 0,
		"terminal ignores bubbled descendant text input");
	if ( pBadTerminal != NULL ) {
		(void)xuiWidgetRemoveFromParent(pBadTerminal);
		xuiWidgetDestroy(pBadTerminal);
		pBadTerminal = NULL;
	}

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
	XUI_TEST_CHECK(iRet == XUI_OK, "clear long osc8 link screen");
	memcpy(sLongUrl, "https://example.com/", 20u);
	for ( i = 20; i < 420; ++i ) sLongUrl[i] = (char)('a' + (i % 26));
	sLongUrl[420] = '\0';
	iOscSize = snprintf(sLongOsc, sizeof(sLongOsc),
		"\x1b]8;;%s\aL\x1b]8;;\a", sLongUrl);
	XUI_TEST_CHECK(iOscSize > 255 && iOscSize < (int)sizeof(sLongOsc), "build long osc8 link");
	iRet = xuiTerminalWriteText(pTerminal, sLongOsc);
	XUI_TEST_CHECK(iRet == XUI_OK, "write long osc8 link");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush long osc8 link");
	iRet = xuiTerminalGetLinkAt(pTerminal, 0, 0, sBuffer, (int)sizeof(sBuffer),
		&iStartColumn, &iLength);
	XUI_TEST_CHECK(iRet == 420 && strcmp(sBuffer, sLongUrl) == 0 &&
		iStartColumn == 0 && iLength == 1, "long osc8 link is not truncated");

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
	iRet = xuiTerminalWriteText(pTerminal, "\xE4\xB8\xAD text /tmp/file-name.txt");
	XUI_TEST_CHECK(iRet == XUI_OK, "write Unicode terminal selection text");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "flush Unicode terminal selection text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "Unicode terminal selection layout");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = __xuiTerminalTestDoubleClick(pContext,
		tWorld.fX + 4.0f + 1.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal double click wide character");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	if ( iRet != 3 || strcmp(sBuffer, "\xE4\xB8\xAD") != 0 ) {
		int iEndLine;
		int iEndColumn;
		(void)xuiTerminalGetSelectionRange(pTerminal, &iLine, &iColumn, &iEndLine, &iEndColumn);
		printf("terminal Unicode selection: bytes=%d text=[%s] range=(%d,%d)-(%d,%d)\n",
			iRet, sBuffer, iLine, iColumn, iEndLine, iEndColumn);
	}
	XUI_TEST_CHECK(iRet == 3 && strcmp(sBuffer, "\xE4\xB8\xAD") == 0,
		"terminal wide character selection uses display columns");
	iRet = __xuiTerminalTestDoubleClick(pContext,
		tWorld.fX + 4.0f + 12.0f * 8.0f + 0.5f,
		tWorld.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal double click path");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 18 && strcmp(sBuffer, "/tmp/file-name.txt") == 0,
		"terminal path remains one selectable word");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear after Unicode selection");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear Unicode selection history");
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
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(74, 144, 239, 255)) > 0, "selection overlay rendered");

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
	XUI_TEST_CHECK(pItem != NULL && pItem->iValue == XUI_TERMINAL_MENU_FIND && (pItem->iState & XUI_MENU_ITEM_ENABLED) != 0u, "terminal find menu enabled");
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
	XUI_TEST_CHECK(xuiTerminalGetFindWindow(pTerminal) != NULL &&
		xuiWindowIsOpen(xuiTerminalGetFindWindow(pTerminal)), "terminal find menu opens window");
	iRet = xuiTerminalGetFindMatch(pTerminal, &iLine, &iColumn, &iLength);
	XUI_TEST_CHECK(iRet == 1 && iLine == 0 && iColumn == 6 && iLength == 4, "terminal find menu first match");
	iRet = xuiTerminalClearSelection(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal find menu clear selection");
	iRet = xuiWindowSetOpen(xuiTerminalGetFindWindow(pTerminal), 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "close terminal find window");
	iRet = xuiSetFocusWidget(pContext, pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore terminal focus after find window");

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
	iRet = xuiTerminalSessionResize(pSession, 32768, 4);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "session rejects oversized terminal dimensions");
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

	memset(&tSessionDesc, 0, sizeof(tSessionDesc));
	tSessionDesc.iSize = sizeof(tSessionDesc);
	tSessionDesc.onResize = __xuiTerminalTestDestroySessionOnResize;
	tSessionDestroyState.ppSession = &pSession;
	tSessionDesc.pResizeUser = &tSessionDestroyState;
	pSession = xuiTerminalCreateFakeSession(&tSessionDesc);
	XUI_TEST_CHECK(pSession != NULL, "self-destroying fake session create");
	iRet = xuiTerminalAttachSession(pTerminal, pSession);
	XUI_TEST_CHECK(iRet == XUI_OK && pSession == NULL && tSessionDestroyState.iCalls == 1,
		"session resize callback may destroy attached session");

	memset(&tProcessDesc, 0, sizeof(tProcessDesc));
	tProcessDesc.iSize = sizeof(tProcessDesc);
	tProcessDesc.sCommandLine = "cmd.exe /d /c exit 0";
	tProcessDesc.iFlags = 0x80000000u;
	XUI_TEST_CHECK(xuiTerminalCreateProcessSession(&tProcessDesc) == NULL,
		"process session rejects unknown flags");
	tProcessDesc.iFlags = 0u;
	tProcessDesc.iColumns = -1;
	XUI_TEST_CHECK(xuiTerminalCreateProcessSession(&tProcessDesc) == NULL,
		"process session rejects negative columns");
	tProcessDesc.iColumns = 80;
	tProcessDesc.iRows = 32768;
	XUI_TEST_CHECK(xuiTerminalCreateProcessSession(&tProcessDesc) == NULL,
		"process session rejects oversized rows");

	tSshDesc.iSize = sizeof(tSshDesc);
	tSshDesc.sHost = "example.com";
	tSshDesc.sUser = "alice";
	tSshDesc.iPort = 2222;
	tSshDesc.sCommand = "echo \"ok\"";
	tSshDesc.sIdentityFile = "C:\\keys\\id key";
	tSshDesc.sExtraOptions = "-o BatchMode=yes";
	tSshDesc.sExecutable = "C:\\Program Files\\OpenSSH\\ssh.exe";
	tSshDesc.iFlags = XUI_TERMINAL_SSH_FORCE_TTY;
	iLength = xuiTerminalBuildSshCommand(&tSshDesc, NULL, 0);
	XUI_TEST_CHECK(iLength > 0, "query SSH command size");
	iRet = xuiTerminalBuildSshCommand(&tSshDesc, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == iLength && iRet == (int)strlen(sBuffer) &&
		strcmp(sBuffer, "\"C:\\Program Files\\OpenSSH\\ssh.exe\" -t -p \"2222\" -i \"C:\\keys\\id key\" -o BatchMode=yes \"alice@example.com\" \"echo \\\"ok\\\"\"") == 0,
		"build quoted SSH command");
	memset(sSmallBuffer, 'x', sizeof(sSmallBuffer));
	iRet = xuiTerminalBuildSshCommand(&tSshDesc, sSmallBuffer, (int)sizeof(sSmallBuffer));
	XUI_TEST_CHECK(iRet == iLength && sSmallBuffer[sizeof(sSmallBuffer) - 1u] == '\0',
		"SSH command reports required size and terminates short buffer");
	tSshDesc.iFlags = 0x80000000u;
	XUI_TEST_CHECK(xuiTerminalBuildSshCommand(&tSshDesc, sBuffer, (int)sizeof(sBuffer)) == XUI_ERROR_INVALID_ARGUMENT,
		"SSH command rejects unknown flags");
	tSshDesc.iFlags = 0u;
	tSshDesc.iProcessFlags = 0x80000000u;
	XUI_TEST_CHECK(xuiTerminalCreateSshSession(&tSshDesc) == NULL,
		"SSH session rejects unknown process flags before launch");

#if defined(_WIN32) || defined(_WIN64)
	memset(&tProcessDesc, 0, sizeof(tProcessDesc));
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

	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear before multipointer selection");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear history before multipointer selection");
	iRet = xuiTerminalWriteText(pTerminal, "abcdef");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTerminalFlush(pTerminal) == XUI_OK,
		"write multipointer selection text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "multipointer terminal layout");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	memset(&tPointer, 0, sizeof(tPointer));
	tPointer.iSize = sizeof(tPointer);
	tPointer.pTarget = pTerminal;
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iPointerId = 1001;
	tPointer.iPointerType = XUI_POINTER_TYPE_TOUCH;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + 4.5f;
	tPointer.fY = tWorld.fY + 12.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1001, XUI_POINTER_TYPE_TOUCH) == pTerminal,
		"terminal touch owns selection capture");
	tPointer.iPointerId = 1002;
	tPointer.fX = tWorld.fX + 20.5f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1001, XUI_POINTER_TYPE_TOUCH) == pTerminal &&
		xuiGetPointerCaptureEx(pContext, 1002, XUI_POINTER_TYPE_TOUCH) == NULL,
		"second touch cannot steal terminal selection");
	tPointer.iType = XUI_EVENT_POINTER_CAPTURE_LOST;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1001, XUI_POINTER_TYPE_TOUCH) == pTerminal,
		"unrelated terminal capture loss is ignored");
	tPointer.iType = XUI_EVENT_POINTER_MOVE;
	tPointer.iPointerId = 1001;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + 36.5f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK,
		"owning touch extends terminal selection");
	iRet = xuiTerminalGetSelectionRange(pTerminal, &iLine, &iColumn, &iCursorX, &iCursorY);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == iCursorX && iColumn == 0 && iCursorY == 4,
		"terminal multipointer selection range");
	iRet = xuiReleasePointerCaptureEx(pContext, 1001, XUI_POINTER_TYPE_TOUCH, pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"matching terminal capture loss cancels selection gesture");
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iPointerId = 1002;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + 12.5f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1002, XUI_POINTER_TYPE_TOUCH) == pTerminal,
		"new touch can select after owning capture loss");
	iRet = xuiReleasePointerCaptureEx(pContext, 1002, XUI_POINTER_TYPE_TOUCH, pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"terminal releases replacement touch capture");
	iRet = xuiEditSetEvent(pTerminal, __xuiTerminalTestFailSelection, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal install failing selection callback");
	tPointer.iType = XUI_EVENT_POINTER_DOWN;
	tPointer.iPointerId = 1003;
	tPointer.iButton = XUI_POINTER_BUTTON_LEFT;
	tPointer.iButtons = XUI_POINTER_BUTTON_LEFT;
	tPointer.fX = tWorld.fX + 4.5f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tPointer) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 1003, XUI_POINTER_TYPE_TOUCH) == pTerminal,
		"terminal selection captures before callback failure");
	tPointer.iType = XUI_EVENT_POINTER_UP;
	tPointer.iButtons = 0;
	tPointer.fX = tWorld.fX + 20.5f;
	iRet = xuiDispatchEvent(pContext, &tPointer);
	XUI_TEST_CHECK(iRet == XUI_ERROR_BACKEND_FAILED &&
		xuiGetPointerCaptureEx(pContext, 1003, XUI_POINTER_TYPE_TOUCH) == NULL,
		"terminal selection callback failure releases capture");
	iRet = xuiEditSetEvent(pTerminal, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal clear failing selection callback");

	tWidgetPadding = (xui_thickness_t){3, 5, 7, 9};
	iRet = xuiWidgetSetPadding(pTerminal, tWidgetPadding);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal widget content padding set");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content layout");
	iRet = xuiTerminalClear(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content clear");
	iRet = xuiTerminalClearScrollback(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content history clear");
	iRet = xuiScrollModelSetOffset(xuiTerminalGetScrollModel(pTerminal), 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content scroll reset");
	tContent = xuiWidgetGetContentRect(pTerminal);
	tCursorRect = xuiEditGetCaretRect(pTerminal);
	XUI_TEST_CHECK(tCursorRect.fX == tContent.fX + 4.0f && tCursorRect.fY == tContent.fY + 4.0f,
		"terminal caret includes content rect origin");
	iRet = xuiTerminalWriteText(pTerminal, "padded");
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content write");
	iRet = xuiTerminalFlush(pTerminal);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content flush");
	tWorld = xuiWidgetGetWorldRect(pTerminal);
	iRet = __xuiTerminalTestDoubleClick(pContext,
		tWorld.fX + tContent.fX + 4.0f + 2.0f * 8.0f + 0.5f,
		tWorld.fY + tContent.fY + 4.0f + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "terminal padded content hit test");
	iRet = xuiTerminalGetSelectionText(pTerminal, sBuffer, (int)sizeof(sBuffer));
	XUI_TEST_CHECK(iRet == 6 && strcmp(sBuffer, "padded") == 0,
		"terminal pointer hit includes content rect origin");

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(pTerminal, xuiWidgetGetStateId(pTerminal));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetTextDrawCount(pCache) > 0, "terminal render text");

	memset(&tDestroyState, 0, sizeof(tDestroyState));
	iRet = __xuiTerminalTestCreateDisposable(pContext, pRoot, pFont, &pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable != NULL, "create disposable input terminal");
	tDestroyState.ppWidget = &pDisposable;
	iRet = xuiTerminalSetInputCallback(pDisposable, __xuiTerminalTestDestroyOnInput, &tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set self-destroying terminal input callback");
	iRet = xuiTerminalInputText(pDisposable, "x");
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable == NULL && tDestroyState.iCalls == 1,
		"terminal input callback may destroy widget");

	memset(&tDestroyState, 0, sizeof(tDestroyState));
	iRet = __xuiTerminalTestCreateDisposable(pContext, pRoot, pFont, &pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable != NULL, "create disposable title terminal");
	tDestroyState.ppWidget = &pDisposable;
	iRet = xuiTerminalSetTitleCallback(pDisposable, __xuiTerminalTestDestroyOnTitle, &tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set self-destroying terminal title callback");
	iRet = xuiTerminalWriteText(pDisposable, "\x1b]0;destroy-title\aunparsed-tail");
	XUI_TEST_CHECK(iRet == XUI_OK, "queue self-destroying terminal title");
	iRet = xuiTerminalFlush(pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable == NULL && tDestroyState.iCalls == 1,
		"terminal title callback may destroy widget during parsing");

	memset(&tDestroyState, 0, sizeof(tDestroyState));
	iRet = __xuiTerminalTestCreateDisposable(pContext, pRoot, pFont, &pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable != NULL, "create disposable resize terminal");
	tDestroyState.ppWidget = &pDisposable;
	iRet = xuiTerminalSetResizeCallback(pDisposable, __xuiTerminalTestDestroyOnResize, &tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set self-destroying terminal resize callback");
	iRet = xuiTerminalResize(pDisposable, 11, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable == NULL && tDestroyState.iCalls == 1,
		"terminal resize callback may destroy widget");

	memset(&tDestroyState, 0, sizeof(tDestroyState));
	iRet = __xuiTerminalTestCreateDisposable(pContext, pRoot, pFont, &pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable != NULL, "create disposable menu terminal");
	tDestroyState.ppWidget = &pDisposable;
	g_pTerminalClipboardDestroyState = &tDestroyState;
	tProxyState.onClipboardGetText = __xuiTerminalTestDestroyOnClipboardRead;
	iRet = xuiTerminalOpenMenu(pDisposable, 10.0f, 10.0f);
	tProxyState.onClipboardGetText = NULL;
	g_pTerminalClipboardDestroyState = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && pDisposable == NULL && tDestroyState.iCalls == 1,
		"terminal menu clipboard callback may destroy widget");

	memset(&tDestroyState, 0, sizeof(tDestroyState));
	iRet = __xuiTerminalTestCreateDisposable(pContext, pRoot, pFont, &pDisposable);
	XUI_TEST_CHECK(iRet == XUI_OK && pDisposable != NULL, "create disposable menu title terminal");
	tDestroyState.ppWidget = &pDisposable;
	g_pTerminalClipboardDestroyState = &tDestroyState;
	tProxyState.onClipboardGetText = __xuiTerminalTestDestroyOnClipboardRead;
	iRet = xuiTerminalSetMenuTitle(pDisposable, XUI_TERMINAL_MENU_COPY, "Copy custom");
	tProxyState.onClipboardGetText = NULL;
	g_pTerminalClipboardDestroyState = NULL;
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE && pDisposable == NULL && tDestroyState.iCalls == 1,
		"terminal menu title clipboard callback may destroy widget");

cleanup:
	free(sOversizedOsc);
	tProxyState.onClipboardGetText = NULL;
	g_pTerminalClipboardDestroyState = NULL;
	if ( pSession != NULL ) xuiTerminalSessionDestroy(pSession);
	if ( pTarget != NULL ) tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_terminal_test passed\n");
	return 0;
}
