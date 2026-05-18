#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* g_arrCards[] = {
	"North Route",
	"Cargo Lift",
	"Signal Mesh",
	"Foundry Bay",
	"Control Wing",
	"Archive Node",
	"Relay Bridge",
	"Sensor Deck",
	"Cooling Loop",
	"Transit Core",
	"Dock Frame",
	"Vector Lab"
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pScrollWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_scroll_view_t tScroll;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bInitOK;
	int bConfigOK;
	int bWheelOK;
	int bDragOK;
	int bVerticalOK;
	int bHorizontalOK;
	int bFrameOK;
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

static void MakeWheelEvent(xge_event_t* pEvent, float fX, float fY, float fDX, float fDY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDX = fDX;
	pEvent->fDY = fDY;
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
			printf("xui-scroll-view-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-scroll-view-proc-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static float ScrollBarButtonSize(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tRect;
	float fLong;
	float fCross;
	float fSize;

	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		return 0.0f;
	}
	if ( pScrollBar->iButtonMode == XGE_XUI_SCROLLBAR_BUTTONS_OFF ) {
		return 0.0f;
	}
	tRect = pScrollBar->pWidget->tContentRect;
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		fLong = tRect.fW;
		fCross = tRect.fH;
	} else {
		fLong = tRect.fH;
		fCross = tRect.fW;
	}
	if ( pScrollBar->iButtonMode != XGE_XUI_SCROLLBAR_BUTTONS_ON && (fLong < 32.0f || fCross < 8.0f) ) {
		return 0.0f;
	}
	fSize = (pScrollBar->fButtonSize > 0.0f) ? pScrollBar->fButtonSize : fCross;
	return (fSize > fCross) ? fCross : fSize;
}

static xge_rect_t ScrollBarTrackRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	float fButton;

	tTrack = pScrollBar->pWidget->tContentRect;
	fButton = ScrollBarButtonSize(pScrollBar);
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tTrack.fX += fButton;
		tTrack.fW -= fButton * 2.0f;
	} else {
		tTrack.fY += fButton;
		tTrack.fH -= fButton * 2.0f;
	}
	if ( tTrack.fW < 0.0f ) {
		tTrack.fW = 0.0f;
	}
	if ( tTrack.fH < 0.0f ) {
		tTrack.fH = 0.0f;
	}
	return tTrack;
}

static xge_rect_t ScrollBarThumbRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	xge_rect_t tThumb;
	float fTrackLen;
	float fLen;
	float fRange;
	float fRate;

	tTrack = ScrollBarTrackRect(pScrollBar);
	tThumb = tTrack;
	fTrackLen = (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? tTrack.fW : tTrack.fH;
	fRange = pScrollBar->fMax - pScrollBar->fMin;
	if ( fRange <= 0.0f ) {
		return tThumb;
	}
	fLen = fTrackLen * (pScrollBar->fPage / (fRange + pScrollBar->fPage));
	if ( fLen < pScrollBar->fMinThumbSize ) {
		fLen = pScrollBar->fMinThumbSize;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	fRate = (pScrollBar->fValue - pScrollBar->fMin) / fRange;
	if ( fRate < 0.0f ) {
		fRate = 0.0f;
	}
	if ( fRate > 1.0f ) {
		fRate = 1.0f;
	}
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tThumb.fX += (fTrackLen - fLen) * fRate;
		tThumb.fW = fLen;
	} else {
		tThumb.fY += (fTrackLen - fLen) * fRate;
		tThumb.fH = fLen;
	}
	return tThumb;
}

static xge_rect_t VerticalThumbRect(xge_xui_scroll_view pScroll)
{
	return ScrollBarThumbRect(&pScroll->tFrame.tVScrollBar);
}

