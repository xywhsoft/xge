#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_statusbar_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_statusbar_test_events_t {
	int iSelectCount;
	int iLastIndex;
	int iLastValue;
	int bClearOnSelect;
	int iClearResult;
	int iContextCount;
	int iContextIndex;
	int iContextValue;
} xui_statusbar_test_events_t;

typedef struct xui_statusbar_destroy_events_t {
	int iCount;
} xui_statusbar_destroy_events_t;

static int __xuiStatusBarNear(float fA, float fB)
{
	float fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static void __xuiStatusBarSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_statusbar_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_statusbar_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iSelectCount++;
		pEvents->iLastIndex = iIndex;
		pEvents->iLastValue = iValue;
		if ( pEvents->bClearOnSelect ) {
			pEvents->iClearResult = xuiStatusBarClear(pWidget);
		}
	}
}

static void __xuiStatusBarDestroySelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_statusbar_destroy_events_t* pEvents = (xui_statusbar_destroy_events_t*)pUser;
	(void)iIndex;
	(void)iValue;
	if ( pEvents != NULL ) pEvents->iCount++;
	(void)xuiWidgetDestroy(pWidget);
}

static int __xuiStatusBarContext(xui_widget pWidget, int iIndex, int iValue,
	float fX, float fY, void* pUser)
{
	xui_statusbar_test_events_t* pEvents = (xui_statusbar_test_events_t*)pUser;
	(void)pWidget;
	(void)fX;
	(void)fY;
	if ( pEvents != NULL ) {
		pEvents->iContextCount++;
		pEvents->iContextIndex = iIndex;
		pEvents->iContextValue = iValue;
	}
	return XUI_OK;
}

static int __xuiStatusBarLayoutRender(xui_context pContext, xui_surface pTarget)
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

