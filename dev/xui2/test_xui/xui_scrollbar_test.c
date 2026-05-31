#include "xui.h"
#include "xui_test_proxy.h"

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
	xui_surface pTarget;
	xui_surface pCache;
	xui_scrollbar_desc_t tDesc;
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
	float fThumbRadius;
	float fButtonSize;
	float fHoldValue;
	int iHoldChanged;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pBar = NULL;
	pTarget = NULL;
	pCache = NULL;
	iChanged = 0;
	iFailed = 0;
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
	iRet = xuiScrollBarCreate(pContext, &pBar, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pBar != NULL, "scrollbar create");
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
	iRet = xuiScrollBarGetMetrics(pBar, &fThickness, &fMinThumbSize, &fThumbRadius, &fButtonSize);
	XUI_TEST_CHECK(iRet == XUI_OK && fThickness == 0.0f && fMinThumbSize == 18.0f && fThumbRadius == -1.0f && fButtonSize == 0.0f, "default metrics");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 240, 220, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
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
	XUI_TEST_CHECK(__xuiScrollBarNear(xuiScrollBarGetValue(pBar), 50.0f) && iChanged == 4, "wheel page step");

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
	iRet = __xuiScrollBarUpdateFrame(pContext, 0.20f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) > 55.0f && iChanged > 1, "repeat button repeats after delay");
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

	iRet = xuiScrollBarSetMode(pBar, XUI_SCROLLBAR_MODE_COMPACT);
	XUI_TEST_CHECK(iRet == XUI_OK, "set compact mode");
	iRet = xuiScrollBarSetButtonMode(pBar, XUI_SCROLLBAR_BUTTONS_OFF);
	XUI_TEST_CHECK(iRet == XUI_OK, "set buttons off");
	iRet = xuiScrollBarSetOrientation(pBar, XUI_ORIENTATION_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK, "set horizontal");
	iRet = xuiScrollBarSetMetrics(pBar, 6.0f, 20.0f, -1.0f, 0.0f);
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
	fStartY = 166.0f + tThumb.fY + tThumb.fH * 0.5f;
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

	iRet = xuiScrollBarSetRange(pBar, 100.0f, 0.0f, -10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "normalize range");
	iRet = xuiScrollBarSetValue(pBar, 200.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "clamp value");
	iRet = xuiScrollBarGetRange(pBar, &fMin, &fMax, &fPage);
	XUI_TEST_CHECK(iRet == XUI_OK && fMin == 0.0f && fMax == 100.0f && fPage == 0.0f && xuiScrollBarGetValue(pBar) == 100.0f, "range normalization");
	iRet = xuiWidgetSetEnabled(pBar, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	XUI_TEST_CHECK((xuiScrollBarGetState(pBar) & XUI_WIDGET_STATE_DISABLED) != 0, "disabled state");
	iRet = __xuiScrollBarClick(pContext, 20.0f, 172.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiScrollBarGetValue(pBar) == 100.0f && xuiGetPointerCapture(pContext) == NULL, "disabled ignores pointer");

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
