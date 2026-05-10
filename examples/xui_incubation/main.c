#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_texture_t tIconTexture;
	xge_xui_widget pPanelWidget;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pIconActionWidget;
	xge_xui_widget pTabsWidget;
	xge_xui_widget pProgressWidget;
	xge_xui_widget pCheckBoxWidget;
	xge_xui_widget pRadioAWidget;
	xge_xui_widget pRadioBWidget;
	xge_xui_widget pToggleWidget;
	xge_xui_widget pComboWidget;
	xge_xui_widget pSeparatorWidget;
	xge_xui_widget pSplitterWidget;
	xge_xui_widget pScrollBarWidget;
	xge_xui_widget pScrollWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pPopupLabelWidget;
	xge_xui_panel_t tPanel;
	xge_xui_button_t tButton;
	xge_xui_button_t tIconAction;
	xge_xui_tabs_t tTabs;
	xge_xui_progress_t tProgress;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_toggle_t tToggle;
	xge_xui_combo_box_t tCombo;
	xge_xui_separator_t tSeparator;
	xge_xui_splitter_t tSplitter;
	xge_xui_scrollbar_t tScrollBar;
	xge_xui_scroll_view_t tScroll;
	xge_xui_popup_t tPopup;
	xge_xui_label_t tPopupLabel;
	float fProgress;
	float fMouseX;
	float fMouseY;
	int bMouseDown;
	int bFontReady;
	int bIconReady;
	int iFrameCount;
	int iClickCount;
	int iIconClickCount;
	int iTabSelected;
	int iTabChangeCount;
	int iCheckBoxChangeCount;
	int bCheckBoxChecked;
	int iRadioValue;
	int iRadioChangeCount;
	int bToggleChecked;
	int iToggleChangeCount;
	int iComboSelected;
	int iComboChangeCount;
	float fSplitterValue;
	int iSplitterChangeCount;
	float fScrollBarValue;
	int iScrollBarChangeCount;
	int iPopupCloseCount;
	char sButtonText[64];
} app_state_t;

static const char* g_arrIncubationTabs[] = { "Main", "State", "Input" };
static const char* g_arrIncubationCombo[] = { "Small", "Medium", "Large", "Custom" };

static void AppTabsSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iTabChangeCount++;
	pApp->iTabSelected = iIndex;
}

static void AppIconActionClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iIconClickCount++;
}

static void AppCheckBoxChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iCheckBoxChangeCount++;
	pApp->bCheckBoxChecked = bChecked;
}

static void AppRadioChange(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iRadioChangeCount++;
	pApp->iRadioValue = iIndex;
}

static void AppToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iToggleChangeCount++;
	pApp->bToggleChecked = bChecked;
}

static void AppComboSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iComboChangeCount++;
	pApp->iComboSelected = iIndex;
}

static void AppSplitterChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iSplitterChangeCount++;
	pApp->fSplitterValue = fValue;
}

static void AppScrollBarChange(xge_xui_widget pWidget, float fValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iScrollBarChangeCount++;
	pApp->fScrollBarValue = fValue;
}

static void AppPopupClose(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iPopupCloseCount++;
	snprintf(pApp->sButtonText, sizeof(pApp->sButtonText), "Open Popup");
	xgeXuiButtonSetText(&pApp->tButton, pApp->bFontReady ? &pApp->tFont : NULL, pApp->sButtonText);
}

static void AppButtonClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	pApp->iClickCount++;
	xgeXuiPopupSetOpen(&pApp->tPopup, xgeXuiPopupIsOpen(&pApp->tPopup) == 0);
	snprintf(pApp->sButtonText, sizeof(pApp->sButtonText), xgeXuiPopupIsOpen(&pApp->tPopup) ? "Close Popup" : "Open Popup");
	xgeXuiButtonSetText(&pApp->tButton, pApp->bFontReady ? &pApp->tFont : NULL, pApp->sButtonText);
}

