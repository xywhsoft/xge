#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_iFailures;
static int g_iChecks;
static int g_iStrokes;
static xui_rect_t g_tStroke;
static int g_iCircles;
static float g_fCircleX;
static float g_fCircleY;

#define CHECK(expr, msg) do { \
	g_iChecks++; \
	if ( !(expr) ) { printf("FAIL: %s (line %d)\n", msg, __LINE__); g_iFailures++; } \
} while ( 0 )

typedef struct keyboard_test_t {
	xui_test_proxy_state_t tProxy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pWidget;
	xui_widget pAfter;
	xui_font pFont;
	int iClicks;
	int iIndex;
	int iValue;
	int iContexts;
	int bClearOnClick;
	int iDefaultActions;
	xui_chart_hit_t tHit;
} keyboard_test_t;

static int record_stroke(xui_proxy pProxy, xui_draw_context pDraw,
	xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	(void)pProxy; (void)pDraw; (void)fWidth; (void)iColor;
	g_iStrokes++;
	g_tStroke = tRect;
	return XUI_OK;
}

static int record_circle(xui_proxy pProxy, xui_draw_context pDraw,
	float fX, float fY, float fRadius, float fWidth, uint32_t iColor)
{
	(void)pProxy; (void)pDraw; (void)fRadius; (void)fWidth; (void)iColor;
	g_iCircles++;
	g_fCircleX = fX;
	g_fCircleY = fY;
	return XUI_OK;
}

static void click_item(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	keyboard_test_t* pTest = (keyboard_test_t*)pUser;
	pTest->iClicks++;
	pTest->iIndex = iIndex;
	pTest->iValue = iValue;
	if ( pTest->bClearOnClick ) (void)xuiBreadcrumbClearItems(pWidget);
}

static void default_action(xui_widget pWidget, void* pUser)
{
	keyboard_test_t* pTest = (keyboard_test_t*)pUser;
	(void)pWidget;
	pTest->iDefaultActions++;
}

static int chart_context(xui_widget pWidget, const xui_chart_hit_t* pHit,
	float fX, float fY, void* pUser)
{
	keyboard_test_t* pTest = (keyboard_test_t*)pUser;
	(void)pWidget; (void)fX; (void)fY;
	pTest->tHit = *pHit;
	pTest->iContexts++;
	return XUI_EVENT_DISPATCH_STOP;
}

static int breadcrumb_context(xui_widget pWidget, int iIndex, int iValue,
	float fX, float fY, void* pUser)
{
	keyboard_test_t* pTest = (keyboard_test_t*)pUser;
	(void)pWidget; (void)fX; (void)fY;
	pTest->iIndex = iIndex;
	pTest->iValue = iValue;
	pTest->iContexts++;
	return XUI_EVENT_DISPATCH_STOP;
}

static void key_down(keyboard_test_t* pTest, int iKey, uint32_t iModifiers)
{
	CHECK(xuiInputKeyDown(pTest->pContext, iKey, iModifiers) == XUI_OK, "key down");
	CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "key down dispatch");
}

static void key_up(keyboard_test_t* pTest, int iKey)
{
	CHECK(xuiInputKeyUp(pTest->pContext, iKey, 0) == XUI_OK, "key up");
	CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "key up dispatch");
}

static void key(keyboard_test_t* pTest, int iKey)
{
	key_down(pTest, iKey, 0);
	key_up(pTest, iKey);
}

static void focus(keyboard_test_t* pTest)
{
	CHECK(xuiSetFocusWidget(pTest->pContext, pTest->pWidget) == XUI_OK, "set focus");
	CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "focus dispatch");
}

