#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_toast pToast;
	int iFrameLimit;
	int iFrameCount;
	int bLoadOK;
	int bItemsOK;
	int bPlacementOK;
	int bStyleOK;
} app_state_t;

static const char g_sPage[] =
	"{"
	"\"xui\":1,"
	"\"tokens\":{\"spacing\":{\"tw\":270,\"th\":58,\"gap\":8},\"colors\":{\"bg\":\"#F7FCFFF5\",\"border\":\"#81AECF\",\"text\":\"#1F3A52\",\"muted\":\"#5C7082\",\"info\":\"#4E9FDC\",\"success\":\"#2BB896\",\"warning\":\"#F4BB44\",\"error\":\"#E05C5C\"}},"
	"\"styles\":{\"toast\":{\"placement\":\"topRight\",\"toastWidth\":\"@spacing.tw\",\"toastHeight\":\"@spacing.th\",\"spacing\":\"@spacing.gap\",\"backgroundColor\":\"@colors.bg\",\"borderColor\":\"@colors.border\",\"textColor\":\"@colors.text\",\"mutedTextColor\":\"@colors.muted\",\"infoColor\":\"@colors.info\",\"successColor\":\"@colors.success\",\"warningColor\":\"@colors.warning\",\"errorColor\":\"@colors.error\"}},"
	"\"tree\":{\"type\":\"toast\",\"id\":\"notify\",\"style\":\"toast\",\"width\":444,\"height\":260,\"items\":[{\"type\":\"info\",\"title\":\"Info\",\"message\":\"Project indexed.\",\"duration\":10},{\"type\":\"success\",\"title\":\"Saved\",\"message\":\"Theme tokens updated.\",\"duration\":10},{\"type\":\"warning\",\"title\":\"Warning\",\"message\":\"One field needs review.\",\"duration\":10}]}"
	"}";

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;
	xge_xui_widget pRoot;

	pApp = (app_state_t*)pScene->pUser;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->bLoadOK = xgeXuiPageLoadMemory(&pApp->tXui, g_sPage, (int)strlen(g_sPage), NULL, &pApp->tPage) == XGE_OK;
	pRoot = xgeXuiPageRoot(&pApp->tPage);
	pApp->pToast = (pRoot != NULL) ? (xge_xui_toast)pRoot->pUser : NULL;
	pApp->bItemsOK = pApp->pToast != NULL && xgeXuiToastGetCount(pApp->pToast) == 3 && pApp->pToast->arrItems[1].iType == XGE_XUI_TOAST_TYPE_SUCCESS;
	pApp->bPlacementOK = pApp->pToast != NULL && pApp->pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
	pApp->bStyleOK = pApp->pToast != NULL && pApp->pToast->fToastWidth == 270.0f && pApp->pToast->iSuccessColor == XGE_COLOR_RGBA(0x2B, 0xB8, 0x96, 0xFF);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
	xgeXuiUnit(&pApp->tXui);
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
			"xui-xson-toast-lab final-summary frames=%d load=%d items=%d placement=%d style=%d count=%d\n",
			pApp->iFrameCount,
			pApp->bLoadOK,
			pApp->bItemsOK,
			pApp->bPlacementOK,
			pApp->bStyleOK,
			pApp->pToast != NULL ? xgeXuiToastGetCount(pApp->pToast) : 0);
		printf("xui-xson-toast-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_XSON_TOAST_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 480;
	tDesc.iHeight = 300;
	tDesc.sTitle = "XGE XUI XSON Toast Lab";
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
	return (iExitCode == XGE_OK && tApp.bLoadOK && tApp.bItemsOK && tApp.bPlacementOK && tApp.bStyleOK) ? 0 : 3;
}