static void AppDispatchMouse(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	float fDX;
	float fDY;
	int bDown;

	xgeMouseGet(&fX, &fY);
	fDX = fX - pApp->fMouseX;
	fDY = fY - pApp->fMouseY;
	if ( (fDX != 0.0f) || (fDY != 0.0f) ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_MOVE;
		tEvent.fX = fX;
		tEvent.fY = fY;
		tEvent.fDX = fDX;
		tEvent.fDY = fDY;
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	}
	bDown = xgeMouseDown(XGE_MOUSE_LEFT);
	if ( (bDown != 0) && (pApp->bMouseDown == 0) ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_DOWN;
		tEvent.iParam1 = XGE_MOUSE_LEFT;
		tEvent.fX = fX;
		tEvent.fY = fY;
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	} else if ( (bDown == 0) && (pApp->bMouseDown != 0) ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_MOUSE_UP;
		tEvent.iParam1 = XGE_MOUSE_LEFT;
		tEvent.fX = fX;
		tEvent.fY = fY;
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	}
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iType = XGE_EVENT_KEY_DOWN;
		tEvent.iParam1 = XGE_KEY_ESCAPE;
		xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	}
	pApp->fMouseX = fX;
	pApp->fMouseY = fY;
	pApp->bMouseDown = bDown;
}

static int AppLoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/NotoSansSC-VF.ttf",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void AppCreateIconTexture(app_state_t* pApp)
{
	unsigned char arrPixels[8 * 8 * 4];
	int x;
	int y;
	int i;

	memset(arrPixels, 0, sizeof(arrPixels));
	for ( y = 0; y < 8; y++ ) {
		for ( x = 0; x < 8; x++ ) {
			i = (y * 8 + x) * 4;
			if ( x == y || x == (7 - y) || y == 3 || y == 4 ) {
				arrPixels[i + 0] = 255;
				arrPixels[i + 1] = 255;
				arrPixels[i + 2] = 255;
				arrPixels[i + 3] = 255;
			}
		}
	}
	if ( xgeTextureCreateRGBA(&pApp->tIconTexture, 8, 8, arrPixels) == XGE_OK ) {
		pApp->bIconReady = 1;
	}
}

