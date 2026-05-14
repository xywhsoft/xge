#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tSummary;
	xge_xui_label_t tRgbLabel;
	xge_xui_label_t tAlphaLabel;
	xge_xui_color_picker_t tRgbPicker;
	xge_xui_color_picker_t tAlphaPicker;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pRgbLabel;
	xge_xui_widget pAlphaLabel;
	xge_xui_widget pRgbPickerWidget;
	xge_xui_widget pAlphaPickerWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bHexOK;
	int bPaletteOK;
	int bAlphaOK;
	int iChangeCount;
	uint32_t iLastColor;
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
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_colorpicker font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_colorpicker font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void ColorChanged(xge_xui_widget pWidget, uint32_t iColor, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
		pApp->iLastColor = iColor;
	}
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	uint32_t arrPalette[11];
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pRgbLabel = xgeXuiWidgetCreate();
	pApp->pAlphaLabel = xgeXuiWidgetCreate();
	pApp->pRgbPickerWidget = xgeXuiWidgetCreate();
	pApp->pAlphaPickerWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pRgbLabel == NULL) || (pApp->pAlphaLabel == NULL) || (pApp->pRgbPickerWidget == NULL) || (pApp->pAlphaPickerWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 632.0f, 250.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "ColorPicker");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 310.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pRgbLabel, (xge_rect_t){ 18.0f, 78.0f, 90.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tRgbLabel, pApp->pRgbLabel, pFont, "RGB");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pRgbLabel);
	xgeXuiWidgetSetRect(pApp->pRgbPickerWidget, (xge_rect_t){ 112.0f, 76.0f, 230.0f, 30.0f });
	xgeXuiColorPickerInit(&pApp->tRgbPicker, &pApp->tXui, pApp->pRgbPickerWidget, pFont);
	xgeXuiColorPickerSetChange(&pApp->tRgbPicker, ColorChanged, pApp);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pRgbPickerWidget);
	xgeXuiWidgetSetRect(pApp->pAlphaLabel, (xge_rect_t){ 18.0f, 124.0f, 90.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tAlphaLabel, pApp->pAlphaLabel, pFont, "RGBA");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pAlphaLabel);
	xgeXuiWidgetSetRect(pApp->pAlphaPickerWidget, (xge_rect_t){ 112.0f, 122.0f, 230.0f, 30.0f });
	xgeXuiColorPickerInit(&pApp->tAlphaPicker, &pApp->tXui, pApp->pAlphaPickerWidget, pFont);
	xgeXuiColorPickerSetChange(&pApp->tAlphaPicker, ColorChanged, pApp);
	xgeXuiColorPickerSetAlphaEnabled(&pApp->tAlphaPicker, 1);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pAlphaPickerWidget);
	arrPalette[0] = XGE_COLOR_RGBA(0, 0, 0, 0);
	arrPalette[1] = XGE_COLOR_RGBA(0, 0, 0, 255);
	arrPalette[2] = XGE_COLOR_RGBA(255, 255, 255, 255);
	arrPalette[3] = XGE_COLOR_RGBA(148, 158, 168, 255);
	arrPalette[4] = XGE_COLOR_RGBA(225, 58, 70, 255);
	arrPalette[5] = XGE_COLOR_RGBA(230, 126, 34, 255);
	arrPalette[6] = XGE_COLOR_RGBA(244, 201, 54, 255);
	arrPalette[7] = XGE_COLOR_RGBA(74, 165, 91, 255);
	arrPalette[8] = XGE_COLOR_RGBA(43, 184, 203, 255);
	arrPalette[9] = XGE_COLOR_RGBA(46, 124, 214, 255);
	arrPalette[10] = XGE_COLOR_RGBA(132, 86, 209, 255);
	for ( i = 0; i < 2; i++ ) {
		xgeXuiColorPickerSetPalette((i == 0) ? &pApp->tRgbPicker : &pApp->tAlphaPicker, arrPalette, 11);
	}
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	uint32_t iTarget;

	xgeXuiColorPickerSetHex(&pApp->tRgbPicker, "#112233");
	xgeXuiColorPickerSetHex(&pApp->tAlphaPicker, "#11223380");
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = (xgeXuiColorPickerGetPaletteCount(&pApp->tRgbPicker) == 11) && (xgeXuiColorPickerGetPaletteCount(&pApp->tAlphaPicker) == 11) && !xgeXuiColorPickerGetAlphaEnabled(&pApp->tRgbPicker) && xgeXuiColorPickerGetAlphaEnabled(&pApp->tAlphaPicker);
	pApp->bHexOK = (xgeXuiColorPickerGetColor(&pApp->tRgbPicker) == XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0xff)) && (strcmp(xgeXuiColorPickerGetHex(&pApp->tRgbPicker), "#112233") == 0);
	pApp->bAlphaOK = (xgeXuiColorPickerGetColor(&pApp->tAlphaPicker) == XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0x80)) && (strcmp(xgeXuiColorPickerGetHex(&pApp->tAlphaPicker), "#11223380") == 0);
	iTarget = pApp->tRgbPicker.arrPalette[6];
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pRgbPickerWidget->tRect.fX + 8.0f, pApp->pRgbPickerWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tRgbPicker.arrPaletteRect[6].fX + 3.0f, pApp->tRgbPicker.arrPaletteRect[6].fY + 3.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bPaletteOK = pApp->iChangeCount == 1 && pApp->iLastColor == iTarget && xgeXuiColorPickerGetColor(&pApp->tRgbPicker) == iTarget;
	xgeXuiPopupSetOpen(pApp->tRgbPicker.pPopup, 0);
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d hex=%d palette=%d changes=%d color=%s",
		pApp->bInitOK,
		pApp->bHexOK && pApp->bAlphaOK,
		pApp->bPaletteOK,
		pApp->iChangeCount,
		xgeXuiColorPickerGetHex(&pApp->tAlphaPicker));
	xgeXuiLabelSetText(&pApp->tSummary, sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	if ( pApp->iFrameLimit > 0 ) {
		RunChecks(pApp);
	}
	UpdateSummary(pApp);
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
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	UpdateSummary(pApp);
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
			"xui_colorpicker final-summary frames=%d init=%d hex=%d alpha=%d palette=%d changes=%d rgb=%s rgba=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bHexOK,
			pApp->bAlphaOK,
			pApp->bPaletteOK,
			pApp->iChangeCount,
			xgeXuiColorPickerGetHex(&pApp->tRgbPicker),
			xgeXuiColorPickerGetHex(&pApp->tAlphaPicker));
		printf("xui_colorpicker summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_COLOR_PICKER_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 700;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI ColorPicker";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bHexOK && tApp.bAlphaOK && tApp.bPaletteOK) ? 0 : 3;
}
