#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_radio_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

static int __xuiRadioRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static void __xuiRadioChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;

	(void)pWidget;
	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static void __xuiRadioGroupChanged(xui_widget pGroup, xui_widget pSelected, int iSelectedIndex, void* pUser)
{
	int* pCount;

	(void)pGroup;
	(void)pSelected;
	(void)iSelectedIndex;
	pCount = (int*)pUser;
	if ( pCount != NULL ) {
		(*pCount)++;
	}
}

static int __xuiRadioClick(xui_context pContext, xui_widget pWidget)
{
	xui_rect_t tRect;
	float fX;
	float fY;
	int iRet;

	tRect = xuiWidgetGetWorldRect(pWidget);
	fX = tRect.fX + tRect.fW * 0.5f;
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
	xui_widget pGroup;
	xui_widget pRadio[3];
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_font pFont;
	xui_radio_group_desc_t tGroupDesc;
	xui_radio_desc_t tDesc;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tLastSrc;
	int iRadioChanged;
	int iGroupChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pGroup = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	pFont = NULL;
	memset(pRadio, 0, sizeof(pRadio));
	iRadioChanged = 0;
	iGroupChanged = 0;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = xuiInputViewport(pContext, 260.0f, 150.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "radio", 5, 16.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 260.0f, 150.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tGroupDesc, 0, sizeof(tGroupDesc));
	tGroupDesc.iSize = sizeof(tGroupDesc);
	tGroupDesc.iOrientation = XUI_RADIO_GROUP_VERTICAL;
	tGroupDesc.iSelectedIndex = 1;
	tGroupDesc.fGap = 4.0f;
	iRet = xuiRadioGroupCreate(pContext, &pGroup, &tGroupDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pGroup != NULL, "radio group create");
	xuiWidgetSetRect(pGroup, (xui_rect_t){0.0f, 0.0f, 220.0f, 120.0f});
	iRet = xuiWidgetAddChild(pRoot, pGroup);
	XUI_TEST_CHECK(iRet == XUI_OK, "add group");
	iRet = xuiRadioGroupSetChange(pGroup, __xuiRadioGroupChanged, &iGroupChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "set group change");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sText = "Small";
	iRet = xuiRadioGroupAddOption(pGroup, &pRadio[0], &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pRadio[0] != NULL, "add option 0");
	xuiRadioSetChange(pRadio[0], __xuiRadioChanged, &iRadioChanged);
	tDesc.sText = "Medium";
	iRet = xuiRadioGroupAddOption(pGroup, &pRadio[1], &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pRadio[1] != NULL, "add option 1");
	xuiRadioSetChange(pRadio[1], __xuiRadioChanged, &iRadioChanged);
	tDesc.sText = "Large";
	iRet = xuiRadioGroupAddOption(pGroup, &pRadio[2], &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pRadio[2] != NULL, "add option 2");
	xuiRadioSetChange(pRadio[2], __xuiRadioChanged, &iRadioChanged);

	XUI_TEST_CHECK(strcmp(xuiRadioGetText(pRadio[1]), "Medium") == 0, "text from desc");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 1, "selected index from desc");
	XUI_TEST_CHECK(xuiRadioGetChecked(pRadio[1]) == 1 && xuiRadioGetChecked(pRadio[0]) == 0, "initial group selection");
	XUI_TEST_CHECK(xuiRadioGetIndicatorSize(pRadio[0]) == 18.0f, "default indicator size");
	XUI_TEST_CHECK(xuiRadioGetGap(pRadio[0]) == 8.0f, "default radio gap");
	XUI_TEST_CHECK(xuiRadioGroupGetGap(pGroup) == 4.0f, "group gap");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tRect = xuiWidgetGetRect(pRadio[0]);
	if ( !(tRect.fW > 20.0f && tRect.fH >= 24.0f) ) {
		printf("radio layout rect: %.1f %.1f %.1f %.1f\n", tRect.fX, tRect.fY, tRect.fW, tRect.fH);
	}
	XUI_TEST_CHECK(tRect.fW > 20.0f && tRect.fH >= 24.0f, "radio layout");

	iRet = __xuiRadioClick(pContext, pRadio[0]);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer click option 0");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 0, "click selects option 0");
	XUI_TEST_CHECK(xuiRadioGetChecked(pRadio[0]) == 1 && xuiRadioGetChecked(pRadio[1]) == 0, "click unchecks sibling");
	XUI_TEST_CHECK(iGroupChanged == 1 && iRadioChanged >= 1, "click callbacks");

	iRet = xuiSetFocusWidget(pContext, pRadio[2]);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus option 2");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space down");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "dispatch space up");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 2 && xuiRadioGetChecked(pRadio[2]) == 1, "space selects option 2");

	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedRadio(pGroup) == pRadio[1], "set selected index");
	iRet = xuiRadioGroupSetOrientation(pGroup, XUI_RADIO_GROUP_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetOrientation(pGroup) == XUI_RADIO_GROUP_HORIZONTAL, "set orientation");
	iRet = xuiRadioGroupSetGap(pGroup, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetGap(pGroup) == 10.0f, "set group gap");
	iRet = xuiRadioSetIndicatorSize(pRadio[1], 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetIndicatorSize(pRadio[1]) == 20.0f, "set radio size");
	iRet = xuiRadioSetGap(pRadio[1], 11.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetGap(pRadio[1]) == 11.0f, "set radio gap");
	iRet = xuiRadioSetColors(pRadio[1], XUI_COLOR_RGBA(24, 166, 113, 255), XUI_COLOR_RGBA(156, 181, 170, 255), XUI_COLOR_RGBA(72, 196, 146, 255), XUI_COLOR_RGBA(24, 166, 113, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set radio colors");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 150, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiBuiltinAssetGetAtlas(pContext, &pAtlas);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL, "builtin atlas available");
	iRet = xuiRadioUseBuiltinAtlas(pRadio[1], 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetUseBuiltinAtlas(pRadio[1]) == 1, "enable radio atlas");
	xuiBuiltinAssetGetRect("radio_checked", &tRect);
	xuiRadioSetIndicatorSurface(pRadio[1], pAtlas, tRect, pAtlas, tRect);
	tFullRect = (xui_rect_i_t){0, 0, 260, 150};
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render atlas radio");
	tLastSrc = tRect;
	XUI_TEST_CHECK(__xuiRadioRectEq(tLastSrc, 104.0f, 34.0f, 34.0f, 34.0f), "checked radio atlas rect");

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
	printf("xui_radio_test passed\n");
	return 0;
}
