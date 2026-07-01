#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_surface pTarget;
	xui_slider_desc_t tDesc;
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tKnob;
	xui_rect_t tWorld;
	float fMin;
	float fMax;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fTrackRadius;
	float fX;
	float fY;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pSlider = NULL;
	pTarget = NULL;
	iChanged = 0;
	iFailed = 0;
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

	iRet = xuiSliderSetOrientation(pSlider, XUI_ORIENTATION_VERTICAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetOrientation(pSlider) == XUI_ORIENTATION_VERTICAL, "set vertical");
	xuiWidgetSetRect(pSlider, (xui_rect_t){30.0f, 70.0f, 30.0f, 140.0f});
	iRet = xuiSliderSetValue(pSlider, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "vertical reset");
	iRet = __xuiSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render vertical");
	tTrack = xuiSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
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
	iRet = xuiWidgetSetEnabled(pSlider, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	XUI_TEST_CHECK((xuiSliderGetState(pSlider) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY + tTrack.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSliderGetValue(pSlider) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");

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
