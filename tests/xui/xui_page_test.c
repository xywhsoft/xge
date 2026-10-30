#include "xui.h"
#include "xui_test_proxy.h"

#include <limits.h>
#include <math.h>
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
	int bDestroyOnChange;
	xui_widget* ppDestroyWidget;
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
		if ( pState->bDestroyOnChange ) {
			xuiWidgetDestroy(pWidget);
			if ( pState->ppDestroyWidget != NULL ) *pState->ppDestroyWidget = NULL;
		}
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
	xui_widget pInvalid;
	xui_widget pChild;
	xui_widget pDestroyPage;
	xui_surface pTarget;
	xui_font pFont;
	xui_page_desc_t tDesc;
	xui_page_desc_t tInvalidDesc;
	xui_page_item_info_t tInfo;
	xui_accessible_node_t tAccessible;
	xui_page_change_state_t tChange;
	xui_page_change_state_t tDestroyChange;
	xui_style_property_t arrStyle[2];
	xui_vec2_t tMeasured;
	xui_rect_t tWorld;
	float fItemHeight;
	float fPageWidth;
	float fTextWidth;
	float fNavWidth;
	float fEllipsisWidth;
	float fMeasuredWidth;
	const char* sFirstText;
	const char* sLastText;
	const char* sPrevText;
	const char* sNextText;
	const char* sUtf8Label;
	const char* sUtf8Expected;
	const char* sGraphemeLabel;
	const char* sGraphemeExpected;
	int i;
	int iNextIndex;
	int iAccessibleIndex;
	int iChangeBefore;
	int iCallbackBefore;
	uint64_t iAccessibleId;
	int iRet;
	int iFailed;

	pContext = NULL;
	pRoot = NULL;
	pPage = NULL;
	pInvalid = NULL;
	pChild = NULL;
	pDestroyPage = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	iNextIndex = -1;
	iAccessibleIndex = -1;
	iAccessibleId = 0;
	memset(&tChange, 0, sizeof(tChange));
	memset(&tDestroyChange, 0, sizeof(tDestroyChange));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont,
		"page", 4, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiInputViewport(pContext, 760.0f, 160.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 160.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");
	pInvalid = pRoot;
	iRet = xuiPageCreate(NULL, &pInvalid, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPageCount = 12;
	tDesc.iCurrentPage = 6;
	tDesc.iWindowSize = 5;
	tInvalidDesc = tDesc;
	tInvalidDesc.fItemHeight = NAN;
	pInvalid = pRoot;
	iRet = xuiPageCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nonfinite descriptor metric");
	tInvalidDesc = tDesc;
	tInvalidDesc.fPageWidth = XUI_LAYOUT_UNBOUNDED;
	pInvalid = pRoot;
	iRet = xuiPageCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject descriptor width that can overflow the item group");
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
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pPage) == 12 &&
		xuiWidgetGetAccessibleNode(pPage, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		strcmp(tAccessible.sValue, "Page 6 of 12") == 0 &&
		tAccessible.tRange.fValue == 6.0 && tAccessible.tRange.fMax == 12.0,
		"page accessibility root");

	iRet = xuiPageGetItemInfo(pPage, 0, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_PREV && tInfo.iPage == 5 && tInfo.bEnabled, "prev item");
	iRet = xuiPageGetItemInfo(pPage, 1, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_FIRST && tInfo.iPage == 1 && tInfo.bEnabled, "first item");
	iRet = xuiPageGetItemInfo(pPage, 2, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_ELLIPSIS && !tInfo.bEnabled, "leading ellipsis");
	iRet = xuiPageGetItemInfo(pPage, 5, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_PAGE && tInfo.iPage == 6 && !tInfo.bEnabled, "current item disabled");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPage, 6, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_BUTTON &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTED) != 0,
		"page accessibility current item");
	for ( i = 0; i < xuiPageGetItemCount(pPage); i++ ) {
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		XUI_TEST_CHECK(iRet == XUI_OK, "accessible item lookup");
		if ( tInfo.iType == XUI_PAGE_ITEM_PAGE && tInfo.iPage == 7 ) {
			iAccessibleIndex = i + 1;
			break;
		}
	}
	XUI_TEST_CHECK(iAccessibleIndex > 0, "accessible target page");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPage, iAccessibleIndex, &tAccessible) == XUI_OK,
		"accessible target node");
	iAccessibleId = tAccessible.iId;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPage, iAccessibleId,
		XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK &&
		xuiPageGetCurrent(pPage) == 7 && tChange.iCount == 1,
		"accessibility selects page");
	XUI_TEST_CHECK(xuiPageSetCurrent(pPage, 6, 0) == XUI_OK, "restore page after accessibility");
	memset(&tChange, 0, sizeof(tChange));

	iRet = xuiPageSetMetrics(pPage, 30.0f, 40.0f, 58.0f, 48.0f, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiPageGetMetrics(pPage, &fItemHeight, &fPageWidth, &fTextWidth, &fNavWidth, &fEllipsisWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fItemHeight == 30.0f && fPageWidth == 40.0f && fTextWidth == 58.0f && fNavWidth == 48.0f && fEllipsisWidth == 30.0f, "get metrics");
	iRet = xuiPageSetMetrics(pPage, NAN, 40.0f, 58.0f, 48.0f, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan metric");
	iRet = xuiPageSetMetrics(pPage, 30.0f, INFINITY, 58.0f, 48.0f, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metric");
	iRet = xuiPageSetMetrics(pPage, 30.0f, XUI_LAYOUT_UNBOUNDED,
		58.0f, 48.0f, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"reject width that can overflow the item group");
	iRet = xuiPageGetMetrics(pPage, &fItemHeight, &fPageWidth, &fTextWidth, &fNavWidth, &fEllipsisWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fItemHeight == 30.0f && fPageWidth == 40.0f && fTextWidth == 58.0f && fNavWidth == 48.0f && fEllipsisWidth == 30.0f, "invalid metrics preserve state");
	iRet = xuiWidgetMeasureContent(pPage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fY == 30.0f && tMeasured.fX > 300.0f, "measure");
	iRet = xuiWidgetMeasureContent(pPage, (xui_vec2_t){160.0f, 18.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 160.0f && tMeasured.fY == 18.0f,
		"content measure respects tight constraints");
	xuiWidgetClearDirty(pPage, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiPageSetText(pPage, "First", "Last", "Prev", "Next");
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pPage) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pPage) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"text update invalidates paint without relayout");
	sUtf8Label = "A" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD";
	sUtf8Expected = "A" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD" "\xE4\xB8\xAD";
	iRet = xuiPageSetText(pPage, sUtf8Label, "Last", "Prev", "Next");
	XUI_TEST_CHECK(iRet == XUI_OK, "set long utf8 text");
	iRet = xuiPageGetText(pPage, &sFirstText, &sLastText, &sPrevText, &sNextText);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(sFirstText, sUtf8Expected) == 0 && strlen(sFirstText) == 13u, "utf8 truncation keeps whole codepoints");
	XUI_TEST_CHECK(strcmp(sLastText, "Last") == 0 && strcmp(sPrevText, "Prev") == 0 && strcmp(sNextText, "Next") == 0, "other page labels preserved");
	sGraphemeLabel = "1234567890123e" "\xCC\x81";
	sGraphemeExpected = "1234567890123";
	iRet = xuiPageSetText(pPage, sGraphemeLabel, "Last", "Prev", "Next");
	XUI_TEST_CHECK(iRet == XUI_OK, "set combining label");
	iRet = xuiPageGetText(pPage, &sFirstText, NULL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(sFirstText, sGraphemeExpected) == 0,
		"text truncation preserves grapheme boundaries");
	iRet = xuiPageSetText(pPage, "First", "Last", "Prev", "Next");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore text");
	xuiWidgetClearDirty(pPage, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiPageSetFont(pPage, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetFont(pPage) == pFont &&
		(xuiWidgetGetDirtyFlags(pPage) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pPage) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"font update invalidates paint without relayout");
	iRet = xuiPageSetFont(pPage, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore font");
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
	arrStyle[0] = __xuiPageStyleProp("page.item.height", __xuiPageStyleFloat(NAN));
	iRet = xuiWidgetSetInlineStyle(pPage, arrStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid style metric");
	iRet = xuiWidgetMeasureContent(pPage, (xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fY == 30.0f, "ignore invalid style metric");
	fMeasuredWidth = tMeasured.fX;
	arrStyle[0] = __xuiPageStyleProp("page.width.page",
		__xuiPageStyleFloat(XUI_LAYOUT_UNBOUNDED));
	iRet = xuiWidgetSetInlineStyle(pPage, arrStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set oversized style metric");
	iRet = xuiWidgetMeasureContent(pPage,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == fMeasuredWidth,
		"ignore oversized style metric");
	iRet = xuiWidgetSetInlineStyle(pPage, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid style");

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
	iRet = xuiInputPointerMove(pContext, tWorld.fX + 1.0f,
		tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPage &&
		xuiPageGetActiveItem(pPage) == -1,
		"dragging outside the pressed item clears its active visual");
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f,
		tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetActiveItem(pPage) == iNextIndex,
		"returning to the pressed item restores its active visual");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiPageGetCurrent(pPage) == 7 && tChange.iCount == 1 && tChange.iOldPage == 6 && tChange.iNewPage == 7, "next activates");
	iRet = __xuiPageRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before multi-touch pointer");
	iNextIndex = -1;
	for ( i = 0; i < xuiPageGetItemCount(pPage); i++ ) {
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		XUI_TEST_CHECK(iRet == XUI_OK, "multi-touch item lookup");
		if ( tInfo.iType == XUI_PAGE_ITEM_NEXT ) {
			iNextIndex = i;
			break;
		}
	}
	XUI_TEST_CHECK(iNextIndex >= 0, "multi-touch next index");
	tWorld = xuiWidgetGetWorldRect(pPage);
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f),
		(int)(tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pPage &&
		xuiPageGetActiveItem(pPage) == iNextIndex,
		"first touch owns page item press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f),
		(int)(tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f),
		(int)(tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f), 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f),
		(int)(tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 7 &&
		tChange.iCount == 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pPage &&
		xuiPageGetActiveItem(pPage) == iNextIndex,
		"unrelated touch move and release preserve page press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f),
		(int)(tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 8 &&
		tChange.iCount == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiPageGetActiveItem(pPage) == -1,
		"owning touch activates page item once");
	iRet = xuiPageSetCurrent(pPage, 7, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore page after multi-touch pointer");

	iRet = __xuiPageRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before blur capture");
	iNextIndex = -1;
	for ( i = 0; i < xuiPageGetItemCount(pPage); i++ ) {
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		XUI_TEST_CHECK(iRet == XUI_OK, "blur item lookup");
		if ( tInfo.iType == XUI_PAGE_ITEM_NEXT ) {
			iNextIndex = i;
			break;
		}
	}
	XUI_TEST_CHECK(iNextIndex >= 0, "blur next index");
	tWorld = xuiWidgetGetWorldRect(pPage);
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "blur pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPage, "blur capture acquired");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiPageGetActiveItem(pPage) == -1, "blur cancels capture");

	iRet = xuiInputPointerDown(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "mutation pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPage, "mutation capture acquired");
	iRet = xuiPageSetWindowSize(pPage, 7);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetWindowSize(pPage) == 7 && xuiGetPointerCapture(pContext) == NULL && xuiPageGetActiveItem(pPage) == -1, "geometry mutation cancels capture");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "page child create");
	iRet = xuiWidgetAddChild(pPage, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "page child add");
	XUI_TEST_CHECK(xuiWidgetSetFocusable(pChild, 1) == XUI_OK &&
		xuiWidgetSetTabStop(pChild, 1) == XUI_OK, "page child focusable");
	iRet = xuiSetFocusWidget(pContext, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus page child");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "child right key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 7,
		"descendant key does not change page");
	xuiWidgetDestroy(pChild);
	pChild = NULL;

	iRet = xuiSetFocusWidget(pContext, pPage);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus page");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, XUI_MOD_CTRL);
	XUI_TEST_CHECK(iRet == XUI_OK, "modified right key");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 7,
		"modified arrow is not consumed");
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
	iChangeBefore = xuiPageGetChangeCount(pPage);
	iCallbackBefore = tChange.iCount;
	iRet = xuiPageSetPageCount(pPage, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetPageCount(pPage) == 3 &&
		xuiPageGetCurrent(pPage) == 3 && xuiPageGetChangeCount(pPage) == iChangeBefore + 1 &&
		tChange.iCount == iCallbackBefore,
		"page count shrink clamps selection without callback");
	iRet = xuiPageSetPageCount(pPage, 11);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageSetCurrent(pPage, 11, 0) == XUI_OK,
		"restore after page count shrink");
	iRet = xuiPageSetTotal(pPage, INT_MAX, 2);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetPageCount(pPage) == (INT_MAX / 2) + 1 && xuiPageGetCurrent(pPage) == 11, "large total avoids overflow");
	iRet = xuiPageSetTotal(pPage, INT_MAX, INT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetPageCount(pPage) == 1 && xuiPageGetCurrent(pPage) == 1, "large equal total avoids overflow");
	iRet = xuiPageSetPageCount(pPage, INT_MAX);
	if ( iRet == XUI_OK ) iRet = xuiPageSetCurrent(pPage, INT_MAX, 0);
	if ( iRet == XUI_OK ) iRet = xuiPageGetItemCount(pPage) > 0 ? XUI_OK : XUI_ERROR_INVALID_STATE;
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == INT_MAX,
		"maximum page builds a bounded item window without overflow");
	iNextIndex = xuiPageGetItemCount(pPage) - 1;
	iRet = xuiPageGetItemInfo(pPage, iNextIndex, &tInfo);
	XUI_TEST_CHECK(iRet == XUI_OK && tInfo.iType == XUI_PAGE_ITEM_NEXT &&
		tInfo.iPage == INT_MAX && !tInfo.bEnabled,
		"maximum page exposes a saturated disabled next item");
	iRet = xuiSetFocusWidget(pContext, pPage);
	if ( iRet == XUI_OK ) iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == INT_MAX,
		"right key saturates at the maximum page");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pPage, 0, &tAccessible);
	if ( iRet == XUI_OK ) {
		iRet = xuiWidgetPerformAccessibleAction(pPage, tAccessible.iId,
			XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	}
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == INT_MAX,
		"accessible increment saturates at the maximum page");
	iRet = xuiPageSetCurrent(pPage, 1, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore current after maximum page checks");
	iRet = xuiPageSetTotal(pPage, 101, 10);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetPageCount(pPage) == 11 && xuiPageGetCurrent(pPage) == 1, "restore total");
	iRet = xuiPageSetCurrent(pPage, 3, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiPageGetCurrent(pPage) == 3, "program current");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiPageGetState(pPage) & XUI_WIDGET_STATE_DISABLED) != 0,
		"parent disable propagates to page state");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiPageGetState(pPage) & XUI_WIDGET_STATE_DISABLED) == 0,
		"parent reenable restores page state");
	iRet = __xuiPageRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before disable capture");
	iNextIndex = -1;
	for ( i = 0; i < xuiPageGetItemCount(pPage); i++ ) {
		iRet = xuiPageGetItemInfo(pPage, i, &tInfo);
		XUI_TEST_CHECK(iRet == XUI_OK, "disable item lookup");
		if ( tInfo.iType == XUI_PAGE_ITEM_NEXT ) {
			iNextIndex = i;
			break;
		}
	}
	XUI_TEST_CHECK(iNextIndex >= 0, "disable next index");
	tWorld = xuiWidgetGetWorldRect(pPage);
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tInfo.tRect.fX + tInfo.tRect.fW * 0.5f, tWorld.fY + tInfo.tRect.fY + tInfo.tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPage, "disable capture acquired");
	iRet = xuiWidgetSetEnabled(pPage, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable page");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiPageGetState(pPage) & XUI_WIDGET_STATE_DISABLED) != 0 && xuiGetPointerCapture(pContext) == NULL && xuiPageGetActiveItem(pPage) == -1, "disable cancels capture");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPageCount = 3;
	tDesc.iCurrentPage = 1;
	iRet = xuiPageCreate(pContext, &pDestroyPage, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyPage != NULL, "destroy callback page create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyPage);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback page add");
	tDestroyChange.bDestroyOnChange = 1;
	tDestroyChange.ppDestroyWidget = &pDestroyPage;
	iRet = xuiPageSetChange(pDestroyPage, __xuiPageChanged, &tDestroyChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	iRet = xuiPageSetCurrent(pDestroyPage, 2, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyPage == NULL && tDestroyChange.iCount == 1,
		"change callback may destroy page");

cleanup:
	if ( pDestroyPage != NULL ) xuiWidgetDestroy(pDestroyPage);
	if ( pChild != NULL ) xuiWidgetDestroy(pChild);
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_page_test passed\n");
	return 0;
}
