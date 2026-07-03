#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_accordion_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_accordion_test_events_t {
	int iSelectCount;
	int iLastIndex;
	int iLastId;
} xui_accordion_test_events_t;

static int __xuiAccordionNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.05f;
}

static void __xuiAccordionSelected(xui_widget pWidget, int iIndex, int iId, void* pUser)
{
	xui_accordion_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_accordion_test_events_t*)pUser;
	if ( pEvents != NULL ) {
		pEvents->iSelectCount++;
		pEvents->iLastIndex = iIndex;
		pEvents->iLastId = iId;
	}
}

static int __xuiAccordionLayoutRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;
	int iRet;

	iRet = xuiLayout(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.016f);
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, 640, 360};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiAccordionDispatchClick(xui_context pContext, float fX, float fY)
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

static int __xuiAccordionAddFixedChild(xui_context pContext, xui_widget pAccordion, int iSection, float fHeight)
{
	xui_widget pChild;
	int iRet;

	iRet = xuiWidgetCreate(pContext, &pChild);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiWidgetSetSizeMode(pChild, XUI_SIZE_FILL, XUI_SIZE_FIXED);
	(void)xuiWidgetSetPreferredSize(pChild, (xui_vec2_t){0.0f, fHeight});
	(void)xuiWidgetSetAlign(pChild, XUI_ALIGN_STRETCH, XUI_ALIGN_START);
	iRet = xuiAccordionAddSectionChild(pAccordion, iSection, pChild);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pChild);
		return iRet;
	}
	return XUI_OK;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_accordion_test_events_t tEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pAccordion;
	xui_widget pClient;
	xui_surface pTarget;
	xui_surface pHeaderCache;
	xui_accordion_desc_t tDesc;
	xui_rect_t tWorld;
	xui_rect_t tHeader;
	xui_rect_t tClient0;
	xui_rect_t tClient1;
	xui_rect_t tArrow;
	uint32_t iBg;
	uint32_t iHeader;
	float fHeaderHeight;
	float fSpacing;
	float fPadding;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pAccordion = NULL;
	pTarget = NULL;
	pHeaderCache = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	tEvents.iLastIndex = -1;
	tEvents.iLastId = 0;
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
	tDesc.iMode = XUI_ACCORDION_MODE_MULTIPLE;
	tDesc.fHeaderHeight = 30.0f;
	tDesc.fSpacing = 6.0f;
	tDesc.fContentPadding = 10.0f;
	iRet = xuiAccordionCreate(pContext, &pAccordion, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pAccordion != NULL, "accordion create");
	iRet = xuiWidgetAddChild(pRoot, pAccordion);
	XUI_TEST_CHECK(iRet == XUI_OK, "accordion add");
	xuiWidgetSetRect(pAccordion, (xui_rect_t){20.0f, 20.0f, 320.0f, 260.0f});
	iRet = xuiAccordionSetSelect(pAccordion, __xuiAccordionSelected, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");

	iRet = xuiAccordionAddSection(pAccordion, "General", 101, 1, &pClient);
	XUI_TEST_CHECK(iRet == XUI_OK && pClient != NULL, "section 0 add");
	iRet = xuiAccordionAddSection(pAccordion, "Advanced", 102, 0, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "section 1 add");
	iRet = xuiAccordionAddSection(pAccordion, "Locked", 103, 1, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "section 2 add");
	iRet = xuiAccordionSetSectionEnabled(pAccordion, 2, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "section disabled");
	iRet = __xuiAccordionAddFixedChild(pContext, pAccordion, 0, 24.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "child 0");
	iRet = __xuiAccordionAddFixedChild(pContext, pAccordion, 1, 30.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "child 1");
	iRet = __xuiAccordionAddFixedChild(pContext, pAccordion, 2, 26.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "child 2");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 640, 360, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");

	XUI_TEST_CHECK(xuiAccordionGetSectionCount(pAccordion) == 3, "section count");
	XUI_TEST_CHECK(strcmp(xuiAccordionGetSectionTitle(pAccordion, 0), "General") == 0, "title");
	XUI_TEST_CHECK(xuiAccordionGetSectionId(pAccordion, 1) == 102, "id");
	XUI_TEST_CHECK(xuiAccordionGetSectionWidget(pAccordion, 0) != NULL, "section widget");
	XUI_TEST_CHECK(xuiAccordionGetHeaderWidget(pAccordion, 0) != NULL, "header widget");
	XUI_TEST_CHECK(xuiAccordionGetButtonWidget(pAccordion, 0) == xuiAccordionGetHeaderWidget(pAccordion, 0), "button alias");
	XUI_TEST_CHECK(xuiAccordionGetClientWidget(pAccordion, 0) != NULL, "client widget");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 0) == 1, "section 0 expanded");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 1) == 0, "section 1 collapsed");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 2) == 1, "section 2 expanded");
	XUI_TEST_CHECK(xuiAccordionIsSectionEnabled(pAccordion, 2) == 0, "section 2 disabled");
	pHeaderCache = xuiWidgetGetCacheSurface(xuiAccordionGetHeaderWidget(pAccordion, 0), xuiWidgetGetStateId(xuiAccordionGetHeaderWidget(pAccordion, 0)));
	XUI_TEST_CHECK((pHeaderCache != NULL) && (xuiTestSurfaceGetRectFillColorCount(pHeaderCache, XUI_COLOR_RGBA(47, 125, 215, 255)) > 0), "initial expanded indicator");
	XUI_TEST_CHECK((pHeaderCache != NULL) && (xuiTestSurfaceGetRectFillColorCount(pHeaderCache, XUI_COLOR_RGBA(127, 196, 229, 255)) >= 3), "expanded header border rects");
	pHeaderCache = xuiWidgetGetCacheSurface(xuiAccordionGetHeaderWidget(pAccordion, 1), xuiWidgetGetStateId(xuiAccordionGetHeaderWidget(pAccordion, 1)));
	XUI_TEST_CHECK((pHeaderCache != NULL) && (xuiTestSurfaceGetRectFillColorCount(pHeaderCache, XUI_COLOR_RGBA(47, 125, 215, 255)) == 0), "collapsed has no indicator");
	XUI_TEST_CHECK((pHeaderCache != NULL) && (xuiTestSurfaceGetRectFillColorCount(pHeaderCache, XUI_COLOR_RGBA(127, 196, 229, 255)) >= 4), "collapsed header border rects");
	tClient0 = xuiAccordionGetClientRect(pAccordion, 0);
	tClient1 = xuiAccordionGetClientRect(pAccordion, 1);
	tArrow = xuiAccordionGetArrowRect(pAccordion, 0);
	XUI_TEST_CHECK(tClient0.fH > 20.0f, "expanded client height");
	XUI_TEST_CHECK(__xuiAccordionNear(tClient1.fH, 0.0f), "collapsed client height");
	XUI_TEST_CHECK(tArrow.fW > 0.0f && tArrow.fH > 0.0f, "arrow rect");
	XUI_TEST_CHECK(xuiAccordionGetContentHeight(pAccordion) > 90.0f, "content height");

	tWorld = xuiWidgetGetWorldRect(pAccordion);
	tHeader = xuiAccordionGetHeaderRect(pAccordion, 1);
	iRet = __xuiAccordionDispatchClick(pContext, tWorld.fX + tHeader.fX + tHeader.fW * 0.5f, tWorld.fY + tHeader.fY + tHeader.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click section 1");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after click");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 1) == 1 && tEvents.iSelectCount == 1 && tEvents.iLastIndex == 1 && tEvents.iLastId == 102, "click expands and notifies");

	tHeader = xuiAccordionGetHeaderRect(pAccordion, 2);
	iRet = __xuiAccordionDispatchClick(pContext, tWorld.fX + tHeader.fX + tHeader.fW * 0.5f, tWorld.fY + tHeader.fY + tHeader.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == 1, "disabled click ignored");

	iRet = xuiAccordionSetMode(pAccordion, XUI_ACCORDION_MODE_SINGLE);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetMode(pAccordion) == XUI_ACCORDION_MODE_SINGLE, "single mode");
	iRet = xuiAccordionSetExpanded(pAccordion, 1, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "single expand section 1");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render single");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 0) == 0 && xuiAccordionIsExpanded(pAccordion, 1) == 1 && xuiAccordionIsExpanded(pAccordion, 2) == 0, "single collapses peers");

	iRet = xuiAccordionSetMetrics(pAccordion, 32.0f, 5.0f, 9.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiAccordionGetMetrics(pAccordion, &fHeaderHeight, &fSpacing, &fPadding);
	XUI_TEST_CHECK(iRet == XUI_OK && fHeaderHeight == 32.0f && fSpacing == 5.0f && fPadding == 9.0f, "get metrics");
	iRet = xuiAccordionSetColors(pAccordion,
		XUI_COLOR_RGBA(245, 252, 248, 255),
		XUI_COLOR_RGBA(219, 242, 234, 255),
		XUI_COLOR_RGBA(203, 235, 224, 255),
		XUI_COLOR_RGBA(205, 238, 228, 255),
		XUI_COLOR_RGBA(250, 254, 252, 255),
		XUI_COLOR_RGBA(84, 180, 148, 255),
		XUI_COLOR_RGBA(30, 82, 66, 255),
		XUI_COLOR_RGBA(18, 102, 78, 255),
		XUI_COLOR_RGBA(126, 150, 142, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiAccordionGetColors(pAccordion, &iBg, &iHeader, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iBg == XUI_COLOR_RGBA(245, 252, 248, 255) && iHeader == XUI_COLOR_RGBA(219, 242, 234, 255), "get colors");

	iRet = xuiSetFocusWidget(pContext, pAccordion);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus accordion");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSelected(pAccordion) == 0, "keyboard step skips disabled");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key space");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionIsExpanded(pAccordion, 0) == 1 && xuiAccordionIsExpanded(pAccordion, 1) == 0 && tEvents.iSelectCount == 2, "keyboard toggles selected");
	XUI_TEST_CHECK((xuiAccordionGetState(pAccordion) & XUI_WIDGET_STATE_FOCUS) != 0, "focus state");
	XUI_TEST_CHECK(xuiAccordionGetChangeCount(pAccordion) >= 3, "change count");

	iRet = xuiAccordionClear(pAccordion);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSectionCount(pAccordion) == 0, "clear");

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
	printf("xui_accordion_test passed\n");
	return 0;
}
