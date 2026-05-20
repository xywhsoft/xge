#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pTitle;
	xge_xui_widget pStatus;
	xge_xui_widget pModalButton;
	xge_xui_widget pFloatButton;
	xge_xui_widget pInputButton;
	xge_xui_widget pMsgWidget;
	xge_xui_widget pInputWidget;
	xge_xui_label_t tTitle;
	xge_xui_label_t tStatus;
	xge_xui_button_t tModalButton;
	xge_xui_button_t tFloatButton;
	xge_xui_button_t tInputButton;
	xge_xui_msg_box_t tMsgBox;
	xge_xui_input_box_t tInputBox;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iMsgResult;
	int bMsgCheck;
	int bInputCheck;
	char sInput[128];
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
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font Font(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(sText, sizeof(sText), "msg result=%d | input=\"%s\" | checks msg=%d input=%d", pApp->iMsgResult, pApp->sInput, pApp->bMsgCheck, pApp->bInputCheck);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
}

static void MsgResult(xge_xui_widget pWidget, int iResult, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iMsgResult = iResult;
	UpdateStatus(pApp);
}

static void InputResult(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	app_state_t* pApp;
	char* sCopy;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	sCopy = xgeXuiInputBoxGetResult(&pApp->tInputBox);
	snprintf(pApp->sInput, sizeof(pApp->sInput), "%s", (sCopy != NULL) ? sCopy : ((sText != NULL) ? sText : ""));
	pApp->sInput[sizeof(pApp->sInput) - 1] = 0;
	if ( sCopy != NULL ) {
		xgeMemoryFree(sCopy);
	}
	UpdateStatus(pApp);
}

static void OpenModalMsg(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiMsgBoxSetModal(&pApp->tMsgBox, 1);
	xgeXuiMsgBoxSetText(&pApp->tMsgBox, Font(pApp), "Blocking MsgBox", "This modal message blocks the rest of XUI until the user chooses a result. Long content automatically wraps before the window grows beyond the target width.");
	xgeXuiMsgBoxSetType(&pApp->tMsgBox, XGE_XUI_MSG_BOX_ICON_QUEST);
	xgeXuiMsgBoxSetButtons(&pApp->tMsgBox, XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 1);
}

static void OpenFloatingMsg(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiMsgBoxSetModal(&pApp->tMsgBox, 0);
	xgeXuiMsgBoxSetText(&pApp->tMsgBox, Font(pApp), "Non-modal MsgBox", "This message stays above normal controls but does not block interaction outside itself.");
	xgeXuiMsgBoxSetType(&pApp->tMsgBox, XGE_XUI_MSG_BOX_ICON_INFO);
	xgeXuiMsgBoxSetButtons(&pApp->tMsgBox, XGE_XUI_MSG_BOX_BUTTON_OK_CANCEL);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 1);
}

static void OpenInputBox(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiInputBoxSetModal(&pApp->tInputBox, 1);
	xgeXuiInputBoxSetText(&pApp->tInputBox, Font(pApp), "InputBox", "Character name", (pApp->sInput[0] != 0) ? pApp->sInput : "Hero");
	xgeXuiInputBoxSetOpen(&pApp->tInputBox, 1);
}

static xge_xui_widget NewWidget(xge_xui_widget pParent, xge_rect_t tRect)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetAdd(pParent, pWidget);
	return pWidget;
}

static void MakeMouse(xge_event_t* pEvent, int iType, xge_rect_t tRect)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = tRect.fX + tRect.fW * 0.5f;
	pEvent->fY = tRect.fY + tRect.fH * 0.5f;
}

