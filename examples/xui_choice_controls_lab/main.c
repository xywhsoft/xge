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
	xge_xui_widget pPanelWidget;
	xge_xui_widget pToggleWidget;
	xge_xui_widget pCheckBoxWidget;
	xge_xui_widget pRadioAWidget;
	xge_xui_widget pRadioBWidget;
	xge_xui_widget pSwitchWidget;
	xge_xui_label_t tStatusLabel;
	xge_xui_panel_t tPanel;
	xge_xui_toggle_t tToggle;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_switch_t tSwitch;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iToggleCallbackCount;
	int iCheckBoxCallbackCount;
	int iRadioCallbackCount;
	int iRadioGroupCallbackCount;
	int iSwitchCallbackCount;
	int iLastToggleValue;
	int iLastCheckBoxValue;
	int iLastRadioValue;
	int iLastGroupValue;
	int iLastSwitchValue;
	int bToggleOK;
	int bCheckBoxOK;
	int bRadioGroupOK;
	int bRadioOK;
	int bSwitchOK;
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

static xge_vec2_t WidgetCenter(xge_xui_widget pWidget)
{
	xge_vec2_t tCenter;

	tCenter.fX = pWidget->tRect.fX + (pWidget->tRect.fW * 0.5f);
	tCenter.fY = pWidget->tRect.fY + (pWidget->tRect.fH * 0.5f);
	return tCenter;
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
			printf("xui-choice-controls-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-choice-controls-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iToggleCallbackCount++;
		pApp->iLastToggleValue = bChecked;
	}
}

static void CheckBoxChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCheckBoxCallbackCount++;
		pApp->iLastCheckBoxValue = bChecked;
	}
}

static void RadioChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;
	xge_xui_radio pRadio;

	pApp = (app_state_t*)pUser;
	pRadio = (xge_xui_radio)((pWidget != NULL) ? pWidget->pUser : NULL);
	if ( (pApp != NULL) && (pRadio != NULL) && bChecked ) {
		pApp->iRadioCallbackCount++;
		pApp->iLastRadioValue = pRadio->iValue;
	}
}

static void RadioGroupChange(xge_xui_widget pWidget, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iRadioGroupCallbackCount++;
		pApp->iLastGroupValue = iValue;
	}
}

