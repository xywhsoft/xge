#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_COUNT 4
#define LABEL_COUNT 4
#define LARGE_ROWS 240

enum {
	TABLE_BASIC = 0,
	TABLE_LARGE = 1,
	TABLE_MERGE = 2,
	TABLE_CUSTOM = 3
};

typedef struct table_adapter_t {
	struct app_state_t* pApp;
	int iTable;
	char sText[96];
} table_adapter_t;

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel[TABLE_COUNT];
	xge_xui_widget pTableWidget[TABLE_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_table_view_t tTable[TABLE_COUNT];
	xge_xui_table_view_row_t arrBasicRows[8];
	xge_xui_table_view_row_t arrMergeRows[8];
	table_adapter_t arrAdapter[TABLE_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount[TABLE_COUNT];
	int iResizeCount[TABLE_COUNT];
	int iSortCount[TABLE_COUNT];
	int iHoverCount[TABLE_COUNT];
	int iCustomPaintCount;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bCustomOK;
	int bGeometryOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_tableview font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, int iIndex, const char* sText)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(28.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(42, 58, 78, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pParent, pWidget);
	return XGE_OK;
}

static void StylePanel(xge_xui_widget pPanel)
{
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetSize(pPanel, xgeXuiSizePercent(100.0f), xgeXuiSizePercent(100.0f));
	xgeXuiWidgetSetPaddingPx(pPanel, 12.0f, 10.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetGap(pPanel, 8.0f);
	xgeXuiWidgetSetBackground(pPanel, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetSetBorder(pPanel, 1.0f, XGE_COLOR_RGBA(170, 184, 202, 255));
	xgeXuiWidgetSetRadius(pPanel, 4.0f);
}

static int AddPanel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sTitle)
{
	xge_xui_widget pPanel;

	pPanel = NewWidget(0.0f);
	if ( pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	StylePanel(pPanel);
	xgeXuiWidgetAdd(pRoot, pPanel);
	pApp->pPanel[iIndex] = pPanel;
	return AddLabel(pApp, pPanel, iIndex, sTitle);
}

static int TableCount(xge_xui_widget pWidget, void* pUser)
{
	table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (table_adapter_t*)pUser;
	if ( pAdapter == NULL ) {
		return 0;
	}
	if ( pAdapter->iTable == TABLE_LARGE ) {
		return LARGE_ROWS;
	}
	return 8;
}

static int TableCell(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser)
{
	static const char* arrBasic[8][4] = {
		{ "Alpha", "Ready", "24", "North" },
		{ "Beta", "Running", "31", "East" },
		{ "Gamma", "Queued", "12", "South" },
		{ "Delta", "Disabled", "6", "West" },
		{ "Epsilon", "Ready", "18", "North" },
		{ "Zeta", "Paused", "42", "East" },
		{ "Eta", "Done", "55", "South" },
		{ "Theta", "Ready", "63", "West" }
	};
	static const char* arrMerge[8][5] = {
		{ "Merged title", "", "", "Fixed", "Note" },
		{ "Row 1", "Wide merged area", "", "A", "Single" },
		{ "Row 2", "", "", "B", "Tall" },
		{ "Row 3", "Plain", "Cell", "C", "Tall" },
		{ "Row 4", "Plain", "Cell", "D", "Cell" },
		{ "Row 5", "Disabled", "Cell", "E", "Cell" },
		{ "Row 6", "Plain", "Cell", "F", "Cell" },
		{ "Row 7", "Plain", "Cell", "G", "Cell" }
	};
	static const char* arrCustom[8][4] = {
		{ "Atlas", "Design", "Published", "88%" },
		{ "Beacon", "Build", "Draft", "41%" },
		{ "Comet", "QA", "Blocked", "12%" },
		{ "Delta", "Ops", "Published", "100%" },
		{ "Echo", "Design", "Draft", "64%" },
		{ "Falcon", "Build", "Published", "77%" },
		{ "Grove", "QA", "Blocked", "22%" },
		{ "Harbor", "Ops", "Draft", "35%" }
	};
	table_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (table_adapter_t*)pUser;
	if ( (pAdapter == NULL) || (pCell == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pAdapter->iTable == TABLE_BASIC ) {
		if ( iColumn == 2 ) {
			pCell->pValue = (const void*)(intptr_t)atoi(arrBasic[iRow][iColumn]);
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_INT;
		} else {
			pCell->sText = arrBasic[iRow][iColumn];
		}
		if ( iRow == 3 ) {
			pCell->bDisabled = 1;
		}
		if ( (iRow == 1) && (iColumn == 1) ) {
			pCell->sTooltip = "Running state is supplied by the row adapter";
		}
		return XGE_OK;
	}
	if ( pAdapter->iTable == TABLE_LARGE ) {
		snprintf(pAdapter->sText, sizeof(pAdapter->sText), "R%03d C%02d", iRow + 1, iColumn + 1);
		pCell->sText = pAdapter->sText;
		return XGE_OK;
	}
	if ( pAdapter->iTable == TABLE_MERGE ) {
		pCell->sText = arrMerge[iRow][iColumn];
		if ( (iRow == 0) && (iColumn == 0) ) {
			pCell->iColSpan = 3;
		}
		if ( (iRow == 1) && (iColumn == 1) ) {
			pCell->iRowSpan = 2;
			pCell->iColSpan = 2;
		}
		if ( (iRow == 2) && (iColumn == 4) ) {
			pCell->iRowSpan = 2;
		}
		if ( (iRow == 5) || ((iRow == 5) && (iColumn == 1)) ) {
			pCell->bDisabled = 1;
		}
		return XGE_OK;
	}
	pCell->sText = arrCustom[iRow][iColumn];
	if ( (iColumn == 2) && (iRow == 2) ) {
		pCell->bInvalid = 1;
		pCell->sTooltip = "Blocked status marks this cell invalid";
	}
	if ( (iColumn == 3) && (iRow == 1) ) {
		pCell->bDirty = 1;
	}
	if ( (iColumn == 3) && (iRow == 3) ) {
		pCell->bEditing = 1;
	}
	return XGE_OK;
}

static int ScoreFormatter(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, char* sBuffer, int iSize, void* pUser)
{
	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)pUser;
	if ( (pCell == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return 0;
	}
	snprintf(sBuffer, (size_t)iSize, "%d", (int)(intptr_t)pCell->pValue);
	sBuffer[iSize - 1] = 0;
	return 1;
}

static void OnTableSelect(xge_xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	app_state_t* pApp;
	int i;

	(void)iRow;
	(void)iColumn;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( pApp->pTableWidget[i] == pWidget ) {
			pApp->iSelectCount[i]++;
			break;
		}
	}
}

static void OnColumnResize(xge_xui_widget pWidget, int iColumn, float fWidth, void* pUser)
{
	app_state_t* pApp;
	int i;

	(void)iColumn;
	(void)fWidth;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( pApp->pTableWidget[i] == pWidget ) {
			pApp->iResizeCount[i]++;
			break;
		}
	}
}

static void OnSort(xge_xui_widget pWidget, int iColumn, int bDescending, void* pUser)
{
	app_state_t* pApp;
	int i;

	(void)iColumn;
	(void)bDescending;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( pApp->pTableWidget[i] == pWidget ) {
			pApp->iSortCount[i]++;
			break;
		}
	}
}

static void OnHover(xge_xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	app_state_t* pApp;
	int i;

	(void)iRow;
	(void)iColumn;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( pApp->pTableWidget[i] == pWidget ) {
			pApp->iHoverCount[i]++;
			break;
		}
	}
}

