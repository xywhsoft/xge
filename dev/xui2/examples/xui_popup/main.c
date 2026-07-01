#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	520
#define DEMO_TARGET_H	340
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define POPUP_ITEM_COUNT 8
#define POPUP_LABEL_COUNT 12

typedef struct xui_popup_demo_t xui_popup_demo_t;

typedef struct xui_popup_item_t {
	xui_popup_demo_t* pDemo;
	uint32_t iColor;
	uint32_t iBorderColor;
} xui_popup_item_t;

struct xui_popup_demo_t {
	xui_proxy_t tProxy;
	xui_context pContext;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pButton;
	xui_widget pPopup;
	xui_widget pItem[POPUP_ITEM_COUNT];
	xui_widget pLabel[POPUP_LABEL_COUNT];
	xui_popup_item_t tItem[POPUP_ITEM_COUNT];
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int iPopupChanges;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bScrollOK;
	int bCloseOK;
};

static void __xuiPopupUsage(void)
{
	printf("usage: xui_popup [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiPopupParseArgs(xui_popup_demo_t* pDemo, int argc, char** argv)
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
			__xuiPopupUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiPopupFindTtf(void)
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

static int __xuiPopupRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_popup_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tBand;
	int iRet;

	(void)iStateId;
	pDemo = (xui_popup_demo_t*)pUser;
	if ( pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(244, 248, 253, 255));
	if ( iRet != XUI_OK ) return iRet;
	tBand = (xui_rect_t){24.0f, 112.0f, 458.0f, 176.0f};
	iRet = pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tBand, XUI_COLOR_RGBA(232, 241, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	return pDemo->tProxy.drawRectStroke(&pDemo->tProxy, pDraw, tBand, 1.0f, XUI_COLOR_RGBA(180, 203, 229, 255));
}

static int __xuiPopupItemRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_popup_item_t* pItem;
	xui_rect_t tRect;
	int iRet;

	(void)iStateId;
	pItem = (xui_popup_item_t*)pUser;
	if ( pItem == NULL || pItem->pDemo == NULL ) return XUI_OK;
	tRect = xuiWidgetGetContentRect(pWidget);
	iRet = pItem->pDemo->tProxy.drawRectFill(&pItem->pDemo->tProxy, pDraw, tRect, pItem->iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pItem->pDemo->tProxy.drawRectStroke(&pItem->pDemo->tProxy, pDraw, tRect, 1.0f, pItem->iBorderColor);
}

static void __xuiPopupButtonClick(xui_widget pWidget, void* pUser)
{
	xui_popup_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_popup_demo_t*)pUser;
	if ( pDemo != NULL && pDemo->pPopup != NULL ) {
		(void)xuiPopupToggle(pDemo->pPopup);
	}
}

static void __xuiPopupChanged(xui_widget pWidget, int bOpen, void* pUser)
{
	xui_popup_demo_t* pDemo;

	(void)pWidget;
	(void)bOpen;
	pDemo = (xui_popup_demo_t*)pUser;
	if ( pDemo != NULL ) {
		pDemo->iPopupChanges++;
	}
}

static int __xuiPopupAddLabel(xui_popup_demo_t* pDemo, xui_widget pParent, int iIndex, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= POPUP_LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
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

static int __xuiPopupAddItem(xui_popup_demo_t* pDemo, xui_widget pParent, int iIndex, xui_rect_t tRect, uint32_t iColor)
{
	xui_widget pItem;
	int iRet;

	if ( (iIndex < 0) || (iIndex >= POPUP_ITEM_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = xuiWidgetCreate(pDemo->pContext, &pItem);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->tItem[iIndex].pDemo = pDemo;
	pDemo->tItem[iIndex].iColor = iColor;
	pDemo->tItem[iIndex].iBorderColor = XUI_COLOR_RGBA(132, 164, 199, 190);
	iRet = xuiWidgetSetCacheRenderCallback(pItem, __xuiPopupItemRender, &pDemo->tItem[iIndex]);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pParent, pItem);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pItem);
		return iRet;
	}
	xuiWidgetSetRect(pItem, tRect);
	pDemo->pItem[iIndex] = pItem;
	return XUI_OK;
}

static int __xuiPopupCreateUi(xui_popup_demo_t* pDemo)
{
	xui_button_desc_t tButtonDesc;
	xui_popup_desc_t tPopupDesc;
	xui_widget pContent;
	int iRet;
	int i;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiPopupRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tButtonDesc, 0, sizeof(tButtonDesc));
	tButtonDesc.iSize = sizeof(tButtonDesc);
	tButtonDesc.sText = "Open popup";
	tButtonDesc.pFont = pDemo->pFont;
	tButtonDesc.iTextColor = XUI_COLOR_RGBA(246, 251, 255, 255);
	tButtonDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tButtonDesc.iNormalColor = XUI_COLOR_RGBA(32, 126, 210, 255);
	tButtonDesc.iHoverColor = XUI_COLOR_RGBA(52, 145, 224, 255);
	tButtonDesc.iActiveColor = XUI_COLOR_RGBA(24, 104, 182, 255);
	tButtonDesc.fRadius = 6.0f;
	iRet = xuiButtonCreate(pDemo->pContext, &pDemo->pButton, &tButtonDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pButton);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pButton, (xui_rect_t){34.0f, 34.0f, 148.0f, 34.0f});
	(void)xuiButtonSetClick(pDemo->pButton, __xuiPopupButtonClick, pDemo);

	iRet = __xuiPopupAddLabel(pDemo, pDemo->pRoot, 0, "Popup uses the overlay layer and a ScrollView payload.", (xui_rect_t){34.0f, 78.0f, 420.0f, 24.0f}, XUI_COLOR_RGBA(47, 64, 86, 255));
	if ( iRet != XUI_OK ) return iRet;

	memset(&tPopupDesc, 0, sizeof(tPopupDesc));
	tPopupDesc.iSize = sizeof(tPopupDesc);
	tPopupDesc.pOwner = pDemo->pButton;
	tPopupDesc.fContentWidth = 282.0f;
	tPopupDesc.fContentHeight = 420.0f;
	tPopupDesc.fMaxWidth = 330.0f;
	tPopupDesc.fMaxHeight = 220.0f;
	tPopupDesc.fGap = 7.0f;
	tPopupDesc.fMargin = 6.0f;
	tPopupDesc.fPadding = 5.0f;
	tPopupDesc.fBorderWidth = 1.0f;
	tPopupDesc.fRadius = 8.0f;
	tPopupDesc.fShadowSize = 4.0f;
	tPopupDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tPopupDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tPopupDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tPopupDesc.iOwnerPolicy = XUI_POPUP_OWNER_TOGGLE;
	tPopupDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tPopupDesc.iFocusPolicy = XUI_POPUP_FOCUS_POPUP;
	tPopupDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tPopupDesc.fScrollbarSize = 8.0f;
	tPopupDesc.iPanelColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	tPopupDesc.iBorderColor = XUI_COLOR_RGBA(152, 180, 211, 255);
	iRet = xuiPopupCreate(pDemo->pContext, &pDemo->pPopup, &tPopupDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiPopupSetChange(pDemo->pPopup, __xuiPopupChanged, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	pContent = xuiPopupGetContentWidget(pDemo->pPopup);
	if ( pContent == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	iRet = __xuiPopupAddLabel(pDemo, pContent, 1, "Project actions", (xui_rect_t){18.0f, 12.0f, 240.0f, 24.0f}, XUI_COLOR_RGBA(33, 51, 75, 255));
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < POPUP_ITEM_COUNT; i++ ) {
		xui_rect_t tItem = {18.0f, 48.0f + (float)i * 44.0f, 246.0f, 34.0f};
		uint32_t iColor = (i % 2) ? XUI_COLOR_RGBA(234, 246, 255, 255) : XUI_COLOR_RGBA(238, 249, 244, 255);
		iRet = __xuiPopupAddItem(pDemo, pContent, i, tItem, iColor);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiPopupAddLabel(pDemo, pContent, i + 2, (i % 2) ? "Open recent workspace" : "Create new scene", (xui_rect_t){32.0f, tItem.fY + 4.0f, 210.0f, 24.0f}, XUI_COLOR_RGBA(48, 66, 88, 255));
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiPopupAddLabel(pDemo, pContent, 10, "ScrollFrame keeps the payload width stable.", (xui_rect_t){18.0f, 390.0f, 250.0f, 24.0f}, XUI_COLOR_RGBA(83, 102, 125, 255));
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static uint32_t __xuiPopupReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiPopupSendButtonTransitions(xui_popup_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiPopupHandleInput(xui_popup_demo_t* pDemo)
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
		if ( xuiPopupIsOpen(pDemo->pPopup) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ESCAPE, 0);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			xgeQuit();
		}
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiPopupReadButtons();
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
	iRet = __xuiPopupSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static void __xuiPopupRunChecks(xui_popup_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tPopup;
	xui_rect_t tViewport;
	float fOffsetX;
	float fOffsetY;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pButton != NULL) && (pDemo->pPopup != NULL) && (xuiPopupGetContentWidget(pDemo->pPopup) != NULL);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiPopupSetOpen(pDemo->pPopup, 1);
		tPopup = xuiPopupGetPopupRect(pDemo->pPopup);
		(void)xuiInputPointerWheel(pDemo->pContext, tPopup.fX + 22.0f, tPopup.fY + 22.0f, 0.0f, -1.0f, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiPopupGetScroll(pDemo->pPopup, &fOffsetX, &fOffsetY);
		pDemo->bScrollOK = (fOffsetY > 0.0f);
		(void)xuiInputPointerDown(pDemo->pContext, 480.0f, 300.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bCloseOK = !xuiPopupIsOpen(pDemo->pPopup);
		(void)xuiPopupSetOpen(pDemo->pPopup, 1);
		pDemo->bExerciseDone = 1;
	}
	tPopup = xuiPopupGetPopupRect(pDemo->pPopup);
	tViewport = xuiPopupGetViewportRect(pDemo->pPopup);
	pDemo->bLayoutOK = xuiPopupIsOpen(pDemo->pPopup) && (tPopup.fW > 0.0f) && (tPopup.fH <= 220.0f) &&
	                   (tViewport.fW > 0.0f) && xuiScrollFrameIsVScrollBarVisible(xuiPopupGetFrameWidget(pDemo->pPopup)) &&
	                   !xuiScrollFrameIsHScrollBarVisible(xuiPopupGetFrameWidget(pDemo->pPopup));
	if ( !bExerciseInput ) {
		pDemo->bScrollOK = 1;
	} else if ( !pDemo->bScrollOK ) {
		(void)xuiPopupGetScroll(pDemo->pPopup, &fOffsetX, &fOffsetY);
		pDemo->bScrollOK = (fOffsetY > 0.0f);
	}
	if ( !bExerciseInput ) {
		pDemo->bCloseOK = 1;
	}
}

static int __xuiPopupCreateAssets(xui_popup_demo_t* pDemo)
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
	sFontPath = __xuiPopupFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiPopupCreateUi(pDemo);
}

static void __xuiPopupDestroyAssets(xui_popup_demo_t* pDemo)
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

static int __xuiPopupFrame(void* pUser)
{
	xui_popup_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_popup_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiPopupHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiPopupRunChecks(pDemo, bAutoRun);
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
		printf("xui_popup final-summary frames=%d create=%d layout=%d scroll=%d close=%d changes=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bScrollOK, pDemo->bCloseOK,
			pDemo->iPopupChanges, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_popup_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiPopupParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiPopupUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Popup";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_popup: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiPopupCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_popup: create assets failed: %d\n", iRet);
		__xuiPopupDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiPopupFrame, &tDemo);
	__xuiPopupDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bScrollOK && tDemo.bCloseOK) ? 0 : 1;
}