static int setup(keyboard_test_t* pTest, int bChart)
{
	memset(pTest, 0, sizeof(*pTest));
	xuiTestProxyInit(&pTest->tProxy);
	pTest->tProxy.tProxy.drawRectStroke = record_stroke;
	pTest->tProxy.tProxy.drawCircleStroke = record_circle;
	if ( xuiCreate(&pTest->pContext) != XUI_OK ) return 0;
	if ( xuiSetProxy(pTest->pContext, &pTest->tProxy.tProxy) != XUI_OK ) return 0;
	if ( pTest->tProxy.tProxy.fontLoadMemory(&pTest->tProxy.tProxy,
		&pTest->pFont, "keyboard", 8, 14.0f, XUI_FONT_FORMAT_TTF) != XUI_OK ) return 0;
	(void)xuiSetDefaultFont(pTest->pContext, pTest->pFont);
	(void)xuiInputViewport(pTest->pContext, 600, 400);
	if ( xuiWidgetCreate(pTest->pContext, &pTest->pRoot) != XUI_OK ) return 0;
	(void)xuiWidgetSetRect(pTest->pRoot, (xui_rect_t){0, 0, 600, 400});
	(void)xuiSetRootWidget(pTest->pContext, pTest->pRoot);
	(void)xuiWidgetSetDefaultAction(pTest->pRoot, default_action, pTest);
	if ( (bChart ? xuiChartCreate(pTest->pContext, &pTest->pWidget, NULL) :
		xuiBreadcrumbCreate(pTest->pContext, &pTest->pWidget, NULL)) != XUI_OK ) return 0;
	(void)xuiWidgetSetRect(pTest->pWidget, (xui_rect_t){30, 40, 420, bChart ? 260 : 36});
	(void)xuiWidgetAddChild(pTest->pRoot, pTest->pWidget);
	if ( xuiWidgetCreate(pTest->pContext, &pTest->pAfter) != XUI_OK ) return 0;
	(void)xuiWidgetSetRect(pTest->pAfter, (xui_rect_t){460, 40, 60, 36});
	(void)xuiWidgetSetFocusable(pTest->pAfter, 1);
	(void)xuiWidgetSetTabStop(pTest->pAfter, 1);
	(void)xuiWidgetAddChild(pTest->pRoot, pTest->pAfter);
	return 1;
}

static void cleanup(keyboard_test_t* pTest)
{
	if ( pTest->pContext != NULL ) xuiDestroy(pTest->pContext);
	if ( pTest->pFont != NULL ) pTest->tProxy.tProxy.fontDestroy(&pTest->tProxy.tProxy, pTest->pFont);
}

static void pointer(keyboard_test_t* pTest, xui_rect_t tLocal, int bDown)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pTest->pWidget);
	int iX = tWorld.fX + tLocal.fX + tLocal.fW / 2;
	int iY = tWorld.fY + tLocal.fY + tLocal.fH / 2;
	if ( bDown ) {
		CHECK(xuiInputPointerMove(pTest->pContext, iX, iY, 0) == XUI_OK, "pointer move");
		CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "move dispatch");
		CHECK(xuiInputPointerDown(pTest->pContext, iX, iY,
			XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "pointer down");
	} else {
		CHECK(xuiInputPointerUp(pTest->pContext, iX, iY,
			XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "pointer up");
	}
	CHECK(xuiDispatchPendingEvents(pTest->pContext) == XUI_OK, "pointer dispatch");
}

static void check_hit(keyboard_test_t* pTest, int iPart, int iSeries, int iItem)
{
	int iContexts = pTest->iContexts;
	key(pTest, XUI_KEY_CONTEXT_MENU);
	CHECK(pTest->iContexts == iContexts + 1, "one context callback");
	CHECK(pTest->tHit.iPart == iPart && pTest->tHit.iSeries == iSeries &&
		pTest->tHit.iItem == iItem, "keyboard and context share chart target");
}

