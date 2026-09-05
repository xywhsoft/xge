#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/xui_internal.h"

static int iAllocationFailure = -1;
static int iOutstandingAllocations;
static void* table_malloc(size_t iSize)
{
	void* pNew;
	if ( iAllocationFailure == 0 ) return NULL;
	if ( iAllocationFailure > 0 ) iAllocationFailure--;
	pNew = xrtMalloc(iSize);
	if ( pNew != NULL ) iOutstandingAllocations++;
	return pNew;
}
static void* table_realloc(void* pData, size_t iSize)
{
	void* pNew;
	if ( iAllocationFailure == 0 ) return NULL;
	if ( iAllocationFailure > 0 ) iAllocationFailure--;
	pNew = xrtRealloc(pData, iSize);
	if ( pNew != NULL && pData == NULL ) iOutstandingAllocations++;
	return pNew;
}
static void table_free(void* pData)
{
	if ( pData != NULL ) iOutstandingAllocations--;
	xrtFree(pData);
}

enum { ROW_READ, ROW_PROBE, MERGE_PROBE, INDEX_STEP_COUNT };
static unsigned long long arrSteps[INDEX_STEP_COUNT];
#define XUI_TABLE_VIEW_INDEX_STEP(kind) (++arrSteps[kind])
#define xrtMalloc table_malloc
#define xrtRealloc table_realloc
#define xrtFree table_free
#include "../src/xui_table_view.c"
#undef xrtMalloc
#undef xrtRealloc
#undef xrtFree
#include "xui_test_proxy.h"

#define CHECK(expr) do { if ( !(expr) ) { \
	printf("FAIL line %d: %s\n", __LINE__, #expr); iFailed++; \
} } while ( 0 )

typedef struct table_index_adapter_t {
	int iRows;
	int bMerge;
	int bCellMerge;
	int iPhase;
	int bRefreshOnce;
	unsigned long long iCells;
	unsigned long long iMerges;
} table_index_adapter_t;

static int adapter_spans(table_index_adapter_t* pAdapter, int r, int c, int* pRows, int* pColumns)
{
	if ( pAdapter->bMerge && r % 8 == pAdapter->iPhase && c == 1 ) {
		*pRows = 3;
		*pColumns = 2;
		return 1;
	}
	return 0;
}

static int count_rows(xui_widget pWidget, void* pUser)
{
	return ((table_index_adapter_t*)pUser)->iRows;
}

static int cell_data(xui_widget pWidget, int r, int c, xui_table_view_cell_t* pCell, void* pUser)
{
	table_index_adapter_t* pAdapter = (table_index_adapter_t*)pUser;
	pAdapter->iCells++;
	pCell->sText = "cell";
	if ( pAdapter->bCellMerge ) adapter_spans(pAdapter, r, c, &pCell->iRowSpan, &pCell->iColSpan);
	if ( pAdapter->bRefreshOnce ) {
		pAdapter->bRefreshOnce = 0;
		xuiTableViewRefresh(pWidget);
	}
	return 1;
}

static int merge_data(xui_widget pWidget, int r, int c, int* pRows, int* pColumns, void* pUser)
{
	table_index_adapter_t* pAdapter = (table_index_adapter_t*)pUser;
	pAdapter->iMerges++;
	return adapter_spans(pAdapter, r, c, pRows, pColumns);
}

