#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_icon_picker_test_destroy_t {
	xui_widget* ppWidget;
	int iCount;
} xui_icon_picker_test_destroy_t;

typedef struct xui_icon_picker_test_popup_destroy_t {
	xui_widget* ppWidget;
	int bOpen;
	int iCount;
} xui_icon_picker_test_popup_destroy_t;

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

static void __xuiIconPickerDestroyOnChange(xui_widget pWidget, xui_icon_id iOldId, xui_icon_id iNewId, void* pUser)
{
	xui_icon_picker_test_destroy_t* pDestroy;

	(void)iOldId;
	(void)iNewId;
	pDestroy = (xui_icon_picker_test_destroy_t*)pUser;
	if ( pDestroy == NULL ) return;
	pDestroy->iCount++;
	xuiWidgetDestroy(pWidget);
	if ( pDestroy->ppWidget != NULL && *pDestroy->ppWidget == pWidget ) *pDestroy->ppWidget = NULL;
}

static void __xuiIconPickerDestroyVictimOnPopupChange(xui_widget pWidget, int bOpen, void* pUser)
{
	xui_icon_picker_test_popup_destroy_t* pDestroy;
	xui_widget pVictim;

	(void)pWidget;
	pDestroy = (xui_icon_picker_test_popup_destroy_t*)pUser;
	if ( pDestroy == NULL || bOpen != pDestroy->bOpen || pDestroy->ppWidget == NULL ||
	     *pDestroy->ppWidget == NULL ) return;
	pVictim = *pDestroy->ppWidget;
	*pDestroy->ppWidget = NULL;
	pDestroy->iCount++;
	xuiWidgetDestroy(pVictim);
}

