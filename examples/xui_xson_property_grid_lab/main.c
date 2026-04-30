#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_xui_property_grid pGrid;
	int iFrameLimit;
	int iFrameCount;
	int bLoadOK;
	int bItemsOK;
	int bFlagsOK;
	int bStyleOK;
} app_state_t;

static const char g_sPage[] =
	"{"
	"\"xui\":1,"
	"\"tokens\":{\"spacing\":{\"row\":22,\"name\":126},\"colors\":{\"bg\":\"#EBF4FC\",\"cat\":\"#D2E7F7\",\"row\":\"#F5FAFF\",\"sel\":\"#BBDCF8\",\"grid\":\"#97BBD7\",\"text\":\"#223040\",\"value\":\"#195484\"}},"
	"\"styles\":{\"props\":{\"rowHeight\":\"@spacing.row\",\"nameWidth\":\"@spacing.name\",\"backgroundColor\":\"@colors.bg\",\"categoryColor\":\"@colors.cat\",\"rowColor\":\"@colors.row\",\"selectedColor\":\"@colors.sel\",\"gridColor\":\"@colors.grid\",\"textColor\":\"@colors.text\",\"valueColor\":\"@colors.value\"}},"
	"\"tree\":{\"type\":\"propertyGrid\",\"id\":\"props\",\"style\":\"props\",\"width\":350,\"height\":178,\"selected\":1,\"categories\":[{\"name\":\"General\",\"expanded\":true,\"properties\":[{\"name\":\"Name\",\"value\":\"Player\",\"editor\":\"text\",\"readonly\":true},{\"name\":\"Count\",\"value\":\"12\",\"editor\":\"number\",\"changed\":true},{\"name\":\"Enabled\",\"value\":\"true\",\"editor\":\"bool\",\"error\":true}]},{\"name\":\"Advanced\",\"expanded\":false,\"properties\":[{\"name\":\"Mode\",\"value\":\"Auto\",\"editor\":\"enum\"},{\"name\":\"Tint\",\"value\":\"#62A8E5\",\"editor\":\"color\"}]}]}"
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
	pApp->pGrid = (pRoot != NULL) ? (xge_xui_property_grid)pRoot->pUser : NULL;
	pApp->bItemsOK = pApp->pGrid != NULL && pApp->pGrid->iItemCount == 7 && xgeXuiPropertyGridGetVisibleCount(pApp->pGrid) == 5 && xgeXuiPropertyGridGetSelected(pApp->pGrid) == 1;
	pApp->bFlagsOK = pApp->pGrid != NULL && pApp->pGrid->arrItems[1].bReadonly && pApp->pGrid->arrItems[2].bDefaultChanged && pApp->pGrid->arrItems[3].bError && pApp->pGrid->arrItems[6].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_COLOR;
	pApp->bStyleOK = pApp->pGrid != NULL && pApp->pGrid->fRowHeight == 22.0f && pApp->pGrid->fNameWidth == 126.0f && pApp->pGrid->iValueColor == XGE_COLOR_RGBA(0x19, 0x54, 0x84, 0xFF);
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
			"xui-xson-property-grid-lab final-summary frames=%d load=%d items=%d flags=%d style=%d count=%d visible=%d\n",
			pApp->iFrameCount,
			pApp->bLoadOK,
			pApp->bItemsOK,
			pApp->bFlagsOK,
			pApp->bStyleOK,
			pApp->pGrid != NULL ? pApp->pGrid->iItemCount : 0,
			pApp->pGrid != NULL ? xgeXuiPropertyGridGetVisibleCount(pApp->pGrid) : 0);
		printf("xui-xson-property-grid-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_XSON_PROPERTY_GRID_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 400;
	tDesc.iHeight = 230;
	tDesc.sTitle = "XGE XUI XSON PropertyGrid Lab";
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
	return (iExitCode == XGE_OK && tApp.bLoadOK && tApp.bItemsOK && tApp.bFlagsOK && tApp.bStyleOK) ? 0 : 3;
}
