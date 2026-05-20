#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XSON_BUFFER_SIZE 32768

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	char sXson[XSON_BUFFER_SIZE];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_propertygrid_xson font loaded: %s\n", arrFonts[i]);
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

static int BuildXson(app_state_t* pApp)
{
	int iOffset;

	iOffset = 0;
	pApp->sXson[0] = 0;
	return AppendText(
		pApp->sXson,
		&iOffset,
		XSON_BUFFER_SIZE,
		"{\"xui\":1,"
		"\"styles\":{"
		"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"gap\":8,\"background\":\"#ECF0F6FF\"},"
		"\"panel\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[12,10,12,12],\"gap\":8,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1,\"radius\":4},"
		"\"title\":{\"height\":28,\"font\":\"@fonts.default\",\"textColor\":\"#2A3A4EFF\"},"
		"\"grid\":{\"width\":\"100%\",\"height\":\"grow\",\"font\":\"@fonts.default\",\"nameWidth\":165,\"rowHeight\":25,\"categoryHeight\":27,\"backgroundColor\":\"#F8FAFDFF\",\"gridColor\":\"#B8D3E8FF\",\"categoryColor\":\"#DAEBF9FF\",\"categoryHoverColor\":\"#CBE2F5FF\",\"categoryTextColor\":\"#28547EFF\",\"nameColor\":\"#EBF2F9FF\",\"nameTextColor\":\"#3A4D60FF\",\"valueColor\":\"#F8FAFDFF\",\"valueTextColor\":\"#2A3442FF\",\"selectedColor\":\"#BEDBF2FF\",\"readonlyTextColor\":\"#74808EFF\"}"
		"},"
		"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
		"{\"type\":\"column\",\"id\":\"panel\",\"style\":\"panel\",\"children\":["
		"{\"type\":\"label\",\"id\":\"title\",\"style\":\"title\",\"text\":\"XSON PropertyGrid: one-level categories and TableGrid editors\"},"
		"{\"type\":\"propertyGrid\",\"id\":\"props\",\"style\":\"grid\",\"selected\":\"appearance.accent\",\"categories\":["
		"{\"id\":\"appearance\",\"name\":\"Appearance\",\"expanded\":true,\"properties\":["
		"{\"id\":\"window.title\",\"name\":\"Title\",\"description\":\"Text property loaded from XSON\",\"type\":\"text\",\"value\":\"Inspector\",\"defaultValue\":\"Inspector\"},"
		"{\"id\":\"appearance.enabled\",\"name\":\"Enabled\",\"description\":\"Bool uses default checkbox display\",\"type\":\"bool\",\"value\":true,\"defaultValue\":false},"
		"{\"id\":\"appearance.accent\",\"name\":\"Accent Color\",\"description\":\"Color uses ColorPicker editor\",\"type\":\"color\",\"value\":\"#2E7CD6\",\"defaultValue\":\"#2E7CD6\"},"
		"{\"id\":\"appearance.theme\",\"name\":\"Theme\",\"description\":\"Enum options are declared in XSON\",\"type\":\"enum\",\"value\":\"Blue\",\"options\":[\"Blue\",\"Green\",\"Orange\",\"Purple\"]}"
		"]},"
		"{\"id\":\"runtime\",\"name\":\"Runtime\",\"expanded\":true,\"properties\":["
		"{\"id\":\"runtime.maxPlayers\",\"name\":\"Max Players\",\"type\":\"int\",\"value\":4,\"defaultValue\":4},"
		"{\"id\":\"runtime.opacity\",\"name\":\"Opacity\",\"type\":\"float\",\"value\":0.85,\"defaultValue\":1.0},"
		"{\"id\":\"runtime.notes\",\"name\":\"Notes\",\"description\":\"Textarea opens a popup editor\",\"type\":\"textarea\",\"value\":\"Line 1 summary\\nFull multiline description\"},"
		"{\"id\":\"asset.path\",\"name\":\"Asset Path\",\"description\":\"Picker only triggers callback in native examples\",\"type\":\"picker\",\"value\":\"Browse\"}"
		"]},"
		"{\"id\":\"states\",\"name\":\"States\",\"expanded\":true,\"properties\":["
		"{\"id\":\"state.readonly\",\"name\":\"Readonly\",\"type\":\"text\",\"value\":\"locked\",\"readonly\":true},"
		"{\"id\":\"state.disabled\",\"name\":\"Disabled\",\"type\":\"text\",\"value\":\"disabled\",\"disabled\":true},"
		"{\"id\":\"state.dirty\",\"name\":\"Dirty\",\"type\":\"text\",\"value\":\"changed\",\"defaultValue\":\"default\"},"
		"{\"id\":\"state.invalid\",\"name\":\"Invalid\",\"type\":\"text\",\"value\":\"bad value\",\"invalid\":true}"
		"]}"
		"]}"
		"]}"
		"]}}");
}

static int CreateUI(app_state_t* pApp)
{
	xge_font pFont;

	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "default", pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( BuildXson(pApp) != XGE_OK ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_propertygrid_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
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
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pWidget;
	xge_xui_property_grid pGrid;

	pWidget = xgeXuiPageFind(&pApp->tPage, "props");
	pGrid = (pWidget != NULL) ? (xge_xui_property_grid)pWidget->pUser : NULL;
	pApp->bCreateOK = (pWidget != NULL) && (pGrid != NULL) && (pGrid->iCategoryCount == 3) && (pGrid->iPropertyCount == 12);
	pApp->bLayoutOK = (pWidget != NULL) &&
		(pWidget->tRect.fW > 500.0f) &&
		(pWidget->tRect.fH > 300.0f) &&
		(pGrid != NULL) &&
		(pGrid->tGrid.tTable.fHeaderHeight == 0.0f) &&
		(pGrid->tGrid.iEditMode == XGE_XUI_TABLE_GRID_EDIT_QUICK) &&
		(pGrid->tGrid.tTable.pFrame != NULL) &&
		(pGrid->tGrid.tTable.pFrame->iScrollbarPolicyX == XGE_XUI_SCROLLBAR_POLICY_HIDDEN);
	pApp->bStateOK = (pGrid != NULL) &&
		(xgeXuiPropertyGridFindProperty(pGrid, "appearance.theme") >= 0) &&
		(xgeXuiPropertyGridFindProperty(pGrid, "state.invalid") >= 0) &&
		((xgeXuiPropertyGridGetPropertyFlags(pGrid, xgeXuiPropertyGridFindProperty(pGrid, "state.invalid")) & XGE_XUI_PROPERTY_FLAG_INVALID) != 0);
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
		printf("xui_propertygrid_xson final-summary frames=%d create=%d layout=%d state=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_PROPERTYGRID_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 600;
	tDesc.sTitle = "XUI PropertyGrid XSON";
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
