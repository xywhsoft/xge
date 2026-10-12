#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_toast_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_toast_test_state_t {
	int iClickCount;
	int iLastClickId;
	int iCloseCount;
	int iLastCloseId;
	int iLastReason;
} xui_toast_test_state_t;

static int __xuiToastDispatchMove(xui_context pContext, float fX, float fY, uint32_t iButtons)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, iButtons);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiToastDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiToastDispatchPointerDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = __xuiToastDispatchMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiToastDispatchDown(pContext, fX, fY);
}

static void __xuiToastClick(xui_toast pToast, int iToastId, void* pUser)
{
	xui_toast_test_state_t* pState;

	(void)pToast;
	pState = (xui_toast_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iClickCount++;
		pState->iLastClickId = iToastId;
	}
}

static void __xuiToastClose(xui_toast pToast, int iToastId, int iReason, void* pUser)
{
	xui_toast_test_state_t* pState;

	(void)pToast;
	pState = (xui_toast_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iCloseCount++;
		pState->iLastCloseId = iToastId;
		pState->iLastReason = iReason;
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_toast_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_toast pToast;
	xui_toast_desc_t tDesc;
	xui_toast_metrics_t tMetrics;
	xui_toast_colors_t tColors;
	xui_surface pCache;
	xui_font pFont;
	xui_rect_t tItem0;
	xui_rect_t tItem1;
	xui_rect_t tClose;
	xui_widget pHit;
	int iLayer;
	int iZ;
	int iId1;
	int iId2;
	int iId3;
	int iId4;
	int iId5;
	int iFailed;
	int iRet;

	memset(&tState, 0, sizeof(tState));
	pContext = NULL;
	pRoot = NULL;
	pToast = NULL;
	pFont = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tProxyState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 640.0f, 420.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "body", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 420.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pFont;
	tDesc.iPlacement = XUI_TOAST_PLACEMENT_TOP_RIGHT;
	tDesc.iDirection = XUI_TOAST_DIRECTION_AUTO;
	tDesc.bHasMetrics = 1;
	tDesc.tMetrics.iSize = sizeof(tDesc.tMetrics);
	tDesc.tMetrics.fWidth = 300.0f;
	tDesc.tMetrics.fMinWidth = 180.0f;
	tDesc.tMetrics.fMinHeight = 58.0f;
	tDesc.tMetrics.fMargin = 18.0f;
	tDesc.tMetrics.fGap = 8.0f;
	tDesc.tMetrics.fPaddingX = 14.0f;
	tDesc.tMetrics.fPaddingY = 10.0f;
	tDesc.tMetrics.fIconSize = 28.0f;
	tDesc.tMetrics.fIconGap = 10.0f;
	tDesc.tMetrics.fCloseSize = 16.0f;
	tDesc.tMetrics.fProgressHeight = 2.0f;
	tDesc.tMetrics.iMaxVisible = 2;
	iRet = xuiToastCreate(pContext, &pToast, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pToast != NULL, "toast create");
	iRet = xuiToastSetClose(pToast, __xuiToastClose, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 0 && xuiToastGetPendingCount(pToast) == 0, "initial counts");

	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_SUCCESS, "Saved", "Settings were written to disk.", 0.10f, NULL, NULL);
	iId2 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Build finished", "Click this toast to open the report.", 5.0f, __xuiToastClick, &tState);
	iId3 = xuiToastShow(pToast, XUI_TOAST_TYPE_WARNING, "Network", "Remote sync is slow. Local cache is being used.", 5.0f, __xuiToastClick, &tState);
	XUI_TEST_CHECK(iId1 > 0 && iId2 > 0 && iId3 > 0, "show ids");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2 && xuiToastGetPendingCount(pToast) == 1, "queue counts");
	XUI_TEST_CHECK(xuiToastGetShowCount(pToast) == 3, "show count");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tItem0 = xuiToastGetItemRect(pToast, 0);
	tItem1 = xuiToastGetItemRect(pToast, 1);
	tMetrics = (xui_toast_metrics_t){0};
	iRet = xuiToastGetMetrics(pToast, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fIconSize >= 28.0f, "large icon metrics");
	XUI_TEST_CHECK(tItem0.fW >= 180.0f && tItem0.fH >= 58.0f && tItem0.fX > 300.0f && tItem0.fY >= 18.0f, "item0 rect");
	XUI_TEST_CHECK(tItem1.fY > tItem0.fY && tItem1.fW == tItem0.fW, "item1 stack");
	iRet = xuiWidgetGetLayer(xuiToastGetItemWidget(pToast, 0), &iLayer, &iZ);
	XUI_TEST_CHECK(iRet == XUI_OK && iLayer == XUI_LAYER_TOOLTIP && iZ >= 120, "tooltip layer");
	pHit = xuiHitTest(pContext, 20.0f, 20.0f, XUI_WIDGET_HIT_DEFAULT);
	XUI_TEST_CHECK(pHit == pRoot, "transparent outside hit area");

	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pCache = xuiWidgetGetCacheSurface(xuiToastGetItemWidget(pToast, 0), xuiWidgetGetStateId(xuiToastGetItemWidget(pToast, 0)));
	XUI_TEST_CHECK(pCache != NULL, "item cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pCache) > 0, "item rendered");
	XUI_TEST_CHECK(xuiTestSurfaceGetTextDrawCount(pCache) > 0, "text rendered");

	iRet = xuiUpdate(pContext, 0.05f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 2 && xuiToastGetPendingCount(pToast) == 1, "before expire");
	iRet = xuiUpdate(pContext, 0.06f);
	XUI_TEST_CHECK(iRet == XUI_OK, "expire update");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2 && xuiToastGetPendingCount(pToast) == 0, "pending activated");
	XUI_TEST_CHECK(xuiToastGetExpireCount(pToast) == 1 && tState.iLastCloseId == iId1 && tState.iLastReason == XUI_TOAST_CLOSE_TIMEOUT, "expire reason");

	tClose = xuiToastGetCloseRect(pToast, 0);
	iRet = __xuiToastDispatchPointerDown(pContext, tClose.fX + tClose.fW * 0.5f, tClose.fY + tClose.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "close button down");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2, "close button deferred");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1, "close button processed");
	XUI_TEST_CHECK(tState.iLastCloseId == iId2 && tState.iLastReason == XUI_TOAST_CLOSE_BUTTON && tState.iClickCount == 0, "close button reason");

	tItem0 = xuiToastGetItemRect(pToast, 0);
	iRet = __xuiToastDispatchPointerDown(pContext, tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "body down");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 1, "body click deferred");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0, "body click processed");
	XUI_TEST_CHECK(tState.iClickCount == 1 && tState.iLastClickId == iId3 && tState.iLastReason == XUI_TOAST_CLOSE_CLICK, "body click callback");

	iRet = xuiToastGetMetrics(pToast, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "metrics get");
	tMetrics.iMaxVisible = 1;
	iRet = xuiToastSetMetrics(pToast, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetMetrics(pToast, &tMetrics) == XUI_OK && tMetrics.iMaxVisible == 1, "metrics set");
	iRet = xuiToastGetColors(pToast, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "colors get");
	tColors.iSuccessColor = XUI_COLOR_RGBA(32, 150, 110, 255);
	iRet = xuiToastSetColors(pToast, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK, "colors set");
	iRet = xuiToastSetPlacement(pToast, XUI_TOAST_PLACEMENT_BOTTOM_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetPlacement(pToast) == XUI_TOAST_PLACEMENT_BOTTOM_LEFT, "placement set");
	iRet = xuiToastSetDirection(pToast, XUI_TOAST_DIRECTION_UP);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetDirection(pToast) == XUI_TOAST_DIRECTION_UP, "direction set");

	iId4 = xuiToastShow(pToast, XUI_TOAST_TYPE_ERROR, "One", "Visible.", 5.0f, NULL, NULL);
	iId5 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Two", "Pending.", 5.0f, NULL, NULL);
	XUI_TEST_CHECK(iId4 > 0 && iId5 > 0 && xuiToastGetActiveCount(pToast) == 1 && xuiToastGetPendingCount(pToast) == 1, "single visible queue");
	iRet = xuiToastClose(pToast, iId5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1 && xuiToastGetPendingCount(pToast) == 0, "close pending");
	XUI_TEST_CHECK(tState.iLastCloseId == iId5 && tState.iLastReason == XUI_TOAST_CLOSE_API, "pending close reason");
	iRet = xuiToastClear(pToast);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0 && xuiToastGetPendingCount(pToast) == 0, "clear");
	XUI_TEST_CHECK(tState.iLastCloseId == iId4 && tState.iLastReason == XUI_TOAST_CLOSE_CLEAR, "clear reason");
	XUI_TEST_CHECK(xuiToastGetCloseCount(pToast) >= 4 && xuiToastGetChangeCount(pToast) > 0, "final counters");

cleanup:
	if ( pToast != NULL ) {
		xuiToastDestroy(pToast);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_toast_test passed\n");
	return 0;
}
