#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_color_picker_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_color_picker_test_change_t {
	int iCount;
	uint32_t iColor;
} xui_color_picker_test_change_t;

typedef struct xui_color_picker_test_destroy_t {
	xui_widget* ppWidget;
	int iCount;
} xui_color_picker_test_destroy_t;

typedef struct xui_color_picker_test_event_destroy_t {
	xui_widget* ppWidget;
	int iEventType;
	int iCount;
} xui_color_picker_test_event_destroy_t;

static void __xuiColorPickerChanged(xui_widget pWidget, uint32_t iColor, void* pUser)
{
	xui_color_picker_test_change_t* pChange;

	(void)pWidget;
	pChange = (xui_color_picker_test_change_t*)pUser;
	pChange->iCount++;
	pChange->iColor = iColor;
}

static void __xuiColorPickerDestroyOnChange(xui_widget pWidget, uint32_t iColor, void* pUser)
{
	xui_color_picker_test_destroy_t* pDestroy = (xui_color_picker_test_destroy_t*)pUser;

	(void)iColor;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
	if ( pDestroy->ppWidget != NULL ) *pDestroy->ppWidget = NULL;
}

static int __xuiColorPickerDestroyVictimOnEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_color_picker_test_event_destroy_t* pDestroy = (xui_color_picker_test_event_destroy_t*)pUser;
	xui_widget pVictim;

	(void)pWidget;
	if ( (pDestroy == NULL) || (pEvent == NULL) || (pEvent->iType != pDestroy->iEventType) ||
	     (pDestroy->ppWidget == NULL) || (*pDestroy->ppWidget == NULL) ) return XUI_OK;
	pVictim = *pDestroy->ppWidget;
	*pDestroy->ppWidget = NULL;
	pDestroy->iCount++;
	xuiWidgetDestroy(pVictim);
	return XUI_OK;
}

