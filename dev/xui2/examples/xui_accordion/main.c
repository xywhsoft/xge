#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	760
#define DEMO_TARGET_H	420
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define ACCORDION_COUNT	3
#define LABEL_COUNT	18

typedef struct xui_accordion_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pAccordion[ACCORDION_COUNT];
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
	int iLastSelect;
	int iLastId;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bMultiOK;
	int bSingleOK;
	int bDisabledOK;
	int bStyleOK;
} xui_accordion_demo_t;

static void __xuiAccordionUsage(void)
{
	printf("usage: xui_accordion [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiAccordionParseArgs(xui_accordion_demo_t* pDemo, int argc, char** argv)
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
			__xuiAccordionUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiAccordionFindTtf(void)
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

static int __xuiAccordionRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_accordion_demo_t* pDemo;
	xui_rect_t tRect;

	(void)iStateId;
	pDemo = (xui_accordion_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(246, 249, 253, 255));
	}
	return pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(246, 249, 253, 255));
}

static void __xuiAccordionSelected(xui_widget pWidget, int iIndex, int iId, void* pUser)
{
	xui_accordion_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_accordion_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iSelectCount++;
		pDemo->iLastSelect = iIndex;
		pDemo->iLastId = iId;
	}
}

static int __xuiAccordionAddLabel(xui_accordion_demo_t* pDemo, xui_widget pAccordion, int iSection, const char* sText, uint32_t iColor)
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
	iRet = xuiAccordionAddSectionChild(pAccordion, iSection, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[pDemo->iLabelCount++] = pLabel;
	return XUI_OK;
}

static int __xuiAccordionCreateOne(xui_accordion_demo_t* pDemo, int iIndex, int iMode)
{
	xui_accordion_desc_t tDesc;
	xui_widget pAccordion;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = iMode;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiAccordionCreate(pDemo->pContext, &pAccordion, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiAccordionSetSelect(pAccordion, __xuiAccordionSelected, pDemo);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pAccordion);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pAccordion);
		return iRet;
	}
	pDemo->pAccordion[iIndex] = pAccordion;
	return XUI_OK;
}

