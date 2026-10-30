#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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
	int iLastLayerChanging;
	int iLastLayerChanged;
	int bAllowLayerSelect;
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

typedef struct timeline_destroy_data_t {
	xui_widget* ppWidget;
	int iCalls;
} timeline_destroy_data_t;

typedef struct timeline_mutate_render_data_t {
	int iCalls;
	int iMutationResult;
} timeline_mutate_render_data_t;

static int __xuiTimelineDestroyChanging(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	timeline_destroy_data_t* pData = (timeline_destroy_data_t*)pUser;
	(void)iLayer;
	(void)iChange;
	if ( pData != NULL ) {
		pData->iCalls++;
		(void)xuiWidgetDestroy(pWidget);
		if ( pData->ppWidget != NULL ) *pData->ppWidget = NULL;
	}
	return 1;
}

static int __xuiTimelineDestroyRender(xui_widget pWidget, int iLayer,
	const xui_timeline_layer_t* pLayer, xui_draw_context pDraw, xui_rect_t tRect,
	int iState, void* pUser)
{
	timeline_destroy_data_t* pData = (timeline_destroy_data_t*)pUser;
	(void)iLayer;
	(void)pLayer;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	if ( pData != NULL ) {
		pData->iCalls++;
		(void)xuiWidgetDestroy(pWidget);
		if ( pData->ppWidget != NULL ) *pData->ppWidget = NULL;
	}
	return 1;
}

static int __xuiTimelineMutateLayerRender(xui_widget pWidget, int iLayer,
	const xui_timeline_layer_t* pLayer, xui_draw_context pDraw, xui_rect_t tRect,
	int iState, void* pUser)
{
	timeline_mutate_render_data_t* pData = (timeline_mutate_render_data_t*)pUser;
	(void)pLayer;
	(void)pDraw;
	(void)tRect;
	(void)iState;
	if ( pData != NULL ) {
		pData->iCalls++;
		if ( pData->iCalls == 1 ) {
			pData->iMutationResult = xuiTimeLineViewSetLayerColor(pWidget, iLayer,
				XUI_COLOR_RGBA(17, 34, 51, 255));
		}
	}
	return 0;
}

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

static int __xuiTimelineTestRightClick(xui_context pContext, float fX, float fY)
{
	int iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
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
	if ( pData != NULL ) {
		pData->iLayerChanging++;
		pData->iLastLayerChanging = iChange;
		if ( iChange == XUI_TIMELINE_LAYER_CHANGE_SELECT && !pData->bAllowLayerSelect ) return 0;
	}
	return 1;
}

