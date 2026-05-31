#include "xui.h"
#include "xui_test_proxy.h"

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
	int iLastSelect;
	int iLastClose;
} xui_tabs_test_events_t;

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

int main(void)
{
	static const char* arrItems[] = {
		"Start",
		"Scene",
		"Disabled",
		"Preview",
		"Export"
	};
	static const int arrEnabled[] = {1, 1, 0, 1, 1};
	static const int arrDirty[] = {0, 1, 0, 0, 0};
	xui_test_proxy_state_t tState;
	xui_tabs_test_events_t tEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pTabs;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_surface arrIcons[5];
	xui_rect_t arrIconSrc[5];
	xui_tabs_desc_t tDesc;
	xui_rect_t tWorld;
	xui_rect_t tTab;
	xui_rect_t tClose;
	xui_rect_t tTabBar;
	xui_rect_t tClient;
	xui_rect_t tDirty;
	xui_rect_t tIcon;
	float fScroll;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pTabs = NULL;
	pTarget = NULL;
	pIcon = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	tEvents.iLastSelect = -1;
	tEvents.iLastClose = -1;
	memset(arrIcons, 0, sizeof(arrIcons));
	memset(arrIconSrc, 0, sizeof(arrIconSrc));
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
	iRet = xuiWidgetAddChild(pRoot, pTabs);
	XUI_TEST_CHECK(iRet == XUI_OK, "tabs add");
	xuiWidgetSetRect(pTabs, (xui_rect_t){20.0f, 20.0f, 330.0f, 190.0f});
	iRet = xuiTabsSetSelect(pTabs, __xuiTabsOnSelect, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	iRet = xuiTabsSetClose(pTabs, __xuiTabsOnClose, 1, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "close callback");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	XUI_TEST_CHECK(xuiTabsGetItemCount(pTabs) == 5, "item count");
	XUI_TEST_CHECK(strcmp(xuiTabsGetItemText(pTabs, 1), "Scene") == 0, "item title");
	XUI_TEST_CHECK(xuiTabsGetSelected(pTabs) == 1, "selected index");
	XUI_TEST_CHECK(xuiTabsGetTabBarWidget(pTabs) != NULL, "tabbar widget");
	XUI_TEST_CHECK(xuiTabsGetClientWidget(pTabs) != NULL, "client widget");
	XUI_TEST_CHECK(xuiTabsGetPageWidget(pTabs, 1) != NULL, "page widget");
	XUI_TEST_CHECK(xuiTabsGetButtonWidget(pTabs, 1) != NULL, "button widget");
	tClient = xuiTabsGetClientRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 1);
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fH, 31.0f), "selected top tab overlaps height");
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fY + tTab.fH, tClient.fY + 1.0f), "selected top tab overlaps client border");

	iRet = xuiTabsSetSelected(pTabs, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1, "disabled set ignored");
	tWorld = xuiWidgetGetWorldRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 2);
	iRet = __xuiTabsDispatchClick(pContext, tWorld.fX + tTab.fX + tTab.fW * 0.5f, tWorld.fY + tTab.fY + tTab.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 1 && tEvents.iSelectCount == 0, "disabled click ignored");

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
	iRet = xuiInputPointerWheel(pContext, tWorld.fX + tTabBar.fX + 12.0f, tWorld.fY + tTabBar.fY + 12.0f, 0.0f, -1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetScroll(pTabs) > 0.0f, "wheel scrolls tabs");

	fScroll = xuiTabsGetScroll(pTabs);
	iRet = xuiTabsSetSelected(pTabs, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetSelected(pTabs) == 4, "select hidden tab");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after ensure");
	tTab = xuiTabsGetTabRect(pTabs, 4);
	tTabBar = xuiTabsGetTabBarRect(pTabs);
	XUI_TEST_CHECK(xuiTabsGetScroll(pTabs) >= fScroll, "ensure scroll advances");
	XUI_TEST_CHECK(tTab.fX >= tTabBar.fX - 0.5f && (tTab.fX + tTab.fW) <= (tTabBar.fX + tTabBar.fW + 0.5f), "selected tab visible");

	iRet = xuiTestSurfaceCreate(&tState, &pIcon, 16, 16, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface");
	arrIcons[0] = pIcon;
	arrIconSrc[0] = (xui_rect_t){0.0f, 0.0f, 16.0f, 16.0f};
	iRet = xuiTabsSetIcons(pTabs, arrIcons, arrIconSrc, 5);
	XUI_TEST_CHECK(iRet == XUI_OK, "set icons");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render icons");
	tDirty = xuiTabsGetDirtyRect(pTabs, 1);
	tIcon = xuiTabsGetIconRect(pTabs, 0);
	XUI_TEST_CHECK(tDirty.fW > 0.0f && tDirty.fH > 0.0f, "dirty rect");
	XUI_TEST_CHECK(tIcon.fW > 0.0f && tIcon.fH > 0.0f, "icon rect");

	iRet = xuiTabsSetPlacement(pTabs, XUI_TABS_PLACEMENT_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTabsGetPlacement(pTabs) == XUI_TABS_PLACEMENT_LEFT, "left placement");
	iRet = xuiTabsSetScroll(pTabs, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset scroll");
	iRet = __xuiTabsLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render left");
	tTabBar = xuiTabsGetTabBarRect(pTabs);
	tTab = xuiTabsGetTabRect(pTabs, 0);
	XUI_TEST_CHECK(__xuiTabsNear(tTabBar.fW, 33.0f), "left tabbar width");
	XUI_TEST_CHECK(__xuiTabsNear(tTab.fH, 92.0f) && __xuiTabsNear(tTab.fW, 30.0f), "left tab size");

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
	if ( iFailed ) {
		return 1;
	}
	printf("xui_tabs_test passed\n");
	return 0;
}
