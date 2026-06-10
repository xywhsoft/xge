#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct xge_svg_demo_t {
	xge_svg pMemorySvg;
	xge_svg pFileSvg;
	xge_texture_t tRasterSvg;
	int iFrame;
	int iMaxFrames;
} xge_svg_demo_t;

static const char g_sMemorySvg[] =
	"<svg viewBox=\"0 0 64 64\" fill=\"none\" stroke=\"#24364b\" stroke-width=\"2\">"
	"<style>.cssAccent{fill:#13579b;stroke:none}#idAccent{fill:#2468ac;stroke:none}polygon{fill:#579b13}</style>"
	"<defs><linearGradient id=\"cardGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
	"<stop offset=\"0%\" stop-color=\"#22a06b\"/>"
	"<stop offset=\"100%\" stop-color=\"#2d7dd2\"/>"
	"</linearGradient></defs>"
	"<defs><radialGradient id=\"sunGrad\" cx=\"50%\" cy=\"45%\" r=\"55%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.95 0.95) translate(0.03 0.02)\">"
	"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
	"<stop offset=\"100%\" stop-color=\"#f04f64\"/>"
	"</radialGradient></defs>"
	"<defs><linearGradient id=\"sharedStops\"><stop offset=\"0%\" stop-color=\"#24364b\"/><stop offset=\"100%\" stop-color=\"#ffb02e\"/></linearGradient>"
	"<linearGradient id=\"cardGradAlias\" href=\"#sharedStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\"/></defs>"
	"<defs><clipPath id=\"sunCrop\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.05 0)\"><rect x=\"0\" y=\"0\" width=\"0.55\" height=\"1\"/><rect x=\"0.35\" y=\"0\" width=\"0.55\" height=\"1\"/></g></clipPath></defs>"
	"<defs><mask id=\"cardMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0.08\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#909090\"/><rect x=\"0.38\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#c8c8c8\"/></mask></defs>"
	"<defs><path id=\"sparkMark\" d=\"M0 -5 L2 0 L0 5 L-2 0 Z\" fill=\"#f04f64\" stroke=\"none\"/></defs>"
	"<defs><g id=\"miniPair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#2d7dd2\" stroke=\"none\"/><rect x=\"4\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#22a06b\" stroke=\"none\"/></g></defs>"
	"<path d=\"M8 40 C18 14 28 14 38 40 S54 58 58 28\" stroke=\"url(#cardGrad)\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
	"<rect x=\"10\" y=\"10\" width=\"16\" height=\"16\" rx=\"4\" fill=\"url(#cardGrad)\" stroke=\"none\" mask=\"url(#cardMask)\"/>"
	"<circle cx=\"42\" cy=\"20\" r=\"10\" fill=\"url(#sunGrad)\" stroke=\"none\" clip-path=\"url(#sunCrop)\"/>"
	"<polygon points=\"18,52 32,34 46,52\" fill=\"#ffb02e\" stroke=\"#24364b\" stroke-width=\"2\"/>"
	"<use href=\"#sparkMark\" x=\"54\" y=\"48\" transform=\"rotate(30 54 48)\"/>"
	"<use href=\"#miniPair\" x=\"12\" y=\"56\"/>"
	"<use href=\"#latePair\" x=\"42\" y=\"56\"/>"
	"<use href=\"#lateSymbol\" x=\"30\" y=\"56\" width=\"8\" height=\"4\"/>"
	"<path class=\"cssAccent\" d=\"M50 8 L58 8 L58 14 Z\"/>"
	"<rect x=\"52\" y=\"18\" width=\"6\" height=\"6\" fill=\"url(#cardGradAlias)\" stroke=\"none\"/>"
	"<path id=\"idAccent\" d=\"M6 6 L14 6 L14 12 Z\"/>"
	"<polygon points=\"6,16 14,16 14,22\"/>"
	"<defs><g id=\"latePair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#0badf0\" stroke=\"none\"/><rect x=\"5\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#0fdbad\" stroke=\"none\"/></g></defs>"
	"<defs><symbol id=\"lateSymbol\" viewBox=\"-1 -1 2 2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#cafe44\" stroke=\"none\"/></symbol></defs>"
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

