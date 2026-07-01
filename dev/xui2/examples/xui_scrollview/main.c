#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	700
#define DEMO_TARGET_H	380
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define VIEW_COUNT	2
#define ITEM_COUNT	12
#define LABEL_COUNT	8

typedef struct xui_scrollview_demo_t xui_scrollview_demo_t;

typedef struct xui_scrollview_item_t {
	xui_scrollview_demo_t* pDemo;
	uint32_t iColor;
	uint32_t iBorderColor;
} xui_scrollview_item_t;

struct xui_scrollview_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pView[VIEW_COUNT];
	xui_widget pItem[ITEM_COUNT];
	xui_widget pLabel[LABEL_COUNT];
	xui_scrollview_item_t tItem[ITEM_COUNT];
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
	int bModeOK;
	int bModelOK;
	int bEnsureOK;
	int bInputOK;
};

static void __xuiScrollViewUsage(void)
{
	printf("usage: xui_scrollview [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiScrollViewParseArgs(xui_scrollview_demo_t* pDemo, int argc, char** argv)
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
			__xuiScrollViewUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiScrollViewFindTtf(void)
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

static int __xuiScrollViewRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_scrollview_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_scrollview_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(244, 248, 253, 255));
}

static int __xuiScrollViewItemRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_scrollview_item_t* pItem;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pItem = (xui_scrollview_item_t*)pUser;
	if ( pItem == NULL || pItem->pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = pItem->pDemo->tProxy.drawRectFill(&pItem->pDemo->tProxy, pDraw, tRect, pItem->iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pItem->pDemo->tProxy.drawRectStroke(&pItem->pDemo->tProxy, pDraw, tRect, 1.0f, pItem->iBorderColor);
}

static void __xuiScrollViewChanged(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	xui_scrollview_demo_t* pDemo;

	(void)pWidget;
	(void)fOffsetX;
	(void)fOffsetY;
	pDemo = (xui_scrollview_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
	}
}

static int __xuiScrollViewAddLabel(xui_scrollview_demo_t* pDemo, xui_widget pParent, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(50, 64, 84, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pParent, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	xuiWidgetSetRect(pLabel, tRect);
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiScrollViewAddItem(xui_scrollview_demo_t* pDemo, xui_widget pParent, int iIndex, xui_rect_t tRect, uint32_t iColor)
{
	xui_widget pItem;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= ITEM_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pDemo->pContext, &pItem);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->tItem[iIndex].pDemo = pDemo;
	pDemo->tItem[iIndex].iColor = iColor;
	pDemo->tItem[iIndex].iBorderColor = XUI_COLOR_RGBA(136, 166, 200, 180);
	iRet = xuiWidgetSetCacheRenderCallback(pItem, __xuiScrollViewItemRender, &pDemo->tItem[iIndex]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pParent, pItem);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pItem);
		return iRet;
	}
	xuiWidgetSetRect(pItem, tRect);
	pDemo->pItem[iIndex] = pItem;
	return XUI_OK;
}

static int __xuiScrollViewCreateView(xui_scrollview_demo_t* pDemo, int iIndex, xui_rect_t tRect, int iMode, float fContentW, float fContentH)
{
	xui_scroll_view_desc_t tDesc;
	xui_widget pView;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fContentWidth = fContentW;
	tDesc.fContentHeight = fContentH;
	tDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iScrollbarMode = iMode;
	tDesc.iWheelAxis = XUI_WHEEL_AXIS_VERTICAL;
	tDesc.iCornerMode = (iMode == XUI_SCROLLBAR_MODE_FULL) ? XUI_SCROLL_FRAME_CORNER_GRIP : XUI_SCROLL_FRAME_CORNER_AUTO;
	tDesc.bContentDragEnabled = 1;
	tDesc.fScrollbarSize = (iMode == XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 8.0f;
	tDesc.fWheelStep = 36.0f;
	tDesc.iBackgroundColor = XUI_COLOR_RGBA(251, 253, 255, 255);
	tDesc.iCornerColor = XUI_COLOR_RGBA(228, 237, 248, 255);
	iRet = xuiScrollViewCreate(pDemo->pContext, &pView, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pView);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pView);
		return iRet;
	}
	xuiWidgetSetRect(pView, tRect);
	iRet = xuiScrollViewSetChange(pView, __xuiScrollViewChanged, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->pView[iIndex] = pView;
	return XUI_OK;
}

