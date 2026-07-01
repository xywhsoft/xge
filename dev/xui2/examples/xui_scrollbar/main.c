#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	680
#define DEMO_TARGET_H	360
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define BAR_COUNT	6
#define LABEL_COUNT	6

typedef struct xui_scrollbar_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pBar[BAR_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iChangeCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bFullOK;
	int bCompactOK;
	int bInputOK;
} xui_scrollbar_demo_t;

static void __xuiScrollBarUsage(void)
{
	printf("usage: xui_scrollbar [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiScrollBarParseArgs(xui_scrollbar_demo_t* pDemo, int argc, char** argv)
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
			__xuiScrollBarUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiScrollBarFindTtf(void)
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

static int __xuiScrollBarRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_scrollbar_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_scrollbar_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static void __xuiScrollBarChanged(xui_widget pWidget, float fValue, void* pUser)
{
	xui_scrollbar_demo_t* pDemo;

	(void)pWidget;
	(void)fValue;
	pDemo = (xui_scrollbar_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiScrollBarAddLabel(xui_scrollbar_demo_t* pDemo, int iIndex, const char* sText)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(55, 67, 86, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiScrollBarAddBar(xui_scrollbar_demo_t* pDemo, int iIndex, int iOrientation, int iMode, float fValue)
{
	xui_scrollbar_desc_t tDesc;
	xui_widget pBar;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= BAR_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fValue = fValue;
	tDesc.fPage = 28.0f;
	tDesc.fSmallStep = 5.0f;
	tDesc.fLargeStep = 25.0f;
	tDesc.iOrientation = iOrientation;
	tDesc.iMode = iMode;
	tDesc.iButtonMode = (iMode == XUI_SCROLLBAR_MODE_FULL) ? XUI_SCROLLBAR_BUTTONS_ON : XUI_SCROLLBAR_BUTTONS_OFF;
	iRet = xuiScrollBarCreate(pDemo->pContext, &pBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiScrollBarSetChange(pBar, __xuiScrollBarChanged, pDemo);
	if ( iMode == XUI_SCROLLBAR_MODE_COMPACT ) {
		(void)xuiScrollBarSetMetrics(pBar, 7.0f, 24.0f, -1.0f, 0.0f);
	}
	iRet = xuiWidgetAddChild(pDemo->pRoot, pBar);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pBar);
		return iRet;
	}
	pDemo->pBar[iIndex] = pBar;
	return XUI_OK;
}

static int __xuiScrollBarCreateUi(xui_scrollbar_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiScrollBarRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiScrollBarAddLabel(pDemo, 0, "Full horizontal") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 0, XUI_ORIENTATION_HORIZONTAL, XUI_SCROLLBAR_MODE_FULL, 30.0f) != XUI_OK ||
	     __xuiScrollBarAddLabel(pDemo, 1, "Compact horizontal") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 1, XUI_ORIENTATION_HORIZONTAL, XUI_SCROLLBAR_MODE_COMPACT, 55.0f) != XUI_OK ||
	     __xuiScrollBarAddLabel(pDemo, 2, "Custom full") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 2, XUI_ORIENTATION_HORIZONTAL, XUI_SCROLLBAR_MODE_FULL, 72.0f) != XUI_OK ||
	     __xuiScrollBarAddLabel(pDemo, 3, "Disabled compact") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 3, XUI_ORIENTATION_HORIZONTAL, XUI_SCROLLBAR_MODE_COMPACT, 42.0f) != XUI_OK ||
	     __xuiScrollBarAddLabel(pDemo, 4, "Full vertical") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 4, XUI_ORIENTATION_VERTICAL, XUI_SCROLLBAR_MODE_FULL, 24.0f) != XUI_OK ||
	     __xuiScrollBarAddLabel(pDemo, 5, "Compact vertical") != XUI_OK ||
	     __xuiScrollBarAddBar(pDemo, 5, XUI_ORIENTATION_VERTICAL, XUI_SCROLLBAR_MODE_COMPACT, 66.0f) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiScrollBarSetColors(pDemo->pBar[2],
		XUI_COLOR_RGBA(219, 239, 233, 255),
		XUI_COLOR_RGBA(39, 150, 106, 245),
		XUI_COLOR_RGBA(31, 170, 120, 250),
		XUI_COLOR_RGBA(19, 125, 88, 255),
		XUI_COLOR_RGBA(39, 150, 106, 180),
		XUI_COLOR_RGBA(158, 174, 168, 135));
	(void)xuiScrollBarSetButtonColors(pDemo->pBar[2], XUI_COLOR_RGBA(240, 249, 246, 255), XUI_COLOR_RGBA(28, 118, 82, 255));
	(void)xuiWidgetSetEnabled(pDemo->pBar[3], 0);
	return XUI_OK;
}