static void SwitchChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSwitchCallbackCount++;
		pApp->iLastSwitchValue = bChecked;
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
	tRoot.fY = 20.0f;
	tRoot.fW = (float)iWidth - 40.0f;
	tRoot.fH = (float)iHeight - 40.0f;
	if ( tRoot.fW < 500.0f ) {
		tRoot.fW = 500.0f;
	}
	if ( tRoot.fH < 280.0f ) {
		tRoot.fH = 280.0f;
	}
	xgeXuiWidgetSetRect(pApp->pRootPanel, tRoot);
	xgeXuiWidgetSetRect(pApp->pStatusWidget, (xge_rect_t){ 0.0f, 0.0f, tRoot.fW, 38.0f });
	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 22.0f, 58.0f, tRoot.fW - 44.0f, tRoot.fH - 78.0f });
	xgeXuiWidgetSetRect(pApp->pToggleWidget, (xge_rect_t){ 26.0f, 40.0f, 200.0f, 30.0f });
	xgeXuiWidgetSetRect(pApp->pCheckBoxWidget, (xge_rect_t){ 26.0f, 86.0f, 200.0f, 30.0f });
	xgeXuiWidgetSetRect(pApp->pRadioAWidget, (xge_rect_t){ 26.0f, 132.0f, 200.0f, 30.0f });
	xgeXuiWidgetSetRect(pApp->pRadioBWidget, (xge_rect_t){ 26.0f, 170.0f, 200.0f, 30.0f });
	xgeXuiWidgetSetRect(pApp->pSwitchWidget, (xge_rect_t){ 26.0f, 216.0f, 220.0f, 30.0f });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"toggle=%d checkbox=%d group=%d radio=%d switch=%d state=%d cb=%d/%d/%d/%d/%d",
		pApp->bToggleOK,
		pApp->bCheckBoxOK,
		pApp->bRadioGroupOK,
		pApp->bRadioOK,
		pApp->bSwitchOK,
		pApp->bStateOK,
		pApp->iToggleCallbackCount,
		pApp->iCheckBoxCallbackCount,
		pApp->iRadioGroupCallbackCount,
		pApp->iRadioCallbackCount,
		pApp->iSwitchCallbackCount);
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
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(20, 26, 36, 255);
	tTheme.iPanelColor = XGE_COLOR_RGBA(32, 40, 54, 255);
	tTheme.iBorderColor = XGE_COLOR_RGBA(84, 96, 120, 255);
	tTheme.iAccentColor = XGE_COLOR_RGBA(96, 214, 144, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(56, 124, 214, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(76, 148, 236, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(38, 96, 176, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(88, 108, 144, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(84, 86, 92, 180);
	tTheme.fRadius = 8.0f;
	tTheme.fPadding = 8.0f;
	tTheme.fSpacing = 10.0f;
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pRootPanel = xgeXuiWidgetCreate();
	pApp->pStatusWidget = xgeXuiWidgetCreate();
	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->pToggleWidget = xgeXuiWidgetCreate();
	pApp->pCheckBoxWidget = xgeXuiWidgetCreate();
	pApp->pRadioAWidget = xgeXuiWidgetCreate();
	pApp->pRadioBWidget = xgeXuiWidgetCreate();
	pApp->pSwitchWidget = xgeXuiWidgetCreate();
	if ( (pApp->pRootPanel == NULL) || (pApp->pStatusWidget == NULL) || (pApp->pPanelWidget == NULL) || (pApp->pToggleWidget == NULL) || (pApp->pCheckBoxWidget == NULL) || (pApp->pRadioAWidget == NULL) || (pApp->pRadioBWidget == NULL) || (pApp->pSwitchWidget == NULL) ) {
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
	xgeXuiLabelInit(&pApp->tStatusLabel, pApp->pStatusWidget, pFont, "xui choice controls lab");
	xgeXuiLabelSetColor(&pApp->tStatusLabel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pStatusWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "Choice Controls");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(32, 40, 54, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(248, 250, 252, 255));
	if ( xgeXuiWidgetAdd(pApp->pRootPanel, pApp->pPanelWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiToggleInit(&pApp->tToggle, &pApp->tXui, pApp->pToggleWidget);
	xgeXuiToggleSetText(&pApp->tToggle, pFont, "Toggle");
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pToggleWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiCheckBoxInit(&pApp->tCheckBox, &pApp->tXui, pApp->pCheckBoxWidget);
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, pFont, "CheckBox");
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pCheckBoxWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pRadioAWidget);
	xgeXuiRadioSetText(&pApp->tRadioA, pFont, "Radio A");
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 7);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pRadioAWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pRadioBWidget);
	xgeXuiRadioSetText(&pApp->tRadioB, pFont, "Radio B");
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 9);
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pRadioBWidget) != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiSwitchInit(&pApp->tSwitch, &pApp->tXui, pApp->pSwitchWidget);
	xgeXuiSwitchSetText(&pApp->tSwitch, pFont, "Switch");
	if ( xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSwitchWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int RunStaticChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_vec2_t tCenter;
	int bToggleDefaultOK;
	int bToggleUpdateOK;
	int bCheckDefaultOK;
	int bCheckUpdateOK;
	int bRadioGroupDefaultOK;
	int bRadioDefaultOK;
	int bRadioSetOK;
	int bRadioMouseOK;
	int bRadioKeyOK;
	int bSwitchDefaultOK;
	int bSwitchUpdateOK;

	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);

	bToggleDefaultOK =
		(pApp->tToggle.pFont == (pApp->bFontReady ? &pApp->tFont : NULL)) &&
		(strcmp(pApp->tToggle.sText, "Toggle") == 0) &&
		(pApp->tToggle.iColorChecked == XGE_COLOR_RGBA(96, 214, 144, 255)) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 0);
	xgeXuiToggleSetChange(&pApp->tToggle, ToggleChange, pApp);
	xgeXuiToggleSetText(&pApp->tToggle, pApp->bFontReady ? &pApp->tFont : NULL, "Toggle Updated");
	xgeXuiToggleSetChecked(&pApp->tToggle, 1);
	xgeXuiToggleSetTextColor(&pApp->tToggle, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiToggleSetColors(
		&pApp->tToggle,
		XGE_COLOR_RGBA(64, 128, 214, 255),
		XGE_COLOR_RGBA(86, 150, 238, 255),
		XGE_COLOR_RGBA(44, 96, 172, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(86, 214, 154, 255));
	xgeXuiSetFocus(&pApp->tXui, pApp->pToggleWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	xgeXuiToggleEvent(&pApp->tToggle, &tEvent);
	tCenter = WidgetCenter(pApp->pToggleWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiToggleEvent(&pApp->tToggle, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiToggleEvent(&pApp->tToggle, &tEvent);
	bToggleUpdateOK =
		bToggleDefaultOK &&
		(strcmp(pApp->tToggle.sText, "Toggle Updated") == 0) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 1) &&
		(pApp->tToggle.iChangeCount == 2) &&
		(pApp->iToggleCallbackCount == 2) &&
		(pApp->iLastToggleValue == 1) &&
		((xgeXuiToggleGetState(&pApp->tToggle) & XGE_XUI_STATE_HOVER) != 0);
	pApp->bToggleOK = bToggleUpdateOK;

	bCheckDefaultOK =
		(strcmp(pApp->tCheckBox.sText, "CheckBox") == 0) &&
		(pApp->tCheckBox.iColorNormal == XGE_COLOR_RGBA(0, 0, 0, 0)) &&
		(pApp->tCheckBox.iColorBox == XGE_COLOR_RGBA(180, 186, 196, 255)) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheckBox) == 0);
	xgeXuiCheckBoxSetChange(&pApp->tCheckBox, CheckBoxChange, pApp);
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, pApp->bFontReady ? &pApp->tFont : NULL, "CheckBox Updated");
	xgeXuiCheckBoxSetChecked(&pApp->tCheckBox, 1);
	xgeXuiCheckBoxSetTextColor(&pApp->tCheckBox, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiCheckBoxSetColors(
		&pApp->tCheckBox,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(74, 90, 116, 255),
		XGE_COLOR_RGBA(46, 60, 82, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(202, 208, 216, 255),
		XGE_COLOR_RGBA(244, 182, 92, 255));
	xgeXuiSetFocus(&pApp->tXui, pApp->pCheckBoxWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	xgeXuiCheckBoxEvent(&pApp->tCheckBox, &tEvent);
	tCenter = WidgetCenter(pApp->pCheckBoxWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiCheckBoxEvent(&pApp->tCheckBox, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiCheckBoxEvent(&pApp->tCheckBox, &tEvent);
	bCheckUpdateOK =
		bCheckDefaultOK &&
		(strcmp(pApp->tCheckBox.sText, "CheckBox Updated") == 0) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheckBox) == 1) &&
		(pApp->tCheckBox.iChangeCount == 2) &&
		(pApp->iCheckBoxCallbackCount == 2) &&
		(pApp->iLastCheckBoxValue == 1);
	pApp->bCheckBoxOK = bCheckUpdateOK;

	bRadioGroupDefaultOK =
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == -1) &&
		(pApp->tRadioGroup.iChangeCount == 0);
	bRadioDefaultOK =
		(strcmp(pApp->tRadioA.sText, "Radio A") == 0) &&
		(strcmp(pApp->tRadioB.sText, "Radio B") == 0) &&
		(pApp->tRadioA.iValue == 7) &&
		(pApp->tRadioB.iValue == 9) &&
		(pApp->tRadioA.iColorRing == XGE_COLOR_RGBA(180, 186, 196, 255)) &&
		(pApp->tRadioB.iColorRing == XGE_COLOR_RGBA(180, 186, 196, 255));
	xgeXuiRadioGroupSetChange(&pApp->tRadioGroup, RadioGroupChange, pApp);
	xgeXuiRadioSetChange(&pApp->tRadioA, RadioChange, pApp);
	xgeXuiRadioSetChange(&pApp->tRadioB, RadioChange, pApp);
	xgeXuiRadioSetTextColor(&pApp->tRadioA, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiRadioSetTextColor(&pApp->tRadioB, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiRadioSetColors(
		&pApp->tRadioA,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(74, 90, 116, 255),
		XGE_COLOR_RGBA(46, 60, 82, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(184, 190, 200, 255),
		XGE_COLOR_RGBA(96, 214, 144, 255));
	xgeXuiRadioSetColors(
		&pApp->tRadioB,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(74, 90, 116, 255),
		XGE_COLOR_RGBA(46, 60, 82, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(184, 190, 200, 255),
		XGE_COLOR_RGBA(96, 214, 144, 255));
	xgeXuiRadioGroupSetSelected(&pApp->tRadioGroup, 7);
	xgeXuiRadioSetChecked(&pApp->tRadioB, 1);
	bRadioSetOK =
		bRadioGroupDefaultOK &&
		bRadioDefaultOK &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 9) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 0) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 1) &&
		(pApp->tRadioGroup.iChangeCount >= 2) &&
		(pApp->iRadioGroupCallbackCount >= 2) &&
		(pApp->iLastGroupValue == 9) &&
		((pApp->tRadioA.iChangeCount + pApp->tRadioB.iChangeCount) >= 3);
	tCenter = WidgetCenter(pApp->pRadioAWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiRadioEvent(&pApp->tRadioA, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiRadioEvent(&pApp->tRadioA, &tEvent);
	bRadioMouseOK =
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 7) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 1) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 0);
	xgeXuiSetFocus(&pApp->tXui, pApp->pRadioBWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	xgeXuiRadioEvent(&pApp->tRadioB, &tEvent);
	bRadioKeyOK =
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 9) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 0) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 1) &&
		(pApp->iRadioGroupCallbackCount >= 4) &&
		(pApp->iRadioCallbackCount >= 4) &&
		(pApp->iLastRadioValue == 9) &&
		(pApp->iLastGroupValue == 9) &&
		((pApp->tRadioA.iChangeCount + pApp->tRadioB.iChangeCount) >= 5);
	pApp->bRadioGroupOK =
		bRadioGroupDefaultOK &&
		bRadioDefaultOK &&
		bRadioSetOK &&
		bRadioMouseOK &&
		bRadioKeyOK &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 9) &&
		(pApp->tRadioGroup.iChangeCount >= 4) &&
		(pApp->iRadioGroupCallbackCount >= 4) &&
		(pApp->iLastGroupValue == 9);
	pApp->bRadioOK =
		pApp->bRadioGroupOK &&
		(pApp->tRadioA.pGroup == &pApp->tRadioGroup) &&
		(pApp->tRadioB.pGroup == &pApp->tRadioGroup) &&
		(pApp->tRadioA.iValue == 7) &&
		(pApp->tRadioB.iValue == 9) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 0) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 1) &&
		(pApp->iRadioCallbackCount >= 4) &&
		(pApp->iLastRadioValue == 9);

	bSwitchDefaultOK =
		(strcmp(pApp->tSwitch.sText, "Switch") == 0) &&
		(pApp->tSwitch.iColorTrack == XGE_COLOR_RGBA(96, 106, 120, 255)) &&
		(pApp->tSwitch.iColorKnob == XGE_COLOR_RGBA(255, 255, 255, 255)) &&
		(xgeXuiSwitchGetChecked(&pApp->tSwitch) == 0);
	xgeXuiSwitchSetChange(&pApp->tSwitch, SwitchChange, pApp);
	xgeXuiSwitchSetText(&pApp->tSwitch, pApp->bFontReady ? &pApp->tFont : NULL, "Switch Updated");
	xgeXuiSwitchSetChecked(&pApp->tSwitch, 1);
	xgeXuiSwitchSetTextColor(&pApp->tSwitch, XGE_COLOR_RGBA(248, 250, 252, 255));
	xgeXuiSwitchSetColors(
		&pApp->tSwitch,
		XGE_COLOR_RGBA(0, 0, 0, 0),
		XGE_COLOR_RGBA(74, 90, 116, 255),
		XGE_COLOR_RGBA(46, 60, 82, 255),
		XGE_COLOR_RGBA(96, 112, 142, 255),
		XGE_COLOR_RGBA(84, 86, 92, 180),
		XGE_COLOR_RGBA(116, 124, 140, 255),
		XGE_COLOR_RGBA(244, 182, 92, 255),
		XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiSetFocus(&pApp->tXui, pApp->pSwitchWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	xgeXuiSwitchEvent(&pApp->tSwitch, &tEvent);
	tCenter = WidgetCenter(pApp->pSwitchWidget);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiSwitchEvent(&pApp->tSwitch, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, tCenter.fX, tCenter.fY);
	xgeXuiSwitchEvent(&pApp->tSwitch, &tEvent);
	bSwitchUpdateOK =
		bSwitchDefaultOK &&
		(strcmp(pApp->tSwitch.sText, "Switch Updated") == 0) &&
		(xgeXuiSwitchGetChecked(&pApp->tSwitch) == 1) &&
		(pApp->tSwitch.iChangeCount == 2) &&
		(pApp->iSwitchCallbackCount == 2) &&
		(pApp->iLastSwitchValue == 1);
	pApp->bSwitchOK = bSwitchUpdateOK;

	xgeXuiWidgetSetEnabled(pApp->pToggleWidget, 0);
	xgeXuiWidgetSetEnabled(pApp->pCheckBoxWidget, 0);
	xgeXuiWidgetSetEnabled(pApp->pSwitchWidget, 0);
	pApp->bStateOK =
		((xgeXuiToggleGetState(&pApp->tToggle) & XGE_XUI_STATE_DISABLED) != 0) &&
		((xgeXuiCheckBoxGetState(&pApp->tCheckBox) & XGE_XUI_STATE_DISABLED) != 0) &&
		((xgeXuiSwitchGetState(&pApp->tSwitch) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pToggleWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pCheckBoxWidget, 1);
	xgeXuiWidgetSetEnabled(pApp->pSwitchWidget, 1);

	UpdateStatus(pApp);
	return XGE_OK;
}

static int AppInit(app_state_t* pApp, int iFrameLimit)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->iFrameLimit = iFrameLimit;
	pApp->iLastToggleValue = -1;
	pApp->iLastCheckBoxValue = -1;
	pApp->iLastRadioValue = -1;
	pApp->iLastGroupValue = -1;
	pApp->iLastSwitchValue = -1;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)LoadFont(pApp);
	return RunStaticChecks(pApp);
}

