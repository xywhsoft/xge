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
	xge_xui_widget pTree;
	xge_xui_widget pTable;
	xge_xui_widget pGrid;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_tree_view_t tTree;
	xge_xui_table_view_t tTable;
	xge_xui_property_grid_t tGrid;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iRows;
	int iCellCalls;
	int iTreeOK;
	int iTableOK;
	int iGridOK;
	int iAdapterOK;
	int iSelectionOK;
	int iGridName;
	int iGridAdvanced;
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
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-data-controls-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-data-controls-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int TableCount(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	return (pApp != NULL) ? pApp->iRows : 0;
}

static int TableCell(xge_xui_widget pWidget, int iRow, int iColumn, char* sBuffer, int iBufferSize, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pApp != NULL ) {
		pApp->iCellCalls++;
	}
	if ( iColumn == 0 ) {
		snprintf(sBuffer, (size_t)iBufferSize, "Asset %02d", iRow);
	} else if ( iColumn == 1 ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%d KB", 12 + iRow * 3);
	} else {
		snprintf(sBuffer, (size_t)iBufferSize, "%s", (iRow & 1) ? "Dirty" : "Ready");
	}
	return XGE_OK;
}

static void SelectProc(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	(void)pUser;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	xge_xui_table_view_column_t arrColumns[3];
	int iGeneral;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pTree = xgeXuiWidgetCreate();
	pApp->pTable = xgeXuiWidgetCreate();
	pApp->pGrid = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pTree == NULL) || (pApp->pTable == NULL) || (pApp->pGrid == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 22.0f, 20.0f, 760.0f, 326.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "XUI Data Controls");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 36.0f, 710.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);

	xgeXuiWidgetSetRect(pApp->pTree, (xge_rect_t){ 18.0f, 78.0f, 214.0f, 206.0f });
	xgeXuiTreeViewInit(&pApp->tTree, &pApp->tXui, pApp->pTree);
	xgeXuiTreeViewSetFont(&pApp->tTree, pFont);
	xgeXuiTreeViewSetMetrics(&pApp->tTree, 20.0f, 13.0f);
	xgeXuiTreeViewSetSelect(&pApp->tTree, SelectProc, pApp);
	xgeXuiTreeViewAddNode(&pApp->tTree, 10, -1, "Project");
	xgeXuiTreeViewAddNode(&pApp->tTree, 20, 10, "src");
	xgeXuiTreeViewAddNode(&pApp->tTree, 30, 20, "xui");
	xgeXuiTreeViewAddNode(&pApp->tTree, 40, 10, "examples");
	xgeXuiTreeViewAddNode(&pApp->tTree, 50, -1, "docs");
	xgeXuiTreeViewSetNodeExpanded(&pApp->tTree, 10, 1);
	xgeXuiTreeViewSetNodeExpanded(&pApp->tTree, 20, 1);
	xgeXuiTreeViewSetSelected(&pApp->tTree, 30);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pTree);

	xgeXuiWidgetSetRect(pApp->pTable, (xge_rect_t){ 252.0f, 78.0f, 250.0f, 206.0f });
	xgeXuiTableViewInit(&pApp->tTable, &pApp->tXui, pApp->pTable);
	xgeXuiTableViewSetFont(&pApp->tTable, pFont);
	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].iId = 1;
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 104.0f;
	arrColumns[0].fMinWidth = 60.0f;
	arrColumns[0].iAlign = XGE_TEXT_ALIGN_LEFT;
	arrColumns[1].iId = 2;
	arrColumns[1].sTitle = "Size";
	arrColumns[1].fWidth = 62.0f;
	arrColumns[1].fMinWidth = 44.0f;
	arrColumns[1].iAlign = XGE_TEXT_ALIGN_RIGHT;
	arrColumns[2].iId = 3;
	arrColumns[2].sTitle = "State";
	arrColumns[2].fWidth = 76.0f;
	arrColumns[2].fMinWidth = 52.0f;
	arrColumns[2].iAlign = XGE_TEXT_ALIGN_CENTER;
	xgeXuiTableViewSetColumns(&pApp->tTable, arrColumns, 3);
	xgeXuiTableViewSetAdapter(&pApp->tTable, TableCount, TableCell, pApp);
	xgeXuiTableViewSetSelect(&pApp->tTable, SelectProc, pApp);
	xgeXuiTableViewSetMetrics(&pApp->tTable, 24.0f, 20.0f);
	xgeXuiTableViewSetSelected(&pApp->tTable, 3);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pTable);

	xgeXuiWidgetSetRect(pApp->pGrid, (xge_rect_t){ 522.0f, 78.0f, 214.0f, 206.0f });
	xgeXuiPropertyGridInit(&pApp->tGrid, &pApp->tXui, pApp->pGrid);
	xgeXuiPropertyGridSetFont(&pApp->tGrid, pFont);
	xgeXuiPropertyGridSetMetrics(&pApp->tGrid, 22.0f, 86.0f);
	xgeXuiPropertyGridSetSelect(&pApp->tGrid, SelectProc, pApp);
	iGeneral = xgeXuiPropertyGridAddCategory(&pApp->tGrid, "General", 1);
	pApp->iGridName = xgeXuiPropertyGridAddProperty(&pApp->tGrid, iGeneral, "Name", "Asset 03", XGE_XUI_PROPERTY_GRID_EDITOR_TEXT);
	xgeXuiPropertyGridAddProperty(&pApp->tGrid, iGeneral, "Enabled", "true", XGE_XUI_PROPERTY_GRID_EDITOR_BOOL);
	pApp->iGridAdvanced = xgeXuiPropertyGridAddCategory(&pApp->tGrid, "Advanced", 1);
	xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iGridAdvanced, "Mode", "Auto", XGE_XUI_PROPERTY_GRID_EDITOR_ENUM);
	xgeXuiPropertyGridAddProperty(&pApp->tGrid, pApp->iGridAdvanced, "Tint", "#62A8E5", XGE_XUI_PROPERTY_GRID_EDITOR_COLOR);
	xgeXuiPropertyGridSetPropertyFlags(&pApp->tGrid, pApp->iGridName, 0, 1, 0);
	xgeXuiPropertyGridSetSelected(&pApp->tGrid, pApp->iGridName);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pGrid);

	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->iCellCalls = 0;
	xgeXuiPaint(&pApp->tXui);
	pApp->iTreeOK = (xgeXuiTreeViewGetVisibleCount(&pApp->tTree) == 5) && (xgeXuiTreeViewGetSelected(&pApp->tTree) == 30);
	pApp->iTableOK = (pApp->tTable.iColumnCount == 3) && (xgeXuiTableViewGetRowCount(&pApp->tTable) == pApp->iRows) && (xgeXuiTableViewGetSelected(&pApp->tTable) == 3);
	pApp->iGridOK = (xgeXuiPropertyGridGetVisibleCount(&pApp->tGrid) == 6) && (xgeXuiPropertyGridGetSelected(&pApp->tGrid) == pApp->iGridName);
	pApp->iAdapterOK = pApp->iCellCalls > 0;
	pApp->iSelectionOK = (xgeXuiTreeViewGetSelected(&pApp->tTree) == 30) && (xgeXuiTableViewGetSelected(&pApp->tTable) == 3) && (xgeXuiPropertyGridGetSelected(&pApp->tGrid) == pApp->iGridName);
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"tree=%d table=%d grid=%d adapter=%d selection=%d cells=%d",
		pApp->iTreeOK,
		pApp->iTableOK,
		pApp->iGridOK,
		pApp->iAdapterOK,
		pApp->iSelectionOK,
		pApp->iCellCalls);
	xgeXuiLabelSetText(&pApp->tSummary, sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	pApp->iRows = 32;
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
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
			"xui-data-controls-lab final-summary frames=%d tree=%d table=%d grid=%d adapter=%d selection=%d cells=%d\n",
			pApp->iFrameCount,
			pApp->iTreeOK,
			pApp->iTableOK,
			pApp->iGridOK,
			pApp->iAdapterOK,
			pApp->iSelectionOK,
			pApp->iCellCalls);
		printf("xui-data-controls-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DATA_CONTROLS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 810;
	tDesc.iHeight = 378;
	tDesc.sTitle = "XGE XUI Data Controls Lab";
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
	return (iExitCode == XGE_OK && tApp.iTreeOK && tApp.iTableOK && tApp.iGridOK && tApp.iAdapterOK && tApp.iSelectionOK) ? 0 : 3;
}
