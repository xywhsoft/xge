#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_combobox_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_combobox_test_select_t {
	int iCount;
	int iIndex;
	int iValue;
} xui_combobox_test_select_t;

static void __xuiComboBoxSelected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_combobox_test_select_t* pSelect;

	(void)pWidget;
	pSelect = (xui_combobox_test_select_t*)pUser;
	pSelect->iCount++;
	pSelect->iIndex = iIndex;
	pSelect->iValue = iValue;
}

static int __xuiComboBoxRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 420, 320};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiComboBoxDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiComboBoxClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	return __xuiComboBoxDispatchDown(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_combobox_test_select_t tSelect;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCombo;
	xui_widget pMenu;
	xui_widget pPopup;
	xui_surface pTarget;
	xui_font pFont;
	xui_combobox_desc_t tDesc;
	xui_combobox_item_t arrItems[5];
	xui_rect_t tButton;
	xui_rect_t tPopupRect;
	uint32_t iState;
	int arrEnabled[5];
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCombo = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tSelect, 0, sizeof(tSelect));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "combobox", 8, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 420.0f, 320.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 420.0f, 320.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(arrItems, 0, sizeof(arrItems));
	arrItems[0].sText = "Alpha";
	arrItems[0].iValue = 100;
	arrItems[0].bEnabled = 1;
	arrItems[1].sText = "Beta disabled";
	arrItems[1].iValue = 200;
	arrItems[1].bEnabled = 0;
	arrItems[2].bSeparator = 1;
	arrItems[3].sText = "Gamma";
	arrItems[3].iValue = 300;
	arrItems[3].bEnabled = 1;
	arrItems[4].sText = "Delta";
	arrItems[4].iValue = 400;
	arrItems[4].bEnabled = 1;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.arrItemData = arrItems;
	tDesc.iItemCount = 5;
	tDesc.iSelected = 0;
	tDesc.pFont = pFont;
	tDesc.fPopupHeight = 132.0f;
	iRet = xuiComboBoxCreate(pContext, &pCombo, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCombo != NULL, "combobox create");
	iRet = xuiComboBoxSetSelect(pCombo, __xuiComboBoxSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	xuiWidgetSetRect(pCombo, (xui_rect_t){36.0f, 32.0f, 160.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK, "add combobox");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 420, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiComboBoxRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiComboBoxGetItemCount(pCombo) == 5, "item count");
	XUI_TEST_CHECK(xuiComboBoxGetSelected(pCombo) == 0 && xuiComboBoxGetSelectedValue(pCombo) == 100, "initial selected");
	XUI_TEST_CHECK(!xuiComboBoxIsItemEnabled(pCombo, 1) && !xuiComboBoxIsItemEnabled(pCombo, 2), "disabled and separator");
	tButton = xuiComboBoxGetButtonRect(pCombo);
	XUI_TEST_CHECK(tButton.fW > 20.0f && tButton.fH > 20.0f, "button rect");

	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pCombo), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiComboBoxRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pMenu = xuiComboBoxGetMenuWidget(pCombo);
	pPopup = xuiComboBoxGetPopupWidget(pCombo);
	XUI_TEST_CHECK(pMenu != NULL && pPopup != NULL && xuiGetFocusWidget(pContext) == pMenu, "menu popup focus");
	XUI_TEST_CHECK((xuiMenuGetItemState(pMenu, 0) & XUI_MENU_ITEM_DEFAULT) == 0u, "selected item has no default focus frame");
	iState = xuiComboBoxGetState(pCombo);
	XUI_TEST_CHECK((iState & XUI_COMBOBOX_STATE_OPEN) != 0u, "open state");
	tPopupRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tPopupRect.fY >= 60.0f, "popup below owner");

	iRet = __xuiComboBoxClickMenuItem(pContext, pMenu, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pCombo), "disabled click keeps open");
	XUI_TEST_CHECK(tSelect.iCount == 0 && xuiComboBoxGetSelected(pCombo) == 0, "disabled no select");
	iRet = __xuiComboBoxClickMenuItem(pContext, pMenu, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo), "enabled click closes");
	XUI_TEST_CHECK(tSelect.iCount == 1 && tSelect.iIndex == 3 && tSelect.iValue == 300, "select callback");
	XUI_TEST_CHECK(xuiComboBoxGetSelected(pCombo) == 3 && xuiComboBoxGetSelectedValue(pCombo) == 300, "selected gamma");

	iRet = xuiComboBoxSetSelectedValue(pCombo, 400);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetSelected(pCombo) == 4, "selected by value");
	iRet = xuiSetFocusWidget(pContext, pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus combo");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "key down input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pCombo), "key down opens");
	iRet = xuiComboBoxClose(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo), "close");

	arrEnabled[0] = 1;
	arrEnabled[1] = 1;
	arrEnabled[2] = 0;
	arrEnabled[3] = 1;
	arrEnabled[4] = 0;
	iRet = xuiComboBoxSetEnabledItems(pCombo, arrEnabled, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetSelected(pCombo) == -1 && xuiComboBoxIsItemEnabled(pCombo, 1), "enabled map");
	iRet = xuiWidgetSetEnabled(pCombo, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable combo");
	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo), "disabled does not open");
	iState = xuiComboBoxGetState(pCombo);
	XUI_TEST_CHECK((iState & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");

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
	printf("xui_combobox_test passed\n");
	return 0;
}
