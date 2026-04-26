#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t app_state_t;

typedef struct event_probe_t {
	app_state_t* pApp;
	xge_xui_button pButton;
	const char* sName;
	int iCaptureCalls;
	int iEventCalls;
	int iFocusIn;
	int iFocusOut;
	int iPointerEnter;
	int iPointerLeave;
	int iCaptureLost;
	int iContextBegin;
	int iContextEnd;
	int iClicks;
} event_probe_t;

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pNoteWidget;
	xge_xui_widget pLeftButtonWidget;
	xge_xui_widget pRightButtonWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tNoteLabel;
	xge_xui_button_t tLeftButton;
	xge_xui_button_t tRightButton;
	event_probe_t tLeftProbe;
	event_probe_t tRightProbe;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bHitOK;
	int bDispatchOK;
	int bQueueOK;
	int bFocusOK;
	int bPointerOK;
	int bCaptureOK;
	int bKeynavOK;
	int bContextOK;
	int bCaptureHookOK;
	int iDispatchConsumed;
	int iQueuedProcessed;
	int iQueuedCountBefore;
	int iQueuedCountAfter;
	int bChecksOK;
} app_state_t;

static app_state_t* g_pCurrentApp = NULL;

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
		"C:/Windows/Fonts/simhei.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], 18.0f) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui-focus-capture-event-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-focus-capture-event-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"hit=%d dispatch=%d queue=%d focus=%d pointer=%d capture=%d keynav=%d context=%d hooks=%d",
		pApp->bHitOK,
		pApp->bDispatchOK,
		pApp->bQueueOK,
		pApp->bFocusOK,
		pApp->bPointerOK,
		pApp->bCaptureOK,
		pApp->bKeynavOK,
		pApp->bContextOK,
		pApp->bCaptureHookOK);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	xge_rect_t tStatus;
	xge_rect_t tNote;
	xge_rect_t tLeft;
	xge_rect_t tRight;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRoot.fX = 24.0f;
	tRoot.fY = 24.0f;
	tRoot.fW = (float)iWidth - 48.0f;
	tRoot.fH = (float)iHeight - 48.0f;
	if ( tRoot.fW < 420.0f ) {
		tRoot.fW = 420.0f;
	}
	if ( tRoot.fH < 220.0f ) {
		tRoot.fH = 220.0f;
	}
	tStatus.fX = 0.0f;
	tStatus.fY = 0.0f;
	tStatus.fW = tRoot.fW;
	tStatus.fH = 38.0f;
	tNote.fX = 0.0f;
	tNote.fY = tRoot.fH - 42.0f;
	tNote.fW = tRoot.fW;
	tNote.fH = 42.0f;
	tLeft.fX = 40.0f;
	tLeft.fY = 84.0f;
	tLeft.fW = 168.0f;
	tLeft.fH = 56.0f;
	tRight.fX = 236.0f;
	tRight.fY = 84.0f;
	tRight.fW = 168.0f;
	tRight.fH = 56.0f;

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, tStatus);
	xgeXuiWidgetSetRect(pApp->pNoteWidget, tNote);
	xgeXuiWidgetSetRect(pApp->pLeftButtonWidget, tLeft);
	xgeXuiWidgetSetRect(pApp->pRightButtonWidget, tRight);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void ButtonClick(xge_xui_widget pWidget, void* pUser)
{
	event_probe_t* pProbe;

	(void)pWidget;
	pProbe = (event_probe_t*)pUser;
	if ( pProbe != NULL ) {
		pProbe->iClicks++;
	}
}

static event_probe_t* ProbeFromWidget(xge_xui_widget pWidget)
{
	if ( (g_pCurrentApp == NULL) || (pWidget == NULL) ) {
		return NULL;
	}
	if ( pWidget == g_pCurrentApp->pLeftButtonWidget ) {
		return &g_pCurrentApp->tLeftProbe;
	}
	if ( pWidget == g_pCurrentApp->pRightButtonWidget ) {
		return &g_pCurrentApp->tRightProbe;
	}
	return NULL;
}

