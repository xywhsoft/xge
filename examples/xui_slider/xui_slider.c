#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLIDER_COUNT 8
#define LABEL_COUNT 8

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pLabelWidget[LABEL_COUNT];
	xge_xui_widget pSliderWidget[SLIDER_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_slider_t tSlider[SLIDER_COUNT];
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
			printf("xui_slider font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
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

static xge_xui_widget NewWidget(void)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(34.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sText)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 0.0f, 2.0f, 0.0f);
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pWidget, AppFont(pApp), sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(42, 52, 68, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pRoot, pWidget);
	pApp->pLabelWidget[iIndex] = pWidget;
	return XGE_OK;
}

static int AddSlider(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, float fValue)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget();
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetPaddingPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	if ( xgeXuiSliderInit(&pApp->tSlider[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiSliderSetRange(&pApp->tSlider[iIndex], 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tSlider[iIndex], fValue);
	xgeXuiSliderSetStep(&pApp->tSlider[iIndex], 5.0f, 20.0f);
	xgeXuiWidgetAdd(pRoot, pWidget);
	pApp->pSliderWidget[iIndex] = pWidget;
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pVertical;
	int i;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetPaddingPx(pRoot, 26.0f, 22.0f, 26.0f, 22.0f);

	for ( i = 0; i < 6; i++ ) {
		static const char* arrText[6] = {
			"Default horizontal",
			"Disabled horizontal",
			"Custom colors",
			"Large knob",
			"Mid value",
			"High value"
		};
		if ( AddLabel(pApp, pRoot, i, arrText[i]) != XGE_OK || AddSlider(pApp, pRoot, i, 12.0f + (float)i * 13.0f) != XGE_OK ) {
			return XGE_ERROR;
		}
	}
	xgeXuiWidgetSetEnabled(pApp->pSliderWidget[1], 0);
	xgeXuiSliderSetColors(&pApp->tSlider[2], XGE_COLOR_RGBA(232, 223, 200, 255), XGE_COLOR_RGBA(212, 113, 37, 255), XGE_COLOR_RGBA(255, 248, 235, 255), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(180, 186, 196, 255));
	xgeXuiSliderSetKnobBorderColor(&pApp->tSlider[2], XGE_COLOR_RGBA(160, 78, 22, 255));
	xgeXuiSliderSetMetrics(&pApp->tSlider[3], 8.0f, 22.0f, -1.0f, -1.0f);
	if ( AddLabel(pApp, pRoot, 6, "Vertical slider") != XGE_OK ) {
		return XGE_ERROR;
	}
	pVertical = xgeXuiWidgetCreate();
	if ( pVertical == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pVertical, xgeXuiSizePercent(100.0f), xgeXuiSizePx(180.0f));
	xgeXuiWidgetSetPaddingPx(pVertical, 0.0f, 4.0f, 0.0f, 4.0f);
	if ( xgeXuiSliderInit(&pApp->tSlider[6], &pApp->tXui, pVertical) != XGE_OK ) {
		xgeXuiWidgetFree(pVertical);
		return XGE_ERROR;
	}
	xgeXuiSliderSetRange(&pApp->tSlider[6], 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tSlider[6], 65.0f);
	xgeXuiSliderSetOrientation(&pApp->tSlider[6], XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiSliderSetMetrics(&pApp->tSlider[6], 6.0f, 18.0f, -1.0f, -1.0f);
	xgeXuiWidgetAdd(pRoot, pVertical);
	pApp->pSliderWidget[6] = pVertical;

	if ( AddLabel(pApp, pRoot, 7, "Vertical disabled") != XGE_OK ) {
		return XGE_ERROR;
	}
	pVertical = xgeXuiWidgetCreate();
	if ( pVertical == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetSize(pVertical, xgeXuiSizePercent(100.0f), xgeXuiSizePx(180.0f));
	if ( xgeXuiSliderInit(&pApp->tSlider[7], &pApp->tXui, pVertical) != XGE_OK ) {
		xgeXuiWidgetFree(pVertical);
		return XGE_ERROR;
	}
	xgeXuiSliderSetRange(&pApp->tSlider[7], 0.0f, 100.0f);
	xgeXuiSliderSetValue(&pApp->tSlider[7], 35.0f);
	xgeXuiSliderSetOrientation(&pApp->tSlider[7], XGE_XUI_SEPARATOR_VERTICAL);
	xgeXuiWidgetSetEnabled(pVertical, 0);
	xgeXuiWidgetAdd(pRoot, pVertical);
	pApp->pSliderWidget[7] = pVertical;
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iWidth;
	int iHeight;
	float fW;
	float fH;
	float fLabelX;
	float fSliderX;
	float fY;
	float fSliderW;
	int i;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 720.0f ) {
		fW = 720.0f;
	}
	if ( fH < 720.0f ) {
		fH = 720.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	fLabelX = 26.0f;
	fSliderX = fW * 0.50f + 8.0f;
	fSliderW = fW - fSliderX - 26.0f;
	if ( fSliderW < 180.0f ) {
		fSliderW = 180.0f;
	}
	fY = 22.0f;
	for ( i = 0; i < 6; i++ ) {
		if ( pApp->pLabelWidget[i] != NULL ) {
			xgeXuiWidgetSetRect(pApp->pLabelWidget[i], (xge_rect_t){ fLabelX, fY, fSliderX - fLabelX - 8.0f, 34.0f });
		}
		if ( pApp->pSliderWidget[i] != NULL ) {
			xgeXuiWidgetSetRect(pApp->pSliderWidget[i], (xge_rect_t){ fSliderX, fY, fSliderW, 34.0f });
		}
		fY += 39.0f;
	}
	if ( pApp->pLabelWidget[6] != NULL ) {
		xgeXuiWidgetSetRect(pApp->pLabelWidget[6], (xge_rect_t){ fLabelX, fY, fSliderX - fLabelX - 8.0f, 34.0f });
	}
	if ( pApp->pSliderWidget[6] != NULL ) {
		xgeXuiWidgetSetRect(pApp->pSliderWidget[6], (xge_rect_t){ fSliderX + (fSliderW - 44.0f) * 0.5f, fY, 44.0f, 180.0f });
	}
	fY += 190.0f;
	if ( pApp->pLabelWidget[7] != NULL ) {
		xgeXuiWidgetSetRect(pApp->pLabelWidget[7], (xge_rect_t){ fLabelX, fY, fSliderX - fLabelX - 8.0f, 34.0f });
	}
	if ( pApp->pSliderWidget[7] != NULL ) {
		xgeXuiWidgetSetRect(pApp->pSliderWidget[7], (xge_rect_t){ fSliderX + (fSliderW - 44.0f) * 0.5f, fY, 44.0f, 180.0f });
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int i;

	pApp->bCreateOK = 1;
	for ( i = 0; i < SLIDER_COUNT; i++ ) {
		if ( (pApp->pSliderWidget[i] == NULL) || (pApp->tSlider[i].pWidget != pApp->pSliderWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK = (pApp->pSliderWidget[0] != NULL) && (pApp->pSliderWidget[0]->tRect.fW > 120.0f) && (pApp->pSliderWidget[6]->tRect.fH > 90.0f);
	pApp->bStateOK =
		(xgeXuiWidgetIsEnabled(pApp->pSliderWidget[1]) == 0) &&
		(pApp->tSlider[2].iColorFill == XGE_COLOR_RGBA(212, 113, 37, 255)) &&
		(pApp->tSlider[3].fKnobSize == 22.0f) &&
		(pApp->pSliderWidget[4] != NULL) &&
		(pApp->pSliderWidget[5] != NULL) &&
		(pApp->tSlider[6].iOrientation == XGE_XUI_SEPARATOR_VERTICAL);
}

static void RunInteractionChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_widget pWidget;
	float fX;
	float fY;
	float fOldValue;
	int iDown;
	int iUp;

	if ( pApp->bInteractionOK != 0 ) {
		return;
	}
	pWidget = pApp->pSliderWidget[0];
	if ( (pWidget == NULL) || (pWidget->tContentRect.fW <= 0.0f) || (pWidget->tContentRect.fH <= 0.0f) ) {
		return;
	}
	fOldValue = xgeXuiSliderGetValue(&pApp->tSlider[0]);
	fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * 0.80f;
	fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * 0.50f;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, fX, fY);
	iDown = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, fX, fY);
	iUp = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bInteractionOK = (iDown == XGE_XUI_EVENT_CONSUMED) && (iUp == XGE_XUI_EVENT_CONSUMED) && (xgeXuiSliderGetValue(&pApp->tSlider[0]) > fOldValue + 20.0f);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( (xgeXuiInit(&pApp->tXui) != XGE_OK) || (CreateUI(pApp) != XGE_OK) ) {
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
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < LABEL_COUNT; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	for ( i = 0; i < SLIDER_COUNT; i++ ) {
		xgeXuiSliderUnit(&pApp->tSlider[i]);
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
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, fDelta);
	RunInteractionChecks(pApp);
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_slider final-summary frames=%d create=%d layout=%d state=%d interaction=%d sliders=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->bInteractionOK, SLIDER_COUNT);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SLIDER_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 720;
	tDesc.sTitle = "XUI Slider";
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
