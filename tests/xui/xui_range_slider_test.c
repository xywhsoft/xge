#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_range_slider_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiRangeSliderNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.02f;
}

static int __xuiRangeSliderNearWithin(float fA, float fB, float fTolerance)
{
	return fabsf(fA - fB) <= fTolerance;
}

static int __xuiRangeSliderPixel(float fValue)
{
	return (int)floorf(fValue + 0.5f);
}

static xui_style_value_t __xuiRangeSliderStyleColor(uint32_t iColor)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_COLOR;
	tValue.iColor = iColor;
	return tValue;
}

static xui_style_value_t __xuiRangeSliderStyleFloat(float fValue)
{
	xui_style_value_t tValue;

	memset(&tValue, 0, sizeof(tValue));
	tValue.iSize = sizeof(tValue);
	tValue.iType = XUI_STYLE_VALUE_FLOAT;
	tValue.fFloat = fValue;
	return tValue;
}

static xui_style_property_t __xuiRangeSliderStyleProp(const char* sName, xui_style_value_t tValue)
{
	xui_style_property_t tProp;

	memset(&tProp, 0, sizeof(tProp));
	tProp.iSize = sizeof(tProp);
	tProp.sName = sName;
	tProp.tValue = tValue;
	return tProp;
}

static void __xuiRangeSliderChanged(xui_widget pWidget, float fStart, float fEnd, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)fStart;
	(void)fEnd;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_range_slider_destroy_test_t {
	int iCount;
	int iDestroyAt;
	xui_widget* ppWidget;
} xui_range_slider_destroy_test_t;

typedef struct xui_range_slider_accessibility_test_t {
	xui_widget pWidget;
	int iBoundsEvents;
	int iValueEvents;
} xui_range_slider_accessibility_test_t;

static void __xuiRangeSliderAccessibilityEvent(xui_context pContext,
	xui_widget pWidget, const xui_accessibility_event_t* pEvent, void* pUser)
{
	xui_range_slider_accessibility_test_t* pState =
		(xui_range_slider_accessibility_test_t*)pUser;

	(void)pContext;
	if ( pState == NULL || pEvent == NULL || pWidget != pState->pWidget ) return;
	if ( pEvent->iType == XUI_ACCESSIBLE_EVENT_BOUNDS_CHANGED ) pState->iBoundsEvents++;
	if ( pEvent->iType == XUI_ACCESSIBLE_EVENT_VALUE_CHANGED ) pState->iValueEvents++;
}

