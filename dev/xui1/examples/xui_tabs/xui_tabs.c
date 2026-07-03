#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_COUNT 10
#define BUTTON_COUNT 2

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_tabs_t tTabs;
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_button_t tButton[BUTTON_COUNT];
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iTabPlacement;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
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

static int ArgTabPlacement(const char* sText, int iDefault)
{
	if ( sText == NULL ) {
		return iDefault;
	}
	if ( strcmp(sText, "top") == 0 ) {
		return XGE_XUI_TABS_PLACEMENT_TOP;
	}
	if ( strcmp(sText, "bottom") == 0 ) {
		return XGE_XUI_TABS_PLACEMENT_BOTTOM;
	}
	if ( strcmp(sText, "left") == 0 ) {
		return XGE_XUI_TABS_PLACEMENT_LEFT;
	}
	if ( strcmp(sText, "right") == 0 ) {
		return XGE_XUI_TABS_PLACEMENT_RIGHT;
	}
	return iDefault;
}

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_tabs font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), fHeight > 0.0f ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= LABEL_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(26.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pLabel = &pApp->tLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, XGE_COLOR_RGBA(48, 58, 72, 255));
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int AddButton(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_button pButton;

	if ( pApp->iButtonCount >= BUTTON_COUNT ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(34.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->tButton[pApp->iButtonCount++];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(56, 126, 204, 255), XGE_COLOR_RGBA(70, 146, 222, 255), XGE_COLOR_RGBA(36, 96, 170, 255), XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(196, 208, 224, 255));
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pTabsWidget;
	xge_xui_widget pPage;
	int iIndex;
	int arrEnabled[4] = { 1, 1, 0, 1 };
	int arrDirty[4] = { 0, 1, 0, 0 };

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 20.0f, 20.0f, 20.0f, 20.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(229, 235, 244, 255));

	pTabsWidget = NewWidget(0.0f);
	if ( pTabsWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiTabsInit(&pApp->tTabs, &pApp->tXui, pTabsWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pTabsWidget);
		return XGE_ERROR;
	}
	xgeXuiTabsSetTabPlacement(&pApp->tTabs, pApp->iTabPlacement);
	xgeXuiTabsSetFont(&pApp->tTabs, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiTabsSetColors(&pApp->tTabs, XGE_COLOR_RGBA(248, 252, 255, 255), XGE_COLOR_RGBA(232, 238, 247, 255), XGE_COLOR_RGBA(218, 230, 246, 255), XGE_COLOR_RGBA(238, 126, 24, 255), XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(206, 211, 218, 180), XGE_COLOR_RGBA(36, 42, 52, 255), XGE_COLOR_RGBA(24, 34, 48, 255));
	xgeXuiWidgetAdd(pRoot, pTabsWidget);

	iIndex = xgeXuiTabsAddPage(&pApp->tTabs, "Overview");
	pPage = xgeXuiTabsGetPageWidget(&pApp->tTabs, iIndex);
	AddLabel(pApp, pPage, "Tabs own the tab bar and page client.");
	AddLabel(pApp, pPage, "The selected page fills the client area.");

	iIndex = xgeXuiTabsAddPage(&pApp->tTabs, "Inventory");
	pPage = xgeXuiTabsGetPageWidget(&pApp->tTabs, iIndex);
	AddLabel(pApp, pPage, "Dirty state is shown as a badge on the tab.");
	AddButton(pApp, pPage, "Equip");

	iIndex = xgeXuiTabsAddPage(&pApp->tTabs, "Disabled");
	pPage = xgeXuiTabsGetPageWidget(&pApp->tTabs, iIndex);
	AddLabel(pApp, pPage, "This page cannot be selected while disabled.");

	iIndex = xgeXuiTabsAddPage(&pApp->tTabs, "Settings");
	pPage = xgeXuiTabsGetPageWidget(&pApp->tTabs, iIndex);
	AddLabel(pApp, pPage, "Page widgets are normal layout containers.");
	AddButton(pApp, pPage, "Save");

	xgeXuiTabsSetEnabledItems(&pApp->tTabs, arrEnabled, 4);
	xgeXuiTabsSetDirtyItems(&pApp->tTabs, arrDirty, 4);
	xgeXuiTabsSetSelected(&pApp->tTabs, 1);
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	float fRootW;
	float fRootH;
	int iWidth;
	int iHeight;

	iWidth = xgeGetWidth();
	iHeight = xgeGetHeight();
	if ( iWidth == pApp->iLastWidth && iHeight == pApp->iLastHeight ) {
		return;
	}
	pRoot = xgeXuiRoot(&pApp->tXui);
	fRootW = (float)iWidth;
	fRootH = (float)iHeight;
	if ( fRootW < 900.0f ) {
		fRootW = 900.0f;
	}
	if ( fRootH < 520.0f ) {
		fRootH = 520.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	int iBefore;
	xge_xui_widget pSelectedPage;

	iBefore = xgeXuiTabsGetSelected(&pApp->tTabs);
	xgeXuiTabsSetSelected(&pApp->tTabs, 2);
	pSelectedPage = xgeXuiTabsGetPageWidget(&pApp->tTabs, iBefore);
	pApp->bCreateOK = (pApp->tTabs.iItemCount == 4) && (pApp->iLabelCount == 5) && (pApp->iButtonCount == 2);
	pApp->bLayoutOK = xgeXuiRoot(&pApp->tXui)->tRect.fW >= 900.0f;
	pApp->bStateOK = (xgeXuiTabsGetSelected(&pApp->tTabs) == iBefore) &&
		(pSelectedPage != NULL) &&
		((pSelectedPage->iFlags & XGE_XUI_WIDGET_VISIBLE) != 0) &&
		((xgeXuiTabsGetButtonWidget(&pApp->tTabs, 2)->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) &&
		(xgeXuiTabsGetTabPlacement(&pApp->tTabs) == pApp->iTabPlacement) &&
		(pApp->tTabs.arrButton[1].bBadgeVisible == 1);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	pApp->bFontReady = (LoadFont(&pApp->tFont) == XGE_OK);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	LayoutRoot(pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	RunChecks(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;
	int i;

	pApp = (app_state_t*)pScene->pUser;
	for ( i = 0; i < pApp->iButtonCount; i++ ) {
		xgeXuiButtonUnit(&pApp->tButton[i]);
	}
	for ( i = 0; i < pApp->iLabelCount; i++ ) {
		xgeXuiLabelUnit(&pApp->tLabel[i]);
	}
	xgeXuiTabsUnit(&pApp->tTabs);
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
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_tabs final-summary frames=%d create=%d layout=%d state=%d selected=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, xgeXuiTabsGetSelected(&pApp->tTabs));
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
	tApp.iTabPlacement = XGE_XUI_TABS_PLACEMENT_TOP;
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TABS_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		} else if ( ((strcmp(argv[i], "--tab-side") == 0) || (strcmp(argv[i], "--tab-placement") == 0)) && ((i + 1) < argc) ) {
			tApp.iTabPlacement = ArgTabPlacement(argv[++i], tApp.iTabPlacement);
		}
	}
	tDesc.iWidth = 920;
	tDesc.iHeight = 540;
	tDesc.sTitle = "XUI Tabs";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 3;
}
