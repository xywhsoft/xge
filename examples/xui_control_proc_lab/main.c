#include "../../xge.h"
#include "../xui_demo_style.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pRootPanel;
	xge_xui_widget pStatusWidget;
	xge_xui_widget pToggleWidget;
	xge_xui_widget pCheckBoxWidget;
	xge_xui_widget pRadioAWidget;
	xge_xui_widget pRadioBWidget;
	xge_xui_widget pSwitchWidget;
	xge_xui_widget pSliderWidget;
	xge_xui_widget pScrollBarWidget;
	xge_xui_widget pSplitterWidget;
	xge_xui_widget pProgressWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_toggle_t tToggle;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_switch_t tSwitch;
	xge_xui_slider_t tSlider;
	xge_xui_scrollbar_t tScrollBar;
	xge_xui_splitter_t tSplitter;
	xge_xui_progress_t tProgress;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iToggleChangeCount;
	int iCheckBoxChangeCount;
	int iRadioChangeCount;
	int iSwitchChangeCount;
	int iSliderChangeCount;
	int iScrollBarChangeCount;
	int iSplitterChangeCount;
	int bToggleOK;
	int bCheckBoxOK;
	int bRadioOK;
	int bSwitchOK;
	int bSliderOK;
	int bScrollBarOK;
	int bSplitterOK;
	int bProgressOK;
	int bDirectPaintOK;
} app_state_t;

static const char* g_sToggleText = "Toggle Event/Paint Proc";
static const char* g_sCheckBoxText = "CheckBox Event/Paint Proc";
static const char* g_sRadioAText = "Radio A";
static const char* g_sRadioBText = "Radio B";
static const char* g_sSwitchText = "Switch Event/Paint Proc";

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

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
}

static float LeftControlWidth(app_state_t* pApp)
{
	xge_vec2_t tSize;
	float fWidth;
	const char* arrTexts[] = {
		g_sToggleText,
		g_sCheckBoxText,
		g_sRadioAText,
		g_sRadioBText,
		g_sSwitchText
	};
	int i;

	fWidth = 240.0f;
	if ( (pApp == NULL) || (pApp->bFontReady == 0) ) {
		return fWidth;
	}
	for ( i = 0; i < (int)(sizeof(arrTexts) / sizeof(arrTexts[0])); i++ ) {
		tSize = xgeTextMeasure(&pApp->tFont, arrTexts[i]);
		if ( tSize.fX + 56.0f > fWidth ) {
			fWidth = tSize.fX + 56.0f;
		}
	}
	return fWidth;
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
			printf("xui-control-proc-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-control-proc-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)bChecked;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iToggleChangeCount++;
	}
}

static void CheckBoxChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)bChecked;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCheckBoxChangeCount++;
	}
}

static void RadioChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)bChecked;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iRadioChangeCount++;
	}
}

static void SwitchChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)bChecked;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSwitchChangeCount++;
	}
}

static void SliderChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSliderChangeCount++;
	}
}

static void ScrollBarChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iScrollBarChangeCount++;
	}
}

