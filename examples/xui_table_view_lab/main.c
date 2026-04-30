#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pTableWidget;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_table_view_t tTable;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iRows;
	int iCellCalls;
	int iSelected;
	int iSelectCount;
	int iSortColumn;
	int bSortDescending;
	int iSortCount;
	int bInitOK;
	int bVirtualOK;
	int bSelectOK;
	int bSortOK;
	int bResizeOK;
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
			printf("xui-table-view-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-table-view-lab font load failed\n");
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
		snprintf(sBuffer, (size_t)iBufferSize, "Item %02d", iRow);
	} else if ( iColumn == 1 ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%d", iRow * 7);
	} else {
		snprintf(sBuffer, (size_t)iBufferSize, "%s", (iRow & 1) ? "Idle" : "Ready");
	}
	return XGE_OK;
}

static void TableSort(xge_xui_widget pWidget, int iColumn, int bDescending, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSortColumn = iColumn;
		pApp->bSortDescending = bDescending;
		pApp->iSortCount++;
	}
}

static void TableSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
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
	xge_xui_table_view_column_t arrColumns[3];

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pTableWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pTableWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 460.0f, 272.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "TableView");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 410.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pTableWidget, (xge_rect_t){ 18.0f, 72.0f, 410.0f, 166.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pTableWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiTableViewInit(&pApp->tTable, &pApp->tXui, pApp->pTableWidget);
	xgeXuiTableViewSetFont(&pApp->tTable, pFont);
	memset(arrColumns, 0, sizeof(arrColumns));
	arrColumns[0].iId = 1;
	arrColumns[0].sTitle = "Name";
	arrColumns[0].fWidth = 150.0f;
	arrColumns[0].fMinWidth = 70.0f;
	arrColumns[0].iAlign = XGE_TEXT_ALIGN_LEFT;
	arrColumns[1].iId = 2;
	arrColumns[1].sTitle = "Value";
	arrColumns[1].fWidth = 90.0f;
	arrColumns[1].fMinWidth = 60.0f;
	arrColumns[1].iAlign = XGE_TEXT_ALIGN_RIGHT;
	arrColumns[2].iId = 3;
	arrColumns[2].sTitle = "State";
	arrColumns[2].fWidth = 120.0f;
	arrColumns[2].fMinWidth = 70.0f;
	arrColumns[2].iAlign = XGE_TEXT_ALIGN_CENTER;
	xgeXuiTableViewSetColumns(&pApp->tTable, arrColumns, 3);
	xgeXuiTableViewSetAdapter(&pApp->tTable, TableCount, TableCell, pApp);
	xgeXuiTableViewSetSort(&pApp->tTable, TableSort, pApp);
	xgeXuiTableViewSetSelect(&pApp->tTable, TableSelect, pApp);
	xgeXuiTableViewSetMetrics(&pApp->tTable, 24.0f, 20.0f);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pTableWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiTableViewSetScroll(&pApp->tTable, 40.0f);
	pApp->iCellCalls = 0;
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (pApp->tTable.iColumnCount == 3) && (xgeXuiTableViewGetRowCount(&pApp->tTable) == pApp->iRows);
	pApp->bVirtualOK = (xgeXuiTableViewGetFirstVisible(&pApp->tTable) == 2) && (xgeXuiTableViewGetPaintVisibleCount(&pApp->tTable) > 0) && (pApp->iCellCalls < pApp->iRows * pApp->tTable.iColumnCount);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pTableWidget->tRect.fX + 40.0f, pApp->pTableWidget->tRect.fY + 52.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->pTableWidget->tRect.fX + 40.0f, pApp->pTableWidget->tRect.fY + 52.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSelectOK = (pApp->iSelected == 3) && (xgeXuiTableViewGetSelected(&pApp->tTable) == 3);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pTableWidget->tRect.fX + 20.0f, pApp->pTableWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bSortOK = (pApp->iSortColumn == 0) && (pApp->iSortCount == 1);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pTableWidget->tRect.fX + 150.0f, pApp->pTableWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_MOVE, pApp->pTableWidget->tRect.fX + 180.0f, pApp->pTableWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->pTableWidget->tRect.fX + 180.0f, pApp->pTableWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bResizeOK = pApp->tTable.arrColumns[0].fWidth == 180.0f;
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d virtual=%d select=%d sort=%d resize=%d row=%d cells=%d",
		pApp->bInitOK,
		pApp->bVirtualOK,
		pApp->bSelectOK,
		pApp->bSortOK,
		pApp->bResizeOK,
		pApp->iSelected,
		pApp->iCellCalls);
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
			"xui-table-view-lab final-summary frames=%d init=%d virtual=%d select=%d sort=%d resize=%d row=%d cells=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bVirtualOK,
			pApp->bSelectOK,
			pApp->bSortOK,
			pApp->bResizeOK,
			pApp->iSelected,
			pApp->iCellCalls);
		printf("xui-table-view-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iRows = 28;
	tApp.iSelected = -1;
	tApp.iSortColumn = -1;
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABLE_VIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 510;
	tDesc.iHeight = 320;
	tDesc.sTitle = "XGE XUI TableView Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bVirtualOK && tApp.bSelectOK && tApp.bSortOK && tApp.bResizeOK) ? 0 : 3;
}
