#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_scroll_frame_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiScrollFrameNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static void __xuiScrollFrameChanged(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)fOffsetX;
	(void)fOffsetY;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_scroll_frame_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_scroll_frame_destroy_state_t;

static void __xuiScrollFrameDestroyOnChange(xui_widget pWidget, float fOffsetX,
	float fOffsetY, void* pUser)
{
	xui_scroll_frame_destroy_state_t* pState = (xui_scroll_frame_destroy_state_t*)pUser;

	(void)fOffsetX;
	(void)fOffsetY;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiScrollFrameDispatchWheel(xui_context pContext, float fX, float fY, float fWheelY)
{
	int iRet;

	iRet = xuiInputPointerWheel(pContext, fX, fY, 0.0f, fWheelY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiScrollFrameDispatchDrag(xui_context pContext, float fStartX, float fStartY, float fEndX, float fEndY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fStartX, fStartY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiScrollFrameRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 320, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pFrame;
	xui_widget pFooter;
	xui_widget pInvalid;
	xui_widget pOuter;
	xui_widget pInner;
	xui_widget pDestroyFrame;
	xui_surface pTarget;
	xui_scroll_frame_desc_t tDesc;
	xui_scroll_frame_desc_t tAuxDesc;
	xui_scroll_frame_destroy_state_t tDestroy;
	xui_event_t tEvent;
	xui_layout_stats_t tLayoutStats;
	xui_rect_t tViewport;
	xui_rect_t tHBar;
	xui_rect_t tVBar;
	xui_rect_t tCorner;
	xui_rect_t tViewportWorld;
	xui_rect_t tFooterBefore;
	xui_rect_t tFooterAfter;
	xui_vec2_t tMeasured;
	float fOffsetX;
	float fOffsetY;
	float fHintWidth;
	float fHintHeight;
	float fMaxX;
	float fMaxY;
	float fBeforeX;
	float fBeforeY;
	float fMetricA;
	float fMetricB;
	float fMetricC;
	int iChanged;
	int iFailed;
	int iReset;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pFrame = NULL;
	pFooter = NULL;
	pInvalid = NULL;
	pOuter = NULL;
	pInner = NULL;
	pDestroyFrame = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	memset(&tDestroy, 0, sizeof(tDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 320.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 320.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fContentWidth = 240.0f;
	tDesc.fContentHeight = 220.0f;
	tDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_FULL;
	tDesc.iWheelAxis = XUI_WHEEL_AXIS_VERTICAL;
	tDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_GRIP;
	tDesc.bContentDragEnabled = 1;
	tDesc.fScrollbarSize = 16.0f;
	tDesc.fWheelStep = 20.0f;
	tDesc.fViewportWidth = 120.0f;
	tDesc.fViewportHeight = 100.0f;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollFrameCreate(NULL, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");
	iRet = xuiScrollFrameCreate(pContext, &pFrame, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFrame != NULL, "frame create");
	tAuxDesc = tDesc;
	tAuxDesc.fContentWidth = INFINITY;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollFrameCreate(pContext, &pInvalid, &tAuxDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid descriptor clears frame output");
	tAuxDesc = tDesc;
	tAuxDesc.iPolicyX = 99;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollFrameCreate(pContext, &pInvalid, &tAuxDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid frame policy is rejected");
	iRet = xuiScrollFrameSetWheelStep(pFrame, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && __xuiScrollFrameNear(xuiScrollFrameGetWheelStep(pFrame), 20.0f),
		"reject infinite wheel step without mutation");
	iRet = xuiScrollFrameGetMetrics(pFrame, &fMetricA, &fMetricB, &fMetricC);
	XUI_TEST_CHECK(iRet == XUI_OK, "get initial metrics");
	iRet = xuiScrollFrameSetMetrics(pFrame, NAN, fMetricB, fMetricC);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan frame metrics");
	iRet = xuiScrollFrameGetMetrics(pFrame, &fBeforeX, &fBeforeY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && fBeforeX == fMetricA && fBeforeY == fMetricB,
		"invalid frame metrics preserve state");
	iRet = xuiWidgetMeasure(pFrame, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(tMeasured.fX, 120.0f) && __xuiScrollFrameNear(tMeasured.fY, 100.0f), "content extent does not define desired viewport");
	iRet = xuiScrollFrameGetViewportHint(pFrame, &fHintWidth, &fHintHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fHintWidth, 120.0f) && __xuiScrollFrameNear(fHintHeight, 100.0f), "viewport hint get");
	iRet = xuiScrollFrameSetContentSize(pFrame, 2400.0f, 2200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "large content extent");
	iRet = xuiWidgetMeasure(pFrame, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(tMeasured.fX, 120.0f) && __xuiScrollFrameNear(tMeasured.fY, 100.0f), "large extent keeps desired viewport");
	iRet = xuiScrollFrameSetContentSize(pFrame, 240.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore content extent");
	iRet = xuiWidgetAddChild(pRoot, pFrame);
	XUI_TEST_CHECK(iRet == XUI_OK, "add frame");
	xuiWidgetSetRect(pFrame, (xui_rect_t){10.0f, 10.0f, 120.0f, 100.0f});
	iRet = xuiScrollFrameSetChange(pFrame, __xuiScrollFrameChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 320, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiScrollFrameRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiScrollFrameIsHScrollBarVisible(pFrame), "hbar visible");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(pFrame), "vbar visible");
	XUI_TEST_CHECK(xuiScrollFrameIsCornerVisible(pFrame), "corner visible");
	tViewport = xuiScrollFrameGetViewportRect(pFrame);
	tHBar = xuiScrollFrameGetHScrollBarRect(pFrame);
	tVBar = xuiScrollFrameGetVScrollBarRect(pFrame);
	tCorner = xuiScrollFrameGetCornerRect(pFrame);
	XUI_TEST_CHECK(__xuiScrollFrameNear(tViewport.fW, 104.0f) && __xuiScrollFrameNear(tViewport.fH, 84.0f), "viewport reserve");
	XUI_TEST_CHECK(__xuiScrollFrameNear(tHBar.fH, 16.0f) && __xuiScrollFrameNear(tVBar.fW, 16.0f), "bar thickness");
	XUI_TEST_CHECK(__xuiScrollFrameNear(tCorner.fW, 16.0f) && __xuiScrollFrameNear(tCorner.fH, 16.0f), "corner size");
	iRet = xuiScrollModelGetMaxOffset(xuiScrollFrameGetModel(pFrame), &fMaxX, &fMaxY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fMaxX, 136.0f) && __xuiScrollFrameNear(fMaxY, 136.0f), "model max offset");
	iRet = xuiScrollFrameSetContentSize(pFrame, XUI_LAYOUT_UNBOUNDED,
		XUI_LAYOUT_UNBOUNDED);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pFrame,
		XUI_LAYOUT_UNBOUNDED - 512.0f, XUI_LAYOUT_UNBOUNDED - 512.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameScrollBy(pFrame,
		XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelGetMaxOffset(
		xuiScrollFrameGetModel(pFrame), &fMaxX, &fMaxY);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame,
		&fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX == fMaxX && fOffsetY == fMaxY,
		"wide relative scroll saturates at the content boundary");
	iRet = xuiScrollFrameSetContentSize(pFrame, 240.0f, 220.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary frame after wide scroll");

	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollFrameGetViewportWidget(pFrame));
	iRet = xuiScrollFrameSetWheelStep(pFrame, 12.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarGetSteps(
		xuiScrollFrameGetHScrollBarWidget(pFrame), &fBeforeX, &fBeforeY);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarGetSteps(
		xuiScrollFrameGetVScrollBarWidget(pFrame), &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fBeforeX, 12.0f) &&
		__xuiScrollFrameNear(fBeforeY, tViewport.fW) &&
		__xuiScrollFrameNear(fOffsetX, 12.0f) && __xuiScrollFrameNear(fOffsetY, tViewport.fH),
		"wheel step synchronizes both scrollbars immediately");
	iRet = xuiScrollFrameSetWheelStep(pFrame, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore wheel step");

	iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = __xuiScrollFrameDispatchWheel(pContext,
		tViewportWorld.fX + 4.0f, tViewportWorld.fY + 4.0f, -1.0f);
	iReset = xuiInputSetModifiers(pContext, 0u);
	if ( iRet == XUI_OK ) iRet = iReset;
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetX, 20.0f) &&
		__xuiScrollFrameNear(fOffsetY, 0.0f), "shift wheel scrolls horizontally");
	iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset offset after shift wheel");

	iRet = __xuiScrollFrameDispatchWheel(pContext, tViewportWorld.fX + 4.0f, tViewportWorld.fY + 4.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetY, 20.0f) && iChanged > 0, "wheel offset");
	XUI_TEST_CHECK(__xuiScrollFrameNear(xuiScrollBarGetValue(xuiScrollFrameGetVScrollBarWidget(pFrame)), fOffsetY), "vbar sync");

	iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEnabled(pRoot, 0);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_WHEEL;
	tEvent.pTarget = pFrame;
	tEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tEvent.fX = tViewportWorld.fX + 4.0f;
	tEvent.fY = tViewportWorld.fY + 4.0f;
	tEvent.fWheelY = -1.0f;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tEvent);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetX, 0.0f) &&
		__xuiScrollFrameNear(fOffsetY, 0.0f),
		"inherited disabled state blocks directly dispatched wheel input");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore enabled root and drag baseline");

	iRet = __xuiScrollFrameDispatchDrag(pContext, tViewportWorld.fX + 30.0f, tViewportWorld.fY + 30.0f, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag dispatch");
	iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX > 0.0f && fOffsetY > 20.0f, "drag offset");

	iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDownEx(pContext, 705u,
		XUI_POINTER_TYPE_TOUCH, (int)(tViewportWorld.fX + 30.0f),
		(int)(tViewportWorld.fY + 30.0f), XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 705u,
		XUI_POINTER_TYPE_TOUCH, (int)(tViewportWorld.fX + 10.0f),
		(int)(tViewportWorld.fY + 5.0f), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetX, 20.0f) &&
		__xuiScrollFrameNear(fOffsetY, 25.0f) &&
		xuiGetPointerCaptureEx(pContext, 705u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"pointer-up position supplies the final content-drag sample");

	iRet = xuiScrollFrameSetOffset(pFrame, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset offset before multi-pointer drag");
	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pFrame,
		"first touch owns frame content drag");
	iRet = xuiInputPointerMoveEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 20.0f), (int)(tViewportWorld.fY + 15.0f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame, &fBeforeX, &fBeforeY);
	XUI_TEST_CHECK(iRet == XUI_OK && fBeforeX > 0.0f && fBeforeY > 0.0f,
		"owning touch moves frame content");
	iRet = xuiInputPointerDownEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 5.0f), (int)(tViewportWorld.fY + 5.0f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 5.0f), (int)(tViewportWorld.fY + 5.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetX, fBeforeX) &&
		__xuiScrollFrameNear(fOffsetY, fBeforeY) &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pFrame &&
		xuiGetPointerCaptureEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move or release frame drag");
	iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 20.0f), (int)(tViewportWorld.fY + 15.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch releases frame drag");
	iRet = xuiInputPointerDownEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetContentDragEnabled(pFrame, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"disabling content drag releases exact capture");
	iRet = xuiScrollFrameSetContentDragEnabled(pFrame, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore content drag");
	(void)xuiInputPointerUpEx(pContext, 703u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);
	iRet = xuiInputPointerDownEx(pContext, 704u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 704u, XUI_POINTER_TYPE_TOUCH) == pFrame,
		"touch owns frame drag before disable");
	iRet = xuiWidgetSetEnabled(pFrame, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 704u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"disabling frame releases exact capture");
	iRet = xuiWidgetSetEnabled(pFrame, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore enabled frame");
	(void)xuiInputPointerUpEx(pContext, 704u, XUI_POINTER_TYPE_TOUCH,
		(int)(tViewportWorld.fX + 30.0f), (int)(tViewportWorld.fY + 30.0f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	memset(&tAuxDesc, 0, sizeof(tAuxDesc));
	tAuxDesc.iSize = sizeof(tAuxDesc);
	tAuxDesc.fContentWidth = 300.0f;
	tAuxDesc.fContentHeight = 300.0f;
	tAuxDesc.iPolicyX = XUI_SCROLLBAR_POLICY_HIDDEN;
	tAuxDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tAuxDesc.iWheelAxis = XUI_WHEEL_AXIS_VERTICAL;
	tAuxDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_NONE;
	tAuxDesc.fWheelStep = 20.0f;
	tAuxDesc.fViewportWidth = 140.0f;
	tAuxDesc.fViewportHeight = 100.0f;
	iRet = xuiScrollFrameCreate(pContext, &pOuter, &tAuxDesc);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiScrollFrameGetCornerMode(pOuter) == XUI_SCROLL_FRAME_CORNER_NONE,
		"frame creation preserves explicit no-corner mode");
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pOuter);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pOuter, (xui_rect_t){150.0f, 130.0f, 140.0f, 100.0f});
	tAuxDesc.fContentWidth = 200.0f;
	tAuxDesc.fContentHeight = 200.0f;
	tAuxDesc.fWheelStep = 10.0f;
	tAuxDesc.fViewportWidth = 80.0f;
	tAuxDesc.fViewportHeight = 60.0f;
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameCreate(pContext, &pInner, &tAuxDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(xuiScrollFrameGetViewportWidget(pOuter), pInner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pInner, (xui_rect_t){10.0f, 10.0f, 80.0f, 60.0f});
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "nested frames prepare");
	iRet = xuiScrollModelGetMaxOffset(xuiScrollFrameGetModel(pInner), NULL, &fMaxY);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pInner, 0.0f, fMaxY);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pOuter, 0.0f, 0.0f);
	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollFrameGetViewportWidget(pInner));
	if ( iRet == XUI_OK ) iRet = __xuiScrollFrameDispatchWheel(pContext,
		tViewportWorld.fX + 4.0f, tViewportWorld.fY + 4.0f, -1.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pOuter, NULL, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetY, 20.0f),
		"wheel chains to parent when inner frame is at edge");
	iRet = xuiScrollFrameSetOffset(pInner, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetOffset(pOuter, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = __xuiScrollFrameDispatchWheel(pContext,
		tViewportWorld.fX + 4.0f, tViewportWorld.fY + 4.0f, -1.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pInner, NULL, &fOffsetY);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetOffset(pOuter, NULL, &fBeforeY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetY, 10.0f) &&
		__xuiScrollFrameNear(fBeforeY, 0.0f),
		"inner frame consumes wheel while it can scroll");
	xuiWidgetDestroy(pOuter);
	pOuter = NULL;
	pInner = NULL;

	tAuxDesc.fContentWidth = 200.0f;
	tAuxDesc.fContentHeight = 200.0f;
	tAuxDesc.fViewportWidth = 80.0f;
	tAuxDesc.fViewportHeight = 60.0f;
	iRet = xuiScrollFrameCreate(pContext, &pDestroyFrame, &tAuxDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroyFrame);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroyFrame, (xui_rect_t){180.0f, 10.0f, 80.0f, 60.0f});
	tDestroy.ppWidget = &pDestroyFrame;
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameSetChange(pDestroyFrame,
		__xuiScrollFrameDestroyOnChange, &tDestroy);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change frame prepare");
	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollFrameGetViewportWidget(pDestroyFrame));
	iRet = __xuiScrollFrameDispatchWheel(pContext, tViewportWorld.fX + 4.0f,
		tViewportWorld.fY + 4.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyFrame == NULL && tDestroy.iCount == 1,
		"change callback may destroy frame without stale access");

	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_HIDDEN, XUI_SCROLLBAR_POLICY_HIDDEN);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden policy");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden layout");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(pFrame) && !xuiScrollFrameIsVScrollBarVisible(pFrame), "bars hidden");
	XUI_TEST_CHECK(!xuiWidgetGetVisible(xuiScrollFrameGetHScrollBarWidget(pFrame)) &&
		!xuiWidgetGetVisible(xuiScrollFrameGetVScrollBarWidget(pFrame)) &&
		!xuiWidgetGetVisible(xuiScrollFrameGetCornerWidget(pFrame)),
		"hidden chrome is removed from input and accessibility trees");
	iRet = xuiGetLayoutStats(pContext, &tLayoutStats);
	XUI_TEST_CHECK(iRet == XUI_OK && tLayoutStats.bStabilized,
		"hidden chrome visibility stabilizes layout");

	iRet = xuiScrollFrameSetContentSize(pFrame, 20.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small content");
	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_AUTO,
		XUI_SCROLLBAR_POLICY_AUTO);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		!xuiWidgetGetVisible(xuiScrollFrameGetHScrollBarWidget(pFrame)) &&
		!xuiWidgetGetVisible(xuiScrollFrameGetVScrollBarWidget(pFrame)) &&
		!xuiWidgetGetVisible(xuiScrollFrameGetCornerWidget(pFrame)),
		"auto policy hides chrome when content fits");
	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_ALWAYS, XUI_SCROLLBAR_POLICY_ALWAYS);
	XUI_TEST_CHECK(iRet == XUI_OK, "always policy");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "always layout");
	XUI_TEST_CHECK(xuiScrollFrameIsHScrollBarVisible(pFrame) && xuiScrollFrameIsVScrollBarVisible(pFrame), "always bars");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiScrollFrameGetHScrollBarWidget(pFrame)) &&
		xuiWidgetGetVisible(xuiScrollFrameGetVScrollBarWidget(pFrame)) &&
		xuiWidgetGetVisible(xuiScrollFrameGetCornerWidget(pFrame)),
		"visible chrome returns to input and accessibility trees");
	iRet = xuiGetLayoutStats(pContext, &tLayoutStats);
	XUI_TEST_CHECK(iRet == XUI_OK && tLayoutStats.bStabilized,
		"visible chrome restoration stabilizes layout");
	iRet = xuiScrollFrameSetCornerMode(pFrame, XUI_SCROLL_FRAME_CORNER_NONE);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiWidgetGetVisible(xuiScrollFrameGetHScrollBarWidget(pFrame)) &&
		xuiWidgetGetVisible(xuiScrollFrameGetVScrollBarWidget(pFrame)) &&
		!xuiWidgetGetVisible(xuiScrollFrameGetCornerWidget(pFrame)),
		"corner-none hides only the corner widget");
	iRet = xuiScrollFrameSetCornerMode(pFrame, XUI_SCROLL_FRAME_CORNER_GRIP);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore grip corner");

	iRet = xuiWidgetCreate(pContext, &pFooter);
	XUI_TEST_CHECK(iRet == XUI_OK && pFooter != NULL, "footer create");
	iRet = xuiWidgetAddChild(pRoot, pFooter);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer add");
	iRet = xuiWidgetSetLayoutType(pRoot, XUI_LAYOUT_COLUMN);
	XUI_TEST_CHECK(iRet == XUI_OK, "column layout");
	iRet = xuiWidgetSetSizeMode(pFrame, XUI_SIZE_FILL, XUI_SIZE_CONTENT);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame viewport sizing");
	iRet = xuiWidgetSetFlex(pFrame, 1.0f, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "frame flex");
	iRet = xuiWidgetSetSizeMode(pFooter, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer sizing");
	iRet = xuiWidgetSetPreferredSize(pFooter, (xui_vec2_t){0.0f, 40.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "footer size");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "column baseline layout");
	tFooterBefore = xuiWidgetGetRect(pFooter);
	iRet = xuiScrollFrameSetContentSize(pFrame, 2000.0f, 20000.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "huge extent in column");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "column layout after extent change");
	tFooterAfter = xuiWidgetGetRect(pFooter);
	XUI_TEST_CHECK(tFooterAfter.fY == tFooterBefore.fY && tFooterAfter.fH == tFooterBefore.fH,
		"content extent does not move or shrink sibling controls");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_scroll_frame_test passed\n");
	return 0;
}
