#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LAYER_COUNT 6

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_widget pTitleWidget;
	xge_xui_widget pTimelineWidget;
	xge_xui_label_t tTitle;
	xge_xui_timeline_view_t tTimeline;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iLastWidth;
	int iLastHeight;
	int iCurrentFrameChanging;
	int iCurrentFrameChanged;
	int iLayerChanged;
	int iFrameChanged;
	int iSpanChanged;
	int iLayerSelected;
	int iContextOpen;
	int iContextCommand;
	int iFrameClick;
	int iSelectionChanged;
	int iCustomPaint;
	int bCreateOK;
	int bLayoutOK;
	int bDataOK;
	int bInteractionOK;
	int bRendererOK;
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
			printf("xui_timelineview font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	return XGE_ERROR_RESOURCE_FAILED;
}

static xge_font AppFont(app_state_t* pApp)
{
	return pApp->bFontReady ? &pApp->tFont : NULL;
}

static int OnCurrentFrameChanging(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pApp = (app_state_t*)pUser;
	pApp->iCurrentFrameChanging++;
	return 1;
}

static void OnCurrentFrameChanged(xge_xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pApp = (app_state_t*)pUser;
	pApp->iCurrentFrameChanged++;
}

static int OnLayerChanging(xge_xui_widget pWidget, int iLayer, int iLayerId, int iChangeType, const char* sOldText, const char* sNewText, int iOldValue, int iNewValue, void* pUser)
{
	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iChangeType;
	(void)sOldText;
	(void)sNewText;
	(void)iOldValue;
	(void)iNewValue;
	(void)pUser;
	return 1;
}

static void OnLayerChanged(xge_xui_widget pWidget, int iLayer, int iLayerId, int iChangeType, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iChangeType;
	pApp = (app_state_t*)pUser;
	pApp->iLayerChanged++;
}

static int OnFrameChanging(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iOldType, int iNewType, void* pUser)
{
	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	(void)pUser;
	return 1;
}

static void OnFrameChanged(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iOldType, int iNewType, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	pApp = (app_state_t*)pUser;
	pApp->iFrameChanged++;
}

static int OnSpanChanging(xge_xui_widget pWidget, int iLayer, int iLayerId, int iSpanId, int iOldStartFrame, int iOldEndFrame, int iNewStartFrame, int iNewEndFrame, int iOldType, int iNewType, const char* sOldLabel, const char* sNewLabel, void* pUser)
{
	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iSpanId;
	(void)iOldStartFrame;
	(void)iOldEndFrame;
	(void)iNewStartFrame;
	(void)iNewEndFrame;
	(void)iOldType;
	(void)iNewType;
	(void)sOldLabel;
	(void)sNewLabel;
	(void)pUser;
	return 1;
}

static void OnSpanChanged(xge_xui_widget pWidget, int iLayer, int iLayerId, int iSpanId, int iOldStartFrame, int iOldEndFrame, int iNewStartFrame, int iNewEndFrame, int iOldType, int iNewType, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iSpanId;
	(void)iOldStartFrame;
	(void)iOldEndFrame;
	(void)iNewStartFrame;
	(void)iNewEndFrame;
	(void)iOldType;
	(void)iNewType;
	pApp = (app_state_t*)pUser;
	pApp->iSpanChanged++;
}

static void OnLayerSelected(xge_xui_widget pWidget, int iLayer, int iLayerId, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	pApp = (app_state_t*)pUser;
	pApp->iLayerSelected++;
}

static int OnContextOpening(xge_xui_widget pWidget, const xge_xui_timeline_hit_t* pHit, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)pHit;
	pApp = (app_state_t*)pUser;
	pApp->iContextOpen++;
	return 1;
}

static void OnContextCommand(xge_xui_widget pWidget, const xge_xui_timeline_hit_t* pHit, int iCommand, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)pHit;
	(void)iCommand;
	pApp = (app_state_t*)pUser;
	pApp->iContextCommand++;
}

static void OnFrameClick(xge_xui_widget pWidget, int iLayer, int iLayerId, int iFrame, int iModifiers, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iLayerId;
	(void)iFrame;
	(void)iModifiers;
	pApp = (app_state_t*)pUser;
	pApp->iFrameClick++;
}

