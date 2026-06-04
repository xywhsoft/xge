#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 820
#define DEMO_TARGET_H 390
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f
#define PAGE_COUNT 4
#define LABEL_COUNT 9

typedef struct xui_page_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pPage[PAGE_COUNT];
	xui_widget pLabel[LABEL_COUNT];
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
	int iChangeCount;
	int iLastOldPage;
	int iLastNewPage;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStyleOK;
	int bInputOK;
	int bKeyboardOK;
} xui_page_demo_t;

static void __xuiPageDemoUsage(void)
{
	printf("usage: xui_page [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiPageDemoParseArgs(xui_page_demo_t* pDemo, int argc, char** argv)
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
			__xuiPageDemoUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiPageDemoFindTtf(void)
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

static int __xuiPageDemoRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_page_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_page_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, tRect.fW - 56.0f, tRect.fH - 44.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiPageDemoChanged(xui_widget pWidget, int iOldPage, int iNewPage, void* pUser)
{
	xui_page_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_page_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iChangeCount++;
		pDemo->iLastOldPage = iOldPage;
		pDemo->iLastNewPage = iNewPage;
	}
}

static int __xuiPageDemoAddLabel(xui_page_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(55, 67, 86, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	if ( ppLabel != NULL ) *ppLabel = pLabel;
	return XUI_OK;
}

static int __xuiPageDemoAddPage(xui_page_demo_t* pDemo, int iIndex, xui_rect_t tRect, int iPageCount, int iCurrentPage, int iWindowSize)
{
	xui_page_desc_t tDesc;
	xui_widget pPage;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= PAGE_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.iPageCount = iPageCount;
	tDesc.iCurrentPage = iCurrentPage;
	tDesc.iWindowSize = iWindowSize;
	iRet = xuiPageCreate(pDemo->pContext, &pPage, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pPage, tRect);
	iRet = xuiPageSetChange(pPage, __xuiPageDemoChanged, pDemo);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pDemo->pRoot, pPage);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pPage);
		return iRet;
	}
	pDemo->pPage[iIndex] = pPage;
	return XUI_OK;
}

