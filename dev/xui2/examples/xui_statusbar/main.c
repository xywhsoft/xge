#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 640
#define DEMO_TARGET_H 360
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_statusbar_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pStatusBar;
	xui_widget pTitle;
	xui_widget pStatus;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bProgressOK;
	int bClickOK;
	int bDisabledOK;
	int bKeyboardOK;
	int iSelectCount;
	int iLastIndex;
	int iLastValue;
	int iProgressIndex;
	int iDisabledIndex;
} xui_statusbar_demo_t;

static void __xuiStatusBarUsage(void)
{
	printf("usage: xui_statusbar [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiStatusBarParseArgs(xui_statusbar_demo_t* pDemo, int argc, char** argv)
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
			__xuiStatusBarUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiStatusBarFindTtf(void)
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

static int __xuiStatusBarRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_statusbar_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_statusbar_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 34.0f, tRect.fW - 48.0f, 270.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 8.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiStatusBarSelected(xui_widget pStatusBar, int iIndex, int iValue, void* pUser)
{
	xui_statusbar_demo_t* pDemo;
	char sText[128];

	(void)pStatusBar;
	pDemo = (xui_statusbar_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastIndex = iIndex;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "Selected index=%d value=%d", iIndex, iValue);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiStatusBarAddLabel(xui_statusbar_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(*ppWidget, tRect);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static int __xuiStatusBarCreateUi(xui_statusbar_demo_t* pDemo)
{
	xui_statusbar_desc_t tDesc;
	xui_statusbar_metrics_t tMetrics;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiStatusBarRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiStatusBarAddLabel(pDemo, &pDemo->pTitle, "XUI StatusBar",
		(xui_rect_t){58.0f, 84.0f, 240.0f, 28.0f}, XUI_COLOR_RGBA(34, 56, 82, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiStatusBarAddLabel(pDemo, &pDemo->pStatus, "Selected index=none",
		(xui_rect_t){58.0f, 124.0f, 340.0f, 28.0f}, XUI_COLOR_RGBA(42, 62, 88, 255));
	if ( iRet != XUI_OK ) return iRet;

	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.fHeight = 28.0f;
	tMetrics.fPaddingX = 8.0f;
	tMetrics.fPaddingY = 2.0f;
	tMetrics.fGap = 6.0f;
	tMetrics.fItemPaddingX = 8.0f;
	tMetrics.fItemPaddingY = 3.0f;
	tMetrics.fProgressHeight = 9.0f;
	tMetrics.fRadius = 4.0f;
	tMetrics.fBorderWidth = 1.0f;
	tMetrics.fTopBorderWidth = 1.0f;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	iRet = xuiStatusBarCreate(pDemo->pContext, &pDemo->pStatusBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pStatusBar, (xui_rect_t){34.0f, 316.0f, 572.0f, 28.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pStatusBar);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiStatusBarSetSelect(pDemo->pStatusBar, __xuiStatusBarSelected, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiStatusBarAddText(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, "Ready", 78.0f, 1, 10);
	pDemo->iProgressIndex = xuiStatusBarAddProgress(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, 0.0f, 100.0f, 64.0f, 112.0f);
	(void)xuiStatusBarAddSpacer(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, 18.0f);
	(void)xuiStatusBarAddFlexibleSpacer(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_LEFT, 1.0f);
	(void)xuiStatusBarAddText(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_CENTER, "Scene: sample.xge", 150.0f, 1, 20);
	(void)xuiStatusBarAddText(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "UTF-8", 62.0f, 1, 30);
	pDemo->iDisabledIndex = xuiStatusBarAddText(pDemo->pStatusBar, XUI_STATUSBAR_SECTION_RIGHT, "Read only", 82.0f, 1, 40);
	(void)xuiStatusBarSetItemEnabled(pDemo->pStatusBar, pDemo->iDisabledIndex, 0);
	return XUI_OK;
}

static uint32_t __xuiStatusBarReadButtons(void)
{
	uint32_t iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiStatusBarSendButtonTransitions(xui_statusbar_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiStatusBarSendKey(xui_statusbar_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiStatusBarHandleInput(xui_statusbar_demo_t* pDemo)
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
	iButtons = __xuiStatusBarReadButtons();
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
	iRet = __xuiStatusBarSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarSendKey(pDemo, XGE_KEY_ENTER, XUI_KEY_ENTER);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarSendKey(pDemo, XGE_KEY_SPACE, XUI_KEY_SPACE);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiStatusBarDispatchMove(xui_statusbar_demo_t* pDemo, float fX, float fY)
{
	int iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiStatusBarDispatchClick(xui_statusbar_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = __xuiStatusBarDispatchMove(pDemo, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiStatusBarClickItem(xui_statusbar_demo_t* pDemo, int iIndex)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pDemo->pStatusBar);
	xui_rect_t tItem = xuiStatusBarGetItemRect(pDemo->pStatusBar, iIndex);
	return __xuiStatusBarDispatchClick(pDemo, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

static void __xuiStatusBarRunChecks(xui_statusbar_demo_t* pDemo, int bExerciseInput)
{
	int iBefore;
	xui_rect_t tReady;
	xui_rect_t tProgress;
	xui_rect_t tFlex;
	xui_rect_t tCenter;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pStatusBar != NULL) && (xuiStatusBarGetItemCount(pDemo->pStatusBar) == 7);
	tReady = xuiStatusBarGetItemRect(pDemo->pStatusBar, 0);
	tProgress = xuiStatusBarGetItemRect(pDemo->pStatusBar, pDemo->iProgressIndex);
	tFlex = xuiStatusBarGetItemRect(pDemo->pStatusBar, 3);
	tCenter = xuiStatusBarGetItemRect(pDemo->pStatusBar, 4);
	pDemo->bLayoutOK = (tProgress.fX > tReady.fX) && (tFlex.fW > 0.0f) && (tCenter.fX > 190.0f) && (tCenter.fX < 250.0f);
	if ( pDemo->iProgressIndex >= 0 ) {
		(void)xuiStatusBarSetProgress(pDemo->pStatusBar, pDemo->iProgressIndex, 128.0f);
		pDemo->bProgressOK = (xuiStatusBarGetProgress(pDemo->pStatusBar, pDemo->iProgressIndex) == 100.0f);
	}
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		iBefore = pDemo->iSelectCount;
		(void)__xuiStatusBarClickItem(pDemo, 0);
		pDemo->bClickOK = (pDemo->iSelectCount == iBefore + 1) && (pDemo->iLastValue == 10);
		iBefore = pDemo->iSelectCount;
		(void)__xuiStatusBarClickItem(pDemo, pDemo->iDisabledIndex);
		pDemo->bDisabledOK = (pDemo->iSelectCount == iBefore);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pStatusBar);
		(void)xuiStatusBarSetHoverIndex(pDemo->pStatusBar, 4);
		iBefore = pDemo->iSelectCount;
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_SPACE, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyboardOK = (pDemo->iSelectCount == iBefore + 1) && (pDemo->iLastValue == 20);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bClickOK = 1;
		pDemo->bDisabledOK = 1;
		pDemo->bKeyboardOK = 1;
	}
}

static int __xuiStatusBarCreateAssets(xui_statusbar_demo_t* pDemo)
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
	sFontPath = __xuiStatusBarFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiStatusBarCreateUi(pDemo);
}

static void __xuiStatusBarDestroyAssets(xui_statusbar_demo_t* pDemo)
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

static int __xuiStatusBarFrame(void* pUser)
{
	xui_statusbar_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_statusbar_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiStatusBarHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiStatusBarRunChecks(pDemo, bAutoRun);
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
		memset(&tCacheStats, 0, sizeof(tCacheStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		(void)xuiGetCacheStats(pDemo->pContext, &tCacheStats);
		printf("xui_statusbar final-summary frames=%d create=%d layout=%d progress=%d click=%d disabled=%d keyboard=%d callbacks=%d last=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bProgressOK, pDemo->bClickOK,
			pDemo->bDisabledOK, pDemo->bKeyboardOK, pDemo->iSelectCount, pDemo->iLastValue,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_statusbar_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastIndex = -1;
	tDemo.iLastValue = -1;
	tDemo.iProgressIndex = -1;
	tDemo.iDisabledIndex = -1;
	iRet = __xuiStatusBarParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiStatusBarUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI StatusBar";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_statusbar: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiStatusBarCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_statusbar: create assets failed: %d\n", iRet);
		__xuiStatusBarDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiStatusBarFrame, &tDemo);
	__xuiStatusBarDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bProgressOK &&
		tDemo.bClickOK && tDemo.bDisabledOK && tDemo.bKeyboardOK) ? 0 : 1;
}
