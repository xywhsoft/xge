#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_table_view_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct table_test_data_t {
	char arrText[72][6][64];
	int iRows;
	int iSelectCallbackCount;
	int iHoverCallbackCount;
	int iResizeCallbackCount;
	int iSortCallbackCount;
	int iHeaderRenderCount;
	int iCellRenderCount;
} table_test_data_t;

static int __xuiTableTestCount(xui_widget pWidget, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	pData = (table_test_data_t*)pUser;
	return (pData != NULL) ? pData->iRows : 0;
}

static int __xuiTableTestCell(xui_widget pWidget, int iRow, int iColumn, xui_table_view_cell_t* pCell, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	pData = (table_test_data_t*)pUser;
	if ( (pData == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= pData->iRows) || (iColumn >= 6) ) {
		return 0;
	}
	pCell->sText = pData->arrText[iRow][iColumn];
	pCell->sTooltip = (iRow == 2 && iColumn == 1) ? "merged tooltip" : NULL;
	if ( iColumn == 2 ) pCell->iType = XUI_TABLE_CELL_TYPE_BOOL;
	if ( iColumn == 3 ) pCell->iType = XUI_TABLE_CELL_TYPE_COLOR;
	if ( iColumn == 4 ) pCell->iType = XUI_TABLE_CELL_TYPE_PICKER;
	if ( iRow == 5 && iColumn == 1 ) pCell->bDisabled = 1;
	if ( iRow == 6 && iColumn == 2 ) pCell->bInvalid = 1;
	if ( iRow == 7 && iColumn == 3 ) pCell->bDirty = 1;
	return 1;
}

static int __xuiTableTestMerge(xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( (iRow == 2) && (iColumn == 1) ) {
		*pRowSpan = 2;
		*pColSpan = 2;
		return 1;
	}
	return 0;
}

static void __xuiTableTestSelect(xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)iMode;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iSelectCallbackCount++;
}

static void __xuiTableTestHover(xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)iMode;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iHoverCallbackCount++;
}

static void __xuiTableTestResize(xui_widget pWidget, int iColumn, float fWidth, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iColumn;
	(void)fWidth;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iResizeCallbackCount++;
}

static void __xuiTableTestSort(xui_widget pWidget, int iColumn, int bDescending, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iColumn;
	(void)bDescending;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iSortCallbackCount++;
}

static int __xuiTableTestHeaderRender(xui_widget pWidget, int iColumn, const xui_table_view_column_t* pColumn, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iColumn;
	(void)pColumn;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iHeaderRenderCount++;
	return 0;
}

static int __xuiTableTestCellRender(xui_widget pWidget, int iRow, int iColumn, const xui_table_view_cell_t* pCell, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	table_test_data_t* pData;

	(void)pWidget;
	(void)iRow;
	(void)iColumn;
	(void)pCell;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pData = (table_test_data_t*)pUser;
	if ( pData != NULL ) pData->iCellRenderCount++;
	return 0;
}