static void __xuiRangeSliderDestroyOnChange(xui_widget pWidget, float fStart,
	float fEnd, void* pUser)
{
	xui_range_slider_destroy_test_t* pState = (xui_range_slider_destroy_test_t*)pUser;

	(void)fStart;
	(void)fEnd;
	if ( pState == NULL ) return;
	pState->iCount++;
	if ( pState->iCount == pState->iDestroyAt ) {
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiRangeSliderRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 280, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiRangeSliderClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, __xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY), 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, __xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, __xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pSlider;
	xui_widget pChild;
	xui_widget pInvalid;
	xui_widget pDestroySlider;
	xui_surface pTarget;
	xui_range_slider_desc_t tDesc;
	xui_style_property_t arrStyle[2];
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tStartKnob;
	xui_rect_t tEndKnob;
	xui_rect_t tWorld;
	xui_accessible_node_t tAccessible;
	xui_accessible_value_t tAccessibleValue;
	xui_event_t tEvent;
	xui_range_slider_destroy_test_t tDestroyState;
	xui_range_slider_accessibility_test_t tAccessibilityState;
	float fMin;
	float fMax;
	float fStart;
	float fEnd;
	float fMaxInterval;
	float fMinInterval;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fExpected;
	float fX;
	float fY;
	float fOwnedStart;
	float fOwnedEnd;
	int iChangeCount;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pSlider = NULL;
	pChild = NULL;
	pInvalid = NULL;
	pDestroySlider = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	memset(&tDestroyState, 0, sizeof(tDestroyState));
	memset(&tAccessibilityState, 0, sizeof(tAccessibilityState));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 280.0f, 260.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 280.0f, 260.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fStart = 25.0f;
	tDesc.fEnd = 75.0f;
	tDesc.fStep = 5.0f;
	tDesc.fPageStep = 20.0f;
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	iRet = xuiRangeSliderCreate(pContext, &pSlider, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSlider != NULL, "range slider create");
	tDesc.fStart = NAN;
	pInvalid = pRoot;
	iRet = xuiRangeSliderCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nan descriptor");
	tDesc.fStart = 25.0f;
	tDesc.fKnobSize = FLT_MAX;
	pInvalid = pRoot;
	iRet = xuiRangeSliderCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject out-of-layout-range descriptor metric");
	tDesc.fKnobSize = 0.0f;
	iRet = xuiWidgetAddChild(pRoot, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "add range slider");
	tAccessibilityState.pWidget = pSlider;
	iRet = xuiSetAccessibilityEventCallback(pContext,
		__xuiRangeSliderAccessibilityEvent, &tAccessibilityState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set range slider accessibility callback");
	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 180.0f, 30.0f});
	iRet = xuiRangeSliderSetChange(pSlider, __xuiRangeSliderChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pSlider) == 1, "focusable");
	XUI_TEST_CHECK(xuiWidgetGetOverflow(pSlider) == XUI_OVERFLOW_CLIP, "overflow clip");
	XUI_TEST_CHECK(xuiRangeSliderGetOrientation(pSlider) == XUI_ORIENTATION_HORIZONTAL, "horizontal orientation");
	XUI_TEST_CHECK(__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 25.0f), "initial start");
	XUI_TEST_CHECK(__xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 75.0f), "initial end");
	XUI_TEST_CHECK(__xuiRangeSliderNear(xuiRangeSliderGetStartRate(pSlider), 0.25f), "initial start rate");
	XUI_TEST_CHECK(__xuiRangeSliderNear(xuiRangeSliderGetEndRate(pSlider), 0.75f), "initial end rate");
	XUI_TEST_CHECK(xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START,
		"keyboard starts at first thumb");
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pSlider) == 3,
		"range slider accessibility node count");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		strcmp(tAccessible.sValue, "25 - 75") == 0 && tAccessible.iColumnCount == 2,
		"range slider accessibility root");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 2, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_SLIDER &&
		strcmp(tAccessible.sName, "End") == 0 && tAccessible.tRange.fValue == 75.0,
		"range slider end accessibility node");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_NUMBER;
	tAccessibleValue.fNumber = 80.0;
	iRet = xuiWidgetPerformAccessibleAction(pSlider, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetEnd(pSlider) == 80.0f &&
		xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_END,
		"range slider accessible end value");
	iRet = xuiRangeSliderSetValues(pSlider, 25.0f, 75.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore values after accessibility action");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 2u,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START,
		"focus accessible start thumb");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "tab input between thumbs");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_END,
		"tab advances to end thumb");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_TAB, XUI_MOD_SHIFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "shift tab input between thumbs");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START,
		"shift tab returns to start thumb");

	iRet = xuiWidgetCreate(pContext, &pChild);
	if ( iRet == XUI_OK ) iRet = xuiWidgetAddChild(pSlider, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "range slider child create");
	fOwnedStart = xuiRangeSliderGetStart(pSlider);
	fOwnedEnd = xuiRangeSliderGetEnd(pSlider);
	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pChild;
	tEvent.fX = 20.0f;
	tEvent.fY = 20.0f;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == fOwnedStart &&
		xuiRangeSliderGetEnd(pSlider) == fOwnedEnd &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount &&
		xuiGetPointerCapture(pContext) == NULL,
		"descendant pointer does not activate range slider");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_WHEEL;
	tEvent.pTarget = pChild;
	tEvent.fWheelY = 1.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == fOwnedStart &&
		xuiRangeSliderGetEnd(pSlider) == fOwnedEnd &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount,
		"descendant wheel does not move range slider");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pChild;
	tEvent.iKey = XUI_KEY_RIGHT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == fOwnedStart &&
		xuiRangeSliderGetEnd(pSlider) == fOwnedEnd &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount,
		"descendant key does not move range slider");
	xuiWidgetDestroy(pChild);
	pChild = NULL;

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 280, 260, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render horizontal range slider");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tFill = xuiRangeSliderGetFillRect(pSlider);
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tEndKnob = xuiRangeSliderGetEndKnobRect(pSlider);
	XUI_TEST_CHECK(tTrack.fW > 140.0f && tTrack.fH >= 4.0f, "track geometry");
	XUI_TEST_CHECK(tFill.fX > tTrack.fX && tFill.fW > 0.0f && tFill.fX + tFill.fW < tTrack.fX + tTrack.fW, "fill geometry");
	XUI_TEST_CHECK(tStartKnob.fX < tEndKnob.fX && tStartKnob.fW > 10.0f && tStartKnob.fH == tStartKnob.fW, "knob geometry");

	tAccessibilityState.iBoundsEvents = 0;
	tAccessibilityState.iValueEvents = 0;
	iRet = xuiRangeSliderSetValues(pSlider, 50.0f, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap thumbs");
	iRet = xuiUpdate(pContext, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessibilityState.iBoundsEvents == 1 &&
		tAccessibilityState.iValueEvents == 1,
		"range value change reports virtual thumb bounds");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 3u,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "activate overlapping end thumb");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tTrack.fX + tTrack.fW * 0.5f;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_END,
		"overlap hit keeps active end thumb");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.8f,
		fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "overlap drag end input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 50.0f) &&
		xuiRangeSliderGetEnd(pSlider) > 75.0f, "overlap drag moves active end only");
	(void)xuiInputPointerUp(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.8f,
		fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);
	iRet = xuiRangeSliderSetValues(pSlider, 25.0f, 75.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore values after overlap test");

	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiRangeSliderGetMetrics(pSlider, &fTrackSize, &fKnobSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fTrackSize == 6.0f && fKnobSize == 18.0f, "get metrics");
	iRet = xuiRangeSliderSetColors(pSlider, XUI_COLOR_RGBA(225, 238, 234, 255), XUI_COLOR_RGBA(0, 184, 169, 255), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(0, 184, 169, 160), XUI_COLOR_RGBA(160, 178, 172, 150));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiRangeSliderSetKnobBorderColor(pSlider, XUI_COLOR_RGBA(0, 150, 136, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetKnobBorderColor(pSlider) == XUI_COLOR_RGBA(0, 150, 136, 255), "knob border");
	iRet = xuiRangeSliderSetStep(pSlider, 5.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set step");
	iRet = xuiRangeSliderGetStep(pSlider, &fStep, &fPageStep);
	XUI_TEST_CHECK(iRet == XUI_OK && fStep == 5.0f && fPageStep == 20.0f, "get step");
	iRet = xuiRangeSliderSetStep(pSlider, NAN, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan step");
	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metrics");
	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject out-of-layout-range metrics");
	iRet = xuiRangeSliderGetMetrics(pSlider, &fTrackSize, &fKnobSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fTrackSize == 6.0f && fKnobSize == 18.0f,
		"invalid metrics preserve state");
	iRet = xuiRangeSliderSetValues(pSlider, NAN, 75.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 25.0f), "reject nan values and preserve state");
	iRet = xuiRangeSliderSetRange(pSlider, 0.0f, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite range");
	iRet = xuiRangeSliderSetIntervalLimits(pSlider, 0.0f, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite interval limit");
	iRet = xuiRangeSliderGetIntervalLimits(pSlider, &fMinInterval, &fMaxInterval);
	XUI_TEST_CHECK(iRet == XUI_OK && fMinInterval == 0.0f && fMaxInterval == 0.0f, "default interval limits");
	arrStyle[0] = __xuiRangeSliderStyleProp("rangeslider.track.size", __xuiRangeSliderStyleFloat(8.0f));
	arrStyle[1] = __xuiRangeSliderStyleProp("rangeslider.fill.color", __xuiRangeSliderStyleColor(XUI_COLOR_RGBA(40, 90, 160, 255)));
	iRet = xuiWidgetSetInlineStyle(pSlider, arrStyle, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	XUI_TEST_CHECK(tTrack.fH == 8.0f, "style track size");
	iRet = xuiWidgetSetInlineStyle(pSlider, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear inline style");
	arrStyle[0] = __xuiRangeSliderStyleProp("rangeslider.track.size",
		__xuiRangeSliderStyleFloat(FLT_MAX));
	iRet = xuiWidgetSetInlineStyle(pSlider, arrStyle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set out-of-layout-range style metric");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	XUI_TEST_CHECK(tTrack.fH == 6.0f,
		"out-of-layout-range style metric is ignored");
	iRet = xuiWidgetSetInlineStyle(pSlider, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear invalid inline style");

	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	iRet = xuiRangeSliderSetIntervalLimits(pSlider, 10.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 30.0f) &&
		__xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 70.0f) &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount + 1,
		"interval limits clamp values in one change");
	iRet = xuiRangeSliderGetIntervalLimits(pSlider, &fMinInterval, &fMaxInterval);
	XUI_TEST_CHECK(iRet == XUI_OK && fMinInterval == 10.0f && fMaxInterval == 40.0f, "get interval limits");
	xuiWidgetClearDirty(pSlider, XUI_WIDGET_DIRTY_ALL);
	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	iRet = xuiRangeSliderSetIntervalLimits(pSlider, 10.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetChangeCount(pSlider) == iChangeCount &&
		xuiWidgetGetDirtyFlags(pSlider) == 0u,
		"unchanged interval limits do not dirty range slider");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider) - xuiRangeSliderGetStart(pSlider), 10.0f), "minimum interval applied");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 30.0f) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 70.0f), "maximum interval applied");
	iRet = xuiRangeSliderSetIntervalLimits(pSlider, 30.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "invalid interval limits");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render interval limits");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tTrack.fX;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = __xuiRangeSliderClick(pContext, fX, fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 30.0f) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 70.0f), "maximum interval clamps start thumb");
	iRet = xuiRangeSliderSetIntervalLimits(pSlider, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear interval limits");

	iChanged = 0;
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset values");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before pointer");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + 1.0f;
	fY = tWorld.fY + tStartKnob.fY + tStartKnob.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, __xuiRangeSliderPixel(fX),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "edge grab pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 20.0f),
		"edge grab preserves horizontal thumb value");
	fExpected = 20.0f + 1200.0f / tTrack.fW;
	iRet = xuiInputPointerMove(pContext, __xuiRangeSliderPixel(fX) + 12,
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "edge grab pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNearWithin(
		xuiRangeSliderGetStart(pSlider), fExpected, 0.75f),
		"edge grab preserves horizontal pointer offset");
	iRet = xuiInputPointerUp(pContext, __xuiRangeSliderPixel(fX) + 12,
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "edge grab pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"edge grab releases pointer");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset values after edge grab");
	iChanged = 0;
	fX = tWorld.fX + tTrack.fX + tTrack.fW * 0.10f;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	fExpected = 100.0f * ((float)(__xuiRangeSliderPixel(fX) - tWorld.fX - tTrack.fX) / (float)tTrack.fW);
	iRet = __xuiRangeSliderClick(pContext, fX, fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), fExpected) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 80.0f), "nearest start pointer");
	XUI_TEST_CHECK(xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START && iChanged == 1, "active start");

	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before drag");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	iRet = xuiInputPointerDown(pContext, __xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider && (xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) != 0, "pointer capture");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW, __xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 80.0f) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 80.0f), "start clamps at end");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tTrack.fX + tTrack.fW, __xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pointer release");

	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set multi-pointer values");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before multi-pointer drag");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	fY = tWorld.fY + tStartKnob.fY + tStartKnob.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	fOwnedStart = xuiRangeSliderGetStart(pSlider);
	fOwnedEnd = xuiRangeSliderGetEnd(pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pSlider &&
		xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START,
		"first touch owns range slider drag");
	iRet = xuiInputPointerDownEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 302u,
		XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(tWorld.fX + tTrack.fX + tTrack.fW),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 302u,
		XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(tWorld.fX + tTrack.fX + tTrack.fW),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), fOwnedStart) &&
		__xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), fOwnedEnd) &&
		xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pSlider &&
		xuiGetPointerCaptureEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move or retarget range slider drag");
	iRet = xuiInputPointerMoveEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(tWorld.fX + tTrack.fX + tTrack.fW * 0.4f),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) > fOwnedStart &&
		__xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), fOwnedEnd),
		"owning touch moves active range slider thumb");
	iRet = xuiInputPointerUpEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(tWorld.fX + tTrack.fX + tTrack.fW * 0.4f),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch releases range slider drag");
	iRet = xuiInputPointerDownEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiRangeSliderSetMetrics(pSlider, 7.0f, 19.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		(xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
		"geometry change cancels exact range slider capture");
	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore metrics after multi-pointer drag");
	(void)xuiInputPointerUpEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		__xuiRangeSliderPixel(fX), __xuiRangeSliderPixel(fY),
		XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel reset");
	iChanged = 0;
	iRet = xuiInputPointerWheel(pContext, tWorld.fX + tTrack.fX + tTrack.fW, fY, 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 25.0f) && iChanged == 1, "wheel active start");

	iRet = xuiSetFocusWidget(pContext, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus range slider");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "key reset");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "right input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 25.0f), "right step active start");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "end input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 80.0f), "end clamps start to end");

	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before escape");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "escape capture");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && (xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0, "escape cancels drag");
	(void)xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSetFocusWidget(pContext, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus before blur drag");
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "blur drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "blur drag capture");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during range drag");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               (xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels range drag capture");
	(void)xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiRangeSliderSetOrientation(pSlider, XUI_ORIENTATION_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetOrientation(pSlider) == XUI_ORIENTATION_VERTICAL, "set vertical");
	xuiWidgetSetRect(pSlider, (xui_rect_t){30.0f, 80.0f, 30.0f, 150.0f});
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical reset");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render vertical");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tFill = xuiRangeSliderGetFillRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	XUI_TEST_CHECK(tTrack.fH > 110.0f && tFill.fY > tTrack.fY && tFill.fH > 0.0f, "vertical geometry");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	fY = tWorld.fY + tStartKnob.fY + tStartKnob.fH - 1.0f;
	iRet = xuiInputPointerDown(pContext, __xuiRangeSliderPixel(fX),
		__xuiRangeSliderPixel(fY), XUI_POINTER_BUTTON_LEFT,
		XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical edge grab pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 20.0f),
		"edge grab preserves vertical thumb value");
	fExpected = 20.0f + 1200.0f / tTrack.fH;
	iRet = xuiInputPointerMove(pContext, __xuiRangeSliderPixel(fX),
		__xuiRangeSliderPixel(fY) - 12, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical edge grab pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNearWithin(
		xuiRangeSliderGetStart(pSlider), fExpected, 0.75f),
		"edge grab preserves vertical pointer offset");
	iRet = xuiInputPointerUp(pContext, __xuiRangeSliderPixel(fX),
		__xuiRangeSliderPixel(fY) - 12, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical edge grab pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"vertical edge grab releases pointer");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset values after vertical edge grab");
	iRet = __xuiRangeSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetEnd(pSlider) > 95.0f, "vertical top high end");

	iRet = xuiRangeSliderSetRange(pSlider, 100.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize range");
	iRet = xuiRangeSliderSetValues(pSlider, 200.0f, -20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clamp values");
	iRet = xuiRangeSliderGetRange(pSlider, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 100.0f, "range normalized");
	iRet = xuiRangeSliderGetValues(pSlider, &fStart, &fEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && fStart == 0.0f && fEnd == 100.0f, "values normalized");
	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	iRet = xuiRangeSliderSetRange(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == 20.0f &&
		xuiRangeSliderGetEnd(pSlider) == 80.0f &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount + 1,
		"range clamp commits one values change");
	xuiWidgetClearDirty(pSlider, XUI_WIDGET_DIRTY_ALL);
	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	iRet = xuiRangeSliderSetRange(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetChangeCount(pSlider) == iChangeCount &&
		xuiWidgetGetDirtyFlags(pSlider) == 0u,
		"unchanged range does not dirty range slider");
	iRet = xuiRangeSliderSetRange(pSlider, 0.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore range after clamp transaction");
	iRet = xuiRangeSliderSetRange(pSlider, -FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite range");
	iRet = xuiRangeSliderSetValues(pSlider, -FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(xuiRangeSliderGetStartRate(pSlider)) &&
	               isfinite(xuiRangeSliderGetEndRate(pSlider)) &&
	               xuiRangeSliderGetStartRate(pSlider) == 0.0f && xuiRangeSliderGetEndRate(pSlider) == 1.0f,
	               "wide range finite endpoint rates");
	iRet = xuiRangeSliderSetStep(pSlider, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite steps");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 3u,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus wide range end thumb");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_UP, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide range end key input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetEnd(pSlider) == FLT_MAX &&
		isfinite(xuiRangeSliderGetEnd(pSlider)),
		"wide range key step saturates at maximum");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 3u,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetEnd(pSlider) == FLT_MAX,
		"wide range accessibility increment saturates at maximum");
	tWorld = xuiWidgetGetWorldRect(pSlider);
	iRet = xuiInputPointerWheel(pContext, tWorld.fX + tWorld.fW * 0.5f,
		tWorld.fY + tWorld.fH * 0.5f, 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide range wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetEnd(pSlider) == FLT_MAX,
		"wide range wheel step saturates at maximum");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 2u,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus wide range start thumb");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wide range start key input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == -FLT_MAX &&
		isfinite(xuiRangeSliderGetStart(pSlider)),
		"wide range key step saturates at minimum");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 2u,
		XUI_ACCESSIBLE_ACTION_DECREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == -FLT_MAX,
		"wide range accessibility decrement saturates at minimum");
	iRet = xuiRangeSliderSetStep(pSlider, 5.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary steps");
	iRet = xuiRangeSliderSetRange(pSlider, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize maximum equal range");
	iRet = xuiRangeSliderGetRange(pSlider, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(fMin) && isfinite(fMax) && fMin < fMax, "maximum equal range remains finite");
	iRet = xuiRangeSliderSetRange(pSlider, 0.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary range");
	iRet = xuiRangeSliderSetValues(pSlider, 0.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary values");
	iRet = xuiWidgetSetEnabled(pSlider, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable before disable drag");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before disable drag");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	fY = tWorld.fY + tStartKnob.fY + tStartKnob.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable range drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "disable range drag capture");
	iRet = xuiWidgetSetEnabled(pSlider, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               (xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "disable cancels range drag capture");
	XUI_TEST_CHECK((xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiRangeSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY + tTrack.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == 0.0f && xuiRangeSliderGetEnd(pSlider) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		tAccessible.iActions == 0u,
		"disabled range root is inert");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 2, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_DISABLED) != 0u &&
		tAccessible.iActions == 0u,
		"disabled range thumb is inert");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 3u,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet != XUI_OK,
		"disabled range rejects accessibility action");

	iRet = xuiWidgetSetEnabled(pSlider, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable before hidden state test");
	iRet = xuiRangeSliderSetOrientation(pSlider, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal before hidden state test");
	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore metrics before hidden state test");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set values before hidden state test");
	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 180.0f, 30.0f});
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before hidden state test");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	fY = tWorld.fY + tStartKnob.fY + tStartKnob.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "hidden state drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider,
		"hidden state drag capture");
	iRet = xuiWidgetSetVisible(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide range slider ancestor");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		(xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
		"hidden ancestor cancels range drag capture");
	fStart = xuiRangeSliderGetStart(pSlider);
	fEnd = xuiRangeSliderGetEnd(pSlider);
	iChangeCount = xuiRangeSliderGetChangeCount(pSlider);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pSlider;
	tEvent.iKey = XUI_KEY_RIGHT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch key to hidden range slider");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_WHEEL;
	tEvent.pTarget = pSlider;
	tEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tEvent.fX = fX;
	tEvent.fY = fY;
	tEvent.fWheelY = 1.0f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiRangeSliderGetStart(pSlider) == fStart &&
		xuiRangeSliderGetEnd(pSlider) == fEnd &&
		xuiRangeSliderGetChangeCount(pSlider) == iChangeCount,
		"hidden range slider ignores direct key and wheel input");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u &&
		tAccessible.iActions == 0u,
		"hidden range root is offscreen and inert");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 2, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0u &&
		tAccessible.iActions == 0u,
		"hidden range thumb is offscreen and inert");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 3u,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet != XUI_OK,
		"hidden range rejects accessibility action");
	iRet = xuiWidgetSetVisible(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "show range slider ancestor");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch visible range state");
	XUI_TEST_CHECK(xuiWidgetGetVisible(pRoot) != 0,
		"range slider ancestor restored before tiny geometry test");
	iRet = xuiRangeSliderSetOrientation(pSlider, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal tiny geometry orientation");
	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, 18.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore metrics before tiny geometry test");
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set tiny geometry values");
	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 5.0f, 5.0f});
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tEndKnob = xuiRangeSliderGetEndKnobRect(pSlider);
	XUI_TEST_CHECK(tTrack.fW == 0.0f &&
		tStartKnob.fX >= 0.0f && tStartKnob.fY >= 0.0f &&
		tStartKnob.fX + tStartKnob.fW <= 5.0f &&
		tStartKnob.fY + tStartKnob.fH <= 5.0f &&
		tEndKnob.fX >= 0.0f && tEndKnob.fY >= 0.0f &&
		tEndKnob.fX + tEndKnob.fW <= 5.0f &&
		tEndKnob.fY + tEndKnob.fH <= 5.0f,
		"tiny range slider keeps empty travel and bounded knobs");
	tWorld = xuiWidgetGetWorldRect(pSlider);
	iRet = __xuiRangeSliderClick(pContext, tWorld.fX + 2.0f, tWorld.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		__xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 20.0f) &&
		__xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 80.0f),
		"tiny range slider without thumb travel preserves values");

	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 0.0f, 0.0f});
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tFill = xuiRangeSliderGetFillRect(pSlider);
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	tEndKnob = xuiRangeSliderGetEndKnobRect(pSlider);
	XUI_TEST_CHECK(tTrack.fW == 0 && tTrack.fH == 0 && tFill.fW == 0 && tFill.fH == 0 &&
		tStartKnob.fW == 0 && tStartKnob.fH == 0 && tEndKnob.fW == 0 && tEndKnob.fH == 0,
		"zero size range slider has empty geometry");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMax = 100.0f;
	tDesc.fStart = 20.0f;
	tDesc.fEnd = 80.0f;
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	iRet = xuiRangeSliderCreate(pContext, &pDestroySlider, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroySlider != NULL, "destroy callback range slider create");
	xuiWidgetSetRect(pDestroySlider, (xui_rect_t){80.0f, 80.0f, 160.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pDestroySlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback range slider add");
	tDestroyState.iDestroyAt = 2;
	tDestroyState.ppWidget = &pDestroySlider;
	iRet = xuiRangeSliderSetChange(pDestroySlider, __xuiRangeSliderDestroyOnChange,
		&tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback range slider set");
	tTrack = xuiRangeSliderGetTrackRect(pDestroySlider);
	tWorld = xuiWidgetGetWorldRect(pDestroySlider);
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.1f,
		fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback range pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 1 && pDestroySlider != NULL,
		"destroy callback range first change");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.4f,
		fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback range pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 2 && pDestroySlider == NULL &&
		xuiGetPointerCapture(pContext) == NULL,
		"change callback may destroy range slider during drag");

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
	printf("xui_range_slider_test passed\n");
	return 0;
}
