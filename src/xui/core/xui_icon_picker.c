#include "xui_config.h"
#if XUI_ENABLE_ICON_PICKER
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define XUI_ICON_PICKER_DEFAULT_ROWS		4
#define XUI_ICON_PICKER_DEFAULT_COLUMNS		6
#define XUI_ICON_PICKER_DEFAULT_CELL_W		48.0f
#define XUI_ICON_PICKER_DEFAULT_CELL_H		48.0f
#define XUI_ICON_PICKER_DEFAULT_GAP		4.0f
#define XUI_ICON_PICKER_DEFAULT_ICON_PAD		7.0f
#define XUI_ICON_PICKER_POPUP_PADDING		4.0f

typedef struct xui_icon_picker_data_t {
	xui_widget pPopup;
	xui_widget pPanel;
	xui_widget pFrame;
	xui_widget pViewport;
	xui_icon_category pCategory;
	xui_icon_id* arrIds;
	int iIconCount;
	int iIconCapacity;
	uint32_t iCategoryGeneration;
	xui_icon_id iSelectedId;
	int iSelectedIndex;
	int iHoverIndex;
	int iFocusIndex;
	int iActiveIndex;
	xui_pointer_owner_t tPointerOwner;
	int iTextMode;
	int iVisibleRows;
	int iVisibleColumns;
	int iPopupPlacement;
	char* sPlaceholder;
	xui_font pFont;
	xui_icon_picker_change_proc onChange;
	void* pChangeUser;
	xui_icon_picker_format_proc onFormat;
	void* pFormatUser;
	float fCellWidth;
	float fCellHeight;
	float fIconPadding;
	float fGapX;
	float fGapY;
	xui_thickness_t tValuePadding;
	float fBorderWidth;
	float fScrollbarSize;
	xui_rect_t tButtonRect;
	xui_rect_t tValueRect;
	uint32_t iTextColor;
	uint32_t iPlaceholderColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iOpenBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iArrowColor;
	uint32_t iDisabledArrowColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonOpenColor;
	uint32_t iPopupPanelColor;
	uint32_t iPopupBorderColor;
	uint32_t iPopupShadowColor;
	uint32_t iItemHoverColor;
	uint32_t iItemSelectedColor;
	uint32_t iItemFocusColor;
	uint32_t iChildStyleHash;
	int bChildStyleSynced;
	int iChangeCount;
	char sTooltip[256];
	char sAccessibleValue[256];
} xui_icon_picker_data_t;

static xui_icon_picker_data_t* __xuiIconPickerGetData(xui_widget pWidget);
static int __xuiIconPickerApplyPopup(xui_widget pWidget, xui_icon_picker_data_t* pData);
static int __xuiIconPickerSyncCategory(xui_widget pWidget, xui_icon_picker_data_t* pData);
static int __xuiIconPickerInvalidate(xui_widget pWidget, xui_icon_picker_data_t* pData);
static void __xuiIconPickerDestroy(xui_widget pWidget, void* pTypeData, void* pUser);

static int __xuiIconPickerAlive(xui_context pContext, xui_widget pWidget,
	const xui_icon_picker_data_t* pExpected)
{
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		xuiInternalWidgetIsValid(pWidget) && __xuiIconPickerGetData(pWidget) == pExpected;
}

static float __xuiIconPickerMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static float __xuiIconPickerMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static uint32_t __xuiIconPickerAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static int __xuiIconPickerFloatValid(float fValue)
{
	return (fValue == fValue) && (fValue >= 0.0f) && (fValue <= XUI_LAYOUT_UNBOUNDED);
}

static int __xuiIconPickerCoordinateValid(float fValue)
{
	return (fValue == fValue) && (fValue >= -FLT_MAX) && (fValue <= FLT_MAX);
}

static int __xuiIconPickerTextModeValid(int iMode)
{
	return (iMode >= XUI_ICON_PICKER_TEXT_NONE) && (iMode <= XUI_ICON_PICKER_TEXT_CUSTOM);
}

static int __xuiIconPickerPlacementValid(int iPlacement)
{
	return (iPlacement == XUI_ICON_PICKER_POPUP_AUTO) ||
	       (iPlacement == XUI_ICON_PICKER_POPUP_BOTTOM) ||
	       (iPlacement == XUI_ICON_PICKER_POPUP_TOP);
}

static int __xuiIconPickerThicknessValid(xui_thickness_t tValue)
{
	return __xuiIconPickerFloatValid(tValue.fLeft) &&
	       __xuiIconPickerFloatValid(tValue.fTop) &&
	       __xuiIconPickerFloatValid(tValue.fRight) &&
	       __xuiIconPickerFloatValid(tValue.fBottom);
}

static int __xuiIconPickerGeometryValid(int iRows, int iColumns, int iIconCount,
	float fCellWidth, float fCellHeight, float fGapX, float fGapY, float fScrollbarSize)
{
	double fGridWidth;
	double fVisibleHeight;
	double fContentHeight;
	double fStrideX;
	double fStrideY;
	int iTotalRows;
	int bNeedScrollbar;

	if ( iRows <= 0 || iColumns <= 0 || iIconCount < 0 ||
	     !__xuiIconPickerFloatValid(fCellWidth) || fCellWidth <= 0.0f ||
	     !__xuiIconPickerFloatValid(fCellHeight) || fCellHeight <= 0.0f ||
	     !__xuiIconPickerFloatValid(fGapX) || !__xuiIconPickerFloatValid(fGapY) ||
	     !__xuiIconPickerFloatValid(fScrollbarSize) ) return 0;
	fStrideX = (double)fCellWidth + (double)fGapX;
	fStrideY = (double)fCellHeight + (double)fGapY;
	fGridWidth = (double)fCellWidth * (double)iColumns +
		(double)fGapX * (double)(iColumns - 1);
	fVisibleHeight = (double)fCellHeight * (double)iRows +
		(double)fGapY * (double)(iRows - 1);
	iTotalRows = (iIconCount > 0) ? ((iIconCount - 1) / iColumns) + 1 : iRows;
	fContentHeight = (double)fCellHeight * (double)iTotalRows +
		(double)fGapY * (double)(iTotalRows - 1);
	bNeedScrollbar = iTotalRows > iRows;
	return fStrideX <= (double)XUI_LAYOUT_UNBOUNDED &&
	       fStrideY <= (double)XUI_LAYOUT_UNBOUNDED &&
	       fGridWidth + (bNeedScrollbar ? (double)fScrollbarSize : 0.0) <=
		(double)XUI_LAYOUT_UNBOUNDED &&
	       fVisibleHeight <= (double)XUI_LAYOUT_UNBOUNDED &&
	       fContentHeight <= (double)XUI_LAYOUT_UNBOUNDED;
}

static int __xuiIconPickerDescValid(const xui_icon_picker_desc_t* pDesc)
{
	float fCellWidth;
	float fCellHeight;
	float fIconPadding;
	float fGapX;
	float fGapY;
	float fScrollbarSize;
	int iRows;
	int iColumns;

	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( (pDesc->iVisibleRows < 0) || (pDesc->iVisibleRows > 1024) ||
	     (pDesc->iVisibleColumns < 0) || (pDesc->iVisibleColumns > 1024) ) return 0;
	if ( !__xuiIconPickerTextModeValid(pDesc->iTextMode) && (pDesc->iTextMode != 0) ) return 0;
	if ( !__xuiIconPickerPlacementValid(pDesc->iPopupPlacement) ) return 0;
	if ( !__xuiIconPickerFloatValid(pDesc->fCellWidth) ||
	     !__xuiIconPickerFloatValid(pDesc->fCellHeight) ||
	     !__xuiIconPickerFloatValid(pDesc->fIconPadding) ||
	     !__xuiIconPickerFloatValid(pDesc->fGapX) ||
	     !__xuiIconPickerFloatValid(pDesc->fGapY) ||
	     !__xuiIconPickerFloatValid(pDesc->fBorderWidth) ||
	     !__xuiIconPickerFloatValid(pDesc->fScrollbarSize) ) return 0;
	if ( pDesc->fScrollbarSize > 8.0f ) return 0;
	if ( !__xuiIconPickerThicknessValid(pDesc->tValuePadding) ) return 0;
	fCellWidth = (pDesc->fCellWidth > 0.0f) ? pDesc->fCellWidth : XUI_ICON_PICKER_DEFAULT_CELL_W;
	fCellHeight = (pDesc->fCellHeight > 0.0f) ? pDesc->fCellHeight : XUI_ICON_PICKER_DEFAULT_CELL_H;
	fIconPadding = (pDesc->fIconPadding > 0.0f) ? pDesc->fIconPadding : XUI_ICON_PICKER_DEFAULT_ICON_PAD;
	if ( fIconPadding * 2.0f >= fCellWidth || fIconPadding * 2.0f >= fCellHeight ) return 0;
	iRows = (pDesc->iVisibleRows > 0) ? pDesc->iVisibleRows : XUI_ICON_PICKER_DEFAULT_ROWS;
	iColumns = (pDesc->iVisibleColumns > 0) ? pDesc->iVisibleColumns : XUI_ICON_PICKER_DEFAULT_COLUMNS;
	fGapX = (pDesc->fGapX > 0.0f) ? pDesc->fGapX : XUI_ICON_PICKER_DEFAULT_GAP;
	fGapY = (pDesc->fGapY > 0.0f) ? pDesc->fGapY : XUI_ICON_PICKER_DEFAULT_GAP;
	fScrollbarSize = (pDesc->fScrollbarSize > 0.0f) ? pDesc->fScrollbarSize : 8.0f;
	if ( !__xuiIconPickerGeometryValid(iRows, iColumns, 0, fCellWidth, fCellHeight,
		fGapX, fGapY, fScrollbarSize) ) return 0;
	return 1;
}

static char* __xuiIconPickerDup(const char* sText)
{
	char* sCopy;
	size_t iLen;

	if ( sText == NULL ) sText = "";
	iLen = strlen(sText);
	sCopy = (char*)xrtMalloc(iLen + 1u);
	if ( sCopy == NULL ) return NULL;
	memcpy(sCopy, sText, iLen + 1u);
	return sCopy;
}

static void __xuiIconPickerDefaults(xui_icon_picker_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->iSelectedId = XUI_ICON_ID_INVALID;
	pData->iSelectedIndex = -1;
	pData->iHoverIndex = -1;
	pData->iFocusIndex = -1;
	pData->iActiveIndex = -1;
	pData->iTextMode = XUI_ICON_PICKER_TEXT_ID;
	pData->iVisibleRows = XUI_ICON_PICKER_DEFAULT_ROWS;
	pData->iVisibleColumns = XUI_ICON_PICKER_DEFAULT_COLUMNS;
	pData->iPopupPlacement = XUI_ICON_PICKER_POPUP_AUTO;
	pData->fCellWidth = XUI_ICON_PICKER_DEFAULT_CELL_W;
	pData->fCellHeight = XUI_ICON_PICKER_DEFAULT_CELL_H;
	pData->fIconPadding = XUI_ICON_PICKER_DEFAULT_ICON_PAD;
	pData->fGapX = XUI_ICON_PICKER_DEFAULT_GAP;
	pData->fGapY = XUI_ICON_PICKER_DEFAULT_GAP;
	pData->tValuePadding = (xui_thickness_t){7.0f, 4.0f, 7.0f, 4.0f};
	pData->fBorderWidth = 1.0f;
	pData->fScrollbarSize = 8.0f;
	pData->iTextColor = XUI_COLOR_RGBA(33, 55, 79, 255);
	pData->iPlaceholderColor = XUI_COLOR_RGBA(116, 132, 150, 220);
	pData->iDisabledTextColor = XUI_COLOR_RGBA(132, 146, 162, 210);
	pData->iBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iHoverBackgroundColor = XUI_COLOR_RGBA(232, 244, 252, 255);
	pData->iOpenBackgroundColor = XUI_COLOR_RGBA(222, 239, 252, 255);
	pData->iDisabledBackgroundColor = XUI_COLOR_RGBA(235, 240, 245, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(132, 174, 214, 255);
	pData->iHoverBorderColor = XUI_COLOR_RGBA(78, 148, 208, 255);
	pData->iFocusBorderColor = XUI_COLOR_RGBA(42, 126, 205, 255);
	pData->iArrowColor = XUI_COLOR_RGBA(42, 92, 136, 255);
	pData->iDisabledArrowColor = XUI_COLOR_RGBA(134, 148, 164, 180);
	pData->iButtonColor = XUI_COLOR_RGBA(236, 246, 253, 255);
	pData->iButtonHoverColor = XUI_COLOR_RGBA(220, 238, 251, 255);
	pData->iButtonOpenColor = XUI_COLOR_RGBA(207, 229, 247, 255);
	pData->iPopupPanelColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	pData->iPopupBorderColor = XUI_COLOR_RGBA(122, 164, 202, 255);
	pData->iPopupShadowColor = XUI_COLOR_RGBA(44, 70, 96, 46);
	pData->iItemHoverColor = XUI_COLOR_RGBA(220, 238, 251, 255);
	pData->iItemSelectedColor = XUI_COLOR_RGBA(190, 220, 246, 255);
	pData->iItemFocusColor = XUI_COLOR_RGBA(42, 126, 205, 255);
}

static void __xuiIconPickerStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty) == XUI_OK &&
	     tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR )
		*pColor = tProperty.tValue.iColor;
}