static int __xuiTableTestRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 620, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTableTestClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTable;
	xui_surface pTarget;
	xui_table_view_column_t arrColumns[6];
	xui_table_view_row_t arrRows[72];
	xui_table_view_desc_t tDesc;
	table_test_data_t tData;
	xui_rect_t tCell;
	xui_rect_t tMerged;
	xui_rect_t tViewport;
	xui_rect_t tTableWorld;
	float fOffsetX;
	float fOffsetY;
	float fHeaderY;
	float fResizeX;
	int iRow;
	int iColumn;
	int iDescending;
	int iFailed;
	int iRet;
	int i;
	int j;

	pContext = NULL;
	pRoot = NULL;
	pTable = NULL;
	pTarget = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	memset(&tData, 0, sizeof(tData));
	tData.iRows = 64;
	for ( i = 0; i < tData.iRows; i++ ) {
		for ( j = 0; j < 6; j++ ) {
			if ( j == 2 ) {
				snprintf(tData.arrText[i][j], sizeof(tData.arrText[i][j]), "%s", (i % 2) ? "false" : "true");
			} else if ( j == 3 ) {
				snprintf(tData.arrText[i][j], sizeof(tData.arrText[i][j]), "#%02X%02X%02X", 60 + i, 120 + j * 10, 180 - j * 8);
			} else {
				snprintf(tData.arrText[i][j], sizeof(tData.arrText[i][j]), "R%02d C%d", i, j);
			}
		}
	}
	snprintf(tData.arrText[2][1], sizeof(tData.arrText[2][1]), "merged 2x2");

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 620.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 620.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 86.0f;
	arrColumns[1].sTitle = "Status";
	arrColumns[1].fWidth = 96.0f;
	arrColumns[2].sTitle = "Flag";
	arrColumns[2].fWidth = 72.0f;
	arrColumns[2].iType = XUI_TABLE_CELL_TYPE_BOOL;
	arrColumns[3].sTitle = "Color";
	arrColumns[3].fWidth = 112.0f;
	arrColumns[3].iType = XUI_TABLE_CELL_TYPE_COLOR;
	arrColumns[4].sTitle = "Action";
	arrColumns[4].fWidth = 105.0f;
	arrColumns[4].iType = XUI_TABLE_CELL_TYPE_PICKER;
	arrColumns[5].sTitle = "Notes";
	arrColumns[5].fWidth = 140.0f;
	for ( i = 0; i < 6; i++ ) {
		arrColumns[i].bVisibleSet = 1;
		arrColumns[i].bVisible = 1;
		arrColumns[i].bResizableSet = 1;
		arrColumns[i].bResizable = 1;
		arrColumns[i].fMinWidth = 44.0f;
	}
	memset(arrRows, 0, sizeof(arrRows));
	arrRows[1].fHeight = 28.0f;
	arrRows[4].bDisabled = 1;
	arrRows[8].bSelected = 1;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.iColumnCount = 6;
	tDesc.arrRows = arrRows;
	tDesc.iRowCount = tData.iRows;
	tDesc.onCount = __xuiTableTestCount;
	tDesc.onCell = __xuiTableTestCell;
	tDesc.pAdapterUser = &tData;
	tDesc.fDefaultRowHeight = 22.0f;
	tDesc.fHeaderHeight = 25.0f;
	tDesc.iSelectionMode = XUI_TABLE_VIEW_SELECTION_CELL;
	iRet = xuiTableViewCreate(pContext, &pTable, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "tableview create");
	iRet = xuiWidgetAddChild(pRoot, pTable);
	XUI_TEST_CHECK(iRet == XUI_OK, "add table");
	xuiWidgetSetRect(pTable, (xui_rect_t){24.0f, 18.0f, 330.0f, 178.0f});
	iRet = xuiTableViewSetSelect(pTable, __xuiTableTestSelect, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiTableViewSetHover(pTable, __xuiTableTestHover, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover callback");
	iRet = xuiTableViewSetColumnResize(pTable, __xuiTableTestResize, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "resize callback");
	iRet = xuiTableViewSetSort(pTable, __xuiTableTestSort, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "sort callback");
	iRet = xuiTableViewSetMergeProvider(pTable, __xuiTableTestMerge, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "merge callback");
	iRet = xuiTableViewSetHeaderRenderer(pTable, __xuiTableTestHeaderRender, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "header renderer");
	iRet = xuiTableViewSetCellRenderer(pTable, __xuiTableTestCellRender, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "cell renderer");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 620, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiTableTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(tData.iHeaderRenderCount > 0 && tData.iCellRenderCount > 0, "renderer callbacks");
	XUI_TEST_CHECK(xuiTableViewGetColumnCount(pTable) == 6, "column count");
	XUI_TEST_CHECK(xuiTableViewGetRowCount(pTable) == tData.iRows, "row count");
	XUI_TEST_CHECK(xuiTableViewGetFrameWidget(pTable) != NULL && xuiTableViewGetViewportWidget(pTable) != NULL, "frame viewport");
	XUI_TEST_CHECK(xuiScrollFrameIsHScrollBarVisible(xuiTableViewGetFrameWidget(pTable)), "h scrollbar visible");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(xuiTableViewGetFrameWidget(pTable)), "v scrollbar visible");

	iRet = xuiTableViewGetCellContentRect(pTable, 2, 2, &tMerged);
	XUI_TEST_CHECK(iRet == XUI_OK && tMerged.fW > arrColumns[1].fWidth && tMerged.fH > arrColumns[2].fWidth * 0.1f, "merged content rect");

	tTableWorld = xuiWidgetGetWorldRect(pTable);
	iRet = xuiTableViewGetCellRect(pTable, 4, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled row rect");
	iRet = __xuiTableTestClick(pContext, tTableWorld.fX + tCell.fX + 12.0f, tTableWorld.fY + tCell.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled row click");
	XUI_TEST_CHECK(xuiTableViewGetSelectedRow(pTable) == -1, "disabled row not selected");

	iRet = xuiTableViewGetCellRect(pTable, 2, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "merged cell rect");
	iRet = __xuiTableTestClick(pContext, tTableWorld.fX + tCell.fX + 12.0f, tTableWorld.fY + tCell.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "cell click");
	iRet = xuiTableViewGetSelectedCell(pTable, &iRow, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow == 2 && iColumn == 1 && tData.iSelectCallbackCount == 1, "cell selected");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pTable, "click focus");
	iRet = xuiInputPointerMove(pContext, tTableWorld.fX + tCell.fX + 15.0f, tTableWorld.fY + tCell.fY + 11.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iHoverCallbackCount > 0, "hover callback fires");

	tViewport = xuiTableViewGetViewportRect(pTable);
	fHeaderY = tTableWorld.fY + tViewport.fY - 12.0f;
	iRet = __xuiTableTestClick(pContext, tTableWorld.fX + tViewport.fX + 12.0f, fHeaderY);
	XUI_TEST_CHECK(iRet == XUI_OK, "header click");
	XUI_TEST_CHECK(xuiTableViewGetSortColumn(pTable, &iDescending) == 0 && iDescending == 0 && tData.iSortCallbackCount == 1, "sort column");

	fResizeX = tTableWorld.fX + tViewport.fX + xuiTableViewGetColumnWidth(pTable, 0);
	iRet = xuiInputPointerDown(pContext, fResizeX, fHeaderY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fResizeX + 18.0f, fHeaderY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fResizeX + 18.0f, fHeaderY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetColumnWidth(pTable, 0) > 100.0f && tData.iResizeCallbackCount > 0, "column resize");

	iRet = xuiTableViewSetSelectedRow(pTable, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectionMode(pTable) == XUI_TABLE_VIEW_SELECTION_ROW, "row selection mode");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectedRow(pTable) == 3, "keyboard down row");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectedRow(pTable) == 5, "keyboard skips disabled row");

	iRet = xuiTableViewEnsureCellVisible(pTable, 63, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	iRet = xuiTableViewGetOffset(pTable, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX > 0.0f && fOffsetY > 0.0f, "ensure scrolls");
	iRet = __xuiTableTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetFirstVisible(pTable) > 0 && xuiTableViewGetPaintVisibleCount(pTable) > 0, "visible counters");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_table_view_test passed\n");
	return 0;
}
