#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_checkbox_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_checkbox_change_state_t {
	int iCount;
	int iLastState;
} xui_checkbox_change_state_t;

static int __xuiCheckBoxRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static void __xuiCheckBoxChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static void __xuiCheckBoxDestroyParentChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;
	xui_widget pParent;

	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	pParent = xuiWidgetGetParent(pWidget);
	if ( pParent != NULL ) xuiWidgetDestroy(pParent);
}

static void __xuiCheckBoxStateChanged(xui_widget pWidget, int iCheckState, void* pUser)
{
	xui_checkbox_change_state_t* pState;

	(void)pWidget;
	pState = (xui_checkbox_change_state_t*)pUser;
	if ( pState == NULL ) return;
	pState->iCount++;
	pState->iLastState = iCheckState;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCheck;
	xui_widget pChild;
	xui_widget pDestroyHost;
	xui_widget pDestroyCheck;
	xui_widget pInvalidWidget;
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_font pFont;
	xui_checkbox_desc_t tDesc;
	xui_checkbox_desc_t tInvalidDesc;
	xui_checkbox_change_state_t tChangeState;
	xui_accessible_node_t tAccessible;
	xui_event_t tEvent;
	xui_vec2_t tMeasured;
	xui_rect_t tRect;
	xui_rect_t tLastSrc;
	xui_rect_i_t tFullRect;
	int iChanged;
	int iDestroyChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCheck = NULL;
	pChild = NULL;
	pDestroyHost = NULL;
	pDestroyCheck = NULL;
	pInvalidWidget = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	pFont = NULL;
	iChanged = 0;
	iDestroyChanged = 0;
	iFailed = 0;
	memset(&tChangeState, 0, sizeof(tChangeState));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont,
		"checkbox", 8, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 220.0f, 80.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Accept";
	tDesc.bChecked = 1;
	iRet = xuiCheckBoxCreate(pContext, &pCheck, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCheck != NULL, "checkbox create");
	iRet = xuiWidgetAddChild(pRoot, pCheck);
	XUI_TEST_CHECK(iRet == XUI_OK, "add checkbox");
	XUI_TEST_CHECK(strcmp(xuiCheckBoxGetText(pCheck), "Accept") == 0, "text set from desc");
	XUI_TEST_CHECK(xuiCheckBoxGetChecked(pCheck) == 1, "checked set from desc");
	XUI_TEST_CHECK(xuiCheckBoxGetIndicatorSize(pCheck) == 18.0f, "default indicator size");
	XUI_TEST_CHECK(xuiCheckBoxGetGap(pCheck) == 8.0f, "default gap");
	iRet = xuiWidgetSetAccessibleName(pCheck, "Terms accepted");
	XUI_TEST_CHECK(iRet == XUI_OK, "set accessible name");
	iRet = xuiWidgetSetAccessibleDescription(pCheck, "Controls whether terms are accepted");
	XUI_TEST_CHECK(iRet == XUI_OK, "set accessible description");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pCheck, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_CHECKBOX &&
	               strcmp(tAccessible.sName, "Terms accepted") == 0 &&
	               strcmp(tAccessible.sDescription, "Controls whether terms are accepted") == 0,
	               "checkbox accessibility metadata");

	xuiWidgetSetRect(pCheck, (xui_rect_t){0.0f, 0.0f, 160.0f, 32.0f});
	xuiWidgetMeasure(pCheck, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 18.0f && tMeasured.fY >= 24.0f, "measure includes text");
	xuiWidgetClearDirty(pCheck, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiCheckBoxSetTextFlags(pCheck,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_RTL);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pCheck) & XUI_WIDGET_DIRTY_LAYOUT) != 0,
		"checkbox rtl change invalidates layout");
	iRet = xuiWidgetMeasureContent(pCheck, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tState.iLastTextMeasureFlags & XUI_TEXT_SHAPE_RTL) != 0,
		"checkbox rtl reaches text measurement");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 220, 80, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	tFullRect = (xui_rect_i_t){0, 0, 220, 80};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render default checkbox");
	xuiTestSurfaceReset(pTarget);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "render prepare");

	iRet = xuiCheckBoxSetChecked(pCheck, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 0, "set unchecked");
	XUI_TEST_CHECK((xuiCheckBoxGetState(pCheck) & XUI_CHECKBOX_STATE_CHECKED) == 0, "unchecked state");
	iRet = xuiCheckBoxSetIndicatorSize(pCheck, 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetIndicatorSize(pCheck) == 20.0f, "set size");
	iRet = xuiCheckBoxSetGap(pCheck, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetGap(pCheck) == 10.0f, "set gap");
	iRet = xuiCheckBoxSetText(pCheck, "Use builtin atlas");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCheckBoxGetText(pCheck), "Use builtin atlas") == 0, "set text");
	iRet = xuiCheckBoxSetText(pCheck, xuiCheckBoxGetText(pCheck) + 4);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiCheckBoxGetText(pCheck), "builtin atlas") == 0,
	               "self-aliased text update");
	iRet = xuiCheckBoxSetChange(pCheck, __xuiCheckBoxChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");
	iRet = xuiInputPointerMove(pContext, 20.0f, 16.0f, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer move");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch pointer move");
	iRet = xuiInputPointerDown(pContext, 20.0f, 16.0f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch pointer down");
	iRet = xuiInputPointerUp(pContext, 20.0f, 16.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch pointer up");
	XUI_TEST_CHECK(xuiCheckBoxGetChecked(pCheck) == 1 && iChanged == 1, "pointer click toggles");
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pCheck,
		"first touch owns checkbox press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, 20, 16, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 1 &&
		iChanged == 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pCheck &&
		(xuiCheckBoxGetState(pCheck) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot click or release checkbox press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 0 &&
		iChanged == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch commits checkbox click once");
	iRet = xuiCheckBoxSetChecked(pCheck, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore checkbox before descendant click");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	iRet = xuiWidgetAddChild(pCheck, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "child add");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_CLICK;
	tEvent.pTarget = pChild;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 1 && iChanged == 2,
	               "descendant click does not toggle checkbox");
	iRet = xuiCheckBoxSetChecked(pCheck, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 0, "reset before keyboard");
	iRet = xuiSetFocusWidget(pContext, pCheck);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus checkbox");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch key down");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch key up");
	XUI_TEST_CHECK(xuiCheckBoxGetChecked(pCheck) == 1 && iChanged == 3, "keyboard space toggles");
	iRet = xuiCheckBoxSetChecked(pCheck, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset before blur");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down before blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiCheckBoxGetState(pCheck) & XUI_WIDGET_STATE_ACTIVE) != 0,
	               "active before blur");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during keyboard press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiCheckBoxGetState(pCheck) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key up after blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 0 && iChanged == 3,
	               "blurred keyboard press does not toggle");
	iRet = xuiCheckBoxSetChecked(pCheck, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 1, "set checked");
	iRet = xuiCheckBoxSetIndicatorSize(pCheck, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiCheckBoxGetIndicatorSize(pCheck) == 20.0f,
	               "reject nan indicator size");
	iRet = xuiCheckBoxSetGap(pCheck, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiCheckBoxGetGap(pCheck) == 10.0f,
	               "reject infinite gap");

	iRet = xuiBuiltinAssetGetAtlas(pContext, &pAtlas);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL, "builtin atlas available");
	iRet = xuiCheckBoxUseBuiltinAtlas(pCheck, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetUseBuiltinAtlas(pCheck) == 1, "enable builtin atlas");
	xuiBuiltinAssetGetRect("checkbox_checked", &tRect);
	xuiCheckBoxSetIndicatorSurface(pCheck, pAtlas, tRect, pAtlas, tRect);
	XUI_TEST_CHECK(xuiCheckBoxGetUseBuiltinAtlas(pCheck) == 1, "builtin atlas flag preserved");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render atlas checkbox");
	tLastSrc = tRect;
	XUI_TEST_CHECK(__xuiCheckBoxRectEq(tLastSrc, 34.0f, 34.0f, 32.0f, 32.0f), "checked atlas rect");

	iRet = xuiCheckBoxSetCheckState(pCheck, XUI_CHECKBOX_CHECK_STATE_MIXED);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               xuiCheckBoxGetCheckState(pCheck) == XUI_CHECKBOX_CHECK_STATE_MIXED &&
	               xuiCheckBoxGetChecked(pCheck) == 0 &&
	               (xuiCheckBoxGetState(pCheck) & XUI_CHECKBOX_STATE_MIXED) != 0u,
	               "set mixed state");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pCheck, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(tAccessible.sValue, "mixed") == 0 &&
	               (tAccessible.iState & XUI_ACCESSIBLE_STATE_MIXED) != 0u &&
	               (tAccessible.iState & XUI_ACCESSIBLE_STATE_CHECKED) == 0u,
	               "mixed state accessibility");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render default mixed indicator with atlas enabled");
	iRet = xuiCheckBoxSetMixedIndicatorSurface(pCheck, pAtlas, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "set mixed indicator surface");
	pAtlas = NULL;
	memset(&tLastSrc, 0, sizeof(tLastSrc));
	iRet = xuiCheckBoxGetMixedIndicatorSurface(pCheck, &pAtlas, &tLastSrc);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL &&
	               __xuiCheckBoxRectEq(tLastSrc, 34.0f, 34.0f, 32.0f, 32.0f),
	               "get mixed indicator surface");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render custom mixed indicator");
	iRet = xuiCheckBoxSetCheckState(pCheck, 3);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
	               xuiCheckBoxGetCheckState(pCheck) == XUI_CHECKBOX_CHECK_STATE_MIXED,
	               "reject invalid check state");
	iRet = xuiCheckBoxSetChange(pCheck, __xuiCheckBoxStateChanged, &tChangeState);
	XUI_TEST_CHECK(iRet == XUI_OK, "set tri-state callback");
	iRet = xuiCheckBoxSetCheckState(pCheck, XUI_CHECKBOX_CHECK_STATE_UNCHECKED);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset tri-state sequence");
	iRet = xuiCheckBoxSetAutoThreeState(pCheck, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetAutoThreeState(pCheck) == 1,
	               "enable automatic three-state cycle");
	iRet = xuiWidgetPerformAccessibleAction(pCheck, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetCheckState(pCheck) == XUI_CHECKBOX_CHECK_STATE_CHECKED,
	               "three-state cycle reaches checked");
	iRet = xuiWidgetPerformAccessibleAction(pCheck, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetCheckState(pCheck) == XUI_CHECKBOX_CHECK_STATE_MIXED,
	               "three-state cycle reaches mixed");
	iRet = xuiWidgetPerformAccessibleAction(pCheck, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetCheckState(pCheck) == XUI_CHECKBOX_CHECK_STATE_UNCHECKED &&
	               tChangeState.iCount == 3 &&
	               tChangeState.iLastState == XUI_CHECKBOX_CHECK_STATE_UNCHECKED,
	               "three-state cycle returns to unchecked and reports exact states");

	iRet = xuiWidgetCreate(pContext, &pDestroyHost);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyHost != NULL, "destroy host create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyHost);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy host add");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiCheckBoxCreate(pContext, &pDestroyCheck, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyCheck != NULL, "destroy checkbox create");
	iRet = xuiWidgetAddChild(pDestroyHost, pDestroyCheck);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy checkbox add");
	iRet = xuiCheckBoxSetChange(pDestroyCheck, __xuiCheckBoxDestroyParentChanged, &iDestroyChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback setup");
	iRet = xuiWidgetPerformAccessibleAction(pDestroyCheck, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyChanged == 1,
	               "change callback may destroy checkbox parent without stale access");
	pDestroyHost = NULL;
	pDestroyCheck = NULL;

	tInvalidDesc = tDesc;
	tInvalidDesc.fGap = NAN;
	pInvalidWidget = pRoot;
	iRet = xuiCheckBoxCreate(pContext, &pInvalidWidget, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
	               "invalid checkbox create clears output");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_checkbox_test passed\n");
	return 0;
}
