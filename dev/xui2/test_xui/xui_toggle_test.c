#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_toggle_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiToggleRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static void __xuiToggleChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiToggleClick(xui_context pContext, xui_widget pWidget)
{
	xui_rect_t tRect;
	float fX;
	float fY;
	int iRet;

	tRect = xuiWidgetGetWorldRect(pWidget);
	fX = tRect.fX + 16.0f;
	fY = tRect.fY + tRect.fH * 0.5f;
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
	xui_widget pToggle;
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_font pFont;
	xui_toggle_desc_t tDesc;
	xui_vec2_t tMeasured;
	xui_rect_t tTrack;
	xui_rect_t tThumb;
	xui_rect_t tInner;
	xui_rect_t tSrc;
	xui_rect_t tLastSrc;
	xui_rect_i_t tFullRect;
	int iChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pToggle = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	pFont = NULL;
	iChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 260.0f, 100.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "toggle", 6, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 100.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Enable feature";
	tDesc.bChecked = 1;
	iRet = xuiToggleCreate(pContext, &pToggle, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pToggle != NULL, "toggle create");
	iRet = xuiWidgetAddChild(pRoot, pToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "add toggle");
	xuiWidgetSetRect(pToggle, (xui_rect_t){0.0f, 0.0f, 180.0f, 34.0f});
	XUI_TEST_CHECK(strcmp(xuiToggleGetText(pToggle), "Enable feature") == 0, "text set from desc");
	XUI_TEST_CHECK(xuiToggleGetChecked(pToggle) == 1, "checked set from desc");
	XUI_TEST_CHECK(xuiToggleGetTrackWidth(pToggle) == 38.0f, "default track width");
	XUI_TEST_CHECK(xuiToggleGetTrackHeight(pToggle) == 22.0f, "default track height");
	XUI_TEST_CHECK(xuiToggleGetThumbSize(pToggle) == 14.0f, "default thumb size");
	XUI_TEST_CHECK(xuiToggleGetGap(pToggle) == 8.0f, "default gap");

	xuiWidgetMeasure(pToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 40.0f && tMeasured.fY >= 24.0f, "measure includes text");
	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 100, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	tFullRect = (xui_rect_i_t){0, 0, 260, 100};
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render default toggle");
	tTrack = xuiToggleGetTrackRect(pToggle);
	tThumb = xuiToggleGetThumbRect(pToggle);
	XUI_TEST_CHECK(tTrack.fW == 38.0f && tTrack.fH == 22.0f, "track rect");
	XUI_TEST_CHECK(tThumb.fW == 14.0f && tThumb.fH == 14.0f && tThumb.fX > tTrack.fX, "checked thumb rect");
	iRet = xuiToggleSetInnerText(pToggle, "OFF", "ON");
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text");
	XUI_TEST_CHECK(strcmp(xuiToggleGetUncheckedText(pToggle), "OFF") == 0, "get unchecked text");
	XUI_TEST_CHECK(strcmp(xuiToggleGetCheckedText(pToggle), "ON") == 0, "get checked text");
	iRet = xuiToggleSetInnerTextColor(pToggle, XUI_COLOR_RGBA(90, 108, 125, 255), XUI_COLOR_RGBA(255, 255, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text color");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, 7.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text metrics");
	xuiWidgetMeasure(pToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 160.0f, "measure includes inner text width");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render inner text toggle");
	tTrack = xuiToggleGetTrackRect(pToggle);
	tThumb = xuiToggleGetThumbRect(pToggle);
	tInner = xuiToggleGetInnerTextRect(pToggle);
	XUI_TEST_CHECK(tTrack.fW >= 54.0f, "inner text expands track");
	XUI_TEST_CHECK(tInner.fW > 0.0f && tInner.fX >= tTrack.fX && (tInner.fX + tInner.fW) <= tThumb.fX, "checked inner text rect");
	iRet = xuiToggleSetInnerText(pToggle, NULL, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetUncheckedText(pToggle)[0] == '\0' && xuiToggleGetCheckedText(pToggle)[0] == '\0', "clear inner text");

	iRet = xuiToggleSetChecked(pToggle, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 0, "set unchecked");
	XUI_TEST_CHECK((xuiToggleGetState(pToggle) & XUI_TOGGLE_STATE_CHECKED) == 0, "unchecked state");
	iRet = xuiToggleSetTrackSize(pToggle, 44.0f, 24.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetTrackWidth(pToggle) == 44.0f && xuiToggleGetTrackHeight(pToggle) == 24.0f, "set track size");
	iRet = xuiToggleSetThumbSize(pToggle, 15.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetThumbSize(pToggle) == 15.0f, "set thumb size");
	iRet = xuiToggleSetGap(pToggle, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetGap(pToggle) == 10.0f, "set gap");
	iRet = xuiToggleSetText(pToggle, "Use custom atlas");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToggleGetText(pToggle), "Use custom atlas") == 0, "set text");
	iRet = xuiToggleSetChange(pToggle, __xuiToggleChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");

	iRet = __xuiToggleClick(pContext, pToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer click");
	XUI_TEST_CHECK(xuiToggleGetChecked(pToggle) == 1 && iChanged == 1, "pointer click toggles");
	iRet = xuiToggleSetChecked(pToggle, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 0, "reset before keyboard");
	iRet = xuiSetFocusWidget(pContext, pToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus toggle");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch key down");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch key up");
	XUI_TEST_CHECK(xuiToggleGetChecked(pToggle) == 1 && iChanged == 2, "keyboard space toggles");
	iRet = xuiToggleSetColors(pToggle, XUI_COLOR_RGBA(24, 166, 113, 255), XUI_COLOR_RGBA(215, 230, 224, 255), XUI_COLOR_RGBA(194, 222, 211, 255), XUI_COLOR_RGBA(24, 166, 113, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");

	iRet = xuiTestSurfaceCreate(&tState, &pAtlas, 96, 24, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL, "custom atlas create");
	tSrc = (xui_rect_t){48.0f, 0.0f, 48.0f, 24.0f};
	iRet = xuiToggleUseBuiltinAtlas(pToggle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetUseBuiltinAtlas(pToggle) == 1, "enable builtin atlas flag");
	iRet = xuiToggleSetIndicatorSurface(pToggle, pAtlas, (xui_rect_t){0.0f, 0.0f, 48.0f, 24.0f}, pAtlas, tSrc);
	XUI_TEST_CHECK(iRet == XUI_OK, "set indicator surface");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render atlas toggle");
	tLastSrc = tSrc;
	XUI_TEST_CHECK(__xuiToggleRectEq(tLastSrc, 48.0f, 0.0f, 48.0f, 24.0f), "checked atlas rect");

cleanup:
	if ( pTarget != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	}
	if ( pAtlas != NULL ) {
		tState.tProxy.surfaceDestroy(&tState.tProxy, pAtlas);
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
	printf("xui_toggle_test passed\n");
	return 0;
}
