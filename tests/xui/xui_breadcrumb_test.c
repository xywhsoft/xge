#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_breadcrumb_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_breadcrumb_test_events_t {
	int iClickCount;
	int iLastIndex;
	int iLastValue;
	int iContextCount;
} xui_breadcrumb_test_events_t;

typedef struct xui_breadcrumb_destroy_test_t {
	xui_widget pVictim;
	int iDestroyCount;
} xui_breadcrumb_destroy_test_t;

static xui_style_value_t __xuiBreadcrumbTestStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiBreadcrumbTestStyleProperty(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.tValue = tValue;
	return tProperty;
}

static void __xuiBreadcrumbTestClick(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_breadcrumb_test_events_t* pEvents = (xui_breadcrumb_test_events_t*)pUser;

	(void)pWidget;
	if ( pEvents == NULL ) return;
	pEvents->iClickCount++;
	pEvents->iLastIndex = iIndex;
	pEvents->iLastValue = iValue;
}

static int __xuiBreadcrumbTestContext(xui_widget pWidget, int iIndex, int iValue,
	float fX, float fY, void* pUser)
{
	xui_breadcrumb_test_events_t* pEvents = (xui_breadcrumb_test_events_t*)pUser;

	(void)pWidget;
	(void)iIndex;
	(void)iValue;
	(void)fX;
	(void)fY;
	if ( pEvents != NULL ) pEvents->iContextCount++;
	return XUI_OK;
}

static void __xuiBreadcrumbTestDestroyClick(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_breadcrumb_destroy_test_t* pState = (xui_breadcrumb_destroy_test_t*)pUser;

	(void)iIndex;
	(void)iValue;
	if ( pState == NULL ) return;
	pState->iDestroyCount++;
	pState->pVictim = NULL;
	xuiWidgetDestroy(pWidget);
}

static int __xuiBreadcrumbTestDestroyOnBlur(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_breadcrumb_destroy_test_t* pState = (xui_breadcrumb_destroy_test_t*)pUser;
	xui_widget pVictim;

	(void)pWidget;
	if ( pState == NULL || pEvent == NULL || pEvent->iType != XUI_EVENT_BLUR || pState->pVictim == NULL ) {
		return XUI_OK;
	}
	pVictim = pState->pVictim;
	pState->pVictim = NULL;
	pState->iDestroyCount++;
	xuiWidgetDestroy(pVictim);
	return XUI_OK;
}

