#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLIDER_COUNT 8

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInteractionOK;
} app_state_t;

static const char sXson[] =
"{\"xui\":1,"
"\"styles\":{"
"\"root\":{\"type\":\"column\",\"width\":\"100%\",\"height\":\"100%\",\"padding\":[26,22,26,22],\"gap\":5,\"background\":\"#E5EBF4FF\"},"
"\"label\":{\"type\":\"label\",\"width\":\"100%\",\"height\":34,\"padding\":[2,0,2,0],\"font\":\"@fonts.body\",\"textColor\":\"#2A3444FF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
"\"slider\":{\"type\":\"slider\",\"width\":\"100%\",\"height\":34,\"padding\":4,\"min\":0,\"max\":100,\"step\":5,\"pageStep\":20,\"trackSize\":4,\"knobSize\":14},"
"\"vertical\":{\"type\":\"slider\",\"width\":\"100%\",\"height\":180,\"orientation\":\"vertical\",\"padding\":[0,4,0,4],\"min\":0,\"max\":100,\"trackSize\":6,\"knobSize\":18}"
"},"
"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
"{\"type\":\"label\",\"id\":\"l0\",\"style\":\"label\",\"text\":\"Default horizontal\"},{\"type\":\"slider\",\"id\":\"s0\",\"style\":\"slider\",\"value\":12},"
"{\"type\":\"label\",\"id\":\"l1\",\"style\":\"label\",\"text\":\"Disabled horizontal\"},{\"type\":\"slider\",\"id\":\"s1\",\"style\":\"slider\",\"value\":25,\"enabled\":false},"
"{\"type\":\"label\",\"id\":\"l2\",\"style\":\"label\",\"text\":\"Custom colors\"},{\"type\":\"slider\",\"id\":\"s2\",\"style\":\"slider\",\"value\":38,\"trackColor\":\"#E8DFC8FF\",\"fillColor\":\"#D47125FF\",\"knobColor\":\"#FFF8EBFF\",\"knobBorderColor\":\"#A04E16FF\"},"
"{\"type\":\"label\",\"id\":\"l3\",\"style\":\"label\",\"text\":\"Large knob\"},{\"type\":\"slider\",\"id\":\"s3\",\"style\":\"slider\",\"value\":51,\"trackSize\":8,\"knobSize\":22},"
"{\"type\":\"label\",\"id\":\"l4\",\"style\":\"label\",\"text\":\"Mid value\"},{\"type\":\"slider\",\"id\":\"s4\",\"style\":\"slider\",\"value\":64},"
"{\"type\":\"label\",\"id\":\"l5\",\"style\":\"label\",\"text\":\"High value\"},{\"type\":\"slider\",\"id\":\"s5\",\"style\":\"slider\",\"value\":77},"
"{\"type\":\"label\",\"id\":\"l6\",\"style\":\"label\",\"text\":\"Vertical slider\"},{\"type\":\"slider\",\"id\":\"s6\",\"style\":\"vertical\",\"value\":65},"
"{\"type\":\"label\",\"id\":\"l7\",\"style\":\"label\",\"text\":\"Vertical disabled\"},{\"type\":\"slider\",\"id\":\"s7\",\"style\":\"vertical\",\"value\":35,\"enabled\":false}"
"]}}";

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
			printf("xui_slider_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, int iButton, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = iButton;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->iPointerId = 1;
}

static int CreateUI(app_state_t* pApp)
{
	if ( xgeXuiTokenSetFont(&pApp->tXui, "body", pApp->bFontReady ? &pApp->tFont : NULL) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( xgeXuiPageLoadMemory(&pApp->tXui, sXson, (int)strlen(sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_slider_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iWidth;
	int iHeight;
	float fW;
	float fH;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiPageRoot(&pApp->tPage);
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 720.0f ) {
		fW = 720.0f;
	}
	if ( fH < 960.0f ) {
		fH = 960.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_widget pWidget;
	xge_xui_slider pSlider;

	pApp->bCreateOK = (pApp->tPage.iSliderCount == SLIDER_COUNT) && (xgeXuiPageFind(&pApp->tPage, "s0") != NULL);
	pWidget = xgeXuiPageFind(&pApp->tPage, "s6");
	pApp->bLayoutOK = (pWidget != NULL) && (pWidget->tRect.fH > 90.0f);
	pWidget = xgeXuiPageFind(&pApp->tPage, "s2");
	pSlider = (pWidget != NULL) ? (xge_xui_slider)pWidget->pUser : NULL;
	pApp->bStateOK = (pSlider != NULL) && (pSlider->iColorFill == XGE_COLOR_RGBA(212, 113, 37, 255));
	pWidget = xgeXuiPageFind(&pApp->tPage, "s1");
	pApp->bStateOK = pApp->bStateOK && (pWidget != NULL) && (xgeXuiWidgetIsEnabled(pWidget) == 0);
	pWidget = xgeXuiPageFind(&pApp->tPage, "s5");
	pApp->bStateOK = pApp->bStateOK && (pWidget != NULL) && (pWidget->pUser != NULL);
}

static void RunInteractionChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_widget pWidget;
	xge_xui_slider pSlider;
	float fX;
	float fY;
	float fOldValue;
	int iDown;
	int iUp;

	if ( pApp->bInteractionOK != 0 ) {
		return;
	}
	pWidget = xgeXuiPageFind(&pApp->tPage, "s0");
	pSlider = (pWidget != NULL) ? (xge_xui_slider)pWidget->pUser : NULL;
	if ( (pWidget == NULL) || (pSlider == NULL) || (pWidget->tContentRect.fW <= 0.0f) || (pWidget->tContentRect.fH <= 0.0f) ) {
		return;
	}
	fOldValue = xgeXuiSliderGetValue(pSlider);
	fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * 0.80f;
	fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * 0.50f;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, fX, fY);
	iDown = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, fX, fY);
	iUp = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bInteractionOK = (iDown == XGE_XUI_EVENT_CONSUMED) && (iUp == XGE_XUI_EVENT_CONSUMED) && (xgeXuiSliderGetValue(pSlider) > fOldValue + 20.0f);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunInteractionChecks(pApp);
	RunChecks(pApp);
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
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunInteractionChecks(pApp);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_slider_xson final-summary frames=%d create=%d layout=%d state=%d interaction=%d sliders=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->bInteractionOK, pApp->tPage.iSliderCount);
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
	static app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SLIDER_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 960;
	tDesc.sTitle = "XUI Slider XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK && tApp.bInteractionOK) ? 0 : 3;
}
