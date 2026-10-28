#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_toolbar_test_destroy_t {
	int iCount;
} xui_toolbar_test_destroy_t;

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

static void __xuiToolbarDestroySelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_toolbar_test_destroy_t* pDestroy = (xui_toolbar_test_destroy_t*)pUser;
	(void)iIndex;
	(void)iValue;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
}

static void __xuiToolbarDestroyOverflow(xui_widget pWidget, int iFirst, int iCount, void* pUser)
{
	xui_toolbar_test_destroy_t* pDestroy = (xui_toolbar_test_destroy_t*)pUser;
	(void)iFirst;
	(void)iCount;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
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

static int __xuiToolbarDispatchDragMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT);
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
	xui_toolbar_test_destroy_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pToolbar;
	xui_widget pOwnedToolbar;
	xui_widget pDestroyToolbar;
	xui_widget pOverflowDestroyToolbar;
	xui_surface pTarget;
	xui_toolbar_desc_t tDesc;
	xui_toolbar_metrics_t tMetrics;
	xui_toolbar_metrics_t tBadMetrics;
	xui_toolbar_colors_t tColors;
	xui_toolbar_item_t arrItems[7];
	xui_toolbar_item_t tOwnedItem;
	xui_toolbar_item_t arrDestroyItems[3];
	xui_accessible_node_t tNode;
	xui_rect_t tItem0;
	xui_rect_t tItem1;
	xui_rect_t tItem4;
	xui_rect_t tOverflow;
	xui_rect_t tWorld;
	int iFailed;
	int iRet;
	int iFirst;
	int iCount;
	int iChangeCount;
	int iBefore;
	char sOwnedText[220];
	char sOwnedTooltip[64];

	pContext = NULL;
	pRoot = NULL;
	pToolbar = NULL;
	pOwnedToolbar = NULL;
	pDestroyToolbar = NULL;
	pOverflowDestroyToolbar = NULL;
	pTarget = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tDestroy, 0, sizeof(tDestroy));
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
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pToolbar) == 8 &&
		xuiWidgetGetAccessibleNode(pToolbar, 0, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_TOOLBAR &&
		tNode.iRowCount == 1 && tNode.iColumnCount == 7, "toolbar accessibility root");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pToolbar, 1, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_BUTTON && strcmp(tNode.sName, "New") == 0 &&
		tNode.iRow == 0 && tNode.iColumn == 0 && tNode.iRowCount == 1 && tNode.iColumnCount == 1,
		"toolbar accessibility item");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pToolbar, 4, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_FOCUSABLE) == 0u && tNode.iActions == 0u,
		"disabled item exposes no accessibility action");

	iRet = __xuiToolbarClickItem(pContext, pToolbar, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetItemChecked(pToolbar, 1) && tEvents.iSelectCount == 1 && tEvents.iLastValue == 11, "toggle click");
	iRet = __xuiToolbarClickItem(pContext, pToolbar, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 1, "disabled ignored");
	tWorld = xuiWidgetGetWorldRect(pToolbar);
	tItem0 = xuiToolbarGetItemRect(pToolbar, 0);
	tItem1 = xuiToolbarGetItemRect(pToolbar, 1);
	iBefore = tEvents.iSelectCount;
	iRet = __xuiToolbarDispatchDown(pContext,
		tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetActiveIndex(pToolbar) == 0 &&
		xuiToolbarGetHoverIndex(pToolbar) == 0, "item drag starts pressed");
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tItem1.fX + tItem1.fW * 0.5f,
		tWorld.fY + tItem1.fY + tItem1.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetActiveIndex(pToolbar) == 0 &&
		xuiToolbarGetHoverIndex(pToolbar) == -1,
		"dragging away preserves origin without hovering another item");
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == 0,
		"dragging back restores pressed target");
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tItem1.fX + tItem1.fW * 0.5f,
		tWorld.fY + tItem1.fY + tItem1.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarDispatchUp(pContext,
		tWorld.fX + tItem1.fX + tItem1.fW * 0.5f,
		tWorld.fY + tItem1.fY + tItem1.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore &&
		xuiToolbarGetActiveIndex(pToolbar) == -1 && xuiGetPointerCapture(pContext) == NULL,
		"release away cancels item activation");
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem0.fX + tItem0.fW * 0.5f),
		(int)(tWorld.fY + tItem0.fY + tItem0.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pToolbar &&
		xuiToolbarGetActiveIndex(pToolbar) == 0,
		"first touch owns toolbar item press");
	tItem1 = xuiToolbarGetItemRect(pToolbar, 1);
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem1.fX + tItem1.fW * 0.5f),
		(int)(tWorld.fY + tItem1.fY + tItem1.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem1.fX + tItem1.fW * 0.5f),
		(int)(tWorld.fY + tItem1.fY + tItem1.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem1.fX + tItem1.fW * 0.5f),
		(int)(tWorld.fY + tItem1.fY + tItem1.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pToolbar &&
		xuiToolbarGetActiveIndex(pToolbar) == 0,
		"second touch cannot alter or commit toolbar item press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem0.fX + tItem0.fW * 0.5f),
		(int)(tWorld.fY + tItem0.fY + tItem0.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 &&
		tEvents.iLastValue == 10 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiToolbarGetActiveIndex(pToolbar) == -1,
		"owning touch commits toolbar item exactly once");
	iRet = __xuiToolbarDispatchDown(pContext, tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pToolbar, "item captures pointer");
	iRet = xuiToolbarSetItemEnabled(pToolbar, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiToolbarGetActiveIndex(pToolbar) == -1, "disabling pressed item releases capture");
	XUI_TEST_CHECK(__xuiToolbarDispatchUp(pContext, tWorld.fX + tItem0.fX, tWorld.fY + tItem0.fY) == XUI_OK,
		"release after disabled press");
	XUI_TEST_CHECK(xuiToolbarSetItemEnabled(pToolbar, 0, 1) == XUI_OK, "re-enable pressed item");

	iRet = xuiSetFocusWidget(pContext, pToolbar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus toolbar");
	iRet = xuiToolbarSetHoverIndex(pToolbar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToolbarGetHoverTooltip(pToolbar), "Create scene") == 0, "hover tooltip");
	iBefore = tEvents.iSelectCount;
	iRet = __xuiToolbarKey(pContext, XUI_KEY_SPACE);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 && tEvents.iLastValue == 10, "keyboard select");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == 1, "keyboard move");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_END);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == 6, "keyboard end");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_HOME);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == 0, "keyboard home");

	iRet = xuiToolbarSetOrientation(pToolbar, XUI_ORIENTATION_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical set");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 70.0f, 72.0f, 230.0f});
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical render");
	XUI_TEST_CHECK(xuiToolbarGetItemRect(pToolbar, 1).fY > xuiToolbarGetItemRect(pToolbar, 0).fY, "vertical layout");

	iRet = xuiToolbarSetOrientation(pToolbar, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal reset");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 20.0f, 350.0f, 34.0f});
	iRet = xuiToolbarSetOverflow(pToolbar, 1, 24.0f, __xuiToolbarOverflow, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow set");
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetOverflowCount(pToolbar) == 0,
		"overflow button does not reserve space when all items fit");
	XUI_TEST_CHECK(xuiToolbarSetHoverIndex(pToolbar, 6) == XUI_OK &&
		xuiToolbarGetHoverIndex(pToolbar) == 6, "focus last item before overflow resize");
	(void)xuiWidgetSetRect(pToolbar, (xui_rect_t){20.0f, 20.0f, 210.0f, 34.0f});
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == -1,
		"overflow resize clears hidden focus");
	iFirst = xuiToolbarGetOverflowFirst(pToolbar);
	iCount = xuiToolbarGetOverflowCount(pToolbar);
	tOverflow = xuiToolbarGetOverflowRect(pToolbar);
	XUI_TEST_CHECK(iFirst > 0 && iCount > 0 && tOverflow.fW > 0.0f, "overflow query");
	XUI_TEST_CHECK(xuiToolbarSetHoverIndex(pToolbar, 6) == XUI_OK &&
		xuiToolbarGetHoverIndex(pToolbar) == -1,
		"hidden overflow item cannot be focused directly");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pToolbar, 7, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u && tNode.iActions == 0u &&
		xuiWidgetPerformAccessibleAction(pToolbar, 8u,
			XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_ERROR_INVALID_STATE,
		"overflowed item exposes no direct accessibility action");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_END);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == -1,
		"keyboard end targets overflow button");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pToolbar, 8, &tNode) == XUI_OK &&
		tNode.iId == (uint64_t)XUI_TOOLBAR_ITEM_CAPACITY + 2u &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_BUTTON &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_SELECTED) != 0u &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_FOCUSED) != 0u &&
		tNode.iRow == 0 && tNode.iColumn == 7,
		"overflow accessibility node tracks keyboard focus");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == 1 &&
		tEvents.iOverflowFirst == iFirst && tEvents.iOverflowItems == iCount,
		"keyboard activates overflow");
	iRet = __xuiToolbarKey(pContext, XUI_KEY_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) >= 0 &&
		xuiToolbarGetHoverIndex(pToolbar) < iFirst &&
		xuiToolbarGetItemRect(pToolbar, xuiToolbarGetHoverIndex(pToolbar)).fW > 0.0f,
		"keyboard leaves overflow for last visible item");
	iRet = xuiWidgetPerformAccessibleAction(pToolbar,
		(uint64_t)XUI_TOOLBAR_ITEM_CAPACITY + 2u, XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetHoverIndex(pToolbar) == -1,
		"accessibility focuses overflow");
	iRet = xuiWidgetPerformAccessibleAction(pToolbar,
		(uint64_t)XUI_TOOLBAR_ITEM_CAPACITY + 2u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == 2,
		"accessibility activates overflow");
	tWorld = xuiWidgetGetWorldRect(pToolbar);
	iBefore = tEvents.iOverflowCount;
	iRet = xuiInputPointerDownEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) == pToolbar &&
		(xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) != 0u,
		"first touch owns toolbar overflow press");
	iRet = xuiInputPointerDownEx(pContext, 512u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 512u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) == pToolbar &&
		(xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) != 0u,
		"second touch cannot commit or release toolbar overflow");
	iRet = xuiInputPointerUpEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == iBefore + 1 &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch commits toolbar overflow exactly once");
	iRet = __xuiToolbarDispatchDown(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) != 0u, "overflow active");
	iBefore = tEvents.iOverflowCount;
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) == 0u &&
		xuiGetPointerCapture(pContext) == pToolbar,
		"overflow press visual clears while dragged away");
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiToolbarGetState(pToolbar) & XUI_TOOLBAR_STATE_OVERFLOW_ACTIVE) != 0u,
		"overflow press visual restores on reentry");
	iRet = __xuiToolbarDispatchDragMove(pContext,
		tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarDispatchUp(pContext,
		tWorld.fX + tItem0.fX + tItem0.fW * 0.5f,
		tWorld.fY + tItem0.fY + tItem0.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == iBefore,
		"release away cancels overflow activation");
	iRet = __xuiToolbarDispatchDown(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	if ( iRet == XUI_OK ) iRet = __xuiToolbarDispatchUp(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iOverflowCount == iBefore + 1 &&
		tEvents.iOverflowFirst == iFirst && tEvents.iOverflowItems == iCount,
		"overflow callback");

	iRet = xuiToolbarGetMetrics(pToolbar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fItemWidth >= 1.0f, "metrics get");
	iRet = xuiToolbarGetColors(pToolbar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	iChangeCount = xuiToolbarGetChangeCount(pToolbar);
	iRet = xuiToolbarSetMetrics(pToolbar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToolbarGetChangeCount(pToolbar) == iChangeCount,
		"idempotent metrics do not change state");
	tBadMetrics = tMetrics;
	tBadMetrics.fItemWidth = NAN;
	XUI_TEST_CHECK(xuiToolbarSetMetrics(pToolbar, &tBadMetrics) == XUI_ERROR_INVALID_ARGUMENT,
		"nan metrics rejected");
	tBadMetrics = tMetrics;
	tBadMetrics.fItemWidth = XUI_LAYOUT_UNBOUNDED;
	iChangeCount = xuiToolbarGetChangeCount(pToolbar);
	XUI_TEST_CHECK(xuiToolbarSetMetrics(pToolbar, &tBadMetrics) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiToolbarGetChangeCount(pToolbar) == iChangeCount,
		"combined geometry overflow rejected transactionally");
	XUI_TEST_CHECK(xuiToolbarSetOverflow(pToolbar, 1, NAN, __xuiToolbarOverflow, &tEvents) == XUI_ERROR_INVALID_ARGUMENT,
		"nan overflow size rejected");
	tOwnedItem = arrItems[0];
	tOwnedItem.iState |= 0x80000000u;
	iChangeCount = xuiToolbarGetChangeCount(pToolbar);
	XUI_TEST_CHECK(xuiToolbarSetItems(pToolbar, &tOwnedItem, 1) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiToolbarGetItemCount(pToolbar) == 7 && xuiToolbarGetChangeCount(pToolbar) == iChangeCount,
		"unknown item state rejected transactionally");
	XUI_TEST_CHECK(xuiToolbarSetItemChecked(pToolbar, 2, 1) == XUI_ERROR_INVALID_ARGUMENT,
		"separator cannot be checked");
	tOwnedItem = arrItems[0];
	tOwnedItem.sText = sOwnedText;
	tOwnedItem.sTooltip = sOwnedTooltip;
	memset(sOwnedText, 'L', sizeof(sOwnedText) - 1u);
	sOwnedText[sizeof(sOwnedText) - 1u] = '\0';
	strcpy(sOwnedTooltip, "Owned tooltip");
	iRet = xuiToolbarCreate(pContext, &pOwnedToolbar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pOwnedToolbar != NULL, "owned toolbar create");
	iRet = xuiToolbarSetItems(pOwnedToolbar, &tOwnedItem, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "owned toolbar items");
	sOwnedText[0] = 'X';
	sOwnedTooltip[0] = 'X';
	XUI_TEST_CHECK(xuiToolbarGetItem(pOwnedToolbar, 0)->sText[0] == 'L' &&
		xuiToolbarGetItemTooltip(pOwnedToolbar, 0)[0] == 'O', "toolbar owns item strings");

	memset(arrDestroyItems, 0, sizeof(arrDestroyItems));
	arrDestroyItems[0] = (xui_toolbar_item_t){"Destroy", "", XUI_TOOLBAR_ITEM_BUTTON,
		XUI_TOOLBAR_ITEM_ENABLED, 77, 0, NULL, {0}, {0}, NULL};
	iRet = xuiToolbarCreate(pContext, &pDestroyToolbar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyToolbar != NULL, "destroy callback toolbar create");
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pDestroyToolbar) == XUI_OK, "destroy callback toolbar add");
	(void)xuiWidgetSetRect(pDestroyToolbar, (xui_rect_t){20.0f, 120.0f, 100.0f, 34.0f});
	XUI_TEST_CHECK(xuiToolbarSetItems(pDestroyToolbar, arrDestroyItems, 1) == XUI_OK &&
		xuiToolbarSetSelect(pDestroyToolbar, __xuiToolbarDestroySelect, &tDestroy) == XUI_OK,
		"destroy callback toolbar setup");
	XUI_TEST_CHECK(__xuiToolbarClickItem(pContext, pDestroyToolbar, 0) == XUI_OK && tDestroy.iCount == 1,
		"select callback may destroy toolbar");
	pDestroyToolbar = NULL;

	arrDestroyItems[1] = (xui_toolbar_item_t){"Two", "", XUI_TOOLBAR_ITEM_BUTTON,
		XUI_TOOLBAR_ITEM_ENABLED, 78, 0, NULL, {0}, {0}, NULL};
	arrDestroyItems[2] = (xui_toolbar_item_t){"Three", "", XUI_TOOLBAR_ITEM_BUTTON,
		XUI_TOOLBAR_ITEM_ENABLED, 79, 0, NULL, {0}, {0}, NULL};
	iRet = xuiToolbarCreate(pContext, &pOverflowDestroyToolbar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pOverflowDestroyToolbar != NULL, "overflow destroy toolbar create");
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pOverflowDestroyToolbar) == XUI_OK, "overflow destroy toolbar add");
	(void)xuiWidgetSetRect(pOverflowDestroyToolbar, (xui_rect_t){140.0f, 120.0f, 90.0f, 34.0f});
	XUI_TEST_CHECK(xuiToolbarSetItems(pOverflowDestroyToolbar, arrDestroyItems, 3) == XUI_OK &&
		xuiToolbarSetOverflow(pOverflowDestroyToolbar, 1, 24.0f, __xuiToolbarDestroyOverflow, &tDestroy) == XUI_OK,
		"overflow destroy toolbar setup");
	iRet = __xuiToolbarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow destroy toolbar render");
	tOverflow = xuiToolbarGetOverflowRect(pOverflowDestroyToolbar);
	tWorld = xuiWidgetGetWorldRect(pOverflowDestroyToolbar);
	XUI_TEST_CHECK(__xuiToolbarDispatchDown(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f) == XUI_OK, "overflow destroy down");
	XUI_TEST_CHECK(__xuiToolbarDispatchUp(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f) == XUI_OK && tDestroy.iCount == 2,
		"overflow callback may destroy toolbar");
	pOverflowDestroyToolbar = NULL;
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
