#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	420
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define LABEL_COUNT	16

typedef struct xui_tabs_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pTabs;
	xui_widget pLabel[LABEL_COUNT];
	int iLabelCount;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iSelectCount;
	int iCloseCount;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bDisabledOK;
	int bKeyboardOK;
	int bCloseOK;
	int bOverflowOK;
	int bMetaOK;
} xui_tabs_demo_t;

static void __xuiTabsUsage(void)
{
	printf("usage: xui_tabs [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiTabsParseArgs(xui_tabs_demo_t* pDemo, int argc, char** argv)
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
			__xuiTabsUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiTabsFindTtf(void)
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

static int __xuiTabsRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_tabs_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_tabs_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
}

static void __xuiTabsSelected(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_demo_t* pDemo;

	(void)pWidget;
	(void)iIndex;
	pDemo = (xui_tabs_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSelectCount++;
	}
}

static void __xuiTabsClosed(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_demo_t* pDemo;

	(void)pWidget;
	(void)iIndex;
	pDemo = (xui_tabs_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iCloseCount++;
	}
}

static int __xuiTabsAddLabel(xui_tabs_demo_t* pDemo, int iPage, const char* sText, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) return XUI_ERROR_OUT_OF_MEMORY;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pLabel, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){0.0f, 26.0f});
	(void)xuiWidgetSetAlign(pLabel, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiTabsAddPageChild(pDemo->pTabs, iPage, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiTabsCreateUi(xui_tabs_demo_t* pDemo)
{
	static const char* arrItems[] = {
		"Start",
		"Scene",
		"Preview",
		"Disabled",
		"Timeline",
		"Export"
	};
	static const int arrEnabled[] = {1, 1, 1, 0, 1, 1};
	static const int arrDirty[] = {0, 1, 0, 0, 1, 0};
	xui_tabs_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiTabsRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.arrEnabled = arrEnabled;
	tDesc.arrDirty = arrDirty;
	tDesc.iItemCount = 6;
	tDesc.iSelected = 1;
	tDesc.bScrollable = 1;
	tDesc.bCloseButtons = 1;
	tDesc.pFont = pDemo->pFont;
	tDesc.fTabWidth = 128.0f;
	tDesc.fTabHeight = 30.0f;
	iRet = xuiTabsCreate(pDemo->pContext, &pDemo->pTabs, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiTabsSetSelect(pDemo->pTabs, __xuiTabsSelected, pDemo);
	(void)xuiTabsSetClose(pDemo->pTabs, __xuiTabsClosed, 1, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pTabs);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiTabsAddLabel(pDemo, 0, "Project overview", XUI_COLOR_RGBA(38, 58, 82, 255)) != XUI_OK ||
	     __xuiTabsAddLabel(pDemo, 1, "Scene graph and properties", XUI_COLOR_RGBA(38, 58, 82, 255)) != XUI_OK ||
	     __xuiTabsAddLabel(pDemo, 2, "Live preview surface", XUI_COLOR_RGBA(38, 58, 82, 255)) != XUI_OK ||
	     __xuiTabsAddLabel(pDemo, 4, "Timeline with dirty marker", XUI_COLOR_RGBA(38, 58, 82, 255)) != XUI_OK ||
	     __xuiTabsAddLabel(pDemo, 5, "Export settings", XUI_COLOR_RGBA(38, 58, 82, 255)) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __xuiTabsLayoutDemo(xui_tabs_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetRect(pDemo->pTabs, (xui_rect_t){34.0f, 34.0f, 692.0f, 336.0f});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiTabsReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiTabsSendButtonTransitions(xui_tabs_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiTabsHandleInput(xui_tabs_demo_t* pDemo)
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
	iButtons = __xuiTabsReadButtons();
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
	iRet = __xuiTabsSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiTabsExerciseClick(xui_tabs_demo_t* pDemo, xui_rect_t tRect)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	tWorld = xuiWidgetGetWorldRect(pDemo->pTabs);
	fX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiTabsRunChecks(xui_tabs_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tTab;
	xui_rect_t tClose;
	xui_rect_t tDirty;
	xui_rect_t tTabBar;
	xui_rect_t tOverflow;
	xui_rect_t tWorld;
	int iBeforeSelect;
	int iBeforeClose;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pTabs != NULL) && (xuiTabsGetItemCount(pDemo->pTabs) == 6);
	pDemo->bLayoutOK = (xuiTabsGetClientRect(pDemo->pTabs).fW > 500.0f) && (xuiTabsGetTabRect(pDemo->pTabs, 1).fW > 80.0f);
	pDemo->bMetaOK = (xuiTabsGetDirtyRect(pDemo->pTabs, 1).fW > 0.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		iBeforeSelect = pDemo->iSelectCount;
		(void)xuiTabsSetSelected(pDemo->pTabs, 3);
		pDemo->bDisabledOK = (xuiTabsGetSelected(pDemo->pTabs) == 1) && (pDemo->iSelectCount == iBeforeSelect);
		(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pTabs);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_RIGHT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bKeyboardOK = (xuiTabsGetSelected(pDemo->pTabs) == 2) && (pDemo->iSelectCount > iBeforeSelect);
		iBeforeClose = pDemo->iCloseCount;
		tClose = xuiTabsGetCloseRect(pDemo->pTabs, 2);
		__xuiTabsExerciseClick(pDemo, tClose);
		pDemo->bCloseOK = (pDemo->iCloseCount > iBeforeClose) && (xuiTabsGetSelected(pDemo->pTabs) == 2);
		tWorld = xuiWidgetGetWorldRect(pDemo->pTabs);
		tTabBar = xuiTabsGetTabBarRect(pDemo->pTabs);
		(void)xuiInputPointerWheel(pDemo->pContext, tWorld.fX + tTabBar.fX + 10.0f, tWorld.fY + tTabBar.fY + 10.0f, 0.0f, -1.0f, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiTabsSetSelected(pDemo->pTabs, 5);
		__xuiTabsLayoutDemo(pDemo);
		tTab = xuiTabsGetTabRect(pDemo->pTabs, 5);
		tDirty = xuiTabsGetDirtyRect(pDemo->pTabs, 4);
		tOverflow = xuiTabsGetOverflowRect(pDemo->pTabs);
		pDemo->bOverflowOK = xuiTabsIsOverflow(pDemo->pTabs) &&
		                      (tOverflow.fW > 0.0f) &&
		                      (xuiTabsGetScroll(pDemo->pTabs) > 0.0f) &&
		                      (tTab.fX >= tTabBar.fX - 1.0f) &&
		                      (tTab.fX + tTab.fW <= tOverflow.fX + 1.0f) &&
		                      (tDirty.fW > 0.0f);
		pDemo->bExerciseDone = 1;
	}
	if ( !bExerciseInput ) {
		pDemo->bDisabledOK = 1;
		pDemo->bKeyboardOK = 1;
		pDemo->bCloseOK = 1;
		pDemo->bOverflowOK = 1;
	}
}

static int __xuiTabsCreateAssets(xui_tabs_demo_t* pDemo)
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
	sFontPath = __xuiTabsFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiTabsCreateUi(pDemo);
}

static void __xuiTabsDestroyAssets(xui_tabs_demo_t* pDemo)
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

static int __xuiTabsFrame(void* pUser)
{
	xui_tabs_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_tabs_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiTabsHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiTabsLayoutDemo(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiTabsRunChecks(pDemo, bAutoRun);
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
		printf("xui_tabs final-summary frames=%d create=%d layout=%d disabled=%d keyboard=%d close=%d overflow=%d meta=%d selected=%d scrollX=%.2f callbacks=%d/%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDisabledOK, pDemo->bKeyboardOK,
			pDemo->bCloseOK, pDemo->bOverflowOK, pDemo->bMetaOK, xuiTabsGetSelected(pDemo->pTabs), xuiTabsGetScroll(pDemo->pTabs),
			pDemo->iSelectCount, pDemo->iCloseCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_tabs_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiTabsParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiTabsUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Tabs";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_tabs: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiTabsCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_tabs: create assets failed: %d\n", iRet);
		__xuiTabsDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiTabsFrame, &tDemo);
	__xuiTabsDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bDisabledOK && tDemo.bKeyboardOK && tDemo.bCloseOK && tDemo.bOverflowOK && tDemo.bMetaOK) ? 0 : 1;
}
