#include "xui.h"
#include "xui_test_proxy.h"

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

static int __xuiRangeSliderRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 280, 260};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiRangeSliderClick(xui_context pContext, float fX, float fY)
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
	xui_range_slider_desc_t tDesc;
	xui_style_property_t arrStyle[2];
	xui_rect_t tTrack;
	xui_rect_t tFill;
	xui_rect_t tStartKnob;
	xui_rect_t tEndKnob;
	xui_rect_t tWorld;
	float fMin;
	float fMax;
	float fStart;
	float fEnd;
	float fStep;
	float fPageStep;
	float fTrackSize;
	float fKnobSize;
	float fTrackRadius;
	float fKnobRadius;
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
	iRet = xuiWidgetAddChild(pRoot, pSlider);
	XUI_TEST_CHECK(iRet == XUI_OK, "add range slider");
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

	iRet = xuiRangeSliderSetMetrics(pSlider, 6.0f, 18.0f, -1.0f, -1.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiRangeSliderGetMetrics(pSlider, &fTrackSize, &fKnobSize, &fTrackRadius, &fKnobRadius);
	XUI_TEST_CHECK(iRet == XUI_OK && fTrackSize == 6.0f && fKnobSize == 18.0f && fTrackRadius == -1.0f && fKnobRadius == -1.0f, "get metrics");
	iRet = xuiRangeSliderSetColors(pSlider, XUI_COLOR_RGBA(225, 238, 234, 255), XUI_COLOR_RGBA(0, 184, 169, 255), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(0, 184, 169, 160), XUI_COLOR_RGBA(160, 178, 172, 150));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = xuiRangeSliderSetKnobBorderColor(pSlider, XUI_COLOR_RGBA(0, 150, 136, 255));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetKnobBorderColor(pSlider) == XUI_COLOR_RGBA(0, 150, 136, 255), "knob border");
	iRet = xuiRangeSliderSetStep(pSlider, 5.0f, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set step");
	iRet = xuiRangeSliderGetStep(pSlider, &fStep, &fPageStep);
	XUI_TEST_CHECK(iRet == XUI_OK && fStep == 5.0f && fPageStep == 20.0f, "get step");
	arrStyle[0] = __xuiRangeSliderStyleProp("rangeslider.track.size", __xuiRangeSliderStyleFloat(8.0f));
	arrStyle[1] = __xuiRangeSliderStyleProp("rangeslider.fill.color", __xuiRangeSliderStyleColor(XUI_COLOR_RGBA(40, 90, 160, 255)));
	iRet = xuiWidgetSetInlineStyle(pSlider, arrStyle, 2);
	XUI_TEST_CHECK(iRet == XUI_OK, "inline style");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	XUI_TEST_CHECK(tTrack.fH == 8.0f, "style track size");
	iRet = xuiWidgetSetInlineStyle(pSlider, NULL, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear inline style");

	iChanged = 0;
	iRet = xuiRangeSliderSetValues(pSlider, 20.0f, 80.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset values");
	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before pointer");
	tTrack = xuiRangeSliderGetTrackRect(pSlider);
	tWorld = xuiWidgetGetWorldRect(pSlider);
	fX = tWorld.fX + tTrack.fX + tTrack.fW * 0.10f;
	fY = tWorld.fY + tTrack.fY + tTrack.fH * 0.5f;
	iRet = __xuiRangeSliderClick(pContext, fX, fY);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 10.0f) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 80.0f), "nearest start pointer");
	XUI_TEST_CHECK(xuiRangeSliderGetActiveThumb(pSlider) == XUI_RANGE_SLIDER_THUMB_START && iChanged == 1, "active start");

	iRet = __xuiRangeSliderRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before drag");
	tStartKnob = xuiRangeSliderGetStartKnobRect(pSlider);
	fX = tWorld.fX + tStartKnob.fX + tStartKnob.fW * 0.5f;
	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pSlider && (xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_ACTIVE) != 0, "pointer capture");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + tTrack.fX + tTrack.fW, fY, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "drag move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiRangeSliderNear(xuiRangeSliderGetStart(pSlider), 80.0f) && __xuiRangeSliderNear(xuiRangeSliderGetEnd(pSlider), 80.0f), "start clamps at end");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + tTrack.fX + tTrack.fW, fY, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL, "pointer release");

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
	iRet = xuiWidgetSetEnabled(pSlider, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	XUI_TEST_CHECK((xuiRangeSliderGetState(pSlider) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiRangeSliderClick(pContext, tWorld.fX + tTrack.fX + tTrack.fW * 0.5f, tWorld.fY + tTrack.fY + tTrack.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRangeSliderGetStart(pSlider) == 0.0f && xuiRangeSliderGetEnd(pSlider) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");

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