static int __xuiBreadcrumbTestClickAt(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiBreadcrumbTestPressAt(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiBreadcrumbTestReleaseAt(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiBreadcrumbTestSetItemsOomPreserves(xui_widget pBreadcrumb)
{
	xallocationbudget* pBudget = NULL;
	xallocationscope tScope;
	xui_breadcrumb_item_t arrReplacement[2] = {
		{"Replacement", 1, 1}, {"Other", 1, 2}
	};
	int iRet;

	if ( pBreadcrumb == NULL ) return 0;
	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) ) return 0;
	if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	iRet = xuiBreadcrumbSetItems(pBreadcrumb, arrReplacement, 2);
	(void)xrtAllocationBudgetLeave(&tScope);
	xrtAllocationBudgetFree(pBudget);
	return iRet == XUI_ERROR_OUT_OF_MEMORY && xuiBreadcrumbGetItemCount(pBreadcrumb) == 3 &&
		strcmp(xuiBreadcrumbGetItemText(pBreadcrumb, 0), "Workspace") == 0;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pBreadcrumb;
	xui_widget pInvalid;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_font pFont;
	xui_breadcrumb_desc_t tDesc;
	xui_breadcrumb_item_t arrItems[3];
	xui_breadcrumb_test_events_t tEvents;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tOldRect;
	xui_vec2_t tIntrinsic;
	xui_vec2_t tMeasured;
	xui_event_t tEvent;
	xui_style_property_t tStyle;
	xui_accessible_node_t tAccessible;
	float fGap;
	float fPaddingX;
	float fPaddingY;
	float fPressX;
	float fPressY;
	int iFailed;
	int iMeasureCount;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pBreadcrumb = NULL;
	pInvalid = NULL;
	pTarget = NULL;
	pIcon = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	tEvents.iLastIndex = -1;
	tEvents.iLastValue = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "breadcrumb", 10, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 180.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 180.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iItemCount = 1;
	pInvalid = pRoot;
	iRet = xuiBreadcrumbCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject missing descriptor items");
	tDesc.iItemCount = 0;
	tDesc.fGap = NAN;
	pInvalid = pRoot;
	iRet = xuiBreadcrumbCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject non-finite descriptor metric");
	tDesc.fGap = 0.0f;
	tDesc.tSeparatorIconSrc.fW = -1;
	pInvalid = pRoot;
	iRet = xuiBreadcrumbCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject negative descriptor source size");

	arrItems[0].sText = "Workspace";
	arrItems[0].bClickable = 1;
	arrItems[0].iValue = 10;
	arrItems[1].sText = "Console";
	arrItems[1].bClickable = 1;
	arrItems[1].iValue = 20;
	arrItems[2].sText = "Visits";
	arrItems[2].bClickable = 0;
	arrItems[2].iValue = 30;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pItems = arrItems;
	tDesc.iItemCount = 3;
	tDesc.sSeparator = "/";
	tDesc.pFont = pFont;
	iRet = xuiBreadcrumbCreate(pContext, &pBreadcrumb, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pBreadcrumb != NULL, "breadcrumb create");
	xuiWidgetSetRect(pBreadcrumb, (xui_rect_t){20.0f, 20.0f, 420.0f, 36.0f});
	iRet = xuiWidgetAddChild(pRoot, pBreadcrumb);
	XUI_TEST_CHECK(iRet == XUI_OK, "breadcrumb add");
	iRet = xuiBreadcrumbSetClick(pBreadcrumb, __xuiBreadcrumbTestClick, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "set click");
	iRet = xuiBreadcrumbSetContextMenu(pBreadcrumb, __xuiBreadcrumbTestContext, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "set context menu");

	XUI_TEST_CHECK(xuiBreadcrumbGetItemCount(pBreadcrumb) == 3, "item count");
	XUI_TEST_CHECK(strcmp(xuiBreadcrumbGetItemText(pBreadcrumb, 1), "Console") == 0, "item text");
	XUI_TEST_CHECK(xuiBreadcrumbGetItemClickable(pBreadcrumb, 2) == 0, "disabled item");
	XUI_TEST_CHECK(xuiBreadcrumbGetItemValue(pBreadcrumb, 1) == 20, "item value");
	XUI_TEST_CHECK(strcmp(xuiBreadcrumbGetSeparator(pBreadcrumb), "/") == 0, "separator default");
	XUI_TEST_CHECK(__xuiBreadcrumbTestSetItemsOomPreserves(pBreadcrumb),
		"failed item replacement preserves breadcrumb state");
	iRet = xuiWidgetMeasureContent(pBreadcrumb,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tIntrinsic);
	XUI_TEST_CHECK(iRet == XUI_OK && tIntrinsic.fX > 0.0f && tIntrinsic.fY >= 32.0f,
		"breadcrumb intrinsic content measure");
	iRet = xuiWidgetMeasureContent(pBreadcrumb,
		(xui_vec2_t){tIntrinsic.fX + 100.0f, tIntrinsic.fY + 100.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == tIntrinsic.fX &&
		tMeasured.fY == tIntrinsic.fY,
		"roomy constraints do not stretch content measure");
	iRet = xuiWidgetMeasureContent(pBreadcrumb,
		(xui_vec2_t){tIntrinsic.fX * 0.5f, 16.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX <= tIntrinsic.fX * 0.5f &&
		tMeasured.fY <= 16.0f,
		"tight constraints clamp content measure");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 180, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 520, 180};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pBreadcrumb, 0) != NULL, "cache surface");
	iMeasureCount = xuiTestProxyGetTextMeasureCount(&tState);
	XUI_TEST_CHECK(iMeasureCount == 4, "breadcrumb measures each item and separator once");
	iRet = xuiWidgetInvalidate(pBreadcrumb, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	XUI_TEST_CHECK(iRet == XUI_OK, "invalidate breadcrumb cache");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestProxyGetTextMeasureCount(&tState) == iMeasureCount,
		"cache rebuild reuses breadcrumb text measurements");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 1);
	XUI_TEST_CHECK(tRect.fW > 0.0f && tRect.fH > 0.0f, "item rect");
	XUI_TEST_CHECK(xuiBreadcrumbGetSeparatorRect(pBreadcrumb, 0).fW > 0.0f, "separator rect");

	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f)) == XUI_CURSOR_HAND,
		"clickable breadcrumb item uses hand cursor");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 2);
	XUI_TEST_CHECK(xuiQueryCursor(pContext,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f)) == XUI_CURSOR_ARROW,
		"non-clickable breadcrumb item uses arrow cursor");
	iRet = xuiBreadcrumbSetItem(pBreadcrumb, 2, "Visits", 1, 30);
	XUI_TEST_CHECK(iRet == XUI_OK, "temporarily enable last item");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestSurfaceGetLastTextColor(xuiWidgetGetCacheSurface(pBreadcrumb, 0)) ==
			XUI_COLOR_RGBA(135, 148, 164, 255),
		"enabled breadcrumb uses normal item color");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable breadcrumb parent");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch inherited disable");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestSurfaceGetLastTextColor(xuiWidgetGetCacheSurface(pBreadcrumb, 0)) ==
			XUI_COLOR_RGBA(88, 102, 121, 255),
		"inherited disable uses disabled item color");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore breadcrumb parent");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch inherited enable");
	iRet = xuiBreadcrumbSetItem(pBreadcrumb, 2, "Visits", 0, 30);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore non-clickable last item");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 1);
	iRet = __xuiBreadcrumbTestClickAt(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click dispatch");
	XUI_TEST_CHECK(tEvents.iClickCount == 1 && tEvents.iLastIndex == 1 && tEvents.iLastValue == 20, "click callback");
	XUI_TEST_CHECK(xuiBreadcrumbGetClickCount(pBreadcrumb) == 1, "click count");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pBreadcrumb, "pointer click focuses breadcrumb");

	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 2);
	iRet = __xuiBreadcrumbTestClickAt(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled click dispatch");
	XUI_TEST_CHECK(tEvents.iClickCount == 1, "disabled item ignores click");

	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 0);
	fPressX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fPressY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = __xuiBreadcrumbTestPressAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBreadcrumb &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == 0, "press captures breadcrumb");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear breadcrumb focus");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == -1, "blur cancels pointer press");
	iRet = __xuiBreadcrumbTestReleaseAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 1, "release after blur stays cancelled");

	iRet = __xuiBreadcrumbTestPressAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBreadcrumb,
		"disable press captures breadcrumb");
	iRet = xuiWidgetSetEnabled(pBreadcrumb, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable breadcrumb");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == -1, "disable cancels pointer press");
	iRet = xuiWidgetSetEnabled(pBreadcrumb, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore breadcrumb enabled");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch breadcrumb enabled");
	iRet = __xuiBreadcrumbTestReleaseAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 1, "release after disable stays cancelled");

	iRet = __xuiBreadcrumbTestPressAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBreadcrumb,
		"hide press captures breadcrumb");
	iRet = xuiWidgetSetVisible(pBreadcrumb, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide breadcrumb");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == -1, "hide cancels pointer press");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_MOVE;
	tEvent.pTarget = pBreadcrumb;
	tEvent.fX = (int)fPressX;
	tEvent.fY = (int)fPressY;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetHoverIndex(pBreadcrumb) == -1,
		"hidden breadcrumb ignores direct pointer event");
	tEvent.iType = XUI_EVENT_CONTEXT_MENU;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iContextCount == 0,
		"hidden breadcrumb ignores direct context event");
	iRet = xuiWidgetSetVisible(pBreadcrumb, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore breadcrumb visible");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch breadcrumb visible");
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iContextCount == 1,
		"restored breadcrumb accepts context event");
	iRet = __xuiBreadcrumbTestReleaseAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 1, "release after hide stays cancelled");

	iRet = xuiBreadcrumbSetSeparator(pBreadcrumb, ">");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiBreadcrumbGetSeparator(pBreadcrumb), ">") == 0, "set separator");
	iRet = xuiTestSurfaceCreate(&tState, &pIcon, 12, 12, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface create");
	iRet = xuiBreadcrumbSetSeparatorIcon(pBreadcrumb, pIcon,
		(xui_rect_t){0.0f, 0.0f, -1.0f, 12.0f}, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiBreadcrumbGetSeparatorIcon(pBreadcrumb) == NULL,
		"reject negative icon source size");
	iRet = xuiBreadcrumbSetSeparatorIcon(pBreadcrumb, pIcon,
		(xui_rect_t){0.0f, 0.0f, 12.0f, 12.0f}, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiBreadcrumbGetSeparatorIcon(pBreadcrumb) == NULL,
		"reject non-finite icon size");
	iRet = xuiBreadcrumbSetSeparatorIcon(pBreadcrumb, pIcon, (xui_rect_t){0.0f, 0.0f, 12.0f, 12.0f}, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetSeparatorIcon(pBreadcrumb) == pIcon, "set separator icon");
	XUI_TEST_CHECK(xuiBreadcrumbGetSeparatorIconSize(pBreadcrumb) == 10.0f, "icon size");
	iRet = xuiBreadcrumbAddItem(pBreadcrumb, "Realtime", 1, 40);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetItemCount(pBreadcrumb) == 4, "add item");
	iRet = xuiBreadcrumbSetItem(pBreadcrumb, 0, "Home", 1, 100);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiBreadcrumbGetItemText(pBreadcrumb, 0), "Home") == 0 && xuiBreadcrumbGetItemValue(pBreadcrumb, 0) == 100, "set item");
	iRet = xuiBreadcrumbSetTextColors(pBreadcrumb, XUI_COLOR_RGBA(80, 90, 100, 255), XUI_COLOR_RGBA(40, 120, 210, 255), XUI_COLOR_RGBA(20, 90, 170, 255), XUI_COLOR_RGBA(160, 170, 180, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiBreadcrumbSetMetrics(pBreadcrumb, 10.0f, 2.0f, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiBreadcrumbGetMetrics(pBreadcrumb, &fGap, &fPaddingX, &fPaddingY);
	XUI_TEST_CHECK(iRet == XUI_OK && fGap == 10.0f && fPaddingX == 2.0f && fPaddingY == 5.0f,
		"get metrics");
	iRet = xuiBreadcrumbSetMetrics(pBreadcrumb, NAN, 2.0f, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan metric");
	iRet = xuiBreadcrumbSetMetrics(pBreadcrumb, 10.0f, INFINITY, 5.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metric");
	iRet = xuiBreadcrumbGetMetrics(pBreadcrumb, &fGap, &fPaddingX, &fPaddingY);
	XUI_TEST_CHECK(iRet == XUI_OK && fGap == 10.0f && fPaddingX == 2.0f && fPaddingY == 5.0f,
		"invalid metrics preserve state");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout updated");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render updated");
	XUI_TEST_CHECK(xuiBreadcrumbGetSeparatorRect(pBreadcrumb, 0).fW == 10.0f, "icon separator rect");

	{
		xui_surface pCache = xuiWidgetGetCacheSurface(pBreadcrumb, xuiWidgetGetStateId(pBreadcrumb));
		XUI_TEST_CHECK(pCache != NULL, "breadcrumb cache before icon update");
		xuiTestSurfaceReset(pCache);
		iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pIcon,
			(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "separator icon update");
		iRet = xuiUpdate(pContext, 0.016f);
		XUI_TEST_CHECK(iRet == XUI_OK, "update after separator icon update");
		iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
		XUI_TEST_CHECK(iRet == XUI_OK, "render after separator icon update");
		pCache = xuiWidgetGetCacheSurface(pBreadcrumb, xuiWidgetGetStateId(pBreadcrumb));
		XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetDrawCount(pCache) > 0,
			"separator icon generation invalidates cache");
	}
	iRet = xuiBreadcrumbSetSeparatorIcon(pBreadcrumb, pIcon,
		(xui_rect_t){0, 0, 0, 0}, 6.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetSeparatorIconSize(pBreadcrumb) == 6.0f,
		"set separator icon with implicit source");
	iRet = xuiLayout(pContext);
	if ( iRet == XUI_OK ) iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiTestSurfaceGetLastSrc(xuiWidgetGetCacheSurface(pBreadcrumb, 0)).fW == 12 &&
		xuiTestSurfaceGetLastSrc(xuiWidgetGetCacheSurface(pBreadcrumb, 0)).fH == 12,
		"implicit separator source uses full surface");
	iRet = xuiBreadcrumbSetSeparatorIcon(pBreadcrumb, pIcon,
		(xui_rect_t){0, 0, 0, 0}, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetSeparatorIconSize(pBreadcrumb) == 0.0f,
		"zero separator icon size restores default");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiBreadcrumbGetSeparatorRect(pBreadcrumb, 0).fW == 12,
		"default separator icon size restored");

	tOldRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 1);
	tStyle = __xuiBreadcrumbTestStyleProperty("breadcrumb.gap",
		__xuiBreadcrumbTestStyleFloat(NAN));
	iRet = xuiWidgetSetInlineStyle(pBreadcrumb, &tStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid style metric");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout invalid style metric");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 1);
	XUI_TEST_CHECK(tRect.fX == tOldRect.fX,
		"ignore invalid style metric");
	iRet = xuiWidgetSetInlineStyle(pBreadcrumb, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid style metric");

	xuiWidgetSetRect(pBreadcrumb, (xui_rect_t){20.0f, 20.0f, 420.0f, 4.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout tiny breadcrumb");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 0);
	XUI_TEST_CHECK(tRect.fH >= 0.0f, "tiny breadcrumb has non-negative geometry");
	xuiWidgetSetRect(pBreadcrumb, (xui_rect_t){20.0f, 20.0f, 420.0f, 36.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore breadcrumb geometry");
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 0);
	fPressX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fPressY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = __xuiBreadcrumbTestPressAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBreadcrumb,
		"metric press captures breadcrumb");
	iRet = xuiBreadcrumbSetMetrics(pBreadcrumb, 8.0f, 3.0f, 4.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == -1,
		"geometry mutation cancels pointer press");
	iRet = __xuiBreadcrumbTestReleaseAt(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 1,
		"release after geometry mutation stays cancelled");

	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pBreadcrumb) == 5,
		"breadcrumb accessibility exposes root and items");
	iRet = xuiWidgetGetAccessibleNode(pBreadcrumb, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		tAccessible.iColumnCount == 4, "breadcrumb accessibility root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pBreadcrumb, 2, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LINK &&
		strcmp(tAccessible.sName, "Console") == 0 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)) != 0,
		"clickable breadcrumb accessibility item");
	iRet = xuiWidgetPerformAccessibleAction(pBreadcrumb, 3u, XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pBreadcrumb,
		"accessibility focuses breadcrumb item");
	iRet = xuiWidgetPerformAccessibleAction(pBreadcrumb, 3u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 2 && tEvents.iLastIndex == 1,
		"accessibility activates breadcrumb item");
	iRet = xuiWidgetPerformAccessibleAction(pBreadcrumb, 4u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE,
		"non-clickable breadcrumb item rejects accessibility activation");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 0);
	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	fPressX = tWorld.fX + tRect.fX + tRect.fW * 0.5f;
	fPressY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)fPressX, (int)fPressY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pBreadcrumb &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == 0,
		"first touch owns breadcrumb press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)fPressX, (int)fPressY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, (int)fPressX, (int)fPressY,
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pBreadcrumb &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == 0,
		"unrelated touch cannot click or release breadcrumb press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)fPressX, (int)fPressY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iClickCount == 3 &&
		tEvents.iLastIndex == 0 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiBreadcrumbGetActiveIndex(pBreadcrumb) == -1,
		"owning touch commits breadcrumb click once");

	{
		xui_widget pContextBreadcrumb = NULL;
		xui_breadcrumb_item_t arrContextItems[2] = {
			{"Parent", 0, 1}, {"Current", 0, 2}
		};
		xui_breadcrumb_desc_t tContextDesc;

		memset(&tContextDesc, 0, sizeof(tContextDesc));
		tContextDesc.iSize = sizeof(tContextDesc);
		tContextDesc.pItems = arrContextItems;
		tContextDesc.iItemCount = 2;
		iRet = xuiBreadcrumbCreate(pContext, &pContextBreadcrumb, &tContextDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pContextBreadcrumb != NULL,
			"context-only breadcrumb create");
		(void)xuiWidgetSetRect(pContextBreadcrumb, (xui_rect_t){20, 125, 240, 36});
		iRet = xuiWidgetAddChild(pRoot, pContextBreadcrumb);
		XUI_TEST_CHECK(iRet == XUI_OK, "context-only breadcrumb add");
		iRet = xuiBreadcrumbSetContextMenu(pContextBreadcrumb,
			__xuiBreadcrumbTestContext, NULL);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetTabStop(pContextBreadcrumb),
			"context-only breadcrumb remains keyboard focusable");
		iRet = xuiWidgetPerformAccessibleAction(pContextBreadcrumb, 2u,
			XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pContextBreadcrumb,
			"context-only breadcrumb item supports accessibility focus");
		iRet = xuiWidgetPerformAccessibleAction(pContextBreadcrumb, 2u,
			XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
		XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_STATE,
			"context-only breadcrumb item does not expose activation");
		xuiWidgetDestroy(pContextBreadcrumb);
	}

	{
		xui_breadcrumb_destroy_test_t tDestroy;
		xui_widget pDestroyBreadcrumb = NULL;
		xui_breadcrumb_item_t tDestroyItem = {"Destroy", 1, 77};
		xui_breadcrumb_desc_t tDestroyDesc;
		xui_rect_t tDestroyRect;
		xui_rect_t tDestroyWorld;

		memset(&tDestroy, 0, sizeof(tDestroy));
		memset(&tDestroyDesc, 0, sizeof(tDestroyDesc));
		tDestroyDesc.iSize = sizeof(tDestroyDesc);
		tDestroyDesc.pItems = &tDestroyItem;
		tDestroyDesc.iItemCount = 1;
		iRet = xuiBreadcrumbCreate(pContext, &pDestroyBreadcrumb, &tDestroyDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pDestroyBreadcrumb != NULL, "destroy callback breadcrumb create");
		tDestroy.pVictim = pDestroyBreadcrumb;
		(void)xuiWidgetSetRect(pDestroyBreadcrumb, (xui_rect_t){20, 80, 180, 36});
		iRet = xuiWidgetAddChild(pRoot, pDestroyBreadcrumb);
		XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback breadcrumb add");
		iRet = xuiBreadcrumbSetClick(pDestroyBreadcrumb, __xuiBreadcrumbTestDestroyClick, &tDestroy);
		XUI_TEST_CHECK(iRet == XUI_OK, "set destroying breadcrumb callback");
		iRet = xuiLayout(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK, "layout destroy callback breadcrumb");
		tDestroyRect = xuiBreadcrumbGetItemRect(pDestroyBreadcrumb, 0);
		tDestroyWorld = xuiWidgetGetWorldRect(pDestroyBreadcrumb);
		iRet = __xuiBreadcrumbTestClickAt(pContext,
			tDestroyWorld.fX + tDestroyRect.fX + tDestroyRect.fW / 2,
			tDestroyWorld.fY + tDestroyRect.fY + tDestroyRect.fH / 2);
		XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iDestroyCount == 1 && tDestroy.pVictim == NULL,
			"click callback may destroy breadcrumb");
	}

	{
		xui_breadcrumb_destroy_test_t tDestroy;
		xui_widget pFocusTrap = NULL;
		xui_widget pDestroyBreadcrumb = NULL;
		xui_breadcrumb_item_t tDestroyItem = {"Focus destroy", 1, 88};
		xui_breadcrumb_desc_t tDestroyDesc;
		xui_rect_t tDestroyRect;
		xui_rect_t tDestroyWorld;

		memset(&tDestroy, 0, sizeof(tDestroy));
		memset(&tDestroyDesc, 0, sizeof(tDestroyDesc));
		tDestroyDesc.iSize = sizeof(tDestroyDesc);
		tDestroyDesc.pItems = &tDestroyItem;
		tDestroyDesc.iItemCount = 1;
		iRet = xuiWidgetCreate(pContext, &pFocusTrap);
		XUI_TEST_CHECK(iRet == XUI_OK && pFocusTrap != NULL, "focus trap create");
		(void)xuiWidgetSetFocusable(pFocusTrap, 1);
		(void)xuiWidgetSetTabStop(pFocusTrap, 1);
		(void)xuiWidgetSetRect(pFocusTrap, (xui_rect_t){220, 80, 80, 36});
		iRet = xuiWidgetAddChild(pRoot, pFocusTrap);
		XUI_TEST_CHECK(iRet == XUI_OK, "focus trap add");
		iRet = xuiBreadcrumbCreate(pContext, &pDestroyBreadcrumb, &tDestroyDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pDestroyBreadcrumb != NULL, "focus destroy breadcrumb create");
		tDestroy.pVictim = pDestroyBreadcrumb;
		(void)xuiWidgetSetRect(pDestroyBreadcrumb, (xui_rect_t){320, 80, 180, 36});
		iRet = xuiWidgetAddChild(pRoot, pDestroyBreadcrumb);
		XUI_TEST_CHECK(iRet == XUI_OK, "focus destroy breadcrumb add");
		iRet = xuiWidgetSetEventHandler(pFocusTrap, XUI_EVENT_BLUR,
			__xuiBreadcrumbTestDestroyOnBlur, &tDestroy);
		XUI_TEST_CHECK(iRet == XUI_OK, "focus trap blur handler");
		iRet = xuiSetFocusWidget(pContext, pFocusTrap);
		XUI_TEST_CHECK(iRet == XUI_OK, "focus trap focus");
		iRet = xuiDispatchPendingEvents(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK, "focus trap dispatch");
		iRet = xuiLayout(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK, "layout focus destroy breadcrumb");
		tDestroyRect = xuiBreadcrumbGetItemRect(pDestroyBreadcrumb, 0);
		tDestroyWorld = xuiWidgetGetWorldRect(pDestroyBreadcrumb);
		iRet = __xuiBreadcrumbTestPressAt(pContext,
			tDestroyWorld.fX + tDestroyRect.fX + tDestroyRect.fW / 2,
			tDestroyWorld.fY + tDestroyRect.fY + tDestroyRect.fH / 2);
		XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iDestroyCount == 1 && tDestroy.pVictim == NULL,
			"focus callback may destroy pressed breadcrumb");
	}

cleanup:
	if ( pIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pIcon);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) return 1;
	printf("xui_breadcrumb_test passed\n");
	return 0;
}