static void test_breadcrumb(void)
{
	keyboard_test_t t;
	xui_breadcrumb_item_t arrItems[] = {
		{"Home", 1, 10}, {"Disabled", 0, 20}, {"Folder", 1, 30}, {"Current", 0, 40}
	};
	xui_rect_t tItem;
	int iClicks;
	if ( !setup(&t, 0) ) { CHECK(0, "breadcrumb setup"); cleanup(&t); return; }
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "empty breadcrumb has no tab stop");
	(void)xuiBreadcrumbSetItems(t.pWidget, arrItems, 4);
	(void)xuiBreadcrumbSetClick(t.pWidget, click_item, &t);
	(void)xuiBreadcrumbSetContextMenu(t.pWidget, breadcrumb_context, &t);
	key(&t, XUI_KEY_TAB);
	CHECK(xuiGetFocusWidget(t.pContext) == t.pWidget, "interactive breadcrumb tab entry");
	CHECK(xuiRenderPrepare(t.pContext) == XUI_OK, "breadcrumb focus render");
	CHECK(g_iStrokes > 0, "breadcrumb visible focus ring");
	tItem = xuiBreadcrumbGetItemRect(t.pWidget, 0);
	CHECK(g_tStroke.fX == tItem.fX && g_tStroke.fW == tItem.fW, "focus ring marks current item");
	key(&t, XUI_KEY_RIGHT);
	CHECK(t.iClicks == 0, "breadcrumb navigation does not click");
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == 1 && t.iIndex == 2 && t.iValue == 30, "right skips disabled and enter activates");
	key(&t, XUI_KEY_RIGHT);
	key(&t, XUI_KEY_SPACE);
	CHECK(t.iClicks == 2 && t.iIndex == 2, "right clamps at last clickable");
	key(&t, XUI_KEY_HOME);
	key_down(&t, XUI_KEY_SPACE, 0);
	key_down(&t, XUI_KEY_SPACE, 0);
	CHECK(t.iClicks == 2, "space waits for release");
	CHECK(xuiBreadcrumbGetActiveIndex(t.pWidget) == 0, "space uses breadcrumb pressed state");
	key_up(&t, XUI_KEY_SPACE);
	CHECK(t.iClicks == 3 && t.iIndex == 0, "space repeat activates exactly once");
	key_down(&t, XUI_KEY_ENTER, 0);
	key_down(&t, XUI_KEY_ENTER, 0);
	key_up(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == 4, "enter repeat activates exactly once");
	CHECK(t.iDefaultActions == 0, "breadcrumb enter does not also invoke parent action");
	key_down(&t, XUI_KEY_SPACE, 0);
	key(&t, XUI_KEY_ESCAPE);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(t.iClicks == 4 && xuiBreadcrumbGetActiveIndex(t.pWidget) == -1, "escape cancels pending activation");
	key(&t, XUI_KEY_END);
	key(&t, XUI_KEY_CONTEXT_MENU);
	CHECK(t.iContexts == 1 && t.iIndex == 2, "context menu follows keyboard focus");
	key(&t, XUI_KEY_LEFT);
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iIndex == 0, "left skips disabled");
	key(&t, XUI_KEY_TAB);
	CHECK(xuiGetFocusWidget(t.pContext) == t.pAfter, "breadcrumb has only one tab stop");
	key_down(&t, XUI_KEY_TAB, XUI_MOD_SHIFT);
	key_up(&t, XUI_KEY_TAB);
	CHECK(xuiGetFocusWidget(t.pContext) == t.pWidget, "reverse tab entry");
	(void)xuiRenderPrepare(t.pContext);
	tItem = xuiBreadcrumbGetItemRect(t.pWidget, 2);
	iClicks = t.iClicks;
	pointer(&t, tItem, 1);
	pointer(&t, tItem, 0);
	CHECK(t.iClicks == iClicks + 1 && t.iIndex == 2, "one pointer click");
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == iClicks + 2 && t.iIndex == 2, "keyboard resumes mouse target");
	iClicks = t.iClicks;
	key_down(&t, XUI_KEY_SPACE, 0);
	pointer(&t, tItem, 1);
	key_up(&t, XUI_KEY_SPACE);
	pointer(&t, tItem, 0);
	CHECK(t.iClicks == iClicks + 1, "mouse takes over pending keyboard press without double click");
	key_down(&t, XUI_KEY_SPACE, 0);
	(void)xuiSetFocusWidget(t.pContext, t.pAfter);
	(void)xuiDispatchPendingEvents(t.pContext);
	focus(&t);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(t.iClicks == iClicks + 1, "blur cancels pending space");
	(void)xuiWidgetSetEnabled(t.pWidget, 0);
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == iClicks + 1, "disabled breadcrumb cannot activate");
	(void)xuiWidgetSetEnabled(t.pWidget, 1);
	focus(&t);
	key(&t, XUI_KEY_END);
	(void)xuiBreadcrumbSetItem(t.pWidget, 2, "Folder", 0, 30);
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iIndex == 0 && t.iClicks == iClicks + 2, "disabled focused item falls back to clickable item");
	(void)xuiBreadcrumbSetItem(t.pWidget, 0, "Home", 0, 10);
	(void)xuiBreadcrumbSetContextMenu(t.pWidget, NULL, NULL);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "all noninteractive items remove tab stop");
	(void)xuiBreadcrumbClearItems(t.pWidget);
	key(&t, XUI_KEY_HOME);
	key(&t, XUI_KEY_ENTER);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "cleared breadcrumb removes tab stop");
	(void)xuiBreadcrumbAddItem(t.pWidget, "New", 1, 99);
	focus(&t);
	t.bClearOnClick = 1;
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iIndex == 0 && t.iValue == 99 && xuiBreadcrumbGetItemCount(t.pWidget) == 0,
		"callback may replace breadcrumb data");
	t.bClearOnClick = 0;
	(void)xuiBreadcrumbSetItems(t.pWidget, arrItems, 4);
	focus(&t);
	iClicks = t.iClicks;
	key_down(&t, XUI_KEY_SPACE, 0);
	(void)xuiWidgetSetEnabled(t.pWidget, 0);
	(void)xuiWidgetSetEnabled(t.pWidget, 1);
	focus(&t);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(t.iClicks == iClicks, "disable and reenable cancels pending breadcrumb press");
	key_down(&t, XUI_KEY_ENTER, 0);
	(void)xuiBreadcrumbSetItem(t.pWidget, 0, "Changed", 1, 123);
	key_up(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == iClicks, "item replacement cancels keyboard press");
	(void)xuiRenderPrepare(t.pContext);
	tItem = xuiBreadcrumbGetItemRect(t.pWidget, 0);
	pointer(&t, tItem, 1);
	CHECK(xuiGetPointerCapture(t.pContext) == t.pWidget, "breadcrumb captures pointer");
	key(&t, XUI_KEY_RIGHT);
	key(&t, XUI_KEY_SPACE);
	pointer(&t, tItem, 0);
	CHECK(t.iClicks == iClicks + 1 && t.iIndex == 0, "pointer press owns breadcrumb activation");
	pointer(&t, tItem, 1);
	(void)xuiBreadcrumbClearItems(t.pWidget);
	CHECK(xuiGetPointerCapture(t.pContext) == NULL, "clearing breadcrumb releases capture");
	pointer(&t, tItem, 0);
	CHECK(t.iClicks == iClicks + 1, "clear during pointer press does not click replacement data");
	(void)xuiBreadcrumbAddItem(t.pWidget, "Context only", 0, 88);
	(void)xuiBreadcrumbSetContextMenu(t.pWidget, breadcrumb_context, &t);
	focus(&t);
	key(&t, XUI_KEY_CONTEXT_MENU);
	CHECK(t.iValue == 88, "context-only breadcrumb gets keyboard entry");
	key(&t, XUI_KEY_ENTER);
	CHECK(t.iClicks == iClicks + 1, "context-only item does not gain a click action");
	cleanup(&t);
}

