#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK_COUNT 4
#define RADIO_COUNT 4
#define TOGGLE_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bStateOK;
	int bInteractionOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":760,\"height\":560,\"background\":\"#E8F0F8FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":650,\"height\":26,\"textColor\":\"#36506AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"choice\":{\"font\":\"@fonts.body\",\"width\":158,\"height\":34,\"textColor\":\"#425066FF\"},"
"\"check\":{\"@parent\":\"choice\",\"type\":\"checkbox\",\"boxColor\":\"#FFFFFFFF\",\"checkedColor\":\"#2F7ED8FF\",\"indicatorSize\":14,\"gap\":7},"
"\"radio\":{\"@parent\":\"choice\",\"type\":\"radio\",\"ringColor\":\"#5A7896FF\",\"checkedColor\":\"#2F7ED8FF\",\"indicatorSize\":14,\"gap\":7},"
"\"toggle\":{\"type\":\"toggle\",\"width\":118,\"height\":34,\"font\":\"@fonts.body\",\"trackWidth\":58,\"trackHeight\":24,\"knobInset\":3,\"textPadding\":7,\"textGap\":4,\"trackColor\":\"#D7E0EAFF\",\"checkedColor\":\"#2F7ED8FF\",\"knobColor\":\"#FFFFFFFF\",\"trackBorderColor\":\"#7FAFD8FF\",\"uncheckedTextColor\":\"#5A6C7DFF\",\"checkedTextColor\":\"#FFFFFFFF\"},"
"\"customCheck\":{\"@parent\":\"check\",\"boxColor\":\"#FFF8E4FF\",\"checkedColor\":\"#D88A2FFF\",\"textColor\":\"#6E4C20FF\",\"indicatorSize\":20,\"gap\":10},"
"\"customRadio\":{\"@parent\":\"radio\",\"ringColor\":\"#8C6BB1FF\",\"checkedColor\":\"#7A4FB3FF\",\"textColor\":\"#4F3670FF\",\"indicatorSize\":20,\"gap\":10},"
"\"customToggle\":{\"@parent\":\"toggle\",\"trackWidth\":76,\"trackHeight\":28,\"knobInset\":3,\"textPadding\":9,\"trackColor\":\"#C9D8EAFF\",\"checkedColor\":\"#2FA86EFF\",\"trackBorderColor\":\"#4C9874FF\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"title0\",\"style\":\"title\",\"x\":26,\"y\":24,\"text\":\"CheckBox: unchecked, checked, disabled, custom metrics/colors\"},"
"{\"type\":\"checkbox\",\"id\":\"c0\",\"style\":\"check\",\"x\":42,\"y\":64,\"text\":\"Unchecked\"},"
"{\"type\":\"checkbox\",\"id\":\"c1\",\"style\":\"check\",\"x\":210,\"y\":64,\"text\":\"Checked\",\"checked\":true},"
"{\"type\":\"checkbox\",\"id\":\"c2\",\"style\":\"check\",\"x\":378,\"y\":64,\"text\":\"Disabled\",\"checked\":true,\"enabled\":false},"
"{\"type\":\"checkbox\",\"id\":\"c3\",\"style\":\"customCheck\",\"x\":546,\"y\":64,\"text\":\"Custom\"},"
"{\"type\":\"label\",\"id\":\"title1\",\"style\":\"title\",\"x\":26,\"y\":122,\"text\":\"Radio: value, checked, disabled, custom metrics/colors\"},"
"{\"type\":\"radio\",\"id\":\"r0\",\"style\":\"radio\",\"x\":42,\"y\":162,\"text\":\"Option A\",\"value\":1},"
"{\"type\":\"radio\",\"id\":\"r1\",\"style\":\"radio\",\"x\":210,\"y\":162,\"text\":\"Option B\",\"value\":2,\"checked\":true},"
"{\"type\":\"radio\",\"id\":\"r2\",\"style\":\"radio\",\"x\":378,\"y\":162,\"text\":\"Disabled\",\"value\":3,\"checked\":true,\"enabled\":false},"
"{\"type\":\"radio\",\"id\":\"r3\",\"style\":\"customRadio\",\"x\":546,\"y\":162,\"text\":\"Custom\",\"value\":4},"
"{\"type\":\"label\",\"id\":\"title2\",\"style\":\"title\",\"x\":26,\"y\":220,\"text\":\"Toggle: off/on, disabled, no text, custom colors\"},"
"{\"type\":\"toggle\",\"id\":\"t0\",\"style\":\"toggle\",\"x\":42,\"y\":260,\"uncheckedText\":\"OFF\",\"checkedText\":\"ON\"},"
"{\"type\":\"toggle\",\"id\":\"t1\",\"style\":\"toggle\",\"x\":168,\"y\":260,\"uncheckedText\":\"OFF\",\"checkedText\":\"ON\",\"checked\":true},"
"{\"type\":\"toggle\",\"id\":\"t2\",\"style\":\"toggle\",\"x\":294,\"y\":260,\"uncheckedText\":\"OFF\",\"checkedText\":\"ON\",\"checked\":true,\"enabled\":false},"
"{\"type\":\"toggle\",\"id\":\"t3\",\"style\":\"toggle\",\"x\":420,\"y\":260,\"uncheckedText\":\"\",\"checkedText\":\"\"},"
"{\"type\":\"toggle\",\"id\":\"t4\",\"style\":\"customToggle\",\"x\":546,\"y\":260,\"uncheckedText\":\"NO\",\"checkedText\":\"YES\",\"checked\":true},"
"{\"type\":\"toggle\",\"id\":\"t5\",\"style\":\"customToggle\",\"x\":42,\"y\":318,\"uncheckedText\":\"LOW\",\"checkedText\":\"HIGH\"},"
"{\"type\":\"label\",\"id\":\"note\",\"style\":\"title\",\"x\":26,\"y\":390,\"width\":700,\"text\":\"XSON creates checkbox/radio/toggle controls and covers checked, disabled, custom color, and metrics states.\"}"
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
			printf("xui_choice_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouse(xge_event_t* pEvent, int iType, xge_rect_t tRect)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = tRect.fX + tRect.fW * 0.5f;
	pEvent->fY = tRect.fY + tRect.fH * 0.5f;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->tPage.iCheckBoxCount == CHECK_COUNT) &&
		(pApp->tPage.iRadioCount == RADIO_COUNT) &&
		(pApp->tPage.iToggleCount == TOGGLE_COUNT) &&
		(xgeXuiPageFind(&pApp->tPage, "c3") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "r3") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "t5") != NULL);
	pApp->bStateOK = pApp->bCreateOK &&
		(xgeXuiCheckBoxGetChecked(&pApp->tPage.arrCheckBox[0]) == 0) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tPage.arrCheckBox[1]) != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrCheckBox[2].pWidget) == 0) &&
		(pApp->tPage.arrCheckBox[3].fIndicatorSize >= 19.0f) &&
		(pApp->tPage.arrCheckBox[3].iColorChecked == XGE_COLOR_RGBA(216, 138, 47, 255)) &&
		(xgeXuiRadioGetChecked(&pApp->tPage.arrRadio[1]) != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrRadio[2].pWidget) == 0) &&
		(pApp->tPage.arrRadio[3].iValue == 4) &&
		(pApp->tPage.arrRadio[3].fIndicatorSize >= 19.0f) &&
		(xgeXuiToggleGetChecked(&pApp->tPage.arrToggle[1]) != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrToggle[2].pWidget) == 0) &&
		(pApp->tPage.arrToggle[3].sCheckedText != NULL) &&
		(pApp->tPage.arrToggle[3].sCheckedText[0] == 0) &&
		(pApp->tPage.arrToggle[4].fTrackWidth >= 75.0f) &&
		(pApp->tPage.arrToggle[4].iColorChecked == XGE_COLOR_RGBA(47, 168, 110, 255));
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tPage.arrCheckBox[0].pWidget->tRect);
	xgeXuiCheckBoxEvent(&pApp->tPage.arrCheckBox[0], &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tPage.arrCheckBox[0].pWidget->tRect);
	xgeXuiCheckBoxEvent(&pApp->tPage.arrCheckBox[0], &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tPage.arrToggle[0].pWidget->tRect);
	xgeXuiToggleEvent(&pApp->tPage.arrToggle[0], &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tPage.arrToggle[0].pWidget->tRect);
	xgeXuiToggleEvent(&pApp->tPage.arrToggle[0], &tEvent);
	pApp->bInteractionOK =
		(xgeXuiCheckBoxGetChecked(&pApp->tPage.arrCheckBox[0]) != 0) &&
		(xgeXuiToggleGetChecked(&pApp->tPage.arrToggle[0]) != 0);
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_choice_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	RunChecks(pApp);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
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
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
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
		printf("xui_choice_xson final-summary frames=%d create=%d state=%d interaction=%d checks=%d radios=%d toggles=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->tPage.iCheckBoxCount,
			pApp->tPage.iRadioCount,
			pApp->tPage.iToggleCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 240, 248, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_CHOICE_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Choice XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bStateOK && tApp.bInteractionOK) ? 0 : 3;
}
