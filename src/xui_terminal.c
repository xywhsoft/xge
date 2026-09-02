#if defined(_WIN32) || defined(_WIN64)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000006
#endif
#endif

#include "xui_internal.h"

#include <limits.h>

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define XUI_TERMINAL_DEFAULT_COLS 80
#define XUI_TERMINAL_DEFAULT_ROWS 24
#define XUI_TERMINAL_DEFAULT_SCROLLBACK 1000
#define XUI_TERMINAL_DEFAULT_PARSE_BUDGET 4096
#define XUI_TERMINAL_MAX_CSI_PARAMS 16
#define XUI_TERMINAL_MAX_OSC 256
#define XUI_TERMINAL_MAX_LINK_ID 65535u
#define XUI_TERMINAL_TRIPLE_CLICK_SECONDS 0.45
#define XUI_TERMINAL_TRIPLE_CLICK_DISTANCE 6.0f
#define XUI_TERMINAL_SESSION_FAKE 1
#define XUI_TERMINAL_SESSION_PROCESS 2
#define XUI_TERMINAL_PROCESS_READ_CHUNK 4096
#define XUI_TERMINAL_PROCESS_POLL_LIMIT 16
#define XUI_TERMINAL_MENU_TITLE_COUNT 6
#define XUI_TERMINAL_HISTORY_INITIAL_VIEW 128

static int __xuiTerminalQueryCursor(xui_widget pWidget, int iX, int iY, void* pUser);

struct xui_terminal_session_t {
	xui_widget pWidget;
	char* sPrompt;
	xui_terminal_session_resize_proc onResize;
	void* pResizeUser;
	int iLastColumns;
	int iLastRows;
	int bEcho;
	int bRunning;
	int iKind;
	uint32_t iProcessFlags;
#if defined(_WIN32) || defined(_WIN64)
	HANDLE hProcess;
	HANDLE hInputWrite;
	HANDLE hOutputRead;
	HPCON hPseudoConsole;
#endif
};

typedef struct xui_terminal_parser_t {
	int iState;
	int iUtfExpected;
	int iUtfSeen;
	uint32_t iUtfCodepoint;
	int arrParams[XUI_TERMINAL_MAX_CSI_PARAMS];
	int iParamCount;
	int iCurrentParam;
	int bParamActive;
	char cPrivate;
	char arrIntermediates[2];
	int iIntermediateCount;
	char cCharsetTarget;
	char sOsc[XUI_TERMINAL_MAX_OSC];
	int iOscSize;
} xui_terminal_parser_t;

typedef struct xui_terminal_history_cell_t {
	uint32_t iCodepoint;
	uint32_t iFgColor;
	uint32_t iBgColor;
	uint32_t iFlags;
	uint32_t iCombiningOffset;
	uint16_t iStyle;
	uint16_t iLinkId;
	uint8_t iWidth;
	uint8_t iCombiningCount;
	uint8_t arrReserved[2];
} xui_terminal_history_cell_t;

typedef struct xui_terminal_history_line_t {
	xui_terminal_history_cell_t* pCells;
	uint32_t* pCombining;
	char* sTextCache;
	uint64_t iId;
	int iCellCount;
	int iCellCapacity;
	int iCombiningCount;
	int iCombiningCapacity;
	int bOpen;
} xui_terminal_history_line_t;

typedef struct xui_terminal_history_view_row_t {
	xui_terminal_history_line_t* pLine;
	int iStart;
	int iEnd;
	char* sTextCache;
} xui_terminal_history_view_row_t;

#if defined(_WIN32) || defined(_WIN64)
typedef HRESULT (WINAPI* xui_terminal_create_pseudo_console_proc)(COORD, HANDLE, HANDLE, DWORD, HPCON*);
typedef HRESULT (WINAPI* xui_terminal_resize_pseudo_console_proc)(HPCON, COORD);
typedef void (WINAPI* xui_terminal_close_pseudo_console_proc)(HPCON);

typedef struct xui_terminal_conpty_api_t {
	xui_terminal_create_pseudo_console_proc createPseudoConsole;
	xui_terminal_resize_pseudo_console_proc resizePseudoConsole;
	xui_terminal_close_pseudo_console_proc closePseudoConsole;
} xui_terminal_conpty_api_t;
#endif

typedef struct xui_terminal_data_t {
	xui_terminal_cell_t* pMain;
	xui_terminal_cell_t* pAlt;
	uint8_t* pMainWrapped;
	uint8_t* pAltWrapped;
	xui_terminal_history_line_t** ppHistory;
	xui_terminal_history_view_row_t* pHistoryView;
	xui_terminal_cell_t* pHistoryRenderCells;
	uint8_t* pQueue;
	uint8_t* pTabStops;
	uint8_t* pDirtyRows;
	xui_surface pLastCacheSurface;
	xui_font pBaseFont;
	xui_font pFont;
	xui_scroll_model_t tScroll;
	xui_terminal_parser_t tParser;
	xui_terminal_session_t* pSession;
	xui_widget pMenu;
	xui_widget pFindWindow;
	xui_widget pFindInput;
	xui_widget pFindPrevButton;
	xui_widget pFindNextButton;
	xui_widget pFindCaseCheck;
	xui_widget pFindWordCheck;
	xui_widget pFindRegexCheck;
	xui_widget pFindStatus;
	char* arrMenuTitle[XUI_TERMINAL_MENU_TITLE_COUNT];
	char* sSearchText;
	char* sImeText;
	int iImeCapacity;
	xui_terminal_data_proc onData;
	void* pDataUser;
	xui_terminal_resize_proc onResize;
	void* pResizeUser;
	xui_terminal_title_proc onTitle;
	void* pTitleUser;
	xui_terminal_link_proc onLink;
	void* pLinkUser;
	char* sHoverLink;
	char** ppLinks;
	int iLinkCount;
	int iLinkCapacity;
	uint16_t iCurrentLinkId;
	uint32_t iLastCodepoint;
	uint32_t arrBasePalette[256];
	uint32_t arrPalette[256];
	uint32_t iBaseBackgroundColor;
	uint32_t iBaseForegroundColor;
	uint32_t iBaseCursorColor;
	uint32_t iBaseSelectionColor;
	uint32_t iBaseSelectionTextColor;
	uint32_t iBaseSearchHighlightColor;
	uint32_t iBaseFocusColor;
	uint32_t iBaseLinkHoverColor;
	uint32_t iBackgroundColor;
	uint32_t iForegroundColor;
	uint32_t iCursorColor;
	uint32_t iSelectionColor;
	uint32_t iSelectionTextColor;
	uint32_t iSearchHighlightColor;
	uint32_t iFocusColor;
	uint32_t iLinkHoverColor;
	uint32_t iCurrentFg;
	uint32_t iCurrentBg;
	uint32_t iCurrentFlags;
	int iColumns;
	int iRows;
	int iMainRowOffset;
	int iAltRowOffset;
	int iCursorX;
	int iCursorY;
	int iSavedCursorX;
	int iSavedCursorY;
	uint32_t iSavedFg;
	uint32_t iSavedBg;
	uint32_t iSavedFlags;
	uint16_t iSavedLinkId;
	int iMainCursorX;
	int iMainCursorY;
	int iAltCursorX;
	int iAltCursorY;
	int iScrollTop;
	int iScrollBottom;
	int iScrollbackLimit;
	int iScrollbackStart;
	int iScrollbackCount;
	int iHistoryCount;
	int iHistoryViewCapacity;
	int iHistoryRenderCapacity;
	int iHistoryViewColumns;
	int bHistoryViewDirty;
	uint64_t iNextHistoryId;
	int iQueueHead;
	int iQueueSize;
	int iQueueCapacity;
	int iQueuePeak;
	int iParseBudget;
	uint64_t iOutputBytesReceived;
	uint64_t iOutputBytesParsed;
	int iChangeCount;
	int iDirtyRowCount;
	int iLastRenderTopLine;
	int iLastRenderOffsetY;
	int iLastRenderColumns;
	int iLastRenderRows;
	int iSearchLine;
	int iSearchColumn;
	int iSearchLength;
	uint64_t iSearchHistoryId;
	int iSearchHistoryCell;
	int iHoverLinkLine;
	int iHoverLinkColumn;
	int iHoverLinkLength;
	uint32_t iSearchFlags;
	int bAltScreen;
	int bCursorVisible;
	int bWrapPending;
	int bAutoWrap;
	int bOriginMode;
	int bInsertMode;
	int bApplicationCursor;
	int bApplicationKeypad;
	int iG0Charset;
	int iG1Charset;
	int iActiveCharset;
	int bFocusReporting;
	int iMouseTracking;
	int iMouseEncoding;
	int iMouseButton;
	int bMouseCaptured;
	int iCursorStyle;
	int bLastCaretBlinkVisible;
	int bBracketedPaste;
	int bLigaturesEnabled;
	int bImeActive;
	int bFullCacheDirty;
	int bCacheRendered;
	int bSelectAll;
	int bSelecting;
	int bDoubleClickPending;
	int iSelectAnchorLine;
	int iSelectAnchorColumn;
	int iSelectEndLine;
	int iSelectEndColumn;
	int iDoubleClickLine;
	float fDoubleClickX;
	float fDoubleClickY;
	double fDoubleClickTime;
	float fBaseCellWidth;
	float fBaseCellHeight;
	float fBasePadding;
	float fCellWidth;
	float fCellHeight;
	float fCursorHeight;
	float fPadding;
} xui_terminal_data_t;

static void __xuiTerminalResolveStyle(xui_widget pWidget, xui_terminal_data_t* pData);
static int __xuiTerminalPointerToCell(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent, int* pLine, int* pColumn);
static int __xuiTerminalPasteClipboard(xui_widget pWidget, xui_terminal_data_t* pData);
static int __xuiTerminalScrollOffsetPixels(xui_terminal_data_t* pData);
static int __xuiTerminalUtf8Next(const char* sText, int iRemaining, uint32_t* pCodepoint, int* pBytes);
static int __xuiTerminalEmitInput(xui_widget pWidget, xui_terminal_data_t* pData, const uint8_t* pBytes, int iSize);
static void __xuiTerminalInsertCharacters(xui_terminal_data_t* pData, int iCount);
static int __xuiTerminalOpenFindWindow(xui_widget pTerminal, xui_terminal_data_t* pData);
static void __xuiTerminalClearFindMatch(xui_terminal_data_t* pData);
static int __xuiTerminalResolveFindAnchor(xui_terminal_data_t* pData);