static void __xuiScrollBarLayout(xui_scrollbar_demo_t* pDemo)
{
	int i;
	float fY;

	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	for ( i = 0; i < 4; i++ ) {
		fY = 34.0f + (float)i * 48.0f;
		(void)xuiWidgetSetRect(pDemo->pLabel[i], (xui_rect_t){28.0f, fY, 150.0f, 24.0f});
		(void)xuiWidgetSetRect(pDemo->pBar[i], (xui_rect_t){190.0f, fY + 2.0f, 410.0f, (i == 1 || i == 3) ? 16.0f : 24.0f});
	}
	(void)xuiWidgetSetRect(pDemo->pLabel[4], (xui_rect_t){28.0f, 248.0f, 140.0f, 24.0f});
	(void)xuiWidgetSetRect(pDemo->pBar[4], (xui_rect_t){190.0f, 222.0f, 22.0f, 112.0f});
	(void)xuiWidgetSetRect(pDemo->pLabel[5], (xui_rect_t){280.0f, 248.0f, 150.0f, 24.0f});
	(void)xuiWidgetSetRect(pDemo->pBar[5], (xui_rect_t){452.0f, 222.0f, 14.0f, 112.0f});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiScrollBarReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiScrollBarSendButtonTransitions(xui_scrollbar_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiScrollBarSendKey(xui_scrollbar_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiScrollBarHandleInput(xui_scrollbar_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiScrollBarReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiScrollBarSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_LEFT, XUI_KEY_LEFT);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_RIGHT, XUI_KEY_RIGHT);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_UP, XUI_KEY_UP);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_DOWN, XUI_KEY_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_PAGE_UP, XUI_KEY_PAGE_UP);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_PAGE_DOWN, XUI_KEY_PAGE_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_HOME, XUI_KEY_HOME);
	if ( iRet == XUI_OK ) iRet = __xuiScrollBarSendKey(pDemo, XGE_KEY_END, XUI_KEY_END);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiScrollBarRunChecks(xui_scrollbar_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tFullButton;
	xui_rect_t tCompactButton;
	xui_rect_t tThumb;
	xui_rect_t tBar;
	float fX;
	float fY;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < BAR_COUNT; i++ ) {
		if ( pDemo->pBar[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tFullButton = xuiScrollBarGetDecreaseRect(pDemo->pBar[0]);
	tCompactButton = xuiScrollBarGetDecreaseRect(pDemo->pBar[1]);
	tThumb = xuiScrollBarGetThumbRect(pDemo->pBar[1]);
	pDemo->bLayoutOK = (xuiWidgetGetRect(pDemo->pBar[0]).fW > 300.0f) && (xuiWidgetGetRect(pDemo->pBar[4]).fH > 100.0f);
	pDemo->bFullOK = (xuiScrollBarGetMode(pDemo->pBar[0]) == XUI_SCROLLBAR_MODE_FULL) && (tFullButton.fW > 0.0f);
	pDemo->bCompactOK = (xuiScrollBarGetMode(pDemo->pBar[1]) == XUI_SCROLLBAR_MODE_COMPACT) && (tCompactButton.fW == 0.0f) && (tThumb.fH <= 8.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tBar = xuiWidgetGetWorldRect(pDemo->pBar[0]);
		fX = tBar.fX + tBar.fW - 8.0f;
		fY = tBar.fY + tBar.fH * 0.5f;
		(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bInputOK = !bExerciseInput || (pDemo->iChangeCount > 0);
}

static int __xuiScrollBarCreateAssets(xui_scrollbar_demo_t* pDemo)
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
	sFontPath = __xuiScrollBarFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiScrollBarCreateUi(pDemo);
}

static void __xuiScrollBarDestroyAssets(xui_scrollbar_demo_t* pDemo)
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

static int __xuiScrollBarFrame(void* pUser)
{
	xui_scrollbar_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_scrollbar_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiScrollBarHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiScrollBarLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiScrollBarRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(229, 235, 244, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(22, 26, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_scrollbar final-summary frames=%d create=%d layout=%d full=%d compact=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bFullOK, pDemo->bCompactOK, pDemo->bInputOK,
			pDemo->iChangeCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_scrollbar_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiScrollBarParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiScrollBarUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI ScrollBar";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_scrollbar: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiScrollBarCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_scrollbar: create assets failed: %d\n", iRet);
		__xuiScrollBarDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiScrollBarFrame, &tDemo);
	__xuiScrollBarDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bFullOK && tDemo.bCompactOK && tDemo.bInputOK) ? 0 : 1;
}
