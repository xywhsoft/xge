#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_breadcrumb pBreadcrumb;
	int iFrameLimit;
	int iFrameCount;
	int bLoadOK;
	int bSegmentsOK;
	int bSelectedOK;
} app_state_t;

static const char g_sPage[] =
	"{"
	"\"xui\":1,"
	"\"tokens\":{\"spacing\":{\"pad\":8,\"sep\":14},\"colors\":{\"bg\":\"#EEF8FF\",\"seg\":\"#F9FDFF\",\"sel\":\"#C8E6F8\",\"border\":\"#86B8D8\",\"text\":\"#1F3A52\",\"sepColor\":\"#3B7DAA\"}},"
	"\"styles\":{\"crumb\":{\"paddingX\":\"@spacing.pad\",\"separatorWidth\":\"@spacing.sep\",\"backgroundColor\":\"@colors.bg\",\"segmentColor\":\"@colors.seg\",\"selectedColor\":\"@colors.sel\",\"borderColor\":\"@colors.border\",\"textColor\":\"@colors.text\",\"separatorColor\":\"@colors.sepColor\"}},"
	"\"tree\":{\"type\":\"breadcrumb\",\"id\":\"path\",\"style\":\"crumb\",\"width\":360,\"height\":30,\"selected\":2,\"segments\":[\"Home\",{\"text\":\"Project\",\"id\":20},{\"text\":\"Assets\",\"id\":30},{\"text\":\"Textures\",\"id\":40}]}"
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
	pApp->pBreadcrumb = (pRoot != NULL) ? (xge_xui_breadcrumb)pRoot->pUser : NULL;
	pApp->bSegmentsOK = pApp->pBreadcrumb != NULL && xgeXuiBreadcrumbGetSegmentCount(pApp->pBreadcrumb) == 4 && pApp->pBreadcrumb->arrSegments[2].iId == 30;
	pApp->bSelectedOK = pApp->pBreadcrumb != NULL && xgeXuiBreadcrumbGetSelected(pApp->pBreadcrumb) == 2;
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
			"xui-xson-breadcrumb-lab final-summary frames=%d load=%d segments=%d selected=%d count=%d\n",
			pApp->iFrameCount,
			pApp->bLoadOK,
			pApp->bSegmentsOK,
			pApp->bSelectedOK,
			pApp->pBreadcrumb != NULL ? xgeXuiBreadcrumbGetSegmentCount(pApp->pBreadcrumb) : 0);
		printf("xui-xson-breadcrumb-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_XSON_BREADCRUMB_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 400;
	tDesc.iHeight = 90;
	tDesc.sTitle = "XGE XUI XSON Breadcrumb Lab";
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
	return (iExitCode == XGE_OK && tApp.bLoadOK && tApp.bSegmentsOK && tApp.bSelectedOK) ? 0 : 3;
}