static int __xuiTerminalEditSetSelection(xui_widget pWidget, int iStart, int iEnd)
{
	xui_terminal_data_t* pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	if ( pData == NULL || iStart < 0 || iEnd < 0 || pData->iColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiTerminalSetSelectionRange(pWidget, iStart / pData->iColumns,
		iStart % pData->iColumns, iEnd / pData->iColumns, iEnd % pData->iColumns);
}

static int __xuiTerminalEditGetSelection(xui_widget pWidget, int* pStart, int* pEnd)
{
	xui_terminal_data_t* pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	int iLine0, iColumn0, iLine1, iColumn1;
	int iRet;
	if ( pData == NULL || pData->iColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiTerminalGetSelectionRange(pWidget, &iLine0, &iColumn0, &iLine1, &iColumn1);
	if ( iRet != XUI_OK ) return iRet;
	if ( pStart != NULL ) *pStart = iLine0 < 0 ? 0 : iLine0 * pData->iColumns + iColumn0;
	if ( pEnd != NULL ) *pEnd = iLine1 < 0 ? 0 : iLine1 * pData->iColumns + iColumn1;
	return XUI_OK;
}

static int __xuiTerminalEditHasSelection(xui_widget pWidget)
{
	int iStart, iEnd;
	return __xuiTerminalEditGetSelection(pWidget, &iStart, &iEnd) == XUI_OK && iStart != iEnd;
}

static int __xuiTerminalEditPaste(xui_widget pWidget)
{
	xui_terminal_data_t* pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	return (pData != NULL) ? __xuiTerminalPasteClipboard(pWidget, pData) : XUI_ERROR_INVALID_ARGUMENT;
}

static xui_rect_t __xuiTerminalEditCaretRect(xui_widget pWidget)
{
	xui_terminal_data_t* pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	xui_rect_t tRect = {0.0f, 0.0f, 0.0f, 0.0f};
	xui_rect_t tContent;
	int iOffsetY;
	if ( pData == NULL ) return tRect;
	__xuiTerminalResolveStyle(pWidget, pData);
	tContent = xuiWidgetGetContentRect(pWidget);
	iOffsetY = __xuiTerminalScrollOffsetPixels(pData);
	tRect.fX = tContent.fX + pData->fPadding + (float)pData->iCursorX * pData->fCellWidth;
	tRect.fY = tContent.fY + pData->fPadding +
		(float)(pData->iCursorY + pData->iScrollbackCount) * pData->fCellHeight - (float)iOffsetY;
	tRect.fW = pData->fCellWidth;
	tRect.fH = pData->fCursorHeight;
	return xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, tRect.fH);
}

static const xui_internal_edit_adapter_t g_xuiTerminalEditAdapter = {
	XUI_EDIT_CAP_SELECTION | XUI_EDIT_CAP_CLIPBOARD | XUI_EDIT_CAP_CARET_RECT |
	XUI_EDIT_CAP_CONTEXT_MENU | XUI_EDIT_CAP_IME | XUI_EDIT_CAP_TERMINAL,
	NULL, NULL, __xuiTerminalEditSetSelection, __xuiTerminalEditGetSelection,
	__xuiTerminalEditHasSelection, xuiTerminalSelectAll, xuiTerminalCopySelection,
	NULL, __xuiTerminalEditPaste, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	__xuiTerminalEditCaretRect, xuiTerminalOpenMenu
};

static int __xuiTerminalMax(int a, int b) { return (a > b) ? a : b; }
static int __xuiTerminalMin(int a, int b) { return (a < b) ? a : b; }
static int __xuiTerminalAlpha(uint32_t iColor) { return (int)(iColor & 0xffu); }

static char* __xuiTerminalStrDup(const char* sText)
{
	size_t iLen;
	char* sCopy;

	if ( sText == NULL ) {
		sText = "";
	}
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) {
		return NULL;
	}
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static int __xuiTerminalImeSetText(xui_terminal_data_t* pData, const char* sText, int iSize)
{
	char* sNew;
	if ( pData == NULL || iSize < 0 || (iSize > 0 && sText == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iSize + 1 > pData->iImeCapacity ) {
		sNew = (char*)xrtRealloc(pData->sImeText, (size_t)iSize + 1u);
		if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		pData->sImeText = sNew;
		pData->iImeCapacity = iSize + 1;
	}
	if ( iSize > 0 ) memcpy(pData->sImeText, sText, (size_t)iSize);
	pData->sImeText[iSize] = '\0';
	return XUI_OK;
}

static void __xuiTerminalFreeLinks(xui_terminal_data_t* pData)
{
	int i;

	if ( pData == NULL ) {
		return;
	}
	if ( pData->ppLinks != NULL ) {
		for ( i = 0; i < pData->iLinkCount; i++ ) {
			if ( pData->ppLinks[i] != NULL ) {
				xrtFree(pData->ppLinks[i]);
			}
		}
		xrtFree(pData->ppLinks);
	}
	pData->ppLinks = NULL;
	pData->iLinkCount = 0;
	pData->iLinkCapacity = 0;
	pData->iCurrentLinkId = 0;
}

static const char* __xuiTerminalGetLinkUrl(const xui_terminal_data_t* pData, uint16_t iLinkId)
{
	int iIndex;

	if ( pData == NULL || iLinkId == 0 ) {
		return NULL;
	}
	iIndex = (int)iLinkId - 1;
	if ( iIndex < 0 || iIndex >= pData->iLinkCount || pData->ppLinks == NULL ) {
		return NULL;
	}
	return pData->ppLinks[iIndex];
}

static uint16_t __xuiTerminalGetOrAddLink(xui_terminal_data_t* pData, const char* sUrl)
{
	char** ppNew;
	char* sCopy;
	int iNewCapacity;
	int i;

	if ( pData == NULL || sUrl == NULL || sUrl[0] == '\0' ) {
		return 0;
	}
	for ( i = 0; i < pData->iLinkCount; i++ ) {
		if ( pData->ppLinks != NULL && pData->ppLinks[i] != NULL && strcmp(pData->ppLinks[i], sUrl) == 0 ) {
			return (uint16_t)(i + 1);
		}
	}
	if ( (uint32_t)pData->iLinkCount >= XUI_TERMINAL_MAX_LINK_ID ) {
		return 0;
	}
	if ( pData->iLinkCount >= pData->iLinkCapacity ) {
		iNewCapacity = (pData->iLinkCapacity > 0) ? (pData->iLinkCapacity * 2) : 16;
		if ( iNewCapacity <= pData->iLinkCount ) iNewCapacity = pData->iLinkCount + 1;
		if ( (uint32_t)iNewCapacity > XUI_TERMINAL_MAX_LINK_ID ) iNewCapacity = (int)XUI_TERMINAL_MAX_LINK_ID;
		ppNew = (char**)xrtRealloc(pData->ppLinks, sizeof(*ppNew) * (size_t)iNewCapacity);
		if ( ppNew == NULL ) {
			return 0;
		}
		memset(ppNew + pData->iLinkCapacity, 0, sizeof(*ppNew) * (size_t)(iNewCapacity - pData->iLinkCapacity));
		pData->ppLinks = ppNew;
		pData->iLinkCapacity = iNewCapacity;
	}
	sCopy = __xuiTerminalStrDup(sUrl);
	if ( sCopy == NULL ) {
		return 0;
	}
	pData->ppLinks[pData->iLinkCount] = sCopy;
	pData->iLinkCount++;
	return (uint16_t)pData->iLinkCount;
}

#if defined(_WIN32) || defined(_WIN64)
static int __xuiTerminalConptyGetApi(xui_terminal_conpty_api_t* pApi)
{
	HMODULE hKernel;

	if ( pApi == NULL ) return 0;
	memset(pApi, 0, sizeof(*pApi));
	hKernel = GetModuleHandleA("kernel32.dll");
	if ( hKernel == NULL ) return 0;
	pApi->createPseudoConsole = (xui_terminal_create_pseudo_console_proc)GetProcAddress(hKernel, "CreatePseudoConsole");
	pApi->resizePseudoConsole = (xui_terminal_resize_pseudo_console_proc)GetProcAddress(hKernel, "ResizePseudoConsole");
	pApi->closePseudoConsole = (xui_terminal_close_pseudo_console_proc)GetProcAddress(hKernel, "ClosePseudoConsole");
	return (pApi->createPseudoConsole != NULL) && (pApi->resizePseudoConsole != NULL) && (pApi->closePseudoConsole != NULL);
}

static COORD __xuiTerminalConptySize(int iColumns, int iRows)
{
	COORD tSize;

	if ( iColumns <= 0 ) iColumns = XUI_TERMINAL_DEFAULT_COLS;
	if ( iRows <= 0 ) iRows = XUI_TERMINAL_DEFAULT_ROWS;
	if ( iColumns > 32767 ) iColumns = 32767;
	if ( iRows > 32767 ) iRows = 32767;
	tSize.X = (SHORT)iColumns;
	tSize.Y = (SHORT)iRows;
	return tSize;
}
#endif

static uint32_t __xuiTerminalProcessDescFlags(const xui_terminal_process_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 0u;
	if ( pDesc->iSize < (uint32_t)(offsetof(xui_terminal_process_desc_t, iFlags) + sizeof(pDesc->iFlags)) ) return 0u;
	return pDesc->iFlags;
}

static int __xuiTerminalProcessDescColumns(const xui_terminal_process_desc_t* pDesc)
{
	if ( pDesc == NULL ) return XUI_TERMINAL_DEFAULT_COLS;
	if ( pDesc->iSize < (uint32_t)(offsetof(xui_terminal_process_desc_t, iColumns) + sizeof(pDesc->iColumns)) ) return XUI_TERMINAL_DEFAULT_COLS;
	return (pDesc->iColumns > 0) ? pDesc->iColumns : XUI_TERMINAL_DEFAULT_COLS;
}

static int __xuiTerminalProcessDescRows(const xui_terminal_process_desc_t* pDesc)
{
	if ( pDesc == NULL ) return XUI_TERMINAL_DEFAULT_ROWS;
	if ( pDesc->iSize < (uint32_t)(offsetof(xui_terminal_process_desc_t, iRows) + sizeof(pDesc->iRows)) ) return XUI_TERMINAL_DEFAULT_ROWS;
	return (pDesc->iRows > 0) ? pDesc->iRows : XUI_TERMINAL_DEFAULT_ROWS;
}

static int __xuiTerminalEncodeUtf8(uint32_t iCodepoint, char* sBuffer)
{
	if ( sBuffer == NULL ) {
		return 0;
	}
	if ( iCodepoint == 0 ) {
		return 0;
	}
	if ( iCodepoint < 0x80u ) {
		sBuffer[0] = (char)iCodepoint;
		return 1;
	}
	if ( iCodepoint < 0x800u ) {
		sBuffer[0] = (char)(0xc0u | (iCodepoint >> 6));
		sBuffer[1] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 2;
	}
	if ( iCodepoint < 0x10000u ) {
		sBuffer[0] = (char)(0xe0u | (iCodepoint >> 12));
		sBuffer[1] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sBuffer[2] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 3;
	}
	if ( iCodepoint <= 0x10ffffu ) {
		sBuffer[0] = (char)(0xf0u | (iCodepoint >> 18));
		sBuffer[1] = (char)(0x80u | ((iCodepoint >> 12) & 0x3fu));
		sBuffer[2] = (char)(0x80u | ((iCodepoint >> 6) & 0x3fu));
		sBuffer[3] = (char)(0x80u | (iCodepoint & 0x3fu));
		return 4;
	}
	return 0;
}

static int __xuiTerminalCodepointWidth(uint32_t iCodepoint)
{
	if ( (iCodepoint >= 0x0300u && iCodepoint <= 0x036fu) ||
	     (iCodepoint >= 0x0483u && iCodepoint <= 0x0489u) ||
	     (iCodepoint >= 0x0591u && iCodepoint <= 0x05bdu) ||
	     (iCodepoint >= 0x05bfu && iCodepoint <= 0x05bfu) ||
	     (iCodepoint >= 0x05c1u && iCodepoint <= 0x05c2u) ||
	     (iCodepoint >= 0x05c4u && iCodepoint <= 0x05c5u) ||
	     (iCodepoint >= 0x0610u && iCodepoint <= 0x061au) ||
	     (iCodepoint >= 0x064bu && iCodepoint <= 0x065fu) ||
	     (iCodepoint >= 0x0670u && iCodepoint <= 0x0670u) ||
	     (iCodepoint >= 0x06d6u && iCodepoint <= 0x06edu) ||
	     (iCodepoint >= 0x1ab0u && iCodepoint <= 0x1affu) ||
	     (iCodepoint >= 0x1dc0u && iCodepoint <= 0x1dffu) ||
	     (iCodepoint >= 0x20d0u && iCodepoint <= 0x20ffu) ||
	     (iCodepoint >= 0xfe00u && iCodepoint <= 0xfe0fu) ||
	     (iCodepoint >= 0xfe20u && iCodepoint <= 0xfe2fu) ||
	     (iCodepoint >= 0xe0100u && iCodepoint <= 0xe01efu) ||
	     iCodepoint == 0x200cu || iCodepoint == 0x200du ) {
		return 0;
	}
	if ( iCodepoint == 0 ) {
		return 1;
	}
	if ( (iCodepoint >= 0x1100u && iCodepoint <= 0x115fu) ||
	     (iCodepoint >= 0x2329u && iCodepoint <= 0x232au) ||
	     (iCodepoint >= 0x2e80u && iCodepoint <= 0xa4cfu) ||
	     (iCodepoint >= 0xac00u && iCodepoint <= 0xd7a3u) ||
	     (iCodepoint >= 0xf900u && iCodepoint <= 0xfaffu) ||
	     (iCodepoint >= 0xfe10u && iCodepoint <= 0xfe19u) ||
	     (iCodepoint >= 0xfe30u && iCodepoint <= 0xfe6fu) ||
	     (iCodepoint >= 0xff00u && iCodepoint <= 0xff60u) ||
	     (iCodepoint >= 0xffe0u && iCodepoint <= 0xffe6u) ||
	     (iCodepoint >= 0x1f300u && iCodepoint <= 0x1faffu) ) {
		return 2;
	}
	return 1;
}

static xui_terminal_cell_t __xuiTerminalBlankCell(const xui_terminal_data_t* pData)
{
	xui_terminal_cell_t tCell;

	memset(&tCell, 0, sizeof(tCell));
	tCell.iSize = sizeof(tCell);
	tCell.iCodepoint = ' ';
	tCell.iFgColor = (pData != NULL) ? pData->iForegroundColor : XUI_COLOR_RGBA(220, 230, 240, 255);
	tCell.iBgColor = (pData != NULL) ? pData->iBackgroundColor : XUI_COLOR_RGBA(18, 24, 32, 255);
	tCell.iWidth = 1;
	return tCell;
}

static xui_terminal_cell_t* __xuiTerminalScreen(xui_terminal_data_t* pData)
{
	return (pData != NULL && pData->bAltScreen) ? pData->pAlt : ((pData != NULL) ? pData->pMain : NULL);
}

static const xui_terminal_cell_t* __xuiTerminalScreenConst(const xui_terminal_data_t* pData)
{
	return (pData != NULL && pData->bAltScreen) ? pData->pAlt : ((pData != NULL) ? pData->pMain : NULL);
}

static int __xuiTerminalBufferRowIndex(const xui_terminal_data_t* pData,
	const xui_terminal_cell_t* pCells, int iRow)
{
	int iOffset = 0;
	if ( pData == NULL || pCells == NULL || pData->iRows <= 0 ) return 0;
	if ( pCells == pData->pMain ) iOffset = pData->iMainRowOffset;
	else if ( pCells == pData->pAlt ) iOffset = pData->iAltRowOffset;
	return (iRow + iOffset) % pData->iRows;
}

static xui_terminal_cell_t* __xuiTerminalBufferRow(xui_terminal_data_t* pData,
	xui_terminal_cell_t* pCells, int iRow)
{
	if ( pData == NULL || pCells == NULL || iRow < 0 || iRow >= pData->iRows ) return NULL;
	return pCells + __xuiTerminalBufferRowIndex(pData, pCells, iRow) * pData->iColumns;
}

static const xui_terminal_cell_t* __xuiTerminalBufferRowConst(const xui_terminal_data_t* pData,
	const xui_terminal_cell_t* pCells, int iRow)
{
	if ( pData == NULL || pCells == NULL || iRow < 0 || iRow >= pData->iRows ) return NULL;
	return pCells + __xuiTerminalBufferRowIndex(pData, pCells, iRow) * pData->iColumns;
}

static xui_terminal_cell_t* __xuiTerminalScreenRow(xui_terminal_data_t* pData, int iRow)
{
	return __xuiTerminalBufferRow(pData, __xuiTerminalScreen(pData), iRow);
}

static const xui_terminal_cell_t* __xuiTerminalScreenRowConst(const xui_terminal_data_t* pData, int iRow)
{
	return __xuiTerminalBufferRowConst(pData, __xuiTerminalScreenConst(pData), iRow);
}

static xui_terminal_cell_t __xuiTerminalEraseCell(const xui_terminal_data_t* pData)
{
	xui_terminal_cell_t tCell = __xuiTerminalBlankCell(pData);
	if ( pData != NULL ) {
		tCell.iFgColor = pData->iCurrentFg;
		tCell.iBgColor = pData->iCurrentBg;
	}
	return tCell;
}

static uint8_t* __xuiTerminalScreenWrapped(xui_terminal_data_t* pData)
{
	return (pData != NULL && pData->bAltScreen) ? pData->pAltWrapped : ((pData != NULL) ? pData->pMainWrapped : NULL);
}

static void __xuiTerminalMarkDirtyRow(xui_terminal_data_t* pData, int iRow)
{
	if ( pData == NULL || pData->pDirtyRows == NULL || iRow < 0 || iRow >= pData->iRows ) {
		return;
	}
	if ( pData->pDirtyRows[iRow] == 0u ) {
		pData->pDirtyRows[iRow] = 1u;
		pData->iDirtyRowCount++;
	}
}

static void __xuiTerminalMarkDirtyRows(xui_terminal_data_t* pData, int iStartRow, int iEndRow)
{
	int i;

	if ( pData == NULL || pData->iRows <= 0 ) {
		return;
	}
	if ( iStartRow < 0 ) iStartRow = 0;
	if ( iEndRow >= pData->iRows ) iEndRow = pData->iRows - 1;
	if ( iEndRow < iStartRow ) return;
	for ( i = iStartRow; i <= iEndRow; i++ ) {
		__xuiTerminalMarkDirtyRow(pData, i);
	}
}

static void __xuiTerminalMarkAllRowsDirty(xui_terminal_data_t* pData)
{
	if ( pData == NULL || pData->pDirtyRows == NULL || pData->iRows <= 0 ) {
		return;
	}
	memset(pData->pDirtyRows, 1, (size_t)pData->iRows);
	pData->iDirtyRowCount = pData->iRows;
}

static void __xuiTerminalMarkFullCacheDirty(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) {
		return;
	}
	pData->bFullCacheDirty = 1;
	__xuiTerminalMarkAllRowsDirty(pData);
}

static void __xuiTerminalClearDirtyRows(xui_terminal_data_t* pData)
{
	if ( pData == NULL || pData->pDirtyRows == NULL || pData->iRows <= 0 ) {
		return;
	}
	memset(pData->pDirtyRows, 0, (size_t)pData->iRows);
	pData->iDirtyRowCount = 0;
}

static void __xuiTerminalResetParser(xui_terminal_parser_t* pParser)
{
	if ( pParser == NULL ) {
		return;
	}
	memset(pParser, 0, sizeof(*pParser));
}

static void __xuiTerminalDefaultPalette(xui_terminal_data_t* pData)
{
	static const uint32_t arrBase[16] = {
		XUI_COLOR_RGBA(0, 0, 0, 255),
		XUI_COLOR_RGBA(205, 49, 49, 255),
		XUI_COLOR_RGBA(13, 188, 121, 255),
		XUI_COLOR_RGBA(229, 229, 16, 255),
		XUI_COLOR_RGBA(36, 114, 200, 255),
		XUI_COLOR_RGBA(188, 63, 188, 255),
		XUI_COLOR_RGBA(17, 168, 205, 255),
		XUI_COLOR_RGBA(229, 229, 229, 255),
		XUI_COLOR_RGBA(102, 102, 102, 255),
		XUI_COLOR_RGBA(241, 76, 76, 255),
		XUI_COLOR_RGBA(35, 209, 139, 255),
		XUI_COLOR_RGBA(245, 245, 67, 255),
		XUI_COLOR_RGBA(59, 142, 234, 255),
		XUI_COLOR_RGBA(214, 112, 214, 255),
		XUI_COLOR_RGBA(41, 184, 219, 255),
		XUI_COLOR_RGBA(255, 255, 255, 255)
	};
	int i;
	int r;
	int g;
	int b;
	int v;
	int idx;
	int arrCube[6] = {0, 95, 135, 175, 215, 255};

	if ( pData == NULL ) {
		return;
	}
	for ( i = 0; i < 16; i++ ) {
		pData->arrBasePalette[i] = arrBase[i];
	}
	idx = 16;
	for ( r = 0; r < 6; r++ ) {
		for ( g = 0; g < 6; g++ ) {
			for ( b = 0; b < 6; b++ ) {
				pData->arrBasePalette[idx++] = XUI_COLOR_RGBA(arrCube[r], arrCube[g], arrCube[b], 255);
			}
		}
	}
	for ( i = 0; i < 24; i++ ) {
		v = 8 + i * 10;
		pData->arrBasePalette[232 + i] = XUI_COLOR_RGBA(v, v, v, 255);
	}
	memcpy(pData->arrPalette, pData->arrBasePalette, sizeof(pData->arrPalette));
}

static void __xuiTerminalHistoryLineDestroy(xui_terminal_history_line_t* pLine)
{
	if ( pLine == NULL ) return;
	xrtFree(pLine->pCells);
	xrtFree(pLine->pCombining);
	xrtFree(pLine->sTextCache);
	xrtFree(pLine);
}

static void __xuiTerminalHistoryViewClear(xui_terminal_data_t* pData)
{
	int i;
	if ( pData == NULL ) return;
	for ( i = 0; i < pData->iScrollbackCount; ++i ) {
		xrtFree(pData->pHistoryView != NULL ? pData->pHistoryView[i].sTextCache : NULL);
		if ( pData->pHistoryView != NULL ) pData->pHistoryView[i].sTextCache = NULL;
	}
	pData->iScrollbackCount = 0;
}

static void __xuiTerminalFreeScrollback(xui_terminal_data_t* pData)
{
	int i;
	int idx;
	if ( pData == NULL ) return;
	__xuiTerminalHistoryViewClear(pData);
	for ( i = 0; i < pData->iHistoryCount; ++i ) {
		idx = (pData->iScrollbackStart + i) % pData->iScrollbackLimit;
		if ( pData->ppHistory != NULL ) {
			__xuiTerminalHistoryLineDestroy(pData->ppHistory[idx]);
			pData->ppHistory[idx] = NULL;
		}
	}
	xrtFree(pData->ppHistory);
	xrtFree(pData->pHistoryView);
	pData->ppHistory = NULL;
	pData->pHistoryView = NULL;
	pData->iScrollbackStart = 0;
	pData->iHistoryCount = 0;
	pData->iHistoryViewCapacity = 0;
	pData->iHistoryViewColumns = 0;
	pData->bHistoryViewDirty = 1;
}

static int __xuiTerminalAllocScrollback(xui_terminal_data_t* pData, int iLimit)
{
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalFreeScrollback(pData);
	pData->iScrollbackLimit = (iLimit > 0) ? iLimit : XUI_TERMINAL_DEFAULT_SCROLLBACK;
	pData->ppHistory = (xui_terminal_history_line_t**)xrtCalloc(
		(size_t)pData->iScrollbackLimit, sizeof(*pData->ppHistory));
	if ( pData->ppHistory == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pData->bHistoryViewDirty = 1;
	return XUI_OK;
}

static char* __xuiTerminalSerializeRow(const xui_terminal_data_t* pData, const xui_terminal_cell_t* pRow)
{
	char sUtf[8];
	char* sLine;
	int iLast;
	int iSize;
	int iCapacity;
	int i;
	int n;

	if ( (pData == NULL) || (pRow == NULL) || (pData->iColumns <= 0) ) {
		return __xuiTerminalStrDup("");
	}
	iLast = pData->iColumns - 1;
	while ( (iLast >= 0) &&
	        ((pRow[iLast].iCodepoint == 0u) || (pRow[iLast].iCodepoint == ' ') || ((pRow[iLast].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u)) ) {
		iLast--;
	}
	iCapacity = (iLast + 2) * 4;
	if ( iCapacity < 1 ) {
		iCapacity = 1;
	}
	sLine = (char*)xrtMalloc((size_t)iCapacity);
	if ( sLine == NULL ) {
		return NULL;
	}
	iSize = 0;
	for ( i = 0; i <= iLast; i++ ) {
		int j;
		if ( (pRow[i].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u ) {
			continue;
		}
		n = __xuiTerminalEncodeUtf8(pRow[i].iCodepoint ? pRow[i].iCodepoint : ' ', sUtf);
		if ( (iSize + n + 1) >= iCapacity ) {
			char* sNew;
			iCapacity *= 2;
			sNew = (char*)xrtRealloc(sLine, (size_t)iCapacity);
			if ( sNew == NULL ) {
				xrtFree(sLine);
				return NULL;
			}
			sLine = sNew;
		}
		if ( n > 0 ) {
			memcpy(sLine + iSize, sUtf, (size_t)n);
			iSize += n;
		}
		for ( j = 0; j < pRow[i].iCombiningCount; ++j ) {
			n = __xuiTerminalEncodeUtf8(pRow[i].arrCombining[j], sUtf);
			if ( (iSize + n + 1) >= iCapacity ) {
				char* sNew;
				while ( (iSize + n + 1) >= iCapacity ) iCapacity *= 2;
				sNew = (char*)xrtRealloc(sLine, (size_t)iCapacity);
				if ( sNew == NULL ) {
					xrtFree(sLine);
					return NULL;
				}
				sLine = sNew;
			}
			if ( n > 0 ) {
				memcpy(sLine + iSize, sUtf, (size_t)n);
				iSize += n;
			}
		}
	}
	sLine[iSize] = '\0';
	return sLine;
}

static void __xuiTerminalClearScrollbackSlot(xui_terminal_data_t* pData, int idx)
{
	if ( pData == NULL || idx < 0 || idx >= pData->iScrollbackLimit || pData->ppHistory == NULL ) return;
	__xuiTerminalHistoryLineDestroy(pData->ppHistory[idx]);
	pData->ppHistory[idx] = NULL;
}

static void __xuiTerminalHistoryLineReset(xui_terminal_history_line_t* pLine)
{
	if ( pLine == NULL ) return;
	xrtFree(pLine->sTextCache);
	pLine->sTextCache = NULL;
	pLine->iCellCount = 0;
	pLine->iCombiningCount = 0;
	pLine->bOpen = 0;
}

static int __xuiTerminalHistoryReserveCells(xui_terminal_history_line_t* pLine, int iNeeded)
{
	xui_terminal_history_cell_t* pNew;
	int iCapacity;
	if ( pLine == NULL || iNeeded < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeeded <= pLine->iCellCapacity ) return XUI_OK;
	if ( pLine->iCellCapacity > 0 ) {
		iCapacity = pLine->iCellCapacity + pLine->iCellCapacity / 2;
		if ( iCapacity < iNeeded ) iCapacity = iNeeded;
	} else {
		iCapacity = iNeeded > 0 ? iNeeded : 16;
	}
	iCapacity = (iCapacity + 15) & ~15;
	pNew = (xui_terminal_history_cell_t*)xrtRealloc(pLine->pCells, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pLine->pCells = pNew;
	pLine->iCellCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTerminalHistoryReserveCombining(xui_terminal_history_line_t* pLine, int iNeeded)
{
	uint32_t* pNew;
	int iCapacity;
	if ( pLine == NULL || iNeeded < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeeded <= pLine->iCombiningCapacity ) return XUI_OK;
	iCapacity = pLine->iCombiningCapacity > 0 ? pLine->iCombiningCapacity : 16;
	while ( iCapacity < iNeeded ) iCapacity *= 2;
	pNew = (uint32_t*)xrtRealloc(pLine->pCombining, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pLine->pCombining = pNew;
	pLine->iCombiningCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTerminalHistoryAppendRow(xui_terminal_history_line_t* pLine,
	const xui_terminal_cell_t* pRow, int iColumns, int bSoftWrapped)
{
	int i;
	int iCount;
	int iCombining;
	int iRet;
	if ( pLine == NULL || pRow == NULL || iColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iCount = iColumns;
	if ( !bSoftWrapped ) {
		while ( iCount > 0 ) {
			const xui_terminal_cell_t* pCell = &pRow[iCount - 1];
			if ( pCell->iCodepoint != 0u && pCell->iCodepoint != ' ' &&
			    (pCell->iFlags & XUI_TERMINAL_CELL_WIDE_CONT) == 0u ) break;
			iCount--;
		}
	}
	iCombining = 0;
	for ( i = 0; i < iCount; ++i ) iCombining += pRow[i].iCombiningCount;
	iRet = __xuiTerminalHistoryReserveCells(pLine, pLine->iCellCount + iCount);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiTerminalHistoryReserveCombining(pLine, pLine->iCombiningCount + iCombining);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < iCount; ++i ) {
		const xui_terminal_cell_t* pSource = &pRow[i];
		xui_terminal_history_cell_t* pTarget = &pLine->pCells[pLine->iCellCount++];
		memset(pTarget, 0, sizeof(*pTarget));
		pTarget->iCodepoint = pSource->iCodepoint;
		pTarget->iFgColor = pSource->iFgColor;
		pTarget->iBgColor = pSource->iBgColor;
		pTarget->iFlags = pSource->iFlags;
		pTarget->iStyle = pSource->iStyle;
		pTarget->iLinkId = pSource->iLinkId;
		pTarget->iWidth = pSource->iWidth;
		pTarget->iCombiningCount = pSource->iCombiningCount;
		pTarget->iCombiningOffset = (uint32_t)pLine->iCombiningCount;
		if ( pSource->iCombiningCount > 0 ) {
			memcpy(pLine->pCombining + pLine->iCombiningCount, pSource->arrCombining,
				sizeof(uint32_t) * (size_t)pSource->iCombiningCount);
			pLine->iCombiningCount += pSource->iCombiningCount;
		}
	}
	xrtFree(pLine->sTextCache);
	pLine->sTextCache = NULL;
	pLine->bOpen = bSoftWrapped != 0;
	return XUI_OK;
}

static xui_terminal_history_line_t* __xuiTerminalHistoryLastLine(xui_terminal_data_t* pData)
{
	int idx;
	if ( pData == NULL || pData->ppHistory == NULL || pData->iHistoryCount <= 0 || pData->iScrollbackLimit <= 0 ) return NULL;
	idx = (pData->iScrollbackStart + pData->iHistoryCount - 1) % pData->iScrollbackLimit;
	return pData->ppHistory[idx];
}

static int __xuiTerminalPushScrollback(xui_terminal_data_t* pData,
	const xui_terminal_cell_t* pRow, int iColumns, int bSoftWrapped)
{
	xui_terminal_history_line_t* pLine;
	int idx;
	int iRet;
	if ( pData == NULL || pData->ppHistory == NULL || pData->iScrollbackLimit <= 0 || pRow == NULL || iColumns <= 0 ) return XUI_OK;
	pLine = __xuiTerminalHistoryLastLine(pData);
	if ( pLine == NULL || !pLine->bOpen ) {
		if ( pData->iHistoryCount < pData->iScrollbackLimit ) {
			idx = (pData->iScrollbackStart + pData->iHistoryCount) % pData->iScrollbackLimit;
			pData->iHistoryCount++;
			pLine = (xui_terminal_history_line_t*)xrtCalloc(1, sizeof(*pLine));
			if ( pLine == NULL ) {
				pData->iHistoryCount--;
				return XUI_ERROR_OUT_OF_MEMORY;
			}
		} else {
			idx = pData->iScrollbackStart;
			pLine = pData->ppHistory[idx];
			pData->ppHistory[idx] = NULL;
			pData->iScrollbackStart = (pData->iScrollbackStart + 1) % pData->iScrollbackLimit;
			if ( pLine == NULL ) {
				pLine = (xui_terminal_history_line_t*)xrtCalloc(1, sizeof(*pLine));
				if ( pLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
			} else {
				__xuiTerminalHistoryLineReset(pLine);
			}
		}
		pLine->iId = ++pData->iNextHistoryId;
		pData->ppHistory[idx] = pLine;
	}
	iRet = __xuiTerminalHistoryAppendRow(pLine, pRow, iColumns, bSoftWrapped);
	if ( iRet != XUI_OK ) return iRet;
	pData->bHistoryViewDirty = 1;
	return XUI_OK;
}

static int __xuiTerminalHistoryViewReserve(xui_terminal_data_t* pData, int iNeeded)
{
	xui_terminal_history_view_row_t* pNew;
	int iCapacity;
	if ( pData == NULL || iNeeded < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeeded <= pData->iHistoryViewCapacity ) return XUI_OK;
	iCapacity = pData->iHistoryViewCapacity > 0 ? pData->iHistoryViewCapacity : XUI_TERMINAL_HISTORY_INITIAL_VIEW;
	while ( iCapacity < iNeeded ) iCapacity *= 2;
	pNew = (xui_terminal_history_view_row_t*)xrtRealloc(pData->pHistoryView, sizeof(*pNew) * (size_t)iCapacity);
	if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pNew + pData->iHistoryViewCapacity, 0,
		sizeof(*pNew) * (size_t)(iCapacity - pData->iHistoryViewCapacity));
	pData->pHistoryView = pNew;
	pData->iHistoryViewCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTerminalRebuildHistoryView(xui_terminal_data_t* pData)
{
	int i;
	int idx;
	int iStart;
	int iEnd;
	int iRet;
	if ( pData == NULL || pData->iColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pData->bHistoryViewDirty && pData->iHistoryViewColumns == pData->iColumns ) return XUI_OK;
	__xuiTerminalHistoryViewClear(pData);
	for ( i = 0; i < pData->iHistoryCount; ++i ) {
		xui_terminal_history_line_t* pLine;
		idx = (pData->iScrollbackStart + i) % pData->iScrollbackLimit;
		pLine = pData->ppHistory != NULL ? pData->ppHistory[idx] : NULL;
		if ( pLine == NULL ) continue;
		if ( pLine->iCellCount <= 0 ) {
			iRet = __xuiTerminalHistoryViewReserve(pData, pData->iScrollbackCount + 1);
			if ( iRet != XUI_OK ) return iRet;
			pData->pHistoryView[pData->iScrollbackCount].pLine = pLine;
			pData->pHistoryView[pData->iScrollbackCount].iStart = 0;
			pData->pHistoryView[pData->iScrollbackCount].iEnd = 0;
			pData->iScrollbackCount++;
			continue;
		}
		iStart = 0;
		while ( iStart < pLine->iCellCount ) {
			iEnd = __xuiTerminalMin(iStart + pData->iColumns, pLine->iCellCount);
			if ( iEnd < pLine->iCellCount && iEnd > iStart &&
			     (pLine->pCells[iEnd - 1].iFlags & XUI_TERMINAL_CELL_WIDE) != 0u &&
			     (pLine->pCells[iEnd].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u ) iEnd--;
			if ( iEnd <= iStart ) iEnd = __xuiTerminalMin(iStart + 1, pLine->iCellCount);
			iRet = __xuiTerminalHistoryViewReserve(pData, pData->iScrollbackCount + 1);
			if ( iRet != XUI_OK ) return iRet;
			pData->pHistoryView[pData->iScrollbackCount].pLine = pLine;
			pData->pHistoryView[pData->iScrollbackCount].iStart = iStart;
			pData->pHistoryView[pData->iScrollbackCount].iEnd = iEnd;
			pData->pHistoryView[pData->iScrollbackCount].sTextCache = NULL;
			pData->iScrollbackCount++;
			iStart = iEnd;
		}
	}
	pData->iHistoryViewColumns = pData->iColumns;
	pData->bHistoryViewDirty = 0;
	return XUI_OK;
}

static xui_terminal_history_view_row_t* __xuiTerminalGetHistoryViewRow(xui_terminal_data_t* pData, int iLine)
{
	if ( pData == NULL || __xuiTerminalRebuildHistoryView(pData) != XUI_OK || iLine < 0 || iLine >= pData->iScrollbackCount ) return NULL;
	return &pData->pHistoryView[iLine];
}

static void __xuiTerminalHistoryCellToPublic(const xui_terminal_history_line_t* pLine,
	int iCell, xui_terminal_cell_t* pTarget)
{
	const xui_terminal_history_cell_t* pSource;
	int iCount;
	if ( pTarget == NULL ) return;
	memset(pTarget, 0, sizeof(*pTarget));
	pTarget->iSize = sizeof(*pTarget);
	if ( pLine == NULL || iCell < 0 || iCell >= pLine->iCellCount ) return;
	pSource = &pLine->pCells[iCell];
	pTarget->iCodepoint = pSource->iCodepoint;
	pTarget->iFgColor = pSource->iFgColor;
	pTarget->iBgColor = pSource->iBgColor;
	pTarget->iFlags = pSource->iFlags;
	pTarget->iStyle = pSource->iStyle;
	pTarget->iLinkId = pSource->iLinkId;
	pTarget->iWidth = pSource->iWidth;
	iCount = pSource->iCombiningCount;
	if ( iCount > XUI_TERMINAL_MAX_COMBINING ) iCount = XUI_TERMINAL_MAX_COMBINING;
	pTarget->iCombiningCount = (uint8_t)iCount;
	if ( iCount > 0 && pLine->pCombining != NULL ) {
		memcpy(pTarget->arrCombining, pLine->pCombining + pSource->iCombiningOffset,
			sizeof(uint32_t) * (size_t)iCount);
	}
}

static const char* __xuiTerminalGetScrollbackLine(xui_terminal_data_t* pData, int iLine)
{
	xui_terminal_history_view_row_t* pView;
	xui_terminal_cell_t tCell;
	char sUtf[8];
	char* sText;
	int i;
	int j;
	int n;
	int iSize;
	int iCapacity;
	pView = __xuiTerminalGetHistoryViewRow(pData, iLine);
	if ( pView == NULL ) return "";
	if ( pView->sTextCache != NULL ) return pView->sTextCache;
	iCapacity = (pView->iEnd - pView->iStart + 1) * 4 + 1;
	if ( iCapacity < 1 ) iCapacity = 1;
	sText = (char*)xrtMalloc((size_t)iCapacity);
	if ( sText == NULL ) return "";
	iSize = 0;
	for ( i = pView->iStart; i < pView->iEnd; ++i ) {
		__xuiTerminalHistoryCellToPublic(pView->pLine, i, &tCell);
		if ( (tCell.iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u ) continue;
		n = __xuiTerminalEncodeUtf8(tCell.iCodepoint != 0u ? tCell.iCodepoint : ' ', sUtf);
		while ( iSize + n + 1 >= iCapacity ) {
			char* sNew;
			iCapacity *= 2;
			sNew = (char*)xrtRealloc(sText, (size_t)iCapacity);
			if ( sNew == NULL ) { xrtFree(sText); return ""; }
			sText = sNew;
		}
		if ( n > 0 ) { memcpy(sText + iSize, sUtf, (size_t)n); iSize += n; }
		for ( j = 0; j < tCell.iCombiningCount; ++j ) {
			n = __xuiTerminalEncodeUtf8(tCell.arrCombining[j], sUtf);
			while ( iSize + n + 1 >= iCapacity ) {
				char* sNew;
				iCapacity *= 2;
				sNew = (char*)xrtRealloc(sText, (size_t)iCapacity);
				if ( sNew == NULL ) { xrtFree(sText); return ""; }
				sText = sNew;
			}
			if ( n > 0 ) { memcpy(sText + iSize, sUtf, (size_t)n); iSize += n; }
		}
	}
	while ( iSize > 0 && sText[iSize - 1] == ' ' ) iSize--;
	sText[iSize] = '\0';
	pView->sTextCache = sText;
	return sText;
}

static void __xuiTerminalBlankRow(xui_terminal_data_t* pData, xui_terminal_cell_t* pCells, int iRow)
{
	xui_terminal_cell_t tBlank;
	xui_terminal_cell_t* pRow;
	int i;

	if ( (pData == NULL) || (pCells == NULL) || (iRow < 0) || (iRow >= pData->iRows) ) {
		return;
	}
	pRow = __xuiTerminalBufferRow(pData, pCells, iRow);
	if ( pRow == NULL ) return;
	tBlank = __xuiTerminalBlankCell(pData);
	for ( i = 0; i < pData->iColumns; i++ ) {
		pRow[i] = tBlank;
	}
	if ( pCells == pData->pMain && pData->pMainWrapped != NULL ) pData->pMainWrapped[iRow] = 0u;
	if ( pCells == pData->pAlt && pData->pAltWrapped != NULL ) pData->pAltWrapped[iRow] = 0u;
	__xuiTerminalMarkDirtyRow(pData, iRow);
}

static void __xuiTerminalClearCells(xui_terminal_data_t* pData, xui_terminal_cell_t* pCells)
{
	int y;

	if ( (pData == NULL) || (pCells == NULL) ) {
		return;
	}
	for ( y = 0; y < pData->iRows; y++ ) {
		__xuiTerminalBlankRow(pData, pCells, y);
	}
}

static void __xuiTerminalSetDefaultTabStops(uint8_t* pTabStops, int iColumns)
{
	int i;

	if ( pTabStops == NULL || iColumns <= 0 ) return;
	for ( i = 0; i < iColumns; i++ ) {
		pTabStops[i] = (uint8_t)((i > 0 && (i % 8) == 0) ? 1 : 0);
	}
}

static int __xuiTerminalNextTabStop(const xui_terminal_data_t* pData)
{
	int i;

	if ( pData == NULL || pData->iColumns <= 0 ) return 0;
	if ( pData->pTabStops != NULL ) {
		for ( i = pData->iCursorX + 1; i < pData->iColumns; i++ ) {
			if ( pData->pTabStops[i] != 0u ) return i;
		}
	}
	return pData->iColumns - 1;
}

static void __xuiTerminalSetTabStop(xui_terminal_data_t* pData, int iColumn, int bSet)
{
	if ( pData == NULL || pData->pTabStops == NULL || iColumn < 0 || iColumn >= pData->iColumns ) return;
	pData->pTabStops[iColumn] = (uint8_t)(bSet ? 1 : 0);
}

static void __xuiTerminalClearAllTabStops(xui_terminal_data_t* pData)
{
	if ( pData == NULL || pData->pTabStops == NULL || pData->iColumns <= 0 ) return;
	memset(pData->pTabStops, 0, (size_t)pData->iColumns);
}

static int __xuiTerminalLastContentRow(const xui_terminal_data_t* pData,
	const xui_terminal_cell_t* pCells, int iColumns, int iRows, int iRowOffset)
{
	int x;
	int y;

	if ( pData == NULL || pCells == NULL || iColumns <= 0 || iRows <= 0 ) return -1;
	for ( y = iRows - 1; y >= 0; y-- ) {
		const xui_terminal_cell_t* pRow = pCells + ((y + iRowOffset) % iRows) * iColumns;
		for ( x = 0; x < iColumns; x++ ) {
			const xui_terminal_cell_t* pCell = &pRow[x];
			if ( pCell->iCodepoint != 0u && pCell->iCodepoint != ' ' ) return y;
		}
	}
	return -1;
}

static void __xuiTerminalPopScrollbackRow(xui_terminal_data_t* pData, xui_terminal_cell_t* pRow)
{
	xui_terminal_history_view_row_t* pView;
	xui_terminal_history_line_t* pLine;
	int i;
	int iIndex;
	int iCount;
	if ( pData == NULL || pRow == NULL || pData->iColumns <= 0 ) return;
	if ( __xuiTerminalRebuildHistoryView(pData) != XUI_OK || pData->iScrollbackCount <= 0 ) return;
	pView = &pData->pHistoryView[pData->iScrollbackCount - 1];
	pLine = pView->pLine;
	iCount = __xuiTerminalMin(pView->iEnd - pView->iStart, pData->iColumns);
	for ( i = 0; i < iCount; ++i ) {
		__xuiTerminalHistoryCellToPublic(pLine, pView->iStart + i, &pRow[i]);
	}
	if ( pView->iStart > 0 ) {
		pLine->iCellCount = pView->iStart;
		pLine->bOpen = 1;
		xrtFree(pLine->sTextCache);
		pLine->sTextCache = NULL;
	} else if ( pData->iHistoryCount > 0 ) {
		iIndex = (pData->iScrollbackStart + pData->iHistoryCount - 1) % pData->iScrollbackLimit;
		__xuiTerminalClearScrollbackSlot(pData, iIndex);
		pData->iHistoryCount--;
		if ( pData->iHistoryCount == 0 ) pData->iScrollbackStart = 0;
	}
	pData->bHistoryViewDirty = 1;
	(void)__xuiTerminalRebuildHistoryView(pData);
}

typedef struct xui_terminal_reflow_t {
	xui_terminal_cell_t* pCells;
	uint8_t* pWrapped;
	int iRows;
	int iCapacity;
	int iColumns;
	int iCursorRow;
	int iCursorColumn;
} xui_terminal_reflow_t;

static void __xuiTerminalReflowUnit(xui_terminal_reflow_t* pReflow)
{
	if ( pReflow == NULL ) return;
	xrtFree(pReflow->pCells);
	xrtFree(pReflow->pWrapped);
	memset(pReflow, 0, sizeof(*pReflow));
}

static int __xuiTerminalReflowReserve(xui_terminal_reflow_t* pReflow, int iRows)
{
	xui_terminal_cell_t* pCells;
	uint8_t* pWrapped;
	int iCapacity;
	if ( pReflow == NULL || iRows < 0 || pReflow->iColumns <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iRows <= pReflow->iCapacity ) return XUI_OK;
	iCapacity = pReflow->iCapacity > 0 ? pReflow->iCapacity : 16;
	while ( iCapacity < iRows ) iCapacity *= 2;
	if ( (size_t)iCapacity > SIZE_MAX / sizeof(*pCells) / (size_t)pReflow->iColumns ) return XUI_ERROR_OUT_OF_MEMORY;
	pCells = (xui_terminal_cell_t*)xrtRealloc(pReflow->pCells,
		sizeof(*pCells) * (size_t)iCapacity * (size_t)pReflow->iColumns);
	if ( pCells == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pWrapped = (uint8_t*)xrtRealloc(pReflow->pWrapped, (size_t)iCapacity);
	if ( pWrapped == NULL ) {
		pReflow->pCells = pCells;
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pReflow->pCells = pCells;
	pReflow->pWrapped = pWrapped;
	pReflow->iCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiTerminalRowContentColumns(const xui_terminal_cell_t* pRow,
	int iColumns, int bWrapped, int iMinimum)
{
	int iCount;
	if ( pRow == NULL || iColumns <= 0 ) return 0;
	iCount = bWrapped ? iColumns : 0;
	if ( !bWrapped ) {
		for ( iCount = iColumns; iCount > 0; --iCount ) {
			const xui_terminal_cell_t* pCell = &pRow[iCount - 1];
			if ( pCell->iCodepoint != 0u && pCell->iCodepoint != ' ' &&
			    (pCell->iFlags & XUI_TERMINAL_CELL_WIDE_CONT) == 0u ) break;
		}
	}
	if ( iCount < iMinimum ) iCount = iMinimum;
	if ( iCount > iColumns ) iCount = iColumns;
	return iCount;
}

static int __xuiTerminalReflowMain(const xui_terminal_data_t* pData,
	const xui_terminal_cell_t* pOld, const uint8_t* pOldWrapped,
	int iOldColumns, int iOldRows, int iOldRowOffset,
	int iNewColumns, xui_terminal_reflow_t* pOut)
{
	xui_terminal_cell_t* pLogical;
	int iLogicalCapacity;
	int iLogicalCount;
	int iLastRow;
	int iGroupStart;
	int iGroupEnd;
	int iCursorOffset;
	int iCursorLogical;
	int iStart;
	int iEnd;
	int i;
	int y;
	int iRet;
	xui_terminal_cell_t tBlank;
	if ( pData == NULL || pOld == NULL || iOldColumns <= 0 || iOldRows <= 0 || iNewColumns <= 0 || pOut == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(pOut, 0, sizeof(*pOut));
	pOut->iColumns = iNewColumns;
	pOut->iCursorRow = 0;
	pOut->iCursorColumn = 0;
	pLogical = NULL;
	iLogicalCapacity = 0;
	iLastRow = __xuiTerminalLastContentRow(pData, pOld, iOldColumns, iOldRows, iOldRowOffset);
	if ( pData->iCursorY > iLastRow ) iLastRow = pData->iCursorY;
	if ( iLastRow < 0 ) iLastRow = 0;
	tBlank = __xuiTerminalEraseCell(pData);
	for ( iGroupStart = 0; iGroupStart <= iLastRow; iGroupStart = iGroupEnd + 1 ) {
		iGroupEnd = iGroupStart;
		while ( iGroupEnd < iLastRow && pOldWrapped != NULL && pOldWrapped[iGroupEnd] != 0u ) iGroupEnd++;
		iLogicalCount = 0;
		iCursorOffset = -1;
		for ( y = iGroupStart; y <= iGroupEnd; ++y ) {
			const xui_terminal_cell_t* pOldRow = pOld + ((y + iOldRowOffset) % iOldRows) * iOldColumns;
			int iMinimum = (y == pData->iCursorY) ? pData->iCursorX + 1 : 0;
			int iCount = __xuiTerminalRowContentColumns(pOldRow, iOldColumns,
				pOldWrapped != NULL && pOldWrapped[y] != 0u, iMinimum);
			if ( iLogicalCount + iCount > iLogicalCapacity ) {
				int iCapacity = iLogicalCapacity > 0 ? iLogicalCapacity : 64;
				xui_terminal_cell_t* pNew;
				while ( iCapacity < iLogicalCount + iCount ) iCapacity *= 2;
				pNew = (xui_terminal_cell_t*)xrtRealloc(pLogical, sizeof(*pNew) * (size_t)iCapacity);
				if ( pNew == NULL ) { xrtFree(pLogical); __xuiTerminalReflowUnit(pOut); return XUI_ERROR_OUT_OF_MEMORY; }
				pLogical = pNew;
				iLogicalCapacity = iCapacity;
			}
			if ( y == pData->iCursorY ) iCursorOffset = iLogicalCount + pData->iCursorX;
			if ( iCount > 0 ) memcpy(pLogical + iLogicalCount, pOldRow,
				sizeof(*pLogical) * (size_t)iCount);
			iLogicalCount += iCount;
		}
		if ( iLogicalCount == 0 ) {
			iRet = __xuiTerminalReflowReserve(pOut, pOut->iRows + 1);
			if ( iRet != XUI_OK ) { xrtFree(pLogical); __xuiTerminalReflowUnit(pOut); return iRet; }
			for ( i = 0; i < iNewColumns; ++i ) pOut->pCells[pOut->iRows * iNewColumns + i] = tBlank;
			pOut->pWrapped[pOut->iRows] = 0u;
			if ( iCursorOffset >= 0 ) { pOut->iCursorRow = pOut->iRows; pOut->iCursorColumn = 0; }
			pOut->iRows++;
			continue;
		}
		iCursorLogical = iCursorOffset;
		iStart = 0;
		while ( iStart < iLogicalCount ) {
			iEnd = __xuiTerminalMin(iStart + iNewColumns, iLogicalCount);
			if ( iEnd < iLogicalCount && iEnd > iStart &&
			     (pLogical[iEnd - 1].iFlags & XUI_TERMINAL_CELL_WIDE) != 0u &&
			     (pLogical[iEnd].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u ) iEnd--;
			if ( iEnd <= iStart ) iEnd = __xuiTerminalMin(iStart + 1, iLogicalCount);
			iRet = __xuiTerminalReflowReserve(pOut, pOut->iRows + 1);
			if ( iRet != XUI_OK ) { xrtFree(pLogical); __xuiTerminalReflowUnit(pOut); return iRet; }
			for ( i = 0; i < iNewColumns; ++i ) pOut->pCells[pOut->iRows * iNewColumns + i] = tBlank;
			memcpy(pOut->pCells + pOut->iRows * iNewColumns, pLogical + iStart,
				sizeof(*pLogical) * (size_t)(iEnd - iStart));
			pOut->pWrapped[pOut->iRows] = (uint8_t)((iEnd < iLogicalCount ||
				(pOldWrapped != NULL && pOldWrapped[iGroupEnd] != 0u)) ? 1 : 0);
			if ( iCursorLogical >= iStart && (iCursorLogical < iEnd || iEnd == iLogicalCount) ) {
				pOut->iCursorRow = pOut->iRows;
				pOut->iCursorColumn = __xuiTerminalMin(iCursorLogical - iStart, iNewColumns - 1);
			}
			pOut->iRows++;
			iStart = iEnd;
		}
	}
	xrtFree(pLogical);
	return XUI_OK;
}

static int __xuiTerminalResizeBuffers(xui_terminal_data_t* pData, int iColumns, int iRows)
{
	xui_terminal_cell_t* pNewMain;
	xui_terminal_cell_t* pNewAlt;
	uint8_t* pNewMainWrapped;
	uint8_t* pNewAltWrapped;
	uint8_t* pNewTabStops;
	uint8_t* pNewDirtyRows;
	xui_terminal_cell_t* pOldMain;
	xui_terminal_cell_t* pOldAlt;
	uint8_t* pOldMainWrapped;
	uint8_t* pOldAltWrapped;
	uint8_t* pOldTabStops;
	uint8_t* pOldDirtyRows;
	xui_terminal_reflow_t tReflow;
	xui_terminal_data_t tMainState;
	int iOldColumns;
	int iOldRows;
	int iOldActiveCursorX;
	int iOldActiveCursorY;
	int iOldMainRowOffset;
	int iOldAltRowOffset;
	int x;
	int y;
	int iCopyColumns;
	int iCopyRows;
	int iMainSourceRow;
	int iMainTargetRow;
	int iRestoreRows;
	int iDropRows;
	int iRet;

	if ( (pData == NULL) || (iColumns <= 0) || (iRows <= 0) || (iColumns > 1000) || (iRows > 1000) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( (pData->iColumns == iColumns) && (pData->iRows == iRows) && (pData->pMain != NULL) && (pData->pAlt != NULL) &&
	     (pData->pMainWrapped != NULL) && (pData->pAltWrapped != NULL) && (pData->pTabStops != NULL) && (pData->pDirtyRows != NULL) ) {
		return XUI_OK;
	}
	memset(&tReflow, 0, sizeof(tReflow));
	pOldMain = pData->pMain;
	pOldAlt = pData->pAlt;
	pOldMainWrapped = pData->pMainWrapped;
	pOldAltWrapped = pData->pAltWrapped;
	pOldTabStops = pData->pTabStops;
	pOldDirtyRows = pData->pDirtyRows;
	iOldColumns = pData->iColumns;
	iOldRows = pData->iRows;
	iOldActiveCursorX = pData->iCursorX;
	iOldActiveCursorY = pData->iCursorY;
	iOldMainRowOffset = pData->iMainRowOffset;
	iOldAltRowOffset = pData->iAltRowOffset;
	if ( pOldMain != NULL && iOldColumns > 0 && iOldRows > 0 ) {
		tMainState = *pData;
		if ( pData->bAltScreen ) {
			tMainState.iCursorX = pData->iMainCursorX;
			tMainState.iCursorY = pData->iMainCursorY;
		}
		iRet = __xuiTerminalReflowMain(&tMainState, pOldMain, pOldMainWrapped,
			iOldColumns, iOldRows, iOldMainRowOffset, iColumns, &tReflow);
		if ( iRet != XUI_OK ) return iRet;
	}
	pNewMain = (xui_terminal_cell_t*)xrtMalloc(sizeof(*pNewMain) * (size_t)iColumns * (size_t)iRows);
	pNewAlt = (xui_terminal_cell_t*)xrtMalloc(sizeof(*pNewAlt) * (size_t)iColumns * (size_t)iRows);
	pNewMainWrapped = (uint8_t*)xrtCalloc((size_t)iRows, sizeof(*pNewMainWrapped));
	pNewAltWrapped = (uint8_t*)xrtCalloc((size_t)iRows, sizeof(*pNewAltWrapped));
	pNewTabStops = (uint8_t*)xrtMalloc((size_t)iColumns);
	pNewDirtyRows = (uint8_t*)xrtCalloc((size_t)iRows, sizeof(*pNewDirtyRows));
	if ( (pNewMain == NULL) || (pNewAlt == NULL) || (pNewMainWrapped == NULL) || (pNewAltWrapped == NULL) ||
	     (pNewTabStops == NULL) || (pNewDirtyRows == NULL) ) {
		if ( pNewMain != NULL ) xrtFree(pNewMain);
		if ( pNewAlt != NULL ) xrtFree(pNewAlt);
		if ( pNewMainWrapped != NULL ) xrtFree(pNewMainWrapped);
		if ( pNewAltWrapped != NULL ) xrtFree(pNewAltWrapped);
		if ( pNewTabStops != NULL ) xrtFree(pNewTabStops);
		if ( pNewDirtyRows != NULL ) xrtFree(pNewDirtyRows);
		__xuiTerminalReflowUnit(&tReflow);
		return XUI_ERROR_OUT_OF_MEMORY;
	}
	pData->pMain = pNewMain;
	pData->pAlt = pNewAlt;
	pData->pMainWrapped = pNewMainWrapped;
	pData->pAltWrapped = pNewAltWrapped;
	pData->pTabStops = pNewTabStops;
	pData->pDirtyRows = pNewDirtyRows;
	pData->iDirtyRowCount = 0;
	pData->iColumns = iColumns;
	pData->iRows = iRows;
	pData->iMainRowOffset = 0;
	pData->iAltRowOffset = 0;
	__xuiTerminalClearCells(pData, pData->pMain);
	__xuiTerminalClearCells(pData, pData->pAlt);
	__xuiTerminalSetDefaultTabStops(pData->pTabStops, pData->iColumns);
	if ( pOldTabStops != NULL && iOldColumns > 0 ) {
		memcpy(pData->pTabStops, pOldTabStops, (size_t)__xuiTerminalMin(iOldColumns, iColumns));
		xrtFree(pOldTabStops);
	}
	if ( pOldDirtyRows != NULL ) {
		xrtFree(pOldDirtyRows);
	}
	pData->bHistoryViewDirty = 1;
	(void)__xuiTerminalRebuildHistoryView(pData);
	if ( (pOldMain != NULL) && (pOldAlt != NULL) && (iOldColumns > 0) && (iOldRows > 0) ) {
		iDropRows = __xuiTerminalMax(0, tReflow.iRows - iRows);
		for ( y = 0; y < iDropRows; ++y ) {
			(void)__xuiTerminalPushScrollback(pData,
				tReflow.pCells + y * iColumns, iColumns,
				tReflow.pWrapped != NULL ? tReflow.pWrapped[y] != 0u : 0);
		}
		(void)__xuiTerminalRebuildHistoryView(pData);
		iRestoreRows = (iRows > tReflow.iRows - iDropRows) ?
			__xuiTerminalMin(iRows - (tReflow.iRows - iDropRows), pData->iScrollbackCount) : 0;
		for ( y = iRestoreRows - 1; y >= 0; y-- ) {
			__xuiTerminalPopScrollbackRow(pData, __xuiTerminalBufferRow(pData, pData->pMain, y));
		}
		iMainSourceRow = iDropRows;
		iMainTargetRow = iRestoreRows;
		iCopyRows = __xuiTerminalMin(tReflow.iRows - iMainSourceRow, iRows - iMainTargetRow);
		for ( y = 0; y < iCopyRows; y++ ) {
			memcpy(__xuiTerminalBufferRow(pData, pData->pMain, y + iMainTargetRow),
				tReflow.pCells + (y + iMainSourceRow) * iColumns,
				sizeof(*pData->pMain) * (size_t)iColumns);
			pData->pMainWrapped[y + iMainTargetRow] = tReflow.pWrapped[y + iMainSourceRow];
		}
		iCopyColumns = __xuiTerminalMin(iOldColumns, iColumns);
		iCopyRows = __xuiTerminalMin(iOldRows, iRows);
		for ( y = 0; y < iCopyRows; y++ ) {
			const xui_terminal_cell_t* pOldAltRow = pOldAlt +
				((y + iOldAltRowOffset) % iOldRows) * iOldColumns;
			xui_terminal_cell_t* pNewAltRow = __xuiTerminalBufferRow(pData, pData->pAlt, y);
			for ( x = 0; x < iCopyColumns; x++ ) {
				pNewAltRow[x] = pOldAltRow[x];
			}
			if ( pOldAltWrapped != NULL ) pData->pAltWrapped[y] = pOldAltWrapped[y];
		}
		xrtFree(pOldMain);
		xrtFree(pOldAlt);
		xrtFree(pOldMainWrapped);
		xrtFree(pOldAltWrapped);
		pData->iMainCursorX = __xuiTerminalMin(tReflow.iCursorColumn, iColumns - 1);
		pData->iMainCursorY = __xuiTerminalMin(__xuiTerminalMax(0,
			tReflow.iCursorRow - iDropRows + iRestoreRows), iRows - 1);
		if ( pData->bAltScreen ) {
			pData->iCursorX = __xuiTerminalMin(__xuiTerminalMax(0, iOldActiveCursorX), iColumns - 1);
			pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(0, iOldActiveCursorY), iRows - 1);
			pData->iAltCursorX = pData->iCursorX;
			pData->iAltCursorY = pData->iCursorY;
		} else {
			pData->iCursorX = pData->iMainCursorX;
			pData->iCursorY = pData->iMainCursorY;
		}
	}
	__xuiTerminalReflowUnit(&tReflow);
	pData->iCursorX = __xuiTerminalMin(pData->iCursorX, iColumns - 1);
	pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(0, pData->iCursorY), iRows - 1);
	pData->iSavedCursorX = __xuiTerminalMin(pData->iSavedCursorX, iColumns - 1);
	pData->iSavedCursorY = __xuiTerminalMin(pData->iSavedCursorY, iRows - 1);
	pData->bWrapPending = 0;
	pData->iScrollTop = 0;
	pData->iScrollBottom = iRows - 1;
	if ( pData->iSearchLine >= 0 ) {
		if ( pData->iSearchHistoryId != 0u ) (void)__xuiTerminalResolveFindAnchor(pData);
		else __xuiTerminalClearFindMatch(pData);
	}
	__xuiTerminalMarkFullCacheDirty(pData);
	return XUI_OK;
}

static int __xuiTerminalIsAtBottom(xui_terminal_data_t* pData)
{
	float fOffsetY;
	float fMaxY;
	int iOffsetLine;
	int iMaxLine;

	if ( pData == NULL ) return 1;
	fOffsetY = 0.0f;
	fMaxY = 0.0f;
	if ( xuiScrollModelGetOffset(&pData->tScroll, NULL, &fOffsetY) != XUI_OK ) return 1;
	if ( xuiScrollModelGetMaxOffset(&pData->tScroll, NULL, &fMaxY) != XUI_OK ) return 1;
	if ( pData->fCellHeight > 0.0f ) {
		iOffsetLine = (int)(fOffsetY / pData->fCellHeight);
		iMaxLine = (int)(fMaxY / pData->fCellHeight);
		return iOffsetLine >= iMaxLine;
	}
	return fOffsetY >= fMaxY - 1.0f;
}

static int __xuiTerminalScrollToBottom(xui_terminal_data_t* pData)
{
	float fOffsetX;
	float fOffsetY;
	float fMaxY;

	if ( pData == NULL ) return 0;
	fOffsetX = 0.0f;
	fOffsetY = 0.0f;
	fMaxY = 0.0f;
	(void)xuiScrollModelGetOffset(&pData->tScroll, &fOffsetX, &fOffsetY);
	if ( xuiScrollModelGetMaxOffset(&pData->tScroll, NULL, &fMaxY) != XUI_OK ) return 0;
	(void)xuiScrollModelSetOffset(&pData->tScroll, fOffsetX, fMaxY);
	return fOffsetY != fMaxY;
}

static int __xuiTerminalScrollOffsetPixels(xui_terminal_data_t* pData)
{
	float fOffsetY = 0.0f;

	if ( pData == NULL ) return 0;
	(void)xuiScrollModelGetOffset(&pData->tScroll, NULL, &fOffsetY);
	if ( fOffsetY <= 0.0f ) return 0;
	return xuiInternalSnapPixel(fOffsetY);
}

static void __xuiTerminalSyncScrollModel(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_rect_t tViewport;
	float fContentHeight;
	float fVisibleHeight;
	int bFollowBottom;

	if ( (pWidget == NULL) || (pData == NULL) ) {
		return;
	}
	bFollowBottom = __xuiTerminalIsAtBottom(pData);
	(void)__xuiTerminalRebuildHistoryView(pData);
	if ( pData->iSearchHistoryId != 0u ) (void)__xuiTerminalResolveFindAnchor(pData);
	tViewport = xuiWidgetGetContentRect(pWidget);
	xuiScrollModelSetViewport(&pData->tScroll, tViewport);
	fVisibleHeight = (float)pData->iRows * pData->fCellHeight + pData->fPadding * 2.0f;
	if ( fVisibleHeight < tViewport.fH ) {
		fVisibleHeight = tViewport.fH;
	}
	fContentHeight = (float)pData->iScrollbackCount * pData->fCellHeight + fVisibleHeight;
	xuiScrollModelSetContentSize(&pData->tScroll, tViewport.fW, fContentHeight);
	if ( bFollowBottom ) {
		(void)__xuiTerminalScrollToBottom(pData);
	}
}

static void __xuiTerminalNotifyResize(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_terminal_session_t* pSession;

	if ( pData == NULL || pData->iColumns <= 0 || pData->iRows <= 0 ) return;
	if ( pData->onResize != NULL ) {
		pData->onResize(pWidget, pData->iColumns, pData->iRows, pData->pResizeUser);
	}
	pSession = pData->pSession;
	if ( pSession != NULL ) {
		(void)xuiTerminalSessionResize(pSession, pData->iColumns, pData->iRows);
	}
}

static void __xuiTerminalInvalidate(xui_widget pWidget, xui_terminal_data_t* pData, uint32_t iFlags)
{
	if ( pData != NULL ) {
		pData->iChangeCount++;
		if ( (iFlags & XUI_WIDGET_DIRTY_CACHE) != 0u ) {
			__xuiTerminalMarkFullCacheDirty(pData);
		}
	}
	if ( pWidget != NULL ) {
		(void)xuiWidgetInvalidate(pWidget, iFlags);
	}
}

static void __xuiTerminalInvalidateDirty(xui_widget pWidget, xui_terminal_data_t* pData, uint32_t iFlags)
{
	if ( pData != NULL ) {
		pData->iChangeCount++;
	}
	if ( pWidget != NULL ) {
		(void)xuiWidgetInvalidate(pWidget, iFlags);
	}
}

static int __xuiTerminalScrollUp(xui_widget pWidget, xui_terminal_data_t* pData, int iTop, int iBottom, int iCount)
{
	xui_terminal_cell_t* pCells;
	uint8_t* pWrapped;
	int i;
	int iRows;
	int* pRowOffset;

	if ( (pData == NULL) || (iCount <= 0) ) {
		return XUI_OK;
	}
	pCells = __xuiTerminalScreen(pData);
	pWrapped = __xuiTerminalScreenWrapped(pData);
	if ( pCells == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iTop < 0 ) iTop = 0;
	if ( iBottom < 0 || iBottom >= pData->iRows ) iBottom = pData->iRows - 1;
	if ( iBottom < iTop ) return XUI_OK;
	iRows = iBottom - iTop + 1;
	if ( iCount > iRows ) iCount = iRows;
	if ( !pData->bAltScreen && iTop == 0 ) {
		for ( i = 0; i < iCount; i++ ) {
			(void)__xuiTerminalPushScrollback(pData,
				__xuiTerminalBufferRow(pData, pCells, iTop + i), pData->iColumns,
				pWrapped != NULL ? pWrapped[iTop + i] != 0u : 0);
		}
		__xuiTerminalMarkFullCacheDirty(pData);
	}
	if ( iTop == 0 && iBottom == pData->iRows - 1 ) {
		pRowOffset = pData->bAltScreen ? &pData->iAltRowOffset : &pData->iMainRowOffset;
		*pRowOffset = (*pRowOffset + iCount) % pData->iRows;
	} else {
		for ( i = iTop; i <= iBottom - iCount; ++i ) {
			memcpy(__xuiTerminalBufferRow(pData, pCells, i),
				__xuiTerminalBufferRow(pData, pCells, i + iCount),
				sizeof(*pCells) * (size_t)pData->iColumns);
		}
	}
	if ( pWrapped != NULL ) {
		memmove(pWrapped + iTop, pWrapped + iTop + iCount, (size_t)(iRows - iCount));
		memset(pWrapped + iBottom - iCount + 1, 0, (size_t)iCount);
	}
	for ( i = iBottom - iCount + 1; i <= iBottom; i++ ) {
		__xuiTerminalBlankRow(pData, pCells, i);
	}
	__xuiTerminalMarkDirtyRows(pData, iTop, iBottom);
	(void)pWidget;
	return XUI_OK;
}

static void __xuiTerminalLineFeed(xui_widget pWidget, xui_terminal_data_t* pData)
{
	if ( pData == NULL ) {
		return;
	}
	if ( pData->iCursorY >= pData->iScrollBottom ) {
		(void)__xuiTerminalScrollUp(pWidget, pData, pData->iScrollTop, pData->iScrollBottom, 1);
		pData->iCursorY = pData->iScrollBottom;
	} else if ( pData->iCursorY < pData->iRows - 1 ) {
		pData->iCursorY++;
	}
}

static void __xuiTerminalClearWideAt(xui_terminal_data_t* pData, xui_terminal_cell_t* pCells, int iX, int iY)
{
	xui_terminal_cell_t* pRow;

	if ( pData == NULL || pCells == NULL || iX < 0 || iX >= pData->iColumns || iY < 0 || iY >= pData->iRows ) return;
	pRow = __xuiTerminalBufferRow(pData, pCells, iY);
	if ( pRow == NULL ) return;
	if ( (pRow[iX].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u && iX > 0 ) {
		pRow[iX - 1] = __xuiTerminalBlankCell(pData);
	}
	if ( (pRow[iX].iFlags & XUI_TERMINAL_CELL_WIDE) != 0u && iX + 1 < pData->iColumns ) {
		pRow[iX + 1] = __xuiTerminalBlankCell(pData);
	}
}

static void __xuiTerminalPutCodepoint(xui_widget pWidget, xui_terminal_data_t* pData, uint32_t iCodepoint)
{
	xui_terminal_cell_t* pCells;
	xui_terminal_cell_t* pRow;
	uint8_t* pWrapped;
	xui_terminal_cell_t tCell;
	int iWidth;
	int iIndex;

	if ( (pData == NULL) || (pData->iColumns <= 0) || (pData->iRows <= 0) ) {
		return;
	}
	pCells = __xuiTerminalScreen(pData);
	pWrapped = __xuiTerminalScreenWrapped(pData);
	if ( pCells == NULL ) {
		return;
	}
	iWidth = __xuiTerminalCodepointWidth(iCodepoint);
	if ( iWidth == 0 ) {
		int iBaseX = pData->bWrapPending ? pData->iCursorX : pData->iCursorX - 1;
		int iBaseY = pData->iCursorY;
		if ( iBaseX < 0 && iBaseY > 0 ) {
			iBaseY--;
			iBaseX = pData->iColumns - 1;
		}
		if ( iBaseX >= 0 ) {
			pRow = __xuiTerminalBufferRow(pData, pCells, iBaseY);
			iIndex = iBaseX;
			if ( pRow != NULL && (pRow[iIndex].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u && iBaseX > 0 ) iIndex--;
			if ( pRow != NULL && pRow[iIndex].iCodepoint != 0u && pRow[iIndex].iCodepoint != ' ' ) {
				if ( pRow[iIndex].iCombiningCount < XUI_TERMINAL_MAX_COMBINING ) {
					pRow[iIndex].arrCombining[pRow[iIndex].iCombiningCount++] = iCodepoint;
					pRow[iIndex].iFlags |= XUI_TERMINAL_CELL_COMBINING;
				} else {
					pRow[iIndex].iFlags |= XUI_TERMINAL_CELL_COMBINING_OVERFLOW;
				}
				__xuiTerminalMarkDirtyRow(pData, iBaseY);
				return;
			}
		}
		iWidth = 1;
	}
	if ( iWidth > pData->iColumns ) {
		iWidth = 1;
	}
	if ( pData->bWrapPending ) {
		pData->bWrapPending = 0;
		if ( pData->bAutoWrap ) {
			if ( pWrapped != NULL ) pWrapped[pData->iCursorY] = 1u;
			pData->iCursorX = 0;
			__xuiTerminalLineFeed(pWidget, pData);
		}
	}
	if ( (pData->iCursorX + iWidth) > pData->iColumns ) {
		if ( pData->bAutoWrap ) {
			if ( pWrapped != NULL ) pWrapped[pData->iCursorY] = 1u;
			pData->iCursorX = 0;
			__xuiTerminalLineFeed(pWidget, pData);
		} else {
			pData->iCursorX = __xuiTerminalMax(0, pData->iColumns - iWidth);
		}
	}
	tCell = __xuiTerminalBlankCell(pData);
	tCell.iCodepoint = iCodepoint;
	tCell.iFgColor = pData->iCurrentFg;
	tCell.iBgColor = pData->iCurrentBg;
	tCell.iFlags = pData->iCurrentFlags;
	tCell.iWidth = (uint8_t)iWidth;
	tCell.iLinkId = pData->iCurrentLinkId;
	pData->iLastCodepoint = iCodepoint;
	if ( iWidth == 2 ) {
		tCell.iFlags |= XUI_TERMINAL_CELL_WIDE;
	}
	if ( pData->bInsertMode ) __xuiTerminalInsertCharacters(pData, iWidth);
	__xuiTerminalClearWideAt(pData, pCells, pData->iCursorX, pData->iCursorY);
	if ( iWidth == 2 ) __xuiTerminalClearWideAt(pData, pCells, pData->iCursorX + 1, pData->iCursorY);
	pRow = __xuiTerminalBufferRow(pData, pCells, pData->iCursorY);
	if ( pRow == NULL ) return;
	iIndex = pData->iCursorX;
	pRow[iIndex] = tCell;
	if ( iWidth == 2 && pData->iCursorX + 1 < pData->iColumns ) {
		pRow[iIndex + 1] = tCell;
		pRow[iIndex + 1].iCodepoint = 0u;
		pRow[iIndex + 1].iFlags &= ~XUI_TERMINAL_CELL_WIDE;
		pRow[iIndex + 1].iFlags |= XUI_TERMINAL_CELL_WIDE_CONT;
		pRow[iIndex + 1].iWidth = 0;
		pRow[iIndex + 1].iCombiningCount = 0;
	}
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
	if ( pData->iCursorX + iWidth >= pData->iColumns ) {
		pData->iCursorX = pData->iColumns - 1;
		pData->bWrapPending = pData->bAutoWrap ? 1 : 0;
	} else {
		pData->iCursorX += iWidth;
	}
}

static void __xuiTerminalEraseLine(xui_terminal_data_t* pData, int iMode)
{
	xui_terminal_cell_t* pCells;
	xui_terminal_cell_t* pRow;
	xui_terminal_cell_t tBlank;
	int iStart;
	int iEnd;
	int i;

	if ( pData == NULL ) return;
	pCells = __xuiTerminalScreen(pData);
	if ( pCells == NULL ) return;
	pRow = __xuiTerminalBufferRow(pData, pCells, pData->iCursorY);
	if ( pRow == NULL ) return;
	tBlank = __xuiTerminalBlankCell(pData);
	iStart = 0;
	iEnd = pData->iColumns - 1;
	if ( iMode == 0 ) {
		iStart = pData->iCursorX;
	} else if ( iMode == 1 ) {
		iEnd = pData->iCursorX;
	}
	for ( i = iStart; i <= iEnd; i++ ) {
		pRow[i] = tBlank;
	}
	if ( iStart == 0 && iEnd == pData->iColumns - 1 ) {
		uint8_t* pWrapped = __xuiTerminalScreenWrapped(pData);
		if ( pWrapped != NULL ) pWrapped[pData->iCursorY] = 0u;
	}
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
}

static void __xuiTerminalEraseDisplay(xui_terminal_data_t* pData, int iMode)
{
	xui_terminal_cell_t* pCells;
	xui_terminal_cell_t tBlank;
	uint8_t* pWrapped;
	int iStartRow;
	int iEndRow;
	int x;
	int y;

	if ( pData == NULL ) return;
	pCells = __xuiTerminalScreen(pData);
	if ( pCells == NULL ) return;
	tBlank = __xuiTerminalBlankCell(pData);
	iStartRow = iMode == 0 ? pData->iCursorY : 0;
	iEndRow = iMode == 1 ? pData->iCursorY : pData->iRows - 1;
	pWrapped = __xuiTerminalScreenWrapped(pData);
	for ( y = iStartRow; y <= iEndRow; ++y ) {
		xui_terminal_cell_t* pRow = __xuiTerminalBufferRow(pData, pCells, y);
		int iStartColumn = (iMode == 0 && y == pData->iCursorY) ? pData->iCursorX : 0;
		int iEndColumn = (iMode == 1 && y == pData->iCursorY) ? pData->iCursorX : pData->iColumns - 1;
		if ( pRow == NULL ) continue;
		for ( x = iStartColumn; x <= iEndColumn; ++x ) pRow[x] = tBlank;
		if ( pWrapped != NULL && iStartColumn == 0 && iEndColumn == pData->iColumns - 1 ) pWrapped[y] = 0u;
	}
	__xuiTerminalMarkDirtyRows(pData, iStartRow, iEndRow);
}

static void __xuiTerminalSaveCursorState(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) return;
	pData->iSavedCursorX = pData->iCursorX;
	pData->iSavedCursorY = pData->iCursorY;
	pData->iSavedFg = pData->iCurrentFg;
	pData->iSavedBg = pData->iCurrentBg;
	pData->iSavedFlags = pData->iCurrentFlags;
	pData->iSavedLinkId = pData->iCurrentLinkId;
}

static void __xuiTerminalRestoreCursorState(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) return;
	pData->iCursorX = __xuiTerminalMin(__xuiTerminalMax(0, pData->iSavedCursorX), pData->iColumns - 1);
	pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(0, pData->iSavedCursorY), pData->iRows - 1);
	pData->iCurrentFg = pData->iSavedFg != 0u ? pData->iSavedFg : pData->iForegroundColor;
	pData->iCurrentBg = pData->iSavedBg != 0u ? pData->iSavedBg : pData->iBackgroundColor;
	pData->iCurrentFlags = pData->iSavedFlags;
	pData->iCurrentLinkId = pData->iSavedLinkId;
	pData->bWrapPending = 0;
}

static void __xuiTerminalScrollDown(xui_terminal_data_t* pData, int iTop, int iBottom, int iCount)
{
	xui_terminal_cell_t* pCells;
	uint8_t* pWrapped;
	int iRows;
	int i;
	int* pRowOffset;
	if ( pData == NULL || iCount <= 0 ) return;
	pCells = __xuiTerminalScreen(pData);
	pWrapped = __xuiTerminalScreenWrapped(pData);
	if ( pCells == NULL ) return;
	if ( iTop < 0 ) iTop = 0;
	if ( iBottom < 0 || iBottom >= pData->iRows ) iBottom = pData->iRows - 1;
	if ( iBottom < iTop ) return;
	iRows = iBottom - iTop + 1;
	if ( iCount > iRows ) iCount = iRows;
	if ( iTop == 0 && iBottom == pData->iRows - 1 ) {
		pRowOffset = pData->bAltScreen ? &pData->iAltRowOffset : &pData->iMainRowOffset;
		*pRowOffset = (*pRowOffset - iCount + pData->iRows) % pData->iRows;
	} else {
		for ( i = iBottom; i >= iTop + iCount; --i ) {
			memcpy(__xuiTerminalBufferRow(pData, pCells, i),
				__xuiTerminalBufferRow(pData, pCells, i - iCount),
				sizeof(*pCells) * (size_t)pData->iColumns);
		}
	}
	if ( pWrapped != NULL ) {
		memmove(pWrapped + iTop + iCount, pWrapped + iTop, (size_t)(iRows - iCount));
		memset(pWrapped + iTop, 0, (size_t)iCount);
	}
	for ( i = iTop; i < iTop + iCount; ++i ) __xuiTerminalBlankRow(pData, pCells, i);
	__xuiTerminalMarkDirtyRows(pData, iTop, iBottom);
}

static void __xuiTerminalInsertCharacters(xui_terminal_data_t* pData, int iCount)
{
	xui_terminal_cell_t* pRow;
	xui_terminal_cell_t tBlank;
	int iAvailable;
	int i;
	if ( pData == NULL || iCount <= 0 ) return;
	pRow = __xuiTerminalScreen(pData);
	if ( pRow == NULL ) return;
	pRow = __xuiTerminalBufferRow(pData, pRow, pData->iCursorY);
	iAvailable = pData->iColumns - pData->iCursorX;
	if ( iCount > iAvailable ) iCount = iAvailable;
	__xuiTerminalClearWideAt(pData, __xuiTerminalScreen(pData), pData->iCursorX, pData->iCursorY);
	memmove(pRow + pData->iCursorX + iCount, pRow + pData->iCursorX,
		sizeof(*pRow) * (size_t)(iAvailable - iCount));
	tBlank = __xuiTerminalEraseCell(pData);
	for ( i = 0; i < iCount; ++i ) pRow[pData->iCursorX + i] = tBlank;
	__xuiTerminalClearWideAt(pData, __xuiTerminalScreen(pData), pData->iColumns - 1, pData->iCursorY);
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
}

static void __xuiTerminalDeleteCharacters(xui_terminal_data_t* pData, int iCount)
{
	xui_terminal_cell_t* pRow;
	xui_terminal_cell_t tBlank;
	int iAvailable;
	int i;
	if ( pData == NULL || iCount <= 0 ) return;
	pRow = __xuiTerminalScreen(pData);
	if ( pRow == NULL ) return;
	pRow = __xuiTerminalBufferRow(pData, pRow, pData->iCursorY);
	iAvailable = pData->iColumns - pData->iCursorX;
	if ( iCount > iAvailable ) iCount = iAvailable;
	__xuiTerminalClearWideAt(pData, __xuiTerminalScreen(pData), pData->iCursorX, pData->iCursorY);
	memmove(pRow + pData->iCursorX, pRow + pData->iCursorX + iCount,
		sizeof(*pRow) * (size_t)(iAvailable - iCount));
	tBlank = __xuiTerminalEraseCell(pData);
	for ( i = pData->iColumns - iCount; i < pData->iColumns; ++i ) pRow[i] = tBlank;
	__xuiTerminalClearWideAt(pData, __xuiTerminalScreen(pData), pData->iCursorX, pData->iCursorY);
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
}

static void __xuiTerminalEraseCharacters(xui_terminal_data_t* pData, int iCount)
{
	xui_terminal_cell_t* pRow;
	xui_terminal_cell_t tBlank;
	int iEnd;
	int i;
	if ( pData == NULL || iCount <= 0 ) return;
	pRow = __xuiTerminalScreen(pData);
	if ( pRow == NULL ) return;
	pRow = __xuiTerminalBufferRow(pData, pRow, pData->iCursorY);
	iEnd = __xuiTerminalMin(pData->iColumns, pData->iCursorX + iCount);
	tBlank = __xuiTerminalEraseCell(pData);
	for ( i = pData->iCursorX; i < iEnd; ++i ) pRow[i] = tBlank;
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
}

static void __xuiTerminalDeleteLines(xui_terminal_data_t* pData, int iCount)
{
	xui_terminal_cell_t* pCells;
	uint8_t* pWrapped;
	int iBottom;
	int iRows;
	int i;
	if ( pData == NULL || iCount <= 0 || pData->iCursorY < pData->iScrollTop || pData->iCursorY > pData->iScrollBottom ) return;
	pCells = __xuiTerminalScreen(pData);
	pWrapped = __xuiTerminalScreenWrapped(pData);
	iBottom = pData->iScrollBottom;
	iRows = iBottom - pData->iCursorY + 1;
	if ( iCount > iRows ) iCount = iRows;
	for ( i = pData->iCursorY; i <= iBottom - iCount; ++i ) {
		memcpy(__xuiTerminalBufferRow(pData, pCells, i),
			__xuiTerminalBufferRow(pData, pCells, i + iCount),
			sizeof(*pCells) * (size_t)pData->iColumns);
	}
	if ( pWrapped != NULL ) {
		memmove(pWrapped + pData->iCursorY, pWrapped + pData->iCursorY + iCount, (size_t)(iRows - iCount));
		memset(pWrapped + iBottom - iCount + 1, 0, (size_t)iCount);
	}
	for ( i = iBottom - iCount + 1; i <= iBottom; ++i ) __xuiTerminalBlankRow(pData, pCells, i);
	__xuiTerminalMarkDirtyRows(pData, pData->iCursorY, iBottom);
}

static int __xuiTerminalParam(const xui_terminal_parser_t* pParser, int iIndex, int iDefault)
{
	if ( (pParser == NULL) || (iIndex < 0) || (iIndex >= pParser->iParamCount) || (pParser->arrParams[iIndex] < 0) ) {
		return iDefault;
	}
	return pParser->arrParams[iIndex];
}

static void __xuiTerminalCsiReset(xui_terminal_parser_t* pParser)
{
	int i;

	if ( pParser == NULL ) return;
	for ( i = 0; i < XUI_TERMINAL_MAX_CSI_PARAMS; i++ ) {
		pParser->arrParams[i] = -1;
	}
	pParser->iParamCount = 0;
	pParser->iCurrentParam = -1;
	pParser->bParamActive = 0;
	pParser->cPrivate = 0;
	pParser->iIntermediateCount = 0;
	memset(pParser->arrIntermediates, 0, sizeof(pParser->arrIntermediates));
}

static void __xuiTerminalCsiAppendDigit(xui_terminal_parser_t* pParser, int iDigit)
{
	if ( pParser == NULL ) return;
	if ( pParser->iParamCount <= 0 ) {
		pParser->iParamCount = 1;
		pParser->iCurrentParam = 0;
		pParser->arrParams[0] = 0;
	}
	if ( !pParser->bParamActive || pParser->arrParams[pParser->iCurrentParam] < 0 ) {
		pParser->arrParams[pParser->iCurrentParam] = 0;
	}
	if ( pParser->arrParams[pParser->iCurrentParam] > (INT_MAX - iDigit) / 10 ) {
		pParser->arrParams[pParser->iCurrentParam] = INT_MAX;
	} else {
		pParser->arrParams[pParser->iCurrentParam] = pParser->arrParams[pParser->iCurrentParam] * 10 + iDigit;
	}
	pParser->bParamActive = 1;
}

static void __xuiTerminalCsiNextParam(xui_terminal_parser_t* pParser)
{
	if ( pParser == NULL ) return;
	if ( pParser->iParamCount <= 0 ) {
		pParser->iParamCount = 1;
		pParser->iCurrentParam = 0;
	}
	if ( pParser->iParamCount < XUI_TERMINAL_MAX_CSI_PARAMS ) {
		pParser->iCurrentParam = pParser->iParamCount;
		pParser->arrParams[pParser->iCurrentParam] = -1;
		pParser->iParamCount++;
	}
	pParser->bParamActive = 0;
}

static void __xuiTerminalApplySgr(xui_terminal_data_t* pData, const xui_terminal_parser_t* pParser)
{
	int i;
	int p;
	int r;
	int g;
	int b;
	int count;

	if ( (pData == NULL) || (pParser == NULL) ) return;
	count = pParser->iParamCount;
	if ( count <= 0 ) {
		count = 1;
	}
	for ( i = 0; i < count; i++ ) {
		p = __xuiTerminalParam(pParser, i, 0);
		if ( p == 0 ) {
			pData->iCurrentFg = pData->iForegroundColor;
			pData->iCurrentBg = pData->iBackgroundColor;
			pData->iCurrentFlags = 0;
		} else if ( p == 1 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_BOLD;
		} else if ( p == 2 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_DIM;
		} else if ( p == 3 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_ITALIC;
		} else if ( p == 4 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_UNDERLINE;
		} else if ( p == 5 || p == 6 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_BLINK;
		} else if ( p == 7 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_INVERSE;
		} else if ( p == 9 ) {
			pData->iCurrentFlags |= XUI_TERMINAL_CELL_STRIKE;
		} else if ( p == 22 ) {
			pData->iCurrentFlags &= ~(XUI_TERMINAL_CELL_BOLD | XUI_TERMINAL_CELL_DIM);
		} else if ( p == 24 ) {
			pData->iCurrentFlags &= ~XUI_TERMINAL_CELL_UNDERLINE;
		} else if ( p == 23 ) {
			pData->iCurrentFlags &= ~XUI_TERMINAL_CELL_ITALIC;
		} else if ( p == 25 ) {
			pData->iCurrentFlags &= ~XUI_TERMINAL_CELL_BLINK;
		} else if ( p == 27 ) {
			pData->iCurrentFlags &= ~XUI_TERMINAL_CELL_INVERSE;
		} else if ( p == 29 ) {
			pData->iCurrentFlags &= ~XUI_TERMINAL_CELL_STRIKE;
		} else if ( p >= 30 && p <= 37 ) {
			pData->iCurrentFg = pData->arrPalette[p - 30];
		} else if ( p == 39 ) {
			pData->iCurrentFg = pData->iForegroundColor;
		} else if ( p >= 40 && p <= 47 ) {
			pData->iCurrentBg = pData->arrPalette[p - 40];
		} else if ( p == 49 ) {
			pData->iCurrentBg = pData->iBackgroundColor;
		} else if ( p >= 90 && p <= 97 ) {
			pData->iCurrentFg = pData->arrPalette[8 + p - 90];
		} else if ( p >= 100 && p <= 107 ) {
			pData->iCurrentBg = pData->arrPalette[8 + p - 100];
		} else if ( (p == 38 || p == 48) && (i + 2 < count) && (__xuiTerminalParam(pParser, i + 1, 0) == 5) ) {
			int idx = __xuiTerminalParam(pParser, i + 2, 0);
			if ( idx < 0 ) idx = 0;
			if ( idx > 255 ) idx = 255;
			if ( p == 38 ) pData->iCurrentFg = pData->arrPalette[idx];
			else pData->iCurrentBg = pData->arrPalette[idx];
			i += 2;
		} else if ( (p == 38 || p == 48) && (i + 4 < count) && (__xuiTerminalParam(pParser, i + 1, 0) == 2) ) {
			r = __xuiTerminalParam(pParser, i + 2, 0);
			g = __xuiTerminalParam(pParser, i + 3, 0);
			b = __xuiTerminalParam(pParser, i + 4, 0);
			if ( r < 0 ) r = 0;
			if ( r > 255 ) r = 255;
			if ( g < 0 ) g = 0;
			if ( g > 255 ) g = 255;
			if ( b < 0 ) b = 0;
			if ( b > 255 ) b = 255;
			if ( p == 38 ) pData->iCurrentFg = XUI_COLOR_RGBA(r, g, b, 255);
			else pData->iCurrentBg = XUI_COLOR_RGBA(r, g, b, 255);
			i += 4;
		}
	}
}

static void __xuiTerminalSetAltScreen(xui_terminal_data_t* pData, int bEnabled, int bClear)
{
	if ( pData == NULL || pData->bAltScreen == bEnabled ) {
		return;
	}
	if ( bEnabled ) {
		pData->iMainCursorX = pData->iCursorX;
		pData->iMainCursorY = pData->iCursorY;
		pData->bAltScreen = 1;
		if ( bClear ) {
			__xuiTerminalClearCells(pData, pData->pAlt);
			pData->iAltCursorX = 0;
			pData->iAltCursorY = 0;
		}
		pData->iCursorX = __xuiTerminalMin(__xuiTerminalMax(0, pData->iAltCursorX), pData->iColumns - 1);
		pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(0, pData->iAltCursorY), pData->iRows - 1);
	} else {
		pData->iAltCursorX = pData->iCursorX;
		pData->iAltCursorY = pData->iCursorY;
		pData->bAltScreen = 0;
		pData->iCursorX = __xuiTerminalMin(__xuiTerminalMax(0, pData->iMainCursorX), pData->iColumns - 1);
		pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(0, pData->iMainCursorY), pData->iRows - 1);
	}
	pData->bWrapPending = 0;
	pData->iScrollTop = 0;
	pData->iScrollBottom = pData->iRows - 1;
	__xuiTerminalMarkFullCacheDirty(pData);
}

static void __xuiTerminalDispatchCsi(xui_widget pWidget, xui_terminal_data_t* pData, int iFinal)
{
	char sResponse[64];
	int n;
	int row;
	int col;
	int mode;
	int i;
	int iTop;
	int iBottom;
	int iSize;

	if ( (pData == NULL) || (pData->iColumns <= 0) || (pData->iRows <= 0) ) return;
	iTop = pData->bOriginMode ? pData->iScrollTop : 0;
	iBottom = pData->bOriginMode ? pData->iScrollBottom : pData->iRows - 1;
	switch ( iFinal ) {
	case 'A':
		pData->bWrapPending = 0;
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		pData->iCursorY = __xuiTerminalMax(iTop, pData->iCursorY - n);
		break;
	case 'B':
		pData->bWrapPending = 0;
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		pData->iCursorY = __xuiTerminalMin(iBottom, pData->iCursorY + n);
		break;
	case 'C':
	case 'a':
		pData->bWrapPending = 0;
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		pData->iCursorX = __xuiTerminalMin(pData->iColumns - 1, pData->iCursorX + n);
		break;
	case 'D':
		pData->bWrapPending = 0;
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		pData->iCursorX = __xuiTerminalMax(0, pData->iCursorX - n);
		break;
	case 'E':
	case 'F':
		pData->bWrapPending = 0;
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		pData->iCursorY = (iFinal == 'E') ? __xuiTerminalMin(iBottom, pData->iCursorY + n) :
			__xuiTerminalMax(iTop, pData->iCursorY - n);
		pData->iCursorX = 0;
		break;
	case 'G':
	case '`':
		pData->bWrapPending = 0;
		col = __xuiTerminalParam(&pData->tParser, 0, 1) - 1;
		pData->iCursorX = __xuiTerminalMin(__xuiTerminalMax(0, col), pData->iColumns - 1);
		break;
	case 'd':
		pData->bWrapPending = 0;
		row = __xuiTerminalParam(&pData->tParser, 0, 1) - 1;
		if ( pData->bOriginMode ) row += pData->iScrollTop;
		pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(iTop, row), iBottom);
		break;
	case 'H':
	case 'f':
		pData->bWrapPending = 0;
		row = __xuiTerminalParam(&pData->tParser, 0, 1) - 1;
		col = __xuiTerminalParam(&pData->tParser, 1, 1) - 1;
		if ( pData->bOriginMode ) row += pData->iScrollTop;
		if ( row < 0 ) row = 0;
		if ( col < 0 ) col = 0;
		pData->iCursorY = __xuiTerminalMin(__xuiTerminalMax(iTop, row), iBottom);
		pData->iCursorX = __xuiTerminalMin(col, pData->iColumns - 1);
		break;
	case 'I':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		while ( n-- > 0 ) pData->iCursorX = __xuiTerminalNextTabStop(pData);
		pData->bWrapPending = 0;
		break;
	case 'Z':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		while ( n-- > 0 ) {
			for ( i = pData->iCursorX - 1; i > 0; --i ) {
				if ( pData->pTabStops != NULL && pData->pTabStops[i] != 0u ) break;
			}
			pData->iCursorX = __xuiTerminalMax(0, i);
		}
		pData->bWrapPending = 0;
		break;
	case 'J':
		pData->bWrapPending = 0;
		mode = __xuiTerminalParam(&pData->tParser, 0, 0);
		if ( mode == 3 && !pData->bAltScreen ) {
			__xuiTerminalFreeScrollback(pData);
			(void)__xuiTerminalAllocScrollback(pData, pData->iScrollbackLimit);
		} else {
			__xuiTerminalEraseDisplay(pData, mode);
		}
		break;
	case 'K':
		pData->bWrapPending = 0;
		__xuiTerminalEraseLine(pData, __xuiTerminalParam(&pData->tParser, 0, 0));
		break;
	case '@':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalInsertCharacters(pData, n > 0 ? n : 1);
		break;
	case 'P':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalDeleteCharacters(pData, n > 0 ? n : 1);
		break;
	case 'X':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalEraseCharacters(pData, n > 0 ? n : 1);
		break;
	case 'L':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalScrollDown(pData, pData->iCursorY, pData->iScrollBottom, n > 0 ? n : 1);
		break;
	case 'M':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalDeleteLines(pData, n > 0 ? n : 1);
		break;
	case 'S':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		(void)__xuiTerminalScrollUp(pWidget, pData, pData->iScrollTop, pData->iScrollBottom, n > 0 ? n : 1);
		break;
	case 'T':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		__xuiTerminalScrollDown(pData, pData->iScrollTop, pData->iScrollBottom, n > 0 ? n : 1);
		break;
	case 'b':
		n = __xuiTerminalParam(&pData->tParser, 0, 1);
		if ( n <= 0 ) n = 1;
		if ( pData->iLastCodepoint != 0u ) while ( n-- > 0 ) __xuiTerminalPutCodepoint(pWidget, pData, pData->iLastCodepoint);
		break;
	case 'g':
		pData->bWrapPending = 0;
		mode = __xuiTerminalParam(&pData->tParser, 0, 0);
		if ( mode == 0 ) {
			__xuiTerminalSetTabStop(pData, pData->iCursorX, 0);
		} else if ( mode == 3 ) {
			__xuiTerminalClearAllTabStops(pData);
		}
		break;
	case 'm':
		__xuiTerminalApplySgr(pData, &pData->tParser);
		break;
	case 'r':
		pData->bWrapPending = 0;
		row = __xuiTerminalParam(&pData->tParser, 0, 1) - 1;
		col = __xuiTerminalParam(&pData->tParser, 1, pData->iRows) - 1;
		if ( row < 0 ) row = 0;
		if ( col < row || col >= pData->iRows ) col = pData->iRows - 1;
		pData->iScrollTop = row;
		pData->iScrollBottom = col;
		pData->iCursorX = 0;
		pData->iCursorY = pData->bOriginMode ? row : 0;
		break;
	case 's':
		pData->bWrapPending = 0;
		__xuiTerminalSaveCursorState(pData);
		break;
	case 'u':
		__xuiTerminalRestoreCursorState(pData);
		break;
	case 'n':
		mode = __xuiTerminalParam(&pData->tParser, 0, 0);
		if ( mode == 5 ) {
			(void)__xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)"\x1b[0n", 4);
		} else if ( mode == 6 ) {
			row = pData->iCursorY + 1;
			if ( pData->bOriginMode ) row -= pData->iScrollTop;
			iSize = snprintf(sResponse, sizeof(sResponse), "\x1b[%d;%dR", row, pData->iCursorX + 1);
			if ( iSize > 0 ) (void)__xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sResponse, iSize);
		}
		break;
	case 'c':
		if ( pData->tParser.cPrivate == '>' ) {
			(void)__xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)"\x1b[>0;1;0c", 10);
		} else {
			(void)__xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)"\x1b[?1;2c", 7);
		}
		break;
	case 'q':
		if ( pData->tParser.iIntermediateCount > 0 && pData->tParser.arrIntermediates[0] == ' ' ) {
			mode = __xuiTerminalParam(&pData->tParser, 0, 0);
			pData->iCursorStyle = __xuiTerminalMin(__xuiTerminalMax(0, mode), 6);
		}
		break;
	case 'h':
	case 'l':
		if ( pData->tParser.cPrivate == '?' ) {
			for ( i = 0; i < pData->tParser.iParamCount; i++ ) {
				mode = __xuiTerminalParam(&pData->tParser, i, 0);
				if ( mode == 1 ) {
					pData->bApplicationCursor = (iFinal == 'h');
				} else if ( mode == 6 ) {
					pData->bOriginMode = (iFinal == 'h');
					pData->iCursorX = 0;
					pData->iCursorY = pData->bOriginMode ? pData->iScrollTop : 0;
				} else if ( mode == 7 ) {
					pData->bAutoWrap = (iFinal == 'h');
					pData->bWrapPending = 0;
				} else if ( mode == 25 ) {
					pData->bCursorVisible = (iFinal == 'h');
				} else if ( mode == 47 ) {
					__xuiTerminalSetAltScreen(pData, iFinal == 'h', 0);
				} else if ( mode == 1000 || mode == 1002 || mode == 1003 ) {
					if ( iFinal == 'h' ) pData->iMouseTracking = mode;
					else if ( pData->iMouseTracking == mode ) pData->iMouseTracking = XUI_TERMINAL_MOUSE_TRACKING_NONE;
				} else if ( mode == 1004 ) {
					pData->bFocusReporting = (iFinal == 'h');
				} else if ( mode == 1005 || mode == 1006 || mode == 1015 ) {
					if ( iFinal == 'h' ) pData->iMouseEncoding = mode;
					else if ( pData->iMouseEncoding == mode ) pData->iMouseEncoding = XUI_TERMINAL_MOUSE_ENCODING_X10;
				} else if ( mode == 1047 ) {
					__xuiTerminalSetAltScreen(pData, iFinal == 'h', iFinal == 'h');
				} else if ( mode == 1048 ) {
					if ( iFinal == 'h' ) __xuiTerminalSaveCursorState(pData);
					else __xuiTerminalRestoreCursorState(pData);
				} else if ( mode == 1049 ) {
					if ( iFinal == 'h' ) {
						__xuiTerminalSaveCursorState(pData);
						__xuiTerminalSetAltScreen(pData, 1, 1);
					} else {
						__xuiTerminalSetAltScreen(pData, 0, 0);
						__xuiTerminalRestoreCursorState(pData);
					}
				} else if ( mode == 2004 ) {
					pData->bBracketedPaste = (iFinal == 'h');
				}
			}
		} else {
			for ( i = 0; i < pData->tParser.iParamCount; ++i ) {
				mode = __xuiTerminalParam(&pData->tParser, i, 0);
				if ( mode == 4 ) pData->bInsertMode = (iFinal == 'h');
			}
		}
		break;
	default:
		(void)pWidget;
		break;
	}
}

static void __xuiTerminalDispatchOsc8(xui_terminal_data_t* pData, const char* sPayload)
{
	const char* sUrl;
	uint16_t iLinkId;

	if ( pData == NULL || sPayload == NULL ) {
		return;
	}
	if ( sPayload[0] != '8' || sPayload[1] != ';' ) {
		return;
	}
	sUrl = strchr(sPayload + 2, ';');
	if ( sUrl == NULL ) {
		return;
	}
	sUrl++;
	if ( sUrl[0] == '\0' ) {
		pData->iCurrentLinkId = 0;
		return;
	}
	iLinkId = __xuiTerminalGetOrAddLink(pData, sUrl);
	if ( iLinkId != 0 ) {
		pData->iCurrentLinkId = iLinkId;
	}
}

static void __xuiTerminalDispatchOsc(xui_widget pWidget, xui_terminal_data_t* pData)
{
	char* sSemi;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->tParser.iOscSize <= 0) ) {
		return;
	}
	pData->tParser.sOsc[pData->tParser.iOscSize] = '\0';
	__xuiTerminalDispatchOsc8(pData, pData->tParser.sOsc);
	sSemi = strchr(pData->tParser.sOsc, ';');
	if ( (sSemi != NULL) && (pData->onTitle != NULL) &&
	     ((pData->tParser.sOsc[0] == '0') || (pData->tParser.sOsc[0] == '2')) ) {
		pData->onTitle(pWidget, sSemi + 1, pData->pTitleUser);
	}
}

static uint32_t __xuiTerminalDecSpecialCodepoint(uint8_t b)
{
	switch ( b ) {
	case '`': return 0x25c6u;
	case 'a': return 0x2592u;
	case 'f': return 0x00b0u;
	case 'g': return 0x00b1u;
	case 'j': return 0x2518u;
	case 'k': return 0x2510u;
	case 'l': return 0x250cu;
	case 'm': return 0x2514u;
	case 'n': return 0x253cu;
	case 'q': return 0x2500u;
	case 't': return 0x251cu;
	case 'u': return 0x2524u;
	case 'v': return 0x2534u;
	case 'w': return 0x252cu;
	case 'x': return 0x2502u;
	case 'y': return 0x2264u;
	case 'z': return 0x2265u;
	case '{': return 0x03c0u;
	case '|': return 0x2260u;
	case '}': return 0x00a3u;
	case '~': return 0x00b7u;
	default: return (uint32_t)b;
	}
}

static uint32_t __xuiTerminalMapGraphicCodepoint(const xui_terminal_data_t* pData, uint8_t b)
{
	int iCharset;
	if ( pData == NULL || b < 0x20u || b > 0x7eu ) return (uint32_t)b;
	iCharset = pData->iActiveCharset != 0 ? pData->iG1Charset : pData->iG0Charset;
	return iCharset != 0 ? __xuiTerminalDecSpecialCodepoint(b) : (uint32_t)b;
}

static void __xuiTerminalProcessByte(xui_widget pWidget, xui_terminal_data_t* pData, uint8_t b)
{
	uint32_t cp;

	if ( pData == NULL ) return;
	if ( pData->tParser.iUtfExpected > 0 ) {
		if ( (b & 0xc0u) == 0x80u ) {
			pData->tParser.iUtfCodepoint = (pData->tParser.iUtfCodepoint << 6) | (uint32_t)(b & 0x3fu);
			pData->tParser.iUtfSeen++;
			if ( pData->tParser.iUtfSeen >= pData->tParser.iUtfExpected ) {
				cp = pData->tParser.iUtfCodepoint;
				pData->tParser.iUtfExpected = 0;
				pData->tParser.iUtfSeen = 0;
				pData->tParser.iUtfCodepoint = 0;
				__xuiTerminalPutCodepoint(pWidget, pData, cp);
			}
			return;
		}
		pData->tParser.iUtfExpected = 0;
		pData->tParser.iUtfSeen = 0;
		pData->tParser.iUtfCodepoint = 0;
	}
	switch ( pData->tParser.iState ) {
	case 0:
		if ( b == 0x9bu ) {
			__xuiTerminalCsiReset(&pData->tParser);
			pData->tParser.iState = 2;
		} else if ( b == 0x9du ) {
			pData->tParser.iOscSize = 0;
			pData->tParser.iState = 3;
		} else if ( b == 0x90u || b == 0x98u || b == 0x9eu || b == 0x9fu ) {
			pData->tParser.iState = 5;
		} else if ( b == 0x1bu ) {
			pData->tParser.iState = 1;
		} else if ( b == '\a' ) {
			return;
		} else if ( b == '\b' ) {
			pData->bWrapPending = 0;
			if ( pData->iCursorX > 0 ) pData->iCursorX--;
		} else if ( b == '\t' ) {
			pData->bWrapPending = 0;
			pData->iCursorX = __xuiTerminalNextTabStop(pData);
		} else if ( b == '\n' ) {
			pData->bWrapPending = 0;
			__xuiTerminalLineFeed(pWidget, pData);
		} else if ( b == '\r' ) {
			pData->bWrapPending = 0;
			pData->iCursorX = 0;
		} else if ( b == 0x0eu ) {
			pData->iActiveCharset = 1;
		} else if ( b == 0x0fu ) {
			pData->iActiveCharset = 0;
		} else if ( b < 0x20u ) {
			return;
		} else if ( b < 0x80u ) {
			__xuiTerminalPutCodepoint(pWidget, pData,
				__xuiTerminalMapGraphicCodepoint(pData, b));
		} else if ( (b & 0xe0u) == 0xc0u ) {
			pData->tParser.iUtfExpected = 1;
			pData->tParser.iUtfSeen = 0;
			pData->tParser.iUtfCodepoint = (uint32_t)(b & 0x1fu);
		} else if ( (b & 0xf0u) == 0xe0u ) {
			pData->tParser.iUtfExpected = 2;
			pData->tParser.iUtfSeen = 0;
			pData->tParser.iUtfCodepoint = (uint32_t)(b & 0x0fu);
		} else if ( (b & 0xf8u) == 0xf0u ) {
			pData->tParser.iUtfExpected = 3;
			pData->tParser.iUtfSeen = 0;
			pData->tParser.iUtfCodepoint = (uint32_t)(b & 0x07u);
		}
		break;
	case 1:
		if ( b == '[' ) {
			__xuiTerminalCsiReset(&pData->tParser);
			pData->tParser.iState = 2;
		} else if ( b == ']' ) {
			pData->tParser.iOscSize = 0;
			pData->tParser.iState = 3;
		} else if ( b == 'P' || b == 'X' || b == '^' || b == '_' ) {
			pData->tParser.iState = 5;
		} else if ( b == '(' || b == ')' || b == '*' || b == '+' ||
		            b == '-' || b == '.' || b == '/' || b == '%' || b == '#' ) {
			pData->tParser.cCharsetTarget = (char)b;
			pData->tParser.iState = 7;
		} else if ( b == '7' ) {
			__xuiTerminalSaveCursorState(pData);
			pData->tParser.iState = 0;
		} else if ( b == '8' ) {
			__xuiTerminalRestoreCursorState(pData);
			pData->tParser.iState = 0;
		} else if ( b == 'H' ) {
			pData->bWrapPending = 0;
			__xuiTerminalSetTabStop(pData, pData->iCursorX, 1);
			pData->tParser.iState = 0;
		} else if ( b == 'D' ) {
			pData->bWrapPending = 0;
			__xuiTerminalLineFeed(pWidget, pData);
			pData->tParser.iState = 0;
		} else if ( b == 'E' ) {
			pData->bWrapPending = 0;
			pData->iCursorX = 0;
			__xuiTerminalLineFeed(pWidget, pData);
			pData->tParser.iState = 0;
		} else if ( b == 'M' ) {
			pData->bWrapPending = 0;
			if ( pData->iCursorY <= pData->iScrollTop ) {
				__xuiTerminalScrollDown(pData, pData->iScrollTop, pData->iScrollBottom, 1);
				pData->iCursorY = pData->iScrollTop;
			} else {
				pData->iCursorY--;
			}
			pData->tParser.iState = 0;
		} else if ( b == '=' ) {
			pData->bApplicationKeypad = 1;
			pData->tParser.iState = 0;
		} else if ( b == '>' ) {
			pData->bApplicationKeypad = 0;
			pData->tParser.iState = 0;
		} else if ( b == 'c' ) {
			pData->bApplicationCursor = 0;
			pData->bApplicationKeypad = 0;
			pData->iG0Charset = 0;
			pData->iG1Charset = 0;
			pData->iActiveCharset = 0;
			pData->bOriginMode = 0;
			pData->bInsertMode = 0;
			pData->bAutoWrap = 1;
			pData->bCursorVisible = 1;
			pData->iMouseTracking = XUI_TERMINAL_MOUSE_TRACKING_NONE;
			pData->iMouseEncoding = XUI_TERMINAL_MOUSE_ENCODING_X10;
			pData->bFocusReporting = 0;
			pData->bBracketedPaste = 0;
			pData->iScrollTop = 0;
			pData->iScrollBottom = pData->iRows - 1;
			pData->iCurrentFg = pData->iForegroundColor;
			pData->iCurrentBg = pData->iBackgroundColor;
			pData->iCurrentFlags = 0;
			pData->iCurrentLinkId = 0;
			(void)xuiTerminalClear(pWidget);
			pData->tParser.iState = 0;
		} else {
			pData->tParser.iState = 0;
		}
		break;
	case 2:
		if ( b >= 0x3cu && b <= 0x3fu && pData->tParser.iParamCount == 0 && pData->tParser.cPrivate == 0 ) {
			pData->tParser.cPrivate = (char)b;
		} else if ( b >= '0' && b <= '9' ) {
			__xuiTerminalCsiAppendDigit(&pData->tParser, b - '0');
		} else if ( b == ';' ) {
			__xuiTerminalCsiNextParam(&pData->tParser);
		} else if ( b >= 0x20u && b <= 0x2fu ) {
			if ( pData->tParser.iIntermediateCount < (int)sizeof(pData->tParser.arrIntermediates) ) {
				pData->tParser.arrIntermediates[pData->tParser.iIntermediateCount++] = (char)b;
			}
		} else if ( b >= 0x40u && b <= 0x7eu ) {
			__xuiTerminalDispatchCsi(pWidget, pData, b);
			pData->tParser.iState = 0;
		}
		break;
	case 3:
		if ( b == '\a' || b == 0x9cu ) {
			__xuiTerminalDispatchOsc(pWidget, pData);
			pData->tParser.iState = 0;
		} else if ( b == 0x1bu ) {
			pData->tParser.iState = 4;
		} else if ( pData->tParser.iOscSize < XUI_TERMINAL_MAX_OSC - 1 ) {
			pData->tParser.sOsc[pData->tParser.iOscSize++] = (char)b;
		}
		break;
	case 4:
		if ( b == '\\' ) {
			__xuiTerminalDispatchOsc(pWidget, pData);
			pData->tParser.iState = 0;
		} else {
			pData->tParser.iState = 3;
		}
		break;
	case 5:
		if ( b == 0x9cu ) {
			pData->tParser.iState = 0;
		} else if ( b == 0x1bu ) {
			pData->tParser.iState = 6;
		}
		break;
	case 6:
		if ( b == '\\' ) pData->tParser.iState = 0;
		else if ( b != 0x1bu ) pData->tParser.iState = 5;
		break;
	case 7:
		if ( pData->tParser.cCharsetTarget == '(' ) {
			pData->iG0Charset = b == '0' ? 1 : 0;
		} else if ( pData->tParser.cCharsetTarget == ')' ) {
			pData->iG1Charset = b == '0' ? 1 : 0;
		}
		pData->tParser.cCharsetTarget = 0;
		pData->tParser.iState = 0;
		break;
	default:
		pData->tParser.iState = 0;
		break;
	}
}

static int __xuiTerminalProcessQueue(xui_widget pWidget, xui_terminal_data_t* pData, int iBudget)
{
	int iCount;
	int i;
	int iIndex;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	if ( pData->iQueueSize <= 0 ) return XUI_OK;
	iCount = pData->iQueueSize;
	if ( (iBudget > 0) && (iCount > iBudget) ) {
		iCount = iBudget;
	}
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
	for ( i = 0; i < iCount; i++ ) {
		iIndex = pData->iQueueHead + i;
		if ( iIndex >= pData->iQueueCapacity ) iIndex -= pData->iQueueCapacity;
		__xuiTerminalProcessByte(pWidget, pData, pData->pQueue[iIndex]);
	}
	pData->iOutputBytesParsed += (uint64_t)iCount;
	if ( pData->iSearchLine >= 0 ) {
		if ( pData->iSearchHistoryId != 0u ) (void)__xuiTerminalResolveFindAnchor(pData);
		else __xuiTerminalClearFindMatch(pData);
	}
	__xuiTerminalMarkDirtyRow(pData, pData->iCursorY);
	pData->iQueueHead = (pData->iQueueHead + iCount) % pData->iQueueCapacity;
	pData->iQueueSize -= iCount;
	if ( pData->iQueueSize == 0 ) pData->iQueueHead = 0;
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalInvalidateDirty(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

static int __xuiTerminalAppendQueue(xui_terminal_data_t* pData, const void* pBytes, int iSize)
{
	uint8_t* pNew;
	int iCapacity;
	int iFirst;
	int iTail;

	if ( (pData == NULL) || (pBytes == NULL) || (iSize < 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( iSize == 0 ) {
		return XUI_OK;
	}
	if ( iSize > 0x7fffffff - pData->iQueueSize ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pData->iQueueSize + iSize > pData->iQueueCapacity ) {
		iCapacity = pData->iQueueCapacity;
		if ( iCapacity < 256 ) iCapacity = 256;
		while ( iCapacity < pData->iQueueSize + iSize ) {
			if ( iCapacity > 0x3fffffff ) {
				iCapacity = pData->iQueueSize + iSize;
				break;
			}
			iCapacity *= 2;
		}
		pNew = (uint8_t*)xrtMalloc((size_t)iCapacity);
		if ( pNew == NULL ) {
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		if ( pData->iQueueSize > 0 ) {
			iFirst = __xuiTerminalMin(pData->iQueueSize,
				pData->iQueueCapacity - pData->iQueueHead);
			memcpy(pNew, pData->pQueue + pData->iQueueHead, (size_t)iFirst);
			if ( iFirst < pData->iQueueSize ) {
				memcpy(pNew + iFirst, pData->pQueue,
					(size_t)(pData->iQueueSize - iFirst));
			}
		}
		if ( pData->pQueue != NULL ) xrtFree(pData->pQueue);
		pData->pQueue = pNew;
		pData->iQueueCapacity = iCapacity;
		pData->iQueueHead = 0;
	}
	iTail = (pData->iQueueHead + pData->iQueueSize) % pData->iQueueCapacity;
	iFirst = __xuiTerminalMin(iSize, pData->iQueueCapacity - iTail);
	memcpy(pData->pQueue + iTail, pBytes, (size_t)iFirst);
	if ( iFirst < iSize ) {
		memcpy(pData->pQueue, (const uint8_t*)pBytes + iFirst, (size_t)(iSize - iFirst));
	}
	pData->iQueueSize += iSize;
	pData->iOutputBytesReceived += (uint64_t)iSize;
	if ( pData->iQueueSize > pData->iQueuePeak ) pData->iQueuePeak = pData->iQueueSize;
	return XUI_OK;
}

static int __xuiTerminalSerializeToBuffer(xui_terminal_data_t* pData, char* sBuffer, int iCapacity)
{
	const xui_terminal_cell_t* pCells;
	char* sLine;
	int iNeeded;
	int iOffset;
	int i;
	int n;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeeded = 0;
	iOffset = 0;
	if ( (sBuffer != NULL) && (iCapacity > 0) ) {
		sBuffer[0] = '\0';
	}
	for ( i = 0; i < pData->iScrollbackCount; i++ ) {
		const char* sScroll = __xuiTerminalGetScrollbackLine(pData, i);
		n = (int)strlen(sScroll);
		iNeeded += n + 1;
		if ( (sBuffer != NULL) && (iCapacity > 0) && (iOffset < iCapacity - 1) ) {
			int iCopy = __xuiTerminalMin(n, iCapacity - 1 - iOffset);
			if ( iCopy > 0 ) memcpy(sBuffer + iOffset, sScroll, (size_t)iCopy);
			iOffset += iCopy;
			if ( iOffset < iCapacity - 1 ) sBuffer[iOffset++] = '\n';
			sBuffer[iOffset] = '\0';
		}
	}
	pCells = __xuiTerminalScreenConst(pData);
	for ( i = 0; i < pData->iRows; i++ ) {
		sLine = __xuiTerminalSerializeRow(pData,
			__xuiTerminalBufferRowConst(pData, pCells, i));
		if ( sLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		n = (int)strlen(sLine);
		iNeeded += n + ((i + 1 < pData->iRows) ? 1 : 0);
		if ( (sBuffer != NULL) && (iCapacity > 0) && (iOffset < iCapacity - 1) ) {
			int iCopy = __xuiTerminalMin(n, iCapacity - 1 - iOffset);
			if ( iCopy > 0 ) memcpy(sBuffer + iOffset, sLine, (size_t)iCopy);
			iOffset += iCopy;
			if ( (i + 1 < pData->iRows) && (iOffset < iCapacity - 1) ) sBuffer[iOffset++] = '\n';
			sBuffer[iOffset] = '\0';
		}
		xrtFree(sLine);
	}
	return iNeeded;
}

static int __xuiTerminalLogicalLineCount(const xui_terminal_data_t* pData)
{
	return (pData != NULL) ? (pData->iScrollbackCount + pData->iRows) : 0;
}

static int __xuiTerminalClampLine(const xui_terminal_data_t* pData, int iLine)
{
	int iCount = __xuiTerminalLogicalLineCount(pData);
	if ( iCount <= 0 ) return 0;
	if ( iLine < 0 ) return 0;
	if ( iLine >= iCount ) return iCount - 1;
	return iLine;
}

static int __xuiTerminalClampColumn(const xui_terminal_data_t* pData, int iColumn)
{
	int iMax = (pData != NULL && pData->iColumns > 0) ? pData->iColumns : 0;
	if ( iColumn < 0 ) return 0;
	if ( iColumn > iMax ) return iMax;
	return iColumn;
}

static char* __xuiTerminalLogicalLineCopy(xui_terminal_data_t* pData, int iLine)
{
	const xui_terminal_cell_t* pCells;

	if ( pData == NULL ) return NULL;
	iLine = __xuiTerminalClampLine(pData, iLine);
	if ( iLine < pData->iScrollbackCount ) {
		return __xuiTerminalStrDup(__xuiTerminalGetScrollbackLine(pData, iLine));
	}
	pCells = __xuiTerminalScreenConst(pData);
	if ( pCells == NULL ) return __xuiTerminalStrDup("");
	iLine -= pData->iScrollbackCount;
	if ( iLine < 0 || iLine >= pData->iRows ) return __xuiTerminalStrDup("");
	return __xuiTerminalSerializeRow(pData,
		__xuiTerminalBufferRowConst(pData, pCells, iLine));
}

static int __xuiTerminalUtf8Next(const char* sText, int iRemaining, uint32_t* pCodepoint, int* pBytes)
{
	const unsigned char* p;
	uint32_t cp;
	int n;

	if ( (sText == NULL) || (sText[0] == '\0') || (iRemaining == 0) ) return 0;
	p = (const unsigned char*)sText;
	cp = p[0];
	n = 1;
	if ( (p[0] & 0x80u) == 0u ) {
		cp = p[0];
		n = 1;
	} else if ( (p[0] & 0xe0u) == 0xc0u && iRemaining != 1 && p[1] != '\0' && ((p[1] & 0xc0u) == 0x80u) ) {
		cp = ((uint32_t)(p[0] & 0x1fu) << 6) | (uint32_t)(p[1] & 0x3fu);
		n = 2;
	} else if ( (p[0] & 0xf0u) == 0xe0u && (iRemaining < 0 || iRemaining >= 3) &&
	            p[1] != '\0' && p[2] != '\0' &&
	            ((p[1] & 0xc0u) == 0x80u) && ((p[2] & 0xc0u) == 0x80u) ) {
		cp = ((uint32_t)(p[0] & 0x0fu) << 12) | ((uint32_t)(p[1] & 0x3fu) << 6) | (uint32_t)(p[2] & 0x3fu);
		n = 3;
	} else if ( (p[0] & 0xf8u) == 0xf0u && (iRemaining < 0 || iRemaining >= 4) &&
	            p[1] != '\0' && p[2] != '\0' && p[3] != '\0' &&
	            ((p[1] & 0xc0u) == 0x80u) && ((p[2] & 0xc0u) == 0x80u) && ((p[3] & 0xc0u) == 0x80u) ) {
		cp = ((uint32_t)(p[0] & 0x07u) << 18) | ((uint32_t)(p[1] & 0x3fu) << 12) |
		     ((uint32_t)(p[2] & 0x3fu) << 6) | (uint32_t)(p[3] & 0x3fu);
		n = 4;
	}
	if ( pCodepoint != NULL ) *pCodepoint = cp;
	if ( pBytes != NULL ) *pBytes = n;
	return 1;
}

static int __xuiTerminalUtf8DisplayColumns(const char* sText, int iBytes)
{
	uint32_t cp;
	int iOffset;
	int iStep;
	int iColumns;
	int iRemaining;

	if ( sText == NULL ) return 0;
	iOffset = 0;
	iColumns = 0;
	while ( sText[iOffset] != '\0' && (iBytes < 0 || iOffset < iBytes) ) {
		iRemaining = (iBytes < 0) ? -1 : (iBytes - iOffset);
		if ( !__xuiTerminalUtf8Next(sText + iOffset, iRemaining, &cp, &iStep) || iStep <= 0 ) break;
		if ( iBytes >= 0 && iOffset + iStep > iBytes ) break;
		iColumns += __xuiTerminalCodepointWidth(cp);
		iOffset += iStep;
	}
	return iColumns;
}

static int __xuiTerminalUtf8ByteFromColumn(const char* sText, int iColumn)
{
	uint32_t iCodepoint;
	int iOffset;
	int iStep;
	int iColumns;
	int iWidth;

	if ( sText == NULL || iColumn <= 0 ) return 0;
	iOffset = 0;
	iColumns = 0;
	while ( sText[iOffset] != '\0' ) {
		if ( !__xuiTerminalUtf8Next(sText + iOffset, -1, &iCodepoint, &iStep) || iStep <= 0 ) break;
		iWidth = __xuiTerminalCodepointWidth(iCodepoint);
		if ( iWidth < 0 ) iWidth = 0;
		if ( iColumns + iWidth > iColumn ) break;
		iColumns += iWidth;
		iOffset += iStep;
		if ( iColumns == iColumn ) break;
	}
	return iOffset;
}

static uint32_t __xuiTerminalSearchToFindFlags(uint32_t iFlags, int bForward)
{
	uint32_t iFindFlags = 0u;
	if ( (iFlags & XUI_TERMINAL_SEARCH_CASE_SENSITIVE) != 0u ) iFindFlags |= XUI_FIND_CASE_SENSITIVE;
	if ( (iFlags & XUI_TERMINAL_SEARCH_WHOLE_WORD) != 0u ) iFindFlags |= XUI_FIND_WHOLE_WORD;
	if ( (iFlags & XUI_TERMINAL_SEARCH_REGEX) != 0u ) iFindFlags |= XUI_FIND_REGEX;
	if ( !bForward ) iFindFlags |= XUI_FIND_BACKWARD;
	return iFindFlags;
}

static int __xuiTerminalFindInLine(const char* sLine, const char* sText, uint32_t iFlags,
	int iStartColumn, int bForward, int* pColumn, int* pLength)
{
	xui_find_result_t tResult;
	char sError[128];
	int iLineBytes;
	int iStartByte;
	int iRet;

	if ( (sLine == NULL) || (sText == NULL) || (sText[0] == '\0') ) return 0;
	if ( iStartColumn < 0 ) return 0;
	iLineBytes = (int)strlen(sLine);
	iStartByte = __xuiTerminalUtf8ByteFromColumn(sLine, iStartColumn);
	memset(&tResult, 0, sizeof(tResult));
	tResult.iSize = sizeof(tResult);
	sError[0] = '\0';
	iRet = xuiFindText(sLine, iLineBytes, sText, iStartByte, 0, iLineBytes,
		__xuiTerminalSearchToFindFlags(iFlags, bForward), &tResult,
		sError, (int)sizeof(sError));
	if ( iRet == XUI_ERROR_UNSUPPORTED ) return 0;
	if ( iRet != XUI_OK ) return iRet;
	if ( pColumn != NULL ) *pColumn = __xuiTerminalUtf8DisplayColumns(sLine, tResult.iStart);
	if ( pLength != NULL ) {
		*pLength = __xuiTerminalUtf8DisplayColumns(sLine + tResult.iStart,
			tResult.iEnd - tResult.iStart);
		if ( *pLength < 1 ) *pLength = 1;
	}
	return 1;
}

static void __xuiTerminalClearFindMatch(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) return;
	pData->iSearchLine = -1;
	pData->iSearchColumn = 0;
	pData->iSearchLength = 0;
	pData->iSearchHistoryId = 0u;
	pData->iSearchHistoryCell = 0;
}

static void __xuiTerminalSetFindAnchor(xui_terminal_data_t* pData, int iLine, int iColumn)
{
	xui_terminal_history_view_row_t* pView;
	if ( pData == NULL ) return;
	pData->iSearchHistoryId = 0u;
	pData->iSearchHistoryCell = 0;
	if ( iLine < 0 || iLine >= pData->iScrollbackCount ) return;
	pView = __xuiTerminalGetHistoryViewRow(pData, iLine);
	if ( pView == NULL || pView->pLine == NULL ) return;
	pData->iSearchHistoryId = pView->pLine->iId;
	pData->iSearchHistoryCell = pView->iStart + __xuiTerminalMax(iColumn, 0);
}

static int __xuiTerminalResolveFindAnchor(xui_terminal_data_t* pData)
{
	int i;
	if ( pData == NULL || pData->iSearchHistoryId == 0u ) return 0;
	if ( __xuiTerminalRebuildHistoryView(pData) != XUI_OK ) return 0;
	for ( i = 0; i < pData->iScrollbackCount; ++i ) {
		xui_terminal_history_view_row_t* pView = &pData->pHistoryView[i];
		if ( pView->pLine == NULL || pView->pLine->iId != pData->iSearchHistoryId ) continue;
		if ( pData->iSearchHistoryCell < pView->iStart ||
		     pData->iSearchHistoryCell >= pView->iEnd ) continue;
		pData->iSearchLine = i;
		pData->iSearchColumn = pData->iSearchHistoryCell - pView->iStart;
		return 1;
	}
	__xuiTerminalClearFindMatch(pData);
	return 0;
}

static void __xuiTerminalClearHoverLinkData(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) return;
	if ( pData->sHoverLink != NULL ) {
		xrtFree(pData->sHoverLink);
		pData->sHoverLink = NULL;
	}
	pData->iHoverLinkLine = -1;
	pData->iHoverLinkColumn = 0;
	pData->iHoverLinkLength = 0;
}

static int __xuiTerminalUrlPrefixAt(const char* sText, int iOffset)
{
	static const char* arrPrefixes[] = {
		"http://",
		"https://",
		"file://",
		"mailto:"
	};
	int i;
	int j;

	if ( sText == NULL || iOffset < 0 ) return 0;
	for ( i = 0; i < (int)(sizeof(arrPrefixes) / sizeof(arrPrefixes[0])); i++ ) {
		for ( j = 0; arrPrefixes[i][j] != '\0'; j++ ) {
			char a = sText[iOffset + j];
			char b = arrPrefixes[i][j];
			if ( a == '\0' ) break;
			if ( (char)tolower((unsigned char)a) != b ) break;
		}
		if ( arrPrefixes[i][j] == '\0' ) return j;
	}
	return 0;
}

static int __xuiTerminalIsUrlChar(unsigned char c)
{
	if ( c <= 0x20u ) return 0;
	if ( c == '"' || c == '\'' || c == '<' || c == '>' ) return 0;
	return 1;
}

static int __xuiTerminalIsUrlTrimChar(unsigned char c)
{
	return c == '.' || c == ',' || c == ';' || c == ':' || c == '!' ||
	       c == '?' || c == ')' || c == ']' || c == '}';
}

static int __xuiTerminalDetectUrlInText(const char* sLine, int iColumn, int* pStartColumn, int* pLength, int* pStartByte, int* pByteLength)
{
	int iLen;
	int iByte;
	int iPrefix;
	int iEnd;
	int iStartColumn;
	int iEndColumn;

	if ( sLine == NULL || iColumn < 0 ) return 0;
	iLen = (int)strlen(sLine);
	for ( iByte = 0; iByte < iLen; iByte++ ) {
		iPrefix = __xuiTerminalUrlPrefixAt(sLine, iByte);
		if ( iPrefix <= 0 ) continue;
		iEnd = iByte + iPrefix;
		while ( iEnd < iLen && __xuiTerminalIsUrlChar((unsigned char)sLine[iEnd]) ) iEnd++;
		while ( iEnd > iByte + iPrefix && __xuiTerminalIsUrlTrimChar((unsigned char)sLine[iEnd - 1]) ) iEnd--;
		if ( iEnd <= iByte + iPrefix ) continue;
		iStartColumn = __xuiTerminalUtf8DisplayColumns(sLine, iByte);
		iEndColumn = __xuiTerminalUtf8DisplayColumns(sLine, iEnd);
		if ( iColumn >= iStartColumn && iColumn < iEndColumn ) {
			if ( pStartColumn != NULL ) *pStartColumn = iStartColumn;
			if ( pLength != NULL ) *pLength = iEndColumn - iStartColumn;
			if ( pStartByte != NULL ) *pStartByte = iByte;
			if ( pByteLength != NULL ) *pByteLength = iEnd - iByte;
			return 1;
		}
	}
	return 0;
}

static int __xuiTerminalOscLinkAt(xui_terminal_data_t* pData, int iLine, int iColumn, char** ppUrl, int* pStartColumn, int* pLength)
{
	const xui_terminal_cell_t* pCells;
	const xui_terminal_cell_t* pRow;
	xui_terminal_history_view_row_t* pHistoryRow;
	const char* sUrl;
	uint16_t iLinkId;
	int iScreenRow;
	int iLinkColumns;
	int iStart;
	int iEnd;
	int iUrlLength;

	if ( ppUrl != NULL ) *ppUrl = NULL;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iLine < 0 || iLine >= __xuiTerminalLogicalLineCount(pData) || iColumn < 0 ) return 0;
	if ( iLine < pData->iScrollbackCount ) {
		pHistoryRow = __xuiTerminalGetHistoryViewRow(pData, iLine);
		if ( pHistoryRow == NULL ) return 0;
		iLinkColumns = pHistoryRow->iEnd - pHistoryRow->iStart;
		if ( iColumn >= iLinkColumns ) return 0;
		iLinkId = pHistoryRow->pLine->pCells[pHistoryRow->iStart + iColumn].iLinkId;
		if ( iLinkId == 0 ) return 0;
		sUrl = __xuiTerminalGetLinkUrl(pData, iLinkId);
		if ( sUrl == NULL || sUrl[0] == '\0' ) return 0;
		iStart = iColumn;
		while ( iStart > 0 && pHistoryRow->pLine->pCells[pHistoryRow->iStart + iStart - 1].iLinkId == iLinkId ) {
			iStart--;
		}
		iEnd = iColumn + 1;
		while ( iEnd < iLinkColumns && pHistoryRow->pLine->pCells[pHistoryRow->iStart + iEnd].iLinkId == iLinkId ) {
			iEnd++;
		}
	} else {
		if ( iColumn >= pData->iColumns ) return 0;
		iScreenRow = iLine - pData->iScrollbackCount;
		if ( iScreenRow < 0 || iScreenRow >= pData->iRows ) return 0;
		pCells = __xuiTerminalScreenConst(pData);
		if ( pCells == NULL ) return 0;
		pRow = __xuiTerminalBufferRowConst(pData, pCells, iScreenRow);
		iLinkId = pRow[iColumn].iLinkId;
		if ( iLinkId == 0 ) return 0;
		sUrl = __xuiTerminalGetLinkUrl(pData, iLinkId);
		if ( sUrl == NULL || sUrl[0] == '\0' ) return 0;
		iStart = iColumn;
		while ( iStart > 0 && pRow[iStart - 1].iLinkId == iLinkId ) {
			iStart--;
		}
		iEnd = iColumn + 1;
		while ( iEnd < pData->iColumns && pRow[iEnd].iLinkId == iLinkId ) {
			iEnd++;
		}
	}
	if ( iLinkId == 0 ) return 0;
	iUrlLength = (int)strlen(sUrl);
	if ( pStartColumn != NULL ) *pStartColumn = iStart;
	if ( pLength != NULL ) *pLength = iEnd - iStart;
	if ( ppUrl != NULL ) {
		*ppUrl = __xuiTerminalStrDup(sUrl);
		if ( *ppUrl == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	}
	return iUrlLength;
}

static int __xuiTerminalLogicalUrlAt(xui_terminal_data_t* pData, int iLine, int iColumn, char** ppUrl, int* pStartColumn, int* pLength)
{
	char* sLine;
	char* sUrl;
	int iStartByte;
	int iByteLength;
	int iRet;

	if ( ppUrl != NULL ) *ppUrl = NULL;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iLine < 0 || iLine >= __xuiTerminalLogicalLineCount(pData) || iColumn < 0 ) return 0;
	iRet = __xuiTerminalOscLinkAt(pData, iLine, iColumn, ppUrl, pStartColumn, pLength);
	if ( iRet != 0 ) return iRet;
	sLine = __xuiTerminalLogicalLineCopy(pData, iLine);
	if ( sLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = __xuiTerminalDetectUrlInText(sLine, iColumn, pStartColumn, pLength, &iStartByte, &iByteLength);
	if ( iRet > 0 && ppUrl != NULL ) {
		sUrl = (char*)xrtMalloc((size_t)iByteLength + 1u);
		if ( sUrl == NULL ) {
			xrtFree(sLine);
			return XUI_ERROR_OUT_OF_MEMORY;
		}
		memcpy(sUrl, sLine + iStartByte, (size_t)iByteLength);
		sUrl[iByteLength] = '\0';
		*ppUrl = sUrl;
	}
	xrtFree(sLine);
	return iRet ? iByteLength : 0;
}

static int __xuiTerminalSetHoverLink(xui_widget pWidget, xui_terminal_data_t* pData, int iLine, int iColumn, int iLength, const char* sUrl)
{
	char* sCopy;
	int bChanged;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sUrl == NULL || sUrl[0] == '\0' || iLine < 0 || iLength <= 0 ) {
		bChanged = (pData->sHoverLink != NULL || pData->iHoverLinkLine >= 0 || pData->iHoverLinkLength > 0);
		__xuiTerminalClearHoverLinkData(pData);
		if ( bChanged ) __xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_OK;
	}
	bChanged = (pData->sHoverLink == NULL) ||
	           (strcmp(pData->sHoverLink, sUrl) != 0) ||
	           (pData->iHoverLinkLine != iLine) ||
	           (pData->iHoverLinkColumn != iColumn) ||
	           (pData->iHoverLinkLength != iLength);
	if ( !bChanged ) return XUI_OK;
	sCopy = __xuiTerminalStrDup(sUrl);
	if ( sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	__xuiTerminalClearHoverLinkData(pData);
	pData->sHoverLink = sCopy;
	pData->iHoverLinkLine = iLine;
	pData->iHoverLinkColumn = iColumn;
	pData->iHoverLinkLength = iLength;
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

static int __xuiTerminalUpdateHoverLink(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	char* sUrl;
	int iLine;
	int iColumn;
	int iStartColumn;
	int iLength;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) != XUI_OK ) {
		return __xuiTerminalSetHoverLink(pWidget, pData, -1, 0, 0, NULL);
	}
	sUrl = NULL;
	iStartColumn = 0;
	iLength = 0;
	iRet = __xuiTerminalLogicalUrlAt(pData, iLine, iColumn, &sUrl, &iStartColumn, &iLength);
	if ( iRet < 0 ) return iRet;
	if ( iRet > 0 && sUrl != NULL ) {
		iRet = __xuiTerminalSetHoverLink(pWidget, pData, iLine, iStartColumn, iLength, sUrl);
		xrtFree(sUrl);
		return iRet;
	}
	return __xuiTerminalSetHoverLink(pWidget, pData, -1, 0, 0, NULL);
}

static int __xuiTerminalSetSearchText(xui_terminal_data_t* pData, const char* sText, uint32_t iFlags)
{
	char* sCopy;

	if ( (pData == NULL) || (sText == NULL) || (sText[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->sSearchText != NULL) && (strcmp(pData->sSearchText, sText) == 0) && (pData->iSearchFlags == iFlags) ) {
		return XUI_OK;
	}
	sCopy = __xuiTerminalStrDup(sText);
	if ( sCopy == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pData->sSearchText != NULL ) xrtFree(pData->sSearchText);
	pData->sSearchText = sCopy;
	pData->iSearchFlags = iFlags;
	__xuiTerminalClearFindMatch(pData);
	return XUI_OK;
}

static int __xuiTerminalFindNavigate(xui_widget pWidget, xui_terminal_data_t* pData, const char* sText, uint32_t iFlags, int bForward, int bContinue, int* pLine, int* pColumn)
{
	char* sLine;
	int iCount;
	int iStartLine;
	int iStartColumn;
	int iLine;
	int iColumn;
	int iLength;
	int i;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (sText == NULL) || (sText[0] == '\0') ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTerminalSetSearchText(pData, sText, iFlags);
	if ( iRet != XUI_OK ) return iRet;
	iCount = __xuiTerminalLogicalLineCount(pData);
	if ( iCount <= 0 ) return 0;
	if ( bContinue && pData->iSearchLine >= 0 ) {
		if ( pData->iSearchHistoryId != 0u ) (void)__xuiTerminalResolveFindAnchor(pData);
		iStartLine = __xuiTerminalClampLine(pData, pData->iSearchLine);
		iStartColumn = bForward ? (pData->iSearchColumn + __xuiTerminalMax(pData->iSearchLength, 1)) : (pData->iSearchColumn - 1);
	} else {
		iStartLine = bForward ? 0 : (iCount - 1);
		iStartColumn = bForward ? 0 : pData->iColumns;
	}
	if ( iStartColumn < 0 ) iStartColumn = -1;
	if ( iStartColumn > pData->iColumns ) iStartColumn = pData->iColumns;
	for ( i = 0; i < iCount; i++ ) {
		iLine = bForward ? ((iStartLine + i) % iCount) : ((iStartLine - i + iCount) % iCount);
		iColumn = (i == 0) ? iStartColumn : (bForward ? 0 : pData->iColumns);
		sLine = __xuiTerminalLogicalLineCopy(pData, iLine);
		if ( sLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iLength = 0;
		iRet = __xuiTerminalFindInLine(sLine, sText, iFlags, iColumn, bForward,
			&iColumn, &iLength);
		xrtFree(sLine);
		if ( iRet < 0 ) return iRet;
		if ( iRet > 0 ) {
			pData->iSearchLine = iLine;
			pData->iSearchColumn = __xuiTerminalClampColumn(pData, iColumn);
			pData->iSearchLength = iLength;
			__xuiTerminalSetFindAnchor(pData, pData->iSearchLine, pData->iSearchColumn);
			if ( pLine != NULL ) *pLine = pData->iSearchLine;
			if ( pColumn != NULL ) *pColumn = pData->iSearchColumn;
			__xuiTerminalSyncScrollModel(pWidget, pData);
			(void)xuiScrollModelSetOffset(&pData->tScroll, 0.0f, (float)pData->iSearchLine * pData->fCellHeight);
			__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return 1;
		}
	}
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return 0;
}

static void __xuiTerminalClearSelectionData(xui_terminal_data_t* pData)
{
	if ( pData == NULL ) return;
	pData->bSelectAll = 0;
	pData->bSelecting = 0;
	pData->iSelectAnchorLine = -1;
	pData->iSelectAnchorColumn = 0;
	pData->iSelectEndLine = -1;
	pData->iSelectEndColumn = 0;
}

static void __xuiTerminalSetSelectionRange(xui_terminal_data_t* pData, int iAnchorLine, int iAnchorColumn, int iEndLine, int iEndColumn)
{
	if ( pData == NULL ) return;
	pData->bSelectAll = 0;
	pData->iSelectAnchorLine = __xuiTerminalClampLine(pData, iAnchorLine);
	pData->iSelectAnchorColumn = __xuiTerminalClampColumn(pData, iAnchorColumn);
	pData->iSelectEndLine = __xuiTerminalClampLine(pData, iEndLine);
	pData->iSelectEndColumn = __xuiTerminalClampColumn(pData, iEndColumn);
}

static int __xuiTerminalSelectionRange(xui_terminal_data_t* pData, int* pLine0, int* pColumn0, int* pLine1, int* pColumn1)
{
	int l0;
	int c0;
	int l1;
	int c1;

	if ( pData == NULL ) return 0;
	if ( pData->bSelectAll ) {
		if ( pLine0 != NULL ) *pLine0 = 0;
		if ( pColumn0 != NULL ) *pColumn0 = 0;
		if ( pLine1 != NULL ) *pLine1 = __xuiTerminalLogicalLineCount(pData) - 1;
		if ( pColumn1 != NULL ) *pColumn1 = pData->iColumns;
		return __xuiTerminalLogicalLineCount(pData) > 0;
	}
	if ( pData->iSelectAnchorLine < 0 || pData->iSelectEndLine < 0 ) return 0;
	l0 = pData->iSelectAnchorLine;
	c0 = pData->iSelectAnchorColumn;
	l1 = pData->iSelectEndLine;
	c1 = pData->iSelectEndColumn;
	if ( (l0 > l1) || (l0 == l1 && c0 > c1) ) {
		int t;
		t = l0; l0 = l1; l1 = t;
		t = c0; c0 = c1; c1 = t;
	}
	if ( l0 == l1 && c0 == c1 ) return 0;
	if ( pLine0 != NULL ) *pLine0 = l0;
	if ( pColumn0 != NULL ) *pColumn0 = c0;
	if ( pLine1 != NULL ) *pLine1 = l1;
	if ( pColumn1 != NULL ) *pColumn1 = c1;
	return 1;
}

static void __xuiTerminalEmitSelectionChange(xui_widget pWidget, xui_terminal_data_t* pData)
{
	int iLine0;
	int iColumn0;
	int iLine1;
	int iColumn1;
	if ( pWidget == NULL || pData == NULL || pData->iColumns <= 0 ) return;
	if ( !__xuiTerminalSelectionRange(pData, &iLine0, &iColumn0, &iLine1, &iColumn1) ) {
		iLine0 = iLine1 = 0;
		iColumn0 = iColumn1 = 0;
	}
	(void)xuiInternalEditEmit(pWidget, XUI_EDIT_EVENT_SELECTION_CHANGED, NULL,
		iLine0 * pData->iColumns + iColumn0,
		iLine1 * pData->iColumns + iColumn1, 0, 0, 1);
}

static int __xuiTerminalPointerToCell(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent, int* pLine, int* pColumn)
{
	xui_rect_t tContent;
	xui_rect_t tWorld;
	float fLocalX;
	float fLocalY;
	int iOffsetY;
	int iColumn;
	int iLine;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	tWorld = xuiWidgetGetWorldRect(pWidget);
	tContent = xuiWidgetGetContentRect(pWidget);
	iOffsetY = __xuiTerminalScrollOffsetPixels(pData);
	fLocalX = pEvent->fX - tWorld.fX - tContent.fX - pData->fPadding;
	fLocalY = pEvent->fY - tWorld.fY - tContent.fY - pData->fPadding + (float)iOffsetY;
	if ( fLocalX < 0.0f ) {
		iColumn = 0;
	} else {
		iColumn = (pData->fCellWidth > 0.0f) ? (int)(fLocalX / pData->fCellWidth) : 0;
		if ( iColumn > pData->iColumns ) iColumn = pData->iColumns;
	}
	iLine = (fLocalY <= 0.0f || pData->fCellHeight <= 0.0f) ? 0 : (int)(fLocalY / pData->fCellHeight);
	iLine = __xuiTerminalClampLine(pData, iLine);
	if ( pLine != NULL ) *pLine = iLine;
	if ( pColumn != NULL ) *pColumn = __xuiTerminalClampColumn(pData, iColumn);
	return XUI_OK;
}

static int __xuiTerminalSelectionTextToBuffer(xui_terminal_data_t* pData, char* sBuffer, int iCapacity)
{
	char* sLine;
	int iLine0;
	int iColumn0;
	int iLine1;
	int iColumn1;
	int iLine;
	int iNeeded;
	int iOffset;
	int iLen;
	int iStart;
	int iEnd;
	int iStartColumn;
	int iEndColumn;
	int iCopy;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiTerminalSelectionRange(pData, &iLine0, &iColumn0, &iLine1, &iColumn1) ) {
		if ( sBuffer != NULL && iCapacity > 0 ) sBuffer[0] = '\0';
		return 0;
	}
	if ( pData->bSelectAll ) {
		return __xuiTerminalSerializeToBuffer(pData, sBuffer, iCapacity);
	}
	iNeeded = 0;
	iOffset = 0;
	if ( sBuffer != NULL && iCapacity > 0 ) sBuffer[0] = '\0';
	for ( iLine = iLine0; iLine <= iLine1; iLine++ ) {
		sLine = __xuiTerminalLogicalLineCopy(pData, iLine);
		if ( sLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		iLen = (int)strlen(sLine);
		iStartColumn = (iLine == iLine0) ? iColumn0 : 0;
		iEndColumn = (iLine == iLine1) ? iColumn1 :
			__xuiTerminalUtf8DisplayColumns(sLine, iLen);
		if ( iStartColumn < 0 ) iStartColumn = 0;
		if ( iEndColumn < 0 ) iEndColumn = 0;
		iStart = __xuiTerminalUtf8ByteFromColumn(sLine, iStartColumn);
		iEnd = __xuiTerminalUtf8ByteFromColumn(sLine, iEndColumn);
		if ( iEnd < iStart ) iEnd = iStart;
		iNeeded += iEnd - iStart;
		if ( sBuffer != NULL && iCapacity > 0 && iOffset < iCapacity - 1 ) {
			iCopy = __xuiTerminalMin(iEnd - iStart, iCapacity - 1 - iOffset);
			if ( iCopy > 0 ) memcpy(sBuffer + iOffset, sLine + iStart, (size_t)iCopy);
			iOffset += iCopy;
			sBuffer[iOffset] = '\0';
		}
		xrtFree(sLine);
		if ( iLine < iLine1 ) {
			iNeeded++;
			if ( sBuffer != NULL && iCapacity > 0 && iOffset < iCapacity - 1 ) {
				sBuffer[iOffset++] = '\n';
				sBuffer[iOffset] = '\0';
			}
		}
	}
	return iNeeded;
}

static int __xuiTerminalCopySingleLineSelectionText(xui_terminal_data_t* pData, char** ppText)
{
	char* sText;
	int iNeeded;
	int iRet;

	if ( ppText != NULL ) *ppText = NULL;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeeded = __xuiTerminalSelectionTextToBuffer(pData, NULL, 0);
	if ( iNeeded <= 0 ) return iNeeded;
	sText = (char*)xrtMalloc((size_t)iNeeded + 1u);
	if ( sText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = __xuiTerminalSelectionTextToBuffer(pData, sText, iNeeded + 1);
	if ( iRet <= 0 ) {
		xrtFree(sText);
		return iRet;
	}
	if ( strchr(sText, '\n') != NULL || strchr(sText, '\r') != NULL ) {
		xrtFree(sText);
		return 0;
	}
	if ( ppText != NULL ) {
		*ppText = sText;
	} else {
		xrtFree(sText);
	}
	return 1;
}

static int __xuiTerminalSelectWordAt(xui_widget pWidget, xui_terminal_data_t* pData, int iLine, int iColumn)
{
	char* sLine;
	int iLen;
	int iByte;
	int iStart;
	int iEnd;
	int iStartColumn;
	int iEndColumn;
	xui_internal_word_kind_t iKind;

	(void)pWidget;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sLine = __xuiTerminalLogicalLineCopy(pData, iLine);
	if ( sLine == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iLen = (int)strlen(sLine);
	if ( iLen <= 0 ) {
		__xuiTerminalSetSelectionRange(pData, iLine, 0, iLine, 0);
		xrtFree(sLine);
		return XUI_OK;
	}
	if ( iColumn < 0 ) iColumn = 0;
	iByte = __xuiTerminalUtf8ByteFromColumn(sLine, iColumn);
	if ( iByte >= iLen ) iByte = xuiInternalTextGraphemePrev(sLine, iLen, iLen);
	iKind = xuiInternalTextWordRange(sLine, iLen, iByte,
		XUI_INTERNAL_WORD_TERMINAL, &iStart, &iEnd);
	if ( iKind == XUI_INTERNAL_WORD_SPACE ) {
		__xuiTerminalSetSelectionRange(pData, iLine, iColumn, iLine, iColumn);
		xrtFree(sLine);
		return XUI_OK;
	}
	iStartColumn = __xuiTerminalUtf8DisplayColumns(sLine, iStart);
	iEndColumn = __xuiTerminalUtf8DisplayColumns(sLine, iEnd);
	__xuiTerminalSetSelectionRange(pData, iLine, iStartColumn, iLine, iEndColumn);
	xrtFree(sLine);
	return XUI_OK;
}

static int __xuiTerminalSelectLineAt(xui_widget pWidget, xui_terminal_data_t* pData, int iLine)
{
	(void)pWidget;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalSetSelectionRange(pData, iLine, 0, iLine, pData->iColumns);
	return XUI_OK;
}

static int __xuiTerminalEmitInput(xui_widget pWidget, xui_terminal_data_t* pData, const uint8_t* pBytes, int iSize)
{
	if ( (pWidget == NULL) || (pData == NULL) || (pBytes == NULL) || (iSize < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iSize == 0 ) return XUI_OK;
	__xuiTerminalSyncScrollModel(pWidget, pData);
	if ( __xuiTerminalScrollToBottom(pData) ) {
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( pData->onData != NULL ) {
		pData->onData(pWidget, pBytes, iSize, pData->pDataUser);
	}
	if ( pData->pSession != NULL ) {
		return xuiTerminalSessionWrite(pData->pSession, pBytes, iSize);
	}
	return XUI_OK;
}

static int __xuiTerminalEmitInputMaybeAlt(xui_widget pWidget, xui_terminal_data_t* pData, const uint8_t* pBytes, int iSize, uint32_t iModifiers)
{
	uint8_t arrBytes[32];

	if ( (iModifiers & XUI_MOD_ALT) == 0u ) {
		return __xuiTerminalEmitInput(pWidget, pData, pBytes, iSize);
	}
	if ( iSize < 0 || iSize + 1 > (int)sizeof(arrBytes) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	arrBytes[0] = 0x1bu;
	if ( iSize > 0 ) {
		memcpy(arrBytes + 1, pBytes, (size_t)iSize);
	}
	return __xuiTerminalEmitInput(pWidget, pData, arrBytes, iSize + 1);
}

static int __xuiTerminalInputModifierCode(uint32_t iModifiers)
{
	int iCode;

	iCode = 1;
	if ( (iModifiers & XUI_MOD_SHIFT) != 0u ) iCode += 1;
	if ( (iModifiers & XUI_MOD_ALT) != 0u ) iCode += 2;
	if ( (iModifiers & XUI_MOD_CTRL) != 0u ) iCode += 4;
	return iCode;
}

static int __xuiTerminalBuildModifiedCursor(char* sSeq, int iCapacity, char cFinal, uint32_t iModifiers)
{
	int iCode;

	if ( sSeq == NULL || iCapacity <= 0 ) return 0;
	iCode = __xuiTerminalInputModifierCode(iModifiers);
	if ( iCode <= 1 ) {
		if ( iCapacity < 4 ) return 0;
		sSeq[0] = 0x1b;
		sSeq[1] = '[';
		sSeq[2] = cFinal;
		return 3;
	}
	return snprintf(sSeq, (size_t)iCapacity, "\x1b[1;%d%c", iCode, cFinal);
}

static int __xuiTerminalBuildModifiedTilde(char* sSeq, int iCapacity, int iPrefix, uint32_t iModifiers)
{
	int iCode;

	if ( sSeq == NULL || iCapacity <= 0 ) return 0;
	iCode = __xuiTerminalInputModifierCode(iModifiers);
	if ( iCode <= 1 ) return snprintf(sSeq, (size_t)iCapacity, "\x1b[%d~", iPrefix);
	return snprintf(sSeq, (size_t)iCapacity, "\x1b[%d;%d~", iPrefix, iCode);
}

static int __xuiTerminalPasteClipboard(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_proxy pProxy;
	char sSmall[4096];
	char* sText;
	int iLength;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy == NULL) || (pProxy->clipboardGetText == NULL) ) return XUI_ERROR_UNSUPPORTED;
	memset(sSmall, 0, sizeof(sSmall));
	iLength = pProxy->clipboardGetText(pProxy, sSmall, (int)sizeof(sSmall));
	if ( iLength < 0 ) return iLength;
	if ( iLength == 0 ) return XUI_OK;
	if ( iLength < (int)sizeof(sSmall) ) {
		return xuiTerminalPasteText(pWidget, sSmall);
	}
	sText = (char*)xrtMalloc((size_t)iLength + 1u);
	if ( sText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	iRet = pProxy->clipboardGetText(pProxy, sText, iLength + 1);
	if ( iRet >= 0 ) {
		iRet = xuiTerminalPasteText(pWidget, sText);
	}
	xrtFree(sText);
	return iRet;
}

static int __xuiTerminalKeyDown(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	char sSeq[16];
	uint8_t b;
	int iSize;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) {
		return XUI_OK;
	}
	iSize = 0;
	if ( (pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_SHIFT | XUI_MOD_ALT)) ==
	     (XUI_MOD_CTRL | XUI_MOD_SHIFT) &&
	     (pEvent->iKey == 'F' || pEvent->iKey == 'f') ) {
		return __xuiTerminalOpenFindWindow(pWidget, pData) | XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_F3 ) {
		if ( pData->sSearchText == NULL || pData->sSearchText[0] == '\0' ) {
			return __xuiTerminalOpenFindWindow(pWidget, pData) | XUI_EVENT_DISPATCH_STOP;
		}
		if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
			return xuiTerminalFindPrev(pWidget, pData->sSearchText, pData->iSearchFlags,
				NULL, NULL) | XUI_EVENT_DISPATCH_STOP;
		}
		return xuiTerminalFindNext(pWidget, pData->sSearchText, pData->iSearchFlags,
			NULL, NULL) | XUI_EVENT_DISPATCH_STOP;
	}
	switch ( pEvent->iKey ) {
	case XUI_KEY_ENTER:
		sSeq[0] = '\r'; iSize = 1; break;
	case XUI_KEY_ESCAPE:
		sSeq[0] = '\x1b'; iSize = 1; break;
	case XUI_KEY_BACKSPACE:
		b = ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ? 0x08u : 0x7fu;
		return __xuiTerminalEmitInputMaybeAlt(pWidget, pData, &b, 1, pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_TAB:
		if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
			memcpy(sSeq, "\x1b[Z", 3); iSize = 3;
		} else {
			sSeq[0] = '\t'; iSize = 1;
		}
		break;
	case XUI_KEY_UP:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'A', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOA" : "\x1b[A", 3); iSize = 3; break;
	case XUI_KEY_DOWN:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'B', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOB" : "\x1b[B", 3); iSize = 3; break;
	case XUI_KEY_RIGHT:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'C', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOC" : "\x1b[C", 3); iSize = 3; break;
	case XUI_KEY_LEFT:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'D', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOD" : "\x1b[D", 3); iSize = 3; break;
	case XUI_KEY_HOME:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'H', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOH" : "\x1b[H", 3); iSize = 3; break;
	case XUI_KEY_END:
		if ( pEvent->iModifiers != 0u ) {
			iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), 'F', pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		memcpy(sSeq, pData->bApplicationCursor ? "\x1bOF" : "\x1b[F", 3); iSize = 3; break;
	case XUI_KEY_INSERT:
		iSize = __xuiTerminalBuildModifiedTilde(sSeq, (int)sizeof(sSeq), 2, pEvent->iModifiers);
		return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_DELETE:
		iSize = __xuiTerminalBuildModifiedTilde(sSeq, (int)sizeof(sSeq), 3, pEvent->iModifiers);
		return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_UP:
		if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
			(void)xuiScrollModelScrollBy(&pData->tScroll, 0.0f, -(float)pData->iRows * pData->fCellHeight);
			__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return XUI_EVENT_DISPATCH_STOP;
		}
		iSize = __xuiTerminalBuildModifiedTilde(sSeq, (int)sizeof(sSeq), 5, pEvent->iModifiers);
		return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
	case XUI_KEY_PAGE_DOWN:
		if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
			(void)xuiScrollModelScrollBy(&pData->tScroll, 0.0f, (float)pData->iRows * pData->fCellHeight);
			__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return XUI_EVENT_DISPATCH_STOP;
		}
		iSize = __xuiTerminalBuildModifiedTilde(sSeq, (int)sizeof(sSeq), 6, pEvent->iModifiers);
		return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
	default:
		if ( pEvent->iKey >= XUI_KEY_F1 && pEvent->iKey <= XUI_KEY_F4 ) {
			static const char arrFinal[4] = {'P', 'Q', 'R', 'S'};
			if ( pEvent->iModifiers == 0u ) {
				sSeq[0] = '\x1b'; sSeq[1] = 'O'; sSeq[2] = arrFinal[pEvent->iKey - XUI_KEY_F1]; iSize = 3;
			} else {
				iSize = __xuiTerminalBuildModifiedCursor(sSeq, (int)sizeof(sSeq), arrFinal[pEvent->iKey - XUI_KEY_F1], pEvent->iModifiers);
				return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
			}
			break;
		}
		if ( pEvent->iKey >= XUI_KEY_F5 && pEvent->iKey <= XUI_KEY_F12 ) {
			static const int arrTilde[8] = {15, 17, 18, 19, 20, 21, 23, 24};
			iSize = __xuiTerminalBuildModifiedTilde(sSeq, (int)sizeof(sSeq), arrTilde[pEvent->iKey - XUI_KEY_F5], pEvent->iModifiers);
			return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSeq, iSize) | XUI_EVENT_DISPATCH_STOP;
		}
		if ( ((pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_SHIFT)) == (XUI_MOD_CTRL | XUI_MOD_SHIFT)) &&
		     (pEvent->iKey == 'C' || pEvent->iKey == 'c') ) {
			return xuiTerminalCopySelection(pWidget) | XUI_EVENT_DISPATCH_STOP;
		}
		if ( ((pEvent->iModifiers & (XUI_MOD_CTRL | XUI_MOD_SHIFT)) == (XUI_MOD_CTRL | XUI_MOD_SHIFT)) &&
		     (pEvent->iKey == 'V' || pEvent->iKey == 'v') ) {
			return __xuiTerminalPasteClipboard(pWidget, pData) | XUI_EVENT_DISPATCH_STOP;
		}
		if ( ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) && pEvent->iKey >= 'A' && pEvent->iKey <= 'Z' ) {
			b = (uint8_t)(pEvent->iKey - 'A' + 1);
			return __xuiTerminalEmitInputMaybeAlt(pWidget, pData, &b, 1, pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
		}
		if ( ((pEvent->iModifiers & XUI_MOD_ALT) != 0u) && pEvent->iKey >= 0x20 && pEvent->iKey <= 0x7e ) {
			b = (uint8_t)pEvent->iKey;
			if ( b >= 'A' && b <= 'Z' && (pEvent->iModifiers & XUI_MOD_SHIFT) == 0u ) {
				b = (uint8_t)(b - 'A' + 'a');
			}
			return __xuiTerminalEmitInputMaybeAlt(pWidget, pData, &b, 1, pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
		}
		break;
	}
	if ( iSize > 0 ) {
		return __xuiTerminalEmitInputMaybeAlt(pWidget, pData, (const uint8_t*)sSeq, iSize, pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiTerminalMouseButtonCode(int iButton)
{
	if ( iButton == XUI_POINTER_BUTTON_LEFT ) return 0;
	if ( iButton == XUI_POINTER_BUTTON_MIDDLE ) return 1;
	if ( iButton == XUI_POINTER_BUTTON_RIGHT ) return 2;
	return 3;
}

static int __xuiTerminalMouseModifierBits(uint32_t iModifiers)
{
	int iBits = 0;
	if ( (iModifiers & XUI_MOD_SHIFT) != 0u ) iBits |= 4;
	if ( (iModifiers & XUI_MOD_ALT) != 0u ) iBits |= 8;
	if ( (iModifiers & XUI_MOD_CTRL) != 0u ) iBits |= 16;
	return iBits;
}

static int __xuiTerminalPointerToApplicationCell(xui_widget pWidget,
	xui_terminal_data_t* pData, const xui_event_t* pEvent, int* pColumn, int* pRow)
{
	int iLine;
	int iColumn;
	if ( __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) != XUI_OK ) return 0;
	if ( iLine < pData->iScrollbackCount || iLine >= pData->iScrollbackCount + pData->iRows ) return 0;
	if ( pColumn != NULL ) *pColumn = iColumn + 1;
	if ( pRow != NULL ) *pRow = iLine - pData->iScrollbackCount + 1;
	return 1;
}

static int __xuiTerminalEmitMouse(xui_widget pWidget, xui_terminal_data_t* pData,
	int iCode, int iColumn, int iRow, int bRelease, int bMotion, uint32_t iModifiers)
{
	char sSequence[64];
	char sUtf[4];
	int iSize;
	int n;
	iCode += __xuiTerminalMouseModifierBits(iModifiers);
	if ( bMotion ) iCode += 32;
	if ( pData->iMouseEncoding == XUI_TERMINAL_MOUSE_ENCODING_SGR ) {
		iSize = snprintf(sSequence, sizeof(sSequence), "\x1b[<%d;%d;%d%c",
			iCode, iColumn, iRow, bRelease ? 'm' : 'M');
		return iSize > 0 ? __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSequence, iSize) : XUI_OK;
	}
	if ( pData->iMouseEncoding == XUI_TERMINAL_MOUSE_ENCODING_URXVT ) {
		if ( bRelease ) iCode = 3 + __xuiTerminalMouseModifierBits(iModifiers);
		iSize = snprintf(sSequence, sizeof(sSequence), "\x1b[%d;%d;%dM", iCode + 32, iColumn, iRow);
		return iSize > 0 ? __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSequence, iSize) : XUI_OK;
	}
	if ( bRelease ) iCode = 3 + __xuiTerminalMouseModifierBits(iModifiers);
	iSize = 0;
	sSequence[iSize++] = '\x1b';
	sSequence[iSize++] = '[';
	sSequence[iSize++] = 'M';
	if ( pData->iMouseEncoding == XUI_TERMINAL_MOUSE_ENCODING_UTF8 ) {
		n = __xuiTerminalEncodeUtf8((uint32_t)(iCode + 32), sUtf);
		if ( n > 0 ) { memcpy(sSequence + iSize, sUtf, (size_t)n); iSize += n; }
		n = __xuiTerminalEncodeUtf8((uint32_t)(iColumn + 32), sUtf);
		if ( n > 0 ) { memcpy(sSequence + iSize, sUtf, (size_t)n); iSize += n; }
		n = __xuiTerminalEncodeUtf8((uint32_t)(iRow + 32), sUtf);
		if ( n > 0 ) { memcpy(sSequence + iSize, sUtf, (size_t)n); iSize += n; }
	} else {
		if ( iColumn > 223 ) iColumn = 223;
		if ( iRow > 223 ) iRow = 223;
		sSequence[iSize++] = (char)(iCode + 32);
		sSequence[iSize++] = (char)(iColumn + 32);
		sSequence[iSize++] = (char)(iRow + 32);
	}
	return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sSequence, iSize);
}

static int __xuiTerminalApplicationMouseEnabled(const xui_terminal_data_t* pData,
	const xui_event_t* pEvent)
{
	return pData != NULL && pEvent != NULL &&
		pData->iMouseTracking != XUI_TERMINAL_MOUSE_TRACKING_NONE &&
		(pEvent->iModifiers & XUI_MOD_SHIFT) == 0u;
}

static int __xuiTerminalPointerDown(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iLine;
	int iColumn;
	int iRow;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	(void)xuiSetFocusWidget(pContext, pWidget);
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) ) {
		if ( !__xuiTerminalPointerToApplicationCell(pWidget, pData, pEvent, &iColumn, &iRow) ) return XUI_EVENT_DISPATCH_STOP;
		pData->iMouseButton = pEvent->iButton;
		pData->bMouseCaptured = 1;
		(void)xuiSetPointerCapture(pContext, pWidget);
		return __xuiTerminalEmitMouse(pWidget, pData,
			__xuiTerminalMouseButtonCode(pEvent->iButton), iColumn, iRow, 0, 0,
			pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) != XUI_OK ) return XUI_OK;
	if ( (pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ) {
		if ( !__xuiTerminalSelectionRange(pData, NULL, NULL, NULL, NULL) ) {
			pData->iSelectAnchorLine = __xuiTerminalClampLine(pData, pData->iScrollbackCount + pData->iCursorY);
			pData->iSelectAnchorColumn = __xuiTerminalClampColumn(pData, pData->iCursorX);
		}
		pData->bSelectAll = 0;
		pData->iSelectEndLine = iLine;
		pData->iSelectEndColumn = iColumn;
	} else {
		__xuiTerminalSetSelectionRange(pData, iLine, iColumn, iLine, iColumn);
	}
	pData->bSelecting = 1;
	(void)xuiSetPointerCapture(pContext, pWidget);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalPointerMove(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	int iLine;
	int iColumn;
	int iRow;
	int iButton;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) ) {
		if ( pData->iMouseTracking == XUI_TERMINAL_MOUSE_TRACKING_PRESS ) return XUI_EVENT_DISPATCH_STOP;
		if ( pData->iMouseTracking == XUI_TERMINAL_MOUSE_TRACKING_DRAG && pEvent->iButtons == 0u ) return XUI_EVENT_DISPATCH_STOP;
		if ( !__xuiTerminalPointerToApplicationCell(pWidget, pData, pEvent, &iColumn, &iRow) ) return XUI_EVENT_DISPATCH_STOP;
		iButton = pData->iMouseButton;
		if ( (pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) != 0u ) iButton = XUI_POINTER_BUTTON_LEFT;
		else if ( (pEvent->iButtons & XUI_POINTER_BUTTON_MIDDLE) != 0u ) iButton = XUI_POINTER_BUTTON_MIDDLE;
		else if ( (pEvent->iButtons & XUI_POINTER_BUTTON_RIGHT) != 0u ) iButton = XUI_POINTER_BUTTON_RIGHT;
		else iButton = 0;
		return __xuiTerminalEmitMouse(pWidget, pData,
			iButton != 0 ? __xuiTerminalMouseButtonCode(iButton) : 3,
			iColumn, iRow, 0, 1, pEvent->iModifiers) | XUI_EVENT_DISPATCH_STOP;
	}
	if ( !pData->bSelecting ) {
		return __xuiTerminalUpdateHoverLink(pWidget, pData, pEvent);
	}
	if ( (pEvent->iButtons & XUI_POINTER_BUTTON_LEFT) == 0u ) {
		pData->bSelecting = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		}
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) == XUI_OK ) {
		pData->iSelectEndLine = iLine;
		pData->iSelectEndColumn = iColumn;
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalPointerUp(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	int iLine;
	int iColumn;
	int iRow;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) || pData->bMouseCaptured ) {
		int iRet = XUI_OK;
		if ( __xuiTerminalPointerToApplicationCell(pWidget, pData, pEvent, &iColumn, &iRow) ) {
			iRet = __xuiTerminalEmitMouse(pWidget, pData,
				__xuiTerminalMouseButtonCode(pEvent->iButton), iColumn, iRow, 1, 0,
				pEvent->iModifiers);
		}
		pData->bMouseCaptured = 0;
		pData->iMouseButton = 0;
		if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
			(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
		}
		return iRet | XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( pData->bSelecting && __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) == XUI_OK ) {
		pData->iSelectEndLine = iLine;
		pData->iSelectEndColumn = iColumn;
	}
	pData->bSelecting = 0;
	__xuiTerminalEmitSelectionChange(pWidget, pData);
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalPointerDoubleClick(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	int iLine;
	int iColumn;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) != XUI_OK ) return XUI_EVENT_DISPATCH_STOP;
	iRet = __xuiTerminalSelectWordAt(pWidget, pData, iLine, iColumn);
	pData->bSelecting = 0;
	pData->bDoubleClickPending = 1;
	pData->iDoubleClickLine = iLine;
	pData->fDoubleClickX = pEvent->fX;
	pData->fDoubleClickY = pEvent->fY;
	pData->fDoubleClickTime = xrtTimer();
	__xuiTerminalEmitSelectionChange(pWidget, pData);
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalLinkSchemeAllowed(const char* sUrl)
{
	static const char* const arrSchemes[] = { "http:", "https:", "mailto:" };
	int i;
	int j;
	unsigned char c;

	if ( sUrl == NULL || sUrl[0] == '\0' ) return 0;
	for ( i = 0; sUrl[i] != '\0'; i++ ) {
		c = (unsigned char)sUrl[i];
		if ( c <= 0x20u || c == 0x7fu ) return 0;
	}
	for ( i = 0; i < (int)(sizeof(arrSchemes) / sizeof(arrSchemes[0])); i++ ) {
		for ( j = 0; arrSchemes[i][j] != '\0'; j++ ) {
			c = (unsigned char)sUrl[j];
			if ( c >= 'A' && c <= 'Z' ) c = (unsigned char)(c + ('a' - 'A'));
			if ( c != (unsigned char)arrSchemes[i][j] ) break;
		}
		if ( arrSchemes[i][j] == '\0' ) return 1;
	}
	return 0;
}

static int __xuiTerminalPointerClick(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	char* sUrl;
	double fNow;
	float fDX;
	float fDY;
	int iLine;
	int iColumn;
	int iStartColumn;
	int iLength;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) ) return XUI_EVENT_DISPATCH_STOP;
	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	sUrl = NULL;
	iStartColumn = 0;
	iLength = 0;
	if ( pData->onLink != NULL &&
	     __xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) == XUI_OK &&
	     __xuiTerminalLogicalUrlAt(pData, iLine, iColumn, &sUrl, &iStartColumn, &iLength) > 0 &&
	     sUrl != NULL ) {
		(void)iStartColumn;
		(void)iLength;
		if ( __xuiTerminalLinkSchemeAllowed(sUrl) ) {
			pData->onLink(pWidget, sUrl, pData->pLinkUser);
		}
		xrtFree(sUrl);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( sUrl != NULL ) xrtFree(sUrl);
	if ( !pData->bDoubleClickPending || (pEvent->iClickCount != 3) ) return XUI_EVENT_DISPATCH_STOP;
	fNow = xrtTimer();
	fDX = pEvent->fX - pData->fDoubleClickX;
	fDY = pEvent->fY - pData->fDoubleClickY;
	if ( ((fNow - pData->fDoubleClickTime) <= XUI_TERMINAL_TRIPLE_CLICK_SECONDS) &&
	     ((fDX * fDX + fDY * fDY) <= (XUI_TERMINAL_TRIPLE_CLICK_DISTANCE * XUI_TERMINAL_TRIPLE_CLICK_DISTANCE)) &&
	     (__xuiTerminalPointerToCell(pWidget, pData, pEvent, &iLine, &iColumn) == XUI_OK) &&
	     (iLine == pData->iDoubleClickLine) ) {
		(void)iColumn;
		iRet = __xuiTerminalSelectLineAt(pWidget, pData, iLine);
		pData->bDoubleClickPending = 0;
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
		return XUI_EVENT_DISPATCH_STOP;
	}
	pData->bDoubleClickPending = 0;
	return XUI_EVENT_DISPATCH_STOP;
}

static uint32_t __xuiTerminalFindWindowFlags(const xui_terminal_data_t* pData)
{
	uint32_t iFlags = 0u;
	if ( pData == NULL ) return 0u;
	if ( pData->pFindCaseCheck != NULL && xuiCheckBoxGetChecked(pData->pFindCaseCheck) ) {
		iFlags |= XUI_TERMINAL_SEARCH_CASE_SENSITIVE;
	}
	if ( pData->pFindWordCheck != NULL && xuiCheckBoxGetChecked(pData->pFindWordCheck) ) {
		iFlags |= XUI_TERMINAL_SEARCH_WHOLE_WORD;
	}
	if ( pData->pFindRegexCheck != NULL && xuiCheckBoxGetChecked(pData->pFindRegexCheck) ) {
		iFlags |= XUI_TERMINAL_SEARCH_REGEX;
	}
	return iFlags;
}

static void __xuiTerminalFindWindowSetStatus(xui_terminal_data_t* pData, const char* sText)
{
	if ( pData != NULL && pData->pFindStatus != NULL ) {
		(void)xuiLabelSetText(pData->pFindStatus, sText != NULL ? sText : "");
	}
}

static int __xuiTerminalFindWindowNavigate(xui_widget pTerminal, int iDirection, int bContinue)
{
	xui_terminal_data_t* pData;
	xui_context pContext;
	const char* sPattern;
	uint32_t iFlags;
	int iRet;

	if ( pTerminal == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pTerminal);
	if ( pData == NULL || pData->pFindInput == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pTerminal);
	sPattern = xuiInputGetText(pData->pFindInput);
	if ( sPattern == NULL || sPattern[0] == '\0' ) {
		(void)xuiTerminalClearFind(pTerminal);
		__xuiTerminalFindWindowSetStatus(pData, "");
		return XUI_OK;
	}
	iFlags = __xuiTerminalFindWindowFlags(pData);
	if ( !bContinue ) iRet = xuiTerminalFindText(pTerminal, sPattern, iFlags, NULL, NULL);
	else if ( iDirection < 0 ) iRet = xuiTerminalFindPrev(pTerminal, sPattern, iFlags, NULL, NULL);
	else iRet = xuiTerminalFindNext(pTerminal, sPattern, iFlags, NULL, NULL);
	if ( iRet > 0 ) {
		__xuiTerminalFindWindowSetStatus(pData, "");
		return XUI_OK;
	}
	if ( iRet == 0 ) {
		__xuiTerminalFindWindowSetStatus(pData, xuiTranslate(pContext, XUI_TR_FIND_NOT_FOUND));
		return XUI_OK;
	}
	__xuiTerminalFindWindowSetStatus(pData, xuiTranslate(pContext, XUI_TR_FIND_INVALID_PATTERN));
	return iRet;
}

static void __xuiTerminalFindInputChange(xui_widget pWidget, const char* sText, void* pUser)
{
	(void)pWidget;
	(void)sText;
	(void)__xuiTerminalFindWindowNavigate((xui_widget)pUser, 1, 0);
}

static void __xuiTerminalFindCheckChange(xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)bChecked;
	(void)__xuiTerminalFindWindowNavigate((xui_widget)pUser, 1, 0);
}

static void __xuiTerminalFindButtonClick(xui_widget pButton, void* pUser)
{
	xui_widget pTerminal = (xui_widget)pUser;
	xui_terminal_data_t* pData;
	if ( pTerminal == NULL ) return;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pTerminal);
	if ( pData == NULL ) return;
	(void)__xuiTerminalFindWindowNavigate(pTerminal,
		pButton == pData->pFindPrevButton ? -1 : 1, 1);
}

static void __xuiTerminalFindWindowClose(xui_widget pWindow, void* pUser)
{
	xui_widget pTerminal = (xui_widget)pUser;
	(void)pWindow;
	if ( pTerminal != NULL ) (void)xuiSetFocusWidget(xuiWidgetGetContext(pTerminal), pTerminal);
}

static int __xuiTerminalFindWindowKeyDown(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pTerminal = (xui_widget)pUser;
	xui_terminal_data_t* pData;
	(void)pWidget;
	if ( pTerminal == NULL || pEvent == NULL || pEvent->iType != XUI_EVENT_KEY_DOWN ) return XUI_OK;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pTerminal);
	if ( pData == NULL || pData->pFindWindow == NULL ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_ENTER || pEvent->iKey == XUI_KEY_F3 ) {
		(void)__xuiTerminalFindWindowNavigate(pTerminal,
			(pEvent->iModifiers & XUI_MOD_SHIFT) != 0u ? -1 : 1, 1);
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		(void)xuiWindowSetOpen(pData->pFindWindow, 0);
		(void)xuiSetFocusWidget(xuiWidgetGetContext(pTerminal), pTerminal);
		return XUI_EVENT_DISPATCH_STOP;
	}
	return XUI_OK;
}

