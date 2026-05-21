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
	int bCreateOK;
	int bStateOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":920,\"height\":620,\"background\":\"#EAF1F8FF\"},"
"\"dock\":{\"type\":\"dockLayout\",\"x\":18,\"y\":18,\"width\":884,\"height\":584,\"background\":\"#F5FAFDFF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"textColor\":\"#253346FF\",\"textVAlign\":\"middle\",\"width\":260,\"height\":28}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"dockLayout\",\"id\":\"mainDock\",\"style\":\"dock\","
"\"regions\":{\"left\":0.22,\"right\":0.20,\"bottom\":0.25},"
"\"dockWindows\":["
"{\"id\":\"document\",\"title\":\"Document\",\"region\":\"document\",\"side\":\"fill\",\"closable\":false,\"children\":["
"{\"type\":\"label\",\"id\":\"docLabel\",\"style\":\"label\",\"x\":22,\"y\":22,\"text\":\"Document client from XSON\"}"
"]},"
"{\"id\":\"output\",\"title\":\"Output\",\"region\":\"bottom\",\"side\":\"fill\",\"portion\":0.25,\"children\":["
"{\"type\":\"label\",\"id\":\"outputLabel\",\"style\":\"label\",\"x\":18,\"y\":18,\"text\":\"Output pane\"}"
"]},"
"{\"id\":\"toolbox\",\"title\":\"Toolbox\",\"region\":\"left\",\"side\":\"fill\",\"portion\":0.22,\"children\":["
"{\"type\":\"label\",\"id\":\"toolLabel\",\"style\":\"label\",\"x\":14,\"y\":14,\"text\":\"Toolbox\"}"
"]}," 
"{\"id\":\"bookmarks\",\"title\":\"Bookmarks\",\"region\":\"left\",\"side\":\"fill\",\"state\":\"autoHide\",\"children\":["
"{\"type\":\"label\",\"id\":\"bookmarksLabel\",\"style\":\"label\",\"x\":14,\"y\":14,\"text\":\"Auto-hide strip item\"}"
"]},"
"{\"id\":\"inspector\",\"title\":\"Inspector\",\"region\":\"right\",\"side\":\"fill\",\"portion\":0.20,\"dockable\":false,\"children\":["
"{\"type\":\"label\",\"id\":\"inspectorLabel\",\"style\":\"label\",\"x\":14,\"y\":14,\"text\":\"Inspector, non-dockable\"}"
"]},"
"{\"id\":\"preview\",\"title\":\"Preview\",\"state\":\"floating\",\"rect\":[560,74,260,170],\"children\":["
"{\"type\":\"label\",\"id\":\"previewLabel\",\"style\":\"label\",\"x\":18,\"y\":18,\"text\":\"Floating dockwindow\"}"
"]},"
"{\"id\":\"search\",\"title\":\"Search\",\"state\":\"hidden\",\"children\":["
"{\"type\":\"label\",\"id\":\"searchLabel\",\"style\":\"label\",\"x\":12,\"y\":12,\"text\":\"Hidden dockwindow\"}"
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
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_dockpanel_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_dock_layout pLayout;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_dockpanel_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	pLayout = (pApp->tPage.iDockLayoutCount > 0) ? pApp->tPage.arrDockLayout[0] : NULL;
	pApp->bCreateOK =
		(pLayout != NULL) &&
		(pApp->tPage.iDockWindowCount == 7) &&
		(xgeXuiPageFind(&pApp->tPage, "docLabel") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "previewLabel") != NULL);
	pApp->bStateOK = pApp->bCreateOK &&
		(pApp->tPage.arrDockWindow[0]->iState == XGE_XUI_DOCK_WINDOW_DOCKED) &&
		(pApp->tPage.arrDockWindow[1]->iState == XGE_XUI_DOCK_WINDOW_DOCKED) &&
		(pApp->tPage.arrDockWindow[2]->iState == XGE_XUI_DOCK_WINDOW_DOCKED) &&
		(pApp->tPage.arrDockWindow[3]->iState == XGE_XUI_DOCK_WINDOW_AUTO_HIDE) &&
		(pApp->tPage.arrDockWindow[3]->iAutoHideRegion == XGE_XUI_DOCK_REGION_LEFT) &&
		(pApp->tPage.arrDockWindow[4]->iState == XGE_XUI_DOCK_WINDOW_DOCKED) &&
		(pApp->tPage.arrDockWindow[4]->bDockable == 0) &&
		(pApp->tPage.arrDockWindow[5]->iState == XGE_XUI_DOCK_WINDOW_FLOATING) &&
		(pApp->tPage.arrDockWindow[6]->iState == XGE_XUI_DOCK_WINDOW_HIDDEN) &&
		(pLayout->arrRegions[XGE_XUI_DOCK_REGION_LEFT].bVisible != 0) &&
		(pLayout->arrRegions[XGE_XUI_DOCK_REGION_RIGHT].bVisible != 0) &&
		(pLayout->arrRegions[XGE_XUI_DOCK_REGION_BOTTOM].bVisible != 0);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_dockpanel_xson final-summary frames=%d create=%d state=%d layouts=%d windows=%d labels=%d\n",
			pApp->iFrameCount, pApp->bCreateOK, pApp->bStateOK, pApp->tPage.iDockLayoutCount, pApp->tPage.iDockWindowCount, pApp->tPage.iLabelCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(234, 241, 248, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DOCKPANEL_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 920;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI DockPanel XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bStateOK) ? 0 : 3;
}
