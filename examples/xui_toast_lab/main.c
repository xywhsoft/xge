#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_toast_t tToast;
	xge_xui_widget pToastWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bCloseOK;
	int bExpireOK;
} app_state_t;

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
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 12.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-toast-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-toast-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	pApp->pToastWidget = xgeXuiWidgetCreate();
	if ( pApp->pToastWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pToastWidget, (xge_rect_t){ 18.0f, 18.0f, 444.0f, 260.0f });
	xgeXuiToastInit(&pApp->tToast, &pApp->tXui, pApp->pToastWidget, pFont);
	xgeXuiToastSetMetrics(&pApp->tToast, 270.0f, 58.0f, 8.0f);
	xgeXuiToastSetPlacement(&pApp->tToast, XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT);
	xgeXuiWidgetAdd(pRoot, pApp->pToastWidget);
	return XGE_OK;
}

static void RunInitialChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiToastShow(&pApp->tToast, XGE_XUI_TOAST_TYPE_INFO, "Info", "Project indexed.", 10.0f);
	xgeXuiToastShow(&pApp->tToast, XGE_XUI_TOAST_TYPE_SUCCESS, "Saved", "Theme tokens updated.", 10.0f);
	xgeXuiToastShow(&pApp->tToast, XGE_XUI_TOAST_TYPE_WARNING, "Warning", "One field needs review.", 0.08f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = xgeXuiToastGetCount(&pApp->tToast) == 3 && pApp->tToast.iShowCount == 3;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tToast.arrItems[0].tCloseRect.fX + 3.0f, pApp->tToast.arrItems[0].tCloseRect.fY + 3.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bCloseOK = xgeXuiToastGetCount(&pApp->tToast) == 2 && pApp->tToast.iCloseCount == 1;
	xgeXuiUpdate(&pApp->tXui, 0.1f);
	pApp->bExpireOK = pApp->tToast.iExpireCount >= 1 && xgeXuiToastGetCount(&pApp->tToast) == 1;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunInitialChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
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
		printf(
			"xui-toast-lab final-summary frames=%d init=%d close=%d expire=%d count=%d show=%d closed=%d expired=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bCloseOK,
			pApp->bExpireOK,
			xgeXuiToastGetCount(&pApp->tToast),
			pApp->tToast.iShowCount,
			pApp->tToast.iCloseCount,
			pApp->tToast.iExpireCount);
		printf("xui-toast-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(238, 248, 255, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TOAST_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 480;
	tDesc.iHeight = 300;
	tDesc.sTitle = "XGE XUI Toast Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bCloseOK && tApp.bExpireOK) ? 0 : 3;
}
