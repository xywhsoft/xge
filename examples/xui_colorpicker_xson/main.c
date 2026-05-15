#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	xge_xui_color_picker pRgbPicker;
	xge_xui_color_picker pAlphaPicker;
	xge_xui_widget pRgbWidget;
	xge_xui_widget pAlphaWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bHexOK;
	int bAlphaOK;
	int bPaletteOK;
	int iChangeCount;
	uint32_t iLastColor;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[36,40,36,40],\"gap\":18,\"background\":\"#E5EBF4FF\"},"
"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":\"100%\",\"height\":28,\"textColor\":\"#2A3444FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"grid\":{\"type\":\"grid\",\"width\":330,\"height\":78,\"columns\":2,\"rowHeight\":30,\"columnGap\":12,\"rowGap\":18},"
"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":80,\"height\":30,\"textColor\":\"#52647AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"picker\":{\"type\":\"colorPicker\",\"width\":230,\"height\":30,\"backgroundColor\":\"#F8FCFFFF\",\"borderColor\":\"#7EA6C8FF\",\"panelColor\":\"#1F2C38FF\",\"textColor\":\"#18384FFF\",\"accentColor\":\"#35AEEAFF\",\"fieldColor\":\"#F6FBFFFF\"}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"title\",\"style\":\"title\",\"text\":\"ColorPicker XSON\"},"
"{\"type\":\"grid\",\"id\":\"grid\",\"style\":\"grid\",\"children\":["
"{\"type\":\"label\",\"id\":\"rgbLabel\",\"style\":\"label\",\"text\":\"RGB\"},"
"{\"type\":\"colorPicker\",\"id\":\"rgbPicker\",\"style\":\"picker\",\"value\":\"#112233\",\"alphaEnabled\":false,"
"\"palette\":[\"#00000000\",\"#000000\",\"#FFFFFF\",\"#949EA8\",\"#E13A46\",\"#E67E22\",\"#F4C936\",\"#4AA55B\",\"#2BB8CB\",\"#2E7CD6\",\"#8456D1\"]},"
"{\"type\":\"label\",\"id\":\"alphaLabel\",\"style\":\"label\",\"text\":\"RGBA\"},"
"{\"type\":\"colorPicker\",\"id\":\"alphaPicker\",\"style\":\"picker\",\"value\":\"#11223380\",\"alphaEnabled\":true,"
"\"palette\":[\"#00000000\",\"#000000\",\"#FFFFFF\",\"#949EA8\",\"#E13A46\",\"#E67E22\",\"#F4C936\",\"#4AA55B\",\"#2BB8CB\",\"#2E7CD6\",\"#8456D1\"]}"
"]}]}}";

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
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_colorpicker_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_colorpicker_xson font load failed\n");
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
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_colorpicker_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	pApp->pRgbWidget = xgeXuiPageFind(&pApp->tPage, "rgbPicker");
	pApp->pAlphaWidget = xgeXuiPageFind(&pApp->tPage, "alphaPicker");
	pApp->pRgbPicker = (pApp->pRgbWidget != NULL) ? (xge_xui_color_picker)pApp->pRgbWidget->pUser : NULL;
	pApp->pAlphaPicker = (pApp->pAlphaWidget != NULL) ? (xge_xui_color_picker)pApp->pAlphaWidget->pUser : NULL;
	if ( (pApp->pRgbPicker == NULL) || (pApp->pAlphaPicker == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiColorPickerSetChange(pApp->pRgbPicker, ColorChanged, pApp);
	xgeXuiColorPickerSetChange(pApp->pAlphaPicker, ColorChanged, pApp);
	pApp->bCreateOK = 1;
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	uint32_t iTarget;

	xgeXuiPaint(&pApp->tXui);
	pApp->bHexOK =
		(xgeXuiColorPickerGetColor(pApp->pRgbPicker) == XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0xff)) &&
		(strcmp(xgeXuiColorPickerGetHex(pApp->pRgbPicker), "#112233") == 0);
	pApp->bAlphaOK =
		xgeXuiColorPickerGetAlphaEnabled(pApp->pAlphaPicker) &&
		(xgeXuiColorPickerGetColor(pApp->pAlphaPicker) == XGE_COLOR_RGBA(0x11, 0x22, 0x33, 0x80)) &&
		(strcmp(xgeXuiColorPickerGetHex(pApp->pAlphaPicker), "#11223380") == 0);
	iTarget = pApp->pRgbPicker->arrPalette[6];
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pRgbWidget->tRect.fX + 8.0f, pApp->pRgbWidget->tRect.fY + 8.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiPaint(&pApp->tXui);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pApp->pRgbPicker->arrPaletteRect[6].fX + 3.0f, pApp->pRgbPicker->arrPaletteRect[6].fY + 3.0f);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bPaletteOK = (pApp->iChangeCount == 1) && (pApp->iLastColor == iTarget) && (xgeXuiColorPickerGetColor(pApp->pRgbPicker) == iTarget);
	xgeXuiPopupSetOpen(&pApp->pRgbPicker->tPopup, 0);
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
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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
			"xui_colorpicker_xson final-summary frames=%d create=%d hex=%d alpha=%d palette=%d changes=%d rgb=%s rgba=%s\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bHexOK,
			pApp->bAlphaOK,
			pApp->bPaletteOK,
			pApp->iChangeCount,
			xgeXuiColorPickerGetHex(pApp->pRgbPicker),
			xgeXuiColorPickerGetHex(pApp->pAlphaPicker));
		printf("xui_colorpicker_xson summary frames=%d/%d\n", pApp->iFrameCount, pApp->iFrameLimit);
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
	static app_state_t tApp;
	xge_desc_t tDesc;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_COLORPICKER_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 700;
	tDesc.iHeight = 560;
	tDesc.sTitle = "XUI ColorPicker XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bHexOK && tApp.bAlphaOK && tApp.bPaletteOK) ? 0 : 3;
}
