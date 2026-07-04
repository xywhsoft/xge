#include "../../xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_W 1160
#define DEMO_H 400

typedef struct xge_svg_demo_t {
	xge_render_target_t tTarget;
	xge_svg pMemorySvg;
	xge_svg pFileSvg;
	xge_texture_t tRasterSvg;
	char sCapturePath[260];
	int bRasterLoaded;
	int bCaptureDone;
	int iFrame;
	int iMaxFrames;
} xge_svg_demo_t;

static const char g_sMemorySvg[] =
	"<svg viewBox=\"0 0 64 64\" fill=\"none\" stroke=\"#24364b\" stroke-width=\"2\">"
	"<style>.cssAccent{fill:#13579b;stroke:none}.cssCurrent{color:goldenrod;fill:currentColor;stroke:transparent}#idAccent{fill:#2468ac;stroke:none}polygon{fill:#579b13}path.cssCompound.hot{fill:#b51a62;stroke:none}.cssPair.extra{fill:#0f9675;stroke:none}stop.cssStop{stop-color:#19c37d;stop-opacity:100%}</style>"
	"<style>path.cssSpecific{fill:#22b8cf;stroke:none}.cssSpecific{fill:#111111;stroke:none}.cssAttr{fill:#7a5cff;stroke:none}</style>"
	"<style><![CDATA[/* cdata style */@font-face{font-family:ignore;src:url(ignore)}path.cssCData{fill:/*inline*/#e07124;stroke:none}]]></style>"
	"<defs><linearGradient id=\"cardGrad\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"0%\" spreadMethod=\"reflect\" gradientTransform=\"translate(0.1 0) scale(0.8 1)\">"
	"<stop offset=\"0%\" stop-color=\"#22a06b\"/>"
	"<stop offset=\"100%\" stop-color=\"#2d7dd2\"/>"
	"</linearGradient></defs>"
	"<defs><radialGradient id=\"sunGrad\" cx=\"50%\" cy=\"45%\" r=\"55%\" spreadMethod=\"repeat\" gradientTransform=\"scale(0.95 0.95) translate(0.03 0.02)\">"
	"<stop offset=\"0%\" stop-color=\"#ffffff\"/>"
	"<stop offset=\"100%\" stop-color=\"#f04f64\"/>"
	"</radialGradient></defs>"
	"<defs><linearGradient id=\"cardGradAlias\" href=\"#lateSharedStops\" x1=\"0%\" y1=\"0%\" x2=\"100%\" y2=\"100%\"/>"
	"<linearGradient id=\"cardGradChain\" xlink:href=\"#cardGradAlias\" spreadMethod=\"repeat\"/>"
	"<linearGradient id=\"lateSharedStops\"><stop offset=\"0%\" stop-color=\"#24364b\"/><stop offset=\"100%\" stop-color=\"#ffb02e\"/></linearGradient></defs>"
	"<defs><linearGradient id=\"currentStopGrad\" color=\"#abcdef\"><stop offset=\"0\" stop-color=\"currentColor\"/><stop offset=\"1\" style=\"stop-color:currentColor\"/></linearGradient>"
	"<linearGradient id=\"cssStopGrad\"><stop class=\"cssStop\" offset=\"0\"/><stop class=\"cssStop\" offset=\"1\"/></linearGradient></defs>"
	"<defs><clipPath id=\"sunCrop\" clipPathUnits=\"objectBoundingBox\"><g transform=\"translate(0.05 0)\"><rect x=\"0\" y=\"0\" width=\"0.55\" height=\"1\"/><rect x=\"0.35\" y=\"0\" width=\"0.55\" height=\"1\"/></g></clipPath></defs>"
	"<defs><clipPath id=\"rectCrop\"><rect x=\"4\" y=\"28\" width=\"18\" height=\"10\"/></clipPath></defs>"
	"<defs><clipPath id=\"boxCrop\" clipPathUnits=\"objectBoundingBox\"><rect x=\"0.2\" y=\"0\" width=\"0.6\" height=\"1\"/></clipPath></defs>"
	"<defs><mask id=\"cardMask\" maskContentUnits=\"objectBoundingBox\"><rect x=\"0.08\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#909090\"/><rect x=\"0.38\" y=\"0\" width=\"0.55\" height=\"1\" fill=\"#c8c8c8\"/></mask></defs>"
	"<defs><path id=\"sparkMark\" d=\"M0 -5 L2 0 L0 5 L-2 0 Z\" fill=\"#f04f64\" stroke=\"none\"/></defs>"
	"<defs><g id=\"miniPair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#2d7dd2\" stroke=\"none\"/><rect x=\"4\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#22a06b\" stroke=\"none\"/></g></defs>"
	"<defs><symbol id=\"scaledSymbol\" viewBox=\"-1 -1 2 2\" preserveAspectRatio=\"none\"><rect x=\"-1\" y=\"-1\" width=\"2\" height=\"2\" fill=\"#cafe44\" stroke=\"none\"/></symbol></defs>"
	"<path d=\"M8 40 C18 14 28 14 38 40 S54 58 58 28\" stroke=\"url(#cardGrad)\" stroke-width=\"4\" stroke-linecap=\"round\"/>"
	"<rect x=\"2\" y=\"26\" width=\"24\" height=\"14\" fill=\"tomato\" stroke=\"none\" clip-path=\"url(#rectCrop)\"/>"
	"<rect x=\"30\" y=\"40\" width=\"24\" height=\"10\" fill=\"royalblue\" stroke=\"none\" clip-path=\"url(#boxCrop)\"/>"
	"<rect x=\"25%\" y=\"2mm\" width=\"12.5%\" height=\"4mm\" fill=\"#22b8cf\" stroke=\"#7a5cff\" stroke-width=\"2%\" opacity=\"0.85\"/>"
	"<svg x=\"34\" y=\"4\" width=\"12\" height=\"8\" viewBox=\"0 0 120 80\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"120\" height=\"80\" fill=\"#39ff88\" stroke=\"none\"/></svg>"
	"<g visibility=\"hidden\"><rect x=\"46\" y=\"4\" width=\"5\" height=\"5\" fill=\"#ff0001\" stroke=\"none\"/><rect x=\"46\" y=\"4\" width=\"5\" height=\"5\" fill=\"#aa33ff\" stroke=\"none\" visibility=\"visible\"/></g>"
	"<g display=\"none\"><rect x=\"52\" y=\"4\" width=\"5\" height=\"5\" fill=\"#12feef\" stroke=\"none\" visibility=\"visible\"/></g>"
	"<rect x=\"52\" y=\"2\" width=\"5\" height=\"3\" fill=\"rgb(100% 50% 0%)\" stroke=\"none\"/>"
	"<rect x=\"58\" y=\"2\" width=\"4\" height=\"3\" fill=\"rgba(18 52 86 / 100%)\" stroke=\"none\"/>"
	"<rect x=\"52\" y=\"6\" width=\"4\" height=\"2\" fill=\"#0f0f\" stroke=\"none\"/>"
	"<rect x=\"58\" y=\"6\" width=\"4\" height=\"2\" fill=\"#336699ff\" stroke=\"none\"/>"
	"<rect x=\"2\" y=\"2\" width=\"4\" height=\"3\" fill=\"hsl(210 100% 50%)\" stroke=\"none\"/>"
	"<rect x=\"7\" y=\"2\" width=\"4\" height=\"3\" fill=\"hsla(300, 100%, 50%, 1)\" stroke=\"none\"/>"
	"<rect x=\"12\" y=\"2\" width=\"4\" height=\"3\" fill=\"#fedcba\" opacity=\"100%\" stroke=\"none\"/>"
	"<rect x=\"17\" y=\"2\" width=\"4\" height=\"3\" fill=\"#55aa11\" fill-opacity=\"100%\" stroke=\"none\"/>"
	"<rect x=\"1\" y=\"58\" width=\"4\" height=\"4\" fill=\"url(#currentStopGrad)\" stroke=\"none\"/>"
	"<rect x=\"6\" y=\"58\" width=\"4\" height=\"4\" fill=\"url(#cssStopGrad)\" stroke=\"none\"/>"
	"<rect x=\"10\" y=\"10\" width=\"16\" height=\"16\" rx=\"4\" fill=\"url(#cardGrad)\" stroke=\"none\" mask=\"url(#cardMask)\"/>"
	"<circle cx=\"42\" cy=\"20\" r=\"10\" fill=\"url(#sunGrad)\" stroke=\"none\" clip-path=\"url(#sunCrop)\"/>"
	"<polygon points=\"18,52 32,34 46,52\" fill=\"#ffb02e\" stroke=\"#24364b\" stroke-width=\"2\"/>"
	"<use href=\"#sparkMark\" x=\"54\" y=\"48\" transform=\"rotate(30 54 48)\"/>"
	"<use href=\"#miniPair\" x=\"12\" y=\"56\"/>"
	"<use href=\"#lateSymbol\" x=\"22\" y=\"56\" width=\"6\" height=\"4\"/>"
	"<use href=\"#latePair\" x=\"42\" y=\"56\"/>"
	"<use href=\"#scaledSymbol\" x=\"30\" y=\"56\" width=\"8\" height=\"4\"/>"
	"<path class=\"cssAccent\" d=\"M50 8 L58 8 L58 14 Z\"/>"
	"<path class=\"cssCurrent\" d=\"M50 28 L58 28 L58 34 Z\"/>"
	"<path class=\"cssCompound hot\" d=\"M50 38 L58 38 L58 44 Z\"/>"
	"<path class=\"cssPair extra\" d=\"M50 48 L58 48 L58 54 Z\"/>"
	"<path class=\"cssCData\" d=\"M6 26 L14 26 L14 32 Z\"/>"
	"<path class=\"cssSpecific\" d=\"M6 36 L14 36 L14 42 Z\"/>"
	"<path class=\"cssAttr\" fill=\"#111111\" d=\"M6 46 L14 46 L14 52 Z\"/>"
	"<rect x=\"52\" y=\"18\" width=\"6\" height=\"6\" fill=\"url(#cardGradChain)\" stroke=\"none\"/>"
	"<path id=\"idAccent\" d=\"M6 6 L14 6 L14 12 Z\"/>"
	"<polygon points=\"6,16 14,16 14,22\"/>"
	"<defs><symbol id=\"lateSymbol\" viewBox=\"0 0 12 8\" preserveAspectRatio=\"none\"><rect x=\"0\" y=\"0\" width=\"12\" height=\"8\" fill=\"#ff66aa\" stroke=\"none\"/></symbol></defs>"
	"<defs><g id=\"latePair\"><circle cx=\"0\" cy=\"0\" r=\"2\" fill=\"#0badf0\" stroke=\"none\"/><rect x=\"5\" y=\"-2\" width=\"4\" height=\"4\" fill=\"#0fdbad\" stroke=\"none\"/></g></defs>"
	"</svg>";

