#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 920
#define DEMO_TARGET_H 470
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define GRID_COUNT 2
#define LABEL_COUNT 4
#define DEMO_ROWS 36
#define DEMO_COLS 8

typedef struct xui_tablegrid_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pStatus;
	xui_widget pGrid[GRID_COUNT];
	xui_widget pTable[GRID_COUNT];
	xui_table_view_column_t arrColumns[DEMO_COLS];
	xui_table_view_row_t arrRows[DEMO_ROWS];
	char arrText[GRID_COUNT][DEMO_ROWS][DEMO_COLS][96];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iSetCount;
	int iValidateCount;
	int iChangeCount;
	int iPickerCount;
	int iConfigCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bEditOK;
	int bValidateOK;
	int bPickerOK;
	int bScrollOK;
	int bQuickOK;
} xui_tablegrid_demo_t;

static void __xuiTableGridUsage(void)
{
	printf("usage: xui_tablegrid [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTableGridParseArgs(xui_tablegrid_demo_t* pDemo, int argc, char** argv)
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
			__xuiTableGridUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTableGridFindTtf(void)
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

static int __xuiTableGridRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_tablegrid_demo_t*)pUser;
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

static int __xuiTableGridIndexOf(xui_tablegrid_demo_t* pDemo, xui_widget pWidget)
{
	int i;

	for ( i = 0; i < GRID_COUNT; i++ ) {
		if ( (pDemo->pGrid[i] == pWidget) || (pDemo->pTable[i] == pWidget) ) return i;
	}
	return 0;
}

static int __xuiTableGridRowCount(xui_widget pWidget, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	int iIndex;

	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( pDemo == NULL ) return 0;
	iIndex = __xuiTableGridIndexOf(pDemo, pWidget);
	return (iIndex == 0) ? DEMO_ROWS : 12;
}

static int __xuiTableGridCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	int iIndex;

	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( (pDemo == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= DEMO_ROWS) || (iColumn >= DEMO_COLS) ) return 0;
	iIndex = __xuiTableGridIndexOf(pDemo, pWidget);
	pCell->sText = pDemo->arrText[iIndex][iRow][iColumn];
	switch ( iColumn ) {
	case 1: pCell->iType = XUI_TABLE_CELL_TYPE_INT; break;
	case 2: pCell->iType = XUI_TABLE_CELL_TYPE_BOOL; break;
	case 3: pCell->iType = XUI_TABLE_CELL_TYPE_ENUM; break;
	case 4: pCell->iType = XUI_TABLE_CELL_TYPE_COLOR; break;
	case 5: pCell->iType = XUI_TABLE_CELL_TYPE_DATE; break;
	case 6: pCell->iType = XUI_TABLE_CELL_TYPE_TEXTAREA; break;
	case 7: pCell->iType = XUI_TABLE_CELL_TYPE_PICKER; break;
	default: pCell->iType = XUI_TABLE_CELL_TYPE_TEXT; break;
	}
	if ( iIndex == 0 && iRow == 5 && iColumn == 1 ) pCell->bInvalid = 1;
	if ( iIndex == 0 && iRow == 6 && iColumn == 0 ) pCell->bDirty = 1;
	return 1;
}

static int __xuiTableGridStrictIntInRange(const char* sValue)
{
	const char* p;
	int iValue;

	if ( (sValue == NULL) || (sValue[0] == '\0') ) return 0;
	p = sValue;
	while ( *p != '\0' ) {
		if ( (*p < '0') || (*p > '9') ) return 0;
		p++;
	}
	iValue = atoi(sValue);
	return (iValue >= 0) && (iValue <= 100);
}