static void OnSelection(xge_xui_widget pWidget, int iSelectionCount, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iSelectionCount;
	pApp = (app_state_t*)pUser;
	pApp->iSelectionChanged++;
}

static int CustomFrameRenderer(xge_xui_widget pWidget, int iLayer, int iFrame, const xge_xui_timeline_frame_t* pFrame, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)pFrame;
	(void)tRect;
	(void)iState;
	pApp = (app_state_t*)pUser;
	pApp->iCustomPaint++;
	return 0;
}

static xge_xui_widget NewWidget(float fHeight)
{
	xge_xui_widget pWidget;

	pWidget = xgeXuiWidgetCreate();
	if ( pWidget != NULL ) {
		xgeXuiWidgetSetSize(pWidget, xgeXuiSizePercent(100.0f), (fHeight > 0.0f) ? xgeXuiSizePx(fHeight) : xgeXuiSizeGrow(1.0f));
	}
	return pWidget;
}

static int AddTitle(app_state_t* pApp, xge_xui_widget pRoot)
{
	xge_xui_widget pWidget;

	pWidget = NewWidget(32.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiLabelInit(&pApp->tTitle, pWidget, AppFont(pApp), "TimelineView: layers, key frames, spans, selection, locking, and horizontal zoom") != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tTitle, XGE_COLOR_RGBA(42, 58, 78, 255));
	xgeXuiLabelSetAlign(&pApp->tTitle, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pRoot, pWidget);
	pApp->pTitleWidget = pWidget;
	return XGE_OK;
}

static int AddTimeline(app_state_t* pApp, xge_xui_widget pRoot)
{
	xge_xui_widget pWidget;
	int arrLayer[LAYER_COUNT];

	pWidget = NewWidget(0.0f);
	if ( pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 251, 255, 255));
	xgeXuiWidgetSetBorder(pWidget, 1.0f, XGE_COLOR_RGBA(156, 184, 210, 255));
	if ( xgeXuiTimelineViewInit(&pApp->tTimeline, &pApp->tXui, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return XGE_ERROR;
	}
	xgeXuiTimelineViewSetFont(&pApp->tTimeline, AppFont(pApp));
	xgeXuiTimelineViewSetFrameCount(&pApp->tTimeline, 180);
	xgeXuiTimelineViewSetFrameRate(&pApp->tTimeline, 24);
	xgeXuiTimelineViewSetMetrics(&pApp->tTimeline, 170.0f, 12.0f, 24.0f, 26.0f);
	xgeXuiTimelineViewSetCurrentFrameProc(&pApp->tTimeline, OnCurrentFrameChanging, OnCurrentFrameChanged, pApp);
	xgeXuiTimelineViewSetLayerProc(&pApp->tTimeline, OnLayerChanging, OnLayerChanged, pApp);
	xgeXuiTimelineViewSetFrameProc(&pApp->tTimeline, OnFrameChanging, OnFrameChanged, pApp);
	xgeXuiTimelineViewSetSpanProc(&pApp->tTimeline, OnSpanChanging, OnSpanChanged, pApp);
	xgeXuiTimelineViewSetLayerSelectedProc(&pApp->tTimeline, OnLayerSelected, pApp);
	xgeXuiTimelineViewSetContextMenuProc(&pApp->tTimeline, OnContextOpening, OnContextCommand, pApp);
	xgeXuiTimelineViewSetFrameClick(&pApp->tTimeline, OnFrameClick, pApp);
	xgeXuiTimelineViewSetSelectionProc(&pApp->tTimeline, OnSelection, pApp);
	xgeXuiTimelineViewSetRenderers(&pApp->tTimeline, NULL, NULL, NULL, NULL, CustomFrameRenderer, pApp, NULL, NULL);
	arrLayer[0] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 101, "camera");
	arrLayer[1] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 102, "character");
	arrLayer[2] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 103, "shadow");
	arrLayer[3] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 104, "effects");
	arrLayer[4] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 105, "locked guide");
	arrLayer[5] = xgeXuiTimelineViewAddLayer(&pApp->tTimeline, 106, "hidden reference");
	xgeXuiTimelineViewSetLayerVisible(&pApp->tTimeline, arrLayer[5], 0);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[0], 0, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[0], 24, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[1], 4, XGE_XUI_TIMELINE_FRAME_BLANK_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[1], 12, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[2], 16, XGE_XUI_TIMELINE_FRAME_NORMAL);
	xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, arrLayer[3], 30, XGE_XUI_TIMELINE_FRAME_KEY);
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, arrLayer[0], 201, 0, 24, XGE_XUI_TIMELINE_SPAN_MOTION, "camera pan");
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, arrLayer[1], 202, 12, 48, XGE_XUI_TIMELINE_SPAN_MOTION, "walk cycle");
	xgeXuiTimelineViewAddSpan(&pApp->tTimeline, arrLayer[3], 203, 30, 58, XGE_XUI_TIMELINE_SPAN_EVENT, "spark");
	xgeXuiTimelineViewSetLayerLocked(&pApp->tTimeline, arrLayer[4], 1);
	xgeXuiTimelineViewEnsureFrameVisible(&pApp->tTimeline, arrLayer[1], 42);
	xgeXuiWidgetAdd(pRoot, pWidget);
	pApp->pTimelineWidget = pWidget;
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, AppFont(pApp));
	XgeXuiDemoApplyRootPanel(pRoot);
	xgeXuiWidgetSetLayout(pRoot, XGE_XUI_LAYOUT_COLUMN);
	xgeXuiWidgetSetPaddingPx(pRoot, 18.0f, 18.0f, 18.0f, 18.0f);
	xgeXuiWidgetSetGap(pRoot, 10.0f);
	if ( AddTitle(pApp, pRoot) != XGE_OK || AddTimeline(pApp, pRoot) != XGE_OK ) {
		return XGE_ERROR;
	}
	return XGE_OK;
}

