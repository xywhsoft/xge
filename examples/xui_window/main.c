#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	430
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define WINDOW_COUNT	4
#define LABEL_COUNT	16

typedef struct xui_window_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pMain;
	xui_widget pTop;
	xui_widget pCollapsed;
	xui_widget pFrameless;
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
	int iCloseCallbacks;
	int bExerciseDone;
	int bCreateOK;
	int bLayerOK;
	int bDragOK;
	int bResizeOK;
	int bCollapseOK;
	int bMaximizeOK;
	int bCloseOK;
} xui_window_demo_t;

static void __xuiWindowUsage(void)
{
	printf("usage: xui_window [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiWindowParseArgs(xui_window_demo_t* pDemo, int argc, char** argv)
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
			__xuiWindowUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiWindowFindTtf(void)
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

static int __xuiWindowRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_window_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_window_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(245, 249, 253, 255));
	}
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(245, 249, 253, 255));
}

static void __xuiWindowClosed(xui_widget pWidget, void* pUser)
{
	xui_window_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_window_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iCloseCallbacks++;
	}
}

static int __xuiWindowAddLabel(xui_window_demo_t* pDemo, xui_widget pWindow, const char* sText, uint32_t iColor)
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
	(void)xuiWidgetSetPreferredSize(pLabel, (xui_vec2_t){0.0f, 24.0f});
	(void)xuiWidgetSetAlign(pLabel, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiWindowAddChild(pWindow, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiWindowCreateOne(xui_window_demo_t* pDemo, xui_widget* ppWidget, const char* sTitle, xui_rect_t tRect, int bTopMost)
{
	xui_window_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = sTitle;
	tDesc.pFont = pDemo->pFont;
	tDesc.bTopMost = bTopMost;
	iRet = xuiWindowCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	return xuiWidgetSetRect(*ppWidget, tRect);
}

static int __xuiWindowCreateUi(xui_window_demo_t* pDemo)
{
	xui_window_desc_t tDesc;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiWindowRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiWindowCreateOne(pDemo, &pDemo->pMain, "Project Window", (xui_rect_t){72.0f, 56.0f, 320.0f, 210.0f}, 0);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWindowSetClose(pDemo->pMain, __xuiWindowClosed, pDemo);
	(void)__xuiWindowAddLabel(pDemo, pDemo->pMain, "Normal floating window", XUI_COLOR_RGBA(34, 52, 78, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pMain, "Drag the title bar or resize the border.", XUI_COLOR_RGBA(76, 94, 116, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pMain, "Application children live inside the client widget.", XUI_COLOR_RGBA(76, 94, 116, 255));

	iRet = __xuiWindowCreateOne(pDemo, &pDemo->pTop, "Top Most Inspector", (xui_rect_t){420.0f, 62.0f, 250.0f, 155.0f}, 1);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWindowSetColors(pDemo->pTop,
		XUI_COLOR_RGBA(252, 255, 253, 255),
		XUI_COLOR_RGBA(248, 253, 250, 255),
		XUI_COLOR_RGBA(212, 238, 228, 255),
		XUI_COLOR_RGBA(30, 86, 66, 255),
		XUI_COLOR_RGBA(72, 170, 138, 255),
		XUI_COLOR_RGBA(235, 249, 243, 255),
		XUI_COLOR_RGBA(210, 239, 227, 255),
		XUI_COLOR_RGBA(178, 224, 206, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pTop, "Top-most stays above normal windows.", XUI_COLOR_RGBA(34, 86, 66, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pTop, "Styled chrome uses the same cache path.", XUI_COLOR_RGBA(68, 116, 96, 255));

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Collapsed Notes";
	tDesc.pFont = pDemo->pFont;
	tDesc.bCollapsed = 1;
	iRet = xuiWindowCreate(pDemo->pContext, &pDemo->pCollapsed, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pCollapsed, (xui_rect_t){86.0f, 300.0f, 250.0f, 72.0f});
	(void)__xuiWindowAddLabel(pDemo, pDemo->pCollapsed, "Hidden until expanded.", XUI_COLOR_RGBA(76, 94, 116, 255));

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sTitle = "Frameless";
	tDesc.pFont = pDemo->pFont;
	tDesc.bNoTitleBar = 1;
	tDesc.bDragAnywhere = 1;
	tDesc.bHideCollapse = 1;
	tDesc.bHideMaximize = 1;
	tDesc.bHideClose = 1;
	iRet = xuiWindowCreate(pDemo->pContext, &pDemo->pFrameless, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pFrameless, (xui_rect_t){450.0f, 276.0f, 170.0f, 94.0f});
	(void)xuiWindowSetColors(pDemo->pFrameless,
		XUI_COLOR_RGBA(255, 252, 246, 255),
		XUI_COLOR_RGBA(255, 252, 246, 255),
		XUI_COLOR_RGBA(255, 252, 246, 255),
		XUI_COLOR_RGBA(92, 70, 38, 255),
		XUI_COLOR_RGBA(196, 166, 116, 255),
		XUI_COLOR_RGBA(255, 250, 238, 255),
		XUI_COLOR_RGBA(246, 231, 205, 255),
		XUI_COLOR_RGBA(232, 210, 172, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pFrameless, "Frameless", XUI_COLOR_RGBA(92, 70, 38, 255));
	(void)__xuiWindowAddLabel(pDemo, pDemo->pFrameless, "Drag anywhere", XUI_COLOR_RGBA(120, 92, 52, 255));
	return XUI_OK;
}

static uint32_t __xuiWindowReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiWindowSendButtonTransitions(xui_window_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiWindowHandleInput(xui_window_demo_t* pDemo)
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
	iButtons = __xuiWindowReadButtons();
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
	iRet = __xuiWindowSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiWindowDispatchMove(xui_window_demo_t* pDemo, float fX, float fY, uint32_t iButtons)
{
	int iRet;

	iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiWindowDispatchClick(xui_window_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = __xuiWindowDispatchMove(pDemo, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiWindowDispatchDrag(xui_window_demo_t* pDemo, float fStartX, float fStartY, float fEndX, float fEndY)
{
	int iRet;

	iRet = __xuiWindowDispatchMove(pDemo, fStartX, fStartY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiWindowDispatchMove(pDemo, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiWindowExercise(xui_window_demo_t* pDemo)
{
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tButton;
	float fOldX;
	float fOldY;
	float fOldW;

	if ( pDemo->bExerciseDone || !pDemo->bCreateOK || !pDemo->bLayerOK ) return;
	tRect = xuiWidgetGetRect(pDemo->pMain);
	fOldX = tRect.fX;
	fOldY = tRect.fY;
	fOldW = tRect.fW;
	tWorld = xuiWidgetGetWorldRect(pDemo->pMain);
	(void)__xuiWindowDispatchDrag(pDemo, tWorld.fX + 80.0f, tWorld.fY + 15.0f, tWorld.fX + 108.0f, tWorld.fY + 33.0f);
	tRect = xuiWidgetGetRect(pDemo->pMain);
	pDemo->bDragOK = (tRect.fX > fOldX + 20.0f) && (tRect.fY > fOldY + 10.0f) && xuiWindowIsActive(pDemo->pMain);

	tWorld = xuiWidgetGetWorldRect(pDemo->pMain);
	(void)__xuiWindowDispatchDrag(pDemo, tWorld.fX + tWorld.fW - 2.0f, tWorld.fY + 90.0f, tWorld.fX + tWorld.fW + 46.0f, tWorld.fY + 90.0f);
	tRect = xuiWidgetGetRect(pDemo->pMain);
	pDemo->bResizeOK = tRect.fW > fOldW + 36.0f;
	(void)xuiLayout(pDemo->pContext);

	(void)xuiWindowSetCollapsed(pDemo->pMain, 1);
	(void)xuiLayout(pDemo->pContext);
	pDemo->bCollapseOK = xuiWindowIsCollapsed(pDemo->pMain) && !xuiWidgetGetVisible(xuiWindowGetClientWidget(pDemo->pMain));
	(void)xuiWindowSetCollapsed(pDemo->pMain, 0);
	(void)xuiLayout(pDemo->pContext);
	pDemo->bCollapseOK = pDemo->bCollapseOK && !xuiWindowIsCollapsed(pDemo->pMain);

	if ( xuiWindowSetMaximized(pDemo->pMain, 1) == XUI_OK ) {
		tRect = xuiWidgetGetRect(pDemo->pMain);
		pDemo->bMaximizeOK = xuiWindowIsMaximized(pDemo->pMain) && (tRect.fW >= (float)DEMO_TARGET_W - 1.0f) && (tRect.fH >= (float)DEMO_TARGET_H - 1.0f);
	}
	(void)xuiWindowSetMaximized(pDemo->pMain, 0);
	(void)xuiLayout(pDemo->pContext);

	tWorld = xuiWidgetGetWorldRect(pDemo->pMain);
	tButton = xuiWindowGetCloseButtonRect(pDemo->pMain);
	(void)__xuiWindowDispatchClick(pDemo, tWorld.fX + tButton.fX + tButton.fW * 0.5f, tWorld.fY + tButton.fY + tButton.fH * 0.5f);
	pDemo->bCloseOK = !xuiWindowIsOpen(pDemo->pMain) && (pDemo->iCloseCallbacks == 1) && (xuiWindowGetCloseCount(pDemo->pMain) == 1);
	(void)xuiWindowSetOpen(pDemo->pMain, 1);
	pDemo->bExerciseDone = 1;
}

static void __xuiWindowRunChecks(xui_window_demo_t* pDemo, int bExerciseInput)
{
	int iLayerMain;
	int iZMain;
	int iLayerTop;
	int iZTop;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) &&
		(pDemo->pMain != NULL) &&
		(pDemo->pTop != NULL) &&
		(pDemo->pCollapsed != NULL) &&
		(pDemo->pFrameless != NULL) &&
		(xuiWindowGetClientWidget(pDemo->pMain) != NULL) &&
		(xuiWidgetGetParent(pDemo->pMain) == xuiOverlayRoot(pDemo->pContext));
	iLayerMain = 0;
	iZMain = 0;
	iLayerTop = 0;
	iZTop = 0;
	if ( pDemo->bCreateOK &&
	     (xuiWidgetGetLayer(pDemo->pMain, &iLayerMain, &iZMain) == XUI_OK) &&
	     (xuiWidgetGetLayer(pDemo->pTop, &iLayerTop, &iZTop) == XUI_OK) ) {
		pDemo->bLayerOK = (iLayerMain == XUI_LAYER_FLOATING) &&
			(iLayerTop == XUI_LAYER_FLOATING) &&
			(iZMain == XUI_WINDOW_Z_NORMAL) &&
			(iZTop == XUI_WINDOW_Z_TOPMOST) &&
			xuiWindowIsTopMost(pDemo->pTop);
	}
	if ( bExerciseInput ) {
		__xuiWindowExercise(pDemo);
	} else {
		pDemo->bDragOK = 1;
		pDemo->bResizeOK = 1;
		pDemo->bCollapseOK = 1;
		pDemo->bMaximizeOK = 1;
		pDemo->bCloseOK = 1;
	}
}

static int __xuiWindowCreateAssets(xui_window_demo_t* pDemo)
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
	sFontPath = __xuiWindowFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiWindowCreateUi(pDemo);
}

static void __xuiWindowDestroyAssets(xui_window_demo_t* pDemo)
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

static int __xuiWindowFrame(void* pUser)
{
	xui_window_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_window_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiWindowHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiWindowRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(230, 239, 248, 255));
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
		printf("xui_window final-summary frames=%d create=%d layer=%d drag=%d resize=%d collapse=%d maximize=%d close=%d callbacks=%d active=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayerOK, pDemo->bDragOK, pDemo->bResizeOK,
			pDemo->bCollapseOK, pDemo->bMaximizeOK, pDemo->bCloseOK, pDemo->iCloseCallbacks,
			xuiWindowGetActive(pDemo->pContext) != NULL, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_window_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiWindowParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiWindowUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Window";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_window: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiWindowCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_window: create assets failed: %d\n", iRet);
		__xuiWindowDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiWindowFrame, &tDemo);
	__xuiWindowDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayerOK && tDemo.bDragOK &&
		tDemo.bResizeOK && tDemo.bCollapseOK && tDemo.bMaximizeOK && tDemo.bCloseOK) ? 0 : 1;
}
