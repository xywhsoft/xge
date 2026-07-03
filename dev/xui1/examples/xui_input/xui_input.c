#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROW_COUNT 8

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pLabelWidget[ROW_COUNT];
	xge_xui_label_t tLabel[ROW_COUNT];
	xge_xui_widget pInputWidget[ROW_COUNT];
	xge_xui_input_t tInput[ROW_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bStateOK;
	int bDecorationOK;
	int iDecorClickCount;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/Deng.ttf",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_input font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static int AddRow(app_state_t* pApp, int iRow, const char* sLabel, const char* sText)
{
	float fY;

	fY = 44.0f + (float)iRow * 62.0f;
	pApp->pLabelWidget[iRow] = xgeXuiWidgetCreate();
	pApp->pInputWidget[iRow] = xgeXuiWidgetCreate();
	if ( (pApp->pLabelWidget[iRow] == NULL) || (pApp->pInputWidget[iRow] == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pLabelWidget[iRow], (xge_rect_t){ 32.0f, fY, 150.0f, 28.0f });
	xgeXuiWidgetSetRect(pApp->pInputWidget[iRow], (xge_rect_t){ 190.0f, fY, 520.0f, 32.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pInputWidget[iRow], 9.0f, 4.0f, 9.0f, 4.0f);
	xgeXuiWidgetAdd(xgeXuiRoot(&pApp->tXui), pApp->pLabelWidget[iRow]);
	xgeXuiWidgetAdd(xgeXuiRoot(&pApp->tXui), pApp->pInputWidget[iRow]);
	if ( xgeXuiLabelInit(&pApp->tLabel[iRow], pApp->pLabelWidget[iRow], AppFont(pApp), sLabel) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iRow], XGE_COLOR_RGBA(66, 78, 94, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iRow], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	if ( xgeXuiInputInit(&pApp->tInput[iRow], &pApp->tXui, pApp->pInputWidget[iRow], AppFont(pApp)) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiInputSetText(&pApp->tInput[iRow], sText);
	return XGE_OK;
}

static void PasswordEyeClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		xgeXuiInputSetPassword(&pApp->tInput[2], pApp->tInput[2].bPassword == 0);
	}
}

static void DecorActionClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iDecorClickCount++;
		xgeXuiInputSetText(&pApp->tInput[6], "decoration clicked");
	}
}

static void InputErrorChange(xge_xui_widget pWidget, int bError, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp == NULL ) {
		return;
	}
	xgeXuiLabelSetText(&pApp->tLabel[3], bError ? "Invalid value" : "Error event");
	xgeXuiLabelSetColor(&pApp->tLabel[3], bError ? XGE_COLOR_RGBA(190, 54, 66, 255) : XGE_COLOR_RGBA(66, 78, 94, 255));
}

static xge_xui_input_decoration AddInputDecoration(xge_xui_input pInput, int iSide, int iKind, int iIcon, const char* sText, float fWidth, xge_xui_click_proc procClick, void* pUser)
{
	xge_xui_input_decoration_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = iKind;
	tDesc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_ALWAYS;
	tDesc.fWidth = fWidth;
	tDesc.fPadding = 4.0f;
	tDesc.iIcon = iIcon;
	tDesc.sText = sText;
	tDesc.iColor = XGE_COLOR_RGBA(68, 126, 166, 255);
	tDesc.iHoverColor = XGE_COLOR_RGBA(38, 108, 178, 255);
	tDesc.iActiveColor = XGE_COLOR_RGBA(24, 86, 150, 255);
	tDesc.iDisabledColor = XGE_COLOR_RGBA(132, 142, 156, 255);
	tDesc.procClick = procClick;
	tDesc.pUser = pUser;
	return xgeXuiInputDecorationAdd(pInput, iSide, &tDesc);
}

static xge_xui_input_decoration AddClearDecoration(xge_xui_input pInput)
{
	xge_xui_input_decoration_desc_t tDesc;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iKind = XGE_XUI_INPUT_DECORATION_CLEAR;
	tDesc.iVisibleMode = XGE_XUI_INPUT_DECORATION_VISIBLE_WHEN_NOT_EMPTY;
	tDesc.fWidth = 22.0f;
	tDesc.fPadding = 3.0f;
	tDesc.iColor = XGE_COLOR_RGBA(96, 126, 148, 255);
	tDesc.iHoverColor = XGE_COLOR_RGBA(72, 102, 124, 255);
	tDesc.iActiveColor = XGE_COLOR_RGBA(72, 102, 124, 255);
	tDesc.iDisabledColor = XGE_COLOR_RGBA(132, 142, 156, 255);
	return xgeXuiInputDecorationAdd(pInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, &tDesc);
}

static int HasDecorationRect(xge_xui_input_decoration pDecoration)
{
	return (pDecoration != NULL) && (pDecoration->tRect.fW > 0.0f) && (pDecoration->tRect.fH > 0.0f);
}

