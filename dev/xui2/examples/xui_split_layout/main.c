#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	500
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define SPLIT_COUNT	2
#define LABEL_COUNT	14
#define PANE_RENDER_COUNT	8

typedef struct xui_split_pane_render_t {
	xui_proxy_t* pProxy;
	uint32_t iColor;
	uint32_t iBorderColor;
} xui_split_pane_render_t;

typedef struct xui_split_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pSplit[SPLIT_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	xui_split_pane_render_t arrPaneRender[PANE_RENDER_COUNT];
	int iLabelCount;
	int iPaneRenderCount;
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
	int bDividerOK;
	int bShadowOK;
	int bLiveOK;
} xui_split_demo_t;

static void __xuiSplitUsage(void)
{
	printf("usage: xui_split_layout [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiSplitParseArgs(xui_split_demo_t* pDemo, int argc, char** argv)
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
			__xuiSplitUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiSplitFindTtf(void)
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

static int __xuiSplitRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_split_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_split_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
}

static int __xuiSplitPaneRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_split_pane_render_t* pRender;
	xui_proxy pProxy;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pRender = (xui_split_pane_render_t*)pUser;
	if ( (pWidget == NULL) || (pRender == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = pRender->pProxy;
	if ( (pProxy == NULL) || (pProxy->drawRoundRectFill == NULL) || (pProxy->drawRectStroke == NULL) ) {
		return XUI_ERROR_NOT_INITIALIZED;
	}
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = pProxy->drawRoundRectFill(pProxy, pDraw, tRect, 6.0f, pRender->iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawRectStroke(pProxy, pDraw, tRect, 1.0f, pRender->iBorderColor);
}

static void __xuiSplitChanged(xui_widget pWidget, int iDivider, void* pUser)
{
	xui_split_demo_t* pDemo;

	(void)pWidget;
	(void)iDivider;
	pDemo = (xui_split_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiSplitAddLabel(xui_split_demo_t* pDemo, xui_widget pParent, const char* sText, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( pDemo->iLabelCount >= LABEL_COUNT ) return XGE_ERROR_OUT_OF_MEMORY;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pLabel, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	(void)xuiWidgetSetAlign(pLabel, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiSplitApplyPaneStyle(xui_split_demo_t* pDemo, xui_widget pPane, uint32_t iFill, uint32_t iBorder)
{
	xui_split_pane_render_t* pRender;

	if ( pDemo->iPaneRenderCount >= PANE_RENDER_COUNT ) return XGE_ERROR_OUT_OF_MEMORY;
	pRender = &pDemo->arrPaneRender[pDemo->iPaneRenderCount++];
	pRender->pProxy = &pDemo->tProxy;
	pRender->iColor = iFill;
	pRender->iBorderColor = iBorder;
	(void)xuiWidgetSetPadding(pPane, (xui_thickness_t){12.0f, 10.0f, 12.0f, 10.0f});
	(void)xuiWidgetSetGap(pPane, 8.0f);
	return xuiWidgetSetCacheRenderCallback(pPane, __xuiSplitPaneRender, pRender);
}

static int __xuiSplitCreateVertical(xui_split_demo_t* pDemo)
{
	xui_split_layout_desc_t tDesc;
	xui_widget pSplit;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 3;
	tDesc.bShadowDrag = 1;
	tDesc.fDividerSize = 10.0f;
	tDesc.fDividerVisualSize = 3.0f;
	tDesc.fDividerHitSize = 14.0f;
	iRet = xuiSplitLayoutCreate(pDemo->pContext, &pSplit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pSplit);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pSplit);
		return iRet;
	}
	pDemo->pSplit[0] = pSplit;
	(void)xuiSplitLayoutSetChange(pSplit, __xuiSplitChanged, pDemo);
	(void)xuiSplitLayoutSetPaneMode(pSplit, 0, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 130.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pSplit, 1, 120.0f);
	(void)xuiSplitLayoutSetPaneWeight(pSplit, 1, 1.6f);
	(void)xuiSplitLayoutSetPaneMode(pSplit, 2, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 124.0f);
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 0), XUI_COLOR_RGBA(247, 250, 255, 255), XUI_COLOR_RGBA(202, 215, 232, 255));
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 1), XUI_COLOR_RGBA(242, 248, 255, 255), XUI_COLOR_RGBA(183, 204, 228, 255));
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 2), XUI_COLOR_RGBA(248, 251, 247, 255), XUI_COLOR_RGBA(200, 218, 197, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 0), "Project", XUI_COLOR_RGBA(48, 65, 88, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 1), "Editor surface", XUI_COLOR_RGBA(35, 64, 98, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 2), "Inspector", XUI_COLOR_RGBA(48, 72, 56, 255));
	return XUI_OK;
}

