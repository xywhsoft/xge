#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pPanel;
	xge_xui_widget pEditWidget;
	xge_xui_widget pHintWidget;
	xge_xui_panel_t tPanel;
	xge_xui_text_edit_t tEdit;
	xge_xui_label_t tHint;
	int bFontReady;
	int bWordWrap;
	int iFrameCount;
} app_state_t;

static int AppLoadFont(app_state_t* pApp)
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
			printf("textedit font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("textedit font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static int AppCreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	xge_xui_theme_t tTheme;
	const char* sInitialText;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	xgeXuiThemeDefault(&tTheme);
	tTheme.pFont = pFont;
	tTheme.iTextColor = XGE_COLOR_RGBA(238, 244, 250, 255);
	tTheme.iBackgroundColor = XGE_COLOR_RGBA(30, 38, 52, 255);
	tTheme.iStateFocus = XGE_COLOR_RGBA(38, 52, 72, 255);
	xgeXuiSetTheme(&pApp->tXui, &tTheme);
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);

	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pEditWidget = xgeXuiWidgetCreate();
	pApp->pHintWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pApp->pPanel == NULL) || (pApp->pEditWidget == NULL) || (pApp->pHintWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}

	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 28.0f, 24.0f, 584.0f, 312.0f });
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "XUI TextEdit Validation");
	xgeXuiPanelSetBackground(&pApp->tPanel, XGE_COLOR_RGBA(24, 32, 46, 255));
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	xgeXuiWidgetSetRect(pApp->pEditWidget, (xge_rect_t){ 22.0f, 48.0f, 540.0f, 214.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pEditWidget, 8.0f, 6.0f, 8.0f, 6.0f);
	xgeXuiTextEditInit(&pApp->tEdit, &pApp->tXui, pApp->pEditWidget, pFont);
	xgeXuiTextEditSetColors(&pApp->tEdit, XGE_COLOR_RGBA(248, 250, 252, 255), XGE_COLOR_RGBA(28, 36, 48, 255), XGE_COLOR_RGBA(36, 50, 70, 255), XGE_COLOR_RGBA(255, 230, 130, 255));
	sInitialText =
		"XGE TextEdit\n"
		"Type English or Chinese IME here.\n"
		"Enter creates new lines.\n"
		"Use arrows, Home/End, Ctrl+Home/End, PageUp/PageDown.\n"
		"Use Ctrl+A/C/X/V for clipboard, Ctrl+Z/Y for undo/redo, Ctrl+W to toggle wrap.\n"
		"\n"
		"Long line: abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz abcdefghijklmnopqrstuvwxyz.\n"
		"Line 8\n"
		"Line 9\n"
		"Line 10\n"
		"Line 11\n"
		"Line 12";
	xgeXuiTextEditSetText(&pApp->tEdit, sInitialText);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pEditWidget);

	xgeXuiWidgetSetRect(pApp->pHintWidget, (xge_rect_t){ 22.0f, 270.0f, 540.0f, 28.0f });
	xgeXuiLabelInit(&pApp->tHint, pApp->pHintWidget, pFont, "Click text area | drag select | Ctrl+Z/Y undo/redo | Ctrl+W wrap | ESC quit");
	xgeXuiLabelSetColor(&pApp->tHint, XGE_COLOR_RGBA(176, 192, 212, 255));
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pHintWidget);
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
	xgeXuiTextEditUnit(&pApp->tEdit);
	xgeXuiLabelUnit(&pApp->tHint);
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
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_KEY_DOWN) && ((pEvent->iParam2 & XGE_KEY_MOD_CTRL) != 0) && (pEvent->iParam1 == 'W' || pEvent->iParam1 == 'w') ) {
		pApp->bWordWrap = (pApp->bWordWrap == 0);
		xgeXuiTextEditSetWordWrap(&pApp->tEdit, pApp->bWordWrap);
		return XGE_OK;
	}
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
	xgeClear(XGE_COLOR_RGBA(15, 20, 28, 255));
	xgeXuiWidgetMarkPaint(xgeXuiRoot(&pApp->tXui));
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
	tDesc.sTitle = "XGE XUI TextEdit Validation";
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
	printf("xui textedit summary: frames=%d text_len=%d cursor=%d select=%d,%d scroll=%.1f,%.1f lines=%d visual=%d undo=%d redo=%d font=%d wrap=%d\n", tApp.iFrameCount, (int)strlen(xgeXuiTextEditGetText(&tApp.tEdit)), tApp.tEdit.tText.iCursor, tApp.tEdit.tText.iSelectStart, tApp.tEdit.tText.iSelectEnd, tApp.tEdit.fScrollX, tApp.tEdit.fScrollY, tApp.tEdit.iLineCount, tApp.tEdit.iVisualLineCount, tApp.tEdit.iUndoCount, tApp.tEdit.iRedoCount, tApp.bFontReady, tApp.bWordWrap);
	xgeUnit();
	return 0;
}
