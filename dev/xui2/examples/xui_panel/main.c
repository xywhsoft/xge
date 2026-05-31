#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	820
#define DEMO_TARGET_H	520
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define PANEL_COUNT	4
#define LABEL_COUNT	12

enum {
	PANEL_DEFAULT = 0,
	PANEL_ICON,
	PANEL_NO_HEADER,
	PANEL_CLIP
};

typedef struct xui_panel_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pIconSurface;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pPanel[PANEL_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	int iLabelCount;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bCreateOK;
	int bLayoutOK;
	int bTitleOK;
	int bIconOK;
	int bStyleOK;
	int bClientOK;
	int bClipOK;
} xui_panel_demo_t;

static uint32_t __xuiPanelColorR(uint32_t iColor) { return (iColor >> 24) & 0xffu; }
static uint32_t __xuiPanelColorG(uint32_t iColor) { return (iColor >> 16) & 0xffu; }
static uint32_t __xuiPanelColorB(uint32_t iColor) { return (iColor >> 8) & 0xffu; }
static uint32_t __xuiPanelColorA(uint32_t iColor) { return iColor & 0xffu; }

static void __xuiPanelUsage(void)
{
	printf("usage: xui_panel [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiPanelParseArgs(xui_panel_demo_t* pDemo, int argc, char** argv)
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
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiPanelUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiPanelFindTtf(void)
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

static void __xuiPanelWritePixel(unsigned char* pPixels, int iWidth, int x, int y, uint32_t iColor)
{
	pPixels[((y * iWidth + x) * 4) + 0] = (unsigned char)__xuiPanelColorR(iColor);
	pPixels[((y * iWidth + x) * 4) + 1] = (unsigned char)__xuiPanelColorG(iColor);
	pPixels[((y * iWidth + x) * 4) + 2] = (unsigned char)__xuiPanelColorB(iColor);
	pPixels[((y * iWidth + x) * 4) + 3] = (unsigned char)__xuiPanelColorA(iColor);
}

static int __xuiPanelCreateIconSurface(xui_panel_demo_t* pDemo)
{
	unsigned char arrPixels[24 * 24 * 4];
	int x;
	int y;
	int bLine;
	int bCore;
	uint32_t iColor;

	memset(arrPixels, 0, sizeof(arrPixels));
	for ( y = 0; y < 24; y++ ) {
		for ( x = 0; x < 24; x++ ) {
			bLine = (x == 4) || (x == 19) || (y == 4) || (y == 19);
			bCore = (x >= 8) && (x <= 15) && (y >= 8) && (y <= 15);
			iColor = XUI_COLOR_RGBA(0, 0, 0, 0);
			if ( bCore ) iColor = XUI_COLOR_RGBA(46, 132, 214, 255);
			if ( bLine ) iColor = XUI_COLOR_RGBA(16, 90, 170, 255);
			__xuiPanelWritePixel(arrPixels, 24, x, y, iColor);
		}
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pIconSurface, 24, 24, arrPixels, 24 * 4, XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiPanelRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_panel_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_panel_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
	}
	return XUI_OK;
}

static void __xuiPanelThickness(float fLeft, float fTop, float fRight, float fBottom, xui_thickness_t* pOut)
{
	pOut->fLeft = fLeft;
	pOut->fTop = fTop;
	pOut->fRight = fRight;
	pOut->fBottom = fBottom;
}

static int __xuiPanelAddContentLabel(xui_panel_demo_t* pDemo, xui_widget pPanel, const char* sText, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sText = sText;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pLabel, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	(void)xuiWidgetSetAlign(pLabel, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiPanelAddChild(pPanel, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiPanelCreateOne(xui_panel_demo_t* pDemo, int iIndex, const char* sTitle, xui_rect_t tRect)
{
	xui_panel_desc_t tDesc;
	xui_widget pPanel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = sTitle;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiPanelCreate(pDemo->pContext, &pPanel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pPanel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pPanel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPanel);
		return iRet;
	}
	pDemo->pPanel[iIndex] = pPanel;
	return XUI_OK;
}

static int __xuiPanelCreateUi(xui_panel_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiPanelRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiPanelCreateOne(pDemo, PANEL_DEFAULT, "Default Panel", (xui_rect_t){36.0f, 36.0f, 340.0f, 166.0f});
	if ( iRet == XUI_OK ) iRet = __xuiPanelCreateOne(pDemo, PANEL_ICON, "Icon + Custom Color", (xui_rect_t){414.0f, 36.0f, 340.0f, 166.0f});
	if ( iRet == XUI_OK ) iRet = __xuiPanelCreateOne(pDemo, PANEL_NO_HEADER, "Hidden Header", (xui_rect_t){36.0f, 238.0f, 340.0f, 166.0f});
	if ( iRet == XUI_OK ) iRet = __xuiPanelCreateOne(pDemo, PANEL_CLIP, "Clipped Client", (xui_rect_t){414.0f, 238.0f, 340.0f, 166.0f});
	if ( iRet != XUI_OK ) return iRet;

	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_DEFAULT], "Panel owns header, title and client widgets.", XUI_COLOR_RGBA(44, 58, 76, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_DEFAULT], "Children are attached to the client area.", XUI_COLOR_RGBA(78, 94, 116, 255));

	(void)xuiPanelSetIcon(pDemo->pPanel[PANEL_ICON], pDemo->pIconSurface, (xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f});
	(void)xuiPanelSetIconSize(pDemo->pPanel[PANEL_ICON], 18.0f);
	(void)xuiPanelSetHeaderColor(pDemo->pPanel[PANEL_ICON], XUI_COLOR_RGBA(218, 238, 252, 255));
	(void)xuiPanelSetClientColor(pDemo->pPanel[PANEL_ICON], XUI_COLOR_RGBA(250, 253, 255, 255));
	(void)xuiPanelSetTitleColor(pDemo->pPanel[PANEL_ICON], XUI_COLOR_RGBA(20, 86, 150, 255));
	(void)xuiPanelSetBorder(pDemo->pPanel[PANEL_ICON], 1.0f, XUI_COLOR_RGBA(92, 165, 220, 255));
	(void)xuiPanelSetRadius(pDemo->pPanel[PANEL_ICON], 6.0f);
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_ICON], "Icon surface uses the built-in Image widget.", XUI_COLOR_RGBA(38, 70, 104, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_ICON], "Colors are controlled by Panel APIs.", XUI_COLOR_RGBA(70, 96, 124, 255));

	(void)xuiPanelSetHeaderHeight(pDemo->pPanel[PANEL_NO_HEADER], 0.0f);
	(void)xuiPanelSetClientColor(pDemo->pPanel[PANEL_NO_HEADER], XUI_COLOR_RGBA(247, 251, 249, 255));
	(void)xuiPanelSetBorder(pDemo->pPanel[PANEL_NO_HEADER], 1.0f, XUI_COLOR_RGBA(145, 190, 168, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_NO_HEADER], "Header height can be set to zero.", XUI_COLOR_RGBA(36, 86, 64, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_NO_HEADER], "The client expands to the full panel.", XUI_COLOR_RGBA(70, 104, 88, 255));

	__xuiPanelThickness(12.0f, 12.0f, 12.0f, 12.0f, &tPadding);
	(void)xuiWidgetSetPadding(xuiPanelGetClientWidget(pDemo->pPanel[PANEL_CLIP]), tPadding);
	(void)xuiPanelSetClientClip(pDemo->pPanel[PANEL_CLIP], 1);
	(void)xuiPanelSetHeaderColor(pDemo->pPanel[PANEL_CLIP], XUI_COLOR_RGBA(242, 236, 225, 255));
	(void)xuiPanelSetClientColor(pDemo->pPanel[PANEL_CLIP], XUI_COLOR_RGBA(255, 252, 246, 255));
	(void)xuiPanelSetBorder(pDemo->pPanel[PANEL_CLIP], 1.0f, XUI_COLOR_RGBA(204, 176, 132, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_CLIP], "Client clipping is enabled for bounded groups.", XUI_COLOR_RGBA(98, 78, 48, 255));
	(void)__xuiPanelAddContentLabel(pDemo, pDemo->pPanel[PANEL_CLIP], "This keeps oversized content inside the panel.", XUI_COLOR_RGBA(118, 92, 54, 255));
	return XUI_OK;
}

