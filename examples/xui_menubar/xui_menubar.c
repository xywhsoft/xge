#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pFrame;
	xge_xui_widget pMenuBar;
	xge_xui_widget pToolBar;
	xge_xui_widget pClient;
	xge_xui_widget pStatusBar;
	xge_xui_widget pClientText;
	xge_xui_panel_t tClientPanel;
	xge_xui_label_t tClientText;
	xge_xui_menubar_t tMenuBar;
	xge_xui_toolbar_t tToolBar;
	xge_xui_status_bar_t tStatusBar;
	xge_xui_menu_t tFileMenu;
	xge_xui_menu_t tEditMenu;
	xge_xui_menu_t tViewMenu;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iMenuSelectCount;
	int iToolSelectCount;
	int iStatusSelectCount;
	int iLastMenuValue;
	int iLastToolIndex;
	int iLastStatusIndex;
	int iReadyIndex;
	int iProgressIndex;
	int bInitOK;
	int bMenuOpenOK;
	int bMenuSameCloseOK;
	int bMenuBlankCloseOK;
	int bToolbarOK;
	int bStatusOK;
} app_state_t;

static xge_xui_menu_item_t g_arrFileItems[] = {
	{ "New", "Ctrl+N", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 10, 1, NULL, NULL },
	{ "Open", "Ctrl+O", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 20, 1, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Exit", "Alt+F4", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_DANGER, 99, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrEditItems[] = {
	{ "Undo", "Ctrl+Z", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 30, 0, NULL, NULL },
	{ "Redo", "Ctrl+Y", XGE_XUI_MENU_ITEM_NORMAL, XGE_XUI_MENU_ITEM_ENABLED, 31, 0, NULL, NULL },
	{ NULL, NULL, XGE_XUI_MENU_ITEM_SEPARATOR, 0, 0, 0, NULL, NULL },
	{ "Snap to pixel", NULL, XGE_XUI_MENU_ITEM_CHECK, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_CHECKED, 32, 0, NULL, NULL }
};

static xge_xui_menu_item_t g_arrViewItems[] = {
	{ "Show toolbar", NULL, XGE_XUI_MENU_ITEM_CHECK, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_CHECKED, 40, 0, NULL, NULL },
	{ "Show status bar", NULL, XGE_XUI_MENU_ITEM_CHECK, XGE_XUI_MENU_ITEM_ENABLED | XGE_XUI_MENU_ITEM_CHECKED, 41, 0, NULL, NULL }
};

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_menubar font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MenuSelect(xge_xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iMenuSelectCount++;
		pApp->iLastMenuValue = iValue;
	}
}

static void ToolSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iToolSelectCount++;
		pApp->iLastToolIndex = iIndex;
	}
}

