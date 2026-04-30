#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pTreeWidget;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_tree_view_t tTree;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iSelected;
	int iSelectCount;
	int bInitOK;
	int bExpandOK;
	int bSelectOK;
	int bKeyboardOK;
	int bCollapseOK;
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
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-tree-view-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-tree-view-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void TreeSelect(xge_xui_widget pWidget, int iNodeId, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iSelected = iNodeId;
	pApp->iSelectCount++;
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
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pTreeWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pTreeWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 390.0f, 250.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "TreeView");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 342.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pTreeWidget, (xge_rect_t){ 18.0f, 72.0f, 342.0f, 146.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pTreeWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiTreeViewInit(&pApp->tTree, &pApp->tXui, pApp->pTreeWidget);
	xgeXuiTreeViewSetFont(&pApp->tTree, pFont);
	xgeXuiTreeViewSetSelect(&pApp->tTree, TreeSelect, pApp);
	xgeXuiTreeViewSetMetrics(&pApp->tTree, 20.0f, 14.0f);
	xgeXuiTreeViewAddNode(&pApp->tTree, 10, -1, "Project");
	xgeXuiTreeViewAddNode(&pApp->tTree, 20, 10, "src");
	xgeXuiTreeViewAddNode(&pApp->tTree, 30, 20, "xge_xui_tree_view.c");
	xgeXuiTreeViewAddNode(&pApp->tTree, 40, 20, "xge_xui_page.c");
	xgeXuiTreeViewAddNode(&pApp->tTree, 50, 10, "examples");
	xgeXuiTreeViewAddNode(&pApp->tTree, 60, 50, "xui_tree_view_lab");
	xgeXuiTreeViewAddNode(&pApp->tTree, 70, -1, "docs");
	xgeXuiTreeViewSetNodeExpanded(&pApp->tTree, 10, 1);
	xgeXuiTreeViewSetNodeExpanded(&pApp->tTree, 20, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pTreeWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (xgeXuiTreeViewGetVisibleCount(&pApp->tTree) == 6) && (xgeXuiTreeViewGetVisibleNodeId(&pApp->tTree, 2) == 30);
	pApp->bExpandOK = xgeXuiTreeViewGetNodeExpanded(&pApp->tTree, 10) && xgeXuiTreeViewGetNodeExpanded(&pApp->tTree, 20);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tTree.arrNodes[2].tRect.fX + 72.0f, pApp->tTree.arrNodes[2].tRect.fY + 10.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tTree.arrNodes[2].tRect.fX + 72.0f, pApp->tTree.arrNodes[2].tRect.fY + 10.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = (pApp->iSelected == 30) && (xgeXuiTreeViewGetSelected(&pApp->tTree) == 30);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_DOWN;
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bKeyboardOK = xgeXuiTreeViewGetSelected(&pApp->tTree) == 40;
	tEvent.iParam1 = XGE_KEY_LEFT;
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bCollapseOK = xgeXuiTreeViewGetSelected(&pApp->tTree) == 20;
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d expand=%d select=%d keyboard=%d collapse=%d selected=%d cb=%d",
		pApp->bInitOK,
		pApp->bExpandOK,
		pApp->bSelectOK,
		pApp->bKeyboardOK,
		pApp->bCollapseOK,
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
			"xui-tree-view-lab final-summary frames=%d init=%d expand=%d select=%d keyboard=%d collapse=%d selected=%d cb=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bExpandOK,
			pApp->bSelectOK,
			pApp->bKeyboardOK,
			pApp->bCollapseOK,
			pApp->iSelected,
			pApp->iSelectCount);
		printf("xui-tree-view-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TREE_VIEW_FRAMES"), 0);
	tApp.iSelected = -1;
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 440;
	tDesc.iHeight = 300;
	tDesc.sTitle = "XGE XUI TreeView Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bExpandOK && tApp.bSelectOK && tApp.bKeyboardOK && tApp.bCollapseOK) ? 0 : 3;
}