static int AppCreateWidgetTree(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_theme_t tTheme;
	xge_font pFont;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(248, 250, 252, 255);
	tTheme.iStateNormal = XGE_COLOR_RGBA(54, 126, 218, 255);
	tTheme.iStateHover = XGE_COLOR_RGBA(76, 150, 240, 255);
	tTheme.iStateActive = XGE_COLOR_RGBA(36, 94, 170, 255);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	snprintf(pApp->sButtonText, sizeof(pApp->sButtonText), "Button");

	pApp->pPanelWidget = xgeXuiWidgetCreate();
	pApp->pButtonWidget = xgeXuiWidgetCreate();
	pApp->pIconActionWidget = xgeXuiWidgetCreate();
	pApp->pTabsWidget = xgeXuiWidgetCreate();
	pApp->pProgressWidget = xgeXuiWidgetCreate();
	pApp->pCheckBoxWidget = xgeXuiWidgetCreate();
	pApp->pRadioAWidget = xgeXuiWidgetCreate();
	pApp->pRadioBWidget = xgeXuiWidgetCreate();
	pApp->pToggleWidget = xgeXuiWidgetCreate();
	pApp->pComboWidget = xgeXuiWidgetCreate();
	pApp->pSeparatorWidget = xgeXuiWidgetCreate();
	pApp->pSplitterWidget = xgeXuiWidgetCreate();
	pApp->pScrollBarWidget = xgeXuiWidgetCreate();
	pApp->pScrollWidget = xgeXuiWidgetCreate();
	pApp->pPopupWidget = xgeXuiWidgetCreate();
	pApp->pPopupLabelWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pApp->pPanelWidget == NULL) || (pApp->pButtonWidget == NULL) || (pApp->pIconActionWidget == NULL) || (pApp->pTabsWidget == NULL) || (pApp->pProgressWidget == NULL) || (pApp->pCheckBoxWidget == NULL) || (pApp->pRadioAWidget == NULL) || (pApp->pRadioBWidget == NULL) || (pApp->pToggleWidget == NULL) || (pApp->pComboWidget == NULL) || (pApp->pSeparatorWidget == NULL) || (pApp->pSplitterWidget == NULL) || (pApp->pScrollBarWidget == NULL) || (pApp->pScrollWidget == NULL) || (pApp->pPopupWidget == NULL) || (pApp->pPopupLabelWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanelWidget, (xge_rect_t){ 32.0f, 32.0f, 430.0f, 300.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanelWidget, 16.0f, 16.0f, 16.0f, 16.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanelWidget);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "XUI Incubation");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(28, 38, 54, 255));
	xgeXuiPanelSetTitleColor(&pApp->tPanel, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pPanelWidget);

	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 56.0f, 88.0f, 170.0f, 44.0f });
	xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget);
	snprintf(pApp->sButtonText, sizeof(pApp->sButtonText), "Open Popup");
	xgeXuiButtonSetText(&pApp->tButton, pFont, pApp->sButtonText);
	xgeXuiButtonSetClick(&pApp->tButton, AppButtonClick, pApp);
	xgeXuiButtonSetTextColor(&pApp->tButton, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiButtonSetColors(&pApp->tButton, XGE_COLOR_RGBA(42, 128, 232, 255), XGE_COLOR_RGBA(66, 154, 255, 255), XGE_COLOR_RGBA(24, 86, 168, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pButtonWidget);

	xgeXuiWidgetSetRect(pApp->pIconActionWidget, (xge_rect_t){ 236.0f, 88.0f, 36.0f, 44.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pIconActionWidget, 8.0f, 10.0f, 8.0f, 10.0f);
	xgeXuiButtonInit(&pApp->tIconAction, &pApp->tXui, pApp->pIconActionWidget);
	xgeXuiButtonSetText(&pApp->tIconAction, pFont, "");
	xgeXuiButtonSetIcon(&pApp->tIconAction, pApp->bIconReady ? &pApp->tIconTexture : NULL, (xge_rect_t){ 0.0f, 0.0f, 16.0f, 16.0f });
	xgeXuiButtonSetIconLayout(&pApp->tIconAction, XGE_XUI_BUTTON_ICON_LEFT, 18.0f, 0.0f);
	xgeXuiButtonSetClick(&pApp->tIconAction, AppIconActionClick, pApp);
	xgeXuiButtonSetColors(&pApp->tIconAction, XGE_COLOR_RGBA(52, 64, 82, 255), XGE_COLOR_RGBA(72, 86, 108, 255), XGE_COLOR_RGBA(36, 48, 66, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pIconActionWidget);
	xgeXuiWidgetSetTooltipText(pApp->pIconActionWidget, "Icon action");

	xgeXuiWidgetSetRect(pApp->pTabsWidget, (xge_rect_t){ 56.0f, 150.0f, 320.0f, 28.0f });
	xgeXuiTabsInit(&pApp->tTabs, &pApp->tXui, pApp->pTabsWidget);
	xgeXuiTabsSetItems(&pApp->tTabs, g_arrIncubationTabs, 3);
	xgeXuiTabsSetFont(&pApp->tTabs, pFont);
	xgeXuiTabsSetTabSize(&pApp->tTabs, 92.0f, 28.0f);
	xgeXuiTabsSetSelect(&pApp->tTabs, AppTabsSelect, pApp);
	xgeXuiTabsSetSelected(&pApp->tTabs, 0);
	pApp->iTabSelected = 0;
	xgeXuiTabsSetColors(&pApp->tTabs, XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(38, 50, 66, 255), XGE_COLOR_RGBA(56, 72, 94, 255), XGE_COLOR_RGBA(72, 214, 128, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255), XGE_COLOR_RGBA(230, 236, 244, 255), XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pTabsWidget);

	xgeXuiWidgetSetRect(pApp->pProgressWidget, (xge_rect_t){ 56.0f, 190.0f, 320.0f, 30.0f });
	xgeXuiProgressInit(&pApp->tProgress, pApp->pProgressWidget);
	xgeXuiProgressSetRange(&pApp->tProgress, 0.0f, 1.0f);
	xgeXuiProgressSetText(&pApp->tProgress, pFont, "Progress");
	xgeXuiProgressSetTextColor(&pApp->tProgress, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiProgressSetColors(&pApp->tProgress, XGE_COLOR_RGBA(44, 54, 70, 255), XGE_COLOR_RGBA(72, 214, 128, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pProgressWidget);

	xgeXuiWidgetSetRect(pApp->pCheckBoxWidget, (xge_rect_t){ 56.0f, 244.0f, 180.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pCheckBoxWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiCheckBoxInit(&pApp->tCheckBox, &pApp->tXui, pApp->pCheckBoxWidget);
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, pFont, "CheckBox");
	xgeXuiCheckBoxSetChange(&pApp->tCheckBox, AppCheckBoxChange, pApp);
	xgeXuiCheckBoxSetTextColor(&pApp->tCheckBox, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiCheckBoxSetColors(&pApp->tCheckBox, XGE_COLOR_RGBA(184, 196, 214, 255), XGE_COLOR_RGBA(72, 214, 128, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pCheckBoxWidget);

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	xgeXuiRadioGroupSetChange(&pApp->tRadioGroup, AppRadioChange, pApp);
	xgeXuiWidgetSetRect(pApp->pRadioAWidget, (xge_rect_t){ 286.0f, 88.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pRadioAWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pRadioAWidget);
	xgeXuiRadioSetText(&pApp->tRadioA, pFont, "Radio A");
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 1);
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pRadioAWidget);

	xgeXuiWidgetSetRect(pApp->pRadioBWidget, (xge_rect_t){ 286.0f, 124.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pRadioBWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pRadioBWidget);
	xgeXuiRadioSetText(&pApp->tRadioB, pFont, "Radio B");
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 2);
	xgeXuiRadioGroupSetSelected(&pApp->tRadioGroup, 1);
	pApp->iRadioValue = 1;
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pRadioBWidget);

	xgeXuiWidgetSetRect(pApp->pToggleWidget, (xge_rect_t){ 286.0f, 162.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pToggleWidget, 3.0f, 3.0f, 3.0f, 3.0f);
	xgeXuiToggleInit(&pApp->tToggle, &pApp->tXui, pApp->pToggleWidget);
	xgeXuiToggleSetInnerText(&pApp->tToggle, pFont, "OFF", "ON");
	xgeXuiToggleSetChange(&pApp->tToggle, AppToggleChange, pApp);
	xgeXuiToggleSetInnerTextColor(&pApp->tToggle, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiToggleSetColors(&pApp->tToggle, XGE_COLOR_RGBA(104, 116, 132, 255), XGE_COLOR_RGBA(72, 214, 128, 255), XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pToggleWidget);

	xgeXuiWidgetSetRect(pApp->pComboWidget, (xge_rect_t){ 286.0f, 244.0f, 130.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pComboWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiComboBoxInit(&pApp->tCombo, &pApp->tXui, pApp->pComboWidget);
	xgeXuiComboBoxSetFont(&pApp->tCombo, pFont);
	xgeXuiComboBoxSetItems(&pApp->tCombo, g_arrIncubationCombo, 4);
	xgeXuiComboBoxSetSelected(&pApp->tCombo, 1);
	xgeXuiComboBoxSetDropDownHeight(&pApp->tCombo, 104.0f);
	xgeXuiComboBoxSetSelect(&pApp->tCombo, AppComboSelect, pApp);
	xgeXuiComboBoxSetColors(&pApp->tCombo, XGE_COLOR_RGBA(50, 66, 86, 255), XGE_COLOR_RGBA(66, 84, 106, 255), XGE_COLOR_RGBA(36, 50, 70, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255), XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(42, 50, 64, 255));
	pApp->iComboSelected = 1;
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pComboWidget);

	xgeXuiWidgetSetRect(pApp->pSeparatorWidget, (xge_rect_t){ 276.0f, 88.0f, 1.0f, 186.0f });
	xgeXuiSeparatorInit(&pApp->tSeparator, pApp->pSeparatorWidget);
	xgeXuiSeparatorSetOrientation(&pApp->tSeparator, XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiSeparatorSetColor(&pApp->tSeparator, XGE_COLOR_RGBA(92, 108, 128, 180));
	xgeXuiWidgetAdd(pApp->pPanelWidget, pApp->pSeparatorWidget);

	xgeXuiWidgetSetRect(pApp->pSplitterWidget, (xge_rect_t){ 482.0f, 32.0f, 14.0f, 300.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pSplitterWidget, 2.0f, 0.0f, 2.0f, 0.0f);
	xgeXuiSplitterInit(&pApp->tSplitter, &pApp->tXui, pApp->pSplitterWidget);
	xgeXuiSplitterSetRange(&pApp->tSplitter, -80.0f, 80.0f);
	xgeXuiSplitterSetChange(&pApp->tSplitter, AppSplitterChange, pApp);
	xgeXuiSplitterSetColors(&pApp->tSplitter, XGE_COLOR_RGBA(80, 96, 118, 255), XGE_COLOR_RGBA(112, 132, 160, 255), XGE_COLOR_RGBA(72, 214, 128, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pSplitterWidget);

	xgeXuiWidgetSetRect(pApp->pScrollBarWidget, (xge_rect_t){ 772.0f, 32.0f, 12.0f, 300.0f });
	xgeXuiScrollBarInit(&pApp->tScrollBar, &pApp->tXui, pApp->pScrollBarWidget);
	xgeXuiScrollBarSetRange(&pApp->tScrollBar, 0.0f, 100.0f, 25.0f);
	xgeXuiScrollBarSetChange(&pApp->tScrollBar, AppScrollBarChange, pApp);
	xgeXuiScrollBarSetColors(&pApp->tScrollBar, XGE_COLOR_RGBA(38, 46, 58, 255), XGE_COLOR_RGBA(172, 188, 210, 230), XGE_COLOR_RGBA(196, 214, 238, 245), XGE_COLOR_RGBA(72, 214, 128, 255), XGE_COLOR_RGBA(255, 218, 96, 255), XGE_COLOR_RGBA(70, 76, 84, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pScrollBarWidget);

	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 506.0f, 32.0f, 250.0f, 300.0f });
	xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScrollWidget);
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 260.0f, 420.0f);
	xgeXuiScrollViewSetColors(&pApp->tScroll, XGE_COLOR_RGBA(28, 34, 42, 255), XGE_COLOR_RGBA(70, 82, 96, 190), XGE_COLOR_RGBA(170, 184, 202, 230));
	xgeXuiWidgetAdd(pRoot, pApp->pScrollWidget);

	xgeXuiWidgetSetRect(pApp->pPopupWidget, (xge_rect_t){ 250.0f, 96.0f, 330.0f, 128.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPopupWidget, 14.0f, 14.0f, 14.0f, 14.0f);
	xgeXuiPopupInit(&pApp->tPopup, &pApp->tXui, pApp->pPopupWidget);
	xgeXuiPopupSetOwner(&pApp->tPopup, pApp->pButtonWidget);
	xgeXuiPopupSetClose(&pApp->tPopup, AppPopupClose, pApp);
	xgeXuiPopupSetBackground(&pApp->tPopup, XGE_COLOR_RGBA(44, 52, 68, 245));
	xgeXuiWidgetAdd(xgeXuiOverlayRoot(&pApp->tXui), pApp->pPopupWidget);

	xgeXuiWidgetSetRect(pApp->pPopupLabelWidget, (xge_rect_t){ 0.0f, 0.0f, 260.0f, 80.0f });
	xgeXuiLabelInit(&pApp->tPopupLabel, pApp->pPopupLabelWidget, pFont, "Popup overlay\nclick outside or ESC to close");
	xgeXuiLabelSetColor(&pApp->tPopupLabel, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiWidgetAdd(pApp->pPopupWidget, pApp->pPopupLabelWidget);

	return XGE_OK;
}

static int AppFrame(void* pUser)
{
	app_state_t* pApp;
	char sProgress[64];

	pApp = (app_state_t*)pUser;
	pApp->iFrameCount++;
	if ( xgeKeyDown(256) ) {
		if ( xgeXuiPopupIsOpen(&pApp->tPopup) == 0 ) {
			xgeQuit();
			return 1;
		}
	}
	AppDispatchMouse(pApp);

	pApp->fProgress += xgeGetDelta() * 0.25f;
	if ( pApp->fProgress > 1.0f ) {
		pApp->fProgress = 0.0f;
	}
	xgeXuiProgressSetValue(&pApp->tProgress, pApp->fProgress);
	snprintf(sProgress, sizeof(sProgress), "Progress %d%%", (int)(pApp->fProgress * 100.0f));
	xgeXuiProgressSetText(&pApp->tProgress, pApp->bFontReady ? &pApp->tFont : NULL, sProgress);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, pApp->fProgress * 180.0f);
	xgeXuiWidgetMarkPaint(xgeXuiRoot(&pApp->tXui));

	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(18, 22, 28, 255));
	xgeXuiUpdate(&pApp->tXui, xgeGetDelta());
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return 0;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	memset(&tApp, 0, sizeof(tApp));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 820;
	tDesc.iHeight = 440;
	tDesc.sTitle = "XGE XUI Incubation";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeXuiSetProcFrameEventDispatch(&tApp.tXui, 0);
	(void)AppLoadFont(&tApp);
	AppCreateIconTexture(&tApp);
	if ( AppCreateWidgetTree(&tApp) != XGE_OK ) {
		if ( tApp.bIconReady != 0 ) {
			xgeTextureFree(&tApp.tIconTexture);
		}
		if ( tApp.bFontReady != 0 ) {
			xgeFontFree(&tApp.tFont);
		}
		xgeXuiUnit(&tApp.tXui);
		xgeUnit();
		return 3;
	}
	xgeRun(AppFrame, &tApp);
	xgeXuiPanelUnit(&tApp.tPanel);
	xgeXuiButtonUnit(&tApp.tButton);
	xgeXuiButtonUnit(&tApp.tIconAction);
	xgeXuiTabsUnit(&tApp.tTabs);
	xgeXuiProgressUnit(&tApp.tProgress);
	xgeXuiCheckBoxUnit(&tApp.tCheckBox);
	xgeXuiRadioUnit(&tApp.tRadioA);
	xgeXuiRadioUnit(&tApp.tRadioB);
	xgeXuiRadioGroupUnit(&tApp.tRadioGroup);
	xgeXuiToggleUnit(&tApp.tToggle);
	xgeXuiComboBoxUnit(&tApp.tCombo);
	xgeXuiSeparatorUnit(&tApp.tSeparator);
	xgeXuiSplitterUnit(&tApp.tSplitter);
	xgeXuiScrollBarUnit(&tApp.tScrollBar);
	xgeXuiScrollViewUnit(&tApp.tScroll);
	xgeXuiPopupUnit(&tApp.tPopup);
	xgeXuiLabelUnit(&tApp.tPopupLabel);
	if ( tApp.bFontReady != 0 ) {
		xgeFontFree(&tApp.tFont);
	}
	if ( tApp.bIconReady != 0 ) {
		xgeTextureFree(&tApp.tIconTexture);
	}
	xgeXuiUnit(&tApp.tXui);
	printf("xui incubation summary: frames=%d progress=%.2f clicks=%d icon_clicks=%d tab=%d tab_changes=%d checkbox=%d checkbox_changes=%d radio=%d radio_changes=%d toggle=%d switch_changes=%d combo=%d combo_changes=%d splitter=%.1f splitter_changes=%d scrollbar=%.1f scrollbar_changes=%d popup_close=%d font=%d icon=%d\n", tApp.iFrameCount, tApp.fProgress, tApp.iClickCount, tApp.iIconClickCount, tApp.iTabSelected, tApp.iTabChangeCount, tApp.bCheckBoxChecked, tApp.iCheckBoxChangeCount, tApp.iRadioValue, tApp.iRadioChangeCount, tApp.bToggleChecked, tApp.iToggleChangeCount, tApp.iComboSelected, tApp.iComboChangeCount, tApp.fSplitterValue, tApp.iSplitterChangeCount, tApp.fScrollBarValue, tApp.iScrollBarChangeCount, tApp.iPopupCloseCount, tApp.bFontReady, tApp.bIconReady);
	xgeUnit();
	return 0;
}
