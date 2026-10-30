#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_carousel_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_carousel_test_state_t {
	int iChanges;
	int iOld;
	int iNew;
	int iEscapes;
	int bDestroyOnChange;
} xui_carousel_test_state_t;

static xui_style_value_t __xuiCarouselStyleBool(int bValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_BOOL;
	tValue.iInt = bValue ? 1 : 0;
	return tValue;
}

static xui_style_property_t __xuiCarouselStyleProp(const char* sName,
	xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static int __xuiCarouselRootEvent(xui_widget pWidget, const xui_event_t* pEvent,
	void* pUser)
{
	xui_carousel_test_state_t* pState = (xui_carousel_test_state_t*)pUser;

	(void)pWidget;
	if ( pState != NULL && pEvent != NULL &&
	     pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iKey == XUI_KEY_ESCAPE ) {
		pState->iEscapes++;
	}
	return XUI_OK;
}

static void __xuiCarouselTestChanged(xui_widget pWidget, int iOldIndex, int iNewIndex, void* pUser)
{
	xui_carousel_test_state_t* pState;

	(void)pWidget;
	pState = (xui_carousel_test_state_t*)pUser;
	if ( pState != NULL ) {
		pState->iChanges++;
		pState->iOld = iOldIndex;
		pState->iNew = iNewIndex;
		if ( pState->bDestroyOnChange ) xuiWidgetDestroy(pWidget);
	}
}

int main(void)
{
	xui_test_proxy_state_t tProxyState;
	xui_carousel_test_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCarousel;
	xui_widget pInvalid;
	xui_widget pChild;
	xui_surface pTarget;
	xui_font pFont;
	xui_font pFontAlt;
	xui_carousel_desc_t tDesc;
	xui_carousel_desc_t tInvalidDesc;
	xui_accessible_node_t tAccessible;
	xui_style_property_t tStyle;
	xui_rect_t tWorld;
	xui_rect_i_t tFullRect;
	xui_vec2_t tMeasured;
	int bAuto;
	float fInterval;
	int iChangeBefore;
	int iCallbackBefore;
	int iEscapeBefore;
	int iCurrentBefore;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCarousel = NULL;
	pInvalid = NULL;
	pChild = NULL;
	pTarget = NULL;
	pFont = NULL;
	pFontAlt = NULL;
	iFailed = 0;
	memset(&tState, 0, sizeof(tState));
	xuiTestProxyInit(&tProxyState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tProxyState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFont, "carousel", 8, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = tProxyState.tProxy.fontLoadMemory(&tProxyState.tProxy, &pFontAlt,
		"carousel-alt", 12, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFontAlt != NULL, "alternate font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");
	iRet = xuiWidgetSetEventHandler(pRoot, XUI_EVENT_KEY_DOWN,
		__xuiCarouselRootEvent, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "root key handler");
	pInvalid = pRoot;
	iRet = xuiCarouselCreate(NULL, &pInvalid, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.iPageCount = 1;
	tInvalidDesc.fAutoInterval = NAN;
	pCarousel = pRoot;
	iRet = xuiCarouselCreate(pContext, &pCarousel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCarousel == NULL, "reject NaN descriptor interval");
	tInvalidDesc.fAutoInterval = 0.0f;
	tInvalidDesc.fIndicatorGap = INFINITY;
	iRet = xuiCarouselCreate(pContext, &pCarousel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCarousel == NULL, "reject infinite descriptor metric");
	tInvalidDesc.fIndicatorGap = 0.0f;
	tInvalidDesc.fIndicatorSize = XUI_LAYOUT_UNBOUNDED;
	iRet = xuiCarouselCreate(pContext, &pCarousel, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pCarousel == NULL,
		"reject descriptor metric that can overflow indicator geometry");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPageCount = 4;
	tDesc.iCurrent = 2;
	tDesc.pFont = pFont;
	tDesc.fAutoInterval = 0.1f;
	tDesc.bShowIndicators = 1;
	iRet = xuiCarouselCreate(pContext, &pCarousel, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCarousel != NULL, "carousel create");
	iRet = xuiWidgetMeasureContent(pCarousel,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 460.0f && tMeasured.fY == 290.0f,
		"carousel intrinsic content measure");
	iRet = xuiWidgetMeasureContent(pCarousel, (xui_vec2_t){180.0f, 120.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 180.0f && tMeasured.fY == 120.0f,
		"carousel content measure respects tight constraints");
	(void)xuiWidgetSetRect(pCarousel, (xui_rect_t){20.0f, 20.0f, 460.0f, 260.0f});
	iRet = xuiCarouselSetChange(pCarousel, __xuiCarouselTestChanged, &tState);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	iRet = xuiWidgetAddChild(pRoot, pCarousel);
	XUI_TEST_CHECK(iRet == XUI_OK, "add carousel");

	XUI_TEST_CHECK(xuiCarouselGetPageCount(pCarousel) == 4, "page count");
	XUI_TEST_CHECK(xuiCarouselGetCurrent(pCarousel) == 2, "initial current");
	XUI_TEST_CHECK(xuiCarouselGetLoop(pCarousel) == 0, "descriptor loop false honored");
	XUI_TEST_CHECK(xuiCarouselGetPageWidget(pCarousel, 0) != NULL, "page widget 0");
	XUI_TEST_CHECK(xuiCarouselGetPageWidget(pCarousel, 3) != NULL, "page widget 3");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiCarouselGetPageWidget(pCarousel, 2)) != 0, "current page visible");
	XUI_TEST_CHECK(xuiWidgetGetVisible(xuiCarouselGetPageWidget(pCarousel, 0)) == 0, "inactive page hidden");
	{
		const char* arrPaintOnlyProperties[] = {
			"carousel.arrow.size",
			"carousel.indicator.size",
			"carousel.indicator.gap",
			"carousel.indicator.bottom",
			"carousel.indicator.visible",
			"carousel.arrow.hover_only"
		};
		xui_style_property_info_t tPropertyInfo;
		uint32_t iPropertyId;
		int i;

		for ( i = 0; i < (int)(sizeof(arrPaintOnlyProperties) /
			sizeof(arrPaintOnlyProperties[0])); i++ ) {
			iPropertyId = xuiStyleFindProperty(pContext, arrPaintOnlyProperties[i]);
			memset(&tPropertyInfo, 0, sizeof(tPropertyInfo));
			tPropertyInfo.iSize = sizeof(tPropertyInfo);
			iRet = xuiStyleGetPropertyInfo(pContext, iPropertyId, &tPropertyInfo);
			XUI_TEST_CHECK(iPropertyId != 0u && iRet == XUI_OK &&
				(tPropertyInfo.iDirtyFlags & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
				(tPropertyInfo.iDirtyFlags & XUI_WIDGET_DIRTY_CACHE) != 0u,
				"carousel chrome style invalidates paint without relayout");
		}
	}
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pCarousel) == 5 &&
		xuiWidgetGetAccessibleNode(pCarousel, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		strcmp(tAccessible.sValue, "Page 3 of 4") == 0,
		"carousel accessibility root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pCarousel, 3, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_BUTTON &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_SELECTED) != 0,
		"carousel accessibility current page");

	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	XUI_TEST_CHECK(xuiWidgetSetFocusable(pChild, 1) == XUI_OK &&
		xuiWidgetSetTabStop(pChild, 1) == XUI_OK, "child focusable");
	iRet = xuiCarouselAddPageChild(pCarousel, 1, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetParent(pChild) == xuiCarouselGetPageWidget(pCarousel, 1), "add page child");
	XUI_TEST_CHECK(xuiCarouselSetCurrent(pCarousel, 1, 0) == XUI_OK, "select child page");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pChild) == XUI_OK, "focus page child");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 1,
		"page child arrow does not navigate carousel");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pCarousel) == XUI_OK, "focus carousel");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_END, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3,
		"end selects last page");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_HOME, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0,
		"home selects first page");
	iEscapeBefore = tState.iEscapes;
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		tState.iEscapes == iEscapeBefore + 1,
		"idle escape bubbles to the parent scope");
	iChangeBefore = tState.iChanges;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pCarousel, 4u,
		XUI_ACCESSIBLE_ACTION_SET_SELECTION, NULL) == XUI_OK &&
		xuiCarouselGetCurrent(pCarousel) == 2 && tState.iChanges == iChangeBefore + 1,
		"accessibility selects page");
	tState.iChanges = 0;
	tState.iOld = -1;
	tState.iNew = -1;
	pChild = NULL;

	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "next");
	XUI_TEST_CHECK(tState.iChanges == 1 && tState.iOld == 2 && tState.iNew == 3, "next callback");
	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "descriptor non-loop end");
	iRet = xuiCarouselSetLoop(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetLoop(pCarousel) == 1, "enable loop");
	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "loop next");
	iRet = xuiCarouselPrev(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "loop prev");
	iRet = xuiCarouselSetLoop(pCarousel, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetLoop(pCarousel) == 0, "set loop false");
	iRet = xuiCarouselNext(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3, "next at end without loop");
	iRet = xuiCarouselSetLoop(pCarousel, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set loop true");
	iRet = xuiCarouselSetCurrent(pCarousel, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "set current");
	iRet = xuiCarouselSetAutoPlay(pCarousel, 1, 0.1f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set autoplay");
	iRet = xuiCarouselGetAutoPlay(pCarousel, &bAuto, &fInterval);
	XUI_TEST_CHECK(iRet == XUI_OK && bAuto && fInterval >= 0.09f, "get autoplay");
	iRet = xuiUpdate(pContext, 0.11f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 1, "auto advance");
	XUI_TEST_CHECK(xuiCarouselSetAutoPlay(pCarousel, 0, 0.0f) == XUI_OK &&
		xuiCarouselSetCurrent(pCarousel, 0, 0) == XUI_OK,
		"prepare style autoplay default interval");
	tStyle = __xuiCarouselStyleProp("carousel.auto.enabled",
		__xuiCarouselStyleBool(1));
	XUI_TEST_CHECK(xuiWidgetSetInlineStyle(pCarousel, &tStyle, 1) == XUI_OK,
		"enable autoplay through style");
	XUI_TEST_CHECK(xuiUpdate(pContext, 2.9f) == XUI_OK &&
		xuiCarouselGetCurrent(pCarousel) == 0,
		"style autoplay waits for its default interval");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.2f) == XUI_OK &&
		xuiCarouselGetCurrent(pCarousel) == 1,
		"style autoplay uses the API default interval");
	XUI_TEST_CHECK(xuiWidgetSetInlineStyle(pCarousel, NULL, 0) == XUI_OK,
		"clear autoplay style");
	iRet = xuiCarouselSetMetrics(pCarousel, 34.0f, 8.0f, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	XUI_TEST_CHECK(xuiCarouselSetMetrics(pCarousel, NAN, 8.0f, 6.0f, 18.0f) == XUI_ERROR_INVALID_ARGUMENT, "reject NaN metrics");
	XUI_TEST_CHECK(xuiCarouselSetMetrics(pCarousel, 34.0f,
		XUI_LAYOUT_UNBOUNDED, 6.0f, 18.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"reject metrics that can overflow indicator geometry");
	XUI_TEST_CHECK(xuiCarouselSetAutoPlay(pCarousel, 1, INFINITY) == XUI_ERROR_INVALID_ARGUMENT, "reject infinite autoplay interval");
	XUI_TEST_CHECK(xuiCarouselSetPageCount(pCarousel, 0) == XUI_ERROR_INVALID_ARGUMENT, "reject zero page count");
	XUI_TEST_CHECK(xuiCarouselSetPageCount(pCarousel, XUI_CAROUSEL_MAX_PAGES + 1) == XUI_ERROR_INVALID_ARGUMENT, "reject excessive page count");
	iRet = xuiCarouselSetColors(pCarousel,
		XUI_COLOR_RGBA(255, 82, 36, 255),
		XUI_COLOR_RGBA(20, 20, 20, 120),
		XUI_COLOR_RGBA(20, 20, 20, 180),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(255, 255, 255, 140),
		XUI_COLOR_RGBA(255, 255, 255, 255),
		XUI_COLOR_RGBA(255, 255, 255, 210));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	XUI_TEST_CHECK(xuiCarouselGetChangeCount(pCarousel) >= 3, "change count");

	iRet = xuiTestSurfaceCreate(&tProxyState, &pTarget, 520, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tFullRect = (xui_rect_i_t){0, 0, 520, 320};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	XUI_TEST_CHECK(xuiWidgetGetCacheSurface(pCarousel, 0) != NULL, "carousel cache");
	xuiWidgetClearDirty(pCarousel, XUI_WIDGET_DIRTY_ALL);
	XUI_TEST_CHECK(xuiCarouselSetIndicatorsVisible(pCarousel, 0) == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"indicator visibility invalidates paint without relayout");
	XUI_TEST_CHECK(xuiCarouselSetIndicatorsVisible(pCarousel, 1) == XUI_OK,
		"restore indicator visibility");
	xuiWidgetClearDirty(pCarousel, XUI_WIDGET_DIRTY_ALL);
	XUI_TEST_CHECK(xuiCarouselSetArrowsOnHover(pCarousel, 1) == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"arrow visibility mode invalidates paint without relayout");
	XUI_TEST_CHECK(xuiCarouselSetArrowsOnHover(pCarousel, 0) == XUI_OK,
		"restore arrow visibility mode");
	xuiWidgetClearDirty(pCarousel, XUI_WIDGET_DIRTY_ALL);
	XUI_TEST_CHECK(xuiCarouselSetMetrics(pCarousel, 35.0f, 9.0f, 7.0f, 19.0f) == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"chrome metrics invalidate paint without relayout");
	XUI_TEST_CHECK(xuiCarouselSetMetrics(pCarousel, 34.0f, 8.0f, 6.0f, 18.0f) == XUI_OK,
		"restore chrome metrics");
	xuiWidgetClearDirty(pCarousel, XUI_WIDGET_DIRTY_ALL);
	XUI_TEST_CHECK(xuiCarouselSetFont(pCarousel, pFontAlt) == XUI_OK &&
		xuiCarouselGetFont(pCarousel) == pFontAlt &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_LAYOUT) == 0u &&
		(xuiWidgetGetDirtyFlags(pCarousel) & XUI_WIDGET_DIRTY_CACHE) != 0u,
		"compatibility font invalidates paint without relayout");
	XUI_TEST_CHECK(xuiCarouselSetFont(pCarousel, pFont) == XUI_OK,
		"restore compatibility font");

	iRet = xuiCarouselSetCurrent(pCarousel, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset before autoplay state tests");
	iRet = xuiCarouselSetAutoPlay(pCarousel, 1, 0.1f);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable autoplay state tests");
	tWorld = xuiWidgetGetWorldRect(pCarousel);
	XUI_TEST_CHECK(xuiInputPointerMove(pContext,
		tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiUpdate(pContext, 0.11f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0,
		"hover pauses autoplay");
	XUI_TEST_CHECK(xuiInputPointerMove(pContext, 2, 2, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiUpdate(pContext, 0.05f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0 &&
		xuiUpdate(pContext, 0.06f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 1,
		"autoplay restarts a full interval after hover");
	XUI_TEST_CHECK(xuiCarouselSetCurrent(pCarousel, 0, 0) == XUI_OK &&
		xuiUpdate(pContext, 0.09f) == XUI_OK &&
		xuiCarouselSetPageCount(pCarousel, 3) == XUI_OK &&
		xuiUpdate(pContext, 0.02f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0,
		"page-count changes reset autoplay timing");
	XUI_TEST_CHECK(xuiCarouselSetPageCount(pCarousel, 4) == XUI_OK,
		"restore page count after autoplay timing test");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCarousel, 0) == XUI_OK, "disable carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch disabled carousel");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.11f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "disabled carousel pauses autoplay");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCarousel, 1) == XUI_OK, "enable carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch enabled carousel");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.05f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "enabled carousel restarts autoplay interval");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.06f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 1, "enabled carousel advances after interval");
	iRet = xuiCarouselSetCurrent(pCarousel, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset before hidden autoplay test");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pCarousel, 0) == XUI_OK, "hide carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch hidden carousel");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.11f) == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0, "hidden carousel pauses autoplay");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pCarousel, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0,
		"hidden carousel accessibility root is offscreen");
	iCurrentBefore = xuiCarouselGetCurrent(pCarousel);
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pCarousel, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiCarouselGetCurrent(pCarousel) == iCurrentBefore,
		"hidden carousel rejects accessibility navigation");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pCarousel, 1) == XUI_OK, "show carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch shown carousel");

	XUI_TEST_CHECK(xuiCarouselSetAutoPlay(pCarousel, 0, 0.0f) == XUI_OK, "disable autoplay for pointer tests");
	XUI_TEST_CHECK(xuiCarouselSetArrowsOnHover(pCarousel, 0) == XUI_OK, "show arrows for pointer tests");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout pointer tests");
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, &tFullRect, 1) == XUI_OK, "render pointer tests");
	tWorld = xuiWidgetGetWorldRect(pCarousel);
	XUI_TEST_CHECK(xuiCarouselSetLoop(pCarousel, 0) == XUI_OK &&
		xuiCarouselSetCurrent(pCarousel, 0, 0) == XUI_OK, "prepare unavailable previous arrow");
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, &tFullRect, 1) == XUI_OK,
		"render unavailable previous arrow");
	XUI_TEST_CHECK(xuiInputPointerDown(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"unavailable previous arrow is not interactive");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"release unavailable previous arrow");
	XUI_TEST_CHECK(xuiCarouselSetLoop(pCarousel, 1) == XUI_OK, "restore loop for pointer tests");
	iChangeBefore = tState.iChanges;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 35), (int)(tWorld.fY + tWorld.fH / 2),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pCarousel,
		"first touch owns carousel arrow press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 35), (int)(tWorld.fY + tWorld.fH / 2),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + 36),
		(int)(tWorld.fY + tWorld.fH / 2), 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + 35),
		(int)(tWorld.fY + tWorld.fH / 2), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 0 &&
		tState.iChanges == iChangeBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pCarousel,
		"unrelated touch move and release preserve carousel press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 35), (int)(tWorld.fY + tWorld.fH / 2),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCarouselGetCurrent(pCarousel) == 3 &&
		tState.iChanges == iChangeBefore + 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch activates carousel arrow once");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "metric mutation pointer down");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pCarousel, "metric mutation capture");
	XUI_TEST_CHECK(xuiCarouselSetMetrics(pCarousel, 35.0f, 8.0f, 6.0f, 18.0f) == XUI_OK, "mutate metrics while pressed");
	XUI_TEST_CHECK(xuiGetPointerCapture(pContext) == NULL, "metric mutation cancels capture");

	iRet = xuiInputPointerDown(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "lost button pointer down");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pCarousel, "lost button capture");
	XUI_TEST_CHECK(xuiInputPointerMove(pContext, tWorld.fX + 36, tWorld.fY + tWorld.fH / 2, 0) == XUI_OK, "lost button pointer move");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "lost button cancels capture");

	XUI_TEST_CHECK(xuiInputPointerDown(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "disable pointer down");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pCarousel, "disable pointer capture");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCarousel, 0) == XUI_OK, "disable pressed carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "disable cancels pointer capture");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pCarousel, 1) == XUI_OK, "reenable pressed carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch reenabled pressed carousel");

	XUI_TEST_CHECK(xuiInputPointerDown(pContext, tWorld.fX + 35, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "hide pointer down");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pCarousel, "hide pointer capture");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pCarousel, 0) == XUI_OK, "hide pressed carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "hide cancels pointer capture");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pCarousel, 1) == XUI_OK, "restore pressed carousel");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch restored pressed carousel");
	XUI_TEST_CHECK(xuiCarouselSetCurrent(pCarousel, 3, 0) == XUI_OK, "select last before shrink");
	iChangeBefore = xuiCarouselGetChangeCount(pCarousel);
	iCallbackBefore = tState.iChanges;
	XUI_TEST_CHECK(xuiCarouselSetPageCount(pCarousel, 2) == XUI_OK &&
		xuiCarouselGetCurrent(pCarousel) == 1 &&
		xuiCarouselGetChangeCount(pCarousel) == iChangeBefore + 1 &&
		tState.iChanges == iCallbackBefore && xuiWidgetGetAccessibleNodeCount(pCarousel) == 3,
		"page shrink clamps selection without user callback");
	XUI_TEST_CHECK(xuiCarouselSetPageCount(pCarousel, 4) == XUI_OK &&
		xuiCarouselSetCurrent(pCarousel, 0, 0) == XUI_OK,
		"restore pages after shrink");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout reentrant callback test");
	XUI_TEST_CHECK(xuiRender(pContext, pTarget, &tFullRect, 1) == XUI_OK, "render reentrant callback test");
	tWorld = xuiWidgetGetWorldRect(pCarousel);
	iChangeBefore = tState.iChanges;
	tState.bDestroyOnChange = 1;
	XUI_TEST_CHECK(xuiInputPointerDown(pContext, tWorld.fX + tWorld.fW - 36, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK, "reentrant callback pointer down");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pCarousel, "reentrant callback capture");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext, tWorld.fX + tWorld.fW - 36, tWorld.fY + tWorld.fH / 2,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK, "reentrant callback pointer up");
	XUI_TEST_CHECK(xuiDispatchPendingEvents(pContext) == XUI_OK, "dispatch reentrant callback");
	XUI_TEST_CHECK(tState.iChanges == iChangeBefore + 1 && xuiGetPointerCapture(pContext) == NULL, "destroying callback completes safely");

cleanup:
	if ( pTarget != NULL ) {
		tProxyState.tProxy.surfaceDestroy(&tProxyState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFont);
	}
	if ( pFontAlt != NULL ) {
		tProxyState.tProxy.fontDestroy(&tProxyState.tProxy, pFontAlt);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_carousel_test passed\n");
	return 0;
}
