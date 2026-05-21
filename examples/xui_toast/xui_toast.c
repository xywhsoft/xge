#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOAST_BUTTON_COUNT 7
#define TOAST_LABEL_COUNT 8

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_label_t arrLabel[TOAST_LABEL_COUNT];
	xge_xui_button_t arrButton[TOAST_BUTTON_COUNT];
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iClickCount;
	int iCloseCount;
	int iLastCloseReason;
	int bQueueOK;
	int bClickOK;
	int bNoCallbackClickOK;
	int bCloseButtonOK;
	int bCloseReenterOK;
	int iReenterCloseShows;
	int bExpireOK;
} app_state_t;

typedef struct toast_action_t {
	app_state_t* pApp;
	int iAction;
} toast_action_t;

static toast_action_t g_arrActions[TOAST_BUTTON_COUNT];

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
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-toast font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-toast font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static void ToastClick(xge_xui_context pContext, int iToastId, void* pUser)
{
	app_state_t* pApp;

	(void)pContext;
	(void)iToastId;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iClickCount++;
	}
}

static void ToastClose(xge_xui_context pContext, int iToastId, int iReason, void* pUser)
{
	app_state_t* pApp;

	(void)pContext;
	(void)iToastId;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCloseCount++;
		pApp->iLastCloseReason = iReason;
	}
}

static void ToastCloseReenter(xge_xui_context pContext, int iToastId, int iReason, void* pUser)
{
	app_state_t* pApp;

	(void)iToastId;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	pApp->iCloseCount++;
	pApp->iLastCloseReason = iReason;
	if ( iReason == XGE_XUI_TOAST_CLOSE_CLICK && pApp->iReenterCloseShows == 0 ) {
		pApp->iReenterCloseShows = 1;
		xgeXuiToastShow(pContext, XGE_XUI_TOAST_TYPE_INFO, "Reenter", "Created by close callback.", 5.0f, NULL, NULL);
	}
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, xge_rect_t tRect, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= TOAST_LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pLabel = &pApp->arrLabel[pApp->iLabelCount];
	if ( xgeXuiLabelInit(pLabel, pWidget, AppFont(pApp), sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, XGE_COLOR_RGBA(54, 73, 92, 255));
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	xgeXuiWidgetSetRect(pWidget, tRect);
	xgeXuiWidgetAdd(pParent, pWidget);
	pApp->iLabelCount++;
	return XGE_OK;
}

static void ButtonClick(xge_xui_widget pWidget, void* pUser)
{
	toast_action_t* pAction;
	app_state_t* pApp;

	(void)pWidget;
	pAction = (toast_action_t*)pUser;
	if ( pAction == NULL || pAction->pApp == NULL ) {
		return;
	}
	pApp = pAction->pApp;
	switch ( pAction->iAction ) {
		case 0:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Info", "Project indexed and ready.", 3.0f, NULL, NULL);
			break;
		case 1:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_SUCCESS, "Saved", "Scene data was saved successfully.", 3.0f, NULL, NULL);
			break;
		case 2:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_WARNING, "Warning", "One item still needs review before export.", 3.0f, NULL, NULL);
			break;
		case 3:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_ERROR, "Error", "Export target is unavailable.", 3.0f, NULL, NULL);
			break;
		case 4:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Clickable", "Click this toast body to run its callback.", 5.0f, ToastClick, pApp);
			break;
		case 5:
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Queue 1", "Visible immediately.", 3.0f, NULL, NULL);
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_SUCCESS, "Queue 2", "Visible immediately.", 3.0f, NULL, NULL);
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_WARNING, "Queue 3", "Queued until a slot is free.", 3.0f, NULL, NULL);
			xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_ERROR, "Queue 4", "Queued behind the third toast.", 3.0f, NULL, NULL);
			break;
		case 6:
			xgeXuiToastClear(&pApp->tXui);
			break;
		default:
			break;
	}
}

