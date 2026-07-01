#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 920
#define DEMO_TARGET_H 430
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define TABLE_COUNT 3
#define LABEL_COUNT 5
#define DEMO_ROWS 96
#define DEMO_COLS 6

typedef struct xui_tableview_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pTable[TABLE_COUNT];
	xui_widget pStatus;
	xui_table_view_column_t arrColumns[DEMO_COLS];
	xui_table_view_row_t arrRows[DEMO_ROWS];
	char arrText[DEMO_ROWS][DEMO_COLS][64];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iSelectCount;
	int iLastRow;
	int iLastColumn;
	int iCustomPaintCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bSelectOK;
	int bSortOK;
	int bScrollOK;
	int bCustomOK;
} xui_tableview_demo_t;

static void __xuiTableViewUsage(void)
{
	printf("usage: xui_tableview [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTableViewParseArgs(xui_tableview_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiTableViewUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTableViewFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\simhei.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrPaths) / sizeof(arrPaths[0])); i++ ) {
		pFile = fopen(arrPaths[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrPaths[i];
		}
	}
	return NULL;
}

static int __xuiTableViewRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tableview_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_tableview_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 22.0f, tRect.fW - 48.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiTableViewIndexOf(xui_tableview_demo_t* pDemo, xui_widget pWidget)
{
	int i;

	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( pDemo->pTable[i] == pWidget ) return i;
	}
	return -1;
}

static int __xuiTableViewRowCount(xui_widget pWidget, void* pUser)
{
	xui_tableview_demo_t* pDemo;
	int iIndex;

	pDemo = (xui_tableview_demo_t*)pUser;
	if ( pDemo == NULL ) return 0;
	iIndex = __xuiTableViewIndexOf(pDemo, pWidget);
	if ( iIndex == 0 ) return 14;
	if ( iIndex == 2 ) return 26;
	return DEMO_ROWS;
}

static int __xuiTableViewCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	xui_tableview_demo_t* pDemo;
	int iIndex;

	pDemo = (xui_tableview_demo_t*)pUser;
	if ( (pDemo == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= DEMO_ROWS) || (iColumn >= DEMO_COLS) ) return 0;
	iIndex = __xuiTableViewIndexOf(pDemo, pWidget);
	pCell->sText = pDemo->arrText[iRow][iColumn];
	pCell->sTooltip = (iIndex == 2 && iRow == 2 && iColumn == 1) ? "merged table cell" : NULL;
	if ( iColumn == 2 ) pCell->iType = XUI_TABLE_CELL_TYPE_BOOL;
	if ( iColumn == 3 ) pCell->iType = XUI_TABLE_CELL_TYPE_COLOR;
	if ( iColumn == 4 ) pCell->iType = XUI_TABLE_CELL_TYPE_PICKER;
	if ( iIndex == 2 && iRow == 6 && iColumn == 2 ) pCell->bInvalid = 1;
	if ( iIndex == 2 && iRow == 7 && iColumn == 3 ) pCell->bDirty = 1;
	return 1;
}

static int __xuiTableViewMerge(xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser)
{
	xui_tableview_demo_t* pDemo;

	pDemo = (xui_tableview_demo_t*)pUser;
	if ( (pDemo != NULL) && (__xuiTableViewIndexOf(pDemo, pWidget) == 2) && (iRow == 2) && (iColumn == 1) ) {
		*pRowSpan = 2;
		*pColSpan = 2;
		return 1;
	}
	return 0;
}