static int __xuiTerminalCreateFindButton(xui_context pContext, xui_widget* ppButton,
	xui_font pFont, const char* sText, xui_widget pTerminal)
{
	xui_button_desc_t tDesc;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = sText;
	tDesc.fBorderWidth = 1.0f;
	iRet = xuiButtonCreate(pContext, ppButton, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(*ppButton, __xuiTerminalFindButtonClick, pTerminal);
	return iRet;
}

static int __xuiTerminalCreateFindCheck(xui_context pContext, xui_widget* ppCheck,
	xui_font pFont, const char* sText, xui_widget pTerminal)
{
	xui_checkbox_desc_t tDesc;
	int iRet;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sText = sText;
	tDesc.fIndicatorSize = 14.0f;
	tDesc.fGap = 4.0f;
	iRet = xuiCheckBoxCreate(pContext, ppCheck, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiCheckBoxSetChange(*ppCheck, __xuiTerminalFindCheckChange, pTerminal);
	return iRet;
}

static int __xuiTerminalCreateFindWindow(xui_widget pTerminal, xui_terminal_data_t* pData)
{
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pClient;
	xui_window_desc_t tWindow;
	xui_input_desc_t tInput;
	xui_label_desc_t tLabel;
	xui_font pFont;
	xui_widget arrChildren[8];
	int iRet;
	int i;

	if ( pTerminal == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pTerminal);
	pRoot = xuiGetRootWidget(pContext);
	if ( pRoot == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pFont = pData->pFont != NULL ? pData->pFont : xuiGetDefaultFont(pContext);
	memset(&tWindow, 0, sizeof(tWindow));
	tWindow.iSize = sizeof(tWindow);
	tWindow.sTitle = xuiTranslate(pContext, XUI_TR_FIND_TITLE);
	tWindow.pFont = pFont;
	tWindow.bClosed = 1;
	tWindow.bTopMost = 1;
	tWindow.bHideCollapse = 1;
	tWindow.bHideMaximize = 1;
	tWindow.bNotResizable = 1;
	tWindow.fTitleBarHeight = 28.0f;
	tWindow.fBorderWidth = 1.0f;
	tWindow.fButtonSize = 18.0f;
	iRet = xuiWindowCreate(pContext, &pData->pFindWindow, &tWindow);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWindowSetClose(pData->pFindWindow, __xuiTerminalFindWindowClose, pTerminal);
	pClient = xuiWindowGetClientWidget(pData->pFindWindow);
	(void)xuiWidgetSetLayoutType(pClient, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetFlowMode(pClient, XUI_FLOW_ABSOLUTE);
	(void)xuiWidgetSetPadding(pClient, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	(void)xuiWidgetSetGap(pClient, 0.0f);
	(void)xuiWidgetSetEventHandler(pClient, XUI_EVENT_KEY_DOWN,
		__xuiTerminalFindWindowKeyDown, pTerminal);
	memset(&tInput, 0, sizeof(tInput));
	tInput.iSize = sizeof(tInput);
	tInput.pFont = pFont;
	tInput.sPlaceholder = xuiTranslate(pContext, XUI_TR_FIND_PLACEHOLDER);
	tInput.fBorderWidth = 1.0f;
	iRet = xuiInputCreate(pContext, &pData->pFindInput, &tInput);
	if ( iRet == XUI_OK ) iRet = __xuiTerminalCreateFindButton(pContext, &pData->pFindPrevButton,
		pFont, xuiTranslate(pContext, XUI_TR_FIND_PREVIOUS), pTerminal);
	if ( iRet == XUI_OK ) iRet = __xuiTerminalCreateFindButton(pContext, &pData->pFindNextButton,
		pFont, xuiTranslate(pContext, XUI_TR_FIND_NEXT), pTerminal);
	if ( iRet == XUI_OK ) iRet = __xuiTerminalCreateFindCheck(pContext, &pData->pFindCaseCheck,
		pFont, xuiTranslate(pContext, XUI_TR_FIND_CASE), pTerminal);
	if ( iRet == XUI_OK ) iRet = __xuiTerminalCreateFindCheck(pContext, &pData->pFindWordCheck,
		pFont, xuiTranslate(pContext, XUI_TR_FIND_WORD), pTerminal);
	if ( iRet == XUI_OK ) iRet = __xuiTerminalCreateFindCheck(pContext, &pData->pFindRegexCheck,
		pFont, xuiTranslate(pContext, XUI_TR_FIND_REGEX), pTerminal);
	memset(&tLabel, 0, sizeof(tLabel));
	tLabel.iSize = sizeof(tLabel);
	tLabel.pFont = pFont;
	tLabel.sText = "";
	tLabel.iTextColor = XUI_COLOR_RGBA(90, 105, 124, 255);
	tLabel.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	if ( iRet == XUI_OK ) iRet = xuiLabelCreate(pContext, &pData->pFindStatus, &tLabel);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiInputSetChange(pData->pFindInput, __xuiTerminalFindInputChange, pTerminal);
	arrChildren[0] = pData->pFindInput;
	arrChildren[1] = pData->pFindPrevButton;
	arrChildren[2] = pData->pFindNextButton;
	arrChildren[3] = pData->pFindCaseCheck;
	arrChildren[4] = pData->pFindWordCheck;
	arrChildren[5] = pData->pFindRegexCheck;
	arrChildren[6] = pData->pFindStatus;
	for ( i = 0; i < 7 && iRet == XUI_OK; ++i ) {
		iRet = xuiWindowAddChild(pData->pFindWindow, arrChildren[i]);
	}
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pData->pFindWindow);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pData->pFindWindow, (xui_rect_t){32.0f, 32.0f, 548.0f, 132.0f});
	(void)xuiWidgetSetRect(pData->pFindInput, (xui_rect_t){12.0f, 12.0f, 300.0f, 28.0f});
	(void)xuiWidgetSetRect(pData->pFindPrevButton, (xui_rect_t){320.0f, 12.0f, 92.0f, 28.0f});
	(void)xuiWidgetSetRect(pData->pFindNextButton, (xui_rect_t){420.0f, 12.0f, 92.0f, 28.0f});
	(void)xuiWidgetSetRect(pData->pFindCaseCheck, (xui_rect_t){12.0f, 50.0f, 150.0f, 24.0f});
	(void)xuiWidgetSetRect(pData->pFindWordCheck, (xui_rect_t){170.0f, 50.0f, 150.0f, 24.0f});
	(void)xuiWidgetSetRect(pData->pFindRegexCheck, (xui_rect_t){328.0f, 50.0f, 100.0f, 24.0f});
	(void)xuiWidgetSetRect(pData->pFindStatus, (xui_rect_t){12.0f, 78.0f, 500.0f, 22.0f});
	return XUI_OK;
}

static int __xuiTerminalOpenFindWindow(xui_widget pTerminal, xui_terminal_data_t* pData)
{
	xui_rect_t tOwner;
	xui_rect_t tWindow;
	char* sSelected = NULL;
	int iRet;

	if ( pTerminal == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFindWindow == NULL ) {
		iRet = __xuiTerminalCreateFindWindow(pTerminal, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiTerminalCopySingleLineSelectionText(pData, &sSelected);
	if ( iRet < 0 ) return iRet;
	if ( iRet > 0 && sSelected != NULL ) {
		(void)xuiInputSetText(pData->pFindInput, sSelected);
	} else if ( pData->sSearchText != NULL ) {
		(void)xuiInputSetText(pData->pFindInput, pData->sSearchText);
	}
	xrtFree(sSelected);
	(void)xuiCheckBoxSetChecked(pData->pFindCaseCheck,
		(pData->iSearchFlags & XUI_TERMINAL_SEARCH_CASE_SENSITIVE) != 0u);
	(void)xuiCheckBoxSetChecked(pData->pFindWordCheck,
		(pData->iSearchFlags & XUI_TERMINAL_SEARCH_WHOLE_WORD) != 0u);
	(void)xuiCheckBoxSetChecked(pData->pFindRegexCheck,
		(pData->iSearchFlags & XUI_TERMINAL_SEARCH_REGEX) != 0u);
	tOwner = xuiWidgetGetWorldRect(pTerminal);
	tWindow = xuiWidgetGetRect(pData->pFindWindow);
	tWindow.fX = tOwner.fX + 16.0f;
	tWindow.fY = tOwner.fY + 16.0f;
	(void)xuiWidgetSetRect(pData->pFindWindow, tWindow);
	(void)xuiWindowSetOpen(pData->pFindWindow, 1);
	(void)xuiWindowBringToFront(pData->pFindWindow);
	(void)xuiSetFocusWidget(xuiWidgetGetContext(pTerminal), pData->pFindInput);
	(void)__xuiTerminalFindWindowNavigate(pTerminal, 1, 0);
	return XUI_OK;
}

static int __xuiTerminalMenuTitleIndexForCommand(int iCommand)
{
	switch ( iCommand ) {
	case XUI_TERMINAL_MENU_COPY: return 0;
	case XUI_TERMINAL_MENU_PASTE: return 1;
	case XUI_TERMINAL_MENU_SELECT_ALL: return 2;
	case XUI_TERMINAL_MENU_CLEAR_SCREEN: return 3;
	case XUI_TERMINAL_MENU_CLEAR_SCROLLBACK: return 4;
	case XUI_TERMINAL_MENU_FIND: return 5;
	default: return -1;
	}
}

static int __xuiTerminalMenuTitleTranslationForCommand(int iCommand)
{
	switch ( iCommand ) {
	case XUI_TERMINAL_MENU_COPY: return XUI_TR_EDIT_COPY;
	case XUI_TERMINAL_MENU_PASTE: return XUI_TR_EDIT_PASTE;
	case XUI_TERMINAL_MENU_SELECT_ALL: return XUI_TR_EDIT_SELECT_ALL;
	case XUI_TERMINAL_MENU_CLEAR_SCREEN: return XUI_TR_TERMINAL_CLEAR_SCREEN;
	case XUI_TERMINAL_MENU_CLEAR_SCROLLBACK: return XUI_TR_TERMINAL_CLEAR_SCROLLBACK;
	case XUI_TERMINAL_MENU_FIND: return XUI_TR_FIND_TITLE;
	default: return 0;
	}
}

static const char* __xuiTerminalMenuTitleForCommand(xui_widget pWidget, xui_terminal_data_t* pData, int iCommand)
{
	xui_context pContext;
	int iIndex;
	int iTranslation;

	if ( pData == NULL ) return "";
	iIndex = __xuiTerminalMenuTitleIndexForCommand(iCommand);
	if ( iIndex < 0 || iIndex >= XUI_TERMINAL_MENU_TITLE_COUNT ) return "";
	if ( pData->arrMenuTitle[iIndex] != NULL ) return pData->arrMenuTitle[iIndex];
	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	iTranslation = __xuiTerminalMenuTitleTranslationForCommand(iCommand);
	return (iTranslation != 0) ? xuiTranslate(pContext, iTranslation) : "";
}

static int __xuiTerminalUpdateMenu(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_menu_item_t arrItems[8];
	xui_proxy pProxy;
	char sProbe[4];
	int bHasSelection;
	int bCanPaste;
	int bCanFind;
	uint32_t iEnabled;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(arrItems, 0, sizeof(arrItems));
	iEnabled = XUI_MENU_ITEM_ENABLED;
	bHasSelection = (__xuiTerminalSelectionTextToBuffer(pData, NULL, 0) > 0);
	bCanPaste = 0;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->clipboardGetText != NULL) ) {
		memset(sProbe, 0, sizeof(sProbe));
		bCanPaste = (pProxy->clipboardGetText(pProxy, sProbe, (int)sizeof(sProbe)) > 0);
	}
	bCanFind = 1;
	arrItems[0].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_COPY);
	arrItems[0].sShortcut = "Ctrl+Shift+C";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = bHasSelection ? iEnabled : 0u;
	arrItems[0].iValue = XUI_TERMINAL_MENU_COPY;
	arrItems[1].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_PASTE);
	arrItems[1].sShortcut = "Ctrl+Shift+V";
	arrItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[1].iState = bCanPaste ? iEnabled : 0u;
	arrItems[1].iValue = XUI_TERMINAL_MENU_PASTE;
	arrItems[2].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_SELECT_ALL);
	arrItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[2].iState = iEnabled;
	arrItems[2].iValue = XUI_TERMINAL_MENU_SELECT_ALL;
	arrItems[3].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[4].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_CLEAR_SCREEN);
	arrItems[4].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[4].iState = iEnabled;
	arrItems[4].iValue = XUI_TERMINAL_MENU_CLEAR_SCREEN;
	arrItems[5].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_CLEAR_SCROLLBACK);
	arrItems[5].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[5].iState = iEnabled;
	arrItems[5].iValue = XUI_TERMINAL_MENU_CLEAR_SCROLLBACK;
	arrItems[6].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[7].sText = __xuiTerminalMenuTitleForCommand(pWidget, pData, XUI_TERMINAL_MENU_FIND);
	arrItems[7].sShortcut = "Ctrl+Shift+F";
	arrItems[7].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[7].iState = bCanFind ? iEnabled : 0u;
	arrItems[7].iValue = XUI_TERMINAL_MENU_FIND;
	return xuiMenuSetItems(pData->pMenu, arrItems, 8);
}

