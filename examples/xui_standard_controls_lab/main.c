#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pStatus;
	xge_xui_widget pButton;
	xge_xui_widget pCheckBox;
	xge_xui_widget pRadioA;
	xge_xui_widget pRadioB;
	xge_xui_widget pSwitch;
	xge_xui_widget pSlider;
	xge_xui_widget pProgress;
	xge_xui_widget pNumeric;
	xge_xui_widget pToolbar;
	xge_xui_widget pStatusBar;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tStatus;
	xge_xui_button_t tButton;
	xge_xui_checkbox_t tCheckBox;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_switch_t tSwitch;
	xge_xui_slider_t tSlider;
	xge_xui_progress_t tProgress;
	xge_xui_numeric_input_t tNumeric;
	xge_xui_toolbar_t tToolbar;
	xge_xui_status_bar_t tStatusBar;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bBasicOK;
	int bChoiceOK;
	int bValueOK;
	int bToolbarOK;
	int bStatusOK;
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
			printf("xui-standard-controls-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-standard-controls-lab font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(void)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetPaddingPx(pWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	}
	return pWidget;
}

static void ClickProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}

static void ToggleProc(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	(void)pWidget;
	(void)bChecked;
	(void)pUser;
}

static void SelectProc(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)iIndex;
	(void)pUser;
}