static void SplitterChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)fValue;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSplitterChangeCount++;
	}
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_rect_t tRoot;
	float fLeftWidth;
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
	if ( tRoot.fW < 700.0f ) {
		tRoot.fW = 700.0f;
	}
	if ( tRoot.fH < 360.0f ) {
		tRoot.fH = 360.0f;
	}
	fLeftWidth = LeftControlWidth(pApp);
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pToggleWidget, (xge_rect_t){ 24.0f, 56.0f, fLeftWidth, 30.0f });
	xgeXuiWidgetSetRect(pApp->pCheckBoxWidget, (xge_rect_t){ 24.0f, 98.0f, fLeftWidth, 30.0f });
	xgeXuiWidgetSetRect(pApp->pRadioAWidget, (xge_rect_t){ 24.0f, 140.0f, fLeftWidth, 30.0f });
	xgeXuiWidgetSetRect(pApp->pRadioBWidget, (xge_rect_t){ 24.0f, 176.0f, fLeftWidth, 30.0f });
	xgeXuiWidgetSetRect(pApp->pSwitchWidget, (xge_rect_t){ 24.0f, 220.0f, fLeftWidth, 32.0f });
	xgeXuiWidgetSetRect(pApp->pSliderWidget, (xge_rect_t){ 324.0f, 74.0f, 240.0f, 32.0f });
	xgeXuiWidgetSetRect(pApp->pScrollBarWidget, (xge_rect_t){ 604.0f, 62.0f, 24.0f, 180.0f });
	xgeXuiWidgetSetRect(pApp->pSplitterWidget, (xge_rect_t){ 342.0f, 144.0f, 180.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pProgressWidget, (xge_rect_t){ 324.0f, 210.0f, 240.0f, 32.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[320];

	snprintf(
		sText,
		sizeof(sText),
		"toggle=%d checkbox=%d radio=%d switch=%d slider=%d scroll=%d split=%d progress=%d paint=%d changes=%d/%d/%d/%d/%d/%d/%d",
		pApp->bToggleOK,
		pApp->bCheckBoxOK,
		pApp->bRadioOK,
		pApp->bSwitchOK,
		pApp->bSliderOK,
		pApp->bScrollBarOK,
		pApp->bSplitterOK,
		pApp->bProgressOK,
		pApp->bDirectPaintOK,
		pApp->iToggleChangeCount,
		pApp->iCheckBoxChangeCount,
		pApp->iRadioChangeCount,
		pApp->iSwitchChangeCount,
		pApp->iSliderChangeCount,
		pApp->iScrollBarChangeCount,
		pApp->iSplitterChangeCount);
	xgeXuiLabelSetText(&pApp->tStatusLabel, sText);
}

static void ResetWidgetVisualState(xge_xui_widget pWidget)
{
	xge_event_t tEvent;

	if ( (pWidget == NULL) || (pWidget->procEvent == NULL) ) {
		return;
	}
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_POINTER_LEAVE;
	(void)pWidget->procEvent(pWidget, &tEvent, pWidget->pUser);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_XUI_CAPTURE_LOST;
	(void)pWidget->procEvent(pWidget, &tEvent, pWidget->pUser);
}

static void ResetVisualStates(app_state_t* pApp)
{
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiSetCapture(&pApp->tXui, NULL);
	xgeXuiSetFocus(&pApp->tXui, NULL);
	ResetWidgetVisualState(pApp->pToggleWidget);
	ResetWidgetVisualState(pApp->pCheckBoxWidget);
	ResetWidgetVisualState(pApp->pRadioAWidget);
	ResetWidgetVisualState(pApp->pRadioBWidget);
	ResetWidgetVisualState(pApp->pSwitchWidget);
	ResetWidgetVisualState(pApp->pSliderWidget);
	ResetWidgetVisualState(pApp->pScrollBarWidget);
	ResetWidgetVisualState(pApp->pSplitterWidget);
	xgeXuiWidgetMarkPaint(pApp->pRootPanel);
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
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(88, 98, 120, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(102, 216, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 104, 176, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(74, 128, 204, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(46, 82, 142, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(82, 98, 128, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 88, 94, 180);
	tTheme.iSelectionColor = XGE_COLOR_RGBA(112, 198, 255, 255);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pToggleWidget = xgeXuiWidgetCreate();
	pApp->pCheckBoxWidget = xgeXuiWidgetCreate();
	pApp->pRadioAWidget = xgeXuiWidgetCreate();
	pApp->pRadioBWidget = xgeXuiWidgetCreate();
	pApp->pSwitchWidget = xgeXuiWidgetCreate();
	pApp->pSliderWidget = xgeXuiWidgetCreate();
	pApp->pScrollBarWidget = xgeXuiWidgetCreate();
	pApp->pSplitterWidget = xgeXuiWidgetCreate();
	pApp->pProgressWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pToggleWidget == NULL) ||
	     (pApp->pCheckBoxWidget == NULL) || (pApp->pRadioAWidget == NULL) || (pApp->pRadioBWidget == NULL) ||
	     (pApp->pSwitchWidget == NULL) || (pApp->pSliderWidget == NULL) || (pApp->pScrollBarWidget == NULL) ||
	     (pApp->pSplitterWidget == NULL) || (pApp->pProgressWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetAdd(pRoot, pApp->pRootPanel);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pToggleWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pCheckBoxWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pRadioAWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pRadioBWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSwitchWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSliderWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pScrollBarWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pSplitterWidget);
	xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pProgressWidget);

	xgeXuiWidgetSetBackground(pApp->pRootPanel, XGE_COLOR_RGBA(18, 22, 30, 255));
	xgeXuiWidgetSetRadius(pApp->pRootPanel, 8.0f);
	xgeXuiWidgetSetBackground(pApp->pStatusWidget, XGE_COLOR_RGBA(46, 58, 78, 255));
	xgeXuiWidgetSetPaddingPx(pApp->pStatusWidget, 10.0f, 8.0f, 10.0f, 8.0f);
	if ( xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui control proc lab") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));

	if ( xgeXuiToggleInit(&pApp->tToggle, &pApp->tXui, pApp->pToggleWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiToggleSetText(&pApp->tToggle, pFont, g_sToggleText);
	xgeXuiToggleSetChange(&pApp->tToggle, ToggleChange, pApp);

	if ( xgeXuiCheckBoxInit(&pApp->tCheckBox, &pApp->tXui, pApp->pCheckBoxWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, pFont, g_sCheckBoxText);
	xgeXuiCheckBoxSetChange(&pApp->tCheckBox, CheckBoxChange, pApp);

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	if ( xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pRadioAWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pRadioBWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 1);
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 2);
	xgeXuiRadioSetText(&pApp->tRadioA, pFont, g_sRadioAText);
	xgeXuiRadioSetText(&pApp->tRadioB, pFont, g_sRadioBText);
	xgeXuiRadioSetChange(&pApp->tRadioA, RadioChange, pApp);
	xgeXuiRadioSetChange(&pApp->tRadioB, RadioChange, pApp);
	xgeXuiRadioGroupSetSelected(&pApp->tRadioGroup, 1);

	if ( xgeXuiSwitchInit(&pApp->tSwitch, &pApp->tXui, pApp->pSwitchWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSwitchSetText(&pApp->tSwitch, pFont, g_sSwitchText);
	xgeXuiSwitchSetChange(&pApp->tSwitch, SwitchChange, pApp);

	if ( xgeXuiSliderInit(&pApp->tSlider, &pApp->tXui, pApp->pSliderWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSliderSetRange(&pApp->tSlider, 0.0f, 10.0f);
	xgeXuiSliderSetValue(&pApp->tSlider, 2.0f);
	xgeXuiSliderSetColors(&pApp->tSlider,
		XGE_COLOR_RGBA(88, 98, 120, 255),
		XGE_COLOR_RGBA(102, 216, 144, 255),
		XGE_COLOR_RGBA(236, 241, 248, 255),
		XGE_COLOR_RGBA(82, 98, 128, 255),
		XGE_COLOR_RGBA(84, 88, 94, 180));
	xgeXuiSliderSetChange(&pApp->tSlider, SliderChange, pApp);

	if ( xgeXuiScrollBarInit(&pApp->tScrollBar, &pApp->tXui, pApp->pScrollBarWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiScrollBarSetRange(&pApp->tScrollBar, 0.0f, 100.0f, 20.0f);
	xgeXuiScrollBarSetValue(&pApp->tScrollBar, 10.0f);
	xgeXuiScrollBarSetChange(&pApp->tScrollBar, ScrollBarChange, pApp);

	if ( xgeXuiSplitterInit(&pApp->tSplitter, &pApp->tXui, pApp->pSplitterWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSplitterSetRange(&pApp->tSplitter, 0.0f, 40.0f);
	xgeXuiSplitterSetValue(&pApp->tSplitter, 10.0f);
	xgeXuiSplitterSetOrientation(&pApp->tSplitter, XGE_XUI_SEPARATOR_HORIZONTAL);
	xgeXuiSplitterSetChange(&pApp->tSplitter, SplitterChange, pApp);

	if ( xgeXuiProgressInit(&pApp->tProgress, pApp->pProgressWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiProgressSetRange(&pApp->tProgress, 0.0f, 10.0f);
	xgeXuiProgressSetValue(&pApp->tProgress, 6.5f);
	xgeXuiProgressSetText(&pApp->tProgress, pFont, "Progress Paint Proc");
	xgeXuiProgressSetTextColor(&pApp->tProgress, XGE_COLOR_RGBA(248, 250, 252, 255));

	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;

	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	tCenter = WidgetCenter(pApp->pToggleWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiToggleEventProc(pApp->pToggleWidget, &tEvent, &pApp->tToggle);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiToggleEventProc(pApp->pToggleWidget, &tEvent, &pApp->tToggle);
	pApp->bToggleOK =
		(pApp->pToggleWidget->procEvent == xgeXuiToggleEventProc) &&
		(pApp->pToggleWidget->procPaint == xgeXuiTogglePaintProc) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 1) &&
		(pApp->iToggleChangeCount >= 1);

	tCenter = WidgetCenter(pApp->pCheckBoxWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiCheckBoxEventProc(pApp->pCheckBoxWidget, &tEvent, &pApp->tCheckBox);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiCheckBoxEventProc(pApp->pCheckBoxWidget, &tEvent, &pApp->tCheckBox);
	pApp->bCheckBoxOK =
		(pApp->pCheckBoxWidget->procEvent == xgeXuiCheckBoxEventProc) &&
		(pApp->pCheckBoxWidget->procPaint == xgeXuiCheckBoxPaintProc) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheckBox) == 1) &&
		(pApp->iCheckBoxChangeCount >= 1);

	tCenter = WidgetCenter(pApp->pRadioBWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiRadioEventProc(pApp->pRadioBWidget, &tEvent, &pApp->tRadioB);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiRadioEventProc(pApp->pRadioBWidget, &tEvent, &pApp->tRadioB);
	pApp->bRadioOK =
		(pApp->pRadioBWidget->procEvent == xgeXuiRadioEventProc) &&
		(pApp->pRadioBWidget->procPaint == xgeXuiRadioPaintProc) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 2) &&
		((xgeXuiRadioGetState(&pApp->tRadioB) & XGE_XUI_STATE_HOVER) != 0) &&
		(pApp->iRadioChangeCount >= 1);

	tCenter = WidgetCenter(pApp->pSwitchWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiSwitchEventProc(pApp->pSwitchWidget, &tEvent, &pApp->tSwitch);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	(void)xgeXuiSwitchEventProc(pApp->pSwitchWidget, &tEvent, &pApp->tSwitch);
	pApp->bSwitchOK =
		(pApp->pSwitchWidget->procEvent == xgeXuiSwitchEventProc) &&
		(pApp->pSwitchWidget->procPaint == xgeXuiSwitchPaintProc) &&
		(xgeXuiSwitchGetChecked(&pApp->tSwitch) == 1) &&
		(pApp->iSwitchChangeCount >= 1) &&
		((xgeXuiSwitchGetState(&pApp->tSwitch) & XGE_XUI_STATE_HOVER) != 0);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, pApp->pSliderWidget->tRect.fX + pApp->pSliderWidget->tRect.fW * 0.2f, pApp->pSliderWidget->tRect.fY + pApp->pSliderWidget->tRect.fH * 0.5f);
	(void)xgeXuiSliderEventProc(pApp->pSliderWidget, &tEvent, &pApp->tSlider);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, pApp->pSliderWidget->tRect.fX + pApp->pSliderWidget->tRect.fW * 0.78f, pApp->pSliderWidget->tRect.fY + pApp->pSliderWidget->tRect.fH * 0.5f);
	(void)xgeXuiSliderEventProc(pApp->pSliderWidget, &tEvent, &pApp->tSlider);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, pApp->pSliderWidget->tRect.fX + pApp->pSliderWidget->tRect.fW * 0.78f, pApp->pSliderWidget->tRect.fY + pApp->pSliderWidget->tRect.fH * 0.5f);
	(void)xgeXuiSliderEventProc(pApp->pSliderWidget, &tEvent, &pApp->tSlider);
	pApp->bSliderOK =
		(pApp->pSliderWidget->procEvent == xgeXuiSliderEventProc) &&
		(pApp->pSliderWidget->procPaint == xgeXuiSliderPaintProc) &&
		(xgeXuiSliderGetValue(&pApp->tSlider) > 6.5f) &&
		(pApp->iSliderChangeCount >= 1);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, pApp->pScrollBarWidget->tRect.fX + pApp->pScrollBarWidget->tRect.fW * 0.5f, pApp->pScrollBarWidget->tRect.fY + pApp->pScrollBarWidget->tRect.fH * 0.9f);
	(void)xgeXuiScrollBarEventProc(pApp->pScrollBarWidget, &tEvent, &pApp->tScrollBar);
	MakeWheelEvent(&tEvent, pApp->pScrollBarWidget->tRect.fX + pApp->pScrollBarWidget->tRect.fW * 0.5f, pApp->pScrollBarWidget->tRect.fY + pApp->pScrollBarWidget->tRect.fH * 0.5f, -1.0f);
	(void)xgeXuiScrollBarEventProc(pApp->pScrollBarWidget, &tEvent, &pApp->tScrollBar);
	pApp->bScrollBarOK =
		(pApp->pScrollBarWidget->procEvent == xgeXuiScrollBarEventProc) &&
		(pApp->pScrollBarWidget->procPaint == xgeXuiScrollBarPaintProc) &&
		(xgeXuiScrollBarGetValue(&pApp->tScrollBar) > 10.0f) &&
		(pApp->iScrollBarChangeCount >= 1);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, pApp->pSplitterWidget->tRect.fX + pApp->pSplitterWidget->tRect.fW * 0.5f, pApp->pSplitterWidget->tRect.fY + pApp->pSplitterWidget->tRect.fH * 0.5f);
	(void)xgeXuiSplitterEventProc(pApp->pSplitterWidget, &tEvent, &pApp->tSplitter);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 0, pApp->pSplitterWidget->tRect.fX + pApp->pSplitterWidget->tRect.fW * 0.5f, pApp->pSplitterWidget->tRect.fY + pApp->pSplitterWidget->tRect.fH * 0.5f + 12.0f);
	(void)xgeXuiSplitterEventProc(pApp->pSplitterWidget, &tEvent, &pApp->tSplitter);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, pApp->pSplitterWidget->tRect.fX + pApp->pSplitterWidget->tRect.fW * 0.5f, pApp->pSplitterWidget->tRect.fY + pApp->pSplitterWidget->tRect.fH * 0.5f + 12.0f);
	(void)xgeXuiSplitterEventProc(pApp->pSplitterWidget, &tEvent, &pApp->tSplitter);
	pApp->bSplitterOK =
		(pApp->pSplitterWidget->procEvent == xgeXuiSplitterEventProc) &&
		(pApp->pSplitterWidget->procPaint == xgeXuiSplitterPaintProc) &&
		(xgeXuiSplitterGetValue(&pApp->tSplitter) > 20.0f) &&
		(pApp->iSplitterChangeCount >= 1);

	pApp->bProgressOK =
		(pApp->pProgressWidget->procPaint == xgeXuiProgressPaintProc) &&
		FloatNear(xgeXuiProgressGetValue(&pApp->tProgress), 6.5f, 0.01f);

	ResetVisualStates(pApp);

	return XGE_OK;
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiProgressUnit(&pApp->tProgress);
	xgeXuiSplitterUnit(&pApp->tSplitter);
	xgeXuiScrollBarUnit(&pApp->tScrollBar);
	xgeXuiSliderUnit(&pApp->tSlider);
	xgeXuiSwitchUnit(&pApp->tSwitch);
	xgeXuiRadioUnit(&pApp->tRadioB);
	xgeXuiRadioUnit(&pApp->tRadioA);
	xgeXuiRadioGroupUnit(&pApp->tRadioGroup);
	xgeXuiCheckBoxUnit(&pApp->tCheckBox);
	xgeXuiToggleUnit(&pApp->tToggle);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	memset(pApp, 0, sizeof(*pApp));
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( LoadFont(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( CreateUI(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	if ( RunStaticChecks(pApp) != XGE_OK ) {
		AppUnit(pApp);
		return XGE_ERROR;
	}
	UpdateStatus(pApp);
	return XGE_OK;
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
		xgeXuiTogglePaintProc(pApp->pToggleWidget, &pApp->tToggle);
		xgeXuiCheckBoxPaintProc(pApp->pCheckBoxWidget, &pApp->tCheckBox);
		xgeXuiRadioPaintProc(pApp->pRadioAWidget, &pApp->tRadioA);
		xgeXuiRadioPaintProc(pApp->pRadioBWidget, &pApp->tRadioB);
		xgeXuiSwitchPaintProc(pApp->pSwitchWidget, &pApp->tSwitch);
		xgeXuiSliderPaintProc(pApp->pSliderWidget, &pApp->tSlider);
		xgeXuiScrollBarPaintProc(pApp->pScrollBarWidget, &pApp->tScrollBar);
		xgeXuiSplitterPaintProc(pApp->pSplitterWidget, &pApp->tSplitter);
		xgeXuiProgressPaintProc(pApp->pProgressWidget, &pApp->tProgress);
		pApp->bDirectPaintOK = 1;
	}
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();

	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui-control-proc-lab final-summary frames=%d toggle=%d checkbox=%d radio=%d switch=%d slider=%d scroll=%d split=%d progress=%d paint=%d values(toggle=%d checkbox=%d radio=%d switch=%d slider=%.2f scroll=%.2f split=%.2f progress=%.2f) changes=%d/%d/%d/%d/%d/%d/%d\n",
			pApp->iFrameCount,
			pApp->bToggleOK,
			pApp->bCheckBoxOK,
			pApp->bRadioOK,
			pApp->bSwitchOK,
			pApp->bSliderOK,
			pApp->bScrollBarOK,
			pApp->bSplitterOK,
			pApp->bProgressOK,
			pApp->bDirectPaintOK,
			xgeXuiToggleGetChecked(&pApp->tToggle),
			xgeXuiCheckBoxGetChecked(&pApp->tCheckBox),
			xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup),
			xgeXuiSwitchGetChecked(&pApp->tSwitch),
			xgeXuiSliderGetValue(&pApp->tSlider),
			xgeXuiScrollBarGetValue(&pApp->tScrollBar),
			xgeXuiSplitterGetValue(&pApp->tSplitter),
			xgeXuiProgressGetValue(&pApp->tProgress),
			pApp->iToggleChangeCount,
			pApp->iCheckBoxChangeCount,
			pApp->iRadioChangeCount,
			pApp->iSwitchChangeCount,
			pApp->iSliderChangeCount,
			pApp->iScrollBarChangeCount,
			pApp->iSplitterChangeCount);
		printf("xui-control-proc-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 760;
	tDesc.iHeight = 380;
	tDesc.sTitle = "XGE XUI Control Proc Lab";
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
		(tApp.bToggleOK && tApp.bCheckBoxOK && tApp.bRadioOK && tApp.bSwitchOK &&
		 tApp.bSliderOK && tApp.bScrollBarOK && tApp.bSplitterOK && tApp.bProgressOK &&
		 tApp.bDirectPaintOK) ? 0 : 3;
	AppUnit(&tApp);
	xgeUnit();
	return iExitCode;
}

