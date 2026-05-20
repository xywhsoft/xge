#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 131072
#define XSON_LARGE_ROWS 64

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
			printf("xui_tableview_xson font loaded: %s\n", arrFonts[i]);
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
		if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "{\"height\":%d%s}", (i % 5 == 0) ? 26 : 22, (i == 7) ? ",\"disabled\":true" : "") != XGE_OK ) {
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
		for ( j = 0; j < 8; j++ ) {
			if ( j > 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "\"R%02d C%02d\"", i + 1, j + 1) != XGE_OK ) {
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
			"\"title\":{\"height\":28,\"font\":\"@fonts.default\",\"textColor\":\"#2A3A4EFF\"},"
			"\"table\":{\"width\":\"100%\",\"height\":\"grow\",\"font\":\"@fonts.default\",\"padding\":2,\"defaultRowHeight\":24,\"headerHeight\":24,\"backgroundColor\":\"#F8FAFDFF\",\"headerColor\":\"#E4F0FBFF\",\"rowColor\":\"#F7FAFDFF\",\"selectedColor\":\"#BEDBF2FF\",\"gridColor\":\"#B8D3E8FF\",\"textColor\":\"#2A3442FF\",\"disabledTextColor\":\"#87909DB8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"}"
			"},"
			"\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_basic\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_basic\",\"style\":\"title\",\"text\":\"XSON row selection: columns, rows and selected\"},"
			"{\"type\":\"tableView\",\"id\":\"table_basic\",\"style\":\"table\",\"selectionMode\":\"row\",\"selectedRow\":1,"
			"\"columns\":[{\"id\":\"name\",\"title\":\"Name\",\"width\":120},{\"id\":\"state\",\"title\":\"State\",\"width\":100},{\"id\":\"score\",\"title\":\"Score\",\"width\":70,\"align\":\"right\"},{\"id\":\"zone\",\"title\":\"Zone\",\"width\":90}],"
			"\"rows\":[{\"height\":24},{\"height\":28},{\"height\":24},{\"height\":24,\"disabled\":true},{\"height\":24}],"
			"\"cells\":[[\"Alpha\",\"Ready\",24,\"North\"],[\"Beta\",{\"text\":\"Running\",\"tooltip\":\"XSON cell tooltip\"},31,\"East\"],[\"Gamma\",\"Queued\",12,\"South\"],[\"Delta\",\"Disabled\",{\"text\":6,\"disabled\":true},\"West\"],[\"Epsilon\",\"Ready\",18,\"North\"]]}]},") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(
			pApp->sXson,
			&iOffset,
			XSON_BUFFER_SIZE,
			"{\"type\":\"column\",\"id\":\"panel_large\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_large\",\"style\":\"title\",\"text\":\"XSON large table: full scrollbar and scroll offsets\"},"
			"{\"type\":\"tableView\",\"id\":\"table_large\",\"style\":\"table\",\"selectionMode\":\"cell\",\"selectedCell\":[22,5],\"scrollbarMode\":\"full\",\"scrollX\":120,\"scrollY\":240,"
			"\"defaultColumnWidth\":86,\"defaultRowHeight\":22,"
			"\"columns\":[{\"title\":\"Record\",\"width\":120},{\"title\":\"A\",\"width\":86},{\"title\":\"B\",\"width\":86},{\"title\":\"C\",\"width\":86},{\"title\":\"D\",\"width\":86},{\"title\":\"E\",\"width\":86},{\"title\":\"F\",\"width\":86},{\"title\":\"Far\",\"width\":96}],"
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
			"{\"type\":\"column\",\"id\":\"panel_merge\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_merge\",\"style\":\"title\",\"text\":\"XSON merged cells: rowSpan / colSpan\"},"
			"{\"type\":\"tableView\",\"id\":\"table_merge\",\"style\":\"table\",\"selectionMode\":\"cell\",\"selectedCell\":[1,1],"
			"\"columns\":[{\"title\":\"Key\",\"width\":84},{\"title\":\"Span A\",\"width\":104},{\"title\":\"Span B\",\"width\":104},{\"title\":\"Fixed\",\"width\":76,\"resizable\":false},{\"title\":\"Note\",\"width\":92}],"
			"\"rows\":[{\"height\":24},{\"height\":30},{\"height\":30},{\"height\":24},{\"height\":24}],"
			"\"cells\":[[{\"text\":\"Merged title\",\"colSpan\":3},\"\",\"\",\"Fixed\",\"Note\"],[\"Row 1\",{\"text\":\"Wide merged area\",\"rowSpan\":2,\"colSpan\":2},\"\",\"A\",\"Single\"],[\"Row 2\",\"\",\"\",\"B\",{\"text\":\"Tall\",\"rowSpan\":2}],[\"Row 3\",\"Plain\",\"Cell\",\"C\",\"\"],[\"Row 4\",\"Plain\",\"Cell\",\"D\",\"Cell\"]]}]},"
			"{\"type\":\"column\",\"id\":\"panel_color\",\"style\":\"panel\",\"children\":["
			"{\"type\":\"label\",\"id\":\"title_color\",\"style\":\"title\",\"text\":\"XSON colors and compact scrollbar\"},"
			"{\"type\":\"tableView\",\"id\":\"table_color\",\"style\":\"table\",\"selectionMode\":\"cell\",\"selectedCell\":[2,2],\"rowColor\":\"#F2F7F7FF\",\"selectedColor\":\"#CDEDE5FF\",\"thumbColor\":\"#43A77CFF\","
			"\"columns\":[{\"title\":\"Project\",\"width\":110},{\"title\":\"Area\",\"width\":90},{\"title\":\"Status\",\"width\":118},{\"title\":\"Progress\",\"width\":92,\"align\":\"right\"}],"
			"\"rows\":[{\"height\":28},{\"height\":28},{\"height\":28},{\"height\":28}],"
			"\"cells\":[[\"Atlas\",\"Design\",\"Published\",\"88%\"],[\"Beacon\",\"Build\",\"Draft\",{\"text\":\"41%\",\"dirty\":true}],[\"Comet\",\"QA\",{\"text\":\"Blocked\",\"invalid\":true},\"12%\"],[\"Delta\",\"Ops\",\"Published\",{\"text\":\"100%\",\"editing\":true}]]}]}]}}") != XGE_OK ) {
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
		printf("xui_tableview_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	xge_xui_widget pBasic;
	xge_xui_widget pLarge;
	xge_xui_widget pMerge;
	xge_xui_widget pColor;
	xge_rect_t tRect;
	float fScrollX;
	float fScrollY;
	int iRow;
	int iColumn;

	pBasic = xgeXuiPageFind(&pApp->tPage, "table_basic");
	pLarge = xgeXuiPageFind(&pApp->tPage, "table_large");
	pMerge = xgeXuiPageFind(&pApp->tPage, "table_merge");
	pColor = xgeXuiPageFind(&pApp->tPage, "table_color");
	pApp->bCreateOK = (pBasic != NULL) && (pLarge != NULL) && (pMerge != NULL) && (pColor != NULL) && (pApp->tPage.iTableViewCount == 4);
	pApp->bLayoutOK = (pBasic != NULL) && (pBasic->tRect.fW > 300.0f) && (pColor != NULL) && (pColor->tRect.fH > 180.0f);
	if ( pApp->tPage.iTableViewCount >= 4 ) {
		xgeXuiTableViewGetOffset(&pApp->tPage.arrTableView[1], &fScrollX, &fScrollY);
		xgeXuiTableViewGetSelectedCell(&pApp->tPage.arrTableView[2], &iRow, &iColumn);
		pApp->bStateOK =
			(xgeXuiTableViewGetSelectionMode(&pApp->tPage.arrTableView[0]) == XGE_XUI_TABLE_VIEW_SELECTION_ROW) &&
			(xgeXuiTableViewGetSelectedRow(&pApp->tPage.arrTableView[0]) == 1) &&
			(xgeXuiTableViewGetRowCount(&pApp->tPage.arrTableView[1]) == XSON_LARGE_ROWS) &&
			(fScrollX > 0.0f) &&
			(fScrollY > 0.0f) &&
			(iRow == 1) &&
			(iColumn == 1) &&
			xgeXuiTableViewGetCellRect(&pApp->tPage.arrTableView[0], 1, 1, &tRect) &&
			(tRect.fW > 20.0f) &&
			(xgeXuiTableViewGetScrollbarMode(&pApp->tPage.arrTableView[1]) == XGE_XUI_SCROLLBAR_MODE_FULL);
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
		printf("xui_tableview_xson final-summary frames=%d create=%d layout=%d state=%d tableCount=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->tPage.iTableViewCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABLEVIEW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 980;
	tDesc.iHeight = 700;
	tDesc.sTitle = "XUI TableView XSON";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		printf("xui_tableview_xson xgeInit failed\n");
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
