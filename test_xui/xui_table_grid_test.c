#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_table_grid_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

#define GRID_ROWS 6
#define GRID_COLS 8

typedef struct table_grid_test_data_t {
	char arrText[GRID_ROWS][GRID_COLS][96];
	int iSetCount;
	int iValidateCount;
	int iChangeCount;
	int iPickerCount;
	int iConfigCount;
	int iLastRow;
	int iLastColumn;
	int iLastType;
	char sLastValue[96];
} table_grid_test_data_t;

static int __xuiTableGridTestCount(xui_widget pWidget, void* pUser)
{
	table_grid_test_data_t* pData;

	(void)pWidget;
	pData = (table_grid_test_data_t*)pUser;
	return (pData != NULL) ? GRID_ROWS : 0;
}

static int __xuiTableGridTestCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	table_grid_test_data_t* pData;

	(void)pWidget;
	pData = (table_grid_test_data_t*)pUser;
	if ( (pData == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= GRID_ROWS) || (iColumn >= GRID_COLS) ) {
		return 0;
	}
	pCell->sText = pData->arrText[iRow][iColumn];
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
	if ( iRow == 5 && iColumn == 0 ) {
		pCell->bDisabled = 1;
	}
	return 1;
}

static void __xuiTableGridTestSet(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	table_grid_test_data_t* pData;

	(void)pWidget;
	pData = (table_grid_test_data_t*)pUser;
	if ( (pData == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= GRID_ROWS) || (iColumn >= GRID_COLS) ) {
		return;
	}
	snprintf(pData->arrText[iRow][iColumn], sizeof(pData->arrText[iRow][iColumn]), "%s", sValue != NULL ? sValue : "");
	snprintf(pData->sLastValue, sizeof(pData->sLastValue), "%s", sValue != NULL ? sValue : "");
	pData->iSetCount++;
	pData->iLastRow = iRow;
	pData->iLastColumn = iColumn;
	pData->iLastType = iType;
}

static int __xuiTableGridTestValidate(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	table_grid_test_data_t* pData;
	int iValue;
	const char* p;

	(void)pWidget;
	(void)iRow;
	(void)iType;
	pData = (table_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iValidateCount++;
	}
	if ( iColumn == 1 ) {
		if ( (sValue == NULL) || (sValue[0] == '\0') ) {
			return 0;
		}
		p = sValue;
		while ( *p != '\0' ) {
			if ( (*p < '0') || (*p > '9') ) {
				return 0;
			}
			p++;
		}
		iValue = atoi(sValue != NULL ? sValue : "");
		return (iValue >= 0) && (iValue <= 100);
	}
	return 1;
}

static void __xuiTableGridTestChange(xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	table_grid_test_data_t* pData;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)sValue;
	(void)iType;
	pData = (table_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iChangeCount++;
	}
}

static int __xuiTableGridTestEditor(xui_widget pWidget, int iRow, int iColumn, const xui_table_view_cell_t* pCell, xui_rect_t tWorldRect, void* pUser)
{
	table_grid_test_data_t* pData;

	(void)pWidget;
	(void)pCell;
	(void)tWorldRect;
	pData = (table_grid_test_data_t*)pUser;
	if ( (pData == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= GRID_ROWS) || (iColumn >= GRID_COLS) ) {
		return 0;
	}
	snprintf(pData->arrText[iRow][iColumn], sizeof(pData->arrText[iRow][iColumn]), "Picked %d:%d", iRow, iColumn);
	pData->iPickerCount++;
	return 1;
}

static int __xuiTableGridTestEditorConfig(xui_widget pWidget, int iRow, int iColumn, int iType, xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	static const char* arrStatus[] = {"Draft", "Ready", "Done"};
	static const uint32_t arrPalette[] = {
		XUI_COLOR_RGBA(47, 128, 237, 255),
		XUI_COLOR_RGBA(67, 167, 124, 255),
		XUI_COLOR_RGBA(219, 72, 99, 255)
	};
	table_grid_test_data_t* pData;

	(void)pWidget;
	(void)iRow;
	pData = (table_grid_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iConfigCount++;
	}
	if ( pConfig == NULL ) {
		return 0;
	}
	if ( (iColumn == 3) && (iType == XUI_TABLE_CELL_TYPE_ENUM) ) {
		pConfig->arrEnumItems = arrStatus;
		pConfig->iEnumItemCount = 3;
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
		pConfig->iPaletteCount = 3;
		return 1;
	}
	return 0;
}

