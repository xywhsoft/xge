#include "xui.h"

#include <stdio.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_layout_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiTestRectEquals(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static xui_thickness_t __xuiTestThickness(float fLeft, float fTop, float fRight, float fBottom)
{
	xui_thickness_t tValue;

	tValue.fLeft = fLeft;
	tValue.fTop = fTop;
	tValue.fRight = fRight;
	tValue.fBottom = fBottom;
	return tValue;
}

static xui_table_track_t __xuiTestTrack(int iSizeMode, float fValue)
{
	xui_table_track_t tTrack;

	tTrack.iSizeMode = iSizeMode;
	tTrack.fValue = fValue;
	tTrack.fMin = 0.0f;
	tTrack.fMax = XUI_LAYOUT_UNBOUNDED;
	tTrack.fWeight = 1.0f;
	return tTrack;
}

int main(void)
{
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pA;
	xui_widget pB;
	xui_widget pC;
	xui_vec2_t tMeasured;
	xui_table_track_t tTrack;
	int iRet;
	int iFailed;
	int iWidthMode;
	int iHeightMode;
	int iRows;
	int iColumns;
	int iRow;
	int iColumn;
	int iRowSpan;
	int iColumnSpan;

	pContext = NULL;
	pRoot = NULL;
	pA = NULL;
	pB = NULL;
	pC = NULL;
	iFailed = 0;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pContext != NULL), "xuiCreate failed");
	iRet = xuiSetViewportSize(pContext, 300.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set viewport failed");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pRoot != NULL), "root create failed");
	iRet = xuiWidgetCreate(pContext, &pA);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pA != NULL), "A create failed");
	iRet = xuiWidgetCreate(pContext, &pB);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pB != NULL), "B create failed");
	iRet = xuiWidgetCreate(pContext, &pC);
	XUI_TEST_CHECK((iRet == XUI_OK) && (pC != NULL), "C create failed");

	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root failed");
	iRet = xuiWidgetAddChild(pRoot, pA);
	XUI_TEST_CHECK(iRet == XUI_OK, "add A failed");
	iRet = xuiWidgetAddChild(pRoot, pB);
	XUI_TEST_CHECK(iRet == XUI_OK, "add B failed");
	iRet = xuiWidgetAddChild(pRoot, pC);
	XUI_TEST_CHECK(iRet == XUI_OK, "add C failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_ROW);
	XUI_TEST_CHECK(iRet == XUI_OK, "set row layout failed");
	iRet = xuiWidgetSetPadding(pRoot, __xuiTestThickness(10.0f, 10.0f, 10.0f, 10.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "set padding failed");
	iRet = xuiWidgetSetGap(pRoot, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set row gap failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){50.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){30.0f, 40.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set C preferred failed");
	iRet = xuiWidgetSetSizeMode(pB, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B fill failed");
	iRet = xuiWidgetGetSizeMode(pB, &iWidthMode, &iHeightMode);
	XUI_TEST_CHECK((iRet == XUI_OK) && (iWidthMode == XUI_SIZE_FILL) && (iHeightMode == XUI_SIZE_FILL), "get B size mode failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "row layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pRoot), 0.0f, 0.0f, 300.0f, 100.0f), "root arrange failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 10.0f, 10.0f, 50.0f, 20.0f), "row A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 65.0f, 10.0f, 190.0f, 80.0f), "row B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 260.0f, 10.0f, 30.0f, 40.0f), "row C rect failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_COLUMN);
	XUI_TEST_CHECK(iRet == XUI_OK, "set column layout failed");
	iRet = xuiWidgetSetPadding(pRoot, __xuiTestThickness(0.0f, 0.0f, 0.0f, 0.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "clear padding failed");
	iRet = xuiWidgetSetGap(pRoot, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set column gap failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){40.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A column preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){30.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set C column preferred failed");
	iRet = xuiWidgetSetAlign(pC, XUI_ALIGN_END, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C align failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "column layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 40.0f, 10.0f), "column A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 0.0f, 14.0f, 300.0f, 62.0f), "column B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 270.0f, 80.0f, 30.0f, 20.0f), "column C rect failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_OVERLAY);
	XUI_TEST_CHECK(iRet == XUI_OK, "set overlay layout failed");
	iRet = xuiWidgetSetAlign(pA, XUI_ALIGN_CENTER, XUI_ALIGN_CENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A center failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){50.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A overlay preferred failed");
	iRet = xuiWidgetSetMargin(pB, __xuiTestThickness(5.0f, 5.0f, 5.0f, 5.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "set B margin failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 125.0f, 40.0f, 50.0f, 20.0f), "overlay A center failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 5.0f, 5.0f, 290.0f, 90.0f), "overlay B fill failed");

	iRet = xuiWidgetMeasure(pA, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tMeasured.fX == 50.0f) && (tMeasured.fY == 20.0f), "measure A failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){60.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "resize A preferred failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlay relayout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 120.0f, 40.0f, 60.0f, 20.0f), "overlay relayout center failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_FLOW);
	XUI_TEST_CHECK(iRet == XUI_OK, "set flow layout failed");
	iRet = xuiWidgetSetGap(pRoot, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set flow gap failed");
	iRet = xuiWidgetSetVisible(pB, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "show B failed");
	iRet = xuiWidgetSetMargin(pA, __xuiTestThickness(0.0f, 0.0f, 0.0f, 0.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "clear A margin failed");
	iRet = xuiWidgetSetMargin(pB, __xuiTestThickness(0.0f, 0.0f, 0.0f, 0.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "clear B margin failed");
	iRet = xuiWidgetSetMargin(pC, __xuiTestThickness(0.0f, 0.0f, 0.0f, 0.0f));
	XUI_TEST_CHECK(iRet == XUI_OK, "clear C margin failed");
	iRet = xuiWidgetSetSizeMode(pA, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A flow size mode failed");
	iRet = xuiWidgetSetSizeMode(pB, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B flow size mode failed");
	iRet = xuiWidgetSetSizeMode(pC, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C flow size mode failed");
	iRet = xuiWidgetSetAlign(pA, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A flow align failed");
	iRet = xuiWidgetSetAlign(pB, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B flow align failed");
	iRet = xuiWidgetSetAlign(pC, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C flow align failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){80.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A flow preferred failed");
	iRet = xuiWidgetSetPreferredSize(pB, (xui_vec2_t){80.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set B flow preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){150.0f, 15.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set C flow preferred failed");
	iRet = xuiWidgetSetFlowMode(pA, XUI_FLOW_INLINE);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A inline failed");
	iRet = xuiWidgetSetFlowMode(pB, XUI_FLOW_INLINE);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B inline failed");
	iRet = xuiWidgetSetFlowMode(pC, XUI_FLOW_INLINE);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C inline failed");
	XUI_TEST_CHECK(xuiWidgetGetFlowMode(pC) == XUI_FLOW_INLINE, "get C flow mode failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "flow layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 80.0f, 10.0f), "flow A inline rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 85.0f, 0.0f, 80.0f, 20.0f), "flow B inline rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 0.0f, 25.0f, 150.0f, 15.0f), "flow C wrap rect failed");
	iRet = xuiWidgetSetFlowMode(pB, XUI_FLOW_BLOCK);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B block failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "flow block layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 80.0f, 10.0f), "flow block A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 0.0f, 15.0f, 80.0f, 20.0f), "flow block B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 0.0f, 40.0f, 150.0f, 15.0f), "flow block C rect failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_TABLE);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table layout failed");
	iRet = xuiWidgetSetGap(pRoot, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table gap failed");
	iRet = xuiWidgetSetTableSize(pRoot, 2, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table size failed");
	iRet = xuiWidgetGetTableSize(pRoot, &iRows, &iColumns);
	XUI_TEST_CHECK((iRet == XUI_OK) && (iRows == 2) && (iColumns == 3), "get table size failed");
	tTrack = __xuiTestTrack(XUI_SIZE_FIXED, 40.0f);
	iRet = xuiWidgetSetTableColumn(pRoot, 0, &tTrack);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table column 0 failed");
	tTrack = __xuiTestTrack(XUI_SIZE_CONTENT, 0.0f);
	iRet = xuiWidgetSetTableColumn(pRoot, 1, &tTrack);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table column 1 failed");
	tTrack = __xuiTestTrack(XUI_SIZE_FILL, 0.0f);
	iRet = xuiWidgetSetTableColumn(pRoot, 2, &tTrack);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table column 2 failed");
	tTrack = __xuiTestTrack(XUI_SIZE_FILL, 0.0f);
	iRet = xuiWidgetSetTableRow(pRoot, 1, &tTrack);
	XUI_TEST_CHECK(iRet == XUI_OK, "set table row 1 failed");
	iRet = xuiWidgetGetTableColumn(pRoot, 2, &tTrack);
	XUI_TEST_CHECK((iRet == XUI_OK) && (tTrack.iSizeMode == XUI_SIZE_FILL), "get table column failed");
	iRet = xuiWidgetSetTableCell(pA, 0, 0, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A cell failed");
	iRet = xuiWidgetSetTableCell(pB, 0, 1, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B cell failed");
	iRet = xuiWidgetSetTableCell(pC, 1, 0, 1, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C cell failed");
	iRet = xuiWidgetGetTableCell(pC, &iRow, &iColumn, &iRowSpan, &iColumnSpan);
	XUI_TEST_CHECK((iRet == XUI_OK) && (iRow == 1) && (iColumn == 0) && (iRowSpan == 1) && (iColumnSpan == 3), "get C cell failed");
	iRet = xuiWidgetSetFlowMode(pB, XUI_FLOW_INLINE);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset B flow mode failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){30.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A table preferred failed");
	iRet = xuiWidgetSetPreferredSize(pB, (xui_vec2_t){50.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set B table preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){100.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set C table preferred failed");
	iRet = xuiWidgetSetSizeMode(pC, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C table fill failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "table layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 30.0f, 10.0f), "table A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 42.0f, 0.0f, 50.0f, 20.0f), "table B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 0.0f, 22.0f, 300.0f, 10.0f), "table C span rect failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_DOCK);
	XUI_TEST_CHECK(iRet == XUI_OK, "set dock layout failed");
	iRet = xuiWidgetSetGap(pRoot, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set dock gap failed");
	iRet = xuiWidgetSetDock(pA, XUI_DOCK_LEFT);
	XUI_TEST_CHECK((iRet == XUI_OK) && (xuiWidgetGetDock(pA) == XUI_DOCK_LEFT), "set A dock left failed");
	iRet = xuiWidgetSetDock(pB, XUI_DOCK_TOP);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B dock top failed");
	iRet = xuiWidgetSetDock(pC, XUI_DOCK_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C dock fill failed");
	iRet = xuiWidgetSetSizeMode(pA, XUI_SIZE_FIXED, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A dock size mode failed");
	iRet = xuiWidgetSetSizeMode(pB, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B dock size mode failed");
	iRet = xuiWidgetSetSizeMode(pC, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C dock size mode failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){40.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A dock preferred failed");
	iRet = xuiWidgetSetPreferredSize(pB, (xui_vec2_t){10.0f, 20.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set B dock preferred failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dock layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 40.0f, 100.0f), "dock A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 40.0f, 0.0f, 260.0f, 20.0f), "dock B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 40.0f, 20.0f, 260.0f, 80.0f), "dock C rect failed");

	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_GRID);
	XUI_TEST_CHECK(iRet == XUI_OK, "set grid layout failed");
	iRet = xuiWidgetSetGap(pRoot, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set grid gap failed");
	iRet = xuiWidgetSetGridMetrics(pRoot, 2, 40.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set grid metrics failed");
	iRet = xuiWidgetGetGridMetrics(pRoot, &iColumns, &tMeasured.fX, &tMeasured.fY);
	XUI_TEST_CHECK((iRet == XUI_OK) && (iColumns == 2) && (tMeasured.fX == 40.0f) && (tMeasured.fY == 20.0f), "get grid metrics failed");
	iRet = xuiWidgetSetSizeMode(pA, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A grid size mode failed");
	iRet = xuiWidgetSetSizeMode(pB, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B grid size mode failed");
	iRet = xuiWidgetSetSizeMode(pC, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C grid size mode failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){10.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set A grid preferred failed");
	iRet = xuiWidgetSetPreferredSize(pB, (xui_vec2_t){20.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set B grid preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){30.0f, 10.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "set C grid preferred failed");
	iRet = xuiWidgetSetAlign(pA, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A grid align failed");
	iRet = xuiWidgetSetAlign(pB, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B grid align failed");
	iRet = xuiWidgetSetAlign(pC, XUI_ALIGN_START, XUI_ALIGN_START);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C grid align failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "grid layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 10.0f, 10.0f), "grid A rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 45.0f, 0.0f, 20.0f, 10.0f), "grid B rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 0.0f, 25.0f, 30.0f, 10.0f), "grid C rect failed");

	iRet = xuiSetViewportSize(pContext, 29.0f, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set fractional distribution viewport failed");
	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_ROW);
	XUI_TEST_CHECK(iRet == XUI_OK, "set snap row layout failed");
	iRet = xuiWidgetSetGap(pRoot, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear snap row gap failed");
	iRet = xuiWidgetSetPreferredSize(pA, (xui_vec2_t){0.0f, 0.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "clear A snap preferred failed");
	iRet = xuiWidgetSetPreferredSize(pB, (xui_vec2_t){0.0f, 0.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "clear B snap preferred failed");
	iRet = xuiWidgetSetPreferredSize(pC, (xui_vec2_t){0.0f, 0.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "clear C snap preferred failed");
	iRet = xuiWidgetSetSizeMode(pA, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A snap fill failed");
	iRet = xuiWidgetSetSizeMode(pB, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B snap fill failed");
	iRet = xuiWidgetSetSizeMode(pC, XUI_SIZE_FILL, XUI_SIZE_FILL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C snap fill failed");
	iRet = xuiWidgetSetAlign(pA, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	XUI_TEST_CHECK(iRet == XUI_OK, "set A snap stretch failed");
	iRet = xuiWidgetSetAlign(pB, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	XUI_TEST_CHECK(iRet == XUI_OK, "set B snap stretch failed");
	iRet = xuiWidgetSetAlign(pC, XUI_ALIGN_STRETCH, XUI_ALIGN_STRETCH);
	XUI_TEST_CHECK(iRet == XUI_OK, "set C snap stretch failed");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "snap row layout failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pRoot), 0.0f, 0.0f, 29.0f, 10.0f), "snap root rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 10.0f, 10.0f), "snap row A 29/3 rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pB), 10.0f, 0.0f, 9.0f, 10.0f), "snap row B 29/3 rect failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pC), 19.0f, 0.0f, 10.0f, 10.0f), "snap row C 29/3 rect failed");

	iRet = xuiWidgetArrange(pA, (xui_rect_t){0.2f, 0.2f, 9.6f, 9.6f});
	XUI_TEST_CHECK(iRet == XUI_OK, "manual arrange snap failed");
	XUI_TEST_CHECK(__xuiTestRectEquals(xuiWidgetGetRect(pA), 0.0f, 0.0f, 10.0f, 10.0f), "manual arrange snap rect failed");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_layout_test passed\n");
	return 0;
}
