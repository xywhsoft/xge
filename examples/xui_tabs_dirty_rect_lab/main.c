#include "../../xge.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pCanvasWidget;
	xge_xui_widget pTabsWidget;
	xge_xui_widget pMeasureWidget;
	xge_xui_widget pBackWidget;
	xge_xui_widget pFrontWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_tabs_t tTabs;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iTabsSelectCount;
	int iLastSelected;
	int iDirtyRectCountSeen;
	int iTabsStateSeen;
	int iBackZ;
	int iFrontZ;
	int bContentSizeOK;
	int bTabsMouseOK;
	int bTabsKeyOK;
	int bTabsProcOK;
	int bTabsStateOK;
	int bMeasureOK;
	int bDirtyOK;
	int bZOK;
	int bScriptRan;
	xge_rect_t tDirtyA;
	xge_rect_t tDirtyB;
	xge_vec2_t tMeasuredSize;
} app_state_t;

static const char* g_arrTabs[] = {
	"Core",
	"Layout",
	"Paint",
	"Docs"
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
			printf("xui-tabs-dirty-rect-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-tabs-dirty-rect-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void TabsSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iTabsSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static xge_vec2_t MeasureCard(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;
	xge_vec2_t tSize;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	tSize.fX = 320.0f;
	tSize.fY = 84.0f;
	if ( (pApp != NULL) && (xgeXuiTabsGetSelected(&pApp->tTabs) == 3) ) {
		tSize.fY = 92.0f;
	}
	return tSize;
}

static void MeasureCardPaint(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;
	uint32_t iAccent;
	xge_rect_t tBand;

	pApp = (app_state_t*)pUser;
	if ( (pWidget == NULL) || (pApp == NULL) ) {
		return;
	}
	switch ( xgeXuiTabsGetSelected(&pApp->tTabs) ) {
		case 0:
			iAccent = XGE_COLOR_RGBA(88, 180, 255, 255);
			break;

		case 1:
			iAccent = XGE_COLOR_RGBA(120, 218, 140, 255);
			break;

		case 2:
			iAccent = XGE_COLOR_RGBA(255, 196, 92, 255);
			break;

		default:
			iAccent = XGE_COLOR_RGBA(214, 122, 255, 255);
			break;
	}
	xgeShapeRectStrokePx(pWidget->tRect, 2.0f, iAccent);
	tBand = pWidget->tRect;
	tBand.fX += 10.0f;
	tBand.fY += 10.0f;
	tBand.fW -= 20.0f;
	tBand.fH = 8.0f;
	if ( tBand.fW > 0.0f ) {
		xgeShapeRectFillPx(tBand, iAccent);
	}
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
	tRoot.fY = 18.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 36.0f;
	if ( tRoot.fW < 640.0f ) {
		tRoot.fW = 640.0f;
	}
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pCanvasWidget, (xge_rect_t){ 0.0f, 54.0f, tRoot.fW, tRoot.fH - 54.0f });
	xgeXuiWidgetSetRect(pApp->pTabsWidget, (xge_rect_t){ 22.0f, 18.0f, tRoot.fW - 44.0f, 46.0f });
	xgeXuiWidgetSetRect(pApp->pMeasureWidget, (xge_rect_t){ 22.0f, 92.0f, 0.0f, 0.0f });
	xgeXuiWidgetSetRect(pApp->pBackWidget, (xge_rect_t){ tRoot.fW - 270.0f, 104.0f, 168.0f, 100.0f });
	xgeXuiWidgetSetRect(pApp->pFrontWidget, (xge_rect_t){ tRoot.fW - 230.0f, 138.0f, 176.0f, 108.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunDirtyRectTest(app_state_t* pApp)
{
	xge_rect_t tRect0;
	xge_rect_t tRect1;
	int iCountAfterClear;

	pApp->tDirtyA = (xge_rect_t){ 12.0f, 18.0f, 40.0f, 30.0f };
	pApp->tDirtyB = (xge_rect_t){ 90.0f, 66.0f, 16.0f, 18.0f };
	memset(&tRect0, 0, sizeof(tRect0));
	memset(&tRect1, 0, sizeof(tRect1));

	xgedbgDirtyRectClear();
	xgeInvalidateRect(pApp->tDirtyA);
	xgeInvalidateRect(pApp->tDirtyB);
	pApp->iDirtyRectCountSeen = xgedbgDirtyRectCount();
	pApp->bDirtyOK =
		(pApp->iDirtyRectCountSeen == 2) &&
		(xgedbgDirtyRectGet(0, &tRect0) == XGE_OK) &&
		(xgedbgDirtyRectGet(1, &tRect1) == XGE_OK) &&
		FloatNear(tRect0.fX, pApp->tDirtyA.fX, 0.01f) &&
		FloatNear(tRect0.fY, pApp->tDirtyA.fY, 0.01f) &&
		FloatNear(tRect0.fW, pApp->tDirtyA.fW, 0.01f) &&
		FloatNear(tRect0.fH, pApp->tDirtyA.fH, 0.01f) &&
		FloatNear(tRect1.fX, pApp->tDirtyB.fX, 0.01f) &&
		FloatNear(tRect1.fY, pApp->tDirtyB.fY, 0.01f) &&
		FloatNear(tRect1.fW, pApp->tDirtyB.fW, 0.01f) &&
		FloatNear(tRect1.fH, pApp->tDirtyB.fH, 0.01f);
	xgedbgDirtyRectClear();
	iCountAfterClear = xgedbgDirtyRectCount();
	if ( iCountAfterClear != 0 ) {
		pApp->bDirtyOK = 0;
	}
}

static void RunTabsScript(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPoint;
	xge_rect_t tBackRect;
	xge_rect_t tFrontRect;
	xge_xui_widget pHit;
	float fOverlapX;
	float fOverlapY;

	if ( pApp->bScriptRan ) {
		return;
	}
	if ( pApp->pTabsWidget->tContentRect.fW <= 0.0f ) {
		return;
	}
	pApp->bScriptRan = 1;
	pApp->bTabsProcOK =
		(pApp->pTabsWidget->procEvent == xgeXuiTabsEventProc) &&
		(pApp->pTabsWidget->procPaint == xgeXuiTabsPaintProc);

	tPoint.fX = pApp->pTabsWidget->tContentRect.fX + (pApp->tTabs.fTabWidth * 2.0f) + 20.0f;
	tPoint.fY = pApp->pTabsWidget->tContentRect.fY + (pApp->tTabs.fTabHeight * 0.5f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPoint.fX, tPoint.fY);
	(void)xgeXuiTabsEventProc(pApp->pTabsWidget, &tEvent, &pApp->tTabs);
	xgeXuiSetFocus(&pApp->tXui, pApp->pTabsWidget);
	pApp->iTabsStateSeen = xgeXuiTabsGetState(&pApp->tTabs);
	pApp->bTabsStateOK =
		((pApp->iTabsStateSeen & XGE_XUI_STATE_HOVER) != 0) &&
		((pApp->iTabsStateSeen & XGE_XUI_STATE_FOCUS) != 0);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiTabsEvent(&pApp->tTabs, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPoint.fX, tPoint.fY);
	(void)xgeXuiTabsEvent(&pApp->tTabs, &tEvent);
	pApp->bTabsMouseOK =
		(xgeXuiTabsGetSelected(&pApp->tTabs) == 2) &&
		(pApp->iTabsSelectCount >= 1) &&
		(pApp->iLastSelected == 2);

	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT);
	(void)xgeXuiTabsEvent(&pApp->tTabs, &tEvent);
	pApp->bTabsKeyOK =
		(xgeXuiTabsGetSelected(&pApp->tTabs) == 3) &&
		(pApp->iTabsSelectCount >= 2) &&
		(pApp->iLastSelected == 3);

	pApp->tMeasuredSize = xgeXuiWidgetGetDesiredSize(pApp->pMeasureWidget);
	pApp->bContentSizeOK =
		(pApp->pMeasureWidget->tStyle.tWidth.iUnit == XGE_XUI_SIZE_CONTENT) &&
		(pApp->pMeasureWidget->tStyle.tHeight.iUnit == XGE_XUI_SIZE_CONTENT);
	pApp->bMeasureOK =
		FloatNear(pApp->tMeasuredSize.fX, 220.0f, 0.1f) &&
		FloatNear(pApp->tMeasuredSize.fY, 64.0f, 0.1f) &&
		FloatNear(pApp->pMeasureWidget->tRect.fW, 220.0f, 0.1f) &&
		FloatNear(pApp->pMeasureWidget->tRect.fH, 64.0f, 0.1f);

	pApp->iBackZ = xgeXuiWidgetGetZ(pApp->pBackWidget);
	pApp->iFrontZ = xgeXuiWidgetGetZ(pApp->pFrontWidget);
	tBackRect = pApp->pBackWidget->tRect;
	tFrontRect = pApp->pFrontWidget->tRect;
	fOverlapX = ((tFrontRect.fX > tBackRect.fX) ? tFrontRect.fX : tBackRect.fX) + 14.0f;
	fOverlapY = ((tFrontRect.fY > tBackRect.fY) ? tFrontRect.fY : tBackRect.fY) + 14.0f;
	pHit = xgeXuiHitTest(&pApp->tXui, fOverlapX, fOverlapY);
	pApp->bZOK = (pApp->iBackZ == 2) && (pApp->iFrontZ == 8) && (pHit == pApp->pFrontWidget);
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"tabs=%d/%d/%d measure=%d dirty=%d z=%d content=%d selected=%d cb=%d",
		pApp->bTabsMouseOK,
		pApp->bTabsKeyOK,
		pApp->bTabsStateOK && pApp->bTabsProcOK,
		pApp->bMeasureOK,
		pApp->bDirtyOK,
		pApp->bZOK,
		pApp->bContentSizeOK,
		xgeXuiTabsGetSelected(&pApp->tTabs),
		pApp->iTabsSelectCount);
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
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 244, 248, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(16, 20, 28, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(28, 36, 48, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(90, 106, 128, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(100, 214, 255, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(72, 110, 170, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(52, 92, 142, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(74, 80, 88, 190);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 10.0f;
	tTheme.fSpacing = 12.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pCanvasWidget = xgeXuiWidgetCreate();
	pApp->pTabsWidget = xgeXuiWidgetCreate();
	pApp->pMeasureWidget = xgeXuiWidgetCreate();
	pApp->pBackWidget = xgeXuiWidgetCreate();
	pApp->pFrontWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pCanvasWidget == NULL) ||
	     (pApp->pTabsWidget == NULL) || (pApp->pMeasureWidget == NULL) || (pApp->pBackWidget == NULL) ||
	     (pApp->pFrontWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pCanvasWidget);
	xgeXuiWidgetAdd(pApp->pCanvasWidget, pApp->pTabsWidget);
	xgeXuiWidgetAdd(pApp->pCanvasWidget, pApp->pMeasureWidget);
	xgeXuiWidgetAdd(pApp->pCanvasWidget, pApp->pBackWidget);
	xgeXuiWidgetAdd(pApp->pCanvasWidget, pApp->pFrontWidget);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 24, 32, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 10.0f);
	xgeXuiWidgetSetBackground(pApp->pCanvasWidget, XGE_COLOR_RGBA(24, 32, 44, 255));
	xgeXuiWidgetSetRadius(pApp->pCanvasWidget, 10.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "tabs=0/0/0 measure=0 dirty=0 z=0 content=0") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(242, 246, 250, 255));
	xgeXuiLabelSetAlign(&pApp->tStatusLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);

	if ( xgeXuiTabsInit(&pApp->tTabs, &pApp->tXui, pApp->pTabsWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiTabsSetItems(&pApp->tTabs, g_arrTabs, (int)(sizeof(g_arrTabs) / sizeof(g_arrTabs[0])));
	xgeXuiTabsSetFont(&pApp->tTabs, pFont);
	xgeXuiTabsSetSelect(&pApp->tTabs, TabsSelect, pApp);
	xgeXuiTabsSetSelected(&pApp->tTabs, 1);
	xgeXuiTabsSetTabSize(&pApp->tTabs, 112.0f, 34.0f);
	xgeXuiTabsSetColors(
		&pApp->tTabs,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(42, 54, 72, 255),
		XGE_COLOR_RGBA(70, 102, 148, 255),
		XGE_COLOR_RGBA(94, 186, 255, 255),
		XGE_COLOR_RGBA(52, 86, 140, 255),
		XGE_COLOR_RGBA(86, 88, 96, 255),
		XGE_COLOR_RGBA(230, 236, 244, 255),
		XGE_COLOR_RGBA(18, 24, 32, 255));

	xgeXuiWidgetSetSize(pApp->pMeasureWidget, xgeXuiSizeContent(), xgeXuiSizeContent());
	xgeXuiWidgetSetMinSize(pApp->pMeasureWidget, xgeXuiSizePx(140.0f), xgeXuiSizePx(50.0f));
	xgeXuiWidgetSetMaxSize(pApp->pMeasureWidget, xgeXuiSizePx(220.0f), xgeXuiSizePx(64.0f));
	xgeXuiWidgetSetBackground(pApp->pMeasureWidget, XGE_COLOR_RGBA(34, 48, 66, 255));
	xgeXuiWidgetSetRadius(pApp->pMeasureWidget, 8.0f);
	xgeXuiWidgetSetPaddingPx(pApp->pMeasureWidget, 12.0f, 12.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetMeasure(pApp->pMeasureWidget, MeasureCard);
	xgeXuiWidgetSetPaint(pApp->pMeasureWidget, MeasureCardPaint, pApp);

	xgeXuiWidgetSetBackground(pApp->pBackWidget, XGE_COLOR_RGBA(255, 176, 92, 180));
	xgeXuiWidgetSetRadius(pApp->pBackWidget, 8.0f);
	xgeXuiWidgetSetZ(pApp->pBackWidget, 2);
	xgeXuiWidgetSetBackground(pApp->pFrontWidget, XGE_COLOR_RGBA(92, 198, 255, 210));
	xgeXuiWidgetSetRadius(pApp->pFrontWidget, 8.0f);
	xgeXuiWidgetSetZ(pApp->pFrontWidget, 8);

	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiTabsUnit(&pApp->tTabs);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	memset(pApp, 0, sizeof(*pApp));
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	xge_xui_size_t tContent;

	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	tContent = xgeXuiSizeContent();
	pApp->bContentSizeOK = (tContent.iUnit == XGE_XUI_SIZE_CONTENT);
	if ( CreateUI(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	RunDirtyRectTest(pApp);
	return XGE_OK;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	RunTabsScript(pApp);
	UpdateStatus(pApp);

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(12, 16, 22, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-tabs-dirty-rect-lab final-summary frames=%d tabs=%d/%d/%d/%d measure=%d dirty=%d z=%d content=%d selected=%d state=%d callbacks=%d dirty=%d desired=%.2fx%.2f z=%d/%d\n",
			pApp->iFrameCount,
			pApp->bTabsMouseOK,
			pApp->bTabsKeyOK,
			pApp->bTabsStateOK,
			pApp->bTabsProcOK,
			pApp->bMeasureOK,
			pApp->bDirtyOK,
			pApp->bZOK,
			pApp->bContentSizeOK,
			xgeXuiTabsGetSelected(&pApp->tTabs),
			pApp->iTabsStateSeen,
			pApp->iTabsSelectCount,
			pApp->iDirtyRectCountSeen,
			pApp->tMeasuredSize.fX,
			pApp->tMeasuredSize.fY,
			pApp->iBackZ,
			pApp->iFrontZ);
		printf("xui-tabs-dirty-rect-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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

	iFrameLimit = 0;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 800;
	tDesc.iHeight = 480;
	tDesc.sTitle = "XGE XUI Tabs Dirty Rect Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	iExitCode =
		(tApp.bTabsMouseOK && tApp.bTabsKeyOK && tApp.bTabsStateOK && tApp.bTabsProcOK &&
		 tApp.bMeasureOK && tApp.bDirtyOK && tApp.bZOK && tApp.bContentSizeOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