static int __xuiSplitCreateHorizontal(xui_split_demo_t* pDemo)
{
	xui_split_layout_desc_t tDesc;
	xui_widget pSplit;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tDesc.iPaneCount = 3;
	tDesc.bShadowDrag = 0;
	tDesc.fDividerSize = 9.0f;
	tDesc.fDividerVisualSize = 3.0f;
	tDesc.fDividerHitSize = 13.0f;
	tDesc.iDividerColor = XUI_COLOR_RGBA(205, 166, 93, 220);
	tDesc.iDividerHoverColor = XUI_COLOR_RGBA(214, 128, 62, 235);
	tDesc.iDividerActiveColor = XUI_COLOR_RGBA(184, 86, 42, 245);
	tDesc.iShadowColor = XUI_COLOR_RGBA(184, 86, 42, 90);
	iRet = xuiSplitLayoutCreate(pDemo->pContext, &pSplit, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pSplit);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pSplit);
		return iRet;
	}
	pDemo->pSplit[1] = pSplit;
	(void)xuiSplitLayoutSetChange(pSplit, __xuiSplitChanged, pDemo);
	(void)xuiSplitLayoutSetPaneMode(pSplit, 0, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 64.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pSplit, 0, 44.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pSplit, 1, 70.0f);
	(void)xuiSplitLayoutSetPaneMode(pSplit, 2, XUI_SPLIT_PANE_FIXED);
	(void)xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 76.0f);
	(void)xuiSplitLayoutSetPaneMinSize(pSplit, 2, 50.0f);
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 0), XUI_COLOR_RGBA(255, 250, 242, 255), XUI_COLOR_RGBA(226, 206, 176, 255));
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 1), XUI_COLOR_RGBA(250, 252, 255, 255), XUI_COLOR_RGBA(204, 213, 226, 255));
	(void)__xuiSplitApplyPaneStyle(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 2), XUI_COLOR_RGBA(248, 248, 252, 255), XUI_COLOR_RGBA(207, 207, 224, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 0), "Output", XUI_COLOR_RGBA(96, 68, 42, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 1), "Timeline", XUI_COLOR_RGBA(55, 67, 86, 255));
	(void)__xuiSplitAddLabel(pDemo, xuiSplitLayoutGetPaneWidget(pSplit, 2), "Log", XUI_COLOR_RGBA(66, 60, 88, 255));
	return XUI_OK;
}

static int __xuiSplitCreateUi(xui_split_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiSplitRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	if ( __xuiSplitCreateVertical(pDemo) != XUI_OK ||
	     __xuiSplitCreateHorizontal(pDemo) != XUI_OK ) {
		return XUI_ERROR;
	}
	return XUI_OK;
}

static void __xuiSplitLayoutDemo(xui_split_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetRect(pDemo->pSplit[0], (xui_rect_t){28.0f, 28.0f, 704.0f, 206.0f});
	(void)xuiWidgetSetRect(pDemo->pSplit[1], (xui_rect_t){28.0f, 264.0f, 704.0f, 190.0f});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiSplitReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiSplitSendButtonTransitions(xui_split_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiSplitHandleInput(xui_split_demo_t* pDemo)
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
	iButtons = __xuiSplitReadButtons();
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
	iRet = __xuiSplitSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiSplitExerciseDrag(xui_split_demo_t* pDemo, xui_widget pSplit, int iDivider, float fDx, float fDy)
{
	xui_rect_t tHit;
	xui_rect_t tWorld;
	float fX;
	float fY;

	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, iDivider);
	tWorld = xuiWidgetGetWorldRect(pSplit);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerMove(pDemo->pContext, fX + fDx, fY + fDy, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUp(pDemo->pContext, fX + fDx, fY + fDy, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiSplitRunChecks(xui_split_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tHit;
	xui_rect_t tVisual;
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pSplit[0] != NULL) && (pDemo->pSplit[1] != NULL);
	pDemo->bLayoutOK = (xuiSplitLayoutGetPaneRect(pDemo->pSplit[0], 1).fW > 300.0f) &&
	                   (xuiSplitLayoutGetPaneRect(pDemo->pSplit[1], 1).fH > 40.0f);
	tHit = xuiSplitLayoutGetDividerHitRect(pDemo->pSplit[0], 0);
	tVisual = xuiSplitLayoutGetDividerVisualRect(pDemo->pSplit[0], 0);
	pDemo->bDividerOK = (tHit.fW > tVisual.fW) && (tVisual.fW >= 3.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		iBefore = pDemo->iChangeCount;
		__xuiSplitExerciseDrag(pDemo, pDemo->pSplit[0], 0, 24.0f, 0.0f);
		pDemo->bShadowOK = (pDemo->iChangeCount > iBefore) && (xuiSplitLayoutGetPaneFixedSize(pDemo->pSplit[0], 0) > 130.0f);
		__xuiSplitLayoutDemo(pDemo);
		iBefore = pDemo->iChangeCount;
		__xuiSplitExerciseDrag(pDemo, pDemo->pSplit[1], 0, 0.0f, 18.0f);
		pDemo->bLiveOK = (pDemo->iChangeCount > iBefore) && (xuiSplitLayoutGetChangeCount(pDemo->pSplit[1]) > 0);
		pDemo->bExerciseDone = 1;
	}
	if ( !bExerciseInput ) {
		pDemo->bShadowOK = 1;
		pDemo->bLiveOK = 1;
	}
}

static int __xuiSplitCreateAssets(xui_split_demo_t* pDemo)
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
	sFontPath = __xuiSplitFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiSplitCreateUi(pDemo);
}

static void __xuiSplitDestroyAssets(xui_split_demo_t* pDemo)
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

static int __xuiSplitFrame(void* pUser)
{
	xui_split_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_split_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiSplitHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiSplitLayoutDemo(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiSplitRunChecks(pDemo, bAutoRun);
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
		printf("xui_split_layout final-summary frames=%d create=%d layout=%d divider=%d shadow=%d live=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bDividerOK, pDemo->bShadowOK, pDemo->bLiveOK,
			pDemo->iChangeCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_split_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiSplitParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiSplitUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI SplitLayout";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_split_layout: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiSplitCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_split_layout: create assets failed: %d\n", iRet);
		__xuiSplitDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiSplitFrame, &tDemo);
	__xuiSplitDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bDividerOK && tDemo.bShadowOK && tDemo.bLiveOK) ? 0 : 1;
}
