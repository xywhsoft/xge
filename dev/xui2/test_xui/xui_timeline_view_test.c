#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_timeline_view_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct timeline_test_data_t {
	int iCurrentChanging;
	int iCurrentChanged;
	int iLayerChanging;
	int iLayerChanged;
	int iFrameChanging;
	int iFrameChanged;
	int iSpanChanging;
	int iSpanChanged;
	int iLayerSelected;
	int iContextOpen;
	int iContextCommand;
	int iFrameClick;
	int iFrameDoubleClick;
	int iSelectionChanged;
	int iRenderFrame;
	int iRenderLayer;
} timeline_test_data_t;

static int __xuiTimelineTestRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;
	tFullRect = (xui_rect_i_t){0, 0, 760, 420};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTimelineTestClickEx(xui_context pContext, float fX, float fY, uint32_t iModifiers);

static int __xuiTimelineTestClick(xui_context pContext, float fX, float fY)
{
	return __xuiTimelineTestClickEx(pContext, fX, fY, 0u);
}

static int __xuiTimelineTestClickEx(xui_context pContext, float fX, float fY, uint32_t iModifiers)
{
	int iRet;
	int iReset;
	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iReset = xuiInputSetModifiers(pContext, 0u);
	if ( iRet == XUI_OK ) iRet = iReset;
	return iRet;
}

static int __xuiTimelineTestDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1, uint32_t iModifiers)
{
	int iRet;
	int iReset;
	iRet = xuiInputSetModifiers(pContext, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fX0, fY0, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iReset = xuiInputSetModifiers(pContext, 0u);
	if ( iRet == XUI_OK ) iRet = iReset;
	return iRet;
}

static int __xuiTimelineCurrentChanging(xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iCurrentChanging++;
	return 1;
}

static void __xuiTimelineCurrentChanged(xui_widget pWidget, int iOldFrame, int iNewFrame, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iOldFrame;
	(void)iNewFrame;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iCurrentChanged++;
}

static int __xuiTimelineLayerChanging(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iChange;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iLayerChanging++;
	return 1;
}

static void __xuiTimelineLayerChanged(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iChange;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iLayerChanged++;
}

static int __xuiTimelineFrameChanging(xui_widget pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iFrameChanging++;
	return 1;
}

static void __xuiTimelineFrameChanged(xui_widget pWidget, int iLayer, int iFrame, int iOldType, int iNewType, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iOldType;
	(void)iNewType;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iFrameChanged++;
}

static int __xuiTimelineSpanChanging(xui_widget pWidget, int iSpanId, int iChange, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iSpanId;
	(void)iChange;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iSpanChanging++;
	return 1;
}

static void __xuiTimelineSpanChanged(xui_widget pWidget, int iSpanId, int iChange, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iSpanId;
	(void)iChange;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iSpanChanged++;
}

static void __xuiTimelineLayerSelected(xui_widget pWidget, int iLayer, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iLayerSelected++;
}

static int __xuiTimelineContextOpening(xui_widget pWidget, const xui_timeline_hit_t* pHit, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)pHit;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iContextOpen++;
	return 1;
}

static void __xuiTimelineContextCommand(xui_widget pWidget, int iCommand, const xui_timeline_hit_t* pHit, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iCommand;
	(void)pHit;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iContextCommand++;
}

static void __xuiTimelineFrameClick(xui_widget pWidget, int iLayer, int iFrame, int iButton, int iModifiers, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iButton;
	(void)iModifiers;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iFrameClick++;
}

static void __xuiTimelineFrameDoubleClick(xui_widget pWidget, int iLayer, int iFrame, int iButton, int iModifiers, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)iButton;
	(void)iModifiers;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iFrameDoubleClick++;
}

static void __xuiTimelineSelectionChanged(xui_widget pWidget, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iSelectionChanged++;
}

static int __xuiTimelineLayerRender(xui_widget pWidget, int iLayer, const xui_timeline_layer_t* pLayer, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)pLayer;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iRenderLayer++;
	return 0;
}

