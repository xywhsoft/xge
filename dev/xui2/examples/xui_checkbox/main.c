#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	640
#define DEMO_TARGET_H	360
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define CHECK_COUNT	7

typedef struct xui_checkbox_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTitle;
	xui_widget pCheck[CHECK_COUNT];
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
	int bStateOK;
	int bInputOK;
} xui_checkbox_demo_t;

static void __xuiCheckBoxUsage(void)
{
	printf("usage: xui_checkbox [--frames N] [--seconds N]\n");
}

static int __xuiCheckBoxParseArgs(xui_checkbox_demo_t* pDemo, int argc, char** argv)
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
		} else if ( (strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0) ) {
			__xuiCheckBoxUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiCheckBoxFindTtf(void)
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

static int __xuiCheckBoxRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_checkbox_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_checkbox_demo_t*)pUser;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo == NULL ) return XUI_OK;
	return pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tRect, 8.0f, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static int __xuiCheckBoxSetFillLayout(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FILL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetDock(pWidget, XUI_DOCK_FILL);
	return iRet;
}

static int __xuiCheckBoxSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	return iRet;
}

static void __xuiCheckBoxChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	xui_checkbox_demo_t* pDemo;

	(void)pWidget;
	(void)bChecked;
	pDemo = (xui_checkbox_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiCheckBoxAddLabel(xui_checkbox_demo_t* pDemo, const char* sText)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(70, 84, 104, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiCheckBoxSetFixedHeight(pLabel, 22.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
	}
	return iRet;
}

static int __xuiCheckBoxAddCheck(xui_checkbox_demo_t* pDemo, int iIndex, const char* sText, int bChecked, int bEnabled, int bAtlas)
{
	xui_checkbox_desc_t tDesc;
	xui_widget pCheck;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= CHECK_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.bChecked = bChecked;
	tDesc.bUseBuiltinAtlas = bAtlas;
	iRet = xuiCheckBoxCreate(pDemo->pContext, &pCheck, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiCheckBoxSetFixedHeight(pCheck, 28.0f);
	if ( !bEnabled ) {
		(void)xuiWidgetSetEnabled(pCheck, 0);
	}
	(void)xuiCheckBoxSetChange(pCheck, __xuiCheckBoxChanged, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pCheck);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pCheck);
		return iRet;
	}
	pDemo->pCheck[iIndex] = pCheck;
	return XUI_OK;
}

static int __xuiCheckBoxCreateUi(xui_checkbox_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 8.0f);
	tPadding = (xui_thickness_t){28.0f, 22.0f, 28.0f, 22.0f};
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiCheckBoxRootRender, pDemo);
	(void)__xuiCheckBoxSetFillLayout(pDemo->pRoot);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiCheckBoxAddLabel(pDemo, "Default style") != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 0, "Unchecked option", 0, 1, 0) != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 1, "Checked option", 1, 1, 0) != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 2, "Disabled unchecked", 0, 0, 0) != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 3, "Disabled checked", 1, 0, 0) != XUI_OK ||
	     __xuiCheckBoxAddLabel(pDemo, "Custom style") != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 4, "Green accent, compact indicator", 1, 1, 0) != XUI_OK ||
	     __xuiCheckBoxAddLabel(pDemo, "Built-in atlas") != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 5, "Atlas unchecked", 0, 1, 1) != XUI_OK ||
	     __xuiCheckBoxAddCheck(pDemo, 6, "Atlas checked", 1, 1, 1) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiCheckBoxSetIndicatorSize(pDemo->pCheck[4], 16.0f);
	(void)xuiCheckBoxSetGap(pDemo->pCheck[4], 9.0f);
	(void)xuiCheckBoxSetColors(pDemo->pCheck[4],
		XUI_COLOR_RGBA(24, 166, 113, 255),
		XUI_COLOR_RGBA(156, 181, 170, 255),
		XUI_COLOR_RGBA(72, 196, 146, 255),
		XUI_COLOR_RGBA(24, 166, 113, 255));
	return XUI_OK;
}

static void __xuiCheckBoxLayout(xui_checkbox_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiCheckBoxReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) {
		iButtons |= XUI_POINTER_BUTTON_LEFT;
	}
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) {
		iButtons |= XUI_POINTER_BUTTON_RIGHT;
	}
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) {
		iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	}
	return iButtons;
}

static int __xuiCheckBoxSendButtonTransitions(xui_checkbox_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiCheckBoxHandleInput(xui_checkbox_demo_t* pDemo)
{
	float fX;
	float fY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	int iRet;

	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		xgeQuit();
	}
	xgeMouseGet(&fX, &fY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiCheckBoxReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiCheckBoxSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiCheckBoxRunChecks(xui_checkbox_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tRect;
	xui_rect_t tCheck;
	float fX;
	float fY;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL);
	for ( i = 0; i < CHECK_COUNT; i++ ) {
		if ( pDemo->pCheck[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tRect = xuiWidgetGetRect(pDemo->pCheck[0]);
	pDemo->bLayoutOK = (tRect.fW > 100.0f) && (tRect.fH >= 24.0f);
	pDemo->bStateOK = (xuiCheckBoxGetChecked(pDemo->pCheck[1]) != 0) &&
	                  (xuiWidgetGetEnabled(pDemo->pCheck[2]) == 0) &&
	                  (xuiCheckBoxGetIndicatorSize(pDemo->pCheck[4]) == 16.0f) &&
	                  (xuiCheckBoxGetUseBuiltinAtlas(pDemo->pCheck[6]) != 0);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tCheck = xuiWidgetGetWorldRect(pDemo->pCheck[0]);
		fX = tCheck.fX + tCheck.fW * 0.5f;
		fY = tCheck.fY + tCheck.fH * 0.5f;
		(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bInputOK = !bExerciseInput || ((pDemo->iChangeCount > 0) && (xuiCheckBoxGetChecked(pDemo->pCheck[0]) != 0));
}

static int __xuiCheckBoxCreateAssets(xui_checkbox_demo_t* pDemo)
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
	sFontPath = __xuiCheckBoxFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, 0);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiCheckBoxCreateUi(pDemo);
}

static void __xuiCheckBoxDestroyAssets(xui_checkbox_demo_t* pDemo)
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

static int __xuiCheckBoxFrame(void* pUser)
{
	xui_checkbox_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_checkbox_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiCheckBoxHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiCheckBoxLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiCheckBoxRunChecks(pDemo, bAutoRun);
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
		printf("xui_checkbox final-summary frames=%d create=%d layout=%d state=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bInputOK, pDemo->iChangeCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_checkbox_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiCheckBoxParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiCheckBoxUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Checkbox";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_checkbox: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiCheckBoxCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_checkbox: create assets failed: %d\n", iRet);
		__xuiCheckBoxDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiCheckBoxFrame, &tDemo);
	__xuiCheckBoxDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bInputOK) ? 0 : 1;
}
