#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 780
#define DEMO_TARGET_H 430
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_BUTTON_COUNT 5

typedef struct xui_toast_demo_t xui_toast_demo_t;

typedef struct xui_toast_action_t {
	xui_toast_demo_t* pDemo;
	int iType;
	const char* sTitle;
	const char* sMessage;
	float fDuration;
} xui_toast_action_t;

struct xui_toast_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget arrButtons[DEMO_BUTTON_COUNT];
	xui_toast_action_t arrActions[DEMO_BUTTON_COUNT];
	xui_toast pToast;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iClickCount;
	int iCloseCount;
	int iLastReason;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bExpireOK;
	int bClickOK;
	int bButtonOK;
};

static void __xuiToastUsage(void)
{
	printf("usage: xui_toast [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiToastParseArgs(xui_toast_demo_t* pDemo, int argc, char** argv)
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
			__xuiToastUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiToastFindTtf(void)
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

static int __xuiToastRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_toast_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_toast_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
		tPanel = (xui_rect_t){38.0f, 42.0f, tRect.fW - 76.0f, 315.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(247, 251, 255, 255));
	}
	if ( pDemo->tProxy.drawText != NULL ) {
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "XUI Toast", (xui_rect_t){58.0f, 58.0f, 220.0f, 28.0f}, XUI_COLOR_RGBA(34, 52, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "Queue, click callback, close button, and automatic timeout.", (xui_rect_t){58.0f, 245.0f, 520.0f, 28.0f}, XUI_COLOR_RGBA(70, 88, 112, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static void __xuiToastClick(xui_toast pToast, int iToastId, void* pUser)
{
	xui_toast_demo_t* pDemo;

	(void)pToast;
	(void)iToastId;
	pDemo = (xui_toast_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iClickCount++;
	}
}

static void __xuiToastClose(xui_toast pToast, int iToastId, int iReason, void* pUser)
{
	xui_toast_demo_t* pDemo;

	(void)pToast;
	(void)iToastId;
	pDemo = (xui_toast_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iCloseCount++;
		pDemo->iLastReason = iReason;
	}
}

static void __xuiToastButtonClick(xui_widget pWidget, void* pUser)
{
	xui_toast_action_t* pAction;

	(void)pWidget;
	pAction = (xui_toast_action_t*)pUser;
	if ( (pAction != NULL) && (pAction->pDemo != NULL) && (pAction->pDemo->pToast != NULL) ) {
		(void)xuiToastShow(pAction->pDemo->pToast, pAction->iType, pAction->sTitle, pAction->sMessage, pAction->fDuration, __xuiToastClick, pAction->pDemo);
	}
}

static void __xuiToastClearClick(xui_widget pWidget, void* pUser)
{
	xui_toast_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_toast_demo_t*)pUser;
	if ( (pDemo != NULL) && (pDemo->pToast != NULL) ) {
		(void)xuiToastClear(pDemo->pToast);
	}
}

static int __xuiToastAddButton(xui_toast_demo_t* pDemo, int iIndex, const char* sText, float fX, float fY)
{
	xui_button_desc_t tDesc;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= DEMO_BUTTON_COUNT) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.fBorderWidth = 1.0f;
	tDesc.iBorderColor = XUI_COLOR_RGBA(162, 194, 224, 255);
	iRet = xuiButtonCreate(pDemo->pContext, &pDemo->arrButtons[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetSetRect(pDemo->arrButtons[iIndex], (xui_rect_t){fX, fY, 112.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->arrButtons[iIndex]);
	return iRet;
}

static int __xuiToastCreateUi(xui_toast_demo_t* pDemo)
{
	xui_toast_desc_t tToastDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiToastRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiToastAddButton(pDemo, 0, "Success", 58.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiToastAddButton(pDemo, 1, "Info", 184.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiToastAddButton(pDemo, 2, "Warning", 310.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiToastAddButton(pDemo, 3, "Error", 436.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiToastAddButton(pDemo, 4, "Clear", 562.0f, 116.0f);
	if ( iRet != XUI_OK ) return iRet;

	pDemo->arrActions[0] = (xui_toast_action_t){pDemo, XUI_TOAST_TYPE_SUCCESS, "Saved", "Project settings were saved.", 3.0f};
	pDemo->arrActions[1] = (xui_toast_action_t){pDemo, XUI_TOAST_TYPE_INFO, "Build finished", "Click to open the report.", 5.0f};
	pDemo->arrActions[2] = (xui_toast_action_t){pDemo, XUI_TOAST_TYPE_WARNING, "Network", "Remote sync is slow. Local cache is being used.", 4.0f};
	pDemo->arrActions[3] = (xui_toast_action_t){pDemo, XUI_TOAST_TYPE_ERROR, "Export failed", "Check the output path and try again.", 4.0f};
	iRet = xuiButtonSetClick(pDemo->arrButtons[0], __xuiToastButtonClick, &pDemo->arrActions[0]);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(pDemo->arrButtons[1], __xuiToastButtonClick, &pDemo->arrActions[1]);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(pDemo->arrButtons[2], __xuiToastButtonClick, &pDemo->arrActions[2]);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(pDemo->arrButtons[3], __xuiToastButtonClick, &pDemo->arrActions[3]);
	if ( iRet == XUI_OK ) iRet = xuiButtonSetClick(pDemo->arrButtons[4], __xuiToastClearClick, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tToastDesc, 0, sizeof(tToastDesc));
	tToastDesc.iSize = sizeof(tToastDesc);
	tToastDesc.pFont = pDemo->pFont;
	tToastDesc.iPlacement = XUI_TOAST_PLACEMENT_TOP_RIGHT;
	tToastDesc.iDirection = XUI_TOAST_DIRECTION_AUTO;
	tToastDesc.bHasMetrics = 1;
	tToastDesc.tMetrics.iSize = sizeof(tToastDesc.tMetrics);
	tToastDesc.tMetrics.fWidth = 320.0f;
	tToastDesc.tMetrics.fMinWidth = 180.0f;
	tToastDesc.tMetrics.fMinHeight = 58.0f;
	tToastDesc.tMetrics.fMargin = 18.0f;
	tToastDesc.tMetrics.fGap = 8.0f;
	tToastDesc.tMetrics.fPaddingX = 14.0f;
	tToastDesc.tMetrics.fPaddingY = 10.0f;
	tToastDesc.tMetrics.fIconSize = 28.0f;
	tToastDesc.tMetrics.fIconGap = 10.0f;
	tToastDesc.tMetrics.fCloseSize = 16.0f;
	tToastDesc.tMetrics.fProgressHeight = 2.0f;
	tToastDesc.tMetrics.iMaxVisible = 3;
	iRet = xuiToastCreate(pDemo->pContext, &pDemo->pToast, &tToastDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiToastSetClose(pDemo->pToast, __xuiToastClose, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_SUCCESS, "Saved", "Project settings were saved.", 3.0f, NULL, NULL);
	(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_INFO, "Build finished", "Click to open the report.", 5.0f, __xuiToastClick, pDemo);
	(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_WARNING, "Network", "Remote sync is slow. Local cache is being used.", 4.0f, __xuiToastClick, pDemo);
	return XUI_OK;
}

static uint32_t __xuiToastReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiToastSendButtonTransitions(xui_toast_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiToastHandleInput(xui_toast_demo_t* pDemo)
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
		return XUI_OK;
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiToastReadButtons();
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
	iRet = __xuiToastSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiToastDispatchPointerDown(xui_toast_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiToastRunChecks(xui_toast_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tItem;
	xui_rect_t tClose;
	int iBeforeExpire;
	int iBeforeClick;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pToast != NULL) && (pDemo->arrButtons[0] != NULL);
	tItem = xuiToastGetItemRect(pDemo->pToast, 0);
	if ( (xuiToastGetActiveCount(pDemo->pToast) > 0) && (tItem.fW >= 180.0f) && (tItem.fH >= 58.0f) ) {
		pDemo->bLayoutOK = 1;
	}
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		(void)xuiToastClear(pDemo->pToast);
		iBeforeExpire = xuiToastGetExpireCount(pDemo->pToast);
		(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_SUCCESS, "Quick", "Automatic timeout check.", 0.02f, NULL, NULL);
		(void)xuiLayout(pDemo->pContext);
		(void)xuiUpdate(pDemo->pContext, 0.03f);
		pDemo->bExpireOK = (xuiToastGetActiveCount(pDemo->pToast) == 0) && (xuiToastGetExpireCount(pDemo->pToast) == iBeforeExpire + 1);
		iBeforeClick = pDemo->iClickCount;
		(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_INFO, "Click", "Body click should call the click callback.", 5.0f, __xuiToastClick, pDemo);
		(void)xuiLayout(pDemo->pContext);
		tItem = xuiToastGetItemRect(pDemo->pToast, 0);
		(void)__xuiToastDispatchPointerDown(pDemo, tItem.fX + tItem.fW * 0.5f, tItem.fY + tItem.fH * 0.5f);
		(void)xuiUpdate(pDemo->pContext, 0.0f);
		pDemo->bClickOK = (pDemo->iClickCount == iBeforeClick + 1) && (pDemo->iLastReason == XUI_TOAST_CLOSE_CLICK);
		(void)xuiToastShow(pDemo->pToast, XUI_TOAST_TYPE_ERROR, "Close", "Close button should not call the click callback.", 5.0f, __xuiToastClick, pDemo);
		(void)xuiLayout(pDemo->pContext);
		tClose = xuiToastGetCloseRect(pDemo->pToast, 0);
		iBeforeClick = pDemo->iClickCount;
		(void)__xuiToastDispatchPointerDown(pDemo, tClose.fX + tClose.fW * 0.5f, tClose.fY + tClose.fH * 0.5f);
		(void)xuiUpdate(pDemo->pContext, 0.0f);
		pDemo->bButtonOK = (pDemo->iClickCount == iBeforeClick) && (pDemo->iLastReason == XUI_TOAST_CLOSE_BUTTON);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bExpireOK = 1;
		pDemo->bClickOK = 1;
		pDemo->bButtonOK = 1;
	}
}

static int __xuiToastCreateAssets(xui_toast_demo_t* pDemo)
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
	sFontPath = __xuiToastFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiToastCreateUi(pDemo);
}

static void __xuiToastDestroyAssets(xui_toast_demo_t* pDemo)
{
	if ( pDemo->pToast != NULL ) {
		xuiToastDestroy(pDemo->pToast);
		pDemo->pToast = NULL;
	}
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

static int __xuiToastFrame(void* pUser)
{
	xui_toast_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_toast_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiToastHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiToastRunChecks(pDemo, bAutoRun);
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
		printf("xui_toast final-summary frames=%d create=%d layout=%d expire=%d click=%d button=%d active=%d pending=%d show=%d close=%d expired=%d callbacks=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bExpireOK, pDemo->bClickOK, pDemo->bButtonOK,
			xuiToastGetActiveCount(pDemo->pToast), xuiToastGetPendingCount(pDemo->pToast), xuiToastGetShowCount(pDemo->pToast),
			xuiToastGetCloseCount(pDemo->pToast), xuiToastGetExpireCount(pDemo->pToast), pDemo->iCloseCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_toast_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiToastParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiToastUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Toast";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_toast: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiToastCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_toast: create assets failed: %d\n", iRet);
		__xuiToastDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiToastFrame, &tDemo);
	__xuiToastDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bExpireOK && tDemo.bClickOK && tDemo.bButtonOK) ? 0 : 1;
}