static int scale_case(int iRows, int bMerge)
{
	xui_context pContext = NULL;
	xui_widget pTable = NULL;
	xui_table_view_desc_t tDesc;
	xui_table_view_column_t arrColumns[6];
	table_index_adapter_t tAdapter;
	xui_table_view_data_t* pData;
	xui_rect_t tRect;
	unsigned long long iCold;
	int iFailed = 0;
	int r, c, i;
	memset(&tDesc, 0, sizeof(tDesc));
	memset(arrColumns, 0, sizeof(arrColumns));
	memset(&tAdapter, 0, sizeof(tAdapter));
	tAdapter.iRows = iRows;
	tAdapter.bMerge = bMerge;
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.iColumnCount = 6;
	tDesc.onCount = count_rows;
	tDesc.onCell = cell_data;
	tDesc.pAdapterUser = &tAdapter;
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
	if ( pTable == NULL ) { xuiDestroy(pContext); return 1; }
	if ( bMerge ) CHECK(xuiTableViewSetMergeProvider(pTable, merge_data, &tAdapter) == XUI_OK);
	pData = __xuiTableViewGetData(pTable);
	CHECK(xuiTableViewGetCellContentRect(pTable, iRows - 2, 5, &tRect) == XUI_OK);
	iCold = tAdapter.iCells;
	CHECK(iCold == (unsigned long long)(iRows - 1) * 6 + 1);
	tAdapter.iCells = tAdapter.iMerges = 0;
	memset(arrSteps, 0, sizeof(arrSteps));
	for ( i = 0; i < 32; i++ ) {
		CHECK(__xuiTableViewMergeOwner(pTable, pData, iRows - 2, 5, &r, &c) == 0);
		CHECK(r == iRows - 2 && c == 5);
		if ( bMerge ) {
			CHECK(__xuiTableViewMergeOwner(pTable, pData, iRows - 7, 2, &r, &c) == 1);
			CHECK(r == iRows - 8 && c == 1);
		}
		CHECK(__xuiTableViewRowAtContentY(pData, (iRows - 2) * 20.0f + 1.0f) == iRows - 2);
		CHECK(__xuiTableViewRowTop(pData, iRows - 2) == (iRows - 2) * 20.0f);
		CHECK(__xuiTableViewContentHeight(pTable, pData) == iRows * 20.0f);
	}
	printf("scale rows=%d merge=%d cold_cells=%llu hot_cells=%llu row_reads=%llu row_probes=%llu merge_probes=%llu\n",
		iRows, bMerge, iCold, tAdapter.iCells, arrSteps[ROW_READ], arrSteps[ROW_PROBE], arrSteps[MERGE_PROBE]);
	CHECK(tAdapter.iCells == 0 && tAdapter.iMerges == 0);
	CHECK(arrSteps[ROW_READ] <= 64);
	CHECK(arrSteps[ROW_PROBE] <= 32 * 24);
	CHECK(arrSteps[MERGE_PROBE] <= 32 * 24);
	CHECK(pData->iMergeCount == (bMerge ? iRows / 8 : 0));
	tAdapter.iCells = 0;
	CHECK(__xuiTableViewMergeOwner(pTable, pData, iRows - 1, 5, &r, &c) == 0);
	CHECK(tAdapter.iCells == 6);
	CHECK(pData->iMergeIndexedRows == iRows);
	xuiWidgetDestroy(pTable);
	xuiDestroy(pContext);
	return iFailed;
}

/* Deliberately retain the pre-fix row-major scan as an independent owner oracle. */
static void reference_owner(xui_widget pTable, xui_table_view_data_t* pData, int row, int column, int* pRow, int* pColumn)
{
	xui_table_view_cell_t tCell;
	int r, c;
	*pRow = row;
	*pColumn = column;
	for ( r = 0; r <= row; r++ ) {
		for ( c = 0; c <= column; c++ ) {
			__xuiTableViewGetCell(pTable, pData, r, c, &tCell);
			if ( (tCell.iRowSpan > 1 || tCell.iColSpan > 1) && row < r + tCell.iRowSpan && column < c + tCell.iColSpan ) {
				*pRow = r;
				*pColumn = c;
				return;
			}
		}
	}
}

static int overlap_cell(xui_widget pWidget, int r, int c, xui_table_view_cell_t* pCell, void* pUser)
{
	unsigned int iValue = (unsigned int)(r * 137 + c * 29 + *(int*)pUser * 83);
	if ( iValue % 5 == 0 ) {
		pCell->iRowSpan = 1 + (int)(iValue % 11);
		pCell->iColSpan = 1 + (int)(iValue % 4);
	}
	if ( r == 30 && c == 4 ) { pCell->iRowSpan = INT_MAX; pCell->iColSpan = INT_MAX; }
	if ( r == 2 && c == 4 ) { pCell->iRowSpan = -2; pCell->iColSpan = 0; }
	return 1;
}

static int override_merge(xui_widget pWidget, int r, int c, int* pRows, int* pColumns, void* pUser)
{
	if ( (r + c) % 3 != 0 ) return 0;
	*pRows = *pColumns = 1;
	return 1;
}

