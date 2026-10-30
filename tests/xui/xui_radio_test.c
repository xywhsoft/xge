#include "xui.h"
#include "xui_test_proxy.h"

#include <float.h>
#include <math.h>
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

static void __xuiRadioDestroyParentChanged(xui_widget pWidget, int bChecked, void* pUser)
{
	int* pCount;
	xui_widget pParent;

	(void)bChecked;
	pCount = (int*)pUser;
	if ( pCount != NULL ) (*pCount)++;
	pParent = xuiWidgetGetParent(pWidget);
	if ( pParent != NULL ) xuiWidgetDestroy(pParent);
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
	xui_widget pDestroyGroup;
	xui_widget pInvalidWidget;
	xui_widget pNonOption;
	xui_widget pRadio[3];
	xui_widget pDestroyRadio[2];
	xui_surface pTarget;
	xui_surface pAtlas;
	xui_surface pCache;
	xui_surface pUncheckedSurface;
	xui_surface pCheckedSurface;
	xui_font pFont;
	xui_radio_group_desc_t tGroupDesc;
	xui_radio_desc_t tDesc;
	xui_radio_desc_t tInvalidDesc;
	xui_accessible_node_t tAccessible;
	xui_vec2_t tMeasured;
	xui_rect_i_t tFullRect;
	xui_rect_t tRect;
	xui_rect_t tTextRect;
	xui_rect_t tInvalidSrc;
	xui_rect_t tLastSrc;
	xui_rect_t tLastCheckedSrc;
	char sLongText[192];
	int iRadioChanged;
	int iGroupChanged;
	int iDestroyChanged;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pGroup = NULL;
	pDestroyGroup = NULL;
	pInvalidWidget = NULL;
	pNonOption = NULL;
	pTarget = NULL;
	pAtlas = NULL;
	pCache = NULL;
	pUncheckedSurface = NULL;
	pCheckedSurface = NULL;
	pFont = NULL;
	memset(pRadio, 0, sizeof(pRadio));
	memset(pDestroyRadio, 0, sizeof(pDestroyRadio));
	iRadioChanged = 0;
	iGroupChanged = 0;
	iDestroyChanged = 0;
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
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pGroup, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_RADIO_GROUP,
		"radio group accessibility");
	XUI_TEST_CHECK((tAccessible.iState & XUI_ACCESSIBLE_STATE_VERTICAL) != 0,
		"vertical radio group accessibility state");

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
	XUI_TEST_CHECK(xuiWidgetFindType(pContext, "checkcard") == NULL,
		"radio-only group does not register check card type");
	iRet = xuiWidgetCreate(pContext, &pNonOption);
	XUI_TEST_CHECK(iRet == XUI_OK && pNonOption != NULL, "create non-option child");
	iRet = xuiWidgetAddChild(pGroup, pNonOption);
	XUI_TEST_CHECK(iRet == XUI_OK, "add non-option child");
	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 99);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiWidgetFindType(pContext, "checkcard") == NULL,
		"group scan has no check card registration side effect");
	xuiWidgetClearDirty(pRadio[0], XUI_WIDGET_DIRTY_ALL);
	iRet = xuiRadioSetTextFlags(pRadio[0],
		XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_RTL);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(xuiWidgetGetDirtyFlags(pRadio[0]) & XUI_WIDGET_DIRTY_LAYOUT) != 0,
		"radio rtl change invalidates layout");
	iRet = xuiWidgetMeasureContent(pRadio[0], (xui_vec2_t){220.0f, 40.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tState.iLastTextMeasureFlags & XUI_TEXT_SHAPE_RTL) != 0,
		"radio rtl reaches text measurement");

	XUI_TEST_CHECK(strcmp(xuiRadioGetText(pRadio[1]), "Medium") == 0, "text from desc");
	memset(sLongText, 'R', sizeof(sLongText) - 1u);
	sLongText[sizeof(sLongText) - 1u] = '\0';
	xuiWidgetClearDirty(pRadio[1], XUI_WIDGET_DIRTY_ALL);
	iRet = xuiRadioSetText(pRadio[1], sLongText);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiRadioGetText(pRadio[1]), sLongText) == 0 &&
		(xuiWidgetGetDirtyFlags(pRadio[1]) & XUI_WIDGET_DIRTY_LAYOUT) != 0u,
		"long radio text grows shared buffer");
	xuiWidgetClearDirty(pRadio[1], XUI_WIDGET_DIRTY_ALL);
	iRet = xuiRadioSetText(pRadio[1], sLongText);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pRadio[1]) == 0u,
		"idempotent radio text stays clean");
	iRet = xuiRadioSetText(pRadio[1], xuiRadioGetText(pRadio[1]) + 1);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(xuiRadioGetText(pRadio[1])) == sizeof(sLongText) - 2u,
		"grown self-aliased text update");
	iRet = xuiRadioSetText(pRadio[1], "Medium");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore aliased text");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 1, "selected index from desc");
	XUI_TEST_CHECK(xuiRadioGetChecked(pRadio[1]) == 1 && xuiRadioGetChecked(pRadio[0]) == 0, "initial group selection");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 0 && xuiWidgetGetTabStop(pRadio[1]) == 1 &&
		xuiWidgetGetTabStop(pRadio[2]) == 0, "selected radio is the only group tab stop");
	XUI_TEST_CHECK(xuiRadioGetIndicatorSize(pRadio[0]) == 18.0f, "default indicator size");
	XUI_TEST_CHECK(xuiRadioGetGap(pRadio[0]) == 8.0f, "default radio gap");
	XUI_TEST_CHECK(xuiRadioGroupGetGap(pGroup) == 4.0f, "group gap");

	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	tRect = xuiWidgetGetRect(pRadio[0]);
	if ( !(tRect.fW > 20.0f && tRect.fH >= 24.0f) ) {
		printf("radio layout rect: %d %d %d %d\n", tRect.fX, tRect.fY, tRect.fW, tRect.fH);
	}
	XUI_TEST_CHECK(tRect.fW > 20.0f && tRect.fH >= 24.0f, "radio layout");

	iRet = xuiWidgetSetEnabled(pRadio[1], 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable middle option");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 1 && xuiWidgetGetTabStop(pRadio[1]) == 0 &&
		xuiWidgetGetTabStop(pRadio[2]) == 0, "disabled selection falls back to first eligible tab stop");
	iRet = xuiSetFocusWidget(pContext, pRadio[0]);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus option 0 for arrow navigation");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_RIGHT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "right key down");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedWidget(pGroup) == pRadio[2] &&
		xuiGetFocusWidget(pContext) == pRadio[2], "arrow navigation skips disabled option");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 0 && xuiWidgetGetTabStop(pRadio[1]) == 0 &&
		xuiWidgetGetTabStop(pRadio[2]) == 1, "arrow selection moves the group tab stop");
	iRet = xuiWidgetSetEnabled(pRadio[1], 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "reenable middle option");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 0 && xuiWidgetGetTabStop(pRadio[1]) == 0 &&
		xuiWidgetGetTabStop(pRadio[2]) == 1, "reenabling an unselected radio preserves the selected tab stop");
	iGroupChanged = 0;
	iRadioChanged = 0;

	iRet = __xuiRadioClick(pContext, pRadio[0]);
	XUI_TEST_CHECK(iRet == XUI_OK, "pointer click option 0");
	XUI_TEST_CHECK(xuiRadioGroupGetSelectedIndex(pGroup) == 0, "click selects option 0");
	XUI_TEST_CHECK(xuiRadioGetChecked(pRadio[0]) == 1 && xuiRadioGetChecked(pRadio[1]) == 0, "click unchecks sibling");
	XUI_TEST_CHECK(iGroupChanged == 1 && iRadioChanged >= 1, "click callbacks");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 1 && xuiWidgetGetTabStop(pRadio[1]) == 0 &&
		xuiWidgetGetTabStop(pRadio[2]) == 0, "pointer selection moves the group tab stop");
	iGroupChanged = 0;
	iRadioChanged = 0;
	tRect = xuiWidgetGetWorldRect(pRadio[1]);
	iRet = xuiInputPointerDownEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tRect.fW * 0.5f), (int)(tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pRadio[1],
		"first touch owns radio press");
	iRet = xuiInputPointerDownEx(pContext, 502u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tRect.fW * 0.5f), (int)(tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 502u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tRect.fW * 0.5f), (int)(tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedIndex(pGroup) == 0 &&
		iGroupChanged == 0 && iRadioChanged == 0 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == pRadio[1] &&
		(xuiRadioGetState(pRadio[1]) & XUI_WIDGET_STATE_ACTIVE) != 0,
		"unrelated touch cannot select or release radio press");
	iRet = xuiInputPointerUpEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH,
		(int)(tRect.fX + tRect.fW * 0.5f), (int)(tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0u);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedIndex(pGroup) == 1 &&
		iGroupChanged == 1 && iRadioChanged >= 1 &&
		xuiGetPointerCaptureEx(pContext, 501u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"owning touch selects radio once");

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
	iRet = xuiSetFocusWidget(pContext, pRadio[0]);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus option 0 before blur");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space down before blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiRadioGetState(pRadio[0]) & XUI_WIDGET_STATE_ACTIVE) != 0,
	               "active before blur");
	iRet = xuiSetFocusWidget(pContext, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "clear focus during keyboard press");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && (xuiRadioGetState(pRadio[0]) & XUI_WIDGET_STATE_ACTIVE) == 0,
	               "blur cancels keyboard active");
	iRet = xuiInputKeyUp(pContext, XUI_KEY_SPACE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "space up after blur");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedIndex(pGroup) == 2,
	               "blurred keyboard press does not select");
	tRect = xuiWidgetGetWorldRect(pRadio[0]);
	iRet = xuiInputPointerMove(pContext, tRect.fX + tRect.fW * 0.5f,
		tRect.fY + tRect.fH * 0.5f, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerDown(pContext,
		tRect.fX + tRect.fW * 0.5f, tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pRadio[0],
		"radio captures pointer before ancestor disable");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == NULL &&
		(xuiRadioGetState(pRadio[0]) & XUI_WIDGET_STATE_DISABLED) != 0 &&
		(xuiRadioGetState(pRadio[0]) & XUI_WIDGET_STATE_ACTIVE) == 0,
		"ancestor disable cancels radio pointer session");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUp(pContext,
		tRect.fX + tRect.fW * 0.5f, tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedIndex(pGroup) == 2,
		"cancelled ancestor-disabled press cannot select later");

	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedRadio(pGroup) == pRadio[1], "set selected index");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 0 && xuiWidgetGetTabStop(pRadio[1]) == 1 &&
		xuiWidgetGetTabStop(pRadio[2]) == 0, "programmatic selection moves the group tab stop");
	iGroupChanged = 0;
	iRadioChanged = 0;
	xuiWidgetClearDirty(pRadio[0], XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pRadio[1], XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pRadio[2], XUI_WIDGET_DIRTY_ALL);
	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && iGroupChanged == 0 && iRadioChanged == 0 &&
		xuiWidgetGetDirtyFlags(pRadio[0]) == 0u &&
		xuiWidgetGetDirtyFlags(pRadio[1]) == 0u &&
		xuiWidgetGetDirtyFlags(pRadio[2]) == 0u,
		"idempotent group selection stays clean and silent");
	iRet = xuiRadioGroupSetSelectedIndex(pGroup, -2);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT &&
		xuiRadioGroupGetSelectedRadio(pGroup) == pRadio[1],
		"only minus one clears radio group selection");
	iRet = xuiRadioGroupSetSelectedIndex(pGroup, -1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedWidget(pGroup) == NULL &&
		xuiRadioGroupGetSelectedIndex(pGroup) == -1,
		"minus one clears radio group selection");
	iRet = xuiRadioGroupSetSelectedIndex(pGroup, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedRadio(pGroup) == pRadio[1],
		"restore selection after clear");
	iRet = xuiRadioGroupSetOrientation(pGroup, XUI_RADIO_GROUP_HORIZONTAL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetOrientation(pGroup) == XUI_RADIO_GROUP_HORIZONTAL, "set orientation");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pGroup, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_VERTICAL) == 0,
		"horizontal radio group accessibility state");
	iRet = xuiRadioGroupSetGap(pGroup, 10.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetGap(pGroup) == 10.0f, "set group gap");
	iRet = xuiRadioSetIndicatorSize(pRadio[1], 20.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetIndicatorSize(pRadio[1]) == 20.0f, "set radio size");
	iRet = xuiRadioSetGap(pRadio[1], 11.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetGap(pRadio[1]) == 11.0f, "set radio gap");
	iRet = xuiRadioSetIndicatorSize(pRadio[1], NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGetIndicatorSize(pRadio[1]) == 20.0f,
	               "reject nan indicator size");
	iRet = xuiRadioSetGap(pRadio[1], INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGetGap(pRadio[1]) == 11.0f,
	               "reject infinite radio gap");
	iRet = xuiRadioSetIndicatorSize(pRadio[1], FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGetIndicatorSize(pRadio[1]) == 20.0f,
		"reject unbounded indicator size");
	iRet = xuiRadioSetGap(pRadio[1], FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGetGap(pRadio[1]) == 11.0f,
		"reject unbounded radio gap");
	iRet = xuiRadioGroupSetGap(pGroup, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGroupGetGap(pGroup) == 10.0f,
	               "reject nan group gap");
	iRet = xuiRadioGroupSetGap(pGroup, FLT_MAX);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiRadioGroupGetGap(pGroup) == 10.0f,
		"reject unbounded group gap");
	iRet = xuiRadioSetColors(pRadio[1], XUI_COLOR_RGBA(24, 166, 113, 255), XUI_COLOR_RGBA(156, 181, 170, 255), XUI_COLOR_RGBA(72, 196, 146, 255), XUI_COLOR_RGBA(24, 166, 113, 255));
	XUI_TEST_CHECK(iRet == XUI_OK, "set radio colors");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 260, 150, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiBuiltinAssetGetAtlas(pContext, &pAtlas);
	XUI_TEST_CHECK(iRet == XUI_OK && pAtlas != NULL, "builtin atlas available");
	iRet = xuiRadioUseBuiltinAtlas(pRadio[1], 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGetUseBuiltinAtlas(pRadio[1]) == 1, "enable radio atlas");
	xuiBuiltinAssetGetRect("radio_checked", &tRect);
	iRet = xuiRadioSetIndicatorSurface(pRadio[1], pAtlas, tRect, pAtlas, tRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "set valid radio indicator surfaces");
	tInvalidSrc = (xui_rect_t){NAN, 0.0f, 1.0f, 1.0f};
	iRet = xuiRadioSetIndicatorSurface(pRadio[1], pAtlas, tInvalidSrc, pAtlas, tRect);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject nan unchecked source rect");
	tInvalidSrc = (xui_rect_t){0.0f, 0.0f, FLT_MAX, 1.0f};
	iRet = xuiRadioSetIndicatorSurface(pRadio[1], pAtlas, tRect, pAtlas, tInvalidSrc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject unbounded checked source rect");
	iRet = xuiRadioGetIndicatorSurface(pRadio[1], &pUncheckedSurface, &tLastSrc,
		&pCheckedSurface, &tLastCheckedSrc);
	XUI_TEST_CHECK(iRet == XUI_OK && pUncheckedSurface == pAtlas && pCheckedSurface == pAtlas &&
		__xuiRadioRectEq(tLastSrc, tRect.fX, tRect.fY, tRect.fW, tRect.fH) &&
		__xuiRadioRectEq(tLastCheckedSrc, tRect.fX, tRect.fY, tRect.fW, tRect.fH),
		"invalid source rect preserves radio indicator surfaces");
	tFullRect = (xui_rect_i_t){0, 0, 260, 150};
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout before render");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	xuiTestProxyResetTextCounters(&tState);
	iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render atlas radio");
	XUI_TEST_CHECK(xuiTestProxyGetTextMeasureCount(&tState) == 0,
		"radio cache render avoids redundant text measurement");
	pCache = xuiWidgetGetCacheSurface(pRadio[1], xuiWidgetGetStateId(pRadio[1]));
	XUI_TEST_CHECK(pCache != NULL, "radio cache surface");
	xuiTestSurfaceReset(pCache);
	iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) == 0,
		"unchanged borrowed indicator surface reuses radio caches");
	iRet = tState.tProxy.surfaceUpdateRGBA(&tState.tProxy, pAtlas,
		(xui_rect_i_t){0, 0, 1, 1}, NULL, 0);
	if ( iRet == XUI_OK ) iRet = xuiRenderPrepare(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiTestSurfaceGetDrawCount(pCache) > 0,
		"borrowed indicator generation rebuilds radio caches");
	tLastSrc = tRect;
	XUI_TEST_CHECK(__xuiRadioRectEq(tLastSrc, 104.0f, 34.0f, 34.0f, 34.0f), "checked radio atlas rect");
	tRect = xuiRadioGetIndicatorRect(pRadio[0]);
	tTextRect = xuiRadioGetTextRect(pRadio[0]);
	XUI_TEST_CHECK(tRect.fX >= tTextRect.fX + tTextRect.fW,
		"rtl radio mirrors indicator after text");
	iRet = xuiRadioSetTextFlags(pRadio[0], XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE);
	if ( iRet == XUI_OK ) iRet = xuiRender(pContext, pTarget, &tFullRect, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "render ltr radio");
	tRect = xuiRadioGetIndicatorRect(pRadio[0]);
	tTextRect = xuiRadioGetTextRect(pRadio[0]);
	XUI_TEST_CHECK(tRect.fX + tRect.fW <= tTextRect.fX,
		"ltr radio keeps indicator before text");

	iRet = xuiRadioGroupSetSelectedWidget(pGroup, pRadio[1]);
	XUI_TEST_CHECK(iRet == XUI_OK, "select before removal reconciliation");
	iRet = xuiWidgetRemoveFromParent(pRadio[0]);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedWidget(pGroup) == pRadio[1] &&
		xuiRadioGroupGetSelectedIndex(pGroup) == 0,
		"removing earlier option reconciles selected index");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[0]) == 1 && xuiWidgetGetTabStop(pRadio[1]) == 1 &&
		xuiWidgetGetTabStop(pRadio[2]) == 0,
		"removed option restores standalone tab stop without changing group entry");
	iRet = xuiWidgetRemoveFromParent(pRadio[1]);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiRadioGroupGetSelectedWidget(pGroup) == NULL &&
		xuiRadioGroupGetSelectedIndex(pGroup) == -1 && xuiRadioGetChecked(pRadio[1]),
		"removing selected option clears group selection without changing option");
	XUI_TEST_CHECK(xuiWidgetGetTabStop(pRadio[1]) == 1 && xuiWidgetGetTabStop(pRadio[2]) == 1,
		"selected removal restores standalone tab stop and group fallback");

	memset(&tGroupDesc, 0, sizeof(tGroupDesc));
	tGroupDesc.iSize = sizeof(tGroupDesc);
	tGroupDesc.iOrientation = XUI_RADIO_GROUP_VERTICAL;
	tGroupDesc.iSelectedIndex = 0;
	iRet = xuiRadioGroupCreate(pContext, &pDestroyGroup, &tGroupDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyGroup != NULL, "destroy callback group create");
	iRet = xuiWidgetAddChild(pRoot, pDestroyGroup);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback group add");
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	iRet = xuiRadioGroupAddOption(pDestroyGroup, &pDestroyRadio[0], &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback option 0");
	iRet = xuiRadioGroupAddOption(pDestroyGroup, &pDestroyRadio[1], &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback option 1");
	iRet = xuiRadioSetChange(pDestroyRadio[0], __xuiRadioDestroyParentChanged, &iDestroyChanged);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback setup");
	iRet = xuiRadioGroupSetSelectedWidget(pDestroyGroup, pDestroyRadio[1]);
	XUI_TEST_CHECK(iRet == XUI_OK && iDestroyChanged == 1,
		"option callback may destroy radio group without stale access");
	pDestroyGroup = NULL;
	pDestroyRadio[0] = NULL;
	pDestroyRadio[1] = NULL;

	tInvalidDesc = tDesc;
	tInvalidDesc.fGap = NAN;
	pInvalidWidget = pRoot;
	iRet = xuiRadioCreate(pContext, &pInvalidWidget, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"invalid radio create clears output");
	tInvalidDesc = tDesc;
	tInvalidDesc.fIndicatorSize = FLT_MAX;
	pInvalidWidget = pRoot;
	iRet = xuiRadioCreate(pContext, &pInvalidWidget, &tInvalidDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"unbounded radio create clears output");
	tGroupDesc.iOrientation = 99;
	pInvalidWidget = pRoot;
	iRet = xuiRadioGroupCreate(pContext, &pInvalidWidget, &tGroupDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"invalid group create clears output");
	memset(&tGroupDesc, 0, sizeof(tGroupDesc));
	tGroupDesc.iSize = sizeof(tGroupDesc);
	tGroupDesc.iOrientation = XUI_RADIO_GROUP_VERTICAL;
	tGroupDesc.iSelectedIndex = -1;
	tGroupDesc.fGap = FLT_MAX;
	pInvalidWidget = pRoot;
	iRet = xuiRadioGroupCreate(pContext, &pInvalidWidget, &tGroupDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"unbounded group create clears output");
	pInvalidWidget = pRoot;
	iRet = xuiRadioCreate(NULL, &pInvalidWidget, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"invalid radio context clears output");
	pInvalidWidget = pRoot;
	iRet = xuiRadioGroupCreate(NULL, &pInvalidWidget, NULL);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalidWidget == NULL,
		"invalid group context clears output");

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
