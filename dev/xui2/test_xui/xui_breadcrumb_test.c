#include "xui.h"
#include "xui_test_proxy.h"

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
} xui_breadcrumb_test_events_t;

static void __xuiBreadcrumbTestClick(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_breadcrumb_test_events_t* pEvents = (xui_breadcrumb_test_events_t*)pUser;

	(void)pWidget;
	if ( pEvents == NULL ) return;
	pEvents->iClickCount++;
	pEvents->iLastIndex = iIndex;
	pEvents->iLastValue = iValue;
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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pBreadcrumb;
	xui_surface pTarget;
	xui_surface pIcon;
	xui_font pFont;
	xui_breadcrumb_desc_t tDesc;
	xui_breadcrumb_item_t arrItems[3];
	xui_breadcrumb_test_events_t tEvents;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pBreadcrumb = NULL;
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

	XUI_TEST_CHECK(xuiBreadcrumbGetItemCount(pBreadcrumb) == 3, "item count");
	XUI_TEST_CHECK(strcmp(xuiBreadcrumbGetItemText(pBreadcrumb, 1), "Console") == 0, "item text");
	XUI_TEST_CHECK(xuiBreadcrumbGetItemClickable(pBreadcrumb, 2) == 0, "disabled item");
	XUI_TEST_CHECK(xuiBreadcrumbGetItemValue(pBreadcrumb, 1) == 20, "item value");
	XUI_TEST_CHECK(strcmp(xuiBreadcrumbGetSeparator(pBreadcrumb), "/") == 0, "separator default");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 180, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 520, 180};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pBreadcrumb, 0) != NULL, "cache surface");
	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 1);
	XUI_TEST_CHECK(tRect.fW > 0.0f && tRect.fH > 0.0f, "item rect");
	XUI_TEST_CHECK(xuiBreadcrumbGetSeparatorRect(pBreadcrumb, 0).fW > 0.0f, "separator rect");

	tWorld = xuiWidgetGetWorldRect(pBreadcrumb);
	iRet = __xuiBreadcrumbTestClickAt(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click dispatch");
	XUI_TEST_CHECK(tEvents.iClickCount == 1 && tEvents.iLastIndex == 1 && tEvents.iLastValue == 20, "click callback");
	XUI_TEST_CHECK(xuiBreadcrumbGetClickCount(pBreadcrumb) == 1, "click count");

	tRect = xuiBreadcrumbGetItemRect(pBreadcrumb, 2);
	iRet = __xuiBreadcrumbTestClickAt(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled click dispatch");
	XUI_TEST_CHECK(tEvents.iClickCount == 1, "disabled item ignores click");

	iRet = xuiBreadcrumbSetSeparator(pBreadcrumb, ">");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiBreadcrumbGetSeparator(pBreadcrumb), ">") == 0, "set separator");
	iRet = xuiTestSurfaceCreate(&tState, &pIcon, 12, 12, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pIcon != NULL, "icon surface create");
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
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout updated");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render updated");
	XUI_TEST_CHECK(xuiBreadcrumbGetSeparatorRect(pBreadcrumb, 0).fW == 10.0f, "icon separator rect");

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
