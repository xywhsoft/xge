#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_chart_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int g_iTooltipCalls = 0;
static int (*g_pChartCircleFill)(xui_proxy, xui_draw_context, float, float, float, uint32_t) = NULL;
static uint32_t g_iChartLodColor = 0;
static float g_fChartLodMinY = FLT_MAX;
static int g_iChartLodCircles = 0;

static int __xuiChartRecordCircle(xui_proxy pProxy, xui_draw_context pDraw,
	float fX, float fY, float fRadius, uint32_t iColor)
{
	if ( iColor == g_iChartLodColor ) {
		if ( fY < g_fChartLodMinY ) g_fChartLodMinY = fY;
		g_iChartLodCircles++;
	}
	return g_pChartCircleFill != NULL ?
		g_pChartCircleFill(pProxy, pDraw, fX, fY, fRadius, iColor) : XUI_OK;
}

static int __xuiChartTooltip(xui_widget pWidget, int iSeries, int iItem, char* sBuffer, int iCapacity, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iTooltipCalls++;
	if ( (sBuffer == NULL) || (iCapacity <= 0) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sBuffer, (size_t)iCapacity, "custom:%d:%d", iSeries, iItem);
	return XUI_OK;
}

static int __xuiChartDestroyTooltip(xui_widget pWidget, int iSeries, int iItem, char* sBuffer, int iCapacity, void* pUser)
{
	xui_widget* ppChart = (xui_widget*)pUser;

	(void)iSeries;
	(void)iItem;
	if ( (sBuffer == NULL) || (iCapacity <= 0) || (ppChart == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	snprintf(sBuffer, (size_t)iCapacity, "destroy");
	xuiWidgetDestroy(pWidget);
	if ( *ppChart == pWidget ) *ppChart = NULL;
	return XUI_OK;
}

static int __xuiChartSameValueSettersStayClean(xui_widget pChart, xui_font pFont,
	int iLine, int iPie, const xui_chart_point_t* pLine, int iLineCount)
{
	xui_thickness_t tPadding;
	uint32_t iBackground;
	uint32_t iPlot;
	uint32_t iGrid;
	uint32_t iAxis;
	uint32_t iText;
	uint32_t iTooltip;
	uint32_t iTooltipText;
	uint32_t iAreaColor;
	float arrDash[8];
	float fDuration;
	float fProgress;
	int bAnimation;
	int bArea;
	int iDashCount;

	if ( xuiChartGetDirtyFlags(pChart) != 0u ||
	     xuiChartGetColors(pChart, &iBackground, &iPlot, &iGrid, &iAxis,
		&iText, &iTooltip, &iTooltipText) != XUI_OK ) return 0;
	tPadding = xuiChartGetPadding(pChart);
	(void)xuiChartGetAnimation(pChart, &bAnimation, &fDuration, &fProgress);
	bArea = xuiChartGetSeriesAreaFill(pChart, iLine, &iAreaColor);
	iDashCount = xuiChartGetSeriesDash(pChart, iLine, arrDash,
		(int)(sizeof(arrDash) / sizeof(arrDash[0])));
	if ( iDashCount <= 0 ) return 0;

	if ( xuiChartSetTitle(pChart, xuiChartGetTitle(pChart)) != XUI_OK ||
	     xuiChartSetFont(pChart, pFont) != XUI_OK ||
	     xuiChartSetColors(pChart, iBackground, iPlot, iGrid, iAxis, iText,
		iTooltip, iTooltipText) != XUI_OK ||
	     xuiChartSetXAxis(pChart, xuiChartGetXAxis(pChart)) != XUI_OK ||
	     xuiChartSetYAxis(pChart, xuiChartGetYAxis(pChart)) != XUI_OK ||
	     xuiChartSetBarMode(pChart, xuiChartGetBarMode(pChart)) != XUI_OK ||
	     xuiChartSetBarDirection(pChart, xuiChartGetBarDirection(pChart)) != XUI_OK ||
	     xuiChartSetLegendVisible(pChart, xuiChartGetLegendVisible(pChart)) != XUI_OK ||
	     xuiChartSetTooltipVisible(pChart, xuiChartGetTooltipVisible(pChart)) != XUI_OK ||
	     xuiChartSetTooltipCallback(pChart, __xuiChartTooltip, NULL) != XUI_OK ||
	     xuiChartSetContextMenu(pChart, NULL, NULL) != XUI_OK ||
	     xuiChartSetPadding(pChart, tPadding) != XUI_OK ||
	     xuiChartResetViewRange(pChart) != XUI_OK ||
	     xuiChartClearBrushRange(pChart) != XUI_OK ||
	     xuiChartSetLodThreshold(pChart, xuiChartGetLodThreshold(pChart)) != XUI_OK ||
	     xuiChartSetAnimation(pChart, bAnimation, fDuration) != XUI_OK ||
	     xuiChartSetSeriesData(pChart, iLine, pLine, iLineCount) != XUI_OK ||
	     xuiChartSetSeriesColor(pChart, iLine, xuiChartGetSeriesColor(pChart, iLine)) != XUI_OK ||
	     xuiChartSetSeriesAreaFill(pChart, iLine, bArea, iAreaColor) != XUI_OK ||
	     xuiChartSetSeriesSmooth(pChart, iLine, xuiChartGetSeriesSmooth(pChart, iLine)) != XUI_OK ||
	     xuiChartSetSeriesDash(pChart, iLine, arrDash, iDashCount) != XUI_OK ||
	     xuiChartSetSeriesVisible(pChart, iPie, xuiChartGetSeriesVisible(pChart, iPie)) != XUI_OK ||
	     xuiChartSetSeriesSymbol(pChart, iLine, xuiChartGetSeriesSymbol(pChart, iLine)) != XUI_OK ||
	     xuiChartSetSeriesSymbolSize(pChart, iLine, xuiChartGetSeriesSymbolSize(pChart, iLine)) != XUI_OK ||
	     xuiChartSetPieMode(pChart, xuiChartGetPieMode(pChart)) != XUI_OK ||
	     xuiChartSetPieInnerRadius(pChart, xuiChartGetPieInnerRadius(pChart)) != XUI_OK ) {
		return 0;
	}
	return xuiChartGetDirtyFlags(pChart) == 0u;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_test_proxy_state_t tNoVectorState;
	xui_context pContext;
	xui_context pNoVectorContext;
	xui_widget pRoot;
	xui_widget pChart;
	xui_surface pCache;
	xui_font pFont;
	xui_painter pPainter;
	xui_path pPath;
	xui_chart_desc_t tDesc;
	xui_path_style_t tPathStyle;
	xui_style_property_t arrStyle[2];
	xui_mesh_vertex_t arrMeshVertices[4];
	uint32_t arrMeshIndices[6];
	xui_path_command_t tPathCommand;
	float arrDashPattern[2];
	float arrChartDash[2];
	char sMutableLabel[32];
	xui_chart_point_t arrLine[4];
	xui_chart_point_t arrExtreme[2];
	xui_chart_point_t arrLineLarge[32];
	xui_chart_point_t arrBarA[3];
	xui_chart_point_t arrBarB[3];
	xui_chart_point_t arrPie[3];
	xui_chart_point_t arrPieB[3];
	xui_chart_point_t arrScatter[3];
	xui_chart_hit_t tHit;
	xui_event_t tEvent;
	xui_accessible_node_t tAccessible;
	xui_rect_t tPlot;
	xui_rect_t tPlotPadded;
	xui_thickness_t tPadding;
	xui_vec2_t tPixel;
	xui_vec2_t tMeasured;
	double fDataX;
	double fDataY;
	double fMinX;
	double fMaxX;
	double fMinY;
	double fMaxY;
	double fAutoMinX;
	double fAutoMaxX;
	double fAutoMinY;
	double fAutoMaxY;
	double fPanMinX;
	double fOwnerMinX;
	double fOwnerMaxX;
	double fOwnerMinY;
	double fOwnerMaxY;
	float fMinSize;
	float fMaxSize;
	float fAnimDuration;
	float fAnimProgress;
	uint32_t iMinColor;
	uint32_t iMaxColor;
	uint32_t iAreaColor;
	int iLine;
	int iBarA;
	int iBarB;
	int iPie;
	int iPieB;
	int iScatter;
	int iUtf8Series;
	int bAnimEnabled;
	int iMeshBase;
	int iPathBase;
	int i;
	int iFailed;
	int iRet;

	pContext = NULL;
	pNoVectorContext = NULL;
	pRoot = NULL;
	pChart = NULL;
	pFont = NULL;
	pPainter = NULL;
	pPath = NULL;
	fMinX = 0.0;
	fMaxX = 0.0;
	fMinY = 0.0;
	fMaxY = 0.0;
	fAutoMinX = 0.0;
	fAutoMaxX = 0.0;
	fAutoMinY = 0.0;
	fAutoMaxY = 0.0;
	fPanMinX = 0.0;
	fMinSize = 0.0f;
	fMaxSize = 0.0f;
	fAnimDuration = 0.0f;
	fAnimProgress = 0.0f;
	iMinColor = 0;
	iMaxColor = 0;
	iAreaColor = 0;
	iMeshBase = 0;
	iPathBase = 0;
	iFailed = 0;
	memset(arrLine, 0, sizeof(arrLine));
	memset(arrExtreme, 0, sizeof(arrExtreme));
	memset(arrMeshVertices, 0, sizeof(arrMeshVertices));
	memset(arrMeshIndices, 0, sizeof(arrMeshIndices));
	memset(arrStyle, 0, sizeof(arrStyle));
	arrDashPattern[0] = 8.0f;
	arrDashPattern[1] = 6.0f;
	arrChartDash[0] = 10.0f;
	arrChartDash[1] = 5.0f;
	memset(arrLineLarge, 0, sizeof(arrLineLarge));
	memset(arrBarA, 0, sizeof(arrBarA));
	memset(arrBarB, 0, sizeof(arrBarB));
	memset(arrPie, 0, sizeof(arrPie));
	memset(arrPieB, 0, sizeof(arrPieB));
	memset(arrScatter, 0, sizeof(arrScatter));
	strcpy(sMutableLabel, "OwnedLabel");
	xuiTestProxyInit(&tState);
	xuiTestProxyInit(&tNoVectorState);
	g_pChartCircleFill = tState.tProxy.drawCircleFill;
	tState.tProxy.drawCircleFill = __xuiChartRecordCircle;
	tNoVectorState.tProxy.drawPath = NULL;

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 420.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 420.0f, 260.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.sTitle = "Chart Test";
	iRet = xuiChartCreate(pContext, &pChart, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pChart != NULL, "chart create");
	iRet = xuiWidgetSetRect(pChart, (xui_rect_t){0.0f, 0.0f, 420.0f, 260.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "chart rect");
	iRet = xuiWidgetAddChild(pRoot, pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "chart add");
	iRet = xuiWidgetMeasureContent(pChart,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 320.0f && tMeasured.fY == 220.0f,
		"chart intrinsic content measure");
	iRet = xuiWidgetMeasureContent(pChart, (xui_vec2_t){640.0f, 440.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 320.0f && tMeasured.fY == 220.0f,
		"roomy constraints do not stretch chart measure");
	iRet = xuiWidgetMeasureContent(pChart, (xui_vec2_t){96.0f, 72.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 96.0f && tMeasured.fY == 72.0f,
		"tight constraints clamp chart measure");

	XUI_TEST_CHECK(strcmp(xuiChartGetTitle(pChart), "Chart Test") == 0, "title");
	iRet = xuiChartSetTitle(pChart, "");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetTitle(pChart) != NULL && xuiChartGetTitle(pChart)[0] == '\0', "empty title accepted");
	iRet = xuiChartSetTitle(pChart, "Chart Test");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiChartGetTitle(pChart), "Chart Test") == 0, "title restore");
	XUI_TEST_CHECK(xuiChartGetSeriesCount(pChart) == 0, "empty series");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_LINE, "Line", &iLine);
	XUI_TEST_CHECK(iRet == XUI_OK && iLine == 0, "line add");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_PIE, "Pie", &iPie);
	XUI_TEST_CHECK(iRet == XUI_OK && iPie == 1, "pie add");
	XUI_TEST_CHECK(xuiChartGetSeriesCount(pChart) == 2, "series count");

	arrLine[0].x = 0.0; arrLine[0].y = 3.0; arrLine[0].value = 3.0; arrLine[0].label = "A";
	arrLine[1].x = 1.0; arrLine[1].y = 7.0; arrLine[1].value = 7.0; arrLine[1].label = "B";
	arrLine[2].x = 2.0; arrLine[2].y = 5.0; arrLine[2].value = 5.0; arrLine[2].label = "C";
	arrLine[3].x = 3.0; arrLine[3].y = 9.0; arrLine[3].value = 9.0; arrLine[3].label = "D";
	for ( i = 0; i < 32; i++ ) {
		arrLineLarge[i].x = (double)i;
		arrLineLarge[i].y = 1.0;
		arrLineLarge[i].value = arrLineLarge[i].y;
	}
	arrLineLarge[5].y = 100.0;
	arrLineLarge[5].value = 100.0;
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "line data");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pChart) == 7,
	               "chart accessibility exposes root, points, and legend");
	iRet = xuiWidgetGetAccessibleNode(pChart, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST_ITEM &&
	               strcmp(tAccessible.sName, "A") == 0 && tAccessible.tRange.fValue == 3.0,
	               "chart point accessibility metadata");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pChart, 5, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX &&
	               (tAccessible.iState & XUI_ACCESSIBLE_STATE_CHECKED) != 0,
	               "chart legend accessibility metadata");
	arrLine[0].y = NAN;
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite chart data rejected");
	arrLine[0].y = 3.0;
	iRet = xuiChartSetSeriesVisible(pChart, iPie, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiChartGetSeriesVisible(pChart, iPie), "hide pie");
	iRet = xuiChartSetSeriesSymbol(pChart, iLine, XUI_CHART_SYMBOL_DIAMOND);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesSymbol(pChart, iLine) == XUI_CHART_SYMBOL_DIAMOND, "symbol");
	iRet = xuiChartSetSeriesSymbolSize(pChart, iLine, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesSymbolSize(pChart, iLine) == 10.0f, "symbol size");
	iRet = xuiChartSetSeriesSymbolSize(pChart, iLine, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetSeriesSymbolSize(pChart, iLine) == 10.0f, "non-finite symbol size rejected transactionally");
	iRet = xuiChartSetSeriesAreaFill(pChart, iLine, 1, XUI_COLOR_RGBA(42, 124, 221, 48));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesAreaFill(pChart, iLine, &iAreaColor) == 1 && iAreaColor == XUI_COLOR_RGBA(42, 124, 221, 48), "area fill set");
	iRet = xuiChartSetSeriesSmooth(pChart, iLine, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesSmooth(pChart, iLine) == 1, "smooth line set");
	iRet = xuiChartSetSeriesDash(pChart, iLine, arrChartDash, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesDash(pChart, iLine, arrDashPattern, 2) == 2 && arrDashPattern[0] == 10.0f && arrDashPattern[1] == 5.0f, "line dash set");
	arrChartDash[1] = NAN;
	iRet = xuiChartSetSeriesDash(pChart, iLine, arrChartDash, 2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetSeriesDash(pChart, iLine, arrDashPattern, 2) == 2 && arrDashPattern[0] == 10.0f && arrDashPattern[1] == 5.0f, "non-finite dash rejected transactionally");
	arrChartDash[1] = 5.0f;
	iRet = xuiChartSetTooltipCallback(pChart, __xuiChartTooltip, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "tooltip callback");
	XUI_TEST_CHECK((xuiChartGetDirtyFlags(pChart) & XUI_CHART_DIRTY_PLOT) != 0, "dirty before render");
	arrStyle[0].iSize = sizeof(arrStyle[0]);
	arrStyle[0].sName = "chart.background.color";
	arrStyle[0].tValue.iSize = sizeof(arrStyle[0].tValue);
	arrStyle[0].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[0].tValue.iColor = XUI_COLOR_RGBA(12, 34, 56, 255);
	arrStyle[1].iSize = sizeof(arrStyle[1]);
	arrStyle[1].sName = "chart.plot.color";
	arrStyle[1].tValue.iSize = sizeof(arrStyle[1].tValue);
	arrStyle[1].tValue.iType = XUI_STYLE_VALUE_COLOR;
	arrStyle[1].tValue.iColor = XUI_COLOR_RGBA(250, 251, 252, 255);
	iRet = xuiWidgetSetInlineStyle(pChart, arrStyle, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "chart inline style");

	tPlot = xuiChartGetPlotRect(pChart);
	XUI_TEST_CHECK(tPlot.fW > 100.0f && tPlot.fH > 100.0f, "plot rect");
	iRet = xuiWidgetSetRect(pChart, (xui_rect_t){0, 0, 20, 16});
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny chart rect");
	tPlotPadded = xuiChartGetPlotRect(pChart);
	XUI_TEST_CHECK(tPlotPadded.fX >= 0 && tPlotPadded.fY >= 0 &&
		tPlotPadded.fX + tPlotPadded.fW <= 20 && tPlotPadded.fY + tPlotPadded.fH <= 16,
		"tiny plot remains inside chart bounds");
	iRet = xuiWidgetSetRect(pChart, (xui_rect_t){0, 0, 420, 260});
	XUI_TEST_CHECK(iRet == XUI_OK, "chart rect restore");
	tPlot = xuiChartGetPlotRect(pChart);
	iRet = xuiChartDataToPixel(pChart, 1.0, 7.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "data to pixel");
	iRet = xuiChartPixelToData(pChart, tPixel.fX, tPixel.fY, &fDataX, &fDataY);
	XUI_TEST_CHECK(iRet == XUI_OK && fDataX > 0.99 && fDataX < 1.01 && fDataY > 6.99 && fDataY < 7.01, "pixel to data");
	iRet = xuiChartDataToPixel(pChart, NAN, 7.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite data coordinate rejected");
	iRet = xuiChartPixelToData(pChart, INFINITY, 20.0f, &fDataX, &fDataY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite pixel coordinate rejected");
	iRet = xuiChartHitTest(pChart, 20.0f, NAN, &tHit);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite hit coordinate rejected");
	iRet = xuiChartSetViewRange(pChart, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "full finite double view range");
	iRet = xuiChartDataToPixel(pChart, 0.0, 0.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(tPixel.fX) && isfinite(tPixel.fY),
		"extreme view range maps to finite pixels");
	iRet = xuiChartPixelToData(pChart, tPixel.fX, tPixel.fY, &fDataX, &fDataY);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(fDataX) && isfinite(fDataY),
		"extreme view range unmaps to finite data");
	iRet = xuiChartResetViewRange(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme view range reset");
	arrExtreme[0].x = -DBL_MAX; arrExtreme[0].y = -DBL_MAX; arrExtreme[0].value = -DBL_MAX;
	arrExtreme[1].x = DBL_MAX; arrExtreme[1].y = DBL_MAX; arrExtreme[1].value = DBL_MAX;
	iRet = xuiChartSetXAxis(pChart, XUI_CHART_AXIS_VALUE);
	if ( iRet == XUI_OK ) iRet = xuiChartSetSeriesData(pChart, iLine, arrExtreme, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme finite series data");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 0 && isfinite(fMinX) && isfinite(fMaxX) &&
		isfinite(fMinY) && isfinite(fMaxY) && fMinX < fMaxX && fMinY < fMaxY,
		"extreme series computes finite ordered ranges");
	iRet = xuiChartDataToPixel(pChart, DBL_MAX, DBL_MAX, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(tPixel.fX) && isfinite(tPixel.fY),
		"extreme series endpoint maps to finite pixels");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme finite series render");
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	if ( iRet == XUI_OK ) iRet = xuiChartSetXAxis(pChart, XUI_CHART_AXIS_CATEGORY);
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme series restore");
	iRet = xuiChartDataToPixel(pChart, 1.0, 7.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "data to pixel restore");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	XUI_TEST_CHECK(xuiChartGetDirtyFlags(pChart) == 0, "dirty cleared after render");
	XUI_TEST_CHECK(__xuiChartSameValueSettersStayClean(pChart, pFont, iLine, iPie,
		arrLine, (int)(sizeof(arrLine) / sizeof(arrLine[0]))),
		"same-value chart setters keep the render cache clean");
	pCache = xuiWidgetGetCacheSurface(pChart, xuiWidgetGetStateId(pChart));
	XUI_TEST_CHECK(pCache != NULL, "cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pCache) >= 1, "background draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(12, 34, 56, 255)) > 0, "style background draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillColorCount(pCache, XUI_COLOR_RGBA(250, 251, 252, 255)) > 0, "style plot draw");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) >= 1, "plot draw");
	XUI_TEST_CHECK(xuiTestProxyGetPathDrawCount(&tState) >= 2, "area and smooth ShapeEx path draw");
	iMeshBase = xuiTestProxyGetMeshDrawCount(&tState);
	arrMeshVertices[0] = (xui_mesh_vertex_t){0.0f, 0.0f, XUI_COLOR_RGBA(255, 0, 0, 255)};
	arrMeshVertices[1] = (xui_mesh_vertex_t){16.0f, 0.0f, XUI_COLOR_RGBA(0, 255, 0, 255)};
	arrMeshVertices[2] = (xui_mesh_vertex_t){0.0f, 16.0f, XUI_COLOR_RGBA(0, 0, 255, 255)};
	arrMeshVertices[3] = (xui_mesh_vertex_t){16.0f, 16.0f, XUI_COLOR_RGBA(255, 255, 255, 255)};
	arrMeshIndices[0] = 0; arrMeshIndices[1] = 1; arrMeshIndices[2] = 2;
	arrMeshIndices[3] = 2; arrMeshIndices[4] = 1; arrMeshIndices[5] = 3;
	iRet = xuiPainterBegin(pContext, pCache, &pPainter);
	XUI_TEST_CHECK(iRet == XUI_OK && pPainter != NULL, "mesh painter begin");
	iRet = xuiPainterDrawMeshTriangles(pPainter, arrMeshVertices, 4, arrMeshIndices, 6, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "mesh painter draw");
	iRet = xuiPainterEnd(pPainter);
	pPainter = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "mesh painter end");
	XUI_TEST_CHECK(xuiTestProxyGetMeshDrawCount(&tState) == iMeshBase + 1, "mesh draw count");
	XUI_TEST_CHECK(xuiTestProxyGetLastMeshVertexCount(&tState) == 4, "mesh vertex count");
	XUI_TEST_CHECK(xuiTestProxyGetLastMeshIndexCount(&tState) == 6, "mesh index count");
	iPathBase = xuiTestProxyGetPathDrawCount(&tState);
	iRet = xuiPathCreate(&pPath);
	XUI_TEST_CHECK(iRet == XUI_OK && pPath != NULL, "path create");
	iRet = xuiPathMoveTo(pPath, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path move");
	iRet = xuiPathLineTo(pPath, 40.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path line");
	iRet = xuiPathQuadTo(pPath, 60.0f, 20.0f, 40.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path quad");
	iRet = xuiPathCubicTo(pPath, 24.0f, 54.0f, 8.0f, 54.0f, 0.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path cubic");
	iRet = xuiPathClose(pPath);
	XUI_TEST_CHECK(iRet == XUI_OK, "path close");
	XUI_TEST_CHECK(xuiPathGetCommandCount(pPath) == 5, "path command count");
	iRet = xuiPathGetCommand(pPath, 2, &tPathCommand);
	XUI_TEST_CHECK(iRet == XUI_OK && tPathCommand.iCommand == XUI_PATH_CMD_QUAD, "path command get");
	iRet = xuiPainterBegin(pContext, pCache, &pPainter);
	XUI_TEST_CHECK(iRet == XUI_OK && pPainter != NULL, "path fill painter begin");
	iRet = xuiPainterFillPath(pPainter, pPath, XUI_COLOR_RGBA(80, 120, 220, 255), 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path fill painter");
	memset(&tPathStyle, 0, sizeof(tPathStyle));
	tPathStyle.iSize = sizeof(tPathStyle);
	tPathStyle.iFillColor = XUI_COLOR_RGBA(40, 160, 120, 255);
	tPathStyle.iFillRule = XUI_PATH_FILL_NON_ZERO;
	tPathStyle.iLineJoin = XUI_PATH_JOIN_MITER;
	tPathStyle.iLineCap = XUI_PATH_CAP_BUTT;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw fill style");
	tPathStyle.iFillColor = 0;
	tPathStyle.iStrokeColor = XUI_COLOR_RGBA(20, 60, 120, 255);
	tPathStyle.fStrokeWidth = 4.0f;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw stroke style");
	tPathStyle.pDashPattern = arrDashPattern;
	tPathStyle.iDashCount = 2;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw dashed stroke style");
	tPathStyle.pDashPattern = NULL;
	tPathStyle.iDashCount = 0;
	tPathStyle.iLineCap = XUI_PATH_CAP_SQUARE;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw square cap stroke style");
	tPathStyle.iLineCap = XUI_PATH_CAP_ROUND;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw round cap stroke style");
	tPathStyle.iLineCap = XUI_PATH_CAP_BUTT;
	tPathStyle.iLineJoin = XUI_PATH_JOIN_BEVEL;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw bevel join stroke style");
	tPathStyle.iLineJoin = XUI_PATH_JOIN_ROUND;
	iRet = xuiPainterDrawPath(pPainter, pPath, &tPathStyle, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "path draw round join stroke style");
	iRet = xuiPainterEnd(pPainter);
	pPainter = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK, "path fill painter end");
	XUI_TEST_CHECK(xuiTestProxyGetPathDrawCount(&tState) == iPathBase + 8, "ShapeEx path draw count");
	XUI_TEST_CHECK(xuiTestProxyGetLastPathCommandCount(&tState) == 5, "ShapeEx path command forwarding");
	iRet = xuiCreate(&pNoVectorContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pNoVectorContext != NULL, "missing vector backend context create");
	iRet = xuiSetProxy(pNoVectorContext, &tNoVectorState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "missing vector backend rejected");
	xuiDestroy(pNoVectorContext);
	pNoVectorContext = NULL;
	xuiPathDestroy(pPath);
	pPath = NULL;
	tPadding = (xui_thickness_t){12.0f, 8.0f, 10.0f, 6.0f};
	iRet = xuiChartSetPadding(pChart, tPadding);
	XUI_TEST_CHECK(iRet == XUI_OK, "set padding");
	tPadding = xuiChartGetPadding(pChart);
	XUI_TEST_CHECK(tPadding.fLeft == 12.0f && tPadding.fTop == 8.0f && tPadding.fRight == 10.0f && tPadding.fBottom == 6.0f, "get padding");
	iRet = xuiChartSetPadding(pChart, (xui_thickness_t){NAN, 8.0f, 10.0f, 6.0f});
	tPadding = xuiChartGetPadding(pChart);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && tPadding.fLeft == 12.0f && tPadding.fTop == 8.0f && tPadding.fRight == 10.0f && tPadding.fBottom == 6.0f, "non-finite padding rejected transactionally");
	XUI_TEST_CHECK((xuiChartGetDirtyFlags(pChart) & XUI_CHART_DIRTY_STATIC) != 0, "padding dirty static");
	tPlotPadded = xuiChartGetPlotRect(pChart);
	XUI_TEST_CHECK(tPlotPadded.fX > tPlot.fX && tPlotPadded.fW < tPlot.fW, "padding plot effect");
	iRet = xuiChartSetPadding(pChart, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "clear padding");

	memset(&tHit, 0, sizeof(tHit));
	iRet = xuiChartHitTest(pChart, tPixel.fX, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iLine && tHit.iItem == 1, "line hit");
	iRet = xuiChartHitTest(pChart, 320.0f, 36.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_LEGEND && tHit.iSeries == 0, "legend hit");
	iRet = xuiChartHitTest(pChart, 418.0f, 36.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_NONE,
		"legend right edge follows half-open bounds");
	iRet = xuiWidgetSetVisible(pChart, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide chart");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_CLICK;
	tEvent.pTarget = pChart;
	tEvent.fX = 320;
	tEvent.fY = 36;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iClickCount = 1;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesVisible(pChart, iLine),
		"hidden chart ignores direct pointer event");
	iRet = xuiWidgetSetVisible(pChart, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore chart visibility");
	iRet = xuiInputPointerMove(pContext, tPixel.fX, tPixel.fY, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch hover");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && g_iTooltipCalls > 0, "custom tooltip render");
	XUI_TEST_CHECK(tState.iDrawClipGetCount > 0 && tState.iDrawClipSetCount > 0 &&
	               tState.iDrawClipClearCount > 0 && !tState.bLastDrawClipActive,
	               "plot clip applied and restored");
	iRet = xuiChartSetTooltipCallback(pChart, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "default tooltip callback");
	arrLine[1].label = sMutableLabel;
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutable label data");
	strcpy(sMutableLabel, "Mutated");
	pCache = xuiWidgetGetCacheSurface(pChart, xuiWidgetGetStateId(pChart));
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetLastText(pCache) != NULL && strstr(xuiTestSurfaceGetLastText(pCache), "OwnedLabel") != NULL, "series labels are owned copies");
	arrLine[1].label = "B";
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "line data restore after ownership test");
	iRet = xuiChartSetTooltipCallback(pChart, __xuiChartTooltip, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "custom tooltip restore");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 0 && fMaxX > fMinX && fMaxY > fMinY, "auto view range");
	fAutoMinX = fMinX;
	fAutoMaxX = fMaxX;
	fAutoMinY = fMinY;
	fAutoMaxY = fMaxY;
	iRet = xuiChartSetViewRange(pChart, NAN, 5.0, 0.0, 12.0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY) == 0, "non-finite view range rejected transactionally");
	iRet = xuiChartSetViewRange(pChart, -1.0, 5.0, 0.0, 12.0);
	XUI_TEST_CHECK(iRet == XUI_OK, "set view range");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX == -1.0 && fMaxX == 5.0 && fMinY == 0.0 && fMaxY == 12.0, "get manual view range");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"manual view range render clears dirty state");
	iRet = xuiChartSetViewRange(pChart, -1.0, 5.0, 0.0, 12.0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"same manual view range stays clean");
	iRet = xuiChartResetViewRange(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY) == 0, "reset view range");
	tPlot = xuiChartGetPlotRect(pChart);
	iRet = xuiInputPointerDown(pContext, tPlot.fX + tPlot.fW * 0.5f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag down dispatch");
	iRet = xuiInputPointerMove(pContext, tPlot.fX + tPlot.fW * 0.75f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag move dispatch");
	iRet = xuiInputPointerUp(pContext, tPlot.fX + tPlot.fW * 0.75f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "full range drag up dispatch");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 0 && fMinX == fAutoMinX && fMaxX == fAutoMaxX &&
	               fMinY == fAutoMinY && fMaxY == fAutoMaxY, "full range cannot be panned away");
	iRet = xuiInputPointerWheel(pContext, tPlot.fX + tPlot.fW * 0.5f, tPlot.fY + tPlot.fH * 0.5f, 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel zoom input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel zoom dispatch");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && (fMaxX - fMinX) < 4.0 && (fMaxY - fMinY) < 10.0, "wheel zoom range");
	fOwnerMinX = fMinX;
	fOwnerMaxX = fMaxX;
	fOwnerMinY = fMinY;
	fOwnerMaxY = fMaxY;
	iRet = xuiInputPointerDownEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPlot.fX + tPlot.fW * 0.5f), (int)(tPlot.fY + tPlot.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH) == pChart,
		"chart owner touch captures pan");
	iRet = xuiInputPointerDownEx(pContext, 602u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPlot.fX + tPlot.fW * 0.5f), (int)(tPlot.fY + tPlot.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 602u,
		XUI_POINTER_TYPE_TOUCH, (int)(tPlot.fX + tPlot.fW * 0.9f),
		(int)(tPlot.fY + tPlot.fH * 0.5f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 602u,
		XUI_POINTER_TYPE_TOUCH, (int)(tPlot.fX + tPlot.fW * 0.9f),
		(int)(tPlot.fY + tPlot.fH * 0.5f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiChartGetViewRange(pChart,
		&fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX == fOwnerMinX && fMaxX == fOwnerMaxX &&
		xuiGetPointerCaptureEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH) == pChart &&
		xuiGetPointerCaptureEx(pContext, 602u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot pan chart or end owner capture");
	iRet = xuiInputPointerMoveEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPlot.fX + tPlot.fW * 0.75f), (int)(tPlot.fY + tPlot.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiChartGetViewRange(pChart,
		&fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX != fOwnerMinX,
		"chart owner touch controls pan");
	iRet = xuiInputPointerUpEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH,
		(int)(tPlot.fX + tPlot.fW * 0.75f), (int)(tPlot.fY + tPlot.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 601u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"chart owner touch releases pan capture");
	iRet = xuiChartSetViewRange(pChart, fOwnerMinX, fOwnerMaxX,
		fOwnerMinY, fOwnerMaxY);
	if ( iRet == XUI_OK ) iRet = xuiChartGetViewRange(pChart,
		&fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX == fOwnerMinX && fMaxX == fOwnerMaxX,
		"restore chart range after owner isolation test");
	iRet = xuiInputPointerDown(pContext, tPlot.fX + tPlot.fW * 0.5f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pChart, "disable capture acquired");
	iRet = xuiWidgetSetEnabled(pChart, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "disable releases chart capture");
	iRet = xuiInputPointerUp(pContext, tPlot.fX + tPlot.fW * 0.5f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "disabled pointer release");
	iRet = xuiWidgetSetEnabled(pChart, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "chart re-enable");
	fPanMinX = fMinX;
	iRet = xuiInputPointerDown(pContext, tPlot.fX + tPlot.fW * 0.5f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan down dispatch");
	iRet = xuiInputPointerMove(pContext, tPlot.fX + tPlot.fW * 4.0f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan move dispatch");
	iRet = xuiInputPointerUp(pContext, tPlot.fX + tPlot.fW * 4.0f, tPlot.fY + tPlot.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "pan up dispatch");
	iRet = xuiChartGetViewRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX < fPanMinX && fabs(fMinX - fAutoMinX) < 1.0e-9,
	               "drag pan range clamped to data bounds");
	iRet = xuiChartResetViewRange(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset view range after interactions");
	iRet = xuiChartSetBrushRange(pChart, 2.0, 0.0, 9.0, 3.0);
	XUI_TEST_CHECK(iRet == XUI_OK, "set brush range");
	iRet = xuiChartSetBrushRange(pChart, 4.0, 1.0, INFINITY, 3.0);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite brush range rejected");
	iRet = xuiChartGetBrushRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMinX == 0.0 && fMaxX == 2.0 && fMinY == 3.0 && fMaxY == 9.0, "get brush range");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"brush range render clears dirty state");
	iRet = xuiChartSetBrushRange(pChart, 2.0, 0.0, 9.0, 3.0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"same normalized brush range stays clean");
	iRet = xuiChartClearBrushRange(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetBrushRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY) == 0, "clear brush range");
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush modifiers");
	iRet = xuiInputPointerDown(pContext, tPlot.fX + tPlot.fW * 0.25f, tPlot.fY + tPlot.fH * 0.75f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush down dispatch");
	iRet = xuiInputPointerMove(pContext, tPlot.fX + tPlot.fW * 0.75f, tPlot.fY + tPlot.fH * 0.25f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush move dispatch");
	iRet = xuiInputPointerUp(pContext, tPlot.fX + tPlot.fW * 0.75f, tPlot.fY + tPlot.fH * 0.25f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush up dispatch");
	iRet = xuiInputSetModifiers(pContext, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear brush modifiers");
	iRet = xuiChartGetBrushRange(pChart, &fMinX, &fMaxX, &fMinY, &fMaxY);
	XUI_TEST_CHECK(iRet == 1 && fMaxX > fMinX && fMaxY > fMinY, "drag brush range");
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		tPlot.fX + tPlot.fW * 0.25f, tPlot.fY + tPlot.fH * 0.75f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pChart,
		"brush captures before programmatic clear");
	iRet = xuiChartClearBrushRange(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"programmatic brush clear releases capture");
	iRet = xuiInputPointerUp(pContext, tPlot.fX + tPlot.fW * 0.25f,
		tPlot.fY + tPlot.fH * 0.75f, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "brush clear cleanup");
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLineLarge, 32);
	XUI_TEST_CHECK(iRet == XUI_OK, "large line data");
	iRet = xuiChartSetSeriesSymbol(pChart, iLine, XUI_CHART_SYMBOL_CIRCLE);
	XUI_TEST_CHECK(iRet == XUI_OK, "lod circle symbol");
	iRet = xuiChartSetLodThreshold(pChart, 8);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetLodThreshold(pChart) == 8, "lod threshold");
	g_iChartLodColor = xuiChartGetSeriesColor(pChart, iLine);
	g_fChartLodMinY = FLT_MAX;
	g_iChartLodCircles = 0;
	iRet = xuiChartDataToPixel(pChart, arrLineLarge[5].x, arrLineLarge[5].y, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "lod peak coordinate");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetLastLodStride(pChart) > 1 &&
	               g_iChartLodCircles > 0 && fabsf(g_fChartLodMinY - tPixel.fY) < 1.0f,
	               "lod render preserves interior extrema");
	iRet = xuiChartSetLodThreshold(pChart, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetLodThreshold(pChart) == 0, "lod disabled");
	iRet = xuiChartSetAnimation(pChart, 1, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress) == 0, "non-finite animation duration rejected");
	iRet = xuiChartSetAnimation(pChart, 1, 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation enable");
	iRet = xuiChartSetSeriesData(pChart, iLine, arrLine, 4);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation trigger data");
	iRet = xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress);
	XUI_TEST_CHECK(iRet == 1 && bAnimEnabled == 1 && fAnimDuration == 0.25f && fAnimProgress == 0.0f, "animation active");
	iRet = xuiChartStepAnimation(pChart, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress) == 1 && fAnimProgress == 0.0f, "non-finite animation delta rejected transactionally");
	iRet = xuiChartStepAnimation(pChart, 0.125f);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation step half");
	iRet = xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress);
	XUI_TEST_CHECK(iRet == 1 && fAnimProgress > 0.49f && fAnimProgress < 0.51f, "animation progress half");
	iRet = xuiChartSetAnimation(pChart, 1, 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK, "same animation configuration");
	iRet = xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress);
	XUI_TEST_CHECK(iRet == 1 && fAnimProgress > 0.49f && fAnimProgress < 0.51f,
		"same animation configuration preserves active progress");
	iRet = xuiChartStepAnimation(pChart, 0.25f);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation step done");
	iRet = xuiChartGetAnimation(pChart, &bAnimEnabled, &fAnimDuration, &fAnimProgress);
	XUI_TEST_CHECK(iRet == 0 && fAnimProgress == 1.0f, "animation complete");
	iRet = xuiChartSetAnimation(pChart, 0, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "animation disable");

	arrBarA[0].y = 10.0; arrBarA[0].value = 10.0; arrBarA[0].label = "A";
	arrBarA[1].y = 30.0; arrBarA[1].value = 30.0; arrBarA[1].label = "B";
	arrBarA[2].y = 20.0; arrBarA[2].value = 20.0; arrBarA[2].label = "C";
	arrBarB[0].y = 8.0; arrBarB[0].value = 8.0; arrBarB[0].label = "A";
	arrBarB[1].y = 18.0; arrBarB[1].value = 18.0; arrBarB[1].label = "B";
	arrBarB[2].y = 12.0; arrBarB[2].value = 12.0; arrBarB[2].label = "C";
	iRet = xuiChartClearSeries(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for bars");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_BAR, "A", &iBarA);
	XUI_TEST_CHECK(iRet == XUI_OK, "bar a add");
	iRet = xuiChartSetSeriesData(pChart, iBarA, arrBarA, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "bar a data");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_BAR, "B", &iBarB);
	XUI_TEST_CHECK(iRet == XUI_OK, "bar b add");
	iRet = xuiChartSetSeriesData(pChart, iBarB, arrBarB, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "bar b data");
	tPlot = xuiChartGetPlotRect(pChart);
	iRet = xuiChartDataToPixel(pChart, 1.0, 15.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "bar data to pixel");
	iRet = xuiChartHitTest(pChart, tPixel.fX - (tPlot.fW / 3.0f) * 0.18f, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iBarA && tHit.iItem == 1, "bar hit");
	iRet = xuiChartSetSeriesVisible(pChart, iBarA, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide bar a");
	iRet = xuiChartHitTest(pChart, tPixel.fX - (tPlot.fW / 3.0f) * 0.18f, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iSeries != iBarA, "hidden bar excluded");
	iRet = xuiChartSetSeriesVisible(pChart, iBarA, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "show bar a");
	iRet = xuiChartSetBarMode(pChart, XUI_CHART_BAR_STACKED);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetBarMode(pChart) == XUI_CHART_BAR_STACKED, "stacked bar mode");
	iRet = xuiChartDataToPixel(pChart, 1.0, 39.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "stacked bar data to pixel");
	iRet = xuiChartHitTest(pChart, tPixel.fX, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iBarB && tHit.iItem == 1, "stacked bar hit");
	iRet = xuiChartHitTest(pChart, tPixel.fX + (tPlot.fW / 3.0f) * 0.25f, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES &&
		tHit.iSeries == iBarB && tHit.iItem == 1, "stacked bar right half hit");
	iRet = xuiChartSetBarMode(pChart, XUI_CHART_BAR_GROUPED);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetBarMode(pChart) == XUI_CHART_BAR_GROUPED, "grouped bar mode restore");
	iRet = xuiChartSetBarDirection(pChart, XUI_CHART_BAR_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetBarDirection(pChart) == XUI_CHART_BAR_HORIZONTAL, "horizontal bar direction");
	iRet = xuiChartDataToPixel(pChart, 15.0, 1.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal bar data to pixel");
	iRet = xuiChartHitTest(pChart, tPixel.fX, tPixel.fY - (tPlot.fH / 3.0f) * 0.18f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iBarA && tHit.iItem == 1, "horizontal bar hit");
	iRet = xuiChartSetBarMode(pChart, XUI_CHART_BAR_STACKED);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal stacked mode");
	iRet = xuiChartDataToPixel(pChart, 39.0, 1.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal stacked data to pixel");
	iRet = xuiChartHitTest(pChart, tPixel.fX, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iBarB && tHit.iItem == 1, "horizontal stacked hit");
	iRet = xuiChartSetBarDirection(pChart, XUI_CHART_BAR_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetBarDirection(pChart) == XUI_CHART_BAR_VERTICAL, "vertical bar direction restore");
	iRet = xuiChartSetBarMode(pChart, XUI_CHART_BAR_GROUPED);
	XUI_TEST_CHECK(iRet == XUI_OK, "grouped bar mode restore 2");

	arrScatter[0].x = 1.0; arrScatter[0].y = 2.0; arrScatter[0].value = 2.0; arrScatter[0].label = "S0";
	arrScatter[1].x = 4.0; arrScatter[1].y = 7.0; arrScatter[1].value = 7.0; arrScatter[1].label = "S1";
	arrScatter[2].x = 8.0; arrScatter[2].y = 3.0; arrScatter[2].value = 3.0; arrScatter[2].label = "S2";
	iRet = xuiChartClearSeries(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear for scatter");
	iRet = xuiChartSetXAxis(pChart, XUI_CHART_AXIS_VALUE);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter x axis");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_SCATTER, "Scatter", &iScatter);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter add");
	iRet = xuiChartSetSeriesData(pChart, iScatter, arrScatter, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter data");
	iRet = xuiChartSetSeriesData(pChart, iScatter, arrLineLarge, 32);
	XUI_TEST_CHECK(iRet == XUI_OK, "large scatter data");
	iRet = xuiChartSetLodThreshold(pChart, 8);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter lod threshold");
	g_iChartLodColor = xuiChartGetSeriesColor(pChart, iScatter);
	g_iChartLodCircles = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && g_iChartLodCircles == 8,
	               "scatter lod uses exact uniform point budget");
	iRet = xuiChartSetLodThreshold(pChart, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter lod disabled");
	iRet = xuiChartSetSeriesData(pChart, iScatter, arrScatter, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter data restore");
	iRet = xuiChartSetSeriesValueRadius(pChart, iScatter, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter value radius set");
	iRet = xuiChartGetSeriesValueRadius(pChart, iScatter, &fMinSize, &fMaxSize);
	XUI_TEST_CHECK(iRet == 1 && fMinSize == 6.0f && fMaxSize == 18.0f, "scatter value radius get");
	iRet = xuiChartSetSeriesValueRadius(pChart, iScatter, NAN, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetSeriesValueRadius(pChart, iScatter, &fMinSize, &fMaxSize) == 1 && fMinSize == 6.0f && fMaxSize == 18.0f, "non-finite scatter radius rejected transactionally");
	iRet = xuiChartSetSeriesValueColor(pChart, iScatter, XUI_COLOR_RGBA(42, 124, 221, 255), XUI_COLOR_RGBA(211, 75, 98, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter value color set");
	iRet = xuiChartGetSeriesValueColor(pChart, iScatter, &iMinColor, &iMaxColor);
	XUI_TEST_CHECK(iRet == 1 && iMinColor == XUI_COLOR_RGBA(42, 124, 221, 255) && iMaxColor == XUI_COLOR_RGBA(211, 75, 98, 255), "scatter value color get");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"scatter value styles render clears dirty state");
	iRet = xuiChartSetSeriesValueRadius(pChart, iScatter, 6.0f, 18.0f);
	if ( iRet == XUI_OK ) {
		iRet = xuiChartSetSeriesValueColor(pChart, iScatter,
			XUI_COLOR_RGBA(42, 124, 221, 255), XUI_COLOR_RGBA(211, 75, 98, 255));
	}
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"same scatter value styles stay clean");
	iRet = xuiChartDataToPixel(pChart, 4.0, 7.0, &tPixel);
	XUI_TEST_CHECK(iRet == XUI_OK, "scatter data to pixel");
	iRet = xuiChartHitTest(pChart, tPixel.fX, tPixel.fY, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iScatter && tHit.iItem == 1, "scatter hit");
	iRet = xuiChartClearSeriesValueRadius(pChart, iScatter);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesValueRadius(pChart, iScatter, &fMinSize, &fMaxSize) == 0, "scatter value radius clear");
	iRet = xuiChartClearSeriesValueColor(pChart, iScatter);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesValueColor(pChart, iScatter, &iMinColor, &iMaxColor) == 0, "scatter value color clear");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"cleared scatter value styles render cleanly");
	iRet = xuiChartClearSeriesValueRadius(pChart, iScatter);
	if ( iRet == XUI_OK ) iRet = xuiChartClearSeriesValueColor(pChart, iScatter);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetDirtyFlags(pChart) == 0u,
		"repeated scatter value style clears stay clean");

	arrPie[0].value = 30.0; arrPie[0].label = "A";
	arrPie[1].value = 20.0; arrPie[1].label = "B";
	arrPie[2].value = 50.0; arrPie[2].label = "C";
	arrPieB[0].value = 25.0; arrPieB[0].label = "A2";
	arrPieB[1].value = 35.0; arrPieB[1].label = "B2";
	arrPieB[2].value = 40.0; arrPieB[2].label = "C2";
	iRet = xuiChartClearSeries(pChart);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesCount(pChart) == 0, "clear");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_PIE, "Share", &iPie);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie add after clear");
	iRet = xuiChartSetSeriesData(pChart, iPie, arrPie, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie data");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_PIE, "Share2", &iPieB);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie ring add");
	iRet = xuiChartSetSeriesData(pChart, iPieB, arrPieB, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie ring data");
	iRet = xuiChartSetPieInnerRadius(pChart, 0.35f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetPieInnerRadius(pChart) == 0.35f, "donut");
	iRet = xuiChartSetPieInnerRadius(pChart, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiChartGetPieInnerRadius(pChart) == 0.35f, "non-finite pie radius rejected transactionally");
	tPlot = xuiChartGetPlotRect(pChart);
	arrPie[0].value = DBL_MAX;
	arrPie[1].value = DBL_MAX;
	arrPie[2].value = DBL_MAX;
	iRet = xuiChartSetSeriesData(pChart, iPie, arrPie, 3);
	if ( iRet == XUI_OK ) iRet = xuiChartHitTest(pChart,
		tPlot.fX + tPlot.fW * 0.5f + 34.0f, tPlot.fY + tPlot.fH * 0.5f - 34.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES &&
		tHit.iSeries == iPie && tHit.iItem == 0,
		"pie hit remains stable when positive total exceeds double range");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "extreme pie total render");
	arrPie[0].value = 30.0;
	arrPie[1].value = 20.0;
	arrPie[2].value = 50.0;
	iRet = xuiChartSetSeriesData(pChart, iPie, arrPie, 3);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie data restore after extreme total");
	iRet = xuiChartHitTest(pChart, tPlot.fX + tPlot.fW * 0.5f + 34.0f, tPlot.fY + tPlot.fH * 0.5f - 34.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iPie && tHit.iItem == 0, "pie hit");
	iRet = xuiChartHitTest(pChart, tPlot.fX + tPlot.fW * 0.5f + 60.0f, tPlot.fY + tPlot.fH * 0.5f - 45.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iPieB && tHit.iItem == 0, "pie ring hit");
	iRet = xuiChartSetPieMode(pChart, XUI_CHART_PIE_ROSE);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetPieMode(pChart) == XUI_CHART_PIE_ROSE, "pie rose mode");
	iRet = xuiChartHitTest(pChart, tPlot.fX + tPlot.fW * 0.5f + 32.0f, tPlot.fY + tPlot.fH * 0.5f - 24.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iPie && tHit.iItem == 0, "rose inner hit");
	iRet = xuiChartHitTest(pChart, tPlot.fX + tPlot.fW * 0.5f + 54.0f, tPlot.fY + tPlot.fH * 0.5f - 40.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iPart == XUI_CHART_HIT_SERIES && tHit.iSeries == iPieB && tHit.iItem == 0, "rose outer hit");
	iRet = xuiChartSetPieMode(pChart, XUI_CHART_PIE_NORMAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetPieMode(pChart) == XUI_CHART_PIE_NORMAL, "pie normal mode restore");
	iMeshBase = xuiTestProxyGetMeshDrawCount(&tState);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "pie render");
	XUI_TEST_CHECK(xuiTestProxyGetMeshDrawCount(&tState) >= iMeshBase + 6, "pie sector mesh draw");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pChart, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LIST_ITEM,
	               "chart accessible point before action");
	iRet = xuiWidgetPerformAccessibleAction(pChart, tAccessible.iId,
	                                      XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "chart point accessibility selection");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pChart, 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && (tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTED) != 0,
	               "chart accessibility reports selected point");
	i = xuiWidgetGetAccessibleNodeCount(pChart) - xuiChartGetSeriesCount(pChart);
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pChart, i, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX,
	               "chart accessible legend before action");
	iRet = xuiWidgetPerformAccessibleAction(pChart, tAccessible.iId,
	                                      XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiChartGetSeriesVisible(pChart, iPie),
	               "chart legend accessibility toggle");
	iRet = xuiWidgetPerformAccessibleAction(pChart, tAccessible.iId,
	                                      XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiChartGetSeriesVisible(pChart, iPie),
	               "chart legend accessibility toggle restore");
	iRet = xuiChartAddSeries(pChart, XUI_CHART_SERIES_LINE,
		"A\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD"
		"\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD"
		"\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD",
		&iUtf8Series);
	XUI_TEST_CHECK(iRet == XUI_OK && iUtf8Series == 2, "long utf-8 series name");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pChart, xuiWidgetGetAccessibleNodeCount(pChart) - 1, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX &&
	               strcmp(tAccessible.sName,
				"A\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD"
				"\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD"
				"\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD\xE4\xB8\xAD") == 0,
	               "series name truncation preserves utf-8 boundary");
	iRet = xuiInputPointerMove(pContext, tPlot.fX + tPlot.fW * 0.5f + 34.0f, tPlot.fY + tPlot.fH * 0.5f - 34.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK, "destroy tooltip hover");
	iRet = xuiChartSetTooltipCallback(pChart, __xuiChartDestroyTooltip, &pChart);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy tooltip callback");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pChart == NULL, "tooltip callback may destroy chart safely");

cleanup:
	if ( pPainter != NULL ) {
		xuiPainterEnd(pPainter);
	}
	if ( pPath != NULL ) {
		xuiPathDestroy(pPath);
	}
	if ( pNoVectorContext != NULL ) {
		xuiDestroy(pNoVectorContext);
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
	printf("xui_chart_test passed\n");
	return 0;
}
