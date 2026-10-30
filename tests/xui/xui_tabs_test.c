#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_tabs_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_tabs_test_events_t {
	int iSelectCount;
	int iCloseCount;
	int iContextCount;
	int iLastSelect;
	int iLastClose;
	int iLastContext;
	float fContextX;
	float fContextY;
} xui_tabs_test_events_t;

typedef struct xui_tabs_destroy_events_t {
	xui_widget* ppWidget;
	int iSelectCount;
} xui_tabs_destroy_events_t;

typedef struct xui_tabs_remove_events_t {
	int iCount;
	int iLastIndex;
	int iResult;
} xui_tabs_remove_events_t;

typedef struct xui_tabs_blur_destroy_events_t {
	xui_widget* ppWidget;
	int iDestroyCount;
} xui_tabs_blur_destroy_events_t;

static xui_style_value_t __xuiTabsStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiTabsStyleProperty(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.tValue = tValue;
	return tProperty;
}

static int __xuiTabsNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static void __xuiTabsOnSelect(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_tabs_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iSelectCount++;
		pEvents->iLastSelect = iIndex;
	}
}

static void __xuiTabsDestroyOnSelect(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_destroy_events_t* pEvents = (xui_tabs_destroy_events_t*)pUser;

	(void)iIndex;
	if ( pEvents == NULL || pEvents->ppWidget == NULL || *pEvents->ppWidget != pWidget ) return;
	pEvents->iSelectCount++;
	xuiWidgetDestroy(pWidget);
	*pEvents->ppWidget = NULL;
}

static void __xuiTabsOnClose(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_tabs_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iCloseCount++;
		pEvents->iLastClose = iIndex;
	}
}

static void __xuiTabsRemoveOnClose(xui_widget pWidget, int iIndex, void* pUser)
{
	xui_tabs_remove_events_t* pEvents = (xui_tabs_remove_events_t*)pUser;

	if ( pEvents == NULL ) return;
	pEvents->iCount++;
	pEvents->iLastIndex = iIndex;
	pEvents->iResult = xuiTabsRemovePage(pWidget, iIndex);
}

static int __xuiTabsDestroyTargetOnBlur(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_tabs_blur_destroy_events_t* pEvents = (xui_tabs_blur_destroy_events_t*)pUser;
	xui_widget pVictim;

	(void)pWidget;
	if ( pEvents == NULL || pEvent == NULL || pEvent->iType != XUI_EVENT_BLUR ||
	     pEvents->ppWidget == NULL || *pEvents->ppWidget == NULL ) {
		return XUI_OK;
	}
	pVictim = *pEvents->ppWidget;
	*pEvents->ppWidget = NULL;
	pEvents->iDestroyCount++;
	xuiWidgetDestroy(pVictim);
	return XUI_OK;
}

