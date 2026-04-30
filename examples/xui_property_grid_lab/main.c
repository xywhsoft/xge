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
	xge_xui_property_grid_t tGrid;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pGridWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iGeneral;
	int iAdvanced;
	int iName;
	int iCount;
	int iEnabled;
	int iMode;
	int iTint;
	int iSelected;
	int iSelectCount;
	int bInitOK;
	int bFlagsOK;
	int bSelectOK;
	int bCollapseOK;
	int bEditorsOK;
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
			printf("xui-property-grid-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-property-grid-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void PropertySelect(xge_xui_widget pWidget, int iIndex, void* pUser)
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
	pApp->pGridWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pGridWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 400.0f, 286.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "PropertyGrid");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 350.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pGridWidget, (xge_rect_t){ 18.0f, 72.0f, 350.0f, 178.0f });
	xgeXuiPropertyGridInit(&pApp->tGrid, &pApp->tXui, pApp->pGridWidget);
	xgeXuiPropertyGridSetFont(&pApp->tGrid, pFont);
	xgeXuiPropertyGridSetMetrics(&pApp->tGrid, 22.0f, 126.0f);
	xgeXuiPropertyGridSetSelect(&pApp->tGrid, PropertySelect, pApp);
	pApp->iGeneral = xgeXuiPropertyGridAddCategory(&pApp->tGrid, "General", 1);
	pApp->iName = xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iGeneral, "Name", "Player", XGE_XUI_PROPERTY_GRID_EDITOR_TEXT);
	pApp->iCount = xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iGeneral, "Count", "12", XGE_XUI_PROPERTY_GRID_EDITOR_NUMBER);
	pApp->iEnabled = xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iGeneral, "Enabled", "true", XGE_XUI_PROPERTY_GRID_EDITOR_BOOL);
	pApp->iAdvanced = xgeXuiPropertyGridAddCategory(&pApp->tGrid, "Advanced", 0);
	pApp->iMode = xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iAdvanced, "Mode", "Auto", XGE_XUI_PROPERTY_GRID_EDITOR_ENUM);
	pApp->iTint = xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iAdvanced, "Tint", "#62A8E5", XGE_XUI_PROPERTY_GRID_EDITOR_COLOR);
	xgeXuiPropertyGridSetPropertyFlags(&pApp->tGrid, pApp->iName, 1, 0, 0);
	xgeXuiPropertyGridSetPropertyFlags(&pApp->tGrid, pApp->iCount, 0, 1, 0);
	xgeXuiPropertyGridSetPropertyFlags(&pApp->tGrid, pApp->iEnabled, 0, 0, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pGridWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (pApp->iGeneral == 0) && (pApp->iAdvanced == 4) && (xgeXuiPropertyGridGetVisibleCount(&pApp->tGrid) == 5);
	pApp->bFlagsOK = pApp->tGrid.arrItems[pApp->iName].bReadonly && pApp->tGrid.arrItems[pApp->iCount].bDefaultChanged && pApp->tGrid.arrItems[pApp->iEnabled].bError;
	pApp->bEditorsOK =
		(pApp->tGrid.arrItems[pApp->iName].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_TEXT) &&
		(pApp->tGrid.arrItems[pApp->iCount].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_NUMBER) &&
		(pApp->tGrid.arrItems[pApp->iEnabled].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_BOOL) &&
		(pApp->tGrid.arrItems[pApp->iMode].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM) &&
		(pApp->tGrid.arrItems[pApp->iTint].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_COLOR);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pGridWidget->tRect.fX + 28.0f, pApp->pGridWidget->tRect.fY + 34.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = (pApp->iSelected == pApp->iName) && (xgeXuiPropertyGridGetSelected(&pApp->tGrid) == pApp->iName);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pGridWidget->tRect.fX + 20.0f, pApp->pGridWidget->tRect.fY + 92.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bCollapseOK = xgeXuiPropertyGridGetVisibleCount(&pApp->tGrid) == 7;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pGridWidget->tRect.fX + 20.0f, pApp->pGridWidget->tRect.fY + 92.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bCollapseOK = pApp->bCollapseOK && (xgeXuiPropertyGridGetVisibleCount(&pApp->tGrid) == 5);
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d flags=%d select=%d collapse=%d editors=%d row=%d cb=%d",
		pApp->bInitOK,
		pApp->bFlagsOK,
		pApp->bSelectOK,
		pApp->bCollapseOK,
		pApp->bEditorsOK,
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
			"xui-property-grid-lab final-summary frames=%d init=%d flags=%d select=%d collapse=%d editors=%d row=%d cb=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bFlagsOK,
			pApp->bSelectOK,
			pApp->bCollapseOK,
			pApp->bEditorsOK,
			pApp->iSelected,
			pApp->iSelectCount);
		printf("xui-property-grid-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PROPERTY_GRID_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 450;
	tDesc.iHeight = 334;
	tDesc.sTitle = "XGE XUI PropertyGrid Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bFlagsOK && tApp.bSelectOK && tApp.bCollapseOK && tApp.bEditorsOK) ? 0 : 3;
}
