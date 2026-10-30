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
	xui_toast* ppToast;
	int iClickCount;
	int iLastClickId;
	int iCloseCount;
	int iLastCloseId;
	int iLastReason;
	int iDestroyCount;
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

static int __xuiToastDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
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

static void __xuiToastDestroyOnClick(xui_toast pToast, int iToastId, void* pUser)
{
	xui_toast_test_state_t* pState;

	pState = (xui_toast_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iClickCount++;
		pState->iLastClickId = iToastId;
		pState->iDestroyCount++;
	}
	xuiToastDestroy(pToast);
	if ( (pState != NULL) && (pState->ppToast != NULL) && (*pState->ppToast == pToast) ) {
		*pState->ppToast = NULL;
	}
}

static void __xuiToastDestroyOnClose(xui_toast pToast, int iToastId, int iReason, void* pUser)
{
	xui_toast_test_state_t* pState;

	pState = (xui_toast_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iCloseCount++;
		pState->iLastCloseId = iToastId;
		pState->iLastReason = iReason;
		pState->iDestroyCount++;
	}
	xuiToastDestroy(pToast);
	if ( (pState != NULL) && (pState->ppToast != NULL) && (*pState->ppToast == pToast) ) {
		*pState->ppToast = NULL;
	}
}

static int __xuiToastShowOomPreserves(xui_context pContext,
	const xui_toast_desc_t* pDesc)
{
	static const size_t arrBudgets[] = {
		1u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u,
		8192u, 16384u, 32768u, 65536u, 131072u, 262144u, 1048576u
	};
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xui_toast pProbe;
	int iBudget;
	int iId;
	int bSawFailure;
	int bSawLateFailure;
	int bSawSuccess;

	if ( pContext == NULL || pDesc == NULL ) return 0;
	bSawFailure = 0;
	bSawLateFailure = 0;
	bSawSuccess = 0;
	for ( iBudget = 0; iBudget < (int)(sizeof(arrBudgets) / sizeof(arrBudgets[0])); iBudget++ ) {
		pProbe = NULL;
		if ( xuiToastCreate(pContext, &pProbe, pDesc) != XUI_OK || pProbe == NULL ) return 0;
		pBudget = NULL;
		memset(&tScope, 0, sizeof(tScope));
		if ( !xrtAllocationBudgetCreate(arrBudgets[iBudget], &pBudget) ) {
			xuiToastDestroy(pProbe);
			return 0;
		}
		if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			xuiToastDestroy(pProbe);
			return 0;
		}
		iId = xuiToastShow(pProbe, XUI_TOAST_TYPE_INFO,
			"Allocation failure transaction",
			"The notification model must remain unchanged when text layout cannot be allocated.",
			5.0f, NULL, NULL);
		(void)xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
		if ( iId < 0 ) {
			bSawFailure = 1;
			if ( arrBudgets[iBudget] >= 512u ) bSawLateFailure = 1;
			if ( xuiToastGetActiveCount(pProbe) != 0 ||
			     xuiToastGetPendingCount(pProbe) != 0 ||
			     xuiToastGetShowCount(pProbe) != 0 ||
			     xuiToastGetDropCount(pProbe) != 0 ||
			     xuiToastGetChangeCount(pProbe) != 0 ) {
				xuiToastDestroy(pProbe);
				return 0;
			}
			iId = xuiToastShow(pProbe, XUI_TOAST_TYPE_INFO,
				"Retry", "The first committed notification keeps id one.",
				5.0f, NULL, NULL);
			if ( iId != 1 ) {
				xuiToastDestroy(pProbe);
				return 0;
			}
		} else {
			bSawSuccess = 1;
			if ( iId != 1 || xuiToastGetActiveCount(pProbe) != 1 ||
			     xuiToastGetPendingCount(pProbe) != 0 ||
			     xuiToastGetShowCount(pProbe) != 1 ) {
				xuiToastDestroy(pProbe);
				return 0;
			}
		}
		xuiToastDestroy(pProbe);
		if ( bSawFailure && bSawLateFailure && bSawSuccess ) break;
	}
	return bSawFailure && bSawLateFailure && bSawSuccess;
}

