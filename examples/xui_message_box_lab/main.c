#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pMessage;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_message_box_t tMessage;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iResultCount;
	int iLastResult;
	int bInitOK;
	int bClickOK;
	int bEscapeOK;
	int bEnterOK;
	int bCallbackOnceOK;
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

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-message-box-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-message-box-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MessageResult(xge_xui_widget pWidget, int iResult, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iResultCount++;
	pApp->iLastResult = iResult;
}

static void MakeKey(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeMouse(xge_event_t* pEvent, xge_rect_t tRect)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_DOWN;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = tRect.fX + 2.0f;
	pEvent->fY = tRect.fY + 2.0f;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pStatus = xgeXuiWidgetCreate();
	pApp->pMessage = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) || (pApp->pMessage == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 22.0f, 20.0f, 500.0f, 250.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "MessageBox");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 36.0f, 455.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);
	xgeXuiWidgetSetRect(pApp->pMessage, (xge_rect_t){ 110.0f, 84.0f, 300.0f, 132.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pMessage, 8.0f, 8.0f, 8.0f, 8.0f);
	if ( xgeXuiMessageBoxInit(&pApp->tMessage, &pApp->tXui, pApp->pMessage) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMessageBoxSetText(&pApp->tMessage, pFont, "Confirm", "Save changes before closing?");
	xgeXuiMessageBoxSetType(&pApp->tMessage, XGE_XUI_MESSAGE_BOX_QUESTION);
	xgeXuiMessageBoxSetButtons(&pApp->tMessage, XGE_XUI_MESSAGE_BOX_YES_NO_CANCEL);
	xgeXuiMessageBoxSetResult(&pApp->tMessage, MessageResult, pApp);
	xgeXuiMessageBoxSetColors(
		&pApp->tMessage,
		XGE_COLOR_RGBA(1, 2, 3, 96),
		XGE_COLOR_RGBA(244, 250, 255, 255),
		XGE_COLOR_RGBA(7, 8, 9, 255),
		XGE_COLOR_RGBA(10, 11, 12, 255),
		XGE_COLOR_RGBA(17, 18, 19, 255),
		XGE_COLOR_RGBA(234, 246, 253, 255),
		XGE_COLOR_RGBA(199, 232, 248, 255),
		XGE_COLOR_RGBA(1, 2, 3, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pMessage);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;

	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = xgeXuiMessageBoxIsOpen(&pApp->tMessage) && (pApp->tMessage.iButtonCount == 3);
	MakeMouse(&tEvent, pApp->tMessage.arrButtonRect[1]);
	pApp->bClickOK = (xgeXuiMessageBoxEvent(&pApp->tMessage, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMessageBoxGetResult(&pApp->tMessage) == XGE_XUI_MESSAGE_BOX_RESULT_NO) &&
		(pApp->iResultCount == 1);
	pApp->bCallbackOnceOK = (xgeXuiMessageBoxEvent(&pApp->tMessage, &tEvent) == XGE_XUI_EVENT_CONTINUE) && (pApp->iResultCount == 1);
	xgeXuiMessageBoxSetOpen(&pApp->tMessage, 1);
	MakeKey(&tEvent, XGE_KEY_ESCAPE);
	pApp->bEscapeOK = (xgeXuiMessageBoxEvent(&pApp->tMessage, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMessageBoxGetResult(&pApp->tMessage) == XGE_XUI_MESSAGE_BOX_RESULT_CANCEL) &&
		(pApp->iResultCount == 2);
	xgeXuiMessageBoxSetOpen(&pApp->tMessage, 1);
	MakeKey(&tEvent, XGE_KEY_ENTER);
	pApp->bEnterOK = (xgeXuiMessageBoxEvent(&pApp->tMessage, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMessageBoxGetResult(&pApp->tMessage) == XGE_XUI_MESSAGE_BOX_RESULT_YES) &&
		(pApp->iResultCount == 3);
	xgeXuiMessageBoxSetOpen(&pApp->tMessage, 1);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d click=%d escape=%d enter=%d once=%d result=%d cb=%d",
		pApp->bInitOK,
		pApp->bClickOK,
		pApp->bEscapeOK,
		pApp->bEnterOK,
		pApp->bCallbackOnceOK,
		pApp->iLastResult,
		pApp->iResultCount);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
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
	UpdateStatus(pApp);
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
			"xui-message-box-lab final-summary frames=%d init=%d click=%d escape=%d enter=%d once=%d result=%d cb=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bClickOK,
			pApp->bEscapeOK,
			pApp->bEnterOK,
			pApp->bCallbackOnceOK,
			pApp->iLastResult,
			pApp->iResultCount);
		printf("xui-message-box-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(238, 248, 255, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MESSAGE_BOX_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 560;
	tDesc.iHeight = 300;
	tDesc.sTitle = "XGE XUI MessageBox Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bClickOK && tApp.bEscapeOK && tApp.bEnterOK && tApp.bCallbackOnceOK) ? 0 : 3;
}
