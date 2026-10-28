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
	int iContextCount;
	int iContextPart;
	int iContextRow;
	int iContextColumn;
	float fContextX;
	float fContextY;
} table_test_data_t;

typedef struct table_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} table_destroy_state_t;

static void __xuiTableTestDestroyWidget(xui_widget pWidget, table_destroy_state_t* pState)
{
	if ( pState == NULL ) return;
	pState->iCount++;
	(void)xuiWidgetDestroy(pWidget);
	if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
}

static void __xuiTableTestDestroyOnSelect(xui_widget pWidget, int iRow, int iColumn, int iMode, void* pUser)
{
	(void)iRow; (void)iColumn; (void)iMode;
	__xuiTableTestDestroyWidget(pWidget, (table_destroy_state_t*)pUser);
}

static void __xuiTableTestDestroyOnSort(xui_widget pWidget, int iColumn, int bDescending, void* pUser)
{
	(void)iColumn; (void)bDescending;
	__xuiTableTestDestroyWidget(pWidget, (table_destroy_state_t*)pUser);
}

static void __xuiTableTestDestroyOnResize(xui_widget pWidget, int iColumn, float fWidth, void* pUser)
{
	(void)iColumn; (void)fWidth;
	__xuiTableTestDestroyWidget(pWidget, (table_destroy_state_t*)pUser);
}

static int __xuiTableTestDestroyOnContext(xui_widget pWidget, int iPart, int iRow, int iColumn, float fX, float fY, void* pUser)
{
	(void)iPart; (void)iRow; (void)iColumn; (void)fX; (void)fY;
	__xuiTableTestDestroyWidget(pWidget, (table_destroy_state_t*)pUser);
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTableTestDestroyOnRender(xui_widget pWidget, int iColumn,
	const xui_table_view_column_t* pColumn, xui_draw_context pDraw,
	xui_rect_t tRect, int iState, void* pUser)
{
	(void)iColumn; (void)pColumn; (void)pDraw; (void)tRect; (void)iState;
	__xuiTableTestDestroyWidget(pWidget, (table_destroy_state_t*)pUser);
	return 1;
}

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

static int __xuiTableTestContext(xui_widget pWidget, int iPart, int iRow, int iColumn, float fX, float fY, void* pUser)
{
	table_test_data_t* pData = (table_test_data_t*)pUser;
	(void)pWidget;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->iContextCount++;
	pData->iContextPart = iPart;
	pData->iContextRow = iRow;
	pData->iContextColumn = iColumn;
	pData->fContextX = fX;
	pData->fContextY = fY;
	return XUI_EVENT_DISPATCH_STOP;
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

static int __xuiTableTestCreateTemporary(xui_context pContext, xui_widget pRoot,
	xui_widget* ppTable)
{
	xui_table_view_column_t arrColumns[2];
	xui_table_view_row_t arrRows[2];
	xui_table_view_desc_t tDesc;
	int iRet;

	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].sTitle = "A";
	arrColumns[0].fWidth = 72.0f;
	arrColumns[1].sTitle = "B";
	arrColumns[1].fWidth = 72.0f;
	memset(arrRows, 0, sizeof(arrRows));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.iColumnCount = 2;
	tDesc.arrRows = arrRows;
	tDesc.iRowCount = 2;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	iRet = xuiTableViewCreate(pContext, ppTable, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, *ppTable);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(*ppTable, (xui_rect_t){390.0f, 20.0f, 190.0f, 110.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	return iRet;
}

static int __xuiTableTestCallbackDestruction(xui_context pContext, xui_widget pRoot,
	xui_surface pTarget)
{
	table_destroy_state_t tDestroy;
	xui_widget pTable = NULL;
	xui_rect_t tWorld;
	xui_rect_t tViewport;
	xui_rect_t tCell;
	xui_event_t tContext;
	float fHeaderY;
	float fResizeX;
	int iFailed = 0;
	int iRet;

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = __xuiTableTestCreateTemporary(pContext, pRoot, &pTable);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "select destroy setup");
	tDestroy.ppWidget = &pTable;
	XUI_TEST_CHECK(xuiTableViewSetSelect(pTable, __xuiTableTestDestroyOnSelect, &tDestroy) == XUI_OK, "select destroy callback");
	tWorld = xuiWidgetGetWorldRect(pTable);
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTable, 0, 0, &tCell) == XUI_OK, "select destroy cell");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tCell.fX + 8.0f,
		tWorld.fY + tCell.fY + 8.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable == NULL && tDestroy.iCount == 1, "select callback may destroy table");
	(void)xuiInputPointerUp(pContext, 0, 0, XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = __xuiTableTestCreateTemporary(pContext, pRoot, &pTable);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "sort destroy setup");
	tDestroy.ppWidget = &pTable;
	XUI_TEST_CHECK(xuiTableViewSetSort(pTable, __xuiTableTestDestroyOnSort, &tDestroy) == XUI_OK, "sort destroy callback");
	tWorld = xuiWidgetGetWorldRect(pTable);
	tViewport = xuiTableViewGetViewportRect(pTable);
	fHeaderY = tWorld.fY + tViewport.fY - 10.0f;
	iRet = __xuiTableTestClick(pContext, tWorld.fX + tViewport.fX + 10.0f, fHeaderY);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable == NULL && tDestroy.iCount == 1, "sort callback may destroy table");

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = __xuiTableTestCreateTemporary(pContext, pRoot, &pTable);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "resize destroy setup");
	tDestroy.ppWidget = &pTable;
	XUI_TEST_CHECK(xuiTableViewSetColumnResize(pTable, __xuiTableTestDestroyOnResize, &tDestroy) == XUI_OK, "resize destroy callback");
	tWorld = xuiWidgetGetWorldRect(pTable);
	tViewport = xuiTableViewGetViewportRect(pTable);
	fHeaderY = tWorld.fY + tViewport.fY - 10.0f;
	fResizeX = tWorld.fX + tViewport.fX + xuiTableViewGetColumnWidth(pTable, 0);
	iRet = xuiInputPointerDown(pContext, fResizeX, fHeaderY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fResizeX + 8.0f, fHeaderY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable == NULL && tDestroy.iCount == 1, "resize callback may destroy table");
	(void)xuiInputPointerUp(pContext, 0, 0, XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = __xuiTableTestCreateTemporary(pContext, pRoot, &pTable);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "context destroy setup");
	tDestroy.ppWidget = &pTable;
	XUI_TEST_CHECK(xuiTableViewSetContextMenu(pTable, __xuiTableTestDestroyOnContext, &tDestroy) == XUI_OK, "context destroy callback");
	tWorld = xuiWidgetGetWorldRect(pTable);
	XUI_TEST_CHECK(xuiTableViewGetCellRect(pTable, 0, 0, &tCell) == XUI_OK, "context destroy cell");
	memset(&tContext, 0, sizeof(tContext));
	tContext.iSize = sizeof(tContext);
	tContext.iType = XUI_EVENT_CONTEXT_MENU;
	tContext.pTarget = pTable;
	tContext.fX = tWorld.fX + tCell.fX + 8.0f;
	tContext.fY = tWorld.fY + tCell.fY + 8.0f;
	iRet = xuiDispatchEvent(pContext, &tContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable == NULL && tDestroy.iCount == 1, "context callback may destroy table");

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = __xuiTableTestCreateTemporary(pContext, pRoot, &pTable);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "renderer destroy setup");
	tDestroy.ppWidget = &pTable;
	XUI_TEST_CHECK(xuiTableViewSetHeaderRenderer(pTable, __xuiTableTestDestroyOnRender, &tDestroy) == XUI_OK, "renderer destroy callback");
	iRet = __xuiTableTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable == NULL && tDestroy.iCount == 1, "renderer callback may destroy table");