static void __xuiTableGridUpdateStatus(xui_tablegrid_demo_t* pDemo)
{
	char sText[192];

	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	snprintf(sText, sizeof(sText), "sets=%d validates=%d changes=%d pickers=%d configs=%d commits=%d rejects=%d cancels=%d",
		pDemo->iSetCount,
		pDemo->iValidateCount,
		pDemo->iChangeCount,
		pDemo->iPickerCount,
		pDemo->iConfigCount,
		xuiTableGridGetCommitCount(pDemo->pGrid[0]),
		xuiTableGridGetRejectCount(pDemo->pGrid[0]),
		xuiTableGridGetCancelCount(pDemo->pGrid[0]));
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static void __xuiTableGridSet(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	int iIndex;

	(void)iType;
	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( (pDemo == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= DEMO_ROWS) || (iColumn >= DEMO_COLS) ) return;
	iIndex = __xuiTableGridIndexOf(pDemo, pWidget);
	snprintf(pDemo->arrText[iIndex][iRow][iColumn], sizeof(pDemo->arrText[iIndex][iRow][iColumn]), "%s", sValue != NULL ? sValue : "");
	pDemo->iSetCount++;
	__xuiTableGridUpdateStatus(pDemo);
}

static int __xuiTableGridValidate(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;

	(void)pWidget;
	(void)iRow;
	(void)iType;
	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iValidateCount++;
		__xuiTableGridUpdateStatus(pDemo);
	}
	return (iColumn == 1) ? __xuiTableGridStrictIntInRange(sValue) : 1;
}

static void __xuiTableGridChange(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)sValue;
	(void)iType;
	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
		__xuiTableGridUpdateStatus(pDemo);
	}
}

static int __xuiTableGridEditor(xui_widget pWidget, int iRow, int iColumn, const xui_table_view_cell_t* pCell, xui_rect_t tWorldRect, void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	int iIndex;

	(void)pCell;
	(void)tWorldRect;
	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( (pDemo == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= DEMO_ROWS) || (iColumn >= DEMO_COLS) ) return 0;
	iIndex = __xuiTableGridIndexOf(pDemo, pWidget);
	snprintf(pDemo->arrText[iIndex][iRow][iColumn], sizeof(pDemo->arrText[iIndex][iRow][iColumn]), "Picked");
	pDemo->iPickerCount++;
	__xuiTableGridUpdateStatus(pDemo);
	return 1;
}

static int __xuiTableGridEditorConfig(xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	static const char* arrStatus[] = {"Draft", "Ready", "Done", "Blocked"};
	static const uint32_t arrPalette[] = {
		XUI_COLOR_RGBA(47, 128, 237, 255),
		XUI_COLOR_RGBA(67, 167, 124, 255),
		XUI_COLOR_RGBA(219, 72, 99, 255),
		XUI_COLOR_RGBA(237, 161, 47, 255)
	};
	xui_tablegrid_demo_t* pDemo;

	(void)pWidget;
	(void)iRow;
	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iConfigCount++;
	}
	if ( pConfig == NULL ) return 0;
	if ( (iColumn == 3) && (iType == XUI_TABLE_CELL_TYPE_ENUM) ) {
		pConfig->arrEnumItems = arrStatus;
		pConfig->iEnumItemCount = 4;
		pConfig->iEnumSelected = 2;
		return 1;
	}
	if ( (iColumn == 1) && (iType == XUI_TABLE_CELL_TYPE_INT) ) {
		pConfig->fMin = 0.0f;
		pConfig->fMax = 100.0f;
		pConfig->fStep = 1.0f;
		pConfig->iPrecision = 0;
		return 1;
	}
	if ( (iColumn == 4) && (iType == XUI_TABLE_CELL_TYPE_COLOR) ) {
		pConfig->arrPalette = arrPalette;
		pConfig->iPaletteCount = 4;
		return 1;
	}
	if ( (iColumn == 5) && (iType == XUI_TABLE_CELL_TYPE_DATE) ) {
		pConfig->sDateFormat = "yyyy-mm-dd";
		pConfig->bShowSecond = 0;
		return 1;
	}
	return 0;
}

