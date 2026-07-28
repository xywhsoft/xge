#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_WIDTH 960
#define DEMO_HEIGHT 600

typedef struct xui_icon_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_icon_category pToolbarIcons;
	xui_icon_category pDocumentIcons;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	char sCapturePath[260];
	int bCaptureDone;
} xui_icon_demo_t;

static const char g_sDocumentSvg[] =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\">"
	"<path fill=\"#47a7e8\" d=\"M14 4h25l11 11v45H14z\"/>"
	"<path fill=\"#bce4ff\" d=\"M39 4v13h13z\"/>"
	"<path fill=\"#ffffff\" d=\"M21 29h22v4H21zm0 9h22v4H21zm0 9h15v4H21z\"/>"
	"</svg>";

static const char g_sPaletteSvg[] =
	"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 64 64\">"
	"<circle cx=\"32\" cy=\"32\" r=\"28\" fill=\"#20262e\"/>"
	"<circle cx=\"32\" cy=\"14\" r=\"7\" fill=\"#ef6b73\"/>"
	"<circle cx=\"48\" cy=\"25\" r=\"7\" fill=\"#f3bd58\"/>"
	"<circle cx=\"44\" cy=\"44\" r=\"7\" fill=\"#5acb91\"/>"
	"<circle cx=\"20\" cy=\"44\" r=\"7\" fill=\"#58a9ed\"/>"
	"<circle cx=\"16\" cy=\"25\" r=\"7\" fill=\"#a781e8\"/>"
	"</svg>";

static int __xuiIconDemoCustomMeasure(xui_icon pIcon, xui_vec2_t* pSize, void* pUser)
{
	(void)pIcon;
	(void)pUser;
	pSize->fX = 64.0f;
	pSize->fY = 64.0f;
	return XUI_OK;
}