static void __xgeSvgDemoUsage(void)
{
	printf("usage: xge_svg [--frames N] [--capture PATH]\n");
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
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[++i]);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[i] + 10);
			pDemo->sCapturePath[sizeof(pDemo->sCapturePath) - 1] = '\0';
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xgeSvgDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static int __xgeSvgDemoCapture(xge_svg_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( (pDemo == NULL) || (pDemo->sCapturePath[0] == '\0') || (pDemo->bCaptureDone != 0) ) {
		return XGE_OK;
	}
	iStride = DEMO_W * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * DEMO_H);
	if ( pPixels == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeRenderTargetReadPixels(&pDemo->tTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(pDemo->sCapturePath, DEMO_W, DEMO_H, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) {
		pDemo->bCaptureDone = 1;
		printf("xge_svg capture saved: %s\n", pDemo->sCapturePath);
	}
	return iRet;
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
	xge_pass_t tPass;
	xge_texture pTargetTexture;
	int iRet;

	pDemo = (xge_svg_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	if ( pDemo->bRasterLoaded == 0 ) {
		iRet = __xgeSvgDemoLoadRasterTexture(&pDemo->tRasterSvg, "examples\\xge_svg\\assets\\compat.svg", "..\\examples\\xge_svg\\assets\\compat.svg", 320, 240);
		if ( iRet != XGE_OK ) {
			printf("xge_svg failed to load svg raster texture: %d\n", iRet);
			return iRet;
		}
		pDemo->bRasterLoaded = 1;
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	xgePassInit(&tPass, &pDemo->tTarget, XGE_PASS_CLEAR_COLOR, XGE_COLOR_RGBA(22, 26, 32, 255));
	iRet = xgePassBegin(&tPass);
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

	iRet = xgePassEnd(&tPass);
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xgeSvgDemoCapture(pDemo);
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(10, 13, 17, 255));
	memset(&tDraw, 0, sizeof(tDraw));
	pTargetTexture = xgeRenderTargetTexture(&pDemo->tTarget);
	tDraw.pTexture = pTargetTexture;
	tDraw.tSrc = __xgeSvgDemoRect(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.tDst = __xgeSvgDemoRect(0.0f, 0.0f, (float)DEMO_W, (float)DEMO_H);
	tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
	xgeDrawEx(&tDraw);
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( pDemo->bCaptureDone || ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ) {
		printf("xge_svg final-summary frames=%d memorySvg=loaded fileSvg=loaded rasterTexture=%dx%d\n",
			pDemo->iFrame,
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
	tDesc.iWidth = DEMO_W;
	tDesc.iHeight = DEMO_H;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		return 1;
	}
	iRet = xgeRenderTargetCreate(&tDemo.tTarget, DEMO_W, DEMO_H);
	if ( iRet != XGE_OK ) {
		printf("xge_svg failed to create render target: %d\n", iRet);
		xgeTextureFree(&tDemo.tRasterSvg);
		xgeSvgDestroy(tDemo.pMemorySvg);
		xgeSvgDestroy(tDemo.pFileSvg);
		xgeSvgCacheClear();
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xgeSvgDemoFrame, &tDemo);
	xgeRenderTargetFree(&tDemo.tTarget);
	xgeTextureFree(&tDemo.tRasterSvg);
	xgeSvgDestroy(tDemo.pMemorySvg);
	xgeSvgDestroy(tDemo.pFileSvg);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
