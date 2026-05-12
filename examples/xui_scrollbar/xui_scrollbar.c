#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BAR_COUNT 8
#define LABEL_COUNT 8

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pLabelWidget[LABEL_COUNT];
	xge_xui_widget pBarWidget[BAR_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_scrollbar_t tBar[BAR_COUNT];
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
			printf("xui_scrollbar font loaded: %s\n", arrFonts[i]);
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

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), xgeXuiSizePx(fHeight));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, const char* sText)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(34.0f);
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

static int AddBar(app_state_t* pApp, xge_xui_widget pRoot, int iIndex, float fValue, int iOrientation, int iMode, int iButtons, float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(fHeight);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePx(34.0f), xgeXuiSizePx(fHeight));
		xgeXuiWidgetSetAlign(pWidget, XGE_XUI_ALIGN_CENTER, XGE_XUI_ALIGN_START);
	}
	xgeXuiWidgetSetPaddingPx(pWidget, 4.0f, 4.0f, 4.0f, 4.0f);
	if ( xgeXuiScrollBarInit(&pApp->tBar[iIndex], &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiScrollBarSetRange(&pApp->tBar[iIndex], 0.0f, 100.0f, 24.0f);
	xgeXuiScrollBarSetValue(&pApp->tBar[iIndex], fValue);
	xgeXuiScrollBarSetOrientation(&pApp->tBar[iIndex], iOrientation);
	xgeXuiScrollBarSetMode(&pApp->tBar[iIndex], iMode);
	xgeXuiScrollBarSetButtonMode(&pApp->tBar[iIndex], iButtons);
	xgeXuiWidgetAdd(pRoot, pWidget);
	pApp->pBarWidget[iIndex] = pWidget;
	return XGE_OK;
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
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetPaddingPx(pRoot, 0.0f, 0.0f, 0.0f, 0.0f);

	for ( i = 0; i < BAR_COUNT; i++ ) {
		static const char* arrText[BAR_COUNT] = {
			"Horizontal full buttons",
			"Horizontal compact",
			"Custom colors",
			"Disabled horizontal",
			"Vertical full buttons",
			"Vertical compact",
			"Vertical custom",
			"Cache force"
		};
		if ( AddLabel(pApp, pRoot, i, arrText[i]) != XGE_OK ) {
			return XGE_ERROR;
		}
		if ( i < 4 ) {
			if ( AddBar(pApp, pRoot, i, 18.0f + (float)i * 17.0f, XGE_XUI_SEPARATOR_HORIZONTAL, (i == 1) ? XGE_XUI_SCROLLBAR_MODE_COMPACT : XGE_XUI_SCROLLBAR_MODE_FULL, (i == 1) ? XGE_XUI_SCROLLBAR_BUTTONS_OFF : XGE_XUI_SCROLLBAR_BUTTONS_ON, 38.0f) != XGE_OK ) {
				return XGE_ERROR;
			}
		} else {
			if ( AddBar(pApp, pRoot, i, 18.0f + (float)i * 9.0f, XGE_XUI_SEPARATOR_VERTICAL, (i == 5) ? XGE_XUI_SCROLLBAR_MODE_COMPACT : XGE_XUI_SCROLLBAR_MODE_FULL, (i == 5) ? XGE_XUI_SCROLLBAR_BUTTONS_OFF : XGE_XUI_SCROLLBAR_BUTTONS_ON, 112.0f) != XGE_OK ) {
				return XGE_ERROR;
			}
		}
	}
	xgeXuiScrollBarSetMetrics(&pApp->tBar[1], 8.0f, 24.0f, -1.0f, 0.0f);
	xgeXuiScrollBarSetColors(&pApp->tBar[2], XGE_COLOR_RGBA(248, 235, 205, 255), XGE_COLOR_RGBA(214, 118, 42, 255), XGE_COLOR_RGBA(226, 136, 56, 255), XGE_COLOR_RGBA(186, 86, 24, 255), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(180, 186, 196, 255));
	xgeXuiScrollBarSetButtonColors(&pApp->tBar[2], XGE_COLOR_RGBA(255, 248, 235, 255), XGE_COLOR_RGBA(156, 82, 24, 255));
	xgeXuiScrollBarSetMetrics(&pApp->tBar[2], 12.0f, 28.0f, -1.0f, 0.0f);
	xgeXuiScrollBarSetPage(&pApp->tBar[2], 48.0f);
	xgeXuiWidgetSetEnabled(pApp->pBarWidget[3], 0);
	xgeXuiScrollBarSetMetrics(&pApp->tBar[5], 8.0f, 24.0f, -1.0f, 0.0f);
	xgeXuiScrollBarSetColors(&pApp->tBar[6], XGE_COLOR_RGBA(220, 240, 234, 255), XGE_COLOR_RGBA(42, 146, 102, 255), XGE_COLOR_RGBA(54, 168, 118, 255), XGE_COLOR_RGBA(28, 118, 82, 255), XGE_COLOR_RGBA(0, 0, 0, 0), XGE_COLOR_RGBA(180, 186, 196, 255));
	xgeXuiScrollBarSetCacheMode(&pApp->tBar[7], XGE_XUI_CACHE_FORCE);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iWidth;
	int iHeight;
	float fW;
	float fH;
	float fBarX;
	float fBarW;
	float fY;
	int i;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( (iWidth == pApp->iLastWidth) && (iHeight == pApp->iLastHeight) ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fW = (float)iWidth;
	fH = (float)iHeight;
	if ( fW < 760.0f ) {
		fW = 760.0f;
	}
	if ( fH < 760.0f ) {
		fH = 760.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fW, fH });
	fBarX = 420.0f;
	fBarW = fW - fBarX - 30.0f;
	if ( fBarW < 280.0f ) {
		fBarW = 280.0f;
	}
	for ( i = 0; i < 4; i++ ) {
		fY = 52.0f + (float)i * 44.0f;
		xgeXuiWidgetSetRect(pApp->pLabelWidget[i], (xge_rect_t){ 28.0f, fY, 340.0f, 30.0f });
		xgeXuiWidgetSetRect(pApp->pBarWidget[i], (xge_rect_t){ fBarX, fY + 2.0f, fBarW, 30.0f });
	}
	for ( i = 4; i < 8; i++ ) {
		fY = 244.0f + (float)(i - 4) * 122.0f;
		xgeXuiWidgetSetRect(pApp->pLabelWidget[i], (xge_rect_t){ 28.0f, fY + 42.0f, 340.0f, 30.0f });
		xgeXuiWidgetSetRect(pApp->pBarWidget[i], (xge_rect_t){ fBarX + 182.0f, fY, 34.0f, 112.0f });
	}
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int i;

	pApp->bCreateOK = 1;
	for ( i = 0; i < BAR_COUNT; i++ ) {
		if ( (pApp->pBarWidget[i] == NULL) || (pApp->tBar[i].pWidget != pApp->pBarWidget[i]) ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bLayoutOK = (pApp->pBarWidget[0] != NULL) && (pApp->pBarWidget[0]->tRect.fW > 120.0f) && (pApp->pBarWidget[4]->tRect.fH > 90.0f);
	pApp->bStateOK =
		(pApp->tBar[1].iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) &&
		(pApp->tBar[1].iButtonMode == XGE_XUI_SCROLLBAR_BUTTONS_OFF) &&
		(pApp->tBar[2].iColorThumb == XGE_COLOR_RGBA(214, 118, 42, 255)) &&
		(xgeXuiWidgetIsEnabled(pApp->pBarWidget[3]) == 0) &&
		(pApp->tBar[4].iOrientation == XGE_XUI_SEPARATOR_VERTICAL) &&
		(pApp->tBar[7].iCacheMode == XGE_XUI_CACHE_FORCE);
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
	int iActivePart;

	if ( pApp->bInteractionOK != 0 ) {
		return;
	}
	pWidget = pApp->pBarWidget[0];
	if ( (pWidget == NULL) || (pWidget->tContentRect.fW <= 0.0f) || (pWidget->tContentRect.fH <= 0.0f) ) {
		return;
	}
	fOldValue = xgeXuiScrollBarGetValue(&pApp->tBar[0]);
	fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 8.0f;
	fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH * 0.50f;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, XGE_MOUSE_LEFT, fX, fY);
	iDown = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	iActivePart = pApp->tBar[0].iActivePart;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, XGE_MOUSE_LEFT, fX, fY);
	iUp = xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bInteractionOK =
		(iDown == XGE_XUI_EVENT_CONSUMED) &&
		(iUp == XGE_XUI_EVENT_CONSUMED) &&
		(iActivePart == XGE_XUI_SCROLLBAR_PART_BUTTON_END) &&
		(pApp->tBar[0].iActivePart == XGE_XUI_SCROLLBAR_PART_NONE) &&
		(pApp->tBar[0].iHoverPart == XGE_XUI_SCROLLBAR_PART_BUTTON_END) &&
		(pApp->tBar[0].bDraggingThumb == 0) &&
		(xgeXuiScrollBarGetValue(&pApp->tBar[0]) > fOldValue);
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
	for ( i = 0; i < BAR_COUNT; i++ ) {
		xgeXuiScrollBarUnit(&pApp->tBar[i]);
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
		printf("xui_scrollbar final-summary frames=%d create=%d layout=%d state=%d interaction=%d scrollbars=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->bInteractionOK, BAR_COUNT);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SCROLLBAR_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 820;
	tDesc.iHeight = 760;
	tDesc.sTitle = "XUI ScrollBar";
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