static int overlap_case(void)
{
	xui_context pContext = NULL;
	xui_widget pTable = NULL;
	xui_table_view_desc_t tDesc = {0};
	xui_table_view_column_t arrColumns[6] = {{0}};
	xui_table_view_data_t* pData;
	int iFailed = 0;
	int seed, r, c, rr, cc, er, ec;
	tDesc.iSize = sizeof(tDesc);
	tDesc.iRowCount = 32;
	tDesc.iColumnCount = 6;
	tDesc.arrColumns = arrColumns;
	tDesc.onCell = overlap_cell;
	tDesc.pAdapterUser = &seed;
	seed = 0;
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
	if ( pTable == NULL ) { xuiDestroy(pContext); return 1; }
	pData = __xuiTableViewGetData(pTable);
	for ( seed = 0; seed < 24; seed++ ) {
		CHECK(xuiTableViewSetMergeProvider(pTable, seed % 2 ? override_merge : NULL, NULL) == XUI_OK);
		CHECK(xuiTableViewRefreshAdapter(pTable) == XUI_OK);
		/* Descending queries also check that a complete cache serves earlier rows. */
		for ( r = 31; r >= 0; r-- ) {
			for ( c = 5; c >= 0; c-- ) {
				reference_owner(pTable, pData, r, c, &er, &ec);
				CHECK(__xuiTableViewMergeOwner(pTable, pData, r, c, &rr, &cc) >= 0);
				CHECK(rr == er && cc == ec);
			}
		}
	}
	printf("overlap oracle: 4608 owner comparisons, cell-only/provider overrides, clipping and invalid spans\n");
	xuiWidgetDestroy(pTable);
	xuiDestroy(pContext);
	return iFailed;
}

static int huge_merge(xui_widget pWidget, int r, int c, int* pRows, int* pColumns, void* pUser)
{
	(*(unsigned long long*)pUser)++;
	if ( r != 0 || c != 0 ) return 0;
	*pRows = INT_MAX;
	*pColumns = INT_MAX;
	return 1;
}

static int huge_merge_case(void)
{
	xui_context pContext = NULL;
	xui_widget pTable = NULL;
	xui_table_view_desc_t tDesc = {0};
	xui_table_view_column_t arrColumns[XUI_TABLE_VIEW_COLUMN_CAPACITY] = {{0}};
	table_index_adapter_t tAdapter = {0};
	xui_table_view_data_t* pData;
	xui_rect_t tRect;
	unsigned long long iCalls = 0;
	int iFailed = 0;
	int c, i;
	tAdapter.iRows = 65536;
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.iColumnCount = XUI_TABLE_VIEW_COLUMN_CAPACITY;
	tDesc.onCount = count_rows;
	tDesc.pAdapterUser = &tAdapter;
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
	if ( pTable == NULL ) { xuiDestroy(pContext); return 1; }
	pData = __xuiTableViewGetData(pTable);
	CHECK(xuiTableViewSetMergeProvider(pTable, huge_merge, &iCalls) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, 65535, 63, &tRect) == XUI_OK);
	CHECK(iCalls == 65536ULL * 64 + 1);
	CHECK(tRect.fX == 0 && tRect.fY == 0 && tRect.fW == 5120 && tRect.fH == 1310720);
	CHECK(pData->iMergeCount == 1);
	for ( c = 0; c < 64; c++ ) CHECK(pData->arrMergeColumns[c].iCount == 1);
	iCalls = 0;
	memset(arrSteps, 0, sizeof(arrSteps));
	for ( i = 0; i < 32; i++ ) CHECK(xuiTableViewGetCellContentRect(pTable, 65535, 63, &tRect) == XUI_OK);
	CHECK(iCalls == 32 && arrSteps[ROW_READ] == 0 && arrSteps[MERGE_PROBE] == 32);
	printf("huge merge: 65536x64 cells stored as 1 owner + 64 bands; 32 hot rect queries = 32 provider calls, 32 probes\n");
	xuiWidgetDestroy(pTable);
	xuiDestroy(pContext);
	return iFailed;
}

