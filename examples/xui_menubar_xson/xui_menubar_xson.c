#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bMenuOpenOK;
	int bMenuSameCloseOK;
	int bMenuBlankCloseOK;
	int bMenuSelectOK;
	int bToolbarOK;
	int bStatusOK;
	int iMenuSelectValue;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[24,20,24,20],\"gap\":0,\"background\":\"#E5EBF4FF\"},"
"\"frame\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"gap\":0,\"background\":\"#DDE7F2FF\",\"borderColor\":\"#7DA8CFFF\",\"borderWidth\":1},"
"\"menubar\":{\"type\":\"menubar\",\"width\":\"100%\",\"height\":26,\"font\":\"@fonts.body\"},"
"\"toolbar\":{\"type\":\"toolbar\",\"width\":\"100%\",\"height\":32,\"font\":\"@fonts.body\",\"itemWidth\":64,\"itemHeight\":24,\"separatorSize\":10},"
"\"client\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"1*\",\"padding\":[18,18,18,18],\"gap\":8,\"background\":\"#EEF4FAFF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":28,\"textColor\":\"#263040FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"status\":{\"type\":\"statusBar\",\"width\":\"100%\",\"height\":26,\"font\":\"@fonts.body\",\"barHeight\":26,\"itemGap\":6,\"itemPadding\":8}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"column\",\"style\":\"frame\",\"children\":["
"{\"type\":\"menubar\",\"style\":\"menubar\",\"items\":["
"{\"text\":\"&File\",\"value\":1,\"items\":["
"{\"text\":\"New\",\"shortcut\":\"Ctrl+N\",\"value\":10},"
"{\"text\":\"Open\",\"shortcut\":\"Ctrl+O\",\"value\":20},"
"{\"separator\":true},"
"{\"text\":\"Exit\",\"shortcut\":\"Alt+F4\",\"danger\":true,\"value\":99}"
"]},"
"{\"text\":\"&Edit\",\"value\":2,\"items\":["
"{\"text\":\"Undo\",\"shortcut\":\"Ctrl+Z\",\"value\":30},"
"{\"text\":\"Redo\",\"shortcut\":\"Ctrl+Y\",\"enabled\":false,\"value\":31},"
"{\"separator\":true},"
"{\"text\":\"Snap to pixel\",\"type\":\"check\",\"checked\":true,\"value\":32}"
"]},"
"{\"text\":\"&View\",\"value\":3,\"items\":["
"{\"text\":\"Show toolbar\",\"type\":\"check\",\"checked\":true,\"value\":40},"
"{\"text\":\"Show status bar\",\"type\":\"check\",\"checked\":true,\"value\":41}"
"]}"
"]},"
"{\"type\":\"toolbar\",\"style\":\"toolbar\",\"items\":["
"{\"text\":\"New\"},"
"{\"text\":\"Open\"},"
"{\"type\":\"separator\"},"
"{\"text\":\"Pin\",\"type\":\"toggle\",\"checked\":true},"
"{\"text\":\"Run\"},"
"{\"text\":\"Disabled\",\"enabled\":false}"
"]},"
"{\"type\":\"column\",\"style\":\"client\",\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"XSON-created MenuBar, Toolbar and StatusBar share the chrome palette.\"},"
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Alt+F opens File. Toolbar toggle and status sections are loaded from XSON.\"}"
"]},"
"{\"type\":\"statusBar\",\"style\":\"status\",\"items\":["
"{\"text\":\"Ready\",\"width\":80,\"clickable\":true},"
"{\"type\":\"progress\",\"min\":0,\"max\":100,\"value\":62,\"width\":120},"
"{\"type\":\"spacer\",\"width\":260},"
"{\"text\":\"XUI\",\"section\":\"right\",\"width\":58}"
"]}"
"]}"
"]}}";

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_menubar_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeKey(xge_event_t* pEvent, int iKey, int iModifiers)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iModifiers;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void OnMenuSelect(xge_xui_widget pOwner, int iIndex, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pOwner;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iMenuSelectValue = iValue;
	}
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_menubar_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	if ( pApp->tPage.iMenuBarCount > 0 ) {
		xgeXuiMenuBarSetSelect(&pApp->tPage.arrMenuBar[0], OnMenuSelect, pApp);
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 760.0f ) {
		fRootW = 760.0f;
	}
	if ( fRootH < 500.0f ) {
		fRootH = 500.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_menubar pMenuBar;
	xge_xui_toolbar pToolbar;
	xge_xui_status_bar pStatusBar;
	xge_xui_menu pMenu;
	xge_rect_t tItem;
	float fClickX;
	float fClickY;

	pApp->bCreateOK = (pApp->tPage.iMenuBarCount == 1) && (pApp->tPage.iToolbarCount == 1) && (pApp->tPage.iStatusBarCount == 1) && (pApp->tPage.iMenuCount == 3);
	if ( !pApp->bCreateOK ) {
		return;
	}
	pMenuBar = &pApp->tPage.arrMenuBar[0];
	pToolbar = &pApp->tPage.arrToolbar[0];
	pStatusBar = &pApp->tPage.arrStatusBar[0];
	MakeKey(&tEvent, 'F', XGE_KEY_MOD_ALT);
	xgeXuiMenuBarEvent(pMenuBar, &tEvent);
	pApp->bMenuOpenOK = (pMenuBar->arrItems[0].pMenu != NULL) && (xgeXuiMenuIsOpen(pMenuBar->arrItems[0].pMenu) != 0);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pMenuBar->arrItems[0].tRect.fX + 4.0f, pMenuBar->arrItems[0].tRect.fY + 4.0f);
	pApp->bMenuSameCloseOK = (pMenuBar->arrItems[0].pMenu != NULL) && (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiMenuIsOpen(pMenuBar->arrItems[0].pMenu) == 0);
	MakeKey(&tEvent, 'F', XGE_KEY_MOD_ALT);
	xgeXuiMenuBarEvent(pMenuBar, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pMenuBar->pWidget->tRect.fX + pMenuBar->pWidget->tRect.fW - 4.0f, pMenuBar->pWidget->tRect.fY + pMenuBar->pWidget->tRect.fH * 0.5f);
	pApp->bMenuBlankCloseOK = (pMenuBar->arrItems[0].pMenu != NULL) && (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiMenuIsOpen(pMenuBar->arrItems[0].pMenu) == 0);
	pApp->iMenuSelectValue = 0;
	MakeKey(&tEvent, 'F', XGE_KEY_MOD_ALT);
	xgeXuiMenuBarEvent(pMenuBar, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pMenu = pMenuBar->arrItems[0].pMenu;
	if ( pMenu != NULL && pMenu->pContentWidget != NULL && pMenu->iItemCount > 1 ) {
		tItem = pMenu->arrItemRect[1];
		fClickX = pMenu->pContentWidget->tRect.fX + tItem.fX + tItem.fW * 0.5f;
		fClickY = pMenu->pContentWidget->tRect.fY + tItem.fY + tItem.fH * 0.5f;
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fClickX, fClickY);
		pApp->bMenuSelectOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (pApp->iMenuSelectValue == 20) && (xgeXuiMenuIsOpen(pMenu) == 0);
	}
	pApp->bToolbarOK = (pToolbar->iItemCount == 6) && (xgeXuiToolbarGetItemChecked(pToolbar, 3) != 0) && (pToolbar->arrItems[5].bEnabled == 0);
	pApp->bStatusOK = (pStatusBar->iItemCount == 4) && (pStatusBar->arrItems[1].iType == XGE_XUI_STATUS_BAR_ITEM_PROGRESS);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	if ( pApp->iFrameLimit > 0 ) {
		RunChecks(pApp);
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
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
	if ( pApp->iFrameLimit > 0 ) {
		RunChecks(pApp);
	}
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_menubar_xson final-summary frames=%d create=%d menu=%d sameClose=%d blankClose=%d select=%d selectValue=%d toolbar=%d status=%d menubars=%d menus=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bMenuOpenOK, pApp->bMenuSameCloseOK, pApp->bMenuBlankCloseOK, pApp->bMenuSelectOK, pApp->iMenuSelectValue, pApp->bToolbarOK, pApp->bStatusOK, pApp->tPage.iMenuBarCount, pApp->tPage.iMenuCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MENUBAR_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 500;
	tDesc.sTitle = "XUI MenuBar XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bMenuOpenOK && tApp.bMenuSameCloseOK && tApp.bMenuBlankCloseOK && tApp.bMenuSelectOK && tApp.bToolbarOK && tApp.bStatusOK) ? 0 : 3;
}
