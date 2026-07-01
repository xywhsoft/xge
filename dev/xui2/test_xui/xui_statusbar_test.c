#include "xui.h"
#include "xui_test_proxy.h"

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
} xui_statusbar_test_events_t;

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
	}
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
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pStatusBar;
	xui_surface pTarget;
	xui_font pFont;
	xui_statusbar_desc_t tDesc;
	xui_statusbar_metrics_t tMetrics;
	xui_statusbar_colors_t tColors;
	xui_statusbar_item_t arrItems[7];
	xui_rect_t tItem0;
	xui_rect_t tItem1;
	xui_rect_t tFlex;
	xui_rect_t tCenter;
	xui_rect_t tRight;
	int iFailed;
	int iRet;
	int iBefore;

	pContext = NULL;
	pRoot = NULL;
	pStatusBar = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	tEvents.iLastIndex = -1;
	tEvents.iLastValue = -1;
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
	XUI_TEST_CHECK(tCenter.fX > 160.0f && tCenter.fX < 220.0f, "center layout");
	XUI_TEST_CHECK(tRight.fX > tCenter.fX, "right layout");
	XUI_TEST_CHECK(xuiStatusBarGetItemAt(pStatusBar, tCenter.fX + 2.0f, tCenter.fY + 2.0f) == 4, "item hit");
	XUI_TEST_CHECK(xuiStatusBarGetItemAt(pStatusBar, xuiStatusBarGetItemRect(pStatusBar, 6).fX + 2.0f, tCenter.fY + 2.0f) == -1, "disabled hit ignored");

	iRet = __xuiStatusBarClickItem(pContext, pStatusBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 1 && tEvents.iLastIndex == 0 && tEvents.iLastValue == 10, "click select");
	iBefore = tEvents.iSelectCount;
	iRet = __xuiStatusBarClickItem(pContext, pStatusBar, 6);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore, "disabled click ignored");

	iRet = xuiSetFocusWidget(pContext, pStatusBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus statusbar");
	iRet = xuiStatusBarSetHoverIndex(pStatusBar, 4);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetHoverIndex(pStatusBar) == 4, "hover set");
	iRet = __xuiStatusBarKey(pContext, XUI_KEY_SPACE);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 && tEvents.iLastValue == 20, "keyboard select");

	iRet = xuiStatusBarSetProgress(pStatusBar, 1, -20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetProgress(pStatusBar, 1) == 0.0f, "progress clamp low");
	iRet = xuiStatusBarSetItemEnabled(pStatusBar, 6, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarIsItemEnabled(pStatusBar, 6), "enable item");
	iRet = xuiStatusBarSetItemClickable(pStatusBar, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarIsItemClickable(pStatusBar, 1), "progress clickable");
	iRet = xuiStatusBarSetItemText(pStatusBar, 0, "Busy");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiStatusBarGetItem(pStatusBar, 0)->sText, "Busy") == 0, "set text");
	iRet = xuiStatusBarSetItemValue(pStatusBar, 0, 99);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiStatusBarGetItemValue(pStatusBar, 0) == 99, "set value");
	iRet = xuiStatusBarSetItemWidth(pStatusBar, 0, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiStatusBarNear(xuiStatusBarGetItemRect(pStatusBar, 0).fW, 80.0f), "set width");
	iRet = xuiStatusBarSetItemFlex(pStatusBar, 3, 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set flex");
	iRet = xuiStatusBarGetMetrics(pStatusBar, &tMetrics);
	XUI_TEST_CHECK(iRet == XUI_OK && tMetrics.fHeight == 28.0f, "metrics get");
	iRet = xuiStatusBarGetColors(pStatusBar, &tColors);
	XUI_TEST_CHECK(iRet == XUI_OK && tColors.iTextColor != 0u, "colors get");
	XUI_TEST_CHECK((xuiStatusBarGetState(pStatusBar) & XUI_WIDGET_STATE_FOCUS) != 0u, "state focus");
	XUI_TEST_CHECK(xuiStatusBarGetSelectCount(pStatusBar) == tEvents.iSelectCount, "select count");
	XUI_TEST_CHECK(xuiStatusBarGetChangeCount(pStatusBar) > 0, "change count");

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
