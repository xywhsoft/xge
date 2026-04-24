#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrItems[] = {
	"Alpha Harbor",
	"Bravo Yard",
	"Charlie Depot",
	"Delta Relay",
	"Echo Lab",
	"Foxtrot Wing",
	"Golf Station",
	"Hotel Annex",
	"India Core",
	"Juliet Tower",
	"Kilo Archive",
	"Lima Dock"
};

static const int g_arrEnabled[] = {
	1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pListWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_list_view_t tList;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iSelectCount;
	int iLastSelected;
	int bInitOK;
	int bConfigOK;
	int bMouseOK;
	int bWheelOK;
	int bDragOK;
	int bKeyboardOK;
	int bDisabledOK;
	int bDirectPaintOK;
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

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return fabsf(fA - fB) <= fEpsilon;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeWheelEvent(xge_event_t* pEvent, float fX, float fY, float fDY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDY = fDY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
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
			printf("xui-list-view-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-list-view-proc-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static xge_rect_t ListThumbRect(xge_xui_list_view pList)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fTrackLen;
	float fLen;
	float fMaxScroll;

	tBar.fX = pList->pWidget->tContentRect.fX + pList->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pList->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pList->pWidget->tContentRect.fH;
	tThumb = tBar;
	fContentH = (float)pList->iItemCount * pList->fItemHeight;
	fTrackLen = tBar.fH;
	if ( fContentH <= pList->pWidget->tContentRect.fH ) {
		return tThumb;
	}
	fLen = fTrackLen * (pList->pWidget->tContentRect.fH / fContentH);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	tThumb.fH = fLen;
	fMaxScroll = fContentH - pList->pWidget->tContentRect.fH;
	if ( fMaxScroll > 0.0f && fTrackLen > fLen ) {
		tThumb.fY += (fTrackLen - fLen) * (pList->fScrollY / fMaxScroll);
	}
	return tThumb;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	tRoot.fX = 20.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 520.0f ) {
		tRoot.fW = 520.0f;
	}
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pListWidget, (xge_rect_t){ 26.0f, 64.0f, 280.0f, 220.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d config=%d mouse=%d wheel=%d drag=%d keys=%d disabled=%d paint=%d sel=%d scroll=%.2f cb=%d last=%d",
		pApp->bInitOK,
		pApp->bConfigOK,
		pApp->bMouseOK,
		pApp->bWheelOK,
		pApp->bDragOK,
		pApp->bKeyboardOK,
		pApp->bDisabledOK,
		pApp->bDirectPaintOK,
		xgeXuiListViewGetSelected(&pApp->tList),
		xgeXuiListViewGetScroll(&pApp->tList),
		pApp->iSelectCount,
		pApp->iLastSelected);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(18, 22, 30, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(34, 42, 56, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 74, 100, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(74, 98, 128, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(52, 104, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(90, 110, 140, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 88, 94, 180);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pListWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pListWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pListWidget);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(42, 54, 72, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui list view proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiListViewInit(&pApp->tList, &pApp->tXui, pApp->pListWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tThumbStart;
	xge_rect_t tThumbEnd;
	float fScrollBefore;
	float fScrollAfter;
	int iRet;
	int iStartSelect;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->pListWidget->procEvent == xgeXuiListViewEventProc) &&
		(pApp->pListWidget->procPaint == xgeXuiListViewPaintProc) &&
		(xgeXuiWidgetIsFocusable(pApp->pListWidget) != 0) &&
		((pApp->pListWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);

	xgeXuiListViewSetFont(&pApp->tList, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiListViewSetItems(&pApp->tList, g_arrItems, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiListViewSetEnabledItems(&pApp->tList, g_arrEnabled, (int)(sizeof(g_arrEnabled) / sizeof(g_arrEnabled[0])));
	xgeXuiListViewSetItemHeight(&pApp->tList, 28.0f);
	xgeXuiListViewSetColors(&pApp->tList, XGE_COLOR_RGBA(28, 34, 42, 255), XGE_COLOR_RGBA(40, 48, 60, 255), XGE_COLOR_RGBA(70, 124, 210, 255), XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(70, 82, 98, 180), XGE_COLOR_RGBA(190, 204, 224, 228));
	xgeXuiListViewSetDisabledTextColor(&pApp->tList, XGE_COLOR_RGBA(130, 138, 150, 220));
	xgeXuiListViewSetSelect(&pApp->tList, ListSelect, pApp);
	xgeXuiListViewSetSelected(&pApp->tList, 2);
	xgeXuiListViewSetScroll(&pApp->tList, 14.0f);
	pApp->bConfigOK =
		(xgeXuiListViewGetSelected(&pApp->tList) == 2) &&
		FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 14.0f, 0.01f) &&
		FloatNear(pApp->tList.fItemHeight, 28.0f, 0.01f) &&
		(pApp->tList.pFont == (pApp->bFontReady ? &pApp->tFont : NULL));

	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->pListWidget->tContentRect.fX + 10.0f,
		pApp->pListWidget->tContentRect.fY + pApp->tList.fItemHeight * 3.0f + 8.0f - xgeXuiListViewGetScroll(&pApp->tList));
	iRet = xgeXuiListViewEvent(&pApp->tList, &tEvent);
	pApp->bMouseOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 3) &&
		(pApp->iSelectCount >= 1) &&
		(pApp->iLastSelected == 3);

	iStartSelect = xgeXuiListViewGetSelected(&pApp->tList);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->pListWidget->tContentRect.fX + 10.0f,
		pApp->pListWidget->tContentRect.fY + pApp->tList.fItemHeight * 4.0f + 8.0f - xgeXuiListViewGetScroll(&pApp->tList));
	iRet = xgeXuiListViewEventProc(pApp->pListWidget, &tEvent, &pApp->tList);
	pApp->bDisabledOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == iStartSelect);

	fScrollBefore = xgeXuiListViewGetScroll(&pApp->tList);
	MakeWheelEvent(
		&tEvent,
		pApp->pListWidget->tContentRect.fX + 20.0f,
		pApp->pListWidget->tContentRect.fY + 20.0f,
		-2.0f);
	iRet = xgeXuiListViewEvent(&pApp->tList, &tEvent);
	fScrollAfter = xgeXuiListViewGetScroll(&pApp->tList);
	pApp->bWheelOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(fScrollAfter > fScrollBefore);

	tThumbStart = ListThumbRect(&pApp->tList);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumbStart.fX + 1.0f, tThumbStart.fY + 1.0f);
	iRet = xgeXuiListViewEventProc(pApp->pListWidget, &tEvent, &pApp->tList);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tThumbStart.fX + 1.0f, tThumbStart.fY + tThumbStart.fH + 36.0f);
	(void)xgeXuiListViewEvent(&pApp->tList, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tThumbStart.fX + 1.0f, tThumbStart.fY + tThumbStart.fH + 36.0f);
	(void)xgeXuiListViewEvent(&pApp->tList, &tEvent);
	tThumbEnd = ListThumbRect(&pApp->tList);
	pApp->bDragOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(tThumbEnd.fY > tThumbStart.fY + 4.0f) &&
		(xgeXuiListViewGetScroll(&pApp->tList) > fScrollAfter);

	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_END);
	iRet = xgeXuiListViewEvent(&pApp->tList, &tEvent);
	pApp->bKeyboardOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 11);
	MakeKeyEvent(&tEvent, XGE_KEY_UP);
	(void)xgeXuiListViewEventProc(pApp->pListWidget, &tEvent, &pApp->tList);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 10);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	(void)xgeXuiListViewEvent(&pApp->tList, &tEvent);
	pApp->bKeyboardOK =
		pApp->bKeyboardOK &&
		(pApp->iLastSelected == 10) &&
		(pApp->iSelectCount >= 2);

	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastSelected = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiListViewUnit(&pApp->tList);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	memset(pApp, 0, sizeof(*pApp));
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
	if ( pApp->bDirectPaintOK == 0 ) {
		xgeXuiListViewPaintProc(pApp->pListWidget, &pApp->tList);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-list-view-proc-lab final-summary frames=%d init=%d config=%d mouse=%d wheel=%d drag=%d keys=%d disabled=%d paint=%d list(selected=%d scroll=%.2f item_h=%.2f hover=%d cb=%d last=%d)\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bConfigOK,
			pApp->bMouseOK,
			pApp->bWheelOK,
			pApp->bDragOK,
			pApp->bKeyboardOK,
			pApp->bDisabledOK,
			pApp->bDirectPaintOK,
			xgeXuiListViewGetSelected(&pApp->tList),
			xgeXuiListViewGetScroll(&pApp->tList),
			pApp->tList.fItemHeight,
			pApp->tList.iHover,
			pApp->iSelectCount,
			pApp->iLastSelected);
		printf("xui-list-view-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	int iExitCode;

	iFrameLimit = 180;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 560;
	tDesc.iHeight = 400;
	tDesc.sTitle = "XGE XUI List View Proc Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		AppUnit(&tApp);
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bInitOK && tApp.bConfigOK && tApp.bMouseOK && tApp.bWheelOK &&
		 tApp.bDragOK && tApp.bKeyboardOK && tApp.bDisabledOK && tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}
