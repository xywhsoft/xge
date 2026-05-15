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
	int bLoadOK;
	int bMenuOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[22,18,22,18],\"gap\":12,\"background\":\"#E8F1F8FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#26384AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"owner\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":220,\"height\":34,\"padding\":[10,6,10,6],\"background\":\"#E8F3FCFF\",\"borderWidth\":1,\"borderColor\":\"#4B8AC4FF\",\"textColor\":\"#284868FF\"}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"title\",\"text\":\"Menu XSON\"},"
"{\"type\":\"label\",\"id\":\"owner0\",\"style\":\"owner\",\"text\":\"menu owner\"},"
"{\"type\":\"menu\",\"id\":\"menu0\",\"owner\":\"owner0\",\"open\":true,\"items\":["
"\"Open\","
"{\"text\":\"Save\",\"shortcut\":\"Ctrl+S\",\"value\":20,\"icon\":1},"
"{\"type\":\"separator\"},"
"{\"text\":\"Show grid\",\"type\":\"check\",\"checked\":true,\"value\":30},"
"{\"text\":\"Disabled\",\"enabled\":false,\"value\":40},"
"{\"text\":\"Delete\",\"danger\":true,\"shortcut\":\"Del\",\"value\":50}"
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

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_menu_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "body", &pApp->tFont);
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_menu_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	pApp->bLoadOK = 1;
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bMenuOK =
		(pApp->tPage.iMenuCount == 1) &&
		(pApp->tPage.arrMenu[0] != NULL) &&
		(pApp->tPage.arrMenu[0]->iItemCount == 6) &&
		xgeXuiMenuIsOpen(pApp->tPage.arrMenu[0]);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	RunChecks(pApp);
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_menu_xson final-summary frames=%d load=%d menu=%d\n", pApp->iFrameCount, pApp->bLoadOK, pApp->bMenuOK);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 241, 248, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MENU_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 460;
	tDesc.sTitle = "XUI Menu XSON";
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
	return (iExitCode == XGE_OK && tApp.bLoadOK && tApp.bMenuOK) ? 0 : 3;
}
