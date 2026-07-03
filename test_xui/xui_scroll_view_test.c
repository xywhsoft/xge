#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_surface pTarget;
	xui_scroll_view_desc_t tDesc;
	xui_rect_t tViewport;
	xui_rect_t tContentRect;
	xui_rect_t tChildWorld;
	xui_rect_t tViewportWorld;
	xui_widget pHit;
	float fOffsetX;
	float fOffsetY;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pView = NULL;
	pContent = NULL;
	pChild = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
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
	iRet = xuiScrollViewCreate(pContext, &pView, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pView != NULL, "scrollview create");
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

	iRet = xuiScrollViewSetOffset(pView, 50.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set offset");
	tContentRect = xuiWidgetGetRect(pContent);
	XUI_TEST_CHECK(__xuiScrollViewNear(tContentRect.fX, -50.0f) && __xuiScrollViewNear(tContentRect.fY, -40.0f), "content shifted");

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
