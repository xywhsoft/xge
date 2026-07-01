#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 540
#define DEMO_TARGET_H 340
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_menu_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pButton;
	xui_widget pStatus;
	xui_widget pMenu;
	xui_widget pSubmenu;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bSelectOK;
	int bCheckOK;
	int bSubmenuOK;
	int bCloseOK;
	int iSelectCount;
	int iLastValue;
} xui_menu_demo_t;

static void __xuiMenuUsage(void)
{
	printf("usage: xui_menu [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMenuParseArgs(xui_menu_demo_t* pDemo, int argc, char** argv)
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
			__xuiMenuUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMenuFindTtf(void)
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

static int __xuiMenuRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_menu_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tBand;

	(void)iStateId;
	pDemo = (xui_menu_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(236, 243, 252, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tBand = (xui_rect_t){24.0f, 24.0f, tRect.fW - 48.0f, 120.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tBand, XUI_COLOR_RGBA(247, 250, 255, 255));
	}
	return XUI_OK;
}

static void __xuiMenuSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menu_demo_t* pDemo;
	char sText[128];

	(void)pMenu;
	(void)iIndex;
	pDemo = (xui_menu_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "Selected command value: %d", iValue);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static void __xuiMenuButtonClicked(xui_widget pWidget, void* pUser)
{
	xui_menu_demo_t* pDemo;

	(void)pWidget;
	pDemo = (xui_menu_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	if ( xuiMenuIsOpen(pDemo->pMenu) ) {
		(void)xuiMenuClose(pDemo->pMenu);
	} else {
		(void)xuiMenuOpenForOwner(pDemo->pMenu, pDemo->pButton);
	}
}

static int __xuiMenuAddLabel(xui_menu_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
{
	xui_label_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = sText;
	tDesc.pFont = pDemo->pFont;
	tDesc.iTextColor = iColor;
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, ppWidget, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(*ppWidget, tRect);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static int __xuiMenuCreateMenus(xui_menu_demo_t* pDemo)
{
	xui_menu_desc_t tDesc;
	xui_menu_item_t arrItems[9];
	xui_menu_item_t arrSubItems[3];
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pDemo->pButton;
	tDesc.pFont = pDemo->pFont;
	iRet = xuiMenuCreate(pDemo->pContext, &pDemo->pSubmenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrSubItems, 0, sizeof(arrSubItems));
	arrSubItems[0].sText = "recent_scene.xge";
	arrSubItems[0].sShortcut = "Ctrl+Shift+1";
	arrSubItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[0].iState = XUI_MENU_ITEM_ENABLED;
	arrSubItems[0].iValue = 51;
	arrSubItems[1].sText = "ui_lab.xge";
	arrSubItems[1].sShortcut = "Ctrl+Shift+2";
	arrSubItems[1].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrSubItems[1].iValue = 52;
	arrSubItems[2].sText = "Clear list";
	arrSubItems[2].iType = XUI_MENU_ITEM_NORMAL;
	arrSubItems[2].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER;
	arrSubItems[2].iValue = 53;
	iRet = xuiMenuSetItems(pDemo->pSubmenu, arrSubItems, 3);
	if ( iRet != XUI_OK ) return iRet;

	iRet = xuiMenuCreate(pDemo->pContext, &pDemo->pMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMenuSetSelect(pDemo->pMenu, __xuiMenuSelected, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0].sText = "New scene";
	arrItems[0].sShortcut = "Ctrl+N";
	arrItems[0].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[0].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT;
	arrItems[0].iValue = 10;
	arrItems[1].sText = "Open recent";
	arrItems[1].iType = XUI_MENU_ITEM_SUBMENU;
	arrItems[1].iState = XUI_MENU_ITEM_ENABLED;
	arrItems[1].pSubmenu = pDemo->pSubmenu;
	arrItems[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[3].sText = "Snap to grid";
	arrItems[3].sShortcut = "G";
	arrItems[3].iType = XUI_MENU_ITEM_CHECK;
	arrItems[3].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED;
	arrItems[3].iValue = 20;
	arrItems[4].sText = "Small icons";
	arrItems[4].iType = XUI_MENU_ITEM_RADIO;
	arrItems[4].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED;
	arrItems[4].iValue = 30;
	arrItems[5].sText = "Large icons";
	arrItems[5].iType = XUI_MENU_ITEM_RADIO;
	arrItems[5].iState = XUI_MENU_ITEM_ENABLED;
	arrItems[5].iValue = 31;
	arrItems[6].iType = XUI_MENU_ITEM_SEPARATOR;
	arrItems[7].sText = "Delete selection";
	arrItems[7].sShortcut = "Del";
	arrItems[7].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[7].iState = XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER;
	arrItems[7].iValue = 40;
	arrItems[8].sText = "Disabled action";
	arrItems[8].iType = XUI_MENU_ITEM_NORMAL;
	arrItems[8].iState = 0;
	arrItems[8].iValue = 60;
	return xuiMenuSetItems(pDemo->pMenu, arrItems, 9);
}

static int __xuiMenuCreateUi(xui_menu_demo_t* pDemo)
{
	xui_button_desc_t tButtonDesc;
	xui_widget pTitle;
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiMenuRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiMenuAddLabel(pDemo, &pTitle, "XUI Menu", (xui_rect_t){40.0f, 34.0f, 160.0f, 24.0f}, XUI_COLOR_RGBA(40, 66, 96, 255));
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuAddLabel(pDemo, &pDemo->pStatus, "Selected command value: none", (xui_rect_t){40.0f, 100.0f, 300.0f, 24.0f}, XUI_COLOR_RGBA(80, 102, 128, 255));
	if ( iRet != XUI_OK ) return iRet;

	memset(&tButtonDesc, 0, sizeof(tButtonDesc));
	tButtonDesc.iSize = sizeof(tButtonDesc);
	tButtonDesc.sText = "File";
	tButtonDesc.pFont = pDemo->pFont;
	tButtonDesc.iTextColor = XUI_COLOR_RGBA(247, 251, 255, 255);
	tButtonDesc.iTextFlags = XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tButtonDesc.iNormalColor = XUI_COLOR_RGBA(34, 118, 206, 255);
	tButtonDesc.iHoverColor = XUI_COLOR_RGBA(56, 139, 222, 255);
	tButtonDesc.iActiveColor = XUI_COLOR_RGBA(24, 96, 176, 255);
	tButtonDesc.fRadius = 6.0f;
	iRet = xuiButtonCreate(pDemo->pContext, &pDemo->pButton, &tButtonDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pButton, (xui_rect_t){40.0f, 64.0f, 104.0f, 32.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pButton);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiButtonSetClick(pDemo->pButton, __xuiMenuButtonClicked, pDemo);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMenuCreateMenus(pDemo);
}

static uint32_t __xuiMenuReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMenuSendButtonTransitions(xui_menu_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiMenuHandleInput(xui_menu_demo_t* pDemo)
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
		if ( xuiMenuIsOpen(pDemo->pMenu) || xuiMenuIsOpen(pDemo->pSubmenu) ) {
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
	iButtons = __xuiMenuReadButtons();
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
	iRet = __xuiMenuSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiMenuClickItem(xui_menu_demo_t* pDemo, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiMenuMoveToItem(xui_menu_demo_t* pDemo, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	iRet = xuiInputPointerMove(pDemo->pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static void __xuiMenuRunChecks(xui_menu_demo_t* pDemo, int bExerciseInput)
{
	xui_rect_t tPopup;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pButton != NULL) && (pDemo->pMenu != NULL) &&
		(xuiMenuGetPopupWidget(pDemo->pMenu) != NULL) && (xuiMenuGetContentWidget(pDemo->pMenu) == pDemo->pMenu);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiMenuOpenForOwner(pDemo->pMenu, pDemo->pButton);
		(void)xuiLayout(pDemo->pContext);
		(void)xuiInputKeyDown(pDemo->pContext, 'G', 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bCheckOK = (xuiMenuGetItemState(pDemo->pMenu, 3) & XUI_MENU_ITEM_CHECKED) == 0u;
		(void)xuiMenuOpenForOwner(pDemo->pMenu, pDemo->pButton);
		(void)xuiLayout(pDemo->pContext);
		(void)__xuiMenuMoveToItem(pDemo, pDemo->pMenu, 1);
		(void)xuiLayout(pDemo->pContext);
		pDemo->bSubmenuOK = xuiMenuIsOpen(pDemo->pSubmenu);
		(void)__xuiMenuClickItem(pDemo, pDemo->pSubmenu, 1);
		pDemo->bSelectOK = (pDemo->iSelectCount >= 2) && (pDemo->iLastValue == 52);
		pDemo->bCloseOK = !xuiMenuIsOpen(pDemo->pMenu) && !xuiMenuIsOpen(pDemo->pSubmenu);
		(void)xuiMenuOpenForOwner(pDemo->pMenu, pDemo->pButton);
		pDemo->bExerciseDone = 1;
	}
	tPopup = xuiPopupGetPopupRect(xuiMenuGetPopupWidget(pDemo->pMenu));
	pDemo->bLayoutOK = xuiMenuIsOpen(pDemo->pMenu) && (tPopup.fW > 120.0f) && (tPopup.fH > 120.0f);
	if ( !bExerciseInput ) {
		pDemo->bSelectOK = 1;
		pDemo->bCheckOK = 1;
		pDemo->bSubmenuOK = 1;
		pDemo->bCloseOK = 1;
	}
}

static int __xuiMenuCreateAssets(xui_menu_demo_t* pDemo)
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
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiMenuFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMenuCreateUi(pDemo);
}

static void __xuiMenuDestroyAssets(xui_menu_demo_t* pDemo)
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

static int __xuiMenuFrame(void* pUser)
{
	xui_menu_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_menu_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMenuHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiMenuRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(236, 243, 252, 255));
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
		printf("xui_menu final-summary frames=%d create=%d layout=%d select=%d check=%d submenu=%d close=%d selected=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bSelectOK, pDemo->bCheckOK, pDemo->bSubmenuOK,
			pDemo->bCloseOK, pDemo->iSelectCount, tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_menu_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiMenuParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiMenuUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI Menu";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_menu: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMenuCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_menu: create assets failed: %d\n", iRet);
		__xuiMenuDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiMenuFrame, &tDemo);
	__xuiMenuDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bSelectOK && tDemo.bCheckOK && tDemo.bSubmenuOK && tDemo.bCloseOK) ? 0 : 1;
}
