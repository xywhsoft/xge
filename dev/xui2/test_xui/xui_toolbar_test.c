#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_toolbar_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_toolbar_test_events_t {
	int iSelectCount;
	int iOverflowCount;
	int iLastIndex;
	int iLastValue;
	int iOverflowFirst;
	int iOverflowItems;
} xui_toolbar_test_events_t;

static int __xuiToolbarNear(float fA, float fB)
{
	float fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static void __xuiToolbarSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_toolbar_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_toolbar_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iSelectCount++;
		pEvents->iLastIndex = iIndex;
		pEvents->iLastValue = iValue;
	}
}

static void __xuiToolbarOverflow(xui_widget pWidget, int iFirst, int iCount, void* pUser)
{
	xui_toolbar_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_toolbar_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iOverflowCount++;
		pEvents->iOverflowFirst = iFirst;
		pEvents->iOverflowItems = iCount;
	}
}

static int __xuiToolbarLayoutRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;
	int iRet;

	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiToolbarDispatchMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiToolbarDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiToolbarDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiToolbarClickItem(xui_context pContext, xui_widget pToolbar, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pToolbar);
	tItem = xuiToolbarGetItemRect(pToolbar, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiToolbarDispatchMove(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiToolbarDispatchDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiToolbarDispatchUp(pContext, fX, fY);
}

static int __xuiToolbarKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_toolbar_test_events_t tEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pToolbar;
	xui_surface pTarget;
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_colors_t tColors;
	xui_toolbar_item_t arrItems[7];
	xui_rect_t tItem0;
	xui_rect_t tItem1;
	xui_rect_t tItem4;
	xui_rect_t tOverflow;
	xui_rect_t tWorld;
	int iFailed;
	int iRet;
	int iFirst;
	int iCount;

	pContext = NULL;
	pRoot = NULL;
	pToolbar = NULL;
	pTarget = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	tEvents.iLastIndex = -1;
	tEvents.iLastValue = -1;
	tEvents.iOverflowFirst = -1;
	tEvents.iOverflowItems = 0;
	memset(arrItems, 0, sizeof(arrItems));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	tMetrics.fItemWidth = 52.0f;
	tMetrics.fItemHeight = 26.0f;
	tMetrics.fSeparatorSize = 10.0f;
	tMetrics.fGroupGap = 8.0f;
	tMetrics.fPaddingX = 4.0f;
	tMetrics.fPaddingY = 3.0f;
	tMetrics.fOverflowSize = 24.0f;
	tMetrics.fBorderWidth = 1.0f;
	tMetrics.fIconSize = 0.0f;
	tMetrics.fIconGap = 4.0f;

	arrItems[0] = (xui_toolbar_item_t){"New", "Create scene", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 10, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[1] = (xui_toolbar_item_t){"Pin", "Toggle pin", XUI_TOOLBAR_ITEM_TOGGLE, XUI_TOOLBAR_ITEM_ENABLED, 11, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[2] = (xui_toolbar_item_t){"", "", XUI_TOOLBAR_ITEM_SEPARATOR, 0, 0, 0, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[3] = (xui_toolbar_item_t){"Save", "Disabled", XUI_TOOLBAR_ITEM_BUTTON, 0, 12, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[4] = (xui_toolbar_item_t){"Run", "Run command", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 13, 1, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[5] = (xui_toolbar_item_t){"Preview", "Preview", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 14, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[6] = (xui_toolbar_item_t){"Export", "Export", XUI_TOOLBAR_ITEM_BUTTON, XUI_TOOLBAR_ITEM_ENABLED, 15, 2, NULL, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 7;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	iRet = xuiToolbarCreate(pContext, &pToolbar, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pToolbar != NULL, "toolbar create");
	iRet = xuiWidgetAddChild(pRoot, pToolbar);
	XUI_TEST_CHECK(iRet == XUI_OK, "toolbar add");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 20.0f, 390.0f, 34.0f});
	iRet = xuiToolbarSetSelect(pToolbar, __xuiToolbarSelect, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");
	XUI_TEST_CHECK(xuiToolbarGetItemCount(pToolbar) == 7, "item count");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pToolbar, xuiWidgetGetStateId(pToolbar)) != NULL, "cache surface");

	tItem0 = xuiToolbarGetItemRect(pToolbar, 0);
	tItem1 = xuiToolbarGetItemRect(pToolbar, 1);
	tItem4 = xuiToolbarGetItemRect(pToolbar, 4);
	XUI_TEST_CHECK(__xuiToolbarNear(tItem0.fW, 52.0f) && tItem1.fX > tItem0.fX, "horizontal layout");
	XUI_TEST_CHECK(tItem4.fX > xuiToolbarGetItemRect(pToolbar, 3).fX, "group layout");
	XUI_TEST_CHECK(strcmp(xuiToolbarGetItemTooltip(pToolbar, 0), "Create scene") == 0, "tooltip metadata");

	iRet = __xuiToolbarClickItem(pContext, pToolbar, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetItemChecked(pToolbar, 1) && tEvents.iSelectCount == 1 && tEvents.iLastValue == 11, "toggle click");
	iRet = __xuiToolbarClickItem(pContext, pToolbar, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 1, "disabled ignored");

	iRet = xuiSetFocusWidget(pContext, pToolbar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus toolbar");
	iRet = xuiToolbarSetHoverIndex(pToolbar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToolbarGetHoverTooltip(pToolbar), "Create scene") == 0, "hover tooltip");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_SPACE);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 2 && tEvents.iLastValue == 10, "keyboard select");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == 1, "keyboard move");

	iRet = xuiToolbarSetOrientation(pToolbar, XUI_ORIENTATION_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical set");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 70.0f, 72.0f, 230.0f});
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical render");
	XUI_TEST_CHECK(xuiToolbarGetItemRect(pToolbar, 1).fY > xuiToolbarGetItemRect(pToolbar, 0).fY, "vertical layout");

	iRet = xuiToolbarSetOrientation(pToolbar, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal reset");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 20.0f, 210.0f, 34.0f});
	iRet = xuiToolbarSetOverflow(pToolbar, 1, 24.0f, __xuiToolbarOverflow, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow set");
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow render");
	iFirst = xuiToolbarGetOverflowFirst(pToolbar);
	iCount = xuiToolbarGetOverflowCount(pToolbar);
	tOverflow = xuiToolbarGetOverflowRect(pToolbar);
	XUI_TEST_CHECK(iFirst > 0 && iCount > 0 && tOverflow.fW > 0.0f, "overflow query");
	tWorld = xuiWidgetGetWorldRect(pToolbar);
	iRet = __xuiToolbarDispatchDown(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) != 0u, "overflow active");
	iRet = __xuiToolbarDispatchUp(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == 1 && tEvents.iOverflowFirst == iFirst && tEvents.iOverflowItems == iCount, "overflow callback");

	iRet = xuiToolbarGetMetrics(pToolbar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fItemWidth >= 1.0f, "metrics get");
	iRet = xuiToolbarGetColors(pToolbar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	XUI_TEST_CHECK(xuiToolbarGetSelectCount(pToolbar) == tEvents.iSelectCount, "select count");
	XUI_TEST_CHECK(xuiToolbarGetOverflowSelectCount(pToolbar) == tEvents.iOverflowCount, "overflow count");
	XUI_TEST_CHECK(xuiToolbarGetChangeCount(pToolbar) > 0, "change count");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_toolbar_test passed\n");
	return 0;
}