static int AddButton(app_state_t* pApp, xge_xui_widget pParent, xge_rect_t tRect, const char* sText, int iAction)
{
	xge_xui_widget pWidget;
	xge_xui_button pButton;
	toast_action_t* pAction;

	if ( pApp->iButtonCount >= TOAST_BUTTON_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->arrButton[pApp->iButtonCount];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, AppFont(pApp), sText);
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(46, 124, 214, 255), XGE_COLOR_RGBA(66, 145, 230, 255), XGE_COLOR_RGBA(28, 105, 187, 255), XGE_COLOR_RGBA(118, 168, 226, 255), XGE_COLOR_RGBA(180, 190, 202, 160));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(31, 98, 172, 255));
	xgeXuiWidgetSetRadius(pWidget, 4.0f);
	xgeXuiWidgetSetRect(pWidget, tRect);
	pAction = &g_arrActions[pApp->iButtonCount];
	pAction->pApp = pApp;
	pAction->iAction = iAction;
	xgeXuiButtonSetClick(pButton, ButtonClick, pAction);
	xgeXuiWidgetAdd(pParent, pWidget);
	pApp->iButtonCount++;
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

static void RunInitialChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_toast pToast;
	int iClickBefore;

	pToast = &pApp->tXui.tToast;
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Info", "Visible slot 1.", 5.0f, NULL, NULL);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_SUCCESS, "Saved", "Visible slot 2.", 5.0f, NULL, NULL);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_WARNING, "Queued", "This waits for an empty slot.", 5.0f, NULL, NULL);
	pApp->bQueueOK = xgeXuiToastGetActiveCount(&pApp->tXui) == 2 && xgeXuiToastGetPendingCount(&pApp->tXui) == 1;
	xgeXuiToastClear(&pApp->tXui);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Clickable", "Synthetic body click.", 5.0f, ToastClick, pApp);
	xgeXuiPaint(&pApp->tXui);
	iClickBefore = pApp->iClickCount;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pToast->arrActive[0].tRect.fX + 18.0f, pToast->arrActive[0].tRect.fY + 18.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bClickOK = pApp->iClickCount == iClickBefore + 1 && xgeXuiToastGetActiveCount(&pApp->tXui) == 0 && pApp->iLastCloseReason == XGE_XUI_TOAST_CLOSE_CLICK;
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_SUCCESS, "No callback", "Body click only closes.", 5.0f, NULL, NULL);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pToast->arrActive[0].tRect.fX + 18.0f, pToast->arrActive[0].tRect.fY + 18.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bNoCallbackClickOK = xgeXuiToastGetActiveCount(&pApp->tXui) == 0 && pApp->iLastCloseReason == XGE_XUI_TOAST_CLOSE_CLICK;
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_WARNING, "Close button", "Synthetic close button click.", 5.0f, NULL, NULL);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pToast->arrActive[0].tCloseRect.fX + 2.0f, pToast->arrActive[0].tCloseRect.fY + 2.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bCloseButtonOK = xgeXuiToastGetActiveCount(&pApp->tXui) == 0 && pApp->iLastCloseReason == XGE_XUI_TOAST_CLOSE_BUTTON;
	xgeXuiToastSetClose(&pApp->tXui, ToastCloseReenter, pApp);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Reentrant close", "Close callback shows another toast.", 5.0f, NULL, NULL);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pToast->arrActive[0].tRect.fX + 18.0f, pToast->arrActive[0].tRect.fY + 18.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pApp->bCloseReenterOK = xgeXuiToastGetActiveCount(&pApp->tXui) == 1 && pToast->arrActive[0].sTitle != NULL && strcmp(pToast->arrActive[0].sTitle, "Reenter") == 0;
	xgeXuiToastClear(&pApp->tXui);
	xgeXuiToastSetClose(&pApp->tXui, ToastClose, pApp);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_ERROR, "Short", "Expires deterministically.", 0.05f, NULL, NULL);
	xgeXuiUpdate(&pApp->tXui, 0.08f);
	pApp->bExpireOK = xgeXuiToastGetActiveCount(&pApp->tXui) == 0 && pToast->iExpireCount >= 1 && pApp->iLastCloseReason == XGE_XUI_TOAST_CLOSE_TIMEOUT;
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_INFO, "Toast ready", "Use the buttons to add notifications. Default lifetime is counted after each toast becomes visible.", 5.0f, NULL, NULL);
	xgeXuiToastShow(&pApp->tXui, XGE_XUI_TOAST_TYPE_SUCCESS, "Queue enabled", "Only two toasts are visible in this demo; extra toasts wait in the context queue.", 5.0f, NULL, NULL);
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
	xgeXuiToastSetFont(&pApp->tXui, AppFont(pApp));
	xgeXuiToastSetPlacement(&pApp->tXui, XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT);
	xgeXuiToastSetDirection(&pApp->tXui, XGE_XUI_TOAST_DIRECTION_DOWN);
	xgeXuiToastSetMetrics(&pApp->tXui, 310.0f, 18.0f, 8.0f, 2);
	xgeXuiToastSetClose(&pApp->tXui, ToastClose, pApp);
	xgeXuiToastSetColors(&pApp->tXui, XGE_COLOR_RGBA(248, 252, 255, 246), XGE_COLOR_RGBA(126, 172, 208, 255), XGE_COLOR_RGBA(30, 56, 78, 255), XGE_COLOR_RGBA(82, 108, 126, 255), XGE_COLOR_RGBA(78, 159, 220, 255), XGE_COLOR_RGBA(43, 184, 150, 255), XGE_COLOR_RGBA(244, 187, 68, 255), XGE_COLOR_RGBA(224, 92, 92, 255));
	if ( AddLabel(pApp, pRoot, (xge_rect_t){ 24.0f, 22.0f, 540.0f, 26.0f }, "Toast: context-level notification service, not an XSON layout node") != XGE_OK ||
	     AddLabel(pApp, pRoot, (xge_rect_t){ 24.0f, 56.0f, 520.0f, 40.0f }, "Default lifetime is 3 seconds after activation. This demo limits visible toasts to two, so extra notifications wait in the queue.") != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 24.0f, 112.0f, 112.0f, 30.0f }, "Info", 0) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 148.0f, 112.0f, 112.0f, 30.0f }, "Success", 1) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 272.0f, 112.0f, 112.0f, 30.0f }, "Warning", 2) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 396.0f, 112.0f, 112.0f, 30.0f }, "Error", 3) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 24.0f, 154.0f, 112.0f, 30.0f }, "Clickable", 4) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 148.0f, 154.0f, 112.0f, 30.0f }, "Queue Burst", 5) != XGE_OK ||
	     AddButton(pApp, pRoot, (xge_rect_t){ 272.0f, 154.0f, 112.0f, 30.0f }, "Clear", 6) != XGE_OK ||
	     AddLabel(pApp, pRoot, (xge_rect_t){ 24.0f, 208.0f, 650.0f, 22.0f }, "Checks: queue, click callback, close reason, and activation-time expiration run automatically at startup.") != XGE_OK ) {
		return XGE_ERROR;
	}
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
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunInitialChecks(pApp);
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
			"xui-toast final-summary frames=%d queue=%d click=%d noCallbackClick=%d closeButton=%d closeReenter=%d expire=%d active=%d pending=%d shown=%d closed=%d expired=%d dropped=%d clickCount=%d\n",
			pApp->iFrameCount,
			pApp->bQueueOK,
			pApp->bClickOK,
			pApp->bNoCallbackClickOK,
			pApp->bCloseButtonOK,
			pApp->bCloseReenterOK,
			pApp->bExpireOK,
			xgeXuiToastGetActiveCount(&pApp->tXui),
			xgeXuiToastGetPendingCount(&pApp->tXui),
			pApp->tXui.tToast.iShowCount,
			pApp->tXui.tToast.iCloseCount,
			pApp->tXui.tToast.iExpireCount,
			pApp->tXui.tToast.iDropCount,
			pApp->iClickCount);
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
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TOAST_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XUI Toast";
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
	return (iExitCode == XGE_OK && tApp.bQueueOK && tApp.bClickOK && tApp.bNoCallbackClickOK && tApp.bCloseButtonOK && tApp.bCloseReenterOK && tApp.bExpireOK) ? 0 : 3;
}