static void SliderProc(xge_xui_widget pWidget, float fValue, void* pUser)
{
	(void)pWidget;
	(void)fValue;
	(void)pUser;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	const char* arrToolText[] = { "New", "Pin", "", "Run" };
	int arrToolType[] = { XGE_XUI_TOOLBAR_ITEM_BUTTON, XGE_XUI_TOOLBAR_ITEM_TOGGLE, XGE_XUI_TOOLBAR_ITEM_SEPARATOR, XGE_XUI_TOOLBAR_ITEM_BUTTON };
	int iStatusProgress;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = NewWidget();
	pApp->pStatus = NewWidget();
	pApp->pButton = NewWidget();
	pApp->pCheckBox = NewWidget();
	pApp->pRadioA = NewWidget();
	pApp->pRadioB = NewWidget();
	pApp->pSwitch = NewWidget();
	pApp->pSlider = NewWidget();
	pApp->pProgress = NewWidget();
	pApp->pNumeric = NewWidget();
	pApp->pToolbar = NewWidget();
	pApp->pStatusBar = NewWidget();
	if ( (pApp->pPanel == NULL) || (pApp->pStatus == NULL) || (pApp->pButton == NULL) || (pApp->pCheckBox == NULL) ||
		(pApp->pRadioA == NULL) || (pApp->pRadioB == NULL) || (pApp->pSwitch == NULL) || (pApp->pSlider == NULL) ||
		(pApp->pProgress == NULL) || (pApp->pNumeric == NULL) || (pApp->pToolbar == NULL) || (pApp->pStatusBar == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 22.0f, 20.0f, 620.0f, 338.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "XUI Standard Controls");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pStatus, (xge_rect_t){ 18.0f, 36.0f, 560.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tStatus, pApp->pStatus, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatus);

	xgeXuiWidgetSetRect(pApp->pButton, (xge_rect_t){ 22.0f, 78.0f, 128.0f, 30.0f });
	xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButton);
	xgeXuiButtonSetText(&pApp->tButton, pFont, "Primary");
	xgeXuiButtonSetClick(&pApp->tButton, ClickProc, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pButton);

	xgeXuiWidgetSetRect(pApp->pCheckBox, (xge_rect_t){ 22.0f, 124.0f, 170.0f, 26.0f });
	xgeXuiCheckBoxInit(&pApp->tCheckBox, &pApp->tXui, pApp->pCheckBox);
	xgeXuiCheckBoxSetText(&pApp->tCheckBox, pFont, "Grid snapping");
	xgeXuiCheckBoxSetChecked(&pApp->tCheckBox, 1);
	xgeXuiCheckBoxSetChange(&pApp->tCheckBox, ToggleProc, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pCheckBox);

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	xgeXuiRadioGroupSetChange(&pApp->tRadioGroup, SelectProc, pApp);
	xgeXuiWidgetSetRect(pApp->pRadioA, (xge_rect_t){ 22.0f, 164.0f, 150.0f, 24.0f });
	xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pRadioA);
	xgeXuiRadioSetText(&pApp->tRadioA, pFont, "Compact");
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pRadioA);
	xgeXuiWidgetSetRect(pApp->pRadioB, (xge_rect_t){ 22.0f, 198.0f, 150.0f, 24.0f });
	xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pRadioB);
	xgeXuiRadioSetText(&pApp->tRadioB, pFont, "Detailed");
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 2);
	xgeXuiRadioGroupSetSelected(&pApp->tRadioGroup, 2);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pRadioB);

	xgeXuiWidgetSetRect(pApp->pSwitch, (xge_rect_t){ 22.0f, 236.0f, 170.0f, 28.0f });
	xgeXuiSwitchInit(&pApp->tSwitch, &pApp->tXui, pApp->pSwitch);
	xgeXuiSwitchSetText(&pApp->tSwitch, pFont, "Live preview");
	xgeXuiSwitchSetChecked(&pApp->tSwitch, 1);
	xgeXuiSwitchSetChange(&pApp->tSwitch, ToggleProc, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSwitch);

	xgeXuiWidgetSetRect(pApp->pSlider, (xge_rect_t){ 238.0f, 82.0f, 220.0f, 26.0f });
	xgeXuiSliderInit(&pApp->tSlider, &pApp->tXui, pApp->pSlider);
	xgeXuiSliderSetRange(&pApp->tSlider, 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tSlider, 64.0f);
	xgeXuiSliderSetChange(&pApp->tSlider, SliderProc, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSlider);

	xgeXuiWidgetSetRect(pApp->pProgress, (xge_rect_t){ 238.0f, 126.0f, 220.0f, 24.0f });
	xgeXuiProgressInit(&pApp->tProgress, pApp->pProgress);
	xgeXuiProgressSetRange(&pApp->tProgress, 0.0f, 100.0f);
	xgeXuiProgressSetValue(&pApp->tProgress, 72.0f);
	xgeXuiProgressSetText(&pApp->tProgress, pFont, "72%");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pProgress);

	xgeXuiWidgetSetRect(pApp->pNumeric, (xge_rect_t){ 238.0f, 174.0f, 150.0f, 30.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pNumeric, 8.0f, 5.0f, 24.0f, 5.0f);
	xgeXuiNumericInputInit(&pApp->tNumeric, &pApp->tXui, pApp->pNumeric, pFont);
	xgeXuiNumericInputSetRange(&pApp->tNumeric, 0.0f, 10.0f);
	xgeXuiNumericInputSetStep(&pApp->tNumeric, 0.5f);
	xgeXuiNumericInputSetValue(&pApp->tNumeric, 4.5f);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pNumeric);

	xgeXuiWidgetSetRect(pApp->pToolbar, (xge_rect_t){ 238.0f, 226.0f, 260.0f, 30.0f });
	xgeXuiToolbarInit(&pApp->tToolbar, &pApp->tXui, pApp->pToolbar);
	xgeXuiToolbarSetFont(&pApp->tToolbar, pFont);
	xgeXuiToolbarSetItems(&pApp->tToolbar, arrToolText, arrToolType, 4);
	xgeXuiToolbarSetItemSize(&pApp->tToolbar, 58.0f, 26.0f, 10.0f);
	xgeXuiToolbarSetItemChecked(&pApp->tToolbar, 1, 1);
	xgeXuiToolbarSetItemEnabled(&pApp->tToolbar, 3, 0);
	xgeXuiToolbarSetSelect(&pApp->tToolbar, SelectProc, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pToolbar);

	xgeXuiWidgetSetRect(pApp->pStatusBar, (xge_rect_t){ 18.0f, 288.0f, 560.0f, 26.0f });
	xgeXuiStatusBarInit(&pApp->tStatusBar, &pApp->tXui, pApp->pStatusBar);
	xgeXuiStatusBarSetFont(&pApp->tStatusBar, pFont);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, "Ready", 70.0f, 1);
	iStatusProgress = xgeXuiStatusBarAddProgress(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, 0.0f, 100.0f, 58.0f, 120.0f);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_CENTER, "Standard controls", 150.0f, 0);
	xgeXuiStatusBarAddText(&pApp->tStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, "TechBlue", 88.0f, 0);
	xgeXuiStatusBarSetProgress(&pApp->tStatusBar, iStatusProgress, 58.0f);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pStatusBar);

	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bBasicOK = (pApp->tButton.pWidget == pApp->pButton) && (pApp->tPanel.pWidget == pApp->pPanel);
	pApp->bChoiceOK = (xgeXuiCheckBoxGetChecked(&pApp->tCheckBox) == 1) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 2) &&
		(xgeXuiSwitchGetChecked(&pApp->tSwitch) == 1);
	pApp->bValueOK = (pApp->tSlider.fValue == 64.0f) &&
		(pApp->tProgress.fValue == 72.0f) &&
		(xgeXuiNumericInputGetValue(&pApp->tNumeric) == 4.5f);
	pApp->bToolbarOK = (pApp->tToolbar.iItemCount == 4) && (xgeXuiToolbarGetItemChecked(&pApp->tToolbar, 1) == 1);
	pApp->bStatusOK = pApp->tStatusBar.iItemCount == 4;
}

static void UpdateStatus(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"basic=%d choice=%d value=%d toolbar=%d status=%d",
		pApp->bBasicOK,
		pApp->bChoiceOK,
		pApp->bValueOK,
		pApp->bToolbarOK,
		pApp->bStatusOK);
	xgeXuiLabelSetText(&pApp->tStatus, sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	UpdateStatus(pApp);
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
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	UpdateStatus(pApp);
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
			"xui-standard-controls-lab final-summary frames=%d basic=%d choice=%d value=%d toolbar=%d status=%d\n",
			pApp->iFrameCount,
			pApp->bBasicOK,
			pApp->bChoiceOK,
			pApp->bValueOK,
			pApp->bToolbarOK,
			pApp->bStatusOK);
		printf("xui-standard-controls-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(238, 248, 255, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_STANDARD_CONTROLS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 670;
	tDesc.iHeight = 390;
	tDesc.sTitle = "XGE XUI Standard Controls Lab";
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
	return (iExitCode == XGE_OK && tApp.bBasicOK && tApp.bChoiceOK && tApp.bValueOK && tApp.bToolbarOK && tApp.bStatusOK) ? 0 : 3;
}
