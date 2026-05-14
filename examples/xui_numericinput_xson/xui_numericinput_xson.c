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
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bStepOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[22,18,22,18],\"gap\":12,\"background\":\"#E8F1F8FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#26384AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"grid\":{\"type\":\"grid\",\"width\":\"100%\",\"height\":240,\"columns\":2,\"rowHeight\":44,\"columnGap\":22,\"rowGap\":12},"
"\"num\":{\"type\":\"numericInput\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":32,\"padding\":[8,5,8,5],\"backgroundColor\":\"#F7FBFFFF\",\"textColor\":\"#233446FF\",\"focusColor\":\"#D4ECFCFF\",\"cursorColor\":\"#205C90FF\"}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"style\":\"title\",\"text\":\"NumericInput XSON\"},"
"{\"type\":\"grid\",\"id\":\"grid\",\"style\":\"grid\",\"children\":["
"{\"type\":\"numericInput\",\"id\":\"int\",\"style\":\"num\",\"integer\":true,\"min\":-10,\"max\":10,\"step\":2,\"value\":4,\"placeholder\":\"Integer\"},"
"{\"type\":\"numericInput\",\"id\":\"float\",\"style\":\"num\",\"min\":-1,\"max\":1,\"step\":0.25,\"precision\":2,\"value\":0.5,\"spinnerWidth\":28},"
"{\"type\":\"numericInput\",\"id\":\"hidden\",\"style\":\"num\",\"min\":0,\"max\":10,\"step\":1,\"value\":3,\"spinner\":false},"
"{\"type\":\"numericInput\",\"id\":\"readonly\",\"style\":\"num\",\"value\":7,\"readonly\":true},"
"{\"type\":\"numericInput\",\"id\":\"disabled\",\"style\":\"num\",\"value\":5,\"disabled\":true},"
"{\"type\":\"numericInput\",\"id\":\"bound\",\"style\":\"num\",\"integer\":true,\"min\":0,\"max\":4,\"step\":1,\"value\":0}"
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

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_numericinput_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_numericinput_xson font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int CreateUI(app_state_t* pApp)
{
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "body", &pApp->tFont);
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_numericinput_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pIntWidget;
	xge_xui_widget pFloatWidget;
	xge_xui_widget pHiddenWidget;
	xge_xui_widget pReadonlyWidget;
	xge_xui_widget pDisabledWidget;
	xge_xui_widget pBoundWidget;
	xge_xui_numeric_input pInt;
	xge_xui_numeric_input pFloat;
	xge_xui_numeric_input pHidden;
	xge_xui_numeric_input pReadonly;
	xge_xui_numeric_input pDisabled;
	xge_xui_numeric_input pBound;
	xge_event_t tEvent;

	pIntWidget = xgeXuiPageFind(&pApp->tPage, "int");
	pFloatWidget = xgeXuiPageFind(&pApp->tPage, "float");
	pHiddenWidget = xgeXuiPageFind(&pApp->tPage, "hidden");
	pReadonlyWidget = xgeXuiPageFind(&pApp->tPage, "readonly");
	pDisabledWidget = xgeXuiPageFind(&pApp->tPage, "disabled");
	pBoundWidget = xgeXuiPageFind(&pApp->tPage, "bound");
	pInt = (pIntWidget != NULL) ? (xge_xui_numeric_input)pIntWidget->pUser : NULL;
	pFloat = (pFloatWidget != NULL) ? (xge_xui_numeric_input)pFloatWidget->pUser : NULL;
	pHidden = (pHiddenWidget != NULL) ? (xge_xui_numeric_input)pHiddenWidget->pUser : NULL;
	pReadonly = (pReadonlyWidget != NULL) ? (xge_xui_numeric_input)pReadonlyWidget->pUser : NULL;
	pDisabled = (pDisabledWidget != NULL) ? (xge_xui_numeric_input)pDisabledWidget->pUser : NULL;
	pBound = (pBoundWidget != NULL) ? (xge_xui_numeric_input)pBoundWidget->pUser : NULL;

	pApp->bCreateOK = (pInt != NULL) && (pFloat != NULL) && (pHidden != NULL) && (pReadonly != NULL) && (pDisabled != NULL) && (pBound != NULL);
	pApp->bLayoutOK = (pIntWidget != NULL) && (pIntWidget->tRect.fW > 100.0f) && (pFloatWidget != NULL) && (pFloatWidget->tRect.fH >= 30.0f);
	pApp->bStateOK =
		(pInt != NULL) && (pInt->bInteger != 0) && (pInt->fMin == -10.0f) && (pInt->fMax == 10.0f) && (pInt->fStep == 2.0f) && (xgeXuiNumericInputGetValue(pInt) == 4.0f) &&
		(pFloat != NULL) && (pFloat->iPrecision == 2) && (pFloat->fSpinnerWidth == 28.0f) && (strcmp(xgeXuiInputGetText(&pFloat->tInput), "0.50") == 0) &&
		(pHidden != NULL) && (pHidden->bShowSpinner == 0) &&
		(pReadonly != NULL) && (pReadonly->tInput.bReadonly != 0) &&
		(pDisabled != NULL) && (pDisabled->tInput.bDisabled != 0) &&
		(pBound != NULL) && (pBound->fValue == 0.0f);

	if ( pInt != NULL && pIntWidget != NULL ) {
		xgeXuiSetFocus(&pApp->tXui, pIntWidget);
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_KEY_DOWN;
		tEvent.iParam1 = XGE_KEY_UP;
		pApp->bStepOK = (xgeXuiDispatchEvent(&pApp->tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED) && (xgeXuiNumericInputGetValue(pInt) == 6.0f);
	}
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
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
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_numericinput_xson final-summary frames=%d create=%d layout=%d state=%d step=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			pApp->bStepOK);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 241, 248, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_NUMERICINPUT_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XUI NumericInput XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bStepOK) ? 0 : 3;
}
