#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 430
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define DEMO_BUTTON_COUNT 4

typedef struct xui_msgtip_demo_t xui_msgtip_demo_t;

typedef struct xui_msgtip_action_t {
	xui_msgtip_demo_t* pDemo;
	int iType;
	const char* sText;
	float fDuration;
} xui_msgtip_action_t;

struct xui_msgtip_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget arrButtons[DEMO_BUTTON_COUNT];
	xui_msgtip pTip;
	xui_msgtip_action_t arrActions[DEMO_BUTTON_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iCloseCount;
	int iLastExpired;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bExpireOK;
	int bClickOK;
};

static void __xuiMsgTipUsage(void)
{
	printf("usage: xui_msgtip [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMsgTipParseArgs(xui_msgtip_demo_t* pDemo, int argc, char** argv)
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
			__xuiMsgTipUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMsgTipFindTtf(void)
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

static int __xuiMsgTipRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgtip_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_msgtip_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
		tPanel = (xui_rect_t){38.0f, 42.0f, tRect.fW - 76.0f, 310.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(247, 251, 255, 255));
	}
	if ( pDemo->tProxy.drawText != NULL ) {
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "XUI MsgTip", (xui_rect_t){58.0f, 58.0f, 220.0f, 28.0f}, XUI_COLOR_RGBA(34, 52, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "Click a button, then click the tip itself to close it.", (xui_rect_t){58.0f, 242.0f, 420.0f, 28.0f}, XUI_COLOR_RGBA(70, 88, 112, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static void __xuiMsgTipClose(xui_msgtip pTip, int bExpired, void* pUser)
{
	xui_msgtip_demo_t* pDemo;

	(void)pTip;
	pDemo = (xui_msgtip_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iCloseCount++;
		pDemo->iLastExpired = bExpired ? 1 : 0;
	}
}

static void __xuiMsgTipButtonClick(xui_widget pWidget, void* pUser)
{
	xui_msgtip_action_t* pAction;

	(void)pWidget;
	pAction = (xui_msgtip_action_t*)pUser;
	if ( (pAction != NULL) && (pAction->pDemo != NULL) && (pAction->pDemo->pTip != NULL) ) {
		(void)xuiMsgTipShow(pAction->pDemo->pTip, pAction->iType, pAction->sText, pAction->fDuration);
	}
}

static int __xuiMsgTipAddButton(xui_msgtip_demo_t* pDemo, int iIndex, const char* sText, int iType, const char* sTipText, float fDuration, float fX, float fY)
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
	pDemo->arrActions[iIndex].pDemo = pDemo;
	pDemo->arrActions[iIndex].iType = iType;
	pDemo->arrActions[iIndex].sText = sTipText;
	pDemo->arrActions[iIndex].fDuration = fDuration;
	iRet = xuiButtonSetClick(pDemo->arrButtons[iIndex], __xuiMsgTipButtonClick, &pDemo->arrActions[iIndex]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDemo->arrButtons[iIndex], (xui_rect_t){fX, fY, 112.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->arrButtons[iIndex]);
	return iRet;
}

static int __xuiMsgTipCreateUi(xui_msgtip_demo_t* pDemo)
{
	xui_msgtip_desc_t tTipDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiMsgTipRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMsgTipAddButton(pDemo, 0, "Info", XUI_MSGTIP_ICON_INFO, "Saved successfully", 1.8f, 58.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiMsgTipAddButton(pDemo, 1, "Question", XUI_MSGTIP_ICON_QUEST, "Apply the current preset?", 2.2f, 184.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiMsgTipAddButton(pDemo, 2, "Warning", XUI_MSGTIP_ICON_WAR, "Network is slow. Local cache is being used.", 2.6f, 310.0f, 116.0f);
	if ( iRet == XUI_OK ) iRet = __xuiMsgTipAddButton(pDemo, 3, "Error", XUI_MSGTIP_ICON_ERROR, "Export failed. Please check the output path.", 0.0f, 436.0f, 116.0f);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tTipDesc, 0, sizeof(tTipDesc));
	tTipDesc.iSize = sizeof(tTipDesc);
	tTipDesc.sText = "Saved successfully";
	tTipDesc.pFont = pDemo->pFont;
	tTipDesc.iType = XUI_MSGTIP_ICON_INFO;
	tTipDesc.fDuration = 1.8f;
	iRet = xuiMsgTipCreate(pDemo->pContext, &pDemo->pTip, &tTipDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMsgTipSetClose(pDemo->pTip, __xuiMsgTipClose, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return xuiMsgTipShow(pDemo->pTip, XUI_MSGTIP_ICON_INFO, "Saved successfully", 1.8f);
}

static uint32_t __xuiMsgTipReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMsgTipSendButtonTransitions(xui_msgtip_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiMsgTipHandleInput(xui_msgtip_demo_t* pDemo)
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
	iButtons = __xuiMsgTipReadButtons();
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
	iRet = __xuiMsgTipSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiMsgTipDispatchClick(xui_msgtip_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
	return iRet;
}

static void __xuiMsgTipRunChecks(xui_msgtip_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tTip;
	xui_rect_t tIcon;
	xui_rect_t tText;
	int iBeforeExpire;
	int iBeforeClose;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pTip != NULL) && (pDemo->arrButtons[0] != NULL);
	tTip = xuiMsgTipGetTipRect(pDemo->pTip);
	tIcon = xuiMsgTipGetIconRect(pDemo->pTip);
	tText = xuiMsgTipGetTextRect(pDemo->pTip);
	pDemo->bLayoutOK = (tTip.fW >= 92.0f) && (tIcon.fW > 0.0f) && (tText.fW > 40.0f) && (xuiMsgTipGetWrapLineCount(pDemo->pTip) >= 1);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		iBeforeExpire = xuiMsgTipGetExpireCount(pDemo->pTip);
		(void)xuiMsgTipShow(pDemo->pTip, XUI_MSGTIP_ICON_WAR, "Automatic duration check", 0.02f);
		(void)xuiLayout(pDemo->pContext);
		(void)xuiUpdate(pDemo->pContext, 0.03f);
		pDemo->bExpireOK = (!xuiMsgTipIsOpen(pDemo->pTip)) && (xuiMsgTipGetExpireCount(pDemo->pTip) == iBeforeExpire + 1);
		(void)xuiMsgTipShow(pDemo->pTip, XUI_MSGTIP_ICON_ERROR, "Click-close check", 0.0f);
		(void)xuiLayout(pDemo->pContext);
		tTip = xuiMsgTipGetTipRect(pDemo->pTip);
		iBeforeClose = xuiMsgTipGetCloseCount(pDemo->pTip);
		(void)__xuiMsgTipDispatchClick(pDemo, tTip.fX + tTip.fW * 0.5f, tTip.fY + tTip.fH * 0.5f);
		pDemo->bClickOK = (!xuiMsgTipIsOpen(pDemo->pTip)) && (xuiMsgTipGetCloseCount(pDemo->pTip) == iBeforeClose + 1);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bExpireOK = 1;
		pDemo->bClickOK = 1;
	}
}

static int __xuiMsgTipCreateAssets(xui_msgtip_demo_t* pDemo)
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
	sFontPath = __xuiMsgTipFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMsgTipCreateUi(pDemo);
}

static void __xuiMsgTipDestroyAssets(xui_msgtip_demo_t* pDemo)
{
	if ( pDemo->pTip != NULL ) {
		xuiMsgTipDestroy(pDemo->pTip);
		pDemo->pTip = NULL;
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

static int __xuiMsgTipFrame(void* pUser)
{
	xui_msgtip_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_msgtip_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMsgTipHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiMsgTipRunChecks(pDemo, bAutoRun);
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
		printf("xui_msgtip final-summary frames=%d create=%d layout=%d expire=%d click=%d show=%d close=%d expired=%d callbacks=%d lastExpired=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bExpireOK, pDemo->bClickOK,
			xuiMsgTipGetShowCount(pDemo->pTip), xuiMsgTipGetCloseCount(pDemo->pTip), xuiMsgTipGetExpireCount(pDemo->pTip),
			pDemo->iCloseCount, pDemo->iLastExpired, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_msgtip_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiMsgTipParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiMsgTipUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI MsgTip";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_msgtip: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMsgTipCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_msgtip: create assets failed: %d\n", iRet);
		__xuiMsgTipDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiMsgTipFrame, &tDemo);
	__xuiMsgTipDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bExpireOK && tDemo.bClickOK) ? 0 : 1;
}