cleanup:
	if ( pTable != NULL ) (void)xuiWidgetDestroy(pTable);
	return !iFailed;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTable;
	xui_widget pInvalid;
	xui_surface pTarget;
	xui_table_view_column_t arrColumns[6];
	xui_table_view_column_t tBadColumn;
	xui_table_view_row_t arrRows[72];
	xui_table_view_row_t tBadRow;
	xui_table_view_desc_t tDesc;
	xui_table_view_desc_t tBadDesc;
	xui_accessible_node_t tAccessible;
	table_test_data_t tData;
	xui_rect_t tCell;
	xui_rect_t tMerged;
	xui_rect_t tViewport;
	xui_rect_t tTableWorld;
	xui_event_t tContextEvent;
	float fOffsetX;
	float fOffsetY;
	float fHeaderY;
	float fResizeX;
	float fWidthBefore;
	float fMetricColumn;
	float fMetricRow;
	float fMetricHeader;
	union { uint32_t iBits; float fValue; } tNaN;
	int iRow;
	int iColumn;
	int iActiveRow;
	int iActiveColumn;
	int iDescending;
	int iChangeBefore;
	int iSortBefore;
	int iSelectBefore;
	int iFailed;
	int iRet;
	int i;
	int j;

	pContext = NULL;
	pRoot = NULL;
	pTable = NULL;
	pInvalid = NULL;
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

	memset(&tBadDesc, 0, sizeof(tBadDesc));
	tBadDesc.iSize = sizeof(tBadDesc);
	tBadDesc.iColumnCount = 1;
	pInvalid = (xui_widget)1;
	iRet = xuiTableViewCreate(pContext, &pInvalid, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid descriptor clears output");

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
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	iRet = xuiTableViewCreate(pContext, &pTable, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTable != NULL, "tableview create");
	XUI_TEST_CHECK(xuiTableViewGetScrollbarMode(pTable) == XUI_SCROLLBAR_MODE_FULL,
	               "descriptor scrollbar mode honored");
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
	iRet = xuiTableViewSetContextMenu(pTable, __xuiTableTestContext, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");
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

	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pTable) == tData.iRows * 6 + 1 &&
		xuiWidgetGetAccessibleNode(pTable, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_TABLE &&
		tAccessible.iRowCount == tData.iRows && tAccessible.iColumnCount == 6,
		"table accessibility root and dimensions");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTable, 1, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CELL &&
		strcmp(tAccessible.sName, "R00 C0") == 0 &&
		tAccessible.iRow == 0 && tAccessible.iColumn == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTABLE) != 0,
		"table accessibility cell metadata");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTable, 3, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_CHECKED) != 0,
		"table accessibility bool state");
	iSelectBefore = tData.iSelectCallbackCount;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pTable, 2u,
		XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK &&
		xuiTableViewGetSelectedCell(pTable, &iRow, &iColumn) == XUI_OK &&
		iRow == 0 && iColumn == 0 && tData.iSelectCallbackCount == iSelectBefore + 1,
		"table accessibility selection action");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTable, 25, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0 &&
		tAccessible.iActions == 0 &&
		xuiWidgetPerformAccessibleAction(pTable, 26u,
			XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_INVALID_STATE,
		"disabled table row rejects accessibility action");
	XUI_TEST_CHECK(xuiTableViewSetSelectedCell(pTable, -1, -1) == XUI_OK,
		"reset accessibility selection");

	tNaN.iBits = 0x7fc00000u;
	tBadColumn = arrColumns[0];
	tBadColumn.fWidth = tNaN.fValue;
	fWidthBefore = xuiTableViewGetColumnWidth(pTable, 0);
	iChangeBefore = xuiTableViewGetChangeCount(pTable);
	iRet = xuiTableViewSetColumns(pTable, &tBadColumn, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiTableViewGetColumnCount(pTable) == 6 &&
		xuiTableViewGetColumnWidth(pTable, 0) == fWidthBefore &&
		xuiTableViewGetChangeCount(pTable) == iChangeBefore,
		"invalid column update is transactional");
	memset(&tBadRow, 0, sizeof(tBadRow));
	tBadRow.fHeight = tNaN.fValue;
	iRet = xuiTableViewSetRows(pTable, &tBadRow, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTableViewGetRowCount(pTable) == tData.iRows,
		"invalid row update is transactional");
	XUI_TEST_CHECK(xuiTableViewGetDefaultMetrics(pTable, &fMetricColumn, &fMetricRow, &fMetricHeader) == XUI_OK,
		"read base metrics");
	iRet = xuiTableViewSetDefaultMetrics(pTable, tNaN.fValue, fMetricRow, fMetricHeader);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid metrics rejected");
	iRet = xuiTableViewSetDefaultMetrics(pTable, fMetricColumn, fMetricRow, XUI_LAYOUT_UNBOUNDED);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiTableViewGetDefaultMetrics(pTable, NULL, NULL, &fMetricHeader) == XUI_OK &&
		fMetricHeader == 25.0f, "header plus border extent rejected transactionally");
	iChangeBefore = xuiTableViewGetChangeCount(pTable);
	iRet = xuiTableViewSetColumnWidth(pTable, 0, fWidthBefore);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetChangeCount(pTable) == iChangeBefore,
		"idempotent column width update");

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
	iSelectBefore = tData.iSelectCallbackCount;
	iRet = __xuiTableTestClick(pContext, tTableWorld.fX + tCell.fX + 12.0f, tTableWorld.fY + tCell.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "cell click");
	iRet = xuiTableViewGetSelectedCell(pTable, &iRow, &iColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iRow == 2 && iColumn == 1 &&
		tData.iSelectCallbackCount == iSelectBefore + 1, "cell selected");
	iRet = xuiTableViewGetActiveCell(pTable, &iActiveRow, &iActiveColumn);
	XUI_TEST_CHECK(iRet == XUI_OK && iActiveRow == -1 && iActiveColumn == -1,
		"cell press state clears on pointer up");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pTable, "click focus");
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pTable;
	tContextEvent.fX = tTableWorld.fX + tCell.fX + 12.0f;
	tContextEvent.fY = tTableWorld.fY + tCell.fY + 10.0f;
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCount == 1 &&
		tData.iContextPart == XUI_CONTEXT_TARGET_ITEM &&
		tData.iContextRow == 2 && tData.iContextColumn == 1,
		"pointer context semantic target");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_CONTEXT_MENU, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCount == 2 &&
		tData.iContextRow == 2 && tData.iContextColumn == 1 &&
		tData.fContextX >= tTableWorld.fX && tData.fContextY >= tTableWorld.fY,
		"keyboard context selected cell anchor");
	iRet = xuiInputPointerMove(pContext, tTableWorld.fX + tCell.fX + 15.0f, tTableWorld.fY + tCell.fY + 11.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iHoverCallbackCount > 0, "hover callback fires");

	tViewport = xuiTableViewGetViewportRect(pTable);
	fHeaderY = tTableWorld.fY + tViewport.fY - 12.0f;
	iSortBefore = xuiTableViewGetSortCount(pTable);
	iRet = xuiInputPointerDown(pContext, tTableWorld.fX + tViewport.fX + 12.0f,
		fHeaderY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSortCount(pTable) == iSortBefore,
		"header press does not sort before release");
	iRet = xuiInputPointerUp(pContext, tTableWorld.fX + tViewport.fX + 12.0f,
		fHeaderY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "header release");
	XUI_TEST_CHECK(xuiTableViewGetSortColumn(pTable, &iDescending) == 0 && iDescending == 0 && tData.iSortCallbackCount == 1, "sort column");

	fResizeX = tTableWorld.fX + tViewport.fX + xuiTableViewGetColumnWidth(pTable, 0);
	iRet = xuiInputPointerDown(pContext, fResizeX, fHeaderY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fResizeX + 18.0f, fHeaderY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fResizeX + 18.0f, fHeaderY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetColumnWidth(pTable, 0) > 100.0f && tData.iResizeCallbackCount > 0, "column resize");

	fResizeX = tTableWorld.fX + tViewport.fX + xuiTableViewGetColumnWidth(pTable, 0);
	fWidthBefore = xuiTableViewGetColumnWidth(pTable, 0);
	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)fResizeX, (int)fHeaderY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pTable,
		"resize captures exact touch pointer");
	iRet = xuiInputPointerMoveEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(fResizeX + 30.0f), (int)fHeaderY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(fResizeX + 30.0f), (int)fHeaderY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetColumnWidth(pTable, 0) == fWidthBefore &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pTable,
		"unrelated touch cannot resize or release capture");
	iRet = xuiInputPointerMoveEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(fResizeX + 12.0f), (int)fHeaderY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(fResizeX + 12.0f), (int)fHeaderY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetColumnWidth(pTable, 0) > fWidthBefore &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch completes resize");

	iSortBefore = xuiTableViewGetSortCount(pTable);
	iRet = xuiInputPointerDownEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)(tTableWorld.fX + tViewport.fX + 12.0f), (int)fHeaderY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH) == pTable,
		"header press captures exact touch pointer");
	iRet = xuiTableViewSetColumns(pTable, arrColumns, 6);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"column replacement cancels active header press");
	iRet = xuiInputPointerUpEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)(tTableWorld.fX + tViewport.fX + 12.0f), (int)fHeaderY,
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSortCount(pTable) == iSortBefore,
		"cancelled header press cannot sort on release");

	iRet = xuiTableViewSetSelectedRow(pTable, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectionMode(pTable) == XUI_TABLE_VIEW_SELECTION_ROW, "row selection mode");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectedRow(pTable) == 3, "keyboard down row");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectedRow(pTable) == 5, "keyboard skips disabled row");

	iSelectBefore = xuiTableViewGetSelectCount(pTable);
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 0) == XUI_OK, "disable table ancestor");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTable, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0 &&
		tAccessible.iActions == 0 &&
		xuiWidgetPerformAccessibleAction(pTable, 1u,
			XUI_ACCESSIBLE_ACTION_FOCUS, NULL) == XUI_ERROR_INVALID_ARGUMENT,
		"disabled table rejects accessibility actions");
	iRet = xuiTableViewGetCellRect(pTable, 6, 1, &tCell);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled ancestor target cell");
	iRet = __xuiTableTestClick(pContext, tTableWorld.fX + tCell.fX + 8.0f,
		tTableWorld.fY + tCell.fY + 8.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetSelectCount(pTable) == iSelectBefore &&
		xuiTableViewGetSelectedRow(pTable) == 5, "disabled ancestor blocks table interaction");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 1) == XUI_OK, "restore table ancestor");

	iRet = xuiTableViewEnsureCellVisible(pTable, 63, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	iRet = xuiTableViewGetOffset(pTable, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX > 0.0f && fOffsetY > 0.0f, "ensure scrolls");
	iRet = __xuiTableTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTableViewGetFirstVisible(pTable) > 0 && xuiTableViewGetPaintVisibleCount(pTable) > 0, "visible counters");
	XUI_TEST_CHECK(__xuiTableTestCallbackDestruction(pContext, pRoot, pTarget),
		"table callbacks tolerate widget destruction");

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