static int __xuiIconPickerRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 640, 480};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiIconPickerDispatchKeyWithModifiers(xui_context pContext, int iKey, uint32_t iModifiers)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, iModifiers);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiIconPickerDispatchKey(xui_context pContext, int iKey)
{
	return __xuiIconPickerDispatchKeyWithModifiers(pContext, iKey, 0u);
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

static int __xuiIconPickerSameValueSettersStayClean(xui_widget pPicker,
	xui_icon_picker_format_proc onFormat, void* pFormatUser)
{
	xui_widget pPopup;
	xui_widget pFrame;
	xui_widget pViewport;
	xui_icon_category pCategory;
	xui_font pFont;
	xui_thickness_t tPadding;
	float fCellWidth;
	float fCellHeight;
	float fGapX;
	float fGapY;
	float fIconPadding;
	float fBorderWidth;
	float fScrollbarSize;
	uint32_t arrColors[7];
	uint32_t arrBorderColors[3];
	uint32_t arrArrowColors[2];
	uint32_t arrButtonColors[3];
	uint32_t arrPopupColors[6];
	int iRows;
	int iColumns;
	int iRet;

	pPopup = xuiIconPickerGetPopupWidget(pPicker);
	pFrame = xuiIconPickerGetFrameWidget(pPicker);
	pViewport = xuiIconPickerGetViewportWidget(pPicker);
	if ( pPopup == NULL || pFrame == NULL || pViewport == NULL ) return 0;
	pCategory = xuiIconPickerGetCategory(pPicker);
	pFont = xuiIconPickerGetFont(pPicker);
	tPadding = xuiIconPickerGetValuePadding(pPicker);
	iRet = xuiIconPickerGetGrid(pPicker, &iRows, &iColumns);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetCellSize(pPicker, &fCellWidth, &fCellHeight);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetSpacing(pPicker, &fGapX, &fGapY, &fIconPadding);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetMetrics(pPicker, &fBorderWidth, &fScrollbarSize);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetColors(pPicker,
		&arrColors[0], &arrColors[1], &arrColors[2], &arrColors[3],
		&arrColors[4], &arrColors[5], &arrColors[6]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetBorderColors(pPicker,
		&arrBorderColors[0], &arrBorderColors[1], &arrBorderColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetArrowColors(pPicker,
		&arrArrowColors[0], &arrArrowColors[1]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetButtonColors(pPicker,
		&arrButtonColors[0], &arrButtonColors[1], &arrButtonColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerGetPopupColors(pPicker,
		&arrPopupColors[0], &arrPopupColors[1], &arrPopupColors[2],
		&arrPopupColors[3], &arrPopupColors[4], &arrPopupColors[5]);
	if ( iRet != XUI_OK ) return 0;
	xuiWidgetClearDirty(pPicker, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pPopup, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pFrame, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pViewport, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiIconPickerSetFormatter(pPicker, onFormat, pFormatUser);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetCategory(pPicker, pCategory);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetTextMode(pPicker, xuiIconPickerGetTextMode(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetPlaceholder(pPicker, xuiIconPickerGetPlaceholder(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetGrid(pPicker, iRows, iColumns);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetCellSize(pPicker, fCellWidth, fCellHeight);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetSpacing(pPicker, fGapX, fGapY, fIconPadding);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetValuePadding(pPicker, tPadding);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetPopupPlacement(pPicker, xuiIconPickerGetPopupPlacement(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetMetrics(pPicker, fBorderWidth, fScrollbarSize);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetColors(pPicker,
		arrColors[0], arrColors[1], arrColors[2], arrColors[3],
		arrColors[4], arrColors[5], arrColors[6]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetBorderColors(pPicker,
		arrBorderColors[0], arrBorderColors[1], arrBorderColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetArrowColors(pPicker,
		arrArrowColors[0], arrArrowColors[1]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetButtonColors(pPicker,
		arrButtonColors[0], arrButtonColors[1], arrButtonColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetPopupColors(pPicker,
		arrPopupColors[0], arrPopupColors[1], arrPopupColors[2],
		arrPopupColors[3], arrPopupColors[4], arrPopupColors[5]);
	if ( iRet == XUI_OK ) iRet = xuiIconPickerSetFont(pPicker, pFont);
	return iRet == XUI_OK && xuiWidgetGetDirtyFlags(pPicker) == 0u &&
		xuiWidgetGetDirtyFlags(pPopup) == 0u && xuiWidgetGetDirtyFlags(pFrame) == 0u &&
		xuiWidgetGetDirtyFlags(pViewport) == 0u;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_icon_picker_test_change_t tChange;
	xui_icon_picker_test_destroy_t tDestroy;
	xui_icon_picker_test_popup_destroy_t tOpenDestroy;
	xui_icon_picker_test_popup_destroy_t tCloseDestroy;
	xui_context pContext;
	xui_context pOtherContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pDestroyPicker;
	xui_widget pFocusDestroyPicker;
	xui_widget pCloseDestroyPicker;
	xui_widget pPopup;
	xui_widget pFrame;
	xui_widget pViewport;
	xui_surface pTarget;
	xui_font pFont;
	xui_icon_category pCategory;
	xui_icon_category pOtherCategory;
	xui_icon_picker_desc_t tDesc;
	xui_icon_picker_desc_t tDestroyDesc;
	xui_icon_desc_t tIconDesc;
	xui_icon_desc_t tLongIconDesc;
	xui_event_t tEvent;
	xui_rect_t tRect;
	xui_rect_t tWorld;
	xui_rect_t tPopupRect;
	xui_rect_t tViewportRect;
	xui_accessible_node_t tAccessible;
	xui_icon_id arrIds[15];
	xui_icon_id iMutationA;
	xui_icon_id iMutationB;
	xui_icon_id iSelectedBefore;
	xui_accessible_value_t tAccessibleValue;
	xui_vec2_t tMeasured;
	char sName[32];
	char sLongDisplay[300];
	float fBorderWidth;
	float fCellHeight;
	float fCellWidth;
	float fGapX;
	float fGapY;
	float fIconPadding;
	float fOffsetY;
	float fScrollbarSize;
	int iPolicyX;
	int iPolicyY;
	int iRefBefore;
	int iRefSame;
	int iChangeBefore;
	int iFocusBefore;
	int iFailed;
	int iRet;
	int i;

	memset(&tState, 0, sizeof(tState));
	memset(&tChange, 0, sizeof(tChange));
	memset(&tDestroy, 0, sizeof(tDestroy));
	memset(&tOpenDestroy, 0, sizeof(tOpenDestroy));
	memset(&tCloseDestroy, 0, sizeof(tCloseDestroy));
	memset(arrIds, 0, sizeof(arrIds));
	iMutationA = XUI_ICON_ID_INVALID;
	iMutationB = XUI_ICON_ID_INVALID;
	pContext = NULL;
	pOtherContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pDestroyPicker = NULL;
	pFocusDestroyPicker = NULL;
	pCloseDestroyPicker = NULL;
	pTarget = NULL;
	pFont = NULL;
	pCategory = NULL;
	pOtherCategory = NULL;
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
	tDesc.fScrollbarSize = 8.0f;
	tDesc.sPlaceholder = "Choose icon";
	tDesc.pFont = pFont;
	{
		xui_icon_picker_desc_t tInvalidDesc = tDesc;
		xui_widget pInvalid = (xui_widget)(uintptr_t)1u;
		tInvalidDesc.iVisibleRows = 1025;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pInvalid, &tInvalidDesc) == XUI_ERROR_INVALID_ARGUMENT,
			"oversized descriptor grid rejected");
		XUI_TEST_CHECK(pInvalid == NULL, "invalid descriptor clears output widget");
		tInvalidDesc = tDesc;
		tInvalidDesc.fIconPadding = 18.0f;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pInvalid, &tInvalidDesc) == XUI_ERROR_INVALID_ARGUMENT,
			"descriptor padding must fit cell");
		tInvalidDesc = tDesc;
		tInvalidDesc.fCellWidth = NAN;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pInvalid, &tInvalidDesc) == XUI_ERROR_INVALID_ARGUMENT,
			"non-finite descriptor metric rejected");
		tInvalidDesc = tDesc;
		tInvalidDesc.fScrollbarSize = 12.0f;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pInvalid, &tInvalidDesc) == XUI_ERROR_INVALID_ARGUMENT,
			"compact scrollbar size above supported maximum rejected");
		tInvalidDesc = tDesc;
		tInvalidDesc.fCellWidth = XUI_LAYOUT_UNBOUNDED;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pInvalid, &tInvalidDesc) == XUI_ERROR_INVALID_ARGUMENT,
			"descriptor rejects oversized combined grid geometry");
	}
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
	XUI_TEST_CHECK(xuiIconPickerGetMetrics(pPicker, &fBorderWidth, &fScrollbarSize) == XUI_OK &&
		fScrollbarSize == 8.0f, "descriptor scrollbar size retained");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible) == XUI_OK &&
		tAccessible.iRole == XUI_ACCESSIBLE_ROLE_COMBOBOX &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_COLLAPSED) != 0u &&
		strcmp(tAccessible.sValue, "icon_02") == 0,
		"accessible combobox exposes selected icon");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, 1, XUI_ACCESSIBLE_ACTION_EXPAND, NULL) == XUI_OK &&
		xuiIconPickerIsOpen(pPicker), "accessible expand opens picker");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, 1, XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL) == XUI_OK &&
		!xuiIconPickerIsOpen(pPicker), "accessible collapse closes picker");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pPicker) == XUI_OK &&
		__xuiIconPickerDispatchKey(pContext, XUI_KEY_F4) == XUI_OK && xuiIconPickerIsOpen(pPicker),
		"F4 opens icon picker");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKeyWithModifiers(pContext, XUI_KEY_F4, XUI_MOD_ALT) == XUI_OK &&
		xuiIconPickerIsOpen(pPicker), "Alt+F4 is not consumed as an icon picker shortcut");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKey(pContext, XUI_KEY_F4) == XUI_OK && !xuiIconPickerIsOpen(pPicker),
		"F4 closes icon picker from popup viewport");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKeyWithModifiers(pContext, XUI_KEY_DOWN, XUI_MOD_ALT) == XUI_OK &&
		xuiIconPickerIsOpen(pPicker), "Alt+Down opens icon picker");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKeyWithModifiers(pContext, XUI_KEY_DOWN, XUI_MOD_ALT) == XUI_OK &&
		xuiIconPickerIsOpen(pPicker), "Alt+Down keeps open icon picker open");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKeyWithModifiers(pContext, XUI_KEY_UP, XUI_MOD_ALT) == XUI_OK &&
		!xuiIconPickerIsOpen(pPicker), "Alt+Up closes icon picker");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKeyWithModifiers(pContext, XUI_KEY_UP, XUI_MOD_ALT) == XUI_OK &&
		!xuiIconPickerIsOpen(pPicker), "Alt+Up keeps closed icon picker closed");
	tRect = xuiIconPickerGetValueRect(pPicker);
	XUI_TEST_CHECK(tRect.fW > 140.0f && tRect.fH > 20.0f, "value rect");
	tRect = xuiIconPickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(tRect.fW >= 24.0f && tRect.fH == 34.0f, "button rect");
	XUI_TEST_CHECK(xuiWidgetMeasureContent(pPicker, (xui_vec2_t){60.0f, 12.0f}, &tMeasured) == XUI_OK &&
		tMeasured.fX == 60.0f && tMeasured.fY == 12.0f,
		"content measure respects tight constraints");
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){32.0f, 24.0f, 7.0f, 3.0f}) == XUI_OK,
		"set tiny icon picker rect");
	tRect = xuiIconPickerGetButtonRect(pPicker);
	tPopupRect = xuiIconPickerGetValueRect(pPicker);
	XUI_TEST_CHECK(tRect.fX >= 0.0f && tRect.fY >= 0.0f &&
		tRect.fX + tRect.fW <= 7.0f && tRect.fY + tRect.fH <= 3.0f &&
		tPopupRect.fX >= 0.0f && tPopupRect.fY >= 0.0f &&
		tPopupRect.fX + tPopupRect.fW <= 7.0f && tPopupRect.fY + tPopupRect.fH <= 3.0f,
		"tiny icon picker geometry stays inside the widget");
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){32.0f, 24.0f, 210.0f, 34.0f}) == XUI_OK,
		"restore icon picker rect");

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
	iFocusBefore = xuiIconPickerGetFocusIndex(pPicker);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pViewport;
	tEvent.iKey = XUI_KEY_RIGHT;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiIconPickerIsOpen(pPicker) &&
		xuiIconPickerGetFocusIndex(pPicker) == iFocusBefore,
		"closed viewport ignores directly targeted keyboard input");

	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK, "reopen");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK, "reopen layout");
	tRect = xuiIconPickerGetItemRect(pPicker, 0);
	tWorld = xuiWidgetGetWorldRect(pViewport);
	XUI_TEST_CHECK(xuiInputPointerDownEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH) == pViewport,
		"icon picker owner touch captures item press");
	XUI_TEST_CHECK(xuiInputPointerDownEx(pContext, 902u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiInputPointerUpEx(pContext, 902u, XUI_POINTER_TYPE_TOUCH,
			(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
			(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
			XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiIconPickerGetSelectedId(pPicker) == arrIds[5] && xuiIconPickerIsOpen(pPicker) &&
		xuiGetPointerCaptureEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH) == pViewport &&
		xuiGetPointerCaptureEx(pContext, 902u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot commit icon or end owner capture");
	XUI_TEST_CHECK(xuiInputPointerMoveEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX - 12.0f), (int)(tWorld.fY - 12.0f),
		XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiInputPointerUpEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH,
			(int)(tWorld.fX - 12.0f), (int)(tWorld.fY - 12.0f),
			XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 901u, XUI_POINTER_TYPE_TOUCH) == NULL &&
		xuiIconPickerGetSelectedId(pPicker) == arrIds[5] && xuiIconPickerIsOpen(pPicker),
		"icon picker owner touch releases cancelled item press");
	XUI_TEST_CHECK(xuiInputPointerDown(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiIconPickerGetSelectedId(pPicker) == arrIds[5] && xuiIconPickerIsOpen(pPicker) &&
		xuiGetPointerCapture(pContext) == pViewport,
		"pointer down focuses item without committing");
	XUI_TEST_CHECK(xuiInputPointerMove(pContext, tWorld.fX - 12.0f, tWorld.fY - 12.0f,
		XUI_POINTER_BUTTON_LEFT) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"drag outside picker item");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext, tWorld.fX - 12.0f, tWorld.fY - 12.0f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"drag-out pointer release dispatch");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == arrIds[5],
		"drag-out release preserves selection");
	XUI_TEST_CHECK(xuiIconPickerIsOpen(pPicker), "drag-out release keeps popup open");
	XUI_TEST_CHECK(xuiGetPointerCapture(pContext) == NULL, "drag-out release clears pointer capture");
	XUI_TEST_CHECK(__xuiIconPickerDispatchClick(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f) == XUI_OK, "mouse select");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == arrIds[0] && tChange.iCount == 2, "mouse selected");
	{
		xui_icon pMutationIcon;
		tIconDesc.sDisplayName = "mutation-a";
		XUI_TEST_CHECK(xuiIconAddSvgPath(pCategory, "mutation-a", "M2 2 H22 V22 H2 Z",
			(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f}, NULL, &tIconDesc, &pMutationIcon) == XUI_OK,
			"add first mutation icon");
		iMutationA = xuiIconGetId(pMutationIcon);
		tIconDesc.sDisplayName = "mutation-b";
		XUI_TEST_CHECK(xuiIconAddSvgPath(pCategory, "mutation-b", "M2 2 H22 V22 H2 Z",
			(xui_rect_t){0.0f, 0.0f, 24.0f, 24.0f}, NULL, &tIconDesc, &pMutationIcon) == XUI_OK,
			"add second mutation icon");
		iMutationB = xuiIconGetId(pMutationIcon);
	}
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK && xuiIconPickerOpen(pPicker) == XUI_OK &&
		xuiLayout(pContext) == XUI_OK && xuiIconPickerEnsureVisible(pPicker, iMutationA) == XUI_OK,
		"open after category growth");
	tRect = xuiIconPickerGetItemRect(pPicker, 15);
	tWorld = xuiWidgetGetWorldRect(pViewport);
	iSelectedBefore = xuiIconPickerGetSelectedId(pPicker);
	iChangeBefore = tChange.iCount;
	XUI_TEST_CHECK(xuiInputPointerDown(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiGetPointerCapture(pContext) == pViewport,
		"category mutation test captures pressed icon");
	XUI_TEST_CHECK(xuiIconRemoveById(pCategory, iMutationA) == XUI_OK &&
		xuiUpdate(pContext, 0.016f) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiGetPointerCapture(pContext) == NULL,
		"category mutation cancels stale item press");
	XUI_TEST_CHECK(xuiInputPointerUp(pContext,
		tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK &&
		xuiIconPickerGetSelectedId(pPicker) == iSelectedBefore && tChange.iCount == iChangeBefore &&
		xuiIconPickerIsOpen(pPicker),
		"release after category mutation cannot commit replacement icon");
	XUI_TEST_CHECK(xuiIconRemoveById(pCategory, iMutationB) == XUI_OK &&
		xuiUpdate(pContext, 0.016f) == XUI_OK, "remove remaining mutation icon");

	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK, "open for ensure");
	XUI_TEST_CHECK(xuiIconPickerEnsureVisible(pPicker, arrIds[14]) == XUI_OK, "ensure last visible");
	fOffsetY = 0.0f;
	XUI_TEST_CHECK(xuiScrollFrameGetOffset(pFrame, NULL, &fOffsetY) == XUI_OK && fOffsetY > 0.0f, "ensure scrolls");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 0) == XUI_OK && !xuiIconPickerIsOpen(pPicker) &&
		!xuiWidgetGetEffectiveEnabled(pPicker) &&
		(xuiIconPickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u,
		"ancestor disable closes and disables icon picker");
	XUI_TEST_CHECK(xuiIconPickerOpen(pPicker) == XUI_OK && !xuiIconPickerIsOpen(pPicker),
		"effectively disabled icon picker does not open");
	XUI_TEST_CHECK(xuiWidgetSetEnabled(pRoot, 1) == XUI_OK && xuiWidgetGetEffectiveEnabled(pPicker),
		"ancestor re-enable icon picker");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pPicker, 0) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"hide icon picker");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pPicker;
	tEvent.iKey = XUI_KEY_F4;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiIconPickerIsOpen(pPicker),
		"hidden icon picker ignores directly targeted key input");
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = 36.0f;
	tEvent.fY = 30.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiIconPickerIsOpen(pPicker),
		"hidden icon picker ignores directly targeted pointer input");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pPicker, 1) == XUI_OK && xuiDispatchPendingEvents(pContext) == XUI_OK,
		"restore icon picker visibility");

	XUI_TEST_CHECK(xuiIconPickerSetFormatter(pPicker, __xuiIconPickerFormat, &tChange) == XUI_OK, "formatter set");
	XUI_TEST_CHECK(xuiIconPickerSetTextMode(pPicker, XUI_ICON_PICKER_TEXT_CUSTOM) == XUI_OK, "custom mode");
	XUI_TEST_CHECK(__xuiIconPickerRender(pContext, pTarget) == XUI_OK && tChange.iFormatCount > 0, "custom formatter used");
	XUI_TEST_CHECK(xuiIconPickerSetTextMode(pPicker, XUI_ICON_PICKER_TEXT_NONE) == XUI_OK, "icon only mode");
	XUI_TEST_CHECK(xuiIconPickerGetTextMode(pPicker) == XUI_ICON_PICKER_TEXT_NONE, "icon only get");
	XUI_TEST_CHECK(__xuiIconPickerSameValueSettersStayClean(pPicker, __xuiIconPickerFormat, &tChange),
		"same-value setters do not dirty icon picker or popup widgets");
	XUI_TEST_CHECK(xuiIconPickerSetGrid(pPicker, 1025, 2) == XUI_ERROR_INVALID_ARGUMENT,
		"oversized grid rejected");
	XUI_TEST_CHECK(xuiIconPickerSetCellSize(pPicker, 10.0f, 10.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"cell smaller than icon padding rejected");
	XUI_TEST_CHECK(xuiIconPickerSetSpacing(pPicker, NAN, 4.0f, 2.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"non-finite spacing rejected");
	XUI_TEST_CHECK(xuiIconPickerSetMetrics(pPicker, 1.0f, 12.0f) == XUI_ERROR_INVALID_ARGUMENT,
		"unsupported compact scrollbar size rejected");
	XUI_TEST_CHECK(xuiIconPickerGetCellSize(pPicker, &fCellWidth, &fCellHeight) == XUI_OK &&
		xuiIconPickerSetCellSize(pPicker, XUI_LAYOUT_UNBOUNDED, fCellHeight) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiIconPickerGetCellSize(pPicker, &tMeasured.fX, &tMeasured.fY) == XUI_OK &&
		tMeasured.fX == fCellWidth && tMeasured.fY == fCellHeight,
		"oversized combined cell geometry is rejected transactionally");
	XUI_TEST_CHECK(xuiIconPickerGetSpacing(pPicker, &fGapX, &fGapY, &fIconPadding) == XUI_OK &&
		xuiIconPickerSetSpacing(pPicker, XUI_LAYOUT_UNBOUNDED, fGapY, fIconPadding) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiIconPickerGetSpacing(pPicker, &tMeasured.fX, &tMeasured.fY, &fBorderWidth) == XUI_OK &&
		tMeasured.fX == fGapX && tMeasured.fY == fGapY && fBorderWidth == fIconPadding,
		"oversized combined spacing is rejected transactionally");

	XUI_TEST_CHECK(xuiIconPickerSetSelectedId(pPicker, arrIds[4]) == XUI_OK, "set before remove");
	iRefSame = xuiIconCategoryGetRefCount(pCategory);
	XUI_TEST_CHECK(xuiIconPickerSetCategory(pPicker, pCategory) == XUI_OK &&
		xuiIconPickerGetSelectedId(pPicker) == arrIds[4] &&
		xuiIconCategoryGetRefCount(pCategory) == iRefSame,
		"setting same category preserves selection and reference count");
	XUI_TEST_CHECK(xuiIconRemoveById(pCategory, arrIds[4]) == XUI_OK, "remove selected icon");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK, "update category generation");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == XUI_ICON_ID_INVALID, "removed selection clears");
	XUI_TEST_CHECK(tChange.iCount == 2, "external removal does not notify during update");

	XUI_TEST_CHECK(xuiIconPickerClose(pPicker) == XUI_OK, "close");
	XUI_TEST_CHECK(xuiIconPickerSetGrid(pPicker, 3, 4) == XUI_OK, "set grid");
	XUI_TEST_CHECK(xuiIconPickerGetGrid(pPicker, &iPolicyY, &iPolicyX) == XUI_OK && iPolicyY == 3 && iPolicyX == 4, "get grid");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_NUMBER;
	tAccessibleValue.fNumber = NAN;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE,
		&tAccessibleValue) == XUI_ERROR_INVALID_ARGUMENT,
		"accessible numeric value rejects NaN");
	tAccessibleValue.fNumber = (double)arrIds[3];
	iChangeBefore = tChange.iCount;
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, 1, XUI_ACCESSIBLE_ACTION_SET_VALUE,
		&tAccessibleValue) == XUI_OK && xuiIconPickerGetSelectedId(pPicker) == arrIds[3] &&
		tChange.iCount == iChangeBefore + 1,
		"accessible set value selects icon and notifies");
	memset(sLongDisplay, 'a', 254u);
	memcpy(sLongDisplay + 254, "\xE4\xB8\xAD-tail", 9u);
	sLongDisplay[263] = '\0';
	memset(&tLongIconDesc, 0, sizeof(tLongIconDesc));
	tLongIconDesc.iSize = sizeof(tLongIconDesc);
	tLongIconDesc.sDisplayName = sLongDisplay;
	XUI_TEST_CHECK(xuiIconSetMetadata(xuiIconFindById(pCategory, arrIds[3]), &tLongIconDesc) == XUI_OK,
		"set long UTF-8 icon display name");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible) == XUI_OK &&
		strlen(tAccessible.sValue) == 254u && tAccessible.sValue[253] == 'a',
		"accessible display name truncates at UTF-8 boundary");

	memset(&tDestroyDesc, 0, sizeof(tDestroyDesc));
	tDestroyDesc = tDesc;
	tDestroyDesc.iSelectedId = arrIds[0];
	XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pDestroyPicker, &tDestroyDesc) == XUI_OK && pDestroyPicker != NULL,
		"destroy callback picker create");
	tDestroy.ppWidget = &pDestroyPicker;
	XUI_TEST_CHECK(xuiIconPickerSetChange(pDestroyPicker, __xuiIconPickerDestroyOnChange, &tDestroy) == XUI_OK &&
		xuiWidgetSetRect(pDestroyPicker, (xui_rect_t){280.0f, 24.0f, 210.0f, 34.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pDestroyPicker) == XUI_OK &&
		xuiIconPickerOpen(pDestroyPicker) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"destroy callback picker open");
	{
		xui_widget pDestroyViewport = xuiIconPickerGetViewportWidget(pDestroyPicker);
		xui_rect_t tDestroyItem = xuiIconPickerGetItemRect(pDestroyPicker, 1);
		xui_rect_t tDestroyWorld = xuiWidgetGetWorldRect(pDestroyViewport);
		XUI_TEST_CHECK(__xuiIconPickerDispatchClick(pContext,
			tDestroyWorld.fX + tDestroyItem.fX + tDestroyItem.fW * 0.5f,
			tDestroyWorld.fY + tDestroyItem.fY + tDestroyItem.fH * 0.5f) == XUI_OK &&
			tDestroy.iCount == 1 && pDestroyPicker == NULL,
			"selection callback may destroy icon picker safely");
	}
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK && __xuiIconPickerRender(pContext, pTarget) == XUI_OK,
		"context remains usable after callback destroys icon picker");

	XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pFocusDestroyPicker, &tDestroyDesc) == XUI_OK &&
		pFocusDestroyPicker != NULL, "focus destroy picker create");
	XUI_TEST_CHECK(xuiWidgetSetRect(pFocusDestroyPicker, (xui_rect_t){280.0f, 72.0f, 210.0f, 34.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pFocusDestroyPicker) == XUI_OK, "focus destroy picker add");
	tOpenDestroy.ppWidget = &pFocusDestroyPicker;
	tOpenDestroy.bOpen = 1;
	pPopup = xuiIconPickerGetPopupWidget(pFocusDestroyPicker);
	XUI_TEST_CHECK(pPopup != NULL && xuiPopupSetChange(pPopup,
		__xuiIconPickerDestroyVictimOnPopupChange, &tOpenDestroy) == XUI_OK,
		"install popup open destroy callback");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, NULL) == XUI_OK &&
		xuiIconPickerOpen(pFocusDestroyPicker) == XUI_OK,
		"open returns safely when popup callback destroys picker");
	XUI_TEST_CHECK(tOpenDestroy.iCount == 1 && pFocusDestroyPicker == NULL,
		"popup open callback destroys picker exactly once");

	XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pCloseDestroyPicker, &tDestroyDesc) == XUI_OK &&
		pCloseDestroyPicker != NULL, "close destroy picker create");
	XUI_TEST_CHECK(xuiWidgetSetRect(pCloseDestroyPicker, (xui_rect_t){280.0f, 120.0f, 210.0f, 34.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pCloseDestroyPicker) == XUI_OK &&
		xuiIconPickerSetChange(pCloseDestroyPicker, __xuiIconPickerChanged, &tChange) == XUI_OK,
		"close destroy picker add");
	tCloseDestroy.ppWidget = &pCloseDestroyPicker;
	tCloseDestroy.bOpen = 0;
	pPopup = xuiIconPickerGetPopupWidget(pCloseDestroyPicker);
	XUI_TEST_CHECK(pPopup != NULL && xuiPopupSetChange(pPopup,
		__xuiIconPickerDestroyVictimOnPopupChange, &tCloseDestroy) == XUI_OK,
		"install popup close destroy callback");
	iChangeBefore = tChange.iCount;
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, NULL) == XUI_OK &&
		xuiIconPickerOpen(pCloseDestroyPicker) == XUI_OK && xuiLayout(pContext) == XUI_OK,
		"close destroy picker open");
	XUI_TEST_CHECK(__xuiIconPickerDispatchKey(pContext, XUI_KEY_RIGHT) == XUI_OK &&
		__xuiIconPickerDispatchKey(pContext, XUI_KEY_ENTER) == XUI_OK,
		"commit returns safely when popup close destroys picker");
	XUI_TEST_CHECK(tCloseDestroy.iCount == 1 && pCloseDestroyPicker == NULL &&
		tChange.iCount == iChangeBefore,
		"popup close destruction suppresses stale change callback");
	XUI_TEST_CHECK(xuiUpdate(pContext, 0.016f) == XUI_OK && __xuiIconPickerRender(pContext, pTarget) == XUI_OK,
		"context remains usable after focus callbacks destroy icon pickers");

	XUI_TEST_CHECK(xuiCreate(&pOtherContext) == XUI_OK && pOtherContext != NULL &&
		xuiIconCategoryCreate(pOtherContext, "foreign_picker_test", NULL, &pOtherCategory) == XUI_OK,
		"foreign category create");
	{
		xui_icon_picker_desc_t tForeignDesc = tDesc;
		xui_widget pForeignPicker = (xui_widget)(uintptr_t)1u;
		tForeignDesc.pCategory = pOtherCategory;
		XUI_TEST_CHECK(xuiIconPickerCreate(pContext, &pForeignPicker, &tForeignDesc) == XUI_ERROR_INVALID_ARGUMENT &&
			pForeignPicker == NULL, "create rejects category from another context");
	}
	XUI_TEST_CHECK(xuiIconPickerSetCategory(pPicker, pOtherCategory) == XUI_ERROR_INVALID_ARGUMENT &&
		xuiIconPickerGetCategory(pPicker) == pCategory,
		"set category rejects another context without changing current category");
	xuiDestroy(pOtherContext);
	pOtherContext = NULL;
	pOtherCategory = NULL;

	XUI_TEST_CHECK(xuiIconPickerSetCategory(pPicker, NULL) == XUI_OK, "clear category");
	XUI_TEST_CHECK(xuiIconCategoryGetRefCount(pCategory) == iRefBefore, "clear category releases ref");
	XUI_TEST_CHECK(xuiIconPickerGetSelectedId(pPicker) == XUI_ICON_ID_INVALID, "category clear selection");

cleanup:
	if ( pTarget != NULL ) tState.tProxy.surfaceDestroy(&tState.tProxy, pTarget);
	if ( pOtherContext != NULL ) xuiDestroy(pOtherContext);
	if ( pContext != NULL ) xuiDestroy(pContext);
	if ( iFailed ) return 1;
	printf("xui_icon_picker_test passed\n");
	return 0;
}
