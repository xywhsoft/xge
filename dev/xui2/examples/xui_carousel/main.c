#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 470
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_PAGE_COUNT 4

typedef struct xui_carousel_demo_t xui_carousel_demo_t;

typedef struct xui_carousel_slide_t {
	xui_carousel_demo_t* pDemo;
	const char* sTitle;
	uint32_t iColor;
	uint32_t iAccent;
} xui_carousel_slide_t;

struct xui_carousel_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pCarousel;
	xui_widget pStatus;
	xui_carousel_slide_t arrSlides[DEMO_PAGE_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int iLastOld;
	int iLastNew;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDynamicOK;
};

static void __xuiCarouselDemoUsage(void)
{
	printf("usage: xui_carousel [--frames N] [--seconds N]\n");
}

static int __xuiCarouselDemoParseArgs(xui_carousel_demo_t* pDemo, int argc, char** argv)
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
			__xuiCarouselDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCarouselDemoFindTtf(void)
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

static int __xuiCarouselDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_carousel_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_carousel_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static int __xuiCarouselDemoSlideRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_carousel_slide_t* pSlide;
	xui_rect_t tRect;
	xui_rect_t tBand;

	(void)iStateId;
	pSlide = (xui_carousel_slide_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pSlide == NULL) || (pSlide->pDemo == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pSlide->pDemo->tProxy.drawRectFill != NULL ) {
		(void)pSlide->pDemo->tProxy.drawRectFill(&pSlide->pDemo->tProxy, pDraw, tRect, pSlide->iColor);
		tBand = (xui_rect_t){0.0f, 0.0f, tRect.fW, 7.0f};
		(void)pSlide->pDemo->tProxy.drawRectFill(&pSlide->pDemo->tProxy, pDraw, tBand, pSlide->iAccent);
	}
	return XUI_OK;
}

static void __xuiCarouselDemoChanged(xui_widget pWidget, int iOldIndex, int iNewIndex, void* pUser)
{
	xui_carousel_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_carousel_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
		pDemo->iLastOld = iOldIndex;
		pDemo->iLastNew = iNewIndex;
	}
}