static int CheckInputDecorations(app_state_t* pApp)
{
	xge_xui_input_decoration pIconTrailing;
	xge_xui_input_decoration pClearTrailing;

	if ( pApp == NULL ) {
		return 0;
	}
	pIconTrailing = pApp->tInput[5].pTrailingDecoration;
	pClearTrailing = (pIconTrailing != NULL) ? pIconTrailing->pNext : NULL;
	if ( (pApp->tInput[2].fTrailingDecorationWidth < 20.0f) || (!HasDecorationRect(pApp->tInput[2].pTrailingDecoration)) ) {
		return 0;
	}
	if ( (pApp->tInput[5].fLeadingDecorationWidth < 20.0f) || (!HasDecorationRect(pApp->tInput[5].pLeadingDecoration)) ) {
		return 0;
	}
	if ( (pApp->tInput[5].fTrailingDecorationWidth < 44.0f) || (!HasDecorationRect(pIconTrailing)) || (!HasDecorationRect(pClearTrailing)) ) {
		return 0;
	}
	if ( (pApp->tInput[6].fTrailingDecorationWidth < 28.0f) || (!HasDecorationRect(pApp->tInput[6].pTrailingDecoration)) ) {
		return 0;
	}
	if ( (pApp->tInput[7].fTrailingDecorationWidth < 26.0f) || (!HasDecorationRect(pApp->tInput[7].pTrailingDecoration)) ) {
		return 0;
	}
	return 1;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 760.0f, 560.0f });
	if ( AddRow(pApp, 0, "Default", "Input text") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 1, "Placeholder", "") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 2, "Password", "secret") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 3, "Error", "invalid value") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 4, "Disabled", "disabled input") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 5, "Icons", "search keyword") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 6, "Decoration event", "click GO") != XGE_OK ) return XGE_ERROR;
	if ( AddRow(pApp, 7, "Right + unit", "128") != XGE_OK ) return XGE_ERROR;

	xgeXuiInputSetPlaceholder(&pApp->tInput[1], "Placeholder text");
	xgeXuiInputSetPassword(&pApp->tInput[2], 1);
	AddInputDecoration(&pApp->tInput[2], XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_ICON, XGE_XUI_INPUT_ICON_EYE, NULL, 24.0f, PasswordEyeClick, pApp);
	xgeXuiInputSetErrorChange(&pApp->tInput[3], InputErrorChange, pApp);
	xgeXuiInputSetError(&pApp->tInput[3], 1);
	xgeXuiInputSetDisabled(&pApp->tInput[4], 1);
	AddInputDecoration(&pApp->tInput[5], XGE_XUI_INPUT_DECORATION_SIDE_LEADING, XGE_XUI_INPUT_DECORATION_ICON, XGE_XUI_INPUT_ICON_SEARCH, NULL, 24.0f, NULL, NULL);
	AddInputDecoration(&pApp->tInput[5], XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_ICON, XGE_XUI_INPUT_ICON_LOCK, NULL, 24.0f, NULL, NULL);
	AddClearDecoration(&pApp->tInput[5]);
	AddInputDecoration(&pApp->tInput[6], XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_TEXT, XGE_XUI_INPUT_ICON_NONE, "GO", 32.0f, DecorActionClick, pApp);
	xgeXuiInputSetTextAlign(&pApp->tInput[7], XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);
	AddInputDecoration(&pApp->tInput[7], XGE_XUI_INPUT_DECORATION_SIDE_TRAILING, XGE_XUI_INPUT_DECORATION_TEXT, XGE_XUI_INPUT_ICON_NONE, "px", 30.0f, NULL, NULL);
	xgeXuiInputSetFrameColors(&pApp->tInput[7], XGE_COLOR_RGBA(255, 252, 244, 255), XGE_COLOR_RGBA(255, 248, 232, 255), XGE_COLOR_RGBA(215, 145, 66, 255), XGE_COLOR_RGBA(230, 126, 34, 255), XGE_COLOR_RGBA(209, 91, 22, 255));
	xgeXuiInputSetColors(&pApp->tInput[7], XGE_COLOR_RGBA(80, 50, 30, 255), XGE_COLOR_RGBA(255, 252, 244, 255), XGE_COLOR_RGBA(209, 91, 22, 255), XGE_COLOR_RGBA(80, 50, 30, 255));
	xgeXuiSetFocus(&pApp->tXui, pApp->pInputWidget[0]);

	pApp->bCreateOK = 1;
	pApp->bStateOK = 1;
	for ( i = 0; i < ROW_COUNT; i++ ) {
		pApp->bStateOK = pApp->bStateOK && (pApp->tInput[i].pWidget == pApp->pInputWidget[i]);
	}
	pApp->bStateOK = pApp->bStateOK && (pApp->tInput[3].bError != 0) && (pApp->tInput[4].bDisabled != 0) && (pApp->tInput[5].pLeadingDecoration != NULL) && (pApp->tInput[5].pTrailingDecoration != NULL) && (pApp->tInput[7].iTextAlign == XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);
	return XGE_OK;
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < ROW_COUNT; i++ ) {
		xgeXuiInputUnit(&pApp->tInput[i]);
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
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
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->bDecorationOK = CheckInputDecorations(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_input final-summary frames=%d create=%d state=%d decoration=%d inputs=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bStateOK, pApp->bDecorationOK, ROW_COUNT);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(229, 235, 244, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_INPUT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI Input";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bStateOK && tApp.bDecorationOK) ? 0 : 3;
}