static void __xuiIconPickerResolve(xui_widget pWidget, const xui_icon_picker_data_t* pData, xui_icon_picker_data_t* pResolved)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	*pResolved = *pData;
	pResolved->pFont = pData->pFont != NULL ? pData->pFont : xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
	__xuiIconPickerStyleColor(pWidget, "iconpicker.text.color", &pResolved->iTextColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.placeholder.color", &pResolved->iPlaceholderColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.text.disabled_color", &pResolved->iDisabledTextColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.background.color", &pResolved->iBackgroundColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.background.hover_color", &pResolved->iHoverBackgroundColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.background.open_color", &pResolved->iOpenBackgroundColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.border.color", &pResolved->iBorderColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.border.hover_color", &pResolved->iHoverBorderColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.border.focus_color", &pResolved->iFocusBorderColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.arrow.color", &pResolved->iArrowColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.arrow.disabled_color", &pResolved->iDisabledArrowColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.button.color", &pResolved->iButtonColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.button.hover_color", &pResolved->iButtonHoverColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.button.open_color", &pResolved->iButtonOpenColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.popup.panel_color", &pResolved->iPopupPanelColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.popup.border_color", &pResolved->iPopupBorderColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.popup.shadow_color", &pResolved->iPopupShadowColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.item.hover_color", &pResolved->iItemHoverColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.item.selected_color", &pResolved->iItemSelectedColor);
	__xuiIconPickerStyleColor(pWidget, "iconpicker.item.focus_color", &pResolved->iItemFocusColor);
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	if ( xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty) == XUI_OK &&
	     tProperty.tValue.iType == XUI_STYLE_VALUE_STRING && tProperty.tValue.sText != NULL ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) pResolved->pFont = pFont;
	}
	if ( xuiWidgetGetResolvedStyleProperty(pWidget, "iconpicker.border.width", &tProperty) == XUI_OK &&
	     tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT && __xuiIconPickerFloatValid(tProperty.tValue.fFloat) )
		pResolved->fBorderWidth = tProperty.tValue.fFloat;
}