static int __xuiScrollViewCreateUi(xui_scrollview_demo_t* pDemo)
{
	xui_widget pContent;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiScrollViewRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiScrollViewAddLabel(pDemo, pDemo->pRoot, 0, "Compact scrollbars", (xui_rect_t){28.0f, 18.0f, 180.0f, 24.0f});
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddLabel(pDemo, pDemo->pRoot, 1, "Compact drag view", (xui_rect_t){360.0f, 18.0f, 180.0f, 24.0f});
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiScrollViewCreateView(pDemo, 0, (xui_rect_t){28.0f, 50.0f, 310.0f, 280.0f}, XUI_SCROLLBAR_MODE_COMPACT, 540.0f, 460.0f);
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewCreateView(pDemo, 1, (xui_rect_t){360.0f, 50.0f, 300.0f, 280.0f}, XUI_SCROLLBAR_MODE_COMPACT, 500.0f, 430.0f);
	if ( iRet != XUI_OK ) return iRet;

	pContent = xuiScrollViewGetContentWidget(pDemo->pView[0]);
	iRet = __xuiScrollViewAddItem(pDemo, pContent, 0, (xui_rect_t){24.0f, 28.0f, 180.0f, 90.0f}, XUI_COLOR_RGBA(221, 239, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 1, (xui_rect_t){230.0f, 28.0f, 250.0f, 70.0f}, XUI_COLOR_RGBA(230, 247, 241, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 2, (xui_rect_t){64.0f, 148.0f, 220.0f, 92.0f}, XUI_COLOR_RGBA(243, 238, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 3, (xui_rect_t){318.0f, 145.0f, 180.0f, 130.0f}, XUI_COLOR_RGBA(255, 244, 222, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 4, (xui_rect_t){34.0f, 302.0f, 250.0f, 92.0f}, XUI_COLOR_RGBA(232, 242, 250, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 5, (xui_rect_t){320.0f, 330.0f, 180.0f, 80.0f}, XUI_COLOR_RGBA(224, 248, 255, 255));
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiScrollViewAddLabel(pDemo, pContent, 2, "content 540 x 460", (xui_rect_t){28.0f, 420.0f, 220.0f, 24.0f});

	pContent = xuiScrollViewGetContentWidget(pDemo->pView[1]);
	iRet = __xuiScrollViewAddItem(pDemo, pContent, 6, (xui_rect_t){18.0f, 24.0f, 180.0f, 78.0f}, XUI_COLOR_RGBA(236, 246, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 7, (xui_rect_t){228.0f, 24.0f, 220.0f, 96.0f}, XUI_COLOR_RGBA(225, 244, 236, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 8, (xui_rect_t){52.0f, 152.0f, 210.0f, 110.0f}, XUI_COLOR_RGBA(245, 238, 255, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 9, (xui_rect_t){300.0f, 170.0f, 160.0f, 90.0f}, XUI_COLOR_RGBA(255, 246, 225, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 10, (xui_rect_t){34.0f, 305.0f, 230.0f, 82.0f}, XUI_COLOR_RGBA(226, 240, 252, 255));
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewAddItem(pDemo, pContent, 11, (xui_rect_t){310.0f, 318.0f, 150.0f, 70.0f}, XUI_COLOR_RGBA(228, 248, 255, 255));
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiScrollViewAddLabel(pDemo, pContent, 3, "content 500 x 430", (xui_rect_t){26.0f, 394.0f, 220.0f, 24.0f});
	return XUI_OK;
}

static void __xuiScrollViewLayout(xui_scrollview_demo_t* pDemo)
{
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiScrollViewReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiScrollViewSendButtonTransitions(xui_scrollview_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiScrollViewHandleInput(xui_scrollview_demo_t* pDemo)
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
	iButtons = __xuiScrollViewReadButtons();
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
	iRet = __xuiScrollViewSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiScrollViewRunChecks(xui_scrollview_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tViewport0;
	xui_rect_t tViewport1;
	xui_rect_t tWorld;
	xui_vec2_t tPoint;
	xui_widget pFrame0;
	xui_widget pFrame1;
	xui_widget pHBar0;
	xui_widget pVBar0;
	xui_widget pHBar1;
	xui_widget pVBar1;
	float fOffsetX;
	float fOffsetY;
	float fHSize0;
	float fVSize0;
	float fHSize1;
	float fVSize1;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pView[0] != NULL) && (pDemo->pView[1] != NULL);
	tViewport0 = xuiScrollViewGetViewportRect(pDemo->pView[0]);
	tViewport1 = xuiScrollViewGetViewportRect(pDemo->pView[1]);
	pFrame0 = xuiScrollViewGetFrameWidget(pDemo->pView[0]);
	pFrame1 = xuiScrollViewGetFrameWidget(pDemo->pView[1]);
	pHBar0 = xuiScrollFrameGetHScrollBarWidget(pFrame0);
	pVBar0 = xuiScrollFrameGetVScrollBarWidget(pFrame0);
	pHBar1 = xuiScrollFrameGetHScrollBarWidget(pFrame1);
	pVBar1 = xuiScrollFrameGetVScrollBarWidget(pFrame1);
	fHSize0 = fVSize0 = fHSize1 = fVSize1 = 0.0f;
	if ( pHBar0 != NULL ) (void)xuiScrollBarGetMetrics(pHBar0, &fHSize0, NULL, NULL, NULL);
	if ( pVBar0 != NULL ) (void)xuiScrollBarGetMetrics(pVBar0, &fVSize0, NULL, NULL, NULL);
	if ( pHBar1 != NULL ) (void)xuiScrollBarGetMetrics(pHBar1, &fHSize1, NULL, NULL, NULL);
	if ( pVBar1 != NULL ) (void)xuiScrollBarGetMetrics(pVBar1, &fVSize1, NULL, NULL, NULL);
	pDemo->bLayoutOK = (tViewport0.fW > 0.0f) && (tViewport0.fW < xuiWidgetGetRect(pDemo->pView[0]).fW) &&
	                   (tViewport1.fH > 0.0f) && xuiScrollFrameIsHScrollBarVisible(xuiScrollViewGetFrameWidget(pDemo->pView[0])) &&
	                   xuiScrollFrameIsVScrollBarVisible(xuiScrollViewGetFrameWidget(pDemo->pView[1]));
	pDemo->bModeOK = (xuiScrollViewGetScrollbarMode(pDemo->pView[0]) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollViewGetScrollbarMode(pDemo->pView[1]) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollFrameGetScrollbarMode(pFrame0) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollFrameGetScrollbarMode(pFrame1) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (pHBar0 != NULL) && (pVBar0 != NULL) && (pHBar1 != NULL) && (pVBar1 != NULL) &&
	                 (xuiScrollBarGetMode(pHBar0) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollBarGetMode(pVBar0) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollBarGetMode(pHBar1) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (xuiScrollBarGetMode(pVBar1) == XUI_SCROLLBAR_MODE_COMPACT) &&
	                 (fHSize0 > 0.0f) && (fHSize0 <= 8.0f) &&
	                 (fVSize0 > 0.0f) && (fVSize0 <= 8.0f) &&
	                 (fHSize1 > 0.0f) && (fHSize1 <= 8.0f) &&
	                 (fVSize1 > 0.0f) && (fVSize1 <= 8.0f);
	tWorld = xuiWidgetGetWorldRect(xuiScrollViewGetViewportWidget(pDemo->pView[0]));
	tPoint = xuiScrollModelScreenToContent(xuiScrollViewGetModel(pDemo->pView[0]), tWorld.fX + 6.0f, tWorld.fY + 6.0f);
	pDemo->bModelOK = (tPoint.fX >= 0.0f) && (tPoint.fY >= 0.0f);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		(void)xuiInputPointerWheel(pDemo->pContext, tWorld.fX + 18.0f, tWorld.fY + 18.0f, 0.0f, -1.0f, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiScrollViewEnsureChildVisible(pDemo->pView[1], pDemo->pItem[11]);
		pDemo->bExerciseDone = 1;
	}
	(void)xuiScrollViewGetOffset(pDemo->pView[1], &fOffsetX, &fOffsetY);
	pDemo->bEnsureOK = !bExerciseInput || (fOffsetX > 0.0f && fOffsetY > 0.0f);
	pDemo->bInputOK = !bExerciseInput || (pDemo->iChangeCount > 0);
}

static int __xuiScrollViewCreateAssets(xui_scrollview_demo_t* pDemo)
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
	sFontPath = __xuiScrollViewFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiScrollViewCreateUi(pDemo);
}

static void __xuiScrollViewDestroyAssets(xui_scrollview_demo_t* pDemo)
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

static int __xuiScrollViewFrame(void* pUser)
{
	xui_scrollview_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_scrollview_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiScrollViewHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiScrollViewLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiScrollViewRunChecks(pDemo, bAutoRun);
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
		printf("xui_scrollview final-summary frames=%d create=%d layout=%d mode=%d model=%d ensure=%d input=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bModeOK, pDemo->bModelOK, pDemo->bEnsureOK, pDemo->bInputOK,
			pDemo->iChangeCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_scrollview_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiScrollViewParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiScrollViewUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI ScrollView";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_scrollview: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiScrollViewCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_scrollview: create assets failed: %d\n", iRet);
		__xuiScrollViewDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiScrollViewFrame, &tDemo);
	__xuiScrollViewDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bModeOK && tDemo.bModelOK && tDemo.bEnsureOK && tDemo.bInputOK) ? 0 : 1;
}
