#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_COUNT 7

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
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":760,\"height\":560,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"width\":150,\"height\":28,\"font\":\"@fonts.body\",\"textColor\":\"#424E5EFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"input\":{\"type\":\"input\",\"width\":520,\"height\":32,\"padding\":[9,4,9,4],\"font\":\"@fonts.body\",\"backgroundColor\":\"#F8FAFDFF\",\"hoverBackgroundColor\":\"#F3F9FDFF\",\"borderColor\":\"#B8D2E6FF\",\"hoverBorderColor\":\"#8FBCDCAFF\",\"focusBorderColor\":\"#3593DAFF\",\"cursorColor\":\"#242A34FF\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"l0\",\"style\":\"label\",\"x\":32,\"y\":44,\"text\":\"Default\"},{\"type\":\"input\",\"id\":\"i0\",\"style\":\"input\",\"x\":190,\"y\":44,\"text\":\"Input text\"},"
"{\"type\":\"label\",\"id\":\"l1\",\"style\":\"label\",\"x\":32,\"y\":106,\"text\":\"Placeholder\"},{\"type\":\"input\",\"id\":\"i1\",\"style\":\"input\",\"x\":190,\"y\":106,\"placeholder\":\"Placeholder text\"},"
"{\"type\":\"label\",\"id\":\"l2\",\"style\":\"label\",\"x\":32,\"y\":168,\"text\":\"Password\"},{\"type\":\"input\",\"id\":\"i2\",\"style\":\"input\",\"x\":190,\"y\":168,\"text\":\"secret\",\"password\":true},"
"{\"type\":\"label\",\"id\":\"l3\",\"style\":\"label\",\"x\":32,\"y\":230,\"text\":\"Error event\"},{\"type\":\"input\",\"id\":\"i3\",\"style\":\"input\",\"x\":190,\"y\":230,\"text\":\"invalid value\",\"error\":true,\"errorBackgroundColor\":\"#FFF4F6FF\",\"errorBorderColor\":\"#DC4A54FF\"},"
"{\"type\":\"label\",\"id\":\"l4\",\"style\":\"label\",\"x\":32,\"y\":292,\"text\":\"Disabled\"},{\"type\":\"input\",\"id\":\"i4\",\"style\":\"input\",\"x\":190,\"y\":292,\"text\":\"disabled input\",\"disabled\":true,\"disabledTextColor\":\"#808C9AFF\",\"disabledBackgroundColor\":\"#E2EAF2FF\",\"disabledBorderColor\":\"#BCC6D0FF\"},"
"{\"type\":\"label\",\"id\":\"l5\",\"style\":\"label\",\"x\":32,\"y\":354,\"text\":\"Selection\"},{\"type\":\"input\",\"id\":\"i5\",\"style\":\"input\",\"x\":190,\"y\":354,\"text\":\"selected text\",\"selection\":[0,8],\"selectionColor\":\"#2E7CD660\"},"
"{\"type\":\"label\",\"id\":\"l6\",\"style\":\"label\",\"x\":32,\"y\":416,\"text\":\"Custom colors\"},{\"type\":\"input\",\"id\":\"i6\",\"style\":\"input\",\"x\":190,\"y\":416,\"text\":\"custom frame\",\"backgroundColor\":\"#FFFCF4FF\",\"hoverBackgroundColor\":\"#FFF8E8FF\",\"borderColor\":\"#D79142FF\",\"hoverBorderColor\":\"#E67E22FF\",\"focusBorderColor\":\"#D15B16FF\",\"textColor\":\"#50321EFF\",\"cursorColor\":\"#50321EFF\"}"
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
			printf("xui_input_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void InputErrorChange(xge_xui_widget pWidget, int bError, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp == NULL) || (pApp->tPage.iLabelCount <= 3) ) {
		return;
	}
	xgeXuiLabelSetText(&pApp->tPage.arrLabel[3], bError ? "Invalid value" : "Error event");
	xgeXuiLabelSetColor(&pApp->tPage.arrLabel[3], bError ? XGE_COLOR_RGBA(190, 54, 66, 255) : XGE_COLOR_RGBA(66, 78, 94, 255));
}

static void RunChecks(app_state_t* pApp)
{
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->tPage.iLabelCount == INPUT_COUNT) &&
		(pApp->tPage.iInputCount == INPUT_COUNT) &&
		(xgeXuiPageFind(&pApp->tPage, "l6") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "i6") != NULL);
	pApp->bLayoutOK = pApp->bCreateOK &&
		(pApp->tPage.arrLabel[0].pWidget->tRect.fX == 32.0f) &&
		(pApp->tPage.arrLabel[0].pWidget->tRect.fY == 44.0f) &&
		(pApp->tPage.arrInput[0].pWidget->tRect.fX == 190.0f) &&
		(pApp->tPage.arrInput[0].pWidget->tRect.fY == 44.0f) &&
		(pApp->tPage.arrInput[0].pWidget->tRect.fW == 520.0f) &&
		(pApp->tPage.arrInput[0].pWidget->tRect.fH == 32.0f) &&
		(pApp->tPage.arrInput[5].pWidget->tRect.fX == 190.0f) &&
		(pApp->tPage.arrInput[5].pWidget->tRect.fY == 354.0f) &&
		(pApp->tPage.arrInput[6].pWidget->tRect.fX == 190.0f) &&
		(pApp->tPage.arrInput[6].pWidget->tRect.fY == 416.0f) &&
		(pApp->tPage.arrInput[6].pWidget->tRect.fW == 520.0f) &&
		(pApp->tPage.arrInput[6].pWidget->tRect.fH == 32.0f);
	pApp->bStateOK = pApp->bCreateOK &&
		(pApp->tPage.arrInput[2].bPassword != 0) &&
		(pApp->tPage.arrInput[3].bError != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrInput[4].pWidget) == 0) &&
		(pApp->tPage.arrInput[6].iBorderColor == XGE_COLOR_RGBA(215, 145, 66, 255));
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_input_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	if ( pApp->tPage.iInputCount > 3 ) {
		xgeXuiInputSetErrorChange(&pApp->tPage.arrInput[3], InputErrorChange, pApp);
		InputErrorChange(pApp->tPage.arrInput[3].pWidget, xgeXuiInputGetError(&pApp->tPage.arrInput[3]), pApp);
	}
	xgeXuiSetFocus(&pApp->tXui, xgeXuiPageFind(&pApp->tPage, "i0"));
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
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_input_xson final-summary frames=%d create=%d layout=%d state=%d labels=%d inputs=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iLabelCount, pApp->tPage.iInputCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_INPUT_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Input XSON";
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
