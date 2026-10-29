#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_scrollbar_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiScrollBarNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.01f;
}

static xui_style_value_t __xuiScrollBarStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiScrollBarStyleProp(const char* sName,
	xui_style_value_t tValue)
{
	xui_style_property_t tProperty;

	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	tProperty.sName = sName;
	tProperty.tValue = tValue;
	return tProperty;
}

static void __xuiScrollBarChanged(xui_widget pWidget, float fValue, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)fValue;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_scrollbar_destroy_state_t {
	xui_widget* ppWidget;
	int iCount;
} xui_scrollbar_destroy_state_t;

static void __xuiScrollBarDestroyOnChange(xui_widget pWidget, float fValue, void* pUser)
{
	xui_scrollbar_destroy_state_t* pState = (xui_scrollbar_destroy_state_t*)pUser;

	(void)fValue;
	if ( pState != NULL ) {
		pState->iCount++;
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiScrollBarClick(xui_context pContext, float fX, float fY)
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

static int __xuiScrollBarRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 240, 220};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiScrollBarUpdateFrame(xui_context pContext, float fDelta)
{
	int iRet;

	iRet = xuiUpdate(pContext, fDelta);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pBar;
	xui_widget pChild;
	xui_widget pInvalid;
	xui_widget pDestroyBar;
	xui_surface pTarget;
	xui_surface pCache;
	xui_scrollbar_desc_t tDesc;
	xui_style_property_t tStyle;
	xui_accessible_node_t tAccessible;
	xui_event_t tEvent;
	xui_scrollbar_destroy_state_t tDestroy;
	xui_rect_t tTrack;
	xui_rect_t tThumb;
	xui_rect_t tDecrease;
	xui_rect_t tIncrease;
	float fMin;
	float fMax;
	float fPage;
	float fStartX;
	float fStartY;
	float fThickness;
	float fMinThumbSize;
	float fButtonSize;
	float fHoldValue;
	int iHoldChanged;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pBar = NULL;
	pChild = NULL;
	pInvalid = NULL;
	pDestroyBar = NULL;
	pTarget = NULL;
	pCache = NULL;
	iChanged = 0;
	iFailed = 0;
	memset(&tDestroy, 0, sizeof(tDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 240.0f, 220.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 240.0f, 220.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fValue = 50.0f;
	tDesc.fPage = 25.0f;
	tDesc.fSmallStep = 5.0f;
	tDesc.fLargeStep = 25.0f;
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iMode = XUI_SCROLLBAR_MODE_FULL;
	tDesc.iButtonMode = XUI_SCROLLBAR_BUTTONS_ON;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollBarCreate(NULL, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");
	iRet = xuiScrollBarCreate(pContext, &pBar, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pBar != NULL, "scrollbar create");
	tDesc.fValue = NAN;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollBarCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nan descriptor");
	tDesc.fValue = 50.0f;
	tDesc.fThickness = FLT_MAX;
	pInvalid = (xui_widget)(void*)1;
	iRet = xuiScrollBarCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject out-of-layout-range descriptor metric");
	tDesc.fThickness = 0.0f;
	iRet = xuiWidgetAddChild(pRoot, pBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "add scrollbar");
	xuiWidgetSetRect(pBar, (xui_rect_t){10.0f, 10.0f, 20.0f, 140.0f});
	iRet = xuiScrollBarSetChange(pBar, __xuiScrollBarChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pBar) == 1, "focusable");
	XUI_TEST_CHECK(xuiWidgetGetOverflow(pBar) == XUI_OVERFLOW_CLIP, "overflow clip");
	XUI_TEST_CHECK(xuiScrollBarGetMode(pBar) == XUI_SCROLLBAR_MODE_FULL, "full mode");
	XUI_TEST_CHECK(xuiScrollBarGetButtonMode(pBar) == XUI_SCROLLBAR_BUTTONS_ON, "button mode");
	XUI_TEST_CHECK(xuiScrollBarGetOrientation(pBar) == XUI_ORIENTATION_VERTICAL, "vertical orientation");
	iRet = xuiScrollBarGetMetrics(pBar, &fThickness, &fMinThumbSize, &fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fThickness == 0.0f && fMinThumbSize == 18.0f && fButtonSize == 0.0f, "default metrics");
	iRet = xuiScrollBarSetValue(pBar, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiScrollBarGetValue(pBar) == 50.0f, "reject nan value and preserve state");
	iRet = xuiScrollBarSetRange(pBar, 0.0f, INFINITY, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite range");
	iRet = xuiScrollBarSetPage(pBar, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite page");
	iRet = xuiScrollBarSetSteps(pBar, NAN, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan steps");
	iRet = xuiScrollBarSetMetrics(pBar, 0.0f, INFINITY, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metrics");
	iRet = xuiScrollBarSetMetrics(pBar, 0.0f, FLT_MAX, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		"reject out-of-layout-range metrics");
	iRet = xuiWidgetSetAccessibleName(pBar, "Document scrollbar");
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetAccessibleDescription(pBar, "Moves through the document");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	if ( iRet == XUI_OK ) iRet = xuiWidgetGetAccessibleNode(pBar, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.sName != NULL && tAccessible.sDescription != NULL &&
	               strcmp(tAccessible.sName, "Document scrollbar") == 0 &&
	               strcmp(tAccessible.sDescription, "Moves through the document") == 0,
	               "explicit accessible metadata");

	iRet = xuiWidgetCreate(pContext, &pChild);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pBar, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "scrollbar child create");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pChild;
	tEvent.fX = 20;
	tEvent.fY = 20;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == 50.0f &&
	               xuiGetPointerCapture(pContext) == NULL,
	               "descendant pointer does not activate scrollbar");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_WHEEL;
	tEvent.pTarget = pChild;
	tEvent.fWheelY = 1.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == 50.0f,
	               "descendant wheel does not move scrollbar");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pChild;
	tEvent.iKey = XUI_KEY_RIGHT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == 50.0f,
	               "descendant key does not move scrollbar");
	xuiWidgetDestroy(pChild);
	pChild = NULL;

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 240, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	XUI_TEST_CHECK(xuiQueryCursor(pContext, 20, 20) == XUI_CURSOR_ARROW,
		"scrollbar uses arrow cursor");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render full scrollbar");
	tTrack = xuiScrollBarGetTrackRect(pBar);
	tThumb = xuiScrollBarGetThumbRect(pBar);
	tDecrease = xuiScrollBarGetDecreaseRect(pBar);
	tIncrease = xuiScrollBarGetIncreaseRect(pBar);
	XUI_TEST_CHECK(tDecrease.fH > 0.0f && tIncrease.fH > 0.0f, "full buttons exist");
	XUI_TEST_CHECK(tTrack.fY >= tDecrease.fH && tTrack.fH > tThumb.fH, "track geometry");
	XUI_TEST_CHECK(tThumb.fW >= 10.0f && tThumb.fW < 20.0f && tThumb.fH >= 18.0f, "full thumb visual geometry");
	tStyle = __xuiScrollBarStyleProp("scrollbar.thickness",
		__xuiScrollBarStyleFloat(FLT_MAX));
	iRet = xuiWidgetSetInlineStyle(pBar, &tStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set out-of-layout-range style metric");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	XUI_TEST_CHECK(tThumb.fW < 20.0f,
		"out-of-layout-range style metric is ignored");
	iRet = xuiWidgetSetInlineStyle(pBar, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid style metric");
	iRet = xuiScrollBarSetPage(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set zero page for style default test");
	tStyle = __xuiScrollBarStyleProp("scrollbar.min_thumb_size",
		__xuiScrollBarStyleFloat(0.0f));
	iRet = xuiWidgetSetInlineStyle(pBar, &tStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set automatic minimum thumb style");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	XUI_TEST_CHECK(tThumb.fH >= 18.0f,
		"zero minimum thumb style uses the documented default");
	iRet = xuiWidgetSetInlineStyle(pBar, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarSetPage(pBar, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore page and inline style");
	xuiWidgetSetRect(pBar, (xui_rect_t){10.0f, 10.0f, 20.0f, 10.0f});
	tTrack = xuiScrollBarGetTrackRect(pBar);
	tThumb = xuiScrollBarGetThumbRect(pBar);
	tDecrease = xuiScrollBarGetDecreaseRect(pBar);
	tIncrease = xuiScrollBarGetIncreaseRect(pBar);
	XUI_TEST_CHECK(tDecrease.fH == 5.0f && tIncrease.fY == 5.0f && tIncrease.fH == 5.0f &&
	               tTrack.fH == 0.0f && tThumb.fW == 0.0f && tThumb.fH == 0.0f,
	               "forced buttons stay bounded in a tiny scrollbar");
	xuiWidgetSetRect(pBar, (xui_rect_t){10.0f, 10.0f, 20.0f, 140.0f});
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render restored full scrollbar");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	pCache = xuiWidgetGetCacheSurface(pBar, xuiWidgetGetStateId(pBar));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillCount(pCache) > 0, "normal cache surface");
	iRet = xuiInputPointerMove(pContext, 10.0f + tThumb.fX + tThumb.fW * 0.5f, 10.0f + tThumb.fY + tThumb.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hover thumb input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetHoverPart(pBar) == XUI_SCROLLBAR_PART_THUMB, "hover thumb part");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render hover scrollbar");
	pCache = xuiWidgetGetCacheSurface(pBar, xuiWidgetGetStateId(pBar));
	XUI_TEST_CHECK(pCache != NULL && xuiTestSurfaceGetRectFillCount(pCache) > 0, "hover part cache surface");
	iRet = xuiInputPointerMove(pContext, 200.0f, 200.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "leave hover input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "leave hover dispatch");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable scrollbar ancestor");
	XUI_TEST_CHECK((xuiScrollBarGetState(pBar) & XUI_WIDGET_STATE_DISABLED) != 0,
		"ancestor disable is reflected in scrollbar state");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore scrollbar ancestor");

	iChanged = 0;
	iRet = __xuiScrollBarClick(pContext, 20.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "decrease click dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 45.0f) && iChanged == 1, "decrease click step");
	iRet = __xuiScrollBarClick(pContext, 20.0f, 140.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "increase click dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f) && iChanged == 2, "increase click step");
	iRet = __xuiScrollBarClick(pContext, 20.0f, 112.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "track click dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 75.0f) && iChanged == 3, "track click page");
	iRet = xuiInputPointerWheel(pContext, 20.0f, 80.0f, 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 70.0f) && iChanged == 4, "wheel small step");

	iRet = xuiScrollBarSetValue(pBar, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set button repeat value");
	iChanged = 0;
	iRet = xuiInputPointerMove(pContext, 20.0f, 140.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat button hover input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat button hover dispatch");
	iRet = xuiInputPointerDown(pContext, 20.0f, 140.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat button down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar, "repeat button capture");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 55.0f) && iChanged == 1, "repeat button immediate step");
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.20f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 55.0f) && iChanged == 1, "repeat button waits initial delay");
	iRet = xuiInputPointerMove(pContext, 200.0f, 200.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "move held pointer outside repeat button");
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.20f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 55.0f) &&
		iChanged == 1, "repeat button pauses while pointer is outside");
	iRet = xuiInputPointerMove(pContext, 20.0f, 140.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "move held pointer back over repeat button");
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.10f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) > 55.0f && iChanged > 1,
		"repeat button resumes after pointer returns");
	fHoldValue = xuiScrollBarGetValue(pBar);
	iHoldChanged = iChanged;
	iRet = xuiInputPointerUp(pContext, 20.0f, 140.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat button up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "repeat button release");
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.60f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), fHoldValue) && iChanged == iHoldChanged, "repeat button stops on release");

	iRet = xuiScrollBarSetValue(pBar, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set track repeat value");
	iChanged = 0;
	iRet = xuiInputPointerMove(pContext, 20.0f, 112.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat track hover input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat track hover dispatch");
	iRet = xuiInputPointerDown(pContext, 20.0f, 112.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat track down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_TRACK, "repeat track active");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 75.0f) && iChanged == 1, "repeat track immediate page");
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.40f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 100.0f) && iChanged == 2, "repeat track reaches pointer");
	iHoldChanged = iChanged;
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.60f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 100.0f) && iChanged == iHoldChanged, "repeat track stops at pointer");
	iRet = xuiInputPointerUp(pContext, 20.0f, 112.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat track up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "repeat track release");

	iRet = xuiScrollBarSetValue(pBar, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset after repeat tests");

	iRet = xuiSetFocusWidget(pContext, pBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus scrollbar");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_PAGE_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 75.0f), "page down value");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_PAGE_UP, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "page up dispatch");
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f), "page up value");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "home input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 0.0f), "home value");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "end input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 100.0f), "end value");

	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set drag value");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before drag");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 10.0f + tThumb.fY + tThumb.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar && xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_THUMB, "drag capture");
	iRet = xuiInputPointerMove(pContext, fStartX, fStartY + 40.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f), "drag value");
	iRet = xuiInputPointerUp(pContext, fStartX, fStartY + 40.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_NONE, "drag release");

	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set pointer-up sample value");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 10.0f + tThumb.fY + tThumb.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar,
		"pointer-up sample drag capture");
	iRet = xuiInputPointerUp(pContext, fStartX, fStartY + 40.0f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f) &&
		xuiGetPointerCapture(pContext) == NULL,
		"pointer-up position supplies the final drag sample");

	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set multi-pointer value");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before multi-pointer drag");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 10.0f + tThumb.fY + tThumb.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH, (int)fStartX,
		(int)fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pBar,
		"first touch owns scrollbar drag");
	iRet = xuiInputPointerDownEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH, (int)fStartX,
		(int)fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH,
		(int)fStartX, (int)(fStartY + 70.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH,
		(int)fStartX, (int)(fStartY + 70.0f), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 0.0f) &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pBar &&
		xuiGetPointerCaptureEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move or release scrollbar drag");
	iRet = xuiInputPointerMoveEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)fStartX, (int)(fStartY + 40.0f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f),
		"owning touch moves scrollbar thumb");
	iRet = xuiInputPointerUpEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)fStartX, (int)(fStartY + 40.0f), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch releases scrollbar drag");
	iRet = xuiInputPointerDownEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH, (int)fStartX,
		(int)fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarSetMetrics(pBar, 12.0f, 18.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"geometry change cancels exact scrollbar capture");
	iRet = xuiScrollBarSetMetrics(pBar, 0.0f, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiScrollBarGetMetrics(pBar,
		&fThickness, &fMinThumbSize, &fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fThickness == 0.0f &&
		fMinThumbSize == 18.0f && fButtonSize == 0.0f,
		"zero metrics restore mode-aware automatic defaults");
	(void)xuiInputPointerUpEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		(int)fStartX, (int)fStartY, XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set escape drag value");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 10.0f + tThumb.fY + tThumb.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar, "escape drag capture");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_NONE, "escape cancels drag");
	(void)xuiInputPointerUp(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSetFocusWidget(pContext, pBar);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus before blur drag");
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "blur drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar, "blur drag capture");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during scrollbar drag");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_NONE,
	               "blur cancels scrollbar drag capture");
	(void)xuiInputPointerUp(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiScrollBarSetMode(pBar, XUI_SCROLLBAR_MODE_COMPACT);
	XUI_TEST_CHECK(iRet == XUI_OK, "set compact mode");
	iRet = xuiScrollBarSetButtonMode(pBar, XUI_SCROLLBAR_BUTTONS_OFF);
	XUI_TEST_CHECK(iRet == XUI_OK, "set buttons off");
	iRet = xuiScrollBarSetOrientation(pBar, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set horizontal");
	iRet = xuiScrollBarSetMetrics(pBar, 6.0f, 20.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set compact metrics");
	xuiWidgetSetRect(pBar, (xui_rect_t){10.0f, 166.0f, 180.0f, 12.0f});
	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset compact value");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render compact");
	tTrack = xuiScrollBarGetTrackRect(pBar);
	tThumb = xuiScrollBarGetThumbRect(pBar);
	tDecrease = xuiScrollBarGetDecreaseRect(pBar);
	tIncrease = xuiScrollBarGetIncreaseRect(pBar);
	XUI_TEST_CHECK(tDecrease.fW == 0.0f && tIncrease.fW == 0.0f, "compact hides buttons");
	XUI_TEST_CHECK(tTrack.fW == 180.0f && tThumb.fH <= 6.0f && tThumb.fW >= 20.0f, "compact geometry");
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 167.0f;
	XUI_TEST_CHECK((fStartY - 166.0f) < tThumb.fY,
		"compact drag starts in the visual thumb gutter");
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "compact drag down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_THUMB, "compact drag active");
	iRet = xuiInputPointerMove(pContext, fStartX + 72.0f, fStartY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "compact drag move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f), "compact drag value");
	iRet = xuiInputPointerUp(pContext, fStartX + 72.0f, fStartY, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "compact drag up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "compact release");
	iRet = xuiScrollBarSetValue(pBar, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set track boundary value");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	iRet = __xuiScrollBarClick(pContext, 10.0f + tThumb.fX + tThumb.fW,
		166.0f + tThumb.fY + tThumb.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiScrollBarNear(xuiScrollBarGetValue(pBar), 75.0f),
		"track starts immediately after the half-open thumb rect");

	iRet = xuiScrollBarSetRange(pBar, 100.0f, 0.0f, -10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize range");
	iRet = xuiScrollBarSetValue(pBar, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clamp value");
	iRet = xuiScrollBarGetRange(pBar, &fMin, &fMax, &fPage);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 100.0f && fPage == 0.0f && xuiScrollBarGetValue(pBar) == 100.0f, "range normalization");
	iRet = xuiScrollBarSetRange(pBar, -FLT_MAX, FLT_MAX, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite range");
	iRet = xuiScrollBarSetValue(pBar, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide range midpoint");
	tTrack = xuiScrollBarGetTrackRect(pBar);
	tThumb = xuiScrollBarGetThumbRect(pBar);
	XUI_TEST_CHECK(tThumb.fX >= tTrack.fX && tThumb.fY >= tTrack.fY &&
	               tThumb.fX + tThumb.fW <= tTrack.fX + tTrack.fW &&
	               tThumb.fY + tThumb.fH <= tTrack.fY + tTrack.fH,
	               "wide range thumb geometry remains finite");
	iRet = xuiScrollBarSetSteps(pBar, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite steps");
	iRet = xuiScrollBarSetValue(pBar, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide range maximum");
	iRet = xuiWidgetPerformAccessibleAction(pBar, 1,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == FLT_MAX,
		"accessible increment saturates at wide range maximum");
	iRet = xuiScrollBarSetRange(pBar, FLT_MAX, FLT_MAX, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize maximum equal range");
	iRet = xuiScrollBarGetRange(pBar, &fMin, &fMax, &fPage);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(fMin) && isfinite(fMax) && fMin < fMax, "maximum equal range remains finite");
	iRet = xuiScrollBarSetRange(pBar, 0.0f, 100.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary range");
	iRet = xuiScrollBarSetSteps(pBar, 5.0f, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary steps");
	iRet = xuiScrollBarSetValue(pBar, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary value");
	iRet = __xuiScrollBarRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before disable drag");
	tThumb = xuiScrollBarGetThumbRect(pBar);
	fStartX = 10.0f + tThumb.fX + tThumb.fW * 0.5f;
	fStartY = 166.0f + tThumb.fY + tThumb.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pBar, "disable drag capture");
	iRet = xuiWidgetSetEnabled(pBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               xuiScrollBarGetActivePart(pBar) == XUI_SCROLLBAR_PART_NONE,
	               "disable cancels scrollbar drag capture");
	XUI_TEST_CHECK((xuiScrollBarGetState(pBar) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiScrollBarClick(pContext, 20.0f, 172.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");

	tDesc.fValue = 50.0f;
	tDesc.iOrientation = XUI_ORIENTATION_VERTICAL;
	tDesc.iMode = XUI_SCROLLBAR_MODE_FULL;
	tDesc.iButtonMode = XUI_SCROLLBAR_BUTTONS_ON;
	iRet = xuiScrollBarCreate(pContext, &pDestroyBar, &tDesc);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pRoot, pDestroyBar);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetRect(pDestroyBar, (xui_rect_t){50.0f, 10.0f, 20.0f, 140.0f});
	tDestroy.ppWidget = &pDestroyBar;
	if ( iRet == XUI_OK ) iRet = xuiScrollBarSetChange(pDestroyBar,
		__xuiScrollBarDestroyOnChange, &tDestroy);
	if ( iRet == XUI_OK ) iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy-on-change scrollbar prepare");
	iRet = __xuiScrollBarClick(pContext, 60.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyBar == NULL && tDestroy.iCount == 1,
		"change callback may destroy scrollbar without stale access");

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
	printf("xui_scrollbar_test passed\n");
	return 0;
}