static int __xuiTerminalRunFindMenuCommand(xui_widget pWidget, xui_terminal_data_t* pData)
{
	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalOpenFindWindow(pWidget, pData);
}

static void __xuiTerminalMenuSelect(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_widget pTerminal;
	xui_terminal_data_t* pData;

	(void)pMenu;
	(void)iIndex;
	pTerminal = (xui_widget)pUser;
	if ( pTerminal == NULL ) return;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pTerminal);
	if ( pData == NULL ) return;
	(void)xuiSetFocusWidget(xuiWidgetGetContext(pTerminal), pTerminal);
	switch ( iValue ) {
	case XUI_TERMINAL_MENU_COPY:
		(void)xuiTerminalCopySelection(pTerminal);
		break;
	case XUI_TERMINAL_MENU_PASTE:
		(void)__xuiTerminalPasteClipboard(pTerminal, pData);
		break;
	case XUI_TERMINAL_MENU_SELECT_ALL:
		(void)xuiTerminalSelectAll(pTerminal);
		break;
	case XUI_TERMINAL_MENU_CLEAR_SCREEN:
		(void)xuiTerminalClear(pTerminal);
		break;
	case XUI_TERMINAL_MENU_CLEAR_SCROLLBACK:
		(void)xuiTerminalClearScrollback(pTerminal);
		break;
	case XUI_TERMINAL_MENU_FIND:
		(void)__xuiTerminalRunFindMenuCommand(pTerminal, pData);
		break;
	default:
		break;
	}
}