static int __xuiTabsOnContext(xui_widget pWidget, int iIndex, float fX, float fY, void* pUser)
{
	xui_tabs_test_events_t* pEvents = (xui_tabs_test_events_t*)pUser;
	(void)pWidget;
	if ( pEvents != NULL ) {
		pEvents->iContextCount++;
		pEvents->iLastContext = iIndex;
		pEvents->fContextX = fX;
		pEvents->fContextY = fY;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiTabsRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiTabsLayoutRender(xui_context pContext, xui_surface pTarget)
{
	int iRet;

	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiTabsRender(pContext, pTarget);
}

static int __xuiTabsDispatchClick(xui_context pContext, float fX, float fY)
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

static int __xuiTabsDispatchContext(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	return iRet;
}

int main(void)
{
	static const char* arrItems[] = {
		"Start",
		"Scene",
		"Disabled",
		"Preview",
		"Export"
	};
	static const char* arrRetitledItems[] = {
		"Start",
		"Scene renamed",
		"Disabled",
		"Preview",
		"Export"
	};
	static const int arrEnabled[] = {1, 1, 0, 1, 1};
	static const int arrEnabledMove[] = {1, 0, 0, 1, 1};
	static const int arrDirty[] = {0, 1, 0, 0, 0};
	static const char* arrMutationItems[] = {"Alpha", "Beta", "Gamma"};
	xui_test_proxy_state_t tState;
	xui_tabs_test_events_t tEvents;
	xui_tabs_destroy_events_t tDestroyEvents;
	xui_tabs_remove_events_t tRemoveEvents;
	xui_tabs_blur_destroy_events_t tBlurDestroyEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTabs;
	xui_widget pDestroyTabs;
	xui_widget pMutationTabs;
	xui_widget pContextTabs;
	xui_widget pBlurProbe;
	xui_widget pProbe;
	xui_widget pUtf8Tabs;
	xui_widget arrMutationPages[3];
	xui_widget pMenu;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_surface pTabCache;
	xui_surface arrIcons[5];
	xui_rect_t arrIconSrc[5];
	xui_tabs_desc_t tDesc;
	xui_tabs_desc_t tBadDesc;
	xui_tabs_desc_t tUtf8Desc;
	xui_accessible_node_t tAccessible;
	xui_style_property_t tStyle;
	xui_event_t tContextEvent;
	xui_rect_t tWorld;
	xui_rect_t tProbeWorld;
	xui_rect_t tTab;
	xui_rect_t tClose;
	xui_rect_t tTabBar;
	xui_rect_t tClient;
	xui_rect_t tDirty;
	xui_rect_t tIcon;
	xui_rect_t tOverflow;
	xui_rect_t tMenuWorld;
	xui_rect_t tMenuItem;
	xui_rect_t tBadIconSrc[5];
	xui_vec2_t tMeasured;
	float fTabWidth;
	float fTabHeight;
	float fCheckWidth;
	float fCheckHeight;
	float fScroll;
	char sLongTitle[80];
	const char* arrUtf8Items[1];
	const char* arrBorrowedTitles[3];
	int iBefore;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTabs = NULL;
	pDestroyTabs = NULL;
	pMutationTabs = NULL;
	pContextTabs = NULL;
	pBlurProbe = NULL;
	pProbe = NULL;
	pUtf8Tabs = NULL;
	pTarget = NULL;
	pIcon = NULL;
	pTabCache = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tDestroyEvents, 0, sizeof(tDestroyEvents));
	memset(&tRemoveEvents, 0, sizeof(tRemoveEvents));
	memset(&tBlurDestroyEvents, 0, sizeof(tBlurDestroyEvents));
	memset(arrMutationPages, 0, sizeof(arrMutationPages));
	tRemoveEvents.iLastIndex = -1;
	tEvents.iLastSelect = -1;
	tEvents.iLastClose = -1;
	tEvents.iLastContext = -1;
	memset(arrIcons, 0, sizeof(arrIcons));
	memset(arrIconSrc, 0, sizeof(arrIconSrc));
	memset(tBadIconSrc, 0, sizeof(tBadIconSrc));
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
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tBadDesc, 0, sizeof(tBadDesc));
	tBadDesc.iSize = sizeof(tBadDesc);
	tBadDesc.fTabWidth = NAN;
	pDestroyTabs = pRoot;
	iRet = xuiTabsCreate(pContext, &pDestroyTabs, &tBadDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pDestroyTabs == NULL,
		"invalid descriptor clears output");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.arrEnabled = arrEnabled;
	tDesc.arrDirty = arrDirty;
	tDesc.iItemCount = 5;
	tDesc.iSelected = 1;
	tDesc.bScrollable = 1;
	tDesc.fTabWidth = 92.0f;
	tDesc.fTabHeight = 30.0f;
	iRet = xuiTabsCreate(pContext, &pTabs, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTabs != NULL, "tabs create");
	iRet = xuiWidgetMeasure(pTabs,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX > 120.0f && tMeasured.fY > 90.0f,
		"top tabs intrinsic measure exceeds finite constraint");
	iRet = xuiWidgetMeasure(pTabs, (xui_vec2_t){120.0f, 90.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTabsNear(tMeasured.fX, 120.0f) &&
		__xuiTabsNear(tMeasured.fY, 90.0f), "top tabs finite measure constraint");
	iRet = xuiWidgetAddChild(pRoot, pTabs);
	XUI_TEST_CHECK(iRet == XUI_OK, "tabs add");
	xuiWidgetSetRect(pTabs, (xui_rect_t){20.0f, 20.0f, 330.0f, 190.0f});
	iRet = xuiTabsSetSelect(pTabs, __xuiTabsOnSelect, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiTabsSetClose(pTabs, __xuiTabsOnClose, 1, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");
	iRet = xuiTabsSetContextMenu(pTabs, __xuiTabsOnContext, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "context callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	XUI_TEST_CHECK(xuiTabsGetItemCount(pTabs) == 5, "item count");
	XUI_TEST_CHECK(strcmp(xuiTabsGetItemText(pTabs, 1), "Scene") == 0, "item title");
	XUI_TEST_CHECK(xuiTabsGetSelected(pTabs) == 1, "selected index");
	XUI_TEST_CHECK(xuiTabsGetTabBarWidget(pTabs) != NULL, "tabbar widget");
	XUI_TEST_CHECK(xuiTabsGetClientWidget(pTabs) != NULL, "client widget");
	XUI_TEST_CHECK(xuiTabsGetOverflowMenu(pTabs) != NULL, "overflow menu widget");
	XUI_TEST_CHECK(xuiTabsGetPageWidget(pTabs, 1) != NULL, "page widget");
	XUI_TEST_CHECK(xuiTabsGetButtonWidget(pTabs, 1) != NULL, "button widget");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pTabs) == 6 &&
		xuiWidgetGetAccessibleNode(pTabs, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_TAB_LIST &&
		strcmp(tAccessible.sValue, "Scene") == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_VERTICAL) == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTION_REQUIRED) != 0,
		"tabs accessibility root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTabs, 2, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_TAB &&
		strcmp(tAccessible.sName, "Scene") == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTED) != 0,
		"tabs accessibility selected tab");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_DISABLED) != 0u,
		"parent disabled state reaches tabs");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTabs, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		tAccessible.iActions == 0u,
		"parent disabled state reaches accessibility");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore parent enabled state");
	iRet = xuiWidgetSetVisible(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide tabs through parent");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTabs, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u &&
		tAccessible.iActions == 0u &&
		xuiWidgetPerformAccessibleAction(pTabs, 1u,
			XUI_ACCESSIBLE_ACTION_FOCUS, NULL) != XUI_OK,
		"hidden tabs are offscreen and reject accessibility actions");
	iRet = xuiWidgetSetVisible(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore parent visibility");
	iRet = xuiTabsGetTabSize(pTabs, &fTabWidth, &fTabHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "get tab size baseline");
	iRet = xuiTabsSetTabSize(pTabs, NAN, fTabHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan tab width");
	iRet = xuiTabsSetTabSize(pTabs, fTabWidth, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite tab height");
	iRet = xuiTabsGetTabSize(pTabs, &fCheckWidth, &fCheckHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fCheckWidth == fTabWidth && fCheckHeight == fTabHeight,
		"invalid tab size preserves values");
	tStyle = __xuiTabsStyleProperty("tabs.tab.width", __xuiTabsStyleFloat(INFINITY));
	iRet = xuiWidgetSetInlineStyle(pTabs, &tStyle, 1);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTabsNear(xuiTabsGetTabRect(pTabs, 1).fW, fTabWidth),
		"ignore non-finite style metric");
	iRet = xuiWidgetSetInlineStyle(pTabs, NULL, 0);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid tabs style");
	tClient = xuiTabsGetClientRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 1);
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fH, 32.0f), "selected top tab fills strip height");
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fY + tTab.fH, tClient.fY), "selected top tab meets client border");

	iRet = xuiTabsSetSelected(pTabs, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1, "disabled set ignored");
	tWorld = xuiWidgetGetWorldRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 2);
	iRet = __xuiTabsDispatchClick(pContext, tWorld.fX + tTab.fX + tTab.fW * 0.5f, tWorld.fY + tTab.fY + tTab.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1 && tEvents.iSelectCount == 0, "disabled click ignored");
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + xuiTabsGetTabRect(pTabs, 1).fX + 4.0f,
		tWorld.fY + xuiTabsGetTabRect(pTabs, 1).fY + 4.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetHoverIndex(pTabs) == 1,
		"hover enabled tab before repeated model update");
	iBefore = xuiTabsGetChangeCount(pTabs);
	iRet = xuiTabsSetItems(pTabs, arrItems, 5);
	if ( iRet == XUI_OK ) iRet = xuiTabsSetSelected(pTabs, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1 &&
		xuiTabsGetHoverIndex(pTabs) == 1 && xuiTabsGetChangeCount(pTabs) == iBefore,
		"repeated items preserve disabled and hover state");
	iRet = xuiTabsSetItems(pTabs, arrRetitledItems, 5);
	if ( iRet == XUI_OK ) iRet = xuiTabsSetSelected(pTabs, 2);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strcmp(xuiTabsGetItemText(pTabs, 1), "Scene renamed") == 0 &&
		xuiTabsGetSelected(pTabs) == 1,
		"changed titles preserve disabled state");
	iRet = xuiTabsSetItems(pTabs, arrItems, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiTabsGetItemText(pTabs, 1), "Scene") == 0,
		"restore tab titles after state preservation test");
	iBefore = xuiTabsGetChangeCount(pTabs);
	iRet = xuiTabsSetEnabledItems(pTabs, arrEnabledMove, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 3 &&
		xuiTabsGetChangeCount(pTabs) == iBefore + 1 && tEvents.iSelectCount == 0,
		"disabling selection moves without user callback");
	iRet = xuiTabsSetEnabledItems(pTabs, arrEnabled, 5);
	if ( iRet == XUI_OK ) iRet = xuiTabsSetSelected(pTabs, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1 && tEvents.iSelectCount == 0,
		"restore selection after enabled fallback");

	iRet = xuiWidgetCreate(pContext, &pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK && pProbe != NULL, "page probe create");
	iRet = xuiWidgetSetSizeMode(pProbe, XUI_SIZE_FIXED, XUI_SIZE_FIXED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPreferredSize(pProbe, (xui_vec2_t){120.0f, 40.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pProbe, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pProbe, 1);
	if ( iRet == XUI_OK ) iRet = xuiTabsAddPageChild(pTabs, 1, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK, "page probe configure");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "page probe layout");
	tProbeWorld = xuiWidgetGetWorldRect(pProbe);
	XUI_TEST_CHECK(tProbeWorld.fW > 0.0f && tProbeWorld.fH > 0.0f, "page probe visible");
	iBefore = tEvents.iContextCount;
	iRet = __xuiTabsDispatchContext(pContext,
		tProbeWorld.fX + tProbeWorld.fW * 0.5f,
		tProbeWorld.fY + tProbeWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iContextCount == iBefore,
		"page content context does not invoke tabs callback");
	iRet = xuiSetFocusWidget(pContext, pProbe);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pProbe, "focus page probe");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1 && tEvents.iSelectCount == 0,
		"page child arrow does not change tab");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, XUI_MOD_CTRL);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 3 &&
		tEvents.iSelectCount == 1 && tEvents.iLastSelect == 3,
		"ctrl tab switches tabs from page child");
	iRet = xuiTabsSetSelected(pTabs, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore page after child routing");
	tEvents.iSelectCount = 0;
	tEvents.iLastSelect = -1;

	iRet = xuiSetFocusWidget(pContext, pTabs);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus tabs");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "right key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 3 && tEvents.iSelectCount == 1 && tEvents.iLastSelect == 3, "keyboard skips disabled");

	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render selected");
	tClose = xuiTabsGetCloseRect(pTabs, 3);
	iRet = __xuiTabsDispatchClick(pContext, tWorld.fX + tClose.fX + tClose.fW * 0.5f, tWorld.fY + tClose.fY + tClose.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iCloseCount == 1 && tEvents.iLastClose == 3, "close callback");
	XUI_TEST_CHECK(tEvents.iSelectCount == 1 && xuiTabsGetSelected(pTabs) == 3, "close does not select");

	tTabBar = xuiTabsGetTabBarRect(pTabs);
	tOverflow = xuiTabsGetOverflowRect(pTabs);
	XUI_TEST_CHECK(xuiTabsIsOverflow(pTabs) && tOverflow.fW > 0.0f && tOverflow.fH > 0.0f, "overflow button visible");
	fScroll = xuiTabsGetScroll(pTabs);
	iRet = xuiTabsSetScroll(pTabs, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTabsGetScroll(pTabs) == fScroll,
		"reject nan scroll without mutation");
	iRet = xuiTabsSetScroll(pTabs, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiTabsGetScroll(pTabs) == fScroll,
		"reject infinite scroll without mutation");
	XUI_TEST_CHECK(xuiTabsGetTabRect(pTabs, 4).fW <= 0.0f
		&& xuiWidgetGetRect(xuiTabsGetButtonWidget(pTabs, 4)).fW <= 0.0f,
		"overflow gives excess tab an empty layout box");
	iRet = __xuiTabsDispatchClick(pContext, tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f, tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f);
	pMenu = xuiTabsGetOverflowMenu(pTabs);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pMenu) && (xuiTabsGetState(pTabs) & XUI_TABS_STATE_OPEN) != 0u, "overflow menu opens");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout overflow menu");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pMenu) == 2, "overflow menu only lists hidden tabs");
	tMenuWorld = xuiWidgetGetWorldRect(pMenu);
	XUI_TEST_CHECK(xuiMenuGetItem(pMenu, 1)->iValue == 4, "overflow menu preserves hidden tab value");
	tMenuItem = xuiMenuGetItemRect(pMenu, 1);
	iRet = __xuiTabsDispatchClick(pContext, tMenuWorld.fX + tMenuItem.fX + tMenuItem.fW * 0.5f, tMenuWorld.fY + tMenuItem.fY + tMenuItem.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 4 && tEvents.iSelectCount == 2 && tEvents.iLastSelect == 4, "overflow menu selects hidden tab");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after overflow select");
	tTab = xuiTabsGetTabRect(pTabs, 4);
	tTabBar = xuiTabsGetTabBarRect(pTabs);
	XUI_TEST_CHECK(tTab.fX >= tTabBar.fX - 0.5f && (tTab.fX + tTab.fW) <= (xuiTabsGetOverflowRect(pTabs).fX + 0.5f), "selected overflow tab visible before menu button");

	iRet = xuiTestSurfaceCreate(&tState, &pIcon, 16, 16, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface");
	arrIcons[0] = pIcon;
	arrIconSrc[0] = (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f};
	iRet = xuiTabsSetIcons(pTabs, arrIcons, arrIconSrc, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "set icons");
	memcpy(tBadIconSrc, arrIconSrc, sizeof(tBadIconSrc));
	tBadIconSrc[0].fW = -1;
	iRet = xuiTabsSetIcons(pTabs, arrIcons, tBadIconSrc, 5);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject invalid icon source rect");
	iRet = xuiTabsSetSelected(pTabs, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 0, "select icon tab");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render icons");
	pTabCache = xuiWidgetGetCacheSurface(xuiTabsGetButtonWidget(pTabs, 0),
		xuiWidgetGetStateId(xuiTabsGetButtonWidget(pTabs, 0)));
	XUI_TEST_CHECK(pTabCache != NULL, "icon tab cache surface");
	xuiTestSurfaceReset(pTabCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTabCache) == 0,
		"unchanged borrowed tab icon reuses cache");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pIcon,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pTabCache) > 0,
		"borrowed tab icon generation rebuilds cache");
	tDirty = xuiTabsGetDirtyRect(pTabs, 1);
	tIcon = xuiTabsGetIconRect(pTabs, 0);
	XUI_TEST_CHECK(tDirty.fW > 0.0f && tDirty.fH > 0.0f, "dirty rect");
	XUI_TEST_CHECK(tIcon.fW > 0.0f && tIcon.fH > 0.0f, "icon rect");

	iRet = xuiTabsSetSelected(pTabs, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 0, "select first before left placement");
	iRet = xuiTabsSetPlacement(pTabs, XUI_TABS_PLACEMENT_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetPlacement(pTabs) == XUI_TABS_PLACEMENT_LEFT, "left placement");
	iRet = xuiWidgetMeasure(pTabs,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX > 100.0f && tMeasured.fY > 80.0f,
		"left tabs intrinsic measure exceeds finite constraint");
	iRet = xuiWidgetMeasure(pTabs, (xui_vec2_t){100.0f, 80.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiTabsNear(tMeasured.fX, 100.0f) &&
		__xuiTabsNear(tMeasured.fY, 80.0f), "left tabs finite measure constraint");
	iRet = xuiTabsSetScroll(pTabs, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset scroll");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render left");
	tTabBar = xuiTabsGetTabBarRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 0);
	XUI_TEST_CHECK(__xuiTabsNear(tTabBar.fW, 32.0f), "left tabbar width");
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fH, 92.0f) && __xuiTabsNear(tTab.fW, 32.0f), "left tab size");

	tWorld = xuiWidgetGetWorldRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 0);
	iRet = __xuiTabsDispatchContext(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iContextCount == 1 && tEvents.iLastContext == 0,
		"context identifies tab");
	XUI_TEST_CHECK(xuiTabsGetSelected(pTabs) == 0, "context selects tab");
	XUI_TEST_CHECK(tEvents.fContextX >= tWorld.fX && tEvents.fContextY >= tWorld.fY,
		"context uses world anchor");

	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTabs, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_VERTICAL) != 0 &&
		tAccessible.iRowCount == 5 && tAccessible.iColumnCount == 1,
		"vertical tabs accessibility orientation");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pTabs, 3, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0 &&
		xuiWidgetPerformAccessibleAction(pTabs, 4u,
			XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_ERROR_INVALID_STATE,
		"disabled tab rejects accessibility selection");
	iBefore = tEvents.iSelectCount;
	iRet = xuiWidgetPerformAccessibleAction(pTabs, 5u,
		XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 3 &&
		tEvents.iSelectCount == iBefore + 1 && tEvents.iLastSelect == 3,
		"accessibility selects tab");

	iRet = xuiTabsSetSelected(pTabs, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "select tab before capture cancellation");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before capture cancellation");
	tWorld = xuiWidgetGetWorldRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 0);
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCapture(pContext) == xuiTabsGetButtonWidget(pTabs, 0) &&
		xuiTabsGetActiveIndex(pTabs) == 0,
		"tab press captures button");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiTabsGetActiveIndex(pTabs) == -1 && xuiTabsGetCloseActiveIndex(pTabs) == -1 &&
		xuiTabsGetHoverIndex(pTabs) == 0 &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_ACTIVE) == 0,
		"escape releases child capture without clearing pointer hover");
	iRet = xuiInputPointerUp(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "release pointer after tab cancellation");
	tClose = xuiTabsGetCloseRect(pTabs, 0);
	iBefore = tEvents.iCloseCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tClose.fX + tClose.fW * 0.5f),
		(int)(tWorld.fY + tClose.fY + tClose.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) ==
			xuiTabsGetButtonWidget(pTabs, 0) &&
		xuiTabsGetCloseActiveIndex(pTabs) == 0,
		"first touch owns tab close press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tClose.fX + tClose.fW * 0.5f),
		(int)(tWorld.fY + tClose.fY + tClose.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tClose.fX + tClose.fW * 0.5f),
		(int)(tWorld.fY + tClose.fY + tClose.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iCloseCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) ==
			xuiTabsGetButtonWidget(pTabs, 0) &&
		xuiTabsGetCloseActiveIndex(pTabs) == 0,
		"unrelated touch cannot close or release tab press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tClose.fX + tClose.fW * 0.5f),
		(int)(tWorld.fY + tClose.fY + tClose.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iCloseCount == iBefore + 1 &&
		tEvents.iLastClose == 0 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiTabsGetCloseActiveIndex(pTabs) == -1,
		"owning touch closes tab once");

	tOverflow = xuiTabsGetOverflowRect(pTabs);
	XUI_TEST_CHECK(tOverflow.fW > 0.0f && tOverflow.fH > 0.0f,
		"vertical overflow button available");
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCapture(pContext) == xuiTabsGetTabBarWidget(pTabs) &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"overflow press captures tab bar");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_ACTIVE) == 0 &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_HOVER) != 0 &&
		!xuiMenuIsOpen(xuiTabsGetOverflowMenu(pTabs)),
		"escape releases overflow capture without clearing pointer hover");
	iRet = xuiInputPointerUp(pContext,
		tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f,
		tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(xuiTabsGetOverflowMenu(pTabs)),
		"cancelled overflow release stays closed");
	iRet = xuiInputPointerDownEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) ==
			xuiTabsGetTabBarWidget(pTabs),
		"first touch owns tabs overflow press");
	iRet = xuiInputPointerDownEx(pContext, 512u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 512u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(xuiTabsGetOverflowMenu(pTabs)) &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) ==
			xuiTabsGetTabBarWidget(pTabs) &&
		(xuiTabsGetState(pTabs) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot open or release tabs overflow");
	iRet = xuiInputPointerUpEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tOverflow.fX + tOverflow.fW * 0.5f),
		(int)(tWorld.fY + tOverflow.fY + tOverflow.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(xuiTabsGetOverflowMenu(pTabs)) &&
		xuiGetPointerCaptureEx(pContext, 511u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch opens tabs overflow once");
	iRet = xuiWidgetSetVisible(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(xuiTabsGetOverflowMenu(pTabs)) &&
		xuiTabsGetHoverIndex(pTabs) == -1 &&
		(xuiTabsGetState(pTabs) & (XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_ACTIVE)) == 0u,
		"parent hide closes overflow and clears transient state");
	iRet = xuiWidgetSetVisible(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore after hiding open overflow");

	iRet = xuiTabsSetSelected(pTabs, 4);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "select tab before model shrink");
	tWorld = xuiWidgetGetWorldRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 4);
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCapture(pContext) == xuiTabsGetButtonWidget(pTabs, 4),
		"last tab captures before model shrink");
	iBefore = xuiTabsGetChangeCount(pTabs);
	iRet = xuiTabsSetItems(pTabs, arrItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetItemCount(pTabs) == 1 &&
		xuiTabsGetSelected(pTabs) == 0 && xuiGetPointerCapture(pContext) == NULL &&
		xuiTabsGetActiveIndex(pTabs) == -1 &&
		xuiTabsGetChangeCount(pTabs) == iBefore + 1,
		"model shrink releases removed tab capture");
	iRet = xuiInputPointerUp(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "release pointer after model shrink");

	memset(sLongTitle, 'a', 61u);
	memcpy(sLongTitle + 61, "\xF0\x9F\x99\x82", 4u);
	sLongTitle[65] = 0;
	arrUtf8Items[0] = sLongTitle;
	memset(&tUtf8Desc, 0, sizeof(tUtf8Desc));
	tUtf8Desc.iSize = sizeof(tUtf8Desc);
	tUtf8Desc.arrItems = arrUtf8Items;
	tUtf8Desc.iItemCount = 1;
	tUtf8Desc.iSelected = 0;
	iRet = xuiTabsCreate(pContext, &pUtf8Tabs, &tUtf8Desc);
	XUI_TEST_CHECK(iRet == XUI_OK && pUtf8Tabs != NULL, "utf8 tabs create");
	XUI_TEST_CHECK(strlen(xuiTabsGetItemText(pUtf8Tabs, 0)) == 61u &&
		memcmp(xuiTabsGetItemText(pUtf8Tabs, 0), sLongTitle, 61u) == 0,
		"title truncation preserves utf8 grapheme boundary");
	xuiWidgetDestroy(pUtf8Tabs);
	pUtf8Tabs = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrMutationItems;
	tDesc.iItemCount = 3;
	tDesc.iSelected = 0;
	tDesc.fTabWidth = 88.0f;
	tDesc.fTabHeight = 30.0f;
	iRet = xuiTabsCreate(pContext, &pMutationTabs, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pMutationTabs != NULL, "mutation tabs create");
	iRet = xuiWidgetAddChild(pRoot, pMutationTabs);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetSetRect(pMutationTabs,
			(xui_rect_t){300.0f, 210.0f, 320.0f, 130.0f});
	}
	XUI_TEST_CHECK(iRet == XUI_OK, "mutation tabs add");
	arrMutationPages[0] = xuiTabsGetPageWidget(pMutationTabs, 0);
	arrMutationPages[1] = xuiTabsGetPageWidget(pMutationTabs, 1);
	arrMutationPages[2] = xuiTabsGetPageWidget(pMutationTabs, 2);
	iRet = xuiTabsSetItemText(pMutationTabs, 1, "Renamed");
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 1), "Renamed") == 0,
		"set item text");
	arrBorrowedTitles[0] = xuiTabsGetItemText(pMutationTabs, 2);
	arrBorrowedTitles[1] = xuiTabsGetItemText(pMutationTabs, 0);
	arrBorrowedTitles[2] = xuiTabsGetItemText(pMutationTabs, 1);
	iRet = xuiTabsSetItems(pMutationTabs, arrBorrowedTitles, 3);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 0), "Gamma") == 0 &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 1), "Alpha") == 0 &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 2), "Renamed") == 0,
		"set items accepts borrowed titles while reordering");
	iRet = xuiTabsSetSelected(pMutationTabs, 1);
	iBefore = xuiTabsGetChangeCount(pMutationTabs);
	if ( iRet == XUI_OK ) iRet = xuiTabsRemovePage(pMutationTabs, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetItemCount(pMutationTabs) == 2 &&
		xuiTabsGetSelected(pMutationTabs) == 0 &&
		xuiTabsGetChangeCount(pMutationTabs) == iBefore + 1 &&
		xuiTabsGetPageWidget(pMutationTabs, 0) == arrMutationPages[1] &&
		xuiTabsGetPageWidget(pMutationTabs, 1) == arrMutationPages[2] &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 0), "Alpha") == 0 &&
		strcmp(xuiTabsGetItemText(pMutationTabs, 1), "Renamed") == 0,
		"remove page preserves remaining page identity and selection");
	iRet = xuiTabsSetClose(pMutationTabs, __xuiTabsRemoveOnClose, 1, &tRemoveEvents);
	if ( iRet == XUI_OK ) iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "remove-on-close tabs layout");
	tWorld = xuiWidgetGetWorldRect(pMutationTabs);
	tClose = xuiTabsGetCloseRect(pMutationTabs, 0);
	iRet = __xuiTabsDispatchClick(pContext,
		tWorld.fX + tClose.fX + tClose.fW * 0.5f,
		tWorld.fY + tClose.fY + tClose.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tRemoveEvents.iCount == 1 &&
		tRemoveEvents.iLastIndex == 0 && tRemoveEvents.iResult == XUI_OK &&
		xuiTabsGetItemCount(pMutationTabs) == 1 &&
		xuiTabsGetPageWidget(pMutationTabs, 0) == arrMutationPages[2] &&
		xuiGetPointerCapture(pContext) == NULL,
		"close callback may remove its page safely");
	xuiWidgetDestroy(pMutationTabs);
	pMutationTabs = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.iItemCount = 2;
	tDesc.iSelected = 0;
	iRet = xuiTabsCreate(pContext, &pContextTabs, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pContextTabs != NULL, "context destroy tabs create");
	iRet = xuiWidgetSetRect(pContextTabs, (xui_rect_t){380.0f, 210.0f, 220.0f, 120.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pContextTabs);
	if ( iRet == XUI_OK ) iRet = xuiTabsSetContextMenu(pContextTabs, __xuiTabsOnContext, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "context destroy tabs add");
	iRet = xuiWidgetCreate(pContext, &pBlurProbe);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pBlurProbe, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pBlurProbe, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pBlurProbe,
		(xui_rect_t){380.0f, 170.0f, 80.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pBlurProbe);
	tBlurDestroyEvents.ppWidget = &pContextTabs;
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pBlurProbe, XUI_EVENT_BLUR,
		__xuiTabsDestroyTargetOnBlur, &tBlurDestroyEvents);
	if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pBlurProbe);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pBlurProbe,
		"context destroy focus probe");
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.iKey = XUI_KEY_CONTEXT_MENU;
	tContextEvent.pTarget = pContextTabs;
	iBefore = tEvents.iContextCount;
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && pContextTabs != NULL &&
		tBlurDestroyEvents.iDestroyCount == 0 && tEvents.iContextCount == iBefore + 1,
		"context callback completes before queued focus events");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContextTabs == NULL &&
		tBlurDestroyEvents.iDestroyCount == 1,
		"queued blur callback may destroy tabs safely");
	xuiWidgetDestroy(pBlurProbe);
	pBlurProbe = NULL;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItems = arrItems;
	tDesc.iItemCount = 2;
	tDesc.iSelected = 0;
	tDesc.fTabWidth = 92.0f;
	tDesc.fTabHeight = 30.0f;
	iRet = xuiTabsCreate(pContext, &pDestroyTabs, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyTabs != NULL, "destroy callback tabs create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyTabs);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback tabs add");
	xuiWidgetSetRect(pDestroyTabs, (xui_rect_t){380.0f, 20.0f, 230.0f, 150.0f});
	tDestroyEvents.ppWidget = &pDestroyTabs;
	iRet = xuiTabsSetSelect(pDestroyTabs, __xuiTabsDestroyOnSelect, &tDestroyEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback layout");
	tWorld = xuiWidgetGetWorldRect(pDestroyTabs);
	tTab = xuiTabsGetTabRect(pDestroyTabs, 1);
	iRet = __xuiTabsDispatchClick(pContext,
		tWorld.fX + tTab.fX + tTab.fW * 0.5f,
		tWorld.fY + tTab.fY + tTab.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyTabs == NULL && tDestroyEvents.iSelectCount == 1,
		"select callback may destroy tabs");

cleanup:
	if ( pUtf8Tabs != NULL ) {
		xuiWidgetDestroy(pUtf8Tabs);
	}
	if ( pIcon != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pIcon);
	}
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_tabs_test passed\n");
	return 0;
}