static int row_index_case(void)
{
	xui_context pContext = NULL;
	xui_widget pTable = NULL;
	xui_table_view_desc_t tDesc = {0};
	xui_table_view_column_t tColumn = {0};
	table_index_adapter_t tAdapter = {0};
	xui_table_view_data_t* pData;
	xui_table_view_row_t* arrRows;
	xui_table_view_row_t* pOriginalRows;
	double fTop, fHeight;
	float fDefault;
	int iFailed = 0;
	int i, pass;
	arrRows = (xui_table_view_row_t*)calloc(65536, sizeof(*arrRows));
	CHECK(arrRows != NULL);
	if ( arrRows == NULL ) return 1;
	for ( i = 0; i < 65536; i++ ) arrRows[i].fHeight = (i % 3 == 0) ? 0.0f : 15.5f + i % 7;
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = &tColumn;
	tDesc.iColumnCount = 1;
	tDesc.onCount = count_rows;
	tDesc.pAdapterUser = &tAdapter;
	tAdapter.iRows = 65536;
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
	if ( pTable == NULL ) { free(arrRows); xuiDestroy(pContext); return 1; }
	pData = __xuiTableViewGetData(pTable);
	CHECK(xuiTableViewSetRows(pTable, arrRows, 65536) == XUI_OK);
	for ( pass = 0; pass < 2; pass++ ) {
		fDefault = pass ? 27.5f : 20.0f;
		memset(arrSteps, 0, sizeof(arrSteps));
		CHECK(xuiTableViewSetDefaultMetrics(pTable, 80.0f, fDefault, 24.0f) == XUI_OK);
		fTop = 0.0;
		for ( i = 0; i < 65536; i++ ) {
			fHeight = arrRows[i].fHeight > 0.0f ? arrRows[i].fHeight : fDefault;
			CHECK(__xuiTableViewRowTop(pData, i) == (float)fTop);
			CHECK(__xuiTableViewRowAtContentY(pData, (float)fTop) == i);
			CHECK(__xuiTableViewRowAtContentY(pData, (float)(fTop + fHeight - 0.5)) == i);
			fTop += fHeight;
		}
		CHECK(__xuiTableViewContentHeight(pTable, pData) == (float)fTop);
		CHECK(__xuiTableViewSpanHeight(pData, 0, INT_MAX) == (float)fTop);
		CHECK(__xuiTableViewRowAtContentY(pData, (float)fTop) == -1);
		CHECK(__xuiTableViewRowAtContentY(pData, -0.5f) == -1);
		CHECK(arrSteps[ROW_READ] == 0);
		CHECK(arrSteps[ROW_PROBE] <= 2ULL * 65536 * 17);
	}
	pOriginalRows = pData->arrRows;
	iAllocationFailure = 1;
	CHECK(xuiTableViewSetRows(pTable, arrRows, 16) == XUI_ERROR_OUT_OF_MEMORY);
	iAllocationFailure = -1;
	CHECK(pData->arrRows == pOriginalRows && pData->iRowStateCount == 65536);
	CHECK(__xuiTableViewContentHeight(pTable, pData) == (float)fTop);
	CHECK(xuiTableViewSetRows(pTable, arrRows, 3) == XUI_OK);
	fTop = 27.5 + arrRows[1].fHeight + arrRows[2].fHeight;
	CHECK(__xuiTableViewRowTop(pData, 60000) == (float)(fTop + (60000 - 3) * 27.5));
	tAdapter.iRows = 2;
	CHECK(xuiTableViewGetRowCount(pTable) == 2);
	CHECK(__xuiTableViewContentHeight(pTable, pData) == 27.5f + arrRows[1].fHeight);
	tAdapter.iRows = 1000000;
	CHECK(xuiTableViewSetRows(pTable, NULL, 0) == XUI_OK);
	CHECK(pData->arrRowPrefix == NULL);
	memset(arrSteps, 0, sizeof(arrSteps));
	CHECK(__xuiTableViewMergeOwner(pTable, pData, 999999, 0, NULL, NULL) == 0);
	CHECK(pData->arrMerges == NULL && pData->iMergeIndexedRows == 0);
	CHECK(__xuiTableViewRowAtContentY(pData, 999999 * 27.5f + 8.0f) == 999999);
	CHECK(arrSteps[ROW_READ] == 0 && arrSteps[ROW_PROBE] <= 20);
	printf("row index: 262144 variable-height boundary hits; metric changes, shrink/grow, default tail, 1M adapter-only rows, allocation rollback\n");
	free(arrRows);
	xuiWidgetDestroy(pTable);
	xuiDestroy(pContext);
	return iFailed;
}