static void __xuiTableViewSelect(xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	xui_tableview_demo_t* pDemo;
	char sText[160];

	(void)pWidget;
	(void)iMode;
	pDemo = (xui_tableview_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastRow = iRow;
	pDemo->iLastColumn = iColumn;
	snprintf(sText, sizeof(sText), "Selected row=%d column=%d changes=%d", iRow, iColumn, pDemo->iSelectCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiTableViewCustomCell(xui_widget pWidget, int iRow, int iColumn, const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	xui_tableview_demo_t* pDemo;
	xui_proxy pProxy;
	xui_rect_t tBar;
	xui_rect_t tFill;
	float fRate;
	uint32_t iAccent;

	(void)pCell;
	(void)iState;
	pDemo = (xui_tableview_demo_t*)pUser;
	if ( pDemo == NULL ) return 0;
	pDemo->iCustomPaintCount++;
	if ( (__xuiTableViewIndexOf(pDemo, pWidget) != 2) || (iColumn != 5) ) {
		return 0;
	}
	pProxy = &pDemo->tProxy;
	fRate = (float)((iRow % 10) + 1) / 10.0f;
	tBar = (xui_rect_t){tRect.fX + 8.0f, tRect.fY + tRect.fH * 0.5f - 4.0f, tRect.fW - 16.0f, 8.0f};
	tFill = tBar;
	tFill.fW *= fRate;
	iAccent = XUI_COLOR_RGBA(51, 143, 105, 245);
	if ( pProxy->drawRectFill != NULL ) {
		(void)pProxy->drawRectFill(pProxy, pDraw, tBar, XUI_COLOR_RGBA(220, 235, 229, 255));
		(void)pProxy->drawRectFill(pProxy, pDraw, tFill, iAccent);
	}
	return 1;
}

static int __xuiTableViewAddLabel(xui_tableview_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiTableViewAddTable(xui_tableview_demo_t* pDemo, int iIndex, xui_rect_t tRect)
{
	xui_table_view_desc_t tDesc;
	xui_widget pTable;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = pDemo->arrColumns;
	tDesc.iColumnCount = DEMO_COLS;
	tDesc.arrRows = pDemo->arrRows;
	tDesc.iRowCount = DEMO_ROWS;
	tDesc.onCount = __xuiTableViewRowCount;
	tDesc.onCell = __xuiTableViewCell;
	tDesc.pAdapterUser = pDemo;
	tDesc.pFont = pDemo->pFont;
	tDesc.fDefaultRowHeight = 22.0f;
	tDesc.fHeaderHeight = 25.0f;
	iRet = xuiTableViewCreate(pDemo->pContext, &pTable, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiTableViewSetSelect(pTable, __xuiTableViewSelect, pDemo);
	(void)xuiTableViewSetMergeProvider(pTable, __xuiTableViewMerge, pDemo);
	if ( iIndex == 0 ) {
		(void)xuiTableViewSetSelectionMode(pTable, XUI_TABLE_VIEW_SELECTION_ROW);
	} else if ( iIndex == 2 ) {
		(void)xuiTableViewSetCellRenderer(pTable, __xuiTableViewCustomCell, pDemo);
		(void)xuiTableViewSetColors(pTable,
			XUI_COLOR_RGBA(247, 253, 250, 255),
			XUI_COLOR_RGBA(226, 242, 235, 255),
			XUI_COLOR_RGBA(255, 255, 255, 255),
			XUI_COLOR_RGBA(42, 151, 110, 255),
			XUI_COLOR_RGBA(189, 221, 210, 255),
			XUI_COLOR_RGBA(31, 76, 60, 255));
		(void)xuiTableViewSetScrollbarColors(pTable,
			XUI_COLOR_RGBA(219, 239, 233, 255),
			XUI_COLOR_RGBA(47, 150, 110, 245),
			XUI_COLOR_RGBA(31, 170, 120, 250),
			XUI_COLOR_RGBA(19, 125, 88, 255),
			XUI_COLOR_RGBA(39, 150, 106, 180),
			XUI_COLOR_RGBA(158, 174, 168, 135));
	}
	xuiWidgetSetRect(pTable, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pTable);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pTable);
		return iRet;
	}
	pDemo->pTable[iIndex] = pTable;
	return XUI_OK;
}

static int __xuiTableViewCreateUi(xui_tableview_demo_t* pDemo)
{
	int i;
	int j;
	int iRet;

	memset(pDemo->arrColumns, 0, sizeof(pDemo->arrColumns));
	pDemo->arrColumns[0].sTitle = "Item";
	pDemo->arrColumns[0].fWidth = 86.0f;
	pDemo->arrColumns[1].sTitle = "Status";
	pDemo->arrColumns[1].fWidth = 100.0f;
	pDemo->arrColumns[2].sTitle = "Flag";
	pDemo->arrColumns[2].fWidth = 68.0f;
	pDemo->arrColumns[2].iType = XUI_TABLE_CELL_TYPE_BOOL;
	pDemo->arrColumns[3].sTitle = "Color";
	pDemo->arrColumns[3].fWidth = 108.0f;
	pDemo->arrColumns[3].iType = XUI_TABLE_CELL_TYPE_COLOR;
	pDemo->arrColumns[4].sTitle = "Action";
	pDemo->arrColumns[4].fWidth = 104.0f;
	pDemo->arrColumns[4].iType = XUI_TABLE_CELL_TYPE_PICKER;
	pDemo->arrColumns[5].sTitle = "Progress";
	pDemo->arrColumns[5].fWidth = 128.0f;
	for ( i = 0; i < DEMO_COLS; i++ ) {
		pDemo->arrColumns[i].bVisibleSet = 1;
		pDemo->arrColumns[i].bVisible = 1;
		pDemo->arrColumns[i].bResizableSet = 1;
		pDemo->arrColumns[i].bResizable = 1;
		pDemo->arrColumns[i].fMinWidth = 44.0f;
	}
	for ( i = 0; i < DEMO_ROWS; i++ ) {
		memset(&pDemo->arrRows[i], 0, sizeof(pDemo->arrRows[i]));
		if ( (i % 7) == 0 ) pDemo->arrRows[i].fHeight = 26.0f;
		for ( j = 0; j < DEMO_COLS; j++ ) {
			if ( j == 2 ) {
				snprintf(pDemo->arrText[i][j], sizeof(pDemo->arrText[i][j]), "%s", (i % 2) ? "false" : "true");
			} else if ( j == 3 ) {
				snprintf(pDemo->arrText[i][j], sizeof(pDemo->arrText[i][j]), "#%02X%02X%02X", 54 + (i * 3) % 160, 130 + (j * 8), 190 - (j * 12));
			} else {
				snprintf(pDemo->arrText[i][j], sizeof(pDemo->arrText[i][j]), "R%02d C%d", i, j);
			}
		}
	}
	snprintf(pDemo->arrText[2][1], sizeof(pDemo->arrText[2][1]), "merged 2x2");
	pDemo->arrRows[3].bDisabled = 1;
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTableViewRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiTableViewAddLabel(pDemo, 0, "XUI TableView", (xui_rect_t){48.0f, 30.0f, 180.0f, 24.0f}) != XUI_OK ||
	     __xuiTableViewAddLabel(pDemo, 1, "Row selection", (xui_rect_t){48.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTableViewAddLabel(pDemo, 2, "Large dataset", (xui_rect_t){348.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTableViewAddLabel(pDemo, 3, "Merged + custom", (xui_rect_t){648.0f, 62.0f, 180.0f, 22.0f}) != XUI_OK ||
	     __xuiTableViewAddLabel(pDemo, 4, "Selected row=none column=none changes=0", (xui_rect_t){48.0f, 370.0f, 500.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[4];
	if ( __xuiTableViewAddTable(pDemo, 0, (xui_rect_t){48.0f, 88.0f, 260.0f, 250.0f}) != XUI_OK ||
	     __xuiTableViewAddTable(pDemo, 1, (xui_rect_t){348.0f, 88.0f, 260.0f, 250.0f}) != XUI_OK ||
	     __xuiTableViewAddTable(pDemo, 2, (xui_rect_t){648.0f, 88.0f, 224.0f, 250.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static uint32_t __xuiTableViewReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTableViewHandleInput(xui_tableview_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	uint32_t iMods;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTableViewReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iMods = 0;
	if ( xgeKeyPressed(XGE_KEY_UP) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, iMods);
	if ( xgeKeyPressed(XGE_KEY_DOWN) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, iMods);
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_LEFT, iMods);
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, iMods);
	if ( xgeKeyPressed(XGE_KEY_HOME) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, iMods);
	if ( xgeKeyPressed(XGE_KEY_END) ) (void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_END, iMods);
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTableViewClickCell(xui_tableview_demo_t* pDemo, xui_widget pTable, int iRow, int iColumn)
{
	xui_rect_t tWorld;
	xui_rect_t tCell;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pTable);
	if ( xuiTableViewGetCellRect(pTable, iRow, iColumn, &tCell) != XUI_OK ) return XUI_ERROR;
	fX = tWorld.fX + tCell.fX + 12.0f;
	fY = tWorld.fY + tCell.fY + tCell.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static int __xuiTableViewClickHeader(xui_tableview_demo_t* pDemo, xui_widget pTable)
{
	xui_rect_t tWorld;
	xui_rect_t tViewport;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pTable);
	tViewport = xuiTableViewGetViewportRect(pTable);
	fX = tWorld.fX + tViewport.fX + 14.0f;
	fY = tWorld.fY + tViewport.fY - 12.0f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiTableViewRunChecks(xui_tableview_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tMerged;
	float fOffsetX;
	float fOffsetY;
	int iDescending;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pTable[0] != NULL) && (xuiTableViewGetFrameWidget(pDemo->pTable[0]) != NULL);
	pDemo->bLayoutOK = xuiScrollFrameIsHScrollBarVisible(xuiTableViewGetFrameWidget(pDemo->pTable[0])) &&
		xuiScrollFrameIsVScrollBarVisible(xuiTableViewGetFrameWidget(pDemo->pTable[1]));
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)__xuiTableViewClickCell(pDemo, pDemo->pTable[0], 4, 0);
		pDemo->bSelectOK = (xuiTableViewGetSelectedRow(pDemo->pTable[0]) == 4) && (pDemo->iLastRow == 4);
		(void)__xuiTableViewClickHeader(pDemo, pDemo->pTable[0]);
		pDemo->bSortOK = (xuiTableViewGetSortColumn(pDemo->pTable[0], &iDescending) == 0);
		(void)xuiTableViewEnsureCellVisible(pDemo->pTable[1], DEMO_ROWS - 1, DEMO_COLS - 1);
		fOffsetX = 0.0f;
		fOffsetY = 0.0f;
		(void)xuiTableViewGetOffset(pDemo->pTable[1], &fOffsetX, &fOffsetY);
		pDemo->bScrollOK = (fOffsetX > 0.0f) && (fOffsetY > 0.0f);
		pDemo->bCustomOK = (pDemo->iCustomPaintCount > 0) &&
			(xuiTableViewGetCellContentRect(pDemo->pTable[2], 2, 2, &tMerged) == XUI_OK) &&
			(tMerged.fW > xuiTableViewGetColumnWidth(pDemo->pTable[2], 1));
		pDemo->bExerciseDone = 1;
	} else if ( bExerciseInput && pDemo->bExerciseDone && !pDemo->bCustomOK ) {
		pDemo->bCustomOK = (pDemo->iCustomPaintCount > 0) &&
			(xuiTableViewGetCellContentRect(pDemo->pTable[2], 2, 2, &tMerged) == XUI_OK) &&
			(tMerged.fW > xuiTableViewGetColumnWidth(pDemo->pTable[2], 1));
	} else if ( !bExerciseInput ) {
		pDemo->bSelectOK = 1;
		pDemo->bSortOK = 1;
		pDemo->bScrollOK = 1;
		pDemo->bCustomOK = 1;
	}
}

static int __xuiTableViewCreateAssets(xui_tableview_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiTableViewFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTableViewCreateUi(pDemo);
}

static void __xuiTableViewDestroyAssets(xui_tableview_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiTableViewFrame(void* pUser)
{
	xui_tableview_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_tableview_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTableViewHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTableViewRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_tableview final-summary frames=%d create=%d layout=%d select=%d sort=%d scroll=%d custom=%d row=%d column=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bSelectOK, pDemo->bSortOK,
			pDemo->bScrollOK, pDemo->bCustomOK, pDemo->iLastRow, pDemo->iLastColumn,
			tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_tableview_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastRow = -1;
	tDemo.iLastColumn = -1;
	iRet = __xuiTableViewParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTableViewUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TableView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_tableview: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTableViewCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_tableview: create assets failed: %d\n", iRet);
		__xuiTableViewDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTableViewFrame, &tDemo);
	__xuiTableViewDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bSelectOK && tDemo.bSortOK && tDemo.bScrollOK && tDemo.bCustomOK) ? 0 : 1;
}
