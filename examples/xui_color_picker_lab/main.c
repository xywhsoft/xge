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
	xge_xui_color_picker_t tPicker;
	xge_xui_widget pPanel;
	xge_xui_widget pSummary;
	xge_xui_widget pPickerWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bInitOK;
	int bHexOK;
	int bPaletteOK;
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
			printf("xui-color-picker-lab font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui-color-picker-lab font load failed\n");
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
	uint32_t arrPalette[6];

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	pApp->pSummary = xgeXuiWidgetCreate();
	pApp->pPickerWidget = xgeXuiWidgetCreate();
	if ( (pApp->pPanel == NULL) || (pApp->pSummary == NULL) || (pApp->pPickerWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 24.0f, 22.0f, 356.0f, 250.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "ColorPicker");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);
	xgeXuiWidgetSetRect(pApp->pSummary, (xge_rect_t){ 18.0f, 38.0f, 310.0f, 24.0f });
	xgeXuiLabelInit(&pApp->tSummary, pApp->pSummary, pFont, "");
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pSummary);
	xgeXuiWidgetSetRect(pApp->pPickerWidget, (xge_rect_t){ 18.0f, 72.0f, 300.0f, 136.0f });
	xgeXuiColorPickerInit(&pApp->tPicker, &pApp->tXui, pApp->pPickerWidget, pFont);
	xgeXuiColorPickerSetChange(&pApp->tPicker, ColorChanged, pApp);
	arrPalette[0] = XGE_COLOR_RGBA(78, 159, 220, 255);
	arrPalette[1] = XGE_COLOR_RGBA(43, 184, 150, 255);
	arrPalette[2] = XGE_COLOR_RGBA(244, 187, 68, 255);
	arrPalette[3] = XGE_COLOR_RGBA(224, 92, 92, 255);
	arrPalette[4] = XGE_COLOR_RGBA(142, 116, 220, 255);
	arrPalette[5] = XGE_COLOR_RGBA(34, 48, 64, 255);
	xgeXuiColorPickerSetPalette(&pApp->tPicker, arrPalette, 6);
	xgeXuiWidgetAdd(pApp->pPanel, pApp->pPickerWidget);
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	uint32_t iTarget;

	xgeXuiColorPickerSetHex(&pApp->tPicker, "#11223344");
	xgeXuiPaint(&pApp->tXui);
	pApp->bInitOK = xgeXuiColorPickerGetPaletteCount(&pApp->tPicker) == 6;
	pApp->bHexOK = xgeXuiColorPickerGetColor(&pApp->tPicker) == XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0x44) && strcmp(xgeXuiColorPickerGetHex(&pApp->tPicker), "#11223344") == 0;
	iTarget = pApp->tPicker.arrPalette[2];
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->tPicker.arrPaletteRect[2].fX + 3.0f, pApp->tPicker.arrPaletteRect[2].fY + 3.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bPaletteOK = pApp->iChangeCount == 1 && pApp->iLastColor == iTarget && xgeXuiColorPickerGetColor(&pApp->tPicker) == iTarget;
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[192];

	snprintf(
		sText,
		sizeof(sText),
		"init=%d hex=%d palette=%d changes=%d color=%s",
		pApp->bInitOK,
		pApp->bHexOK,
		pApp->bPaletteOK,
		pApp->iChangeCount,
		xgeXuiColorPickerGetHex(&pApp->tPicker));
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
	RunChecks(pApp);
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
			"xui-color-picker-lab final-summary frames=%d init=%d hex=%d palette=%d changes=%d color=%s\n",
			pApp->iFrameCount,
			pApp->bInitOK,
			pApp->bHexOK,
			pApp->bPaletteOK,
			pApp->iChangeCount,
			xgeXuiColorPickerGetHex(&pApp->tPicker));
		printf("xui-color-picker-lab summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	tDesc.iWidth = 404;
	tDesc.iHeight = 296;
	tDesc.sTitle = "XGE XUI ColorPicker Lab";
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
	return (iExitCode == XGE_OK && tApp.bInitOK && tApp.bHexOK && tApp.bPaletteOK) ? 0 : 3;
}
