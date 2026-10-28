#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

static void __xuiVirtualJoystickDestroyOnChange(
	xui_widget pWidget,
	const xui_virtual_joystick_state_t* pState,
	void* pUser)
{
	int* pCount = (int*)pUser;

	(void)pState;
	if ( pCount != NULL ) (*pCount)++;
	xuiWidgetDestroy(pWidget);
}

static int __xuiVirtualJoystickRootKey(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	int* pCount = (int*)pUser;

	(void)pWidget;
	if ( pEvent != NULL && pEvent->iType == XUI_EVENT_KEY_DOWN && pEvent->iKey == XUI_KEY_ESCAPE &&
	     pCount != NULL ) (*pCount)++;
	return XUI_OK;
}

static xui_widget_cache_render_proc g_xuiVirtualJoystickOriginalRender;
static void* g_xuiVirtualJoystickOriginalRenderUser;
static int g_xuiVirtualJoystickRenderCount;

static int __xuiVirtualJoystickCountRender(
	xui_widget pWidget,
	xui_draw_context pDraw,
	uint32_t iState,
	void* pUser)
{
	(void)pUser;
	g_xuiVirtualJoystickRenderCount++;
	return g_xuiVirtualJoystickOriginalRender(
		pWidget, pDraw, iState, g_xuiVirtualJoystickOriginalRenderUser);
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
	xui_widget pLifecycleJoystick;
	xui_widget pInvalid;
	xui_widget pChild;
	xui_surface pTarget;
	xui_surface pCustomSurface;
	xui_surface pCache;
	xui_virtual_joystick_desc_t tDesc;
	xui_virtual_joystick_desc_t tInvalidDesc;
	xui_virtual_joystick_state_t tJoyState;
	xui_accessible_node_t tAccessible;
	xui_event_t tKeyEvent;
	xui_rect_t tBase;
	xui_rect_t tKnob;
	xui_rect_t tWorld;
	xui_rect_t tLastSrc;
	float fRadius;
	float fKnob;
	float fDeadZone;
	float fPressed;
	uint32_t iDirtyFlags;
	int iPressed;
	int iChanged;
	int iDestroyChanges;
	int iRootEscapeCount;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pJoystick = NULL;
	pLifecycleJoystick = NULL;
	pInvalid = NULL;
	pChild = NULL;
	pTarget = NULL;
	pCustomSurface = NULL;
	pCache = NULL;
	iChanged = 0;
	iDestroyChanges = 0;
	iRootEscapeCount = 0;
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
	iRet = xuiWidgetSetEventHandler(pRoot, XUI_EVENT_KEY_DOWN,
		__xuiVirtualJoystickRootKey, &iRootEscapeCount);
	XUI_TEST_CHECK(iRet == XUI_OK, "root key observer");
	pInvalid = pRoot;
	iRet = xuiVirtualJoystickCreate(NULL, &pInvalid, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject invalid context");

	memset(&tInvalidDesc, 0, sizeof(tInvalidDesc));
	tInvalidDesc.iSize = sizeof(tInvalidDesc);
	tInvalidDesc.fRadius = NAN;
	pLifecycleJoystick = pRoot;
	iRet = xuiVirtualJoystickCreate(pContext, &pLifecycleJoystick, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecycleJoystick == NULL,
		"reject NaN descriptor");
	tInvalidDesc.fRadius = 0.0f;
	tInvalidDesc.tKnobSrc.fX = INFINITY;
	pLifecycleJoystick = pRoot;
	iRet = xuiVirtualJoystickCreate(pContext, &pLifecycleJoystick, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecycleJoystick == NULL,
		"reject infinite source rect");
	tInvalidDesc.tKnobSrc.fX = 0.0f;
	tInvalidDesc.fRadius = XUI_LAYOUT_UNBOUNDED;
	pLifecycleJoystick = pRoot;
	iRet = xuiVirtualJoystickCreate(pContext, &pLifecycleJoystick, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pLifecycleJoystick == NULL,
		"reject descriptor radius that can overflow layout geometry");

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
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	iRet = xuiWidgetAddChild(pJoystick, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "child add");
	(void)xuiWidgetSetHitTestVisible(pChild, 0);
	(void)xuiWidgetSetRect(pJoystick, (xui_rect_t){30.0f, 25.0f, 170.0f, 170.0f});
	iRet = xuiWidgetGetCacheRenderCallback(pJoystick,
		&g_xuiVirtualJoystickOriginalRender, &g_xuiVirtualJoystickOriginalRenderUser);
	XUI_TEST_CHECK(iRet == XUI_OK && g_xuiVirtualJoystickOriginalRender != NULL,
		"get cache renderer");
	iRet = xuiWidgetSetCacheRenderCallback(pJoystick, __xuiVirtualJoystickCountRender, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "install cache renderer probe");
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
	g_xuiVirtualJoystickRenderCount = 0;
	iRet = __xuiVirtualJoystickRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "initial render");
	XUI_TEST_CHECK(g_xuiVirtualJoystickRenderCount == 1, "only current state cache rendered");
	XUI_TEST_CHECK(xuiTestSurfaceGetDrawCount(pTarget) > 0, "draw count");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNodeCount(pJoystick) == 1 &&
		xuiWidgetGetAccessibleNode(pJoystick, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_GROUP &&
		strcmp(tAccessible.sValue, "X 0.00, Y 0.00") == 0 &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_FOCUSABLE) != 0,
		"joystick accessibility root");
	iRet = xuiWidgetPerformAccessibleAction(pJoystick, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pJoystick,
		"joystick accessibility focus action");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pJoystick, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK, "hide joystick");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pJoystick, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_OFFSCREEN) != 0 &&
		tAccessible.iActions == 0,
		"hidden joystick accessibility is offscreen and inert");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pJoystick, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_FOCUS, NULL) == XUI_ERROR_INVALID_ARGUMENT,
		"hidden joystick rejects accessibility focus");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pJoystick, 1) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK, "show joystick");
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
	XUI_TEST_CHECK(xuiVirtualJoystickSetValue(pJoystick, NAN, 0.0f, 1) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN program value");
	XUI_TEST_CHECK(xuiVirtualJoystickSetChannel(pJoystick,
		XUI_VIRTUAL_JOYSTICK_CHANNEL_LEFT, 1, INFINITY, 1) == XUI_ERROR_INVALID_ARGUMENT,
		"reject infinite channel value");
	XUI_TEST_CHECK(xuiVirtualJoystickSetMetrics(pJoystick, NAN, 44.0f, 0.1f) ==
		XUI_ERROR_INVALID_ARGUMENT, "reject NaN metrics");
	XUI_TEST_CHECK(xuiVirtualJoystickSetMetrics(pJoystick, XUI_LAYOUT_UNBOUNDED,
		44.0f, 0.1f) == XUI_ERROR_INVALID_ARGUMENT,
		"reject radius that can overflow layout geometry");
	XUI_TEST_CHECK(xuiVirtualJoystickSetSurface(pJoystick, XUI_VIRTUAL_JOYSTICK_PART_BASE,
		NULL, (xui_rect_t){NAN, 0.0f, 0.0f, 0.0f}) == XUI_ERROR_INVALID_ARGUMENT,
		"reject NaN surface rect");
	iChanged = 0;
	iRet = xuiVirtualJoystickSetValue(pJoystick, 0.25f, 0.0f, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged == 1, "program set sends one notification");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pJoystick, 0, &tAccessible) == XUI_OK &&
		strcmp(tAccessible.sValue, "X 0.25, Y 0.00") == 0,
		"accessibility value follows joystick state");
	iChanged = 0;
	iRet = xuiVirtualJoystickReset(pJoystick, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && iChanged == 0, "silent reset stays silent");

	iRet = xuiVirtualJoystickClearChannels(pJoystick, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear channels");
	iRet = xuiVirtualJoystickReset(pJoystick, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && !tJoyState.bActive && tJoyState.iSource == XUI_VIRTUAL_JOYSTICK_SOURCE_NONE, "reset state");
	XUI_TEST_CHECK(xuiVirtualJoystickSetMetrics(pJoystick, 70.0f, 46.0f, 0.0f) == XUI_OK &&
		xuiVirtualJoystickSetValue(pJoystick, 0.05f, 0.0f, 0) == XUI_OK,
		"prepare deadzone metric update");
	XUI_TEST_CHECK(xuiVirtualJoystickSetMetrics(pJoystick, 70.0f, 46.0f, 0.10f) == XUI_OK &&
		xuiVirtualJoystickGetState(pJoystick, &tJoyState) == XUI_OK &&
		!tJoyState.bActive && __xuiVirtualJoystickNear(tJoyState.fMagnitude, 0.0f),
		"deadzone metric update immediately normalizes current value");
	XUI_TEST_CHECK(xuiVirtualJoystickSetMetrics(pJoystick, 70.0f, 46.0f, 0.05f) == XUI_OK,
		"restore pointer test metrics");

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
	memset(&tKeyEvent, 0, sizeof(tKeyEvent));
	tKeyEvent.iSize = sizeof(tKeyEvent);
	tKeyEvent.iType = XUI_EVENT_KEY_DOWN;
	tKeyEvent.pTarget = pChild;
	tKeyEvent.iKey = XUI_KEY_ESCAPE;
	iRet = xuiDispatchEvent(pContext, &tKeyEvent);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == pJoystick &&
		iRootEscapeCount == 1,
		"descendant escape bubbles without cancelling joystick");
	iRet = xuiInputPointerDownEx(pContext, 8, XUI_POINTER_TYPE_TOUCH,
		tWorld.fX + tBase.fX + tBase.fW * 0.5f,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "second touch down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == pJoystick &&
		xuiGetPointerCaptureEx(pContext, 8, XUI_POINTER_TYPE_TOUCH) == NULL,
		"second touch cannot steal joystick");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && tJoyState.bActive && tJoyState.iPointerId == 7,
		"second touch preserves active pointer");
	iRet = xuiSetPointerCaptureEx(pContext, 8, XUI_POINTER_TYPE_TOUCH, pJoystick);
	XUI_TEST_CHECK(iRet == XUI_OK, "capture unrelated pointer");
	iRet = xuiInputPointerUpEx(pContext, 8, XUI_POINTER_TYPE_TOUCH,
		tWorld.fX + tBase.fX + tBase.fW * 0.5f,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "captured unrelated touch up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == pJoystick &&
		xuiGetPointerCaptureEx(pContext, 8, XUI_POINTER_TYPE_TOUCH) == NULL,
		"captured unrelated touch release preserves active capture");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && tJoyState.bActive && tJoyState.iPointerId == 7,
		"captured unrelated touch release preserves active pointer state");
	iRet = xuiInputPointerUpEx(pContext, 7, XUI_POINTER_TYPE_TOUCH, tWorld.fX + tBase.fX + tBase.fW, tWorld.fY + tBase.fY + tBase.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "touch up input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCaptureEx(pContext, 7, XUI_POINTER_TYPE_TOUCH) == NULL, "touch release");
	iRet = xuiVirtualJoystickGetState(pJoystick, &tJoyState);
	XUI_TEST_CHECK(iRet == XUI_OK && !tJoyState.bActive, "touch release state");

	memset(&tKeyEvent, 0, sizeof(tKeyEvent));
	tKeyEvent.iSize = sizeof(tKeyEvent);
	tKeyEvent.iType = XUI_EVENT_KEY_DOWN;
	tKeyEvent.pTarget = pJoystick;
	tKeyEvent.iKey = XUI_KEY_ESCAPE;
	iRet = xuiDispatchEvent(pContext, &tKeyEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && iRootEscapeCount == 2,
		"inactive escape bubbles to parent");

	iRet = xuiInputPointerDown(pContext,
		tWorld.fX + tBase.fX + tBase.fW,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "mouse down before lost button");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pJoystick,
		"mouse capture before lost button");
	iRet = xuiInputPointerMove(pContext,
		tWorld.fX + tBase.fX + tBase.fW * 0.75f,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "mouse move without held button");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		xuiVirtualJoystickGetState(pJoystick, &tJoyState) == XUI_OK && !tJoyState.bActive,
		"lost mouse button cancels joystick capture");

	XUI_TEST_CHECK(xuiInputPointerDown(pContext,
		tWorld.fX + tBase.fX + tBase.fW,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == pJoystick,
		"capture before ancestor disable");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == NULL &&
		xuiVirtualJoystickGetState(pJoystick, &tJoyState) == XUI_OK && !tJoyState.bActive,
		"ancestor disable cancels joystick capture");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 1) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK,
		"restore ancestor enabled state");

	XUI_TEST_CHECK(xuiInputPointerDown(pContext,
		tWorld.fX + tBase.fX + tBase.fW,
		tWorld.fY + tBase.fY + tBase.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == pJoystick,
		"capture before ancestor hide");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pRoot, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == NULL &&
		xuiVirtualJoystickGetState(pJoystick, &tJoyState) == XUI_OK && !tJoyState.bActive,
		"ancestor hide cancels joystick capture");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pRoot, 1) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK,
		"restore ancestor visibility");

	iRet = xuiVirtualJoystickSetMetrics(pJoystick, 64.0f, 44.0f, 0.10f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set metrics");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare before idempotence");
	iDirtyFlags = xuiWidgetGetDirtyFlags(pJoystick);
	iRet = xuiVirtualJoystickSetMetrics(pJoystick, 64.0f, 44.0f, 0.10f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pJoystick) == iDirtyFlags,
		"same metrics are idempotent");
	iRet = xuiVirtualJoystickUseBuiltinAtlas(pJoystick, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiVirtualJoystickGetUseBuiltinAtlas(pJoystick) == 0, "disable atlas");
	iRet = xuiVirtualJoystickSetColors(pJoystick, XUI_COLOR_RGBA(30, 40, 52, 130), XUI_COLOR_RGBA(40, 120, 210, 180), XUI_COLOR_RGBA(230, 238, 248, 240), XUI_COLOR_RGBA(255, 255, 255, 255), XUI_COLOR_RGBA(72, 154, 255, 120), XUI_COLOR_RGBA(72, 154, 255, 180), XUI_COLOR_RGBA(150, 160, 170, 120));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");
	iRet = __xuiVirtualJoystickRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "fallback render");
	iRet = xuiTestSurfaceCreate(&tState, &pCustomSurface, 16, 16,
		XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pCustomSurface != NULL,
		"custom joystick surface create");
	XUI_TEST_CHECK(xuiVirtualJoystickSetSurface(pJoystick,
		XUI_VIRTUAL_JOYSTICK_PART_BASE, pCustomSurface,
		(xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f}) == XUI_OK &&
		xuiVirtualJoystickSetSurface(pJoystick,
			XUI_VIRTUAL_JOYSTICK_PART_KNOB, pCustomSurface,
			(xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f}) == XUI_OK &&
		xuiVirtualJoystickSetValue(pJoystick, 0.5f, 0.0f, 0) == XUI_OK,
		"set normal custom surfaces for active state");
	XUI_TEST_CHECK(__xuiVirtualJoystickRender(pContext, pTarget) == XUI_OK,
		"prime active custom surface cache");
	pCache = xuiWidgetGetCacheSurface(pJoystick, xuiWidgetGetStateId(pJoystick));
	XUI_TEST_CHECK(pCache != NULL, "active custom surface cache");
	xuiTestSurfaceReset(pCache);
	XUI_TEST_CHECK(xuiWidgetInvalidate(pJoystick,
		XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) == XUI_OK &&
		__xuiVirtualJoystickRender(pContext, pTarget) == XUI_OK,
		"render active custom surface fallback");
	tLastSrc = xuiTestSurfaceGetLastSrc(pCache);
	XUI_TEST_CHECK(tLastSrc.fX == 0.0f && tLastSrc.fY == 0.0f &&
		tLastSrc.fW == 16.0f && tLastSrc.fH == 16.0f,
		"active state reuses normal custom surface and resolves full source rect");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.onChange = __xuiVirtualJoystickDestroyOnChange;
	tDesc.pChangeUser = &iDestroyChanges;
	iRet = xuiVirtualJoystickCreate(pContext, &pLifecycleJoystick, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pLifecycleJoystick != NULL, "lifecycle joystick create");
	iRet = xuiWidgetAddChild(pRoot, pLifecycleJoystick);
	XUI_TEST_CHECK(iRet == XUI_OK, "lifecycle joystick add");
	iRet = xuiVirtualJoystickSetValue(pLifecycleJoystick, 0.5f, 0.0f, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyChanges == 1 &&
		xuiVirtualJoystickGetChangeCount(pLifecycleJoystick) == 0,
		"change callback may destroy joystick");
	pLifecycleJoystick = NULL;

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pCustomSurface != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pCustomSurface);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_virtual_joystick_test passed\n");
	return 0;
}
