#include "xui.h"
#include "xui_test_proxy.h"

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

static void __xuiColorPickerChanged(xui_widget pWidget, uint32_t iColor, void* pUser)
{
	xui_color_picker_test_change_t* pChange;

	(void)pWidget;
	pChange = (xui_color_picker_test_change_t*)pUser;
	pChange->iCount++;
	pChange->iColor = iColor;
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

static int __xuiColorPickerDispatchKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
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
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_surface pTarget;
	xui_font pFont;
	xui_color_picker_desc_t tDesc;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	uint32_t arrPalette[5];
	uint32_t iTarget;
	uint32_t iColor;
	int iR;
	int iG;
	int iB;
	int iA;
	int iCountBefore;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tChange, 0, sizeof(tChange));
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

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 520, 560, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiColorPickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiColorPickerGetPaletteCount(pPicker) == 5, "palette count");
	XUI_TEST_CHECK(xuiColorPickerGetAlphaEnabled(pPicker) == 1, "alpha enabled");
	XUI_TEST_CHECK(strcmp(xuiColorPickerGetHex(pPicker), "#11223380") == 0, "initial hex");
	iRet = xuiColorPickerGetRGBA(pPicker, &iR, &iG, &iB, &iA);
	XUI_TEST_CHECK(iRet == XUI_OK && iR == 0x11 && iG == 0x22 && iB == 0x33 && iA == 0x80, "rgba get");
	iRet = xuiColorPickerSetHex(pPicker, "#445566");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiColorPickerGetHex(pPicker), "#445566FF") == 0, "rgb hex normalizes alpha");
	iRet = xuiColorPickerSetHex(pPicker, "#11223380");
	XUI_TEST_CHECK(iRet == XUI_OK, "set alpha hex");

	tRect = xuiColorPickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(tRect.fW >= 24.0f && tRect.fH > 20.0f, "button rect");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiColorPickerIsOpen(pPicker), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiColorPickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pPopup = xuiColorPickerGetPopupWidget(pPicker);
	pPanel = xuiColorPickerGetPanelWidget(pPicker);
	XUI_TEST_CHECK(pPopup != NULL && pPanel != NULL && xuiGetFocusWidget(pContext) == pPanel, "popup panel focus");
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
	tRect = xuiColorPickerGetHexRect(pPicker);
	iRet = __xuiColorPickerClickPanelRectFull(pContext, pPanel, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit click");
	iRet = __xuiColorPickerDispatchText(pContext, "ABCDEF40");
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit text");
	iRet = __xuiColorPickerDispatchKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "hex edit commit");
	XUI_TEST_CHECK(strcmp(xuiColorPickerGetHex(pPicker), "#ABCDEF40") == 0 && tChange.iCount > iCountBefore, "hex edit applies typed color");

	iCountBefore = tChange.iCount;
	tRect = xuiColorPickerGetHexRect(pPicker);
	tRect.fY = 84.0f;
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

	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker), "escape closes");

	iRet = xuiWidgetSetEnabled(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiColorPickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiColorPickerIsOpen(pPicker), "disabled does not open");
	XUI_TEST_CHECK((xuiColorPickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");

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