static int ProbeCaptureEvent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	event_probe_t* pProbe;

	(void)pUser;
	pProbe = ProbeFromWidget(pWidget);
	if ( (pProbe == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pProbe->iCaptureCalls++;
	return XGE_XUI_EVENT_CONTINUE;
}

static int ProbeButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	event_probe_t* pProbe;
	xge_xui_button pButton;

	pProbe = ProbeFromWidget(pWidget);
	pButton = (xge_xui_button)pUser;
	if ( (pProbe == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pProbe->iEventCalls++;
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_FOCUS_IN:
			pProbe->iFocusIn++;
			break;

		case XGE_EVENT_XUI_FOCUS_OUT:
			pProbe->iFocusOut++;
			break;

		case XGE_EVENT_XUI_POINTER_ENTER:
			pProbe->iPointerEnter++;
			break;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pProbe->iPointerLeave++;
			break;

		case XGE_EVENT_XUI_CAPTURE_LOST:
			pProbe->iCaptureLost++;
			break;

		case XGE_EVENT_XUI_CONTEXT_BEGIN:
			pProbe->iContextBegin++;
			break;

		case XGE_EVENT_XUI_CONTEXT_END:
			pProbe->iContextEnd++;
			break;

		default:
			break;
	}
	return xgeXuiButtonEvent((pButton != NULL) ? pButton : pProbe->pButton, pEvent);
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iMods)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iMods;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_rect_t tLeft;
	xge_rect_t tRight;
	xge_event_t tEvent;
	xge_event_t tQueued;
	xge_xui_widget pHitLeft;
	xge_xui_widget pHitRight;
	float fLeftX;
	float fLeftY;
	float fRightX;
	float fRightY;
	int iRetEsc;
	int iProcessed;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	tLeft = pApp->pLeftButtonWidget->tRect;
	tRight = pApp->pRightButtonWidget->tRect;
	fLeftX = tLeft.fX + tLeft.fW * 0.5f;
	fLeftY = tLeft.fY + tLeft.fH * 0.5f;
	fRightX = tRight.fX + tRight.fW * 0.5f;
	fRightY = tRight.fY + tRight.fH * 0.5f;

	pHitLeft = xgeXuiHitTest(&pApp->tXui, fLeftX, fLeftY);
	pHitRight = xgeXuiHitTest(&pApp->tXui, fRightX, fRightY);
	pApp->bHitOK = (pHitLeft == pApp->pLeftButtonWidget) && (pHitRight == pApp->pRightButtonWidget);

	xgeXuiSetFocus(&pApp->tXui, pApp->pLeftButtonWidget);
	xgeXuiSetCapture(&pApp->tXui, pApp->pRightButtonWidget);
	xgeXuiSetCapture(&pApp->tXui, pApp->pLeftButtonWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ESCAPE, 0);
	iRetEsc = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, fLeftX, fLeftY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, fRightX, fRightY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_RIGHT, fRightX, fRightY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_RIGHT, fRightX, fRightY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);

	MakeKeyEvent(&tEvent, XGE_KEY_TAB, 0);
	if ( xgeXuiEventPush(&pApp->tXui, &tEvent) == XGE_OK ) {
		pApp->iQueuedCountBefore = xgeXuiEventCount(&pApp->tXui);
	}
	memset(&tQueued, 0, sizeof(tQueued));
	if ( xgeXuiEventPop(&pApp->tXui, &tQueued) == XGE_OK ) {
		xgeXuiDispatchEvent(&pApp->tXui, &tQueued);
	}

	MakeKeyEvent(&tEvent, XGE_KEY_TAB, XGE_KEY_MOD_SHIFT);
	xgeXuiEventPush(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER, 0);
	xgeXuiEventPush(&pApp->tXui, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE, 0);
	xgeXuiEventPush(&pApp->tXui, &tEvent);
	iProcessed = xgeXuiDispatchQueuedEvents(&pApp->tXui);
	pApp->iQueuedProcessed = iProcessed;
	pApp->iQueuedCountAfter = xgeXuiEventCount(&pApp->tXui);
	pApp->iDispatchConsumed = iRetEsc;

	pApp->bDispatchOK = (iRetEsc == XGE_XUI_EVENT_CONSUMED) && (pApp->tXui.pCapture == NULL);
	pApp->bQueueOK = (pApp->iQueuedCountBefore == 1) && (iProcessed == 3) && (pApp->iQueuedCountAfter == 0);
	pApp->bFocusOK =
		(pApp->tXui.pFocus == pApp->pLeftButtonWidget) &&
		(pApp->tLeftProbe.iFocusIn >= 2) &&
		(pApp->tLeftProbe.iFocusOut >= 1) &&
		(pApp->tRightProbe.iFocusIn >= 1) &&
		(pApp->tRightProbe.iFocusOut >= 1);
	pApp->bPointerOK =
		(pApp->tXui.pHover == pApp->pRightButtonWidget) &&
		(pApp->tLeftProbe.iPointerEnter >= 1) &&
		(pApp->tLeftProbe.iPointerLeave >= 1) &&
		(pApp->tRightProbe.iPointerEnter >= 1);
	pApp->bCaptureOK =
		(pApp->tRightProbe.iCaptureLost >= 1) &&
		(pApp->tLeftProbe.iCaptureLost >= 1);
	pApp->bKeynavOK =
		(pApp->tLeftProbe.iClicks >= 2) &&
		(pApp->tRightProbe.iClicks == 0);
	pApp->bContextOK =
		(pApp->tRightProbe.iContextBegin >= 1) &&
		(pApp->tRightProbe.iContextEnd >= 1);
	pApp->bCaptureHookOK =
		(pApp->pLeftButtonWidget->procCaptureEvent == ProbeCaptureEvent) &&
		(pApp->pLeftButtonWidget->pCaptureUser == &pApp->tLeftProbe) &&
		(pApp->pRightButtonWidget->procCaptureEvent == ProbeCaptureEvent) &&
		(pApp->pRightButtonWidget->pCaptureUser == NULL);
	pApp->bChecksOK =
		pApp->bHitOK &&
		pApp->bDispatchOK &&
		pApp->bQueueOK &&
		pApp->bFocusOK &&
		pApp->bPointerOK &&
		pApp->bCaptureOK &&
		pApp->bKeynavOK &&
		pApp->bContextOK &&
		pApp->bCaptureHookOK;
	UpdateStatus(pApp);
	if ( pApp->bChecksOK == 0 ) {
		printf(
			"xui-focus-capture-event-lab checks pending hit=%d dispatch=%d queue=%d focus=%d pointer=%d capture=%d keynav=%d context=%d hooks=%d\n",
			pApp->bHitOK,
			pApp->bDispatchOK,
			pApp->bQueueOK,
			pApp->bFocusOK,
			pApp->bPointerOK,
			pApp->bCaptureOK,
			pApp->bKeynavOK,
			pApp->bContextOK,
			pApp->bCaptureHookOK);
	}
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_theme_t tTheme;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(236, 240, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(24, 30, 42, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(92, 164, 255, 255);
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 8.0f;
	tTheme.fRadius = 6.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pNoteWidget = xgeXuiWidgetCreate();
	pApp->pLeftButtonWidget = xgeXuiWidgetCreate();
	pApp->pRightButtonWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pNoteWidget == NULL) || (pApp->pLeftButtonWidget == NULL) || (pApp->pRightButtonWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(24, 30, 42, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 76, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui focus/capture/event lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);

	xgeXuiWidgetSetBackground(pApp->pNoteWidget, XGE_COLOR_RGBA(34, 42, 56, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pNoteWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tNoteLabel, pApp->pNoteWidget, pFont, "covers hit-test, dispatch, queue, focus, capture, Tab/Shift+Tab, Enter/Space, right click");
	xgeXuiLabelSetColor(&pApp->tNoteLabel, XGE_COLOR_RGBA(214, 223, 237, 255));
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pNoteWidget);

	xgeXuiButtonInit(&pApp->tLeftButton, &pApp->tXui, pApp->pLeftButtonWidget);
	xgeXuiButtonSetText(&pApp->tLeftButton, pFont, "Primary");
	xgeXuiButtonSetTextColor(&pApp->tLeftButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetClick(&pApp->tLeftButton, ButtonClick, &pApp->tLeftProbe);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pLeftButtonWidget);

	xgeXuiButtonInit(&pApp->tRightButton, &pApp->tXui, pApp->pRightButtonWidget);
	xgeXuiButtonSetText(&pApp->tRightButton, pFont, "Secondary");
	xgeXuiButtonSetTextColor(&pApp->tRightButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetClick(&pApp->tRightButton, ButtonClick, &pApp->tRightProbe);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pRightButtonWidget);

	memset(&pApp->tLeftProbe, 0, sizeof(pApp->tLeftProbe));
	memset(&pApp->tRightProbe, 0, sizeof(pApp->tRightProbe));
	pApp->tLeftProbe.pApp = pApp;
	pApp->tLeftProbe.pButton = &pApp->tLeftButton;
	pApp->tLeftProbe.sName = "left";
	pApp->tRightProbe.pApp = pApp;
	pApp->tRightProbe.pButton = &pApp->tRightButton;
	pApp->tRightProbe.sName = "right";

	pApp->pLeftButtonWidget->procEvent = ProbeButtonEventProc;
	xgeXuiWidgetSetCaptureEventUser(pApp->pLeftButtonWidget, ProbeCaptureEvent, &pApp->tLeftProbe);

	pApp->pRightButtonWidget->procEvent = ProbeButtonEventProc;
	xgeXuiWidgetSetCaptureEvent(pApp->pRightButtonWidget, ProbeCaptureEvent);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	g_pCurrentApp = pApp;
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		printf("xui-focus-capture-event-lab xui init failed\n");
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	if ( CreateUI(pApp) != XGE_OK ) {
		printf("xui-focus-capture-event-lab create ui failed\n");
		xgeXuiUnit(&pApp->tXui);
		if ( pApp->bFontReady ) {
			xgeFontFree(&pApp->tFont);
		}
		return XGE_ERROR;
	}
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiButtonUnit(&pApp->tLeftButton);
	xgeXuiButtonUnit(&pApp->tRightButton);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
	if ( g_pCurrentApp == pApp ) {
		g_pCurrentApp = NULL;
	}
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-focus-capture-event-lab final-summary frames=%d hit=%d dispatch=%d queue=%d focus=%d pointer=%d capture=%d keynav=%d context=%d hooks=%d left(click=%d focus=%d/%d ptr=%d/%d lost=%d ctx=%d/%d cap=%d) right(click=%d focus=%d/%d ptr=%d/%d lost=%d ctx=%d/%d cap=%d) queue=%d/%d/%d consumed=%d\n",
			pApp->iFrameCount,
			pApp->bHitOK,
			pApp->bDispatchOK,
			pApp->bQueueOK,
			pApp->bFocusOK,
			pApp->bPointerOK,
			pApp->bCaptureOK,
			pApp->bKeynavOK,
			pApp->bContextOK,
			pApp->bCaptureHookOK,
			pApp->tLeftProbe.iClicks,
			pApp->tLeftProbe.iFocusIn,
			pApp->tLeftProbe.iFocusOut,
			pApp->tLeftProbe.iPointerEnter,
			pApp->tLeftProbe.iPointerLeave,
			pApp->tLeftProbe.iCaptureLost,
			pApp->tLeftProbe.iContextBegin,
			pApp->tLeftProbe.iContextEnd,
			pApp->tLeftProbe.iCaptureCalls,
			pApp->tRightProbe.iClicks,
			pApp->tRightProbe.iFocusIn,
			pApp->tRightProbe.iFocusOut,
			pApp->tRightProbe.iPointerEnter,
			pApp->tRightProbe.iPointerLeave,
			pApp->tRightProbe.iCaptureLost,
			pApp->tRightProbe.iContextBegin,
			pApp->tRightProbe.iContextEnd,
			pApp->tRightProbe.iCaptureCalls,
			pApp->iQueuedCountBefore,
			pApp->iQueuedProcessed,
			pApp->iQueuedCountAfter,
			pApp->iDispatchConsumed);
		printf("xui-focus-capture-event-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return 0;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_FOCUS_CAPTURE_EVENT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 520;
	tDesc.iHeight = 240;
	tDesc.sTitle = "XGE XUI Focus Capture Event Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-focus-capture-event-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}

