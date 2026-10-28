#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_slider_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiSliderNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) {
		fD = -fD;
	}
	return fD < 0.02f;
}

static void __xuiSliderChanged(xui_widget pWidget, float fValue, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)fValue;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

typedef struct xui_slider_destroy_test_t {
	int iCount;
	int iDestroyAt;
	xui_widget* ppWidget;
} xui_slider_destroy_test_t;

static void __xuiSliderDestroyOnChange(xui_widget pWidget, float fValue, void* pUser)
{
	xui_slider_destroy_test_t* pState = (xui_slider_destroy_test_t*)pUser;

	(void)fValue;
	if ( pState == NULL ) return;
	pState->iCount++;
	if ( pState->iCount == pState->iDestroyAt ) {
		xuiWidgetDestroy(pWidget);
		if ( pState->ppWidget != NULL ) *pState->ppWidget = NULL;
	}
}

static int __xuiSliderRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 260, 240};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiSliderClick(xui_context pContext, float fX, float fY)
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
	xui_widget pSlider;
	xui_widget pInvalid;
	xui_widget pDestroySlider;
	xui_surface pTarget;
	xui_slider_desc_t tDesc;
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tKnob;
	xui_rect_t tWorld;
	xui_accessible_node_t tAccessible;
	xui_accessible_value_t tAccessibleValue;
	xui_slider_destroy_test_t tDestroyState;
	float fMin;
	float fMax;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fTrackRadius;
	float fX;
	float fY;
	float fExpected;
	float fOwnedValue;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pSlider = NULL;
	pInvalid = NULL;
	pDestroySlider = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
	memset(&tDestroyState, 0, sizeof(tDestroyState));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 260.0f, 240.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 240.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMin = 0.0f;
	tDesc.fMax = 100.0f;
	tDesc.fValue = 25.0f;
	tDesc.fStep = 5.0f;
	tDesc.fPageStep = 20.0f;
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	iRet = xuiSliderCreate(pContext, &pSlider, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSlider != NULL, "slider create");
	tDesc.fValue = NAN;
	pInvalid = pRoot;
	iRet = xuiSliderCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject nan descriptor");
	tDesc.fValue = 25.0f;
	tDesc.fKnobSize = FLT_MAX;
	pInvalid = pRoot;
	iRet = xuiSliderCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject out of layout range descriptor metric");
	tDesc.fKnobSize = 0.0f;
	iRet = xuiWidgetAddChild(pRoot, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "add slider");
	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 140.0f, 30.0f});
	iRet = xuiSliderSetChange(pSlider, __xuiSliderChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pSlider) == 1, "focusable");
	XUI_TEST_CHECK(xuiWidgetGetOverflow(pSlider) == XUI_OVERFLOW_CLIP, "overflow clip");
	XUI_TEST_CHECK(xuiSliderGetOrientation(pSlider) == XUI_ORIENTATION_HORIZONTAL, "horizontal orientation");
	XUI_TEST_CHECK(__xuiSliderNear(xuiSliderGetValue(pSlider), 25.0f), "initial value");
	XUI_TEST_CHECK(__xuiSliderNear(xuiSliderGetRate(pSlider), 0.25f), "initial rate");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pSlider, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_SLIDER &&
		tAccessible.tRange.iSize == sizeof(tAccessible.tRange) &&
		tAccessible.tRange.fMin == 0.0 && tAccessible.tRange.fMax == 100.0 &&
		tAccessible.tRange.fValue == 25.0,
		"slider accessibility range");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_NUMBER;
	tAccessibleValue.fNumber = 30.0;
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 0, XUI_ACCESSIBLE_ACTION_SET_VALUE,
		&tAccessibleValue);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == 30.0f,
		"slider accessibility set value");
	iRet = xuiSliderSetValue(pSlider, 25.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore value after accessibility action");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 240, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render horizontal slider");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tFill = xuiSliderGetFillRect(pSlider);
	tKnob = xuiSliderGetKnobRect(pSlider);
	XUI_TEST_CHECK(tTrack.fW > 100.0f && tTrack.fH >= 4.0f, "track geometry");
	XUI_TEST_CHECK(tFill.fW > 0.0f && tFill.fW < tTrack.fW, "fill geometry");
	XUI_TEST_CHECK(tKnob.fW > 10.0f && tKnob.fH == tKnob.fW, "knob geometry");

	iRet = xuiSliderSetMetrics(pSlider, 6.0f, 18.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiSliderGetMetrics(pSlider, &fTrackSize, &fKnobSize, &fTrackRadius);
	XUI_TEST_CHECK(iRet == XUI_OK && fTrackSize == 6.0f && fKnobSize == 18.0f && fTrackRadius == -1.0f, "get metrics");
	iRet = xuiSliderSetColors(pSlider, XUI_COLOR_RGBA(225, 238, 234, 255), XUI_COLOR_RGBA(32, 150, 112, 255), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(32, 150, 112, 160), XUI_COLOR_RGBA(160, 178, 172, 150));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiSliderSetKnobBorderColor(pSlider, XUI_COLOR_RGBA(25, 118, 88, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetKnobBorderColor(pSlider) == XUI_COLOR_RGBA(25, 118, 88, 255), "knob border");
	iRet = xuiSliderSetStep(pSlider, 5.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set step");
	iRet = xuiSliderGetStep(pSlider, &fStep, &fPageStep);
	XUI_TEST_CHECK(iRet == XUI_OK && fStep == 5.0f && fPageStep == 20.0f, "get step");
	iRet = xuiSliderSetStep(pSlider, NAN, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan step");
	iRet = xuiSliderSetMetrics(pSlider, 6.0f, INFINITY, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite metrics");
	iRet = xuiSliderSetMetrics(pSlider, 6.0f, FLT_MAX, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject out of layout range metrics");
	iRet = xuiSliderGetMetrics(pSlider, &fTrackSize, &fKnobSize, &fTrackRadius);
	XUI_TEST_CHECK(iRet == XUI_OK && fTrackSize == 6.0f && fKnobSize == 18.0f &&
		fTrackRadius == -1.0f, "invalid metrics preserve state");
	iRet = xuiSliderSetValue(pSlider, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && __xuiSliderNear(xuiSliderGetValue(pSlider), 25.0f), "reject nan value and preserve state");
	iRet = xuiSliderSetRange(pSlider, 0.0f, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite range");

	iRet = xuiSliderSetValue(pSlider, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set thumb offset value");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tKnob = xuiSliderGetKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tKnob.fX + tKnob.fW - 1.0f;
	fY = tWorld.fY + tKnob.fY + tKnob.fH * 0.5f;
	iChanged = 0;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 50.0f) &&
		iChanged == 0 && xuiGetPointerCapture(pContext) == pSlider,
		"thumb edge press preserves value and captures pointer");
	fExpected = 50.0f + 10.0f * 100.0f / tTrack.fW;
	iRet = xuiInputPointerMove(pContext, fX + 10.0f, fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), fExpected) &&
		iChanged == 1, "thumb drag preserves grab offset");
	iRet = xuiInputPointerUp(pContext, fX + 10.0f, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"thumb offset drag releases pointer");

	iChanged = 0;
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset value");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before pointer");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tTrack.fX + tTrack.fW * 0.5f;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = xuiInputPointerMove(pContext, fX, fY, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move dispatch");
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider && (xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) != 0, "pointer capture");
	XUI_TEST_CHECK(__xuiSliderNear(xuiSliderGetValue(pSlider), 50.0f) && iChanged == 1, "pointer down value");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 100.0f) && iChanged == 2, "drag value");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tTrack.fX + tTrack.fW, fY, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pointer release");

	iRet = xuiSliderSetValue(pSlider, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set multi-pointer value");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before multi-pointer drag");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tTrack.fX + tTrack.fW * 0.2f;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = xuiInputPointerDownEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	fOwnedValue = xuiSliderGetValue(pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pSlider,
		"first touch owns slider drag");
	iRet = xuiInputPointerDownEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 302u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + tTrack.fX + tTrack.fW),
		(int)fY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 302u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + tTrack.fX + tTrack.fW),
		(int)fY, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), fOwnedValue) &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == pSlider &&
		xuiGetPointerCaptureEx(pContext, 302u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot move or release slider drag");
	iRet = xuiInputPointerMoveEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tTrack.fX + tTrack.fW * 0.8f), (int)fY,
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) > fOwnedValue,
		"owning touch moves slider knob");
	iRet = xuiInputPointerUpEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tTrack.fX + tTrack.fW * 0.8f), (int)fY,
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 301u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch releases slider drag");
	iRet = xuiInputPointerDownEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiSliderSetMetrics(pSlider, 7.0f, 19.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		(xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
		"geometry change cancels exact slider capture");
	iRet = xuiSliderSetMetrics(pSlider, 6.0f, 18.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore metrics after multi-pointer drag");
	(void)xuiInputPointerUpEx(pContext, 303u, XUI_POINTER_TYPE_TOUCH,
		(int)fX, (int)fY, XUI_POINTER_BUTTON_LEFT, 0u);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSliderSetValue(pSlider, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel reset");
	iChanged = 0;
	iRet = xuiInputPointerWheel(pContext, fX, fY, 0.0f, 1.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "wheel input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 55.0f) && iChanged == 1, "wheel step");

	iRet = xuiSetFocusWidget(pContext, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus slider");
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "key reset");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "right input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 5.0f), "right step");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_PAGE_UP, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 25.0f), "page up step");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "left input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 20.0f), "left step");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_PAGE_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "page down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 0.0f), "page down step");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_END, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "end input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 100.0f), "end value");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_HOME, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "home input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 0.0f), "home value");

	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before escape");
	tKnob = xuiSliderGetKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tKnob.fX + tKnob.fW * 0.5f;
	fY = tWorld.fY + tKnob.fY + tKnob.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "escape capture");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL && (xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0, "escape cancels drag");
	(void)xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSetFocusWidget(pContext, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus before blur drag");
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "blur drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "blur drag capture");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during drag");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               (xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels drag capture");
	(void)xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	(void)xuiDispatchPendingEvents(pContext);

	iRet = xuiSliderSetOrientation(pSlider, XUI_ORIENTATION_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetOrientation(pSlider) == XUI_ORIENTATION_VERTICAL, "set vertical");
	xuiWidgetSetRect(pSlider, (xui_rect_t){30.0f, 70.0f, 30.0f, 140.0f});
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical reset");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render vertical");
	iRet = xuiSliderSetValue(pSlider, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical thumb offset value");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tKnob = xuiSliderGetKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tKnob.fX + tKnob.fW * 0.5f;
	fY = tWorld.fY + tKnob.fY + tKnob.fH - 1.0f;
	iRet = xuiInputPointerDown(pContext, fX, fY,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 50.0f),
		"vertical thumb edge press preserves value");
	fExpected = 50.0f - 10.0f * 100.0f / tTrack.fH;
	iRet = xuiInputPointerMove(pContext, fX, fY + 10.0f, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), fExpected),
		"vertical thumb drag preserves grab offset");
	iRet = xuiInputPointerUp(pContext, fX, fY + 10.0f, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL,
		"vertical thumb offset drag releases pointer");
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical endpoint reset");
	iRet = __xuiSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) > 95.0f, "vertical top high value");
	iRet = __xuiSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY + tTrack.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) < 5.0f, "vertical bottom low value");

	iRet = xuiSliderSetRange(pSlider, 100.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize range");
	iRet = xuiSliderSetValue(pSlider, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clamp value");
	iRet = xuiSliderGetRange(pSlider, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 100.0f && xuiSliderGetValue(pSlider) == 100.0f, "range clamp");
	iRet = xuiSliderSetRange(pSlider, -FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite range");
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(xuiSliderGetRate(pSlider)) && __xuiSliderNear(xuiSliderGetRate(pSlider), 0.5f), "wide range finite midpoint rate");
	iRet = xuiSliderSetStep(pSlider, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide finite steps");
	iRet = xuiSliderSetValue(pSlider, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide range maximum");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == FLT_MAX,
		"keyboard increment saturates at wide range maximum");
	iRet = xuiWidgetPerformAccessibleAction(pSlider, 0,
		XUI_ACCESSIBLE_ACTION_INCREMENT, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == FLT_MAX,
		"accessible increment saturates at wide range maximum");
	iRet = xuiInputPointerWheel(pContext,
		tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f,
		0.0f, 1.0f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == FLT_MAX,
		"wheel increment saturates at wide range maximum");
	iRet = xuiSliderSetValue(pSlider, -FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "set wide range minimum");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == -FLT_MAX,
		"keyboard decrement saturates at wide range minimum");
	iRet = xuiSliderSetRange(pSlider, FLT_MAX, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize maximum equal range");
	iRet = xuiSliderGetRange(pSlider, &fMin, &fMax);
	XUI_TEST_CHECK(iRet == XUI_OK && isfinite(fMin) && isfinite(fMax) && fMin < fMax, "maximum equal range remains finite");
	iRet = xuiSliderSetRange(pSlider, 0.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary range");
	iRet = xuiSliderSetStep(pSlider, 5.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary steps");
	iRet = xuiSliderSetValue(pSlider, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore ordinary value");
	iRet = xuiWidgetSetEnabled(pSlider, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable before disable drag");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before disable drag");
	tKnob = xuiSliderGetKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tKnob.fX + tKnob.fW * 0.5f;
	fY = tWorld.fY + tKnob.fY + tKnob.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider, "disable drag capture");
	iRet = xuiWidgetSetEnabled(pSlider, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
	               (xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "disable cancels drag capture");
	XUI_TEST_CHECK((xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY + tTrack.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");
	iRet = xuiWidgetSetEnabled(pSlider, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable before tiny geometry");
	iRet = xuiSliderSetOrientation(pSlider, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "horizontal before tiny geometry");
	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 5.0f, 5.0f});
	iRet = xuiSliderSetValue(pSlider, 50.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "tiny geometry value");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tKnob = xuiSliderGetKnobRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	XUI_TEST_CHECK(tTrack.fX >= 0.0f && tTrack.fY >= 0.0f &&
		tTrack.fX + tTrack.fW <= 5.0f && tTrack.fY + tTrack.fH <= 5.0f &&
		tKnob.fX >= 0.0f && tKnob.fY >= 0.0f &&
		tKnob.fX + tKnob.fW <= 5.0f && tKnob.fY + tKnob.fH <= 5.0f,
		"tiny slider geometry stays inside content");
	iRet = __xuiSliderClick(pContext, tWorld.fX + 4.0f, tWorld.fY + 2.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiSliderNear(xuiSliderGetValue(pSlider), 50.0f),
		"slider without thumb travel preserves value");

	xuiWidgetSetRect(pSlider, (xui_rect_t){10.0f, 10.0f, 0.0f, 0.0f});
	tTrack = xuiSliderGetTrackRect(pSlider);
	tFill = xuiSliderGetFillRect(pSlider);
	tKnob = xuiSliderGetKnobRect(pSlider);
	XUI_TEST_CHECK(tTrack.fW == 0 && tTrack.fH == 0 && tFill.fW == 0 &&
		tFill.fH == 0 && tKnob.fW == 0 && tKnob.fH == 0,
		"zero size slider has empty geometry");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fMax = 100.0f;
	tDesc.iOrientation = XUI_ORIENTATION_HORIZONTAL;
	iRet = xuiSliderCreate(pContext, &pDestroySlider, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroySlider != NULL, "destroy callback slider create");
	xuiWidgetSetRect(pDestroySlider, (xui_rect_t){80.0f, 80.0f, 140.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pDestroySlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback slider add");
	tDestroyState.iDestroyAt = 2;
	tDestroyState.ppWidget = &pDestroySlider;
	iRet = xuiSliderSetChange(pDestroySlider, __xuiSliderDestroyOnChange, &tDestroyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	tTrack = xuiSliderGetTrackRect(pDestroySlider);
	tWorld = xuiWidgetGetWorldRect(pDestroySlider);
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f,
		fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback pointer down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 1 && pDestroySlider != NULL,
		"destroy callback first change");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW,
		fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback pointer move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroyState.iCount == 2 && pDestroySlider == NULL &&
		xuiGetPointerCapture(pContext) == NULL,
		"change callback may destroy slider during drag");

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
	printf("xui_slider_test passed\n");
	return 0;
}