static int __xuiCarouselDemoAddLabel(xui_carousel_demo_t* pDemo, const char* sText, xui_rect_t tRect, xui_widget pParent, uint32_t iColor, uint32_t iFlags, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = iFlags | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pLabel, tRect);
	(void)xuiWidgetSetHitTestVisible(pLabel, 0);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiCarouselDemoCreateSlides(xui_carousel_demo_t* pDemo)
{
	xui_widget pPage;
	char sText[64];
	int i;
	int iRet;

	pDemo->arrSlides[0] = (xui_carousel_slide_t){pDemo, "Item One", XUI_COLOR_RGBA(255, 86, 42, 255), XUI_COLOR_RGBA(255, 188, 80, 255)};
	pDemo->arrSlides[1] = (xui_carousel_slide_t){pDemo, "Item Two", XUI_COLOR_RGBA(38, 142, 214, 255), XUI_COLOR_RGBA(117, 213, 255, 255)};
	pDemo->arrSlides[2] = (xui_carousel_slide_t){pDemo, "Item Three", XUI_COLOR_RGBA(45, 172, 122, 255), XUI_COLOR_RGBA(168, 235, 188, 255)};
	pDemo->arrSlides[3] = (xui_carousel_slide_t){pDemo, "Item Four", XUI_COLOR_RGBA(130, 94, 210, 255), XUI_COLOR_RGBA(222, 184, 255, 255)};
	for ( i = 0; i < DEMO_PAGE_COUNT; i++ ) {
		pPage = xuiCarouselGetPageWidget(pDemo->pCarousel, i);
		if ( pPage == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		(void)xuiWidgetSetCacheRenderCallback(pPage, __xuiCarouselDemoSlideRender, &pDemo->arrSlides[i]);
		snprintf(sText, sizeof(sText), "%s", pDemo->arrSlides[i].sTitle);
		iRet = __xuiCarouselDemoAddLabel(
			pDemo,
			sText,
			(xui_rect_t){0.0f, 0.0f, 540.0f, 280.0f},
			pPage,
			XUI_COLOR_RGBA(255, 255, 255, 255),
			XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE,
			NULL);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCarouselDemoCreateUi(xui_carousel_demo_t* pDemo)
{
	xui_carousel_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCarouselDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiCarouselDemoAddLabel(pDemo, "XUI Carousel", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, pDemo->pRoot, XUI_COLOR_RGBA(45, 58, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iPageCount = DEMO_PAGE_COUNT;
	tDesc.iCurrent = 0;
	tDesc.bAutoPlay = 1;
	tDesc.fAutoInterval = 2.5f;
	tDesc.fArrowSize = 36.0f;
	tDesc.fIndicatorSize = 9.0f;
	tDesc.fIndicatorGap = 7.0f;
	tDesc.fIndicatorBottom = 18.0f;
	iRet = xuiCarouselCreate(pDemo->pContext, &pDemo->pCarousel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pCarousel, (xui_rect_t){90.0f, 82.0f, 540.0f, 280.0f});
	iRet = xuiCarouselSetChange(pDemo->pCarousel, __xuiCarouselDemoChanged, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pCarousel);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCarouselDemoCreateSlides(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiCarouselDemoAddLabel(pDemo, "Hover the carousel to show side arrows. Click arrows or bottom dots to switch pages.", (xui_rect_t){58.0f, 390.0f, 620.0f, 22.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiCarouselDemoAddLabel(pDemo, "current=0 changes=0", (xui_rect_t){58.0f, 420.0f, 620.0f, 22.0f}, pDemo->pRoot, XUI_COLOR_RGBA(74, 88, 108, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE, &pDemo->pStatus);
	return iRet;
}

static uint32_t __xuiCarouselDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiCarouselDemoSendButtonTransitions(xui_carousel_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
{
	int iRet;

	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiCarouselDemoHandleInput(xui_carousel_demo_t* pDemo)
{
	float fX;
	float fY;
	float fUiX;
	float fUiY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	fUiX = fX - DEMO_OFFSET_X;
	fUiY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCarouselDemoReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, fUiX, fUiY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiCarouselDemoSendButtonTransitions(pDemo, fUiX, fUiY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	if ( xgeKeyPressed(XGE_KEY_LEFT) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_LEFT, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_RIGHT) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCarouselDemoUpdateStatus(xui_carousel_demo_t* pDemo)
{
	char sText[160];

	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	snprintf(sText, sizeof(sText), "current=%d changes=%d last=%d->%d auto=2.5s loop=%d",
		xuiCarouselGetCurrent(pDemo->pCarousel),
		pDemo->iChangeCount,
		pDemo->iLastOld,
		pDemo->iLastNew,
		xuiCarouselGetLoop(pDemo->pCarousel));
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static void __xuiCarouselDemoRunChecks(xui_carousel_demo_t* pDemo, int bAutoRun)
{
	int iOk;

	if ( (pDemo == NULL) || !bAutoRun || pDemo->bExerciseDone ) {
		return;
	}
	iOk = (xuiCarouselSetCurrent(pDemo->pCarousel, DEMO_PAGE_COUNT - 1, 1) == XUI_OK);
	iOk = iOk && (xuiCarouselNext(pDemo->pCarousel, 1) == XUI_OK);
	iOk = iOk && (xuiCarouselGetCurrent(pDemo->pCarousel) == 0);
	iOk = iOk && (xuiCarouselSetAutoPlay(pDemo->pCarousel, 1, 0.1f) == XUI_OK);
	pDemo->bDynamicOK = iOk;
	pDemo->bExerciseDone = 1;
}

static int __xuiCarouselDemoCreateAssets(xui_carousel_demo_t* pDemo)
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
	sFontPath = __xuiCarouselDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCarouselDemoCreateUi(pDemo);
}

static void __xuiCarouselDemoDestroyAssets(xui_carousel_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
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

static int __xuiCarouselDemoFrame(void* pUser)
{
	xui_carousel_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_carousel_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCarouselDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCarouselDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCarouselDemoUpdateStatus(pDemo);
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bLayoutOK = (xuiCarouselGetPageWidget(pDemo->pCarousel, xuiCarouselGetCurrent(pDemo->pCarousel)) != NULL);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
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
		printf("xui_carousel final-summary frames=%d create=%d layout=%d dynamic=%d current=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDynamicOK,
			xuiCarouselGetCurrent(pDemo->pCarousel), xuiCarouselGetChangeCount(pDemo->pCarousel),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_carousel_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCarouselDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCarouselDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Carousel";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_carousel: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCarouselDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_carousel: create assets failed: %d\n", iRet);
		__xuiCarouselDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	tDemo.bCreateOK = 1;
	iRet = xgeRun(__xuiCarouselDemoFrame, &tDemo);
	__xuiCarouselDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || tDemo.bDynamicOK)) ? 0 : 1;
}
