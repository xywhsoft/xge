#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_table_view pTable;
	int iFrameLimit;
	int iFrameCount;
	int bLoadOK;
	int bColumnsOK;
	int bRowsOK;
	int bStyleOK;
} app_state_t;

static const char g_sPage[] =
	"{"
	"\"xui\":1,"
	"\"tokens\":{\"spacing\":{\"head\":24,\"row\":20},\"colors\":{\"bg\":\"#EBF4FC\",\"head\":\"#D2E7F7\",\"row\":\"#F5FAFF\",\"sel\":\"#BBDCF8\",\"grid\":\"#97BBD7\",\"text\":\"#223040\"}},"
	"\"styles\":{\"table\":{\"headerHeight\":\"@spacing.head\",\"rowHeight\":\"@spacing.row\",\"backgroundColor\":\"@colors.bg\",\"headerColor\":\"@colors.head\",\"rowColor\":\"@colors.row\",\"selectedColor\":\"@colors.sel\",\"gridColor\":\"@colors.grid\",\"textColor\":\"@colors.text\"}},"
	"\"tree\":{\"type\":\"tableView\",\"id\":\"table\",\"style\":\"table\",\"width\":410,\"height\":166,\"selected\":2,\"columns\":[{\"id\":1,\"title\":\"Name\",\"width\":150,\"minWidth\":70,\"align\":\"left\"},{\"id\":2,\"title\":\"Value\",\"width\":90,\"minWidth\":60,\"align\":\"right\"},{\"id\":3,\"title\":\"State\",\"width\":120,\"minWidth\":70,\"align\":\"center\"}],\"rows\":[[\"Item 00\",0,\"Ready\"],[\"Item 01\",7,\"Idle\"],[\"Item 02\",14,\"Ready\"],[\"Item 03\",21,\"Idle\"],[\"Item 04\",28,\"Ready\"]]}"
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
	pApp->pTable = (pRoot != NULL) ? (xge_xui_table_view)pRoot->pUser : NULL;
	pApp->bColumnsOK = pApp->pTable != NULL && pApp->pTable->iColumnCount == 3 && pApp->pTable->arrColumns[1].iAlign == XGE_TEXT_ALIGN_RIGHT;
	pApp->bRowsOK = pApp->pTable != NULL && pApp->tPage.arrTableViewAdapter[0] != NULL && xgeXuiTableViewGetRowCount(pApp->pTable) == 5 && xgeXuiTableViewGetSelected(pApp->pTable) == 2 && strcmp(pApp->tPage.arrTableViewAdapter[0]->arrCell[2][1], "14") == 0;
	pApp->bStyleOK = pApp->pTable != NULL && pApp->pTable->fHeaderHeight == 24.0f && pApp->pTable->fRowHeight == 20.0f && pApp->pTable->iGridColor == XGE_COLOR_RGBA(0x97, 0xBB, 0xD7, 0xFF);
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
			"xui-xson-table-view-lab final-summary frames=%d load=%d columns=%d rows=%d style=%d count=%d selected=%d\n",
			pApp->iFrameCount,
			pApp->bLoadOK,
			pApp->bColumnsOK,
			pApp->bRowsOK,
			pApp->bStyleOK,
			pApp->pTable != NULL ? xgeXuiTableViewGetRowCount(pApp->pTable) : 0,
			pApp->pTable != NULL ? xgeXuiTableViewGetSelected(pApp->pTable) : -1);
		printf("xui-xson-table-view-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_XSON_TABLE_VIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 460;
	tDesc.iHeight = 220;
	tDesc.sTitle = "XGE XUI XSON TableView Lab";
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
	return (iExitCode == XGE_OK && tApp.bLoadOK && tApp.bColumnsOK && tApp.bRowsOK && tApp.bStyleOK) ? 0 : 3;
}
