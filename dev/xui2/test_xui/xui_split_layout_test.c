#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_split_layout_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiSplitNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.05f;
}

static void __xuiSplitChanged(xui_widget pWidget, int iDivider, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)iDivider;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiSplitRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiSplitDispatchLayoutRender(xui_context pContext, xui_surface pTarget)
{
	int iRet;

	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiSplitRender(pContext, pTarget);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pSplit;
	xui_surface pTarget;
	xui_split_layout_desc_t tDesc;
	xui_rect_t tPane0;
	xui_rect_t tPane1;
	xui_rect_t tPane2;
	xui_rect_t tDivider;
	xui_rect_t tVisual;
	xui_rect_t tHit;
	xui_rect_t tShadow;
	xui_rect_t tWorld;
	float fLayoutSize;
	float fVisualSize;
	float fHitSize;
	float fX;
	float fY;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pSplit = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 360.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 360.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iPaneCount = 3;
	tDesc.bShadowDrag = 1;
	tDesc.fDividerSize = 10.0f;
	tDesc.fDividerVisualSize = 3.0f;
	tDesc.fDividerHitSize = 14.0f;
	iRet = xuiSplitLayoutCreate(pContext, &pSplit, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSplit != NULL, "split create");
	iRet = xuiWidgetAddChild(pRoot, pSplit);
	XUI_TEST_CHECK(iRet == XUI_OK, "add split");
	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 460.0f, 180.0f});
	iRet = xuiSplitLayoutSetChange(pSplit, __xuiSplitChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change");

	iRet = xuiSplitLayoutSetPaneMode(pSplit, 0, XUI_SPLIT_PANE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "left fixed");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "left fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "center min");
	iRet = xuiSplitLayoutSetPaneMode(pSplit, 2, XUI_SPLIT_PANE_FIXED);
	XUI_TEST_CHECK(iRet == XUI_OK, "right fixed");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 130.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "right fixed size");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	XUI_TEST_CHECK(xuiSplitLayoutGetPaneCount(pSplit) == 3, "pane count");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 0) != NULL, "pane widget 0");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 1) != NULL, "pane widget 1");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneWidget(pSplit, 2) != NULL, "pane widget 2");
	XUI_TEST_CHECK(xuiSplitLayoutGetOrientation(pSplit) == XUI_ORIENTATION_VERTICAL, "vertical orientation");
	iRet = xuiSplitLayoutGetDividerMetrics(pSplit, &fLayoutSize, &fVisualSize, &fHitSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fLayoutSize == 10.0f && fVisualSize == 3.0f && fHitSize == 14.0f, "metrics");

	tPane0 = xuiSplitLayoutGetPaneRect(pSplit, 0);
	tPane1 = xuiSplitLayoutGetPaneRect(pSplit, 1);
	tPane2 = xuiSplitLayoutGetPaneRect(pSplit, 2);
	tDivider = xuiSplitLayoutGetDividerLayoutRect(pSplit, 0);
	tVisual = xuiSplitLayoutGetDividerVisualRect(pSplit, 0);
	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, 0);
	XUI_TEST_CHECK(__xuiSplitNear(tPane0.fW, 120.0f), "left size");
	XUI_TEST_CHECK(__xuiSplitNear(tPane1.fW, 190.0f), "center grow size");
	XUI_TEST_CHECK(__xuiSplitNear(tPane2.fW, 130.0f), "right size");
	XUI_TEST_CHECK(__xuiSplitNear(tDivider.fX, 120.0f) && __xuiSplitNear(tDivider.fW, 10.0f), "divider layout rect");
	XUI_TEST_CHECK(__xuiSplitNear(tVisual.fW, 3.0f) && tHit.fW > tVisual.fW, "visual hit separation");

	tWorld = xuiWidgetGetWorldRect(pSplit);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetHoverDivider(pSplit) == 0, "hover divider");
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pSplit) == 0, "active divider");
	iRet = xuiInputPointerMove(pContext, fX + 30.0f, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged == 0, "shadow move no commit");
	XUI_TEST_CHECK(xuiOverlayTop(pContext) != NULL &&
	               xuiOverlayGetOwner(xuiOverlayTop(pContext)) == pSplit &&
	               xuiWidgetGetVisible(xuiOverlayTop(pContext)) != 0, "shadow overlay visible");
	tShadow = xuiSplitLayoutGetShadowRect(pSplit);
	XUI_TEST_CHECK(tShadow.fW >= 3.0f && tShadow.fH >= 170.0f, "shadow rect");
	iRet = xuiInputPointerUp(pContext, fX + 30.0f, fY, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "shadow up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiSplitLayoutGetActiveDivider(pSplit) == -1, "shadow release");
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after shadow commit");
	XUI_TEST_CHECK(iChanged == 1, "shadow commit callback");
	XUI_TEST_CHECK(__xuiSplitNear(xuiSplitLayoutGetPaneFixedSize(pSplit, 0), 150.0f), "shadow commit fixed size");

	iRet = xuiSplitLayoutSetShadowDrag(pSplit, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetShadowDrag(pSplit) == 0, "live drag mode");
	tHit = xuiSplitLayoutGetDividerHitRect(pSplit, 1);
	tWorld = xuiWidgetGetWorldRect(pSplit);
	fX = tWorld.fX + tHit.fX + tHit.fW * 0.5f;
	fY = tWorld.fY + tHit.fY + tHit.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "live down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetActiveDivider(pSplit) == 1, "live active");
	iRet = xuiInputPointerMove(pContext, fX - 20.0f, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "live move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSplitLayoutGetChangeCount(pSplit) > 1 && iChanged > 1, "live commit on move");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiSplitLayoutGetActiveDivider(pSplit) == -1, "escape cancels live drag");
	(void)xuiInputPointerUp(pContext, fX - 20.0f, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSplitLayoutSetOrientation(pSplit, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal set");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 0, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal top fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 0, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal top min size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 1, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal center min size");
	iRet = xuiSplitLayoutSetPaneFixedSize(pSplit, 2, 60.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal bottom fixed size");
	iRet = xuiSplitLayoutSetPaneMinSize(pSplit, 2, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal bottom min size");
	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 260.0f, 160.0f});
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal render");
	tPane0 = xuiSplitLayoutGetPaneRect(pSplit, 0);
	tDivider = xuiSplitLayoutGetDividerLayoutRect(pSplit, 0);
	XUI_TEST_CHECK(tPane0.fH > 0.0f && tPane0.fW > 250.0f, "horizontal pane spans width");
	XUI_TEST_CHECK(__xuiSplitNear(tDivider.fY, tPane0.fY + tPane0.fH), "horizontal divider y");

	xuiWidgetSetRect(pSplit, (xui_rect_t){20.0f, 20.0f, 170.0f, 100.0f});
	iRet = __xuiSplitDispatchLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "oversubscribed render");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 0) >= 0.0f, "oversub pane 0 nonnegative");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 1) >= 0.0f, "oversub pane 1 nonnegative");
	XUI_TEST_CHECK(xuiSplitLayoutGetPaneSize(pSplit, 2) >= 0.0f, "oversub pane 2 nonnegative");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_split_layout_test passed\n");
	return 0;
}
