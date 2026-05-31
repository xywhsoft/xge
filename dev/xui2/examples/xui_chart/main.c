#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 980
#define DEMO_H 720
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_chart_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pChart[4];
	int iFrame;
	int iMaxFrames;
	int bPrevLeftDown;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
} xui_chart_demo_t;

static void __xuiChartUsage(void)
{
	printf("usage: xui_chart [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiChartParseArgs(xui_chart_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiChartUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiChartFindTtf(void)
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

static int __xuiChartCreateChart(xui_chart_demo_t* pDemo, int iIndex, const char* sTitle, xui_rect_t tRect)
{
	xui_chart_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sTitle = sTitle;
	iRet = xuiChartCreate(pDemo->pContext, &pDemo->pChart[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pChart[iIndex], tRect);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pChart[iIndex]);
}

static int __xuiChartCreateUi(xui_chart_demo_t* pDemo)
{
	xui_chart_point_t arrLineA[] = {
		{0.0, 12.0, 12.0, "Jan", 0},
		{1.0, 19.0, 19.0, "Feb", 0},
		{2.0, 15.0, 15.0, "Mar", 0},
		{3.0, 28.0, 28.0, "Apr", 0},
		{4.0, 24.0, 24.0, "May", 0},
		{5.0, 34.0, 34.0, "Jun", 0}
	};
	xui_chart_point_t arrLineB[] = {
		{0.0, 9.0, 9.0, "Jan", 0},
		{1.0, 13.0, 13.0, "Feb", 0},
		{2.0, 20.0, 20.0, "Mar", 0},
		{3.0, 23.0, 23.0, "Apr", 0},
		{4.0, 18.0, 18.0, "May", 0},
		{5.0, 30.0, 30.0, "Jun", 0}
	};
	xui_chart_point_t arrBarA[] = {
		{0.0, 48.0, 48.0, "A", 0},
		{1.0, 62.0, 62.0, "B", 0},
		{2.0, 55.0, 55.0, "C", 0},
		{3.0, 70.0, 70.0, "D", 0}
	};
	xui_chart_point_t arrBarB[] = {
		{0.0, 32.0, 32.0, "A", 0},
		{1.0, 42.0, 42.0, "B", 0},
		{2.0, 50.0, 50.0, "C", 0},
		{3.0, 38.0, 38.0, "D", 0}
	};
	xui_chart_point_t arrPie[] = {
		{0.0, 0.0, 35.0, "Desktop", XUI_COLOR_RGBA(42, 124, 221, 255)},
		{0.0, 0.0, 25.0, "Mobile", XUI_COLOR_RGBA(28, 164, 115, 255)},
		{0.0, 0.0, 18.0, "Cloud", XUI_COLOR_RGBA(238, 145, 42, 255)},
		{0.0, 0.0, 22.0, "Tools", XUI_COLOR_RGBA(211, 75, 98, 255)}
	};
	xui_chart_point_t arrPieB[] = {
		{0.0, 0.0, 28.0, "Desktop", XUI_COLOR_RGBA(79, 150, 235, 255)},
		{0.0, 0.0, 32.0, "Mobile", XUI_COLOR_RGBA(61, 185, 140, 255)},
		{0.0, 0.0, 15.0, "Cloud", XUI_COLOR_RGBA(242, 167, 74, 255)},
		{0.0, 0.0, 25.0, "Tools", XUI_COLOR_RGBA(222, 104, 124, 255)}
	};
	xui_chart_point_t arrScatter[] = {
		{1.0, 4.0, 4.0, "A", 0},
		{2.4, 5.8, 5.8, "B", 0},
		{3.0, 2.8, 2.8, "C", 0},
		{4.5, 8.0, 8.0, "D", 0},
		{5.2, 6.2, 6.2, "E", 0},
		{6.8, 9.0, 9.0, "F", 0},
		{7.6, 4.5, 4.5, "G", 0}
	};
	float arrDash[] = {10.0f, 5.0f};
	int iSeries;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H});
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiChartCreateChart(pDemo, 0, "Line Chart", (xui_rect_t){18.0f, 18.0f, 460.0f, 320.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiChartAddSeries(pDemo->pChart[0], XUI_CHART_SERIES_LINE, "Actual", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesAreaFill(pDemo->pChart[0], iSeries, 1, XUI_COLOR_RGBA(42, 124, 221, 44));
	(void)xuiChartSetSeriesSmooth(pDemo->pChart[0], iSeries, 1);
	(void)xuiChartSetSeriesData(pDemo->pChart[0], iSeries, arrLineA, (int)(sizeof(arrLineA) / sizeof(arrLineA[0])));
	iRet = xuiChartAddSeries(pDemo->pChart[0], XUI_CHART_SERIES_LINE, "Plan", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesColor(pDemo->pChart[0], iSeries, XUI_COLOR_RGBA(211, 75, 98, 255));
	(void)xuiChartSetSeriesSymbol(pDemo->pChart[0], iSeries, XUI_CHART_SYMBOL_DIAMOND);
	(void)xuiChartSetSeriesDash(pDemo->pChart[0], iSeries, arrDash, 2);
	(void)xuiChartSetSeriesData(pDemo->pChart[0], iSeries, arrLineB, (int)(sizeof(arrLineB) / sizeof(arrLineB[0])));

	iRet = __xuiChartCreateChart(pDemo, 1, "Bar Chart", (xui_rect_t){500.0f, 18.0f, 460.0f, 320.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiChartAddSeries(pDemo->pChart[1], XUI_CHART_SERIES_BAR, "2025", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesData(pDemo->pChart[1], iSeries, arrBarA, (int)(sizeof(arrBarA) / sizeof(arrBarA[0])));
	iRet = xuiChartAddSeries(pDemo->pChart[1], XUI_CHART_SERIES_BAR, "2026", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesColor(pDemo->pChart[1], iSeries, XUI_COLOR_RGBA(28, 164, 115, 255));
	(void)xuiChartSetSeriesData(pDemo->pChart[1], iSeries, arrBarB, (int)(sizeof(arrBarB) / sizeof(arrBarB[0])));
	(void)xuiChartSetBarMode(pDemo->pChart[1], XUI_CHART_BAR_STACKED);
	(void)xuiChartSetBarDirection(pDemo->pChart[1], XUI_CHART_BAR_HORIZONTAL);

	iRet = __xuiChartCreateChart(pDemo, 2, "Pie Chart", (xui_rect_t){18.0f, 360.0f, 460.0f, 320.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiChartAddSeries(pDemo->pChart[2], XUI_CHART_SERIES_PIE, "Share", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesData(pDemo->pChart[2], iSeries, arrPie, (int)(sizeof(arrPie) / sizeof(arrPie[0])));
	iRet = xuiChartAddSeries(pDemo->pChart[2], XUI_CHART_SERIES_PIE, "Growth", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesData(pDemo->pChart[2], iSeries, arrPieB, (int)(sizeof(arrPieB) / sizeof(arrPieB[0])));
	(void)xuiChartSetPieInnerRadius(pDemo->pChart[2], 0.32f);
	(void)xuiChartSetPieMode(pDemo->pChart[2], XUI_CHART_PIE_ROSE);

	iRet = __xuiChartCreateChart(pDemo, 3, "Scatter Chart", (xui_rect_t){500.0f, 360.0f, 460.0f, 320.0f});
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetXAxis(pDemo->pChart[3], XUI_CHART_AXIS_VALUE);
	iRet = xuiChartAddSeries(pDemo->pChart[3], XUI_CHART_SERIES_SCATTER, "Samples", &iSeries);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiChartSetSeriesSymbolSize(pDemo->pChart[3], iSeries, 10.0f);
	(void)xuiChartSetSeriesValueRadius(pDemo->pChart[3], iSeries, 6.0f, 18.0f);
	(void)xuiChartSetSeriesValueColor(pDemo->pChart[3], iSeries, XUI_COLOR_RGBA(42, 124, 221, 255), XUI_COLOR_RGBA(211, 75, 98, 255));
	(void)xuiChartSetSeriesData(pDemo->pChart[3], iSeries, arrScatter, (int)(sizeof(arrScatter) / sizeof(arrScatter[0])));

	return XUI_OK;
}

static int __xuiChartCreateAssets(xui_chart_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_W, (float)DEMO_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_W;
	tSurfaceDesc.iHeight = DEMO_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiChartFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiChartCreateUi(pDemo);
}

static void __xuiChartDestroyAssets(xui_chart_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) xuiDestroy(pDemo->pContext);
	if ( pDemo->pFont != NULL ) pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
	if ( pDemo->pTarget != NULL ) pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
}

static void __xuiChartHandleInput(xui_chart_demo_t* pDemo)
{
	float fX;
	float fY;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	fX -= DEMO_OFFSET_X;
	fY -= DEMO_OFFSET_Y;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( xgeMouseDown(XGE_MOUSE_LEFT) && !pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	}
	if ( !xgeMouseDown(XGE_MOUSE_LEFT) && pDemo->bPrevLeftDown ) {
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	}
	pDemo->bPrevLeftDown = xgeMouseDown(XGE_MOUSE_LEFT) ? 1 : 0;
}

static int __xuiChartFrame(void* pUser)
{
	xui_chart_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;

	pDemo = (xui_chart_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	__xuiChartHandleInput(pDemo);
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bCreateOK = (pDemo->pChart[0] != NULL) && (pDemo->pChart[1] != NULL) && (pDemo->pChart[2] != NULL) && (pDemo->pChart[3] != NULL);
	pDemo->bLayoutOK = xuiChartGetPlotRect(pDemo->pChart[0]).fW > 100.0f;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(226, 234, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_W, DEMO_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 22, 28, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_W, (float)DEMO_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XGE_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		memset(&tStats, 0, sizeof(tStats));
		tStats.iSize = sizeof(tStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_chart final-summary frames=%d create=%d layout=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_chart_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiChartParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiChartUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_W + 20;
	tDesc.iHeight = DEMO_H + 40;
	tDesc.sTitle = "XUI Chart";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_chart: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiChartCreateAssets(&tDemo);
	if ( iRet == XGE_OK ) {
		iRet = xgeRun(__xuiChartFrame, &tDemo);
	} else {
		printf("xui_chart: create failed: %d\n", iRet);
	}
	__xuiChartDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK) ? 0 : 1;
}
