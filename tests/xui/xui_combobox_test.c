#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_combobox_test_text_t {
	int iCount;
	char sText[64];
} xui_combobox_test_text_t;

typedef struct xui_combobox_test_destroy_t {
	xui_widget* ppWidget;
	int iCount;
} xui_combobox_test_destroy_t;

typedef struct xui_combobox_test_event_destroy_t {
	xui_widget* ppWidget;
	int iEventType;
	int iCount;
} xui_combobox_test_event_destroy_t;

static void __xuiComboBoxSelected(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_combobox_test_select_t* pSelect;

	(void)pWidget;
	pSelect = (xui_combobox_test_select_t*)pUser;
	pSelect->iCount++;
	pSelect->iIndex = iIndex;
	pSelect->iValue = iValue;
}

static void __xuiComboBoxTextChanged(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_combobox_test_text_t* pText;

	(void)pWidget;
	pText = (xui_combobox_test_text_t*)pUser;
	if ( pText == NULL ) return;
	pText->iCount++;
	snprintf(pText->sText, sizeof(pText->sText), "%s", (sText != NULL) ? sText : "");
}

static void __xuiComboBoxDestroyOnSelect(xui_widget pWidget, int iIndex, int iValue, void* pUser)
{
	xui_combobox_test_destroy_t* pDestroy = (xui_combobox_test_destroy_t*)pUser;

	(void)iIndex;
	(void)iValue;
	if ( pDestroy == NULL ) return;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
	if ( pDestroy->ppWidget != NULL && *pDestroy->ppWidget == pWidget ) *pDestroy->ppWidget = NULL;
}

static void __xuiComboBoxDestroyOnTextChange(xui_widget pWidget, const char* sText, void* pUser)
{
	xui_combobox_test_destroy_t* pDestroy = (xui_combobox_test_destroy_t*)pUser;

	(void)sText;
	if ( pDestroy == NULL ) return;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
	if ( pDestroy->ppWidget != NULL && *pDestroy->ppWidget == pWidget ) *pDestroy->ppWidget = NULL;
}

static int __xuiComboBoxDestroyVictimOnEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_combobox_test_event_destroy_t* pDestroy = (xui_combobox_test_event_destroy_t*)pUser;
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

static int __xuiComboBoxDispatchClick(xui_context pContext, float fX, float fY)
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

static int __xuiComboBoxDispatchRightClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, XUI_POINTER_BUTTON_RIGHT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_RIGHT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiComboBoxDispatchLongPress(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pContext, 0.60f);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiComboBoxDispatchDrag(xui_context pContext, float fStartX, float fStartY, float fEndX, float fEndY)
{
	int iRet;

	iRet = xuiInputPointerMove(pContext, fStartX, fStartY, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerDown(pContext, fStartX, fStartY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerMove(pContext, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, fEndX, fEndY, XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiComboBoxClickMenuItem(xui_context pContext, xui_widget pMenu, int iIndex)
{
	xui_rect_t tWorld;
	xui_rect_t tItem;

	tWorld = xuiWidgetGetWorldRect(pMenu);
	tItem = xuiMenuGetItemRect(pMenu, iIndex);
	return __xuiComboBoxDispatchClick(pContext, tWorld.fX + tItem.fX + tItem.fW * 0.5f, tWorld.fY + tItem.fY + tItem.fH * 0.5f);
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_combobox_test_select_t tSelect;
	xui_combobox_test_select_t tEditSelect;
	xui_combobox_test_text_t tText;
	xui_combobox_test_destroy_t tDestroy;
	xui_combobox_test_destroy_t tTextDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pCombo;
	xui_widget pEditCombo;
	xui_widget pEditInput;
	xui_widget pDestroyCombo;
	xui_widget pDestroyMenu;
	xui_widget pDestroyPopup;
	xui_widget pTextDestroyCombo;
	xui_widget pTextDestroyInput;
	xui_widget pMenu;
	xui_widget pInputMenu;
	xui_widget pPopup;
	xui_surface pTarget;
	xui_font pFont;
	xui_combobox_desc_t tDesc;
	xui_combobox_desc_t tEditDesc;
	xui_combobox_item_t arrItems[5];
	const char* arrOversizedItems[XUI_COMBOBOX_ITEM_CAPACITY + 1];
	const char* arrOwnedItems[1];
	char sOwnedItem[16];
	xui_menu_colors_t tMenuColors;
	xui_rect_t tButton;
	xui_rect_t tInputRect;
	xui_rect_t tInputWorld;
	xui_rect_t tInputText;
	xui_rect_t tPopupRect;
	xui_event_t tContextEvent;
	uint32_t iState;
	int arrEnabled[5];
	int iSelStart;
	int iSelEnd;
	int iFailed;
	int iRet;

	pContext = NULL;
	pRoot = NULL;
	pCombo = NULL;
	pEditCombo = NULL;
	pEditInput = NULL;
	pDestroyCombo = NULL;
	pDestroyMenu = NULL;
	pDestroyPopup = NULL;
	pTextDestroyCombo = NULL;
	pTextDestroyInput = NULL;
	pInputMenu = NULL;
	pTarget = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tSelect, 0, sizeof(tSelect));
	memset(&tEditSelect, 0, sizeof(tEditSelect));
	memset(&tText, 0, sizeof(tText));
	memset(&tDestroy, 0, sizeof(tDestroy));
	memset(&tTextDestroy, 0, sizeof(tTextDestroy));
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
	memset(arrOversizedItems, 0, sizeof(arrOversizedItems));
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
	{
		xui_combobox_desc_t tInvalidDesc = tDesc;
		xui_widget pInvalid = (xui_widget)(uintptr_t)1;
		tInvalidDesc.iItemCount = XUI_COMBOBOX_ITEM_CAPACITY + 1;
		iRet = xuiComboBoxCreate(pContext, &pInvalid, &tInvalidDesc);
		XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "oversized descriptor rejected and output cleared");
		tInvalidDesc = tDesc;
		tInvalidDesc.fPopupHeight = NAN;
		iRet = xuiComboBoxCreate(pContext, &pInvalid, &tInvalidDesc);
		XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite descriptor rejected");
		tInvalidDesc = tDesc;
		tInvalidDesc.fItemHeight = XUI_LAYOUT_UNBOUNDED * 2.0f;
		iRet = xuiComboBoxCreate(pContext, &pInvalid, &tInvalidDesc);
		XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT,
		               "oversized descriptor metric rejected");
	}
	iRet = xuiComboBoxCreate(pContext, &pCombo, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pCombo != NULL, "combobox create");
	iRet = xuiComboBoxSetSelect(pCombo, __xuiComboBoxSelected, &tSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "select callback");
	xuiWidgetSetRect(pCombo, (xui_rect_t){36.0f, 32.0f, 160.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK, "add combobox");

	memset(&tEditDesc, 0, sizeof(tEditDesc));
	tEditDesc.iSize = sizeof(tEditDesc);
	tEditDesc.arrItemData = arrItems;
	tEditDesc.iItemCount = 5;
	tEditDesc.iSelected = 3;
	tEditDesc.iMode = XUI_COMBOBOX_MODE_EDIT;
	tEditDesc.sPlaceholder = "type or choose";
	tEditDesc.pFont = pFont;
	{
		xui_combobox_desc_t tLimitedDesc = tEditDesc;
		xui_widget pInvalid = (xui_widget)(uintptr_t)1;
		tLimitedDesc.iMaxLength = 3;
		iRet = xuiComboBoxCreate(pContext, &pInvalid, &tLimitedDesc);
		XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL && pInvalid == NULL,
		               "editable descriptor rejects selected item that exceeds max length");
	}
	iRet = xuiComboBoxCreate(pContext, &pEditCombo, &tEditDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pEditCombo != NULL, "editable combobox create");
	iRet = xuiComboBoxSetSelect(pEditCombo, __xuiComboBoxSelected, &tEditSelect);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable select callback");
	iRet = xuiComboBoxSetTextChange(pEditCombo, __xuiComboBoxTextChanged, &tText);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable text callback");
	xuiWidgetSetRect(pEditCombo, (xui_rect_t){216.0f, 32.0f, 164.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pEditCombo);
	XUI_TEST_CHECK(iRet == XUI_OK, "add editable combobox");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 420, 320, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiComboBoxRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");
	{
		xui_vec2_t tMeasured;
		iRet = xuiWidgetMeasureContent(pCombo,
			(xui_vec2_t){XUI_LAYOUT_UNBOUNDED, XUI_LAYOUT_UNBOUNDED}, &tMeasured);
		XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX > 0.0f && tMeasured.fY > 0.0f,
		               "combobox intrinsic measure");
		iRet = xuiWidgetMeasureContent(pCombo, (xui_vec2_t){60.0f, 12.0f}, &tMeasured);
		XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX <= 60.0f && tMeasured.fY <= 12.0f,
		               "combobox measure honors finite constraints");
	}

	XUI_TEST_CHECK(xuiComboBoxGetItemCount(pCombo) == 5, "item count");
	XUI_TEST_CHECK(xuiComboBoxGetSelected(pCombo) == 0 && xuiComboBoxGetSelectedValue(pCombo) == 100, "initial selected");
	XUI_TEST_CHECK(!xuiComboBoxIsItemEnabled(pCombo, 1) && !xuiComboBoxIsItemEnabled(pCombo, 2), "disabled and separator");
	snprintf(sOwnedItem, sizeof(sOwnedItem), "%s", "Owned");
	arrOwnedItems[0] = sOwnedItem;
	iRet = xuiComboBoxSetItems(pCombo, arrOwnedItems, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "set caller-owned item text");
	sOwnedItem[0] = 'X';
	XUI_TEST_CHECK(strcmp(xuiComboBoxGetItemText(pCombo, 0), "Owned") == 0, "item text is deep copied");
	iRet = xuiComboBoxSetItemData(pCombo, arrItems, 5);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxSetSelected(pCombo, 0) == XUI_OK, "restore item data after ownership test");
	tButton = xuiComboBoxGetButtonRect(pCombo);
	XUI_TEST_CHECK(tButton.fW > 20.0f && tButton.fH > 20.0f, "button rect");
	{
		xui_rect_t tOldRect = xuiWidgetGetRect(pCombo);
		xui_rect_t tContent;
		xui_rect_t tText;
		iRet = xuiWidgetSetRect(pCombo, (xui_rect_t){36.0f, 32.0f, 7.0f, 3.0f});
		XUI_TEST_CHECK(iRet == XUI_OK, "tiny combobox rect set");
		tContent = xuiWidgetGetContentRect(pCombo);
		tButton = xuiComboBoxGetButtonRect(pCombo);
		tText = xuiComboBoxGetTextRect(pCombo);
		XUI_TEST_CHECK(tButton.fW >= 0.0f && tButton.fH >= 0.0f &&
		               tButton.fX >= tContent.fX && tButton.fY >= tContent.fY &&
		               tButton.fX + tButton.fW <= tContent.fX + tContent.fW &&
		               tButton.fY + tButton.fH <= tContent.fY + tContent.fH,
		               "tiny combobox button stays inside content");
		XUI_TEST_CHECK(tText.fW >= 0.0f && tText.fH >= 0.0f &&
		               tText.fX >= tContent.fX && tText.fY >= tContent.fY &&
		               tText.fX + tText.fW <= tContent.fX + tContent.fW &&
		               tText.fY + tText.fH <= tContent.fY + tContent.fH,
		               "tiny combobox text stays inside content");
		iRet = xuiWidgetSetRect(pCombo, tOldRect);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiLayout(pContext) == XUI_OK,
		               "restore combobox geometry after tiny bounds");
	}
	pEditInput = xuiComboBoxGetInputWidget(pEditCombo);
	XUI_TEST_CHECK(pEditInput != NULL && xuiWidgetGetVisible(pEditInput), "editable input visible");
	tButton = xuiComboBoxGetButtonRect(pEditCombo);
	tInputRect = xuiWidgetGetRect(pEditInput);
	XUI_TEST_CHECK(tButton.fW == 30.0f && tInputRect.fX == 1.0f &&
	               tInputRect.fW + tButton.fW == 163.0f && tInputRect.fH == 30.0f,
	               "editable input and cross button tracks");
	xuiWidgetSetRect(pEditCombo, (xui_rect_t){216.0f, 32.0f, 164.0f, 40.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable tall layout");
	tButton = xuiComboBoxGetButtonRect(pEditCombo);
	tInputRect = xuiWidgetGetRect(pEditInput);
	XUI_TEST_CHECK(tButton.fW == 36.0f && tInputRect.fW == 127.0f,
	               "cross button track follows height and maximum");
	xuiWidgetSetRect(pEditCombo, (xui_rect_t){216.0f, 32.0f, 164.0f, 30.0f});
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable geometry restore");
	pInputMenu = xuiComboBoxGetInputMenuWidget(pEditCombo);
	XUI_TEST_CHECK(pInputMenu != NULL && pInputMenu == xuiInputGetMenuWidget(pEditInput), "editable input menu widget");
	XUI_TEST_CHECK(strcmp(xuiComboBoxGetInputMenuTitle(pEditCombo, XUI_INPUT_MENU_COPY), xuiInputGetMenuTitle(pEditInput, XUI_INPUT_MENU_COPY)) == 0, "editable input menu title mirrors input");
	iRet = xuiComboBoxSetInputMenuTitle(pEditCombo, XUI_INPUT_MENU_COPY, "Copy Text");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetInputMenuTitle(pEditCombo, XUI_INPUT_MENU_COPY), "Copy Text") == 0, "editable input menu title set");
	iRet = xuiComboBoxSetInputMenuTitle(pEditCombo, XUI_INPUT_MENU_COPY, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetInputMenuTitle(pEditCombo, XUI_INPUT_MENU_COPY), xuiInputGetMenuTitle(pEditInput, XUI_INPUT_MENU_COPY)) == 0, "editable input menu title reset");
	XUI_TEST_CHECK(xuiComboBoxGetMode(pEditCombo) == XUI_COMBOBOX_MODE_EDIT, "editable mode");
	XUI_TEST_CHECK(strcmp(xuiComboBoxGetText(pEditCombo), "Gamma") == 0, "editable initial text");
	iRet = xuiComboBoxSetMaxLength(pEditCombo, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetText(pEditCombo), "Gam") == 0 &&
	               xuiComboBoxGetSelected(pEditCombo) == -1,
	               "max length truncation clears mismatched selection");
	{
		int iChangeCount = xuiComboBoxGetChangeCount(pEditCombo);
		xuiWidgetClearDirty(pEditCombo, XUI_WIDGET_DIRTY_ALL);
		xuiWidgetClearDirty(pEditInput, XUI_WIDGET_DIRTY_ALL);
		iRet = xuiComboBoxSetText(pEditCombo, "Gamma");
		XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetText(pEditCombo), "Gam") == 0 &&
		               xuiComboBoxGetChangeCount(pEditCombo) == iChangeCount &&
		               xuiWidgetGetDirtyFlags(pEditCombo) == 0u &&
		               xuiWidgetGetDirtyFlags(pEditInput) == 0u,
		               "truncated text setter compares applied value and is idempotent");
		iRet = xuiComboBoxSetSelected(pEditCombo, 0);
		XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL &&
		               xuiComboBoxGetSelected(pEditCombo) == -1 &&
		               strcmp(xuiComboBoxGetText(pEditCombo), "Gam") == 0 &&
		               xuiComboBoxGetChangeCount(pEditCombo) == iChangeCount,
		               "selection that exceeds max length rolls back transactionally");
	}
	iRet = xuiComboBoxSetMaxLength(pEditCombo, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxSetSelected(pEditCombo, 3) == XUI_OK &&
	               strcmp(xuiComboBoxGetText(pEditCombo), "Gamma") == 0,
	               "editable selection restores full item after max length reset");
	iRet = xuiSetFocusWidget(pContext, pEditInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus editable input for F4");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_F4, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK && xuiComboBoxIsOpen(pEditCombo), "editable input F4 opens dropdown");
	iRet = xuiComboBoxClose(pEditCombo);
	XUI_TEST_CHECK(iRet == XUI_OK, "close editable F4 dropdown");
	iRet = xuiSetFocusWidget(pContext, pEditInput);
	XUI_TEST_CHECK(iRet == XUI_OK, "refocus editable input for alt down");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_ALT);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK && xuiComboBoxIsOpen(pEditCombo), "editable input alt down opens dropdown");
	iRet = xuiComboBoxClose(pEditCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiSetFocusWidget(pContext, pEditInput) == XUI_OK, "close editable alt dropdown and restore input focus");
	iRet = xuiComboBoxSetText(pEditCombo, "Custom");
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetSelected(pEditCombo) == -1 && strcmp(xuiComboBoxGetText(pEditCombo), "Custom") == 0, "editable custom text clears selection");
	iRet = xuiSetFocusWidget(pContext, pEditInput);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pEditInput, "focus editable input");
	iRet = xuiInputText(pContext, '!');
	XUI_TEST_CHECK(iRet == XUI_OK, "editable input text");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && tText.iCount == 1 && strcmp(tText.sText, "Custom!") == 0, "editable text change");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable click layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable click update");
	tInputWorld = xuiWidgetGetWorldRect(pEditInput);
	tInputText = xuiInputGetTextRect(pEditInput);
	iRet = __xuiComboBoxDispatchClick(pContext, tInputWorld.fX + tInputText.fX + 1.0f, tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pEditInput, "editable input mouse click focuses input");
	iRet = xuiInputText(pContext, '^');
	XUI_TEST_CHECK(iRet == XUI_OK, "editable click cursor text input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetText(pEditCombo), "^Custom!") == 0, "editable mouse click moves input cursor");
	iRet = xuiComboBoxSetText(pEditCombo, "DragText");
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiComboBoxGetText(pEditCombo), "DragText") == 0, "editable drag reset text");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable drag layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable drag update");
	tInputWorld = xuiWidgetGetWorldRect(pEditInput);
	tInputText = xuiInputGetTextRect(pEditInput);
	iRet = __xuiComboBoxDispatchDrag(pContext,
		tInputWorld.fX + tInputText.fX + 1.0f,
		tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f,
		tInputWorld.fX + tInputText.fX + tInputText.fW - 1.0f,
		tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable input drag");
	iRet = xuiInputGetSelection(pEditInput, &iSelStart, &iSelEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && iSelStart == 0 && iSelEnd == (int)strlen(xuiComboBoxGetText(pEditCombo)), "editable input drag selects text");
	iRet = xuiComboBoxOpenInputMenu(pEditCombo, tInputWorld.fX + tInputText.fX + 8.0f, tInputWorld.fY + tInputText.fY + tInputText.fH);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pInputMenu), "editable input menu opens by API");
	XUI_TEST_CHECK(xuiMenuGetItemCount(pInputMenu) == 9, "editable input menu item count");
	XUI_TEST_CHECK((xuiMenuGetItemState(pInputMenu, 4) & XUI_MENU_ITEM_ENABLED) != 0u, "editable input menu copy enabled");
	iRet = xuiMenuClose(pInputMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close editable input menu");
	iRet = __xuiComboBoxDispatchRightClick(pContext, tInputWorld.fX + tInputText.fX + 10.0f, tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pInputMenu), "editable input menu opens by right click");
	iRet = xuiMenuClose(pInputMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close right click input menu");
	memset(&tContextEvent, 0, sizeof(tContextEvent));
	tContextEvent.iSize = sizeof(tContextEvent);
	tContextEvent.iType = XUI_EVENT_CONTEXT_MENU;
	tContextEvent.pTarget = pEditCombo;
	tContextEvent.fX = tInputWorld.fX + tInputText.fX + 10.0f;
	tContextEvent.fY = tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f;
	tContextEvent.iButton = XUI_POINTER_BUTTON_RIGHT;
	iRet = xuiDispatchEvent(pContext, &tContextEvent);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pInputMenu), "editable input menu opens when combobox receives context event");
	iRet = xuiMenuClose(pInputMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close combobox context input menu");
	iRet = __xuiComboBoxDispatchLongPress(pContext, tInputWorld.fX + tInputText.fX + 12.0f, tInputWorld.fY + tInputText.fY + tInputText.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiMenuIsOpen(pInputMenu), "editable input menu opens by long press");
	iRet = xuiMenuClose(pInputMenu);
	XUI_TEST_CHECK(iRet == XUI_OK, "close long press input menu");
	iRet = xuiWidgetSetVisible(pEditCombo, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide editable combobox");
	iRet = xuiComboBoxOpenInputMenu(pEditCombo, 64.0f, 64.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiMenuIsOpen(pInputMenu),
	               "hidden editable combobox does not open input menu");
	iRet = xuiWidgetSetVisible(pEditCombo, 1);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "restore editable combobox visibility");
	iRet = xuiComboBoxOpenInputMenu(pCombo, 40.0f, 40.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_UNSUPPORTED, "select mode has no editable input menu");
	{
		xui_rect_t tWorld = xuiWidgetGetWorldRect(pCombo);
		iRet = xuiWidgetSetVisible(pCombo, 0);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet == XUI_OK ) iRet = xuiSetFocusWidget(pContext, pEditInput);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pEditInput,
		               "focus sibling before hidden targeted input");
		memset(&tContextEvent, 0, sizeof(tContextEvent));
		tContextEvent.iSize = sizeof(tContextEvent);
		tContextEvent.iType = XUI_EVENT_POINTER_DOWN;
		tContextEvent.pTarget = pCombo;
		tContextEvent.iButton = XUI_POINTER_BUTTON_LEFT;
		tContextEvent.fX = tWorld.fX + 4.0f;
		tContextEvent.fY = tWorld.fY + 4.0f;
		iRet = xuiDispatchEvent(pContext, &tContextEvent);
		XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo) &&
		               xuiGetFocusWidget(pContext) == pEditInput,
		               "hidden combobox ignores targeted pointer down");
		memset(&tContextEvent, 0, sizeof(tContextEvent));
		tContextEvent.iSize = sizeof(tContextEvent);
		tContextEvent.iType = XUI_EVENT_KEY_DOWN;
		tContextEvent.pTarget = pCombo;
		tContextEvent.iKey = XUI_KEY_F4;
		iRet = xuiDispatchEvent(pContext, &tContextEvent);
		XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo),
		               "hidden combobox ignores targeted keyboard open");
		iRet = xuiWidgetSetVisible(pCombo, 1);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK, "restore combobox visibility");
	}

	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pCombo), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiComboBoxRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pMenu = xuiComboBoxGetMenuWidget(pCombo);
	pPopup = xuiComboBoxGetPopupWidget(pCombo);
	XUI_TEST_CHECK(pMenu != NULL && pPopup != NULL && xuiGetFocusWidget(pContext) == pMenu, "menu popup focus");
	memset(&tMenuColors, 0, sizeof(tMenuColors));
	tMenuColors.iSize = sizeof(tMenuColors);
	iRet = xuiMenuGetColors(pMenu, &tMenuColors);
	XUI_TEST_CHECK(iRet == XUI_OK && (tMenuColors.iFocusColor & 0xffu) == 0u,
		"combobox popup menu has no implicit focus frame");
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

	iRet = xuiComboBoxOpen(pEditCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pEditCombo), "editable open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "editable open layout");
	pMenu = xuiComboBoxGetMenuWidget(pEditCombo);
	iRet = __xuiComboBoxClickMenuItem(pContext, pMenu, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pEditCombo), "editable select closes");
	XUI_TEST_CHECK(tEditSelect.iCount == 1 && xuiComboBoxGetSelected(pEditCombo) == 0 && strcmp(xuiComboBoxGetText(pEditCombo), "Alpha") == 0, "editable select syncs text");

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
	iRet = xuiComboBoxSetPopupHeight(pCombo, NAN);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiComboBoxGetPopupHeight(pCombo) == 132.0f, "non-finite popup height rejected transactionally");
	iRet = xuiComboBoxSetPopupMaxHeight(pCombo, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite popup max height rejected");
	iRet = xuiComboBoxSetMetrics(pCombo, NAN, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "non-finite metrics rejected");
	iRet = xuiComboBoxSetPopupHeight(pCombo, XUI_LAYOUT_UNBOUNDED * 2.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiComboBoxGetPopupHeight(pCombo) == 132.0f,
	               "oversized popup height rejected transactionally");
	iRet = xuiComboBoxSetPopupMaxHeight(pCombo, XUI_LAYOUT_UNBOUNDED * 2.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "oversized popup max height rejected");
	iRet = xuiComboBoxSetMetrics(pCombo, XUI_LAYOUT_UNBOUNDED * 2.0f, 1.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "oversized metrics rejected");
	{
		uint32_t iText, iDisabledText, iBackground, iHoverBackground, iOpenBackground;
		uint32_t iDisabledBackground, iBorder, iHoverBorder, iFocusBorder;
		uint32_t iArrow, iDisabledArrow, iButton, iButtonHover, iButtonOpen;
		uint32_t iPanel, iPopupBorder, iShadow, iHover, iPopupText, iHoverText;
		uint32_t iPopupDisabledText, iSeparator;
		float fItemHeight, fBorderWidth;
		int arrCurrentEnabled[5] = {1, 0, 0, 1, 1};

		XUI_TEST_CHECK(xuiComboBoxGetColors(pCombo, &iText, &iDisabledText, &iBackground,
		               &iHoverBackground, &iOpenBackground, &iDisabledBackground) == XUI_OK,
		               "get combobox colors for idempotence");
		XUI_TEST_CHECK(xuiComboBoxGetBorderColors(pCombo, &iBorder, &iHoverBorder,
		               &iFocusBorder) == XUI_OK &&
		               xuiComboBoxGetArrowColors(pCombo, &iArrow, &iDisabledArrow) == XUI_OK &&
		               xuiComboBoxGetButtonColors(pCombo, &iButton, &iButtonHover,
		               &iButtonOpen) == XUI_OK, "get combobox chrome colors for idempotence");
		XUI_TEST_CHECK(xuiComboBoxGetPopupColors(pCombo, &iPanel, &iPopupBorder, &iShadow,
		               &iHover, &iPopupText, &iHoverText, &iPopupDisabledText,
		               &iSeparator) == XUI_OK &&
		               xuiComboBoxGetMetrics(pCombo, &fItemHeight, &fBorderWidth) == XUI_OK,
		               "get combobox popup style for idempotence");
		xuiWidgetClearDirty(pCombo, XUI_WIDGET_DIRTY_ALL);
		xuiWidgetClearDirty(pMenu, XUI_WIDGET_DIRTY_ALL);
		xuiWidgetClearDirty(pPopup, XUI_WIDGET_DIRTY_ALL);
		iRet = xuiComboBoxSetPopupHeight(pCombo, xuiComboBoxGetPopupHeight(pCombo));
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetPopupMaxHeight(pCombo, xuiComboBoxGetPopupMaxHeight(pCombo));
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetPopupPlacement(pCombo, xuiComboBoxGetPopupPlacement(pCombo));
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetMetrics(pCombo, fItemHeight, fBorderWidth);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetColors(pCombo, iText, iDisabledText,
			iBackground, iHoverBackground, iOpenBackground, iDisabledBackground);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetBorderColors(pCombo, iBorder, iHoverBorder, iFocusBorder);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetArrowColors(pCombo, iArrow, iDisabledArrow);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetButtonColors(pCombo, iButton, iButtonHover, iButtonOpen);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetPopupColors(pCombo, iPanel, iPopupBorder,
			iShadow, iHover, iPopupText, iHoverText, iPopupDisabledText, iSeparator);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetFont(pCombo, xuiComboBoxGetFont(pCombo));
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetMaxLength(pCombo, xuiComboBoxGetMaxLength(pCombo));
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetEnabledItems(pCombo, arrCurrentEnabled, 5);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxClose(pCombo);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetDirtyFlags(pCombo) == 0u &&
		               xuiWidgetGetDirtyFlags(pMenu) == 0u &&
		               xuiWidgetGetDirtyFlags(pPopup) == 0u,
		               "same combobox setters and closed close are idempotent");
	}
	iRet = xuiComboBoxSetItems(pCombo, arrOversizedItems, XUI_COMBOBOX_ITEM_CAPACITY + 1);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && xuiComboBoxGetItemCount(pCombo) == 5, "oversized items rejected without truncation");
	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxIsOpen(pCombo), "open before ancestor disable");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo) && !xuiWidgetGetEffectiveEnabled(pCombo) &&
	               (xuiComboBoxGetState(pCombo) & XUI_WIDGET_STATE_DISABLED) != 0u,
	               "ancestor disable closes and disables combobox");
	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo), "effectively disabled combobox does not open");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetGetEffectiveEnabled(pCombo), "ancestor re-enable combobox");

	arrEnabled[0] = 1;
	arrEnabled[1] = 1;
	arrEnabled[2] = 0;
	arrEnabled[3] = 1;
	arrEnabled[4] = 0;
	{
		int iChangeCount = xuiComboBoxGetChangeCount(pCombo);
		iRet = xuiComboBoxSetEnabledItems(pCombo, arrEnabled, 5);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetSelected(pCombo) == -1 &&
		               xuiComboBoxIsItemEnabled(pCombo, 1) &&
		               xuiComboBoxGetChangeCount(pCombo) == iChangeCount + 1,
		               "enabled map clears selection and records change");
		xuiWidgetClearDirty(pCombo, XUI_WIDGET_DIRTY_ALL);
		xuiWidgetClearDirty(xuiComboBoxGetMenuWidget(pCombo), XUI_WIDGET_DIRTY_ALL);
		iRet = xuiComboBoxSetEnabledItems(pCombo, arrEnabled, 5);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetChangeCount(pCombo) == iChangeCount + 1 &&
		               xuiWidgetGetDirtyFlags(pCombo) == 0u &&
		               xuiWidgetGetDirtyFlags(xuiComboBoxGetMenuWidget(pCombo)) == 0u,
		               "same enabled map is idempotent");
	}
	iRet = xuiWidgetSetEnabled(pCombo, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable combo");
	iRet = xuiComboBoxOpen(pCombo);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiComboBoxIsOpen(pCombo), "disabled does not open");
	iState = xuiComboBoxGetState(pCombo);
	XUI_TEST_CHECK((iState & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");

	{
		xui_widget pModeCombo = NULL;
		xui_widget pModeInput;

		iRet = xuiComboBoxCreate(pContext, &pModeCombo, &tDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pModeCombo != NULL, "mode transfer combobox create");
		(void)xuiWidgetSetRect(pModeCombo, (xui_rect_t){36.0f, 140.0f, 160.0f, 30.0f});
		XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pModeCombo) == XUI_OK, "mode transfer combobox add");
		pModeInput = xuiComboBoxGetInputWidget(pModeCombo);
		XUI_TEST_CHECK(pModeInput != NULL && xuiSetFocusWidget(pContext, pModeCombo) == XUI_OK,
		               "mode transfer select focus");
		iRet = xuiComboBoxSetMaxLength(pModeCombo, 3);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxGetSelected(pModeCombo) == 0 &&
		               strcmp(xuiComboBoxGetText(pModeCombo), "Alpha") == 0,
		               "select mode max length preserves selected item");
		iRet = xuiComboBoxSetMode(pModeCombo, XUI_COMBOBOX_MODE_EDIT);
		XUI_TEST_CHECK(iRet == XUI_ERROR_BUFFER_TOO_SMALL &&
		               xuiComboBoxGetMode(pModeCombo) == XUI_COMBOBOX_MODE_SELECT &&
		               xuiComboBoxGetSelected(pModeCombo) == 0 &&
		               xuiGetFocusWidget(pContext) == pModeCombo &&
		               !xuiWidgetGetVisible(pModeInput),
		               "select to edit rejects truncated selection transactionally");
		iRet = xuiComboBoxSetMaxLength(pModeCombo, 0);
		if ( iRet == XUI_OK ) iRet = xuiComboBoxSetMode(pModeCombo, XUI_COMBOBOX_MODE_EDIT);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pModeInput &&
		               xuiWidgetGetVisible(pModeInput) &&
		               strcmp(xuiComboBoxGetText(pModeCombo), "Alpha") == 0,
		               "select to edit transfers focus and selected text to input");
		iRet = xuiComboBoxSetMode(pModeCombo, XUI_COMBOBOX_MODE_SELECT);
		XUI_TEST_CHECK(iRet == XUI_OK && xuiGetFocusWidget(pContext) == pModeCombo &&
		               !xuiWidgetGetVisible(pModeInput), "edit to select transfers focus to owner");
		xuiWidgetDestroy(pModeCombo);
	}

	tDestroy.ppWidget = &pDestroyCombo;
	iRet = xuiComboBoxCreate(pContext, &pDestroyCombo, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyCombo != NULL, "destroy callback combobox create");
	iRet = xuiWidgetSetRect(pDestroyCombo, (xui_rect_t){36.0f, 92.0f, 160.0f, 30.0f});
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetAddChild(pRoot, pDestroyCombo) == XUI_OK, "destroy callback combobox add");
	iRet = xuiComboBoxSetSelect(pDestroyCombo, __xuiComboBoxDestroyOnSelect, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiComboBoxOpen(pDestroyCombo) == XUI_OK && xuiLayout(pContext) == XUI_OK, "destroy callback combobox open");
	pDestroyMenu = xuiComboBoxGetMenuWidget(pDestroyCombo);
	pDestroyPopup = xuiComboBoxGetPopupWidget(pDestroyCombo);
	XUI_TEST_CHECK(pDestroyMenu != NULL && pDestroyPopup != NULL, "destroy callback popup handles");
	iRet = __xuiComboBoxClickMenuItem(pContext, pDestroyMenu, 3);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 && pDestroyCombo == NULL,
	               "selection callback may destroy combobox safely");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) != pDestroyMenu && xuiGetFocusWidget(pContext) != pDestroyPopup,
	               "destroying combobox clears owned popup focus");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiComboBoxRender(pContext, pTarget) == XUI_OK,
	               "context remains usable after callback destroys combobox popup tree");

	tTextDestroy.ppWidget = &pTextDestroyCombo;
	iRet = xuiComboBoxCreate(pContext, &pTextDestroyCombo, &tEditDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pTextDestroyCombo != NULL, "text destroy callback combobox create");
	iRet = xuiWidgetSetRect(pTextDestroyCombo, (xui_rect_t){216.0f, 92.0f, 164.0f, 30.0f});
	XUI_TEST_CHECK(iRet == XUI_OK && xuiWidgetAddChild(pRoot, pTextDestroyCombo) == XUI_OK,
	               "text destroy callback combobox add");
	iRet = xuiComboBoxSetTextChange(pTextDestroyCombo, __xuiComboBoxDestroyOnTextChange, &tTextDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "text destroy callback set");
	pTextDestroyInput = xuiComboBoxGetInputWidget(pTextDestroyCombo);
	XUI_TEST_CHECK(pTextDestroyInput != NULL && xuiSetFocusWidget(pContext, pTextDestroyInput) == XUI_OK,
	               "text destroy callback input focus");
	iRet = xuiInputText(pContext, '!');
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
	               tTextDestroy.iCount == 1 && pTextDestroyCombo == NULL,
	               "text callback may destroy combobox safely");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK && __xuiComboBoxRender(pContext, pTarget) == XUI_OK,
	               "context remains usable after text callback destroys combobox");

	{
		xui_combobox_test_event_destroy_t tFocusDestroy;
		xui_widget pFocusTrap = NULL;
		xui_widget pFocusDestroyCombo = NULL;
		xui_rect_t tWorld;

		memset(&tFocusDestroy, 0, sizeof(tFocusDestroy));
		tFocusDestroy.iEventType = XUI_EVENT_BLUR;
		tFocusDestroy.ppWidget = &pFocusDestroyCombo;
		iRet = xuiComboBoxCreate(pContext, &pFocusDestroyCombo, &tDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pFocusDestroyCombo != NULL, "focus destroy combobox create");
		(void)xuiWidgetSetRect(pFocusDestroyCombo, (xui_rect_t){36.0f, 140.0f, 160.0f, 30.0f});
		XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pFocusDestroyCombo) == XUI_OK, "focus destroy combobox add");
		XUI_TEST_CHECK(xuiWidgetCreate(pContext, &pFocusTrap) == XUI_OK && pFocusTrap != NULL,
		               "combobox focus trap create");
		(void)xuiWidgetSetFocusable(pFocusTrap, 1);
		(void)xuiWidgetSetTabStop(pFocusTrap, 1);
		(void)xuiWidgetSetRect(pFocusTrap, (xui_rect_t){300.0f, 140.0f, 80.0f, 30.0f});
		XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pFocusTrap) == XUI_OK, "combobox focus trap add");
		XUI_TEST_CHECK(xuiWidgetSetEventCallback(pFocusTrap, __xuiComboBoxDestroyVictimOnEvent,
		               &tFocusDestroy) == XUI_OK, "combobox focus trap callback");
		XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pFocusTrap) == XUI_OK &&
		               xuiDispatchPendingEvents(pContext) == XUI_OK, "combobox focus trap focus");
		XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "focus destroy combobox layout");
		tWorld = xuiWidgetGetWorldRect(pFocusDestroyCombo);
		iRet = __xuiComboBoxDispatchDown(pContext, tWorld.fX + 12.0f, tWorld.fY + tWorld.fH * 0.5f);
		XUI_TEST_CHECK(iRet == XUI_OK && tFocusDestroy.iCount == 1 && pFocusDestroyCombo == NULL,
		               "focus transition may destroy pressed combobox safely");
		(void)xuiInputPointerUp(pContext, tWorld.fX + 12.0f, tWorld.fY + tWorld.fH * 0.5f,
		                        XUI_POINTER_BUTTON_LEFT, 0);
		(void)xuiDispatchPendingEvents(pContext);
		xuiWidgetDestroy(pFocusTrap);
	}

	{
		xui_combobox_test_event_destroy_t tModeDestroy;
		xui_widget pModeDestroyCombo = NULL;
		xui_widget pModeInput;

		memset(&tModeDestroy, 0, sizeof(tModeDestroy));
		tModeDestroy.iEventType = XUI_EVENT_VISIBLE_CHANGED;
		tModeDestroy.ppWidget = &pModeDestroyCombo;
		iRet = xuiComboBoxCreate(pContext, &pModeDestroyCombo, &tDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pModeDestroyCombo != NULL, "mode state destroy combobox create");
		(void)xuiWidgetSetRect(pModeDestroyCombo, (xui_rect_t){36.0f, 184.0f, 160.0f, 30.0f});
		XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pModeDestroyCombo) == XUI_OK, "mode state destroy combobox add");
		pModeInput = xuiComboBoxGetInputWidget(pModeDestroyCombo);
		XUI_TEST_CHECK(pModeInput != NULL && xuiWidgetSetEventHandler(pModeInput,
		               XUI_EVENT_VISIBLE_CHANGED, __xuiComboBoxDestroyVictimOnEvent, &tModeDestroy) == XUI_OK,
		               "mode state destroy callback");
		iRet = xuiComboBoxSetMode(pModeDestroyCombo, XUI_COMBOBOX_MODE_EDIT);
		if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
		XUI_TEST_CHECK(iRet == XUI_OK && tModeDestroy.iCount == 1 && pModeDestroyCombo == NULL,
		               "mode state callback may destroy combobox safely");
	}

	{
		xui_combobox_test_event_destroy_t tCommitDestroy;
		xui_widget pCommitDestroyCombo = NULL;
		xui_widget pCommitInput;
		xui_widget pCommitMenu;

		memset(&tCommitDestroy, 0, sizeof(tCommitDestroy));
		tCommitDestroy.iEventType = XUI_EVENT_FOCUS;
		tCommitDestroy.ppWidget = &pCommitDestroyCombo;
		iRet = xuiComboBoxCreate(pContext, &pCommitDestroyCombo, &tEditDesc);
		XUI_TEST_CHECK(iRet == XUI_OK && pCommitDestroyCombo != NULL, "commit focus destroy combobox create");
		(void)xuiWidgetSetRect(pCommitDestroyCombo, (xui_rect_t){216.0f, 184.0f, 164.0f, 30.0f});
		XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pCommitDestroyCombo) == XUI_OK, "commit focus destroy combobox add");
		pCommitInput = xuiComboBoxGetInputWidget(pCommitDestroyCombo);
		XUI_TEST_CHECK(pCommitInput != NULL && xuiSetFocusWidget(pContext, pCommitInput) == XUI_OK &&
		               xuiDispatchPendingEvents(pContext) == XUI_OK, "commit focus destroy input focus");
		XUI_TEST_CHECK(xuiComboBoxOpen(pCommitDestroyCombo) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		               "commit focus destroy dropdown open");
		pCommitMenu = xuiComboBoxGetMenuWidget(pCommitDestroyCombo);
		XUI_TEST_CHECK(pCommitMenu != NULL && xuiWidgetSetEventHandler(pCommitInput, XUI_EVENT_FOCUS,
		               __xuiComboBoxDestroyVictimOnEvent, &tCommitDestroy) == XUI_OK,
		               "commit focus destroy callback");
		iRet = __xuiComboBoxClickMenuItem(pContext, pCommitMenu, 0);
		XUI_TEST_CHECK(iRet == XUI_OK && tCommitDestroy.iCount == 1 && pCommitDestroyCombo == NULL,
		               "dropdown close focus callback may destroy combobox safely");
	}

	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK && __xuiComboBoxRender(pContext, pTarget) == XUI_OK,
	               "context remains usable after combobox focus callbacks destroy controls");

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
