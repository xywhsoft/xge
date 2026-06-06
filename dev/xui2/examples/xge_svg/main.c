#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct xge_svg_demo_t {
	xge_svg pMemorySvg;
	xge_svg pFileSvg;
	int iFrame;
	int iMaxFrames;
} xge_svg_demo_t;

static const char g_sMemorySvg[] =
	"<svg viewBox=\"0 0 64 64\" fill=\"none\" stroke=\"#24364b\" stroke-width=\"2\">"
	"<defs><linearGradient id=\"cardGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\">"
	"<stop offset=\"0%\" stop-color=\"#22a06b\"/>"
	"<stop offset=\"100%\" stop-color=\"#2d7dd2\"/>"
	"</linearGradient></defs>"
	"<defs><radialGradient id=\"sunGrad\" cx=\"50%\" cy=\"45%\" r=\"55%\">"
	"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
	"<stop offset=\"100%\" stop-color=\"#f04f64\"/>"
	"</radialGradient></defs>"
	"<path d=\"M8 40 C18 14 28 14 38 40 S54 58 58 28\" stroke=\"#2d7dd2\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
	"<rect x=\"10\" y=\"10\" width=\"16\" height=\"16\" rx=\"4\" fill=\"url(#cardGrad)\" stroke=\"none\"/>"
	"<circle cx=\"42\" cy=\"20\" r=\"10\" fill=\"url(#sunGrad)\" stroke=\"none\"/>"
	"<polygon points=\"18,52 32,34 46,52\" fill=\"#ffb02e\" stroke=\"#24364b\" stroke-width=\"2\"/>"
	"</svg>";

static void __xgeSvgDemoUsage(void)
{
	printf("usage: xge_svg [--frames N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xgeSvgDemoParseArgs(xge_svg_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xgeSvgDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static xge_rect_t __xgeSvgDemoRect(float fX, float fY, float fW, float fH)
{
	xge_rect_t tRect;

	tRect.fX = fX;
	tRect.fY = fY;
	tRect.fW = fW;
	tRect.fH = fH;
	return tRect;
}

static void __xgeSvgDemoPanel(xge_rect_t tPanel)
{
	xgeShapeRectFillPx(tPanel, XGE_COLOR_RGBA(247, 250, 253, 255));
	xgeShapeRectStrokePx(tPanel, 1.0f, XGE_COLOR_RGBA(189, 200, 212, 255));
}

static int __xgeSvgDemoFrame(void* pUser)
{
	xge_svg_demo_t* pDemo;
	xge_rect_t tPanelA;
	xge_rect_t tPanelB;
	xge_rect_t tSvgA;
	xge_rect_t tSvgB;
	int iRet;

	pDemo = (xge_svg_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(22, 26, 32, 255));

	tPanelA = __xgeSvgDemoRect(40.0f, 48.0f, 332.0f, 304.0f);
	tPanelB = __xgeSvgDemoRect(428.0f, 48.0f, 332.0f, 304.0f);
	tSvgA = __xgeSvgDemoRect(78.0f, 80.0f, 256.0f, 240.0f);
	tSvgB = __xgeSvgDemoRect(466.0f, 80.0f, 256.0f, 240.0f);
	__xgeSvgDemoPanel(tPanelA);
	__xgeSvgDemoPanel(tPanelB);

	(void)xgeSvgDrawPx(pDemo->pMemorySvg, tSvgA, 0.75f);
	(void)xgeSvgDrawPx(pDemo->pFileSvg, tSvgB, 0.75f);

	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames) ) {
		printf("xge_svg final-summary frames=%d memoryPaths=%d filePaths=%d\n",
			pDemo->iFrame,
			xgeSvgGetPathCount(pDemo->pMemorySvg),
			xgeSvgGetPathCount(pDemo->pFileSvg));
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_svg_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xgeSvgDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xgeSvgDemoUsage();
		return 1;
	}
	iRet = xgeSvgCreate(&tDemo.pMemorySvg);
	if ( iRet == XGE_OK ) iRet = xgeSvgLoadMemory(tDemo.pMemorySvg, g_sMemorySvg, (int)sizeof(g_sMemorySvg) - 1);
	if ( iRet == XGE_OK ) iRet = xgeSvgLoadCached("examples\\xge_svg\\assets\\shapes.svg", &tDemo.pFileSvg);
	if ( iRet == XGE_OK ) iRet = xgeSvgSetPreserveAspectRatio(tDemo.pFileSvg, "xMidYMid slice");
	if ( iRet != XGE_OK ) {
		printf("xge_svg failed to load svg assets: %d\n", iRet);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		return 1;
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.sTitle = "XGE SVG";
	tDesc.iWidth = 800;
	tDesc.iHeight = 400;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		return 1;
	}
	iRet = xgeRun(__xgeSvgDemoFrame, &tDemo);
	xgeSvgDestroy(tDemo.pMemorySvg);
	xgeSvgDestroy(tDemo.pFileSvg);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