static int __xuiTerminalContextMenu(xui_widget pWidget, xui_terminal_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tContent;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iOffsetY;

	if ( (pWidget == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) && pEvent->iKey != XUI_KEY_CONTEXT_MENU ) return XUI_EVENT_DISPATCH_STOP;
	(void)xuiSetFocusWidget(xuiWidgetGetContext(pWidget), pWidget);
	pData->bSelecting = 0;
	if ( xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( pEvent->iKey == XUI_KEY_CONTEXT_MENU ) {
		__xuiTerminalResolveStyle(pWidget, pData);
		tWorld = xuiWidgetGetWorldRect(pWidget);
		tContent = xuiWidgetGetContentRect(pWidget);
		iOffsetY = __xuiTerminalScrollOffsetPixels(pData);
		fX = tWorld.fX + tContent.fX + pData->fPadding + (float)pData->iCursorX * pData->fCellWidth;
		fY = tWorld.fY + tContent.fY + pData->fPadding +
			(float)(pData->iCursorY + pData->iScrollbackCount + 1) * pData->fCellHeight - (float)iOffsetY;
		if ( fX < tWorld.fX ) fX = tWorld.fX;
		if ( fY < tWorld.fY ) fY = tWorld.fY;
		if ( fX > tWorld.fX + tWorld.fW ) fX = tWorld.fX + tWorld.fW;
		if ( fY > tWorld.fY + tWorld.fH ) fY = tWorld.fY + tWorld.fH;
	} else {
		fX = pEvent->fX;
		fY = pEvent->fY;
	}
	(void)xuiTerminalOpenMenu(pWidget, fX, fY);
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalPointerWheel(xui_widget pWidget, xui_terminal_data_t* pData,
	const xui_event_t* pEvent)
{
	float fDelta;
	int iCode;
	int iColumn;
	int iRow;
	int iSteps;
	int i;
	int iRet = XUI_OK;

	if ( pWidget == NULL || pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE ) return XUI_OK;
	if ( __xuiTerminalApplicationMouseEnabled(pData, pEvent) ) {
		if ( !__xuiTerminalPointerToApplicationCell(pWidget, pData, pEvent, &iColumn, &iRow) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		if ( pEvent->fWheelY != 0.0f ) {
			fDelta = pEvent->fWheelY;
			iCode = fDelta > 0.0f ? 64 : 65;
		} else if ( pEvent->fWheelX != 0.0f ) {
			fDelta = pEvent->fWheelX;
			iCode = fDelta > 0.0f ? 66 : 67;
		} else {
			return XUI_EVENT_DISPATCH_STOP;
		}
		iSteps = (int)(fDelta < 0.0f ? -fDelta : fDelta);
		if ( iSteps < 1 ) iSteps = 1;
		for ( i = 0; i < iSteps; ++i ) {
			iRet = __xuiTerminalEmitMouse(pWidget, pData, iCode,
				iColumn, iRow, 0, 0, pEvent->iModifiers);
			if ( iRet != XUI_OK ) break;
		}
		return iRet | XUI_EVENT_DISPATCH_STOP;
	}
	__xuiTerminalResolveStyle(pWidget, pData);
	(void)xuiScrollModelScrollBy(&pData->tScroll, 0.0f,
		-pEvent->fWheelY * pData->fCellHeight * 3.0f);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTerminalEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_terminal_data_t* pData;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_OK;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	if ( pData == NULL ) return XUI_OK;
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_LEAVE:
		if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) {
			return __xuiTerminalSetHoverLink(pWidget, pData, -1, 0, 0, NULL);
		}
		break;
	case XUI_EVENT_POINTER_DOWN:
		return __xuiTerminalPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_MOVE:
		return __xuiTerminalPointerMove(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_UP:
		return __xuiTerminalPointerUp(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		return __xuiTerminalPointerClick(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_DOUBLE_CLICK:
		return __xuiTerminalPointerDoubleClick(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CAPTURE_LOST:
		pData->bSelecting = 0;
		pData->bMouseCaptured = 0;
		pData->iMouseButton = 0;
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_CONTEXT_MENU:
		return __xuiTerminalContextMenu(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_WHEEL:
		return __xuiTerminalPointerWheel(pWidget, pData, pEvent);
	case XUI_EVENT_TEXT:
		if ( pEvent->iPhase != XUI_EVENT_PHASE_BUBBLE ) {
			if ( pData->bImeActive ) return XUI_EVENT_DISPATCH_STOP;
			if ( pEvent->iTextSize > 0 ) {
				return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)pEvent->sText, pEvent->iTextSize) | XUI_EVENT_DISPATCH_STOP;
			}
			if ( pEvent->iCodepoint != 0u ) {
				char sUtf8[4];
				int iSize = __xuiTerminalEncodeUtf8(pEvent->iCodepoint, sUtf8);
				if ( iSize > 0 ) {
					return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sUtf8, iSize) | XUI_EVENT_DISPATCH_STOP;
				}
			}
		}
		break;
	case XUI_EVENT_IME_COMPOSITION:
		if ( pEvent->bCompositionActive ) {
			if ( __xuiTerminalImeSetText(pData, pEvent->sText, pEvent->iTextSize) != XUI_OK ) {
				return XUI_EVENT_DISPATCH_STOP;
			}
			pData->bImeActive = 1;
			(void)xuiInternalEditEmitSized(pWidget, XUI_EDIT_EVENT_COMPOSITION_CHANGED,
				pData->sImeText, pEvent->iTextSize, 0, 0, 0, pEvent->iTextSize, 1);
			__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
			return XUI_EVENT_DISPATCH_STOP;
		}
		pData->bImeActive = 0;
		(void)__xuiTerminalImeSetText(pData, NULL, 0);
		if ( pEvent->iTextSize > 0 ) {
			(void)__xuiTerminalEmitInput(pWidget, pData,
				(const uint8_t*)pEvent->sText, pEvent->iTextSize);
		}
		(void)xuiInternalEditEmitSized(pWidget, XUI_EDIT_EVENT_COMPOSITION_CHANGED,
			NULL, 0, 0, 0, 0, 0, 1);
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		return XUI_EVENT_DISPATCH_STOP;
	case XUI_EVENT_KEY_DOWN:
		return __xuiTerminalKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
	case XUI_EVENT_BOUNDS_CHANGED:
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( pEvent->iType == XUI_EVENT_BOUNDS_CHANGED ) {
			(void)xuiTerminalFit(pWidget);
		}
		if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE && pData->bFocusReporting ) {
			static const uint8_t sFocusIn[] = "\x1b[I";
			static const uint8_t sFocusOut[] = "\x1b[O";
			if ( pEvent->iType == XUI_EVENT_FOCUS ) {
				(void)__xuiTerminalEmitInput(pWidget, pData, sFocusIn, (int)sizeof(sFocusIn) - 1);
			} else if ( pEvent->iType == XUI_EVENT_BLUR ) {
				(void)__xuiTerminalEmitInput(pWidget, pData, sFocusOut, (int)sizeof(sFocusOut) - 1);
			}
		}
		if ( pEvent->iType == XUI_EVENT_BLUR ) {
			pData->bImeActive = 0;
			pData->bSelecting = 0;
			pData->bMouseCaptured = 0;
			pData->iMouseButton = 0;
			(void)__xuiTerminalImeSetText(pData, NULL, 0);
		}
		__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		break;
	default:
		break;
	}
	return XUI_OK;
}

static xui_rect_t __xuiTerminalImeRect(xui_widget pWidget, void* pUser)
{
	xui_rect_t tRect = __xuiTerminalEditCaretRect(pWidget);
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pWidget);
	(void)pUser;
	tRect.fX += tWorld.fX;
	tRect.fY += tWorld.fY;
	return tRect;
}

static int __xuiTerminalInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_UP, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOUBLE_CLICK, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_CONTEXT_MENU, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_WHEEL, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_TEXT, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_IME_COMPOSITION, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiTerminalEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiTerminalEvent, NULL);
	return iRet;
}