static int __xuiIconPickerApplyPopupColors(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_icon_picker_data_t tResolved;
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pFrame;
	xui_widget pViewport;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pFrame = pData->pFrame;
	pViewport = pData->pViewport;
	__xuiIconPickerResolve(pWidget, pData, &tResolved);
	if ( pPopup != NULL ) {
		iRet = xuiPopupSetColors(pPopup, tResolved.iPopupPanelColor,
			tResolved.iPopupBorderColor, tResolved.iPopupShadowColor, 0u);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( pFrame != NULL &&
	     xuiScrollFrameGetBackgroundColor(pFrame) != tResolved.iPopupPanelColor ) {
		iRet = xuiScrollFrameSetBackgroundColor(pFrame, tResolved.iPopupPanelColor);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( pViewport != NULL ) {
		iRet = xuiWidgetInvalidate(pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiIconPickerSyncStyle(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	uint32_t iHash = xuiWidgetGetStyleHash(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bChildStyleSynced && pData->iChildStyleHash == iHash ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiIconPickerApplyPopupColors(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	pData->iChildStyleHash = iHash;
	pData->bChildStyleSynced = 1;
	return XUI_OK;
}

static int __xuiIconPickerSetPlaceholderData(xui_icon_picker_data_t* pData, const char* sText)
{
	char* sNew;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sNew = __xuiIconPickerDup(sText);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	if ( pData->sPlaceholder != NULL ) xrtFree(pData->sPlaceholder);
	pData->sPlaceholder = sNew;
	return XUI_OK;
}

static int __xuiIconPickerApplyCategoryData(xui_icon_picker_data_t* pData, xui_icon_category pCategory)
{
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pCategory == pData->pCategory ) return XUI_OK;
	if ( pCategory != NULL ) {
		iRet = xuiIconCategoryAddRef(pCategory);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pData->pCategory != NULL ) {
		(void)xuiIconCategoryRelease(pData->pCategory);
	}
	pData->pCategory = pCategory;
	pData->iCategoryGeneration = 0u;
	return XUI_OK;
}

static void __xuiIconPickerApplyDesc(xui_icon_picker_data_t* pData, const xui_icon_picker_desc_t* pDesc)
{
	if ( (pData == NULL) || (pDesc == NULL) ) return;
	pData->pFont = pDesc->pFont;
	pData->iSelectedId = pDesc->iSelectedId;
	if ( __xuiIconPickerTextModeValid(pDesc->iTextMode) ) pData->iTextMode = pDesc->iTextMode;
	if ( pDesc->iVisibleRows > 0 ) pData->iVisibleRows = pDesc->iVisibleRows;
	if ( pDesc->iVisibleColumns > 0 ) pData->iVisibleColumns = pDesc->iVisibleColumns;
	if ( __xuiIconPickerPlacementValid(pDesc->iPopupPlacement) ) pData->iPopupPlacement = pDesc->iPopupPlacement;
	if ( pDesc->fCellWidth > 0.0f ) pData->fCellWidth = pDesc->fCellWidth;
	if ( pDesc->fCellHeight > 0.0f ) pData->fCellHeight = pDesc->fCellHeight;
	if ( pDesc->fIconPadding > 0.0f ) pData->fIconPadding = pDesc->fIconPadding;
	if ( pDesc->fGapX > 0.0f ) pData->fGapX = pDesc->fGapX;
	if ( pDesc->fGapY > 0.0f ) pData->fGapY = pDesc->fGapY;
	if ( pDesc->tValuePadding.fLeft > 0.0f || pDesc->tValuePadding.fTop > 0.0f ||
	     pDesc->tValuePadding.fRight > 0.0f || pDesc->tValuePadding.fBottom > 0.0f ) {
		pData->tValuePadding = pDesc->tValuePadding;
	}
	if ( pDesc->fBorderWidth > 0.0f ) pData->fBorderWidth = pDesc->fBorderWidth;
	if ( pDesc->fScrollbarSize > 0.0f ) pData->fScrollbarSize = pDesc->fScrollbarSize;
	if ( __xuiIconPickerAlpha(pDesc->iTextColor) ) pData->iTextColor = pDesc->iTextColor;
	if ( __xuiIconPickerAlpha(pDesc->iPlaceholderColor) ) pData->iPlaceholderColor = pDesc->iPlaceholderColor;
	if ( __xuiIconPickerAlpha(pDesc->iDisabledTextColor) ) pData->iDisabledTextColor = pDesc->iDisabledTextColor;
	if ( __xuiIconPickerAlpha(pDesc->iBackgroundColor) ) pData->iBackgroundColor = pDesc->iBackgroundColor;
	if ( __xuiIconPickerAlpha(pDesc->iHoverBackgroundColor) ) pData->iHoverBackgroundColor = pDesc->iHoverBackgroundColor;
	if ( __xuiIconPickerAlpha(pDesc->iOpenBackgroundColor) ) pData->iOpenBackgroundColor = pDesc->iOpenBackgroundColor;
	if ( __xuiIconPickerAlpha(pDesc->iDisabledBackgroundColor) ) pData->iDisabledBackgroundColor = pDesc->iDisabledBackgroundColor;
	if ( __xuiIconPickerAlpha(pDesc->iBorderColor) ) pData->iBorderColor = pDesc->iBorderColor;
	if ( __xuiIconPickerAlpha(pDesc->iHoverBorderColor) ) pData->iHoverBorderColor = pDesc->iHoverBorderColor;
	if ( __xuiIconPickerAlpha(pDesc->iFocusBorderColor) ) pData->iFocusBorderColor = pDesc->iFocusBorderColor;
	if ( __xuiIconPickerAlpha(pDesc->iArrowColor) ) pData->iArrowColor = pDesc->iArrowColor;
	if ( __xuiIconPickerAlpha(pDesc->iDisabledArrowColor) ) pData->iDisabledArrowColor = pDesc->iDisabledArrowColor;
	if ( __xuiIconPickerAlpha(pDesc->iButtonColor) ) pData->iButtonColor = pDesc->iButtonColor;
	if ( __xuiIconPickerAlpha(pDesc->iButtonHoverColor) ) pData->iButtonHoverColor = pDesc->iButtonHoverColor;
	if ( __xuiIconPickerAlpha(pDesc->iButtonOpenColor) ) pData->iButtonOpenColor = pDesc->iButtonOpenColor;
	if ( __xuiIconPickerAlpha(pDesc->iPopupPanelColor) ) pData->iPopupPanelColor = pDesc->iPopupPanelColor;
	if ( __xuiIconPickerAlpha(pDesc->iPopupBorderColor) ) pData->iPopupBorderColor = pDesc->iPopupBorderColor;
	if ( pDesc->iPopupShadowColor != 0u ) pData->iPopupShadowColor = pDesc->iPopupShadowColor;
	if ( __xuiIconPickerAlpha(pDesc->iItemHoverColor) ) pData->iItemHoverColor = pDesc->iItemHoverColor;
	if ( __xuiIconPickerAlpha(pDesc->iItemSelectedColor) ) pData->iItemSelectedColor = pDesc->iItemSelectedColor;
	if ( __xuiIconPickerAlpha(pDesc->iItemFocusColor) ) pData->iItemFocusColor = pDesc->iItemFocusColor;
}

static int __xuiIconPickerFindIndex(const xui_icon_picker_data_t* pData, xui_icon_id iId)
{
	int i;

	if ( (pData == NULL) || (iId == XUI_ICON_ID_INVALID) ) return -1;
	for ( i = 0; i < pData->iIconCount; i++ ) {
		if ( pData->arrIds[i] == iId ) return i;
	}
	return -1;
}

static int __xuiIconPickerReserveIds(xui_icon_picker_data_t* pData, int iNeeded)
{
	xui_icon_id* arrNew;
	int iCapacity;

	if ( pData == NULL || iNeeded < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iNeeded <= pData->iIconCapacity ) return XUI_OK;
	iCapacity = (pData->iIconCapacity > 0) ? pData->iIconCapacity : 16;
	while ( iCapacity < iNeeded ) {
		if ( iCapacity > 0x3fffffff ) return XUI_ERROR_OUT_OF_MEMORY;
		iCapacity *= 2;
	}
	arrNew = (xui_icon_id*)xrtRealloc(pData->arrIds, (size_t)iCapacity * sizeof(*arrNew));
	if ( arrNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	pData->arrIds = arrNew;
	pData->iIconCapacity = iCapacity;
	return XUI_OK;
}

static int __xuiIconPickerCancelPress(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_context pContext;

	if ( pData == NULL ) return XUI_OK;
	pContext = (pWidget != NULL) ? xuiWidgetGetContext(pWidget) : NULL;
	pData->iActiveIndex = -1;
	return xuiInternalPointerOwnerCancel(pContext, pData->pViewport, &pData->tPointerOwner);
}

static int __xuiIconPickerSyncCategory(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_icon pIcon;
	xui_icon_id iOldFocusId;
	xui_icon_id iOldSelectedId;
	uint32_t iGeneration;
	uint32_t iSlotCount;
	uint32_t iSlot;
	int iCount;
	int iRet;
	int iInvalidateRet;
	int bHadHover;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iGeneration = (pData->pCategory != NULL) ? xuiIconCategoryGetGeneration(pData->pCategory) : 0u;
	iCount = (pData->pCategory != NULL) ? xuiIconCategoryGetIconCount(pData->pCategory) : 0;
	if ( iGeneration == pData->iCategoryGeneration && iCount == pData->iIconCount ) return XUI_OK;
	iOldFocusId = (pData->iFocusIndex >= 0 && pData->iFocusIndex < pData->iIconCount) ?
		pData->arrIds[pData->iFocusIndex] : XUI_ICON_ID_INVALID;
	bHadHover = pData->iHoverIndex >= 0;
	iOldSelectedId = pData->iSelectedId;
	if ( pData->tPointerOwner.bActive || pData->iActiveIndex >= 0 ) {
		iRet = __xuiIconPickerCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		iCount, pData->fCellWidth, pData->fCellHeight, pData->fGapX, pData->fGapY,
		pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiIconPickerReserveIds(pData, iCount);
	if ( iRet != XUI_OK ) return iRet;
	pData->iIconCount = 0;
	iSlotCount = (pData->pCategory != NULL) ? xuiIconCategoryGetIconSlotCount(pData->pCategory) : 0u;
	for ( iSlot = 0u; iSlot < iSlotCount; ++iSlot ) {
		pIcon = xuiIconFindById(pData->pCategory, (xui_icon_id)(iSlot + 1u));
		if ( pIcon != NULL ) {
			pData->arrIds[pData->iIconCount++] = xuiIconGetId(pIcon);
		}
	}
	pData->iCategoryGeneration = iGeneration;
	pData->iSelectedIndex = __xuiIconPickerFindIndex(pData, pData->iSelectedId);
	if ( pData->iSelectedIndex < 0 ) pData->iSelectedId = XUI_ICON_ID_INVALID;
	pData->iFocusIndex = __xuiIconPickerFindIndex(pData, iOldFocusId);
	if ( pData->iFocusIndex < 0 ) {
		pData->iFocusIndex = (pData->iSelectedIndex >= 0) ? pData->iSelectedIndex :
			((pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) && pData->iIconCount > 0) ? 0 : -1);
	}
	pData->iHoverIndex = -1;
	if ( bHadHover ) xuiInternalTooltipCancel(xuiWidgetGetContext(pWidget));
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( pData->pViewport != NULL ) {
		iInvalidateRet = xuiWidgetInvalidate(pData->pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet == XUI_OK ) iRet = iInvalidateRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	if ( iOldSelectedId != pData->iSelectedId ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	return iRet;
}

static float __xuiIconPickerGridWidth(const xui_icon_picker_data_t* pData)
{
	return pData->fCellWidth * (float)pData->iVisibleColumns +
	       pData->fGapX * (float)(pData->iVisibleColumns - 1);
}

static int __xuiIconPickerTotalRows(const xui_icon_picker_data_t* pData)
{
	if ( (pData == NULL) || (pData->iVisibleColumns <= 0) || (pData->iIconCount <= 0) ) return 0;
	return ((pData->iIconCount - 1) / pData->iVisibleColumns) + 1;
}

static float __xuiIconPickerVisibleHeight(const xui_icon_picker_data_t* pData)
{
	return pData->fCellHeight * (float)pData->iVisibleRows +
	       pData->fGapY * (float)(pData->iVisibleRows - 1);
}

static float __xuiIconPickerContentHeight(const xui_icon_picker_data_t* pData)
{
	int iRows;

	iRows = __xuiIconPickerTotalRows(pData);
	if ( iRows <= 0 ) return __xuiIconPickerVisibleHeight(pData);
	return pData->fCellHeight * (float)iRows + pData->fGapY * (float)(iRows - 1);
}

static int __xuiIconPickerNeedScrollbar(const xui_icon_picker_data_t* pData)
{
	return __xuiIconPickerTotalRows(pData) > pData->iVisibleRows;
}

static xui_rect_t __xuiIconPickerItemContentRect(const xui_icon_picker_data_t* pData, int iIndex)
{
	int iColumn;
	int iRow;

	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= pData->iIconCount) ) {
		return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	}
	iColumn = iIndex % pData->iVisibleColumns;
	iRow = iIndex / pData->iVisibleColumns;
	return (xui_rect_t){
		(float)iColumn * (pData->fCellWidth + pData->fGapX),
		(float)iRow * (pData->fCellHeight + pData->fGapY),
		pData->fCellWidth,
		pData->fCellHeight
	};
}

static int __xuiIconPickerUpdateRects(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_rect_t tRect;
	float fButtonWidth;
	float fLeft;
	float fTop;
	float fRight;
	float fBottom;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		pData->tButtonRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		pData->tValueRect = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		return XUI_OK;
	}
	fButtonWidth = __xuiIconPickerMin(36.0f, __xuiIconPickerMax(24.0f, tRect.fH));
	fButtonWidth = __xuiIconPickerMin(fButtonWidth, tRect.fW);
	pData->tButtonRect = xuiInternalSnapRect((xui_rect_t){tRect.fW - fButtonWidth, 0.0f, fButtonWidth, tRect.fH});
	pData->tValueRect = (xui_rect_t){0.0f, 0.0f, __xuiIconPickerMax(0.0f, tRect.fW - fButtonWidth), tRect.fH};
	fLeft = __xuiIconPickerMin((float)pData->tValuePadding.fLeft, (float)pData->tValueRect.fW);
	fTop = __xuiIconPickerMin((float)pData->tValuePadding.fTop, (float)pData->tValueRect.fH);
	fRight = __xuiIconPickerMin((float)pData->tValuePadding.fRight,
		(float)pData->tValueRect.fW - fLeft);
	fBottom = __xuiIconPickerMin((float)pData->tValuePadding.fBottom,
		(float)pData->tValueRect.fH - fTop);
	pData->tValueRect.fX += (int)fLeft;
	pData->tValueRect.fY += (int)fTop;
	pData->tValueRect.fW -= (int)(fLeft + fRight);
	pData->tValueRect.fH -= (int)(fTop + fBottom);
	pData->tValueRect = xuiInternalSnapRect(pData->tValueRect);
	return XUI_OK;
}

static uint32_t __xuiIconPickerState(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	uint32_t iState;

	iState = xuiWidgetGetInputState(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) iState |= XUI_WIDGET_STATE_DISABLED;
	if ( xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ||
	     xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pData->pViewport ) {
		iState |= XUI_WIDGET_STATE_FOCUS;
	}
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) {
		iState |= XUI_ICON_PICKER_STATE_OPEN;
	}
	return iState;
}

static int __xuiIconPickerSyncState(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_context pContext;
	uint32_t iState;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iState = __xuiIconPickerState(pWidget, pData);
	if ( xuiWidgetGetStateId(pWidget) == iState ) return XUI_OK;
	iRet = xuiWidgetSetStateId(pWidget, iState);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return __xuiIconPickerInvalidate(pWidget, pData);
}

static int __xuiIconPickerFormatValue(xui_widget pWidget, xui_icon_picker_data_t* pData, xui_icon pIcon, char* sBuffer, int iCapacity)
{
	const char* sText;
	int iRet;

	if ( (sBuffer == NULL) || (iCapacity <= 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	sBuffer[0] = '\0';
	if ( pIcon == NULL || pData == NULL || pData->iTextMode == XUI_ICON_PICKER_TEXT_NONE ) return XUI_OK;
	switch ( pData->iTextMode ) {
	case XUI_ICON_PICKER_TEXT_ID:
		snprintf(sBuffer, (size_t)iCapacity, "%u", (unsigned)xuiIconGetId(pIcon));
		break;
	case XUI_ICON_PICKER_TEXT_NAME:
		sText = xuiIconGetName(pIcon);
		snprintf(sBuffer, (size_t)iCapacity, "%s", (sText != NULL) ? sText : "");
		break;
	case XUI_ICON_PICKER_TEXT_DISPLAY_NAME:
		sText = xuiIconGetDisplayName(pIcon);
		if ( sText == NULL || sText[0] == '\0' ) sText = xuiIconGetName(pIcon);
		snprintf(sBuffer, (size_t)iCapacity, "%s", (sText != NULL) ? sText : "");
		break;
	case XUI_ICON_PICKER_TEXT_CUSTOM:
		if ( pData->onFormat != NULL ) {
			iRet = pData->onFormat(pWidget, pIcon, sBuffer, iCapacity, pData->pFormatUser);
			if ( iRet != XUI_OK ) {
				sBuffer[0] = '\0';
				return iRet;
			}
			sBuffer[iCapacity - 1] = '\0';
		}
		break;
	default:
		break;
	}
	xuiInternalUtf8TrimIncomplete(sBuffer);
	return XUI_OK;
}

static int __xuiIconPickerDrawIcon(xui_widget pWidget, xui_draw_context pDraw, xui_icon pIcon, xui_rect_t tRect, int bEnabled)
{
	xui_icon_draw_desc_t tDesc;
	xui_painter_t tPainter;

	if ( pIcon == NULL || tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	memset(&tPainter, 0, sizeof(tPainter));
	tPainter.iMagic = XUI_PAINTER_MAGIC;
	tPainter.pContext = xuiWidgetGetContext(pWidget);
	tPainter.pDraw = pDraw;
	xuiIconDrawDescDefault(&tDesc);
	tDesc.iFlags |= XUI_ICON_DRAW_IGNORE_CATEGORY_SIZE;
	tDesc.iFitMode = XUI_IMAGE_CONTAIN;
	tDesc.iColor = bEnabled ? XUI_COLOR_WHITE : XUI_COLOR_RGBA(255, 255, 255, 128);
	return xuiIconDraw(&tPainter, pIcon, xuiInternalSnapRect(tRect), &tDesc);
}

static int __xuiIconPickerDrawChevron(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int bOpen, uint32_t iColor)
{
	float fCX;
	float fCY;
	float fStep;
	int iRet;

	if ( pProxy->drawLine == NULL || __xuiIconPickerAlpha(iColor) == 0u ||
	     tRect.fW < 6.0f || tRect.fH < 6.0f ) return XUI_OK;
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	fStep = __xuiIconPickerMin(4.0f, __xuiIconPickerMin(tRect.fW, tRect.fH) * 0.18f);
	if ( bOpen ) {
		iRet = pProxy->drawLine(pProxy, pDraw, fCX - fStep, fCY + 1.5f, fCX, fCY - 1.5f, 1.0f, iColor);
		if ( iRet != XUI_OK ) return iRet;
		return pProxy->drawLine(pProxy, pDraw, fCX, fCY - 1.5f, fCX + fStep, fCY + 1.5f, 1.0f, iColor);
	}
	iRet = pProxy->drawLine(pProxy, pDraw, fCX - fStep, fCY - 1.5f, fCX, fCY + 1.5f, 1.0f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawLine(pProxy, pDraw, fCX, fCY + 1.5f, fCX + fStep, fCY - 1.5f, 1.0f, iColor);
}

static int __xuiIconPickerCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_icon_picker_data_t* pData;
	xui_icon_picker_data_t tResolved;
	xui_context pContext;
	xui_proxy pProxy;
	xui_icon pIcon;
	xui_font pFont;
	xui_rect_t tRect;
	xui_rect_t tIcon;
	xui_rect_t tText;
	uint32_t iBackground;
	uint32_t iButton;
	uint32_t iBorder;
	uint32_t iTextColor;
	uint32_t iArrow;
	float fIconSide;
	float fGap;
	char sValue[256];
	int bEnabled;
	int bOpen;
	int bHover;
	int bFocus;
	int iRet;

	(void)iStateId;
	(void)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiIconPickerUpdateRects(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	__xuiIconPickerResolve(pWidget, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	bEnabled = xuiWidgetGetEffectiveEnabled(pWidget);
	bOpen = (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup));
	bHover = (xuiWidgetGetInputState(pWidget) & XUI_WIDGET_STATE_HOVER) != 0u;
	bFocus = (xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pWidget ||
	          xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pData->pViewport);
	iBackground = !bEnabled ? tResolved.iDisabledBackgroundColor :
	              bOpen ? tResolved.iOpenBackgroundColor :
	              bHover ? tResolved.iHoverBackgroundColor : tResolved.iBackgroundColor;
	iButton = bOpen ? tResolved.iButtonOpenColor :
	          bHover ? tResolved.iButtonHoverColor : tResolved.iButtonColor;
	iBorder = bFocus ? tResolved.iFocusBorderColor :
	          bHover ? tResolved.iHoverBorderColor : tResolved.iBorderColor;
	iTextColor = bEnabled ? tResolved.iTextColor : tResolved.iDisabledTextColor;
	iArrow = bEnabled ? tResolved.iArrowColor : tResolved.iDisabledArrowColor;
	tRect = xuiWidgetGetContentRect(pWidget);
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tRect), iBackground);
		if ( iRet != XUI_OK ) return iRet;
		iRet = pProxy->drawRectFill(pProxy, pDraw, pData->tButtonRect, iButton);
		if ( iRet != XUI_OK ) return iRet;
	}
	pIcon = (pData->pCategory != NULL && pData->iSelectedId != XUI_ICON_ID_INVALID) ?
	        xuiIconFindById(pData->pCategory, pData->iSelectedId) : NULL;
	sValue[0] = '\0';
	if ( pIcon != NULL ) {
		iRet = __xuiIconPickerFormatValue(pWidget, pData, pIcon, sValue, (int)sizeof(sValue));
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pIcon != NULL ) {
		if ( sValue[0] != '\0' ) {
			fGap = 7.0f;
			fIconSide = __xuiIconPickerMin(pData->tValueRect.fH,
				__xuiIconPickerMax(0.0f, pData->tValueRect.fW - fGap - 20.0f));
			tIcon = (xui_rect_t){pData->tValueRect.fX,
				pData->tValueRect.fY + (pData->tValueRect.fH - fIconSide) * 0.5f,
				fIconSide, fIconSide};
			tText = (xui_rect_t){tIcon.fX + tIcon.fW + fGap, pData->tValueRect.fY,
				__xuiIconPickerMax(0.0f, pData->tValueRect.fW - tIcon.fW - fGap),
				pData->tValueRect.fH};
		} else {
			fIconSide = __xuiIconPickerMin(pData->tValueRect.fW, pData->tValueRect.fH);
			tIcon = (xui_rect_t){
				pData->tValueRect.fX + (pData->tValueRect.fW - fIconSide) * 0.5f,
				pData->tValueRect.fY + (pData->tValueRect.fH - fIconSide) * 0.5f,
				fIconSide, fIconSide};
			tText = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		}
		iRet = __xuiIconPickerDrawIcon(pWidget, pDraw, pIcon, tIcon, bEnabled);
		if ( iRet != XUI_OK ) return iRet;
	} else {
		tText = pData->tValueRect;
		snprintf(sValue, sizeof(sValue), "%s", (pData->sPlaceholder != NULL) ? pData->sPlaceholder : "");
		iTextColor = bEnabled ? tResolved.iPlaceholderColor : tResolved.iDisabledTextColor;
	}
	pFont = tResolved.pFont;
	if ( sValue[0] != '\0' && tText.fW > 0.0f && pFont != NULL && pProxy->drawText != NULL ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sValue, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, xuiInternalSnapRect(tText), iTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiIconPickerDrawChevron(pProxy, pDraw, pData->tButtonRect, bOpen, iArrow);
	if ( iRet != XUI_OK ) return iRet;
	if ( pProxy->drawRectStroke != NULL && tResolved.fBorderWidth > 0.0f ) {
		return pProxy->drawRectStroke(pProxy, pDraw, xuiInternalSnapRect(tRect), tResolved.fBorderWidth, iBorder);
	}
	return XUI_OK;
}

static int __xuiIconPickerViewportRender(xui_widget pViewport, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pWidget;
	xui_icon_picker_data_t* pData;
	xui_icon_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_icon pIcon;
	xui_rect_t tViewport;
	xui_rect_t tCell;
	xui_rect_t tFill;
	xui_rect_t tIcon;
	float fOffsetY;
	float fRowStride;
	int iStartRow;
	int iEndRow;
	int iStart;
	int iEnd;
	int i;
	int iRet;

	(void)iStateId;
	pWidget = (xui_widget)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( (pViewport == NULL) || (pData == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	__xuiIconPickerResolve(pWidget, pData, &tResolved);
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	tViewport = xuiWidgetGetRect(pViewport);
	tViewport.fX = 0.0f;
	tViewport.fY = 0.0f;
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalSnapRect(tViewport), tResolved.iPopupPanelColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	fOffsetY = 0.0f;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, NULL, &fOffsetY);
	if ( iRet != XUI_OK ) return iRet;
	fRowStride = pData->fCellHeight + pData->fGapY;
	iStartRow = (fRowStride > 0.0f) ? (int)(fOffsetY / fRowStride) : 0;
	if ( iStartRow < 0 ) iStartRow = 0;
	iEndRow = (fRowStride > 0.0f) ? (int)((fOffsetY + tViewport.fH) / fRowStride) + 2 : 0;
	iStart = iStartRow * pData->iVisibleColumns;
	iEnd = iEndRow * pData->iVisibleColumns;
	if ( iStart < 0 ) iStart = 0;
	if ( iEnd > pData->iIconCount ) iEnd = pData->iIconCount;
	for ( i = iStart; i < iEnd; i++ ) {
		tCell = __xuiIconPickerItemContentRect(pData, i);
		tCell.fY -= fOffsetY;
		tCell = xuiInternalSnapRect(tCell);
		tFill = xuiInternalInsetRect(tCell, 1.0f);
		if ( i == pData->iSelectedIndex && pProxy->drawRectFill != NULL ) {
			iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, tResolved.iItemSelectedColor);
			if ( iRet != XUI_OK ) return iRet;
		} else if ( i == pData->iHoverIndex && pProxy->drawRectFill != NULL ) {
			iRet = pProxy->drawRectFill(pProxy, pDraw, tFill, tResolved.iItemHoverColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( i == pData->iFocusIndex && pProxy->drawRectStroke != NULL &&
		     xuiGetFocusWidget(xuiWidgetGetContext(pWidget)) == pViewport ) {
			iRet = pProxy->drawRectStroke(pProxy, pDraw, tFill, 1.0f, tResolved.iItemFocusColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		pIcon = (pData->pCategory != NULL) ? xuiIconFindById(pData->pCategory, pData->arrIds[i]) : NULL;
		tIcon = xuiInternalInsetRect(tCell, pData->fIconPadding);
		if ( tIcon.fW > 0.0f && tIcon.fH > 0.0f ) {
			iRet = __xuiIconPickerDrawIcon(pWidget, pDraw, pIcon, tIcon, 1);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiIconPickerHitWorld(xui_icon_picker_data_t* pData, float fX, float fY)
{
	xui_rect_t tWorld;
	float fOffsetX;
	float fOffsetY;
	float fLocalX;
	float fLocalY;
	float fStrideX;
	float fStrideY;
	float fCellX;
	float fCellY;
	int iColumn;
	int iRow;
	int iIndex;

	if ( (pData == NULL) || (pData->pViewport == NULL) || (pData->pFrame == NULL) ||
	     !__xuiIconPickerCoordinateValid(fX) || !__xuiIconPickerCoordinateValid(fY) ) return -1;
	tWorld = xuiWidgetGetWorldRect(pData->pViewport);
	if ( fX < tWorld.fX || fY < tWorld.fY ||
	     fX >= tWorld.fX + tWorld.fW || fY >= tWorld.fY + tWorld.fH ) return -1;
	fOffsetX = 0.0f;
	fOffsetY = 0.0f;
	if ( xuiScrollFrameGetOffset(pData->pFrame, &fOffsetX, &fOffsetY) != XUI_OK ) return -1;
	fLocalX = fX - tWorld.fX + fOffsetX;
	fLocalY = fY - tWorld.fY + fOffsetY;
	fStrideX = pData->fCellWidth + pData->fGapX;
	fStrideY = pData->fCellHeight + pData->fGapY;
	if ( fLocalX < 0.0f || fLocalY < 0.0f || fStrideX <= 0.0f || fStrideY <= 0.0f ) return -1;
	iColumn = (int)(fLocalX / fStrideX);
	iRow = (int)(fLocalY / fStrideY);
	if ( iColumn < 0 || iColumn >= pData->iVisibleColumns || iRow < 0 ) return -1;
	fCellX = fLocalX - (float)iColumn * fStrideX;
	fCellY = fLocalY - (float)iRow * fStrideY;
	if ( fCellX >= pData->fCellWidth || fCellY >= pData->fCellHeight ) return -1;
	iIndex = iRow * pData->iVisibleColumns + iColumn;
	return (iIndex >= 0 && iIndex < pData->iIconCount) ? iIndex : -1;
}

static int __xuiIconPickerInvalidate(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	int iRet;
	int iViewportRet;

	iRet = XUI_OK;
	if ( pWidget != NULL ) {
		iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	if ( pData != NULL && pData->pViewport != NULL ) {
		iViewportRet = xuiWidgetInvalidate(pData->pViewport, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet == XUI_OK ) iRet = iViewportRet;
	}
	return iRet;
}

static int __xuiIconPickerSetHover(xui_widget pWidget, xui_icon_picker_data_t* pData, int iIndex)
{
	if ( pData == NULL || pData->iHoverIndex == iIndex ) return XUI_OK;
	pData->iHoverIndex = iIndex;
	return __xuiIconPickerInvalidate(pWidget, pData);
}

static int __xuiIconPickerEnsureItemVisible(xui_widget pWidget, xui_icon_picker_data_t* pData, int iIndex)
{
	float fOldX;
	float fOldY;
	float fNewX;
	float fNewY;
	int iRet;

	if ( pData == NULL || pData->pFrame == NULL || iIndex < 0 || iIndex >= pData->iIconCount ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fOldX, &fOldY);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameEnsureRectVisible(pData->pFrame,
		__xuiIconPickerItemContentRect(pData, iIndex));
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameGetOffset(pData->pFrame, &fNewX, &fNewY);
	if ( iRet != XUI_OK ) return iRet;
	if ( fOldX != fNewX || fOldY != fNewY ) {
		xuiInternalTooltipCancel(xuiWidgetGetContext(pWidget));
		return __xuiIconPickerSetHover(pWidget, pData, -1);
	}
	return XUI_OK;
}

static int __xuiIconPickerSetFocusIndex(xui_widget pWidget, xui_icon_picker_data_t* pData, int iIndex)
{
	xui_context pContext;
	int iOldIndex;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pData->iIconCount <= 0 ) iIndex = -1;
	if ( iIndex < -1 ) iIndex = -1;
	if ( iIndex >= pData->iIconCount ) iIndex = pData->iIconCount - 1;
	if ( pData->iFocusIndex == iIndex ) return XUI_OK;
	iOldIndex = pData->iFocusIndex;
	pData->iFocusIndex = iIndex;
	if ( iIndex >= 0 ) {
		iRet = __xuiIconPickerEnsureItemVisible(pWidget, pData, iIndex);
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
		if ( iRet != XUI_OK ) {
			pData->iFocusIndex = iOldIndex;
			return iRet;
		}
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

static void __xuiIconPickerNotify(xui_widget pWidget, xui_icon_picker_data_t* pData, xui_icon_id iOldId, xui_icon_id iNewId)
{
	xui_icon_picker_change_proc onChange;
	void* pUser;

	if ( pWidget == NULL || pData == NULL || iOldId == iNewId ) return;
	if ( pData->iChangeCount < INT_MAX ) pData->iChangeCount++;
	onChange = pData->onChange;
	pUser = pData->pChangeUser;
	if ( onChange != NULL ) onChange(pWidget, iOldId, iNewId, pUser);
}

static int __xuiIconPickerSetSelectedInternal(xui_widget pWidget, xui_icon_picker_data_t* pData, xui_icon_id iId, int bNotify)
{
	xui_icon_id iOldId;
	int iIndex;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iIndex = __xuiIconPickerFindIndex(pData, iId);
	if ( iId != XUI_ICON_ID_INVALID && iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	iOldId = pData->iSelectedId;
	if ( iOldId == iId ) return XUI_OK;
	if ( pData->tPointerOwner.bActive || pData->iActiveIndex >= 0 ) {
		iRet = __xuiIconPickerCancelPress(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	pData->iSelectedId = iId;
	pData->iSelectedIndex = iIndex;
	pData->iFocusIndex = iIndex;
	iRet = __xuiIconPickerInvalidate(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->iSelectedId = iOldId;
		pData->iSelectedIndex = __xuiIconPickerFindIndex(pData, iOldId);
		pData->iFocusIndex = pData->iSelectedIndex;
		return iRet;
	}
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	if ( bNotify ) __xuiIconPickerNotify(pWidget, pData, iOldId, iId);
	return XUI_OK;
}

static int __xuiIconPickerCommitIndex(xui_widget pWidget, xui_icon_picker_data_t* pData, int iIndex)
{
	xui_context pContext;
	xui_icon_id iOldId;
	xui_icon_id iNewId;
	int iRet;

	if ( pData == NULL || iIndex < 0 || iIndex >= pData->iIconCount ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iOldId = pData->iSelectedId;
	iNewId = pData->arrIds[iIndex];
	iRet = __xuiIconPickerSetSelectedInternal(pWidget, pData, iNewId, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiIconPickerClose(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	__xuiIconPickerNotify(pWidget, pData, iOldId, iNewId);
	return XUI_OK;
}

static void __xuiIconPickerFrameChanged(xui_widget pFrame, float fOffsetX, float fOffsetY, void* pUser)
{
	xui_widget pWidget;
	xui_icon_picker_data_t* pData;

	(void)pFrame;
	(void)fOffsetX;
	(void)fOffsetY;
	pWidget = (xui_widget)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return;
	xuiInternalTooltipCancel(xuiWidgetGetContext(pWidget));
	(void)__xuiIconPickerSetHover(pWidget, pData, -1);
}

static int __xuiIconPickerViewportKeyDown(xui_widget pWidget, xui_icon_picker_data_t* pData, const xui_event_t* pEvent)
{
	int iCurrent;
	int iTarget;
	int iPage;
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	if ( (pEvent->iKey == XUI_KEY_F4 && pEvent->iModifiers == 0u) ||
	     (((pEvent->iModifiers & XUI_MOD_ALT) != 0u) && pEvent->iKey == XUI_KEY_UP) ) {
		iRet = xuiIconPickerClose(pWidget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( ((pEvent->iModifiers & XUI_MOD_ALT) != 0u) && pEvent->iKey == XUI_KEY_DOWN ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	iCurrent = pData->iFocusIndex;
	if ( iCurrent < 0 && pData->iIconCount > 0 ) iCurrent = 0;
	iTarget = iCurrent;
	iPage = pData->iVisibleRows * pData->iVisibleColumns;
	switch ( pEvent->iKey ) {
	case XUI_KEY_LEFT: iTarget = iCurrent - 1; break;
	case XUI_KEY_RIGHT: iTarget = iCurrent + 1; break;
	case XUI_KEY_UP: iTarget = iCurrent - pData->iVisibleColumns; break;
	case XUI_KEY_DOWN: iTarget = iCurrent + pData->iVisibleColumns; break;
	case XUI_KEY_PAGE_UP: iTarget = iCurrent - iPage; break;
	case XUI_KEY_PAGE_DOWN: iTarget = iCurrent + iPage; break;
	case XUI_KEY_HOME: iTarget = 0; break;
	case XUI_KEY_END: iTarget = pData->iIconCount - 1; break;
	case XUI_KEY_ENTER:
	case XUI_KEY_SPACE:
		iRet = __xuiIconPickerCommitIndex(pWidget, pData, pData->iFocusIndex);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	case XUI_KEY_ESCAPE:
		iRet = xuiIconPickerClose(pWidget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	default:
		return XUI_OK;
	}
	if ( pData->iIconCount > 0 ) {
		if ( iTarget < 0 ) iTarget = 0;
		if ( iTarget >= pData->iIconCount ) iTarget = pData->iIconCount - 1;
		iRet = __xuiIconPickerSetFocusIndex(pWidget, pData, iTarget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	return XUI_OK;
}

static int __xuiIconPickerViewportEvent(xui_widget pViewport, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pWidget;
	xui_icon_picker_data_t* pData;
	xui_context pContext;
	int iActiveIndex;
	int iIndex;
	int iRet;

	pWidget = (xui_widget)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) return XUI_OK;
		pData->iActiveIndex = -1;
		return __xuiIconPickerSetHover(pWidget, pData, -1);
	}
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ||
	     pData->pPopup == NULL || !xuiPopupIsOpen(pData->pPopup) ) {
		if ( pData->tPointerOwner.bActive || pData->iActiveIndex >= 0 ) {
			iRet = __xuiIconPickerCancelPress(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
		}
		return __xuiIconPickerSetHover(pWidget, pData, -1);
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_MOVE:
		if ( pEvent->iPhase != XUI_EVENT_PHASE_CAPTURE ) {
			if ( pData->tPointerOwner.bActive &&
			     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
				return XUI_OK;
			}
			iIndex = __xuiIconPickerHitWorld(pData, pEvent->fX, pEvent->fY);
			return __xuiIconPickerSetHover(pWidget, pData, iIndex);
		}
		break;
	case XUI_EVENT_POINTER_LEAVE:
		if ( pData->tPointerOwner.bActive &&
		     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
			return XUI_OK;
		}
		return __xuiIconPickerSetHover(pWidget, pData, -1);
	case XUI_EVENT_POINTER_DOWN:
		if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
		if ( pData->tPointerOwner.bActive ) {
			return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
				XUI_EVENT_DISPATCH_STOP : XUI_OK;
		}
		iIndex = __xuiIconPickerHitWorld(pData, pEvent->fX, pEvent->fY);
		if ( iIndex >= 0 ) {
			iRet = xuiSetFocusWidget(pContext, pViewport);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			iRet = __xuiIconPickerSetFocusIndex(pWidget, pData, iIndex);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			pData->iActiveIndex = iIndex;
			iRet = xuiInternalPointerOwnerBegin(pContext, pViewport, pEvent,
				&pData->tPointerOwner);
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
			if ( iRet != XUI_OK ) {
				pData->iActiveIndex = -1;
				return iRet;
			}
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_POINTER_UP:
		if ( pEvent->iPhase == XUI_EVENT_PHASE_CAPTURE || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
		if ( !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) return XUI_OK;
		iActiveIndex = pData->iActiveIndex;
		pData->iActiveIndex = -1;
		iIndex = __xuiIconPickerHitWorld(pData, pEvent->fX, pEvent->fY);
		iRet = xuiInternalPointerOwnerEnd(pContext, pViewport, pEvent,
			&pData->tPointerOwner);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
		iRet = __xuiIconPickerInvalidate(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( iActiveIndex >= 0 && iActiveIndex == iIndex ) {
			iRet = __xuiIconPickerCommitIndex(pWidget, pData, iIndex);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		return (iActiveIndex >= 0) ? (int)XUI_EVENT_DISPATCH_STOP : XUI_OK;
	case XUI_EVENT_KEY_DOWN:
		return __xuiIconPickerViewportKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiIconPickerSyncState(pWidget, pData);
	case XUI_EVENT_POINTER_WHEEL:
		return XUI_OK;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiIconPickerTooltipResolve(xui_context pContext, xui_widget pViewport, xui_tooltip_desc_t* pDesc, void* pUser)
{
	xui_widget pWidget;
	xui_icon_picker_data_t* pData;
	xui_icon pIcon;
	const char* sName;
	const char* sDisplay;
	int iIndex;

	(void)pViewport;
	pWidget = (xui_widget)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL || pDesc == NULL || pContext == NULL || pData->pCategory == NULL ) return 0;
	iIndex = __xuiIconPickerHitWorld(pData, (float)pContext->fPointerX, (float)pContext->fPointerY);
	if ( iIndex < 0 || iIndex >= pData->iIconCount ) return 0;
	pIcon = xuiIconFindById(pData->pCategory, pData->arrIds[iIndex]);
	if ( pIcon == NULL ) return 0;
	sName = xuiIconGetName(pIcon);
	sDisplay = xuiIconGetDisplayName(pIcon);
	if ( sDisplay != NULL && sDisplay[0] != '\0' && sName != NULL && strcmp(sDisplay, sName) != 0 ) {
		snprintf(pData->sTooltip, sizeof(pData->sTooltip), "%u  %s  (%s)",
			(unsigned)xuiIconGetId(pIcon), sDisplay, sName);
	} else {
		snprintf(pData->sTooltip, sizeof(pData->sTooltip), "%u  %s",
			(unsigned)xuiIconGetId(pIcon), (sName != NULL) ? sName : "");
	}
	xuiInternalUtf8TrimIncomplete(pData->sTooltip);
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iSize = sizeof(*pDesc);
	pDesc->iType = XUI_TOOLTIP_TEXT;
	pDesc->sText = pData->sTooltip;
	pDesc->iAnchor = XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fDelay = 0.45f;
	return 1;
}

static void __xuiIconPickerPopupChanged(xui_widget pPopup, int bOpen, void* pUser)
{
	xui_widget pWidget;
	xui_icon_picker_data_t* pData;

	(void)pPopup;
	pWidget = (xui_widget)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return;
	if ( !bOpen ) {
		(void)__xuiIconPickerCancelPress(pWidget, pData);
		pData->iHoverIndex = -1;
		pData->iFocusIndex = pData->iSelectedIndex;
	}
	(void)__xuiIconPickerSyncState(pWidget, pData);
}

static int __xuiIconPickerPointerDown(xui_widget pWidget, xui_icon_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE || pEvent->iButton != XUI_POINTER_BUTTON_LEFT ) return XUI_OK;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	iRet = xuiIconPickerToggle(pWidget);
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiIconPickerKeyDown(xui_widget pWidget, xui_icon_picker_data_t* pData, const xui_event_t* pEvent)
{
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE || !xuiWidgetGetEffectiveEnabled(pWidget) ||
	     !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_F4 && pEvent->iModifiers == 0u ) {
		iRet = xuiIconPickerToggle(pWidget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( (pEvent->iModifiers & XUI_MOD_ALT) != 0u &&
	     (pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN) ) {
		if ( pEvent->iKey == XUI_KEY_UP ) {
			iRet = xuiIconPickerIsOpen(pWidget) ? xuiIconPickerClose(pWidget) : XUI_OK;
		} else {
			iRet = xuiIconPickerIsOpen(pWidget) ? XUI_OK : xuiIconPickerOpen(pWidget);
		}
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	switch ( pEvent->iKey ) {
	case XUI_KEY_ENTER:
	case XUI_KEY_SPACE:
		iRet = xuiIconPickerToggle(pWidget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	case XUI_KEY_UP:
	case XUI_KEY_DOWN:
		iRet = xuiIconPickerOpen(pWidget);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	case XUI_KEY_ESCAPE:
		if ( xuiIconPickerIsOpen(pWidget) ) {
			iRet = xuiIconPickerClose(pWidget);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		break;
	default:
		break;
	}
	return XUI_OK;
}

static int __xuiIconPickerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_icon_picker_data_t* pData;
	xui_context pContext;
	xui_widget pPopup;
	int iRet;

	(void)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL || pEvent == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		return __xuiIconPickerPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT &&
		     xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_KEY_DOWN:
		return __xuiIconPickerKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiIconPickerSyncState(pWidget, pData);
	case XUI_EVENT_BOUNDS_CHANGED:
		iRet = __xuiIconPickerUpdateRects(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( xuiIconPickerIsOpen(pWidget) ) {
			iRet = __xuiIconPickerApplyPopup(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
			pPopup = pData->pPopup;
			iRet = xuiPopupApplyPlacement(pPopup);
			if ( iRet != XUI_OK ) return iRet;
		}
		return XUI_OK;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			iRet = xuiIconPickerClose(pWidget);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		}
		return __xuiIconPickerSyncState(pWidget, pData);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiIconPickerDefaultAction(xui_widget pWidget, void* pUser)
{
	(void)pUser;
	(void)xuiIconPickerToggle(pWidget);
}

static void __xuiIconPickerCancelAction(xui_widget pWidget, void* pUser)
{
	(void)pUser;
	(void)xuiIconPickerClose(pWidget);
}

static int __xuiIconPickerUpdate(xui_widget pWidget, float fDelta, void* pUser)
{
	xui_icon_picker_data_t* pData;
	xui_context pContext;
	xui_widget pPopup;
	uint32_t iGeneration;
	int iRet;

	(void)fDelta;
	(void)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iGeneration = (pData->pCategory != NULL) ? xuiIconCategoryGetGeneration(pData->pCategory) : 0u;
	if ( iGeneration != pData->iCategoryGeneration ) {
		iRet = __xuiIconPickerSyncCategory(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( xuiIconPickerIsOpen(pWidget) ) {
			iRet = __xuiIconPickerApplyPopup(pWidget, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
			pPopup = pData->pPopup;
			return xuiPopupApplyPlacement(pPopup);
		}
	}
	return XUI_OK;
}

static int __xuiIconPickerContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_icon_picker_data_t* pData;
	xui_icon_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;
	xui_font pFont;

	(void)pUser;
	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL || pSize == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pSize->fX = 190.0f;
	pSize->fY = 32.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	__xuiIconPickerResolve(pWidget, pData, &tResolved);
	pFont = tResolved.pFont;
	if ( pProxy != NULL && pProxy->fontGetMetrics != NULL && pFont != NULL &&
	     pProxy->fontGetMetrics(pProxy, pFont, &tMetrics) == XUI_OK ) {
		pSize->fY = __xuiIconPickerMax(32.0f, tMetrics.fLineHeight + 12.0f);
	}
	pSize->fX = xuiInternalSnapSize(pSize->fX);
	pSize->fY = xuiInternalSnapSize(pSize->fY);
	if ( tConstraint.fX >= 0.0f && tConstraint.fX < XUI_LAYOUT_UNBOUNDED &&
	     pSize->fX > tConstraint.fX ) pSize->fX = tConstraint.fX;
	if ( tConstraint.fY >= 0.0f && tConstraint.fY < XUI_LAYOUT_UNBOUNDED &&
	     pSize->fY > tConstraint.fY ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static void __xuiIconPickerDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iMeasureContainment = XUI_MEASURE_CONTAIN_BOTH;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_BLOCK;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiIconPickerDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiIconPickerInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetDefaultAction(pWidget, __xuiIconPickerDefaultAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCancelAction(pWidget, __xuiIconPickerCancelAction, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiIconPickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiIconPickerEvent, NULL);
	return iRet;
}

static int __xuiIconPickerInitViewport(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_cache_policy_t tPolicy;
	int iRet;

	memset(&tPolicy, 0, sizeof(tPolicy));
	tPolicy.iSize = sizeof(tPolicy);
	tPolicy.iPolicy = XUI_CACHE_POLICY_SELF;
	tPolicy.iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	tPolicy.iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiWidgetSetLayoutType(pData->pViewport, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pData->pViewport, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pData->pViewport, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pData->pViewport, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetImeMode(pData->pViewport, XUI_IME_DISABLED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pData->pViewport, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pData->pViewport, __xuiIconPickerViewportRender, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_MOVE, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_LEAVE, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_DOWN, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_UP, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_WHEEL, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_KEY_DOWN, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_FOCUS, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pData->pViewport, XUI_EVENT_BLUR, __xuiIconPickerViewportEvent, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTooltipResolver(pData->pViewport, __xuiIconPickerTooltipResolve, pWidget);
	return iRet;
}

static int __xuiIconPickerCreatePopup(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_popup_desc_t tPopupDesc;
	xui_scroll_frame_desc_t tFrameDesc;
	xui_widget pPopupScroll;
	int iRet;

	memset(&tPopupDesc, 0, sizeof(tPopupDesc));
	tPopupDesc.iSize = sizeof(tPopupDesc);
	tPopupDesc.pOwner = pWidget;
	tPopupDesc.fPadding = XUI_ICON_PICKER_POPUP_PADDING;
	tPopupDesc.fBorderWidth = 1.0f;
	tPopupDesc.fShadowSize = 5.0f;
	tPopupDesc.fGap = 2.0f;
	tPopupDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tPopupDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tPopupDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tPopupDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tPopupDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tPopupDesc.iFocusPolicy = XUI_POPUP_FOCUS_CUSTOM;
	tPopupDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tPopupDesc.iPanelColor = pData->iPopupPanelColor;
	tPopupDesc.iBorderColor = pData->iPopupBorderColor;
	tPopupDesc.iShadowColor = pData->iPopupShadowColor;
	iRet = xuiPopupCreate(xuiWidgetGetContext(pWidget), &pData->pPopup, &tPopupDesc);
	if ( iRet != XUI_OK ) return iRet;
	pData->pPanel = xuiPopupGetContentWidget(pData->pPopup);
	if ( pData->pPanel == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	pPopupScroll = xuiPopupGetScrollViewWidget(pData->pPopup);
	if ( pPopupScroll != NULL ) {
		iRet = xuiScrollViewSetScrollbarPolicy(pPopupScroll, XUI_SCROLLBAR_POLICY_HIDDEN, XUI_SCROLLBAR_POLICY_HIDDEN);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = xuiWidgetSetLayoutType(pData->pPanel, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pData->pPanel, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pData->pPanel, XUI_OVERFLOW_VISIBLE);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tFrameDesc, 0, sizeof(tFrameDesc));
	tFrameDesc.iSize = sizeof(tFrameDesc);
	tFrameDesc.iPolicyX = XUI_SCROLLBAR_POLICY_HIDDEN;
	tFrameDesc.iPolicyY = XUI_SCROLLBAR_POLICY_AUTO;
	tFrameDesc.iScrollbarMode = XUI_SCROLLBAR_MODE_COMPACT;
	tFrameDesc.iWheelAxis = XUI_WHEEL_AXIS_VERTICAL;
	tFrameDesc.fScrollbarSize = pData->fScrollbarSize;
	tFrameDesc.fWheelStep = pData->fCellHeight + pData->fGapY;
	tFrameDesc.iBackgroundColor = pData->iPopupPanelColor;
	iRet = xuiScrollFrameCreate(xuiWidgetGetContext(pWidget), &pData->pFrame, &tFrameDesc);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiWidgetAddChild(pData->pPanel, pData->pFrame);
	if ( iRet != XUI_OK ) return iRet;
	pData->pViewport = xuiScrollFrameGetViewportWidget(pData->pFrame);
	if ( pData->pViewport == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	iRet = __xuiIconPickerInitViewport(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiScrollFrameSetChange(pData->pFrame, __xuiIconPickerFrameChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetChange(pData->pPopup, __xuiIconPickerPopupChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusPolicy(pData->pPopup, XUI_POPUP_FOCUS_CUSTOM, pData->pViewport);
	return iRet;
}

static int __xuiIconPickerApplyPopup(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_widget pFrame;
	xui_widget pViewport;
	xui_rect_t tFrame;
	float fGridWidth;
	float fVisibleHeight;
	float fContentHeight;
	float fOuterContentWidth;
	int bNeedScrollbar;
	int iRet;

	if ( pWidget == NULL || pData == NULL || pData->pPopup == NULL || pData->pPanel == NULL ||
	     pData->pFrame == NULL || pData->pViewport == NULL ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	pFrame = pData->pFrame;
	pViewport = pData->pViewport;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		pData->iIconCount, pData->fCellWidth, pData->fCellHeight, pData->fGapX,
		pData->fGapY, pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	fGridWidth = __xuiIconPickerGridWidth(pData);
	fVisibleHeight = __xuiIconPickerVisibleHeight(pData);
	fContentHeight = __xuiIconPickerContentHeight(pData);
	bNeedScrollbar = __xuiIconPickerNeedScrollbar(pData);
	fOuterContentWidth = fGridWidth + (bNeedScrollbar ? pData->fScrollbarSize : 0.0f);
	iRet = xuiPopupSetOwner(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusRestore(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetContentSize(pPopup, fOuterContentWidth, fVisibleHeight);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = __xuiIconPickerApplyPopupColors(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetMetrics(pPopup, XUI_ICON_PICKER_POPUP_PADDING, 1.0f, 5.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE, XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_CLOSE);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, pViewport);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetGap(pPopup, 2.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pData->iPopupPlacement == XUI_ICON_PICKER_POPUP_TOP ) {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_TOP_LEFT);
		if ( iRet == XUI_OK && __xuiIconPickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_UP);
	} else {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_BOTTOM_LEFT);
		if ( iRet == XUI_OK && __xuiIconPickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupClearAnchorRect(pPopup);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiWidgetSetRect(pPanel, (xui_rect_t){0.0f, 0.0f, fOuterContentWidth, fVisibleHeight});
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	tFrame = (xui_rect_t){0.0f, 0.0f, fOuterContentWidth, fVisibleHeight};
	iRet = xuiWidgetSetRect(pFrame, tFrame);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiScrollFrameSetScrollbarPolicy(pFrame, XUI_SCROLLBAR_POLICY_HIDDEN,
		bNeedScrollbar ? XUI_SCROLLBAR_POLICY_AUTO : XUI_SCROLLBAR_POLICY_HIDDEN);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiScrollFrameSetMetrics(pFrame, pData->fScrollbarSize, 18.0f, 0.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiScrollFrameSetWheelStep(pFrame, pData->fCellHeight + pData->fGapY);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiScrollFrameSetContentSize(pFrame, fGridWidth, fContentHeight);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	return xuiWidgetArrangeChild(pPanel, pFrame, tFrame);
}

static int __xuiIconPickerRefreshPopup(xui_widget pWidget, xui_icon_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pPopup;
	int iRet;

	if ( pWidget == NULL || pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiIconPickerApplyPopup(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	pPopup = pData->pPopup;
	if ( pPopup != NULL && xuiPopupIsOpen(pPopup) ) return xuiPopupApplyPlacement(pPopup);
	return XUI_OK;
}

static int __xuiIconPickerInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_icon_picker_data_t* pData;
	const xui_icon_picker_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_icon_picker_data_t*)pTypeData;
	pDesc = (const xui_icon_picker_desc_t*)pCreateData;
	if ( pWidget == NULL || pData == NULL || !__xuiIconPickerDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDesc != NULL && pDesc->pCategory != NULL &&
	     !xuiInternalIconCategoryBelongsToContext(pDesc->pCategory, xuiWidgetGetContext(pWidget)) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	__xuiIconPickerDefaults(pData);
	iRet = __xuiIconPickerSetPlaceholderData(pData, (pDesc != NULL) ? pDesc->sPlaceholder : "");
	if ( iRet != XUI_OK ) goto fail;
	__xuiIconPickerApplyDesc(pData, pDesc);
	if ( pDesc != NULL && pDesc->pCategory != NULL ) {
		iRet = __xuiIconPickerApplyCategoryData(pData, pDesc->pCategory);
		if ( iRet != XUI_OK ) goto fail;
	}
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetImeMode(pWidget, XUI_IME_DISABLED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = __xuiIconPickerInitEvents(pWidget);
	if ( iRet != XUI_OK ) goto fail;
	iRet = __xuiIconPickerCreatePopup(pWidget, pData);
	if ( iRet != XUI_OK ) goto fail;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) goto fail;
	if ( pData->iSelectedId != XUI_ICON_ID_INVALID && pData->iSelectedIndex < 0 ) {
		pData->iSelectedId = XUI_ICON_ID_INVALID;
	}
	iRet = __xuiIconPickerUpdateRects(pWidget, pData);
	if ( iRet == XUI_OK ) iRet = __xuiIconPickerApplyPopup(pWidget, pData);
	if ( iRet == XUI_OK ) iRet = __xuiIconPickerSyncState(pWidget, pData);
	if ( iRet == XUI_OK ) return XUI_OK;

fail:
	__xuiIconPickerDestroy(pWidget, pData, NULL);
	return iRet;
}

static void __xuiIconPickerDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_icon_picker_data_t* pData;
	xui_widget pPopup;

	(void)pUser;
	pData = (xui_icon_picker_data_t*)pTypeData;
	if ( pData == NULL ) return;
	(void)__xuiIconPickerCancelPress(pWidget, pData);
	pPopup = pData->pPopup;
	pData->pPopup = NULL;
	pData->pPanel = NULL;
	pData->pFrame = NULL;
	pData->pViewport = NULL;
	if ( xuiInternalWidgetIsValid(pPopup) ) {
		(void)xuiPopupSetChange(pPopup, NULL, NULL);
		xuiWidgetDestroy(pPopup);
	}
	if ( pData->pCategory != NULL ) (void)xuiIconCategoryRelease(pData->pCategory);
	if ( pData->arrIds != NULL ) xrtFree(pData->arrIds);
	if ( pData->sPlaceholder != NULL ) xrtFree(pData->sPlaceholder);
	memset(pData, 0, sizeof(*pData));
}

static xui_icon_picker_data_t* __xuiIconPickerGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "iconpicker");
	if ( pType == NULL || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_icon_picker_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiIconPickerAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_icon_picker_data_t* pData;
	xui_icon pIcon;
	const char* sValue;

	pData = __xuiIconPickerGetData(pWidget);
	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_COMBOBOX;
	pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	pNode->iState |= xuiIconPickerIsOpen(pWidget) ? XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_SET_VALUE);
	pNode->sName = (pData->sPlaceholder != NULL) ? pData->sPlaceholder : "";
	pData->sAccessibleValue[0] = '\0';
	pIcon = (pData->pCategory != NULL && pData->iSelectedId != XUI_ICON_ID_INVALID) ?
		xuiIconFindById(pData->pCategory, pData->iSelectedId) : NULL;
	if ( pIcon != NULL ) {
		sValue = xuiIconGetDisplayName(pIcon);
		if ( sValue == NULL || sValue[0] == '\0' ) sValue = xuiIconGetName(pIcon);
		if ( sValue != NULL && sValue[0] != '\0' ) {
			snprintf(pData->sAccessibleValue, sizeof(pData->sAccessibleValue), "%s", sValue);
			xuiInternalUtf8TrimIncomplete(pData->sAccessibleValue);
		} else {
			snprintf(pData->sAccessibleValue, sizeof(pData->sAccessibleValue), "%u", (unsigned)pData->iSelectedId);
		}
	}
	pNode->sValue = pData->sAccessibleValue;
	return XUI_OK;
}

static int __xuiIconPickerAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	xui_icon_picker_data_t* pData;
	const xui_accessible_value_t* pValue;
	xui_icon pIcon;
	xui_icon_id iId;

	pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND ) return xuiIconPickerOpen(pWidget);
	if ( iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) return xuiIconPickerClose(pWidget);
	if ( iAction != XUI_ACCESSIBLE_ACTION_SET_VALUE ) return XUI_ERROR_UNSUPPORTED;
	pValue = (const xui_accessible_value_t*)pPayload;
	if ( pValue == NULL || pValue->iSize < sizeof(*pValue) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pValue->iType == XUI_ACCESSIBLE_VALUE_NUMBER ) {
		if ( pValue->fNumber != pValue->fNumber ||
		     pValue->fNumber < 0.0 || pValue->fNumber > (double)UINT32_MAX ) return XUI_ERROR_INVALID_ARGUMENT;
		iId = (xui_icon_id)pValue->fNumber;
		if ( (double)iId != pValue->fNumber ) return XUI_ERROR_INVALID_ARGUMENT;
		return __xuiIconPickerSetSelectedInternal(pWidget, pData, iId, 1);
	}
	if ( pValue->iType == XUI_ACCESSIBLE_VALUE_TEXT ) {
		if ( pValue->sText == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		if ( pValue->sText[0] == '\0' ) return __xuiIconPickerSetSelectedInternal(pWidget, pData, XUI_ICON_ID_INVALID, 1);
		pIcon = (pData->pCategory != NULL) ? xuiIconFind(pData->pCategory, pValue->sText) : NULL;
		if ( pIcon == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
		return __xuiIconPickerSetSelectedInternal(pWidget, pData, xuiIconGetId(pIcon), 1);
	}
	return XUI_ERROR_INVALID_ARGUMENT;
}

static const xui_internal_accessibility_adapter_t g_xuiIconPickerAccessible = {
	.getNode = __xuiIconPickerAccessibleNode,
	.performAction = __xuiIconPickerAccessibleAction
};

static void __xuiIconPickerRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	static const char* arrColors[] = {
		"iconpicker.text.color",
		"iconpicker.placeholder.color",
		"iconpicker.text.disabled_color",
		"iconpicker.background.color",
		"iconpicker.background.hover_color",
		"iconpicker.background.open_color",
		"iconpicker.background.disabled_color",
		"iconpicker.border.color",
		"iconpicker.border.hover_color",
		"iconpicker.border.focus_color",
		"iconpicker.arrow.color",
		"iconpicker.arrow.disabled_color",
		"iconpicker.button.color",
		"iconpicker.button.hover_color",
		"iconpicker.button.open_color",
		"iconpicker.popup.panel_color",
		"iconpicker.popup.border_color",
		"iconpicker.popup.shadow_color",
		"iconpicker.item.hover_color",
		"iconpicker.item.selected_color",
		"iconpicker.item.focus_color",
	};
	xui_style_property_info_t tInfo;
	size_t i;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.pWidgetType = pType;
	tInfo.iValueType = XUI_STYLE_VALUE_COLOR;
	tInfo.iDirtyFlags = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	for ( i = 0; i < sizeof(arrColors) / sizeof(arrColors[0]); ++i ) {
		tInfo.sName = arrColors[i];
		(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
	}
	tInfo.sName = "iconpicker.border.width";
	tInfo.iValueType = XUI_STYLE_VALUE_FLOAT;
	tInfo.iDirtyFlags |= XUI_WIDGET_DIRTY_LAYOUT;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
	tInfo.sName = "font.name";
	tInfo.iValueType = XUI_STYLE_VALUE_STRING;
	tInfo.iFlags = XUI_STYLE_PROPERTY_INHERITED;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

XUI_API xui_widget_type xuiIconPickerGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "iconpicker");
	if ( pType != NULL ) {
		__xuiIconPickerRegisterStyleProperties(pContext, pType);
		pType->onPreparePaint = __xuiIconPickerSyncStyle;
		pType->pAccessibleAdapter = &g_xuiIconPickerAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "iconpicker";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_icon_picker_data_t);
	tDesc.onInit = __xuiIconPickerInit;
	tDesc.onDestroy = __xuiIconPickerDestroy;
	tDesc.onContentMeasure = __xuiIconPickerContentMeasure;
	tDesc.onCacheRender = __xuiIconPickerCacheRender;
	tDesc.onUpdate = __xuiIconPickerUpdate;
	__xuiIconPickerDefaultLayout(&tDesc.tLayout);
	__xuiIconPickerDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet == XUI_OK ) {
		__xuiIconPickerRegisterStyleProperties(pContext, pType);
		pType->onPreparePaint = __xuiIconPickerSyncStyle;
		pType->pAccessibleAdapter = &g_xuiIconPickerAccessible;
	}
	return (iRet == XUI_OK) ? pType : NULL;
}

XUI_API int xuiIconPickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_icon_picker_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !__xuiIconPickerDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pDesc != NULL && pDesc->pCategory != NULL &&
	     !xuiInternalIconCategoryBelongsToContext(pDesc->pCategory, pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pType = xuiIconPickerGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiIconPickerSetChange(xui_widget pWidget, xui_icon_picker_change_proc onChange, void* pUser)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetFormatter(xui_widget pWidget, xui_icon_picker_format_proc onFormat, void* pUser)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->onFormat == onFormat && pData->pFormatUser == pUser ) return XUI_OK;
	pData->onFormat = onFormat;
	pData->pFormatUser = pUser;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiIconPickerSetCategory(xui_widget pWidget, xui_icon_category pCategory)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	xui_icon_category pOldCategory;
	xui_icon_id iOldSelectedId;
	int iOldFocusIndex;
	int iOldHoverIndex;
	int iNewCount;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	if ( pCategory != NULL && !xuiInternalIconCategoryBelongsToContext(pCategory, pContext) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pCategory == pData->pCategory ) {
		uint32_t iOldGeneration = pData->iCategoryGeneration;
		int iOldCount = pData->iIconCount;
		iRet = __xuiIconPickerSyncCategory(pWidget, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( iOldGeneration == pData->iCategoryGeneration && iOldCount == pData->iIconCount ) return XUI_OK;
		return __xuiIconPickerRefreshPopup(pWidget, pData);
	}
	iNewCount = (pCategory != NULL) ? xuiIconCategoryGetIconCount(pCategory) : 0;
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		iNewCount, pData->fCellWidth, pData->fCellHeight, pData->fGapX, pData->fGapY,
		pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiIconPickerReserveIds(pData, iNewCount);
	if ( iRet != XUI_OK ) return iRet;
	pOldCategory = pData->pCategory;
	if ( pOldCategory != NULL ) {
		iRet = xuiIconCategoryAddRef(pOldCategory);
		if ( iRet != XUI_OK ) return iRet;
	}
	iOldSelectedId = pData->iSelectedId;
	iOldFocusIndex = pData->iFocusIndex;
	iOldHoverIndex = pData->iHoverIndex;
	iRet = xuiIconPickerClose(pWidget);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) {
		if ( pOldCategory != NULL ) (void)xuiIconCategoryRelease(pOldCategory);
		return (iRet == XUI_OK) ? XUI_OK : iRet;
	}
	if ( iRet != XUI_OK ) goto release_old_category;
	iRet = __xuiIconPickerApplyCategoryData(pData, pCategory);
	if ( iRet != XUI_OK ) goto release_old_category;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) goto rollback_category;
	pData->iSelectedId = XUI_ICON_ID_INVALID;
	pData->iSelectedIndex = -1;
	pData->iFocusIndex = -1;
	pData->iHoverIndex = -1;
	pData->iActiveIndex = -1;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) {
		if ( pOldCategory != NULL ) (void)xuiIconCategoryRelease(pOldCategory);
		return (iRet == XUI_OK) ? XUI_OK : iRet;
	}
	if ( iRet != XUI_OK ) goto rollback_category;
	iRet = __xuiIconPickerInvalidate(pWidget, pData);
	if ( iRet == XUI_OK ) {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_SELECTION_CHANGED);
	}
	if ( pOldCategory != NULL ) (void)xuiIconCategoryRelease(pOldCategory);
	return iRet;

rollback_category:
	(void)__xuiIconPickerApplyCategoryData(pData, pOldCategory);
	(void)__xuiIconPickerSyncCategory(pWidget, pData);
	pData->iSelectedId = iOldSelectedId;
	pData->iSelectedIndex = __xuiIconPickerFindIndex(pData, iOldSelectedId);
	pData->iFocusIndex = iOldFocusIndex;
	pData->iHoverIndex = iOldHoverIndex;
	pData->iActiveIndex = -1;
	(void)__xuiIconPickerRefreshPopup(pWidget, pData);
release_old_category:
	if ( pOldCategory != NULL ) (void)xuiIconCategoryRelease(pOldCategory);
	return iRet;
}

XUI_API xui_icon_category xuiIconPickerGetCategory(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->pCategory : NULL;
}

XUI_API int xuiIconPickerSetSelectedId(xui_widget pWidget, xui_icon_id iId)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? __xuiIconPickerSetSelectedInternal(pWidget, pData, iId, 0) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API xui_icon_id xuiIconPickerGetSelectedId(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iSelectedId : XUI_ICON_ID_INVALID;
}

XUI_API xui_icon xuiIconPickerGetSelectedIcon(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL || pData->pCategory == NULL || pData->iSelectedId == XUI_ICON_ID_INVALID ) return NULL;
	return xuiIconFindById(pData->pCategory, pData->iSelectedId);
}

XUI_API int xuiIconPickerClearSelection(xui_widget pWidget)
{
	return xuiIconPickerSetSelectedId(pWidget, XUI_ICON_ID_INVALID);
}

XUI_API int xuiIconPickerSetTextMode(xui_widget pWidget, int iMode)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	int iOldMode;
	int iRet;
	if ( pData == NULL || !__xuiIconPickerTextModeValid(iMode) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTextMode == iMode ) return XUI_OK;
	iOldMode = pData->iTextMode;
	pData->iTextMode = iMode;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) pData->iTextMode = iOldMode;
	else xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return iRet;
}

XUI_API int xuiIconPickerGetTextMode(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iTextMode : XUI_ICON_PICKER_TEXT_NONE;
}

XUI_API int xuiIconPickerSetPlaceholder(xui_widget pWidget, const char* sText)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	char* sNew;
	char* sOld;
	const char* sValue;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	sValue = (sText != NULL) ? sText : "";
	if ( strcmp((pData->sPlaceholder != NULL) ? pData->sPlaceholder : "", sValue) == 0 ) return XUI_OK;
	sNew = __xuiIconPickerDup(sValue);
	if ( sNew == NULL ) return XUI_ERROR_OUT_OF_MEMORY;
	sOld = pData->sPlaceholder;
	pData->sPlaceholder = sNew;
	iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->sPlaceholder = sOld;
		xrtFree(sNew);
		return iRet;
	}
	if ( sOld != NULL ) xrtFree(sOld);
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
	return XUI_OK;
}

XUI_API const char* xuiIconPickerGetPlaceholder(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL && pData->sPlaceholder != NULL) ? pData->sPlaceholder : "";
}

XUI_API int xuiIconPickerSetGrid(xui_widget pWidget, int iRows, int iColumns)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	int iOldRows;
	int iOldColumns;
	int iRet;
	if ( pData == NULL || iRows <= 0 || iColumns <= 0 || iRows > 1024 || iColumns > 1024 ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiIconPickerGeometryValid(iRows, iColumns, pData->iIconCount,
		pData->fCellWidth, pData->fCellHeight, pData->fGapX, pData->fGapY,
		pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iVisibleRows == iRows && pData->iVisibleColumns == iColumns ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iOldRows = pData->iVisibleRows;
	iOldColumns = pData->iVisibleColumns;
	pData->iVisibleRows = iRows;
	pData->iVisibleColumns = iColumns;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->iVisibleRows = iOldRows;
		pData->iVisibleColumns = iOldColumns;
		(void)__xuiIconPickerRefreshPopup(pWidget, pData);
		return iRet;
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetGrid(xui_widget pWidget, int* pRows, int* pColumns)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pRows != NULL ) *pRows = pData->iVisibleRows;
	if ( pColumns != NULL ) *pColumns = pData->iVisibleColumns;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetCellSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	float fOldWidth;
	float fOldHeight;
	int iRet;
	if ( pData == NULL || fWidth <= 0.0f || fHeight <= 0.0f ||
	     !__xuiIconPickerFloatValid(fWidth) || !__xuiIconPickerFloatValid(fHeight) ||
	     pData->fIconPadding * 2.0f >= fWidth || pData->fIconPadding * 2.0f >= fHeight ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		pData->iIconCount, fWidth, fHeight, pData->fGapX, pData->fGapY,
		pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fCellWidth == fWidth && pData->fCellHeight == fHeight ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	fOldWidth = pData->fCellWidth;
	fOldHeight = pData->fCellHeight;
	pData->fCellWidth = fWidth;
	pData->fCellHeight = fHeight;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->fCellWidth = fOldWidth;
		pData->fCellHeight = fOldHeight;
		(void)__xuiIconPickerRefreshPopup(pWidget, pData);
		return iRet;
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetCellSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fCellWidth;
	if ( pHeight != NULL ) *pHeight = pData->fCellHeight;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetSpacing(xui_widget pWidget, float fGapX, float fGapY, float fIconPadding)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	float fOldGapX;
	float fOldGapY;
	float fOldIconPadding;
	int iRet;
	if ( pData == NULL || !__xuiIconPickerFloatValid(fGapX) || !__xuiIconPickerFloatValid(fGapY) ||
	     !__xuiIconPickerFloatValid(fIconPadding) || fIconPadding * 2.0f >= pData->fCellWidth ||
	     fIconPadding * 2.0f >= pData->fCellHeight ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		pData->iIconCount, pData->fCellWidth, pData->fCellHeight, fGapX, fGapY,
		pData->fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fGapX == fGapX && pData->fGapY == fGapY && pData->fIconPadding == fIconPadding ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	fOldGapX = pData->fGapX;
	fOldGapY = pData->fGapY;
	fOldIconPadding = pData->fIconPadding;
	pData->fGapX = fGapX;
	pData->fGapY = fGapY;
	pData->fIconPadding = fIconPadding;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->fGapX = fOldGapX;
		pData->fGapY = fOldGapY;
		pData->fIconPadding = fOldIconPadding;
		(void)__xuiIconPickerRefreshPopup(pWidget, pData);
		return iRet;
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetSpacing(xui_widget pWidget, float* pGapX, float* pGapY, float* pIconPadding)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pGapX != NULL ) *pGapX = pData->fGapX;
	if ( pGapY != NULL ) *pGapY = pData->fGapY;
	if ( pIconPadding != NULL ) *pIconPadding = pData->fIconPadding;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetValuePadding(xui_widget pWidget, xui_thickness_t tPadding)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_thickness_t tOldPadding;
	int iRet;
	if ( pData == NULL || !__xuiIconPickerThicknessValid(tPadding) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->tValuePadding.fLeft == tPadding.fLeft &&
	     pData->tValuePadding.fTop == tPadding.fTop &&
	     pData->tValuePadding.fRight == tPadding.fRight &&
	     pData->tValuePadding.fBottom == tPadding.fBottom ) return XUI_OK;
	tOldPadding = pData->tValuePadding;
	pData->tValuePadding = tPadding;
	iRet = __xuiIconPickerUpdateRects(pWidget, pData);
	if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) {
		pData->tValuePadding = tOldPadding;
		(void)__xuiIconPickerUpdateRects(pWidget, pData);
	}
	return iRet;
}

XUI_API xui_thickness_t xuiIconPickerGetValuePadding(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->tValuePadding : (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f};
}

XUI_API int xuiIconPickerSetPopupPlacement(xui_widget pWidget, int iPlacement)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	int iOldPlacement;
	int iRet;
	if ( pData == NULL || !__xuiIconPickerPlacementValid(iPlacement) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPopupPlacement == iPlacement ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iOldPlacement = pData->iPopupPlacement;
	pData->iPopupPlacement = iPlacement;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->iPopupPlacement = iOldPlacement;
		(void)__xuiIconPickerRefreshPopup(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiIconPickerGetPopupPlacement(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iPopupPlacement : XUI_ICON_PICKER_POPUP_AUTO;
}

XUI_API int xuiIconPickerOpen(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pViewport;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiIconPickerApplyPopup(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	pPopup = pData->pPopup;
	pViewport = pData->pViewport;
	pData->iHoverIndex = -1;
	pData->iFocusIndex = (pData->iSelectedIndex >= 0) ? pData->iSelectedIndex :
		((pData->iIconCount > 0) ? 0 : -1);
	iRet = xuiPopupSetOpen(pPopup, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pData->iFocusIndex >= 0 ) {
		iRet = __xuiIconPickerEnsureItemVisible(pWidget, pData, pData->iFocusIndex);
		if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
		if ( iRet != XUI_OK ) goto fail_open;
	}
	iRet = xuiSetFocusWidget(pContext, pViewport);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) goto fail_open;
	iRet = __xuiIconPickerSyncState(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) goto fail_open;
	return XUI_OK;

fail_open:
	(void)xuiPopupSetOpen(pPopup, 0);
	if ( __xuiIconPickerAlive(pContext, pWidget, pData) ) (void)__xuiIconPickerSyncState(pWidget, pData);
	return iRet;
}

XUI_API int xuiIconPickerClose(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	xui_widget pPopup;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pPopup == NULL ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	iRet = __xuiIconPickerCancelPress(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetOpen(pPopup, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = __xuiIconPickerSyncState(pWidget, pData);
	return iRet;
}

XUI_API int xuiIconPickerToggle(xui_widget pWidget)
{
	return xuiIconPickerIsOpen(pWidget) ? xuiIconPickerClose(pWidget) : xuiIconPickerOpen(pWidget);
}

XUI_API int xuiIconPickerIsOpen(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL && pData->pPopup != NULL) ? xuiPopupIsOpen(pData->pPopup) : 0;
}

XUI_API int xuiIconPickerEnsureVisible(xui_widget pWidget, xui_icon_id iId)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	int iIndex;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iRet = __xuiIconPickerSyncCategory(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	iIndex = __xuiIconPickerFindIndex(pData, iId);
	if ( iIndex < 0 ) return XUI_ERROR_INVALID_ARGUMENT;
	return __xuiIconPickerEnsureItemVisible(pWidget, pData, iIndex);
}

XUI_API int xuiIconPickerSetMetrics(xui_widget pWidget, float fBorderWidth, float fScrollbarSize)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	float fOldBorderWidth;
	float fOldScrollbarSize;
	int iRet;
	if ( pData == NULL || !__xuiIconPickerFloatValid(fBorderWidth) ||
	     fScrollbarSize <= 0.0f || fScrollbarSize > 8.0f ||
	     !__xuiIconPickerFloatValid(fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiIconPickerGeometryValid(pData->iVisibleRows, pData->iVisibleColumns,
		pData->iIconCount, pData->fCellWidth, pData->fCellHeight, pData->fGapX,
		pData->fGapY, fScrollbarSize) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fBorderWidth == fBorderWidth && pData->fScrollbarSize == fScrollbarSize ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	fOldBorderWidth = pData->fBorderWidth;
	fOldScrollbarSize = pData->fScrollbarSize;
	pData->fBorderWidth = fBorderWidth;
	pData->fScrollbarSize = fScrollbarSize;
	iRet = __xuiIconPickerRefreshPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->fBorderWidth = fOldBorderWidth;
		pData->fScrollbarSize = fOldScrollbarSize;
		(void)__xuiIconPickerRefreshPopup(pWidget, pData);
		return iRet;
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetMetrics(xui_widget pWidget, float* pBorderWidth, float* pScrollbarSize)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	if ( pScrollbarSize != NULL ) *pScrollbarSize = pData->fScrollbarSize;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iPlaceholder, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTextColor == iText && pData->iPlaceholderColor == iPlaceholder &&
	     pData->iDisabledTextColor == iDisabledText && pData->iBackgroundColor == iBackground &&
	     pData->iHoverBackgroundColor == iHoverBackground && pData->iOpenBackgroundColor == iOpenBackground &&
	     pData->iDisabledBackgroundColor == iDisabledBackground ) return XUI_OK;
	pData->iTextColor = iText;
	pData->iPlaceholderColor = iPlaceholder;
	pData->iDisabledTextColor = iDisabledText;
	pData->iBackgroundColor = iBackground;
	pData->iHoverBackgroundColor = iHoverBackground;
	pData->iOpenBackgroundColor = iOpenBackground;
	pData->iDisabledBackgroundColor = iDisabledBackground;
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pPlaceholder, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pPlaceholder != NULL ) *pPlaceholder = pData->iPlaceholderColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pOpenBackground != NULL ) *pOpenBackground = pData->iOpenBackgroundColor;
	if ( pDisabledBackground != NULL ) *pDisabledBackground = pData->iDisabledBackgroundColor;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBorderColor == iBorder && pData->iHoverBorderColor == iHoverBorder &&
	     pData->iFocusBorderColor == iFocusBorder ) return XUI_OK;
	pData->iBorderColor = iBorder;
	pData->iHoverBorderColor = iHoverBorder;
	pData->iFocusBorderColor = iFocusBorder;
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pFocusBorder != NULL ) *pFocusBorder = pData->iFocusBorderColor;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iArrowColor == iArrow && pData->iDisabledArrowColor == iDisabledArrow ) return XUI_OK;
	pData->iArrowColor = iArrow;
	pData->iDisabledArrowColor = iDisabledArrow;
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pArrow != NULL ) *pArrow = pData->iArrowColor;
	if ( pDisabledArrow != NULL ) *pDisabledArrow = pData->iDisabledArrowColor;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetButtonColors(xui_widget pWidget, uint32_t iButton, uint32_t iHover, uint32_t iOpen)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iButtonColor == iButton && pData->iButtonHoverColor == iHover &&
	     pData->iButtonOpenColor == iOpen ) return XUI_OK;
	pData->iButtonColor = iButton;
	pData->iButtonHoverColor = iHover;
	pData->iButtonOpenColor = iOpen;
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetButtonColors(xui_widget pWidget, uint32_t* pButton, uint32_t* pHover, uint32_t* pOpen)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pButton != NULL ) *pButton = pData->iButtonColor;
	if ( pHover != NULL ) *pHover = pData->iButtonHoverColor;
	if ( pOpen != NULL ) *pOpen = pData->iButtonOpenColor;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iHover, uint32_t iSelected, uint32_t iFocus)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_context pContext;
	uint32_t arrOldColors[6];
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPopupPanelColor == iPanel && pData->iPopupBorderColor == iBorder &&
	     pData->iPopupShadowColor == iShadow && pData->iItemHoverColor == iHover &&
	     pData->iItemSelectedColor == iSelected && pData->iItemFocusColor == iFocus ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	arrOldColors[0] = pData->iPopupPanelColor;
	arrOldColors[1] = pData->iPopupBorderColor;
	arrOldColors[2] = pData->iPopupShadowColor;
	arrOldColors[3] = pData->iItemHoverColor;
	arrOldColors[4] = pData->iItemSelectedColor;
	arrOldColors[5] = pData->iItemFocusColor;
	pData->iPopupPanelColor = iPanel;
	pData->iPopupBorderColor = iBorder;
	pData->iPopupShadowColor = iShadow;
	pData->iItemHoverColor = iHover;
	pData->iItemSelectedColor = iSelected;
	pData->iItemFocusColor = iFocus;
	iRet = __xuiIconPickerApplyPopup(pWidget, pData);
	if ( !__xuiIconPickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
	if ( iRet != XUI_OK ) {
		pData->iPopupPanelColor = arrOldColors[0];
		pData->iPopupBorderColor = arrOldColors[1];
		pData->iPopupShadowColor = arrOldColors[2];
		pData->iItemHoverColor = arrOldColors[3];
		pData->iItemSelectedColor = arrOldColors[4];
		pData->iItemFocusColor = arrOldColors[5];
		(void)__xuiIconPickerApplyPopup(pWidget, pData);
		return iRet;
	}
	return __xuiIconPickerInvalidate(pWidget, pData);
}

XUI_API int xuiIconPickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pHover, uint32_t* pSelected, uint32_t* pFocus)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPanel != NULL ) *pPanel = pData->iPopupPanelColor;
	if ( pBorder != NULL ) *pBorder = pData->iPopupBorderColor;
	if ( pShadow != NULL ) *pShadow = pData->iPopupShadowColor;
	if ( pHover != NULL ) *pHover = pData->iItemHoverColor;
	if ( pSelected != NULL ) *pSelected = pData->iItemSelectedColor;
	if ( pFocus != NULL ) *pFocus = pData->iItemFocusColor;
	return XUI_OK;
}

XUI_API int xuiIconPickerSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pData->pFont = pFont;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API xui_font xuiIconPickerGetFont(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API xui_widget xuiIconPickerGetPopupWidget(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->pPopup : NULL;
}

XUI_API xui_widget xuiIconPickerGetFrameWidget(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->pFrame : NULL;
}

XUI_API xui_widget xuiIconPickerGetViewportWidget(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->pViewport : NULL;
}

XUI_API xui_rect_t xuiIconPickerGetButtonRect(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	(void)__xuiIconPickerUpdateRects(pWidget, pData);
	return pData->tButtonRect;
}

XUI_API xui_rect_t xuiIconPickerGetValueRect(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	(void)__xuiIconPickerUpdateRects(pWidget, pData);
	return pData->tValueRect;
}

XUI_API xui_rect_t xuiIconPickerGetItemRect(xui_widget pWidget, int iIndex)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	xui_rect_t tRect;
	float fOffsetX;
	float fOffsetY;

	if ( pData == NULL ) return (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
	tRect = __xuiIconPickerItemContentRect(pData, iIndex);
	fOffsetX = 0.0f;
	fOffsetY = 0.0f;
	if ( pData->pFrame != NULL ) (void)xuiScrollFrameGetOffset(pData->pFrame, &fOffsetX, &fOffsetY);
	tRect.fX -= fOffsetX;
	tRect.fY -= fOffsetY;
	return tRect;
}

XUI_API int xuiIconPickerGetHoverIndex(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iHoverIndex : -1;
}

XUI_API int xuiIconPickerGetFocusIndex(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iFocusIndex : -1;
}

XUI_API uint32_t xuiIconPickerGetState(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? __xuiIconPickerState(pWidget, pData) : 0u;
}

XUI_API int xuiIconPickerGetChangeCount(xui_widget pWidget)
{
	xui_icon_picker_data_t* pData = __xuiIconPickerGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

#endif
