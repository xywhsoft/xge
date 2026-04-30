#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_breadcrumb_t tBreadcrumb;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pBreadcrumbWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelected;
	int iSelectCount;
	int bInitOK;
	int bOverflowOK;
	int bSelectOK;
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
			printf("xui-breadcrumb-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-breadcrumb-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void BreadcrumbSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelected = iIndex;
		pApp->iSelectCount++;
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

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pBreadcrumbWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pBreadcrumbWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 390.0f, 168.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Breadcrumb");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 335.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pBreadcrumbWidget, (xge_rect_t){ 18.0f, 78.0f, 235.0f, 28.0f });
	xgeXuiBreadcrumbInit(&pApp->tBreadcrumb, &pApp->tXui, pApp->pBreadcrumbWidget);
	xgeXuiBreadcrumbSetFont(&pApp->tBreadcrumb, pFont);
	xgeXuiBreadcrumbSetSelect(&pApp->tBreadcrumb, BreadcrumbSelect, pApp);
	xgeXuiBreadcrumbSetMetrics(&pApp->tBreadcrumb, 9.0f, 15.0f);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "Home", 1);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "Workspace", 2);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "Assets", 3);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "Textures", 4);
	xgeXuiBreadcrumbAddSegment(&pApp->tBreadcrumb, "Sky", 5);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pBreadcrumbWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tLast;
	int iLast;

	iLast = xgeXuiBreadcrumbGetSegmentCount(&pApp->tBreadcrumb) - 1;
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = xgeXuiBreadcrumbGetSegmentCount(&pApp->tBreadcrumb) == 5;
	pApp->bOverflowOK = xgeXuiBreadcrumbGetCollapsedCount(&pApp->tBreadcrumb) > 0 && xgeXuiBreadcrumbGetFirstVisible(&pApp->tBreadcrumb) > 0;
	tLast = pApp->tBreadcrumb.arrSegments[iLast].tRect;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, tLast.fX + 4.0f, tLast.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = (pApp->iSelected == iLast) && (xgeXuiBreadcrumbGetSelected(&pApp->tBreadcrumb) == iLast);
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[160];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d overflow=%d select=%d collapsed=%d row=%d cb=%d",
		pApp->bInitOK,
		pApp->bOverflowOK,
		pApp->bSelectOK,
		xgeXuiBreadcrumbGetCollapsedCount(&pApp->tBreadcrumb),
		pApp->iSelected,
		pApp->iSelectCount);
	xgeXuiLabelSetText(&pApp->tSummary, sText);
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
	RunChecks(pApp);
	UpdateSummary(pApp);
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
	UpdateSummary(pApp);
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
			"xui-breadcrumb-lab final-summary frames=%d init=%d overflow=%d select=%d collapsed=%d row=%d cb=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bOverflowOK,
			pApp->bSelectOK,
			xgeXuiBreadcrumbGetCollapsedCount(&pApp->tBreadcrumb),
			pApp->iSelected,
			pApp->iSelectCount);
		printf("xui-breadcrumb-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iSelected = -1;
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_BREADCRUMB_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 438;
	tDesc.iHeight = 214;
	tDesc.sTitle = "XGE XUI Breadcrumb Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bOverflowOK && tApp.bSelectOK) ? 0 : 3;
}