static int __xuiColorPickerRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 520, 560};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiColorPickerDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiColorPickerDispatchClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiColorPickerDispatchText(xui_context pContext, const char* sText)
{
	int iRet;
	int i;

	if ( sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	for ( i = 0; sText[i] != '\0'; i++ ) {
		iRet = xuiInputText(pContext, (uint32_t)(unsigned char)sText[i]);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiColorPickerDispatchKeyMods(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiColorPickerDispatchKey(xui_context pContext, int iKey)
{
	return __xuiColorPickerDispatchKeyMods(pContext, iKey, 0u);
}

static int __xuiColorPickerDispatchDrag(xui_context pContext, float fX0, float fY0, float fX1, float fY1)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX0, fY0, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX1, fY1, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiColorPickerClickPanelRect(xui_context pContext, xui_widget pPanel, xui_rect_t tRect)
{
	xui_rect_t tWorld;

	tWorld = xuiWidgetGetWorldRect(pPanel);
	return __xuiColorPickerDispatchDown(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
}

static int __xuiColorPickerClickPanelRectFull(xui_context pContext, xui_widget pPanel, xui_rect_t tRect)
{
	xui_rect_t tWorld;

	tWorld = xuiWidgetGetWorldRect(pPanel);
	return __xuiColorPickerDispatchClick(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_color_picker_test_change_t tChange;
	xui_color_picker_test_destroy_t tDestroy;
	xui_color_picker_test_event_destroy_t tFocusDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pFocusTarget;
	xui_widget pFocusDestroyPicker;
	xui_widget pFocusDestroyPanel;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_surface pTarget;
	xui_font pFont;
	xui_color_picker_desc_t tDesc;
	xui_accessible_node_t tAccessible;
	xui_accessible_value_t tAccessibleValue;
	xui_event_t tEvent;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_vec2_t tMeasured;
	uint32_t arrPalette[5];
	uint32_t arrTooMany[XUI_COLOR_PICKER_PALETTE_CAPACITY + 1];
	uint32_t iTarget;
	uint32_t iColor;
	int iR;
	int iG;
	int iB;
	int iA;
	int iCountBefore;
	int iFailed;
	int iRet;
	float fPopupWidth;
	float fPopupHeight;
	float fBorderWidth;

	pContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pFocusTarget = NULL;
	pFocusDestroyPicker = NULL;
	pFocusDestroyPanel = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
	memset(&tDestroy, 0, sizeof(tDestroy));
	memset(&tFocusDestroy, 0, sizeof(tFocusDestroy));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "colorpicker", 11, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 520.0f, 560.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 520.0f, 560.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	arrPalette[0] = XUI_COLOR_RGBA(0, 0, 0, 255);
	arrPalette[1] = XUI_COLOR_RGBA(255, 255, 255, 255);
	arrPalette[2] = XUI_COLOR_RGBA(244, 201, 54, 255);
	arrPalette[3] = XUI_COLOR_RGBA(46, 124, 214, 255);
	arrPalette[4] = XUI_COLOR_RGBA(132, 86, 209, 128);
	memset(arrTooMany, 0, sizeof(arrTooMany));
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fPopupWidth = NAN;
	pPicker = (xui_widget)(uintptr_t)1;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPicker == NULL, "nan descriptor rejected and output cleared");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrPalette = arrTooMany;
	tDesc.iPaletteCount = XUI_COLOR_PICKER_PALETTE_CAPACITY + 1;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPicker == NULL, "oversized descriptor palette rejected");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fPopupWidth = 320.0f;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPicker == NULL, "undersized descriptor popup rejected");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.fPopupWidth = XUI_LAYOUT_UNBOUNDED * 2.0f;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPicker == NULL, "oversized descriptor popup rejected");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iPopupPlacement = 99;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pPicker == NULL, "invalid descriptor placement rejected");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iColor = XUI_COLOR_RGBA(0x11, 0x22, 0x33, 0x80);
	tDesc.arrPalette = arrPalette;
	tDesc.iPaletteCount = 5;
	tDesc.bAlphaEnabled = 1;
	tDesc.pFont = pFont;
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPicker != NULL, "color picker create");
	iRet = xuiColorPickerSetChange(pPicker, __xuiColorPickerChanged, &tChange);
	XUI_TEST_CHECK(iRet == XUI_OK, "change callback");
	xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 190.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "add picker");
	iRet = xuiWidgetCreate(pContext, &pFocusTarget);
	XUI_TEST_CHECK(iRet == XUI_OK && pFocusTarget != NULL, "focus target create");
	XUI_TEST_CHECK(xuiWidgetSetFocusable(pFocusTarget, 1) == XUI_OK &&
		xuiWidgetSetRect(pFocusTarget, (xui_rect_t){470.0f, 520.0f, 20.0f, 20.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pFocusTarget) == XUI_OK, "focus target attach");
	iRet = xuiWidgetMeasureContent(pPicker,
		(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 176.0f && tMeasured.fY == 30.0f,
		"unbounded content measure");
	iRet = xuiWidgetMeasureContent(pPicker, (xui_vec2_t){60.0f, 12.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 60.0f && tMeasured.fY == 12.0f,
		"content measure respects tight constraints");
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 12.0f, 8.0f}) == XUI_OK,
		"set tiny owner rect");
	{
		xui_rect_t tSwatch = xuiColorPickerGetSwatchRect(pPicker);
		xui_rect_t tButton = xuiColorPickerGetButtonRect(pPicker);
		xui_rect_t tText = xuiColorPickerGetTextRect(pPicker);
		XUI_TEST_CHECK(tSwatch.fX >= 0 && tSwatch.fY >= 0 && tSwatch.fW >= 0 && tSwatch.fH >= 0 &&
			tSwatch.fX + tSwatch.fW <= 12 && tSwatch.fY + tSwatch.fH <= 8 &&
			tButton.fX >= 0 && tButton.fY >= 0 && tButton.fW >= 0 && tButton.fH >= 0 &&
			tButton.fX + tButton.fW <= 12 && tButton.fY + tButton.fH <= 8 &&
			tText.fX >= 0 && tText.fY >= 0 && tText.fW >= 0 && tText.fH >= 0 &&
			tText.fX + tText.fW <= 12 && tText.fY + tText.fH <= 8,
			"tiny owner geometry stays inside bounds");
	}
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 190.0f, 30.0f}) == XUI_OK,
		"restore picker rect");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 560, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiColorPickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	iRet = xuiColorPickerGetMetrics(pPicker, &fBorderWidth);
	XUI_TEST_CHECK(iRet == XUI_OK, "get border width for idempotence");
	xuiWidgetClearDirty(pPicker, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiColorPickerSetMetrics(pPicker, fBorderWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pPicker) == 0u,
		"same metrics do not invalidate picker");

	XUI_TEST_CHECK(xuiColorPickerGetPaletteCount(pPicker) == 5, "palette count");
	XUI_TEST_CHECK(xuiColorPickerGetAlphaEnabled(pPicker) == 1, "alpha enabled");
	XUI_TEST_CHECK(strcmp(xuiColorPickerGetHex(pPicker), "#11223380") == 0, "initial hex");
	iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
	XUI_TEST_CHECK(iRet == XUI_OK && iR == 0x11 && iG == 0x22 && iB == 0x33 && iA == 0x80, "rgba get");
	iRet = xuiColorPickerSetPalette(pPicker, arrTooMany, XUI_COLOR_PICKER_PALETTE_CAPACITY + 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiColorPickerGetPaletteCount(pPicker) == 5, "oversized palette preserves state");
	iRet = xuiColorPickerSetPopupSize(pPicker, NAN, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "nan popup width rejected");
	iRet = xuiColorPickerSetPopupSize(pPicker, 320.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "undersized popup width rejected");
	iRet = xuiColorPickerSetPopupSize(pPicker, XUI_LAYOUT_UNBOUNDED * 2.0f, 0.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "oversized popup width rejected");
	iRet = xuiColorPickerSetMetrics(pPicker, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "infinite border rejected");
	iRet = xuiColorPickerSetMetrics(pPicker, XUI_LAYOUT_UNBOUNDED * 2.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "oversized border rejected");
	iRet = xuiColorPickerGetPopupSize(pPicker, &fPopupWidth, &fPopupHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fPopupWidth == 432.0f && fPopupHeight == 260.0f, "invalid popup settings preserve size");
	iRet = xuiColorPickerSetHex(pPicker, "#445566");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiColorPickerGetHex(pPicker), "#445566FF") == 0, "rgb hex normalizes alpha");
	iRet = xuiColorPickerSetHex(pPicker, "#11223380");
	XUI_TEST_CHECK(iRet == XUI_OK, "set alpha hex");

	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_COMBOBOX &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_READONLY) != 0u &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_COLLAPSED) != 0u &&
		strcmp(tAccessible.sValue, "#11223380") == 0,
		"color picker accessibility node");
	iRet = xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker), "accessibility expands popup");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_EXPANDED) != 0u,
		"accessibility reports expanded state");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL) == XUI_OK && !xuiColorPickerIsOpen(pPicker),
		"accessibility collapses popup");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_TEXT;
	tAccessibleValue.sText = "#0A0B0C7F";
	iCountBefore = tChange.iCount;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_OK &&
		xuiColorPickerGetColor(pPicker) == XUI_COLOR_RGBA(0x0A, 0x0B, 0x0C, 0x7F) &&
		tChange.iCount == iCountBefore + 1,
		"accessibility sets color and notifies");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_OK &&
		tChange.iCount == iCountBefore + 1,
		"accessibility same color is idempotent");
	tAccessibleValue.sText = "not-a-color";
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiColorPickerGetColor(pPicker) == XUI_COLOR_RGBA(0x0A, 0x0B, 0x0C, 0x7F),
		"accessibility rejects invalid color without mutation");
	XUI_TEST_CHECK(xuiColorPickerSetHex(pPicker, "#11223380") == XUI_OK,
		"restore color after accessibility actions");
	memset(&tChange, 0, sizeof(tChange));

	tRect = xuiColorPickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(tRect.fW >= 24.0f && tRect.fH > 20.0f, "button rect");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiColorPickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	XUI_TEST_CHECK(xuiTestProxyGetMeshDrawCount(&tState) >= 3, "smooth color gradients use shared meshes");
	pPopup = xuiColorPickerGetPopupWidget(pPicker);
	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	XUI_TEST_CHECK(pPopup != NULL && pPanel != NULL && xuiGetFocusWidget(pContext) == pPanel, "popup panel focus");
	{
		xui_rect_t tSv = xuiColorPickerGetSvRect(pPicker);
		xui_rect_t tHex = xuiColorPickerGetHexRect(pPicker);
		xui_rect_t tOld = xuiColorPickerGetOldRect(pPicker);
		xui_rect_t tNew = xuiColorPickerGetNewRect(pPicker);
		XUI_TEST_CHECK(tHex.fY + tHex.fH == tSv.fY + tSv.fH, "hex aligns with color panel bottom");
		XUI_TEST_CHECK(tOld.fX == tHex.fX && tNew.fX + tNew.fW == tHex.fX + tHex.fW, "swatches align with control columns");
	}
	{
		xui_rect_t tSv = xuiColorPickerGetSvRect(pPicker);
		xui_rect_t tPanelWorld = xuiWidgetGetWorldRect(pPanel);
		iRet = xuiInputPointerMove(pContext,
			tPanelWorld.fX + tSv.fX + tSv.fW * 0.5f,
			tPanelWorld.fY + tSv.fY + tSv.fH * 0.5f, 0);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK &&
			xuiColorPickerGetHoverPart(pPicker) == XUI_COLOR_PICKER_PART_SV,
			"panel pointer move sets hover part");
		memset(&tEvent, 0, sizeof(tEvent));
		tEvent.iSize = sizeof(tEvent);
		tEvent.iType = XUI_EVENT_POINTER_LEAVE;
		tEvent.pTarget = pPanel;
		iRet = xuiDispatchEvent(pContext, &tEvent);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker) &&
			xuiColorPickerGetHoverPart(pPicker) == XUI_COLOR_PICKER_PART_NONE,
			"panel leave clears stale hover state");
	}
	{
		xui_rect_t tSv = xuiColorPickerGetSvRect(pPicker);
		xui_rect_t tPanelWorld = xuiWidgetGetWorldRect(pPanel);
		iRet = xuiColorPickerSetColor(pPicker, XUI_COLOR_RGBA(0, 255, 0, 255));
		XUI_TEST_CHECK(iRet == XUI_OK, "set green before hsv persistence test");
		iRet = __xuiColorPickerDispatchClick(pContext,
			tPanelWorld.fX + tSv.fX,
			tPanelWorld.fY + tSv.fY);
		XUI_TEST_CHECK(iRet == XUI_OK, "desaturate green through sv panel");
		iRet = xuiColorPickerSetHex(pPicker, "#FFFFFF40");
		XUI_TEST_CHECK(iRet == XUI_OK, "alpha-only edit while desaturated");
		iRet = __xuiColorPickerDispatchClick(pContext,
			tPanelWorld.fX + tSv.fX + tSv.fW - 1.0f,
			tPanelWorld.fY + tSv.fY + 1.0f);
		XUI_TEST_CHECK(iRet == XUI_OK, "resaturate through sv panel");
		iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
		XUI_TEST_CHECK(iRet == XUI_OK && iR < 16 && iG > 240 && iB < 16 && iA == 0x40,
			"alpha-only edit preserves zero-saturation hue");
		iRet = xuiColorPickerSetColor(pPicker, XUI_COLOR_RGBA(0, 0, 0, 255));
		XUI_TEST_CHECK(iRet == XUI_OK, "set black before hue redraw test");
		iRet = __xuiColorPickerRender(pContext, pTarget);
		XUI_TEST_CHECK(iRet == XUI_OK, "render black before hue redraw test");
		tRect = xuiColorPickerGetHueRect(pPicker);
		iRet = xuiInputPointerWheel(pContext,
			(int)(tPanelWorld.fX + tRect.fX + tRect.fW * 0.5f),
			(int)(tPanelWorld.fY + tRect.fY + tRect.fH * 0.5f),
			0.0f, -10.0f, 0);
		XUI_TEST_CHECK(iRet == XUI_OK, "post hue wheel while black");
		iRet = xuiDispatchPendingEvents(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK &&
			(xuiWidgetGetDirtyFlags(pPanel) & XUI_WIDGET_DIRTY_CACHE) != 0u,
			"hue-only edit invalidates panel cache");

		iRet = xuiColorPickerSetColor(pPicker, XUI_COLOR_RGBA(0, 255, 0, 255));
		XUI_TEST_CHECK(iRet == XUI_OK, "set green before keyboard navigation");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard enters sv field");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_LEFT);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard adjusts saturation");
		iColor = xuiColorPickerGetColor(pPicker);
		XUI_TEST_CHECK(((iColor >> 24) & 0xffu) > 0u && ((iColor >> 16) & 0xffu) > 240u,
			"sv keyboard adjustment changes rendered color");
		iTarget = iColor;
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard advances to hue");
		iRet = __xuiColorPickerDispatchKeyMods(pContext, XUI_KEY_DOWN, XUI_MOD_SHIFT);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) != iTarget,
			"hue keyboard adjustment uses accelerated step");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard advances to old swatch");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) == XUI_COLOR_RGBA(0x11, 0x22, 0x33, 0x80),
			"keyboard restores old color");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard advances to red channel");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_RIGHT);
		XUI_TEST_CHECK(iRet == XUI_OK, "keyboard increments red channel");
		iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
		XUI_TEST_CHECK(iRet == XUI_OK && iR == 0x12 && iG == 0x22 && iB == 0x33 && iA == 0x80,
			"channel keyboard adjustment is exact");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB) == XUI_OK, "keyboard advances to green channel");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB) == XUI_OK, "keyboard advances to blue channel");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB) == XUI_OK, "keyboard advances to alpha channel");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB) == XUI_OK, "keyboard advances to hex field");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_TAB) == XUI_OK, "keyboard advances to palette");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) == arrPalette[0],
			"keyboard applies focused palette color");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_RIGHT) == XUI_OK, "keyboard moves palette focus");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) == arrPalette[1],
			"keyboard applies next palette color");
		iRet = __xuiColorPickerDispatchKeyMods(pContext, XUI_KEY_TAB, XUI_MOD_SHIFT);
		XUI_TEST_CHECK(iRet == XUI_OK, "shift-tab returns to hex field");
		XUI_TEST_CHECK(__xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER) == XUI_OK, "keyboard starts hex edit");
		XUI_TEST_CHECK(__xuiColorPickerDispatchText(pContext, "12345678") == XUI_OK, "keyboard hex text");
		iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
		XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiColorPickerGetHex(pPicker), "#12345678") == 0,
			"keyboard commits hex edit");
		iRet = __xuiColorPickerRender(pContext, pTarget);
		XUI_TEST_CHECK(iRet == XUI_OK, "render keyboard focus state");
		iRet = xuiColorPickerSetColor(pPicker, XUI_COLOR_RGBA(0x11, 0x22, 0x33, 0x80));
		XUI_TEST_CHECK(iRet == XUI_OK, "restore color after hsv tests");
		memset(&tChange, 0, sizeof(tChange));
	}
	XUI_TEST_CHECK((xuiColorPickerGetState(pPicker) & XUI_COLOR_PICKER_STATE_OPEN) != 0u, "open state");
	tRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tRect.fY >= 60.0f, "popup below owner");

	iTarget = arrPalette[2];
	iRet = __xuiColorPickerClickPanelRect(pContext, pPanel, xuiColorPickerGetPaletteRect(pPicker, 2));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker), "palette keeps popup open");
	XUI_TEST_CHECK(tChange.iCount == 1 && tChange.iColor == iTarget && xuiColorPickerGetColor(pPicker) == iTarget, "palette callback");
	iRet = __xuiColorPickerClickPanelRect(pContext, pPanel, xuiColorPickerGetOldRect(pPicker));
	XUI_TEST_CHECK(iRet == XUI_OK && tChange.iCount == 2 && xuiColorPickerGetColor(pPicker) == XUI_COLOR_RGBA(0x11, 0x22, 0x33, 0x80), "old color restore");

	tRect = xuiColorPickerGetAlphaRect(pPicker);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = __xuiColorPickerDispatchDrag(pContext,
		tWorld.fX + tRect.fX + 2.0f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		tWorld.fX + tRect.fX + tRect.fW * 0.25f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && tChange.iCount >= 3, "alpha drag callback");
	iColor = xuiColorPickerGetColor(pPicker);
	XUI_TEST_CHECK((iColor & 0xffu) >= 50u && (iColor & 0xffu) <= 80u, "alpha drag value");
	iCountBefore = tChange.iCount;
	iRet = __xuiColorPickerDispatchClick(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.75f,
		tWorld.fY + tRect.fY - 3.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "alpha knob-height hit target click");
	iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
	XUI_TEST_CHECK(iRet == XUI_OK && iA >= 185 && iA <= 200 && tChange.iCount > iCountBefore,
		"alpha track accepts pointer across visible knob height");

	iCountBefore = tChange.iCount;
	tRect = xuiColorPickerGetHexRect(pPicker);
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit click");
	iRet = __xuiColorPickerDispatchText(pContext, "ABCDEF40");
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit text");
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "repeat click keeps uncommitted hex text");
	iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit commit");
	XUI_TEST_CHECK(strcmp(xuiColorPickerGetHex(pPicker), "#ABCDEF40") == 0 && tChange.iCount > iCountBefore, "hex edit applies typed color");

	iCountBefore = tChange.iCount;
	tRect = xuiColorPickerGetOldRect(pPicker);
	tRect.fY = tRect.fY + tRect.fH + 6.0f;
	tRect.fW = 42.0f;
	tRect.fH = 22.0f;
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "component field click");
	iRet = __xuiColorPickerDispatchText(pContext, "12");
	XUI_TEST_CHECK(iRet == XUI_OK, "component field text");
	iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "component field commit");
	iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
	XUI_TEST_CHECK(iRet == XUI_OK && iR == 12 && iG == 0xCD && iB == 0xEF && iA == 0x40 && tChange.iCount > iCountBefore, "component edit applies decimal channel");

	tRect = xuiColorPickerGetHexRect(pPicker);
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit for invalid text");
	iRet = __xuiColorPickerDispatchText(pContext, "Z");
	XUI_TEST_CHECK(iRet == XUI_OK, "invalid hex input ignored");
	iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "unchanged hex commits after invalid input");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker), "escape closes");

	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pFocusTarget) == XUI_OK,
		"focus target before closed panel event");
	iColor = xuiColorPickerGetColor(pPicker);
	tRect = xuiColorPickerGetAlphaRect(pPicker);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pPanel;
	tEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = tWorld.fX + tRect.fX + tRect.fW * 0.75f;
	tEvent.fY = tWorld.fY + tRect.fY + tRect.fH * 0.5f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) == iColor &&
		xuiGetFocusWidget(pContext) == pFocusTarget,
		"closed panel ignores direct pointer event");
	iRet = xuiWidgetSetVisible(pPicker, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker), "hide picker");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pFocusTarget) == XUI_OK,
		"focus target before hidden owner event");
	tWorld = xuiWidgetGetWorldRect(pPicker);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.pTarget = pPicker;
	tEvent.iPointerType = XUI_POINTER_TYPE_MOUSE;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.iButtons = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = tWorld.fX + tWorld.fW * 0.5f;
	tEvent.fY = tWorld.fY + tWorld.fH * 0.5f;
	iRet = xuiDispatchEvent(pContext, &tEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker) &&
		xuiGetFocusWidget(pContext) == pFocusTarget,
		"hidden owner ignores direct pointer event");
	iRet = xuiWidgetSetVisible(pPicker, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore picker visibility");

	iRet = xuiWidgetSetEnabled(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker), "disabled does not open");
	XUI_TEST_CHECK((xuiColorPickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");
	iRet = xuiWidgetSetEnabled(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "re-enable picker");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable parent");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker) &&
		(xuiColorPickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u, "effective disabled parent blocks open");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable parent");

	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker), "reopen for capture cancellation");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "capture layout");
	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	tRect = xuiColorPickerGetAlphaRect(pPicker);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = xuiInputPointerDownEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + 2.0f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	iTarget = xuiColorPickerGetColor(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pPanel &&
		xuiColorPickerGetActivePart(pPicker) == XUI_COLOR_PICKER_PART_ALPHA,
		"color picker owner touch captures slider");
	iRet = xuiInputPointerDownEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + 2.0f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerMoveEx(pContext, 702u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + tRect.fX + tRect.fW - 2.0f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 702u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + tRect.fX + tRect.fW - 2.0f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) == iTarget &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == pPanel &&
		xuiGetPointerCaptureEx(pContext, 702u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot edit color or end owner capture");
	iRet = xuiInputPointerMoveEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.75f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f), XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerGetColor(pPicker) != iTarget,
		"color picker owner touch controls slider");
	iRet = xuiInputPointerUpEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.75f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 701u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiColorPickerGetActivePart(pPicker) == XUI_COLOR_PICKER_PART_NONE,
		"color picker owner touch releases slider capture");
	iRet = __xuiColorPickerDispatchDown(pContext, tWorld.fX + tRect.fX + 2.0f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPanel &&
		xuiColorPickerGetActivePart(pPicker) == XUI_COLOR_PICKER_PART_ALPHA, "alpha slider acquires capture with public part id");
	iRet = xuiColorPickerClose(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) != pPanel &&
		xuiColorPickerGetActivePart(pPicker) == XUI_COLOR_PICKER_PART_NONE, "close cancels hidden slider capture");

	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "open for destroy callback");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback layout");
	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	tDestroy.ppWidget = &pPicker;
	iRet = xuiColorPickerSetChange(pPicker, __xuiColorPickerDestroyOnChange, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback set");
	iRet = __xuiColorPickerClickPanelRect(pContext, pPanel, xuiColorPickerGetPaletteRect(pPicker, 1));
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 && pPicker == NULL, "change callback may destroy picker");

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = xuiColorPickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPicker != NULL, "create picker for edit destroy callback");
	tDestroy.ppWidget = &pPicker;
	xuiWidgetSetRect(pPicker, (xui_rect_t){260.0f, 32.0f, 190.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "add picker for edit destroy callback");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "open picker for edit destroy callback");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout picker for edit destroy callback");
	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, xuiColorPickerGetHexRect(pPicker));
	XUI_TEST_CHECK(iRet == XUI_OK, "begin edit before destroy callback");
	iRet = xuiColorPickerSetChange(pPicker, __xuiColorPickerDestroyOnChange, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "set edit destroy callback");
	iRet = __xuiColorPickerDispatchText(pContext, "01020304");
	XUI_TEST_CHECK(iRet == XUI_OK, "type edit before destroy callback");
	iRet = __xuiColorPickerClickPanelRect(pContext, pPanel, xuiColorPickerGetOldRect(pPicker));
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 && pPicker == NULL,
		"edit commit callback may destroy picker safely");

	iRet = xuiColorPickerCreate(pContext, &pFocusDestroyPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFocusDestroyPicker != NULL,
		"create picker for focus destroy callback");
	XUI_TEST_CHECK(xuiWidgetSetRect(pFocusDestroyPicker,
		(xui_rect_t){36.0f, 84.0f, 190.0f, 30.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pFocusDestroyPicker) == XUI_OK,
		"attach picker for focus destroy callback");
	pFocusDestroyPanel = xuiColorPickerGetPanelWidget(pFocusDestroyPicker);
	tFocusDestroy.ppWidget = &pFocusDestroyPicker;
	tFocusDestroy.iEventType = XUI_EVENT_FOCUS;
	XUI_TEST_CHECK(pFocusDestroyPanel != NULL &&
		xuiWidgetSetEventHandler(pFocusDestroyPanel, XUI_EVENT_FOCUS,
			__xuiColorPickerDestroyVictimOnEvent, &tFocusDestroy) == XUI_OK,
		"install focus destroy callback");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK && xuiSetFocusWidget(pContext, NULL) == XUI_OK,
		"prepare focus destroy callback");
	tWorld = xuiWidgetGetWorldRect(pFocusDestroyPicker);
	iRet = __xuiColorPickerDispatchDown(pContext,
		tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus destroy pointer dispatch returns safely");
	XUI_TEST_CHECK(tFocusDestroy.iCount == 1 && pFocusDestroyPicker == NULL,
		"focus callback destroys picker exactly once");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_color_picker_test passed\n");
	return 0;
}
