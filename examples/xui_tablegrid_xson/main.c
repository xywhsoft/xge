#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 131072
#define XSON_LARGE_ROWS 48

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	char sXson[XSON_BUFFER_SIZE];
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_tablegrid_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int AppendText(char* sBuffer, int* pOffset, int iCapacity, const char* sText)
{
	int iLen;

	if ( (sBuffer == NULL) || (pOffset == NULL) || (sText == NULL) || (*pOffset >= iCapacity) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iLen = (int)strlen(sText);
	if ( (*pOffset + iLen) >= iCapacity ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memcpy(sBuffer + *pOffset, sText, (size_t)iLen);
	*pOffset += iLen;
	sBuffer[*pOffset] = 0;
	return XGE_OK;
}

static int AppendFormat(char* sBuffer, int* pOffset, int iCapacity, const char* sFormat, ...)
{
	va_list args;
	int iWritten;

	if ( (sBuffer == NULL) || (pOffset == NULL) || (sFormat == NULL) || (*pOffset >= iCapacity) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	va_start(args, sFormat);
	iWritten = vsnprintf(sBuffer + *pOffset, (size_t)(iCapacity - *pOffset), sFormat, args);
	va_end(args);
	if ( (iWritten < 0) || ((*pOffset + iWritten) >= iCapacity) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	*pOffset += iWritten;
	return XGE_OK;
}

static int AppendRows(char* sBuffer, int* pOffset, int iCount)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( i > 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "{\"height\":%d%s}", (i % 6 == 0) ? 26 : 22, (i == 5) ? ",\"disabled\":true" : "") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return XGE_OK;
}

static int AppendLargeCells(char* sBuffer, int* pOffset)
{
	int i;
	int j;

	for ( i = 0; i < XSON_LARGE_ROWS; i++ ) {
		if ( i > 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "[") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		for ( j = 0; j < 7; j++ ) {
			if ( j > 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "\"Editable %02d-%02d\"", i + 1, j + 1) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		}
		if ( AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, "]") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return XGE_OK;
}

static int BuildXson(app_state_t* pApp)
{
	int iOffset;

	iOffset = 0;
	pApp->sXson[0] = 0;
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"xui\":1,"
			"\"styles\":{"
			"\"root\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"background\":\"#ECF0F6FF\",\"columnGap\":12,\"rowGap\":12},"
			"\"panel\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[12,10,12,12],\"gap\":8,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"radius\":4},"
			"\"title\":{\"height\":28,\"textColor\":\"#2A3A4EFF\"},"
			"\"grid\":{\"width\":\"100%\",\"height\":\"grow\",\"padding\":2,\"defaultRowHeight\":24,\"headerHeight\":24,\"backgroundColor\":\"#F8FAFDFF\",\"headerColor\":\"#E4F0FBFF\",\"rowColor\":\"#F7FAFDFF\",\"selectedColor\":\"#BEDBF2FF\",\"gridColor\":\"#B8D3E8FF\",\"textColor\":\"#2A3442FF\",\"disabledTextColor\":\"#87909DB8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"}"
			"},"
			"\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_edit\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_edit\",\"style\":\"title\",\"text\":\"XSON TableGrid: text, int, float and bool\"},"
			"{\"type\":\"tableGrid\",\"id\":\"grid_edit\",\"style\":\"grid\",\"selectionMode\":\"cell\",\"selectedCell\":[0,0],"
			"\"columns\":[{\"id\":\"name\",\"title\":\"Name\",\"width\":120,\"type\":\"text\"},{\"id\":\"count\",\"title\":\"Count\",\"width\":80,\"align\":\"right\",\"type\":\"int\"},{\"id\":\"price\",\"title\":\"Price\",\"width\":86,\"align\":\"right\",\"type\":\"float\"},{\"id\":\"enabled\",\"title\":\"Enabled\",\"width\":90,\"type\":\"bool\"}],"
			"\"rows\":[{\"height\":24},{\"height\":24},{\"height\":24},{\"height\":24,\"disabled\":true}],"
			"\"cells\":[[\"Alpha\",12,3.5,true],[\"Beta\",20,8.25,false],[\"Gamma\",{\"text\":31,\"dirty\":true},12.75,true],[\"Delta\",7,{\"text\":4.5,\"disabled\":true},false]]}]}," ) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_quick\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_quick\",\"style\":\"title\",\"text\":\"XSON quick edit and merged cells\"},"
			"{\"type\":\"tableGrid\",\"id\":\"grid_quick\",\"style\":\"grid\",\"editMode\":\"quick\",\"selectionMode\":\"cell\",\"selectedCell\":[1,1],\"selectedColor\":\"#CDEDE5FF\",\"thumbColor\":\"#43A77CFF\","
			"\"columns\":[{\"title\":\"Key\",\"width\":86},{\"title\":\"Span A\",\"width\":100},{\"title\":\"Span B\",\"width\":100},{\"title\":\"Enum\",\"width\":82,\"type\":\"enum\"},{\"title\":\"Color\",\"width\":86,\"type\":\"color\"},{\"title\":\"Date\",\"width\":94,\"type\":\"date\"},{\"title\":\"Time\",\"width\":72,\"type\":\"time\"},{\"title\":\"Notes\",\"width\":100,\"type\":\"textarea\"},{\"title\":\"Picker\",\"width\":84,\"type\":\"picker\"},{\"title\":\"File\",\"width\":82,\"type\":\"file\"},{\"title\":\"Image\",\"width\":82,\"type\":\"image\"}],"
			"\"rows\":[{\"height\":24},{\"height\":30},{\"height\":30},{\"height\":24}],"
			"\"cells\":[[{\"text\":\"Header span\",\"colSpan\":3},\"\",\"\",\"Ready\",\"#2E7CD6\",\"2026-05-18\",\"09:30\",\"Long note\",\"...\",\"file.txt\",\"image.png\"],[\"Row 1\",{\"text\":\"Merged edit area\",\"rowSpan\":2,\"colSpan\":2},\"\",\"Draft\",\"#43A77C\",\"2026-05-19\",\"10:00\",\"Note 1\",\"Open\",\"Browse\",\"Pick\"],[\"Row 2\",\"\",\"\",\"Done\",\"#D5504A\",\"2026-05-20\",\"10:30\",\"Note 2\",\"Open\",\"Browse\",\"Pick\"],[\"Row 3\",\"Plain\",\"Cell\",\"Blocked\",\"#8456D1\",\"2026-05-21\",\"11:00\",\"Note 3\",\"Open\",\"Browse\",\"Pick\"]]}]}," ) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_large\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_large\",\"style\":\"title\",\"text\":\"XSON large editable grid: scroll and static data adapter\"},"
			"{\"type\":\"tableGrid\",\"id\":\"grid_large\",\"style\":\"grid\",\"selectionMode\":\"cell\",\"selectedCell\":[18,5],\"scrollbarMode\":\"full\",\"scrollX\":96,\"scrollY\":180,"
			"\"defaultColumnWidth\":92,\"defaultRowHeight\":22,"
			"\"columns\":[{\"title\":\"Record\",\"width\":120},{\"title\":\"A\",\"width\":92},{\"title\":\"B\",\"width\":92},{\"title\":\"C\",\"width\":92},{\"title\":\"D\",\"width\":92},{\"title\":\"E\",\"width\":92},{\"title\":\"Far\",\"width\":110}],"
			"\"rows\":[") != XGE_OK ||
	     AppendRows(pApp->sXson, &iOffset, XSON_LARGE_ROWS) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "],\"cells\":[") != XGE_OK ||
	     AppendLargeCells(pApp->sXson, &iOffset) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}]},") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_state\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_state\",\"style\":\"title\",\"text\":\"XSON states: invalid, editing, dirty, disabled\"},"
			"{\"type\":\"tableGrid\",\"id\":\"grid_state\",\"style\":\"grid\",\"selectionMode\":\"row\",\"selectedRow\":2,"
			"\"columns\":[{\"title\":\"Field\",\"width\":120},{\"title\":\"Value\",\"width\":140},{\"title\":\"State\",\"width\":110}],"
			"\"rows\":[{\"height\":28},{\"height\":28},{\"height\":28},{\"height\":28,\"disabled\":true}],"
			"\"cells\":[[\"Name\",{\"text\":\"Atlas\",\"editing\":true},\"editing\"],[\"Score\",{\"text\":\"bad\",\"invalid\":true},\"invalid\"],[\"Status\",{\"text\":\"Changed\",\"dirty\":true},\"dirty\"],[\"Readonly\",{\"text\":\"Disabled\",\"disabled\":true},\"disabled\"]]}]}]}}") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "default", &pApp->tFont);
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_tablegrid_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	float fPanelH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiPageFind(&pApp->tPage, "root");
	if ( pRoot == NULL ) {
		return;
	}
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 980.0f ) {
		fRootW = 980.0f;
	}
	if ( fRootH < 700.0f ) {
		fRootH = 700.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	fPanelH = (fRootH - 36.0f - 12.0f) * 0.5f;
	xgeXuiWidgetSetGrid(pRoot, 2, fPanelH, 12.0f, 12.0f);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pEdit;
	xge_xui_widget pQuick;
	xge_xui_widget pLarge;
	xge_xui_widget pState;
	xge_xui_table_view pTable;
	xge_rect_t tRect;
	float fScrollX;
	float fScrollY;
	int iRow;
	int iColumn;
	int bEditOK;
	int bBoolOK;

	pEdit = xgeXuiPageFind(&pApp->tPage, "grid_edit");
	pQuick = xgeXuiPageFind(&pApp->tPage, "grid_quick");
	pLarge = xgeXuiPageFind(&pApp->tPage, "grid_large");
	pState = xgeXuiPageFind(&pApp->tPage, "grid_state");
	pApp->bCreateOK = (pEdit != NULL) && (pQuick != NULL) && (pLarge != NULL) && (pState != NULL) && (pApp->tPage.iTableGridCount == 4);
	pApp->bLayoutOK = (pEdit != NULL) && (pEdit->tRect.fW > 300.0f) && (pState != NULL) && (pState->tRect.fH > 180.0f);
	if ( pApp->tPage.iTableGridCount >= 4 ) {
		bEditOK = 0;
		if ( xgeXuiTableGridBeginEdit(&pApp->tPage.arrTableGrid[0], 0, 0) != 0 ) {
			xgeXuiInputSetText(&pApp->tPage.arrTableGrid[0].tEditInput, "Edited from XSON");
			bEditOK = xgeXuiTableGridEndEdit(&pApp->tPage.arrTableGrid[0], 1) &&
			          (strcmp(pApp->tPage.arrTableGridAdapter[0]->arrCell[0][0], "Edited from XSON") == 0) &&
			          (pApp->tPage.arrTableGridAdapter[0]->arrCellDirty[0][0] != 0);
		}
		bBoolOK = xgeXuiTableGridBeginEdit(&pApp->tPage.arrTableGrid[0], 0, 3) &&
		          (strcmp(pApp->tPage.arrTableGridAdapter[0]->arrCell[0][3], "false") == 0);
		pTable = xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[2]);
		xgeXuiTableViewGetOffset(pTable, &fScrollX, &fScrollY);
		xgeXuiTableViewGetSelectedCell(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1]), &iRow, &iColumn);
		pApp->bStateOK =
			bEditOK &&
			bBoolOK &&
			(xgeXuiTableGridGetEditMode(&pApp->tPage.arrTableGrid[1]) == XGE_XUI_TABLE_GRID_EDIT_QUICK) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[3].iType == XGE_XUI_TABLE_CELL_TYPE_ENUM) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[4].iType == XGE_XUI_TABLE_CELL_TYPE_COLOR) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[5].iType == XGE_XUI_TABLE_CELL_TYPE_DATE) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[6].iType == XGE_XUI_TABLE_CELL_TYPE_TIME) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[7].iType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[8].iType == XGE_XUI_TABLE_CELL_TYPE_PICKER) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[9].iType == XGE_XUI_TABLE_CELL_TYPE_FILE) &&
			(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[1])->arrColumns[10].iType == XGE_XUI_TABLE_CELL_TYPE_IMAGE) &&
			(xgeXuiTableViewGetSelectionMode(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[3])) == XGE_XUI_TABLE_VIEW_SELECTION_ROW) &&
			(xgeXuiTableViewGetSelectedRow(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[3])) == 2) &&
			(xgeXuiTableViewGetRowCount(pTable) == XSON_LARGE_ROWS) &&
			(fScrollX > 0.0f) &&
			(fScrollY > 0.0f) &&
			(iRow == 1) &&
			(iColumn == 1) &&
			xgeXuiTableViewGetCellRect(xgeXuiTableGridGetTableView(&pApp->tPage.arrTableGrid[0]), 0, 0, &tRect) &&
			(tRect.fW > 20.0f) &&
			(xgeXuiTableViewGetScrollbarMode(pTable) == XGE_XUI_SCROLLBAR_MODE_FULL);
	}
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_tablegrid_xson final-summary frames=%d create=%d layout=%d state=%d gridCount=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->tPage.iTableGridCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABLEGRID_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI TableGrid XSON";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("xui_tablegrid_xson xgeInit failed\n");
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
