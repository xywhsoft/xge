#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[20,20,20,20],\"gap\":12,\"background\":\"#E5EBF4FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":32,\"textColor\":\"#26486CFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"body\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":\"100%\",\"columns\":2,\"columnGap\":12,\"rowGap\":12,\"background\":\"#00000000\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":24,\"textColor\":\"#303A48FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"hint\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":42,\"textColor\":\"#566980FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"button\":{\"type\":\"button\",\"font\":\"@fonts.body\",\"height\":32,\"color\":\"#387ECCFF\",\"hoverColor\":\"#4692DEFF\",\"activeColor\":\"#2460AAFF\",\"textColor\":\"#F8FCFFFF\"},"
"\"split\":{\"type\":\"splitLayout\",\"width\":\"100%\",\"height\":\"100%\",\"background\":\"#F4F8FCFF\",\"borderColor\":\"#5C9ADEFF\",\"borderWidth\":1,\"dividerSize\":10,\"dividerVisualSize\":3,\"dividerHitSize\":14}"
"},"
"\"tree\":{\"type\":\"column\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"title\",\"text\":\"SplitLayout XSON: panes[].children attach to internal pane widgets\"},"
"{\"type\":\"grid\",\"style\":\"body\",\"children\":["
"{\"type\":\"splitLayout\",\"style\":\"split\",\"orientation\":\"vertical\",\"shadowDrag\":true,"
"\"panes\":["
"{\"mode\":\"fixed\",\"size\":120,\"minSize\":100,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#E8F1FAFF\",\"borderColor\":\"#A6BCD6FF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Fixed left pane\"},"
"{\"type\":\"label\",\"style\":\"hint\",\"text\":\"120 px fixed\"}"
"]},"
"{\"mode\":\"grow\",\"weight\":1,\"minSize\":100,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#F8FBFFFF\",\"borderColor\":\"#A6BCD6FF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Grow content pane\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Child button\"},"
"{\"type\":\"label\",\"style\":\"hint\",\"text\":\"Divider visual is thinner than the hit area.\"}"
"]},"
"{\"mode\":\"fixed\",\"size\":130,\"minSize\":110,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#EEF6FAFF\",\"borderColor\":\"#A6BCD6FF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Fixed inspector\"},"
"{\"type\":\"label\",\"style\":\"hint\",\"text\":\"Shadow drag is enabled.\"}"
"]}"
"]},"
"{\"type\":\"splitLayout\",\"style\":\"split\",\"orientation\":\"horizontal\",\"shadowDrag\":false,\"dividerSize\":9,\"dividerVisualSize\":4,\"dividerHitSize\":13,\"dividerColor\":\"#DE8430D2\",\"dividerHoverColor\":\"#EE9E46EB\",\"dividerActiveColor\":\"#C4601EF5\","
"\"panes\":["
"{\"mode\":\"fixed\",\"size\":74,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#FEF6E8FF\",\"borderColor\":\"#D8B68AFF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Fixed top pane\"}"
"]},"
"{\"mode\":\"grow\",\"weight\":1,\"minSize\":100,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#FFFCF6FF\",\"borderColor\":\"#D8B68AFF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Live drag pane\"},"
"{\"type\":\"button\",\"style\":\"button\",\"text\":\"Apply layout\"}"
"]},"
"{\"mode\":\"fixed\",\"size\":92,\"layout\":\"column\",\"padding\":[12,10,12,10],\"gap\":7,\"background\":\"#FCF0E0FF\",\"borderColor\":\"#D8B68AFF\",\"borderWidth\":1,\"children\":["
"{\"type\":\"label\",\"style\":\"label\",\"text\":\"Fixed bottom\"}"
"]}"
"]}"
"]}"
"]}}";

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
			printf("xui_split_layout_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_split_layout_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	pRoot = xgeXuiRoot(&pApp->tXui);
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
	xge_xui_split_layout pVertical;
	xge_xui_split_layout pHorizontal;

	pVertical = pApp->tPage.arrSplitLayout[0];
	pHorizontal = pApp->tPage.arrSplitLayout[1];
	pApp->bCreateOK = (pApp->tPage.iSplitLayoutCount == 2) && (pApp->tPage.iLabelCount >= 9) && (pApp->tPage.iButtonCount == 2);
	pApp->bLayoutOK = (pVertical != NULL) && (pHorizontal != NULL) &&
		(xgeXuiSplitLayoutGetPaneSize(pVertical, 0) > 0.0f) &&
		(xgeXuiSplitLayoutGetPaneSize(pHorizontal, 1) > 0.0f);
	pApp->bStateOK = (pVertical != NULL) && (pHorizontal != NULL) &&
		(xgeXuiSplitLayoutGetPaneMode(pVertical, 0) == XGE_XUI_SPLIT_PANE_FIXED) &&
		(xgeXuiSplitLayoutGetPaneMode(pVertical, 1) == XGE_XUI_SPLIT_PANE_GROW) &&
		(pVertical->fDividerVisualSize < pVertical->fDividerHitSize) &&
		(pHorizontal->bShadowDrag == 0);
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
		printf("xui_split_layout_xson final-summary frames=%d create=%d layout=%d state=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SPLIT_LAYOUT_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 580;
	tDesc.sTitle = "XUI SplitLayout XSON";
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	memset(&tApp.tScene, 0, sizeof(tApp.tScene));
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	tApp.tScene.pUser = &tApp;
	xgeSceneSet(&tApp.tScene);
	iExitCode = (xgeRun(NULL, NULL) == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 2;
	xgeSceneSet(NULL);
	xgeUnit();
	return iExitCode;
}