static int __xuiTableGridAddLabel(xui_tablegrid_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
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

static int __xuiTableGridAddGrid(xui_tablegrid_demo_t* pDemo, int iIndex, xui_rect_t tRect)
{
	xui_table_grid_desc_t tDesc;
	xui_widget pGrid;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= GRID_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = pDemo->arrColumns;
	tDesc.iColumnCount = DEMO_COLS;
	tDesc.arrRows = pDemo->arrRows;
	tDesc.iRowCount = DEMO_ROWS;
	tDesc.onCount = __xuiTableGridRowCount;
	tDesc.onCell = __xuiTableGridCell;
	tDesc.onSet = __xuiTableGridSet;
	tDesc.pAdapterUser = pDemo;
	tDesc.onValidate = __xuiTableGridValidate;
	tDesc.pValidateUser = pDemo;
	tDesc.onChange = __xuiTableGridChange;
	tDesc.pChangeUser = pDemo;
	tDesc.onEditor = __xuiTableGridEditor;
	tDesc.pEditorUser = pDemo;
	tDesc.onEditorConfig = __xuiTableGridEditorConfig;
	tDesc.pEditorConfigUser = pDemo;
	tDesc.pFont = pDemo->pFont;
	tDesc.fDefaultRowHeight = 28.0f;
	tDesc.fHeaderHeight = 28.0f;
	tDesc.iSelectionMode = XUI_TABLE_VIEW_SELECTION_CELL;
	tDesc.iEditMode = (iIndex == 0) ? XUI_TABLE_GRID_EDIT_DISPLAY : XUI_TABLE_GRID_EDIT_QUICK;
	iRet = xuiTableGridCreate(pDemo->pContext, &pGrid, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pGrid, tRect);
	if ( iIndex == 1 ) {
		(void)xuiTableGridSetColors(pGrid,
			XUI_COLOR_RGBA(247, 253, 250, 255),
			XUI_COLOR_RGBA(226, 242, 235, 255),
			XUI_COLOR_RGBA(255, 255, 255, 255),
			XUI_COLOR_RGBA(42, 151, 110, 255),
			XUI_COLOR_RGBA(189, 221, 210, 255),
			XUI_COLOR_RGBA(31, 76, 60, 255));
	}
	iRet = xuiWidgetAddChild(pDemo->pRoot, pGrid);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pGrid);
		return iRet;
	}
	pDemo->pGrid[iIndex] = pGrid;
	pDemo->pTable[iIndex] = xuiTableGridGetTableView(pGrid);
	return XUI_OK;
}

