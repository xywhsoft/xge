#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_accordion_destroy_events_t {
	int iCount;
} xui_accordion_destroy_events_t;

typedef struct xui_accordion_reentrant_destroy_t {
	xui_widget pAccordion;
	xui_widget pAddedClient;
	int iDestroyCount;
	int iAddResult;
} xui_accordion_reentrant_destroy_t;

static xui_style_value_t __xuiAccordionStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiAccordionStyleProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

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

static void __xuiAccordionDestroySelected(xui_widget pWidget, int iIndex, int iId, void* pUser)
{
	xui_accordion_destroy_events_t* pEvents = (xui_accordion_destroy_events_t*)pUser;
	(void)iIndex;
	(void)iId;
	if ( pEvents != NULL ) pEvents->iCount++;
	(void)xuiWidgetDestroy(pWidget);
}

static void __xuiAccordionReentrantChildDestroy(xui_widget pWidget,
	void* pTypeData, void* pUser)
{
	xui_accordion_reentrant_destroy_t* pState =
		(xui_accordion_reentrant_destroy_t*)pUser;

	(void)pWidget;
	(void)pTypeData;
	if ( pState == NULL ) return;
	pState->iDestroyCount++;
	pState->iAddResult = xuiAccordionAddSection(pState->pAccordion,
		"Added during destroy", 802, 0, &pState->pAddedClient);
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

static int __xuiAccordionDispatchPress(xui_context pContext, xui_widget pAccordion, int iSection, float* pX, float* pY)
{
	xui_widget pHeader;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iRet;

	pHeader = xuiAccordionGetHeaderWidget(pAccordion, iSection);
	if ( pHeader == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pHeader);
	fX = tWorld.fX + tWorld.fW * 0.5f;
	fY = tWorld.fY + tWorld.fH * 0.5f;
	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK && pX != NULL ) *pX = fX;
	if ( iRet == XUI_OK && pY != NULL ) *pY = fY;
	return iRet;
}