static void AppUnit(app_state_t* pApp)
{
	xgeXuiSwitchUnit(&pApp->tSwitch);
	xgeXuiRadioUnit(&pApp->tRadioB);
	xgeXuiRadioUnit(&pApp->tRadioA);
	xgeXuiRadioGroupUnit(&pApp->tRadioGroup);
	xgeXuiCheckBoxUnit(&pApp->tCheckBox);
	xgeXuiToggleUnit(&pApp->tToggle);
	xgeXuiPanelUnit(&pApp->tPanel);
	xgeXuiLabelUnit(&pApp->tStatusLabel);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
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
	if ( pApp->iFrameCount >= pApp->iFrameLimit ) {
		printf(
			"xui-choice-controls-lab final-summary frames=%d toggle=%d checkbox=%d group=%d radio=%d switch=%d state=%d checked(toggle=%d checkbox=%d radio=%d/%d switch=%d group=%d) callbacks=%d/%d/%d/%d/%d changes=%d/%d/%d/%d/%d\n",
			pApp->iFrameCount,
			pApp->bToggleOK,
			pApp->bCheckBoxOK,
			pApp->bRadioGroupOK,
			pApp->bRadioOK,
			pApp->bSwitchOK,
			pApp->bStateOK,
			xgeXuiToggleGetChecked(&pApp->tToggle),
			xgeXuiCheckBoxGetChecked(&pApp->tCheckBox),
			xgeXuiRadioGetChecked(&pApp->tRadioA),
			xgeXuiRadioGetChecked(&pApp->tRadioB),
			xgeXuiSwitchGetChecked(&pApp->tSwitch),
			xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup),
			pApp->iToggleCallbackCount,
			pApp->iCheckBoxCallbackCount,
			pApp->iRadioGroupCallbackCount,
			pApp->iRadioCallbackCount,
			pApp->iSwitchCallbackCount,
			pApp->tToggle.iChangeCount,
			pApp->tCheckBox.iChangeCount,
			pApp->tRadioGroup.iChangeCount,
			pApp->tRadioA.iChangeCount + pApp->tRadioB.iChangeCount,
			pApp->tSwitch.iChangeCount);
		printf("xui-choice-controls-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	iFrameLimit = ArgInt(getenv("XGE_XUI_CHOICE_CONTROLS_FRAMES"), 180);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			iFrameLimit = ArgInt(argv[++i], iFrameLimit);
		}
	}

	tDesc.iWidth = 560;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE XUI Choice Controls Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( AppInit(&tApp, iFrameLimit) != XGE_OK ) {
		printf("xui-choice-controls-lab init failed\n");
		xgeUnit();
		return 2;
	}
	xgeRun(AppFrame, &tApp);
	AppUnit(&tApp);
	xgeUnit();
	return 0;
}