static xge_rect_t HorizontalThumbRect(xge_xui_scroll_view pScroll)
{
	return ScrollBarThumbRect(&pScroll->tFrame.tHScrollBar);
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
	if ( tRoot.fW < 620.0f ) {
		tRoot.fW = 620.0f;
	}
	if ( tRoot.fH < 420.0f ) {
		tRoot.fH = 420.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 40.0f });
	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 24.0f, 64.0f, 360.0f, 240.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];
	float fX;
	float fY;

	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	snprintf(
		sText,
		sizeof(sText),
		"init=%d config=%d wheel=%d drag=%d vbar=%d hbar=%d frame=%d offset=%.2f,%.2f dragmode=%d",
		pApp->bInitOK,
		pApp->bConfigOK,
		pApp->bWheelOK,
		pApp->bDragOK,
		pApp->bVerticalOK,
		pApp->bHorizontalOK,
		pApp->bFrameOK,
		fX,
		fY,
		pApp->tScroll.tFrame.bDraggingContent);
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
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pScrollWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pScrollWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollWidget);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(42, 54, 72, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);

	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui scroll view proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScrollWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_rect_t tThumb;
	float fX;
	float fY;
	float fMaxX;
	float fMaxY;
	int iRet;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	pApp->bInitOK =
		(pApp->pScrollWidget->procEvent == xgeXuiScrollViewEventProc) &&
		(pApp->pScrollWidget->procLayout == xgeXuiScrollViewLayoutProc) &&
		(pApp->tScroll.pContentWidget != NULL) &&
		(pApp->tScroll.tFrame.pViewportWidget != NULL) &&
		((pApp->tScroll.tFrame.pViewportWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);

	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 720.0f, 520.0f);
	xgeXuiScrollViewSetColors(&pApp->tScroll, XGE_COLOR_RGBA(28, 34, 42, 255), XGE_COLOR_RGBA(72, 84, 100, 190), XGE_COLOR_RGBA(196, 214, 238, 245));
	xgeXuiScrollViewSetWheelAxis(&pApp->tScroll, XGE_XUI_WHEEL_AXIS_BOTH);
	xgeXuiScrollViewSetContentDragEnabled(&pApp->tScroll, 1);
	xgeXuiScrollModelGetMaxOffset(&pApp->tScroll.tModel, &fMaxX, &fMaxY);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 999.0f, 999.0f);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	pApp->bConfigOK =
		FloatNear(fX, fMaxX, 0.01f) &&
		FloatNear(fY, fMaxY, 0.01f) &&
		(pApp->pScrollWidget->tStyle.iBackgroundColor == XGE_COLOR_RGBA(28, 34, 42, 255)) &&
		(pApp->tScroll.tFrame.tHScrollBar.iColorTrack == XGE_COLOR_RGBA(72, 84, 100, 190)) &&
		(pApp->tScroll.tFrame.tVScrollBar.iColorThumb == XGE_COLOR_RGBA(196, 214, 238, 245));
	pApp->bFrameOK =
		(pApp->tScroll.tFrame.bShowHScroll != 0) &&
		(pApp->tScroll.tFrame.bShowVScroll != 0) &&
		(pApp->tScroll.pContentWidget->pParent == pApp->tScroll.tFrame.pViewportWidget);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	MakeWheelEvent(
		&tEvent,
		pApp->tScroll.tFrame.tViewportRect.fX + 30.0f,
		pApp->tScroll.tFrame.tViewportRect.fY + 30.0f,
		-1.0f,
		-1.0f);
	iRet = xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	pApp->bWheelOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(fX > 0.0f) &&
		(fY > 0.0f);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 48.0f, 52.0f);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_DOWN,
		XGE_MOUSE_LEFT,
		pApp->tScroll.tFrame.tViewportRect.fX + 80.0f,
		pApp->tScroll.tFrame.tViewportRect.fY + 64.0f);
	iRet = xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_MOVE,
		0,
		pApp->tScroll.tFrame.tViewportRect.fX + 28.0f,
		pApp->tScroll.tFrame.tViewportRect.fY + 26.0f);
	(void)xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent);
	MakeMouseEvent(
		&tEvent,
		XGE_EVENT_MOUSE_UP,
		XGE_MOUSE_LEFT,
		pApp->tScroll.tFrame.tViewportRect.fX + 28.0f,
		pApp->tScroll.tFrame.tViewportRect.fY + 26.0f);
	(void)xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	pApp->bDragOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(fX > 80.0f) &&
		(fY > 80.0f) &&
		(pApp->tScroll.tFrame.bDraggingContent == 0);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	tThumb = VerticalThumbRect(&pApp->tScroll);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + tThumb.fH + 24.0f);
	iRet = xgeXuiScrollBarEvent(&pApp->tScroll.tFrame.tVScrollBar, &tEvent);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	pApp->bVerticalOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(fY > 80.0f);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	tThumb = HorizontalThumbRect(&pApp->tScroll);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + 1.0f, tThumb.fY + 1.0f);
	iRet = xgeXuiScrollBarEventProc(pApp->tScroll.tFrame.pHScrollWidget, &tEvent, &pApp->tScroll.tFrame.tHScrollBar);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tThumb.fX + 84.0f, tThumb.fY + 1.0f);
	(void)xgeXuiScrollBarEventProc(pApp->tScroll.tFrame.pHScrollWidget, &tEvent, &pApp->tScroll.tFrame.tHScrollBar);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tThumb.fX + 84.0f, tThumb.fY + 1.0f);
	(void)xgeXuiScrollBarEventProc(pApp->tScroll.tFrame.pHScrollWidget, &tEvent, &pApp->tScroll.tFrame.tHScrollBar);
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	pApp->bHorizontalOK =
		(iRet == XGE_XUI_EVENT_CONSUMED) &&
		(fX > 100.0f) &&
		FloatNear(fY, 0.0f, 0.01f);

	UpdateStatus(pApp);
	return XGE_OK;
}