static void test_chart(void)
{
	keyboard_test_t t;
	xui_chart_point_t arrPoints[] = {{0, 2, 2, "A", 0}, {1, 4, 4, "B", 0}, {2, 3, 3, "C", 0}};
	xui_chart_point_t arrPie[] = {{0, 0, 0, "zero", 0}, {0, 0, 2, "positive", 0}, {0, 0, -1, "negative", 0}};
	const int arrTypes[] = {XUI_CHART_SERIES_LINE, XUI_CHART_SERIES_BAR, XUI_CHART_SERIES_SCATTER, XUI_CHART_SERIES_PIE};
	xui_rect_t tItem;
	xui_vec2_t tPixel;
	xui_chart_hit_t tMouseHit;
	double fMinX, fMaxX, fMinY, fMaxY;
	double fOldMinX, fOldSpanX;
	int iSeries, iHidden, iEmpty;
	int iType;
	int i;
	int iDirection, iMode;
	int iStrokes;
	if ( !setup(&t, 1) ) { CHECK(0, "chart setup"); cleanup(&t); return; }
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "empty chart has no tab stop");
	key(&t, XUI_KEY_TAB);
	CHECK(xuiGetFocusWidget(t.pContext) == t.pAfter, "tab skips empty chart");
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_LINE, "Line", &iSeries);
	(void)xuiChartSetSeriesData(t.pWidget, iSeries, arrPoints, 3);
	(void)xuiChartSetLegendVisible(t.pWidget, 0);
	(void)xuiChartSetTooltipVisible(t.pWidget, 0);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "display-only chart has no tab stop");
	(void)xuiChartSetTooltipVisible(t.pWidget, 1);
	(void)xuiChartSetContextMenu(t.pWidget, chart_context, &t);
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_LINE, "Hidden", &iHidden);
	(void)xuiChartSetSeriesData(t.pWidget, iHidden, arrPoints, 3);
	(void)xuiChartSetSeriesVisible(t.pWidget, iHidden, 0);
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_LINE, "Empty", &iEmpty);
	focus(&t);
	iStrokes = g_iStrokes;
	CHECK(xuiRenderPrepare(t.pContext) == XUI_OK, "chart focus render");
	CHECK(g_iStrokes > iStrokes, "chart visible focus ring");
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 0);
	key(&t, XUI_KEY_RIGHT);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 1);
	key(&t, XUI_KEY_END);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 2);
	key(&t, XUI_KEY_RIGHT);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 2);
	key(&t, XUI_KEY_HOME);
	key(&t, XUI_KEY_LEFT);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 0);
	key(&t, XUI_KEY_DOWN);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 1);
	key(&t, XUI_KEY_UP);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 0);
	(void)xuiChartDataToPixel(t.pWidget, 1, 4, &tPixel);
	tItem = (xui_rect_t){(int)tPixel.fX - 1, (int)tPixel.fY - 1, 2, 2};
	pointer(&t, tItem, 1);
	pointer(&t, tItem, 0);
	key(&t, XUI_KEY_RIGHT);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 2);
	key(&t, XUI_KEY_ENTER);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 2);
	CHECK(t.iDefaultActions == 0, "chart enter does not also invoke parent action");
	(void)xuiChartSetLegendVisible(t.pWidget, 1);
	key(&t, XUI_KEY_END);
	check_hit(&t, XUI_CHART_HIT_LEGEND, iEmpty, -1);
	key(&t, XUI_KEY_LEFT);
	check_hit(&t, XUI_CHART_HIT_LEGEND, iHidden, -1);
	key_down(&t, XUI_KEY_SPACE, 0);
	key_down(&t, XUI_KEY_SPACE, 0);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "chart space waits for release");
	key_up(&t, XUI_KEY_SPACE);
	CHECK(xuiChartGetSeriesVisible(t.pWidget, iHidden), "space shows hidden legend series once");
	key_down(&t, XUI_KEY_ENTER, 0);
	key_down(&t, XUI_KEY_ENTER, 0);
	key_up(&t, XUI_KEY_ENTER);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "enter toggles legend once");
	key_down(&t, XUI_KEY_SPACE, 0);
	(void)xuiSetFocusWidget(t.pContext, t.pAfter);
	(void)xuiDispatchPendingEvents(t.pContext);
	focus(&t);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "chart blur cancels pending space");
	key_down(&t, XUI_KEY_SPACE, 0);
	(void)xuiWidgetSetEnabled(t.pWidget, 0);
	(void)xuiWidgetSetEnabled(t.pWidget, 1);
	focus(&t);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "chart disable cancels pending space");
	check_hit(&t, XUI_CHART_HIT_LEGEND, iHidden, -1);
	tItem = t.tHit.tRect;
	key_down(&t, XUI_KEY_SPACE, 0);
	pointer(&t, tItem, 1);
	key_up(&t, XUI_KEY_SPACE);
	pointer(&t, tItem, 0);
	CHECK(xuiChartGetSeriesVisible(t.pWidget, iHidden), "pointer takes over chart space with one toggle");
	key(&t, XUI_KEY_ENTER);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "keyboard resumes mouse legend target");
	key_down(&t, XUI_KEY_SPACE, 0);
	key(&t, XUI_KEY_ESCAPE);
	key_up(&t, XUI_KEY_SPACE);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iHidden), "escape cancels chart activation");
	key(&t, XUI_KEY_HOME);
	(void)xuiChartSetSeriesData(t.pWidget, iSeries, NULL, 0);
	key(&t, XUI_KEY_ENTER);
	check_hit(&t, XUI_CHART_HIT_LEGEND, iSeries, -1);
	(void)xuiWidgetSetEnabled(t.pWidget, 0);
	key(&t, XUI_KEY_ENTER);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iSeries), "disabled chart does not toggle legend");
	(void)xuiWidgetSetEnabled(t.pWidget, 1);
	(void)xuiChartClearSeries(t.pWidget);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "cleared chart has no tab stop");
	(void)xuiChartSetLegendVisible(t.pWidget, 0);
	for ( i = 0; i < 4; i++ ) {
		iType = arrTypes[i];
		(void)xuiChartClearSeries(t.pWidget);
		(void)xuiChartAddSeries(t.pWidget, iType, "Series", &iSeries);
		(void)xuiChartSetSeriesData(t.pWidget, iSeries,
			iType == XUI_CHART_SERIES_PIE ? arrPie : arrPoints, 3);
		focus(&t);
		key(&t, XUI_KEY_END);
		check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, iType == XUI_CHART_SERIES_PIE ? 1 : 2);
		memset(&tMouseHit, 0, sizeof(tMouseHit));
		CHECK(xuiChartHitTest(t.pWidget, t.tHit.fX, t.tHit.fY, &tMouseHit) == XUI_OK &&
			tMouseHit.iPart == XUI_CHART_HIT_SERIES && tMouseHit.iItem == t.tHit.iItem,
			"keyboard marker lies on the same mouse target for every chart type");
		CHECK(xuiRenderPrepare(t.pContext) == XUI_OK, "all chart types render keyboard target");
		if ( iType == XUI_CHART_SERIES_PIE ) continue;
		key(&t, XUI_KEY_PAGE_UP);
		CHECK(xuiChartGetViewRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) == 1,
			"page up zooms chart");
		fOldMinX = fMinX;
		fOldSpanX = fMaxX - fMinX;
		key_down(&t, XUI_KEY_LEFT, XUI_MOD_CTRL);
		key_up(&t, XUI_KEY_LEFT);
		(void)xuiChartGetViewRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY);
		CHECK(fMinX < fOldMinX && fabs(fMaxX - fMinX - fOldSpanX) < 1.0e-9,
			"ctrl arrow pans without changing zoom");
		key(&t, XUI_KEY_PAGE_DOWN);
		(void)xuiChartGetViewRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY);
		CHECK(fMaxX - fMinX > fOldSpanX, "page down zooms out");
		key_down(&t, XUI_KEY_HOME, XUI_MOD_CTRL);
		key_up(&t, XUI_KEY_HOME);
		CHECK(xuiChartGetViewRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) == 0,
			"ctrl home resets chart view");
		key(&t, XUI_KEY_HOME);
		key_down(&t, XUI_KEY_RIGHT, XUI_MOD_SHIFT);
		key_up(&t, XUI_KEY_RIGHT);
		CHECK(xuiChartGetBrushRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) == 1 && fMaxX > fMinX,
			"shift navigation creates chart brush");
		key(&t, XUI_KEY_ESCAPE);
		CHECK(xuiChartGetBrushRange(t.pWidget, &fMinX, &fMaxX, &fMinY, &fMaxY) == 0,
			"escape clears chart brush");
		key(&t, XUI_KEY_PAGE_UP);
		tItem = xuiChartGetPlotRect(t.pWidget);
		pointer(&t, tItem, 1);
		CHECK(xuiGetPointerCapture(t.pContext) == t.pWidget, "zoomed chart captures pan");
		(void)xuiChartSetSeriesData(t.pWidget, iSeries, NULL, 0);
		CHECK(xuiGetPointerCapture(t.pContext) == NULL && !xuiWidgetGetTabStop(t.pWidget),
			"removing chart data cancels pan and focus");
		pointer(&t, tItem, 0);
		(void)xuiChartResetViewRange(t.pWidget);
	}
	(void)xuiChartSetSeriesData(t.pWidget, iSeries, arrPie, 1);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "zero-only pie has no actionable data");
	for ( iDirection = XUI_CHART_BAR_VERTICAL; iDirection <= XUI_CHART_BAR_HORIZONTAL; iDirection++ ) {
		for ( iMode = XUI_CHART_BAR_GROUPED; iMode <= XUI_CHART_BAR_STACKED; iMode++ ) {
			(void)xuiChartClearSeries(t.pWidget);
			(void)xuiChartSetBarDirection(t.pWidget, iDirection);
			(void)xuiChartSetBarMode(t.pWidget, iMode);
			(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_BAR, "First", &iSeries);
			(void)xuiChartSetSeriesData(t.pWidget, iSeries, arrPoints, 3);
			(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_BAR, "Second", &iHidden);
			(void)xuiChartSetSeriesData(t.pWidget, iHidden, arrPoints, 3);
			focus(&t);
			key(&t, XUI_KEY_HOME);
			for ( i = 0; i < 3; i++ ) key(&t, XUI_KEY_RIGHT);
			check_hit(&t, XUI_CHART_HIT_SERIES, iHidden, 0);
			CHECK(xuiChartHitTest(t.pWidget, t.tHit.fX, t.tHit.fY, &tMouseHit) == XUI_OK &&
				tMouseHit.iSeries == iHidden && tMouseHit.iItem == 0,
				"grouped/stacked horizontal/vertical keyboard marker matches mouse");
			key(&t, XUI_KEY_END);
			check_hit(&t, XUI_CHART_HIT_SERIES, iHidden, 2);
			(void)xuiChartSetSeriesVisible(t.pWidget, iHidden, 0);
			check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 0);
		}
	}
	(void)xuiChartClearSeries(t.pWidget);
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_PIE, "Rose", &iSeries);
	(void)xuiChartSetSeriesData(t.pWidget, iSeries, arrPie, 3);
	(void)xuiChartSetPieMode(t.pWidget, XUI_CHART_PIE_ROSE);
	(void)xuiChartSetPieInnerRadius(t.pWidget, 0.4f);
	focus(&t);
	key(&t, XUI_KEY_END);
	check_hit(&t, XUI_CHART_HIT_SERIES, iSeries, 1);
	CHECK(xuiChartHitTest(t.pWidget, t.tHit.fX, t.tHit.fY, &tMouseHit) == XUI_OK &&
		tMouseHit.iSeries == iSeries && tMouseHit.iItem == 1, "rose donut keyboard marker matches mouse");
	key(&t, XUI_KEY_TAB);
	CHECK(xuiGetFocusWidget(t.pContext) == t.pAfter, "chart exposes exactly one tab stop");
	(void)xuiChartSetSeriesVisible(t.pWidget, iSeries, 0);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "hidden data without legend removes chart tab stop");
	(void)xuiChartSetSeriesVisible(t.pWidget, iSeries, 1);
	focus(&t);
	(void)xuiWidgetSetEnabled(t.pRoot, 0);
	key(&t, XUI_KEY_ENTER);
	CHECK(xuiGetFocusWidget(t.pContext) == NULL, "disabled ancestor prevents chart keyboard focus");
	cleanup(&t);
}