static void StatusSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iStatusSelectCount++;
		pApp->iLastStatusIndex = iIndex;
	}
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKey(xge_event_t* pEvent, int iKey, int iModifiers)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iModifiers;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fW;
	float fH;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return;
	}
	fW = (float)xgeGetWidth();
	fH = (float)xgeGetHeight();
	if ( fW < 760.0f ) {
		fW = 760.0f;
	}
	if ( fH < 500.0f ) {
		fH = 500.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	const char* arrToolText[] = { "New", "Open", "", "Pin", "Run" };
	int arrToolType[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_BUTTON };
	xge_xui_menubar_item_t arrMenuItems[3];

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	LayoutRoot(pApp);
	pApp->pFrame = xgeXuiWidgetCreate();
	pApp->pMenuBar = xgeXuiWidgetCreate();
	pApp->pToolBar = xgeXuiWidgetCreate();
	pApp->pClient = xgeXuiWidgetCreate();
	pApp->pStatusBar = xgeXuiWidgetCreate();
	pApp->pClientText = xgeXuiWidgetCreate();
	if ( (pApp->pFrame == NULL) || (pApp->pMenuBar == NULL) || (pApp->pToolBar == NULL) || (pApp->pClient == NULL) || (pApp->pStatusBar == NULL) || (pApp->pClientText == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pFrame, (xge_rect_t){ 26.0f, 24.0f, 708.0f, 438.0f });
	xgeXuiWidgetSetLayout(pApp->pFrame, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetGap(pApp->pFrame, 0.0f);
	xgeXuiWidgetSetBackground(pApp->pFrame, xgeXuiGetChromeStyle(&pApp->tXui)->tWindow.iFrameBackground);
	xgeXuiWidgetSetBorder(pApp->pFrame, 1.0f, xgeXuiGetChromeStyle(&pApp->tXui)->tWindow.iBorder);
	xgeXuiWidgetAdd(pRoot, pApp->pFrame);

	xgeXuiWidgetSetSize(pApp->pMenuBar, xgeXuiSizePercent(100.0f), xgeXuiSizePx(26.0f));
	xgeXuiMenuBarInit(&pApp->tMenuBar, &pApp->tXui, pApp->pMenuBar);
	xgeXuiMenuBarSetFont(&pApp->tMenuBar, pFont);
	xgeXuiMenuBarSetSelect(&pApp->tMenuBar, MenuSelect, pApp);
	xgeXuiWidgetAdd(pApp->pFrame, pApp->pMenuBar);

	xgeXuiWidgetSetSize(pApp->pToolBar, xgeXuiSizePercent(100.0f), xgeXuiSizePx(32.0f));
	xgeXuiToolbarInit(&pApp->tToolBar, &pApp->tXui, pApp->pToolBar);
	xgeXuiToolbarSetFont(&pApp->tToolBar, pFont);
	xgeXuiToolbarSetItems(&pApp->tToolBar, arrToolText, arrToolType, 5);
	xgeXuiToolbarSetItemSize(&pApp->tToolBar, 64.0f, 24.0f, 10.0f);
	xgeXuiToolbarSetItemGroup(&pApp->tToolBar, 3, 1);
	xgeXuiToolbarSetSelect(&pApp->tToolBar, ToolSelect, pApp);
	xgeXuiWidgetAdd(pApp->pFrame, pApp->pToolBar);

	xgeXuiWidgetSetSize(pApp->pClient, xgeXuiSizePercent(100.0f), xgeXuiSizeGrow(1.0f));
	xgeXuiWidgetSetPaddingPx(pApp->pClient, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetSetBackground(pApp->pClient, xgeXuiGetChromeStyle(&pApp->tXui)->tWindow.iClientBackground);
	xgeXuiPanelInit(&pApp->tClientPanel, pApp->pClient);
	xgeXuiPanelSetTitle(&pApp->tClientPanel, pFont, "Client Area");
	xgeXuiWidgetAdd(pApp->pFrame, pApp->pClient);

	xgeXuiWidgetSetRect(pApp->pClientText, (xge_rect_t){ 32.0f, 54.0f, 500.0f, 32.0f });
	xgeXuiLabelInit(&pApp->tClientText, pApp->pClientText, pFont, "Window chrome uses one palette; client content uses another.");
	xgeXuiLabelSetColor(&pApp->tClientText, xgeXuiGetChromeStyle(&pApp->tXui)->tBarColors.iText);
	xgeXuiWidgetAdd(pApp->pClient, pApp->pClientText);

	xgeXuiWidgetSetSize(pApp->pStatusBar, xgeXuiSizePercent(100.0f), xgeXuiSizePx(26.0f));
	xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusBar);
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, pFont);
	xgeXuiStatusBarSetSelect(&pApp->tStatusBar, StatusSelect, pApp);
	pApp->iReadyIndex = xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 80.0f, 1);
	pApp->iProgressIndex = xgeXuiStatusBarAddProgress(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 0.0f, 100.0f, 62.0f, 120.0f);
	xgeXuiStatusBarAddFlexibleSpacer(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 1.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "XUI", 58.0f, 0);
	xgeXuiWidgetAdd(pApp->pFrame, pApp->pStatusBar);

	xgeXuiMenuInit(&pApp->tFileMenu, &pApp->tXui);
	xgeXuiMenuInit(&pApp->tEditMenu, &pApp->tXui);
	xgeXuiMenuInit(&pApp->tViewMenu, &pApp->tXui);
	xgeXuiMenuSetItems(&pApp->tFileMenu, g_arrFileItems, (int)(sizeof(g_arrFileItems) / sizeof(g_arrFileItems[0])));
	xgeXuiMenuSetItems(&pApp->tEditMenu, g_arrEditItems, (int)(sizeof(g_arrEditItems) / sizeof(g_arrEditItems[0])));
	xgeXuiMenuSetItems(&pApp->tViewMenu, g_arrViewItems, (int)(sizeof(g_arrViewItems) / sizeof(g_arrViewItems[0])));
	xgeXuiMenuSetSelect(&pApp->tFileMenu, MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tEditMenu, MenuSelect, pApp);
	xgeXuiMenuSetSelect(&pApp->tViewMenu, MenuSelect, pApp);
	memset(arrMenuItems, 0, sizeof(arrMenuItems));
	arrMenuItems[0].sText = "&File";
	arrMenuItems[0].iState = XGE_XUI_MENUBAR_ITEM_ENABLED;
	arrMenuItems[0].iValue = 1;
	arrMenuItems[0].pMenu = &pApp->tFileMenu;
	arrMenuItems[1].sText = "&Edit";
	arrMenuItems[1].iState = XGE_XUI_MENUBAR_ITEM_ENABLED;
	arrMenuItems[1].iValue = 2;
	arrMenuItems[1].pMenu = &pApp->tEditMenu;
	arrMenuItems[2].sText = "&View";
	arrMenuItems[2].iState = XGE_XUI_MENUBAR_ITEM_ENABLED;
	arrMenuItems[2].iValue = 3;
	arrMenuItems[2].pMenu = &pApp->tViewMenu;
	xgeXuiMenuBarSetItems(&pApp->tMenuBar, arrMenuItems, 3);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (pApp->tMenuBar.iItemCount == 3) && (pApp->tToolBar.iItemCount == 5) && (pApp->tStatusBar.iItemCount == 4);

	MakeKey(&tEvent, 'F', XGE_KEY_MOD_ALT);
	xgeXuiMenuBarEvent(&pApp->tMenuBar, &tEvent);
	pApp->bMenuOpenOK = xgeXuiMenuIsOpen(&pApp->tFileMenu) != 0;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tMenuBar.arrItems[0].tRect.fX + 4.0f, pApp->tMenuBar.arrItems[0].tRect.fY + 4.0f);
	pApp->bMenuSameCloseOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiMenuIsOpen(&pApp->tFileMenu) == 0);

	MakeKey(&tEvent, 'F', XGE_KEY_MOD_ALT);
	xgeXuiMenuBarEvent(&pApp->tMenuBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pMenuBar->tRect.fX + pApp->pMenuBar->tRect.fW - 4.0f, pApp->pMenuBar->tRect.fY + pApp->pMenuBar->tRect.fH * 0.5f);
	pApp->bMenuBlankCloseOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiMenuIsOpen(&pApp->tFileMenu) == 0);

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tToolBar.arrItems[3].tRect.fX + 4.0f, pApp->tToolBar.arrItems[3].tRect.fY + 4.0f);
	xgeXuiToolbarEvent(&pApp->tToolBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tToolBar.arrItems[3].tRect.fX + 4.0f, pApp->tToolBar.arrItems[3].tRect.fY + 4.0f);
	pApp->bToolbarOK = (xgeXuiToolbarEvent(&pApp->tToolBar, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiToolbarGetItemChecked(&pApp->tToolBar, 3) != 0);

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fX + 4.0f, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fY + 4.0f);
	xgeXuiStatusBarEvent(&pApp->tStatusBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fX + 4.0f, pApp->tStatusBar.arrItems[pApp->iReadyIndex].tRect.fY + 4.0f);
	pApp->bStatusOK = (xgeXuiStatusBarEvent(&pApp->tStatusBar, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->iStatusSelectCount == 1);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiMenuUnit(&pApp->tViewMenu);
	xgeXuiMenuUnit(&pApp->tEditMenu);
	xgeXuiMenuUnit(&pApp->tFileMenu);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_menubar final-summary frames=%d init=%d menu=%d sameClose=%d blankClose=%d toolbar=%d status=%d menuSelect=%d toolSelect=%d statusSelect=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bMenuOpenOK,
			pApp->bMenuSameCloseOK,
			pApp->bMenuBlankCloseOK,
			pApp->bToolbarOK,
			pApp->bStatusOK,
			pApp->iMenuSelectCount,
			pApp->iToolSelectCount,
			pApp->iStatusSelectCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(230, 240, 248, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MENUBAR_FRAMES"), 0);
	tApp.iLastMenuValue = -1;
	tApp.iLastToolIndex = -1;
	tApp.iLastStatusIndex = -1;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 500;
	tDesc.sTitle = "XUI MenuBar";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bMenuOpenOK && tApp.bMenuSameCloseOK && tApp.bMenuBlankCloseOK && tApp.bToolbarOK && tApp.bStatusOK) ? 0 : 3;
}
