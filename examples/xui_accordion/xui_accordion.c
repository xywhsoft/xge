#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCORDION_COUNT 3
#define LABEL_COUNT 12
#define BUTTON_COUNT 2

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_accordion_t tAccordion[ACCORDION_COUNT];
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_button_t tButton[BUTTON_COUNT];
	int iAccordionCount;
	int iLabelCount;
	int iButtonCount;
	int iSelectCount;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int bCreateOK;
	int bLayoutOK;
	int bStateOK;
	int bInteractionDone;
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
	const char* arrFonts[] = { "C:/Windows/Fonts/simsun.ttc", "C:/Windows/Fonts/Deng.ttf", "C:/Windows/Fonts/msyh.ttc", "C:/Windows/Fonts/arial.ttf" };
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(pFont, 0, sizeof(*pFont));
		if ( xgeFontLoad(pFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			printf("xui_accordion font loaded: %s\n", arrFonts[i]);
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
	pWidget = NewWidget(24.0f);
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
	pWidget = NewWidget(32.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pButton = &pApp->tButton[pApp->iButtonCount++];
	if ( xgeXuiButtonInit(pButton, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, pApp->bFontReady ? &pApp->tFont : NULL, sText);
	xgeXuiButtonSetSemantic(pButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	return xgeXuiWidgetAdd(pParent, pWidget);
}

static void AccordionSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
	}
}

static void MakeMouse(xge_event_t* pEvent, int iType, xge_xui_widget pWidget)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->iPointerId = 1;
	pEvent->fX = pWidget->tRect.fX + pWidget->tRect.fW * 0.5f;
	pEvent->fY = pWidget->tRect.fY + pWidget->tRect.fH * 0.5f;
}

static void ClickWidget(app_state_t* pApp, xge_xui_widget pWidget)
{
	xge_event_t tEvent;

	if ( (pApp == NULL) || (pWidget == NULL) ) {
		return;
	}
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, pWidget);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, pWidget);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
}

static xge_xui_accordion AddAccordion(app_state_t* pApp, xge_xui_widget pRoot)
{
	xge_xui_widget pWidget;
	xge_xui_accordion pAccordion;

	if ( pApp->iAccordionCount >= ACCORDION_COUNT ) {
		return NULL;
	}
	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return NULL;
	}
	pAccordion = &pApp->tAccordion[pApp->iAccordionCount++];
	if ( xgeXuiAccordionInit(pAccordion, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	xgeXuiAccordionSetFont(pAccordion, pApp->bFontReady ? &pApp->tFont : NULL);
	xgeXuiAccordionSetSelect(pAccordion, AccordionSelect, pApp);
	if ( xgeXuiWidgetAdd(pRoot, pWidget) != XGE_OK ) {
		xgeXuiAccordionUnit(pAccordion);
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	return pAccordion;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_xui_accordion pAccordion;
	xge_xui_widget pClient;
	int iIndex;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	pRoot = xgeXuiRoot(&pApp->tXui);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_GRID);
	xgeXuiWidgetSetGrid(pRoot, 3, 0.0f, 12.0f, 12.0f);
	xgeXuiWidgetSetPaddingPx(pRoot, 20.0f, 20.0f, 20.0f, 20.0f);
	xgeXuiWidgetSetBackground(pRoot, XGE_COLOR_RGBA(229, 235, 244, 255));

	pAccordion = AddAccordion(pApp, pRoot);
	if ( pAccordion == NULL ) {
		return XGE_ERROR;
	}
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Multiple section A", 1, 101);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Expanded client uses normal column layout.");
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Multiple section B", 0, 102);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Collapsed content should not occupy layout space.");
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Disabled header", 1, 103);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "The header is disabled but client remains visible.");
	xgeXuiAccordionSetSectionEnabled(pAccordion, iIndex, 0);

	pAccordion = AddAccordion(pApp, pRoot);
	if ( pAccordion == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiAccordionSetMode(pAccordion, XGE_XUI_ACCORDION_MODE_SINGLE);
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Single page one", 1, 201);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Single mode keeps only one expanded section.");
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Single page two", 1, 202);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Adding an expanded section collapses the earlier one.");
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Client action row", 0, 203);
	pClient = xgeXuiAccordionGetClientWidget(pAccordion, iIndex);
	xgeXuiWidgetSetLayout(pClient, XGE_XUI_LAYOUT_ROW);
	xgeXuiWidgetSetGap(pClient, 8.0f);
	AddButton(pApp, pClient, "Apply");
	AddButton(pApp, pClient, "Reset");

	pAccordion = AddAccordion(pApp, pRoot);
	if ( pAccordion == NULL ) {
		return XGE_ERROR;
	}
	xgeXuiAccordionSetMetrics(pAccordion, 32.0f, 6.0f, 10.0f);
	xgeXuiAccordionSetColors(pAccordion, XGE_COLOR_RGBA(244, 250, 252, 255), XGE_COLOR_RGBA(214, 232, 244, 255), 0, 0, XGE_COLOR_RGBA(236, 248, 252, 255), XGE_COLOR_RGBA(86, 154, 200, 255), XGE_COLOR_RGBA(36, 54, 72, 255));
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Custom colors", 1, 301);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Header, client and border colors are configurable.");
	iIndex = xgeXuiAccordionAddSection(pAccordion, "Custom padding", 1, 302);
	AddLabel(pApp, xgeXuiAccordionGetClientWidget(pAccordion, iIndex), "Content padding and gap are applied to client widgets.");
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
	if ( fRootW < 920.0f ) {
		fRootW = 920.0f;
	}
	if ( fRootH < 500.0f ) {
		fRootH = 500.0f;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, fRootW, fRootH });
	pApp->iLastWidth = iWidth;
	pApp->iLastHeight = iHeight;
}

