#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pPanelWidget;
	xge_xui_widget pSliderCaptionWidget;
	xge_xui_widget pSliderWidget;
	xge_xui_widget pProgressCaptionWidget;
	xge_xui_widget pProgressWidget;
	xge_xui_widget pSplitterCaptionWidget;
	xge_xui_widget pSplitterWidget;
	xge_xui_widget pScrollBarCaptionWidget;
	xge_xui_widget pScrollBarWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_label_t tSliderCaption;
	xge_xui_label_t tProgressCaption;
	xge_xui_label_t tSplitterCaption;
	xge_xui_label_t tScrollBarCaption;
	xge_xui_panel_t tPanel;
	xge_xui_slider_t tSlider;
	xge_xui_progress_t tProgress;
	xge_xui_splitter_t tSplitter;
	xge_xui_scrollbar_t tScrollBar;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	float fLastLayoutSplitterValue;
	int iSliderCallbackCount;
	int iSplitterCallbackCount;
	int iScrollBarCallbackCount;
	float fLastSliderValue;
	float fLastSplitterValue;
	float fLastScrollBarValue;
	float fScrollBeforeWheel;
	int bSplitterDragging;
	float fSplitterDragStartY;
	float fSplitterDragStartValue;
	int bPrevMouseLeftDown;
	int bDebugLogEnabled;
	int bSliderOK;
	int bProgressOK;
	int bSplitterOK;
	int bScrollBarOK;
	int bStateOK;
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

static float ClampFloat(float fValue, float fMin, float fMax)
{
	if ( fValue < fMin ) {
		return fMin;
	}
	if ( fValue > fMax ) {
		return fMax;
	}
	return fValue;
}

static int RectContains(xge_rect_t tRect, float fX, float fY)
{
	return (fX >= tRect.fX) && (fY >= tRect.fY) && (fX <= (tRect.fX + tRect.fW)) && (fY <= (tRect.fY + tRect.fH));
}

static void DebugLog(app_state_t* pApp, const char* sFormat, ...)
{
	va_list args;

	if ( (pApp == NULL) || (pApp->bDebugLogEnabled == 0) || (sFormat == NULL) ) {
		return;
	}
	va_start(args, sFormat);
	printf("[xui_value_controls_lab] ");
	vprintf(sFormat, args);
	printf("\n");
	va_end(args);
	fflush(stdout);
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

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
}

static xge_vec2_t SliderPoint(xge_xui_widget pWidget, float fRate)
{
	xge_vec2_t tPoint;

	fRate = ClampFloat(fRate, 0.0f, 1.0f);
	tPoint.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * fRate;
	tPoint.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * 0.5f;
	return tPoint;
}

static xge_rect_t ScrollBarThumbRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	xge_rect_t tThumb;
	float fTrackLen;
	float fRange;
	float fTotal;
	float fThumbLen;
	float fTravel;
	float fRate;

	tTrack = pScrollBar->pWidget->tContentRect;
	tThumb = tTrack;
	fTrackLen = (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? tTrack.fW : tTrack.fH;
	fRange = (pScrollBar->fMax > pScrollBar->fMin) ? (pScrollBar->fMax - pScrollBar->fMin) : 0.0f;
	if ( fRange <= 0.0f ) {
		return tThumb;
	}
	fTotal = fRange + ((pScrollBar->fPage > 0.0f) ? pScrollBar->fPage : 0.0f);
	fThumbLen = (fTotal > 0.0f) ? (fTrackLen * (pScrollBar->fPage / fTotal)) : 8.0f;
	if ( fThumbLen < 8.0f ) {
		fThumbLen = 8.0f;
	}
	if ( fThumbLen > fTrackLen ) {
		fThumbLen = fTrackLen;
	}
	fTravel = fTrackLen - fThumbLen;
	fRate = ClampFloat((pScrollBar->fValue - pScrollBar->fMin) / fRange, 0.0f, 1.0f);
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tThumb.fX = tTrack.fX + fTravel * fRate;
		tThumb.fW = fThumbLen;
	} else {
		tThumb.fY = tTrack.fY + fTravel * fRate;
		tThumb.fH = fThumbLen;
	}
	return tThumb;
}

static xge_vec2_t ScrollBarThumbCenter(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tThumb;
	xge_vec2_t tCenter;

	tThumb = ScrollBarThumbRect(pScrollBar);
	tCenter.fX = tThumb.fX + tThumb.fW * 0.5f;
	tCenter.fY = tThumb.fY + tThumb.fH * 0.5f;
	return tCenter;
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
			printf("xui-value-controls-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-value-controls-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void SliderChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSliderCallbackCount++;
		pApp->fLastSliderValue = fValue;
	}
}