static int __xuiAccordionDispatchRelease(xui_context pContext, float fX, float fY)
{
	int iRet;

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
	xui_accordion_destroy_events_t tDestroy;
	xui_accordion_reentrant_destroy_t tReentrantDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pAccordion;
	xui_widget pMutationAccordion;
	xui_widget pDestroyAccordion;
	xui_widget pInvalid;
	xui_widget pClient;
	xui_widget pReentrantChild;
	xui_widget arrMutationClients[3];
	xui_widget_type pReentrantType;
	xui_widget_type_desc_t tTypeDesc;
	xui_surface pTarget;
	xui_surface pHeaderCache;
	xui_accordion_desc_t tDesc;
	xui_accordion_desc_t tInvalidDesc;
	xui_accessible_node_t tNode;
	xui_style_property_t tStyle;
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
	float fPressX;
	float fPressY;
	char sLongTitle[68];
	char sExpectedTitle[64];
	int i;
	int iFailed;
	int iRet;
	int iBefore;

	pContext = NULL;
	pRoot = NULL;
	pAccordion = NULL;
	pMutationAccordion = NULL;
	pDestroyAccordion = NULL;
	pInvalid = NULL;
	pReentrantChild = NULL;
	pReentrantType = NULL;
	pTarget = NULL;
	pHeaderCache = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tDestroy, 0, sizeof(tDestroy));
	memset(&tReentrantDestroy, 0, sizeof(tReentrantDestroy));
	tReentrantDestroy.iAddResult = XUI_ERROR_INVALID_STATE;
	tEvents.iLastIndex = -1;
	tEvents.iLastId = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	memset(&tTypeDesc, 0, sizeof(tTypeDesc));
	tTypeDesc.iSize = sizeof(tTypeDesc);
	tTypeDesc.sName = "test.accordion.reentrant-child";
	tTypeDesc.pParent = xuiWidgetGetBaseType();
	tTypeDesc.pUser = &tReentrantDestroy;
	tTypeDesc.onDestroy = __xuiAccordionReentrantChildDestroy;
	iRet = xuiWidgetRegisterType(pContext, &pReentrantType, &tTypeDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pReentrantType != NULL,
		"reentrant child type register");
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
	tInvalidDesc = tDesc;
	tInvalidDesc.fHeaderHeight = NAN;
	iRet = xuiAccordionCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nonfinite descriptor metric");
	tInvalidDesc = tDesc;
	tInvalidDesc.iSectionCount = 1;
	tInvalidDesc.arrSections = NULL;
	iRet = xuiAccordionCreate(pContext, &pInvalid, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject missing descriptor sections");
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
	sLongTitle[0] = 'A';
	for ( i = 0; i < 21; i++ ) {
		sLongTitle[1 + i * 3] = (char)0xe4;
		sLongTitle[2 + i * 3] = (char)0xb8;
		sLongTitle[3 + i * 3] = (char)0xad;
	}
	sLongTitle[64] = '\0';
	sExpectedTitle[0] = 'A';
	for ( i = 0; i < 20; i++ ) {
		sExpectedTitle[1 + i * 3] = (char)0xe4;
		sExpectedTitle[2 + i * 3] = (char)0xb8;
		sExpectedTitle[3 + i * 3] = (char)0xad;
	}
	sExpectedTitle[61] = '\0';
	iRet = xuiAccordionSetSectionTitle(pAccordion, 1, sLongTitle);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiAccordionGetSectionTitle(pAccordion, 1), sExpectedTitle) == 0, "title truncation keeps whole utf8 codepoints");
	memset(sLongTitle, 'a', XUI_ACCORDION_TITLE_CAPACITY - 2u);
	memcpy(sLongTitle + XUI_ACCORDION_TITLE_CAPACITY - 2u, "e\xCC\x81", 4u);
	iRet = xuiAccordionSetSectionTitle(pAccordion, 1, sLongTitle);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strlen(xuiAccordionGetSectionTitle(pAccordion, 1)) == XUI_ACCORDION_TITLE_CAPACITY - 2u,
		"title truncation preserves grapheme boundary");
	iRet = xuiAccordionSetSectionTitle(pAccordion, 1, "Advanced");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore section title");
	iRet = xuiAccordionSetSectionTitle(pAccordion, 1,
		xuiAccordionGetSectionTitle(pAccordion, 1));
	XUI_TEST_CHECK(iRet == XUI_OK &&
		strcmp(xuiAccordionGetSectionTitle(pAccordion, 1), "Advanced") == 0,
		"borrowed section title may be assigned back");
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
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pAccordion) == 4 &&
		xuiWidgetGetAccessibleNode(pAccordion, 0, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_GROUP && tNode.iRowCount == 3 &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_VERTICAL) != 0,
		"accordion accessibility root");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pAccordion, 1, &tNode) == XUI_OK &&
		tNode.iRole == XUI_ACCESSIBLE_ROLE_BUTTON && strcmp(tNode.sName, "General") == 0 &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_EXPANDED) != 0 &&
		(tNode.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE)) != 0,
		"accordion accessibility section");
	memset(&tNode, 0, sizeof(tNode));
	tNode.iSize = sizeof(tNode);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pAccordion, 3, &tNode) == XUI_OK &&
		(tNode.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0 &&
		xuiWidgetPerformAccessibleAction(pAccordion, 4u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_ERROR_INVALID_STATE,
		"disabled accordion section rejects accessibility action");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pAccordion, 2u, XUI_ACCESSIBLE_ACTION_FOCUS, NULL) == XUI_OK &&
		xuiAccordionGetSelected(pAccordion) == 0 && xuiGetFocusWidget(pContext) == pAccordion,
		"accessibility focuses accordion section");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiAccordionGetState(pAccordion) & XUI_WIDGET_STATE_DISABLED) != 0,
		"ancestor disable updates accordion state");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiAccordionGetState(pAccordion) & XUI_WIDGET_STATE_DISABLED) == 0,
		"ancestor enable restores accordion state");

	tWorld = xuiWidgetGetWorldRect(pAccordion);
	tHeader = xuiAccordionGetHeaderRect(pAccordion, 1);
	iBefore = tEvents.iSelectCount;
	iRet = __xuiAccordionDispatchClick(pContext, tWorld.fX + tHeader.fX + tHeader.fW * 0.5f, tWorld.fY + tHeader.fY + tHeader.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "click section 1");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after click");
	XUI_TEST_CHECK(xuiAccordionIsExpanded(pAccordion, 1) == 1 && tEvents.iSelectCount == iBefore + 1 && tEvents.iLastIndex == 1 && tEvents.iLastId == 102, "click expands and notifies");

	tHeader = xuiAccordionGetHeaderRect(pAccordion, 2);
	iBefore = tEvents.iSelectCount;
	iRet = __xuiAccordionDispatchClick(pContext, tWorld.fX + tHeader.fX + tHeader.fW * 0.5f, tWorld.fY + tHeader.fY + tHeader.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore, "disabled click ignored");

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
	iRet = xuiAccordionSetMetrics(pAccordion, NAN, 5.0f, 9.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan metric");
	iRet = xuiAccordionSetMetrics(pAccordion, 32.0f, INFINITY, 9.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metric");
	iRet = xuiAccordionGetMetrics(pAccordion, &fHeaderHeight, &fSpacing, &fPadding);
	XUI_TEST_CHECK(iRet == XUI_OK && fHeaderHeight == 32.0f && fSpacing == 5.0f && fPadding == 9.0f, "invalid metrics preserve state");
	tStyle = __xuiAccordionStyleProp("accordion.header.height", __xuiAccordionStyleFloat(NAN));
	iRet = xuiWidgetSetInlineStyle(pAccordion, &tStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set invalid style metric");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiAccordionNear(xuiAccordionGetHeaderRect(pAccordion, 0).fH, 32.0f), "ignore invalid style metric");
	iRet = xuiWidgetSetInlineStyle(pAccordion, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid style");
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
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key space");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionIsExpanded(pAccordion, 0) == 1 &&
		xuiAccordionIsExpanded(pAccordion, 1) == 0 && tEvents.iSelectCount == iBefore + 1,
		"keyboard toggles selected");
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputKeyDown(pContext, XUI_KEY_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiAccordionIsExpanded(pAccordion, 0) == 0 && xuiAccordionGetSelected(pAccordion) == 0 &&
		tEvents.iSelectCount == iBefore + 1, "left collapses and preserves header focus");
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiAccordionIsExpanded(pAccordion, 0) == 1 && xuiAccordionGetSelected(pAccordion) == 0 &&
		tEvents.iSelectCount == iBefore + 1, "right expands selected section");
	XUI_TEST_CHECK((xuiAccordionGetState(pAccordion) & XUI_WIDGET_STATE_FOCUS) != 0, "focus state");
	XUI_TEST_CHECK(xuiAccordionGetChangeCount(pAccordion) >= 3, "change count");

	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before capture tests");
	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0) && xuiAccordionGetActiveIndex(pAccordion) == 0, "header press captures header");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiAccordionGetActiveIndex(pAccordion) == -1, "escape cancels header capture");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after escape");

	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0), "blur capture acquired");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear accordion focus");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiAccordionGetActiveIndex(pAccordion) == -1, "blur cancels header capture");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after blur");

	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0), "section disable capture acquired");
	iRet = xuiAccordionSetSectionEnabled(pAccordion, 0, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiAccordionGetActiveIndex(pAccordion) == -1, "section disable cancels header capture");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after section disable");
	iRet = xuiAccordionSetSectionEnabled(pAccordion, 0, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore section enabled");

	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0), "widget disable capture acquired");
	iRet = xuiWidgetSetEnabled(pAccordion, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable accordion");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiAccordionGetActiveIndex(pAccordion) == -1, "widget disable cancels header capture");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after widget disable");
	iRet = xuiWidgetSetEnabled(pAccordion, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore accordion enabled");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch accordion enabled");

	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0), "widget hide capture acquired");
	iRet = xuiWidgetSetVisible(pAccordion, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide accordion");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiAccordionGetActiveIndex(pAccordion) == -1, "widget hide cancels header capture");
	iRet = xuiWidgetSetVisible(pAccordion, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore accordion visible");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch accordion visible");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after widget hide");

	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before clear capture");
	tWorld = xuiWidgetGetWorldRect(xuiAccordionGetHeaderWidget(pAccordion, 0));
	fPressX = tWorld.fX + tWorld.fW * 0.5f;
	fPressY = tWorld.fY + tWorld.fH * 0.5f;
	iBefore = tEvents.iSelectCount;
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)fPressX, (int)fPressY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) ==
			xuiAccordionGetHeaderWidget(pAccordion, 0) &&
		xuiAccordionGetActiveIndex(pAccordion) == 0,
		"first touch owns accordion header press");
	tHeader = xuiWidgetGetWorldRect(xuiAccordionGetHeaderWidget(pAccordion, 1));
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tHeader.fX + tHeader.fW * 0.5f),
		(int)(tHeader.fY + tHeader.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tHeader.fX + tHeader.fW * 0.5f),
		(int)(tHeader.fY + tHeader.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) ==
			xuiAccordionGetHeaderWidget(pAccordion, 0) &&
		xuiAccordionGetActiveIndex(pAccordion) == 0,
		"unrelated touch cannot toggle or release accordion header");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)fPressX, (int)fPressY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iSelectCount == iBefore + 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiAccordionGetActiveIndex(pAccordion) == -1,
		"owning touch toggles accordion header once");
	iRet = __xuiAccordionLayoutRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render after multi-touch header test");
	iRet = __xuiAccordionDispatchPress(pContext, pAccordion, 0, &fPressX, &fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == xuiAccordionGetHeaderWidget(pAccordion, 0), "clear capture acquired");
	iRet = xuiAccordionClear(pAccordion);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSectionCount(pAccordion) == 0 &&
		xuiAccordionGetContentHeight(pAccordion) == 0.0f &&
		xuiGetPointerCapture(pContext) == NULL &&
		xuiAccordionGetActiveIndex(pAccordion) == -1,
		"clear cancels header capture and resets cached geometry");
	iRet = __xuiAccordionDispatchRelease(pContext, fPressX, fPressY);
	XUI_TEST_CHECK(iRet == XUI_OK, "release after clear");

	tReentrantDestroy.pAccordion = pAccordion;
	iRet = xuiAccordionAddSection(pAccordion, "Removed", 801, 0, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetCreateTyped(pContext,
		pReentrantType, &pReentrantChild, NULL);
	if ( iRet == XUI_OK ) iRet = xuiAccordionAddSectionChild(pAccordion, 0,
		pReentrantChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "reentrant clear setup");
	iRet = xuiAccordionClear(pAccordion);
	pReentrantChild = NULL;
	XUI_TEST_CHECK(iRet == XUI_OK && tReentrantDestroy.iDestroyCount == 1 &&
		tReentrantDestroy.iAddResult == XUI_OK &&
		xuiAccordionGetSectionCount(pAccordion) == 1 &&
		xuiAccordionGetSectionId(pAccordion, 0) == 802 &&
		xuiAccordionGetClientWidget(pAccordion, 0) ==
			tReentrantDestroy.pAddedClient,
		"clear defers child destruction so reentrant additions survive");
	XUI_TEST_CHECK(xuiAccordionClear(pAccordion) == XUI_OK &&
		xuiAccordionGetSectionCount(pAccordion) == 0 &&
		tReentrantDestroy.iDestroyCount == 1,
		"reentrant clear cleanup");

	iRet = xuiAccordionCreate(pContext, &pMutationAccordion, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pMutationAccordion);
	if ( iRet == XUI_OK ) iRet = xuiAccordionAddSection(pMutationAccordion,
		"Alpha", 201, 1, &arrMutationClients[0]);
	if ( iRet == XUI_OK ) iRet = xuiAccordionAddSection(pMutationAccordion,
		"Beta", 202, 0, &arrMutationClients[1]);
	if ( iRet == XUI_OK ) iRet = xuiAccordionAddSection(pMutationAccordion,
		"Gamma", 203, 0, &arrMutationClients[2]);
	if ( iRet == XUI_OK ) iRet = xuiAccordionSetExpanded(pMutationAccordion, 2, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSelected(pMutationAccordion) == 2,
		"mutation accordion setup");
	iBefore = xuiAccordionGetChangeCount(pMutationAccordion);
	iRet = xuiAccordionRemoveSection(pMutationAccordion, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSectionCount(pMutationAccordion) == 2 &&
		xuiAccordionGetSelected(pMutationAccordion) == 1 &&
		xuiAccordionGetChangeCount(pMutationAccordion) == iBefore + 1 &&
		xuiAccordionGetSectionId(pMutationAccordion, 0) == 201 &&
		xuiAccordionGetSectionId(pMutationAccordion, 1) == 203 &&
		xuiAccordionGetClientWidget(pMutationAccordion, 0) == arrMutationClients[0] &&
		xuiAccordionGetClientWidget(pMutationAccordion, 1) == arrMutationClients[2],
		"remove section preserves remaining identity and selected item");
	iRet = xuiAccordionRemoveSection(pMutationAccordion, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSectionCount(pMutationAccordion) == 1 &&
		xuiAccordionGetSelected(pMutationAccordion) == 0 &&
		xuiAccordionGetSectionId(pMutationAccordion, 0) == 201,
		"remove selected section chooses remaining enabled item");
	iRet = xuiAccordionRemoveSection(pMutationAccordion, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiAccordionGetSectionCount(pMutationAccordion) == 0 &&
		xuiAccordionGetSelected(pMutationAccordion) == -1 &&
		xuiAccordionGetContentHeight(pMutationAccordion) == 0.0f &&
		xuiAccordionRemoveSection(pMutationAccordion, 0) == XUI_ERROR_INVALID_ARGUMENT,
		"remove final section resets model");
	xuiWidgetDestroy(pMutationAccordion);
	pMutationAccordion = NULL;

	iRet = xuiAccordionCreate(pContext, &pDestroyAccordion, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyAccordion != NULL, "destroy callback accordion create");
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pDestroyAccordion) == XUI_OK &&
		xuiAccordionAddSection(pDestroyAccordion, "Destroy", 901, 0, NULL) == XUI_OK &&
		xuiAccordionSetSelect(pDestroyAccordion, __xuiAccordionDestroySelected, &tDestroy) == XUI_OK,
		"destroy callback accordion setup");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pDestroyAccordion, 2u,
		XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL) == XUI_OK && tDestroy.iCount == 1,
		"selection callback may destroy accordion");
	pDestroyAccordion = NULL;

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
