#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACTION_COUNT 8
#define LABEL_COUNT 2

typedef struct app_state_t app_state_t;

typedef struct tip_action_t {
	app_state_t* pApp;
	const char* sButton;
	const char* sText;
	int iType;
	float fDuration;
	int bCustomColor;
} tip_action_t;

struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_button_t tButton[ACTION_COUNT];
	xge_xui_msg_tip_t tTip;
	xge_xui_widget pLabelWidget[LABEL_COUNT];
	xge_xui_widget pButtonWidget[ACTION_COUNT];
	xge_xui_widget pTipWidget;
	tip_action_t arrAction[ACTION_COUNT];
	int iButtonCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bExpireOK;
	int bClickOK;
	int bStyleOK;
};

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
			printf("xui_msgtip font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
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

static int AddLabel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, xge_rect_t tRect, const char* sText)
{
	xge_xui_widget pWidget;

	if ( (iIndex < 0) || (iIndex >= LABEL_COUNT) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidget = NewWidget(pRoot, tRect);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, AppFont(pApp), sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(52, 68, 86, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	pApp->pLabelWidget[iIndex] = pWidget;
	return XGE_OK;
}

static uint32_t BlendColor(uint32_t iA, uint32_t iB, int iStep, int iTotal)
{
	int r;
	int g;
	int b;
	int a;

	if ( iTotal <= 0 ) {
		return iA;
	}
	r = (int)XGE_COLOR_GET_R(iA) + ((int)XGE_COLOR_GET_R(iB) - (int)XGE_COLOR_GET_R(iA)) * iStep / iTotal;
	g = (int)XGE_COLOR_GET_G(iA) + ((int)XGE_COLOR_GET_G(iB) - (int)XGE_COLOR_GET_G(iA)) * iStep / iTotal;
	b = (int)XGE_COLOR_GET_B(iA) + ((int)XGE_COLOR_GET_B(iB) - (int)XGE_COLOR_GET_B(iA)) * iStep / iTotal;
	a = (int)XGE_COLOR_GET_A(iA) + ((int)XGE_COLOR_GET_A(iB) - (int)XGE_COLOR_GET_A(iA)) * iStep / iTotal;
	return XGE_COLOR_RGBA(r, g, b, a);
}

static void ApplyButtonStyle(xge_xui_button pButton, xge_xui_widget pWidget)
{
	uint32_t iNormal;
	uint32_t iHover;
	uint32_t iActive;

	iNormal = XGE_COLOR_RGBA(230, 238, 247, 255);
	iHover = XGE_COLOR_RGBA(216, 230, 248, 255);
	iActive = XGE_COLOR_RGBA(197, 217, 244, 255);
	xgeXuiButtonSetColors(pButton, iNormal, iHover, iActive, XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(206, 211, 218, 170));
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(36, 54, 74, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(126, 166, 220, 255));
	xgeXuiWidgetSetRadius(pWidget, 3.0f);
}

static void ResetTipStyle(app_state_t* pApp)
{
	xgeXuiMsgTipSetColors(&pApp->tTip, XGE_COLOR_RGBA(32, 32, 32, 205), XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(72, 190, 148, 255));
}

static void ShowTipAction(xge_xui_widget pWidget, void* pUser)
{
	tip_action_t* pAction;
	app_state_t* pApp;
	char sStatus[160];

	(void)pWidget;
	pAction = (tip_action_t*)pUser;
	if ( (pAction == NULL) || (pAction->pApp == NULL) ) {
		return;
	}
	pApp = pAction->pApp;
	if ( pAction->bCustomColor ) {
		xgeXuiMsgTipSetColors(&pApp->tTip, XGE_COLOR_RGBA(245, 252, 255, 238), XGE_COLOR_RGBA(26, 58, 84, 255), XGE_COLOR_RGBA(46, 124, 214, 255));
	} else {
		ResetTipStyle(pApp);
	}
	xgeXuiMsgTipShow(&pApp->tTip, pAction->iType, pAction->sText, pAction->fDuration);
	snprintf(sStatus, sizeof(sStatus), "show=%d close=%d expire=%d type=%d duration=%.1f", pApp->tTip.iShowCount, pApp->tTip.iCloseCount, pApp->tTip.iExpireCount, pAction->iType, pAction->fDuration);
	xgeXuiLabelSetText(&pApp->tLabel[1], sStatus);
}

static int AddAction(app_state_t* pApp, xge_xui_widget pRoot, int iCol, int iRow, const char* sButton, int iType, const char* sText, float fDuration, int bCustomColor)
{
	xge_xui_widget pWidget;
	tip_action_t* pAction;
	xge_xui_button pButton;
	float fX;
	float fY;
	int iIndex;

	if ( pApp->iButtonCount >= ACTION_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iIndex = pApp->iButtonCount++;
	fX = 34.0f + (float)iCol * 176.0f;
	fY = 116.0f + (float)iRow * 46.0f;
	pWidget = NewWidget(pRoot, (xge_rect_t){ fX, fY, 150.0f, 30.0f });
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->tButton[iIndex];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, AppFont(pApp), sButton);
	ApplyButtonStyle(pButton, pWidget);
	pAction = &pApp->arrAction[iIndex];
	pAction->pApp = pApp;
	pAction->sButton = sButton;
	pAction->sText = sText;
	pAction->iType = iType;
	pAction->fDuration = fDuration;
	pAction->bCustomColor = bCustomColor;
	xgeXuiButtonSetClick(pButton, ShowTipAction, pAction);
	pApp->pButtonWidget[iIndex] = pWidget;
	return XGE_OK;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
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

	ResetTipStyle(pApp);
	xgeXuiMsgTipShow(&pApp->tTip, XGE_XUI_MSG_TIP_ICON_INFO, "Short auto close", 0.05f);
	pApp->bCreateOK = xgeXuiMsgTipIsOpen(&pApp->tTip) && pApp->tTip.bShowIcon;
	xgeXuiUpdate(&pApp->tXui, 0.06f);
	pApp->bExpireOK = !xgeXuiMsgTipIsOpen(&pApp->tTip) && pApp->tTip.iExpireCount >= 1;
	xgeXuiMsgTipSetColors(&pApp->tTip, XGE_COLOR_RGBA(245, 252, 255, 238), XGE_COLOR_RGBA(26, 58, 84, 255), XGE_COLOR_RGBA(46, 124, 214, 255));
	xgeXuiMsgTipShow(&pApp->tTip, XGE_XUI_MSG_TIP_ICON_QUEST, "Click this custom colored tip to close it.", 0.0f);
	pApp->bStyleOK = pApp->tTip.iBackgroundColor == XGE_COLOR_RGBA(245, 252, 255, 238) && pApp->tTip.tRect.fW >= pApp->tTip.fMinWidth;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tTip.tRect.fX + pApp->tTip.tRect.fW * 0.5f, pApp->tTip.tRect.fY + pApp->tTip.tRect.fH * 0.5f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bClickOK = !xgeXuiMsgTipIsOpen(&pApp->tTip) && pApp->tTip.iCloseCount >= 1;
	ResetTipStyle(pApp);
	xgeXuiMsgTipShow(&pApp->tTip, XGE_XUI_MSG_TIP_ICON_WAR, "MsgTip ready. Use buttons to switch message type.", 8.0f);
	xgeXuiLabelSetText(&pApp->tLabel[1], "ready: buttons show types, custom colors, wrapping, auto close, and persistent close");
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	if ( AddLabel(pApp, pRoot, 0, (xge_rect_t){ 28.0f, 28.0f, 700.0f, 30.0f }, "MsgTip: single transient XUI service, not an XSON layout node") != XGE_OK ||
	     AddLabel(pApp, pRoot, 1, (xge_rect_t){ 28.0f, 66.0f, 700.0f, 30.0f }, "ready") != XGE_OK ||
	     AddAction(pApp, pRoot, 0, 0, "None", XGE_XUI_MSG_TIP_ICON_NONE, "Saved", 1.6f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 1, 0, "Info", XGE_XUI_MSG_TIP_ICON_INFO, "Project settings updated.", 2.0f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 2, 0, "Question", XGE_XUI_MSG_TIP_ICON_QUEST, "Use the selected preset?", 2.0f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 3, 0, "Warning", XGE_XUI_MSG_TIP_ICON_WAR, "This operation will reload the current tool window.", 2.2f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 0, 1, "Error", XGE_XUI_MSG_TIP_ICON_ERROR, "Unable to save: target file is read-only.", 2.2f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 1, 1, "Long Text", XGE_XUI_MSG_TIP_ICON_INFO, "A longer message wraps inside the configured maximum width without changing the host widget size.", 3.0f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 2, 1, "Persistent", XGE_XUI_MSG_TIP_ICON_WAR, "Persistent tip. Click the tip body to close.", 0.0f, 0) != XGE_OK ||
	     AddAction(pApp, pRoot, 3, 1, "Custom Color", XGE_XUI_MSG_TIP_ICON_INFO, "Custom light surface and blue icon.", 3.0f, 1) != XGE_OK ) {
		return XGE_ERROR;
	}
	pApp->pTipWidget = xgeXuiWidgetCreate();
	if ( pApp->pTipWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pTipWidget, (xge_rect_t){ 0.0f, 0.0f, 760.0f, 420.0f });
	xgeXuiWidgetAdd(pRoot, pApp->pTipWidget);
	if ( xgeXuiMsgTipInit(&pApp->tTip, &pApp->tXui, pApp->pTipWidget, AppFont(pApp)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiMsgTipSetMetrics(&pApp->tTip, 96.0f, 360.0f, 42.0f, -38.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiMsgTipUnit(&pApp->tTip);
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
			"xui_msgtip final-summary frames=%d create=%d expire=%d click=%d style=%d show=%d close=%d expired=%d open=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bExpireOK,
			pApp->bClickOK,
			pApp->bStyleOK,
			pApp->tTip.iShowCount,
			pApp->tTip.iCloseCount,
			pApp->tTip.iExpireCount,
			xgeXuiMsgTipIsOpen(&pApp->tTip));
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_MSGTIP_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 420;
	tDesc.sTitle = "XUI MsgTip";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bExpireOK && tApp.bClickOK && tApp.bStyleOK) ? 0 : 3;
}