static int __xuiTimelineFrameRender(xui_widget pWidget, int iLayer, int iFrame, const xui_timeline_frame_t* pFrame, xui_draw_context pDraw, xui_rect_t tRect, int iState, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iFrame;
	(void)pFrame;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) pData->iRenderFrame++;
	return 0;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTimeline;
	xui_surface pTarget;
	xui_font pFont;
	xui_timeline_view_desc_t tDesc;
	xui_timeline_frame_t tFrame;
	xui_timeline_span_t tSpan;
	xui_timeline_layer_t tLayer;
	xui_timeline_selection_t tSelection;
	xui_timeline_hit_t tHit;
	timeline_test_data_t tData;
	int iLayerBg;
	int iLayerFx;
	int iLayerCam;
	int iSpan;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTimeline = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tData, 0, sizeof(tData));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "timeline", 8, 13.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 760.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 420.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iFrameCount = 180;
	tDesc.fFrameRate = 24.0f;
	tDesc.fLayerHeaderWidth = 150.0f;
	tDesc.fFrameWidth = 10.0f;
	tDesc.fRowHeight = 24.0f;
	tDesc.fRulerHeight = 26.0f;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	iRet = xuiTimeLineViewCreate(pContext, &pTimeline, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTimeline != NULL, "timeline create");
	xuiWidgetSetRect(pTimeline, (xui_rect_t){20.0f, 20.0f, 700.0f, 260.0f});
	iRet = xuiWidgetAddChild(pRoot, pTimeline);
	XUI_TEST_CHECK(iRet == XUI_OK, "timeline add");

	iRet = xuiTimeLineViewSetCurrentFrameCallbacks(pTimeline, __xuiTimelineCurrentChanging, __xuiTimelineCurrentChanged, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "current callbacks");
	iRet = xuiTimeLineViewSetLayerCallbacks(pTimeline, __xuiTimelineLayerChanging, __xuiTimelineLayerChanged, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "layer callbacks");
	iRet = xuiTimeLineViewSetFrameCallbacks(pTimeline, __xuiTimelineFrameChanging, __xuiTimelineFrameChanged, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame callbacks");
	iRet = xuiTimeLineViewSetSpanCallbacks(pTimeline, __xuiTimelineSpanChanging, __xuiTimelineSpanChanged, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "span callbacks");
	iRet = xuiTimeLineViewSetLayerSelected(pTimeline, __xuiTimelineLayerSelected, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "layer selected callback");
	iRet = xuiTimeLineViewSetContextMenu(pTimeline, __xuiTimelineContextOpening, __xuiTimelineContextCommand, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callbacks");
	iRet = xuiTimeLineViewSetFrameClick(pTimeline, __xuiTimelineFrameClick, __xuiTimelineFrameDoubleClick, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "click callbacks");
	iRet = xuiTimeLineViewSetSelectionChange(pTimeline, __xuiTimelineSelectionChanged, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "selection callback");
	iRet = xuiTimeLineViewSetRenderers(pTimeline, __xuiTimelineLayerRender, NULL, __xuiTimelineFrameRender, NULL, &tData);
	XUI_TEST_CHECK(iRet == XUI_OK, "render callbacks");

	iRet = xuiTimeLineViewAddLayer(pTimeline, "Background", &iLayerBg);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerBg == 0, "layer bg");
	iRet = xuiTimeLineViewAddLayer(pTimeline, "Effects", &iLayerFx);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerFx == 1, "layer fx");
	iRet = xuiTimeLineViewAddLayer(pTimeline, "Camera", &iLayerCam);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayerCam == 2, "layer camera");
	XUI_TEST_CHECK(xuiTimeLineViewGetLayerCount(pTimeline) == 3, "layer count");

	iRet = xuiTimeLineViewSetLayerColor(pTimeline, iLayerFx, XUI_COLOR_RGBA(36, 173, 118, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "layer color");
	iRet = xuiTimeLineViewSetLayerVisible(pTimeline, iLayerFx, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetLayerVisible(pTimeline, iLayerFx) == 0, "layer visible toggle");
	iRet = xuiTimeLineViewSetLayerLocked(pTimeline, iLayerCam, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetLayerLocked(pTimeline, iLayerCam) == 1, "layer lock toggle");
	iRet = xuiTimeLineViewGetLayer(pTimeline, iLayerFx, &tLayer);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLayer.sName, "Effects") == 0, "get layer");

	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerBg, 0, XUI_TIMELINE_FRAME_KEY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame key");
	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerBg, 8, XUI_TIMELINE_FRAME_NORMAL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame normal");
	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerFx, 12, XUI_TIMELINE_FRAME_BLANK_KEY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame blank key");
	iRet = xuiTimeLineViewGetFrame(pTimeline, iLayerFx, 12, &tFrame);
	XUI_TEST_CHECK(iRet == XUI_OK && tFrame.iType == XUI_TIMELINE_FRAME_BLANK_KEY, "get frame");

	iRet = xuiTimeLineViewAddSpan(pTimeline, iLayerBg, 0, 20, XUI_TIMELINE_SPAN_HOLD, "intro", &iSpan);
	XUI_TEST_CHECK(iRet == XUI_OK && iSpan > 0, "add span");
	iRet = xuiTimeLineViewSetSpanColor(pTimeline, iSpan, XUI_COLOR_RGBA(85, 133, 219, 210));
	XUI_TEST_CHECK(iRet == XUI_OK, "span color");
	iRet = xuiTimeLineViewSetSpanCustomType(pTimeline, iSpan, "clip");
	XUI_TEST_CHECK(iRet == XUI_OK, "span custom type");
	iRet = xuiTimeLineViewGetSpan(pTimeline, iSpan, &tSpan);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tSpan.sLabel, "intro") == 0 && strcmp(tSpan.sCustomType, "clip") == 0, "get span");

	iRet = xuiTimeLineViewSetCurrentFrame(pTimeline, 8);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 8, "current frame");
	iRet = xuiTimeLineViewSelectRange(pTimeline, iLayerBg, 2, iLayerFx, 4, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 6, "range selection");
	iRet = xuiTimeLineViewGetSelection(pTimeline, 0, &tSelection);
	XUI_TEST_CHECK(iRet == XUI_OK && tSelection.iLayer >= 0, "get selection");
	XUI_TEST_CHECK(xuiTimeLineViewIsFrameSelected(pTimeline, iLayerBg, 2) == 1, "selection contains");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 760, 420, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "surface create");
	iRet = __xuiTimelineTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iRenderFrame > 0 && tData.iRenderLayer > 0, "render callbacks fire");

	iRet = xuiTimeLineViewHitTest(pTimeline, 150.0f + 8.0f * 10.0f + 5.0f, 26.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && (tHit.iType == XUI_TIMELINE_HIT_SPAN || tHit.iType == XUI_TIMELINE_HIT_FRAME || tHit.iType == XUI_TIMELINE_HIT_SELECTION) && tHit.iFrame == 8, "hit frame");
	iRet = xuiTimeLineViewHitTest(pTimeline, 114.0f, 26.0f + 24.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_TIMELINE_HIT_LAYER_LOCK && tHit.iLayer == iLayerFx, "hit lock icon");

	iRet = __xuiTimelineTestClick(pContext, 20.0f + 150.0f + 5.0f * 10.0f + 5.0f, 20.0f + 26.0f + 24.0f + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 5 && tData.iFrameClick > 0, "click frame");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 6, "keyboard right");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_CONTEXT_MENU, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextOpen > 0, "context opening");
	iRet = xuiTimeLineViewRunContextCommand(pTimeline, XUI_TIMELINE_MENU_FRAME_KEY);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCommand > 0, "context command");

	iRet = __xuiTimelineTestClickEx(pContext, 20.0f + 150.0f + 8.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 4 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerFx, 5) == 1 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerFx, 8) == 1, "shift click range selection");
	iRet = __xuiTimelineTestClickEx(pContext, 20.0f + 150.0f + 2.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerCam * 24.0f + 12.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 5 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerCam, 2) == 1, "ctrl click adds selection");
	iRet = __xuiTimelineTestClickEx(pContext, 20.0f + 150.0f + 2.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerCam * 24.0f + 12.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 4 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerCam, 2) == 0, "ctrl click removes selection");
	iRet = __xuiTimelineTestDrag(pContext,
		20.0f + 150.0f + 1.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerBg * 24.0f + 12.0f,
		20.0f + 150.0f + 4.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerBg * 24.0f + 12.0f, 0u);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 4 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerBg, 1) == 1 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerBg, 4) == 1 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerFx, 5) == 0, "drag range selection");
	iRet = __xuiTimelineTestDrag(pContext,
		20.0f + 150.0f + 10.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f,
		20.0f + 150.0f + 12.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pTimeline) == 7 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerBg, 1) == 1 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerFx, 10) == 1 &&
		xuiTimeLineViewIsFrameSelected(pTimeline, iLayerFx, 12) == 1, "ctrl drag adds range selection");

	iRet = xuiTimeLineViewEnsureFrameVisible(pTimeline, iLayerBg, 90);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure visible");
	iRet = xuiTimeLineViewSetOffset(pTimeline, 30.0f, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset");
	iRet = xuiTimeLineViewGetOffset(pTimeline, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "get offset accepts nulls");

	XUI_TEST_CHECK(tData.iCurrentChanging > 0 && tData.iCurrentChanged > 0, "current callbacks fired");
	XUI_TEST_CHECK(tData.iLayerChanging >= 2 && tData.iLayerChanged >= 2, "layer callbacks fired");
	XUI_TEST_CHECK(tData.iFrameChanging >= 3 && tData.iFrameChanged >= 3, "frame callbacks fired");
	XUI_TEST_CHECK(tData.iSpanChanging >= 1 && tData.iSpanChanged >= 1, "span callbacks fired");
	XUI_TEST_CHECK(tData.iSelectionChanged > 0, "selection callback fired");
	XUI_TEST_CHECK(xuiTimeLineViewGetChangeCount(pTimeline) > 0, "change count");

cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_timeline_view_test passed\n");
	return 0;
}
