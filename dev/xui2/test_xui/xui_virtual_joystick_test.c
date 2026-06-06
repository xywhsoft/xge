#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_virtual_joystick_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiVirtualJoystickNear(float fA, float fB)
{
	float fD;

	fD = fA - fB;
	if ( fD < 0.0f ) fD = -fD;
	return fD < 0.03f;
}

static void __xuiVirtualJoystickChanged(xui_widget pWidget, const xui_virtual_joystick_state_t* pState, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)pState;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiVirtualJoystickRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 260, 240};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pJoystick;
	xui_surface pTarget;
	xui_virtual_joystick_desc_t tDesc;
	xui_virtual_joystick_state_t tJoyState;
	xui_rect_t tBase;
	xui_rect_t tKnob;
	xui_rect_t tWorld;
	float fRadius;
	float fKnob;
	float fDeadZone;
	float fPressed;
	int iPressed;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pJoystick = NULL;
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
	(void)xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 240.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fRadius = 70.0f;
	tDesc.fKnobSize = 46.0f;
	tDesc.fDeadZone = 0.05f;
	tDesc.bUseBuiltinAtlas = 1;
	tDesc.onChange = __xuiVirtualJoystickChanged;
	tDesc.pChangeUser = &iChanged;
	iRet = xuiVirtualJoystickCreate(pContext, &pJoystick, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pJoystick != NULL, "joystick create");
	iRet = xuiWidgetAddChild(pRoot, pJoystick);
	XUI_TEST_CHECK(iRet == XUI_OK, "add joystick");
	(void)xuiWidgetSetRect(pJoystick, (xui_rect_t){30.0f, 25.0f, 170.0f, 170.0f});
	XUI_TEST_CHECK(xuiWidgetGetFocusable(pJoystick) == 1, "focusable");
	XUI_TEST_CHECK(xuiVirtualJoystickGetUseBuiltinAtlas(pJoystick) == 1, "builtin atlas enabled");

	iRet = xuiVirtualJoystickGetMetrics(pJoystick, &fRadius, &fKnob, &fDeadZone);
	XUI_TEST_CHECK(iRet == XUI_OK && fRadius == 70.0f && fKnob == 46.0f && fDeadZone == 0.05f, "metrics");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 240, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiVirtualJoystickRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pTarget) > 0, "draw count");
	tBase = xuiVirtualJoystickGetBaseRect(pJoystick);
	tKnob = xuiVirtualJoystickGetKnobRect(pJoystick);
	XUI_TEST_CHECK(tBase.fW > 120.0f && tBase.fH > 120.0f, "base rect");
	XUI_TEST_CHECK(tKnob.fW > 40.0f && tKnob.fH > 40.0f, "knob rect");

	iChanged = 0;
	iRet = xuiVirtualJoystickSetChannel(pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, 1, 0.5f, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "right channel set");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK, "get state");
	XUI_TEST_CHECK(tJoyState.bActive && tJoyState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_CHANNEL, "channel source");
	XUI_TEST_CHECK(__xuiVirtualJoystickNear(tJoyState.fX, 0.5f) && __xuiVirtualJoystickNear(tJoyState.fY, 0.0f), "right channel vector");
	XUI_TEST_CHECK(iChanged == 1 && xuiVirtualJoystickGetChangeCount(pJoystick) >= 1, "channel change count");
	iRet = xuiVirtualJoystickGetChannel(pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_RIGHT, &iPressed, &fPressed);
	XUI_TEST_CHECK(iRet == XUI_OK && iPressed == 1 && fPressed == 0.5f, "get channel");

	iRet = xuiVirtualJoystickSetChannel(pJoystick, XUI_VIRTUAL_JOYSTICK_CHANNEL_UP, 1, 1.0f, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "up channel set");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && tJoyState.fX > 0.40f && tJoyState.fY < -0.80f, "diagonal channel vector");

	iRet = xuiVirtualJoystickSetValue(pJoystick, 2.0f, 0.0f, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "program set");
	XUI_TEST_CHECK(__xuiVirtualJoystickNear(xuiVirtualJoystickGetX(pJoystick), 1.0f), "program clamp x");
	XUI_TEST_CHECK(xuiVirtualJoystickGetMagnitude(pJoystick) <= 1.0f, "program clamp magnitude");

	iRet = xuiVirtualJoystickClearChannels(pJoystick, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear channels");
	iRet = xuiVirtualJoystickReset(pJoystick, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && !tJoyState.bActive && tJoyState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_NONE, "reset state");

	iRet = __xuiVirtualJoystickRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render before touch");
	tBase = xuiVirtualJoystickGetBaseRect(pJoystick);
	tWorld = xuiWidgetGetWorldRect(pJoystick);
	iRet = xuiInputPointerMoveEx(pContext, 7, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW * 0.5f, tWorld.fY + tBase.fY + tBase.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch move input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch move dispatch");
	iRet = xuiInputPointerDownEx(pContext, 7, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == pJoystick, "touch capture");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && tJoyState.bActive && tJoyState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_POINTER && tJoyState.iPointerId == 7, "touch state");
	XUI_TEST_CHECK(tJoyState.fX > 0.90f && __xuiVirtualJoystickNear(tJoyState.fY, 0.0f), "touch vector");
	iRet = xuiInputPointerUpEx(pContext, 7, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == NULL, "touch release");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && !tJoyState.bActive, "touch release state");

	iRet = xuiVirtualJoystickSetMetrics(pJoystick, 64.0f, 44.0f, 0.10f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiVirtualJoystickUseBuiltinAtlas(pJoystick, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiVirtualJoystickGetUseBuiltinAtlas(pJoystick) == 0, "disable atlas");
	iRet = xuiVirtualJoystickSetColors(pJoystick, XUI_COLOR_RGBA(30, 40, 52, 130), XUI_COLOR_RGBA(40, 120, 210, 180), XUI_COLOR_RGBA(230, 238, 248, 240), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(72, 154, 255, 120), XUI_COLOR_RGBA(72, 154, 255, 180), XUI_COLOR_RGBA(150, 160, 170, 120));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = __xuiVirtualJoystickRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback render");

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
	printf("xui_virtual_joystick_test passed\n");
	return 0;
}