static void LayoutRoot(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	int iW;
	int iH;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return;
	}
	iW = xgeGetWidth();
	iH = xgeGetHeight();
	if ( (iW != pApp->iLastWidth) || (iH != pApp->iLastHeight) ) {
		xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, (float)iW, (float)iH });
		pApp->iLastWidth = iW;
		pApp->iLastHeight = iH;
	}
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY, int iModifiers)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->iParam2 = iModifiers;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->iPointerId = 1;
}

static void MakeWheelEvent(xge_event_t* pEvent, float fX, float fY, float fWheelY, int iModifiers)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->iParam2 = iModifiers;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDY = fWheelY;
	pEvent->iPointerId = 1;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey, int iModifiers)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
	pEvent->iParam2 = iModifiers;
}

static void FramePoint(app_state_t* pApp, int iLayer, int iFrame, float* pX, float* pY)
{
	xge_rect_t tViewport;
	float fX;
	float fY;
	float fTop;
	int i;

	tViewport = xgeXuiScrollFrameGetViewportRect(pApp->tTimeline.pFrame);
	fTop = 0.0f;
	for ( i = 0; i < iLayer; i++ ) {
		fTop += xgeXuiTimelineViewGetLayer(&pApp->tTimeline, i)->fHeight;
	}
	fX = tViewport.fX + pApp->tTimeline.fLayerHeaderWidth + (float)iFrame * pApp->tTimeline.fFrameWidth - pApp->tTimeline.tScroll.fScrollX + pApp->tTimeline.fFrameWidth * 0.5f;
	fY = tViewport.fY + pApp->tTimeline.fRulerHeight + fTop - pApp->tTimeline.tScroll.fScrollY + xgeXuiTimelineViewGetLayer(&pApp->tTimeline, iLayer)->fHeight * 0.5f;
	if ( pX != NULL ) {
		*pX = fX;
	}
	if ( pY != NULL ) {
		*pY = fY;
	}
}

