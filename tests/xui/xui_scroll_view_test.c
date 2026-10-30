#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_scroll_view_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiScrollViewNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static void __xuiScrollViewChanged(xui_widget pWidget, float fOffsetX, float fOffsetY, void* pUser)
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

typedef struct xui_scroll_view_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_scroll_view_destroy_state_t;

static void __xuiScrollViewDestroyOnChange(xui_widget pWidget, float fOffsetX,
	float fOffsetY, void* pUser)
{
	xui_scroll_view_destroy_state_t* pState = (xui_scroll_view_destroy_state_t*)pUser;

	(void)fOffsetX;
	(void)fOffsetY;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiScrollViewDispatchWheel(xui_context pContext, float fX, float fY, float fWheelY)
{
	int iRet;

	iRet = xuiInputPointerWheel(pContext, fX, fY, 0.0f, fWheelY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiScrollViewRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 360, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pView;
	xui_widget pContent;
	xui_widget pChild;
	xui_widget pInvalid;
	xui_widget pDestroyView;
	xui_surface pTarget;
	xui_scroll_view_desc_t tDesc;
	xui_scroll_view_desc_t tInvalidDesc;
	xui_scroll_view_destroy_state_t tDestroy;
	xui_rect_t tViewport;
	xui_rect_t tContentRect;
	xui_rect_t tChildWorld;
	xui_rect_t tViewportWorld;
	xui_vec2_t tMeasured;
	xui_widget pHit;
	float fOffsetX;
	float fOffsetY;
	float fHintWidth;
	float fHintHeight;
	float fFrameHintWidth;
	float fFrameHintHeight;
	float fSmallStep;
	float fLargeStep;
	float fMaxX;
	float fMaxY;
	float fRejectedX;
	float fRejectedY;
	int iChanged;
	int iFailed;
	int iReset;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pView = NULL;
	pContent = NULL;
	pChild = NULL;
	pInvalid = NULL;
	pDestroyView = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	memset(&tDestroy, 0, sizeof(tDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 360.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 360.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fContentWidth = 300.0f;
	tDesc.fContentHeight = 220.0f;
	tDesc.iPolicyX = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tDesc.iWheelAxis = XUI_WHEEL_AXIS_VERTICAL;
	tDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_AUTO;
	tDesc.bContentDragEnabled = 1;
	tDesc.fScrollbarSize = 8.0f;
	tDesc.fWheelStep = 20.0f;
	tDesc.fViewportWidth = 140.0f;
	tDesc.fViewportHeight = 110.0f;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollViewCreate(NULL, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");
	iRet = xuiScrollViewCreate(pContext, &pView, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pView != NULL, "scrollview create");
	tInvalidDesc = tDesc;
	tInvalidDesc.fWheelStep = INFINITY;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollViewCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid descriptor clears scrollview output");
	tInvalidDesc = tDesc;
	tInvalidDesc.iWheelAxis = 99;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollViewCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"invalid scrollview wheel axis is rejected");
	tInvalidDesc = tDesc;
	tInvalidDesc.iCornerMode = XUI_SCROLL_FRAME_CORNER_NONE;
	iRet = xuiScrollViewCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pInvalid != NULL &&
		xuiScrollViewGetCornerMode(pInvalid) == XUI_SCROLL_FRAME_CORNER_NONE,
		"scrollview creation preserves explicit no-corner mode");
	xuiWidgetDestroy(pInvalid);
	pInvalid = NULL;
	iRet = xuiScrollViewSetWheelStep(pView, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		__xuiScrollViewNear(xuiScrollViewGetWheelStep(pView), 20.0f),
		"scrollview rejects infinite wheel step without mutation");
	iRet = xuiScrollViewSetWheelStep(pView, 12.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarGetSteps(
		xuiScrollFrameGetHScrollBarWidget(xuiScrollViewGetFrameWidget(pView)),
		&fSmallStep, &fLargeStep);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(fSmallStep, 12.0f),
		"scrollview wheel step immediately reaches frame bars");
	iRet = xuiScrollViewSetWheelStep(pView, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore scrollview wheel step");
	iRet = xuiWidgetMeasure(pView, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(tMeasured.fX, 140.0f) && __xuiScrollViewNear(tMeasured.fY, 110.0f), "scrollview desired viewport");
	iRet = xuiWidgetMeasure(pView, (xui_vec2_t){90.0f, 70.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(tMeasured.fX, 90.0f) &&
		__xuiScrollViewNear(tMeasured.fY, 70.0f),
		"scrollview desired viewport honors finite constraints");
	iRet = xuiScrollViewSetViewportHint(pView, 150.0f, 115.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewGetViewportHint(pView, &fHintWidth, &fHintHeight);
	if ( iRet == XUI_OK ) iRet = xuiScrollFrameGetViewportHint(xuiScrollViewGetFrameWidget(pView),
		&fFrameHintWidth, &fFrameHintHeight);
	if ( iRet == XUI_OK ) iRet = xuiWidgetMeasure(pView,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(fHintWidth, 150.0f) &&
		__xuiScrollViewNear(fHintHeight, 115.0f) &&
		__xuiScrollViewNear(fFrameHintWidth, fHintWidth) &&
		__xuiScrollViewNear(fFrameHintHeight, fHintHeight) &&
		__xuiScrollViewNear(tMeasured.fX, fHintWidth) &&
		__xuiScrollViewNear(tMeasured.fY, fHintHeight),
		"scrollview uses frame viewport hint as one source of truth");
	iRet = xuiScrollViewSetViewportHint(pView, 140.0f, 110.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore scrollview viewport hint");
	iRet = xuiScrollViewSetContentSize(pView, 3000.0f, 2200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "scrollview large content extent");
	iRet = xuiWidgetMeasure(pView, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(tMeasured.fX, 140.0f) && __xuiScrollViewNear(tMeasured.fY, 110.0f), "scrollview extent does not change desired viewport");
	iRet = xuiScrollViewSetContentSize(pView, 300.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore scrollview content extent");
	iRet = xuiScrollViewSetContentSize(pView, XUI_LAYOUT_UNBOUNDED,
		XUI_LAYOUT_UNBOUNDED);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewSetOffset(pView,
		XUI_LAYOUT_UNBOUNDED - 512.0f, XUI_LAYOUT_UNBOUNDED - 512.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewScrollBy(pView,
		XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED);
	if ( iRet == XUI_OK ) iRet = xuiScrollModelGetMaxOffset(
		xuiScrollViewGetModel(pView), &fMaxX, &fMaxY);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewGetOffset(pView, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX == fMaxX && fOffsetY == fMaxY,
		"wide relative scroll saturates at the content boundary");
	iRet = xuiScrollViewSetContentSize(pView, 300.0f, 220.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewSetOffset(pView, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary scrollview after wide scroll");
	iRet = xuiWidgetAddChild(pRoot, pView);
	XUI_TEST_CHECK(iRet == XUI_OK, "add scrollview");
	xuiWidgetSetRect(pView, (xui_rect_t){20.0f, 20.0f, 140.0f, 110.0f});
	iRet = xuiScrollViewSetChange(pView, __xuiScrollViewChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change");

	pContent = xuiScrollViewGetContentWidget(pView);
	XUI_TEST_CHECK(pContent != NULL && xuiWidgetGetParent(pContent) == xuiScrollViewGetViewportWidget(pView), "content parent");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	xuiWidgetSetRect(pChild, (xui_rect_t){220.0f, 150.0f, 40.0f, 30.0f});
	iRet = xuiWidgetAddChild(pContent, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "add child to content");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 360, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiScrollViewRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	tViewport = xuiScrollViewGetViewportRect(pView);
	XUI_TEST_CHECK(tViewport.fW < 140.0f && tViewport.fH < 110.0f, "viewport reserves bars");
	XUI_TEST_CHECK(xuiScrollFrameIsHScrollBarVisible(xuiScrollViewGetFrameWidget(pView)), "view hbar visible");
	XUI_TEST_CHECK(xuiScrollFrameIsVScrollBarVisible(xuiScrollViewGetFrameWidget(pView)), "view vbar visible");

	pHit = xuiHitTest(pContext, 20.0f + 220.0f + 4.0f, 20.0f + 150.0f + 4.0f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHit != pChild, "child clipped from hit outside viewport");

	iRet = xuiScrollViewSetOffset(pView, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiInputSetModifiers(pContext, XUI_MOD_SHIFT);
	if ( iRet == XUI_OK ) iRet = __xuiScrollViewDispatchWheel(pContext,
		20.0f + 8.0f, 20.0f + 8.0f, -1.0f);
	iReset = xuiInputSetModifiers(pContext, 0u);
	if ( iRet == XUI_OK ) iRet = iReset;
	if ( iRet == XUI_OK ) iRet = xuiScrollViewGetOffset(pView, &fOffsetX, &fOffsetY);
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(fOffsetX, 20.0f) &&
		__xuiScrollViewNear(fOffsetY, 0.0f) && __xuiScrollViewNear(tContentRect.fX, -20.0f),
		"shift wheel scrolls and synchronizes content horizontally");

	iRet = xuiScrollViewSetOffset(pView, 50.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset");
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(__xuiScrollViewNear(tContentRect.fX, -50.0f) && __xuiScrollViewNear(tContentRect.fY, -40.0f), "content shifted");
	iRet = xuiScrollViewScrollBy(pView, 15.0f, 10.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollViewGetOffset(pView, &fOffsetX, &fOffsetY);
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(fOffsetX, 65.0f) &&
		__xuiScrollViewNear(fOffsetY, 50.0f) &&
		__xuiScrollViewNear(tContentRect.fX, -fOffsetX) &&
		__xuiScrollViewNear(tContentRect.fY, -fOffsetY),
		"relative scroll synchronizes the content container");
	iRet = xuiScrollViewScrollBy(pView, INFINITY, 0.0f);
	if ( iRet == XUI_ERROR_INVALID_ARGUMENT ) iRet = xuiScrollViewGetOffset(
		pView, &fRejectedX, &fRejectedY);
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollViewNear(fRejectedX, fOffsetX) &&
		__xuiScrollViewNear(fRejectedY, fOffsetY) &&
		__xuiScrollViewNear(tContentRect.fX, -fOffsetX) &&
		__xuiScrollViewNear(tContentRect.fY, -fOffsetY),
		"invalid relative scroll leaves model and content unchanged");

	iRet = xuiScrollViewEnsureChildVisible(pView, pRoot);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject unrelated ensure-visible widget");
	iRet = xuiScrollViewEnsureChildVisible(pView, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "ensure child");
	iRet = xuiScrollViewGetOffset(pView, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX > 100.0f && fOffsetY > 60.0f, "ensure child offset");
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(__xuiScrollViewNear(tContentRect.fX, -fOffsetX) && __xuiScrollViewNear(tContentRect.fY, -fOffsetY), "content resynced");

	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollViewGetViewportWidget(pView));
	iRet = __xuiScrollViewDispatchWheel(pContext, tViewportWorld.fX + 8.0f, tViewportWorld.fY + 8.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	iRet = xuiScrollViewGetOffset(pView, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged > 0, "wheel change count");
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(__xuiScrollViewNear(tContentRect.fY, -fOffsetY), "wheel content sync");

	tChildWorld = xuiWidgetGetWorldRect(pChild);
	pHit = xuiHitTest(pContext, tChildWorld.fX + 2.0f, tChildWorld.fY + 2.0f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHit == pChild, "visible child hit");

	tInvalidDesc = tDesc;
	tInvalidDesc.fContentWidth = 200.0f;
	tInvalidDesc.fContentHeight = 200.0f;
	tInvalidDesc.fViewportWidth = 80.0f;
	tInvalidDesc.fViewportHeight = 60.0f;
	iRet = xuiScrollViewCreate(pContext, &pDestroyView, &tInvalidDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroyView);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroyView,
		(xui_rect_t){200.0f, 20.0f, 80.0f, 60.0f});
	tDestroy.ppWidget = &pDestroyView;
	if ( iRet == XUI_OK ) iRet = xuiScrollViewSetChange(pDestroyView,
		__xuiScrollViewDestroyOnChange, &tDestroy);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change scrollview prepare");
	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollViewGetViewportWidget(pDestroyView));
	iRet = __xuiScrollViewDispatchWheel(pContext, tViewportWorld.fX + 4.0f,
		tViewportWorld.fY + 4.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyView == NULL && tDestroy.iCount == 1,
		"change callback may destroy scrollview without stale frame access");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_scroll_view_test passed\n");
	return 0;
}