static int __xuiTerminalInitMenu(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_menu_desc_t tDesc;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.pFont = pData->pFont;
	iRet = xuiMenuCreate(xuiWidgetGetContext(pWidget), &pData->pMenu, &tDesc);
	if ( iRet != XUI_OK ) {
		pData->pMenu = NULL;
		return iRet;
	}
	iRet = xuiMenuSetSelect(pData->pMenu, __xuiTerminalMenuSelect, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTerminalUpdateMenu(pWidget, pData);
}

static int __xuiTerminalFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiTerminalStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pColor == NULL) ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiTerminalStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pValue == NULL) ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) && __xuiTerminalFloatValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiTerminalStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	if ( pWidget == NULL ) return pBaseFont;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) &&
	     (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) &&
	     (tProperty.tValue.sText != NULL) ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) return pFont;
	}
	return pBaseFont;
}

static uint32_t __xuiTerminalMapPaletteColor(uint32_t iColor, const uint32_t* arrOldPalette, const uint32_t* arrNewPalette)
{
	int i;

	if ( (arrOldPalette == NULL) || (arrNewPalette == NULL) ) return iColor;
	for ( i = 0; i < 256; i++ ) {
		if ( arrOldPalette[i] == iColor ) return arrNewPalette[i];
	}
	return iColor;
}

static void __xuiTerminalResolveStyle(xui_widget pWidget, xui_terminal_data_t* pData)
{
	xui_proxy pProxy;
	xui_vec2_t tSize;
	xui_font_metrics_t tMetrics;
	uint32_t arrOldPalette[256];
	uint32_t iOldBackground;
	uint32_t iOldForeground;
	uint32_t iOldCursor;
	uint32_t iOldSelection;
	uint32_t iOldSelectionText;
	uint32_t iOldSearch;
	uint32_t iOldFocus;
	uint32_t iOldLink;
	uint32_t iOldCurrentFg;
	uint32_t iOldCurrentBg;
	xui_font pOldFont;
	float fOldCellWidth;
	float fOldCellHeight;
	float fOldCursorHeight;
	float fOldPadding;
	char sName[64];
	int i;

	if ( (pWidget == NULL) || (pData == NULL) ) return;
	memcpy(arrOldPalette, pData->arrPalette, sizeof(arrOldPalette));
	iOldBackground = pData->iBackgroundColor;
	iOldForeground = pData->iForegroundColor;
	iOldCursor = pData->iCursorColor;
	iOldSelection = pData->iSelectionColor;
	iOldSelectionText = pData->iSelectionTextColor;
	iOldSearch = pData->iSearchHighlightColor;
	iOldFocus = pData->iFocusColor;
	iOldLink = pData->iLinkHoverColor;
	iOldCurrentFg = pData->iCurrentFg;
	iOldCurrentBg = pData->iCurrentBg;
	pOldFont = pData->pFont;
	fOldCellWidth = pData->fCellWidth;
	fOldCellHeight = pData->fCellHeight;
	fOldCursorHeight = pData->fCursorHeight;
	fOldPadding = pData->fPadding;

	pData->pFont = __xuiTerminalStyleFont(pWidget, (pData->pBaseFont != NULL) ? pData->pBaseFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget)));
	pData->iBackgroundColor = pData->iBaseBackgroundColor;
	pData->iForegroundColor = pData->iBaseForegroundColor;
	pData->iCursorColor = pData->iBaseCursorColor;
	pData->iSelectionColor = pData->iBaseSelectionColor;
	pData->iSelectionTextColor = pData->iBaseSelectionTextColor;
	pData->iSearchHighlightColor = pData->iBaseSearchHighlightColor;
	pData->iFocusColor = pData->iBaseFocusColor;
	pData->iLinkHoverColor = pData->iBaseLinkHoverColor;
	pData->fCellWidth = pData->fBaseCellWidth;
	pData->fCellHeight = pData->fBaseCellHeight;
	pData->fPadding = pData->fBasePadding;
	memcpy(pData->arrPalette, pData->arrBasePalette, sizeof(pData->arrPalette));

	(void)__xuiTerminalStyleColor(pWidget, "terminal.background.color", &pData->iBackgroundColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.foreground.color", &pData->iForegroundColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.cursor.color", &pData->iCursorColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.selection.color", &pData->iSelectionColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.selection.text_color", &pData->iSelectionTextColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.search.highlight_color", &pData->iSearchHighlightColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.focus.color", &pData->iFocusColor);
	(void)__xuiTerminalStyleColor(pWidget, "terminal.link.hover_color", &pData->iLinkHoverColor);
	for ( i = 0; i < 16; i++ ) {
		snprintf(sName, sizeof(sName), "terminal.palette.%d", i);
		(void)__xuiTerminalStyleColor(pWidget, sName, &pData->arrPalette[i]);
	}
	(void)__xuiTerminalStyleFloat(pWidget, "terminal.cell.width", &pData->fCellWidth);
	(void)__xuiTerminalStyleFloat(pWidget, "terminal.cell.height", &pData->fCellHeight);
	(void)__xuiTerminalStyleFloat(pWidget, "terminal.padding", &pData->fPadding);

	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pData->fCellWidth <= 0.0f && pProxy != NULL && pProxy->textMeasure != NULL ) {
		if ( pProxy->textMeasure(pProxy, pData->pFont, "M", &tSize) == XUI_OK && tSize.fX > 0.0f ) {
			pData->fCellWidth = tSize.fX;
		}
		if ( pProxy->textMeasure(pProxy, pData->pFont, "W", &tSize) == XUI_OK && tSize.fX > pData->fCellWidth ) {
			pData->fCellWidth = tSize.fX;
		}
	}
	if ( pData->fCellHeight <= 0.0f && pProxy != NULL && pProxy->fontGetMetrics != NULL ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pData->pFont, &tMetrics) == XUI_OK && tMetrics.fLineHeight > 0.0f ) {
			pData->fCellHeight = tMetrics.fLineHeight;
		}
	}
	if ( pData->fCellWidth <= 0.0f ) pData->fCellWidth = 8.0f;
	if ( pData->fCellHeight <= 0.0f ) pData->fCellHeight = 16.0f;
	if ( pData->fPadding < 0.0f ) pData->fPadding = 0.0f;
	pData->fCellWidth = (float)xuiInternalSnapSize(pData->fCellWidth);
	pData->fCellHeight = (float)xuiInternalSnapSize(pData->fCellHeight);
	pData->fPadding = (float)xuiInternalSnapPixel(pData->fPadding);
	/* The terminal grid may deliberately use extra leading. Keep the cursor aligned
	 * with the font's typographic box instead of stretching it across that leading. */
	pData->fCursorHeight = pData->fCellHeight;
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL && pData->pFont != NULL ) {
		memset(&tMetrics, 0, sizeof(tMetrics));
		if ( pProxy->fontGetMetrics(pProxy, pData->pFont, &tMetrics) == XUI_OK ) {
			float fGlyphHeight = tMetrics.fAscent + ((tMetrics.fDescent < 0.0f) ? -tMetrics.fDescent : tMetrics.fDescent);
			if ( fGlyphHeight > 0.0f && fGlyphHeight < pData->fCursorHeight ) pData->fCursorHeight = fGlyphHeight;
		}
	}
	if ( pData->fCursorHeight < 1.0f ) pData->fCursorHeight = 1.0f;
	pData->fCursorHeight = (float)xuiInternalSnapSize(pData->fCursorHeight);

	if ( iOldCurrentFg == iOldForeground ) {
		pData->iCurrentFg = pData->iForegroundColor;
	} else {
		pData->iCurrentFg = __xuiTerminalMapPaletteColor(pData->iCurrentFg, arrOldPalette, pData->arrPalette);
	}
	if ( iOldCurrentBg == iOldBackground ) {
		pData->iCurrentBg = pData->iBackgroundColor;
	} else {
		pData->iCurrentBg = __xuiTerminalMapPaletteColor(pData->iCurrentBg, arrOldPalette, pData->arrPalette);
	}
	if ( (pData->pMenu != NULL) && (pOldFont != pData->pFont) ) {
		(void)xuiMenuSetFont(pData->pMenu, pData->pFont);
	}
	if ( (pOldFont != pData->pFont) ||
	     (fOldCellWidth != pData->fCellWidth) ||
	     (fOldCellHeight != pData->fCellHeight) ||
	     (fOldCursorHeight != pData->fCursorHeight) ||
	     (fOldPadding != pData->fPadding) ||
	     (iOldBackground != pData->iBackgroundColor) ||
	     (iOldForeground != pData->iForegroundColor) ||
	     (iOldCursor != pData->iCursorColor) ||
	     (iOldSelection != pData->iSelectionColor) ||
	     (iOldSelectionText != pData->iSelectionTextColor) ||
	     (iOldSearch != pData->iSearchHighlightColor) ||
	     (iOldFocus != pData->iFocusColor) ||
	     (iOldLink != pData->iLinkHoverColor) ||
	     (memcmp(arrOldPalette, pData->arrPalette, sizeof(arrOldPalette)) != 0) ) {
		__xuiTerminalMarkFullCacheDirty(pData);
	}
}

static int __xuiTerminalContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_terminal_data_t* pData;

	(void)tConstraint;
	(void)pUser;
	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	pSize->fX = pData->fPadding * 2.0f + (float)pData->iColumns * pData->fCellWidth;
	pSize->fY = pData->fPadding * 2.0f + (float)pData->iRows * pData->fCellHeight;
	return XUI_OK;
}

static int __xuiTerminalLayoutComplete(xui_widget pWidget, xui_rect_t tContentRect, void* pUser)
{
	(void)tContentRect;
	(void)pUser;
	(void)xuiTerminalFit(pWidget);
	return XUI_OK;
}

static int __xuiTerminalDrawText(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, const char* sText, xui_rect_t tRect, uint32_t iColor, uint32_t iFlags)
{
	if ( (pProxy == NULL) || (pProxy->drawText == NULL) || (pDraw == NULL) || (pFont == NULL) || (sText == NULL) || (sText[0] == '\0') ) {
		return XUI_OK;
	}
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (__xuiTerminalAlpha(iColor) == 0) ) {
		return XUI_OK;
	}
	return pProxy->drawText(pProxy, pDraw, pFont, sText,
		xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, tRect.fH),
		iColor, iFlags | XUI_TEXT_CLIP);
}

static int __xuiTerminalRenderSearchHighlightLine(xui_proxy pProxy, xui_draw_context pDraw, xui_terminal_data_t* pData, int iLogicalLine, float fX, float fY, float fWidth)
{
	xui_rect_t tMatch;
	int iStart;
	int iEnd;

	if ( pProxy == NULL || pDraw == NULL || pData == NULL || pProxy->drawRectFill == NULL ) return XUI_OK;
	if ( __xuiTerminalAlpha(pData->iSearchHighlightColor) == 0 ) return XUI_OK;
	if ( pData->iSearchLine != iLogicalLine || pData->iSearchLength <= 0 || pData->iSearchColumn < 0 ) return XUI_OK;
	iStart = __xuiTerminalClampColumn(pData, pData->iSearchColumn);
	iEnd = __xuiTerminalClampColumn(pData, pData->iSearchColumn + pData->iSearchLength);
	if ( iEnd <= iStart ) return XUI_OK;
	tMatch.fX = fX + (float)iStart * pData->fCellWidth;
	tMatch.fY = fY;
	tMatch.fW = (float)(iEnd - iStart) * pData->fCellWidth;
	tMatch.fH = pData->fCellHeight;
	if ( tMatch.fW > fWidth - (tMatch.fX - fX) ) tMatch.fW = fWidth - (tMatch.fX - fX);
	if ( tMatch.fW <= 0.0f ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw,
		xuiInternalRectFromFloatNearest(tMatch.fX, tMatch.fY, tMatch.fW, tMatch.fH),
		pData->iSearchHighlightColor);
}

static int __xuiTerminalSelectionColumns(xui_terminal_data_t* pData, int iLogicalLine, int* pStart, int* pEnd)
{
	int iLine0;
	int iColumn0;
	int iLine1;
	int iColumn1;
	int iStart;
	int iEnd;

	if ( pStart != NULL ) *pStart = 0;
	if ( pEnd != NULL ) *pEnd = 0;
	if ( pData == NULL || !__xuiTerminalSelectionRange(pData, &iLine0, &iColumn0, &iLine1, &iColumn1) ) return 0;
	if ( iLogicalLine < iLine0 || iLogicalLine > iLine1 ) return 0;
	if ( pData->bSelectAll ) {
		iStart = 0;
		iEnd = pData->iColumns;
	} else if ( iLine0 == iLine1 ) {
		iStart = iColumn0;
		iEnd = iColumn1;
	} else if ( iLogicalLine == iLine0 ) {
		iStart = iColumn0;
		iEnd = pData->iColumns;
	} else if ( iLogicalLine == iLine1 ) {
		iStart = 0;
		iEnd = iColumn1;
	} else {
		iStart = 0;
		iEnd = pData->iColumns;
	}
	iStart = __xuiTerminalClampColumn(pData, iStart);
	iEnd = __xuiTerminalClampColumn(pData, iEnd);
	if ( iEnd <= iStart ) return 0;
	if ( pStart != NULL ) *pStart = iStart;
	if ( pEnd != NULL ) *pEnd = iEnd;
	return 1;
}

static int __xuiTerminalRenderSelectionLine(xui_proxy pProxy, xui_draw_context pDraw, xui_terminal_data_t* pData, int iLogicalLine, float fX, float fY, float fWidth)
{
	xui_rect_t tSel;
	int iStart;
	int iEnd;

	if ( pProxy == NULL || pDraw == NULL || pData == NULL || pProxy->drawRectFill == NULL ) return XUI_OK;
	if ( __xuiTerminalAlpha(pData->iSelectionColor) == 0 ) return XUI_OK;
	if ( !__xuiTerminalSelectionColumns(pData, iLogicalLine, &iStart, &iEnd) ) return XUI_OK;
	tSel.fX = fX + (float)iStart * pData->fCellWidth;
	tSel.fY = fY;
	tSel.fW = (float)(iEnd - iStart) * pData->fCellWidth;
	tSel.fH = pData->fCellHeight;
	if ( tSel.fW > fWidth - (tSel.fX - fX) ) tSel.fW = fWidth - (tSel.fX - fX);
	if ( tSel.fW <= 0.0f ) return XUI_OK;
	return pProxy->drawRectFill(pProxy, pDraw,
		xuiInternalRectFromFloatNearest(tSel.fX, tSel.fY, tSel.fW, tSel.fH),
		pData->iSelectionColor);
}

static int __xuiTerminalRenderHoverLinkLine(xui_proxy pProxy, xui_draw_context pDraw, xui_terminal_data_t* pData, int iLogicalLine, float fX, float fY)
{
	float fX0;
	float fX1;
	float fLineY;

	if ( pProxy == NULL || pProxy->drawLine == NULL || pData == NULL ) return XUI_OK;
	if ( __xuiTerminalAlpha(pData->iLinkHoverColor) == 0 ) return XUI_OK;
	if ( pData->sHoverLink == NULL || pData->iHoverLinkLine != iLogicalLine || pData->iHoverLinkLength <= 0 ) return XUI_OK;
	fX0 = fX + (float)pData->iHoverLinkColumn * pData->fCellWidth;
	fX1 = fX + (float)(pData->iHoverLinkColumn + pData->iHoverLinkLength) * pData->fCellWidth;
	fLineY = fY + pData->fCellHeight - 2.0f;
	return pProxy->drawLine(pProxy, pDraw, fX0, fLineY, fX1, fLineY, 1.0f, pData->iLinkHoverColor);
}

static int __xuiTerminalCellUtf8(const xui_terminal_cell_t* pCell, char* sText, int iCapacity)
{
	int iOffset = 0;
	int i;
	int n;

	if ( pCell == NULL || sText == NULL || iCapacity <= 0 ) return 0;
	n = __xuiTerminalEncodeUtf8(pCell->iCodepoint, sText);
	if ( n <= 0 || n >= iCapacity ) {
		sText[0] = '\0';
		return 0;
	}
	iOffset = n;
	for ( i = 0; i < pCell->iCombiningCount; ++i ) {
		n = __xuiTerminalEncodeUtf8(pCell->arrCombining[i], sText + iOffset);
		if ( n <= 0 || iOffset + n >= iCapacity ) break;
		iOffset += n;
	}
	sText[iOffset] = '\0';
	return iOffset;
}

static int __xuiTerminalRenderStrike(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tTextRect, uint32_t iColor)
{
	float fY;
	if ( pProxy == NULL || pProxy->drawLine == NULL ) return XUI_OK;
	fY = tTextRect.fY + tTextRect.fH * 0.55f;
	return pProxy->drawLine(pProxy, pDraw, tTextRect.fX, fY,
		tTextRect.fX + tTextRect.fW, fY, 1.0f, iColor);
}