static int __xuiPageDemoCreateUi(xui_page_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiPageDemoRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiPageDemoAddLabel(pDemo, 0, "XUI Page", (xui_rect_t){48.0f, 38.0f, 180.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddLabel(pDemo, 1, "Default window", (xui_rect_t){58.0f, 86.0f, 136.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddPage(pDemo, 0, (xui_rect_t){190.0f, 82.0f, 600.0f, 30.0f}, 18, 9, 5);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddLabel(pDemo, 2, "Compact text", (xui_rect_t){58.0f, 146.0f, 136.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddPage(pDemo, 1, (xui_rect_t){190.0f, 142.0f, 430.0f, 26.0f}, 7, 1, 3);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddLabel(pDemo, 3, "Total + colors", (xui_rect_t){58.0f, 206.0f, 136.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddPage(pDemo, 2, (xui_rect_t){190.0f, 202.0f, 600.0f, 30.0f}, 10, 4, 7);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddLabel(pDemo, 4, "Disabled", (xui_rect_t){58.0f, 266.0f, 136.0f, 24.0f}, NULL);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddPage(pDemo, 3, (xui_rect_t){190.0f, 262.0f, 500.0f, 28.0f}, 12, 6, 5);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoAddLabel(pDemo, 5, "Click buttons or focus a page bar and use Left / Right / Home / End.", (xui_rect_t){58.0f, 330.0f, 650.0f, 22.0f}, &pDemo->pStatus);
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiPageSetText(pDemo->pPage[1], "First", "Last", "Prev", "Next");
	(void)xuiPageSetMetrics(pDemo->pPage[1], 24.0f, 32.0f, 50.0f, 42.0f, 26.0f);
	(void)xuiPageSetTotal(pDemo->pPage[2], 186, 10);
	(void)xuiPageSetColors(pDemo->pPage[2],
		XUI_COLOR_RGBA(250, 255, 252, 255),
		XUI_COLOR_RGBA(146, 210, 178, 255),
		XUI_COLOR_RGBA(35, 78, 61, 255),
		XUI_COLOR_RGBA(229, 247, 238, 255),
		XUI_COLOR_RGBA(205, 237, 220, 255),
		XUI_COLOR_RGBA(43, 158, 106, 255),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(145, 166, 154, 255));
	(void)xuiPageSetFocusColor(pDemo->pPage[2], XUI_COLOR_RGBA(43, 158, 106, 180));
	(void)xuiWidgetSetEnabled(pDemo->pPage[3], 0);
	return XUI_OK;
}

static uint32_t __xuiPageDemoReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiPageDemoSendButtonTransitions(xui_page_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiPageDemoSendKey(xui_page_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiPageDemoHandleInput(xui_page_demo_t* pDemo)
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
	iButtons = __xuiPageDemoReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiPageDemoSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_LEFT, XUI_KEY_LEFT);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_RIGHT, XUI_KEY_RIGHT);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_UP, XUI_KEY_UP);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_DOWN, XUI_KEY_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_HOME, XUI_KEY_HOME);
	if ( iRet == XUI_OK ) iRet = __xuiPageDemoSendKey(pDemo, XGE_KEY_END, XUI_KEY_END);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiPageDemoClickItem(xui_page_demo_t* pDemo, xui_widget pPage, int iType)
{
	xui_page_item_info_t tInfo;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int i;
	int iCount;
	int iRet;

	iCount = xuiPageGetItemCount(pPage);
	tWorld = xuiWidgetGetWorldRect(pPage);
	for ( i = 0; i < iCount; i++ ) {
		memset(&tInfo, 0, sizeof(tInfo));
		tInfo.iSize = sizeof(tInfo);
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		if ( iRet != XUI_OK ) return iRet;
		if ( (tInfo.iType == iType) && tInfo.bEnabled ) {
			fX = tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f;
			fY = tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f;
			iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
			if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
			if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
			if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
			if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
			if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pDemo->pContext);
			return iRet;
		}
	}
	return XUI_ERROR_INVALID_ARGUMENT;
}

static void __xuiPageDemoUpdateStatus(xui_page_demo_t* pDemo)
{
	char sText[160];

	if ( (pDemo == NULL) || (pDemo->pStatus == NULL) ) return;
	snprintf(sText, sizeof(sText), "changes=%d last=%d->%d default=%d compact=%d green=%d",
		pDemo->iChangeCount,
		pDemo->iLastOldPage,
		pDemo->iLastNewPage,
		xuiPageGetCurrent(pDemo->pPage[0]),
		xuiPageGetCurrent(pDemo->pPage[1]),
		xuiPageGetCurrent(pDemo->pPage[2]));
	sText[sizeof(sText) - 1] = 0;
	(void)xuiLabelSetText(pDemo->pStatus, sText);
}

static void __xuiPageDemoRunChecks(xui_page_demo_t* pDemo, int bAutoRun)
{
	float fItemHeight;
	float fPageWidth;
	float fTextWidth;
	float fNavWidth;
	float fEllipsisWidth;
	int iBefore;
	int iRet;

	if ( pDemo == NULL ) return;
	pDemo->bCreateOK = (pDemo->pRoot != NULL) &&
	                   (pDemo->pPage[0] != NULL) &&
	                   (pDemo->pPage[1] != NULL) &&
	                   (pDemo->pPage[2] != NULL) &&
	                   (pDemo->pPage[3] != NULL);
	pDemo->bLayoutOK = (xuiPageGetItemCount(pDemo->pPage[0]) >= 10) &&
	                   (xuiPageGetItemCount(pDemo->pPage[1]) >= 7) &&
	                   (xuiWidgetGetRect(pDemo->pPage[0]).fW > 500.0f);
	iRet = xuiPageGetMetrics(pDemo->pPage[1], &fItemHeight, &fPageWidth, &fTextWidth, &fNavWidth, &fEllipsisWidth);
	pDemo->bStyleOK = (iRet == XUI_OK) &&
	                  (fItemHeight == 24.0f) &&
	                  (fPageWidth == 32.0f) &&
	                  (fTextWidth == 50.0f) &&
	                  (fNavWidth == 42.0f) &&
	                  (fEllipsisWidth == 26.0f) &&
	                  (xuiPageGetPageCount(pDemo->pPage[2]) == 19) &&
	                  ((xuiPageGetState(pDemo->pPage[3]) & XUI_WIDGET_STATE_DISABLED) != 0);
	if ( !bAutoRun || pDemo->bExerciseDone ) {
		__xuiPageDemoUpdateStatus(pDemo);
		return;
	}
	iBefore = xuiPageGetCurrent(pDemo->pPage[0]);
	iRet = __xuiPageDemoClickItem(pDemo, pDemo->pPage[0], XUI_PAGE_ITEM_NEXT);
	pDemo->bInputOK = (iRet == XUI_OK) &&
	                  (xuiPageGetCurrent(pDemo->pPage[0]) == iBefore + 1) &&
	                  (pDemo->iChangeCount > 0);
	(void)xuiSetFocusWidget(pDemo->pContext, pDemo->pPage[0]);
	(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_HOME, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_END, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	pDemo->bKeyboardOK = (xuiPageGetCurrent(pDemo->pPage[0]) == xuiPageGetPageCount(pDemo->pPage[0]));
	pDemo->bExerciseDone = 1;
	__xuiPageDemoUpdateStatus(pDemo);
}

static int __xuiPageDemoCreateAssets(xui_page_demo_t* pDemo)
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
	sFontPath = __xuiPageDemoFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiPageDemoCreateUi(pDemo);
}

static void __xuiPageDemoDestroyAssets(xui_page_demo_t* pDemo)
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

static int __xuiPageDemoFrame(void* pUser)
{
	xui_page_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_page_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiPageDemoHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiPageDemoRunChecks(pDemo, bAutoRun);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
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
		printf("xui_page final-summary frames=%d create=%d layout=%d style=%d input=%d keyboard=%d changes=%d current=%d items=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStyleOK, pDemo->bInputOK, pDemo->bKeyboardOK,
			pDemo->iChangeCount, xuiPageGetCurrent(pDemo->pPage[0]), xuiPageGetItemCount(pDemo->pPage[0]),
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_page_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiPageDemoParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiPageDemoUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Page";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_page: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiPageDemoCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_page: create assets failed: %d\n", iRet);
		__xuiPageDemoDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiPageDemoFrame, &tDemo);
	__xuiPageDemoDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStyleOK &&
	        ((tDemo.iMaxFrames <= 0 && tDemo.fMaxSeconds <= 0.0) || (tDemo.bInputOK && tDemo.bKeyboardOK))) ? 0 : 1;
}