static int __xuiIconDemoCustomDraw(xui_icon pIcon, xui_painter pPainter, xui_rect_t tRect, const xui_icon_draw_desc_t* pDesc, void* pUser)
{
	float fCellW;
	float fCellH;
	float fGap;
	uint32_t iColor;
	int iRow;
	int iCol;

	(void)pIcon;
	(void)pUser;
	iColor = (pDesc != NULL) ? pDesc->iColor : XUI_COLOR_WHITE;
	fGap = tRect.fW * 0.08f;
	fCellW = (tRect.fW - fGap * 3.0f) * 0.5f;
	fCellH = (tRect.fH - fGap * 3.0f) * 0.5f;
	for ( iRow = 0; iRow < 2; iRow++ ) {
		for ( iCol = 0; iCol < 2; iCol++ ) {
			xui_rect_t tCell;
			int iRet;

			tCell.fX = tRect.fX + fGap + (float)iCol * (fCellW + fGap);
			tCell.fY = tRect.fY + fGap + (float)iRow * (fCellH + fGap);
			tCell.fW = fCellW;
			tCell.fH = fCellH;
			iRet = xuiPainterFillRect(pPainter, tCell, iColor);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiIconDemoParseArgs(xui_icon_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
		} else if ( strcmp(argv[i], "--capture") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[++i]);
		} else if ( strncmp(argv[i], "--capture=", 10) == 0 ) {
			snprintf(pDemo->sCapturePath, sizeof(pDemo->sCapturePath), "%s", argv[i] + 10);
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	if ( (pDemo->iMaxFrames < 0) || (pDemo->fMaxSeconds < 0.0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	return XGE_OK;
}

static int __xuiIconDemoCreateIcons(xui_icon_demo_t* pDemo)
{
	xui_icon_category_desc_t tCategoryDesc;
	xui_icon_custom_desc_t tCustomDesc;
	xui_path_style_t tPathStyle;
	xui_icon pDocument;
	int iRet;

	xuiIconCategoryDescDefault(&tCategoryDesc);
	tCategoryDesc.fWidth = 32.0f;
	tCategoryDesc.fHeight = 32.0f;
	iRet = xuiIconCategoryCreate(pDemo->pContext, "toolbar", &tCategoryDesc, &pDemo->pToolbarIcons);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tPathStyle, 0, sizeof(tPathStyle));
	tPathStyle.iSize = sizeof(tPathStyle);
	tPathStyle.iFillColor = XUI_COLOR_WHITE;
	tPathStyle.iStrokeColor = XUI_COLOR_WHITE;
	tPathStyle.iFillRule = XUI_PATH_FILL_NON_ZERO;
	tPathStyle.iLineJoin = XUI_PATH_JOIN_ROUND;
	tPathStyle.iLineCap = XUI_PATH_CAP_ROUND;
	iRet = xuiIconAddSvgPath(
		pDemo->pToolbarIcons,
		"home",
		"M3 12 L12 4 L21 12 V21 H15 V15 H9 V21 H3 Z",
		(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
		&tPathStyle,
		NULL,
		NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconAddSvgPath(
		pDemo->pToolbarIcons,
		"star",
		"M12 2 L15 9 L22 9 L17 14 L19 22 L12 17 L5 22 L7 14 L2 9 L9 9 Z",
		(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
		&tPathStyle,
		NULL,
		NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconAddAlias(
		pDemo->pToolbarIcons,
		"favorite",
		xuiIconFind(pDemo->pToolbarIcons, "star"),
		NULL,
		NULL);
	if ( iRet != XUI_OK ) return iRet;

	xuiIconCategoryDescDefault(&tCategoryDesc);
	tCategoryDesc.iSizeMode = XUI_ICON_SIZE_UNRESTRICTED;
	tCategoryDesc.fWidth = 0.0f;
	tCategoryDesc.fHeight = 0.0f;
	tCategoryDesc.iCacheCapacity = 4;
	iRet = xuiIconCategoryCreate(pDemo->pContext, "documents", &tCategoryDesc, &pDemo->pDocumentIcons);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconAddSvgMemory(
		pDemo->pDocumentIcons,
		"document",
		g_sDocumentSvg,
		(int)sizeof(g_sDocumentSvg) - 1,
		NULL,
		&pDocument);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconAddSvgMemory(
		pDemo->pDocumentIcons,
		"palette",
		g_sPaletteSvg,
		(int)sizeof(g_sPaletteSvg) - 1,
		NULL,
		NULL);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconAddAlias(pDemo->pDocumentIcons, "document_alias", pDocument, NULL, NULL);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tCustomDesc, 0, sizeof(tCustomDesc));
	tCustomDesc.iSize = sizeof(tCustomDesc);
	tCustomDesc.onMeasure = __xuiIconDemoCustomMeasure;
	tCustomDesc.onDraw = __xuiIconDemoCustomDraw;
	return xuiIconAddCustom(pDemo->pDocumentIcons, "grid", &tCustomDesc, NULL, NULL);
}

static void __xuiIconDemoDrawText(xui_icon_demo_t* pDemo, xui_painter pPainter, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	if ( pDemo->pFont != NULL ) {
		(void)xuiPainterDrawText(
			pPainter,
			pDemo->pFont,
			sText,
			tRect,
			iColor,
			XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
}

static int __xuiIconDemoDraw(xui_icon_demo_t* pDemo)
{
	xui_icon_draw_desc_t tDrawDesc;
	xui_painter pPainter;
	xui_rect_t tCard;
	int iRet;

	pPainter = NULL;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(20, 24, 29, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPainterBegin(pDemo->pContext, pDemo->pTarget, &pPainter);
	if ( iRet != XUI_OK ) return iRet;
	xuiIconDrawDescDefault(&tDrawDesc);

	tCard = (xui_rect_t){28.0f, 30.0f, 844.0f, 132.0f};
	(void)xuiPainterFillRect(pPainter, tCard, XUI_COLOR_RGBA(32, 38, 46, 255));
	(void)xuiPainterStrokeRect(pPainter, tCard, 1.0f, XUI_COLOR_RGBA(71, 82, 95, 255));
	__xuiIconDemoDrawText(pDemo, pPainter, "Fixed 32 x 32 category", (xui_rect_t){48.0f, 42.0f, 300.0f, 24.0f}, XUI_COLOR_RGBA(210, 220, 232, 255));
	tDrawDesc.iColor = XUI_COLOR_RGBA(87, 180, 238, 255);
	(void)xuiIconDrawByName(pPainter, pDemo->pToolbarIcons, "home", (xui_rect_t){70.0f, 74.0f, 72.0f, 72.0f}, &tDrawDesc);
	tDrawDesc.iColor = XUI_COLOR_RGBA(245, 190, 76, 255);
	(void)xuiIconDrawByName(pPainter, pDemo->pToolbarIcons, "star", (xui_rect_t){190.0f, 74.0f, 72.0f, 72.0f}, &tDrawDesc);
	tDrawDesc.iColor = XUI_COLOR_RGBA(237, 104, 130, 255);
	(void)xuiIconDrawByName(pPainter, pDemo->pToolbarIcons, "favorite", (xui_rect_t){310.0f, 74.0f, 72.0f, 72.0f}, &tDrawDesc);

	tCard = (xui_rect_t){28.0f, 182.0f, 844.0f, 250.0f};
	(void)xuiPainterFillRect(pPainter, tCard, XUI_COLOR_RGBA(32, 38, 46, 255));
	(void)xuiPainterStrokeRect(pPainter, tCard, 1.0f, XUI_COLOR_RGBA(71, 82, 95, 255));
	__xuiIconDemoDrawText(pDemo, pPainter, "Unrestricted SVG surfaces, aliases and custom icons", (xui_rect_t){48.0f, 194.0f, 520.0f, 24.0f}, XUI_COLOR_RGBA(210, 220, 232, 255));
	tDrawDesc.iColor = XUI_COLOR_WHITE;
	(void)xuiIconDrawByName(pPainter, pDemo->pDocumentIcons, "document", (xui_rect_t){56.0f, 234.0f, 72.0f, 72.0f}, &tDrawDesc);
	(void)xuiIconDrawByName(pPainter, pDemo->pDocumentIcons, "document", (xui_rect_t){160.0f, 226.0f, 112.0f, 112.0f}, &tDrawDesc);
	(void)xuiIconDrawByName(pPainter, pDemo->pDocumentIcons, "palette", (xui_rect_t){314.0f, 226.0f, 112.0f, 112.0f}, &tDrawDesc);
	(void)xuiIconDrawByName(pPainter, pDemo->pDocumentIcons, "document_alias", (xui_rect_t){470.0f, 234.0f, 72.0f, 72.0f}, &tDrawDesc);
	tDrawDesc.iColor = XUI_COLOR_RGBA(106, 214, 163, 255);
	(void)xuiIconDrawByName(pPainter, pDemo->pDocumentIcons, "grid", (xui_rect_t){602.0f, 226.0f, 112.0f, 112.0f}, &tDrawDesc);
	__xuiIconDemoDrawText(pDemo, pPainter, "same SVG, two cached sizes", (xui_rect_t){56.0f, 352.0f, 250.0f, 24.0f}, XUI_COLOR_RGBA(145, 159, 176, 255));
	__xuiIconDemoDrawText(pDemo, pPainter, "full color SVG", (xui_rect_t){314.0f, 352.0f, 150.0f, 24.0f}, XUI_COLOR_RGBA(145, 159, 176, 255));
	__xuiIconDemoDrawText(pDemo, pPainter, "alias", (xui_rect_t){470.0f, 352.0f, 100.0f, 24.0f}, XUI_COLOR_RGBA(145, 159, 176, 255));
	__xuiIconDemoDrawText(pDemo, pPainter, "custom", (xui_rect_t){602.0f, 352.0f, 100.0f, 24.0f}, XUI_COLOR_RGBA(145, 159, 176, 255));

	__xuiIconDemoDrawText(
		pDemo,
		pPainter,
		"Array order + name dictionary + stable IDs + lazy per-size SVG cache",
		(xui_rect_t){32.0f, 456.0f, 820.0f, 30.0f},
		XUI_COLOR_RGBA(119, 203, 247, 255));
	return xuiPainterEnd(pPainter);
}

static int __xuiIconDemoCapture(xui_icon_demo_t* pDemo)
{
	unsigned char* pPixels;
	int iStride;
	int iRet;

	if ( (pDemo->sCapturePath[0] == '\0') || pDemo->bCaptureDone ) return XGE_OK;
	iStride = 900 * 4;
	pPixels = (unsigned char*)malloc((size_t)iStride * 520u);
	if ( pPixels == NULL ) return XGE_ERROR_OUT_OF_MEMORY;
	iRet = pDemo->tProxy.surfaceReadRGBA(&pDemo->tProxy, pDemo->pTarget, pPixels, iStride);
	if ( iRet == XGE_OK ) {
		iRet = xgeImageSavePNG(pDemo->sCapturePath, 900, 520, pPixels, iStride);
	}
	free(pPixels);
	if ( iRet == XGE_OK ) pDemo->bCaptureDone = 1;
	return iRet;
}

static int __xuiIconDemoFrame(void* pUser)
{
	xui_icon_demo_t* pDemo;
	xui_rect_t tSource;
	xui_rect_t tTarget;
	int iRet;

	pDemo = (xui_icon_demo_t*)pUser;
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiIconDemoDraw(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiIconDemoCapture(pDemo);
	if ( iRet != XGE_OK ) return iRet;
	xgeClear(XGE_COLOR_RGBA(12, 15, 18, 255));
	tSource = (xui_rect_t){0.0f, 0.0f, 900.0f, 520.0f};
	tTarget = (xui_rect_t){30.0f, 30.0f, 900.0f, 520.0f};
	iRet = pDemo->tProxy.surfaceDraw(
		&pDemo->tProxy,
		pDemo->pTarget,
		tSource,
		tTarget,
		XUI_COLOR_WHITE,
		XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	pDemo->iFrame++;
	if ( pDemo->bCaptureDone ||
	     ((pDemo->iMaxFrames > 0) && (pDemo->iFrame >= pDemo->iMaxFrames)) ||
	     ((pDemo->fMaxSeconds > 0.0) && (xgeTimer() >= pDemo->fMaxSeconds)) ) {
		xgeQuit();
	}
	return iRet;
}

int main(int argc, char** argv)
{
	xui_icon_demo_t tDemo;
	xui_surface_desc_t tSurfaceDesc;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiIconDemoParseArgs(&tDemo, argc, argv);
	if ( iRet != XGE_OK ) {
		printf("usage: xui_icon [--frames N] [--seconds N] [--capture PATH]\n");
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_WIDTH;
	tDesc.iHeight = DEMO_HEIGHT;
	tDesc.sTitle = "XUI Icon Manager";
	tDesc.iFlags = XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) return 1;

	tDemo.tProxy = xuiProxyXge();
	iRet = xuiCreate(&tDemo.pContext);
	if ( iRet == XUI_OK ) iRet = xuiSetProxy(tDemo.pContext, &tDemo.tProxy);
	if ( iRet == XUI_OK ) iRet = xuiSetVirtualDpi(tDemo.pContext, 1.0f);
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = 900;
	tSurfaceDesc.iHeight = 520;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	if ( iRet == XUI_OK ) {
		iRet = tDemo.tProxy.surfaceCreate(&tDemo.tProxy, &tDemo.pTarget, &tSurfaceDesc);
	}
	if ( iRet == XUI_OK ) {
		iRet = tDemo.tProxy.fontLoadFile(
			&tDemo.tProxy,
			&tDemo.pFont,
			"C:\\Windows\\Fonts\\segoeui.ttf",
			15.0f,
			XUI_FONT_FORMAT_TTF);
		if ( iRet != XUI_OK ) {
			tDemo.pFont = NULL;
			iRet = XUI_OK;
		}
	}
	if ( iRet == XUI_OK ) iRet = __xuiIconDemoCreateIcons(&tDemo);
	if ( iRet == XUI_OK ) iRet = xgeRun(__xuiIconDemoFrame, &tDemo);

	if ( tDemo.pFont != NULL ) tDemo.tProxy.fontDestroy(&tDemo.tProxy, tDemo.pFont);
	if ( tDemo.pContext != NULL ) xuiDestroy(tDemo.pContext);
	if ( tDemo.pTarget != NULL ) tDemo.tProxy.surfaceDestroy(&tDemo.tProxy, tDemo.pTarget);
	xgeUnit();
	return (iRet == XGE_OK) ? 0 : 1;
}
