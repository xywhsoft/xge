#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_surface pTarget;
	xui_scroll_frame_desc_t tDesc;
	xui_rect_t tViewport;
	xui_rect_t tHBar;
	xui_rect_t tVBar;
	xui_rect_t tCorner;
	xui_rect_t tViewportWorld;
	float fOffsetX;
	float fOffsetY;
	float fMaxX;
	float fMaxY;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pFrame = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
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
	iRet = xuiScrollFrameCreate(pContext, &pFrame, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFrame != NULL, "frame create");
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

	tViewportWorld = xuiWidgetGetWorldRect(xuiScrollFrameGetViewportWidget(pFrame));
	iRet = __xuiScrollFrameDispatchWheel(pContext, tViewportWorld.fX + 4.0f, tViewportWorld.fY + 4.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollFrameNear(fOffsetY, 20.0f) && iChanged > 0, "wheel offset");
	XUI_TEST_CHECK(__xuiScrollFrameNear(xuiScrollBarGetValue(xuiScrollFrameGetVScrollBarWidget(pFrame)), fOffsetY), "vbar sync");

	iRet = __xuiScrollFrameDispatchDrag(pContext, tViewportWorld.fX + 30.0f, tViewportWorld.fY + 30.0f, tViewportWorld.fX + 20.0f, tViewportWorld.fY + 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag dispatch");
	iRet = xuiScrollFrameGetOffset(pFrame, &fOffsetX, &fOffsetY);
	XUI_TEST_CHECK(iRet == XUI_OK && fOffsetX > 0.0f && fOffsetY > 20.0f, "drag offset");

	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_HIDDEN, XUI_SCROLLBAR_POLICY_HIDDEN);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden policy");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden layout");
	XUI_TEST_CHECK(!xuiScrollFrameIsHScrollBarVisible(pFrame) && !xuiScrollFrameIsVScrollBarVisible(pFrame), "bars hidden");

	iRet = xuiScrollFrameSetContentSize(pFrame, 20.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "small content");
	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_ALWAYS, XUI_SCROLLBAR_POLICY_ALWAYS);
	XUI_TEST_CHECK(iRet == XUI_OK, "always policy");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "always layout");
	XUI_TEST_CHECK(xuiScrollFrameIsHScrollBarVisible(pFrame) && xuiScrollFrameIsVScrollBarVisible(pFrame), "always bars");

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