static void DrawScrollContent(app_state_t* pApp)
{
	xge_rect_t tClip;
	xge_rect_t tCard;
	xge_rect_t tText;
	float fX;
	float fY;
	int i;
	uint32_t iStripe;

	if ( pApp->bFontReady == 0 ) {
		return;
	}
	xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
	tClip = pApp->tScroll.tFrame.tViewportRect;
	if ( tClip.fW > 6.0f ) {
		tClip.fW -= 6.0f;
	}
	if ( tClip.fH > 6.0f ) {
		tClip.fH -= 6.0f;
	}
	xgeClipSet(tClip);
	for ( i = 0; i < 12; i++ ) {
		tCard.fX = pApp->tScroll.tFrame.tViewportRect.fX + 18.0f + (float)((i % 3) * 210) - fX;
		tCard.fY = pApp->tScroll.tFrame.tViewportRect.fY + 18.0f + (float)(i * 42) - fY;
		tCard.fW = 176.0f;
		tCard.fH = 28.0f;
		iStripe = (i % 2 == 0) ? XGE_COLOR_RGBA(52, 70, 94, 255) : XGE_COLOR_RGBA(40, 56, 76, 255);
		xgeShapeRectFillPx(tCard, iStripe);
		xgeShapeRectStrokePx(tCard, 1.0f, XGE_COLOR_RGBA(98, 116, 146, 255));
		tText = tCard;
		tText.fX += 8.0f;
		tText.fW -= 16.0f;
		xgeTextDrawRect(&pApp->tFont, g_arrCards[i], tText, XGE_COLOR_RGBA(248, 250, 252, 255), XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	xgeClipClear();
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
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
	xgeXuiScrollViewUnit(&pApp->tScroll);
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
	float fX;
	float fY;

	pApp = (app_state_t*)pUser;
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	DrawScrollContent(pApp);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		xgeXuiScrollViewGetOffset(&pApp->tScroll, &fX, &fY);
		printf(
			"xui-scroll-view-proc-lab final-summary frames=%d init=%d config=%d wheel=%d drag=%d vbar=%d hbar=%d frame=%d scroll=%.2f,%.2f content=%.2fx%.2f drag=%d\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bConfigOK,
			pApp->bWheelOK,
			pApp->bDragOK,
			pApp->bVerticalOK,
			pApp->bHorizontalOK,
			pApp->bFrameOK,
			fX,
			fY,
			pApp->tScroll.tModel.fContentW,
			pApp->tScroll.tModel.fContentH,
			pApp->tScroll.tFrame.bDraggingContent);
		printf("xui-scroll-view-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 660;
	tDesc.iHeight = 440;
	tDesc.sTitle = "XGE XUI Scroll View Proc Lab";
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
		(tApp.bInitOK && tApp.bConfigOK && tApp.bWheelOK && tApp.bDragOK &&
		 tApp.bVerticalOK && tApp.bHorizontalOK && tApp.bFrameOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