static void RunChecks(app_state_t* pApp)
{
	xge_xui_accordion pMulti;
	xge_xui_accordion pSingle;
	float fExpandedHeight;
	float fCollapsedHeight;
	int iBeforeDisabledClick;

	pMulti = &pApp->tAccordion[0];
	pSingle = &pApp->tAccordion[1];
	fExpandedHeight = xgeXuiAccordionGetClientWidget(pMulti, 0)->tDesiredSize.fY;
	fCollapsedHeight = xgeXuiAccordionGetClientWidget(pSingle, 0)->tDesiredSize.fY;
	pApp->bCreateOK = (pApp->iAccordionCount == ACCORDION_COUNT) && (pApp->iLabelCount == 7) && (pApp->iButtonCount == 2);
	pApp->bLayoutOK = (xgeXuiRoot(&pApp->tXui)->tRect.fW >= 920.0f) && (fExpandedHeight > 0.0f) && (fCollapsedHeight == 0.0f);
	if ( pApp->bInteractionDone == 0 ) {
		ClickWidget(pApp, xgeXuiAccordionGetHeaderWidget(pMulti, 1));
		iBeforeDisabledClick = pApp->iSelectCount;
		ClickWidget(pApp, xgeXuiAccordionGetHeaderWidget(pMulti, 2));
		ClickWidget(pApp, xgeXuiAccordionGetHeaderWidget(pSingle, 2));
		pApp->bInteractionOK = xgeXuiAccordionIsExpanded(pMulti, 0) &&
			xgeXuiAccordionIsExpanded(pMulti, 1) &&
			xgeXuiAccordionIsExpanded(pMulti, 2) &&
			(pApp->iSelectCount == iBeforeDisabledClick + 1) &&
			!xgeXuiAccordionIsExpanded(pSingle, 0) &&
			!xgeXuiAccordionIsExpanded(pSingle, 1) &&
			xgeXuiAccordionIsExpanded(pSingle, 2);
		pApp->bInteractionDone = 1;
	}
	pApp->bStateOK = xgeXuiAccordionIsExpanded(pMulti, 0) &&
		xgeXuiAccordionIsExpanded(pMulti, 1) &&
		xgeXuiAccordionIsExpanded(pMulti, 2) &&
		((xgeXuiAccordionGetHeaderWidget(pMulti, 2)->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) &&
		!xgeXuiAccordionIsExpanded(pSingle, 0) &&
		!xgeXuiAccordionIsExpanded(pSingle, 1) &&
		xgeXuiAccordionIsExpanded(pSingle, 2) &&
		pApp->bInteractionOK;
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
	for ( i = 0; i < pApp->iAccordionCount; i++ ) {
		xgeXuiAccordionUnit(&pApp->tAccordion[i]);
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
		printf("xui_accordion final-summary frames=%d create=%d layout=%d state=%d interaction=%d accordions=%d\n", pApp->iFrameCount, pApp->bCreateOK, pApp->bLayoutOK, pApp->bStateOK, pApp->bInteractionOK, pApp->iAccordionCount);
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_ACCORDION_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 940;
	tDesc.iHeight = 520;
	tDesc.sTitle = "XUI Accordion";
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
