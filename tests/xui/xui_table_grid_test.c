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
	int iContextCount;
	int iContextPart;
	int iContextRow;
	int iContextColumn;
	int bReenterEnd;
	int iReenterCount;
	int iReenterResult;
	int bFloatCell;
	int bInvalidConfig;
	int bReenterConfig;
	int iConfigReenterCount;
	int iConfigReenterResult;
	char sLastValue[96];
} table_grid_test_data_t;

typedef struct table_grid_adapter_test_data_t {
	int iCount;
	int iCountCalls;
} table_grid_adapter_test_data_t;

typedef struct table_grid_lifecycle_test_data_t {
	char sText[64];
	int iType;
	int iCallbackCount;
} table_grid_lifecycle_test_data_t;

static int __xuiTableGridTestContext(xui_widget pWidget, int iPart, int iRow, int iColumn,
	float fX, float fY, void* pUser)
{
	table_grid_test_data_t* pData = (table_grid_test_data_t*)pUser;
	(void)pWidget;
	(void)fX;
	(void)fY;
	if ( pData != NULL ) {
		pData->iContextCount++;
		pData->iContextPart = iPart;
		pData->iContextRow = iRow;
		pData->iContextColumn = iColumn;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

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
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pCell->sText = pData->arrText[iRow][iColumn];
	switch ( iColumn ) {
	case 1:
		pCell->iType = (pData->bFloatCell && iRow == 4) ?
			XUI_TABLE_CELL_TYPE_FLOAT : XUI_TABLE_CELL_TYPE_INT;
		break;
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
	return XUI_OK;
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
	if ( pData->bReenterEnd ) {
		pData->bReenterEnd = 0;
		pData->iReenterCount++;
		pData->iReenterResult = xuiTableGridEndEdit(pWidget, 1);
	}
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
		if ( pData->bReenterConfig ) {
			pData->bReenterConfig = 0;
			pData->iConfigReenterCount++;
			pData->iConfigReenterResult = xuiTableGridBeginEdit(pWidget, 3, 0);
		}
	}
	if ( pConfig == NULL ) {
		return 0;
	}
	if ( pData != NULL && pData->bInvalidConfig ) {
		pConfig->fMin = 10.0f;
		pConfig->fMax = 1.0f;
		return 1;
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

static int __xuiTableGridAdapterCount(xui_widget pWidget, void* pUser)
{
	table_grid_adapter_test_data_t* pData = (table_grid_adapter_test_data_t*)pUser;
	(void)pWidget;
	if ( pData == NULL ) return 0;
	pData->iCountCalls++;
	return pData->iCount;
}

static int __xuiTableGridAdapterCell(xui_widget pWidget, int iRow, int iColumn,
	xui_table_view_cell_t* pCell, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( pCell == NULL || iRow < 0 || iColumn != 0 ) return 0;
	pCell->sText = "adapter";
	pCell->iType = XUI_TABLE_CELL_TYPE_TEXT;
	return 1;
}

static int __xuiTableGridLifecycleCell(xui_widget pWidget, int iRow, int iColumn,
	xui_table_view_cell_t* pCell, void* pUser)
{
	table_grid_lifecycle_test_data_t* pData = (table_grid_lifecycle_test_data_t*)pUser;
	(void)pWidget;
	if ( pData == NULL || pCell == NULL || iRow != 0 || iColumn != 0 ) return 0;
	pCell->sText = pData->sText;
	pCell->iType = pData->iType;
	return 1;
}

static int __xuiTableGridDestroyValidate(xui_widget pWidget, int iRow, int iColumn,
	const char* sValue, int iType, void* pUser)
{
	table_grid_lifecycle_test_data_t* pData = (table_grid_lifecycle_test_data_t*)pUser;
	(void)iRow;
	(void)iColumn;
	(void)sValue;
	(void)iType;
	if ( pData != NULL ) pData->iCallbackCount++;
	xuiWidgetDestroy(pWidget);
	return 1;
}

static void __xuiTableGridDestroyChange(xui_widget pWidget, int iRow, int iColumn,
	const char* sValue, int iType, void* pUser)
{
	table_grid_lifecycle_test_data_t* pData = (table_grid_lifecycle_test_data_t*)pUser;
	(void)iRow;
	(void)iColumn;
	(void)sValue;
	(void)iType;
	if ( pData != NULL ) pData->iCallbackCount++;
	xuiWidgetDestroy(pWidget);
}

static int __xuiTableGridDestroyEditor(xui_widget pWidget, int iRow, int iColumn,
	const xui_table_view_cell_t* pCell, xui_rect_t tWorldRect, void* pUser)
{
	table_grid_lifecycle_test_data_t* pData = (table_grid_lifecycle_test_data_t*)pUser;
	(void)iRow;
	(void)iColumn;
	(void)pCell;
	(void)tWorldRect;
	if ( pData != NULL ) pData->iCallbackCount++;
	xuiWidgetDestroy(pWidget);
	return 1;
}

static int __xuiTableGridCreateLifecycleGrid(xui_context pContext, xui_widget* ppGrid,
	xui_font pFont, table_grid_lifecycle_test_data_t* pData,
	xui_table_grid_validate_proc onValidate, xui_table_grid_change_proc onChange,
	xui_table_grid_editor_proc onEditor)
{
	xui_table_view_column_t tColumn;
	xui_table_view_row_t tRow;
	xui_table_grid_desc_t tDesc;

	memset(&tColumn, 0, sizeof(tColumn));
	tColumn.sTitle = "Value";
	tColumn.fWidth = 120.0f;
	tColumn.iType = pData->iType;
	memset(&tRow, 0, sizeof(tRow));
	tRow.fHeight = 28.0f;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = &tColumn;
	tDesc.arrRows = &tRow;
	tDesc.iColumnCount = 1;
	tDesc.iRowCount = 1;
	tDesc.onCell = __xuiTableGridLifecycleCell;
	tDesc.pAdapterUser = pData;
	tDesc.onValidate = onValidate;
	tDesc.pValidateUser = pData;
	tDesc.onChange = onChange;
	tDesc.pChangeUser = pData;
	tDesc.onEditor = onEditor;
	tDesc.pEditorUser = pData;
	tDesc.pFont = pFont;
	tDesc.fDefaultRowHeight = 28.0f;
	tDesc.fHeaderHeight = 28.0f;
	return xuiTableGridCreate(pContext, ppGrid, &tDesc);
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

static int __xuiTableGridTestContextClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
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

static xui_widget __xuiTableGridTestFindChildType(xui_widget pGrid, xui_widget_type pType)
{
	xui_widget pChild;

	if ( pGrid == NULL || pType == NULL ) return NULL;
	for ( pChild = xuiWidgetGetFirstChild(pGrid); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		if ( xuiWidgetIsType(pChild, pType) ) return pChild;
	}
	return NULL;
}

static xui_widget __xuiTableGridTestFindVisibleEditor(xui_widget pGrid, xui_widget pTable)
{
	xui_widget pChild;

	if ( pGrid == NULL ) {
		return NULL;
	}
	for ( pChild = xuiWidgetGetFirstChild(pGrid); pChild != NULL; pChild = xuiWidgetGetNextSibling(pChild) ) {
		if ( pChild != pTable && xuiWidgetGetVisible(pChild) ) {
			return pChild;
		}
	}
	return NULL;
}

static int __xuiTableGridTestEditorMatchesCell(xui_widget pGrid, xui_widget pTable, int iRow, int iColumn)
{
	xui_widget pEditor;
	xui_rect_t tCell;
	xui_rect_t tTable;
	xui_rect_t tGridWorld;
	xui_rect_t tEditorWorld;

	if ( (pGrid == NULL) || (pTable == NULL) ||
	     (xuiTableViewGetCellRect(pTable, iRow, iColumn, &tCell) != XUI_OK) ) {
		return 0;
	}
	pEditor = __xuiTableGridTestFindVisibleEditor(pGrid, pTable);
	if ( pEditor == NULL ) {
		return 0;
	}
	tTable = xuiWidgetGetRect(pTable);
	tGridWorld = xuiWidgetGetWorldRect(pGrid);
	tEditorWorld = xuiWidgetGetWorldRect(pEditor);
	return tEditorWorld.fX == tGridWorld.fX + tTable.fX + tCell.fX &&
	       tEditorWorld.fY == tGridWorld.fY + tTable.fY + tCell.fY &&
	       tEditorWorld.fW == tCell.fW &&
	       tEditorWorld.fH == tCell.fH;
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
	xui_widget pTempGrid;
	xui_widget pTempTable;
	xui_widget pEditor;
	xui_surface pTarget;
	xui_font pFont;
	xui_table_view_column_t arrColumns[GRID_COLS];
	xui_table_view_row_t arrRows[GRID_ROWS];
	xui_table_grid_desc_t tDesc;
	xui_table_grid_desc_t tTempDesc;
	xui_table_view_column_t tTempColumn;
	xui_table_view_column_t tInvalidColumn;
	xui_table_view_row_t tInvalidRow;
	xui_table_view_colors_t tPalette;
	table_grid_test_data_t tData;
	table_grid_adapter_test_data_t tOldAdapter;
	table_grid_adapter_test_data_t tBadAdapter;
	table_grid_lifecycle_test_data_t tLifecycle;
	xui_rect_t tCell;
	xui_rect_t tWorld;
	xui_accessible_node_t tAccessible;
	uint32_t iBackground;
	uint32_t iText;
	uint32_t iBorder;
	uint32_t iFocus;
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
	pTempGrid = NULL;
	pTempTable = NULL;
	pEditor = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	memset(&tData, 0, sizeof(tData));
	tData.iReenterResult = -1;
	tData.iConfigReenterResult = -1;
	memset(&tOldAdapter, 0, sizeof(tOldAdapter));
	memset(&tBadAdapter, 0, sizeof(tBadAdapter));
	memset(&tLifecycle, 0, sizeof(tLifecycle));
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
	tTempDesc = tDesc;
	tTempDesc.iEditMode = 99;
	pTempGrid = pRoot;
	iRet = xuiTableGridCreate(pContext, &pTempGrid, &tTempDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pTempGrid == NULL,
		"invalid descriptor clears output");
	iRet = xuiTableGridCreate(pContext, &pGrid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGrid != NULL, "tablegrid create");
	xuiWidgetSetRect(pGrid, (xui_rect_t){18.0f, 18.0f, 720.0f, 262.0f});
	iRet = xuiWidgetAddChild(pRoot, pGrid);
	XUI_TEST_CHECK(iRet == XUI_OK, "add tablegrid");
	pTable = xuiTableGridGetTableView(pGrid);
	XUI_TEST_CHECK(pTable != NULL, "inner table");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pGrid, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP, "tablegrid exposes editor host role");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTable, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_TABLE, "inner table owns table semantics");
	iRet = xuiTableGridSetContextMenu(pGrid, __xuiTableGridTestContext, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");

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
	XUI_TEST_CHECK(xuiTableViewGetColors(pTable, &tPalette) == XUI_OK, "tablegrid palette read");
	tPalette.iRowColor = XUI_COLOR_RGBA(232, 241, 250, 255);
	tPalette.iAltRowColor = XUI_COLOR_RGBA(222, 235, 247, 255);
	tPalette.iHoverColor = XUI_COLOR_RGBA(207, 227, 244, 255);
	tPalette.iDisabledColor = XUI_COLOR_RGBA(210, 214, 220, 255);
	tPalette.iGridColor = XUI_COLOR_RGBA(102, 124, 148, 255);
	tPalette.iTextColor = XUI_COLOR_RGBA(24, 42, 61, 255);
	tPalette.iDisabledTextColor = XUI_COLOR_RGBA(116, 126, 137, 255);
	tPalette.iFocusRingColor = XUI_COLOR_RGBA(42, 126, 224, 255);
	XUI_TEST_CHECK(xuiTableGridSetColorStyle(pGrid, &tPalette) == XUI_OK, "tablegrid palette set");
	pEditor = __xuiTableGridTestFindChildType(pGrid, xuiInputGetType(pContext));
	XUI_TEST_CHECK(pEditor != NULL && xuiInputGetColors(pEditor, &iBackground, &iText, &iBorder, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor, "input editor inherits palette");
	pEditor = __xuiTableGridTestFindChildType(pGrid, xuiNumericInputGetType(pContext));
	XUI_TEST_CHECK(pEditor != NULL && xuiNumericInputGetColors(pEditor, &iBackground, &iText, &iBorder, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor, "numeric editor inherits palette");
	pEditor = __xuiTableGridTestFindChildType(pGrid, xuiComboBoxGetType(pContext));
	XUI_TEST_CHECK(pEditor != NULL && xuiComboBoxGetColors(pEditor, &iText, NULL, &iBackground, NULL, NULL, NULL) == XUI_OK &&
		xuiComboBoxGetBorderColors(pEditor, &iBorder, NULL, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor, "enum editor inherits palette");
	pEditor = __xuiTableGridTestFindChildType(pGrid, xuiColorPickerGetType(pContext));
	XUI_TEST_CHECK(pEditor != NULL && xuiColorPickerGetColors(pEditor, &iText, NULL, &iBackground, NULL, NULL, NULL) == XUI_OK &&
		xuiColorPickerGetBorderColors(pEditor, &iBorder, NULL, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor, "color editor inherits palette");
	pEditor = __xuiTableGridTestFindChildType(pGrid, xuiDatePickerGetType(pContext));
	XUI_TEST_CHECK(pEditor != NULL && xuiDatePickerGetColors(pEditor, &iText, NULL, &iBackground, NULL, NULL, NULL) == XUI_OK &&
		xuiDatePickerGetBorderColors(pEditor, &iBorder, NULL, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor, "date editor inherits palette");
	tCell = xuiWidgetGetRect(pTable);
	XUI_TEST_CHECK(tCell.fX == 0.0f && tCell.fY == 0.0f && tCell.fW == 720.0f && tCell.fH == 262.0f, "inner table fill");
	tCell = xuiTableViewGetViewportRect(pTable);
	XUI_TEST_CHECK(tCell.fW > 4.0f && tCell.fH > 4.0f, "inner table viewport");
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTable, 0, 0, &tCell) == XUI_OK && tCell.fW > 20.0f, "cell rect");
	tWorld = xuiWidgetGetWorldRect(pTable);
	iRet = __xuiTableGridTestContextClick(pContext,
		tWorld.fX + tCell.fX + tCell.fW * 0.5f,
		tWorld.fY + tCell.fY + tCell.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCount == 1,
		"context forwarded from table");
	XUI_TEST_CHECK(tData.iContextPart == XUI_CONTEXT_TARGET_ITEM &&
		tData.iContextRow == 0 && tData.iContextColumn == 0,
		"context cell semantics");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "text begin");
	iRow = -1;
	iColumn = -1;
	XUI_TEST_CHECK(xuiTableViewGetSelectedCell(pTable, &iRow, &iColumn) == XUI_OK &&
		iRow == 0 && iColumn == 0, "programmatic edit synchronizes current cell");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 0, 0), "text editor exact cell geometry");
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
	XUI_TEST_CHECK(__xuiTableGridTestEditorMatchesCell(pGrid, pTable, 0, 1), "numeric editor exact cell geometry");
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

	tData.bFloatCell = 1;
	snprintf(tData.arrText[4][1], sizeof(tData.arrText[4][1]), "1.25");
	iRet = xuiTableGridBeginEdit(pGrid, 4, 1);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "float begin with inferred precision");
	XUI_TEST_CHECK(__xuiTableGridTestCheckNumericEditor(pContext, pGrid, "1.25"),
		"default float editor preserves fractional precision");
	XUI_TEST_CHECK(xuiTableGridEndEdit(pGrid, 0) != 0, "float precision edit cancel");
	tData.bFloatCell = 0;

	iRet = xuiTableGridBeginEdit(pGrid, 0, 2);
	XUI_TEST_CHECK(iRet != 0 && strcmp(tData.arrText[0][2], "false") == 0, "bool toggle");
	XUI_TEST_CHECK(!xuiTableGridIsEditing(pGrid), "bool does not stay editing");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 3);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "enum begin");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 0, 3), "enum editor exact cell geometry");
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && strcmp(tData.arrText[0][3], "Done") == 0, "enum commit");
	XUI_TEST_CHECK(tData.iConfigCount >= 2, "editor config used");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 4);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "color begin");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 0, 4), "color editor exact cell geometry");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "color cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 5);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "date begin");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 0, 5), "date editor exact cell geometry");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "date cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 0, 6);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "textarea begin");
	pEditor = xuiGetFocusWidget(pContext);
	XUI_TEST_CHECK(pEditor != NULL && xuiWidgetIsType(pEditor, xuiTextEditGetType(pContext)) &&
		xuiTextEditGetMaxLength(pEditor) == XUI_TABLE_GRID_VALUE_CAPACITY - 1,
		"textarea uses bounded value capacity");
	XUI_TEST_CHECK(xuiTextEditGetColors(pEditor, &iBackground, &iText, &iBorder, &iFocus) == XUI_OK &&
		iBackground == tPalette.iRowColor && iText == tPalette.iTextColor &&
		iBorder == tPalette.iGridColor && iFocus == tPalette.iFocusRingColor,
		"textarea editor inherits palette");
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridGetCancelCount(pGrid) == 5, "textarea cancel");

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
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 3), "quick enum layout geometry");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 3), "quick enum update geometry");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 3), "quick enum next frame geometry");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pPopupOwner) &&
	               __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 3), "quick enum popup survives render");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               (xuiWidgetGetDirtyFlags(pPopupOwner) & (XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)) == 0 &&
	               __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 3), "quick enum stable frame geometry");
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
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 4), "quick color layout geometry");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 4), "quick color update geometry");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 4), "quick color next frame geometry");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPopupOwner) &&
	               __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 4), "quick color popup survives render");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               (xuiWidgetGetDirtyFlags(pPopupOwner) & (XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER)) == 0 &&
	               __xuiTableGridTestEditorMatchesCell(pGrid, pTable, 1, 4), "quick color stable frame geometry");
	(void)xuiColorPickerClose(pPopupOwner);
	iRet = xuiTableGridEndEdit(pGrid, 0);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "quick color cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "row replacement edit begin");
	memset(&tInvalidRow, 0, sizeof(tInvalidRow));
	tInvalidRow.fHeight = -1.0f;
	iRet = xuiTableGridSetRows(pGrid, &tInvalidRow, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTableGridIsEditing(pGrid),
		"invalid row replacement preserves active edit");
	iRet = xuiTableGridSetRows(pGrid, arrRows, GRID_ROWS);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiTableGridIsEditing(pGrid), "row replacement cancels edit");
	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "column replacement edit begin");
	memset(&tInvalidColumn, 0, sizeof(tInvalidColumn));
	tInvalidColumn.fWidth = -1.0f;
	iRet = xuiTableGridSetColumns(pGrid, &tInvalidColumn, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTableGridIsEditing(pGrid),
		"invalid column replacement preserves active edit");
	iRet = xuiTableGridSetColumns(pGrid, arrColumns, GRID_COLS);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiTableGridIsEditing(pGrid), "column replacement cancels edit");

	XUI_TEST_CHECK(xuiTableViewSetSelectedCell(pTable, 3, 0) == XUI_OK &&
		xuiSetFocusWidget(pContext, pTable) == XUI_OK, "F2 edit target focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_F2, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "F2 begins selected cell edit");
	iRow = -1;
	iColumn = -1;
	XUI_TEST_CHECK(xuiTableGridGetEditingCell(pGrid, &iRow, &iColumn) == XUI_OK &&
		iRow == 3 && iColumn == 0, "F2 edits selected cell");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_F2, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridEndEdit(pGrid, 0) != 0, "F2 edit cancel");

	tData.bInvalidConfig = 1;
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pGrid, 2, 0) == 0 && !xuiTableGridIsEditing(pGrid),
		"invalid editor config rolls back edit");
	tData.bInvalidConfig = 0;
	tData.bReenterConfig = 1;
	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "outer edit survives config reentry");
	XUI_TEST_CHECK(tData.iConfigReenterCount == 1 && tData.iConfigReenterResult == 0,
		"nested begin rejected during editor setup");
	XUI_TEST_CHECK(xuiTableGridEndEdit(pGrid, 0) != 0, "config reentry edit cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "disable edit begin");
	iRet = xuiWidgetSetEnabled(pGrid, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiTableGridIsEditing(pGrid), "disable cancels edit");
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pGrid, 2, 0) == 0, "disabled grid blocks edit");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pGrid, 1) == XUI_OK, "reenable grid");
	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "hide edit begin");
	iRet = xuiWidgetSetVisible(pGrid, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiTableGridIsEditing(pGrid), "hide cancels edit");
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pGrid, 2, 0) == 0, "hidden grid blocks edit");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pGrid, 1) == XUI_OK, "show grid");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "relayout shown grid");

	iRet = xuiTableGridSetEditMode(pGrid, XUI_TABLE_GRID_EDIT_IMMEDIATE);
	XUI_TEST_CHECK(iRet == XUI_OK, "immediate mode set");
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTable, 2, 0, &tCell) == XUI_OK, "immediate cell rect");
	tWorld = xuiWidgetGetWorldRect(pTable);
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f,
		tWorld.fY + tCell.fY + tCell.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableGridIsEditing(pGrid), "immediate edit starts on pointer down");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tCell.fX + tCell.fW * 0.5f,
		tWorld.fY + tCell.fY + tCell.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "immediate pointer release");
	XUI_TEST_CHECK(xuiTableGridEndEdit(pGrid, 0) != 0, "immediate edit cancel");

	iRet = xuiTableGridBeginEdit(pGrid, 2, 0);
	XUI_TEST_CHECK(iRet != 0 && xuiTableGridIsEditing(pGrid), "reentrant commit begin");
	pEditor = __xuiTableGridTestFindVisibleEditor(pGrid, pTable);
	XUI_TEST_CHECK(pEditor != NULL && xuiInputSetText(pEditor, "Reentrant") == XUI_OK,
		"reentrant commit value");
	tData.bReenterEnd = 1;
	iRet = xuiTableGridEndEdit(pGrid, 1);
	XUI_TEST_CHECK(iRet != 0 && !xuiTableGridIsEditing(pGrid), "outer commit survives nested end");
	XUI_TEST_CHECK(tData.iReenterCount == 1 && tData.iReenterResult == 0 &&
		strcmp(tData.arrText[2][0], "Reentrant") == 0, "nested end rejected without recursion");

	memset(&tTempColumn, 0, sizeof(tTempColumn));
	tTempColumn.sTitle = "Adapter";
	tTempColumn.fWidth = 100.0f;
	memset(&tTempDesc, 0, sizeof(tTempDesc));
	tOldAdapter.iCount = 1;
	tBadAdapter.iCount = 4;
	tTempDesc.iSize = sizeof(tTempDesc);
	tTempDesc.arrColumns = &tTempColumn;
	tTempDesc.iColumnCount = 1;
	tTempDesc.onCount = __xuiTableGridAdapterCount;
	tTempDesc.onCell = __xuiTableGridAdapterCell;
	tTempDesc.pAdapterUser = &tOldAdapter;
	tTempDesc.pFont = pFont;
	tTempDesc.fDefaultRowHeight = XUI_LAYOUT_UNBOUNDED * 0.3f;
	iRet = xuiTableGridCreate(pContext, &pTempGrid, &tTempDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "adapter rollback grid create");
	pTempTable = xuiTableGridGetTableView(pTempGrid);
	XUI_TEST_CHECK(pTempTable != NULL && xuiTableViewGetRowCount(pTempTable) == 1,
		"adapter rollback initial provider");
	i = tOldAdapter.iCountCalls;
	iRet = xuiTableGridSetAdapter(pTempGrid, __xuiTableGridAdapterCount,
		__xuiTableGridAdapterCell, NULL, &tBadAdapter);
	XUI_TEST_CHECK(iRet != XUI_OK && tBadAdapter.iCountCalls > 0, "invalid adapter rejected");
	j = tBadAdapter.iCountCalls;
	iRet = xuiTableViewRefresh(pTempTable);
	XUI_TEST_CHECK(iRet == XUI_OK && tOldAdapter.iCountCalls > i && tBadAdapter.iCountCalls == j &&
		xuiTableViewGetRowCount(pTempTable) == 1, "adapter failure restores callback tuple");
	xuiWidgetDestroy(pTempGrid);
	pTempGrid = NULL;
	pTempTable = NULL;

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "final layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "final update");
	iRet = __xuiTableGridTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTarget) > 0, "final render");

	memset(&tLifecycle, 0, sizeof(tLifecycle));
	snprintf(tLifecycle.sText, sizeof(tLifecycle.sText), "before");
	tLifecycle.iType = XUI_TABLE_CELL_TYPE_TEXT;
	iRet = __xuiTableGridCreateLifecycleGrid(pContext, &pTempGrid, pFont, &tLifecycle,
		__xuiTableGridDestroyValidate, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "destroying validate grid create");
	xuiWidgetSetRect(pTempGrid, (xui_rect_t){20.0f, 290.0f, 180.0f, 60.0f});
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pTempGrid) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"destroying validate grid attach");
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pTempGrid, 0, 0) != 0, "destroying validate edit begin");
	pEditor = __xuiTableGridTestFindVisibleEditor(pTempGrid, xuiTableGridGetTableView(pTempGrid));
	XUI_TEST_CHECK(pEditor != NULL && xuiInputSetText(pEditor, "after") == XUI_OK,
		"destroying validate edit value");
	XUI_TEST_CHECK(xuiTableGridEndEdit(pTempGrid, 1) != 0 && tLifecycle.iCallbackCount == 1 &&
		xuiTableGridGetTableView(pTempGrid) == NULL, "validate callback may destroy grid");
	pTempGrid = NULL;

	memset(&tLifecycle, 0, sizeof(tLifecycle));
	snprintf(tLifecycle.sText, sizeof(tLifecycle.sText), "before");
	tLifecycle.iType = XUI_TABLE_CELL_TYPE_TEXT;
	iRet = __xuiTableGridCreateLifecycleGrid(pContext, &pTempGrid, pFont, &tLifecycle,
		NULL, __xuiTableGridDestroyChange, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "destroying change grid create");
	xuiWidgetSetRect(pTempGrid, (xui_rect_t){210.0f, 290.0f, 180.0f, 60.0f});
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pTempGrid) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"destroying change grid attach");
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pTempGrid, 0, 0) != 0, "destroying change edit begin");
	pEditor = __xuiTableGridTestFindVisibleEditor(pTempGrid, xuiTableGridGetTableView(pTempGrid));
	XUI_TEST_CHECK(pEditor != NULL && xuiInputSetText(pEditor, "after") == XUI_OK,
		"destroying change edit value");
	XUI_TEST_CHECK(xuiTableGridEndEdit(pTempGrid, 1) != 0 && tLifecycle.iCallbackCount == 1 &&
		xuiTableGridGetTableView(pTempGrid) == NULL, "change callback may destroy grid");
	pTempGrid = NULL;

	memset(&tLifecycle, 0, sizeof(tLifecycle));
	snprintf(tLifecycle.sText, sizeof(tLifecycle.sText), "...");
	tLifecycle.iType = XUI_TABLE_CELL_TYPE_PICKER;
	iRet = __xuiTableGridCreateLifecycleGrid(pContext, &pTempGrid, pFont, &tLifecycle,
		NULL, NULL, __xuiTableGridDestroyEditor);
	XUI_TEST_CHECK(iRet == XUI_OK && pTempGrid != NULL, "destroying editor grid create");
	xuiWidgetSetRect(pTempGrid, (xui_rect_t){400.0f, 290.0f, 180.0f, 60.0f});
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pTempGrid) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"destroying editor grid attach");
	XUI_TEST_CHECK(xuiTableGridBeginEdit(pTempGrid, 0, 0) != 0 && tLifecycle.iCallbackCount == 1 &&
		xuiTableGridGetTableView(pTempGrid) == NULL, "editor callback may destroy grid");
	pTempGrid = NULL;

cleanup:
	if ( pTempGrid != NULL ) {
		xuiWidgetDestroy(pTempGrid);
	}
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