static int __xuiTerminalRenderCellRow(xui_proxy pProxy, xui_draw_context pDraw,
	xui_terminal_data_t* pData, const xui_terminal_cell_t* pCells, int iCellCount,
	int iLogicalLine, float fX, float fY)
{
	char sRun[1024];
	char sCell[4 * (XUI_TERMINAL_MAX_COMBINING + 1) + 1];
	xui_rect_t tCellRect;
	xui_rect_t tTextRect;
	uint32_t iFg;
	uint32_t iBg;
	uint32_t iFlags;
	int iRunStart;
	int iRunBytes;
	int x;
	int n;
	int iRet;
	int iTextFlags;
	int iSelectionStart;
	int iSelectionEnd;
	int bHasSelection;
	int bRunSelected;
	int iCellBytes;
	int iCellWidth;

	if ( pCells == NULL || iCellCount <= 0 ) return XUI_OK;
	bHasSelection = __xuiTerminalSelectionColumns(pData, iLogicalLine, &iSelectionStart, &iSelectionEnd);
	for ( x = 0; x < iCellCount; x++ ) {
		iBg = pCells[x].iBgColor;
		if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_INVERSE) != 0u ) {
			iBg = pCells[x].iFgColor;
		}
		if ( iBg != pData->iBackgroundColor && __xuiTerminalAlpha(iBg) != 0 && pProxy != NULL && pProxy->drawRectFill != NULL ) {
			tCellRect.fX = fX + (float)x * pData->fCellWidth;
			tCellRect.fY = fY;
			tCellRect.fW = pData->fCellWidth;
			tCellRect.fH = pData->fCellHeight;
			iRet = pProxy->drawRectFill(pProxy, pDraw,
				xuiInternalRectFromFloatNearest(tCellRect.fX, tCellRect.fY, tCellRect.fW, tCellRect.fH), iBg);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( !pData->bLigaturesEnabled ) {
		for ( x = 0; x < iCellCount; ++x ) {
			if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u ||
			     pCells[x].iCodepoint == 0u || pCells[x].iCodepoint == ' ' ) continue;
			if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_BLINK) != 0u &&
			     !pData->bLastCaretBlinkVisible ) continue;
			iCellBytes = __xuiTerminalCellUtf8(&pCells[x], sCell, (int)sizeof(sCell));
			if ( iCellBytes <= 0 ) continue;
			iFg = ((pCells[x].iFlags & XUI_TERMINAL_CELL_INVERSE) != 0u) ? pCells[x].iBgColor : pCells[x].iFgColor;
			if ( bHasSelection && x >= iSelectionStart && x < iSelectionEnd ) iFg = pData->iSelectionTextColor;
			iCellWidth = (pCells[x].iWidth > 0) ? pCells[x].iWidth : 1;
			tTextRect.fX = fX + (float)x * pData->fCellWidth;
			tTextRect.fY = fY;
			tTextRect.fW = (float)iCellWidth * pData->fCellWidth;
			tTextRect.fH = pData->fCellHeight;
			iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
			if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_UNDERLINE) != 0u ) iTextFlags |= XUI_TEXT_UNDERLINE;
			iRet = __xuiTerminalDrawText(pProxy, pDraw, pData->pFont, sCell, tTextRect, iFg, iTextFlags);
			if ( iRet != XUI_OK ) return iRet;
			if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_BOLD) != 0u ) {
				xui_rect_t tBoldRect = tTextRect;
				tBoldRect.fX += 1.0f;
				iRet = __xuiTerminalDrawText(pProxy, pDraw, pData->pFont, sCell,
					tBoldRect, iFg, iTextFlags);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( (pCells[x].iFlags & XUI_TERMINAL_CELL_STRIKE) != 0u ) {
				iRet = __xuiTerminalRenderStrike(pProxy, pDraw, tTextRect, iFg);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		return XUI_OK;
	}
	x = 0;
	while ( x < iCellCount ) {
		while ( x < iCellCount &&
		        (((pCells[x].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) != 0u) ||
		         (pCells[x].iCodepoint == 0u) ||
		         (pCells[x].iCodepoint == ' ')) ) {
			x++;
		}
		if ( x >= iCellCount ) break;
		iRunStart = x;
		iFg = pCells[x].iFgColor;
		iBg = pCells[x].iBgColor;
		iFlags = pCells[x].iFlags & (XUI_TERMINAL_CELL_BOLD | XUI_TERMINAL_CELL_DIM |
			XUI_TERMINAL_CELL_UNDERLINE | XUI_TERMINAL_CELL_INVERSE |
			XUI_TERMINAL_CELL_ITALIC | XUI_TERMINAL_CELL_BLINK | XUI_TERMINAL_CELL_STRIKE);
		bRunSelected = bHasSelection && x >= iSelectionStart && x < iSelectionEnd;
		iRunBytes = 0;
		while ( x < iCellCount &&
		        ((pCells[x].iFlags & XUI_TERMINAL_CELL_WIDE_CONT) == 0u) &&
		        (pCells[x].iCodepoint != 0u) &&
		        (pCells[x].iCodepoint != ' ') &&
		        (pCells[x].iFgColor == iFg) &&
		        (pCells[x].iBgColor == iBg) &&
		        ((bHasSelection && x >= iSelectionStart && x < iSelectionEnd) == bRunSelected) &&
		        ((pCells[x].iFlags & (XUI_TERMINAL_CELL_BOLD | XUI_TERMINAL_CELL_DIM |
		         XUI_TERMINAL_CELL_UNDERLINE | XUI_TERMINAL_CELL_INVERSE |
		         XUI_TERMINAL_CELL_ITALIC | XUI_TERMINAL_CELL_BLINK |
		         XUI_TERMINAL_CELL_STRIKE)) == iFlags) ) {
			n = __xuiTerminalCellUtf8(&pCells[x], sCell, (int)sizeof(sCell));
			if ( n <= 0 || iRunBytes + n >= (int)sizeof(sRun) - 1 ) break;
			memcpy(sRun + iRunBytes, sCell, (size_t)n);
			iRunBytes += n;
			x += (pCells[x].iWidth > 0) ? pCells[x].iWidth : 1;
		}
		sRun[iRunBytes] = '\0';
		tTextRect.fX = fX + (float)iRunStart * pData->fCellWidth;
		tTextRect.fY = fY;
		tTextRect.fW = (float)(x - iRunStart) * pData->fCellWidth;
		tTextRect.fH = pData->fCellHeight;
		if ( (iFlags & XUI_TERMINAL_CELL_INVERSE) != 0u ) iFg = iBg;
		if ( bRunSelected ) iFg = pData->iSelectionTextColor;
		iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP;
		if ( (iFlags & XUI_TERMINAL_CELL_UNDERLINE) != 0u ) iTextFlags |= XUI_TEXT_UNDERLINE;
		if ( (iFlags & XUI_TERMINAL_CELL_BLINK) == 0u || pData->bLastCaretBlinkVisible ) {
			iRet = __xuiTerminalDrawText(pProxy, pDraw, pData->pFont, sRun, tTextRect, iFg, iTextFlags);
			if ( iRet != XUI_OK ) return iRet;
			if ( (iFlags & XUI_TERMINAL_CELL_BOLD) != 0u ) {
				xui_rect_t tBoldRect = tTextRect;
				tBoldRect.fX += 1.0f;
				iRet = __xuiTerminalDrawText(pProxy, pDraw, pData->pFont, sRun,
					tBoldRect, iFg, iTextFlags);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( (iFlags & XUI_TERMINAL_CELL_STRIKE) != 0u ) {
				iRet = __xuiTerminalRenderStrike(pProxy, pDraw, tTextRect, iFg);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiTerminalRenderScreenRow(xui_proxy pProxy, xui_draw_context pDraw,
	xui_terminal_data_t* pData, int iLogicalLine, int iScreenRow, float fX, float fY)
{
	const xui_terminal_cell_t* pCells = __xuiTerminalScreenConst(pData);
	if ( pCells == NULL || iScreenRow < 0 || iScreenRow >= pData->iRows ) return XUI_OK;
	return __xuiTerminalRenderCellRow(pProxy, pDraw, pData,
		__xuiTerminalBufferRowConst(pData, pCells, iScreenRow), pData->iColumns,
		iLogicalLine, fX, fY);
}

static int __xuiTerminalRenderHistoryRow(xui_proxy pProxy, xui_draw_context pDraw,
	xui_terminal_data_t* pData, int iLogicalLine, float fX, float fY)
{
	xui_terminal_history_view_row_t* pView;
	xui_terminal_cell_t* pNew;
	xui_terminal_cell_t tBlank;
	int i;
	int iCount;
	pView = __xuiTerminalGetHistoryViewRow(pData, iLogicalLine);
	if ( pView == NULL ) return XUI_OK;
	if ( pData->iHistoryRenderCapacity < pData->iColumns ) {
		pNew = (xui_terminal_cell_t*)xrtRealloc(pData->pHistoryRenderCells,
			sizeof(*pNew) * (size_t)pData->iColumns);
		if ( pNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
		pData->pHistoryRenderCells = pNew;
		pData->iHistoryRenderCapacity = pData->iColumns;
	}
	tBlank = __xuiTerminalEraseCell(pData);
	for ( i = 0; i < pData->iColumns; ++i ) pData->pHistoryRenderCells[i] = tBlank;
	iCount = __xuiTerminalMin(pView->iEnd - pView->iStart, pData->iColumns);
	for ( i = 0; i < iCount; ++i ) {
		__xuiTerminalHistoryCellToPublic(pView->pLine, pView->iStart + i,
			&pData->pHistoryRenderCells[i]);
	}
	return __xuiTerminalRenderCellRow(pProxy, pDraw, pData,
		pData->pHistoryRenderCells, pData->iColumns, iLogicalLine, fX, fY);
}

static xui_rect_t __xuiTerminalIntersectRect(xui_rect_t a, xui_rect_t b)
{
	float fRight = (a.fX + a.fW < b.fX + b.fW) ? a.fX + a.fW : b.fX + b.fW;
	float fBottom = (a.fY + a.fH < b.fY + b.fH) ? a.fY + a.fH : b.fY + b.fH;
	xui_rect_t tRect;

	tRect.fX = (a.fX > b.fX) ? a.fX : b.fX;
	tRect.fY = (a.fY > b.fY) ? a.fY : b.fY;
	tRect.fW = (fRight > tRect.fX) ? fRight - tRect.fX : 0.0f;
	tRect.fH = (fBottom > tRect.fY) ? fBottom - tRect.fY : 0.0f;
	return tRect;
}

static int __xuiTerminalClipBegin(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tClip,
	xui_rect_t* pOldClip, int* pHadOldClip, int* pActive)
{
	int iRet;

	if ( pHadOldClip != NULL ) *pHadOldClip = 0;
	if ( pActive != NULL ) *pActive = 0;
	if ( pProxy == NULL || pDraw == NULL || pOldClip == NULL || pHadOldClip == NULL || pActive == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pProxy->drawClipGet == NULL || pProxy->drawClipSet == NULL || pProxy->drawClipClear == NULL ) return XUI_OK;
	iRet = pProxy->drawClipGet(pProxy, pDraw, pOldClip, pHadOldClip);
	if ( iRet != XUI_OK ) return iRet;
	if ( *pHadOldClip ) tClip = __xuiTerminalIntersectRect(tClip, *pOldClip);
	iRet = pProxy->drawClipSet(pProxy, pDraw,
		xuiInternalRectFromFloatNearest(tClip.fX, tClip.fY, tClip.fW, tClip.fH));
	if ( iRet != XUI_OK ) return iRet;
	*pActive = 1;
	return XUI_OK;
}

static int __xuiTerminalClipEnd(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tOldClip, int bHadOldClip, int bActive)
{
	if ( !bActive ) return XUI_OK;
	if ( pProxy == NULL || pDraw == NULL || pProxy->drawClipSet == NULL || pProxy->drawClipClear == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	return bHadOldClip ? pProxy->drawClipSet(pProxy, pDraw, tOldClip) : pProxy->drawClipClear(pProxy, pDraw);
}

static int __xuiTerminalCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_terminal_data_t* pData;
	xui_proxy pProxy;
	xui_surface pCache;
	xui_rect_t tRect;
	xui_rect_t tLineRect;
	xui_rect_t tCursor;
	xui_rect_t tOldClip;
	float fLineX;
	float fLineY;
	int iOffsetY;
	int iTopLine;
	int iScrollRemainder;
	int iLine;
	int i;
	int iRenderRows;
	int iScreenLine;
	int iRet;
	int bFullRender;
	int bCaretBlinkVisible;
	int bCursorBlink;
	int bHadOldClip;
	int bClipActive;
	int iClipRet;
	uint32_t iFocusColor;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pCache = xuiWidgetGetCacheSurface(pWidget, iStateId);
	if ( pCache != pData->pLastCacheSurface ) {
		__xuiTerminalMarkFullCacheDirty(pData);
	}
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalSyncScrollModel(pWidget, pData);
	bCaretBlinkVisible = xuiInternalCaretBlinkVisible(pWidget);
	tRect = xuiWidgetGetContentRect(pWidget);
	iOffsetY = __xuiTerminalScrollOffsetPixels(pData);
	iTopLine = (pData->fCellHeight > 0.0f) ? iOffsetY / (int)pData->fCellHeight : 0;
	iScrollRemainder = iOffsetY - iTopLine * (int)pData->fCellHeight;
	fLineX = tRect.fX + pData->fPadding;
	fLineY = tRect.fY + pData->fPadding - (float)iScrollRemainder;
	bFullRender = pData->bFullCacheDirty ||
	              !pData->bCacheRendered ||
	              pData->bLastCaretBlinkVisible != bCaretBlinkVisible ||
	              pData->iLastRenderOffsetY != iOffsetY ||
	              pData->iLastRenderTopLine != iTopLine ||
	              pData->iLastRenderColumns != pData->iColumns ||
	              pData->iLastRenderRows != pData->iRows;
	if ( !bFullRender && iTopLine != pData->iScrollbackCount ) {
		bFullRender = 1;
	}
	if ( !bFullRender && pData->iDirtyRowCount <= 0 ) {
		return XUI_OK;
	}
	pData->bLastCaretBlinkVisible = bCaretBlinkVisible;
	if ( bFullRender && pProxy->drawRectFill != NULL && __xuiTerminalAlpha(pData->iBackgroundColor) != 0 ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw,
			xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, tRect.fH),
			pData->iBackgroundColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	bHadOldClip = 0;
	bClipActive = 0;
	memset(&tOldClip, 0, sizeof(tOldClip));
	iRet = __xuiTerminalClipBegin(pProxy, pDraw, tRect, &tOldClip, &bHadOldClip, &bClipActive);
	if ( iRet != XUI_OK ) return iRet;
	iRenderRows = pData->iRows + ((iScrollRemainder > 0) ? 1 : 0);
	for ( i = 0; i < iRenderRows; i++ ) {
		iLine = iTopLine + i;
		if ( iLine < 0 || iLine >= __xuiTerminalLogicalLineCount(pData) ) continue;
		iScreenLine = iLine - pData->iScrollbackCount;
		tLineRect.fX = fLineX;
		tLineRect.fY = fLineY + (float)i * pData->fCellHeight;
		tLineRect.fW = tRect.fW - (fLineX - tRect.fX) - pData->fPadding;
		tLineRect.fH = pData->fCellHeight;
		if ( !bFullRender ) {
			if ( iScreenLine < 0 || iScreenLine >= pData->iRows || pData->pDirtyRows == NULL || pData->pDirtyRows[iScreenLine] == 0u ) {
				continue;
			}
			if ( pProxy->drawRectFill != NULL && __xuiTerminalAlpha(pData->iBackgroundColor) != 0 ) {
				iRet = pProxy->drawRectFill(pProxy, pDraw,
					xuiInternalRectFromFloatNearest(tLineRect.fX, tLineRect.fY, tLineRect.fW, tLineRect.fH),
					pData->iBackgroundColor);
				if ( iRet != XUI_OK ) goto render_done;
			}
		}
		iRet = __xuiTerminalRenderSearchHighlightLine(pProxy, pDraw, pData, iLine, tLineRect.fX, tLineRect.fY, tLineRect.fW);
		if ( iRet != XUI_OK ) goto render_done;
		iRet = __xuiTerminalRenderSelectionLine(pProxy, pDraw, pData, iLine, tLineRect.fX, tLineRect.fY, tLineRect.fW);
		if ( iRet != XUI_OK ) goto render_done;
		if ( iLine < pData->iScrollbackCount ) {
			iRet = __xuiTerminalRenderHistoryRow(pProxy, pDraw, pData,
				iLine, tLineRect.fX, tLineRect.fY);
			if ( iRet != XUI_OK ) goto render_done;
		} else {
			iRet = __xuiTerminalRenderScreenRow(pProxy, pDraw, pData, iLine, iScreenLine, tLineRect.fX, tLineRect.fY);
			if ( iRet != XUI_OK ) goto render_done;
		}
		iRet = __xuiTerminalRenderHoverLinkLine(pProxy, pDraw, pData, iLine, tLineRect.fX, tLineRect.fY);
		if ( iRet != XUI_OK ) goto render_done;
	}
	if ( pData->bImeActive && pData->sImeText != NULL && pData->sImeText[0] != '\0' ) {
		xui_rect_t tIme;
		int iImeColumns;
		tIme.fX = fLineX + (float)pData->iCursorX * pData->fCellWidth;
		tIme.fY = tRect.fY + pData->fPadding +
			(float)(pData->iCursorY + pData->iScrollbackCount) * pData->fCellHeight - (float)iOffsetY;
		iImeColumns = __xuiTerminalUtf8DisplayColumns(pData->sImeText, -1);
		if ( iImeColumns < 1 ) iImeColumns = 1;
		tIme.fW = (float)iImeColumns * pData->fCellWidth;
		if ( tIme.fW > tRect.fX + tRect.fW - tIme.fX - pData->fPadding ) {
			tIme.fW = tRect.fX + tRect.fW - tIme.fX - pData->fPadding;
		}
		tIme.fH = pData->fCellHeight;
		if ( tIme.fW > 0.0f ) {
			iRet = __xuiTerminalDrawText(pProxy, pDraw, pData->pFont, pData->sImeText,
				tIme, pData->iForegroundColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP);
			if ( iRet != XUI_OK ) goto render_done;
			if ( pProxy->drawRectFill != NULL ) {
				xui_rect_t tUnderline = {tIme.fX, tIme.fY + tIme.fH - 1.0f,
					tIme.fW, 1.0f};
				iRet = pProxy->drawRectFill(pProxy, pDraw,
					xuiInternalRectFromFloatNearest(tUnderline.fX, tUnderline.fY, tUnderline.fW, tUnderline.fH),
					pData->iCursorColor);
				if ( iRet != XUI_OK ) goto render_done;
			}
		}
	}
	bCursorBlink = pData->iCursorStyle == 0 || pData->iCursorStyle == 1 ||
		pData->iCursorStyle == 3 || pData->iCursorStyle == 5;
	if ( pData->bCursorVisible && (!bCursorBlink || bCaretBlinkVisible) &&
	     xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget &&
	     __xuiTerminalAlpha(pData->iCursorColor) != 0 ) {
		tCursor.fX = fLineX + (float)pData->iCursorX * pData->fCellWidth;
		tCursor.fY = tRect.fY + pData->fPadding +
			(float)(pData->iCursorY + pData->iScrollbackCount) * pData->fCellHeight - (float)iOffsetY;
		tCursor.fW = pData->fCellWidth;
		tCursor.fH = pData->fCursorHeight;
		if ( pData->iCursorStyle == 3 || pData->iCursorStyle == 4 ) {
			tCursor.fY += tCursor.fH - 2.0f;
			tCursor.fH = 2.0f;
			iRet = pProxy->drawRectFill != NULL ? pProxy->drawRectFill(pProxy, pDraw,
				xuiInternalRectFromFloatNearest(tCursor.fX, tCursor.fY, tCursor.fW, tCursor.fH),
				pData->iCursorColor) : XUI_OK;
		} else if ( pData->iCursorStyle == 5 || pData->iCursorStyle == 6 ) {
			tCursor.fW = 2.0f;
			iRet = pProxy->drawRectFill != NULL ? pProxy->drawRectFill(pProxy, pDraw,
				xuiInternalRectFromFloatNearest(tCursor.fX, tCursor.fY, tCursor.fW, tCursor.fH),
				pData->iCursorColor) : XUI_OK;
		} else {
			iRet = pProxy->drawRectStroke != NULL ? pProxy->drawRectStroke(pProxy, pDraw,
				xuiInternalRectFromFloatNearest(tCursor.fX, tCursor.fY, tCursor.fW, tCursor.fH),
				1.0f, pData->iCursorColor) : XUI_OK;
		}
		if ( iRet != XUI_OK ) goto render_done;
	}
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget && pProxy->drawRectStroke != NULL ) {
		iFocusColor = xuiWidgetGetEnabled(pWidget) ? pData->iFocusColor : XUI_COLOR_RGBA(115, 135, 155, 150);
		if ( __xuiTerminalAlpha(iFocusColor) != 0 ) {
			iRet = pProxy->drawRectStroke(pProxy, pDraw,
				xuiInternalInsetRect(xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, tRect.fH), 0.5f),
				1.0f, iFocusColor);
			if ( iRet != XUI_OK ) goto render_done;
		}
	}

render_done:
	iClipRet = __xuiTerminalClipEnd(pProxy, pDraw, tOldClip, bHadOldClip, bClipActive);
	if ( iRet == XUI_OK ) iRet = iClipRet;
	if ( iRet != XUI_OK ) return iRet;
	__xuiTerminalClearDirtyRows(pData);
	pData->bFullCacheDirty = 0;
	pData->bCacheRendered = 1;
	pData->bLastCaretBlinkVisible = bCaretBlinkVisible;
	pData->pLastCacheSurface = pCache;
	pData->iLastRenderTopLine = iTopLine;
	pData->iLastRenderOffsetY = iOffsetY;
	pData->iLastRenderColumns = pData->iColumns;
	pData->iLastRenderRows = pData->iRows;
	return XUI_OK;
}

static int __xuiTerminalUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_terminal_data_t* pData;

	(void)fDelta;
	pData = (xui_terminal_data_t*)pUser;
	if ( pData == NULL ) return XUI_OK;
	if ( pData->pSession != NULL ) {
		(void)xuiTerminalSessionPoll(pData->pSession);
	}
	if ( pData->iQueueSize > 0 ) __xuiTerminalClearHoverLinkData(pData);
	return __xuiTerminalProcessQueue(pWidget, pData, pData->iParseBudget);
}

static void __xuiTerminalDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_FIXED;
	pLayout->iHeightMode = XUI_SIZE_FIXED;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_CLIP;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fPreferredWidth = 640.0f;
	pLayout->fPreferredHeight = 360.0f;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fGrow = 0.0f;
	pLayout->fShrink = 1.0f;
}

static void __xuiTerminalDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = 0;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiTerminalInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_terminal_data_t* pData;
	const xui_terminal_desc_t* pDesc;
	xui_context pContext;
	int iRet;
	int iColumns;
	int iRows;

	(void)pUser;
	if ( (pWidget == NULL) || (pTypeData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_terminal_data_t*)pTypeData;
	pDesc = (const xui_terminal_desc_t*)pCreateData;
	memset(pData, 0, sizeof(*pData));
	pData->bLastCaretBlinkVisible = -1;
	pContext = xuiWidgetGetContext(pWidget);
	pData->pBaseFont = (pDesc != NULL && pDesc->pFont != NULL) ? pDesc->pFont : xuiGetDefaultFont(pContext);
	pData->pFont = pData->pBaseFont;
	pData->iBaseBackgroundColor = (pDesc != NULL && pDesc->iBackgroundColor != 0u) ? pDesc->iBackgroundColor : XUI_COLOR_RGBA(18, 24, 32, 255);
	pData->iBaseForegroundColor = (pDesc != NULL && pDesc->iForegroundColor != 0u) ? pDesc->iForegroundColor : XUI_COLOR_RGBA(220, 230, 240, 255);
	pData->iBaseCursorColor = (pDesc != NULL && pDesc->iCursorColor != 0u) ? pDesc->iCursorColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iBaseSelectionColor = (pDesc != NULL && pDesc->iSelectionColor != 0u) ? pDesc->iSelectionColor : XUI_COLOR_RGBA(74, 144, 239, 255);
	pData->iBaseSelectionTextColor = (pDesc != NULL && pDesc->iSelectionTextColor != 0u) ? pDesc->iSelectionTextColor : XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iBaseSearchHighlightColor = (pDesc != NULL && pDesc->iSearchHighlightColor != 0u) ? pDesc->iSearchHighlightColor : XUI_COLOR_RGBA(255, 197, 66, 110);
	pData->iBaseFocusColor = (pDesc != NULL && pDesc->iFocusColor != 0u) ? pDesc->iFocusColor : XUI_COLOR_RGBA(47, 128, 237, 255);
	pData->iBaseLinkHoverColor = (pDesc != NULL && pDesc->iLinkHoverColor != 0u) ? pDesc->iLinkHoverColor : XUI_COLOR_RGBA(86, 156, 214, 255);
	pData->iBackgroundColor = pData->iBaseBackgroundColor;
	pData->iForegroundColor = pData->iBaseForegroundColor;
	pData->iCursorColor = pData->iBaseCursorColor;
	pData->iSelectionColor = pData->iBaseSelectionColor;
	pData->iSelectionTextColor = pData->iBaseSelectionTextColor;
	pData->iSearchHighlightColor = pData->iBaseSearchHighlightColor;
	pData->iFocusColor = pData->iBaseFocusColor;
	pData->iLinkHoverColor = pData->iBaseLinkHoverColor;
	pData->iCurrentFg = pData->iForegroundColor;
	pData->iCurrentBg = pData->iBackgroundColor;
	pData->iSavedFg = pData->iForegroundColor;
	pData->iSavedBg = pData->iBackgroundColor;
	pData->iScrollbackLimit = (pDesc != NULL && pDesc->iScrollbackLimit > 0) ? pDesc->iScrollbackLimit : XUI_TERMINAL_DEFAULT_SCROLLBACK;
	pData->iParseBudget = (pDesc != NULL && pDesc->iParseBudgetBytes > 0) ? pDesc->iParseBudgetBytes : XUI_TERMINAL_DEFAULT_PARSE_BUDGET;
	pData->fBaseCellWidth = (pDesc != NULL) ? pDesc->fCellWidth : 0.0f;
	pData->fBaseCellHeight = (pDesc != NULL) ? pDesc->fCellHeight : 0.0f;
	pData->fBasePadding = (pDesc != NULL && pDesc->fPadding > 0.0f) ? pDesc->fPadding : 8.0f;
	pData->fCellWidth = pData->fBaseCellWidth;
	pData->fCellHeight = pData->fBaseCellHeight;
	pData->fPadding = pData->fBasePadding;
	pData->bCursorVisible = 1;
	pData->bAutoWrap = 1;
	pData->iMouseTracking = XUI_TERMINAL_MOUSE_TRACKING_NONE;
	pData->iMouseEncoding = XUI_TERMINAL_MOUSE_ENCODING_X10;
	__xuiTerminalClearSelectionData(pData);
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalClearHoverLinkData(pData);
	__xuiTerminalDefaultPalette(pData);
	__xuiTerminalResetParser(&pData->tParser);
	xuiScrollModelInit(&pData->tScroll);
	iRet = __xuiTerminalAllocScrollback(pData, pData->iScrollbackLimit);
	if ( iRet != XUI_OK ) return iRet;
	iColumns = (pDesc != NULL && pDesc->iColumns > 0) ? pDesc->iColumns : XUI_TERMINAL_DEFAULT_COLS;
	iRows = (pDesc != NULL && pDesc->iRows > 0) ? pDesc->iRows : XUI_TERMINAL_DEFAULT_ROWS;
	iRet = __xuiTerminalResizeBuffers(pData, iColumns, iRows);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetFocusable(pWidget, 1);
	(void)xuiWidgetSetTabStop(pWidget, 1);
	(void)xuiWidgetSetImeMode(pWidget, XUI_IME_ENABLED);
	(void)xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_CLIP);
	{
		xui_edit_behavior_t tBehavior;
		memset(&tBehavior, 0, sizeof(tBehavior));
		tBehavior.iSize = sizeof(tBehavior);
		tBehavior.iTabBehavior = XUI_EDIT_TAB_DEFAULT;
		tBehavior.iEnterBehavior = XUI_EDIT_ENTER_DEFAULT;
		tBehavior.iEscapeBehavior = XUI_EDIT_ESCAPE_DEFAULT;
		iRet = xuiInternalEditRegister(pWidget, &g_xuiTerminalEditAdapter, &tBehavior);
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)xuiWidgetSetImeCandidateRect(pWidget, __xuiTerminalImeRect, NULL);
	iRet = __xuiTerminalInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTerminalInitMenu(pWidget, pData);
}

static void __xuiTerminalDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_terminal_data_t* pData;
	xui_widget pPopup;
	int i;

	(void)pUser;
	pData = (xui_terminal_data_t*)pTypeData;
	if ( pData == NULL ) return;
	if ( xuiInternalWidgetIsValid(pWidget) && xuiGetPointerCapture(xuiWidgetGetContext(pWidget)) == pWidget ) {
		(void)xuiReleasePointerCapture(xuiWidgetGetContext(pWidget), pWidget);
	}
	if ( pData->pFindWindow != NULL ) {
		xuiWidgetDestroy(pData->pFindWindow);
		pData->pFindWindow = NULL;
	}
	if ( pData->pMenu != NULL ) {
		pPopup = xuiMenuGetPopupWidget(pData->pMenu);
		if ( pPopup != NULL ) {
			xuiWidgetDestroy(pPopup);
		} else {
			xuiWidgetDestroy(pData->pMenu);
		}
		pData->pMenu = NULL;
	}
	if ( pData->pMain != NULL ) xrtFree(pData->pMain);
	if ( pData->pAlt != NULL ) xrtFree(pData->pAlt);
	if ( pData->pMainWrapped != NULL ) xrtFree(pData->pMainWrapped);
	if ( pData->pAltWrapped != NULL ) xrtFree(pData->pAltWrapped);
	if ( pData->pHistoryRenderCells != NULL ) xrtFree(pData->pHistoryRenderCells);
	if ( pData->pQueue != NULL ) xrtFree(pData->pQueue);
	if ( pData->pTabStops != NULL ) xrtFree(pData->pTabStops);
	if ( pData->pDirtyRows != NULL ) xrtFree(pData->pDirtyRows);
	if ( pData->sSearchText != NULL ) xrtFree(pData->sSearchText);
	if ( pData->sHoverLink != NULL ) xrtFree(pData->sHoverLink);
	if ( pData->sImeText != NULL ) xrtFree(pData->sImeText);
	for ( i = 0; i < XUI_TERMINAL_MENU_TITLE_COUNT; ++i ) {
		xrtFree(pData->arrMenuTitle[i]);
		pData->arrMenuTitle[i] = NULL;
	}
	__xuiTerminalFreeLinks(pData);
	__xuiTerminalFreeScrollback(pData);
	if ( pData->pSession != NULL && pData->pSession->pWidget == pWidget ) {
		pData->pSession->pWidget = NULL;
	}
	memset(pData, 0, sizeof(*pData));
}

static void __xuiTerminalRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiTerminalRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;
	char sName[64];
	int i;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.foreground.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.cursor.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.selection.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.search.highlight_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.focus.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.link.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.cell.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.cell.height", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiTerminalRegisterStyleProperty(pContext, pType, "terminal.padding", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	for ( i = 0; i < 16; i++ ) {
		snprintf(sName, sizeof(sName), "terminal.palette.%d", i);
		__xuiTerminalRegisterStyleProperty(pContext, pType, sName, XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	}
	__xuiTerminalRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

XUI_API xui_widget_type xuiTerminalGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_cache_policy_t tPolicy;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "terminal");
	if ( pType != NULL ) {
		__xuiTerminalRegisterStyleProperties(pContext, pType);
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "terminal";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_terminal_data_t);
	tDesc.onInit = __xuiTerminalInit;
	tDesc.onDestroy = __xuiTerminalDestroy;
	tDesc.onContentMeasure = __xuiTerminalContentMeasure;
	tDesc.onLayoutComplete = __xuiTerminalLayoutComplete;
	tDesc.onCacheRender = __xuiTerminalCacheRender;
	tDesc.onUpdate = __xuiTerminalUpdate;
	tDesc.onQueryCursor = __xuiTerminalQueryCursor;
	__xuiTerminalDefaultLayout(&tDesc.tLayout);
	__xuiTerminalDefaultCachePolicy(&tPolicy);
	tDesc.tCachePolicy = tPolicy;
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiTerminalRegisterStyleProperties(pContext, pType);
	return pType;
}

XUI_API int xuiTerminalCreate(xui_context pContext, xui_widget* ppWidget, const xui_terminal_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	pType = xuiTerminalGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

static xui_terminal_data_t* __xuiTerminalGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "terminal");
	if ( pType == NULL || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_terminal_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiTerminalQueryCursor(xui_widget pWidget, int iX, int iY, void* pUser)
{
	(void)iX;
	(void)iY;
	(void)pUser;
	return xuiWidgetGetEnabled(pWidget) ? XUI_CURSOR_IBEAM : XUI_CURSOR_NOT_ALLOWED;
}

XUI_API int xuiTerminalWrite(xui_widget pWidget, const void* pDataBytes, int iSize)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalAppendQueue(pData, pDataBytes, iSize);
}

XUI_API int xuiTerminalWriteText(xui_widget pWidget, const char* sText)
{
	if ( sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return xuiTerminalWrite(pWidget, sText, (int)strlen(sText));
}

XUI_API int xuiTerminalFlush(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iQueueSize > 0 ) __xuiTerminalClearHoverLinkData(pData);
	return __xuiTerminalProcessQueue(pWidget, pData, 0);
}

XUI_API int xuiTerminalClear(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalClearCells(pData, __xuiTerminalScreen(pData));
	pData->iCursorX = 0;
	pData->iCursorY = 0;
	pData->bWrapPending = 0;
	__xuiTerminalClearSelectionData(pData);
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalClearHoverLinkData(pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalClearScrollback(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalFreeScrollback(pData);
	if ( __xuiTerminalAllocScrollback(pData, pData->iScrollbackLimit) != XUI_OK ) return XUI_ERROR_OUT_OF_MEMORY;
	__xuiTerminalClearSelectionData(pData);
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalClearHoverLinkData(pData);
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalSetScrollbackLimit(xui_widget pWidget, int iLimit)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	xui_terminal_history_line_t** ppOldHistory;
	xui_terminal_history_line_t** ppNewHistory;
	int iOldLimit;
	int iOldStart;
	int iOldCount;
	int iKeepCount;
	int iDropCount;
	int i;
	int iOldIndex;

	if ( pData == NULL || iLimit <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iLimit == pData->iScrollbackLimit ) return XUI_OK;
	ppNewHistory = (xui_terminal_history_line_t**)xrtCalloc((size_t)iLimit, sizeof(*ppNewHistory));
	if ( ppNewHistory == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	__xuiTerminalHistoryViewClear(pData);
	ppOldHistory = pData->ppHistory;
	iOldLimit = pData->iScrollbackLimit;
	iOldStart = pData->iScrollbackStart;
	iOldCount = pData->iHistoryCount;
	iKeepCount = (iOldCount < iLimit) ? iOldCount : iLimit;
	iDropCount = iOldCount - iKeepCount;
	if ( iOldLimit <= 0 ) {
		iOldLimit = 1;
		iOldStart = 0;
		iOldCount = 0;
		iKeepCount = 0;
		iDropCount = 0;
	}
	for ( i = 0; i < iDropCount; ++i ) {
		iOldIndex = (iOldStart + i) % iOldLimit;
		if ( ppOldHistory != NULL ) {
			__xuiTerminalHistoryLineDestroy(ppOldHistory[iOldIndex]);
			ppOldHistory[iOldIndex] = NULL;
		}
	}
	for ( i = 0; i < iKeepCount; ++i ) {
		iOldIndex = (iOldStart + iDropCount + i) % iOldLimit;
		if ( ppOldHistory != NULL ) {
			ppNewHistory[i] = ppOldHistory[iOldIndex];
			ppOldHistory[iOldIndex] = NULL;
		}
	}
	xrtFree(ppOldHistory);
	pData->ppHistory = ppNewHistory;
	pData->iScrollbackLimit = iLimit;
	pData->iScrollbackStart = 0;
	pData->iHistoryCount = iKeepCount;
	pData->bHistoryViewDirty = 1;
	(void)__xuiTerminalRebuildHistoryView(pData);
	__xuiTerminalClearSelectionData(pData);
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetScrollbackLimit(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->iScrollbackLimit : 0;
}

XUI_API int xuiTerminalSetParseBudget(xui_widget pWidget, int iBytesPerUpdate)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL || iBytesPerUpdate < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iParseBudget = iBytesPerUpdate;
	return XUI_OK;
}

XUI_API int xuiTerminalGetParseBudget(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->iParseBudget : 0;
}

XUI_API int xuiTerminalSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->pBaseFont = pFont;
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API xui_font xuiTerminalGetFont(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->pBaseFont : NULL;
}

XUI_API int xuiTerminalSetMetrics(xui_widget pWidget, float fCellWidth, float fCellHeight, float fPadding)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL || fCellWidth < 0.0f || fCellHeight < 0.0f || fPadding < 0.0f ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->fBaseCellWidth = fCellWidth;
	pData->fBaseCellHeight = fCellHeight;
	pData->fBasePadding = fPadding;
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetMetrics(xui_widget pWidget, float* pCellWidth, float* pCellHeight, float* pPadding)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	if ( pCellWidth != NULL ) *pCellWidth = pData->fCellWidth;
	if ( pCellHeight != NULL ) *pCellHeight = pData->fCellHeight;
	if ( pPadding != NULL ) *pPadding = pData->fPadding;
	return XUI_OK;
}

XUI_API int xuiTerminalSetColors(xui_widget pWidget, uint32_t iBackground, uint32_t iForeground, uint32_t iCursor, uint32_t iSelection, uint32_t iSelectionText, uint32_t iSearchHighlight, uint32_t iFocus, uint32_t iLinkHover)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iBaseBackgroundColor = iBackground;
	pData->iBaseForegroundColor = iForeground;
	pData->iBaseCursorColor = iCursor;
	pData->iBaseSelectionColor = iSelection;
	pData->iBaseSelectionTextColor = iSelectionText;
	pData->iBaseSearchHighlightColor = iSearchHighlight;
	pData->iBaseFocusColor = iFocus;
	pData->iBaseLinkHoverColor = iLinkHover;
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetColors(xui_widget pWidget, uint32_t* pBackground, uint32_t* pForeground, uint32_t* pCursor, uint32_t* pSelection, uint32_t* pSelectionText, uint32_t* pSearchHighlight, uint32_t* pFocus, uint32_t* pLinkHover)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pForeground != NULL ) *pForeground = pData->iForegroundColor;
	if ( pCursor != NULL ) *pCursor = pData->iCursorColor;
	if ( pSelection != NULL ) *pSelection = pData->iSelectionColor;
	if ( pSelectionText != NULL ) *pSelectionText = pData->iSelectionTextColor;
	if ( pSearchHighlight != NULL ) *pSearchHighlight = pData->iSearchHighlightColor;
	if ( pFocus != NULL ) *pFocus = pData->iFocusColor;
	if ( pLinkHover != NULL ) *pLinkHover = pData->iLinkHoverColor;
	return XUI_OK;
}

