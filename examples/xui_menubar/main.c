#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W 640
#define DEMO_TARGET_H 360
#define DEMO_OFFSET_X 10.0f
#define DEMO_OFFSET_Y 20.0f

typedef struct xui_menubar_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pMenuBar;
	xui_widget pStatus;
	xui_widget pHint;
	xui_widget pFileMenu;
	xui_widget pEditMenu;
	xui_widget pViewMenu;
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
	int bOpenOK;
	int bSwitchOK;
	int bMnemonicOK;
	int bSelectOK;
	int bCloseOK;
	int iSelectCount;
	int iLastValue;
} xui_menubar_demo_t;

static void __xuiMenuBarUsage(void)
{
	printf("usage: xui_menubar [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiMenuBarParseArgs(xui_menubar_demo_t* pDemo, int argc, char** argv)
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
			__xuiMenuBarUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiMenuBarFindTtf(void)
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

static int __xuiMenuBarRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_menubar_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_menubar_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(232, 240, 249, 255));
	}
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		tPanel = (xui_rect_t){24.0f, 34.0f, tRect.fW - 48.0f, 260.0f};
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tPanel, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiMenuBarSelected(xui_widget pMenu, int iIndex, int iValue, void* pUser)
{
	xui_menubar_demo_t* pDemo;
	char sText[128];

	(void)pMenu;
	(void)iIndex;
	pDemo = (xui_menubar_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iSelectCount++;
	pDemo->iLastValue = iValue;
	snprintf(sText, sizeof(sText), "Selected command value: %d", iValue);
	if ( pDemo->pStatus != NULL ) {
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static int __xuiMenuBarAddLabel(xui_menubar_demo_t* pDemo, xui_widget* ppWidget, const char* sText, xui_rect_t tRect, uint32_t iColor)
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
	(void)xuiWidgetSetRect(*ppWidget, tRect);
	return xuiWidgetAddChild(pDemo->pRoot, *ppWidget);
}

static int __xuiMenuBarCreateMenus(xui_menubar_demo_t* pDemo)
{
	xui_menu_desc_t tDesc;
	xui_menu_item_t arrFile[5];
	xui_menu_item_t arrEdit[4];
	xui_menu_item_t arrView[4];
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pDemo->pMenuBar;
	tDesc.pFont = pDemo->pFont;

	iRet = xuiMenuCreate(pDemo->pContext, &pDemo->pFileMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrFile, 0, sizeof(arrFile));
	arrFile[0] = (xui_menu_item_t){"New scene", "Ctrl+N", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DEFAULT, 10, 0, NULL, NULL};
	arrFile[1] = (xui_menu_item_t){"Open...", "Ctrl+O", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 11, 0, NULL, NULL};
	arrFile[2].iType = XUI_MENU_ITEM_SEPARATOR;
	arrFile[3] = (xui_menu_item_t){"Save", "Ctrl+S", XUI_MENU_ITEM_NORMAL, 0, 12, 0, NULL, NULL};
	arrFile[4] = (xui_menu_item_t){"Exit", "Alt+F4", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_DANGER, 13, 0, NULL, NULL};
	iRet = xuiMenuSetItems(pDemo->pFileMenu, arrFile, 5);
	if ( iRet != XUI_OK ) return iRet;

	iRet = xuiMenuCreate(pDemo->pContext, &pDemo->pEditMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrEdit, 0, sizeof(arrEdit));
	arrEdit[0] = (xui_menu_item_t){"Undo", "Ctrl+Z", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 20, 0, NULL, NULL};
	arrEdit[1] = (xui_menu_item_t){"Cut", "Ctrl+X", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 21, 0, NULL, NULL};
	arrEdit[2] = (xui_menu_item_t){"Copy", "Ctrl+C", XUI_MENU_ITEM_NORMAL, XUI_MENU_ITEM_ENABLED, 22, 0, NULL, NULL};
	arrEdit[3] = (xui_menu_item_t){"Paste", "Ctrl+V", XUI_MENU_ITEM_NORMAL, 0, 23, 0, NULL, NULL};
	iRet = xuiMenuSetItems(pDemo->pEditMenu, arrEdit, 4);
	if ( iRet != XUI_OK ) return iRet;

	iRet = xuiMenuCreate(pDemo->pContext, &pDemo->pViewMenu, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	memset(arrView, 0, sizeof(arrView));
	arrView[0] = (xui_menu_item_t){"Show toolbar", NULL, XUI_MENU_ITEM_CHECK, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 30, 0, NULL, NULL};
	arrView[1].iType = XUI_MENU_ITEM_SEPARATOR;
	arrView[2] = (xui_menu_item_t){"Small icons", NULL, XUI_MENU_ITEM_RADIO, XUI_MENU_ITEM_ENABLED | XUI_MENU_ITEM_CHECKED, 31, 0, NULL, NULL};
	arrView[3] = (xui_menu_item_t){"Large icons", NULL, XUI_MENU_ITEM_RADIO, XUI_MENU_ITEM_ENABLED, 32, 0, NULL, NULL};
	return xuiMenuSetItems(pDemo->pViewMenu, arrView, 4);
}

static int __xuiMenuBarCreateUi(xui_menubar_demo_t* pDemo)
{
	xui_menubar_desc_t tDesc;
	xui_menubar_item_t arrItems[4];
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiMenuBarRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	iRet = xuiMenuBarCreate(pDemo->pContext, &pDemo->pMenuBar, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pMenuBar, (xui_rect_t){42.0f, 56.0f, 530.0f, 28.0f});
	iRet = xuiWidgetAddChild(pDemo->pRoot, pDemo->pMenuBar);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiMenuBarCreateMenus(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiMenuBarSetSelect(pDemo->pMenuBar, __xuiMenuBarSelected, pDemo);
	if ( iRet != XUI_OK ) return iRet;

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0] = (xui_menubar_item_t){"&File", XUI_MENUBAR_ITEM_ENABLED, 1, 0, pDemo->pFileMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[1] = (xui_menubar_item_t){"&Edit", XUI_MENUBAR_ITEM_ENABLED, 2, 0, pDemo->pEditMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[2] = (xui_menubar_item_t){"&View", XUI_MENUBAR_ITEM_ENABLED, 3, 0, pDemo->pViewMenu, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[3] = (xui_menubar_item_t){"&Help", XUI_MENUBAR_ITEM_ENABLED, 99, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	iRet = xuiMenuBarSetItems(pDemo->pMenuBar, arrItems, 4);
	if ( iRet != XUI_OK ) return iRet;

	iRet = __xuiMenuBarAddLabel(pDemo, &pDemo->pStatus, "Selected command value: none",
		(xui_rect_t){58.0f, 126.0f, 340.0f, 28.0f}, XUI_COLOR_RGBA(42, 62, 88, 255));
	if ( iRet != XUI_OK ) return iRet;
	return __xuiMenuBarAddLabel(pDemo, &pDemo->pHint, "Use mouse, Alt+letter, F10, arrows, Enter, Space, and Escape.",
		(xui_rect_t){58.0f, 166.0f, 500.0f, 28.0f}, XUI_COLOR_RGBA(84, 102, 124, 255));
}

static uint32_t __xuiMenuBarReadButtons(void)
{
	uint32_t iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiMenuBarSendButtonTransitions(xui_menubar_demo_t* pDemo, float fX, float fY, uint32_t iButtons, uint32_t iPressed, uint32_t iReleased)
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

static int __xuiMenuBarSendKey(xui_menubar_demo_t* pDemo, int iXgeKey, int iXuiKey)
{
	if ( xgeKeyPressed(iXgeKey) ) {
		return xuiInputKeyDown(pDemo->pContext, iXuiKey, 0);
	}
	return XUI_OK;
}

static int __xuiMenuBarHandleInput(xui_menubar_demo_t* pDemo)
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
		if ( xuiMenuBarIsOpen(pDemo->pMenuBar) || xuiMenuIsOpen(pDemo->pFileMenu) || xuiMenuIsOpen(pDemo->pEditMenu) || xuiMenuIsOpen(pDemo->pViewMenu) ) {
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
	iButtons = __xuiMenuBarReadButtons();
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
	iRet = __xuiMenuBarSendButtonTransitions(pDemo, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons, iPressed, iReleased);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_LEFT, XUI_KEY_LEFT);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_RIGHT, XUI_KEY_RIGHT);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_UP, XUI_KEY_UP);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_DOWN, XUI_KEY_DOWN);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_ENTER, XUI_KEY_ENTER);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_SPACE, XUI_KEY_SPACE);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_F10, XUI_KEY_F10);
	if ( iRet == XUI_OK ) iRet = __xuiMenuBarSendKey(pDemo, XGE_KEY_MENU, XUI_KEY_CONTEXT_MENU);
	if ( iRet != XUI_OK ) return iRet;
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiMenuBarDispatchMove(xui_menubar_demo_t* pDemo, float fX, float fY)
{
	int iRet = xuiInputPointerMove(pDemo->pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	return xuiUpdate(pDemo->pContext, 0.016f);
}

static int __xuiMenuBarDispatchClick(xui_menubar_demo_t* pDemo, float fX, float fY)
{
	int iRet;

	iRet = __xuiMenuBarDispatchMove(pDemo, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pDemo->pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiMenuBarExerciseClickItem(xui_menubar_demo_t* pDemo, int iIndex)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pDemo->pMenuBar);
	xui_rect_t tItem = xuiMenuBarGetItemRect(pDemo->pMenuBar, iIndex);
	return __xuiMenuBarDispatchClick(pDemo, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

static int __xuiMenuBarExerciseMoveItem(xui_menubar_demo_t* pDemo, int iIndex)
{
	xui_rect_t tWorld = xuiWidgetGetWorldRect(pDemo->pMenuBar);
	xui_rect_t tItem = xuiMenuBarGetItemRect(pDemo->pMenuBar, iIndex);
	return __xuiMenuBarDispatchMove(pDemo, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

static void __xuiMenuBarRunChecks(xui_menubar_demo_t* pDemo, int bExerciseInput)
{
	int iBefore;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pMenuBar != NULL) && (pDemo->pFileMenu != NULL) && (xuiMenuBarGetItemCount(pDemo->pMenuBar) == 4);
	pDemo->bLayoutOK = (xuiMenuBarGetItemRect(pDemo->pMenuBar, 0).fW > 20.0f) && (xuiMenuBarGetItemRect(pDemo->pMenuBar, 1).fX > xuiMenuBarGetItemRect(pDemo->pMenuBar, 0).fX);
	if ( bExerciseInput && !pDemo->bExerciseDone && pDemo->bCreateOK && pDemo->bLayoutOK ) {
		(void)__xuiMenuBarExerciseClickItem(pDemo, 0);
		pDemo->bOpenOK = xuiMenuBarIsOpen(pDemo->pMenuBar) && xuiMenuIsOpen(pDemo->pFileMenu) && (xuiMenuBarGetOpenIndex(pDemo->pMenuBar) == 0);
		(void)__xuiMenuBarExerciseMoveItem(pDemo, 1);
		pDemo->bSwitchOK = xuiMenuIsOpen(pDemo->pEditMenu) && !xuiMenuIsOpen(pDemo->pFileMenu) && (xuiMenuBarGetOpenIndex(pDemo->pMenuBar) == 1);
		(void)xuiInputKeyDown(pDemo->pContext, 'V', XUI_MOD_ALT);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bMnemonicOK = xuiMenuIsOpen(pDemo->pViewMenu) && (xuiMenuBarGetOpenIndex(pDemo->pMenuBar) == 2);
		iBefore = pDemo->iSelectCount;
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		(void)xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, 0);
		(void)xuiDispatchPendingEvents(pDemo->pContext);
		pDemo->bSelectOK = (pDemo->iSelectCount > iBefore) && (pDemo->iLastValue == 32);
		(void)xuiUpdate(pDemo->pContext, 0.016f);
		pDemo->bCloseOK = !xuiMenuBarIsOpen(pDemo->pMenuBar);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bOpenOK = 1;
		pDemo->bSwitchOK = 1;
		pDemo->bMnemonicOK = 1;
		pDemo->bSelectOK = 1;
		pDemo->bCloseOK = 1;
	}
}

static int __xuiMenuBarCreateAssets(xui_menubar_demo_t* pDemo)
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
	sFontPath = __xuiMenuBarFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiMenuBarCreateUi(pDemo);
}

static void __xuiMenuBarDestroyAssets(xui_menubar_demo_t* pDemo)
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

static int __xuiMenuBarFrame(void* pUser)
{
	xui_menubar_demo_t* pDemo;
	xui_render_stats_t tStats;
	xui_cache_stats_t tCacheStats;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	int iRet;
	int bAutoRun;

	pDemo = (xui_menubar_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiMenuBarHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiMenuBarRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(232, 240, 249, 255));
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
		printf("xui_menubar final-summary frames=%d create=%d layout=%d open=%d switch=%d mnemonic=%d select=%d close=%d callbacks=%d last=%d updatedCaches=%d drawnCaches=%d cacheSurfaces=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bOpenOK, pDemo->bSwitchOK,
			pDemo->bMnemonicOK, pDemo->bSelectOK, pDemo->bCloseOK, pDemo->iSelectCount, pDemo->iLastValue,
			tStats.iUpdatedCaches, tStats.iDrawnCaches, tCacheStats.iSurfaceCount);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_menubar_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiMenuBarParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiMenuBarUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI MenuBar";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_menubar: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiMenuBarCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_menubar: create assets failed: %d\n", iRet);
		__xuiMenuBarDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiMenuBarFrame, &tDemo);
	__xuiMenuBarDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bOpenOK &&
		tDemo.bSwitchOK && tDemo.bMnemonicOK && tDemo.bSelectOK && tDemo.bCloseOK) ? 0 : 1;
}