static int merge_failure_case(void)
{
	int iFailed = 0;
	int fail;
	for ( fail = 0; fail < 3; fail++ ) {
		xui_context pContext = NULL;
		xui_widget pTable = NULL;
		xui_table_view_desc_t tDesc = {0};
		xui_table_view_column_t arrColumns[3] = {{0}};
		table_index_adapter_t tAdapter = {0};
		xui_table_view_data_t* pData;
		xui_rect_t tRect;
		tDesc.iSize = sizeof(tDesc);
		tDesc.arrColumns = arrColumns;
		tDesc.iColumnCount = 3;
		tDesc.onCount = count_rows;
		tDesc.onCell = cell_data;
		tDesc.pAdapterUser = &tAdapter;
		tAdapter.iRows = 32;
		tAdapter.bMerge = tAdapter.bCellMerge = 1;
		CHECK(xuiCreate(&pContext) == XUI_OK);
		CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
		if ( pTable == NULL ) { xuiDestroy(pContext); return iFailed + 1; }
		pData = __xuiTableViewGetData(pTable);
		iAllocationFailure = fail;
		CHECK(xuiTableViewGetCellContentRect(pTable, 2, 2, &tRect) == XUI_ERROR_OUT_OF_MEMORY);
		iAllocationFailure = -1;
		CHECK(pData->iMergeCount == 0 && pData->iMergeIndexedRows == 0 && !pData->bMergeBuilding);
		CHECK(xuiTableViewGetCellContentRect(pTable, 2, 2, &tRect) == XUI_OK);
		CHECK(tRect.fX == 80.0f && tRect.fY == 0.0f && tRect.fW == 160.0f && tRect.fH == 60.0f);
		CHECK(xuiTableViewRefresh(pTable) == XUI_OK);
		tAdapter.bRefreshOnce = 1;
		CHECK(xuiTableViewGetCellContentRect(pTable, 2, 2, &tRect) == XUI_ERROR_INVALID_ARGUMENT);
		CHECK(pData->iMergeCount == 0 && !pData->bMergeBuilding);
		CHECK(xuiTableViewGetCellContentRect(pTable, 2, 2, &tRect) == XUI_OK);
		xuiWidgetDestroy(pTable);
		xuiDestroy(pContext);
	}
	printf("merge index: allocation failure rollback/retry and generation change during discovery\n");
	return iFailed;
}

typedef struct paint_capture_t {
	int iBase;
	int iCount;
	int iTargetCount;
	int iDuplicates;
	int arrRows[128];
	int arrColumns[128];
	xui_rect_t tTarget;
} paint_capture_t;

static int capture_cell(xui_widget pWidget, int r, int c, const xui_table_view_cell_t* pCell,
	xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	paint_capture_t* pCapture = (paint_capture_t*)pUser;
	int i;
	for ( i = 0; i < pCapture->iCount && i < 128; i++ ) {
		if ( pCapture->arrRows[i] == r && pCapture->arrColumns[i] == c ) pCapture->iDuplicates++;
	}
	if ( pCapture->iCount < 128 ) {
		pCapture->arrRows[pCapture->iCount] = r;
		pCapture->arrColumns[pCapture->iCount] = c;
	}
	pCapture->iCount++;
	if ( r == pCapture->iBase && c == 1 ) {
		pCapture->iTargetCount++;
		pCapture->tTarget = tRect;
	}
	return 1;
}

static void change_sort(xui_widget pWidget, int c, int bDescending, void* pUser)
{
	((table_index_adapter_t*)pUser)->iPhase = 3;
}

static void change_resize(xui_widget pWidget, int c, float fWidth, void* pUser)
{
	((table_index_adapter_t*)pUser)->iPhase = 2;
}

