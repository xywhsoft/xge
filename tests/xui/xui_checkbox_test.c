#include "xui.h"
#include "xui_test_proxy.h"

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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCheck;
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_checkbox_desc_t tDesc;
	xui_vec2_t tMeasured;
	xui_rect_t tRect;
	xui_rect_t tLastSrc;
	xui_rect_i_t tFullRect;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCheck = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	iChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
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

	xuiWidgetSetRect(pCheck, (xui_rect_t){0.0f, 0.0f, 160.0f, 32.0f});
	xuiWidgetMeasure(pCheck, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 18.0f && tMeasured.fY >= 24.0f, "measure includes text");
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
	XUI_TEST_CHECK(xuiCheckBoxGetChecked(pCheck) == 1 && iChanged == 2, "keyboard space toggles");
	iRet = xuiCheckBoxSetChecked(pCheck, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiCheckBoxGetChecked(pCheck) == 1, "set checked");

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
	printf("xui_checkbox_test passed\n");
	return 0;
}