static void LayerNamePoint(app_state_t* pApp, int iLayer, float* pX, float* pY)
{
	xge_rect_t tViewport;
	float fTop;
	int i;

	tViewport = xgeXuiScrollFrameGetViewportRect(pApp->tTimeline.pFrame);
	fTop = 0.0f;
	for ( i = 0; i < iLayer; i++ ) {
		fTop += xgeXuiTimelineViewGetLayer(&pApp->tTimeline, i)->fHeight;
	}
	if ( pX != NULL ) {
		*pX = tViewport.fX + 10.0f;
	}
	if ( pY != NULL ) {
		*pY = tViewport.fY + pApp->tTimeline.fRulerHeight + fTop - pApp->tTimeline.tScroll.fScrollY + xgeXuiTimelineViewGetLayer(&pApp->tTimeline, iLayer)->fHeight * 0.5f;
	}
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_tooltip_desc_t tTooltip;
	xge_xui_timeline_hit_t tHit;
	xge_xui_timeline_span_t* pSpan;
	xge_rect_t tViewport;
	float fX1;
	float fY1;
	float fX2;
	float fY2;
	float fLayerX;
	float fLayerY;
	float fBlankX;
	float fBlankY;
	float fLayerTotal;
	float fOldFrameWidth;
	float fZoomFrameWidth;
	int iOverlapResult;
	int iLockedResult;
	int iRenameLockedResult;
	int iSetSpanResult;
	int iUserDataResult;
	int iHitResult;
	int iFeatureResult;
	int iMenuResult;
	int iBlankCloseResult;
	int iSpanSelectionMenuResult;
	int iRenameResult;
	int iTooltipResult;
	int i;

	pApp->bCreateOK =
		(pApp->pTimelineWidget != NULL) &&
		(pApp->tTimeline.pFrame != NULL) &&
		(xgeXuiTimelineViewGetLayerCount(&pApp->tTimeline) == LAYER_COUNT) &&
		(xgeXuiTimelineViewGetFrameCount(&pApp->tTimeline) == 180);
	pApp->bLayoutOK =
		(pApp->pTimelineWidget != NULL) &&
		(pApp->pTimelineWidget->tRect.fW > 400.0f) &&
		(pApp->pTimelineWidget->tRect.fH > 260.0f) &&
		(xgeXuiScrollFrameGetViewportRect(pApp->tTimeline.pFrame).fW > 200.0f);
	iSetSpanResult = xgeXuiTimelineViewSetSpan(&pApp->tTimeline, 1, 202, 12, 52, XGE_XUI_TIMELINE_SPAN_MOTION, "walk extended");
	pSpan = xgeXuiTimelineViewGetSpan(&pApp->tTimeline, 1, 202);
	iUserDataResult =
		(xgeXuiTimelineViewSetLayerUserData(&pApp->tTimeline, 0, pApp) == XGE_OK) &&
		(xgeXuiTimelineViewGetLayerUserData(&pApp->tTimeline, 0) == pApp) &&
		(xgeXuiTimelineViewSetFrameUserData(&pApp->tTimeline, 1, 12, pApp) == XGE_OK) &&
		(xgeXuiTimelineViewGetFrameUserData(&pApp->tTimeline, 1, 12) == pApp) &&
		(xgeXuiTimelineViewSetSpanUserData(&pApp->tTimeline, 1, 202, pApp) == XGE_OK) &&
		(xgeXuiTimelineViewGetSpanUserData(&pApp->tTimeline, 1, 202) == pApp);
	xgeXuiTimelineViewSetFeatureFlags(&pApp->tTimeline, 0, 0);
	iFeatureResult = (pApp->tTimeline.bShowVisibilityFeature == 0) && (pApp->tTimeline.bShowLockFeature == 0);
	xgeXuiTimelineViewSetFeatureFlags(&pApp->tTimeline, 1, 1);
	iOverlapResult = xgeXuiTimelineViewAddSpan(&pApp->tTimeline, 1, 301, 20, 28, XGE_XUI_TIMELINE_SPAN_MOTION, "overlap rejected");
	iLockedResult = xgeXuiTimelineViewSetFrameType(&pApp->tTimeline, 4, 12, XGE_XUI_TIMELINE_FRAME_KEY);
	iRenameLockedResult = xgeXuiTimelineViewSetLayerName(&pApp->tTimeline, 4, "locked guide renamed");
	xgeXuiTimelineViewSetCurrentFrame(&pApp->tTimeline, 42);
	xgeXuiTimelineViewSelectRange(&pApp->tTimeline, 1, 12, 3, 18);
	xgeXuiTimelineViewEnsureFrameVisible(&pApp->tTimeline, 2, 60);
	FramePoint(pApp, 1, 60, &fX1, &fY1);
	FramePoint(pApp, 3, 66, &fX2, &fY2);
	iHitResult = xgeXuiTimelineViewHitTest(&pApp->tTimeline, fX1, fY1, &tHit) &&
		(tHit.iType == XGE_XUI_TIMELINE_HIT_FRAME) &&
		(tHit.iLayer == 1) &&
		(tHit.iFrame == 60);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, fX1, fY1, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, fX2, fY2, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, fX2, fY2, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT, XGE_KEY_MOD_SHIFT);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	LayerNamePoint(pApp, 0, &fLayerX, &fLayerY);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_MENU, fLayerX, fLayerY, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	iMenuResult = (xgeXuiMenuIsOpen(pApp->tTimeline.pLayerMenu) != 0);
	if ( pApp->tTimeline.pLayerMenu != NULL && pApp->tTimeline.pLayerMenu->procSelect != NULL ) {
		pApp->tTimeline.pLayerMenu->procSelect(pApp->pTimelineWidget, 0, XGE_XUI_TIMELINE_MENU_LAYER_SHOW_HIDE, pApp->tTimeline.pLayerMenu->pUser);
		xgeXuiTimelineViewSetLayerVisible(&pApp->tTimeline, 0, 1);
	}
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_MENU, fX1, fY1, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	iMenuResult = iMenuResult && (xgeXuiMenuIsOpen(pApp->tTimeline.pFrameMenu) != 0);
	if ( pApp->tTimeline.pFrameMenu != NULL && pApp->tTimeline.pFrameMenu->procSelect != NULL ) {
		pApp->tTimeline.pFrameMenu->procSelect(pApp->pTimelineWidget, 0, XGE_XUI_TIMELINE_MENU_FRAME_INSERT, pApp->tTimeline.pFrameMenu->pUser);
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pApp->tTimeline.pFrame);
	fLayerTotal = 0.0f;
	for ( i = 0; i < xgeXuiTimelineViewGetLayerCount(&pApp->tTimeline); i++ ) {
		fLayerTotal += xgeXuiTimelineViewGetLayer(&pApp->tTimeline, i)->fHeight;
	}
	fBlankX = tViewport.fX + pApp->tTimeline.fLayerHeaderWidth + 12.0f;
	fBlankY = tViewport.fY + pApp->tTimeline.fRulerHeight + fLayerTotal - pApp->tTimeline.tScroll.fScrollY + 10.0f;
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_MENU, fX1, fY1, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, fBlankX, fBlankY, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	iBlankCloseResult = (xgeXuiMenuIsOpen(pApp->tTimeline.pLayerMenu) == 0) && (xgeXuiMenuIsOpen(pApp->tTimeline.pFrameMenu) == 0);
	xgeXuiTimelineViewSelectRange(&pApp->tTimeline, 2, 70, 2, 74);
	xgeXuiTimelineViewEnsureFrameVisible(&pApp->tTimeline, 2, 72);
	FramePoint(pApp, 2, 72, &fX2, &fY2);
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_CONTEXT_MENU, fX2, fY2, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	iSpanSelectionMenuResult = (xgeXuiMenuIsOpen(pApp->tTimeline.pFrameMenu) != 0);
	if ( pApp->tTimeline.pFrameMenu != NULL && pApp->tTimeline.pFrameMenu->procSelect != NULL ) {
		pApp->tTimeline.pFrameMenu->procSelect(pApp->pTimelineWidget, 0, XGE_XUI_TIMELINE_MENU_SPAN_CREATE, pApp->tTimeline.pFrameMenu->pUser);
	}
	FramePoint(pApp, 2, 72, &fX2, &fY2);
	iSpanSelectionMenuResult = iSpanSelectionMenuResult &&
		xgeXuiTimelineViewHitTest(&pApp->tTimeline, fX2, fY2, &tHit) &&
		(tHit.iType == XGE_XUI_TIMELINE_HIT_SPAN) &&
		(tHit.iLayer == 2) &&
		(tHit.iFrame == 70);
	fOldFrameWidth = pApp->tTimeline.fFrameWidth;
	MakeWheelEvent(&tEvent, fX2, fY2, 1.0f, XGE_KEY_MOD_CTRL);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	fZoomFrameWidth = pApp->tTimeline.fFrameWidth;
	MakeMouseEvent(&tEvent, XGE_EVENT_XUI_DOUBLE_CLICK, fLayerX, fLayerY, 0);
	xgeXuiTimelineViewEvent(&pApp->tTimeline, &tEvent);
	iRenameResult = xgeXuiInputBoxIsOpen(pApp->tTimeline.pRenameBox);
	xgeXuiInputBoxSetOpen(pApp->tTimeline.pRenameBox, 0);
	memset(&tTooltip, 0, sizeof(tTooltip));
	pApp->tXui.fTooltipMouseX = fLayerX;
	pApp->tXui.fTooltipMouseY = fLayerY;
	iTooltipResult = (pApp->pTimelineWidget->procTooltipResolve != NULL) &&
		pApp->pTimelineWidget->procTooltipResolve(&pApp->tXui, pApp->pTimelineWidget, &tTooltip, pApp->pTimelineWidget->pTooltipUser) &&
		(tTooltip.iType == XGE_XUI_TOOLTIP_TEXT) &&
		(tTooltip.sText != NULL) &&
		(strstr(tTooltip.sText, "Layer") != NULL);
	pApp->bDataOK =
		(iSetSpanResult == XGE_OK) &&
		(pSpan != NULL) &&
		(pSpan->iEndFrame == 52) &&
		(strcmp(pSpan->sLabel, "walk extended") == 0) &&
		iUserDataResult &&
		iFeatureResult &&
		(iOverlapResult < 0) &&
		(iLockedResult != XGE_OK) &&
		(iRenameLockedResult == XGE_OK) &&
		(xgeXuiTimelineViewGetFrameType(&pApp->tTimeline, 1, 12) == XGE_XUI_TIMELINE_FRAME_KEY) &&
		(xgeXuiTimelineViewGetFrameType(&pApp->tTimeline, 1, 4) == XGE_XUI_TIMELINE_FRAME_BLANK_KEY) &&
		(xgeXuiTimelineViewGetLayer(&pApp->tTimeline, 5)->bVisible == 0) &&
		(xgeXuiTimelineViewGetLayer(&pApp->tTimeline, 4)->bLocked != 0);
	pApp->bInteractionOK =
		iHitResult &&
		iMenuResult &&
		iBlankCloseResult &&
		iSpanSelectionMenuResult &&
		(fZoomFrameWidth > fOldFrameWidth) &&
		iRenameResult &&
		iTooltipResult &&
		(xgeXuiTimelineViewGetCurrentFrame(&pApp->tTimeline) == 61) &&
		(xgeXuiTimelineViewGetSelectionCount(&pApp->tTimeline) >= 2) &&
		(pApp->iCurrentFrameChanging > 0) &&
		(pApp->iCurrentFrameChanged > 0) &&
		(pApp->iSpanChanged > 0) &&
		(pApp->iLayerSelected > 0) &&
		(pApp->iContextOpen >= 2) &&
		(pApp->iContextCommand >= 2) &&
		(pApp->iFrameClick > 0) &&
		(pApp->iSelectionChanged > 0);
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

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiTimelineViewUnit(&pApp->tTimeline);
	xgeXuiLabelUnit(&pApp->tTitle);
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
	pApp->bRendererOK = pApp->iCustomPaint > 0;
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf("xui_timelineview final-summary frames=%d create=%d layout=%d data=%d interaction=%d renderer=%d current=%d selection=%d frameClick=%d spanChanged=%d layerSelected=%d menuOpen=%d menuCommand=%d customPaint=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bLayoutOK,
			pApp->bDataOK,
			pApp->bInteractionOK,
			pApp->bRendererOK,
			xgeXuiTimelineViewGetCurrentFrame(&pApp->tTimeline),
			xgeXuiTimelineViewGetSelectionCount(&pApp->tTimeline),
			pApp->iFrameClick,
			pApp->iSpanChanged,
			pApp->iLayerSelected,
			pApp->iContextOpen,
			pApp->iContextCommand,
			pApp->iCustomPaint);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(236, 240, 246, 255));
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
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_TIMELINEVIEW_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 1040;
	tDesc.iHeight = 620;
	tDesc.sTitle = "XUI TimelineView";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bLayoutOK && tApp.bDataOK && tApp.bInteractionOK && tApp.bRendererOK) ? 0 : 3;
}