static int __xuiAccordionCreateUi(xui_accordion_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiAccordionRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiAccordionCreateOne(pDemo, 0, XUI_ACCORDION_MODE_MULTIPLE);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionCreateOne(pDemo, 1, XUI_ACCORDION_MODE_SINGLE);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionCreateOne(pDemo, 2, XUI_ACCORDION_MODE_MULTIPLE);
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiAccordionAddSection(pDemo->pAccordion[0], "Project", 10, 1, NULL);
	(void)xuiAccordionAddSection(pDemo->pAccordion[0], "Renderer", 11, 0, NULL);
	(void)xuiAccordionAddSection(pDemo->pAccordion[0], "Disabled Tools", 12, 1, NULL);
	(void)xuiAccordionSetSectionEnabled(pDemo->pAccordion[0], 2, 0);
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[0], 0, "Scenes, assets, and build settings", XUI_COLOR_RGBA(46, 64, 86, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[0], 0, "Multiple sections can stay open", XUI_COLOR_RGBA(78, 96, 116, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[0], 1, "Pipeline, material, and viewport options", XUI_COLOR_RGBA(46, 64, 86, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[0], 2, "Disabled headers do not toggle", XUI_COLOR_RGBA(126, 148, 160, 255));

	(void)xuiAccordionAddSection(pDemo->pAccordion[1], "Scene", 20, 0, NULL);
	(void)xuiAccordionAddSection(pDemo->pAccordion[1], "Inspector", 21, 1, NULL);
	(void)xuiAccordionAddSection(pDemo->pAccordion[1], "Console", 22, 0, NULL);
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[1], 0, "Hierarchy and selection state", XUI_COLOR_RGBA(46, 64, 86, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[1], 1, "Single mode collapses the previous page", XUI_COLOR_RGBA(46, 64, 86, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[1], 2, "Logs and command output", XUI_COLOR_RGBA(46, 64, 86, 255));

	(void)xuiAccordionAddSection(pDemo->pAccordion[2], "Green Accent", 30, 1, NULL);
	(void)xuiAccordionAddSection(pDemo->pAccordion[2], "Compact Group", 31, 0, NULL);
	(void)xuiAccordionSetMetrics(pDemo->pAccordion[2], 26.0f, 3.0f, 7.0f);
	(void)xuiAccordionSetColors(pDemo->pAccordion[2],
		XUI_COLOR_RGBA(246, 252, 249, 255),
		XUI_COLOR_RGBA(219, 242, 234, 255),
		XUI_COLOR_RGBA(203, 235, 224, 255),
		XUI_COLOR_RGBA(205, 238, 228, 255),
		XUI_COLOR_RGBA(250, 254, 252, 255),
		XUI_COLOR_RGBA(84, 180, 148, 255),
		XUI_COLOR_RGBA(30, 82, 66, 255),
		XUI_COLOR_RGBA(18, 102, 78, 255),
		XUI_COLOR_RGBA(126, 150, 142, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[2], 0, "Metrics and colors are configurable", XUI_COLOR_RGBA(34, 84, 66, 255));
	(void)__xuiAccordionAddLabel(pDemo, pDemo->pAccordion[2], 1, "The client remains a normal widget", XUI_COLOR_RGBA(34, 84, 66, 255));
	return XUI_OK;
}

static void __xuiAccordionLayoutDemo(xui_accordion_demo_t* pDemo)
{
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetRect(pDemo->pAccordion[0], (xui_rect_t){34.0f, 34.0f, 320.0f, 336.0f});
	(void)xuiWidgetSetRect(pDemo->pAccordion[1], (xui_rect_t){396.0f, 34.0f, 330.0f, 188.0f});
	(void)xuiWidgetSetRect(pDemo->pAccordion[2], (xui_rect_t){396.0f, 252.0f, 330.0f, 118.0f});
	(void)xuiLayout(pDemo->pContext);
}

static uint32_t __xuiAccordionReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiAccordionSendButtonTransitions(xui_accordion_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiAccordionSendKey(xui_accordion_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiAccordionHandleInput(xui_accordion_demo_t* pDemo)
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
	iButtons = __xuiAccordionReadButtons();
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
	iRet = __xuiAccordionSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_UP, XUI_KEY_UP);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_DOWN, XUI_KEY_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_HOME, XUI_KEY_HOME);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_END, XUI_KEY_END);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_SPACE, XUI_KEY_SPACE);
	if ( iRet == XUI_OK ) iRet = __xuiAccordionSendKey(pDemo, XGE_KEY_ENTER, XUI_KEY_ENTER);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiAccordionExerciseClick(xui_accordion_demo_t* pDemo, xui_widget pAccordion, xui_rect_t tRect)
{
	xui_rect_t tWorld;
	float fX;
	float fY;

	tWorld = xuiWidgetGetWorldRect(pAccordion);
	fX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	(void)xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
	(void)xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiAccordionRunChecks(xui_accordion_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tClient0;
	xui_rect_t tClient1;
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) &&
		(pDemo->pAccordion[0] != NULL) &&
		(pDemo->pAccordion[1] != NULL) &&
		(pDemo->pAccordion[2] != NULL) &&
		(xuiAccordionGetSectionCount(pDemo->pAccordion[0]) == 3);
	tClient0 = xuiAccordionGetClientRect(pDemo->pAccordion[0], 0);
	tClient1 = xuiAccordionGetClientRect(pDemo->pAccordion[0], 1);
	pDemo->bLayoutOK = (tClient0.fH > 20.0f) &&
		(tClient1.fW > 200.0f) &&
		(xuiAccordionGetArrowRect(pDemo->pAccordion[0], 0).fW > 0.0f);
	pDemo->bStyleOK = (xuiAccordionGetContentHeight(pDemo->pAccordion[2]) > 20.0f) &&
		(xuiAccordionGetMode(pDemo->pAccordion[2]) == XUI_ACCORDION_MODE_MULTIPLE);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bLayoutOK ) {
		iBefore = pDemo->iSelectCount;
		__xuiAccordionExerciseClick(pDemo, pDemo->pAccordion[0], xuiAccordionGetHeaderRect(pDemo->pAccordion[0], 1));
		pDemo->bMultiOK = xuiAccordionIsExpanded(pDemo->pAccordion[0], 0) &&
			xuiAccordionIsExpanded(pDemo->pAccordion[0], 1) &&
			xuiAccordionIsExpanded(pDemo->pAccordion[0], 2) &&
			(pDemo->iSelectCount > iBefore) &&
			(pDemo->iLastId == 11);
		iBefore = pDemo->iSelectCount;
		__xuiAccordionExerciseClick(pDemo, pDemo->pAccordion[0], xuiAccordionGetHeaderRect(pDemo->pAccordion[0], 2));
		pDemo->bDisabledOK = (pDemo->iSelectCount == iBefore) && xuiAccordionIsExpanded(pDemo->pAccordion[0], 2);
		__xuiAccordionExerciseClick(pDemo, pDemo->pAccordion[1], xuiAccordionGetHeaderRect(pDemo->pAccordion[1], 0));
		pDemo->bSingleOK = xuiAccordionIsExpanded(pDemo->pAccordion[1], 0) &&
			!xuiAccordionIsExpanded(pDemo->pAccordion[1], 1) &&
			!xuiAccordionIsExpanded(pDemo->pAccordion[1], 2);
		pDemo->bExerciseDone = 1;
	}
	if ( !bExerciseInput ) {
		pDemo->bMultiOK = 1;
		pDemo->bSingleOK = 1;
		pDemo->bDisabledOK = 1;
	}
}

static int __xuiAccordionCreateAssets(xui_accordion_demo_t* pDemo)
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
	sFontPath = __xuiAccordionFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 15.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiAccordionCreateUi(pDemo);
}

static void __xuiAccordionDestroyAssets(xui_accordion_demo_t* pDemo)
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

static int __xuiAccordionFrame(void* pUser)
{
	xui_accordion_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_accordion_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiAccordionHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	__xuiAccordionLayoutDemo(pDemo);
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiAccordionRunChecks(pDemo, bAutoRun);
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
		printf("xui_accordion final-summary frames=%d create=%d layout=%d multi=%d single=%d disabled=%d style=%d callbacks=%d last=%d/%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bMultiOK, pDemo->bSingleOK, pDemo->bDisabledOK,
			pDemo->bStyleOK, pDemo->iSelectCount, pDemo->iLastSelect, pDemo->iLastId,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_accordion_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	tDemo.iLastSelect = -1;
	iRet = __xuiAccordionParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiAccordionUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Accordion";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_accordion: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiAccordionCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_accordion: create assets failed: %d\n", iRet);
		__xuiAccordionDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiAccordionFrame, &tDemo);
	__xuiAccordionDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bMultiOK && tDemo.bSingleOK && tDemo.bDisabledOK && tDemo.bStyleOK) ? 0 : 1;
}