static void SplitterChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSplitterCallbackCount++;
		pApp->fLastSplitterValue = fValue;
	}
}

static void ScrollBarChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iScrollBarCallbackCount++;
		pApp->fLastScrollBarValue = fValue;
	}
}

static void SplitterApplyLiveValue(app_state_t* pApp, float fValue, int bNotify)
{
	float fOldValue;
	float fNewValue;

	if ( pApp == NULL ) {
		return;
	}
	fOldValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
	xgeXuiSplitterSetValue(&pApp->tSplitter, fValue);
	fNewValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
	if ( bNotify && (FloatNear(fOldValue, fNewValue, 0.001f) == 0) ) {
		SplitterChange(pApp->pSplitterWidget, fNewValue, pApp);
	}
}

static void SplitterUpdateLiveRange(app_state_t* pApp, float fMinValue, float fMaxValue)
{
	float fValue;

	if ( pApp == NULL ) {
		return;
	}
	xgeXuiSplitterSetRange(&pApp->tSplitter, fMinValue, fMaxValue);
	fValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
	if ( fValue < fMinValue ) {
		xgeXuiSplitterSetValue(&pApp->tSplitter, fMinValue);
	} else if ( fValue > fMaxValue ) {
		xgeXuiSplitterSetValue(&pApp->tSplitter, fMaxValue);
	}
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fPanelW;
	float fSplitterValue;
	float fMinSplitterValue;
	float fMaxSplitterValue;
	float fSplitOffset;
	float fSplitterY;
	float fScrollCaptionY;
	float fScrollBarY;
	float fPanelContentBottom;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	fSplitterValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) && FloatNear(fSplitterValue, pApp->fLastLayoutSplitterValue, 0.001f) ) {
		return;
	}
	tRoot.fX = 20.0f;
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 560.0f ) {
		tRoot.fW = 560.0f;
	}
	if ( tRoot.fH < 340.0f ) {
		tRoot.fH = 340.0f;
	}
	fPanelW = tRoot.fW - 44.0f;
	fMinSplitterValue = -20.0f;
	fMaxSplitterValue = 6.0f + ((tRoot.fH - 340.0f) + 34.0f);
	if ( fMaxSplitterValue < 40.0f ) {
		fMaxSplitterValue = 40.0f;
	}
	SplitterUpdateLiveRange(pApp, fMinSplitterValue, fMaxSplitterValue);
	fSplitterValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
	fSplitOffset = fSplitterValue - 6.0f;
	if ( fSplitOffset < -32.0f ) {
		fSplitOffset = -32.0f;
	}
	if ( fSplitOffset > 72.0f ) {
		fSplitOffset = 72.0f;
	}
	fSplitterY = 202.0f + fSplitOffset;
	fScrollCaptionY = 242.0f + fSplitOffset;
	fScrollBarY = 266.0f + fSplitOffset;
	fPanelContentBottom = tRoot.fH - 18.0f;
	if ( fScrollBarY + 24.0f > fPanelContentBottom ) {
		fScrollBarY = fPanelContentBottom - 24.0f;
		fScrollCaptionY = fScrollBarY - 24.0f;
		fSplitterY = fScrollCaptionY - 40.0f;
	}

	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 22.0f, 58.0f, fPanelW, tRoot.fH - 80.0f });
	xgeXuiWidgetSetRect(pApp->pSliderCaptionWidget, (xge_rect_t){ 22.0f, 36.0f, fPanelW - 44.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pSliderWidget, (xge_rect_t){ 22.0f, 58.0f, fPanelW - 44.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pProgressCaptionWidget, (xge_rect_t){ 22.0f, 104.0f, fPanelW - 44.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pProgressWidget, (xge_rect_t){ 22.0f, 126.0f, fPanelW - 44.0f, 34.0f });
	xgeXuiWidgetSetRect(pApp->pSplitterCaptionWidget, (xge_rect_t){ 22.0f, 178.0f, fPanelW - 44.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pSplitterWidget, (xge_rect_t){ 22.0f, fSplitterY, fPanelW - 44.0f, 22.0f });
	xgeXuiWidgetSetRect(pApp->pScrollBarCaptionWidget, (xge_rect_t){ 22.0f, fScrollCaptionY, fPanelW - 44.0f, 20.0f });
	xgeXuiWidgetSetRect(pApp->pScrollBarWidget, (xge_rect_t){ 22.0f, fScrollBarY, fPanelW - 44.0f, 24.0f });

	DebugLog(
		pApp,
		"layout size=%dx%d slider_rect=(%.1f,%.1f,%.1f,%.1f) slider_content=(%.1f,%.1f,%.1f,%.1f) splitter_rect=(%.1f,%.1f,%.1f,%.1f) scroll_rect=(%.1f,%.1f,%.1f,%.1f) split_value=%.2f",
		iWidth,
		iHeight,
		pApp->pSliderWidget->tRect.fX,
		pApp->pSliderWidget->tRect.fY,
		pApp->pSliderWidget->tRect.fW,
		pApp->pSliderWidget->tRect.fH,
		pApp->pSliderWidget->tContentRect.fX,
		pApp->pSliderWidget->tContentRect.fY,
		pApp->pSliderWidget->tContentRect.fW,
		pApp->pSliderWidget->tContentRect.fH,
		pApp->pSplitterWidget->tRect.fX,
		pApp->pSplitterWidget->tRect.fY,
		pApp->pSplitterWidget->tRect.fW,
		pApp->pSplitterWidget->tRect.fH,
		pApp->pScrollBarWidget->tRect.fX,
		pApp->pScrollBarWidget->tRect.fY,
		pApp->pScrollBarWidget->tRect.fW,
		pApp->pScrollBarWidget->tRect.fH,
		fSplitterValue);
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
	pApp->fLastLayoutSplitterValue = fSplitterValue;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"slider=%d progress=%d splitter=%d scrollbar=%d state=%d value=%.1f/%.1f/%.1f cb=%d/%d/%d",
		pApp->bSliderOK,
		pApp->bProgressOK,
		pApp->bSplitterOK,
		pApp->bScrollBarOK,
		pApp->bStateOK,
		xgeXuiSliderGetValue(&pApp->tSlider),
		xgeXuiSplitterGetValue(&pApp->tSplitter),
		xgeXuiScrollBarGetValue(&pApp->tScrollBar),
		pApp->iSliderCallbackCount,
		pApp->iSplitterCallbackCount,
		pApp->iScrollBarCallbackCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_theme_t tTheme;
	xge_xui_widget pRoot;
	xge_font pFont;
	uint32_t iCaptionColor;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}

	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 98, 122, 255);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(96, 164, 255, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(56, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(78, 146, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(42, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(86, 108, 144, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->pSliderCaptionWidget = xgeXuiWidgetCreate();
	pApp->pSliderWidget = xgeXuiWidgetCreate();
	pApp->pProgressCaptionWidget = xgeXuiWidgetCreate();
	pApp->pProgressWidget = xgeXuiWidgetCreate();
	pApp->pSplitterCaptionWidget = xgeXuiWidgetCreate();
	pApp->pSplitterWidget = xgeXuiWidgetCreate();
	pApp->pScrollBarCaptionWidget = xgeXuiWidgetCreate();
	pApp->pScrollBarWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPanelWidget == NULL) || (pApp->pSliderCaptionWidget == NULL) || (pApp->pSliderWidget == NULL) || (pApp->pProgressCaptionWidget == NULL) || (pApp->pProgressWidget == NULL) || (pApp->pSplitterCaptionWidget == NULL) || (pApp->pSplitterWidget == NULL) || (pApp->pScrollBarCaptionWidget == NULL) || (pApp->pScrollBarWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetLayout(pApp->pRootPanel, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(22, 28, 38, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	if ( xgeXuiWidgetAdd(pRoot, pApp->pRootPanel) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(44, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui value controls lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Value Controls");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(32, 40, 54, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	iCaptionColor = XGE_COLOR_RGBA(198, 208, 224, 255);
	xgeXuiLabelInit(&pApp->tSliderCaption, pApp->pSliderCaptionWidget, pFont, "Slider");
	xgeXuiLabelSetColor(&pApp->tSliderCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSliderCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiSliderInit(&pApp->tSlider, &pApp->tXui, pApp->pSliderWidget);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSliderWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tProgressCaption, pApp->pProgressCaptionWidget, pFont, "Progress");
	xgeXuiLabelSetColor(&pApp->tProgressCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pProgressCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiProgressInit(&pApp->tProgress, pApp->pProgressWidget);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pProgressWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tSplitterCaption, pApp->pSplitterCaptionWidget, pFont, "Splitter");
	xgeXuiLabelSetColor(&pApp->tSplitterCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSplitterCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiSplitterInit(&pApp->tSplitter, &pApp->tXui, pApp->pSplitterWidget);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSplitterWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiLabelInit(&pApp->tScrollBarCaption, pApp->pScrollBarCaptionWidget, pFont, "ScrollBar");
	xgeXuiLabelSetColor(&pApp->tScrollBarCaption, iCaptionColor);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pScrollBarCaptionWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiScrollBarInit(&pApp->tScrollBar, &pApp->tXui, pApp->pScrollBarWidget);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pScrollBarWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tPointA;
	xge_vec2_t tPointB;
	xge_vec2_t tCenter;
	xge_rect_t tThumb;
	int bSliderDefaultOK;
	int bProgressDefaultOK;
	int bSplitterDefaultOK;
	int bScrollDefaultOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	bSliderDefaultOK =
		FloatNear(pApp->tSlider.fMin, 0.0f, 0.001f) &&
		FloatNear(pApp->tSlider.fMax, 1.0f, 0.001f) &&
		FloatNear(xgeXuiSliderGetValue(&pApp->tSlider), 0.0f, 0.001f) &&
		(pApp->tSlider.iColorTrack == XGE_COLOR_RGBA(162, 174, 190, 255)) &&
		(pApp->tSlider.iColorFill == XGE_COLOR_RGBA(46, 124, 214, 255)) &&
		(pApp->tSlider.iColorKnob == XGE_COLOR_RGBA(236, 240, 246, 255));
	xgeXuiSliderSetChange(&pApp->tSlider, SliderChange, pApp);
	xgeXuiSliderSetRange(&pApp->tSlider, 10.0f, 2.0f);
	xgeXuiSliderSetValue(&pApp->tSlider, 6.5f);
	xgeXuiSliderSetColors(
		&pApp->tSlider,
		XGE_COLOR_RGBA(72, 82, 102, 255),
		XGE_COLOR_RGBA(112, 220, 168, 255),
		XGE_COLOR_RGBA(240, 244, 250, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180));
	xgeXuiSetFocus(&pApp->tXui, pApp->pSliderWidget);
	tPointA = SliderPoint(pApp->pSliderWidget, 0.0f);
	tPointB = SliderPoint(pApp->pSliderWidget, 0.8f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tPointA.fX, tPointA.fY);
	xgeXuiSliderEvent(&pApp->tSlider, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tPointB.fX, tPointB.fY);
	xgeXuiSliderEvent(&pApp->tSlider, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tPointB.fX, tPointB.fY);
	xgeXuiSliderEvent(&pApp->tSlider, &tEvent);
	pApp->bSliderOK =
		bSliderDefaultOK &&
		FloatNear(pApp->tSlider.fMin, 2.0f, 0.001f) &&
		FloatNear(pApp->tSlider.fMax, 10.0f, 0.001f) &&
		FloatNear(xgeXuiSliderGetValue(&pApp->tSlider), 8.4f, 0.3f) &&
		(pApp->tSlider.iColorTrack == XGE_COLOR_RGBA(72, 82, 102, 255)) &&
		(pApp->tSlider.iColorFill == XGE_COLOR_RGBA(112, 220, 168, 255)) &&
		(pApp->tSlider.iColorKnob == XGE_COLOR_RGBA(240, 244, 250, 255)) &&
		(pApp->tSlider.iChangeCount >= 4) &&
		(pApp->iSliderCallbackCount >= 2) &&
		FloatNear(pApp->fLastSliderValue, xgeXuiSliderGetValue(&pApp->tSlider), 0.3f) &&
		((xgeXuiSliderGetState(&pApp->tSlider) & XGE_XUI_STATE_HOVER) != 0) &&
		((xgeXuiSliderGetState(&pApp->tSlider) & XGE_XUI_STATE_FOCUS) != 0);

	bProgressDefaultOK =
		FloatNear(pApp->tProgress.fMin, 0.0f, 0.001f) &&
		FloatNear(pApp->tProgress.fMax, 1.0f, 0.001f) &&
		FloatNear(xgeXuiProgressGetValue(&pApp->tProgress), 0.0f, 0.001f) &&
		(pApp->tProgress.iColorTrack == XGE_COLOR_RGBA(92, 100, 112, 255)) &&
		(pApp->tProgress.iColorFill == XGE_COLOR_RGBA(62, 172, 110, 255)) &&
		(pApp->tProgress.iTextColor == XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiProgressSetRange(&pApp->tProgress, 3.0f, 3.0f);
	xgeXuiProgressSetValue(&pApp->tProgress, 99.0f);
	xgeXuiProgressSetRange(&pApp->tProgress, 12.0f, 4.0f);
	xgeXuiProgressSetValue(&pApp->tProgress, 9.0f);
	xgeXuiProgressSetText(&pApp->tProgress, pApp->bFontReady ? &pApp->tFont : NULL, "Progress 9/12");
	xgeXuiProgressSetTextColor(&pApp->tProgress, XGE_COLOR_RGBA(18, 20, 26, 255));
	xgeXuiProgressSetColors(&pApp->tProgress, XGE_COLOR_RGBA(80, 92, 110, 255), XGE_COLOR_RGBA(244, 182, 92, 255));
	pApp->bProgressOK =
		bProgressDefaultOK &&
		FloatNear(pApp->tProgress.fMin, 4.0f, 0.001f) &&
		FloatNear(pApp->tProgress.fMax, 12.0f, 0.001f) &&
		FloatNear(xgeXuiProgressGetValue(&pApp->tProgress), 9.0f, 0.001f) &&
		(pApp->tProgress.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(strcmp(pApp->tProgress.sText, "Progress 9/12") == 0) &&
		(pApp->tProgress.iTextColor == XGE_COLOR_RGBA(18, 20, 26, 255)) &&
		(pApp->tProgress.iColorTrack == XGE_COLOR_RGBA(80, 92, 110, 255)) &&
		(pApp->tProgress.iColorFill == XGE_COLOR_RGBA(244, 182, 92, 255));

	bSplitterDefaultOK =
		FloatNear(pApp->tSplitter.fMin, -20.0f, 0.001f) &&
		(pApp->tSplitter.fMax >= 40.0f) &&
		FloatNear(xgeXuiSplitterGetValue(&pApp->tSplitter), 0.0f, 0.001f) &&
		(pApp->tSplitter.iOrientation == XGE_XUI_SEPARATOR_VERTICAL) &&
		(pApp->tSplitter.iColorNormal == XGE_COLOR_RGBA(162, 174, 190, 255));
	xgeXuiSplitterSetChange(&pApp->tSplitter, SplitterChange, pApp);
	xgeXuiSplitterSetRange(&pApp->tSplitter, 40.0f, -20.0f);
	xgeXuiSplitterSetValue(&pApp->tSplitter, 6.0f);
	xgeXuiSplitterSetOrientation(&pApp->tSplitter, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiSplitterSetColors(
		&pApp->tSplitter,
		XGE_COLOR_RGBA(86, 100, 120, 255),
		XGE_COLOR_RGBA(120, 184, 255, 255),
		XGE_COLOR_RGBA(244, 182, 92, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180));
	xgeXuiSetFocus(&pApp->tXui, pApp->pSplitterWidget);
	tCenter = WidgetCenter(pApp->pSplitterWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiSplitterEvent(&pApp->tSplitter, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX, tCenter.fY + 8.0f);
	xgeXuiSplitterEvent(&pApp->tSplitter, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY + 8.0f);
	xgeXuiSplitterEvent(&pApp->tSplitter, &tEvent);
	pApp->bSplitterOK =
		bSplitterDefaultOK &&
		FloatNear(pApp->tSplitter.fMin, -20.0f, 0.001f) &&
		FloatNear(pApp->tSplitter.fMax, 40.0f, 0.001f) &&
		FloatNear(xgeXuiSplitterGetValue(&pApp->tSplitter), 14.0f, 0.3f) &&
		(pApp->tSplitter.iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) &&
		(pApp->tSplitter.iColorNormal == XGE_COLOR_RGBA(86, 100, 120, 255)) &&
		(pApp->tSplitter.iColorHover == XGE_COLOR_RGBA(120, 184, 255, 255)) &&
		(pApp->tSplitter.iColorActive == XGE_COLOR_RGBA(244, 182, 92, 255)) &&
		(pApp->tSplitter.iChangeCount >= 2) &&
		(pApp->iSplitterCallbackCount >= 1) &&
		FloatNear(pApp->fLastSplitterValue, xgeXuiSplitterGetValue(&pApp->tSplitter), 0.3f) &&
		((xgeXuiSplitterGetState(&pApp->tSplitter) & XGE_XUI_STATE_HOVER) != 0) &&
		((xgeXuiSplitterGetState(&pApp->tSplitter) & XGE_XUI_STATE_FOCUS) != 0);
	bScrollDefaultOK =
		FloatNear(pApp->tScrollBar.fMin, 0.0f, 0.001f) &&
		FloatNear(pApp->tScrollBar.fMax, 1.0f, 0.001f) &&
		FloatNear(pApp->tScrollBar.fPage, 0.2f, 0.001f) &&
		FloatNear(xgeXuiScrollBarGetValue(&pApp->tScrollBar), 0.0f, 0.001f) &&
		(pApp->tScrollBar.iOrientation == XGE_XUI_SEPARATOR_VERTICAL) &&
		(pApp->tScrollBar.iColorTrack == XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiScrollBarSetChange(&pApp->tScrollBar, ScrollBarChange, pApp);
	xgeXuiScrollBarSetRange(&pApp->tScrollBar, 100.0f, 20.0f, 18.0f);
	xgeXuiScrollBarSetPage(&pApp->tScrollBar, -4.0f);
	xgeXuiScrollBarSetPage(&pApp->tScrollBar, 12.0f);
	xgeXuiScrollBarSetValue(&pApp->tScrollBar, 40.0f);
	xgeXuiScrollBarSetOrientation(&pApp->tScrollBar, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiScrollBarSetColors(
		&pApp->tScrollBar,
		XGE_COLOR_RGBA(72, 82, 102, 255),
		XGE_COLOR_RGBA(96, 164, 255, 255),
		XGE_COLOR_RGBA(120, 184, 255, 255),
		XGE_COLOR_RGBA(244, 182, 92, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180));
	tThumb = ScrollBarThumbRect(&pApp->tScrollBar);
	tCenter = WidgetCenter(pApp->pScrollBarWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tThumb.fX + tThumb.fW + 10.0f, tCenter.fY);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tThumb.fX + tThumb.fW + 10.0f, tCenter.fY);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	tCenter = ScrollBarThumbCenter(&pApp->tScrollBar);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, tCenter.fX + 80.0f, tCenter.fY);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX + 80.0f, tCenter.fY);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	pApp->fScrollBeforeWheel = xgeXuiScrollBarGetValue(&pApp->tScrollBar);
	tCenter = WidgetCenter(pApp->pScrollBarWidget);
	MakeWheelEvent(&tEvent, tCenter.fX, tCenter.fY, -1.0f);
	xgeXuiScrollBarEvent(&pApp->tScrollBar, &tEvent);
	pApp->bScrollBarOK =
		bScrollDefaultOK &&
		FloatNear(pApp->tScrollBar.fMin, 20.0f, 0.001f) &&
		FloatNear(pApp->tScrollBar.fMax, 100.0f, 0.001f) &&
		FloatNear(pApp->tScrollBar.fPage, 12.0f, 0.001f) &&
		(pApp->tScrollBar.iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) &&
		(pApp->tScrollBar.iColorTrack == XGE_COLOR_RGBA(72, 82, 102, 255)) &&
		(pApp->tScrollBar.iColorThumb == XGE_COLOR_RGBA(96, 164, 255, 255)) &&
		(pApp->tScrollBar.iColorHover == XGE_COLOR_RGBA(120, 184, 255, 255)) &&
		(pApp->tScrollBar.iColorActive == XGE_COLOR_RGBA(244, 182, 92, 255)) &&
		(pApp->tScrollBar.iChangeCount >= 5) &&
		(pApp->iScrollBarCallbackCount >= 3) &&
		(xgeXuiScrollBarGetValue(&pApp->tScrollBar) > pApp->fScrollBeforeWheel) &&
		FloatNear(pApp->fLastScrollBarValue, xgeXuiScrollBarGetValue(&pApp->tScrollBar), 0.3f) &&
		(pApp->tScrollBar.bDraggingThumb == 0) &&
		((xgeXuiScrollBarGetState(&pApp->tScrollBar) & XGE_XUI_STATE_HOVER) != 0) &&
		((xgeXuiScrollBarGetState(&pApp->tScrollBar) & XGE_XUI_STATE_FOCUS) != 0);

	xgeXuiWidgetSetEnabled(pApp->pSliderWidget, 0);
	xgeXuiWidgetSetEnabled(pApp->pSplitterWidget, 0);
	xgeXuiWidgetSetEnabled(pApp->pScrollBarWidget, 0);
	pApp->bStateOK =
		((xgeXuiSliderGetState(&pApp->tSlider) & XGE_XUI_STATE_DISABLED) != 0) &&
		((xgeXuiSplitterGetState(&pApp->tSplitter) & XGE_XUI_STATE_DISABLED) != 0) &&
		((xgeXuiScrollBarGetState(&pApp->tScrollBar) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pSliderWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pSplitterWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pScrollBarWidget, 1);

	UpdateStatus(pApp);
	return XGE_OK;
}

static void ResetLiveDemoState(app_state_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiSliderSetValue(&pApp->tSlider, 6.5f);
	xgeXuiProgressSetValue(&pApp->tProgress, 9.0f);
	SplitterApplyLiveValue(pApp, 6.0f, 0);
	xgeXuiScrollBarSetValue(&pApp->tScrollBar, 40.0f);
	pApp->bSplitterDragging = 0;
	xgeXuiSetCapture(&pApp->tXui, NULL);
	xgeXuiSetFocus(&pApp->tXui, NULL);
	(void)xgeXuiSliderGetState(&pApp->tSlider);
	(void)xgeXuiSplitterGetState(&pApp->tSplitter);
	(void)xgeXuiScrollBarGetState(&pApp->tScrollBar);
	UpdateStatus(pApp);
	DebugLog(
		pApp,
		"reset live-state slider=%.2f splitter=%.2f scrollbar=%.2f",
		xgeXuiSliderGetValue(&pApp->tSlider),
		xgeXuiSplitterGetValue(&pApp->tSplitter),
		xgeXuiScrollBarGetValue(&pApp->tScrollBar));
}

static int AppSetup(app_state_t* pApp)
{
	const char* sDebug;

	sDebug = getenv("XGE_XUI_VALUE_CONTROLS_DEBUG");
	pApp->bDebugLogEnabled = (sDebug != NULL && sDebug[0] != 0 && strcmp(sDebug, "0") != 0) ? 1 : 0;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	if ( RunStaticChecks(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	ResetLiveDemoState(pApp);
	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiScrollBarUnit(&pApp->tScrollBar);
	xgeXuiSplitterUnit(&pApp->tSplitter);
	xgeXuiProgressUnit(&pApp->tProgress);
	xgeXuiSliderUnit(&pApp->tSlider);
	xgeXuiPanelUnit(&pApp->tPanel);
	xgeXuiLabelUnit(&pApp->tScrollBarCaption);
	xgeXuiLabelUnit(&pApp->tSplitterCaption);
	xgeXuiLabelUnit(&pApp->tProgressCaption);
	xgeXuiLabelUnit(&pApp->tSliderCaption);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
}

static void SyncLayout(app_state_t* pApp, float fDelta)
{
	LayoutRoot(pApp);
	UpdateStatus(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	return AppSetup(pApp);
}

static int AppLeave(xge_scene pScene)
{
	AppUnit((app_state_t*)pScene->pUser);
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent == NULL ) {
		return XGE_OK;
	}
	SyncLayout(pApp, 0.0f);
	if ( pEvent->iType == XGE_EVENT_RESIZE ) {
		DebugLog(pApp, "event resize window=%d x %d root_rect=(%.1f,%.1f,%.1f,%.1f)", pEvent->iParam1, pEvent->iParam2, pApp->tXui.pRoot->tRect.fX, pApp->tXui.pRoot->tRect.fY, pApp->tXui.pRoot->tRect.fW, pApp->tXui.pRoot->tRect.fH);
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN) || (pEvent->iType == XGE_EVENT_MOUSE_UP)) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		DebugLog(
			pApp,
			"event %s x=%.1f y=%.1f inside_slider=%d inside_splitter=%d inside_scroll=%d splitter_rect=(%.1f,%.1f,%.1f,%.1f)",
			(pEvent->iType == XGE_EVENT_MOUSE_DOWN) ? "mouse-down" : "mouse-up",
			pEvent->fX,
			pEvent->fY,
			RectContains(pApp->pSliderWidget->tRect, pEvent->fX, pEvent->fY),
			RectContains(pApp->pSplitterWidget->tRect, pEvent->fX, pEvent->fY),
			RectContains(pApp->pScrollBarWidget->tRect, pEvent->fX, pEvent->fY),
			pApp->pSplitterWidget->tRect.fX,
			pApp->pSplitterWidget->tRect.fY,
			pApp->pSplitterWidget->tRect.fW,
			pApp->pSplitterWidget->tRect.fH);
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) && RectContains(pApp->pSplitterWidget->tRect, pEvent->fX, pEvent->fY) ) {
		pApp->bSplitterDragging = 1;
		pApp->fSplitterDragStartY = pEvent->fY;
		pApp->fSplitterDragStartValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
		xgeXuiSetFocus(&pApp->tXui, pApp->pSplitterWidget);
		xgeXuiSetCapture(&pApp->tXui, pApp->pSplitterWidget);
		DebugLog(pApp, "splitter drag-start y=%.1f value=%.2f", pEvent->fY, pApp->fSplitterDragStartValue);
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_MOVE) && (pApp->bSplitterDragging != 0) ) {
		SplitterApplyLiveValue(pApp, pApp->fSplitterDragStartValue + (pEvent->fY - pApp->fSplitterDragStartY), 1);
		DebugLog(pApp, "splitter drag-move y=%.1f value=%.2f", pEvent->fY, xgeXuiSplitterGetValue(&pApp->tSplitter));
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) && (pEvent->iParam1 == XGE_MOUSE_LEFT) && (pApp->bSplitterDragging != 0) ) {
		SplitterApplyLiveValue(pApp, pApp->fSplitterDragStartValue + (pEvent->fY - pApp->fSplitterDragStartY), 1);
		pApp->bSplitterDragging = 0;
		xgeXuiSetCapture(&pApp->tXui, NULL);
		DebugLog(pApp, "splitter drag-end y=%.1f value=%.2f", pEvent->fY, xgeXuiSplitterGetValue(&pApp->tSplitter));
		return XGE_OK;
	}
	if ( xgeXuiDispatchEvent(&pApp->tXui, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;
	float fMouseX;
	float fMouseY;
	int bMouseLeftDown;

	pApp = (app_state_t*)pScene->pUser;
	SyncLayout(pApp, fDelta);
	xgeMouseGet(&fMouseX, &fMouseY);
	bMouseLeftDown = xgeMouseDown(XGE_MOUSE_LEFT);
	if ( (bMouseLeftDown != 0) && (pApp->bPrevMouseLeftDown == 0) && RectContains(pApp->pSplitterWidget->tRect, fMouseX, fMouseY) ) {
		pApp->bSplitterDragging = 1;
		pApp->fSplitterDragStartY = fMouseY;
		pApp->fSplitterDragStartValue = xgeXuiSplitterGetValue(&pApp->tSplitter);
		xgeXuiSetFocus(&pApp->tXui, pApp->pSplitterWidget);
		xgeXuiSetCapture(&pApp->tXui, pApp->pSplitterWidget);
		DebugLog(pApp, "splitter poll-start mouse=(%.1f,%.1f) value=%.2f", fMouseX, fMouseY, pApp->fSplitterDragStartValue);
	}
	if ( (pApp->bSplitterDragging != 0) && (bMouseLeftDown != 0) ) {
		SplitterApplyLiveValue(pApp, pApp->fSplitterDragStartValue + (fMouseY - pApp->fSplitterDragStartY), 1);
		DebugLog(pApp, "splitter poll-update mouse=(%.1f,%.1f) value=%.2f", fMouseX, fMouseY, xgeXuiSplitterGetValue(&pApp->tSplitter));
	}
	if ( (pApp->bSplitterDragging != 0) && (bMouseLeftDown == 0) ) {
		pApp->bSplitterDragging = 0;
		xgeXuiSetCapture(&pApp->tXui, NULL);
		DebugLog(pApp, "splitter poll-end mouse=(%.1f,%.1f) value=%.2f", fMouseX, fMouseY, xgeXuiSplitterGetValue(&pApp->tSplitter));
	}
	pApp->bPrevMouseLeftDown = bMouseLeftDown;

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-value-controls-lab final-summary frames=%d slider=%d progress=%d splitter=%d scrollbar=%d state=%d slider=%.2f split=%.2f scroll=%.2f callbacks=%d/%d/%d changes=%d/%d/%d progress=%.2f range=%.2f..%.2f page=%.2f orient=%d/%d\n",
			pApp->iFrameCount,
			pApp->bSliderOK,
			pApp->bProgressOK,
			pApp->bSplitterOK,
			pApp->bScrollBarOK,
			pApp->bStateOK,
			xgeXuiSliderGetValue(&pApp->tSlider),
			xgeXuiSplitterGetValue(&pApp->tSplitter),
			xgeXuiScrollBarGetValue(&pApp->tScrollBar),
			pApp->iSliderCallbackCount,
			pApp->iSplitterCallbackCount,
			pApp->iScrollBarCallbackCount,
			pApp->tSlider.iChangeCount,
			pApp->tSplitter.iChangeCount,
			pApp->tScrollBar.iChangeCount,
			xgeXuiProgressGetValue(&pApp->tProgress),
			pApp->tProgress.fMin,
			pApp->tProgress.fMax,
			pApp->tScrollBar.fPage,
			pApp->tSplitter.iOrientation,
			pApp->tScrollBar.iOrientation);
		printf("xui-value-controls-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void AppStateInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->fLastSliderValue = -1.0f;
	pApp->fLastSplitterValue = -1.0f;
	pApp->fLastScrollBarValue = -1.0f;
	pApp->fLastLayoutSplitterValue = 1000000.0f;
	pApp->tScene.pUser = pApp;
	pApp->tScene.onEnter = AppEnter;
	pApp->tScene.onLeave = AppLeave;
	pApp->tScene.onEvent = AppEvent;
	pApp->tScene.onUpdate = AppUpdate;
	pApp->tScene.onDraw = AppDraw;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int iFrameLimit;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	iFrameLimit = ArgInt(getenv("XGE_XUI_VALUE_CONTROLS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 620;
	tDesc.iHeight = 400;
	tDesc.sTitle = "XGE XUI Value Controls Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	AppStateInit(&tApp, iFrameLimit);
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		printf("xui-value-controls-lab scene set failed\n");
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bSliderOK && tApp.bProgressOK && tApp.bSplitterOK && tApp.bScrollBarOK && tApp.bStateOK) ? 0 : 3;
}