static int __xuiStatusBarDispatchMove(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiStatusBarDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiStatusBarDispatchUp(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiStatusBarClickItem(xui_context pContext, xui_widget pStatusBar, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;
	float fX;
	float fY;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pStatusBar);
	tItem = xuiStatusBarGetItemRect(pStatusBar, iIndex);
	fX = tWorld.fX + tItem.fX + tItem.fW * 0.5f;
	fY = tWorld.fY + tItem.fY + tItem.fH * 0.5f;
	iRet = __xuiStatusBarDispatchMove(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiStatusBarDispatchDown(pContext, fX, fY);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiStatusBarDispatchUp(pContext, fX, fY);
}

static int __xuiStatusBarKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_statusbar_test_events_t tEvents;
	xui_statusbar_destroy_events_t tDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pStatusBar;
	xui_widget pChild;
	xui_widget pDestroyStatusBar;
	xui_surface pTarget;
	xui_font pFont;
	xui_statusbar_desc_t tDesc;
	xui_statusbar_metrics_t tMetrics;
	xui_statusbar_colors_t tColors;
	xui_statusbar_item_t arrItems[7];
	xui_statusbar_item_t arrEquivalent[7];
	xui_statusbar_item_t arrInvalid[2];
	xui_accessible_node_t tNode;
	xui_event_t tContextEvent;
	xui_event_t tEvent;
	const xui_statusbar_item_t* pFirstItem;
	xui_rect_t tItem0;
	xui_rect_t tItem1;
	xui_rect_t tFlex;
	xui_rect_t tCenter;
	xui_rect_t tRight;
	xui_rect_t tWorld;
	int iFailed;
	int iRet;
	int iBefore;
	int i;
	char sMutableText[32];

	pContext = NULL;
	pRoot = NULL;
	pStatusBar = NULL;
	pChild = NULL;
	pDestroyStatusBar = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tDestroy, 0, sizeof(tDestroy));
	tEvents.iLastIndex = -1;
	tEvents.iLastValue = -1;
	tEvents.iContextIndex = -1;
	memset(arrItems, 0, sizeof(arrItems));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadFile(&tState.tProxy, &pFont, "test.ttf", 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font load");
	(void)xuiSetDefaultFont(pContext, pFont);

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tMetrics, 0, sizeof(tMetrics));
	tMetrics.iSize = sizeof(tMetrics);
	tMetrics.fHeight = 28.0f;
	tMetrics.fPaddingX = 6.0f;
	tMetrics.fPaddingY = 2.0f;
	tMetrics.fGap = 4.0f;
	tMetrics.fItemPaddingX = 6.0f;
	tMetrics.fItemPaddingY = 3.0f;
	tMetrics.fProgressHeight = 8.0f;
	tMetrics.fBorderWidth = 1.0f;
	tMetrics.fTopBorderWidth = 1.0f;

	arrItems[0] = (xui_statusbar_item_t){"Ready", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_LEFT, XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE, 10, 74.0f, 0.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[1] = (xui_statusbar_item_t){"", XUI_STATUSBAR_ITEM_PROGRESS, XUI_STATUSBAR_SECTION_LEFT, XUI_STATUSBAR_ITEM_ENABLED, 0, 100.0f, 0.0f, 0.0f, 100.0f, 140.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[2] = (xui_statusbar_item_t){"", XUI_STATUSBAR_ITEM_SPACER, XUI_STATUSBAR_SECTION_LEFT, 0, 0, 20.0f, 0.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[3] = (xui_statusbar_item_t){"", XUI_STATUSBAR_ITEM_SPACER, XUI_STATUSBAR_SECTION_LEFT, 0, 0, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[4] = (xui_statusbar_item_t){"Center", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_CENTER, XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE, 20, 82.0f, 0.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[5] = (xui_statusbar_item_t){"UTF-8", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_RIGHT, XUI_STATUSBAR_ITEM_ENABLED | XUI_STATUSBAR_ITEM_CLICKABLE, 30, 58.0f, 0.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};
	arrItems[6] = (xui_statusbar_item_t){"Disabled", XUI_STATUSBAR_ITEM_TEXT, XUI_STATUSBAR_SECTION_RIGHT, XUI_STATUSBAR_ITEM_CLICKABLE, 40, 76.0f, 0.0f, 0.0f, 1.0f, 0.0f, {0.0f, 0.0f, 0.0f, 0.0f}, NULL};

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 7;
	tDesc.tMetrics = tMetrics;
	tDesc.bHasMetrics = 1;
	tDesc.pFont = pFont;
	iRet = xuiStatusBarCreate(pContext, &pStatusBar, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pStatusBar != NULL, "statusbar create");
	iRet = xuiWidgetAddChild(pRoot, pStatusBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "statusbar add");
	(void)xuiWidgetSetRect(pStatusBar, (xui_rect_t){20.0f, 260.0f, 460.0f, 28.0f});
	iRet = xuiStatusBarSetSelect(pStatusBar, __xuiStatusBarSelect, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiStatusBarSetContextMenu(pStatusBar, __xuiStatusBarContext, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiStatusBarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");
	XUI_TEST_CHECK(xuiStatusBarGetItemCount(pStatusBar) == 7, "item count");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pStatusBar, xuiWidgetGetStateId(pStatusBar)) != NULL, "cache surface");
	XUI_TEST_CHECK(xuiStatusBarGetProgress(pStatusBar, 1) == 100.0f, "progress clamp setitems");

	tItem0 = xuiStatusBarGetItemRect(pStatusBar, 0);
	tItem1 = xuiStatusBarGetItemRect(pStatusBar, 1);
	tFlex = xuiStatusBarGetItemRect(pStatusBar, 3);
	tCenter = xuiStatusBarGetItemRect(pStatusBar, 4);
	tRight = xuiStatusBarGetItemRect(pStatusBar, 5);
	XUI_TEST_CHECK(__xuiStatusBarNear(tItem0.fW, 74.0f) && tItem1.fX > tItem0.fX, "left layout");
	XUI_TEST_CHECK(tFlex.fW > 0.0f, "flex layout");
	XUI_TEST_CHECK(tCenter.fX >= 220.0f && tCenter.fX <= 235.0f, "center layout");
	XUI_TEST_CHECK(tItem1.fX + tItem1.fW <= tCenter.fX &&
		tCenter.fX + tCenter.fW <= tRight.fX, "statusbar sections do not overlap");
	XUI_TEST_CHECK(xuiStatusBarGetItemAt(pStatusBar, tCenter.fX + 2.0f, tCenter.fY + 2.0f) == 4, "item hit");
	XUI_TEST_CHECK(xuiStatusBarGetItemAt(pStatusBar, xuiStatusBarGetItemRect(pStatusBar, 6).fX + 2.0f, tCenter.fY + 2.0f) == -1, "disabled hit ignored");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pStatusBar) == 6 &&
		xuiWidgetGetAccessibleNode(pStatusBar, 0, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_GROUP && tNode.iRowCount == 1 && tNode.iColumnCount == 5,
		"statusbar accessibility root");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pStatusBar, 2, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_PROGRESS && tNode.tRange.iSize == sizeof(tNode.tRange) &&
		tNode.tRange.fMin == 0.0 && tNode.tRange.fMax == 100.0 && tNode.tRange.fValue == 100.0,
		"progress accessibility range");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pStatusBar, 5, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_BUTTON &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u && tNode.iActions == 0u,
		"disabled clickable item preserves button role without actions");

	iRet = xuiWidgetCreate(pContext, &pChild);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pStatusBar, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "statusbar child create");
	tWorld = xuiWidgetGetWorldRect(pStatusBar);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_MOVE;
	tEvent.pTarget = pChild;
	tEvent.fX = tWorld.fX + tItem0.fX + 2.0f;
	tEvent.fY = tWorld.fY + tItem0.fY + 2.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == -1,
		"descendant pointer move does not hover status item");
	XUI_TEST_CHECK(xuiStatusBarSetHoverIndex(pStatusBar, 0) == XUI_OK,
		"set hover before descendant leave");
	tEvent.iType = XUI_EVENT_POINTER_LEAVE;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 0,
		"descendant pointer leave does not clear status hover");
	XUI_TEST_CHECK(xuiStatusBarSetHoverIndex(pStatusBar, -1) == XUI_OK &&
		xuiSetFocusWidget(pContext, pStatusBar) == XUI_OK,
		"prepare descendant key isolation");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pChild;
	tEvent.iKey = XUI_KEY_RIGHT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == -1,
		"descendant key does not navigate status items");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pChild;
	tEvent.fX = tWorld.fX + tItem0.fX + 2.0f;
	tEvent.fY = tWorld.fY + tItem0.fY + 2.0f;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == -1 &&
		xuiGetPointerCapture(pContext) == NULL,
		"descendant pointer down does not activate status item");
	xuiWidgetDestroy(pChild);
	pChild = NULL;

	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tWorld = xuiWidgetGetWorldRect(pStatusBar);
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pStatusBar;
	tContextEvent.iButton = XUI_POINTER_BUTTON_RIGHT;
	tContextEvent.fX = tWorld.fX + tItem1.fX + 2.0f;
	tContextEvent.fY = tWorld.fY + tItem1.fY + 2.0f;
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iContextCount == 1 &&
		tEvents.iContextIndex == 1 && tEvents.iContextValue == 0,
		"context menu identifies non-clickable status item");

	iRet = __xuiStatusBarClickItem(pContext, pStatusBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 1 && tEvents.iLastIndex == 0 && tEvents.iLastValue == 10, "click select");
	iBefore = tEvents.iSelectCount;
	iRet = __xuiStatusBarClickItem(pContext, pStatusBar, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore, "disabled click ignored");

	tWorld = xuiWidgetGetWorldRect(pStatusBar);
	tItem0 = xuiStatusBarGetItemRect(pStatusBar, 0);
	tCenter = xuiStatusBarGetItemRect(pStatusBar, 4);
	iBefore = tEvents.iSelectCount;
	iRet = __xuiStatusBarDispatchMove(pContext,
		tWorld.fX + tItem0.fX + 2.0f, tWorld.fY + tItem0.fY + 2.0f);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarDispatchDown(pContext,
		tWorld.fX + tItem0.fX + 2.0f, tWorld.fY + tItem0.fY + 2.0f);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarDispatchMove(pContext,
		tWorld.fX + tCenter.fX + 2.0f, tWorld.fY + tCenter.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == 0 &&
		xuiStatusBarGetHoverIndex(pStatusBar) == -1 && tEvents.iSelectCount == iBefore,
		"dragging outside pressed item clears pressed hover");
	iRet = __xuiStatusBarDispatchMove(pContext,
		tWorld.fX + tItem0.fX + 2.0f, tWorld.fY + tItem0.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == 0 &&
		xuiStatusBarGetHoverIndex(pStatusBar) == 0,
		"dragging back into pressed item restores pressed hover");
	iRet = __xuiStatusBarDispatchUp(pContext,
		tWorld.fX + tItem0.fX + 2.0f, tWorld.fY + tItem0.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1,
		"release after re-entry activates pressed item once");
	iBefore = tEvents.iSelectCount;
	iRet = __xuiStatusBarDispatchDown(pContext,
		tWorld.fX + tItem0.fX + 2.0f, tWorld.fY + tItem0.fY + 2.0f);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarDispatchMove(pContext,
		tWorld.fX + tCenter.fX + 2.0f, tWorld.fY + tCenter.fY + 2.0f);
	if ( iRet == XUI_OK ) iRet = __xuiStatusBarDispatchUp(pContext,
		tWorld.fX + tCenter.fX + 2.0f, tWorld.fY + tCenter.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore &&
		xuiStatusBarGetActiveIndex(pStatusBar) == -1,
		"release over another item cancels original press");

	tWorld = xuiWidgetGetWorldRect(pStatusBar);
	tItem0 = xuiStatusBarGetItemRect(pStatusBar, 0);
	tCenter = xuiStatusBarGetItemRect(pStatusBar, 4);
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem0.fX + tItem0.fW * 0.5f),
		(int)(tWorld.fY + tItem0.fY + tItem0.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pStatusBar &&
		xuiStatusBarGetActiveIndex(pStatusBar) == 0,
		"first touch owns statusbar item press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tCenter.fX + tCenter.fW * 0.5f),
		(int)(tWorld.fY + tCenter.fY + tCenter.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tCenter.fX + tCenter.fW * 0.5f),
		(int)(tWorld.fY + tCenter.fY + tCenter.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tCenter.fX + tCenter.fW * 0.5f),
		(int)(tWorld.fY + tCenter.fY + tCenter.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pStatusBar &&
		xuiStatusBarGetActiveIndex(pStatusBar) == 0 &&
		xuiStatusBarGetHoverIndex(pStatusBar) == 0,
		"second touch cannot alter or commit statusbar item press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tItem0.fX + tItem0.fW * 0.5f),
		(int)(tWorld.fY + tItem0.fY + tItem0.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 &&
		tEvents.iLastValue == 10 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiStatusBarGetActiveIndex(pStatusBar) == -1,
		"owning touch commits statusbar item exactly once");

	iRet = xuiSetFocusWidget(pContext, pStatusBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus statusbar");
	iRet = xuiStatusBarSetHoverIndex(pStatusBar, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 4, "hover set");
	for ( i = 0; i < 7; i++ ) arrEquivalent[i] = *xuiStatusBarGetItem(pStatusBar, i);
	iBefore = xuiStatusBarGetChangeCount(pStatusBar);
	iRet = xuiStatusBarSetItems(pStatusBar, arrEquivalent, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 4 &&
		xuiStatusBarGetChangeCount(pStatusBar) == iBefore,
		"repeated items preserve hover state and skip mutation");
	iBefore = tEvents.iSelectCount;
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_SPACE);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 && tEvents.iLastValue == 20, "keyboard select");
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 5, "keyboard next item");
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 4, "keyboard previous item");
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_HOME);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 0, "keyboard home");
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_END);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 5, "keyboard end");
	iBefore = tEvents.iSelectCount;
	iRet = xuiWidgetPerformAccessibleAction(pStatusBar, 2u, XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 0, "accessibility focuses status item");
	iRet = xuiWidgetPerformAccessibleAction(pStatusBar, 2u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 && tEvents.iLastValue == 10,
		"accessibility activates status item");

	iRet = xuiStatusBarSetProgress(pStatusBar, 1, -20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetProgress(pStatusBar, 1) == 0.0f, "progress clamp low");
	iRet = xuiStatusBarSetItemEnabled(pStatusBar, 6, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarIsItemEnabled(pStatusBar, 6), "enable item");
	XUI_TEST_CHECK(xuiStatusBarSetItemEnabled(pStatusBar, 6, 0) == XUI_OK &&
		xuiStatusBarIsItemClickable(pStatusBar, 6),
		"clickable capability is independent from enabled state");
	XUI_TEST_CHECK(xuiStatusBarSetItemEnabled(pStatusBar, 6, 1) == XUI_OK,
		"restore enabled item");
	iRet = xuiStatusBarSetItemClickable(pStatusBar, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarIsItemClickable(pStatusBar, 1), "progress clickable");
	strcpy(sMutableText, "Busy");
	iRet = xuiStatusBarSetItemText(pStatusBar, 0, sMutableText);
	sMutableText[0] = 'X';
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiStatusBarGetItem(pStatusBar, 0)->sText, "Busy") == 0, "set text");
	iRet = xuiStatusBarSetItemValue(pStatusBar, 0, 99);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetItemValue(pStatusBar, 0) == 99, "set value");
	iRet = xuiStatusBarSetItemWidth(pStatusBar, 0, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiStatusBarNear(xuiStatusBarGetItemRect(pStatusBar, 0).fW, 80.0f), "set width");
	iRet = xuiStatusBarSetItemFlex(pStatusBar, 3, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set flex");
	XUI_TEST_CHECK(xuiStatusBarSetItemFlex(pStatusBar, 4, 1.0f) == XUI_OK &&
		xuiStatusBarSetItemFlex(pStatusBar, 5, 1.0f) == XUI_OK &&
		xuiStatusBarGetItemRect(pStatusBar, 3).fW > 0 &&
		xuiStatusBarGetItemRect(pStatusBar, 4).fW > 0 &&
		xuiStatusBarGetItemRect(pStatusBar, 5).fW > 0,
		"flex items distribute remaining width in every section");
	XUI_TEST_CHECK(xuiStatusBarSetItemWidth(pStatusBar, 4, 82.0f) == XUI_OK &&
		xuiStatusBarSetItemWidth(pStatusBar, 5, 58.0f) == XUI_OK,
		"restore fixed section widths");
	iRet = xuiStatusBarSetItemWidth(pStatusBar, 0, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiStatusBarGetItem(pStatusBar, 0)->fWidth == 80.0f, "nan width rejected");
	iRet = xuiStatusBarSetItemFlex(pStatusBar, 3, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiStatusBarGetItem(pStatusBar, 3)->fFlex == 2.0f, "infinite flex rejected");
	iRet = xuiStatusBarSetProgress(pStatusBar, 1, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiStatusBarGetProgress(pStatusBar, 1) == 0.0f, "nan progress rejected");
	iRet = xuiStatusBarGetMetrics(pStatusBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fHeight == 28.0f, "metrics get");
	tMetrics.fGap = NAN;
	iRet = xuiStatusBarSetMetrics(pStatusBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "nan metrics rejected");
	iRet = xuiStatusBarGetMetrics(pStatusBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK, "metrics restored for geometry validation");
	tMetrics.fGap = XUI_LAYOUT_UNBOUNDED;
	XUI_TEST_CHECK(xuiStatusBarSetMetrics(pStatusBar, &tMetrics) == XUI_ERROR_INVALID_ARGUMENT,
		"combined metric overflow rejected");
	XUI_TEST_CHECK(xuiStatusBarSetItemWidth(pStatusBar, 0,
		XUI_LAYOUT_UNBOUNDED * 2.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"out-of-range item width rejected");
	iRet = xuiStatusBarGetColors(pStatusBar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	XUI_TEST_CHECK((xuiStatusBarGetState(pStatusBar) & XUI_WIDGET_STATE_FOCUS) != 0u, "state focus");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetEnabled(pStatusBar) &&
		!xuiWidgetGetEffectiveEnabled(pStatusBar) &&
		(xuiStatusBarGetState(pStatusBar) & XUI_WIDGET_STATE_DISABLED) != 0u,
		"state reflects inherited disable");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pStatusBar, 0, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u && tNode.iActions == 0u,
		"inherited disable removes statusbar accessibility actions");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	iBefore = tEvents.iSelectCount;
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pStatusBar, 1, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u && tNode.iActions == 0u,
		"inherited disable removes status item accessibility actions");
	iRet = xuiWidgetPerformAccessibleAction(pStatusBar, 2u,
		XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && tEvents.iSelectCount == iBefore,
		"inherited disable rejects status item accessibility activation");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiStatusBarGetState(pStatusBar) & XUI_WIDGET_STATE_DISABLED) == 0u,
		"state clears inherited disable");
	XUI_TEST_CHECK(xuiStatusBarGetSelectCount(pStatusBar) == tEvents.iSelectCount, "select count");
	XUI_TEST_CHECK(xuiStatusBarGetChangeCount(pStatusBar) > 0, "change count");

	arrInvalid[0] = *xuiStatusBarGetItem(pStatusBar, 0);
	arrInvalid[1] = *xuiStatusBarGetItem(pStatusBar, 1);
	arrInvalid[1].iType = 99;
	iRet = xuiStatusBarSetItems(pStatusBar, arrInvalid, 2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiStatusBarGetItemCount(pStatusBar) == 7, "invalid setitems preserves prior items");
	arrInvalid[1] = *xuiStatusBarGetItem(pStatusBar, 1);
	arrInvalid[1].iState |= 0x80000000u;
	XUI_TEST_CHECK(xuiStatusBarSetItems(pStatusBar, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiStatusBarGetItemCount(pStatusBar) == 7, "unknown item state rejected");
	arrInvalid[1] = *xuiStatusBarGetItem(pStatusBar, 1);
	arrInvalid[1].iSection = 99;
	XUI_TEST_CHECK(xuiStatusBarSetItems(pStatusBar, arrInvalid, 2) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiStatusBarGetItemCount(pStatusBar) == 7, "invalid section rejected");
	iBefore = xuiStatusBarGetItemCount(pStatusBar);
	XUI_TEST_CHECK(xuiStatusBarAddProgress(pStatusBar, XUI_STATUSBAR_SECTION_LEFT, NAN, 1.0f, 0.0f, 20.0f) == -1 && xuiStatusBarGetItemCount(pStatusBar) == iBefore, "nan add progress rejected without mutation");
	XUI_TEST_CHECK(xuiStatusBarAddFlexibleSpacer(pStatusBar, XUI_STATUSBAR_SECTION_LEFT, INFINITY) == -1 && xuiStatusBarGetItemCount(pStatusBar) == iBefore, "infinite flex spacer rejected without mutation");
	XUI_TEST_CHECK(xuiStatusBarAddText(pStatusBar, 99, "Bad", 20.0f, 0, 0) == -1 &&
		xuiStatusBarGetItemCount(pStatusBar) == iBefore, "invalid add section rejected");

	(void)xuiWidgetSetRect(pStatusBar, (xui_rect_t){20.0f, 260.0f, 40.0f, 28.0f});
	(void)xuiStatusBarGetItemRect(pStatusBar, 4);
	iRet = xuiStatusBarSetHoverIndex(pStatusBar, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == -1,
		"programmatic hover ignores fully clipped item");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pStatusBar, 3, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u && tNode.iActions == 0u &&
		xuiWidgetPerformAccessibleAction(pStatusBar, 6u,
			XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_ERROR_INVALID_STATE,
		"clipped item exposes no accessibility actions");
	(void)xuiWidgetSetRect(pStatusBar, (xui_rect_t){20.0f, 260.0f, 460.0f, 28.0f});
	iRet = __xuiStatusBarLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore layout after clipping test");

	tItem0 = xuiStatusBarGetItemRect(pStatusBar, 0);
	iRet = __xuiStatusBarDispatchDown(pContext, 20.0f + tItem0.fX + 2.0f, 260.0f + tItem0.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == 0 && xuiGetPointerCapture(pContext) == pStatusBar, "pointer press captured");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable statusbar ancestor during press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == -1 &&
		xuiStatusBarGetHoverIndex(pStatusBar) == -1 &&
		xuiGetPointerCapture(pContext) != pStatusBar,
		"ancestor disable cancels statusbar press and hover");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable statusbar ancestor");
	iRet = __xuiStatusBarDispatchDown(pContext, 20.0f + tItem0.fX + 2.0f, 260.0f + tItem0.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == 0 &&
		xuiGetPointerCapture(pContext) == pStatusBar,
		"pointer press recaptured after ancestor reenable");
	iRet = xuiWidgetSetEnabled(pStatusBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable during press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetActiveIndex(pStatusBar) == -1 && xuiGetPointerCapture(pContext) != pStatusBar, "disable cancels press and capture");
	iRet = xuiWidgetSetEnabled(pStatusBar, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable statusbar");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch reenable");

	iRet = xuiStatusBarCreate(pContext, &pDestroyStatusBar, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyStatusBar != NULL, "destroy callback statusbar create");
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pDestroyStatusBar) == XUI_OK, "destroy callback statusbar add");
	(void)xuiWidgetSetRect(pDestroyStatusBar, (xui_rect_t){20.0f, 210.0f, 180.0f, 28.0f});
	XUI_TEST_CHECK(xuiStatusBarAddText(pDestroyStatusBar, XUI_STATUSBAR_SECTION_LEFT,
		"Destroy", 90.0f, 1, 77) == 0 &&
		xuiStatusBarSetSelect(pDestroyStatusBar, __xuiStatusBarDestroySelect, &tDestroy) == XUI_OK,
		"destroy callback statusbar setup");
	XUI_TEST_CHECK(__xuiStatusBarClickItem(pContext, pDestroyStatusBar, 0) == XUI_OK &&
		tDestroy.iCount == 1, "select callback may destroy statusbar");
	pDestroyStatusBar = NULL;

	pFirstItem = xuiStatusBarGetItem(pStatusBar, 0);
	iRet = xuiStatusBarSetItems(pStatusBar, pFirstItem, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetItemCount(pStatusBar) == 1 && strcmp(xuiStatusBarGetItem(pStatusBar, 0)->sText, "Busy") == 0, "setitems supports self alias");
	tEvents.bClearOnSelect = 1;
	tEvents.iClearResult = XUI_ERROR_INVALID_STATE;
	iRet = __xuiStatusBarClickItem(pContext, pStatusBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClearResult == XUI_OK, "select callback clear");
	XUI_TEST_CHECK(xuiStatusBarGetItemCount(pStatusBar) == 0 && xuiStatusBarGetHoverIndex(pStatusBar) == -1 && xuiStatusBarGetActiveIndex(pStatusBar) == -1, "select reentry preserves cleared state");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_statusbar_test passed\n");
	return 0;
}