static int CustomHeaderRenderer(xge_xui_widget pWidget, int iColumn, const xge_xui_table_view_column_t* pColumn, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tText;

	(void)pWidget;
	(void)iColumn;
	(void)iState;
	pApp = (app_state_t*)pUser;
	xgeShapeRectFill(tRect, XGE_COLOR_RGBA(224, 238, 246, 255));
	xgeShapeRectStroke(tRect, 1.0f, XGE_COLOR_RGBA(156, 184, 206, 255));
	tText = (xge_rect_t){ tRect.fX + 8.0f, tRect.fY, tRect.fW - 16.0f, tRect.fH };
	if ( (pApp != NULL) && pApp->bFontReady && (pColumn != NULL) ) {
		xgeTextDrawRect(&pApp->tFont, pColumn->sTitle, tText, XGE_COLOR_RGBA(38, 58, 76, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	return 1;
}

static int CustomCellRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tInner;
	uint32_t iColor;

	(void)pWidget;
	(void)iRow;
	(void)iState;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pCell == NULL) ) {
		return 0;
	}
	pApp->iCustomPaintCount++;
	if ( iColumn != 2 ) {
		return 0;
	}
	iColor = XGE_COLOR_RGBA(70, 146, 214, 255);
	if ( strcmp(pCell->sText, "Published") == 0 ) {
		iColor = XGE_COLOR_RGBA(70, 170, 118, 255);
	} else if ( strcmp(pCell->sText, "Blocked") == 0 ) {
		iColor = XGE_COLOR_RGBA(214, 92, 86, 255);
	}
	tInner = (xge_rect_t){ tRect.fX + 10.0f, tRect.fY + 4.0f, tRect.fW - 20.0f, tRect.fH - 8.0f };
	xgeShapeRectFill(tInner, iColor);
	if ( pApp->bFontReady ) {
		xgeTextDrawRect(&pApp->tFont, pCell->sText, tInner, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	return 1;
}

static void SetupColumns(xge_xui_table_view_column_t* pColumn, int iCount)
{
	int i;

	memset(pColumn, 0, sizeof(xge_xui_table_view_column_t) * (size_t)iCount);
	for ( i = 0; i < iCount; i++ ) {
		pColumn[i].iId = i;
		pColumn[i].bVisible = 1;
		pColumn[i].bVisibleSet = 1;
		pColumn[i].bResizable = 1;
		pColumn[i].bResizableSet = 1;
		pColumn[i].iAlign = XGE_TEXT_ALIGN_LEFT;
		pColumn[i].iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	}
}

static int AddTable(app_state_t* pApp, int iIndex)
{
	xge_xui_widget pWidget;
	xge_xui_table_view_column_t arrColumns[8];
	int i;

	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiTableViewInit(&pApp->tTable[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	pApp->pTableWidget[iIndex] = pWidget;
	pApp->arrAdapter[iIndex].pApp = pApp;
	pApp->arrAdapter[iIndex].iTable = iIndex;
	xgeXuiTableViewSetFont(&pApp->tTable[iIndex], pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiTableViewSetSelect(&pApp->tTable[iIndex], OnTableSelect, pApp);
	xgeXuiTableViewSetSort(&pApp->tTable[iIndex], OnSort, pApp);
	xgeXuiTableViewSetColumnResize(&pApp->tTable[iIndex], OnColumnResize, pApp);
	xgeXuiTableViewSetHover(&pApp->tTable[iIndex], OnHover, pApp);
	if ( iIndex == TABLE_BASIC ) {
		SetupColumns(arrColumns, 4);
		arrColumns[0].sTitle = "Name"; arrColumns[0].fWidth = 116.0f;
		arrColumns[1].sTitle = "State"; arrColumns[1].fWidth = 96.0f;
		arrColumns[2].sTitle = "Score"; arrColumns[2].fWidth = 76.0f; arrColumns[2].iAlign = XGE_TEXT_ALIGN_RIGHT;
		arrColumns[3].sTitle = "Zone"; arrColumns[3].fWidth = 96.0f;
		for ( i = 0; i < 8; i++ ) {
			pApp->arrBasicRows[i].fHeight = (i == 1) ? 28.0f : 24.0f;
		}
		pApp->arrBasicRows[3].bDisabled = 1;
		xgeXuiTableViewSetColumns(&pApp->tTable[iIndex], arrColumns, 4);
		xgeXuiTableViewSetColumnFormatter(&pApp->tTable[iIndex], 2, ScoreFormatter, pApp);
		xgeXuiTableViewSetRows(&pApp->tTable[iIndex], pApp->arrBasicRows, 8);
		xgeXuiTableViewSetSelectionMode(&pApp->tTable[iIndex], XGE_XUI_TABLE_VIEW_SELECTION_ROW);
		xgeXuiTableViewSetSelectedRow(&pApp->tTable[iIndex], 1);
	} else if ( iIndex == TABLE_LARGE ) {
		SetupColumns(arrColumns, 8);
		for ( i = 0; i < 8; i++ ) {
			snprintf(pApp->arrAdapter[iIndex].sText, sizeof(pApp->arrAdapter[iIndex].sText), "Column %d", i + 1);
			arrColumns[i].sTitle = (i == 0) ? "Record" : ((i == 7) ? "Far column" : "Metric");
			arrColumns[i].fWidth = (i == 0) ? 120.0f : 86.0f;
		}
		xgeXuiTableViewSetColumns(&pApp->tTable[iIndex], arrColumns, 8);
		xgeXuiTableViewSetDefaultMetrics(&pApp->tTable[iIndex], 86.0f, 22.0f, 24.0f);
		xgeXuiTableViewSetSelectionMode(&pApp->tTable[iIndex], XGE_XUI_TABLE_VIEW_SELECTION_CELL);
		xgeXuiTableViewSetScrollbarMode(&pApp->tTable[iIndex], XGE_XUI_SCROLLBAR_MODE_FULL);
	} else if ( iIndex == TABLE_MERGE ) {
		SetupColumns(arrColumns, 5);
		arrColumns[0].sTitle = "Key"; arrColumns[0].fWidth = 84.0f;
		arrColumns[1].sTitle = "Span A"; arrColumns[1].fWidth = 104.0f;
		arrColumns[2].sTitle = "Span B"; arrColumns[2].fWidth = 104.0f;
		arrColumns[3].sTitle = "Fixed"; arrColumns[3].fWidth = 76.0f; arrColumns[3].bResizable = 0; arrColumns[3].bResizableSet = 1;
		arrColumns[4].sTitle = "Note"; arrColumns[4].fWidth = 92.0f;
		for ( i = 0; i < 8; i++ ) {
			pApp->arrMergeRows[i].fHeight = (i == 1 || i == 2) ? 30.0f : 24.0f;
		}
		pApp->arrMergeRows[5].bDisabled = 1;
		xgeXuiTableViewSetColumns(&pApp->tTable[iIndex], arrColumns, 5);
		xgeXuiTableViewSetRows(&pApp->tTable[iIndex], pApp->arrMergeRows, 8);
		xgeXuiTableViewSetSelectionMode(&pApp->tTable[iIndex], XGE_XUI_TABLE_VIEW_SELECTION_CELL);
		xgeXuiTableViewSetSelectedCell(&pApp->tTable[iIndex], 1, 1);
	} else {
		SetupColumns(arrColumns, 4);
		arrColumns[0].sTitle = "Project"; arrColumns[0].fWidth = 110.0f;
		arrColumns[1].sTitle = "Area"; arrColumns[1].fWidth = 90.0f;
		arrColumns[2].sTitle = "Status"; arrColumns[2].fWidth = 118.0f;
		arrColumns[3].sTitle = "Progress"; arrColumns[3].fWidth = 92.0f; arrColumns[3].iAlign = XGE_TEXT_ALIGN_RIGHT;
		xgeXuiTableViewSetColumns(&pApp->tTable[iIndex], arrColumns, 4);
		xgeXuiTableViewSetDefaultMetrics(&pApp->tTable[iIndex], 100.0f, 28.0f, 26.0f);
		xgeXuiTableViewSetHeaderRenderer(&pApp->tTable[iIndex], CustomHeaderRenderer, pApp);
		xgeXuiTableViewSetCellRenderer(&pApp->tTable[iIndex], CustomCellRenderer, pApp);
	}
	xgeXuiTableViewSetAdapter(&pApp->tTable[iIndex], TableCount, TableCell, &pApp->arrAdapter[iIndex]);
	if ( iIndex == TABLE_LARGE ) {
		xgeXuiTableViewSetSelectedCell(&pApp->tTable[iIndex], 120, 5);
	} else if ( iIndex == TABLE_CUSTOM ) {
		xgeXuiTableViewSetSelectedCell(&pApp->tTable[iIndex], 2, 2);
	}
	xgeXuiWidgetAdd(pApp->pPanel[iIndex], pWidget);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 2, 300.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	if ( AddPanel(pApp, pRoot, TABLE_BASIC, "Row selection: headers, disabled row, keyboard") != XGE_OK ||
	     AddPanel(pApp, pRoot, TABLE_LARGE, "Large table: 240 rows, horizontal and vertical scroll") != XGE_OK ||
	     AddPanel(pApp, pRoot, TABLE_MERGE, "Merged cells: row height, fixed column, disabled row") != XGE_OK ||
	     AddPanel(pApp, pRoot, TABLE_CUSTOM, "Custom renderers: header and status badges") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddTable(pApp, TABLE_BASIC) != XGE_OK ||
	     AddTable(pApp, TABLE_LARGE) != XGE_OK ||
	     AddTable(pApp, TABLE_MERGE) != XGE_OK ||
	     AddTable(pApp, TABLE_CUSTOM) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 980.0f ) {
		fRootW = 980.0f;
	}
	if ( fRootH < 700.0f ) {
		fRootH = 700.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fPanelH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fPanelH, 12.0f, 12.0f);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tRect;
	xge_rect_t tContentRect;
	float fScrollX;
	float fScrollY;
	int iRow;
	int iColumn;
	int iActiveRow;
	int iActiveColumn;
	int i;

	pApp->bCreateOK = 1;
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		if ( (pApp->pTableWidget[i] == NULL) || (pApp->tTable[i].pWidget != pApp->pTableWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK = (pApp->pPanel[0] != NULL) && (pApp->pPanel[0]->tRect.fW > 300.0f) && (pApp->pPanel[3] != NULL) && (pApp->pPanel[3]->tRect.fH > 220.0f);
	xgeXuiTableViewEnsureCellVisible(&pApp->tTable[TABLE_LARGE], 120, 5);
	xgeXuiTableViewGetOffset(&pApp->tTable[TABLE_LARGE], &fScrollX, &fScrollY);
	xgeXuiTableViewGetSelectedCell(&pApp->tTable[TABLE_MERGE], &iRow, &iColumn);
	xgeXuiTableViewGetActiveCell(&pApp->tTable[TABLE_LARGE], &iActiveRow, &iActiveColumn);
	pApp->bGeometryOK =
		xgeXuiTableViewGetCellRect(&pApp->tTable[TABLE_BASIC], 1, 1, &tRect) &&
		xgeXuiTableViewGetCellContentRect(&pApp->tTable[TABLE_MERGE], 2, 2, &tContentRect) &&
		(tRect.fW > 20.0f) &&
		(tRect.fH > 10.0f) &&
		(tContentRect.fW > 150.0f) &&
		(xgeXuiTableViewGetViewportWidget(&pApp->tTable[TABLE_BASIC]) != NULL) &&
		(xgeXuiTableViewGetBodyWidget(&pApp->tTable[TABLE_BASIC]) != NULL);
	if ( pApp->bGeometryOK ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_MOVE;
		tEvent.fX = tRect.fX + tRect.fW * 0.5f;
		tEvent.fY = tRect.fY + tRect.fH * 0.5f;
		xgeXuiTableViewEvent(&pApp->tTable[TABLE_BASIC], &tEvent);
	}
	pApp->bStateOK =
		(xgeXuiTableViewGetSelectionMode(&pApp->tTable[TABLE_BASIC]) == XGE_XUI_TABLE_VIEW_SELECTION_ROW) &&
		(xgeXuiTableViewGetSelectedRow(&pApp->tTable[TABLE_BASIC]) == 1) &&
		(xgeXuiTableViewGetRowCount(&pApp->tTable[TABLE_LARGE]) == LARGE_ROWS) &&
		(fScrollY > 1.0f) &&
		(iRow == 1) &&
		(iColumn == 1) &&
		(iActiveRow == 120) &&
		(iActiveColumn == 5) &&
		(pApp->iHoverCount[TABLE_BASIC] > 0) &&
		(xgeXuiTableViewGetScrollbarMode(&pApp->tTable[TABLE_LARGE]) == XGE_XUI_SCROLLBAR_MODE_FULL);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < TABLE_COUNT; i++ ) {
		xgeXuiTableViewUnit(&pApp->tTable[i]);
	}
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->bCustomOK = pApp->iCustomPaintCount > 0;
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_tableview final-summary frames=%d create=%d layout=%d state=%d custom=%d largeFirst=%d customPaint=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bCustomOK && pApp->bGeometryOK,
			xgeXuiTableViewGetFirstVisible(&pApp->tTable[TABLE_LARGE]),
			pApp->iCustomPaintCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABLEVIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI TableView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bCustomOK && tApp.bGeometryOK) ? 0 : 3;
}