XUI_API int xuiTerminalSetLigaturesEnabled(xui_widget pWidget, int bEnabled)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bLigaturesEnabled = bEnabled ? 1 : 0;
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetLigaturesEnabled(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->bLigaturesEnabled : 0;
}

XUI_API int xuiTerminalFit(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	xui_rect_t tRect;
	int iColumns;
	int iRows;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalResolveStyle(pWidget, pData);
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	iColumns = (int)((tRect.fW - pData->fPadding * 2.0f) / pData->fCellWidth);
	iRows = (int)((tRect.fH - pData->fPadding * 2.0f) / pData->fCellHeight);
	if ( iColumns < 1 ) iColumns = 1;
	if ( iRows < 1 ) iRows = 1;
	if ( iColumns == pData->iColumns && iRows == pData->iRows ) return XUI_OK;
	iRet = __xuiTerminalResizeBuffers(pData, iColumns, iRows);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalNotifyResize(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalResize(xui_widget pWidget, int iColumns, int iRows)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTerminalResizeBuffers(pData, iColumns, iRows);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTerminalSyncScrollModel(pWidget, pData);
	__xuiTerminalNotifyResize(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetColumns(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->iColumns : 0;
}

XUI_API int xuiTerminalGetRows(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->iRows : 0;
}

XUI_API int xuiTerminalGetCursor(xui_widget pWidget, int* pColumn, int* pRow)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pColumn != NULL ) *pColumn = pData->iCursorX;
	if ( pRow != NULL ) *pRow = pData->iCursorY;
	return XUI_OK;
}

XUI_API int xuiTerminalSetInputCallback(xui_widget pWidget, xui_terminal_data_proc onData, void* pUser)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onData = onData;
	pData->pDataUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTerminalSetResizeCallback(xui_widget pWidget, xui_terminal_resize_proc onResize, void* pUser)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onResize = onResize;
	pData->pResizeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTerminalSetTitleCallback(xui_widget pWidget, xui_terminal_title_proc onTitle, void* pUser)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onTitle = onTitle;
	pData->pTitleUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTerminalSetLinkCallback(xui_widget pWidget, xui_terminal_link_proc onLink, void* pUser)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onLink = onLink;
	pData->pLinkUser = pUser;
	return XUI_OK;
}

XUI_API int xuiTerminalSetPalette(xui_widget pWidget, int iIndex, uint32_t iColor)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= 256 ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->arrBasePalette[iIndex] = iColor;
	__xuiTerminalResolveStyle(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API uint32_t xuiTerminalGetPalette(xui_widget pWidget, int iIndex)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL || iIndex < 0 || iIndex >= 256 ) return 0;
	__xuiTerminalResolveStyle(pWidget, pData);
	return pData->arrPalette[iIndex];
}

XUI_API xui_scroll_model_t* xuiTerminalGetScrollModel(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? &pData->tScroll : NULL;
}

XUI_API int xuiTerminalGetCell(xui_widget pWidget, int iColumn, int iRow, xui_terminal_cell_t* pCell)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	const xui_terminal_cell_t* pCells;
	if ( pData == NULL || pCell == NULL || iColumn < 0 || iRow < 0 || iColumn >= pData->iColumns || iRow >= pData->iRows ) return XUI_ERROR_INVALID_ARGUMENT;
	pCells = __xuiTerminalScreenConst(pData);
	*pCell = __xuiTerminalBufferRowConst(pData, pCells, iRow)[iColumn];
	pCell->iSize = sizeof(*pCell);
	return XUI_OK;
}

XUI_API int xuiTerminalSetBracketedPaste(xui_widget pWidget, int bEnabled)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bBracketedPaste = bEnabled ? 1 : 0;
	return XUI_OK;
}

XUI_API int xuiTerminalGetBracketedPaste(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->bBracketedPaste : 0;
}

XUI_API int xuiTerminalInputText(xui_widget pWidget, const char* sText)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sText, (int)strlen(sText));
}

XUI_API int xuiTerminalPasteText(xui_widget pWidget, const char* sText)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	int iRet;

	if ( pData == NULL || sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bBracketedPaste ) {
		iRet = __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)"\x1b[200~", 6);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sText, (int)strlen(sText));
		if ( iRet != XUI_OK ) return iRet;
		return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)"\x1b[201~", 6);
	}
	return __xuiTerminalEmitInput(pWidget, pData, (const uint8_t*)sText, (int)strlen(sText));
}

XUI_API int xuiTerminalSelectAll(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->bSelectAll = 1;
	pData->bSelecting = 0;
	pData->iSelectAnchorLine = 0;
	pData->iSelectAnchorColumn = 0;
	pData->iSelectEndLine = __xuiTerminalLogicalLineCount(pData) - 1;
	pData->iSelectEndColumn = pData->iColumns;
	__xuiTerminalEmitSelectionChange(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalClearSelection(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalClearSelectionData(pData);
	__xuiTerminalEmitSelectionChange(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalSetSelectionRange(xui_widget pWidget, int iAnchorLine, int iAnchorColumn, int iEndLine, int iEndColumn)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiTerminalSetSelectionRange(pData, iAnchorLine, iAnchorColumn, iEndLine, iEndColumn);
	__xuiTerminalEmitSelectionChange(pWidget, pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetSelectionRange(xui_widget pWidget, int* pAnchorLine, int* pAnchorColumn, int* pEndLine, int* pEndColumn)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	int iLine0;
	int iColumn0;
	int iLine1;
	int iColumn1;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiTerminalSelectionRange(pData, &iLine0, &iColumn0, &iLine1, &iColumn1) ) {
		iLine0 = -1;
		iColumn0 = 0;
		iLine1 = -1;
		iColumn1 = 0;
	}
	if ( pAnchorLine != NULL ) *pAnchorLine = iLine0;
	if ( pAnchorColumn != NULL ) *pAnchorColumn = iColumn0;
	if ( pEndLine != NULL ) *pEndLine = iLine1;
	if ( pEndColumn != NULL ) *pEndColumn = iColumn1;
	return XUI_OK;
}

XUI_API int xuiTerminalGetSelectionText(xui_widget pWidget, char* sBuffer, int iCapacity)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalSelectionTextToBuffer(pData, sBuffer, iCapacity);
}

XUI_API int xuiTerminalCopySelection(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	xui_proxy pProxy;
	char* sText;
	int iNeeded;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNeeded = xuiTerminalGetSelectionText(pWidget, NULL, 0);
	if ( iNeeded <= 0 ) return XUI_OK;
	sText = (char*)xrtMalloc((size_t)iNeeded + 1u);
	if ( sText == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	(void)xuiTerminalGetSelectionText(pWidget, sText, iNeeded + 1);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	iRet = (pProxy != NULL && pProxy->clipboardSetText != NULL) ? pProxy->clipboardSetText(pProxy, sText) : XUI_ERROR_UNSUPPORTED;
	xrtFree(sText);
	return iRet;
}

XUI_API int xuiTerminalSerializeText(xui_widget pWidget, char* sBuffer, int iCapacity)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalSerializeToBuffer(pData, sBuffer, iCapacity);
}

XUI_API int xuiTerminalFindText(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalFindNavigate(pWidget, pData, sText, iFlags, 1, 0, pLine, pColumn);
}

XUI_API int xuiTerminalFindNext(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalFindNavigate(pWidget, pData, sText, iFlags, 1, 1, pLine, pColumn);
}

XUI_API int xuiTerminalFindPrev(xui_widget pWidget, const char* sText, uint32_t iFlags, int* pLine, int* pColumn)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalFindNavigate(pWidget, pData, sText, iFlags, 0, 1, pLine, pColumn);
}

XUI_API int xuiTerminalClearFind(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->sSearchText != NULL ) {
		xrtFree(pData->sSearchText);
		pData->sSearchText = NULL;
	}
	pData->iSearchFlags = 0;
	__xuiTerminalClearFindMatch(pData);
	__xuiTerminalInvalidate(pWidget, pData, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return XUI_OK;
}

XUI_API int xuiTerminalGetFindMatch(xui_widget pWidget, int* pLine, int* pColumn, int* pLength)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pLine != NULL ) *pLine = pData->iSearchLine;
	if ( pColumn != NULL ) *pColumn = pData->iSearchColumn;
	if ( pLength != NULL ) *pLength = pData->iSearchLength;
	return (pData->iSearchLine >= 0 && pData->iSearchLength > 0) ? 1 : 0;
}

XUI_API int xuiTerminalOpenFind(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiTerminalOpenFindWindow(pWidget, pData);
}

XUI_API xui_widget xuiTerminalGetFindWindow(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return pData != NULL ? pData->pFindWindow : NULL;
}

XUI_API int xuiTerminalGetLinkAt(xui_widget pWidget, int iLine, int iColumn, char* sBuffer, int iCapacity, int* pStartColumn, int* pLength)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	char* sUrl;
	int iRet;
	int iCopy;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sBuffer != NULL && iCapacity > 0 ) sBuffer[0] = '\0';
	sUrl = NULL;
	iRet = __xuiTerminalLogicalUrlAt(pData, iLine, iColumn, &sUrl, pStartColumn, pLength);
	if ( iRet <= 0 ) return iRet;
	if ( sBuffer != NULL && iCapacity > 0 && sUrl != NULL ) {
		iCopy = __xuiTerminalMin(iRet, iCapacity - 1);
		if ( iCopy > 0 ) memcpy(sBuffer, sUrl, (size_t)iCopy);
		sBuffer[iCopy] = '\0';
	}
	if ( sUrl != NULL ) xrtFree(sUrl);
	return iRet;
}

XUI_API int xuiTerminalOpenMenu(xui_widget pWidget, float fX, float fY)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	int iRet;

	if ( (pData == NULL) || (pData->pMenu == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiTerminalUpdateMenu(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiMenuSetSelect(pData->pMenu, __xuiTerminalMenuSelect, pWidget);
	return xuiMenuOpenAt(pData->pMenu, pWidget, fX, fY);
}

XUI_API xui_widget xuiTerminalGetMenuWidget(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->pMenu : NULL;
}

XUI_API int xuiTerminalSetMenuTitle(xui_widget pWidget, int iCommand, const char* sTitle)
{
	xui_terminal_data_t* pData;
	char* sNew;
	int iIndex;

	pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iIndex = __xuiTerminalMenuTitleIndexForCommand(iCommand);
	if ( iIndex < 0 || iIndex >= XUI_TERMINAL_MENU_TITLE_COUNT ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( sTitle == NULL || sTitle[0] == '\0' ) {
		xrtFree(pData->arrMenuTitle[iIndex]);
		pData->arrMenuTitle[iIndex] = NULL;
		if ( pData->pMenu != NULL ) (void)__xuiTerminalUpdateMenu(pWidget, pData);
		return XUI_OK;
	}
	sNew = __xuiTerminalStrDup(sTitle);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	xrtFree(pData->arrMenuTitle[iIndex]);
	pData->arrMenuTitle[iIndex] = sNew;
	if ( pData->pMenu != NULL ) (void)__xuiTerminalUpdateMenu(pWidget, pData);
	return XUI_OK;
}

XUI_API const char* xuiTerminalGetMenuTitle(xui_widget pWidget, int iCommand)
{
	xui_terminal_data_t* pData;

	pData = __xuiTerminalGetData(pWidget);
	return __xuiTerminalMenuTitleForCommand(pWidget, pData, iCommand);
}

XUI_API int xuiTerminalAttachSession(xui_widget pWidget, xui_terminal_session_t* pSession)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	xui_terminal_data_t* pPreviousData;
	if ( pData == NULL || pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pSession->pWidget != NULL) && (pSession->pWidget != pWidget) ) {
		pPreviousData = __xuiTerminalGetData(pSession->pWidget);
		if ( (pPreviousData != NULL) && (pPreviousData->pSession == pSession) ) {
			pPreviousData->pSession = NULL;
		}
	}
	if ( pData->pSession != NULL && pData->pSession->pWidget == pWidget ) pData->pSession->pWidget = NULL;
	pData->pSession = pSession;
	pSession->pWidget = pWidget;
	__xuiTerminalNotifyResize(pWidget, pData);
	if ( pSession->sPrompt != NULL && pSession->sPrompt[0] != '\0' ) {
		(void)xuiTerminalWriteText(pWidget, pSession->sPrompt);
		(void)xuiTerminalFlush(pWidget);
	}
	return XUI_OK;
}

XUI_API int xuiTerminalDetachSession(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pSession != NULL && pData->pSession->pWidget == pWidget ) pData->pSession->pWidget = NULL;
	pData->pSession = NULL;
	return XUI_OK;
}

static void __xuiTerminalProcessCloseHandles(xui_terminal_session_t* pSession)
{
#if defined(_WIN32) || defined(_WIN64)
	xui_terminal_conpty_api_t tApi;

	if ( pSession == NULL ) return;
	if ( pSession->hPseudoConsole != NULL ) {
		if ( __xuiTerminalConptyGetApi(&tApi) ) {
			tApi.closePseudoConsole(pSession->hPseudoConsole);
		}
		pSession->hPseudoConsole = NULL;
	}
	if ( pSession->hInputWrite != NULL ) {
		CloseHandle(pSession->hInputWrite);
		pSession->hInputWrite = NULL;
	}
	if ( pSession->hOutputRead != NULL ) {
		CloseHandle(pSession->hOutputRead);
		pSession->hOutputRead = NULL;
	}
	if ( pSession->hProcess != NULL ) {
		CloseHandle(pSession->hProcess);
		pSession->hProcess = NULL;
	}
#else
	(void)pSession;
#endif
}

static int __xuiTerminalSessionResizeTransport(xui_terminal_session_t* pSession, int iColumns, int iRows)
{
#if defined(_WIN32) || defined(_WIN64)
	xui_terminal_conpty_api_t tApi;
	HRESULT iResult;

	if ( pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pSession->iKind != XUI_TERMINAL_SESSION_PROCESS ) return XUI_OK;
	if ( pSession->hPseudoConsole == NULL ) return XUI_OK;
	if ( !__xuiTerminalConptyGetApi(&tApi) ) return XUI_ERROR_UNSUPPORTED;
	iResult = tApi.resizePseudoConsole(pSession->hPseudoConsole, __xuiTerminalConptySize(iColumns, iRows));
	return SUCCEEDED(iResult) ? XUI_OK : XUI_ERROR_BACKEND_FAILED;
#else
	(void)pSession;
	(void)iColumns;
	(void)iRows;
	return XUI_ERROR_UNSUPPORTED;
#endif
}

static int __xuiTerminalProcessRefreshRunning(xui_terminal_session_t* pSession)
{
#if defined(_WIN32) || defined(_WIN64)
	DWORD iWait;

	if ( pSession == NULL || pSession->iKind != XUI_TERMINAL_SESSION_PROCESS ) return 0;
	if ( pSession->hProcess == NULL ) {
		pSession->bRunning = 0;
		return 0;
	}
	if ( !pSession->bRunning ) return 0;
	iWait = WaitForSingleObject(pSession->hProcess, 0);
	if ( iWait == WAIT_OBJECT_0 ) {
		pSession->bRunning = 0;
	}
	return pSession->bRunning;
#else
	(void)pSession;
	return 0;
#endif
}

static int __xuiTerminalProcessTerminate(xui_terminal_session_t* pSession)
{
#if defined(_WIN32) || defined(_WIN64)
	int iRet;

	if ( pSession == NULL || pSession->iKind != XUI_TERMINAL_SESSION_PROCESS ) return XUI_OK;
	iRet = XUI_OK;
	if ( pSession->hProcess != NULL && __xuiTerminalProcessRefreshRunning(pSession) ) {
		if ( !TerminateProcess(pSession->hProcess, 1u) ) {
			iRet = XUI_ERROR_BACKEND_FAILED;
		} else {
			(void)WaitForSingleObject(pSession->hProcess, 1000);
			pSession->bRunning = 0;
		}
	}
	__xuiTerminalProcessCloseHandles(pSession);
	return iRet;
#else
	(void)pSession;
	return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API xui_terminal_session_t* xuiTerminalCreateFakeSession(const xui_terminal_session_desc_t* pDesc)
{
	xui_terminal_session_t* pSession;

	pSession = (xui_terminal_session_t*)xrtCalloc(1, sizeof(*pSession));
	if ( pSession == NULL ) return NULL;
	pSession->iKind = XUI_TERMINAL_SESSION_FAKE;
	pSession->bRunning = 1;
	pSession->bEcho = (pDesc == NULL) ? 1 : (pDesc->bEcho ? 1 : 0);
	if ( pDesc != NULL && pDesc->iSize >= sizeof(*pDesc) ) {
		pSession->onResize = pDesc->onResize;
		pSession->pResizeUser = pDesc->pResizeUser;
	}
	pSession->sPrompt = __xuiTerminalStrDup((pDesc != NULL && pDesc->sPrompt != NULL) ? pDesc->sPrompt : "");
	if ( pSession->sPrompt == NULL ) {
		xrtFree(pSession);
		return NULL;
	}
	return pSession;
}

static int __xuiTerminalProcessCreatePipeTransport(xui_terminal_session_t* pSession, const xui_terminal_process_desc_t* pDesc, char* sCommandLine)
{
#if defined(_WIN32) || defined(_WIN64)
	SECURITY_ATTRIBUTES tSecurity;
	STARTUPINFOA tStartup;
	PROCESS_INFORMATION tProcessInfo;
	HANDLE hInputRead;
	HANDLE hInputWrite;
	HANDLE hOutputRead;
	HANDLE hOutputWrite;
	BOOL bCreated;

	hInputRead = NULL;
	hInputWrite = NULL;
	hOutputRead = NULL;
	hOutputWrite = NULL;
	memset(&tSecurity, 0, sizeof(tSecurity));
	tSecurity.nLength = sizeof(tSecurity);
	tSecurity.bInheritHandle = TRUE;
	if ( !CreatePipe(&hOutputRead, &hOutputWrite, &tSecurity, 0) ) goto failed;
	if ( !SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0) ) goto failed;
	if ( !CreatePipe(&hInputRead, &hInputWrite, &tSecurity, 0) ) goto failed;
	if ( !SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0) ) goto failed;

	memset(&tStartup, 0, sizeof(tStartup));
	memset(&tProcessInfo, 0, sizeof(tProcessInfo));
	tStartup.cb = sizeof(tStartup);
	tStartup.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	tStartup.wShowWindow = SW_HIDE;
	tStartup.hStdInput = hInputRead;
	tStartup.hStdOutput = hOutputWrite;
	tStartup.hStdError = hOutputWrite;
	bCreated = CreateProcessA(NULL, sCommandLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, pDesc->sWorkingDirectory, &tStartup, &tProcessInfo);
	if ( hInputRead != NULL ) {
		CloseHandle(hInputRead);
		hInputRead = NULL;
	}
	if ( hOutputWrite != NULL ) {
		CloseHandle(hOutputWrite);
		hOutputWrite = NULL;
	}
	if ( !bCreated ) goto failed;
	if ( tProcessInfo.hThread != NULL ) CloseHandle(tProcessInfo.hThread);
	pSession->hProcess = tProcessInfo.hProcess;
	pSession->hInputWrite = hInputWrite;
	pSession->hOutputRead = hOutputRead;
	hInputWrite = NULL;
	hOutputRead = NULL;
	return XUI_OK;

failed:
	if ( hInputRead != NULL ) CloseHandle(hInputRead);
	if ( hInputWrite != NULL ) CloseHandle(hInputWrite);
	if ( hOutputRead != NULL ) CloseHandle(hOutputRead);
	if ( hOutputWrite != NULL ) CloseHandle(hOutputWrite);
	return XUI_ERROR_BACKEND_FAILED;
#else
	(void)pSession;
	(void)pDesc;
	(void)sCommandLine;
	return XUI_ERROR_UNSUPPORTED;
#endif
}

static int __xuiTerminalProcessCreateConptyTransport(xui_terminal_session_t* pSession, const xui_terminal_process_desc_t* pDesc, char* sCommandLine)
{
#if defined(_WIN32) || defined(_WIN64)
	SECURITY_ATTRIBUTES tSecurity;
	STARTUPINFOEXA tStartup;
	PROCESS_INFORMATION tProcessInfo;
	xui_terminal_conpty_api_t tApi;
	LPPROC_THREAD_ATTRIBUTE_LIST pAttributeList;
	SIZE_T iAttributeListSize;
	HANDLE hInputRead;
	HANDLE hInputWrite;
	HANDLE hOutputRead;
	HANDLE hOutputWrite;
	HPCON hPseudoConsole;
	HRESULT iResult;
	BOOL bCreated;
	int bAttributeListReady;

	if ( !__xuiTerminalConptyGetApi(&tApi) ) return XUI_ERROR_UNSUPPORTED;
	hInputRead = NULL;
	hInputWrite = NULL;
	hOutputRead = NULL;
	hOutputWrite = NULL;
	hPseudoConsole = NULL;
	pAttributeList = NULL;
	iAttributeListSize = 0;
	bAttributeListReady = 0;
	memset(&tSecurity, 0, sizeof(tSecurity));
	tSecurity.nLength = sizeof(tSecurity);
	tSecurity.bInheritHandle = TRUE;
	if ( !CreatePipe(&hInputRead, &hInputWrite, &tSecurity, 0) ) goto failed;
	if ( !SetHandleInformation(hInputWrite, HANDLE_FLAG_INHERIT, 0) ) goto failed;
	if ( !CreatePipe(&hOutputRead, &hOutputWrite, &tSecurity, 0) ) goto failed;
	if ( !SetHandleInformation(hOutputRead, HANDLE_FLAG_INHERIT, 0) ) goto failed;
	iResult = tApi.createPseudoConsole(__xuiTerminalConptySize(__xuiTerminalProcessDescColumns(pDesc), __xuiTerminalProcessDescRows(pDesc)), hInputRead, hOutputWrite, 0, &hPseudoConsole);
	if ( FAILED(iResult) || hPseudoConsole == NULL ) goto failed;
	InitializeProcThreadAttributeList(NULL, 1, 0, &iAttributeListSize);
	if ( iAttributeListSize == 0 ) goto failed;
	pAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)xrtMalloc((size_t)iAttributeListSize);
	if ( pAttributeList == NULL ) goto failed;
	if ( !InitializeProcThreadAttributeList(pAttributeList, 1, 0, &iAttributeListSize) ) goto failed;
	bAttributeListReady = 1;
	if ( !UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPseudoConsole, sizeof(hPseudoConsole), NULL, NULL) ) goto failed;
	memset(&tStartup, 0, sizeof(tStartup));
	memset(&tProcessInfo, 0, sizeof(tProcessInfo));
	tStartup.StartupInfo.cb = sizeof(tStartup);
	tStartup.lpAttributeList = pAttributeList;
	bCreated = CreateProcessA(NULL, sCommandLine, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, pDesc->sWorkingDirectory, &tStartup.StartupInfo, &tProcessInfo);
	if ( !bCreated ) goto failed;
	if ( hInputRead != NULL ) {
		CloseHandle(hInputRead);
		hInputRead = NULL;
	}
	if ( hOutputWrite != NULL ) {
		CloseHandle(hOutputWrite);
		hOutputWrite = NULL;
	}
	if ( tProcessInfo.hThread != NULL ) CloseHandle(tProcessInfo.hThread);
	if ( bAttributeListReady ) DeleteProcThreadAttributeList(pAttributeList);
	xrtFree(pAttributeList);
	pSession->hProcess = tProcessInfo.hProcess;
	pSession->hInputWrite = hInputWrite;
	pSession->hOutputRead = hOutputRead;
	pSession->hPseudoConsole = hPseudoConsole;
	hInputWrite = NULL;
	hOutputRead = NULL;
	hPseudoConsole = NULL;
	return XUI_OK;

failed:
	if ( bAttributeListReady ) DeleteProcThreadAttributeList(pAttributeList);
	if ( pAttributeList != NULL ) xrtFree(pAttributeList);
	if ( hPseudoConsole != NULL ) tApi.closePseudoConsole(hPseudoConsole);
	if ( hInputRead != NULL ) CloseHandle(hInputRead);
	if ( hInputWrite != NULL ) CloseHandle(hInputWrite);
	if ( hOutputRead != NULL ) CloseHandle(hOutputRead);
	if ( hOutputWrite != NULL ) CloseHandle(hOutputWrite);
	return XUI_ERROR_BACKEND_FAILED;
#else
	(void)pSession;
	(void)pDesc;
	(void)sCommandLine;
	return XUI_ERROR_UNSUPPORTED;
#endif
}

XUI_API xui_terminal_session_t* xuiTerminalCreateProcessSession(const xui_terminal_process_desc_t* pDesc)
{
	xui_terminal_session_t* pSession;
	char* sCommandLine;
	uint32_t iFlags;
	int iRet;

	if ( pDesc == NULL || pDesc->iSize < (uint32_t)offsetof(xui_terminal_process_desc_t, iFlags) || pDesc->sCommandLine == NULL || pDesc->sCommandLine[0] == '\0' ) return NULL;
	pSession = (xui_terminal_session_t*)xrtCalloc(1, sizeof(*pSession));
	if ( pSession == NULL ) return NULL;
	sCommandLine = __xuiTerminalStrDup(pDesc->sCommandLine);
	if ( sCommandLine == NULL ) {
		xrtFree(pSession);
		return NULL;
	}
	iFlags = __xuiTerminalProcessDescFlags(pDesc);
	pSession->iKind = XUI_TERMINAL_SESSION_PROCESS;
	pSession->bRunning = 1;
	pSession->bEcho = 0;
	pSession->iProcessFlags = iFlags;
	pSession->iLastColumns = __xuiTerminalProcessDescColumns(pDesc);
	pSession->iLastRows = __xuiTerminalProcessDescRows(pDesc);
	pSession->onResize = pDesc->onResize;
	pSession->pResizeUser = pDesc->pResizeUser;
	pSession->sPrompt = __xuiTerminalStrDup("");
	if ( pSession->sPrompt == NULL ) {
		xrtFree(sCommandLine);
		xrtFree(pSession);
		return NULL;
	}
	if ( (iFlags & XUI_TERMINAL_PROCESS_CONPTY) != 0u ) {
		iRet = __xuiTerminalProcessCreateConptyTransport(pSession, pDesc, sCommandLine);
	} else {
		iRet = __xuiTerminalProcessCreatePipeTransport(pSession, pDesc, sCommandLine);
	}
	xrtFree(sCommandLine);
	if ( iRet != XUI_OK ) {
		__xuiTerminalProcessCloseHandles(pSession);
		if ( pSession->sPrompt != NULL ) xrtFree(pSession->sPrompt);
		xrtFree(pSession);
		return NULL;
	}
	return pSession;
}

XUI_API void xuiTerminalSessionDestroy(xui_terminal_session_t* pSession)
{
	if ( pSession == NULL ) return;
	if ( pSession->pWidget != NULL ) {
		xui_terminal_data_t* pData = __xuiTerminalGetData(pSession->pWidget);
		if ( (pData != NULL) && (pData->pSession == pSession) ) {
			pData->pSession = NULL;
		}
		pSession->pWidget = NULL;
	}
	if ( pSession->iKind == XUI_TERMINAL_SESSION_PROCESS ) {
		(void)__xuiTerminalProcessTerminate(pSession);
	}
	if ( pSession->sPrompt != NULL ) xrtFree(pSession->sPrompt);
	xrtFree(pSession);
}

XUI_API int xuiTerminalSessionWrite(xui_terminal_session_t* pSession, const void* pDataBytes, int iSize)
{
	const uint8_t* pBytes;
#if defined(_WIN32) || defined(_WIN64)
	DWORD iWritten;
#endif
	int i;

	if ( pSession == NULL || iSize < 0 || (pDataBytes == NULL && iSize > 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iSize == 0 ) return XUI_OK;
	if ( pSession->iKind == XUI_TERMINAL_SESSION_PROCESS ) {
#if defined(_WIN32) || defined(_WIN64)
		if ( pSession->hInputWrite == NULL || !__xuiTerminalProcessRefreshRunning(pSession) ) return XUI_ERROR_BACKEND_FAILED;
		if ( !WriteFile(pSession->hInputWrite, pDataBytes, (DWORD)iSize, &iWritten, NULL) || iWritten != (DWORD)iSize ) return XUI_ERROR_BACKEND_FAILED;
		return XUI_OK;
#else
		return XUI_ERROR_UNSUPPORTED;
#endif
	}
	if ( !pSession->bEcho || pSession->pWidget == NULL ) return XUI_OK;
	pBytes = (const uint8_t*)pDataBytes;
	for ( i = 0; i < iSize; i++ ) {
		if ( pBytes[i] == '\r' || pBytes[i] == '\n' ) {
			(void)xuiTerminalWriteText(pSession->pWidget, "\r\n");
			if ( pSession->sPrompt != NULL && pSession->sPrompt[0] != '\0' ) {
				(void)xuiTerminalWriteText(pSession->pWidget, pSession->sPrompt);
			}
		} else if ( pBytes[i] == 0x7fu || pBytes[i] == 0x08u ) {
			(void)xuiTerminalWriteText(pSession->pWidget, "\b \b");
		} else {
			(void)xuiTerminalWrite(pSession->pWidget, &pBytes[i], 1);
		}
	}
	return xuiTerminalFlush(pSession->pWidget);
}

XUI_API int xuiTerminalSessionPoll(xui_terminal_session_t* pSession)
{
#if defined(_WIN32) || defined(_WIN64)
	uint8_t arrBuffer[XUI_TERMINAL_PROCESS_READ_CHUNK];
	DWORD iAvailable;
	DWORD iRead;
	DWORD iChunk;
	DWORD iError;
	int iTotal;
	int iRet;
	int iPoll;

	if ( pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pSession->iKind != XUI_TERMINAL_SESSION_PROCESS ) return XUI_OK;
	if ( pSession->hOutputRead == NULL ) {
		(void)__xuiTerminalProcessRefreshRunning(pSession);
		return XUI_OK;
	}
	iTotal = 0;
	for ( iPoll = 0; iPoll < XUI_TERMINAL_PROCESS_POLL_LIMIT; iPoll++ ) {
		iAvailable = 0;
		if ( !PeekNamedPipe(pSession->hOutputRead, NULL, 0, NULL, &iAvailable, NULL) ) {
			iError = GetLastError();
			if ( iError == ERROR_BROKEN_PIPE || iError == ERROR_HANDLE_EOF ) {
				pSession->bRunning = 0;
				break;
			}
			return (iTotal > 0) ? iTotal : XUI_ERROR_BACKEND_FAILED;
		}
		if ( iAvailable == 0 ) break;
		iChunk = (iAvailable > XUI_TERMINAL_PROCESS_READ_CHUNK) ? XUI_TERMINAL_PROCESS_READ_CHUNK : iAvailable;
		if ( !ReadFile(pSession->hOutputRead, arrBuffer, iChunk, &iRead, NULL) ) {
			iError = GetLastError();
			if ( iError == ERROR_BROKEN_PIPE || iError == ERROR_HANDLE_EOF ) {
				pSession->bRunning = 0;
				break;
			}
			return (iTotal > 0) ? iTotal : XUI_ERROR_BACKEND_FAILED;
		}
		if ( iRead == 0 ) break;
		iTotal += (int)iRead;
		if ( pSession->pWidget != NULL ) {
			iRet = xuiTerminalWrite(pSession->pWidget, arrBuffer, (int)iRead);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	if ( iTotal > 0 && pSession->pWidget != NULL ) {
		iRet = xuiTerminalFlush(pSession->pWidget);
		if ( iRet != XUI_OK ) return iRet;
	}
	(void)__xuiTerminalProcessRefreshRunning(pSession);
	return iTotal;
#else
	if ( pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return (pSession->iKind == XUI_TERMINAL_SESSION_PROCESS) ? XUI_ERROR_UNSUPPORTED : XUI_OK;
#endif
}

XUI_API int xuiTerminalSessionIsRunning(xui_terminal_session_t* pSession)
{
	if ( pSession == NULL ) return 0;
	if ( pSession->iKind == XUI_TERMINAL_SESSION_PROCESS ) {
		return __xuiTerminalProcessRefreshRunning(pSession);
	}
	return pSession->bRunning;
}

XUI_API int xuiTerminalSessionTerminate(xui_terminal_session_t* pSession)
{
	if ( pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pSession->iKind == XUI_TERMINAL_SESSION_PROCESS ) {
		return __xuiTerminalProcessTerminate(pSession);
	}
	pSession->bRunning = 0;
	return XUI_OK;
}

XUI_API int xuiTerminalSessionResize(xui_terminal_session_t* pSession, int iColumns, int iRows)
{
	int iRet;

	if ( pSession == NULL || iColumns <= 0 || iRows <= 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pSession->iLastColumns == iColumns && pSession->iLastRows == iRows ) {
		if ( pSession->onResize != NULL ) {
			pSession->onResize(pSession, iColumns, iRows, pSession->pResizeUser);
		}
		return XUI_OK;
	}
	iRet = __xuiTerminalSessionResizeTransport(pSession, iColumns, iRows);
	if ( iRet != XUI_OK ) return iRet;
	pSession->iLastColumns = iColumns;
	pSession->iLastRows = iRows;
	if ( pSession->onResize != NULL ) {
		pSession->onResize(pSession, iColumns, iRows, pSession->pResizeUser);
	}
	return XUI_OK;
}

XUI_API int xuiTerminalSessionSetResizeCallback(xui_terminal_session_t* pSession, xui_terminal_session_resize_proc onResize, void* pUser)
{
	xui_terminal_data_t* pData;

	if ( pSession == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pSession->onResize = onResize;
	pSession->pResizeUser = pUser;
	if ( pSession->pWidget != NULL && pSession->onResize != NULL ) {
		pData = __xuiTerminalGetData(pSession->pWidget);
		if ( pData != NULL && pData->pSession == pSession && pData->iColumns > 0 && pData->iRows > 0 ) {
			return xuiTerminalSessionResize(pSession, pData->iColumns, pData->iRows);
		}
	}
	return XUI_OK;
}

XUI_API int xuiTerminalGetChangeCount(xui_widget pWidget)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

XUI_API int xuiTerminalGetStats(xui_widget pWidget, xui_terminal_stats_t* pStats)
{
	xui_terminal_data_t* pData = __xuiTerminalGetData(pWidget);
	size_t iHistoryBytes;
	int i;
	int idx;

	if ( pData == NULL || pStats == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( __xuiTerminalRebuildHistoryView(pData) != XUI_OK ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(pStats, 0, sizeof(*pStats));
	pStats->iSize = sizeof(*pStats);
	pStats->iColumns = pData->iColumns;
	pStats->iRows = pData->iRows;
	pStats->iHistoryLogicalLines = pData->iHistoryCount;
	pStats->iHistoryDisplayRows = pData->iScrollbackCount;
	pStats->iQueuedBytes = pData->iQueueSize;
	pStats->iQueueCapacity = pData->iQueueCapacity;
	pStats->iQueuePeakBytes = pData->iQueuePeak;
	pStats->iDirtyRows = pData->iDirtyRowCount;
	pStats->iChangeCount = pData->iChangeCount;
	pStats->iOutputBytesReceived = pData->iOutputBytesReceived;
	pStats->iOutputBytesParsed = pData->iOutputBytesParsed;
	pStats->iScreenMemoryBytes =
		sizeof(xui_terminal_cell_t) * (size_t)pData->iColumns * (size_t)pData->iRows * 2u +
		(size_t)pData->iRows * 3u + (size_t)pData->iColumns;
	pStats->iQueueMemoryBytes = (size_t)pData->iQueueCapacity;
	iHistoryBytes = sizeof(*pData->ppHistory) * (size_t)pData->iScrollbackLimit +
		sizeof(*pData->pHistoryView) * (size_t)pData->iHistoryViewCapacity +
		sizeof(*pData->pHistoryRenderCells) * (size_t)pData->iHistoryRenderCapacity;
	for ( i = 0; i < pData->iHistoryCount; ++i ) {
		xui_terminal_history_line_t* pLine;
		idx = (pData->iScrollbackStart + i) % pData->iScrollbackLimit;
		pLine = pData->ppHistory != NULL ? pData->ppHistory[idx] : NULL;
		if ( pLine == NULL ) continue;
		iHistoryBytes += sizeof(*pLine) +
			sizeof(*pLine->pCells) * (size_t)pLine->iCellCapacity +
			sizeof(*pLine->pCombining) * (size_t)pLine->iCombiningCapacity;
		if ( pLine->sTextCache != NULL ) iHistoryBytes += strlen(pLine->sTextCache) + 1u;
	}
	for ( i = 0; i < pData->iScrollbackCount; ++i ) {
		if ( pData->pHistoryView[i].sTextCache != NULL ) {
			iHistoryBytes += strlen(pData->pHistoryView[i].sTextCache) + 1u;
		}
	}
	pStats->iHistoryMemoryBytes = iHistoryBytes;
	return XUI_OK;
}
