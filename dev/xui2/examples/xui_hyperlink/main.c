#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	640
#define DEMO_TARGET_H	360
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define LINK_COUNT	4

typedef struct xui_hyperlink_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pStatus;
	xui_widget pLink[LINK_COUNT];
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
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInputOK;
} xui_hyperlink_demo_t;

static void __xuiHyperlinkUsage(void)
{
	printf("usage: xui_hyperlink [--frames N] [--seconds N]\n");
}

static int __xuiHyperlinkParseArgs(xui_hyperlink_demo_t* pDemo, int argc, char** argv)
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
			__xuiHyperlinkUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiHyperlinkFindTtf(void)
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

static int __xuiHyperlinkRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_hyperlink_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_hyperlink_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static int __xuiHyperlinkSetFixedHeight(xui_widget pWidget, float fHeight)
{
	int iRet;

	iRet = xuiWidgetSetSizeMode(pWidget, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pWidget, (xui_vec2_t){0.0f, fHeight});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAlign(pWidget, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	return iRet;
}

static int __xuiHyperlinkAddLabel(xui_hyperlink_demo_t* pDemo, const char* sText, xui_widget* ppLabel)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(52, 67, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiHyperlinkSetFixedHeight(pLabel, 26.0f);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	if ( ppLabel != NULL ) {
		*ppLabel = pLabel;
	}
	return XUI_OK;
}

static void __xuiHyperlinkClicked(xui_widget pWidget, void* pUser)
{
	xui_hyperlink_demo_t* pDemo;
	char sStatus[128];

	(void)pWidget;
	pDemo = (xui_hyperlink_demo_t*)pUser;
	if ( pDemo == NULL ) {
		return;
	}
	pDemo->iClickCount++;
	snprintf(sStatus, sizeof(sStatus), "clicks=%d state=%u", pDemo->iClickCount, xuiHyperlinkGetState(pWidget));
	(void)xuiLabelSetText(pDemo->pStatus, sStatus);
}

static int __xuiHyperlinkAddLink(xui_hyperlink_demo_t* pDemo, int iIndex, const char* sText, int bEnabled)
{
	xui_hyperlink_desc_t tDesc;
	xui_widget pLink;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= LINK_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.bHoverUnderline = 1;
	tDesc.bActiveUnderline = 1;
	iRet = xuiHyperlinkCreate(pDemo->pContext, &pLink, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)__xuiHyperlinkSetFixedHeight(pLink, 28.0f);
	if ( !bEnabled ) {
		(void)xuiWidgetSetEnabled(pLink, 0);
	}
	(void)xuiHyperlinkSetClick(pLink, __xuiHyperlinkClicked, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLink);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLink);
		return iRet;
	}
	pDemo->pLink[iIndex] = pLink;
	return XUI_OK;
}

static int __xuiHyperlinkCreateUi(xui_hyperlink_demo_t* pDemo)
{
	xui_thickness_t tPadding;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_COLUMN);
	(void)xuiWidgetSetGap(pDemo->pRoot, 8.0f);
	tPadding = (xui_thickness_t){28.0f, 22.0f, 28.0f, 22.0f};
	(void)xuiWidgetSetPadding(pDemo->pRoot, tPadding);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiHyperlinkRootRender, pDemo);
	(void)xuiWidgetSetSizeMode(pDemo->pRoot, XUI_SIZE_FILL, XUI_SIZE_FILL);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	if ( __xuiHyperlinkAddLabel(pDemo, "XUI Hyperlink", NULL) != XUI_OK ||
	     __xuiHyperlinkAddLink(pDemo, 0, "Default link", 1) != XUI_OK ||
	     __xuiHyperlinkAddLink(pDemo, 1, "Green custom link", 1) != XUI_OK ||
	     __xuiHyperlinkAddLink(pDemo, 2, "Disabled link", 0) != XUI_OK ||
	     __xuiHyperlinkAddLink(pDemo, 3, "Wrapped link text keeps the Label layout path but still handles click states.", 1) != XUI_OK ||
	     __xuiHyperlinkAddLabel(pDemo, "clicks=0", &pDemo->pStatus) != XUI_OK ) {
		return XUI_ERROR;
	}
	(void)xuiHyperlinkSetTextColors(
		pDemo->pLink[1],
		XUI_COLOR_RGBA(22, 150, 102, 255),
		XUI_COLOR_RGBA(28, 178, 120, 255),
		XUI_COLOR_RGBA(12, 112, 74, 255),
		XUI_COLOR_RGBA(150, 160, 168, 255));
	(void)xuiHyperlinkSetUnderline(pDemo->pLink[1], 0, 1, 1);
	(void)xuiHyperlinkSetWrapMode(pDemo->pLink[3], XUI_TEXT_WRAP_WORD);
	(void)xuiHyperlinkSetTextFlags(pDemo->pLink[3], XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_TOP | XUI_TEXT_CLIP);
	(void)__xuiHyperlinkSetFixedHeight(pDemo->pLink[3], 48.0f);
	return XUI_OK;
}