static int __xuiTableGridTestRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 760, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTableGridTestTypeText(xui_context pContext, const char* sText)
{
	int iRet;

	if ( sText == NULL ) return XUI_OK;
	while ( *sText != '\0' ) {
		iRet = xuiInputText(pContext, (uint32_t)(unsigned char)*sText++);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiTableGridTestClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

static xui_widget __xuiTableGridTestFindNumericEditor(xui_context pContext, xui_widget pGrid)
{
	xui_widget_type pNumericType;
	xui_widget pChild;

	if ( (pContext == NULL) || (pGrid == NULL) ) {
		return NULL;
	}
	pNumericType = xuiNumericInputGetType(pContext);
	for ( pChild = xuiWidgetGetFirstChild(pGrid); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		if ( xuiWidgetGetVisible(pChild) && xuiWidgetIsType(pChild, pNumericType) ) {
			return pChild;
		}
	}
	return NULL;
}

static xui_widget __xuiTableGridTestFindPopupOwner(xui_context pContext, xui_widget_type pOwnerType)
{
	xui_widget pScan;
	xui_widget pOwner;
	xui_widget_type pPopupType;

	if ( pContext == NULL ) return NULL;
	pPopupType = xuiPopupGetType(pContext);
	pScan = xuiGetFocusWidget(pContext);
	while ( pScan != NULL ) {
		if ( xuiWidgetIsType(pScan, pPopupType) ) {
			pOwner = xuiPopupGetOwner(pScan);
			if ( pOwner != NULL && (pOwnerType == NULL || xuiWidgetIsType(pOwner, pOwnerType)) ) {
				return pOwner;
			}
		}
		pScan = xuiWidgetGetParent(pScan);
	}
	return NULL;
}

static int __xuiTableGridTestCheckNumericEditor(xui_context pContext, xui_widget pGrid, const char* sExpected)
{
	xui_widget pEditor;
	xui_widget pInput;
	int iEditorLayer;
	int iEditorZ;
	int iInputLayer;
	int iInputZ;
	int iStart;
	int iEnd;
	int iExpectedLength;
	int iNodeCount;
	int iEditorNode;
	int iInputNode;
	int i;
	xui_render_node_t tNode;

	if ( (pContext == NULL) || (pGrid == NULL) || (sExpected == NULL) ) {
		return 0;
	}
	pEditor = __xuiTableGridTestFindNumericEditor(pContext, pGrid);
	if ( pEditor == NULL ) {
		return 0;
	}
	pInput = xuiNumericInputGetInputWidget(pEditor);
	if ( (pInput == NULL) || !xuiWidgetGetVisible(pInput) ) {
		return 0;
	}
	if ( strcmp(xuiNumericInputGetText(pEditor), sExpected) != 0 ) {
		return 0;
	}
	if ( strcmp(xuiInputGetText(pInput), sExpected) != 0 ) {
		return 0;
	}
	if ( xuiInputGetSelection(pInput, &iStart, &iEnd) != XUI_OK ) {
		return 0;
	}
	iExpectedLength = (int)strlen(sExpected);
	if ( (iStart != 0) || (iEnd != iExpectedLength) ) {
		return 0;
	}
	if ( xuiWidgetGetLayer(pEditor, &iEditorLayer, &iEditorZ) != XUI_OK ) {
		return 0;
	}
	if ( xuiWidgetGetLayer(pInput, &iInputLayer, &iInputZ) != XUI_OK ) {
		return 0;
	}
	if ( (iInputLayer != iEditorLayer) || (iInputZ < iEditorZ) ) {
		return 0;
	}
	iEditorNode = -1;
	iInputNode = -1;
	iNodeCount = xuiGetRenderNodeCount(pContext);
	for ( i = 0; i < iNodeCount; i++ ) {
		if ( xuiGetRenderNode(pContext, i, &tNode) != XUI_OK ) {
			return 0;
		}
		if ( tNode.pWidget == pEditor ) {
			iEditorNode = i;
		} else if ( tNode.pWidget == pInput ) {
			iInputNode = i;
		}
	}
	if ( (iEditorNode < 0) || (iInputNode <= iEditorNode) ) {
		return 0;
	}
	return 1;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pGrid;
	xui_widget pTable;
	xui_widget pNumericEditor;
	xui_widget pNumericInput;
	xui_widget pPopupOwner;
	xui_surface pTarget;
	xui_font pFont;
	xui_table_view_column_t arrColumns[GRID_COLS];
	xui_table_view_row_t arrRows[GRID_ROWS];
	xui_table_grid_desc_t tDesc;
	table_grid_test_data_t tData;
	xui_rect_t tCell;
	xui_rect_t tWorld;
	int iRow;
	int iColumn;
	int iFailed;
	int iRet;
	int i;
	int j;

	pContext = NULL;
	pRoot = NULL;
	pGrid = NULL;
	pTable = NULL;
	pNumericEditor = NULL;
	pNumericInput = NULL;
	pPopupOwner = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	memset(&tData, 0, sizeof(tData));
	for ( i = 0; i < GRID_ROWS; i++ ) {
		snprintf(tData.arrText[i][0], sizeof(tData.arrText[i][0]), "Item %d", i);
		snprintf(tData.arrText[i][1], sizeof(tData.arrText[i][1]), "%d", 10 + i);
		snprintf(tData.arrText[i][2], sizeof(tData.arrText[i][2]), "%s", (i % 2) ? "false" : "true");
		snprintf(tData.arrText[i][3], sizeof(tData.arrText[i][3]), "%s", (i % 3) == 0 ? "Draft" : "Ready");
		snprintf(tData.arrText[i][4], sizeof(tData.arrText[i][4]), "#2F80ED");
		snprintf(tData.arrText[i][5], sizeof(tData.arrText[i][5]), "2026-05-%02d", 18 + i);
		snprintf(tData.arrText[i][6], sizeof(tData.arrText[i][6]), "Note %d", i);
		snprintf(tData.arrText[i][7], sizeof(tData.arrText[i][7]), "...");
	}

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "tablegrid", 9, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 760.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 112.0f;
	arrColumns[0].iType = XUI_TABLE_CELL_TYPE_TEXT;
	arrColumns[1].sTitle = "Qty";
	arrColumns[1].fWidth = 78.0f;
	arrColumns[1].iType = XUI_TABLE_CELL_TYPE_INT;
	arrColumns[1].iAlign = XUI_TEXT_ALIGN_RIGHT | XUI_TEXT_ALIGN_MIDDLE;
	arrColumns[2].sTitle = "Flag";
	arrColumns[2].fWidth = 78.0f;
	arrColumns[2].iType = XUI_TABLE_CELL_TYPE_BOOL;
	arrColumns[3].sTitle = "Status";
	arrColumns[3].fWidth = 96.0f;
	arrColumns[3].iType = XUI_TABLE_CELL_TYPE_ENUM;
	arrColumns[4].sTitle = "Color";
	arrColumns[4].fWidth = 106.0f;
	arrColumns[4].iType = XUI_TABLE_CELL_TYPE_COLOR;
	arrColumns[5].sTitle = "Date";
	arrColumns[5].fWidth = 122.0f;
	arrColumns[5].iType = XUI_TABLE_CELL_TYPE_DATE;
	arrColumns[6].sTitle = "Notes";
	arrColumns[6].fWidth = 148.0f;
	arrColumns[6].iType = XUI_TABLE_CELL_TYPE_TEXTAREA;
	arrColumns[7].sTitle = "Pick";
	arrColumns[7].fWidth = 88.0f;
	arrColumns[7].iType = XUI_TABLE_CELL_TYPE_PICKER;
	memset(arrRows, 0, sizeof(arrRows));
	for ( j = 0; j < GRID_ROWS; j++ ) {
		arrRows[j].fHeight = 30.0f;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.arrRows = arrRows;
	tDesc.iColumnCount = GRID_COLS;
	tDesc.iRowCount = GRID_ROWS;
	tDesc.onCount = __xuiTableGridTestCount;
	tDesc.onCell = __xuiTableGridTestCell;
	tDesc.onSet = __xuiTableGridTestSet;
	tDesc.pAdapterUser = &tData;
	tDesc.onValidate = __xuiTableGridTestValidate;
	tDesc.pValidateUser = &tData;
	tDesc.onChange = __xuiTableGridTestChange;
	tDesc.pChangeUser = &tData;
	tDesc.onEditor = __xuiTableGridTestEditor;
	tDesc.pEditorUser = &tData;
	tDesc.onEditorConfig = __xuiTableGridTestEditorConfig;
	tDesc.pEditorConfigUser = &tData;
	tDesc.pFont = pFont;
	tDesc.fDefaultRowHeight = 30.0f;
	tDesc.fHeaderHeight = 30.0f;
	tDesc.iSelectionMode = XUI_TABLE_VIEW_SELECTION_CELL;
	tDesc.iEditMode = XUI_TABLE_GRID_EDIT_DISPLAY;
	iRet = xuiTableGridCreate(pContext, &pGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGrid != NULL, "tablegrid create");
	xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 720.0f, 262.0f});
	iRet = xuiWidgetAddChild(pRoot, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK, "add tablegrid");
	pTable = xuiTableGridGetTableView(pGrid);
	XUI_TEST_CHECK(pTable != NULL, "inner table");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 760, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiTableViewGetColumnCount(pTable) == GRID_COLS, "column count");
	XUI_TEST_CHECK(xuiTableViewGetRowCount(pTable) == GRID_ROWS, "row count");
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTable, 0, 0, &tCell) == XUI_OK && tCell.fW > 20.0f, "cell rect");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "text begin");
	iRet = __xuiTableGridTestTypeText(pContext, "Renamed");
	XUI_TEST_CHECK(iRet == XUI_OK, "text input");
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "text commit");
	XUI_TEST_CHECK(strcmp(tData.arrText[0][0], "Renamed") == 0, "text set");
	XUI_TEST_CHECK(tData.iSetCount == 1 && tData.iChangeCount == 1 && xuiTableGridGetCommitCount(pGrid) == 1, "text counters");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 1);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "numeric begin invalid");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric editor layout invalid");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric editor render invalid");
	XUI_TEST_CHECK(__xuiTableGridTestCheckNumericEditor(pContext, pGrid, "10"), "numeric editor shows initial value invalid");
	iRet = __xuiTableGridTestTypeText(pContext, "bad");
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric input invalid");
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet == 0 && xuiTableGridIsEditing(pGrid), "numeric reject keeps editor");
	XUI_TEST_CHECK(xuiTableGridGetRejectCount(pGrid) == 1, "reject count");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid) && xuiTableGridGetCancelCount(pGrid) == 1, "numeric cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 1);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "numeric begin valid");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric editor layout valid");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric editor render valid");
	XUI_TEST_CHECK(__xuiTableGridTestCheckNumericEditor(pContext, pGrid, "10"), "numeric editor shows initial value valid");
	iRet = __xuiTableGridTestTypeText(pContext, "42");
	XUI_TEST_CHECK(iRet == XUI_OK, "numeric input valid");
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(tData.arrText[0][1], "42") == 0, "numeric commit");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 2);
	XUI_TEST_CHECK(iRet != 0 && strcmp(tData.arrText[0][2], "false") == 0, "bool toggle");
	XUI_TEST_CHECK(!xuiTableGridIsEditing(pGrid), "bool does not stay editing");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 3);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "enum begin");
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(tData.arrText[0][3], "Done") == 0, "enum commit");
	XUI_TEST_CHECK(tData.iConfigCount >= 2, "editor config used");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 6);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "textarea begin");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridGetCancelCount(pGrid) == 2, "textarea cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 7);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "picker begin");
	XUI_TEST_CHECK(tData.iPickerCount == 1 && xuiTableGridGetPickerCount(pGrid) == 1 && strcmp(tData.arrText[0][7], "Picked 0:7") == 0, "picker callback");

	iRet = xuiTableGridBeginEdit(pGrid, 5, 0);
	XUI_TEST_CHECK(iRet == 0, "disabled cell not editable");
	iRet = xuiTableGridSetEditMode(pGrid, XUI_TABLE_GRID_EDIT_QUICK);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridGetEditMode(pGrid) == XUI_TABLE_GRID_EDIT_QUICK, "edit mode");
	iRet = xuiTableViewGetCellRect(pTable, 1, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick numeric rect");
	tWorld = xuiWidgetGetWorldRect(pTable);
	iRet = __xuiTableGridTestClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "quick numeric click edit");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick numeric layout");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick numeric render");
	XUI_TEST_CHECK(__xuiTableGridTestCheckNumericEditor(pContext, pGrid, "11"), "quick numeric editor shows initial value");
	pNumericEditor = __xuiTableGridTestFindNumericEditor(pContext, pGrid);
	XUI_TEST_CHECK(pNumericEditor != NULL, "quick numeric editor visible");
	pNumericInput = xuiNumericInputGetInputWidget(pNumericEditor);
	XUI_TEST_CHECK(pNumericInput != NULL, "quick numeric input visible");
	iRet = xuiInputSetSelection(pNumericInput, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick numeric input collapse selection");
	tWorld = xuiWidgetGetWorldRect(pNumericInput);
	iRet = __xuiTableGridTestClick(pContext, tWorld.fX + tWorld.fW * 0.35f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "quick numeric input click keeps editor");
	iRet = xuiInputGetSelection(pNumericInput, &iRow, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && !(iRow == 0 && iColumn == (int)strlen(xuiInputGetText(pNumericInput))), "quick numeric input click does not restart edit");
	tWorld = xuiWidgetGetWorldRect(pNumericEditor);
	iRet = xuiInputPointerWheel(pContext, tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f, 0.0f, -1.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "quick numeric wheel keeps editing");
	XUI_TEST_CHECK(strcmp(xuiNumericInputGetText(pNumericEditor), "10") == 0, "quick numeric wheel steps editor");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "quick numeric cancel");
	iRow = -1;
	iColumn = -1;
	iRet = xuiTableGridGetEditingCell(pGrid, &iRow, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow == -1 && iColumn == -1, "editing cell clear");

	iRet = xuiTableViewGetCellRect(pTable, 1, 3, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick enum rect");
	tWorld = xuiWidgetGetWorldRect(pTable);
	iRet = __xuiTableGridTestClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "quick enum click edit");
	pPopupOwner = __xuiTableGridTestFindPopupOwner(pContext, xuiComboBoxGetType(pContext));
	XUI_TEST_CHECK(pPopupOwner != NULL && xuiComboBoxIsOpen(pPopupOwner), "quick enum popup open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick enum layout");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pPopupOwner), "quick enum popup survives render");
	(void)xuiComboBoxClose(pPopupOwner);
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "quick enum cancel");

	iRet = xuiTableViewGetCellRect(pTable, 1, 4, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick color rect");
	tWorld = xuiWidgetGetWorldRect(pTable);
	iRet = __xuiTableGridTestClick(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f, tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "quick color click edit");
	pPopupOwner = __xuiTableGridTestFindPopupOwner(pContext, xuiColorPickerGetType(pContext));
	XUI_TEST_CHECK(pPopupOwner != NULL && xuiColorPickerIsOpen(pPopupOwner), "quick color popup open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "quick color layout");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPopupOwner), "quick color popup survives render");
	(void)xuiColorPickerClose(pPopupOwner);
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "quick color cancel");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "final layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "final update");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "final render");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_table_grid_test passed\n");
	return 0;
}
