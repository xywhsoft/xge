#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LABEL_CAPACITY 32
#define BUTTON_CAPACITY 8
#define SPLIT_COUNT 2

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_label_t arrLabel[LABEL_CAPACITY];
	xge_xui_button_t arrButton[BUTTON_CAPACITY];
	xge_xui_split_layout_t arrSplit[SPLIT_COUNT];
	int iLabelCount;
	int iButtonCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
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

static int LoadFont(xge_font pFont)
{
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_split_layout font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_xui_widget NewWidget(float fW, float fH)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, fW > 0.0f ? xgeXuiSizePx(fW) : xgeXuiSizeGrow(1.0f), fH > 0.0f ? xgeXuiSizePx(fH) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, const char* sText, float fHeight, uint32_t iColor)
{
	xge_xui_widget pWidget;
	xge_xui_label pLabel;

	if ( pApp->iLabelCount >= LABEL_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(0.0f, fHeight);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pLabel = &pApp->arrLabel[pApp->iLabelCount++];
	if ( xgeXuiLabelInit(pLabel, pWidget, pApp->bFontReady ? &pApp->tFont : NULL, sText) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(pLabel, iColor);
	xgeXuiLabelSetAlign(pLabel, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static int AddButton(app_state_t* pApp, xge_xui_widget pParent, const char* sText)
{
	xge_xui_widget pWidget;
	xge_xui_button pButton;

	if ( pApp->iButtonCount >= BUTTON_CAPACITY ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget = NewWidget(0.0f, 32.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->arrButton[pApp->iButtonCount++];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetTextColor(pButton, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiButtonSetColors(pButton, XGE_COLOR_RGBA(56, 126, 204, 255), XGE_COLOR_RGBA(70, 146, 222, 255), XGE_COLOR_RGBA(36, 96, 170, 255), XGE_COLOR_RGBA(126, 166, 220, 255), XGE_COLOR_RGBA(196, 208, 224, 255));
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static void StylePane(xge_xui_widget pPane, uint32_t iBackground)
{
	xgeXuiWidgetSetBackground(pPane, iBackground);
	xgeXuiWidgetSetBorder(pPane, 1.0f, XGE_COLOR_RGBA(166, 188, 214, 255));
	xgeXuiWidgetSetPaddingPx(pPane, 12.0f, 10.0f, 12.0f, 10.0f);
	xgeXuiWidgetSetGap(pPane, 7.0f);
}

static int CreateVerticalSplit(app_state_t* pApp, xge_xui_widget pParent)
{
	xge_xui_widget pWidget;
	xge_xui_widget pPane;

	pWidget = NewWidget(0.0f, 0.0f);
	if ( pWidget == NULL || xgeXuiWidgetAdd(pParent, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(244, 248, 252, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(92, 154, 222, 255));
	if ( xgeXuiSplitLayoutInit(&pApp->arrSplit[0], &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSplitLayoutSetOrientation(&pApp->arrSplit[0], XGE_XUI_ORIENTATION_VERTICAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->arrSplit[0], 3);
	xgeXuiSplitLayoutSetDividerSize(&pApp->arrSplit[0], 10.0f);
	xgeXuiSplitLayoutSetDividerVisualSize(&pApp->arrSplit[0], 3.0f);
	xgeXuiSplitLayoutSetDividerHitSize(&pApp->arrSplit[0], 14.0f);
	xgeXuiSplitLayoutSetPaneFixedSize(&pApp->arrSplit[0], 0, 120.0f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->arrSplit[0], 1, 1.0f);
	xgeXuiSplitLayoutSetPaneFixedSize(&pApp->arrSplit[0], 2, 130.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->arrSplit[0], 1, 100.0f);
	xgeXuiSplitLayoutSetShadowDrag(&pApp->arrSplit[0], 1);

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[0], 0);
	StylePane(pPane, XGE_COLOR_RGBA(232, 241, 250, 255));
	(void)AddLabel(pApp, pPane, "Fixed left pane", 24.0f, XGE_COLOR_RGBA(40, 62, 86, 255));
	(void)AddLabel(pApp, pPane, "120 px fixed", 22.0f, XGE_COLOR_RGBA(86, 105, 128, 255));

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[0], 1);
	StylePane(pPane, XGE_COLOR_RGBA(248, 251, 255, 255));
	(void)AddLabel(pApp, pPane, "Grow content pane", 24.0f, XGE_COLOR_RGBA(40, 62, 86, 255));
	(void)AddButton(pApp, pPane, "Child button");
	(void)AddLabel(pApp, pPane, "Divider has thin visual and wider hit area.", 42.0f, XGE_COLOR_RGBA(86, 105, 128, 255));

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[0], 2);
	StylePane(pPane, XGE_COLOR_RGBA(238, 246, 250, 255));
	(void)AddLabel(pApp, pPane, "Fixed inspector", 24.0f, XGE_COLOR_RGBA(40, 62, 86, 255));
	(void)AddLabel(pApp, pPane, "Shadow drag is enabled.", 22.0f, XGE_COLOR_RGBA(86, 105, 128, 255));
	return XGE_OK;
}

static int CreateHorizontalSplit(app_state_t* pApp, xge_xui_widget pParent)
{
	xge_xui_widget pWidget;
	xge_xui_widget pPane;

	pWidget = NewWidget(0.0f, 0.0f);
	if ( pWidget == NULL || xgeXuiWidgetAdd(pParent, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(244, 248, 252, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(92, 154, 222, 255));
	if ( xgeXuiSplitLayoutInit(&pApp->arrSplit[1], &pApp->tXui, pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiSplitLayoutSetOrientation(&pApp->arrSplit[1], XGE_XUI_ORIENTATION_HORIZONTAL);
	xgeXuiSplitLayoutSetPaneCount(&pApp->arrSplit[1], 3);
	xgeXuiSplitLayoutSetDividerSize(&pApp->arrSplit[1], 9.0f);
	xgeXuiSplitLayoutSetDividerVisualSize(&pApp->arrSplit[1], 4.0f);
	xgeXuiSplitLayoutSetDividerHitSize(&pApp->arrSplit[1], 13.0f);
	xgeXuiSplitLayoutSetPaneFixedSize(&pApp->arrSplit[1], 0, 74.0f);
	xgeXuiSplitLayoutSetPaneWeight(&pApp->arrSplit[1], 1, 1.0f);
	xgeXuiSplitLayoutSetPaneFixedSize(&pApp->arrSplit[1], 2, 92.0f);
	xgeXuiSplitLayoutSetPaneMinSize(&pApp->arrSplit[1], 1, 100.0f);
	xgeXuiSplitLayoutSetShadowDrag(&pApp->arrSplit[1], 0);
	xgeXuiSplitLayoutSetColors(&pApp->arrSplit[1], XGE_COLOR_RGBA(222, 132, 48, 210), XGE_COLOR_RGBA(238, 158, 70, 235), XGE_COLOR_RGBA(196, 96, 30, 245), XGE_COLOR_RGBA(222, 132, 48, 100));

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[1], 0);
	StylePane(pPane, XGE_COLOR_RGBA(254, 246, 232, 255));
	(void)AddLabel(pApp, pPane, "Fixed top pane", 24.0f, XGE_COLOR_RGBA(78, 56, 36, 255));

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[1], 1);
	StylePane(pPane, XGE_COLOR_RGBA(255, 252, 246, 255));
	(void)AddLabel(pApp, pPane, "Live drag pane", 24.0f, XGE_COLOR_RGBA(78, 56, 36, 255));
	(void)AddButton(pApp, pPane, "Apply layout");

	pPane = xgeXuiSplitLayoutGetPaneWidget(&pApp->arrSplit[1], 2);
	StylePane(pPane, XGE_COLOR_RGBA(252, 240, 224, 255));
	(void)AddLabel(pApp, pPane, "Fixed bottom", 24.0f, XGE_COLOR_RGBA(78, 56, 36, 255));
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_widget pTitle;
	xge_xui_widget pBody;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 20.0f, 20.0f, 20.0f, 20.0f);
	xgeXuiWidgetSetGap(pRoot, 12.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(229, 235, 244, 255));

	pTitle = NewWidget(0.0f, 32.0f);
	pBody = NewWidget(0.0f, 0.0f);
	if ( (pTitle == NULL) || (pBody == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetBackground(pTitle, XGE_COLOR_RGBA(236, 244, 252, 255));
	xgeXuiWidgetSetPaddingPx(pTitle, 10.0f, 0.0f, 10.0f, 0.0f);
	xgeXuiWidgetSetLayout(pTitle, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetLayout(pBody, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pBody, 2, 0.0f, 12.0f, 12.0f);
	(void)xgeXuiWidgetAdd(pRoot, pTitle);
	(void)xgeXuiWidgetAdd(pRoot, pBody);
	(void)AddLabel(pApp, pTitle, "SplitLayout: fixed/grow panes, visual divider, hit divider, shadow/live drag", 30.0f, XGE_COLOR_RGBA(38, 72, 108, 255));
	return (CreateVerticalSplit(pApp, pBody) == XGE_OK && CreateHorizontalSplit(pApp, pBody) == XGE_OK) ? XGE_OK : XGE_ERROR;
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
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 560.0f ) {
		fRootH = 560.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	pApp->bCreateOK = (pApp->iLabelCount >= 9) && (pApp->iButtonCount == 2) &&
		(xgeXuiSplitLayoutGetPaneCount(&pApp->arrSplit[0]) == 3) &&
		(xgeXuiSplitLayoutGetPaneCount(&pApp->arrSplit[1]) == 3);
	pApp->bLayoutOK = (xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[0], 0) > 0.0f) &&
		(xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[0], 1) > 0.0f) &&
		(xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[1], 1) > 0.0f);
	pApp->bStateOK = (xgeXuiSplitLayoutGetPaneMode(&pApp->arrSplit[0], 0) == XGE_XUI_SPLIT_PANE_FIXED) &&
		(xgeXuiSplitLayoutGetPaneMode(&pApp->arrSplit[0], 1) == XGE_XUI_SPLIT_PANE_GROW) &&
		(pApp->arrSplit[0].fDividerVisualSize < pApp->arrSplit[0].fDividerHitSize) &&
		(pApp->arrSplit[1].bShadowDrag == 0);
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
	for ( i = 0; i < SPLIT_COUNT; i++ ) {
		xgeXuiSplitLayoutUnit(&pApp->arrSplit[i]);
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
	RunChecks(pApp);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_split_layout final-summary frames=%d create=%d layout=%d state=%d v=%.0f/%.0f/%.0f h=%.0f/%.0f/%.0f\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bStateOK,
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[0], 0),
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[0], 1),
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[0], 2),
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[1], 0),
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[1], 1),
			xgeXuiSplitLayoutGetPaneSize(&pApp->arrSplit[1], 2));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_SPLIT_LAYOUT_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 580;
	tDesc.sTitle = "XUI SplitLayout";
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	memset(&tApp.tScene, 0, sizeof(tApp.tScene));
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	tApp.tScene.pUser = &tApp;
	xgeSceneSet(&tApp.tScene);
	iExitCode = (xgeRun(NULL, NULL) == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bStateOK) ? 0 : 2;
	xgeSceneSet(NULL);
	xgeUnit();
	return iExitCode;
}