static int __xuiPanelCreateAssets(xui_panel_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiPanelFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiPanelCreateIconSurface(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiPanelCreateUi(pDemo);
}

static void __xuiPanelDestroyAssets(xui_panel_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pIconSurface != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pIconSurface);
		pDemo->pIconSurface = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static void __xuiPanelRunChecks(xui_panel_demo_t* pDemo)
{
	xui_rect_t tHeader;
	xui_rect_t tClient;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) &&
		(pDemo->pPanel[PANEL_DEFAULT] != NULL) &&
		(xuiPanelGetClientWidget(pDemo->pPanel[PANEL_DEFAULT]) != NULL);
	tHeader = xuiPanelGetHeaderRect(pDemo->pPanel[PANEL_DEFAULT]);
	tClient = xuiPanelGetClientRect(pDemo->pPanel[PANEL_DEFAULT]);
	pDemo->bLayoutOK = (tHeader.fW == 340.0f) && (tHeader.fH == 28.0f) && (tClient.fY == 28.0f) && (tClient.fH > 120.0f);
	pDemo->bTitleOK = (strcmp(xuiPanelGetTitle(pDemo->pPanel[PANEL_DEFAULT]), "Default Panel") == 0) &&
		(strcmp(xuiLabelGetText(xuiPanelGetTitleWidget(pDemo->pPanel[PANEL_ICON])), "Icon + Custom Color") == 0);
	pDemo->bIconOK = (xuiPanelGetIconSurface(pDemo->pPanel[PANEL_ICON]) == pDemo->pIconSurface) &&
		(xuiPanelGetIconRect(pDemo->pPanel[PANEL_ICON]).fW == 18.0f);
	pDemo->bStyleOK = (xuiPanelGetHeaderColor(pDemo->pPanel[PANEL_ICON]) == XUI_COLOR_RGBA(218, 238, 252, 255)) &&
		(xuiPanelGetRadius(pDemo->pPanel[PANEL_ICON]) == 6.0f);
	pDemo->bClientOK = (xuiWidgetGetParent(pDemo->pLabel[0]) == xuiPanelGetClientWidget(pDemo->pPanel[PANEL_DEFAULT])) &&
		(xuiPanelGetClientRect(pDemo->pPanel[PANEL_NO_HEADER]).fY == 0.0f);
	pDemo->bClipOK = xuiPanelGetClientClip(pDemo->pPanel[PANEL_CLIP]) &&
		(xuiWidgetGetOverflow(xuiPanelGetClientWidget(pDemo->pPanel[PANEL_CLIP])) == XUI_OVERFLOW_CLIP);
}

static int __xuiPanelFrame(void* pUser)
{
	xui_panel_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;

	pDemo = (xui_panel_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiPanelRunChecks(pDemo);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(232, 240, 249, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_panel final-summary frames=%d create=%d layout=%d title=%d icon=%d style=%d client=%d clip=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bTitleOK, pDemo->bIconOK,
			pDemo->bStyleOK, pDemo->bClientOK, pDemo->bClipOK, tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_panel_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiPanelParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiPanelUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Panel";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_panel: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiPanelCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_panel: create assets failed: %d\n", iRet);
		__xuiPanelDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiPanelFrame, &tDemo);
	__xuiPanelDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bTitleOK &&
		tDemo.bIconOK && tDemo.bStyleOK && tDemo.bClientOK && tDemo.bClipOK) ? 0 : 1;
}