static void MakeMousePoint(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	char* sResult;
	xge_rect_t tBefore;
	float fDragX;
	float fDragY;

	xgeXuiMsgBoxSetModal(&pApp->tMsgBox, 1);
	xgeXuiMsgBoxSetText(&pApp->tMsgBox, Font(pApp), "Check", "Result mapping");
	xgeXuiMsgBoxSetType(&pApp->tMsgBox, XGE_XUI_MSG_BOX_ICON_QUEST);
	xgeXuiMsgBoxSetButtons(&pApp->tMsgBox, XGE_XUI_MSG_BOX_BUTTON_YES_NO_CANCEL);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 1);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tMsgBox.arrButtonRect[1]);
	pApp->bMsgCheck = (xgeXuiMsgBoxEvent(&pApp->tMsgBox, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiMsgBoxGetResult(&pApp->tMsgBox) == XGE_XUI_MSG_BOX_RESULT_NO);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 1);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tMsgBox.tWindow.pCloseButtonWidget->tRect);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tMsgBox.tWindow.pCloseButtonWidget->tRect);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bMsgCheck = pApp->bMsgCheck && (xgeXuiMsgBoxGetResult(&pApp->tMsgBox) == XGE_XUI_MSG_BOX_RESULT_CLOSE);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 1);
	xgeXuiPaint(&pApp->tXui);
	tBefore = pApp->tMsgBox.tMessageRect;
	fDragX = pApp->pMsgWidget->tRect.fX + 80.0f;
	fDragY = pApp->pMsgWidget->tRect.fY + 10.0f;
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_DOWN, fDragX, fDragY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_MOVE, fDragX + 42.0f, fDragY + 28.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_UP, fDragX + 42.0f, fDragY + 28.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bMsgCheck = pApp->bMsgCheck && (pApp->tMsgBox.tMessageRect.fX > tBefore.fX + 20.0f) && (pApp->tMsgBox.tMessageRect.fY > tBefore.fY + 10.0f);
	xgeXuiMsgBoxSetOpen(&pApp->tMsgBox, 0);

	xgeXuiInputBoxSetModal(&pApp->tInputBox, 1);
	xgeXuiInputBoxSetText(&pApp->tInputBox, Font(pApp), "Input Check", "Name", "Checked");
	xgeXuiInputBoxSetOpen(&pApp->tInputBox, 1);
	xgeXuiPaint(&pApp->tXui);
	tBefore = pApp->tInputBox.tPromptRect;
	fDragX = pApp->pInputWidget->tRect.fX + 80.0f;
	fDragY = pApp->pInputWidget->tRect.fY + 10.0f;
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_DOWN, fDragX, fDragY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_MOVE, fDragX + 36.0f, fDragY + 24.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMousePoint(&tEvent, XGE_EVENT_MOUSE_UP, fDragX + 36.0f, fDragY + 24.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bInputCheck = (pApp->tInputBox.tPromptRect.fX > tBefore.fX + 20.0f) && (pApp->tInputBox.tPromptRect.fY > tBefore.fY + 10.0f);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tInputBox.pOkWidget->tRect);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pApp->tInputBox.pOkWidget->tRect);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	sResult = xgeXuiInputBoxGetResult(&pApp->tInputBox);
	pApp->bInputCheck = pApp->bInputCheck && (xgeXuiInputBoxGetResultCode(&pApp->tInputBox) == XGE_XUI_MSG_BOX_RESULT_OK) &&
		(sResult != NULL) && (strcmp(sResult, "Checked") == 0);
	if ( sResult != NULL ) {
		xgeMemoryFree(sResult);
	}
	snprintf(pApp->sInput, sizeof(pApp->sInput), "Hero");
	pApp->iMsgResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
	UpdateStatus(pApp);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, Font(pApp));
	pApp->iMsgResult = XGE_XUI_MSG_BOX_RESULT_CLOSE;
	pApp->pTitle = NewWidget(pRoot, (xge_rect_t){ 30.0f, 30.0f, 520.0f, 34.0f });
	pApp->pStatus = NewWidget(pRoot, (xge_rect_t){ 30.0f, 76.0f, 520.0f, 28.0f });
	pApp->pModalButton = NewWidget(pRoot, (xge_rect_t){ 30.0f, 126.0f, 170.0f, 36.0f });
	pApp->pFloatButton = NewWidget(pRoot, (xge_rect_t){ 220.0f, 126.0f, 170.0f, 36.0f });
	pApp->pInputButton = NewWidget(pRoot, (xge_rect_t){ 410.0f, 126.0f, 170.0f, 36.0f });
	pApp->pMsgWidget = xgeXuiWidgetCreate();
	pApp->pInputWidget = xgeXuiWidgetCreate();
	if ( (pApp->pTitle == NULL) || (pApp->pStatus == NULL) || (pApp->pModalButton == NULL) || (pApp->pFloatButton == NULL) || (pApp->pInputButton == NULL) || (pApp->pMsgWidget == NULL) || (pApp->pInputWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pMsgWidget, (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget, (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f });
	xgeXuiLabelInit(&pApp->tTitle, pApp->pTitle, Font(pApp), "Window-based MsgBox and InputBox");
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, Font(pApp), "");
	xgeXuiButtonInit(&pApp->tModalButton, &pApp->tXui, pApp->pModalButton);
	xgeXuiButtonSetText(&pApp->tModalButton, Font(pApp), "Modal MsgBox");
	xgeXuiButtonSetClick(&pApp->tModalButton, OpenModalMsg, pApp);
	xgeXuiButtonInit(&pApp->tFloatButton, &pApp->tXui, pApp->pFloatButton);
	xgeXuiButtonSetText(&pApp->tFloatButton, Font(pApp), "Floating MsgBox");
	xgeXuiButtonSetClick(&pApp->tFloatButton, OpenFloatingMsg, pApp);
	xgeXuiButtonInit(&pApp->tInputButton, &pApp->tXui, pApp->pInputButton);
	xgeXuiButtonSetText(&pApp->tInputButton, Font(pApp), "InputBox");
	xgeXuiButtonSetClick(&pApp->tInputButton, OpenInputBox, pApp);
	if ( xgeXuiMsgBoxInit(&pApp->tMsgBox, &pApp->tXui, pApp->pMsgWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMsgBoxSetResult(&pApp->tMsgBox, MsgResult, pApp);
	if ( xgeXuiInputBoxInit(&pApp->tInputBox, &pApp->tXui, pApp->pInputWidget, Font(pApp)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputBoxSetResult(&pApp->tInputBox, InputResult, pApp);
	snprintf(pApp->sInput, sizeof(pApp->sInput), "Hero");
	RunChecks(pApp);
	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiInputBoxUnit(&pApp->tInputBox);
	xgeXuiMsgBoxUnit(&pApp->tMsgBox);
	xgeXuiButtonUnit(&pApp->tInputButton);
	xgeXuiButtonUnit(&pApp->tFloatButton);
	xgeXuiButtonUnit(&pApp->tModalButton);
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
		if ( xgeXuiMsgBoxIsOpen(&pApp->tMsgBox) || xgeXuiInputBoxIsOpen(&pApp->tInputBox) ) {
			xgeXuiDispatchEvent(&pApp->tXui, pEvent);
			return XGE_OK;
		}
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
		printf("xui-msgbox-inputbox summary frames=%d result=%d input=%s msgCheck=%d inputCheck=%d\n", pApp->iFrameCount, pApp->iMsgResult, pApp->sInput, pApp->bMsgCheck, pApp->bInputCheck);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 242, 252, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MSGBOX_INPUTBOX_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XUI MsgBox / InputBox";
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
	return (iExitCode == XGE_OK) ? 0 : 3;
}