static void __xuiHyperlinkLayout(xui_hyperlink_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiHyperlinkReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiHyperlinkSendButtonTransitions(xui_hyperlink_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiHyperlinkHandleInput(xui_hyperlink_demo_t* pDemo)
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
	iButtons = __xuiHyperlinkReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	iRet = __xuiHyperlinkSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiHyperlinkRunChecks(xui_hyperlink_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tRect;
	float fX;
	float fY;
	int i;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pStatus != NULL);
	for ( i = 0; i < LINK_COUNT; i++ ) {
		if ( pDemo->pLink[i] == NULL ) pDemo->bCreateOK = 0;
	}
	tRect = xuiWidgetGetRect(pDemo->pLink[0]);
	pDemo->bLayoutOK = (tRect.fW > 100.0f) && (tRect.fH >= 24.0f);
	pDemo->bStateOK = (xuiWidgetGetCacheStateCount(pDemo->pLink[0]) == 3) &&
	                  (xuiWidgetGetEnabled(pDemo->pLink[2]) == 0) &&
	                  (xuiHyperlinkGetTextColor(pDemo->pLink[1]) == XUI_COLOR_RGBA(22, 150, 102, 255)) &&
	                  (xuiHyperlinkGetWrapMode(pDemo->pLink[3]) == XUI_TEXT_WRAP_WORD);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		tRect = xuiWidgetGetWorldRect(pDemo->pLink[0]);
		fX = tRect.fX + tRect.fW * 0.5f;
		fY = tRect.fY + tRect.fH * 0.5f;
		(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bExerciseDone = 1;
	}
	pDemo->bInputOK = !bExerciseInput || ((pDemo->iClickCount > 0) && (xuiHyperlinkGetClickCount(pDemo->pLink[0]) > 0));
}

static int __xuiHyperlinkCreateAssets(xui_hyperlink_demo_t* pDemo)
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
	sFontPath = __xuiHyperlinkFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, 0);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiHyperlinkCreateUi(pDemo);
}

static void __xuiHyperlinkDestroyAssets(xui_hyperlink_demo_t* pDemo)
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

static int __xuiHyperlinkFrame(void* pUser)
{
	xui_hyperlink_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_hyperlink_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiHyperlinkHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiHyperlinkLayout(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiHyperlinkRunChecks(pDemo, bAutoRun);
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
		printf("xui_hyperlink final-summary frames=%d create=%d layout=%d state=%d input=%d clicks=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bStateOK, pDemo->bInputOK, pDemo->iClickCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_hyperlink_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiHyperlinkParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiHyperlinkUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Hyperlink";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_hyperlink: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiHyperlinkCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_hyperlink: create assets failed: %d\n", iRet);
		__xuiHyperlinkDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiHyperlinkFrame, &tDemo);
	__xuiHyperlinkDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bStateOK && tDemo.bInputOK) ? 0 : 1;
}
