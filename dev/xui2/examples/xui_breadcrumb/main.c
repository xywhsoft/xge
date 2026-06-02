#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	430
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define BREADCRUMB_COUNT 4

typedef struct xui_breadcrumb_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_surface pChevronIcon;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pBreadcrumb[BREADCRUMB_COUNT];
	xui_widget pStatus;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bPrevLeftDown;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bClickOK;
	int iClickCount;
	int iLastIndex;
	int iLastValue;
} xui_breadcrumb_demo_t;

static void __xuiBreadcrumbDemoUsage(void)
{
	printf("usage: xui_breadcrumb [--frames N] [--seconds N]\n");
}

static int __xuiBreadcrumbDemoParseArgs(xui_breadcrumb_demo_t* pDemo, int argc, char** argv)
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
			__xuiBreadcrumbDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiBreadcrumbDemoFindTtf(void)
{
	static const char* arrPaths[] = {
		"C:\\Windows\\Fonts\\msyh.ttc",
		"C:\\Windows\\Fonts\\segoeui.ttf",
		"C:\\Windows\\Fonts\\arial.ttf",
		"C:\\Windows\\Fonts\\calibri.ttf",
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

static uint32_t __xuiBreadcrumbDemoPackPremul(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	unsigned int pr = ((unsigned int)r * (unsigned int)a + 127u) / 255u;
	unsigned int pg = ((unsigned int)g * (unsigned int)a + 127u) / 255u;
	unsigned int pb = ((unsigned int)b * (unsigned int)a + 127u) / 255u;
	return (uint32_t)(pr | (pg << 8) | (pb << 16) | ((unsigned int)a << 24));
}

static int __xuiBreadcrumbDemoCreateChevron(xui_breadcrumb_demo_t* pDemo)
{
	uint32_t arrPixels[12 * 12];
	int x;
	int y;
	int i;

	if ( (pDemo == NULL) || (pDemo->tProxy.surfaceCreateRGBA == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; i < 12 * 12; i++ ) arrPixels[i] = 0u;
	for ( y = 0; y < 12; y++ ) {
		for ( x = 0; x < 12; x++ ) {
			int d1 = x - y - 1;
			int d2 = x + y - 10;
			if ( (d1 >= -1 && d1 <= 1 && y >= 2 && y <= 6) ||
			     (d2 >= -1 && d2 <= 1 && y >= 5 && y <= 9) ) {
				arrPixels[y * 12 + x] = __xuiBreadcrumbDemoPackPremul(255, 255, 255, 255);
			}
		}
	}
	return pDemo->tProxy.surfaceCreateRGBA(&pDemo->tProxy, &pDemo->pChevronIcon, 12, 12, arrPixels, 12 * (int)sizeof(uint32_t), XUI_SURFACE_ALPHA_PREMULTIPLIED);
}

static int __xuiBreadcrumbDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_breadcrumb_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_breadcrumb_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiBreadcrumbDemoAddLabel(xui_breadcrumb_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(57, 71, 91, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static void __xuiBreadcrumbDemoClick(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_breadcrumb_demo_t* pDemo;
	char sText[160];

	(void)pWidget;
	pDemo = (xui_breadcrumb_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iClickCount++;
	pDemo->iLastIndex = iIndex;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "clicked index=%d value=%d total=%d", iIndex, iValue, pDemo->iClickCount);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiBreadcrumbDemoAddBreadcrumb(xui_breadcrumb_demo_t* pDemo, int iIndex, const char* sSeparator, xui_surface pIcon, xui_rect_t tRect, const xui_breadcrumb_item_t* pItems)
{
	xui_breadcrumb_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = pItems;
	tDesc.iItemCount = 3;
	tDesc.sSeparator = sSeparator;
	tDesc.pSeparatorIcon = pIcon;
	tDesc.tSeparatorIconSrc = (pIcon != NULL) ? (xui_rect_t){0.0f, 0.0f, 12.0f, 12.0f} : (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	tDesc.fSeparatorIconSize = 12.0f;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiBreadcrumbCreate(pDemo->pContext, &pDemo->pBreadcrumb[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pBreadcrumb[iIndex], tRect);
	iRet = xuiBreadcrumbSetClick(pDemo->pBreadcrumb[iIndex], __xuiBreadcrumbDemoClick, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pBreadcrumb[iIndex]);
}

static int __xuiBreadcrumbDemoCreateUi(xui_breadcrumb_demo_t* pDemo)
{
	xui_breadcrumb_item_t arrDefault[3];
	xui_breadcrumb_item_t arrMusic[3];
	xui_breadcrumb_item_t arrMood[3];
	xui_breadcrumb_item_t arrIcon[3];
	int iRet;

	arrDefault[0] = (xui_breadcrumb_item_t){"Workspace", 1, 101};
	arrDefault[1] = (xui_breadcrumb_item_t){"Console", 1, 102};
	arrDefault[2] = (xui_breadcrumb_item_t){"Visits", 0, 103};
	arrMusic[0] = (xui_breadcrumb_item_t){"Hot music", 1, 201};
	arrMusic[1] = (xui_breadcrumb_item_t){"Wangsu", 1, 202};
	arrMusic[2] = (xui_breadcrumb_item_t){"Who is the prodigal son", 0, 203};
	arrMood[0] = (xui_breadcrumb_item_t){"Today", 1, 301};
	arrMood[1] = (xui_breadcrumb_item_t){"Some", 1, 302};
	arrMood[2] = (xui_breadcrumb_item_t){"Not happy", 0, 303};
	arrIcon[0] = (xui_breadcrumb_item_t){"Today", 1, 401};
	arrIcon[1] = (xui_breadcrumb_item_t){"Some", 1, 402};
	arrIcon[2] = (xui_breadcrumb_item_t){"Not happy", 0, 403};

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiBreadcrumbDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "XUI Breadcrumb", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "Default separator", (xui_rect_t){58.0f, 90.0f, 150.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddBreadcrumb(pDemo, 0, "/", NULL, (xui_rect_t){220.0f, 84.0f, 420.0f, 34.0f}, arrDefault);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "Specified separator", (xui_rect_t){58.0f, 150.0f, 150.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddBreadcrumb(pDemo, 1, ">", NULL, (xui_rect_t){220.0f, 144.0f, 460.0f, 34.0f}, arrMusic);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "Dash separator", (xui_rect_t){58.0f, 210.0f, 150.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddBreadcrumb(pDemo, 2, "-", NULL, (xui_rect_t){220.0f, 204.0f, 420.0f, 34.0f}, arrMood);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "Icon separator", (xui_rect_t){58.0f, 270.0f, 150.0f, 22.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddBreadcrumb(pDemo, 3, "/", pDemo->pChevronIcon, (xui_rect_t){220.0f, 264.0f, 420.0f, 34.0f}, arrIcon);
	if ( iRet == XUI_OK ) iRet = __xuiBreadcrumbDemoAddLabel(pDemo, "Click any clickable node.", (xui_rect_t){58.0f, 352.0f, 500.0f, 22.0f}, &pDemo->pStatus);
	return iRet;
}

static uint32_t __xuiBreadcrumbDemoButtons(void)
{
	uint32_t iButtons = 0;

	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiBreadcrumbDemoProcessInput(xui_breadcrumb_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	int bLeftDown;
	int iRet;

	xgeMouseGet(&fX, &fY);
	fX -= DEMO_OFFSET_X;
	fY -= DEMO_OFFSET_Y;
	iButtons = __xuiBreadcrumbDemoButtons();
	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	bLeftDown = xgeMouseDown(XGE_MOUSE_LEFT) ? 1 : 0;
	if ( bLeftDown && !pDemo->bPrevLeftDown ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	} else if ( !bLeftDown && pDemo->bPrevLeftDown ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bPrevLeftDown = bLeftDown;
	return XUI_OK;
}

static int __xuiBreadcrumbDemoAutoClick(xui_breadcrumb_demo_t* pDemo)
{
	xui_rect_t tRect;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iRet;

	if ( (pDemo == NULL) || pDemo->bExerciseDone ) return XUI_OK;
	tRect = xuiBreadcrumbGetItemRect(pDemo->pBreadcrumb[1], 1);
	tWorld = xuiWidgetGetWorldRect(pDemo->pBreadcrumb[1]);
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	fX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bClickOK = (pDemo->iClickCount > 0 && pDemo->iLastIndex == 1 && pDemo->iLastValue == 202);
	pDemo->bExerciseDone = 1;
	return XUI_OK;
}

static int __xuiBreadcrumbDemoCreateAssets(xui_breadcrumb_demo_t* pDemo)
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
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiBreadcrumbDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	iRet = __xuiBreadcrumbDemoCreateChevron(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiBreadcrumbDemoCreateUi(pDemo);
}

static void __xuiBreadcrumbDemoDestroyAssets(xui_breadcrumb_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pChevronIcon != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pChevronIcon);
		pDemo->pChevronIcon = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiBreadcrumbDemoFrame(void* pUser)
{
	xui_breadcrumb_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_breadcrumb_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	if ( !bAutoRun ) {
		iRet = __xuiBreadcrumbDemoProcessInput(pDemo);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( bAutoRun ) {
		iRet = __xuiBreadcrumbDemoAutoClick(pDemo);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bLayoutOK = (xuiBreadcrumbGetItemRect(pDemo->pBreadcrumb[0], 1).fW > 0.0f) &&
	                   (xuiBreadcrumbGetSeparatorRect(pDemo->pBreadcrumb[3], 0).fW > 0.0f);
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		tStats.iSize = sizeof(tStats);
		tCacheStats.iSize = sizeof(tCacheStats);
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_breadcrumb final-summary frames=%d create=%d layout=%d click=%d clicks=%d lastIndex=%d lastValue=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bClickOK,
			pDemo->iClickCount, pDemo->iLastIndex, pDemo->iLastValue,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_breadcrumb_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastIndex = -1;
	iRet = __xuiBreadcrumbDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiBreadcrumbDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Breadcrumb";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_breadcrumb: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiBreadcrumbDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_breadcrumb: create assets failed: %d\n", iRet);
		__xuiBreadcrumbDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiBreadcrumbDemoFrame, &tDemo);
	__xuiBreadcrumbDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bClickOK)) ? 0 : 1;
}
