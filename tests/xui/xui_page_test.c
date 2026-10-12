#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_page_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_page_change_state_t {
	int iCount;
	int iOldPage;
	int iNewPage;
} xui_page_change_state_t;

static xui_style_value_t __xuiPageStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_value_t __xuiPageStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_property_t __xuiPageStyleProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static void __xuiPageChanged(xui_widget pWidget, int iOldPage, int iNewPage, void* pUser)
{
	xui_page_change_state_t* pState;

	(void)pWidget;
	pState = (xui_page_change_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iCount++;
		pState->iOldPage = iOldPage;
		pState->iNewPage = iNewPage;
	}
}

static int __xuiPageRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 760, 160};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPage;
	xui_surface pTarget;
	xui_page_desc_t tDesc;
	xui_page_item_info_t tInfo;
	xui_page_change_state_t tChange;
	xui_style_property_t arrStyle[2];
	xui_vec2_t tMeasured;
	xui_rect_t tWorld;
	float fItemHeight;
	float fPageWidth;
	float fTextWidth;
	float fNavWidth;
	float fEllipsisWidth;
	int i;
	int iNextIndex;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pPage = NULL;
	pTarget = NULL;
	iFailed = 0;
	iNextIndex = -1;
	memset(&tChange, 0, sizeof(tChange));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 760.0f, 160.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 160.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPageCount = 12;
	tDesc.iCurrentPage = 6;
	tDesc.iWindowSize = 5;
	iRet = xuiPageCreate(pContext, &pPage, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPage != NULL, "page create");
	iRet = xuiWidgetAddChild(pRoot, pPage);
	XUI_TEST_CHECK(iRet == XUI_OK, "add page");
	xuiWidgetSetRect(pPage, (xui_rect_t){10.0f, 20.0f, 700.0f, 28.0f});
	iRet = xuiPageSetChange(pPage, __xuiPageChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change");
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pPage) == 1, "focusable");
	XUI_TEST_CHECK(xuiPageGetPageCount(pPage) == 12 && xuiPageGetCurrent(pPage) == 6, "initial page state");
	XUI_TEST_CHECK(xuiPageGetWindowSize(pPage) == 5, "window size");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 760, 160, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiPageRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render page");
	XUI_TEST_CHECK(xuiPageGetItemCount(pPage) == 11, "item count with ellipsis");

	iRet = xuiPageGetItemInfo(pPage, 0, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_PREV && tInfo.iPage == 5 && tInfo.bEnabled, "prev item");
	iRet = xuiPageGetItemInfo(pPage, 1, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_FIRST && tInfo.iPage == 1 && tInfo.bEnabled, "first item");
	iRet = xuiPageGetItemInfo(pPage, 2, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_ELLIPSIS && !tInfo.bEnabled, "leading ellipsis");
	iRet = xuiPageGetItemInfo(pPage, 5, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_PAGE && tInfo.iPage == 6 && !tInfo.bEnabled, "current item disabled");

	iRet = xuiPageSetMetrics(pPage, 30.0f, 40.0f, 58.0f, 48.0f, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiPageGetMetrics(pPage, &fItemHeight, &fPageWidth, &fTextWidth, &fNavWidth, &fEllipsisWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fItemHeight == 30.0f && fPageWidth == 40.0f && fTextWidth == 58.0f && fNavWidth == 48.0f && fEllipsisWidth == 30.0f, "get metrics");
	iRet = xuiWidgetMeasureContent(pPage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fY == 30.0f && tMeasured.fX > 300.0f, "measure");
	iRet = xuiPageSetText(pPage, "First", "Last", "Prev", "Next");
	XUI_TEST_CHECK(iRet == XUI_OK, "set text");
	iRet = xuiPageSetColors(pPage, XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(20, 40, 60, 255), XUI_COLOR_RGBA(30, 44, 64, 255), XUI_COLOR_RGBA(230, 240, 250, 255), XUI_COLOR_RGBA(210, 226, 244, 255), XUI_COLOR_RGBA(24, 150, 136, 255), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(150, 160, 170, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiPageSetFocusColor(pPage, XUI_COLOR_RGBA(24, 150, 136, 180));
	XUI_TEST_CHECK(iRet == XUI_OK, "set focus color");

	arrStyle[0] = __xuiPageStyleProp("page.item.height", __xuiPageStyleFloat(32.0f));
	arrStyle[1] = __xuiPageStyleProp("page.current.color", __xuiPageStyleColor(XUI_COLOR_RGBA(60, 120, 220, 255)));
	iRet = xuiWidgetSetInlineStyle(pPage, arrStyle, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	iRet = xuiWidgetMeasureContent(pPage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fY == 32.0f, "style item height");
	iRet = xuiWidgetSetInlineStyle(pPage, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear style");

	iRet = __xuiPageRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before pointer");
	for ( i = 0; i < xuiPageGetItemCount(pPage); i++ ) {
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		XUI_TEST_CHECK(iRet == XUI_OK, "item lookup");
		if ( tInfo.iType == XUI_PAGE_ITEM_NEXT ) {
			iNextIndex = i;
			break;
		}
	}
	XUI_TEST_CHECK(iNextIndex >= 0, "next index");
	tWorld = xuiWidgetGetWorldRect(pPage);
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetHoverItem(pPage) == iNextIndex, "hover next");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPage && xuiPageGetActiveItem(pPage) == iNextIndex, "active next");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiPageGetCurrent(pPage) == 7 && tChange.iCount == 1 && tChange.iOldPage == 6 && tChange.iNewPage == 7, "next activates");

	iRet = xuiSetFocusWidget(pContext, pPage);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus page");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "left key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 6, "left current");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "home key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 1, "home current");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "end key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 12, "end current");

	iRet = xuiPageSetTotal(pPage, 101, 10);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetPageCount(pPage) == 11 && xuiPageGetCurrent(pPage) == 11, "set total");
	iRet = xuiPageSetCurrent(pPage, 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 3, "program current");
	iRet = xuiWidgetSetEnabled(pPage, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiPageGetState(pPage) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_page_test passed\n");
	return 0;
}