static int __xuiToastMetricsOomPreserves(xui_context pContext,
	const xui_toast_desc_t* pDesc)
{
	static const size_t arrBudgets[] = {
		1u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u,
		8192u, 16384u, 32768u, 65536u, 131072u, 262144u, 1048576u
	};
	xallocationbudget* pBudget;
	xallocationscope tScope;
	xui_toast_desc_t tProbeDesc;
	xui_toast_metrics_t tBefore;
	xui_toast_metrics_t tAfter;
	xui_toast_metrics_t tRequested;
	xui_toast pProbe;
	xui_rect_t tRectBefore;
	xui_rect_t tRectAfter;
	int iChangeBefore;
	int iBudget;
	int iRet;
	int bSawFailure;
	int bSawSuccess;

	if ( pContext == NULL || pDesc == NULL ) return 0;
	tProbeDesc = *pDesc;
	tProbeDesc.tMetrics.iMaxVisible = 1;
	bSawFailure = 0;
	bSawSuccess = 0;
	for ( iBudget = 0; iBudget < (int)(sizeof(arrBudgets) / sizeof(arrBudgets[0])); iBudget++ ) {
		pProbe = NULL;
		if ( xuiToastCreate(pContext, &pProbe, &tProbeDesc) != XUI_OK || pProbe == NULL ) return 0;
		if ( xuiToastShow(pProbe, XUI_TOAST_TYPE_INFO,
			"Visible", "The first notification remains visible.", 0.0f, NULL, NULL) <= 0 ||
		     xuiToastShow(pProbe, XUI_TOAST_TYPE_INFO,
			"Pending", "The second notification remains queued.", 0.0f, NULL, NULL) <= 0 ||
		     xuiToastGetMetrics(pProbe, &tBefore) != XUI_OK ) {
			xuiToastDestroy(pProbe);
			return 0;
		}
		tRequested = tBefore;
		tRequested.iMaxVisible = 2;
		tRectBefore = xuiToastGetItemRect(pProbe, 0);
		iChangeBefore = xuiToastGetChangeCount(pProbe);
		pBudget = NULL;
		memset(&tScope, 0, sizeof(tScope));
		if ( !xrtAllocationBudgetCreate(arrBudgets[iBudget], &pBudget) ) {
			xuiToastDestroy(pProbe);
			return 0;
		}
		if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
			xrtAllocationBudgetFree(pBudget);
			xuiToastDestroy(pProbe);
			return 0;
		}
		iRet = xuiToastSetMetrics(pProbe, &tRequested);
		(void)xrtAllocationBudgetLeave(&tScope);
		xrtAllocationBudgetFree(pBudget);
		if ( iRet == XUI_ERROR_OUT_OF_MEMORY ) {
			bSawFailure = 1;
			tAfter = (xui_toast_metrics_t){0};
			tRectAfter = xuiToastGetItemRect(pProbe, 0);
			if ( xuiToastGetMetrics(pProbe, &tAfter) != XUI_OK ||
			     memcmp(&tBefore, &tAfter, sizeof(tBefore)) != 0 ||
			     xuiToastGetActiveCount(pProbe) != 1 ||
			     xuiToastGetPendingCount(pProbe) != 1 ||
			     xuiToastGetChangeCount(pProbe) != iChangeBefore ||
			     memcmp(&tRectBefore, &tRectAfter, sizeof(tRectBefore)) != 0 ) {
				xuiToastDestroy(pProbe);
				return 0;
			}
		} else if ( iRet == XUI_OK ) {
			bSawSuccess = 1;
			if ( xuiToastGetMetrics(pProbe, &tAfter) != XUI_OK ||
			     tAfter.iMaxVisible != 2 ||
			     xuiToastGetActiveCount(pProbe) != 2 ||
			     xuiToastGetPendingCount(pProbe) != 0 ||
			     xuiToastGetChangeCount(pProbe) != iChangeBefore + 1 ) {
				xuiToastDestroy(pProbe);
				return 0;
			}
		} else {
			xuiToastDestroy(pProbe);
			return 0;
		}
		xuiToastDestroy(pProbe);
		if ( bSawFailure && bSawSuccess ) break;
	}
	return bSawFailure && bSawSuccess;
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_toast_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_toast pToast;
	xui_toast pInvalidToast;
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
	int i;
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
	XUI_TEST_CHECK(__xuiToastShowOomPreserves(pContext, &tDesc),
		"show allocation failures preserve queue, counters, and id sequence");
	XUI_TEST_CHECK(__xuiToastMetricsOomPreserves(pContext, &tDesc),
		"metrics allocation failures preserve queue, metrics, and geometry");
	pInvalidToast = (xui_toast)(uintptr_t)1;
	tDesc.iPlacement = -1;
	iRet = xuiToastCreate(pContext, &pInvalidToast, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidToast == NULL, "invalid create clears output");
	tDesc.iPlacement = XUI_TOAST_PLACEMENT_TOP_RIGHT;
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
	xuiTestProxyResetTextCounters(&tProxyState);
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestProxyGetTextMeasureCount(&tProxyState) == 0 &&
		xuiTestProxyGetTextShapeCount(&tProxyState) == 0,
		"stable update skips text relayout");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){20, 30, 240, 180});
	XUI_TEST_CHECK(iRet == XUI_OK, "resize root");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestProxyGetTextShapeCount(&tProxyState) > 0,
		"root resize relayout");
	tItem0 = xuiToastGetItemRect(pToast, 0);
	tItem1 = xuiToastGetItemRect(pToast, 1);
	XUI_TEST_CHECK(tItem0.fX >= 20 && tItem0.fY >= 30 &&
		tItem0.fX + tItem0.fW <= 260 &&
		tItem1.fY + tItem1.fH <= 210,
		"resized stack stays inside root");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 16, 80});
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root relayout");
	tItem0 = xuiToastGetItemRect(pToast, 0);
	tItem1 = xuiToastGetItemRect(pToast, 1);
	tClose = xuiToastGetCloseRect(pToast, 0);
	XUI_TEST_CHECK(tItem0.fX >= 0 && tItem0.fY >= 0 &&
		tItem0.fX + tItem0.fW <= 16 &&
		tItem1.fY + tItem1.fH <= 80,
		"tiny stack stays inside root");
	XUI_TEST_CHECK(tClose.fX >= tItem0.fX && tClose.fY >= tItem0.fY &&
		tClose.fX + tClose.fW <= tItem0.fX + tItem0.fW &&
		tClose.fY + tClose.fH <= tItem0.fY + tItem0.fH,
		"tiny close button stays inside item");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny root render");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0, 0, 640, 420});
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore root relayout");
	tItem0 = xuiToastGetItemRect(pToast, 0);
	tItem1 = xuiToastGetItemRect(pToast, 1);
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
	xuiTestProxyResetTextCounters(&tProxyState);
	iRet = xuiWidgetInvalidate(xuiToastGetItemWidget(pToast, 0), XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate toast text");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tProxyState.iLastTextDrawSize > 0, "toast lines use bounded text lengths");

	iRet = xuiUpdate(pContext, 0.05f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 2 && xuiToastGetPendingCount(pToast) == 1, "before expire");
	iRet = xuiUpdate(pContext, 0.06f);
	XUI_TEST_CHECK(iRet == XUI_OK, "expire update");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2 && xuiToastGetPendingCount(pToast) == 0, "pending activated");
	XUI_TEST_CHECK(xuiToastGetExpireCount(pToast) == 1 && tState.iLastCloseId == iId1 && tState.iLastReason == XUI_TOAST_CLOSE_TIMEOUT, "expire reason");

	tClose = xuiToastGetCloseRect(pToast, 0);
	iRet = __xuiToastDispatchPointerDown(pContext, tClose.fX + tClose.fW * 0.5f, tClose.fY + tClose.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "close button down");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2, "close button waits for release");
	iRet = __xuiToastDispatchUp(pContext, tClose.fX + tClose.fW * 0.5f, tClose.fY + tClose.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "close button up");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 2, "close button deferred");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1, "close button processed");
	XUI_TEST_CHECK(tState.iLastCloseId == iId2 && tState.iLastReason == XUI_TOAST_CLOSE_BUTTON && tState.iClickCount == 0, "close button reason");

	tItem0 = xuiToastGetItemRect(pToast, 0);
	iRet = __xuiToastDispatchPointerDown(pContext, tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "body down");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 1, "body waits for release");
	iRet = __xuiToastDispatchUp(pContext, tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "body up");
	XUI_TEST_CHECK(xuiToastGetActiveCount(pToast) == 1, "body click deferred");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0, "body click processed");
	XUI_TEST_CHECK(tState.iClickCount == 1 && tState.iLastClickId == iId3 && tState.iLastReason == XUI_TOAST_CLOSE_CLICK, "body click callback");

	iRet = xuiToastGetMetrics(pToast, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "metrics get");
	tMetrics.fWidth = tMetrics.fMinWidth - 1.0f;
	XUI_TEST_CHECK(xuiToastSetMetrics(pToast, &tMetrics) == XUI_ERROR_INVALID_ARGUMENT, "reject inverted metrics");
	tMetrics.fWidth = 300.0f;
	tMetrics.iMaxVisible = XUI_TOAST_VISIBLE_CAPACITY + 1;
	XUI_TEST_CHECK(xuiToastSetMetrics(pToast, &tMetrics) == XUI_ERROR_INVALID_ARGUMENT, "reject excessive visible count");
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
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "First", "Expires after a long frame.", 0.10f, NULL, NULL);
	iId2 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Second", "Must receive its own visible duration.", 0.10f, NULL, NULL);
	XUI_TEST_CHECK(iId1 > 0 && iId2 > 0 &&
		xuiToastGetActiveCount(pToast) == 1 &&
		xuiToastGetPendingCount(pToast) == 1,
		"long frame queue setup");
	iRet = xuiUpdate(pContext, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1 &&
		xuiToastGetPendingCount(pToast) == 0 &&
		xuiToastGetExpireCount(pToast) == 2,
		"long frame expires only previously visible toast");
	iRet = xuiUpdate(pContext, 0.05f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1,
		"promoted toast starts with fresh duration");
	iRet = xuiUpdate(pContext, 0.06f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0 &&
		xuiToastGetExpireCount(pToast) == 3,
		"promoted toast expires on its own timeline");

	XUI_TEST_CHECK(xuiToastShow(pToast, -1, "Bad", "Type", 1.0f, NULL, NULL) < 0, "reject invalid type");
	XUI_TEST_CHECK(xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Bad", "Duration", -1.0f, NULL, NULL) < 0, "reject negative duration");
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Sticky", "Persistent notification.", 0.0f, NULL, NULL);
	XUI_TEST_CHECK(iId1 > 0 && xuiToastGetActiveCount(pToast) == 1, "sticky show");
	iRet = xuiUpdate(pContext, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 1, "zero duration remains visible");
	iRet = xuiToastClose(pToast, iId1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0, "close sticky");
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Active", "Keeps the visible slot occupied.", 0.0f, NULL, NULL);
	iId2 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Oldest", "Oldest pending toast.", 0.0f, NULL, NULL);
	XUI_TEST_CHECK(iId1 > 0 && iId2 > 0, "capacity queue setup");
	xuiTestProxyResetTextCounters(&tProxyState);
	for ( i = 0; i < XUI_TOAST_QUEUE_CAPACITY; i++ ) {
		iId3 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO,
			"Queued", "Pending toast.", 0.0f, NULL, NULL);
		XUI_TEST_CHECK(iId3 > 0, "capacity queue fill");
	}
	XUI_TEST_CHECK(xuiToastGetPendingCount(pToast) == XUI_TOAST_QUEUE_CAPACITY &&
		xuiToastGetDropCount(pToast) == 1 &&
		tState.iLastCloseId == iId2 &&
		tState.iLastReason == XUI_TOAST_CLOSE_CAPACITY_DROP,
		"capacity drops oldest pending toast");
	XUI_TEST_CHECK(xuiTestProxyGetTextShapeCount(&tProxyState) == 0,
		"pending enqueue skips visible relayout");
	iRet = xuiToastClear(pToast);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0 &&
		xuiToastGetPendingCount(pToast) == 0,
		"clear capacity queue");
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO,
		"Hover", "Hover pauses the visible timeout.", 0.10f, NULL, NULL);
	tItem0 = xuiToastGetItemRect(pToast, 0);
	iRet = xuiInputPointerMove(pContext,
		tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && iId1 > 0 &&
		xuiToastGetActiveCount(pToast) == 1,
		"hover pauses toast timeout");
	iRet = xuiInputPointerMove(pContext, 639.0f, 5.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiUpdate(pContext, 0.11f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToastGetActiveCount(pToast) == 0,
		"toast timeout resumes after pointer leave");

	tState.ppToast = &pToast;
	iRet = xuiToastSetClose(pToast, __xuiToastDestroyOnClose, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy close callback set");
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Destroy", "Close callback.", 5.0f, NULL, NULL);
	XUI_TEST_CHECK(iId1 > 0, "destroy close show");
	iRet = xuiToastClose(pToast, iId1);
	XUI_TEST_CHECK(iRet == XUI_OK && pToast == NULL && tState.iDestroyCount == 1, "close callback may destroy toast");

	iRet = xuiToastCreate(pContext, &pToast, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pToast != NULL, "toast recreate");
	tState.ppToast = &pToast;
	iId1 = xuiToastShow(pToast, XUI_TOAST_TYPE_INFO, "Destroy", "Click callback.", 5.0f, __xuiToastDestroyOnClick, &tState);
	XUI_TEST_CHECK(iId1 > 0, "destroy click show");
	tItem0 = xuiToastGetItemRect(pToast, 0);
	iRet = __xuiToastDispatchPointerDown(pContext, tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiToastDispatchUp(pContext, tItem0.fX + tItem0.fW * 0.5f, tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy click dispatch");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && pToast == NULL && tState.iDestroyCount == 2, "click callback may destroy toast");

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
