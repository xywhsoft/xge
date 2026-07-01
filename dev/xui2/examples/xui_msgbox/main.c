#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 760
#define DEMO_TARGET_H 430
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_msgbox_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_msgbox pBox;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iResultCount;
	int iLastResult;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bEnterOK;
	int bCustomOK;
} xui_msgbox_demo_t;

static void __xuiMsgBoxUsage(void)
{
	printf("usage: xui_msgbox [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMsgBoxParseArgs(xui_msgbox_demo_t* pDemo, int argc, char** argv)
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
			__xuiMsgBoxUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMsgBoxFindTtf(void)
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

static int __xuiMsgBoxRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_msgbox_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tBand;

	(void)iStateId;
	pDemo = (xui_msgbox_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
		tBand = (xui_rect_t){38.0f, 42.0f, tRect.fW - 76.0f, 310.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tBand, XUI_COLOR_RGBA(247, 251, 255, 255));
	}
	if ( pDemo->tProxy.drawText != NULL ) {
		(void)pDemo->tProxy.drawText(&pDemo->tProxy, pDraw, pDemo->pFont, "XUI MsgBox", (xui_rect_t){58.0f, 58.0f, 220.0f, 28.0f}, XUI_COLOR_RGBA(34, 52, 78, 255), XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static void __xuiMsgBoxResult(xui_msgbox pBox, int iResult, void* pUser)
{
	xui_msgbox_demo_t* pDemo;

	(void)pBox;
	pDemo = (xui_msgbox_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iResultCount++;
		pDemo->iLastResult = iResult;
	}
}

static int __xuiMsgBoxCreateUi(xui_msgbox_demo_t* pDemo)
{
	xui_msgbox_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiMsgBoxRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Project";
	tDesc.sMessage = "The scene has unsaved changes. Save it before opening another project?";
	tDesc.pFont = pDemo->pFont;
	tDesc.iType = XUI_MSGBOX_ICON_QUEST;
	tDesc.iButtons = XUI_MSGBOX_BUTTON_YES_NO_CANCEL;
	tDesc.bHasModal = 1;
	tDesc.bModal = 1;
	iRet = xuiMsgBoxCreate(pDemo->pContext, &pDemo->pBox, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMsgBoxSetResult(pDemo->pBox, __xuiMsgBoxResult, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return xuiMsgBoxSetOpen(pDemo->pBox, 1);
}

static uint32_t __xuiMsgBoxReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMsgBoxSendButtonTransitions(xui_msgbox_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiMsgBoxSendKey(xui_msgbox_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiMsgBoxHandleInput(xui_msgbox_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) && !xuiMsgBoxIsOpen(pDemo->pBox) ) {
		xgeQuit();
		return XUI_OK;
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiMsgBoxReadButtons();
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
	iRet = __xuiMsgBoxSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet == XUI_OK ) iRet = __xuiMsgBoxSendKey(pDemo, XGE_KEY_ENTER, XUI_KEY_ENTER);
	if ( iRet == XUI_OK ) iRet = __xuiMsgBoxSendKey(pDemo, XGE_KEY_ESCAPE, XUI_KEY_ESCAPE);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiMsgBoxDispatchMove(xui_msgbox_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiMsgBoxDispatchClick(xui_msgbox_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = __xuiMsgBoxDispatchMove(pDemo, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiMsgBoxRunChecks(xui_msgbox_demo_t* pDemo, int bExerciseInput)
{
	xui_msgbox_button_t arrButtons[2];
	xui_rect_t tIcon;
	xui_rect_t tMessage;
	xui_rect_t tButton;
	xui_rect_t tWorld;
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pBox != NULL) && (xuiMsgBoxGetButtonCount(pDemo->pBox) >= 1);
	tIcon = xuiMsgBoxGetIconRect(pDemo->pBox);
	tMessage = xuiMsgBoxGetMessageRect(pDemo->pBox);
	pDemo->bLayoutOK = (tIcon.fW > 0.0f) && (tMessage.fW > 80.0f) && (xuiMsgBoxGetWrapLineCount(pDemo->pBox) >= 1);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		(void)xuiSetFocusWidget(pDemo->pContext, xuiMsgBoxGetWindowWidget(pDemo->pBox));
		iBefore = pDemo->iResultCount;
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bEnterOK = (!xuiMsgBoxIsOpen(pDemo->pBox)) && (pDemo->iResultCount == iBefore + 1);
		arrButtons[0].sText = "Retry";
		arrButtons[0].iResult = 10;
		arrButtons[0].iSemantic = XUI_BUTTON_SEMANTIC_PRIMARY;
		arrButtons[1].sText = "Ignore";
		arrButtons[1].iResult = 20;
		arrButtons[1].iSemantic = XUI_BUTTON_SEMANTIC_DEFAULT;
		(void)xuiMsgBoxSetType(pDemo->pBox, XUI_MSGBOX_ICON_WAR);
		(void)xuiMsgBoxSetCustomButtons(pDemo->pBox, arrButtons, 2);
		(void)xuiMsgBoxSetOpen(pDemo->pBox, 1);
		(void)xuiLayout(pDemo->pContext);
		tButton = xuiMsgBoxGetButtonRect(pDemo->pBox, 1);
		tWorld = xuiWidgetGetWorldRect(xuiMsgBoxGetContentWidget(pDemo->pBox));
		iBefore = pDemo->iResultCount;
		(void)__xuiMsgBoxDispatchClick(pDemo, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
		pDemo->bCustomOK = (!xuiMsgBoxIsOpen(pDemo->pBox)) && (pDemo->iResultCount == iBefore + 1) && (pDemo->iLastResult == 20);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bEnterOK = 1;
		pDemo->bCustomOK = 1;
	}
}

static int __xuiMsgBoxCreateAssets(xui_msgbox_demo_t* pDemo)
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
	sFontPath = __xuiMsgBoxFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMsgBoxCreateUi(pDemo);
}

static void __xuiMsgBoxDestroyAssets(xui_msgbox_demo_t* pDemo)
{
	if ( pDemo->pBox != NULL ) {
		xuiMsgBoxDestroy(pDemo->pBox);
		pDemo->pBox = NULL;
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

static int __xuiMsgBoxFrame(void* pUser)
{
	xui_msgbox_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_msgbox_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMsgBoxHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiMsgBoxRunChecks(pDemo, bAutoRun);
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
		printf("xui_msgbox final-summary frames=%d create=%d layout=%d enter=%d custom=%d results=%d last=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bEnterOK, pDemo->bCustomOK,
			pDemo->iResultCount, pDemo->iLastResult, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_msgbox_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastResult = XUI_MSGBOX_RESULT_CLOSE;
	iRet = __xuiMsgBoxParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiMsgBoxUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI MsgBox";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_msgbox: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMsgBoxCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_msgbox: create assets failed: %d\n", iRet);
		__xuiMsgBoxDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiMsgBoxFrame, &tDemo);
	__xuiMsgBoxDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bEnterOK && tDemo.bCustomOK) ? 0 : 1;
}
