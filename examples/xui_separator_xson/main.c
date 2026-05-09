#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_COUNT 17
#define SEPARATOR_COUNT 12

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
"\"root\":{\"type\":\"row\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":18,\"background\":\"#E5EBF4FF\",\"gap\":14},"
"\"left\":{\"type\":\"column\",\"width\":{\"unit\":\"grow\",\"value\":1.35},\"height\":\"grow\",\"padding\":14,\"gap\":8,\"background\":\"#F8FAFDFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1},"
"\"right\":{\"type\":\"column\",\"width\":\"grow\",\"height\":\"grow\",\"gap\":12},"
"\"group\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"grow\",\"padding\":10,\"gap\":10,\"background\":\"#F1F5FAFF\",\"borderColor\":\"#AAB8CAFF\",\"borderWidth\":1},"
"\"row\":{\"type\":\"row\",\"width\":\"100%\",\"height\":72,\"padding\":8,\"gap\":8,\"background\":\"#F8FAFDFF\"},"
"\"text\":{\"font\":\"@fonts.body\",\"textColor\":\"#303A48FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\",\"width\":\"grow\",\"height\":\"100%\",\"padding\":[6,5,6,5]},"
"\"hsep\":{\"width\":\"100%\"},"
"\"vsep\":{\"width\":14,\"height\":\"100%\"}"
"},"
"\"tree\":{\"type\":\"row\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"column\",\"id\":\"left\",\"style\":\"left\",\"children\":["
"{\"type\":\"label\",\"id\":\"p0\",\"style\":\"text\",\"height\":54,\"text\":\"A compact page reads better when related thoughts stay together.\"},"
"{\"type\":\"separator\",\"id\":\"h0\",\"style\":\"hsep\",\"height\":14,\"orientation\":\"horizontal\",\"lineStyle\":\"solid\",\"align\":\"center\",\"thickness\":1,\"color\":\"#506682BE\"},"
"{\"type\":\"label\",\"id\":\"p1\",\"style\":\"text\",\"height\":58,\"text\":\"The first break is quiet. It marks a pause without becoming a decoration.\"},"
"{\"type\":\"separator\",\"id\":\"h1\",\"style\":\"hsep\",\"height\":16,\"orientation\":\"horizontal\",\"lineStyle\":\"dot\",\"align\":\"center\",\"thickness\":2,\"color\":\"#2E7CD2D2\"},"
"{\"type\":\"label\",\"id\":\"p2\",\"style\":\"text\",\"height\":58,\"text\":\"A dotted break is lighter and works well between small notes or metadata rows.\"},"
"{\"type\":\"separator\",\"id\":\"h2\",\"style\":\"hsep\",\"height\":18,\"orientation\":\"horizontal\",\"lineStyle\":\"dash\",\"align\":\"start\",\"thickness\":3,\"color\":\"#D2842ADC\"},"
"{\"type\":\"label\",\"id\":\"p3\",\"style\":\"text\",\"height\":64,\"text\":\"A dashed break is stronger. Aligning it to the top edge shows the line is independent of the widget height.\"},"
"{\"type\":\"separator\",\"id\":\"h3\",\"style\":\"hsep\",\"height\":18,\"orientation\":\"horizontal\",\"lineStyle\":\"dashDot\",\"align\":\"end\",\"thickness\":2,\"color\":\"#5C74D2DC\"},"
"{\"type\":\"label\",\"id\":\"p4\",\"style\":\"text\",\"height\":\"grow\",\"text\":\"The last break sits on the lower edge, leaving the paragraph rhythm intact.\"}"
"]},"
"{\"type\":\"column\",\"id\":\"right\",\"style\":\"right\",\"children\":["
"{\"type\":\"column\",\"id\":\"group0\",\"style\":\"group\",\"children\":["
"{\"type\":\"row\",\"id\":\"r0\",\"style\":\"row\",\"children\":["
"{\"type\":\"label\",\"style\":\"text\",\"text\":\"Profile\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"solid\",\"align\":\"center\",\"thickness\":2,\"color\":\"#2C74BEDC\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Scope\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dot\",\"align\":\"center\",\"thickness\":2,\"color\":\"#D2842AE6\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Status\"}"
"]},"
"{\"type\":\"row\",\"id\":\"r1\",\"style\":\"row\",\"children\":["
"{\"type\":\"label\",\"style\":\"text\",\"text\":\"Author\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"solid\",\"align\":\"center\",\"thickness\":2,\"color\":\"#2C74BEDC\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Review\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dot\",\"align\":\"center\",\"thickness\":2,\"color\":\"#D2842AE6\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Ready\"}"
"]}"
"]},"
"{\"type\":\"column\",\"id\":\"group1\",\"style\":\"group\",\"children\":["
"{\"type\":\"row\",\"id\":\"r2\",\"style\":\"row\",\"children\":["
"{\"type\":\"label\",\"style\":\"text\",\"text\":\"Plan\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dash\",\"align\":\"center\",\"thickness\":2,\"color\":\"#2C74BEDC\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Build\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dashDot\",\"align\":\"center\",\"thickness\":2,\"color\":\"#D2842AE6\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Verify\"}"
"]},"
"{\"type\":\"row\",\"id\":\"r3\",\"style\":\"row\",\"children\":["
"{\"type\":\"label\",\"style\":\"text\",\"text\":\"Input\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dash\",\"align\":\"center\",\"thickness\":2,\"color\":\"#2C74BEDC\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Output\"},{\"type\":\"separator\",\"style\":\"vsep\",\"orientation\":\"vertical\",\"lineStyle\":\"dashDot\",\"align\":\"center\",\"thickness\":2,\"color\":\"#D2842AE6\"},{\"type\":\"label\",\"style\":\"text\",\"text\":\"Trace\"}"
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
			printf("xui_separator_xson font loaded: %s\n", arrFonts[i]);
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
		printf("xui_separator_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
	if ( fRootW < 900.0f ) {
		fRootW = 900.0f;
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
	xge_xui_widget pLeft;
	xge_xui_widget pSep;
	xge_xui_separator pControl;

	pLeft = xgeXuiPageFind(&pApp->tPage, "left");
	pSep = xgeXuiPageFind(&pApp->tPage, "h3");
	pControl = (pSep != NULL) ? (xge_xui_separator)pSep->pUser : NULL;
	pApp->bCreateOK = (pApp->tPage.iLabelCount == LABEL_COUNT) && (pApp->tPage.iSeparatorCount == SEPARATOR_COUNT);
	pApp->bLayoutOK = (pLeft != NULL) && (pLeft->tRect.fW > 300.0f);
	pApp->bStateOK = (pControl != NULL) && (pControl->iLineStyle == XGE_XUI_SEPARATOR_DASH_DOT) && (pControl->iAlign == XGE_XUI_ALIGN_END);
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
	return xgeXuiDispatchEvent(&pApp->tXui, pEvent);
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
		printf("xui_separator_xson final-summary frames=%d create=%d layout=%d state=%d labels=%d separators=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iLabelCount, pApp->tPage.iSeparatorCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SEPARATOR_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 1000;
	tDesc.iHeight = 640;
	tDesc.sTitle = "XUI Separator XSON";
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