static int __xgeSvgDemoLoadCachedAsset(const char* sPath, const char* sBuildPath, xge_svg* ppSvg)
{
	int iRet;

	iRet = xgeSvgLoadCached(sPath, ppSvg);
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	return xgeSvgLoadCached(sBuildPath, ppSvg);
}

static int __xgeSvgDemoLoadRasterTexture(xge_texture pTexture, const char* sPath, const char* sBuildPath, int iWidth, int iHeight)
{
	int iRet;

	iRet = xgeSvgTextureLoad(pTexture, sPath, iWidth, iHeight);
	if ( iRet == XGE_OK ) {
		return XGE_OK;
	}
	xgeTextureFree(pTexture);
	memset(pTexture, 0, sizeof(*pTexture));
	return xgeSvgTextureLoad(pTexture, sBuildPath, iWidth, iHeight);
}

static int __xgeSvgDemoFrame(void* pUser)
{
	xge_svg_demo_t* pDemo;
	xge_rect_t tPanelA;
	xge_rect_t tPanelB;
	xge_rect_t tPanelC;
	xge_rect_t tSvgA;
	xge_rect_t tSvgB;
	xge_rect_t tSvgC;
	xge_draw_t tDraw;
	int iRet;

	pDemo = (xge_svg_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(22, 26, 32, 255));

	tPanelA = __xgeSvgDemoRect(32.0f, 48.0f, 332.0f, 304.0f);
	tPanelB = __xgeSvgDemoRect(414.0f, 48.0f, 332.0f, 304.0f);
	tPanelC = __xgeSvgDemoRect(796.0f, 48.0f, 332.0f, 304.0f);
	tSvgA = __xgeSvgDemoRect(70.0f, 80.0f, 256.0f, 240.0f);
	tSvgB = __xgeSvgDemoRect(452.0f, 80.0f, 256.0f, 240.0f);
	tSvgC = __xgeSvgDemoRect(834.0f, 80.0f, 256.0f, 240.0f);
	__xgeSvgDemoPanel(tPanelA);
	__xgeSvgDemoPanel(tPanelB);
	__xgeSvgDemoPanel(tPanelC);

	(void)xgeSvgDrawPx(pDemo->pMemorySvg, tSvgA, 0.75f);
	(void)xgeSvgDrawPx(pDemo->pFileSvg, tSvgB, 0.75f);
	memset(&tDraw, 0, sizeof(tDraw));
	tDraw.pTexture = &pDemo->tRasterSvg;
	tDraw.tDst = tSvgC;
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);

	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames) ) {
		printf("xge_svg final-summary frames=%d memoryPaths=%d filePaths=%d rasterTexture=%dx%d\n",
			pDemo->iFrame,
			xgeSvgGetPathCount(pDemo->pMemorySvg),
			xgeSvgGetPathCount(pDemo->pFileSvg),
			pDemo->tRasterSvg.iWidth,
			pDemo->tRasterSvg.iHeight);
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
	if ( iRet == XGE_OK ) iRet = __xgeSvgDemoLoadCachedAsset("examples\\xge_svg\\assets\\shapes.svg", "..\\examples\\xge_svg\\assets\\shapes.svg", &tDemo.pFileSvg);
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
	tDesc.iWidth = 1160;
	tDesc.iHeight = 400;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		return 1;
	}
	iRet = __xgeSvgDemoLoadRasterTexture(&tDemo.tRasterSvg, "examples\\xge_svg\\assets\\compat.svg", "..\\examples\\xge_svg\\assets\\compat.svg", 320, 240);
	if ( iRet != XGE_OK ) {
		printf("xge_svg failed to load svg raster texture: %d\n", iRet);
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xgeSvgDemoFrame, &tDemo);
	xgeTextureFree(&tDemo.tRasterSvg);
	xgeSvgDestroy(tDemo.pMemorySvg);
	xgeSvgDestroy(tDemo.pFileSvg);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
