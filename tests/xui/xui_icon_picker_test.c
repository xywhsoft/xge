#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_icon_picker_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_icon_picker_test_change_t {
	int iCount;
	xui_icon_id iOldId;
	xui_icon_id iNewId;
	int iFormatCount;
} xui_icon_picker_test_change_t;

static void __xuiIconPickerChanged(xui_widget pWidget, xui_icon_id iOldId, xui_icon_id iNewId, void* pUser)
{
	xui_icon_picker_test_change_t* pChange;

	(void)pWidget;
	pChange = (xui_icon_picker_test_change_t*)pUser;
	pChange->iCount++;
	pChange->iOldId = iOldId;
	pChange->iNewId = iNewId;
}

static int __xuiIconPickerFormat(xui_widget pWidget, xui_icon pIcon, char* sBuffer, int iCapacity, void* pUser)
{
	xui_icon_picker_test_change_t* pChange;

	(void)pWidget;
	pChange = (xui_icon_picker_test_change_t*)pUser;
	pChange->iFormatCount++;
	snprintf(sBuffer, (size_t)iCapacity, "icon:%u", (unsigned)xuiIconGetId(pIcon));
	return XUI_OK;
}

static int __xuiIconPickerRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 480};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiIconPickerDispatchKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiIconPickerDispatchClick(xui_context pContext, float fX, float fY)
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

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_icon_picker_test_change_t tChange;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pPopup;
	xui_widget pFrame;
	xui_widget pViewport;
	xui_surface pTarget;
	xui_font pFont;
	xui_icon_category pCategory;
	xui_icon_picker_desc_t tDesc;
	xui_icon_desc_t tIconDesc;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tPopupRect;
	xui_rect_t tViewportRect;
	xui_icon_id arrIds[15];
	char sName[32];
	float fOffsetY;
	int iPolicyX;
	int iPolicyY;
	int iRefBefore;
	int iFailed;
	int iRet;
	int i;

	memset(&tState, 0, sizeof(tState));
	memset(&tChange, 0, sizeof(tChange));
	memset(arrIds, 0, sizeof(arrIds));
	pContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pTarget = NULL;
	pFont = NULL;
	pCategory = NULL;
	iFailed = 0;
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "iconpicker", 10, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	XUI_TEST_CHECK(xuiSetDefaultFont(pContext, pFont) == XUI_OK, "default font");
	XUI_TEST_CHECK(xuiInputViewport(pContext, 640.0f, 480.0f) == XUI_OK, "viewport");
	XUI_TEST_CHECK(xuiWidgetCreate(pContext, &pRoot) == XUI_OK, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 640.0f, 480.0f});
	XUI_TEST_CHECK(xuiSetRootWidget(pContext, pRoot) == XUI_OK, "set root");

	XUI_TEST_CHECK(xuiIconCategoryCreate(pContext, "picker_test", NULL, &pCategory) == XUI_OK, "category create");
	xuiIconDescDefault(&tIconDesc);
	for ( i = 0; i < 15; i++ ) {
		xui_icon pIcon;
		snprintf(sName, sizeof(sName), "icon_%02d", i + 1);
		tIconDesc.sDisplayName = sName;
		XUI_TEST_CHECK(xuiIconAddSvgPath(
			pCategory,
			sName,
			"M2 2 H22 V22 H2 Z",
			(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f},
			NULL,
			&tIconDesc,
			&pIcon) == XUI_OK, "icon add");
		arrIds[i] = xuiIconGetId(pIcon);
	}
	iRefBefore = xuiIconCategoryGetRefCount(pCategory);

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pCategory = pCategory;
	tDesc.iSelectedId = arrIds[1];
	tDesc.iTextMode = XUI_ICON_PICKER_TEXT_ID;
	tDesc.iVisibleRows = 2;
	tDesc.iVisibleColumns = 3;
	tDesc.fCellWidth = 40.0f;
	tDesc.fCellHeight = 36.0f;
	tDesc.fGapX = 4.0f;
	tDesc.fGapY = 4.0f;
	tDesc.fIconPadding = 6.0f;
	tDesc.fScrollbarSize = 12.0f;
	tDesc.sPlaceholder = "Choose icon";
	tDesc.pFont = pFont;
	XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pPicker, &tDesc) == XUI_OK && pPicker != NULL, "picker create");
	XUI_TEST_CHECK(xuiIconCategoryGetRefCount(pCategory) == iRefBefore + 1, "picker retains category");
	XUI_TEST_CHECK(xuiIconPickerSetChange(pPicker, __xuiIconPickerChanged, &tChange) == XUI_OK, "change callback");
	xuiWidgetSetRect(pPicker, (xui_rect_t){32.0f, 24.0f, 210.0f, 34.0f});
	XUI_TEST_CHECK(xuiWidgetAddChild(pRoot, pPicker) == XUI_OK, "add picker");

	XUI_TEST_CHECK(xuiTestSurfaceCreate(&tState, &pTarget, 640, 480, XUI_SURFACE_USAGE_TARGET) == XUI_OK, "target create");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "layout");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "update");
	XUI_TEST_CHECK(__xuiIconPickerRender(pContext, pTarget) == XUI_OK, "closed render");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == arrIds[1], "initial selected id");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedIcon(pPicker) == xuiIconFindById(pCategory, arrIds[1]), "selected icon");
	XUI_TEST_CHECK(xuiIconPickerGetTextMode(pPicker) == XUI_ICON_PICKER_TEXT_ID, "text mode");
	XUI_TEST_CHECK(strcmp(xuiIconPickerGetPlaceholder(pPicker), "Choose icon") == 0, "placeholder");
	tRect = xuiIconPickerGetValueRect(pPicker);
	XUI_TEST_CHECK(tRect.fW > 140.0f && tRect.fH > 20.0f, "value rect");
	tRect = xuiIconPickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(tRect.fW >= 24.0f && tRect.fH == 34.0f, "button rect");

	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK && xuiIconPickerIsOpen(pPicker), "open");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "open layout");
	XUI_TEST_CHECK(__xuiIconPickerRender(pContext, pTarget) == XUI_OK, "open render");
	pPopup = xuiIconPickerGetPopupWidget(pPicker);
	pFrame = xuiIconPickerGetFrameWidget(pPicker);
	pViewport = xuiIconPickerGetViewportWidget(pPicker);
	XUI_TEST_CHECK(pPopup != NULL && pFrame != NULL && pViewport != NULL, "popup parts");
	XUI_TEST_CHECK(xuiGetFocusWidget(pContext) == pViewport, "viewport focus");
	XUI_TEST_CHECK((xuiIconPickerGetState(pPicker) & XUI_ICON_PICKER_STATE_OPEN) != 0u, "open state");
	XUI_TEST_CHECK(xuiScrollFrameGetScrollbarPolicy(pFrame, &iPolicyX, &iPolicyY) == XUI_OK, "scroll policy");
	XUI_TEST_CHECK(iPolicyX == XUI_SCROLLBAR_POLICY_HIDDEN && iPolicyY == XUI_SCROLLBAR_POLICY_AUTO, "vertical only");
	tPopupRect = xuiPopupGetPopupRect(pPopup);
	tViewportRect = xuiScrollFrameGetViewportRect(pFrame);
	XUI_TEST_CHECK(tPopupRect.fW >= 145.0f, "popup reserves scrollbar width");
	XUI_TEST_CHECK(tViewportRect.fW >= 127.0f && tViewportRect.fW <= 129.0f, "grid viewport width preserved");
	XUI_TEST_CHECK(xuiWidgetGetRect(pFrame).fW - tViewportRect.fW >= 7.0f, "scrollbar uses extra width");
	tRect = xuiIconPickerGetItemRect(pPicker, 4);
	XUI_TEST_CHECK(tRect.fX == 44.0f && tRect.fY == 40.0f && tRect.fW == 40.0f && tRect.fH == 36.0f, "item grid rect");

	XUI_TEST_CHECK(__xuiIconPickerDispatchKey(pContext, XUI_KEY_RIGHT) == XUI_OK, "keyboard right");
	XUI_TEST_CHECK(xuiIconPickerGetFocusIndex(pPicker) == 2, "focus right");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKey(pContext, XUI_KEY_DOWN) == XUI_OK, "keyboard down");
	XUI_TEST_CHECK(xuiIconPickerGetFocusIndex(pPicker) == 5, "focus down");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKey(pContext, XUI_KEY_ENTER) == XUI_OK, "keyboard commit");
	XUI_TEST_CHECK(!xuiIconPickerIsOpen(pPicker), "commit closes popup");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == arrIds[5], "keyboard selected id");
	XUI_TEST_CHECK(tChange.iCount == 1 && tChange.iOldId == arrIds[1] && tChange.iNewId == arrIds[5], "keyboard callback");

	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK, "reopen");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "reopen layout");
	tRect = xuiIconPickerGetItemRect(pPicker, 0);
	tWorld = xuiWidgetGetWorldRect(pViewport);
	XUI_TEST_CHECK(__xuiIconPickerDispatchClick(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f) == XUI_OK, "mouse select");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == arrIds[0] && tChange.iCount == 2, "mouse selected");

	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK, "open for ensure");
	XUI_TEST_CHECK(xuiIconPickerEnsureVisible(pPicker, arrIds[14]) == XUI_OK, "ensure last visible");
	fOffsetY = 0.0f;
	XUI_TEST_CHECK(xuiScrollFrameGetOffset(pFrame, NULL, &fOffsetY) == XUI_OK && fOffsetY > 0.0f, "ensure scrolls");

	XUI_TEST_CHECK(xuiIconPickerSetFormatter(pPicker, __xuiIconPickerFormat, &tChange) == XUI_OK, "formatter set");
	XUI_TEST_CHECK(xuiIconPickerSetTextMode(pPicker, XUI_ICON_PICKER_TEXT_CUSTOM) == XUI_OK, "custom mode");
	XUI_TEST_CHECK(__xuiIconPickerRender(pContext, pTarget) == XUI_OK && tChange.iFormatCount > 0, "custom formatter used");
	XUI_TEST_CHECK(xuiIconPickerSetTextMode(pPicker, XUI_ICON_PICKER_TEXT_NONE) == XUI_OK, "icon only mode");
	XUI_TEST_CHECK(xuiIconPickerGetTextMode(pPicker) == XUI_ICON_PICKER_TEXT_NONE, "icon only get");

	XUI_TEST_CHECK(xuiIconPickerSetSelectedId(pPicker, arrIds[4]) == XUI_OK, "set before remove");
	XUI_TEST_CHECK(xuiIconRemoveById(pCategory, arrIds[4]) == XUI_OK, "remove selected icon");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "update category generation");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == XUI_ICON_ID_INVALID, "removed selection clears");
	XUI_TEST_CHECK(tChange.iCount == 2, "external removal does not notify during update");

	XUI_TEST_CHECK(xuiIconPickerClose(pPicker) == XUI_OK, "close");
	XUI_TEST_CHECK(xuiIconPickerSetGrid(pPicker, 3, 4) == XUI_OK, "set grid");
	XUI_TEST_CHECK(xuiIconPickerGetGrid(pPicker, &iPolicyY, &iPolicyX) == XUI_OK && iPolicyY == 3 && iPolicyX == 4, "get grid");
	XUI_TEST_CHECK(xuiIconPickerSetCategory(pPicker, NULL) == XUI_OK, "clear category");
	XUI_TEST_CHECK(xuiIconCategoryGetRefCount(pCategory) == iRefBefore, "clear category releases ref");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == XUI_ICON_ID_INVALID, "category clear selection");

cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_icon_picker_test passed\n");
	return 0;
}