static int __xuiTableGridCreateUi(xui_tablegrid_demo_t* pDemo)
{
	int i;
	int j;
	int k;
	int iRet;

	memset(pDemo->arrColumns, 0, sizeof(pDemo->arrColumns));
	pDemo->arrColumns[0].sTitle = "Item";
	pDemo->arrColumns[0].fWidth = 114.0f;
	pDemo->arrColumns[0].iType = XUI_TABLE_CELL_TYPE_TEXT;
	pDemo->arrColumns[1].sTitle = "Qty";
	pDemo->arrColumns[1].fWidth = 72.0f;
	pDemo->arrColumns[1].iType = XUI_TABLE_CELL_TYPE_INT;
	pDemo->arrColumns[1].iAlign = XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE;
	pDemo->arrColumns[2].sTitle = "Enabled";
	pDemo->arrColumns[2].fWidth = 82.0f;
	pDemo->arrColumns[2].iType = XUI_TABLE_CELL_TYPE_BOOL;
	pDemo->arrColumns[3].sTitle = "Status";
	pDemo->arrColumns[3].fWidth = 108.0f;
	pDemo->arrColumns[3].iType = XUI_TABLE_CELL_TYPE_ENUM;
	pDemo->arrColumns[4].sTitle = "Color";
	pDemo->arrColumns[4].fWidth = 108.0f;
	pDemo->arrColumns[4].iType = XUI_TABLE_CELL_TYPE_COLOR;
	pDemo->arrColumns[5].sTitle = "Due";
	pDemo->arrColumns[5].fWidth = 124.0f;
	pDemo->arrColumns[5].iType = XUI_TABLE_CELL_TYPE_DATE;
	pDemo->arrColumns[6].sTitle = "Notes";
	pDemo->arrColumns[6].fWidth = 154.0f;
	pDemo->arrColumns[6].iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
	pDemo->arrColumns[7].sTitle = "Action";
	pDemo->arrColumns[7].fWidth = 90.0f;
	pDemo->arrColumns[7].iType = XUI_TABLE_CELL_TYPE_PICKER;
	for ( i = 0; i < DEMO_COLS; i++ ) {
		pDemo->arrColumns[i].bVisibleSet = 1;
		pDemo->arrColumns[i].bVisible = 1;
		pDemo->arrColumns[i].bResizableSet = 1;
		pDemo->arrColumns[i].bResizable = 1;
		pDemo->arrColumns[i].fMinWidth = 44.0f;
	}
	for ( i = 0; i < DEMO_ROWS; i++ ) {
		memset(&pDemo->arrRows[i], 0, sizeof(pDemo->arrRows[i]));
		pDemo->arrRows[i].fHeight = ((i % 5) == 0) ? 31.0f : 28.0f;
		for ( j = 0; j < GRID_COUNT; j++ ) {
			for ( k = 0; k < DEMO_COLS; k++ ) {
				switch ( k ) {
				case 1:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "%d", 20 + i);
					break;
				case 2:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "%s", (i % 2) ? "false" : "true");
					break;
				case 3:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "%s", (i % 3) == 0 ? "Draft" : "Ready");
					break;
				case 4:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "#%02X%02X%02X", 52 + (i * 7) % 170, 132 + (j * 18), 198 - (i * 3) % 80);
					break;
				case 5:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "2026-05-%02d", 1 + (i % 28));
					break;
				case 6:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "Note for row %02d", i);
					break;
				case 7:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "...");
					break;
				default:
					snprintf(pDemo->arrText[j][i][k], sizeof(pDemo->arrText[j][i][k]), "Task %02d", i);
					break;
				}
			}
		}
	}
	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTableGridRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiTableGridAddLabel(pDemo, 0, "XUI TableGrid", (xui_rect_t){48.0f, 30.0f, 180.0f, 24.0f}) != XUI_OK ||
	     __xuiTableGridAddLabel(pDemo, 1, "Display edit mode", (xui_rect_t){48.0f, 64.0f, 220.0f, 22.0f}) != XUI_OK ||
	     __xuiTableGridAddLabel(pDemo, 2, "Quick edit mode", (xui_rect_t){48.0f, 282.0f, 220.0f, 22.0f}) != XUI_OK ||
	     __xuiTableGridAddLabel(pDemo, 3, "sets=0 validates=0 changes=0 pickers=0 configs=0 commits=0 rejects=0 cancels=0", (xui_rect_t){48.0f, 424.0f, 820.0f, 24.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[3];
	if ( __xuiTableGridAddGrid(pDemo, 0, (xui_rect_t){48.0f, 90.0f, 824.0f, 170.0f}) != XUI_OK ||
	     __xuiTableGridAddGrid(pDemo, 1, (xui_rect_t){48.0f, 308.0f, 824.0f, 94.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static uint32_t __xuiTableGridReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTableGridXgeKeyToXui(int iKey)
{
	switch ( iKey ) {
	case XGE_KEY_ENTER: return XUI_KEY_ENTER;
	case XGE_KEY_TAB: return XUI_KEY_TAB;
	case XGE_KEY_SPACE: return XUI_KEY_SPACE;
	case XGE_KEY_BACKSPACE: return 8;
	case XGE_KEY_DELETE: return 46;
	case XGE_KEY_LEFT: return XUI_KEY_LEFT;
	case XGE_KEY_RIGHT: return XUI_KEY_RIGHT;
	case XGE_KEY_UP: return XUI_KEY_UP;
	case XGE_KEY_DOWN: return XUI_KEY_DOWN;
	case XGE_KEY_HOME: return XUI_KEY_HOME;
	case XGE_KEY_END: return XUI_KEY_END;
	case XGE_KEY_ESCAPE: return XUI_KEY_ESCAPE;
	default: return 0;
	}
}

static int __xuiTableGridHandleKeys(xui_tablegrid_demo_t* pDemo)
{
	static const int arrKeys[] = {
		XGE_KEY_ENTER,
		XGE_KEY_TAB,
		XGE_KEY_SPACE,
		XGE_KEY_BACKSPACE,
		XGE_KEY_DELETE,
		XGE_KEY_LEFT,
		XGE_KEY_RIGHT,
		XGE_KEY_UP,
		XGE_KEY_DOWN,
		XGE_KEY_HOME,
		XGE_KEY_END,
		XGE_KEY_ESCAPE
	};
	uint32_t iText;
	int i;
	int iKey;
	int iRet;

	for ( i = 0; i < (int)(sizeof(arrKeys) / sizeof(arrKeys[0])); i++ ) {
		if ( xgeKeyPressed(arrKeys[i]) ) {
			if ( arrKeys[i] == XGE_KEY_ESCAPE && !xuiTableGridIsEditing(pDemo->pGrid[0]) && !xuiTableGridIsEditing(pDemo->pGrid[1]) ) {
				xgeQuit();
				continue;
			}
			iKey = __xuiTableGridXgeKeyToXui(arrKeys[i]);
			if ( iKey != 0 ) {
				iRet = xuiInputKeyDown(pDemo->pContext, iKey, 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
		if ( xgeKeyReleased(arrKeys[i]) ) {
			iKey = __xuiTableGridXgeKeyToXui(arrKeys[i]);
			if ( iKey != 0 ) {
				iRet = xuiInputKeyUp(pDemo->pContext, iKey, 0);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		iRet = xuiInputText(pDemo->pContext, iText);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTableGridHandleInput(xui_tablegrid_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiTableGridReadButtons();
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
	iRet = __xuiTableGridHandleKeys(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiTableGridTypeText(xui_tablegrid_demo_t* pDemo, const char* sText)
{
	int iRet;

	if ( sText == NULL ) return XUI_OK;
	while ( *sText != '\0' ) {
		iRet = xuiInputText(pDemo->pContext, (uint32_t)(unsigned char)*sText++);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pDemo->pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTableGridClickCell(xui_tablegrid_demo_t* pDemo, xui_widget pTable, int iRow, int iColumn)
{
	xui_rect_t tWorld;
	xui_rect_t tCell;
	float fX;
	float fY;
	int iRet;

	if ( xuiTableViewGetCellRect(pTable, iRow, iColumn, &tCell) != XUI_OK ) return XUI_ERROR;
	tWorld = xuiWidgetGetWorldRect(pTable);
	fX = tWorld.fX + tCell.fX + tCell.fW * 0.5f;
	fY = tWorld.fY + tCell.fY + tCell.fH * 0.5f;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiTableGridRunChecks(xui_tablegrid_demo_t* pDemo, int bExerciseInput)
{
	float fOffsetX;
	float fOffsetY;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pGrid[0] != NULL) && (pDemo->pGrid[1] != NULL) && (pDemo->pTable[0] != NULL);
	pDemo->bLayoutOK = pDemo->bCreateOK && (xuiTableViewGetFrameWidget(pDemo->pTable[0]) != NULL) && (xuiTableViewGetColumnCount(pDemo->pTable[0]) == DEMO_COLS);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiTableGridBeginEdit(pDemo->pGrid[0], 0, 0);
		(void)__xuiTableGridTypeText(pDemo, "Edited");
		(void)xuiTableGridEndEdit(pDemo->pGrid[0], 1);
		(void)xuiTableGridBeginEdit(pDemo->pGrid[0], 0, 1);
		(void)__xuiTableGridTypeText(pDemo, "bad");
		pDemo->bValidateOK = (xuiTableGridEndEdit(pDemo->pGrid[0], 1) == 0) && (xuiTableGridGetRejectCount(pDemo->pGrid[0]) > 0);
		(void)xuiTableGridEndEdit(pDemo->pGrid[0], 0);
		(void)xuiTableGridBeginEdit(pDemo->pGrid[0], 0, 2);
		(void)xuiTableGridBeginEdit(pDemo->pGrid[0], 0, 3);
		(void)xuiTableGridEndEdit(pDemo->pGrid[0], 1);
		(void)xuiTableGridBeginEdit(pDemo->pGrid[0], 0, 7);
		pDemo->bEditOK = (strcmp(pDemo->arrText[0][0][0], "Edited") == 0) &&
			(strcmp(pDemo->arrText[0][0][2], "false") == 0) &&
			(strcmp(pDemo->arrText[0][0][3], "Done") == 0) &&
			(xuiTableGridGetCommitCount(pDemo->pGrid[0]) >= 3);
		pDemo->bPickerOK = (pDemo->iPickerCount > 0) && (strcmp(pDemo->arrText[0][0][7], "Picked") == 0);
		(void)xuiTableViewEnsureCellVisible(pDemo->pTable[0], DEMO_ROWS - 1, DEMO_COLS - 1);
		fOffsetX = 0.0f;
		fOffsetY = 0.0f;
		(void)xuiTableViewGetOffset(pDemo->pTable[0], &fOffsetX, &fOffsetY);
		pDemo->bScrollOK = (fOffsetX > 0.0f) || (fOffsetY > 0.0f);
		(void)__xuiTableGridClickCell(pDemo, pDemo->pTable[1], 0, 0);
		pDemo->bQuickOK = xuiTableGridIsEditing(pDemo->pGrid[1]) != 0;
		(void)xuiTableGridEndEdit(pDemo->pGrid[1], 0);
		pDemo->bExerciseDone = 1;
		__xuiTableGridUpdateStatus(pDemo);
	} else if ( !bExerciseInput ) {
		pDemo->bEditOK = 1;
		pDemo->bValidateOK = 1;
		pDemo->bPickerOK = 1;
		pDemo->bScrollOK = 1;
		pDemo->bQuickOK = 1;
	}
}

static int __xuiTableGridCreateAssets(xui_tablegrid_demo_t* pDemo)
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
	sFontPath = __xuiTableGridFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTableGridCreateUi(pDemo);
}

static void __xuiTableGridDestroyAssets(xui_tablegrid_demo_t* pDemo)
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

static int __xuiTableGridFrame(void* pUser)
{
	xui_tablegrid_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_tablegrid_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTableGridHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTableGridRunChecks(pDemo, bAutoRun);
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
		printf("xui_tablegrid final-summary frames=%d create=%d layout=%d edit=%d validate=%d picker=%d scroll=%d quick=%d commits=%d rejects=%d cancels=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame,
			pDemo->bCreateOK,
			pDemo->bLayoutOK,
			pDemo->bEditOK,
			pDemo->bValidateOK,
			pDemo->bPickerOK,
			pDemo->bScrollOK,
			pDemo->bQuickOK,
			xuiTableGridGetCommitCount(pDemo->pGrid[0]),
			xuiTableGridGetRejectCount(pDemo->pGrid[0]),
			xuiTableGridGetCancelCount(pDemo->pGrid[0]),
			tStats.iUpdatedCaches,
			tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_tablegrid_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTableGridParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTableGridUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI TableGrid";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_tablegrid: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTableGridCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_tablegrid: create assets failed: %d\n", iRet);
		__xuiTableGridDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTableGridFrame, &tDemo);
	__xuiTableGridDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bEditOK && tDemo.bValidateOK && tDemo.bPickerOK && tDemo.bScrollOK && tDemo.bQuickOK) ? 0 : 1;
}