static void test_chart_presentation_mouse(void)
{
	keyboard_test_t t;
	xui_chart_point_t arrPoints[] = {{0, 2, 2, "A", 0}, {1, 4, 4, "B", 0}, {2, 3, 3, "C", 0}};
	xui_vec2_t tPixel;
	xui_rect_t tPlot, tWorld, tItem;
	double x0, x1, y0, y1, fOldSpan, fOldMin;
	int iSeries, i;
	int iCircles;
	if ( !setup(&t, 1) ) { CHECK(0, "presentation mouse setup"); cleanup(&t); return; }
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_LINE, "Line", &iSeries);
	(void)xuiChartSetSeriesData(t.pWidget, iSeries, arrPoints, 3);
	(void)xuiChartSetLegendVisible(t.pWidget, 0);
	(void)xuiChartSetTooltipVisible(t.pWidget, 0);
	(void)xuiChartSetContextMenu(t.pWidget, NULL, NULL);
	CHECK(!xuiWidgetGetTabStop(t.pWidget), "presentation configuration opts out of automatic tab entry only");
	(void)xuiRenderPrepare(t.pContext);
	(void)xuiChartDataToPixel(t.pWidget, 1, 4, &tPixel);
	tItem = (xui_rect_t){(int)tPixel.fX - 1, (int)tPixel.fY - 1, 2, 2};
	pointer(&t, tItem, 1);
	pointer(&t, tItem, 0);
	iCircles = g_iCircles;
	(void)xuiRenderPrepare(t.pContext);
	CHECK(g_iCircles == iCircles + 1 && fabsf(g_fCircleX - tPixel.fX) < 1.0f &&
		fabsf(g_fCircleY - tPixel.fY) < 1.0f, "all presentation options off still allow mouse selection");
	(void)xuiChartSetTooltipVisible(t.pWidget, 0);
	iCircles = g_iCircles;
	(void)xuiRenderPrepare(t.pContext);
	CHECK(g_iCircles == iCircles + 1 && fabsf(g_fCircleX - tPixel.fX) < 1.0f,
		"presentation setter preserves valid pointer selection");
	tPlot = xuiChartGetPlotRect(t.pWidget);
	tWorld = xuiWidgetGetWorldRect(t.pWidget);
	(void)xuiChartGetViewRange(t.pWidget, &x0, &x1, &y0, &y1);
	fOldSpan = x1 - x0;
	CHECK(xuiInputPointerWheel(t.pContext, tWorld.fX + tPlot.fX + tPlot.fW / 2,
		tWorld.fY + tPlot.fY + tPlot.fH / 2, 0.0f, 1.0f, 0) == XUI_OK, "presentation wheel");
	(void)xuiDispatchPendingEvents(t.pContext);
	CHECK(xuiChartGetViewRange(t.pWidget, &x0, &x1, &y0, &y1) == 1 && x1 - x0 < fOldSpan,
		"all presentation options off still allow wheel zoom");
	fOldMin = x0;
	pointer(&t, tPlot, 1);
	CHECK(xuiGetPointerCapture(t.pContext) == t.pWidget, "presentation mouse pan captures pointer");
	(void)xuiInputPointerMove(t.pContext, tWorld.fX + tPlot.fX + tPlot.fW * 3 / 4,
		tWorld.fY + tPlot.fY + tPlot.fH / 2, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(t.pContext);
	pointer(&t, tPlot, 0);
	(void)xuiChartGetViewRange(t.pWidget, &x0, &x1, &y0, &y1);
	CHECK(x0 < fOldMin, "all presentation options off still allow mouse pan");
	(void)xuiChartResetViewRange(t.pWidget);
	(void)xuiInputSetModifiers(t.pContext, XUI_MOD_SHIFT);
	tItem = (xui_rect_t){tPlot.fX + tPlot.fW / 4, tPlot.fY + tPlot.fH / 4, 2, 2};
	pointer(&t, tItem, 1);
	tItem.fX += tPlot.fW / 2;
	tItem.fY += tPlot.fH / 2;
	(void)xuiInputPointerMove(t.pContext, tWorld.fX + tItem.fX + 1, tWorld.fY + tItem.fY + 1, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(t.pContext);
	pointer(&t, tItem, 0);
	(void)xuiInputSetModifiers(t.pContext, 0);
	CHECK(xuiChartGetBrushRange(t.pWidget, &x0, &x1, &y0, &y1) == 1 && x1 > x0 && y1 > y0,
		"all presentation options off still allow mouse brush");
	(void)xuiWidgetSetFocusable(t.pWidget, 1);
	(void)xuiWidgetSetTabStop(t.pWidget, 1);
	focus(&t);
	key(&t, XUI_KEY_ESCAPE);
	CHECK(xuiChartGetBrushRange(t.pWidget, &x0, &x1, &y0, &y1) == 0,
		"explicit focus opt-in keeps keyboard equivalence without presentation features");
	(void)xuiChartSetViewRange(t.pWidget, -1234.125, 9876.875, 12.5, 4096.25);
	for ( i = 0; i < 1000; i++ ) {
		key(&t, XUI_KEY_PAGE_UP);
		if ( i == 0 ) {
			(void)xuiChartGetViewRange(t.pWidget, &x0, &x1, &y0, &y1);
			CHECK(fabs(x1 - x0 - 11111.0 * 0.8) < 1.0e-8, "page zoom actually changes the view before round trip");
		}
		key(&t, XUI_KEY_PAGE_DOWN);
	}
	(void)xuiChartGetViewRange(t.pWidget, &x0, &x1, &y0, &y1);
	CHECK(fabs(x0 + 1234.125) < 1.0e-8 && fabs(x1 - 9876.875) < 1.0e-8 &&
		fabs(y0 - 12.5) < 1.0e-8 && fabs(y1 - 4096.25) < 1.0e-8, "page zoom round trips retain double precision");
	(void)xuiChartClearSeries(t.pWidget);
	(void)xuiChartSetLegendVisible(t.pWidget, 1);
	(void)xuiChartAddSeries(t.pWidget, XUI_CHART_SERIES_LINE, "Empty", &iSeries);
	(void)xuiRenderPrepare(t.pContext);
	tItem = (xui_rect_t){312, 36, 106, 18};
	pointer(&t, tItem, 1);
	pointer(&t, tItem, 0);
	CHECK(!xuiChartGetSeriesVisible(t.pWidget, iSeries), "zero-data legend remains mouse clickable");
	focus(&t);
	key(&t, XUI_KEY_ENTER);
	CHECK(xuiChartGetSeriesVisible(t.pWidget, iSeries), "zero-data legend retains keyboard equivalent");
	cleanup(&t);
}

int main(void)
{
	test_breadcrumb();
	test_chart();
	test_chart_presentation_mouse();
	printf("chart/breadcrumb keyboard: %d checks, %d failures\n", g_iChecks, g_iFailures);
	return g_iFailures ? 1 : 0;
}
