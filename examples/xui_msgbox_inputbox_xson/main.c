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
	int bStateOK;
	int bInteractionOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"absolute\",\"width\":760,\"height\":420,\"background\":\"#E8F0F8FF\"},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#36506AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"button\":{\"type\":\"button\",\"font\":\"@fonts.body\",\"width\":156,\"height\":34,\"padding\":[10,5,10,5],\"color\":\"#317FD1FF\",\"hoverColor\":\"#4293E4FF\",\"activeColor\":\"#2169B8FF\",\"borderColor\":\"#1C5C9AFF\",\"textColor\":\"#FFFFFFFF\"},"
"\"box\":{\"font\":\"@fonts.body\",\"backgroundColor\":\"#F8FBFEFF\",\"titleColor\":\"#263040FF\",\"closeColor\":\"#3D5C76FF\",\"textColor\":\"#425066FF\",\"buttonColor\":\"#F2F6FAFF\",\"buttonHoverColor\":\"#D9ECFCFF\",\"buttonTextColor\":\"#25364AFF\",\"backdropColor\":\"#1A2C4018\"}"
"},"
"\"tree\":{\"type\":\"absolute\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"title\",\"style\":\"label\",\"x\":30,\"y\":30,\"width\":660,\"text\":\"XSON MsgBox and InputBox overlay controls\"},"
"{\"type\":\"label\",\"id\":\"status\",\"style\":\"label\",\"x\":30,\"y\":72,\"width\":660,\"text\":\"Both overlays are created from XSON. MsgBox starts open; InputBox is opened by the runtime check.\"},"
"{\"type\":\"button\",\"id\":\"button0\",\"style\":\"button\",\"x\":30,\"y\":124,\"text\":\"Normal control\"},"
"{\"type\":\"msgBox\",\"id\":\"msg\",\"style\":\"box\",\"title\":\"XSON MsgBox\",\"message\":\"This message box is declared by XSON and starts open.\",\"messageType\":\"info\",\"buttons\":\"okCancel\",\"modal\":true,\"open\":true},"
"{\"type\":\"inputBox\",\"id\":\"inputbox\",\"style\":\"box\",\"title\":\"XSON InputBox\",\"prompt\":\"Display name\",\"initial\":\"Hero\",\"modal\":false,\"open\":false}"
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
			printf("xui_msgbox_inputbox_xson font loaded: %s\n", arrFonts[i]);
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
	xge_xui_msg_box pMsg;
	xge_xui_input_box pInput;
	xge_event_t tEvent;
	char* sResult;

	pMsg = (pApp->tPage.iMsgBoxCount > 0) ? pApp->tPage.arrMsgBox[0] : NULL;
	pInput = (pApp->tPage.iInputBoxCount > 0) ? pApp->tPage.arrInputBox[0] : NULL;
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK =
		(pMsg != NULL) &&
		(pInput != NULL) &&
		(pApp->tPage.iMsgBoxCount == 1) &&
		(pApp->tPage.iInputBoxCount == 1) &&
		(xgeXuiPageFind(&pApp->tPage, "msg") != NULL) &&
		(xgeXuiPageFind(&pApp->tPage, "inputbox") != NULL);
	pApp->bStateOK = pApp->bCreateOK &&
		(xgeXuiMsgBoxIsOpen(pMsg) != 0) &&
		(pMsg->iType == XGE_XUI_MSG_BOX_ICON_INFO) &&
		(pMsg->iButtons == XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL) &&
		(pMsg->bModal != 0) &&
		(pInput->bModal == 0) &&
		(strcmp(pInput->sTitle, "XSON InputBox") == 0) &&
		(strcmp(pInput->sPrompt, "Display name") == 0) &&
		(strcmp(xgeXuiInputGetText(&pInput->tInput), "Hero") == 0);
	if ( pMsg != NULL ) {
		xgeXuiMsgBoxSetOpen(pMsg, 0);
	}
	if ( pInput != NULL ) {
		xgeXuiInputBoxSetOpen(pInput, 1);
		xgeXuiUpdate(&pApp->tXui, 0.0f);
		xgeXuiPaint(&pApp->tXui);
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pInput->pOkWidget->tRect);
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
		MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pInput->pOkWidget->tRect);
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
		sResult = xgeXuiInputBoxGetResult(pInput);
		pApp->bInteractionOK =
			(xgeXuiInputBoxGetResultCode(pInput) == XGE_XUI_MSG_BOX_RESULT_OK) &&
			(sResult != NULL) &&
			(strcmp(sResult, "Hero") == 0);
		if ( sResult != NULL ) {
			xgeMemoryFree(sResult);
		}
	}
}

static int CreateUI(app_state_t* pApp)
{
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_msgbox_inputbox_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
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
		printf("xui_msgbox_inputbox_xson final-summary frames=%d create=%d state=%d interaction=%d msg=%d input=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bStateOK,
			pApp->bInteractionOK,
			pApp->tPage.iMsgBoxCount,
			pApp->tPage.iInputBoxCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MSGBOX_INPUTBOX_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XUI MsgBox / InputBox XSON";
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