static int viewport_case(int iRows, int bMerge)
{
	xui_test_proxy_state_t tProxy;
	xui_context pContext = NULL;
	xui_widget pRoot = NULL, pTable = NULL;
	xui_surface pTarget = NULL;
	xui_table_view_desc_t tDesc = {0};
	xui_table_view_column_t arrColumns[6] = {{0}};
	xui_table_view_row_t* arrRows;
	xui_table_view_data_t* pData;
	table_index_adapter_t tAdapter = {0};
	table_index_adapter_t tReplacement;
	paint_capture_t tCapture;
	xui_rect_i_t tFull = {0, 0, 620, 360};
	xui_rect_t tView, tRect, tWorld;
	xui_style_property_t tProperty = {0};
	float fX, fY, fAnchorTop, fRowTop;
	int iFailed = 0;
	int base = iRows - 16;
	int r, c, i, pass;
	unsigned long long iCalls;
	arrRows = (xui_table_view_row_t*)calloc((size_t)iRows, sizeof(*arrRows));
	CHECK(arrRows != NULL);
	if ( arrRows == NULL ) return 1;
	for ( i = 0; i < iRows; i++ ) arrRows[i].fHeight = (i % 8 == 0) ? 30.0f : 0.0f;
	xuiTestProxyInit(&tProxy);
	CHECK(xuiCreate(&pContext) == XUI_OK);
	CHECK(xuiSetProxy(pContext, &tProxy.tProxy) == XUI_OK);
	CHECK(xuiInputViewport(pContext, 620.0f, 360.0f) == XUI_OK);
	CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK);
	CHECK(xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 620, 360}) == XUI_OK);
	CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK);
	tAdapter.iRows = iRows;
	tAdapter.bMerge = bMerge;
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrColumns = arrColumns;
	tDesc.iColumnCount = 6;
	tDesc.arrRows = arrRows;
	tDesc.iRowCount = iRows;
	tDesc.onCount = count_rows;
	tDesc.onCell = cell_data;
	tDesc.pAdapterUser = &tAdapter;
	CHECK(xuiTableViewCreate(pContext, &pTable, &tDesc) == XUI_OK);
	if ( pTable == NULL ) { free(arrRows); xuiDestroy(pContext); return 1; }
	pData = __xuiTableViewGetData(pTable);
	CHECK(xuiWidgetAddChild(pRoot, pTable) == XUI_OK);
	CHECK(xuiWidgetSetRect(pTable, (xui_rect_t){24, 18, 220, 178}) == XUI_OK);
	if ( bMerge ) CHECK(xuiTableViewSetMergeProvider(pTable, merge_data, &tAdapter) == XUI_OK);
	CHECK(xuiTableViewSetCellRenderer(pTable, capture_cell, &tCapture) == XUI_OK);
	CHECK(xuiTestSurfaceCreate(&tProxy, &pTarget, 620, 360, XUI_SURFACE_USAGE_TARGET) == XUI_OK);
	CHECK(xuiLayout(pContext) == XUI_OK);
	fAnchorTop = __xuiTableViewRowTop(pData, base);
	fRowTop = fAnchorTop + 30.0f;
	CHECK(xuiTableViewSetOffset(pTable, 165.0f, fRowTop + 4.0f) == XUI_OK);
	/* Warm once, then invalidate only painting: refresh intentionally invalidates data. */
	memset(&tCapture, 0, sizeof(tCapture));
	tCapture.iBase = base;
	CHECK(xuiRender(pContext, pTarget, &tFull, 1) == XUI_OK);
	for ( pass = 0; pass < 3; pass++ ) {
		memset(&tCapture, 0, sizeof(tCapture));
		tCapture.iBase = base;
		tAdapter.iCells = tAdapter.iMerges = 0;
		memset(arrSteps, 0, sizeof(arrSteps));
		CHECK(xuiTableViewSetOffset(pTable, 165.0f, fRowTop + 4.0f + (pass & 1)) == XUI_OK);
		CHECK(xuiWidgetInvalidate(pData->pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK);
		CHECK(xuiRender(pContext, pTarget, &tFull, 1) == XUI_OK);
		CHECK(xuiTableViewGetFirstVisible(pTable) == base + 1);
		CHECK(xuiTableViewGetPaintVisibleCount(pTable) > 0 && xuiTableViewGetPaintVisibleCount(pTable) <= 10);
		CHECK(tCapture.iCount > 0 && tCapture.iCount <= 48 && tCapture.iDuplicates == 0);
		CHECK(tAdapter.iCells <= 48 && tAdapter.iMerges <= 48);
		CHECK(arrSteps[ROW_READ] == 0 && arrSteps[ROW_PROBE] <= 3 * 17 && arrSteps[MERGE_PROBE] <= 80);
		if ( bMerge ) {
			CHECK(tCapture.iTargetCount == 1);
			CHECK(tCapture.tTarget.fX == -85.0f && tCapture.tTarget.fY == -34.0f - (pass & 1));
			CHECK(tCapture.tTarget.fW == 160.0f && tCapture.tTarget.fH == 70.0f);
		}
	}
	printf("viewport rows=%d merge=%d hot_cells=%llu drawn=%d row_reads=%llu row_probes=%llu merge_probes=%llu\n",
		iRows, bMerge, tAdapter.iCells, tCapture.iCount, arrSteps[ROW_READ], arrSteps[ROW_PROBE], arrSteps[MERGE_PROBE]);
	tView = xuiTableViewGetViewportRect(pTable);
	tWorld = xuiWidgetGetWorldRect(pTable);
	tAdapter.iCells = tAdapter.iMerges = 0;
	for ( i = 0; i < 32; i++ ) {
		CHECK(xuiTableViewGetItemAt(pTable, tView.fX + 1, tView.fY + 1, &r, &c) == XUI_OK);
		CHECK(r == (bMerge ? base : base + 1) && c == (bMerge ? 1 : 2));
	}
	CHECK(tAdapter.iCells == 0 && tAdapter.iMerges == 0);
	CHECK(xuiInputPointerMove(pContext, tWorld.fX + tView.fX + 1, tWorld.fY + tView.fY + 1, 0) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(xuiTableViewGetHoverCell(pTable, &r, &c) == XUI_OK);
	CHECK(r == (bMerge ? base : base + 1) && c == (bMerge ? 1 : 2));
	iCalls = tAdapter.iCells;
	for ( i = 0; i < 32; i++ ) CHECK(xuiTableViewEnsureCellVisible(pTable, base + 2, 2) == XUI_OK);
	CHECK(tAdapter.iCells - iCalls == 32);
	CHECK(xuiTableViewGetOffset(pTable, &fX, &fY) == XUI_OK);
	if ( bMerge ) CHECK(fX == 80.0f && fY == fAnchorTop);

	tAdapter.bMerge = 1;
	tAdapter.iPhase = 1;
	CHECK(xuiTableViewSetMergeProvider(pTable, merge_data, &tAdapter) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fRowTop && tRect.fH == 60.0f);
	tAdapter.iPhase = 0;
	CHECK(xuiTableViewRefresh(pTable) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fAnchorTop && tRect.fH == 70.0f);
	CHECK(xuiTableViewSetMergeProvider(pTable, NULL, NULL) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fX == 160.0f && tRect.fW == 80.0f && tRect.fH == 20.0f);
	tAdapter.bCellMerge = 1;
	CHECK(xuiTableViewSetAdapter(pTable, count_rows, cell_data, &tAdapter) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fAnchorTop && tRect.fH == 70.0f);
	tReplacement = tAdapter;
	tReplacement.iPhase = 1;
	CHECK(xuiTableViewSetAdapter(pTable, count_rows, cell_data, &tReplacement) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fRowTop && tRect.fH == 60.0f);
	tReplacement.iRows = base + 2;
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 1, 2, &tRect) == XUI_OK);
	CHECK(tRect.fH == 20.0f);
	tReplacement.iRows = iRows;
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fH == 60.0f);
	CHECK(xuiTableViewSetColumns(pTable, arrColumns, 2) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 1, &tRect) == XUI_OK);
	CHECK(tRect.fW == 80.0f);
	CHECK(xuiTableViewSetColumns(pTable, arrColumns, 6) == XUI_OK);
	tReplacement.iPhase = 2;
	CHECK(xuiTableViewSetColumnWidth(pTable, 1, 100.0f) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 3, 2, &tRect) == XUI_OK);
	CHECK(tRect.fW == 180.0f && tRect.fY == fAnchorTop + 50.0f);
	tReplacement.iPhase = 1;
	arrColumns[0].bVisibleSet = 1;
	arrColumns[0].bVisible = 0;
	CHECK(xuiTableViewSetColumns(pTable, arrColumns, 6) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(tRect.fX == 0.0f && tRect.fW == 160.0f);
	arrColumns[0].bVisibleSet = 0;
	CHECK(xuiTableViewSetColumns(pTable, arrColumns, 6) == XUI_OK);
	CHECK(xuiTableViewSetSort(pTable, change_sort, &tReplacement) == XUI_OK);
	CHECK(xuiTableViewSetColumnResize(pTable, change_resize, &tReplacement) == XUI_OK);
	CHECK(xuiTableViewSetOffset(pTable, 0, 0) == XUI_OK);
	CHECK(xuiLayout(pContext) == XUI_OK);
	tView = xuiTableViewGetViewportRect(pTable);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 2, 2, &tRect) == XUI_OK);
	CHECK(xuiInputPointerDown(pContext, tWorld.fX + tView.fX + 80, tWorld.fY + 10, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(xuiInputPointerMove(pContext, tWorld.fX + tView.fX + 96, tWorld.fY + 10, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(xuiInputPointerUp(pContext, tWorld.fX + tView.fX + 96, tWorld.fY + 10, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(tReplacement.iPhase == 2);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 3, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fAnchorTop + 50.0f);
	CHECK(xuiInputPointerDown(pContext, tWorld.fX + tView.fX + 20, tWorld.fY + 10, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(xuiInputPointerUp(pContext, tWorld.fX + tView.fX + 20, tWorld.fY + 10, XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK);
	CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK);
	CHECK(tReplacement.iPhase == 3);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 4, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == fAnchorTop + 70.0f);
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = "tableview.default.row_height";
	tProperty.iDirtyFlags = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	tProperty.tValue.iSize = sizeof(tProperty.tValue);
	tProperty.tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tProperty.tValue.fFloat = 25.0f;
	tReplacement.iPhase = 4;
	CHECK(xuiWidgetSetInlineStyle(pTable, &tProperty, 1) == XUI_OK);
	CHECK(xuiLayout(pContext) == XUI_OK);
	CHECK(pData->fDefaultRowHeight == 25.0f);
	CHECK(__xuiTableViewRowTop(pData, 8) == 205.0f);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 5, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == __xuiTableViewRowTop(pData, base + 4) && tRect.fH == 75.0f);
	tReplacement.iPhase = 0;
	CHECK(xuiTableViewSetRows(pTable, NULL, 0) == XUI_OK);
	CHECK(__xuiTableViewRowTop(pData, 8) == 200.0f);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 1, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == base * 25.0f && tRect.fH == 75.0f);
	/* Very large default metrics must not turn overscan into an offscreen scan. */
	CHECK(xuiWidgetSetInlineStyle(pTable, NULL, 0) == XUI_OK);
	for ( i = 0; i < iRows; i++ ) arrRows[i].fHeight = 1.0f;
	CHECK(xuiTableViewSetRows(pTable, arrRows, iRows) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 1, 2, &tRect) == XUI_OK);
	tReplacement.iPhase = 3;
	CHECK(xuiTableViewSetDefaultMetrics(pTable, 80, 2000000, 24) == XUI_OK);
	CHECK(xuiTableViewGetCellContentRect(pTable, base + 4, 2, &tRect) == XUI_OK);
	CHECK(tRect.fY == base + 3.0f && tRect.fH == 3.0f);
	CHECK(xuiLayout(pContext) == XUI_OK);
	CHECK(xuiTableViewSetOffset(pTable, 165, (float)(iRows / 2)) == XUI_OK);
	memset(&tCapture, 0, sizeof(tCapture));
	CHECK(xuiRender(pContext, pTarget, &tFull, 1) == XUI_OK);
	tReplacement.iCells = 0;
	memset(arrSteps, 0, sizeof(arrSteps));
	memset(&tCapture, 0, sizeof(tCapture));
	CHECK(xuiWidgetInvalidate(pData->pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK);
	CHECK(xuiRender(pContext, pTarget, &tFull, 1) == XUI_OK);
	CHECK(xuiTableViewGetPaintVisibleCount(pTable) == iRows / 2);
	CHECK(tReplacement.iCells > 0 && tReplacement.iCells <= 1024);
	CHECK(arrSteps[ROW_READ] == 0 && arrSteps[ROW_PROBE] <= 3 * 17);
	printf("overscan rows=%d counter=%d hot_cells=%llu row_reads=%llu row_probes=%llu\n",
		iRows, xuiTableViewGetPaintVisibleCount(pTable), tReplacement.iCells, arrSteps[ROW_READ], arrSteps[ROW_PROBE]);
	tReplacement.iRows = 0;
	CHECK(xuiTableViewRefreshAdapter(pTable) == XUI_OK);
	CHECK(xuiRender(pContext, pTarget, &tFull, 1) == XUI_OK);
	CHECK(xuiTableViewGetFirstVisible(pTable) == 0 && xuiTableViewGetPaintVisibleCount(pTable) == 0);
	free(arrRows);
	xuiDestroy(pContext);
	tProxy.tProxy.surfaceDestroy(&tProxy.tProxy, pTarget);
	return iFailed;
}

int main(void)
{
	int iFailed = 0;
	iFailed += scale_case(4096, 0);
	iFailed += scale_case(65536, 0);
	iFailed += scale_case(4096, 1);
	iFailed += scale_case(65536, 1);
	iFailed += overlap_case();
	iFailed += huge_merge_case();
	iFailed += row_index_case();
	iFailed += merge_failure_case();
	iFailed += viewport_case(4096, 0);
	iFailed += viewport_case(65536, 0);
	iFailed += viewport_case(4096, 1);
	iFailed += viewport_case(65536, 1);
	CHECK(iOutstandingAllocations == 0);
	printf("xui_table_view_index_test: %s (%d failures)\n", iFailed ? "FAILED" : "passed", iFailed);
	return iFailed ? 1 : 0;
}
