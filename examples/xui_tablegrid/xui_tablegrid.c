#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRID_COUNT 4
#define LABEL_COUNT 4
#define DATA_ROWS 8
#define DATA_COLS 10
#define LARGE_ROWS 180

enum {
	GRID_DISPLAY = 0,
	GRID_IMMEDIATE = 1,
	GRID_STATES = 2,
	GRID_LARGE = 3
};

typedef struct grid_adapter_t {
	struct app_state_t* pApp;
	int iGrid;
	char sText[96];
} grid_adapter_t;

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel[GRID_COUNT];
	xge_xui_widget pGridWidget[GRID_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_table_grid_t tGrid[GRID_COUNT];
	xge_xui_table_view_row_t arrRows[GRID_COUNT][DATA_ROWS];
	grid_adapter_t arrAdapter[GRID_COUNT];
	char arrData[GRID_COUNT][DATA_ROWS][DATA_COLS][64];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSetCount;
	int iChangeCount;
	int iValidateCount;
	int iPickerCount;
	int iCustomPaintCount;
	int bCreateOK;
	int bLayoutOK;
	int bEditOK;
	int bStateOK;
	int bCustomOK;
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
			printf("xui_tablegrid font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int DemoIsLeapYear(int64 iYear)
{
	return (iYear % 4 == 0 && iYear % 100 != 0) || (iYear % 400 == 0);
}

static int DemoDaysInMonth(int64 iYear, int iMonth)
{
	static const int arrDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if ( iMonth == 2 && DemoIsLeapYear(iYear) ) {
		return 29;
	}
	if ( iMonth < 1 || iMonth > 12 ) {
		return 30;
	}
	return arrDays[iMonth - 1];
}

static xtime DemoTimeSerial(int iHour, int iMinute, int iSecond)
{
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + iSecond;
}

static xtime DemoDateSerial(int64 iYear, int iMonth, int iDay)
{
	xtime tDate;
	int64 i;
	int iM;

	tDate = (xtime)(iDay - 1) * XRT_TIME_DAY;
	for ( iM = 1; iM < iMonth; iM++ ) {
		tDate += (xtime)DemoDaysInMonth(iYear, iM) * XRT_TIME_DAY;
	}
	for ( i = 0; i < iYear; i++ ) {
		tDate += (xtime)(DemoIsLeapYear(i) ? 366 : 365) * XRT_TIME_DAY;
	}
	return tDate;
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

static const char* g_arrStatusItems[] = { "Draft", "Ready", "Blocked", "Done" };
static const uint32_t g_arrColorPalette[] = {
	XGE_COLOR_RGBA(46, 124, 214, 255),
	XGE_COLOR_RGBA(67, 167, 124, 255),
	XGE_COLOR_RGBA(213, 80, 74, 255),
	XGE_COLOR_RGBA(230, 156, 52, 255),
	XGE_COLOR_RGBA(132, 86, 209, 255)
};

static void SeedData(app_state_t* pApp)
{
	static const char* arrNames[DATA_ROWS] = { "Atlas", "Beacon", "Comet", "Delta", "Echo", "Falcon", "Grove", "Harbor" };
	static const char* arrActions[DATA_ROWS] = { "Browse", "Pick", "Details", "Browse", "Pick", "Details", "Browse", "Pick" };
	static const char* arrColors[DATA_ROWS] = { "#2E7CD6", "#43A77C", "#D5504A", "#E69C34", "#8456D1", "#2E7CD6", "#43A77C", "#D5504A" };
	int g;
	int r;

	for ( g = 0; g < GRID_COUNT; g++ ) {
		for ( r = 0; r < DATA_ROWS; r++ ) {
			snprintf(pApp->arrData[g][r][0], sizeof(pApp->arrData[g][r][0]), "%s", arrNames[r]);
			snprintf(pApp->arrData[g][r][1], sizeof(pApp->arrData[g][r][1]), "%d", 10 + r * 3 + g);
			snprintf(pApp->arrData[g][r][2], sizeof(pApp->arrData[g][r][2]), "%.2f", 1.25f + (float)r * 0.35f);
			snprintf(pApp->arrData[g][r][3], sizeof(pApp->arrData[g][r][3]), "%s", (r % 2) == 0 ? "true" : "false");
			snprintf(pApp->arrData[g][r][4], sizeof(pApp->arrData[g][r][4]), "%s", g_arrStatusItems[(r + g) % 4]);
			snprintf(pApp->arrData[g][r][5], sizeof(pApp->arrData[g][r][5]), "%s", arrColors[(r + g) % DATA_ROWS]);
			snprintf(pApp->arrData[g][r][6], sizeof(pApp->arrData[g][r][6]), "2026-05-%02d", 10 + r);
			snprintf(pApp->arrData[g][r][7], sizeof(pApp->arrData[g][r][7]), "%02d:%02d", 9 + (r % 5), (r % 2) ? 30 : 0);
			snprintf(pApp->arrData[g][r][8], sizeof(pApp->arrData[g][r][8]), "Line %d summary", r + 1);
			snprintf(pApp->arrData[g][r][9], sizeof(pApp->arrData[g][r][9]), "%s", arrActions[r]);
		}
	}
	snprintf(pApp->arrData[GRID_STATES][2][1], sizeof(pApp->arrData[GRID_STATES][2][1]), "bad");
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

static int GridCount(xge_xui_widget pWidget, void* pUser)
{
	grid_adapter_t* pAdapter;

	(void)pWidget;
	pAdapter = (grid_adapter_t*)pUser;
	if ( pAdapter == NULL ) {
		return 0;
	}
	return (pAdapter->iGrid == GRID_LARGE) ? LARGE_ROWS : DATA_ROWS;
}

static int GridCell(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser)
{
	grid_adapter_t* pAdapter;
	app_state_t* pApp;

	(void)pWidget;
	pAdapter = (grid_adapter_t*)pUser;
	if ( (pAdapter == NULL) || (pCell == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pApp = pAdapter->pApp;
	if ( pAdapter->iGrid == GRID_LARGE ) {
		snprintf(pAdapter->sText, sizeof(pAdapter->sText), "R%03d C%02d", iRow + 1, iColumn + 1);
		pCell->sText = pAdapter->sText;
		pCell->iType = (iColumn == 3) ? XGE_XUI_TABLE_CELL_TYPE_BOOL : XGE_XUI_TABLE_CELL_TYPE_TEXT;
		return XGE_OK;
	}
	pCell->sText = pApp->arrData[pAdapter->iGrid][iRow][iColumn];
	switch ( iColumn ) {
		case 1:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_INT;
			break;
		case 2:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_FLOAT;
			break;
		case 3:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_BOOL;
			break;
		case 4:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_ENUM;
			break;
		case 5:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_COLOR;
			break;
		case 6:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_DATE;
			break;
		case 7:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_TIME;
			break;
		case 8:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_TEXTAREA;
			break;
		case 9:
			if ( iRow == 4 ) {
				pCell->iType = XGE_XUI_TABLE_CELL_TYPE_FILE;
			} else if ( iRow == 5 ) {
				pCell->iType = XGE_XUI_TABLE_CELL_TYPE_IMAGE;
			} else {
				pCell->iType = XGE_XUI_TABLE_CELL_TYPE_PICKER;
			}
			break;
		default:
			pCell->iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
			break;
	}
	if ( pAdapter->iGrid == GRID_STATES ) {
		if ( iRow == 1 ) {
			pCell->bDisabled = 1;
		}
		if ( iRow == 2 && iColumn == 1 ) {
			pCell->bInvalid = 1;
			pCell->sTooltip = "Validation should reject this value";
		}
		if ( iRow == 3 && iColumn == 2 ) {
			pCell->bDirty = 1;
		}
	}
	return XGE_OK;
}

static void GridSet(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	grid_adapter_t* pAdapter;
	app_state_t* pApp;

	(void)pWidget;
	(void)iType;
	pAdapter = (grid_adapter_t*)pUser;
	if ( (pAdapter == NULL) || (pAdapter->pApp == NULL) || (pAdapter->iGrid == GRID_LARGE) || (iRow < 0) || (iRow >= DATA_ROWS) || (iColumn < 0) || (iColumn >= DATA_COLS) ) {
		return;
	}
	pApp = pAdapter->pApp;
	snprintf(pApp->arrData[pAdapter->iGrid][iRow][iColumn], sizeof(pApp->arrData[pAdapter->iGrid][iRow][iColumn]), "%s", sValue != NULL ? sValue : "");
	pApp->iSetCount++;
}

static int GridValidate(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	app_state_t* pApp;
	char* pEnd;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iValidateCount++;
	}
	if ( (iType == XGE_XUI_TABLE_CELL_TYPE_INT) || (iType == XGE_XUI_TABLE_CELL_TYPE_FLOAT) ) {
		double fValue;

		if ( (sValue == NULL) || (sValue[0] == 0) ) {
			return 1;
		}
		fValue = strtod(sValue, &pEnd);
		return (pEnd != NULL) && (*pEnd == 0) && (fValue >= 0.0);
	}
	return 1;
}

static void GridChange(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)sValue;
	(void)iType;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
	}
}

static int GridEditor(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)pCell;
	(void)tRect;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iPickerCount++;
	}
	return 1;
}

static int GridEditorConfig(xge_xui_widget pWidget, int iRow, int iColumn, int iType, xge_xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	(void)pWidget;
	(void)iRow;
	(void)pUser;
	if ( pConfig == NULL ) {
		return 0;
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_ENUM && iColumn == 4 ) {
		pConfig->arrEnumItems = g_arrStatusItems;
		pConfig->iEnumItemCount = (int)(sizeof(g_arrStatusItems) / sizeof(g_arrStatusItems[0]));
		return 1;
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_COLOR && iColumn == 5 ) {
		pConfig->arrPalette = g_arrColorPalette;
		pConfig->iPaletteCount = (int)(sizeof(g_arrColorPalette) / sizeof(g_arrColorPalette[0]));
		pConfig->bAlphaEnabled = 0;
		return 1;
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_TIME && iColumn == 7 ) {
		pConfig->bShowSecond = 0;
		return 1;
	}
	return 0;
}

static int GridCellRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;
	xge_rect_t tBox;
	xge_rect_t tMark;
	xge_rect_t tText;
	xge_rect_t tButton;
	uint32_t iColor;
	int bOn;

	(void)pWidget;
	(void)iRow;
	(void)iState;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pCell == NULL) ) {
		return 0;
	}
	if ( iColumn == 3 ) {
		pApp->iCustomPaintCount++;
		bOn = (pCell->sText != NULL) && (strcmp(pCell->sText, "true") == 0);
		tBox = (xge_rect_t){ tRect.fX + 8.0f, tRect.fY + (tRect.fH - 12.0f) * 0.5f, 12.0f, 12.0f };
		xgeShapeRectFill(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
		xgeShapeRectStroke(tBox, 1.0f, XGE_COLOR_RGBA(126, 166, 200, 255));
		if ( bOn ) {
			tMark = (xge_rect_t){ tBox.fX + 3.0f, tBox.fY + 3.0f, 6.0f, 6.0f };
			xgeShapeRectFill(tMark, XGE_COLOR_RGBA(62, 145, 214, 255));
		}
		if ( pApp->bFontReady ) {
			tText = tRect;
			tText.fX += 26.0f;
			tText.fW -= 30.0f;
			xgeTextDrawRect(&pApp->tFont, bOn ? "enabled" : "off", tText, XGE_COLOR_RGBA(54, 76, 96, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		return 1;
	}
	if ( iColumn == 5 ) {
		pApp->iCustomPaintCount++;
		tBox = (xge_rect_t){ tRect.fX + 8.0f, tRect.fY + (tRect.fH - 12.0f) * 0.5f, 28.0f, 12.0f };
		iColor = XGE_COLOR_RGBA(130, 183, 55, 255);
		if ( pCell->sText != NULL && pCell->sText[0] == '#' ) {
			int r;
			int g;
			int b;

			if ( sscanf(pCell->sText + 1, "%02x%02x%02x", &r, &g, &b) == 3 ) {
				iColor = XGE_COLOR_RGBA(r, g, b, 255);
			}
		}
		xgeShapeRectFill(tBox, iColor);
		xgeShapeRectStroke(tBox, 1.0f, XGE_COLOR_RGBA(142, 184, 216, 255));
		if ( pApp->bFontReady ) {
			tText = tRect;
			tText.fX += 44.0f;
			tText.fW -= 48.0f;
			xgeTextDrawRect(&pApp->tFont, pCell->sText != NULL ? pCell->sText : "", tText, XGE_COLOR_RGBA(54, 76, 96, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		return 1;
	}
	if ( iColumn == 9 ) {
		pApp->iCustomPaintCount++;
		tText = tRect;
		tText.fX += 6.0f;
		tText.fW -= 32.0f;
		if ( pApp->bFontReady ) {
			xgeTextDrawRect(&pApp->tFont, pCell->sText != NULL ? pCell->sText : "", tText, XGE_COLOR_RGBA(54, 76, 96, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		iColor = ((iState & XGE_XUI_TABLE_CELL_HOVER) != 0) ? XGE_COLOR_RGBA(218, 236, 250, 255) : XGE_COLOR_RGBA(232, 243, 252, 255);
		tButton = (xge_rect_t){ tRect.fX + tRect.fW - 25.0f, tRect.fY + 3.0f, 20.0f, tRect.fH - 6.0f };
		xgeShapeRectFill(tButton, iColor);
		xgeShapeRectStroke(tButton, 1.0f, XGE_COLOR_RGBA(142, 184, 216, 255));
		if ( pApp->bFontReady ) {
			xgeTextDrawRect(&pApp->tFont, "...", tButton, XGE_COLOR_RGBA(54, 76, 96, 255), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		return 1;
	}
	return 0;
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

static int AddGrid(app_state_t* pApp, int iIndex)
{
	xge_xui_widget pWidget;
	xge_xui_table_view_column_t arrColumns[DATA_COLS];
	int i;

	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiTableGridInit(&pApp->tGrid[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	pApp->pGridWidget[iIndex] = pWidget;
	pApp->arrAdapter[iIndex].pApp = pApp;
	pApp->arrAdapter[iIndex].iGrid = iIndex;
	xgeXuiTableGridSetFont(&pApp->tGrid[iIndex], pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiTableGridSetValidate(&pApp->tGrid[iIndex], GridValidate, pApp);
	xgeXuiTableGridSetChange(&pApp->tGrid[iIndex], GridChange, pApp);
	xgeXuiTableGridSetEditor(&pApp->tGrid[iIndex], GridEditor, pApp);
	xgeXuiTableGridSetEditorConfig(&pApp->tGrid[iIndex], GridEditorConfig, pApp);
	if ( iIndex == GRID_LARGE ) {
		SetupColumns(arrColumns, 8);
		for ( i = 0; i < 8; i++ ) {
			arrColumns[i].sTitle = (i == 0) ? "Record" : "Value";
			arrColumns[i].fWidth = (i == 0) ? 120.0f : 86.0f;
		}
		xgeXuiTableGridSetColumns(&pApp->tGrid[iIndex], arrColumns, 8);
		xgeXuiTableGridSetDefaultMetrics(&pApp->tGrid[iIndex], 86.0f, 22.0f, 24.0f);
		xgeXuiTableGridSetScrollbarMode(&pApp->tGrid[iIndex], XGE_XUI_SCROLLBAR_MODE_FULL);
	} else {
		SetupColumns(arrColumns, DATA_COLS);
		arrColumns[0].sTitle = "Name"; arrColumns[0].fWidth = 118.0f;
		arrColumns[1].sTitle = "Qty"; arrColumns[1].fWidth = 76.0f; arrColumns[1].iAlign = XGE_TEXT_ALIGN_RIGHT; arrColumns[1].iType = XGE_XUI_TABLE_CELL_TYPE_INT;
		arrColumns[2].sTitle = "Price"; arrColumns[2].fWidth = 84.0f; arrColumns[2].iAlign = XGE_TEXT_ALIGN_RIGHT; arrColumns[2].iType = XGE_XUI_TABLE_CELL_TYPE_FLOAT;
		arrColumns[3].sTitle = "Enabled"; arrColumns[3].fWidth = 106.0f; arrColumns[3].iType = XGE_XUI_TABLE_CELL_TYPE_BOOL;
		arrColumns[4].sTitle = "Status"; arrColumns[4].fWidth = 86.0f; arrColumns[4].iType = XGE_XUI_TABLE_CELL_TYPE_ENUM;
		arrColumns[5].sTitle = "Color"; arrColumns[5].fWidth = 92.0f; arrColumns[5].iType = XGE_XUI_TABLE_CELL_TYPE_COLOR;
		arrColumns[6].sTitle = "Due"; arrColumns[6].fWidth = 100.0f; arrColumns[6].iType = XGE_XUI_TABLE_CELL_TYPE_DATE;
		arrColumns[7].sTitle = "Time"; arrColumns[7].fWidth = 76.0f; arrColumns[7].iType = XGE_XUI_TABLE_CELL_TYPE_TIME;
		arrColumns[8].sTitle = "Notes"; arrColumns[8].fWidth = 116.0f; arrColumns[8].iType = XGE_XUI_TABLE_CELL_TYPE_TEXTAREA;
		arrColumns[9].sTitle = "Picker"; arrColumns[9].fWidth = 112.0f; arrColumns[9].iType = XGE_XUI_TABLE_CELL_TYPE_PICKER;
		for ( i = 0; i < DATA_ROWS; i++ ) {
			pApp->arrRows[iIndex][i].fHeight = (i == 0) ? 28.0f : 24.0f;
		}
		if ( iIndex == GRID_STATES ) {
			pApp->arrRows[iIndex][1].bDisabled = 1;
		}
		xgeXuiTableGridSetColumns(&pApp->tGrid[iIndex], arrColumns, DATA_COLS);
		xgeXuiTableGridSetRows(&pApp->tGrid[iIndex], pApp->arrRows[iIndex], DATA_ROWS);
		if ( iIndex == GRID_IMMEDIATE ) {
		xgeXuiTableGridSetEditMode(&pApp->tGrid[iIndex], XGE_XUI_TABLE_GRID_EDIT_QUICK);
		}
	}
	xgeXuiTableGridSetAdapter(&pApp->tGrid[iIndex], GridCount, GridCell, GridSet, &pApp->arrAdapter[iIndex]);
	xgeXuiTableViewSetCellRenderer(xgeXuiTableGridGetTableView(&pApp->tGrid[iIndex]), GridCellRenderer, pApp);
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
	if ( AddPanel(pApp, pRoot, GRID_DISPLAY, "Display/edit: double click or Enter edits Input-backed cells") != XGE_OK ||
	     AddPanel(pApp, pRoot, GRID_IMMEDIATE, "Quick edit: single click opens editor for supported cells") != XGE_OK ||
	     AddPanel(pApp, pRoot, GRID_STATES, "States: disabled row, invalid value, dirty marker, picker callback") != XGE_OK ||
	     AddPanel(pApp, pRoot, GRID_LARGE, "Large editable grid: TableView scrolling reused by TableGrid") != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( AddGrid(pApp, GRID_DISPLAY) != XGE_OK ||
	     AddGrid(pApp, GRID_IMMEDIATE) != XGE_OK ||
	     AddGrid(pApp, GRID_STATES) != XGE_OK ||
	     AddGrid(pApp, GRID_LARGE) != XGE_OK ) {
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
	xge_rect_t tRect;
	xge_rect_t tQuickBoolRect;
	xge_event_t tEvent;
	float fScrollX;
	float fScrollY;
	int iRow;
	int iColumn;
	int bReject;
	int bQuickBoolStable;
	int bPickerEditorReady;

	pApp->bCreateOK = (pApp->pGridWidget[0] != NULL) && (xgeXuiTableGridGetTableView(&pApp->tGrid[0]) != NULL);
	pApp->bLayoutOK = (pApp->pPanel[0] != NULL) && (pApp->pPanel[0]->tRect.fW > 300.0f) && (pApp->pPanel[3] != NULL) && (pApp->pPanel[3]->tRect.fH > 220.0f);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 0);
	xgeXuiInputSetText(&pApp->tGrid[GRID_DISPLAY].tEditInput, "Edited");
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 3);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 1);
	xgeXuiInputSetText(&pApp->tGrid[GRID_DISPLAY].tEditInput, "-99");
	bReject = (xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1) == 0);
	xgeXuiInputSetText(&pApp->tGrid[GRID_DISPLAY].tEditInput, "99");
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 4);
	xgeXuiComboBoxSetSelected(pApp->tGrid[GRID_DISPLAY].pCombo, 2);
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 5);
	xgeXuiColorPickerSetHex(&pApp->tGrid[GRID_DISPLAY].tColorPicker, "#8456D1");
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 6);
	xgeXuiDatePickerSetValue(&pApp->tGrid[GRID_DISPLAY].tDatePicker, DemoDateSerial(2026, 6, 25));
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 7);
	xgeXuiDatePickerSetValue(&pApp->tGrid[GRID_DISPLAY].tDatePicker, DemoTimeSerial(15, 30, 0));
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_DISPLAY], 0, 8);
	xgeXuiInputSetReadonly(&pApp->tGrid[GRID_DISPLAY].tEditInput, 0);
	xgeXuiInputSetText(&pApp->tGrid[GRID_DISPLAY].tEditInput, "Updated textarea summary");
	xgeXuiTableGridEndEdit(&pApp->tGrid[GRID_DISPLAY], 1);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_STATES], 0, 9);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_STATES], 4, 9);
	xgeXuiTableGridBeginEdit(&pApp->tGrid[GRID_STATES], 5, 9);
	bPickerEditorReady = (pApp->tGrid[GRID_STATES].bPickerEditor != 0) && (pApp->tGrid[GRID_STATES].pPickerDecoration != NULL);
	bQuickBoolStable = 0;
	if ( xgeXuiTableViewGetCellRect(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_IMMEDIATE]), 0, 3, &tQuickBoolRect) != 0 ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_DOWN;
		tEvent.iParam1 = XGE_MOUSE_LEFT;
		tEvent.fX = tQuickBoolRect.fX + tQuickBoolRect.fW * 0.5f;
		tEvent.fY = tQuickBoolRect.fY + tQuickBoolRect.fH * 0.5f;
		(void)xgeXuiTableGridEvent(&pApp->tGrid[GRID_IMMEDIATE], &tEvent);
		tEvent.iType = XGE_EVENT_XUI_DOUBLE_CLICK;
		(void)xgeXuiTableGridEvent(&pApp->tGrid[GRID_IMMEDIATE], &tEvent);
		bQuickBoolStable = (strcmp(pApp->arrData[GRID_IMMEDIATE][0][3], "false") == 0);
	}
	xgeXuiTableViewEnsureCellVisible(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_LARGE]), 120, 6);
	xgeXuiTableViewGetOffset(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_LARGE]), &fScrollX, &fScrollY);
	xgeXuiTableViewSetSelectedCell(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_IMMEDIATE]), 2, 2);
	xgeXuiTableViewGetSelectedCell(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_IMMEDIATE]), &iRow, &iColumn);
	pApp->bEditOK =
		(strcmp(pApp->arrData[GRID_DISPLAY][0][0], "Edited") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][1], "99") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][3], "false") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][4], "Blocked") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][5], "#8456D1") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][6], "2026-06-25") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][7], "15:30") == 0) &&
		(strcmp(pApp->arrData[GRID_DISPLAY][0][8], "Updated textarea summary") == 0) &&
		bQuickBoolStable &&
		bReject &&
		(pApp->iSetCount >= 8) &&
		(pApp->iChangeCount >= 8) &&
		(pApp->iValidateCount >= 2) &&
		(pApp->iPickerCount >= 3) &&
		bPickerEditorReady;
	pApp->bStateOK =
		(xgeXuiTableGridGetEditMode(&pApp->tGrid[GRID_IMMEDIATE]) == XGE_XUI_TABLE_GRID_EDIT_QUICK) &&
		(fScrollY > 1.0f) &&
		(iRow == 2) &&
		(iColumn == 2) &&
		(xgeXuiTableViewGetCellRect(xgeXuiTableGridGetTableView(&pApp->tGrid[GRID_DISPLAY]), 0, 0, &tRect) != 0) &&
		(tRect.fW > 20.0f);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	SeedData(pApp);
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
	for ( i = 0; i < GRID_COUNT; i++ ) {
		xgeXuiTableGridUnit(&pApp->tGrid[i]);
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
		printf("xui_tablegrid final-summary frames=%d create=%d layout=%d edit=%d state=%d custom=%d set=%d change=%d picker=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bEditOK,
			pApp->bStateOK,
			pApp->bCustomOK,
			pApp->iSetCount,
			pApp->iChangeCount,
			pApp->iPickerCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABLEGRID_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI TableGrid";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bEditOK && tApp.bStateOK && tApp.bCustomOK) ? 0 : 3;
}