static void __xuiTimelineLayerChanged(xui_widget pWidget, int iLayer, int iChange, void* pUser)
{
	timeline_test_data_t* pData;
	(void)pWidget;
	(void)iLayer;
	(void)iChange;
	pData = (timeline_test_data_t*)pUser;
	if ( pData != NULL ) {
		pData->iLayerChanged++;
		pData->iLastLayerChanged = iChange;
	}
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
	xui_widget pScratch;
	xui_widget pDestroyChanging;
	xui_widget pDestroyRender;
	xui_surface pTarget;
	xui_surface pTimelineCache;
	xui_font pFont;
	xui_timeline_view_desc_t tDesc;
	xui_timeline_view_desc_t tScratchDesc;
	xui_timeline_view_colors_t tColors;
	xui_timeline_frame_t tFrame;
	xui_timeline_span_t tSpan;
	xui_timeline_layer_t tLayer;
	xui_timeline_selection_t tSelection;
	xui_timeline_hit_t tHit;
	xui_event_t tEvent;
	xui_vec2_t tNatural;
	xui_vec2_t tMeasured;
	timeline_test_data_t tData;
	timeline_destroy_data_t tDestroyChanging;
	timeline_destroy_data_t tDestroyRender;
	timeline_mutate_render_data_t tMutateRender;
	int iLayerBg;
	int iLayerFx;
	int iLayerCam;
	int iSpan;
	int iContextOpenBefore;
	int iContextCommandBefore;
	int iCurrentChangedBefore;
	int iLayerChangingBefore;
	int iLayerChangedBefore;
	int iLayerSelectedBefore;
	int iSelectionChangedBefore;
	int iSelectionBefore;
	int iSpanChangeBefore;
	int iOwnerFrame;
	int i;
	int iFailed;
	int iRet;
	float fFrameWidthBefore;
	float fOffsetX;
	char sLongUtf8[128];

	pContext = NULL;
	pRoot = NULL;
	pTimeline = NULL;
	pScratch = NULL;
	pDestroyChanging = NULL;
	pDestroyRender = NULL;
	pTarget = NULL;
	pTimelineCache = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tData, 0, sizeof(tData));
	tData.bAllowLayerSelect = 1;
	memset(&tDestroyChanging, 0, sizeof(tDestroyChanging));
	memset(&tDestroyRender, 0, sizeof(tDestroyRender));
	memset(&tMutateRender, 0, sizeof(tMutateRender));
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
	tScratchDesc = tDesc;
	tScratchDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	iRet = xuiTimeLineViewCreate(pContext, &pScratch, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pScratch != NULL &&
		xuiTimeLineViewGetScrollbarMode(pScratch) == XUI_SCROLLBAR_MODE_FULL,
		"descriptor full scrollbar mode");
	(void)xuiWidgetDestroy(pScratch);
	pScratch = pRoot;
	tScratchDesc.fFrameRate = NAN;
	iRet = xuiTimeLineViewCreate(pContext, &pScratch, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pScratch == NULL,
		"invalid descriptor clears output");
	iRet = xuiTimeLineViewCreate(pContext, &pTimeline, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTimeline != NULL, "timeline create");
	iRet = xuiWidgetMeasure(pTimeline,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tNatural);
	XUI_TEST_CHECK(iRet == XUI_OK && tNatural.fX > 140.0f && tNatural.fY > 90.0f,
		"timeline intrinsic measure exceeds finite constraint");
	iRet = xuiWidgetMeasure(pTimeline, (xui_vec2_t){140.0f, 90.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 140.0f && tMeasured.fY == 90.0f,
		"timeline finite measure constraint");
	iRet = xuiWidgetMeasure(pTimeline,
		(xui_vec2_t){140.0f, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 140.0f &&
		tMeasured.fY == tNatural.fY, "timeline single-axis measure constraint");
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
	sLongUtf8[0] = 'a';
	for ( i = 0; i < 40; i++ ) memcpy(sLongUtf8 + 1 + i * 3, "\xE4\xB8\xAD", 3u);
	sLongUtf8[121] = '\0';
	iRet = xuiTimeLineViewSetLayerName(pTimeline, iLayerFx, sLongUtf8);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewGetLayer(pTimeline, iLayerFx, &tLayer);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(tLayer.sName) == 94u,
		"layer name truncates at utf8 boundary");
	iRet = xuiTimeLineViewSetLayerName(pTimeline, iLayerFx, "Effects");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore layer name");

	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerBg, 0, XUI_TIMELINE_FRAME_KEY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame key");
	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerBg, 8, XUI_TIMELINE_FRAME_NORMAL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame normal");
	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerFx, 12, XUI_TIMELINE_FRAME_BLANK_KEY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame blank key");
	iRet = xuiTimeLineViewSetFrame(pTimeline, iLayerFx, 12, 999, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "frame rejects invalid type");
	iRet = xuiTimeLineViewGetFrame(pTimeline, iLayerFx, 12, &tFrame);
	XUI_TEST_CHECK(iRet == XUI_OK && tFrame.iType == XUI_TIMELINE_FRAME_BLANK_KEY, "get frame");
	iRet = xuiTimeLineViewSetFrameUserData(pTimeline, iLayerCam, 17, &tData);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "empty frame rejects user data");

	iRet = xuiTimeLineViewAddSpan(pTimeline, iLayerBg, 0, 20, XUI_TIMELINE_SPAN_HOLD, "intro", &iSpan);
	XUI_TEST_CHECK(iRet == XUI_OK && iSpan > 0, "add span");
	iRet = xuiTimeLineViewSetSpanColor(pTimeline, iSpan, XUI_COLOR_RGBA(85, 133, 219, 210));
	XUI_TEST_CHECK(iRet == XUI_OK, "span color");
	iRet = xuiTimeLineViewSetSpanCustomType(pTimeline, iSpan, "clip");
	XUI_TEST_CHECK(iRet == XUI_OK, "span custom type");
	iRet = xuiTimeLineViewGetSpan(pTimeline, iSpan, &tSpan);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tSpan.sLabel, "intro") == 0 && strcmp(tSpan.sCustomType, "clip") == 0, "get span");
	iSpanChangeBefore = xuiTimeLineViewGetSpanChangeCount(pTimeline);
	iRet = xuiTimeLineViewSetSpanCustomType(pTimeline, iSpan, sLongUtf8);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetSpanCustomType(pTimeline, iSpan, sLongUtf8);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTimeLineViewGetSpanChangeCount(pTimeline) == iSpanChangeBefore + 1,
		"truncated span custom type repeat is idempotent");
	iRet = xuiTimeLineViewSetSpanCustomType(pTimeline, iSpan, "clip");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore span custom type");
	iRet = xuiTimeLineViewSetSpan(pTimeline, iSpan, 0, 20, 999, "invalid");
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "span rejects invalid type");
	iRet = xuiTimeLineViewGetSpan(pTimeline, iSpan, &tSpan);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tSpan.sLabel, "intro") == 0 && tSpan.iType == XUI_TIMELINE_SPAN_HOLD,
		"invalid span type does not mutate span");

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
	iLayerChangingBefore = tData.iLayerChanging;
	iLayerChangedBefore = tData.iLayerChanged;
	iLayerSelectedBefore = tData.iLayerSelected;
	tData.bAllowLayerSelect = 0;
	iRet = __xuiTimelineTestClick(pContext, 20.0f + 40.0f,
		20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewGetLayer(pTimeline, iLayerFx, &tLayer);
	XUI_TEST_CHECK(iRet == XUI_OK && !tLayer.bSelected &&
		tData.iLayerChanging == iLayerChangingBefore + 1 &&
		tData.iLayerChanged == iLayerChangedBefore &&
		tData.iLayerSelected == iLayerSelectedBefore &&
		tData.iLastLayerChanging == XUI_TIMELINE_LAYER_CHANGE_SELECT,
		"layer select changing callback can veto activation");
	tData.bAllowLayerSelect = 1;
	iRet = __xuiTimelineTestClick(pContext, 20.0f + 40.0f,
		20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewGetLayer(pTimeline, iLayerFx, &tLayer);
	XUI_TEST_CHECK(iRet == XUI_OK && tLayer.bSelected &&
		tData.iLayerChanging == iLayerChangingBefore + 2 &&
		tData.iLayerChanged == iLayerChangedBefore + 1 &&
		tData.iLayerSelected == iLayerSelectedBefore + 1 &&
		tData.iLastLayerChanged == XUI_TIMELINE_LAYER_CHANGE_SELECT,
		"layer select callbacks wrap activation");
	fFrameWidthBefore = xuiTimeLineViewGetFrameWidth(pTimeline);
	iRet = xuiInputSetModifiers(pContext, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerWheel(pContext, 25.0f, 25.0f, 1.0f, 0.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( xuiInputSetModifiers(pContext, 0u) != XUI_OK && iRet == XUI_OK ) iRet = XUI_ERROR;
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetFrameWidth(pTimeline) == fFrameWidthBefore,
		"ctrl horizontal wheel does not zoom timeline");
	XUI_TEST_CHECK(xuiQueryCursor(pContext, 20 + 150 + 8 * 10, 20 + 8) == XUI_CURSOR_RESIZE_EW,
		"playhead uses horizontal drag cursor");

	iRet = xuiTimeLineViewHitTest(pTimeline, 150.0f + 8.0f * 10.0f + 5.0f, 26.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && (tHit.iType == XUI_TIMELINE_HIT_SPAN || tHit.iType == XUI_TIMELINE_HIT_FRAME || tHit.iType == XUI_TIMELINE_HIT_SELECTION) && tHit.iFrame == 8, "hit frame");
	iRet = xuiTimeLineViewHitTest(pTimeline, 114.0f, 26.0f + 24.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_TIMELINE_HIT_LAYER_LOCK && tHit.iLayer == iLayerFx, "hit lock icon");
	iRet = xuiTimeLineViewSetFeatureFlags(pTimeline, 0, 0);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewHitTest(pTimeline, 134.0f, 26.0f + 24.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_TIMELINE_HIT_LAYER_NAME && tHit.iLayer == iLayerFx,
		"disabled layer tools release name space");
	iRet = xuiTimeLineViewSetFeatureFlags(pTimeline, 0, 1);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewHitTest(pTimeline, 135.0f, 26.0f + 24.0f + 12.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_OK && tHit.iType == XUI_TIMELINE_HIT_LAYER_LOCK && tHit.iLayer == iLayerFx,
		"lock tool occupies trailing slot when visibility is hidden");
	iRet = xuiTimeLineViewSetFeatureFlags(pTimeline, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore layer tools");

	iRet = __xuiTimelineTestClick(pContext, 20.0f + 150.0f + 5.0f * 10.0f + 5.0f, 20.0f + 26.0f + 24.0f + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 5 && tData.iFrameClick > 0, "click frame");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 6, "keyboard right");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_CONTEXT_MENU, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextOpen > 0 && xuiOverlayTop(pContext) != NULL, "keyboard context menu opening");
	iContextCommandBefore = tData.iContextCommand;
	iRet = xuiTimeLineViewRunContextCommand(pTimeline, XUI_TIMELINE_MENU_FRAME_KEY);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCommand > iContextCommandBefore, "context command");
	iRet = xuiTimeLineViewGetFrame(pTimeline, iLayerFx, 6, &tFrame);
	XUI_TEST_CHECK(iRet == XUI_OK && tFrame.iType == XUI_TIMELINE_FRAME_KEY, "context command applies frame");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "keyboard context menu close");
	iContextOpenBefore = tData.iContextOpen;
	iRet = __xuiTimelineTestRightClick(pContext, 20.0f + 150.0f + 7.0f * 10.0f + 5.0f, 20.0f + 26.0f + (float)iLayerFx * 24.0f + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextOpen > iContextOpenBefore && xuiOverlayTop(pContext) != NULL, "right click context menu opening");
	iContextCommandBefore = tData.iContextCommand;
	iRet = xuiTimeLineViewRunContextCommand(pTimeline, XUI_TIMELINE_MENU_FRAME_BLANK_KEY);
	XUI_TEST_CHECK(iRet == XUI_OK && tData.iContextCommand > iContextCommandBefore, "right click context command");
	iRet = xuiTimeLineViewGetFrame(pTimeline, iLayerFx, 7, &tFrame);
	XUI_TEST_CHECK(iRet == XUI_OK && tFrame.iType == XUI_TIMELINE_FRAME_BLANK_KEY, "right click command applies frame");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "right click context menu close");

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
	iRet = xuiTimeLineViewGetOffset(pTimeline, &fOffsetX, NULL);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetCurrentFrame(pTimeline, 8);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		20.0f + 150.0f + 8.0f * 10.0f - fOffsetX + 0.5f, 20.0f + 8.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMove(pContext,
		20.0f + 150.0f + 10.0f * 10.0f - fOffsetX + 0.5f, 290.0f,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 10,
		"captured playhead drag keeps scrolled content coordinates outside widget");
	iRet = xuiInputPointerUp(pContext,
		20.0f + 150.0f + 10.0f * 10.0f - fOffsetX + 0.5f, 290.0f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"captured playhead drag releases after outside move");

	iRet = xuiTimeLineViewSetFrameRate(pTimeline, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTimeLineViewGetFrameRate(pTimeline) == 24.0f,
		"frame rate rejects nan");
	iRet = xuiTimeLineViewSetFrameWidth(pTimeline, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTimeLineViewGetFrameWidth(pTimeline) == 10.0f,
		"frame width rejects nan");
	iRet = xuiTimeLineViewSetMetrics(pTimeline, NAN, 0.0f, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "metrics reject nan");
	iRet = xuiTimeLineViewSetLayerHeight(pTimeline, iLayerBg, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "layer height rejects nan");
	iRet = xuiTimeLineViewHitTest(pTimeline, NAN, 10.0f, &tHit);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "hit test rejects nan");
	iRet = xuiTimeLineViewSetOffset(pTimeline, NAN, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "offset rejects nan");

	iSelectionBefore = xuiTimeLineViewGetSelectionCount(pTimeline);
	iRet = xuiTimeLineViewSetFrameCount(pTimeline, 100000);
	XUI_TEST_CHECK(iRet == XUI_OK, "large valid frame count");
	iRet = xuiTimeLineViewSelectRange(pTimeline, 0, 0, 2, 99999, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY &&
		xuiTimeLineViewGetSelectionCount(pTimeline) == iSelectionBefore,
		"oversized selection is fast and transactional");
	iRet = xuiTimeLineViewClearSelection(pTimeline);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSelectRange(pTimeline, 0, 0, 0,
		XUI_TIMELINE_SELECTION_CAPACITY - 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTimeLineViewGetSelectionCount(pTimeline) == XUI_TIMELINE_SELECTION_CAPACITY,
		"selection reaches exact capacity");
	iRet = xuiTimeLineViewSelectFrame(pTimeline, 1, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_OUT_OF_MEMORY &&
		xuiTimeLineViewGetSelectionCount(pTimeline) == XUI_TIMELINE_SELECTION_CAPACITY &&
		xuiTimeLineViewIsFrameSelected(pTimeline, 1, 0) == 0,
		"single frame selection reports capacity exhaustion");
	iRet = xuiTimeLineViewClearSelection(pTimeline);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear capacity selection");
	iRet = xuiTimeLineViewSetFrameCount(pTimeline, 180);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore frame count");
	iRet = xuiTimeLineViewSetCurrentFrame(pTimeline, 179);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSelectFrame(pTimeline, 0, 179, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare frame count trim");
	iCurrentChangedBefore = tData.iCurrentChanged;
	iSelectionChangedBefore = tData.iSelectionChanged;
	iRet = xuiTimeLineViewSetFrameCount(pTimeline, 100);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetCurrentFrame(pTimeline) == 99 &&
		xuiTimeLineViewGetSelectionCount(pTimeline) == 0 &&
		tData.iCurrentChanged == iCurrentChangedBefore + 1 &&
		tData.iSelectionChanged == iSelectionChangedBefore + 1,
		"frame count trim notifies current frame and selection changes");
	iRet = xuiTimeLineViewSetFrameCount(pTimeline, 180);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore frame count after trim");

	iRet = xuiTimeLineViewGetColors(pTimeline, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "get colors for alpha regression");
	tColors.iBackgroundColor = XUI_COLOR_RGBA(0, 220, 0, 255);
	iRet = xuiTimeLineViewSetColors(pTimeline, &tColors);
	if ( iRet == XUI_OK ) iRet = __xuiTimelineTestRender(pContext, pTarget);
	pTimelineCache = xuiWidgetGetCacheSurface(pTimeline, xuiWidgetGetStateId(pTimeline));
	XUI_TEST_CHECK(iRet == XUI_OK && pTimelineCache != NULL &&
		xuiTestSurfaceGetRectFillColorCount(pTimelineCache, tColors.iBackgroundColor) > 0,
		"rgba low byte controls timeline alpha");

	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		195, 28, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iOwnerFrame = xuiTimeLineViewGetCurrentFrame(pTimeline);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pTimeline,
		"timeline owner touch captures pointer");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		245, 28, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, 295, 28, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, 295, 28, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTimeLineViewGetCurrentFrame(pTimeline) == iOwnerFrame &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pTimeline &&
		xuiGetPointerCaptureEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move playhead or end owner capture");
	iRet = xuiInputPointerMoveEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		295, 28, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTimeLineViewGetCurrentFrame(pTimeline) != iOwnerFrame,
		"timeline owner touch moves playhead");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		295, 28, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"timeline owner touch releases capture");

	iRet = xuiInputPointerDown(pContext, 20.0f + 150.0f + 25.0f, 20.0f + 8.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pTimeline,
		"ruler drag captures pointer");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"ancestor disable releases timeline capture");
	iOwnerFrame = xuiTimeLineViewGetCurrentFrame(pTimeline);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pTimeline;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = 20.0f + 150.0f + 45.0f;
	tEvent.fY = 20.0f + 8.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiTimeLineViewGetCurrentFrame(pTimeline) == iOwnerFrame,
		"disabled ancestor blocks direct timeline interaction");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable timeline ancestor");

	tScratchDesc = tDesc;
	iRet = xuiTimeLineViewCreate(pContext, &pScratch, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pScratch != NULL, "create interaction timeline");
	xuiWidgetSetRect(pScratch, (xui_rect_t){20.0f, 300.0f, 500.0f, 100.0f});
	iRet = xuiWidgetAddChild(pRoot, pScratch);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pScratch, "A", NULL);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pScratch, "B", NULL);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pScratch, "C", NULL);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout interaction timeline");
	iRet = __xuiTimelineTestClick(pContext,
		20.0f + 150.0f + 3.0f * 10.0f + 5.0f,
		300.0f + 26.0f + 12.0f);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTimeLineViewGetSelectionCount(pScratch) == 2 &&
		xuiTimeLineViewIsFrameSelected(pScratch, 0, 3) == 1 &&
		xuiTimeLineViewIsFrameSelected(pScratch, 1, 3) == 1,
		"shift down extends selection across layers");
	iRet = xuiTimeLineViewRemoveLayer(pScratch, 0);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewGetLayer(pScratch, 0, &tLayer);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tLayer.sName, "B") == 0 && tLayer.bSelected &&
		xuiTimeLineViewGetSelectionCount(pScratch) == 1 &&
		xuiTimeLineViewIsFrameSelected(pScratch, 0, 3) == 1,
		"removing preceding layer preserves active layer and remaps selection");
	iRet = __xuiTimelineTestRightClick(pContext,
		20.0f + 150.0f + 3.0f * 10.0f + 5.0f,
		300.0f + 26.0f + 24.0f + 12.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiOverlayTop(pContext) != NULL,
		"interaction timeline context menu opens");
	iRet = xuiTimeLineViewMoveLayer(pScratch, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiOverlayTop(pContext) == NULL,
		"structural mutation closes stale context menu");
	iRet = xuiInputPointerDown(pContext,
		20.0f + 150.0f + 4.0f * 10.0f + 5.0f,
		300.0f + 26.0f + 12.0f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pScratch,
		"interaction timeline captures pointer");
	iRet = xuiTimeLineViewSetFrameCount(pScratch, 179);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"structural mutation cancels pointer interaction");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch cancelled interaction");
	(void)xuiWidgetDestroy(pScratch);
	pScratch = NULL;

	tScratchDesc = tDesc;
	iRet = xuiTimeLineViewCreate(pContext, &pScratch, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pScratch != NULL, "create mutation render timeline");
	xuiWidgetSetRect(pScratch, (xui_rect_t){20.0f, 300.0f, 500.0f, 100.0f});
	iRet = xuiWidgetAddChild(pRoot, pScratch);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pScratch, "A", NULL);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pScratch, "B", NULL);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetRenderers(pScratch,
		__xuiTimelineMutateLayerRender, NULL, NULL, NULL, &tMutateRender);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = __xuiTimelineTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && tMutateRender.iMutationResult == XUI_OK &&
		tMutateRender.iCalls == 1,
		"render mutation aborts stale layer traversal");
	(void)xuiWidgetDestroy(pScratch);
	pScratch = NULL;

	tScratchDesc = tDesc;
	iRet = xuiTimeLineViewCreate(pContext, &pDestroyChanging, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyChanging != NULL, "create callback destroy timeline");
	iRet = xuiTimeLineViewAddLayer(pDestroyChanging, "destroy", NULL);
	tDestroyChanging.ppWidget = &pDestroyChanging;
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetLayerCallbacks(pDestroyChanging,
		__xuiTimelineDestroyChanging, NULL, &tDestroyChanging);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetLayerVisible(pDestroyChanging, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyChanging == NULL && tDestroyChanging.iCalls == 1,
		"changing callback may destroy control");

	iRet = xuiTimeLineViewCreate(pContext, &pDestroyRender, &tScratchDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyRender != NULL, "create render destroy timeline");
	xuiWidgetSetRect(pDestroyRender, (xui_rect_t){20.0f, 300.0f, 300.0f, 100.0f});
	iRet = xuiWidgetAddChild(pRoot, pDestroyRender);
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewAddLayer(pDestroyRender, "render", NULL);
	tDestroyRender.ppWidget = &pDestroyRender;
	if ( iRet == XUI_OK ) iRet = xuiTimeLineViewSetRenderers(pDestroyRender,
		__xuiTimelineDestroyRender, NULL, NULL, NULL, &tDestroyRender);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = __xuiTimelineTestRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyRender == NULL && tDestroyRender.iCalls == 1,
		"render callback may destroy control");

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
