#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pNormalLabel;
	xge_xui_widget pNormalInput;
	xge_xui_widget pPasswordLabel;
	xge_xui_widget pPasswordInput;
	xge_xui_widget pReadonlyLabel;
	xge_xui_widget pReadonlyInput;
	xge_xui_widget pDisabledLabel;
	xge_xui_widget pDisabledInput;
	xge_xui_widget pHintLabel;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tNormalLabel;
	xge_xui_label_t tPasswordLabel;
	xge_xui_label_t tReadonlyLabel;
	xge_xui_label_t tDisabledLabel;
	xge_xui_label_t tHintLabel;
	xge_xui_input_t tNormalInput;
	xge_xui_input_t tPasswordInput;
	xge_xui_input_t tReadonlyInput;
	xge_xui_input_t tDisabledInput;
	int bFontReady;
	int iFrameCount;
} app_state_t;

static int AppLoadFont(app_state_t* pApp)
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
			printf("input font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("input font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget AppWidget(void)
{
	return xgeXuiWidgetCreate();
}

static int AppCreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	xge_xui_theme_t tTheme;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(240, 245, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(36, 46, 62, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(46, 66, 92, 255);
	tTheme.iStateDisabled = XGE_COLOR_RGBA(36, 40, 48, 255);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);

	pApp->pPanel = AppWidget();
	pApp->pNormalLabel = AppWidget();
	pApp->pNormalInput = AppWidget();
	pApp->pPasswordLabel = AppWidget();
	pApp->pPasswordInput = AppWidget();
	pApp->pReadonlyLabel = AppWidget();
	pApp->pReadonlyInput = AppWidget();
	pApp->pDisabledLabel = AppWidget();
	pApp->pDisabledInput = AppWidget();
	pApp->pHintLabel = AppWidget();
	if ( (pRoot == NULL) || (pApp->pPanel == NULL) || (pApp->pNormalLabel == NULL) || (pApp->pNormalInput == NULL) || (pApp->pPasswordLabel == NULL) || (pApp->pPasswordInput == NULL) || (pApp->pReadonlyLabel == NULL) || (pApp->pReadonlyInput == NULL) || (pApp->pDisabledLabel == NULL) || (pApp->pDisabledInput == NULL) || (pApp->pHintLabel == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 32.0f, 28.0f, 520.0f, 292.0f });
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "XUI Input Validation");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(28, 36, 50, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pNormalLabel, (xge_rect_t){ 24.0f, 54.0f, 126.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tNormalLabel, pApp->pNormalLabel, pFont, "normal");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pNormalLabel);
	xgeXuiWidgetSetRect(pApp->pNormalInput, (xge_rect_t){ 150.0f, 50.0f, 320.0f, 34.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pNormalInput, 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiInputInit(&pApp->tNormalInput, &pApp->tXui, pApp->pNormalInput, pFont);
	xgeXuiInputSetPlaceholder(&pApp->tNormalInput, "type English or Chinese IME here");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pNormalInput);

	xgeXuiWidgetSetRect(pApp->pPasswordLabel, (xge_rect_t){ 24.0f, 98.0f, 126.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tPasswordLabel, pApp->pPasswordLabel, pFont, "password");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pPasswordLabel);
	xgeXuiWidgetSetRect(pApp->pPasswordInput, (xge_rect_t){ 150.0f, 94.0f, 320.0f, 34.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPasswordInput, 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiInputInit(&pApp->tPasswordInput, &pApp->tXui, pApp->pPasswordInput, pFont);
	xgeXuiInputSetPlaceholder(&pApp->tPasswordInput, "password mask");
	xgeXuiInputSetPassword(&pApp->tPasswordInput, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pPasswordInput);

	xgeXuiWidgetSetRect(pApp->pReadonlyLabel, (xge_rect_t){ 24.0f, 142.0f, 126.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tReadonlyLabel, pApp->pReadonlyLabel, pFont, "readonly");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pReadonlyLabel);
	xgeXuiWidgetSetRect(pApp->pReadonlyInput, (xge_rect_t){ 150.0f, 138.0f, 320.0f, 34.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pReadonlyInput, 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiInputInit(&pApp->tReadonlyInput, &pApp->tXui, pApp->pReadonlyInput, pFont);
	xgeXuiInputSetText(&pApp->tReadonlyInput, "selectable but readonly");
	xgeXuiInputSetReadonly(&pApp->tReadonlyInput, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pReadonlyInput);

	xgeXuiWidgetSetRect(pApp->pDisabledLabel, (xge_rect_t){ 24.0f, 186.0f, 126.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tDisabledLabel, pApp->pDisabledLabel, pFont, "disabled");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pDisabledLabel);
	xgeXuiWidgetSetRect(pApp->pDisabledInput, (xge_rect_t){ 150.0f, 182.0f, 320.0f, 34.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pDisabledInput, 8.0f, 5.0f, 8.0f, 5.0f);
	xgeXuiInputInit(&pApp->tDisabledInput, &pApp->tXui, pApp->pDisabledInput, pFont);
	xgeXuiInputSetText(&pApp->tDisabledInput, "disabled input");
	xgeXuiInputSetDisabled(&pApp->tDisabledInput, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pDisabledInput);

	xgeXuiWidgetSetRect(pApp->pHintLabel, (xge_rect_t){ 24.0f, 236.0f, 456.0f, 36.0f });
	xgeXuiLabelInit(&pApp->tHintLabel, pApp->pHintLabel, pFont, "Tab focus | Ctrl+A/C/X/V | Home/End | Ctrl+Left/Right | double click word | ESC quit");
	xgeXuiLabelSetColor(&pApp->tHintLabel, XGE_COLOR_RGBA(184, 198, 216, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pHintLabel);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	(void)AppLoadFont(pApp);
	return AppCreateUI(pApp);
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiInputUnit(&pApp->tNormalInput);
	xgeXuiInputUnit(&pApp->tPasswordInput);
	xgeXuiInputUnit(&pApp->tReadonlyInput);
	xgeXuiInputUnit(&pApp->tDisabledInput);
	xgeXuiLabelUnit(&pApp->tNormalLabel);
	xgeXuiLabelUnit(&pApp->tPasswordLabel);
	xgeXuiLabelUnit(&pApp->tReadonlyLabel);
	xgeXuiLabelUnit(&pApp->tDisabledLabel);
	xgeXuiLabelUnit(&pApp->tHintLabel);
	xgeXuiPanelUnit(&pApp->tPanel);
	if ( pApp->bFontReady != 0 ) {
		xgeFontFree(&pApp->tFont);
	}
	xgeXuiUnit(&pApp->tXui);
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( xgeXuiDispatchEvent(&pApp->tXui, pEvent) == XGE_XUI_EVENT_CONSUMED ) {
		return XGE_OK;
	}
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeQuit();
		return 1;
	}
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->iFrameCount++;
	return xgeXuiUpdate(&pApp->tXui, fDelta);
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(16, 21, 29, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

static void AppInit(app_state_t* pApp)
{
	memset(pApp, 0, sizeof(*pApp));
	pApp->tScene.pUser = pApp;
	pApp->tScene.onEnter = AppEnter;
	pApp->tScene.onLeave = AppLeave;
	pApp->tScene.onEvent = AppEvent;
	pApp->tScene.onUpdate = AppUpdate;
	pApp->tScene.onDraw = AppDraw;
}

int main(int argc, char** argv)
{
	app_state_t tApp;
	xge_desc_t tDesc;

	(void)argc;
	(void)argv;
	AppInit(&tApp);
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 640;
	tDesc.iHeight = 360;
	tDesc.sTitle = "XGE XUI Input Validation";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	xgeRun(NULL, NULL);
	printf("xui input summary: frames=%d normal=\"%s\" normal_cursor=%d normal_select=%d,%d normal_scroll=%.1f password_len=%d readonly=\"%s\" readonly_cursor=%d disabled=\"%s\" font=%d\n", tApp.iFrameCount, xgeXuiInputGetText(&tApp.tNormalInput), tApp.tNormalInput.tText.iCursor, tApp.tNormalInput.tText.iSelectStart, tApp.tNormalInput.tText.iSelectEnd, tApp.tNormalInput.fScrollX, (int)strlen(xgeXuiInputGetText(&tApp.tPasswordInput)), xgeXuiInputGetText(&tApp.tReadonlyInput), tApp.tReadonlyInput.tText.iCursor, xgeXuiInputGetText(&tApp.tDisabledInput), tApp.bFontReady);
	xgeUnit();
	return 0;
}
