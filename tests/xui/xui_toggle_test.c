#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
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

static xui_draw_surface_proc g_xuiToggleDrawSurface;
static int g_iToggleDrawSurfaceCount;

static int __xuiToggleDrawSurface(xui_proxy pProxy, xui_draw_context pDraw,
	xui_surface pSurface, xui_rect_t tSrc, xui_rect_t tDst,
	uint32_t iColor, uint32_t iFlags)
{
	g_iToggleDrawSurfaceCount++;
	return g_xuiToggleDrawSurface(pProxy, pDraw, pSurface, tSrc, tDst,
		iColor, iFlags);
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

static void __xuiToggleDestroyParentChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;
	xui_widget pParent;

	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	pParent = xuiWidgetGetParent(pWidget);
	if ( pParent != NULL ) xuiWidgetDestroy(pParent);
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
	xui_widget pRtlToggle;
	xui_widget pChild;
	xui_widget pDestroyHost;
	xui_widget pDestroyToggle;
	xui_widget pInvalidWidget;
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_font pFont;
	xui_toggle_desc_t tDesc;
	xui_toggle_desc_t tRtlDesc;
	xui_vec2_t tMeasured;
	xui_rect_t tTrack;
	xui_rect_t tThumb;
	xui_rect_t tText;
	xui_rect_t tInner;
	xui_rect_t tSrc;
	xui_rect_t tStoredUncheckedSrc;
	xui_rect_t tStoredCheckedSrc;
	xui_rect_t tLastSrc;
	xui_rect_i_t tFullRect;
	xui_accessible_node_t tAccessible;
	xui_event_t tEvent;
	char sLongText[192];
	char sLongUncheckedText[160];
	char sLongCheckedText[176];
	int iChanged;
	int iDestroyChanged;
	int iFailed;
	int iRet;
	float fInnerPadding;
	float fInnerGap;
	uint32_t iUncheckedTextColor;
	uint32_t iCheckedTextColor;
	xui_surface pStoredUncheckedSurface;
	xui_surface pStoredCheckedSurface;
	xui_toggle_desc_t tInvalidDesc;

	pContext = NULL;
	pRoot = NULL;
	pToggle = NULL;
	pRtlToggle = NULL;
	pChild = NULL;
	pDestroyHost = NULL;
	pDestroyToggle = NULL;
	pInvalidWidget = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	pFont = NULL;
	iChanged = 0;
	iDestroyChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);
	g_xuiToggleDrawSurface = tState.tProxy.drawSurface;
	tState.tProxy.drawSurface = __xuiToggleDrawSurface;

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
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE;
	tDesc.iUncheckedTextColor = XUI_COLOR_RGBA(18, 52, 86, 255);
	tDesc.iCheckedTextColor = XUI_COLOR_RGBA(101, 67, 33, 255);
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
	iRet = xuiToggleGetInnerTextColor(pToggle, &iUncheckedTextColor, &iCheckedTextColor);
	XUI_TEST_CHECK(iRet == XUI_OK && iUncheckedTextColor == tDesc.iUncheckedTextColor &&
	               iCheckedTextColor == tDesc.iCheckedTextColor,
	               "descriptor inner text colors");
	iRet = xuiToggleGetInnerTextMetrics(pToggle, &fInnerPadding, &fInnerGap);
	XUI_TEST_CHECK(iRet == XUI_OK && fInnerPadding == 6.0f && fInnerGap == 2.0f,
	               "default inner text metrics");
	iRet = xuiWidgetSetAccessibleName(pToggle, "Feature switch");
	XUI_TEST_CHECK(iRet == XUI_OK, "set accessible name");
	iRet = xuiWidgetSetAccessibleDescription(pToggle, "Controls the optional feature");
	XUI_TEST_CHECK(iRet == XUI_OK, "set accessible description");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pToggle, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_SWITCH &&
	               strcmp(tAccessible.sName, "Feature switch") == 0 &&
	               strcmp(tAccessible.sDescription, "Controls the optional feature") == 0,
	               "toggle accessibility metadata");

	iRet = xuiWidgetMeasureContent(pToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 158.0f && tMeasured.fY == 24.0f,
		"toggle intrinsic content measure");
	iRet = xuiWidgetMeasureContent(pToggle, (xui_vec2_t){30.0f, 10.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 30.0f && tMeasured.fY == 10.0f,
		"toggle content measure respects finite constraints");
	xuiWidgetMeasure(pToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 40.0f && tMeasured.fY >= 24.0f, "measure includes text");
	XUI_TEST_CHECK((tState.iLastTextMeasureFlags & XUI_TEXT_SHAPE_RTL) == 0,
		"ltr toggle uses ltr shaping");

	memset(&tRtlDesc, 0, sizeof(tRtlDesc));
	tRtlDesc.iSize = sizeof(tRtlDesc);
	tRtlDesc.sText = "RTL switch";
	tRtlDesc.sUncheckedText = "OFF";
	tRtlDesc.sCheckedText = "ON";
	tRtlDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_RTL;
	tRtlDesc.bChecked = 1;
	iRet = xuiToggleCreate(pContext, &pRtlToggle, &tRtlDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pRtlToggle != NULL, "rtl toggle create");
	iRet = xuiWidgetAddChild(pRoot, pRtlToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "add rtl toggle");
	xuiWidgetSetRect(pRtlToggle, (xui_rect_t){0.0f, 50.0f, 180.0f, 34.0f});
	xuiWidgetMeasure(pRtlToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK((tState.iLastTextMeasureFlags & XUI_TEXT_SHAPE_RTL) != 0,
	               "rtl toggle uses rtl shaping");
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
	tTrack = xuiToggleGetTrackRect(pRtlToggle);
	tThumb = xuiToggleGetThumbRect(pRtlToggle);
	tText = xuiToggleGetTextRect(pRtlToggle);
	tInner = xuiToggleGetInnerTextRect(pRtlToggle);
	XUI_TEST_CHECK(tTrack.fX >= tText.fX + tText.fW,
	               "rtl toggle mirrors track after outer text");
	XUI_TEST_CHECK(tThumb.fX < tTrack.fX + tTrack.fW * 0.5f,
	               "rtl checked thumb is on leading side");
	XUI_TEST_CHECK(tInner.fX >= tThumb.fX + tThumb.fW &&
	               tInner.fX + tInner.fW <= tTrack.fX + tTrack.fW,
	               "rtl checked inner text is opposite thumb");
	iRet = xuiToggleSetChecked(pRtlToggle, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "rtl toggle unchecked");
	tThumb = xuiToggleGetThumbRect(pRtlToggle);
	tInner = xuiToggleGetInnerTextRect(pRtlToggle);
	XUI_TEST_CHECK(tThumb.fX >= tTrack.fX + tTrack.fW * 0.5f,
	               "rtl unchecked thumb is on trailing side");
	XUI_TEST_CHECK(tInner.fX >= tTrack.fX && tInner.fX + tInner.fW <= tThumb.fX,
	               "rtl unchecked inner text is opposite thumb");
	XUI_TEST_CHECK((xuiToggleGetTextFlags(pToggle) & XUI_TEXT_RTL) == 0,
	               "get ltr toggle text flags");
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetTextFlags(pToggle,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_RTL);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               (xuiToggleGetTextFlags(pToggle) & XUI_TEXT_RTL) != 0 &&
	               (xuiWidgetGetDirtyFlags(pToggle) & XUI_WIDGET_DIRTY_LAYOUT) != 0,
	               "runtime rtl toggle flags invalidate layout");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render runtime rtl toggle");
	tTrack = xuiToggleGetTrackRect(pToggle);
	tText = xuiToggleGetTextRect(pToggle);
	XUI_TEST_CHECK(tTrack.fX >= tText.fX + tText.fW,
	               "runtime rtl toggle mirrors geometry");
	iRet = xuiToggleSetTextFlags(pToggle,
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet == XUI_OK ) iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore runtime ltr toggle");
	iRet = xuiToggleSetInnerText(pToggle, "OFF", "ON");
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text");
	XUI_TEST_CHECK(strcmp(xuiToggleGetUncheckedText(pToggle), "OFF") == 0, "get unchecked text");
	XUI_TEST_CHECK(strcmp(xuiToggleGetCheckedText(pToggle), "ON") == 0, "get checked text");
	iRet = xuiToggleSetInnerText(pToggle, xuiToggleGetCheckedText(pToggle),
	                            xuiToggleGetUncheckedText(pToggle));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToggleGetUncheckedText(pToggle), "ON") == 0 &&
	               strcmp(xuiToggleGetCheckedText(pToggle), "OFF") == 0,
	               "cross-aliased inner text swap");
	iRet = xuiToggleSetInnerText(pToggle, "OFF", "ON");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore swapped inner text");
	iRet = xuiToggleSetInnerText(pToggle, xuiToggleGetUncheckedText(pToggle) + 1,
	                            xuiToggleGetCheckedText(pToggle) + 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToggleGetUncheckedText(pToggle), "FF") == 0 &&
	               strcmp(xuiToggleGetCheckedText(pToggle), "N") == 0,
	               "self-aliased inner text update");
	memset(sLongUncheckedText, 'U', sizeof(sLongUncheckedText) - 1u);
	sLongUncheckedText[sizeof(sLongUncheckedText) - 1u] = '\0';
	memset(sLongCheckedText, 'K', sizeof(sLongCheckedText) - 1u);
	sLongCheckedText[sizeof(sLongCheckedText) - 1u] = '\0';
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetInnerText(pToggle, sLongUncheckedText, sLongCheckedText);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               strcmp(xuiToggleGetUncheckedText(pToggle), sLongUncheckedText) == 0 &&
	               strcmp(xuiToggleGetCheckedText(pToggle), sLongCheckedText) == 0 &&
	               (xuiWidgetGetDirtyFlags(pToggle) & XUI_WIDGET_DIRTY_LAYOUT) != 0u,
	               "long inner text grows shared buffers");
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetInnerText(pToggle, sLongUncheckedText, sLongCheckedText);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pToggle) == 0u,
	               "idempotent inner text stays clean");
	iRet = xuiToggleSetInnerText(pToggle, "OFF", "ON");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore inner text");
	iRet = xuiToggleSetInnerTextColor(pToggle, XUI_COLOR_RGBA(90, 108, 125, 255), XUI_COLOR_RGBA(255, 255, 255, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text color");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, 7.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set inner text metrics");
	xuiTestProxyResetTextCounters(&tState);
	xuiWidgetMeasure(pToggle, (xui_vec2_t){300.0f, 100.0f}, &tMeasured);
	XUI_TEST_CHECK(tMeasured.fX > 160.0f, "measure includes inner text width");
	XUI_TEST_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 3,
	               "measure shapes outer and two inner labels once");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render inner text toggle");
	tTrack = xuiToggleGetTrackRect(pToggle);
	tThumb = xuiToggleGetThumbRect(pToggle);
	tInner = xuiToggleGetInnerTextRect(pToggle);
	XUI_TEST_CHECK(tTrack.fW >= 54.0f, "inner text expands track");
	XUI_TEST_CHECK(tInner.fW > 0.0f && tInner.fX >= tTrack.fX && (tInner.fX + tInner.fW) <= tThumb.fX, "checked inner text rect");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, 0.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "set zero inner text metrics");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render zero inner text metrics");
	tTrack = xuiToggleGetTrackRect(pToggle);
	tInner = xuiToggleGetInnerTextRect(pToggle);
	XUI_TEST_CHECK(tInner.fX == tTrack.fX, "zero inner padding is preserved");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, 7.0f, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore inner text metrics");
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
	memset(sLongText, 'T', sizeof(sLongText) - 1u);
	sLongText[sizeof(sLongText) - 1u] = '\0';
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetText(pToggle, sLongText);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToggleGetText(pToggle), sLongText) == 0 &&
	               (xuiWidgetGetDirtyFlags(pToggle) & XUI_WIDGET_DIRTY_LAYOUT) != 0u,
	               "long outer text grows shared buffer");
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetText(pToggle, sLongText);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pToggle) == 0u,
	               "idempotent outer text stays clean");
	iRet = xuiToggleSetText(pToggle, xuiToggleGetText(pToggle) + 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(xuiToggleGetText(pToggle)) == sizeof(sLongText) - 2u,
	               "grown self-aliased outer text update");
	iRet = xuiToggleSetText(pToggle, "Use custom atlas");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiToggleGetText(pToggle), "Use custom atlas") == 0,
	               "restore outer text");
	iRet = xuiToggleSetChange(pToggle, __xuiToggleChanged, &iChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set change callback");

	iRet = __xuiToggleClick(pContext, pToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer click");
	XUI_TEST_CHECK(xuiToggleGetChecked(pToggle) == 1 && iChanged == 1, "pointer click toggles");
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pToggle,
		"first touch owns toggle press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH, 20, 16, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 1 &&
		iChanged == 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pToggle &&
		(xuiToggleGetState(pToggle) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot click or release toggle press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		20, 16, XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 0 &&
		iChanged == 2 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch commits toggle click once");
	iRet = xuiToggleSetChecked(pToggle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore toggle before descendant click");
	iRet = xuiWidgetCreate(pContext, &pChild);
	XUI_TEST_CHECK(iRet == XUI_OK && pChild != NULL, "child create");
	iRet = xuiWidgetAddChild(pToggle, pChild);
	XUI_TEST_CHECK(iRet == XUI_OK, "child add");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_CLICK;
	tEvent.pTarget = pChild;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 1 && iChanged == 2,
	               "descendant click does not toggle switch");
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
	XUI_TEST_CHECK(xuiToggleGetChecked(pToggle) == 1 && iChanged == 3, "keyboard space toggles");
	iRet = xuiToggleSetChecked(pToggle, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "reset before blur");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down before blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiToggleGetState(pToggle) & XUI_WIDGET_STATE_ACTIVE) != 0,
	               "active before blur");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during keyboard press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiToggleGetState(pToggle) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key up after blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 0 && iChanged == 3,
	               "blurred keyboard press does not toggle");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               (xuiToggleGetState(pToggle) & XUI_WIDGET_STATE_DISABLED) != 0,
	               "ancestor disable reaches toggle state");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pToggle;
	tEvent.iKey = XUI_KEY_SPACE;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	tEvent.iType = XUI_EVENT_KEY_UP;
	if ( iRet == XUI_OK ) iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetChecked(pToggle) == 0 && iChanged == 3,
	               "ancestor-disabled toggle rejects injected keyboard activation");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK &&
	               (xuiToggleGetState(pToggle) & XUI_WIDGET_STATE_DISABLED) == 0,
	               "ancestor re-enable restores toggle state");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, NAN, 3.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan inner padding");
	iRet = xuiToggleSetTrackSize(pToggle, INFINITY, 24.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiToggleGetTrackWidth(pToggle) == 44.0f,
	               "reject infinite track width");
	iRet = xuiToggleSetThumbSize(pToggle, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiToggleGetThumbSize(pToggle) == 15.0f,
	               "reject nan thumb size");
	iRet = xuiToggleSetGap(pToggle, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiToggleGetGap(pToggle) == 10.0f,
	               "reject infinite gap");
	iRet = xuiToggleSetTrackSize(pToggle, FLT_MAX, 24.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiToggleGetTrackWidth(pToggle) == 44.0f,
	               "reject unbounded track width");
	iRet = xuiToggleSetThumbSize(pToggle, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiToggleGetThumbSize(pToggle) == 15.0f,
	               "reject unbounded thumb size");
	iRet = xuiToggleSetInnerTextMetrics(pToggle, 7.0f, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
	               "reject unbounded inner text gap");
	iRet = xuiToggleSetColors(pToggle, XUI_COLOR_RGBA(24, 166, 113, 255), XUI_COLOR_RGBA(215, 230, 224, 255), XUI_COLOR_RGBA(194, 222, 211, 255), XUI_COLOR_RGBA(24, 166, 113, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set colors");

	iRet = xuiTestSurfaceCreate(&tState, &pAtlas, 96, 24, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL, "custom atlas create");
	tSrc = (xui_rect_t){48.0f, 0.0f, 48.0f, 24.0f};
	iRet = xuiToggleUseBuiltinAtlas(pToggle, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiToggleGetUseBuiltinAtlas(pToggle) == 1, "enable builtin atlas flag");
	iRet = xuiToggleSetIndicatorSurface(pToggle, pAtlas, (xui_rect_t){0.0f, 0.0f, 48.0f, 24.0f}, pAtlas, tSrc);
	XUI_TEST_CHECK(iRet == XUI_OK, "set indicator surface");
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiToggleSetIndicatorSurface(pToggle, pAtlas,
		(xui_rect_t){-0.0f, 0.0f, 48.0f, 24.0f}, pAtlas, tSrc);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pToggle) == 0,
	               "equivalent source rectangles are a no-op");
	iRet = xuiToggleGetIndicatorSurface(pToggle, &pStoredUncheckedSurface,
		&tStoredUncheckedSrc, &pStoredCheckedSurface, &tStoredCheckedSrc);
	XUI_TEST_CHECK(iRet == XUI_OK, "get stored indicator surfaces");
	iRet = xuiToggleSetIndicatorSurface(pToggle, pAtlas,
		(xui_rect_t){0.0f, 0.0f, -1.0f, 24.0f}, pAtlas, tSrc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject negative source width");
	iRet = xuiToggleSetIndicatorSurface(pToggle, pAtlas,
		(xui_rect_t){FLT_MAX, 0.0f, 48.0f, 24.0f}, pAtlas, tSrc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject unbounded source coordinate");
	iRet = xuiToggleGetIndicatorSurface(pToggle, &pStoredUncheckedSurface,
		&tStoredUncheckedSrc, &pStoredCheckedSurface, &tStoredCheckedSrc);
	XUI_TEST_CHECK(iRet == XUI_OK && pStoredUncheckedSurface == pAtlas &&
	               pStoredCheckedSurface == pAtlas &&
	               __xuiToggleRectEq(tStoredUncheckedSrc, 0.0f, 0.0f, 48.0f, 24.0f) &&
	               __xuiToggleRectEq(tStoredCheckedSrc, 48.0f, 0.0f, 48.0f, 24.0f),
	               "invalid source rectangles preserve prior state");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render atlas toggle");
	tLastSrc = tSrc;
	XUI_TEST_CHECK(__xuiToggleRectEq(tLastSrc, 48.0f, 0.0f, 48.0f, 24.0f), "checked atlas rect");
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "prepare borrowed indicator dependencies");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "consume prepared indicator caches");
	xuiWidgetClearDirty(pToggle, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pRoot, XUI_WIDGET_DIRTY_ALL);
	g_iToggleDrawSurfaceCount = 0;
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && g_iToggleDrawSurfaceCount == 0,
	               "unchanged borrowed indicator surface reuses toggle caches");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pAtlas,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && g_iToggleDrawSurfaceCount > 0,
	               "borrowed indicator generation rebuilds toggle caches");

	iRet = xuiWidgetCreate(pContext, &pDestroyHost);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyHost != NULL, "destroy host create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyHost);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy host add");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiToggleCreate(pContext, &pDestroyToggle, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyToggle != NULL, "destroy toggle create");
	iRet = xuiWidgetAddChild(pDestroyHost, pDestroyToggle);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy toggle add");
	iRet = xuiToggleSetChange(pDestroyToggle, __xuiToggleDestroyParentChanged, &iDestroyChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback setup");
	iRet = xuiWidgetPerformAccessibleAction(pDestroyToggle, 1, XUI_ACCESSIBLE_ACTION_TOGGLE, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyChanged == 1,
	               "change callback may destroy toggle parent without stale access");
	pDestroyHost = NULL;
	pDestroyToggle = NULL;

	tInvalidDesc = tDesc;
	tInvalidDesc.fGap = NAN;
	pInvalidWidget = pRoot;
	iRet = xuiToggleCreate(pContext, &pInvalidWidget, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
	               "invalid toggle create clears output");
	pInvalidWidget = pRoot;
	iRet = xuiToggleCreate(NULL, &pInvalidWidget, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
	               "invalid context clears toggle output");

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
