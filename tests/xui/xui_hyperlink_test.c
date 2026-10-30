#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_hyperlink_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static void __xuiHyperlinkClicked(xui_widget pWidget, void* pUser)
{
	int* pCount;

	(void)pWidget;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_hyperlink_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_hyperlink_destroy_state_t;

static void __xuiHyperlinkDestroyClicked(xui_widget pWidget, void* pUser)
{
	xui_hyperlink_destroy_state_t* pState = (xui_hyperlink_destroy_state_t*)pUser;

	if ( pState != NULL ) {
		pState->iCount++;
		if ( pState->ppWidget != NULL && *pState->ppWidget == pWidget ) *pState->ppWidget = NULL;
	}
	xuiWidgetDestroy(pWidget);
}

static int __xuiHyperlinkSetTextOomPreserves(xui_widget pLink)
{
	xallocationbudget* pBudget = NULL;
	xallocationscope tScope;
	int iRet;

	memset(&tScope, 0, sizeof(tScope));
	if ( !xrtAllocationBudgetCreate(1u, &pBudget) ) return 0;
	if ( !xrtAllocationBudgetEnter(pBudget, &tScope) ) {
		xrtAllocationBudgetFree(pBudget);
		return 0;
	}
	iRet = xuiHyperlinkSetText(pLink,
		"This replacement is intentionally longer than the retained hyperlink allocation.");
	(void)xrtAllocationBudgetLeave(&tScope);
	xrtAllocationBudgetFree(pBudget);
	return iRet == XUI_ERROR_OUT_OF_MEMORY &&
		strcmp(xuiHyperlinkGetText(pLink), "Open document") == 0;
}

static xui_style_value_t __xuiHyperlinkTestColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_property_t __xuiHyperlinkTestProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static xui_style_desc_t __xuiHyperlinkTestStyle(const xui_style_property_t* pProps, int iCount)
{
	xui_style_desc_t tStyle;

	memset(&tStyle, 0, sizeof(tStyle));
	tStyle.iSize = sizeof(tStyle);
	tStyle.pProperties = pProps;
	tStyle.iPropertyCount = iCount;
	return tStyle;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pLink;
	xui_font pFont;
	xui_hyperlink_desc_t tDesc;
	xui_style_property_t arrProps[1];
	xui_style_desc_t tStyle;
	xui_vec2_t tMeasure;
	xui_rect_t tTextRect;
	xui_surface pNormalCache;
	xui_surface pHoverCache;
	xui_surface pActiveCache;
	xui_surface pFocusCache;
	xui_surface pHoverFocusCache;
	xui_surface pActiveFocusCache;
	xui_accessible_node_t tAccessible;
	xui_theme_t tTheme;
	xui_hyperlink_destroy_state_t tDestroyState;
	xui_widget pDestroyLink;
	xui_widget pProbe;
	int iClickCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pLink = NULL;
	pFont = NULL;
	pDestroyLink = NULL;
	pProbe = NULL;
	iClickCount = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	memset(&tDestroyState, 0, sizeof(tDestroyState));
	memset(&tTheme, 0, sizeof(tTheme));
	tTheme.iSize = sizeof(tTheme);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 240.0f, 120.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "input viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "font", 4, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiGetTheme(pContext, &tTheme);
	XUI_TEST_CHECK(iRet == XUI_OK, "theme get");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	iRet = xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 120.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "root rect");
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "root set");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Open document";
	tDesc.pFont = pFont;
	tDesc.iTextColor = XUI_COLOR_RGBA(20, 92, 210, 255);
	tDesc.iHoverTextColor = XUI_COLOR_RGBA(8, 128, 230, 255);
	tDesc.iActiveTextColor = XUI_COLOR_RGBA(6, 60, 160, 255);
	tDesc.iDisabledTextColor = XUI_COLOR_RGBA(120, 130, 140, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	tDesc.bHoverUnderline = 1;
	tDesc.bActiveUnderline = 1;
	tDesc.fLineGap = INFINITY;
	pProbe = pRoot;
	iRet = xuiHyperlinkCreate(pContext, &pProbe, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pProbe == NULL,
		"invalid create clears output");
	tDesc.fLineGap = 0.0f;
	iRet = xuiHyperlinkCreate(pContext, &pLink, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLink != NULL, "hyperlink create");
	XUI_TEST_CHECK(strcmp(xuiHyperlinkGetText(pLink), "Open document") == 0, "text getter");
	XUI_TEST_CHECK(xuiHyperlinkGetTextColor(pLink) == XUI_COLOR_RGBA(20, 92, 210, 255), "text color getter");
	XUI_TEST_CHECK(xuiHyperlinkGetHoverUnderline(pLink) == 1 && xuiHyperlinkGetActiveUnderline(pLink) == 1, "underline defaults");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateCount(pLink) == 6, "six cache states");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 0) == 0, "normal cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 1) == XUI_WIDGET_STATE_HOVER, "hover cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 2) == XUI_WIDGET_STATE_ACTIVE, "active cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 3) == XUI_WIDGET_STATE_FOCUS, "focus cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 4) ==
		(XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS), "hover focus cache id");
	XUI_TEST_CHECK(xuiWidgetGetCacheStateId(pLink, 5) ==
		(XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS), "active focus cache id");
	XUI_TEST_CHECK(__xuiHyperlinkSetTextOomPreserves(pLink), "text OOM preserves value");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pLink) == 1, "accessible node count");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pLink, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_LINK &&
		strcmp(tAccessible.sName, "Open document") == 0 &&
		(tAccessible.iActions & XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_ACTIVATE)) != 0,
		"accessible link node");
	XUI_TEST_CHECK(xuiWidgetSetAccessibleName(pLink, "Open the current document") == XUI_OK,
		"accessible name set");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pLink, 0, &tAccessible) == XUI_OK &&
		strcmp(tAccessible.sName, "Open the current document") == 0,
		"explicit accessible name");
	XUI_TEST_CHECK(xuiWidgetSetAccessibleName(pLink, NULL) == XUI_OK, "accessible name clear");
	iRet = xuiHyperlinkSetClick(pLink, __xuiHyperlinkClicked, &iClickCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "click callback");
	iRet = xuiWidgetSetRect(pLink, (xui_rect_t){20, 20, 160, 25});
	XUI_TEST_CHECK(iRet == XUI_OK, "link rect");
	iRet = xuiWidgetAddChild(pRoot, pLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "add link");

	iRet = xuiWidgetMeasureContent(pLink, (xui_vec2_t){300.0f, 100.0f}, &tMeasure);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasure.fX > 20.0f && tMeasure.fY == 16.0f, "measure content");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	pHoverCache = xuiWidgetGetCacheSurface(pLink, XUI_WIDGET_STATE_HOVER);
	pActiveCache = xuiWidgetGetCacheSurface(pLink, XUI_WIDGET_STATE_ACTIVE);
	pFocusCache = xuiWidgetGetCacheSurface(pLink, XUI_WIDGET_STATE_FOCUS);
	pHoverFocusCache = xuiWidgetGetCacheSurface(pLink,
		XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS);
	pActiveFocusCache = xuiWidgetGetCacheSurface(pLink,
		XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS);
	XUI_TEST_CHECK(pNormalCache != NULL && pHoverCache != NULL && pActiveCache != NULL &&
		pFocusCache != NULL && pHoverFocusCache != NULL && pActiveFocusCache != NULL,
		"cache surfaces");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(20, 92, 210, 255), "normal color cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pHoverCache) == XUI_COLOR_RGBA(8, 128, 230, 255), "hover color cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pActiveCache) == XUI_COLOR_RGBA(6, 60, 160, 255), "active color cache");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastColor(pFocusCache) == tTheme.iStateFocusColor,
		"focus cache outline");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pHoverFocusCache) ==
		XUI_COLOR_RGBA(8, 128, 230, 255) &&
		xuiTestSurfaceGetLastColor(pHoverFocusCache) == tTheme.iStateFocusColor,
		"hover focus cache keeps text state and outline");
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pActiveFocusCache) ==
		XUI_COLOR_RGBA(6, 60, 160, 255) &&
		xuiTestSurfaceGetLastColor(pActiveFocusCache) == tTheme.iStateFocusColor,
		"active focus cache keeps text state and outline");
	tTextRect = xuiTestSurfaceGetLastRect(pFocusCache);
	XUI_TEST_CHECK(tTextRect.fX == 0 && tTextRect.fY == 0 &&
		tTextRect.fW == 160 && tTextRect.fH == 25,
		"focus outline uses symmetric content bounds");
	tTextRect = xuiTestSurfaceGetLastTextRect(pNormalCache);
	XUI_TEST_CHECK(tTextRect.fY == 4 && tTextRect.fH == 17,
		"fractional centered line keeps final pixel row");
	XUI_TEST_CHECK(tState.iLastTextDrawSize >= 0, "bounded display text draw");
	xuiWidgetClearDirty(pLink, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiHyperlinkSetTextFlags(pLink, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE |
		XUI_TEXT_CLIP | XUI_TEXT_RTL);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pLink) & XUI_WIDGET_DIRTY_LAYOUT) != 0,
		"rtl flags invalidate layout");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "rtl render");
	XUI_TEST_CHECK((tState.iLastTextShapeFlags & XUI_TEXT_SHAPE_RTL) != 0 &&
	               (tState.iLastTextDrawItemFlags & XUI_TEXT_SHAPE_RTL) != 0,
	               "rtl shaping flags");

	iRet = xuiInputPointerMove(pContext, 30.0f, 30.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_HOVER) != 0, "hover state");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pLink) == XUI_WIDGET_STATE_HOVER, "hover visual state");
	iRet = xuiInputPointerDown(pContext, 30.0f, 30.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch down");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0, "active state");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pLink) ==
		(XUI_WIDGET_STATE_ACTIVE | XUI_WIDGET_STATE_FOCUS),
		"focused active visual state");
	iRet = xuiInputPointerUp(pContext, 30.0f, 30.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch up");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 1 && iClickCount == 1, "pointer click");
	XUI_TEST_CHECK(xuiWidgetGetStateId(pLink) ==
		(XUI_WIDGET_STATE_HOVER | XUI_WIDGET_STATE_FOCUS),
		"focused hover visual state");
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		30, 30, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pLink,
		"first touch owns hyperlink press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		30, 30, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, 30, 30, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiHyperlinkGetClickCount(pLink) == 1 &&
		iClickCount == 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pLink &&
		(xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot click or release hyperlink press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		30, 30, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiHyperlinkGetClickCount(pLink) == 2 &&
		iClickCount == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch completes one hyperlink click");

	iRet = xuiSetFocusWidget(pContext, pLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space down");
	XUI_TEST_CHECK((xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0, "keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space up");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 3 && iClickCount == 3, "keyboard click");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ENTER, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "enter default");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch enter default");
	XUI_TEST_CHECK(xuiHyperlinkGetClickCount(pLink) == 4 && iClickCount == 4, "enter default click");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down before blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) != 0,
	               "active before blur");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during keyboard press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiHyperlinkGetState(pLink) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up after blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiHyperlinkGetClickCount(pLink) == 4 && iClickCount == 4,
	               "blurred keyboard press does not click");
	iRet = xuiWidgetPerformAccessibleAction(pLink, 1u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiHyperlinkGetClickCount(pLink) == 5 && iClickCount == 5,
		"accessible activate click");
	iRet = xuiHyperlinkSetLineGap(pLink, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan line gap");
	iRet = xuiHyperlinkSetParagraphGap(pLink, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite paragraph gap");

	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable parent");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "disabled render");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(120, 130, 140, 255),
		"inherited disabled color");
	XUI_TEST_CHECK(xuiQueryCursor(pContext, 30, 30) == XUI_CURSOR_ARROW,
		"disabled subtree does not claim cursor");
	iRet = xuiWidgetPerformAccessibleAction(pLink, 1u, XUI_ACCESSIBLE_ACTION_ACTIVATE, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && iClickCount == 5,
		"inherited disabled action rejected");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable parent");

	arrProps[0] = __xuiHyperlinkTestProp("hyperlink.text.color", __xuiHyperlinkTestColor(XUI_COLOR_RGBA(30, 170, 100, 255)));
	tStyle = __xuiHyperlinkTestStyle(arrProps, 1);
	iRet = xuiStyleSetClass(pContext, "green-link", &tStyle);
	XUI_TEST_CHECK(iRet == XUI_OK, "style set");
	iRet = xuiWidgetAddStyleClass(pLink, "green-link");
	XUI_TEST_CHECK(iRet == XUI_OK, "style add");
	iRet = xuiInputPointerMove(pContext, 220.0f, 90.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move out");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch move out");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "style render");
	pNormalCache = xuiWidgetGetCacheSurface(pLink, 0);
	XUI_TEST_CHECK(xuiTestSurfaceGetLastTextColor(pNormalCache) == XUI_COLOR_RGBA(30, 170, 100, 255), "style color");

	tDestroyState.ppWidget = &pDestroyLink;
	tDesc.sText = "Destroying link";
	iRet = xuiHyperlinkCreate(pContext, &pDestroyLink, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyLink != NULL, "destroying link create");
	iRet = xuiWidgetSetRect(pDestroyLink, (xui_rect_t){20.0f, 60.0f, 160.0f, 24.0f});
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link rect");
	iRet = xuiWidgetAddChild(pRoot, pDestroyLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link add");
	iRet = xuiHyperlinkSetClick(pDestroyLink, __xuiHyperlinkDestroyClicked, &tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying callback set");
	iRet = xuiSetFocusWidget(pContext, pDestroyLink);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link focus");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link space down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link dispatch down");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroying link space up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 1 && pDestroyLink == NULL,
		"keyboard callback may destroy hyperlink");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "context usable after callback destruction");

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
	printf("xui_hyperlink_test passed\n");
	return 0;
}
