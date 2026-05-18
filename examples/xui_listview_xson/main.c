#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 65536
#define XSON_LARGE_COUNT 128

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
			printf("xui_listview_xson font loaded: %s\n", arrFonts[i]);
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

static int AppendListItems(char* sBuffer, int* pOffset, const char** arrItems, const int* arrEnabled, int iCount)
{
	int i;

	for ( i = 0; i < iCount; i++ ) {
		if ( i > 0 && AppendText(sBuffer, pOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( arrEnabled != NULL && arrEnabled[i] == 0 ) {
			if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "{\"text\":\"%s\",\"enabled\":false}", arrItems[i]) != XGE_OK ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
		} else if ( AppendFormat(sBuffer, pOffset, XSON_BUFFER_SIZE, "\"%s\"", arrItems[i]) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	return XGE_OK;
}

static int BuildXson(app_state_t* pApp)
{
	static const char* arrBasic[] = {
		"Dashboard", "Inventory", "Crafting disabled", "Character",
		"Quest log", "Guild disabled", "Mail", "Settings"
	};
	static const int arrBasicEnabled[] = { 1, 1, 0, 1, 1, 0, 1, 1 };
	static const char* arrMulti[] = {
		"Iron ore", "Copper ore", "Silver ore", "Gold ore",
		"Oak plank", "Birch plank", "Ancient fiber", "Moon crystal"
	};
	static const char* arrStyled[] = {
		"Warning report", "Green deployment", "Blue archive", "Purple artifact",
		"Locked disabled row", "Ready item", "Queued item", "Completed item"
	};
	static const int arrStyledEnabled[] = { 1, 1, 1, 1, 0, 1, 1, 1 };
	int iOffset;
	int i;

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
			"\"list\":{\"width\":\"100%\",\"height\":\"grow\",\"padding\":2,\"itemHeight\":28,\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F4F8FCFF\",\"selectedColor\":\"#D1E6FAFF\",\"textColor\":\"#2A3442FF\",\"disabledTextColor\":\"#87909DB8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"},"
			"\"compactList\":{\"width\":\"100%\",\"height\":\"grow\",\"padding\":2,\"itemHeight\":24,\"scrollbarMode\":\"full\",\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F6F9FCFF\",\"selectedColor\":\"#D9EBFAFF\",\"textColor\":\"#2A3442FF\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#4A8ED2FF\"},"
			"\"accentList\":{\"width\":\"100%\",\"height\":\"grow\",\"padding\":2,\"itemHeight\":30,\"backgroundColor\":\"#F8FAFDFF\",\"rowColor\":\"#F2F7F7FF\",\"selectedColor\":\"#CDEDE5FF\",\"textColor\":\"#243442FF\",\"disabledTextColor\":\"#8A94A0B8\",\"barColor\":\"#E8EEF7FF\",\"thumbColor\":\"#43A77CFF\"}"
			"}," 
			"\"tree\":{\"type\":\"grid\",\"id\":\"root\",\"style\":\"root\",\"children\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"column\",\"id\":\"panel_basic\",\"style\":\"panel\",\"children\":[{\"type\":\"label\",\"id\":\"title_basic\",\"style\":\"title\",\"text\":\"Basic list: disabled rows and selected item\"},{\"type\":\"listView\",\"id\":\"basic\",\"style\":\"list\",\"selected\":1,\"items\":[") != XGE_OK ||
	     AppendListItems(pApp->sXson, &iOffset, arrBasic, arrBasicEnabled, (int)(sizeof(arrBasic) / sizeof(arrBasic[0]))) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}]},") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"column\",\"id\":\"panel_large\",\"style\":\"panel\",\"children\":[{\"type\":\"label\",\"id\":\"title_large\",\"style\":\"title\",\"text\":\"Large list through XSON: declared rows and scroll\"},{\"type\":\"listView\",\"id\":\"large\",\"style\":\"compactList\",\"selected\":42,\"scrollY\":720,\"items\":[") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	for ( i = 0; i < XSON_LARGE_COUNT; i++ ) {
		if ( i > 0 && AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, ",") != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		if ( AppendFormat(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "\"Declared item %03d\"", i + 1) != XGE_OK ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
	}
	if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}]},") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"column\",\"id\":\"panel_multi\",\"style\":\"panel\",\"children\":[{\"type\":\"label\",\"id\":\"title_multi\",\"style\":\"title\",\"text\":\"Range selection: selectedItems in XSON\"},{\"type\":\"listView\",\"id\":\"multi\",\"style\":\"list\",\"selectionMode\":\"range\",\"selectedItems\":[1,3,4],\"items\":[") != XGE_OK ||
	     AppendListItems(pApp->sXson, &iOffset, arrMulti, NULL, (int)(sizeof(arrMulti) / sizeof(arrMulti[0]))) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}]},") != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "{\"type\":\"column\",\"id\":\"panel_styled\",\"style\":\"panel\",\"children\":[{\"type\":\"label\",\"id\":\"title_styled\",\"style\":\"title\",\"text\":\"Styled list: colors, full scrollbar, disabled item\"},{\"type\":\"listView\",\"id\":\"styled\",\"style\":\"accentList\",\"scrollbarMode\":\"full\",\"selected\":2,\"items\":[") != XGE_OK ||
	     AppendListItems(pApp->sXson, &iOffset, arrStyled, arrStyledEnabled, (int)(sizeof(arrStyled) / sizeof(arrStyled[0]))) != XGE_OK ||
	     AppendText(pApp->sXson, &iOffset, XSON_BUFFER_SIZE, "]}]}]}}") != XGE_OK ) {
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
		printf("xui_listview_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 640.0f ) {
		fRootH = 640.0f;
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
	xge_xui_widget pMulti;
	xge_xui_widget pStyled;

	pBasic = xgeXuiPageFind(&pApp->tPage, "basic");
	pLarge = xgeXuiPageFind(&pApp->tPage, "large");
	pMulti = xgeXuiPageFind(&pApp->tPage, "multi");
	pStyled = xgeXuiPageFind(&pApp->tPage, "styled");
	pApp->bCreateOK =
		(pBasic != NULL) && (pBasic->procEvent == xgeXuiListViewEventProc) &&
		(pLarge != NULL) && (pLarge->procEvent == xgeXuiListViewEventProc) &&
		(pMulti != NULL) && (pMulti->procEvent == xgeXuiListViewEventProc) &&
		(pStyled != NULL) && (pStyled->procEvent == xgeXuiListViewEventProc) &&
		(pApp->tPage.iListViewCount == 4);
	pApp->bLayoutOK = (pBasic != NULL) && (pBasic->tRect.fW > 300.0f) && (pStyled != NULL) && (pStyled->tRect.fH > 180.0f);
	pApp->bStateOK =
		(pApp->tPage.iListViewCount == 4) &&
		(pApp->tPage.arrListView[0] != NULL) &&
		(pApp->tPage.arrListView[1] != NULL) &&
		(pApp->tPage.arrListView[2] != NULL) &&
		(pApp->tPage.arrListView[3] != NULL) &&
		(xgeXuiListViewGetSelected(pApp->tPage.arrListView[0]) == 1) &&
		(xgeXuiListViewGetSelected(pApp->tPage.arrListView[1]) == 42) &&
		(xgeXuiListViewGetSelectionMode(pApp->tPage.arrListView[2]) == XGE_XUI_SELECTION_RANGE) &&
		xgeXuiListViewIsItemSelected(pApp->tPage.arrListView[2], 3) &&
		(pApp->tPage.arrListView[3]->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL);
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
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_listview_xson final-summary frames=%d create=%d layout=%d state=%d listCount=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->tPage.iListViewCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_LISTVIEW_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 680;
	tDesc.sTitle = "XUI ListView XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
