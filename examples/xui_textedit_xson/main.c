#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXTEDIT_COUNT 4

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
"\"root\":{\"type\":\"absolute\",\"width\":900,\"height\":620,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"width\":390,\"height\":24,\"font\":\"@fonts.body\",\"textColor\":\"#424E5EFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"edit\":{\"type\":\"textEdit\",\"width\":390,\"height\":190,\"padding\":[8,6,8,6],\"font\":\"@fonts.body\",\"backgroundColor\":\"#F8FAFDFF\",\"hoverBackgroundColor\":\"#F3F9FDFF\",\"borderColor\":\"#B8D2E6FF\",\"hoverBorderColor\":\"#8FBCDCAFF\",\"focusBorderColor\":\"#3593DAFF\",\"cursorColor\":\"#242A34FF\",\"selectionColor\":\"#2E7CD660\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"l0\",\"style\":\"label\",\"x\":30,\"y\":42,\"text\":\"Default multi-line editor\"},{\"type\":\"textEdit\",\"id\":\"e0\",\"style\":\"edit\",\"x\":30,\"y\":72,\"text\":\"alpha\\nbeta\\ngamma\\n\\nThe editor keeps IME and selection behavior.\"},"
"{\"type\":\"label\",\"id\":\"l1\",\"style\":\"label\",\"x\":450,\"y\":42,\"text\":\"Line numbers and wrap\"},{\"type\":\"textEdit\",\"id\":\"e1\",\"style\":\"edit\",\"x\":450,\"y\":72,\"text\":\"1. wrapped text keeps paragraphs readable inside the control.\\n2. line numbers use a separate background.\\n3. colors are loaded from XSON.\",\"wordWrap\":true,\"lineNumbers\":true,\"lineNumberWidth\":40,\"currentLineColor\":\"#FFF6C278\",\"lineNumberTextColor\":\"#5C768EFF\",\"lineNumberBackgroundColor\":\"#E8F2FAFF\"},"
"{\"type\":\"label\",\"id\":\"l2\",\"style\":\"label\",\"x\":30,\"y\":300,\"text\":\"Custom color palette\"},{\"type\":\"textEdit\",\"id\":\"e2\",\"style\":\"edit\",\"x\":30,\"y\":330,\"text\":\"Custom palette\\nBlue border\\nSoft current line\\nGreen scrollbar thumb\",\"backgroundColor\":\"#F6FCFFFF\",\"hoverBackgroundColor\":\"#EEF8FFFF\",\"borderColor\":\"#5B99CAFF\",\"hoverBorderColor\":\"#3785C4FF\",\"focusBorderColor\":\"#2173BAFF\",\"currentLineColor\":\"#E2F3FFB4\",\"scrollbarThumbColor\":\"#37945CF5\"},"
"{\"type\":\"label\",\"id\":\"l3\",\"style\":\"label\",\"x\":450,\"y\":300,\"text\":\"Readonly disabled style\"},{\"type\":\"textEdit\",\"id\":\"e3\",\"style\":\"edit\",\"x\":450,\"y\":330,\"text\":\"Readonly content\\nDisabled text uses a quieter color.\",\"readonly\":true,\"disabled\":true,\"disabledTextColor\":\"#808C9AFF\",\"disabledBackgroundColor\":\"#E2EAF2FF\",\"disabledBorderColor\":\"#BCC6D0FF\"}"
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
			printf("xui_textedit_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void RunChecks(app_state_t* pApp)
{
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pApp->tPage.iLabelCount == TEXTEDIT_COUNT) &&
		(pApp->tPage.iTextEditCount == TEXTEDIT_COUNT) &&
		(xgeXuiPageFind(&pApp->tPage, "l3") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "e3") != NULL);
	pApp->bLayoutOK = pApp->bCreateOK &&
		(pApp->tPage.arrLabel[0].pWidget->tRect.fX == 30.0f) &&
		(pApp->tPage.arrLabel[0].pWidget->tRect.fY == 42.0f) &&
		(pApp->tPage.arrTextEdit[0].pWidget->tRect.fX == 30.0f) &&
		(pApp->tPage.arrTextEdit[0].pWidget->tRect.fY == 72.0f) &&
		(pApp->tPage.arrTextEdit[0].pWidget->tRect.fW == 390.0f) &&
		(pApp->tPage.arrTextEdit[0].pWidget->tRect.fH == 190.0f) &&
		(pApp->tPage.arrLabel[1].pWidget->tRect.fX == 450.0f) &&
		(pApp->tPage.arrLabel[1].pWidget->tRect.fY == 42.0f) &&
		(pApp->tPage.arrTextEdit[1].pWidget->tRect.fX == 450.0f) &&
		(pApp->tPage.arrTextEdit[1].pWidget->tRect.fY == 72.0f) &&
		(pApp->tPage.arrTextEdit[2].pWidget->tRect.fX == 30.0f) &&
		(pApp->tPage.arrTextEdit[2].pWidget->tRect.fY == 330.0f) &&
		(pApp->tPage.arrTextEdit[3].pWidget->tRect.fX == 450.0f) &&
		(pApp->tPage.arrTextEdit[3].pWidget->tRect.fY == 330.0f) &&
		(pApp->tPage.arrTextEdit[3].pWidget->tRect.fW == 390.0f) &&
		(pApp->tPage.arrTextEdit[3].pWidget->tRect.fH == 190.0f);
	pApp->bStateOK = pApp->bCreateOK &&
		(pApp->tPage.arrTextEdit[1].bLineNumbers != 0) &&
		(pApp->tPage.arrTextEdit[1].bWordWrap != 0) &&
		(pApp->tPage.arrTextEdit[3].bReadonly != 0) &&
		(xgeXuiWidgetIsEnabled(pApp->tPage.arrTextEdit[3].pWidget) == 0) &&
		(pApp->tPage.arrTextEdit[2].iFocusBorderColor == XGE_COLOR_RGBA(33, 115, 186, 255));
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_textedit_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	xgeXuiSetFocus(&pApp->tXui, xgeXuiPageFind(&pApp->tPage, "e0"));
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
		printf("xui_textedit_xson final-summary frames=%d create=%d layout=%d state=%d labels=%d edits=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->tPage.iLabelCount, pApp->tPage.iTextEditCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TEXTEDIT_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 900;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI TextEdit XSON";
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
