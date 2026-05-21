static const char* __xgeXuiComboBoxItemText(xge_xui_combo_box pCombo, int iIndex)
{
	if ( (pCombo == NULL) || (iIndex < 0) || (iIndex >= pCombo->iItemCount) ) {
		return "";
	}
	if ( pCombo->arrItemData != NULL ) {
		return (pCombo->arrItemData[iIndex].sText != NULL) ? pCombo->arrItemData[iIndex].sText : "";
	}
	if ( pCombo->arrItems != NULL ) {
		return (pCombo->arrItems[iIndex] != NULL) ? pCombo->arrItems[iIndex] : "";
	}
	return "";
}

static int __xgeXuiComboBoxItemValue(xge_xui_combo_box pCombo, int iIndex)
{
	if ( (pCombo == NULL) || (iIndex < 0) || (iIndex >= pCombo->iItemCount) ) {
		return -1;
	}
	if ( pCombo->arrItemData != NULL ) {
		return pCombo->arrItemData[iIndex].iValue;
	}
	return iIndex;
}

static int __xgeXuiComboBoxItemEnabled(xge_xui_combo_box pCombo, int iIndex)
{
	if ( (pCombo == NULL) || (iIndex < 0) || (iIndex >= pCombo->iItemCount) ) {
		return 0;
	}
	if ( (pCombo->arrItemData != NULL) && ((pCombo->arrItemData[iIndex].bEnabled == 0) || (pCombo->arrItemData[iIndex].bSeparator != 0)) ) {
		return 0;
	}
	if ( (pCombo->arrEnabled != NULL) && (iIndex < pCombo->tList.iEnabledCount) ) {
		return pCombo->arrEnabled[iIndex] != 0;
	}
	return 1;
}

static int __xgeXuiComboBoxFindEnabled(xge_xui_combo_box pCombo, int iStart, int iStep)
{
	int i;

	if ( (pCombo == NULL) || (pCombo->iItemCount <= 0) || (iStep == 0) ) {
		return -1;
	}
	i = iStart;
	while ( (i >= 0) && (i < pCombo->iItemCount) ) {
		if ( __xgeXuiComboBoxItemEnabled(pCombo, i) ) {
			return i;
		}
		i += iStep;
	}
	return -1;
}

static int __xgeXuiComboBoxNextEnabled(xge_xui_combo_box pCombo, int iCurrent, int iStep)
{
	int i;

	if ( pCombo == NULL ) {
		return -1;
	}
	i = iCurrent + iStep;
	if ( i < 0 ) {
		i = pCombo->iItemCount - 1;
	}
	if ( i >= pCombo->iItemCount ) {
		i = 0;
	}
	for ( int n = 0; n < pCombo->iItemCount; n++ ) {
		if ( __xgeXuiComboBoxItemEnabled(pCombo, i) ) {
			return i;
		}
		i += iStep;
		if ( i < 0 ) {
			i = pCombo->iItemCount - 1;
		}
		if ( i >= pCombo->iItemCount ) {
			i = 0;
		}
	}
	return -1;
}

static void __xgeXuiComboBoxSetState(xge_xui_combo_box pCombo, int iState)
{
	int iVisualState;

	if ( pCombo == NULL ) {
		return;
	}
	if ( (pCombo->pWidget == NULL) || ((pCombo->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pCombo->pContext != NULL && pCombo->pContext->pFocus == pCombo->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		iState |= XGE_XUI_STATE_ACTIVE;
	}
	if ( pCombo->iState != iState ) {
		pCombo->iState = iState;
		xgeXuiWidgetMarkPaint(pCombo->pWidget);
	}
	if ( pCombo->pWidget != NULL ) {
		iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED);
		xgeXuiWidgetSetVisualState(pCombo->pWidget, iVisualState);
	}
}

static float __xgeXuiComboBoxPreferredPopupHeight(xge_xui_combo_box pCombo)
{
	float fHeight;
	float fMax;

	if ( pCombo == NULL ) {
		return 0.0f;
	}
	if ( pCombo->fPopupHeight > 0.0f ) {
		return pCombo->fPopupHeight;
	}
	fHeight = (float)pCombo->iItemCount * pCombo->fItemHeight + 4.0f;
	if ( fHeight < pCombo->fItemHeight + 4.0f ) {
		fHeight = pCombo->fItemHeight + 4.0f;
	}
	fMax = (pCombo->fPopupMaxHeight > 0.0f) ? pCombo->fPopupMaxHeight : 168.0f;
	if ( fHeight > fMax ) {
		fHeight = fMax;
	}
	return fHeight;
}

static void __xgeXuiComboBoxLayoutPopup(xge_xui_combo_box pCombo)
{
	xge_rect_t tAnchor;
	float fHeight;
	int iDirection;

	if ( (pCombo == NULL) || (pCombo->pWidget == NULL) || (pCombo->pPopupWidget == NULL) || (pCombo->pListWidget == NULL) ) {
		return;
	}
	tAnchor = pCombo->pWidget->tBorderRect;
	if ( (tAnchor.fW <= 0.0f) || (tAnchor.fH <= 0.0f) ) {
		tAnchor = pCombo->pWidget->tRect;
	}
	fHeight = __xgeXuiComboBoxPreferredPopupHeight(pCombo);
	iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN;
	if ( pCombo->iPopupPlacement == XGE_XUI_COMBO_POPUP_TOP ) {
		iDirection = XGE_XUI_POPUP_DIRECTION_RIGHT_UP;
	}
	xgeXuiPopupSetAnchorRect(&pCombo->tPopup, tAnchor);
	xgeXuiPopupSetAnchorPoint(&pCombo->tPopup, (iDirection == XGE_XUI_POPUP_DIRECTION_RIGHT_UP) ? XGE_XUI_POPUP_ANCHOR_TOP_LEFT : XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT);
	xgeXuiPopupSetDirection(&pCombo->tPopup, iDirection);
	xgeXuiPopupSetGap(&pCombo->tPopup, 0.0f);
	xgeXuiWidgetSetRect(pCombo->pListWidget, (xge_rect_t){ 0.0f, 0.0f, tAnchor.fW, fHeight });
	xgeXuiPopupSetContentSize(&pCombo->tPopup, tAnchor.fW, fHeight);
	xgeXuiPopupSetScroll(&pCombo->tPopup, 0.0f, 0.0f);
}

static void __xgeXuiComboBoxSetHighlight(xge_xui_combo_box pCombo, int iIndex)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pCombo->iItemCount) || !__xgeXuiComboBoxItemEnabled(pCombo, iIndex) ) {
		iIndex = -1;
	}
	pCombo->iHighlight = iIndex;
	xgeXuiListViewSetSelected(&pCombo->tList, iIndex);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

static void __xgeXuiComboBoxSetOpen(xge_xui_combo_box pCombo, int bOpen)
{
	int iHighlight;

	if ( pCombo == NULL ) {
		return;
	}
	if ( bOpen ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
		iHighlight = __xgeXuiComboBoxItemEnabled(pCombo, pCombo->iSelected) ? pCombo->iSelected : __xgeXuiComboBoxFindEnabled(pCombo, 0, 1);
		xgeXuiPopupSetOpen(&pCombo->tPopup, 1);
		__xgeXuiComboBoxSetHighlight(pCombo, iHighlight);
		xgeXuiSetFocus(pCombo->pContext, pCombo->pListWidget);
	} else {
		xgeXuiPopupSetOpen(&pCombo->tPopup, 0);
		xgeXuiSetFocus(pCombo->pContext, pCombo->pWidget);
	}
	__xgeXuiComboBoxSetState(pCombo, pCombo->iState & XGE_XUI_STATE_HOVER);
}

static void __xgeXuiComboBoxPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_combo_box pCombo;

	(void)pWidget;
	pCombo = (xge_xui_combo_box)pUser;
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->iHighlight = pCombo->iSelected;
	__xgeXuiComboBoxSetState(pCombo, pCombo->iState & XGE_XUI_STATE_HOVER);
}

static void __xgeXuiComboBoxCommit(xge_xui_combo_box pCombo, int iIndex, int bNotify)
{
	int iOld;

	if ( pCombo == NULL || !__xgeXuiComboBoxItemEnabled(pCombo, iIndex) ) {
		return;
	}
	iOld = pCombo->iSelected;
	xgeXuiComboBoxSetSelected(pCombo, iIndex);
	__xgeXuiComboBoxSetOpen(pCombo, 0);
	if ( bNotify && (iOld != pCombo->iSelected) && pCombo->procSelect != NULL ) {
		pCombo->procSelect(pCombo->pWidget, pCombo->iSelected, pCombo->pUser);
	}
}

static void __xgeXuiComboBoxListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_combo_box pCombo;

	(void)pWidget;
	pCombo = (xge_xui_combo_box)pUser;
	__xgeXuiComboBoxCommit(pCombo, iIndex, 1);
}

static int __xgeXuiComboBoxItemProc(xge_xui_widget pWidget, int iIndex, xge_rect_t tRect, int iState, void* pUser)
{
	xge_xui_combo_box pCombo;
	xge_rect_t tText;
	xge_rect_t tSep;
	uint32_t iRow;
	uint32_t iText;

	(void)pWidget;
	pCombo = (xge_xui_combo_box)pUser;
	if ( pCombo == NULL ) {
		return 0;
	}
	tRect.fH -= 1.0f;
	if ( tRect.fH < 1.0f ) {
		tRect.fH = 1.0f;
	}
	if ( pCombo->arrItemData != NULL && pCombo->arrItemData[iIndex].bSeparator != 0 ) {
		tSep = tRect;
		tSep.fX += 8.0f;
		tSep.fW -= 16.0f;
		tSep.fY += tSep.fH * 0.5f;
		tSep.fH = 1.0f;
		__xgeXuiHostDrawRect(tSep, pCombo->iBorderColor);
		return 1;
	}
	if ( !__xgeXuiComboBoxItemEnabled(pCombo, iIndex) ) {
		iRow = pCombo->iItemDisabledColor;
		iText = pCombo->iDisabledTextColor;
	} else if ( (iState & XGE_XUI_LIST_ITEM_HOVER) != 0 ) {
		iRow = pCombo->iItemHoverColor;
		iText = pCombo->iTextColor;
	} else if ( (iState & XGE_XUI_LIST_ITEM_SELECTED) != 0 ) {
		iRow = pCombo->iItemSelectedColor;
		iText = pCombo->iTextColor;
	} else {
		iRow = pCombo->tList.iRowColor;
		iText = pCombo->iTextColor;
	}
	__xgeXuiHostDrawRect(tRect, iRow);
	if ( pCombo->pFont != NULL ) {
		tText = tRect;
		tText.fX += 8.0f;
		tText.fW -= 16.0f;
		__xgeXuiHostDrawTextRect(pCombo->pFont, __xgeXuiComboBoxItemText(pCombo, iIndex), tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	return 1;
}

int xgeXuiComboBoxInit(xge_xui_combo_box pCombo, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pCombo == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pCombo, 0, sizeof(*pCombo));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pCombo->pContext = pContext;
	pCombo->pWidget = pWidget;
	pCombo->pFont = pTheme->pFont;
	pCombo->iSelected = -1;
	pCombo->iHighlight = -1;
	pCombo->fItemHeight = 24.0f;
	pCombo->fPopupMaxHeight = 168.0f;
	pCombo->iPopupPlacement = XGE_XUI_COMBO_POPUP_AUTO;
	pCombo->iTextColor = pTheme->iTextColor;
	pCombo->iDisabledTextColor = XGE_COLOR_RGBA(142, 152, 166, 210);
	pCombo->iColorNormal = XGE_COLOR_RGBA(248, 252, 255, 255);
	pCombo->iColorHover = XGE_COLOR_RGBA(232, 244, 252, 255);
	pCombo->iColorFocus = pTheme->iStateFocus;
	pCombo->iColorDisabled = XGE_COLOR_RGBA(235, 240, 245, 255);
	pCombo->iBorderColor = XGE_COLOR_RGBA(136, 180, 216, 255);
	pCombo->iArrowColor = XGE_COLOR_RGBA(45, 92, 132, 255);
	pCombo->iPopupColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pCombo->iItemHoverColor = XGE_COLOR_RGBA(228, 242, 252, 255);
	pCombo->iItemSelectedColor = XGE_COLOR_RGBA(200, 226, 246, 255);
	pCombo->iItemDisabledColor = XGE_COLOR_RGBA(246, 248, 250, 255);
	xgeXuiWidgetSetBackground(pWidget, pCombo->iColorNormal);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pCombo->iBorderColor);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_HOVER, pCombo->iColorHover);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_DISABLED, pCombo->iColorDisabled);
	xgeXuiWidgetSetStateBorder(pWidget, XGE_XUI_STATE_FOCUS, 1.0f, pCombo->iColorFocus);
	xgeXuiWidgetSetStateBorder(pWidget, XGE_XUI_STATE_DISABLED, 1.0f, pCombo->iBorderColor);
	xgeXuiWidgetSetClip(pWidget, 0);
	pCombo->pPopupWidget = xgeXuiWidgetCreate();
	pCombo->pListWidget = xgeXuiWidgetCreate();
	if ( (pCombo->pPopupWidget == NULL) || (pCombo->pListWidget == NULL) ) {
		xgeXuiWidgetFree(pCombo->pPopupWidget);
		xgeXuiWidgetFree(pCombo->pListWidget);
		memset(pCombo, 0, sizeof(*pCombo));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiComboBoxEventProc, NULL);
	pWidget->procPaint = xgeXuiComboBoxPaintProc;
	pWidget->pUser = pCombo;
	xgeXuiPopupInit(&pCombo->tPopup, pContext, pCombo->pPopupWidget);
	xgeXuiPopupSetOwner(&pCombo->tPopup, pWidget);
	xgeXuiPopupSetFocusRestore(&pCombo->tPopup, pWidget);
	xgeXuiPopupSetPlacement(&pCombo->tPopup, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
	xgeXuiPopupSetContentWidget(&pCombo->tPopup, pCombo->pListWidget);
	xgeXuiPopupSetMatchOwnerWidth(&pCombo->tPopup, 1);
	xgeXuiPopupSetClose(&pCombo->tPopup, __xgeXuiComboBoxPopupClose, pCombo);
	xgeXuiPopupSetBackground(&pCombo->tPopup, pCombo->iPopupColor);
	xgeXuiPopupSetBorder(&pCombo->tPopup, pCombo->iBorderColor);
	xgeXuiListViewInit(&pCombo->tList, pContext, pCombo->pListWidget);
	pCombo->tList.bNotifyRepeatSelect = 1;
	xgeXuiWidgetSetClip(pCombo->pListWidget, 1);
	xgeXuiWidgetSetPaddingPx(pCombo->pListWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiListViewSetFont(&pCombo->tList, pCombo->pFont);
	xgeXuiListViewSetItemHeight(&pCombo->tList, pCombo->fItemHeight);
	xgeXuiListViewSetColors(&pCombo->tList, pCombo->iPopupColor, XGE_COLOR_RGBA(250, 252, 255, 255), pCombo->iItemSelectedColor, pCombo->iTextColor, XGE_COLOR_RGBA(218, 232, 244, 210), XGE_COLOR_RGBA(126, 166, 200, 230));
	pCombo->tList.iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeXuiListViewSetDisabledTextColor(&pCombo->tList, pCombo->iDisabledTextColor);
	xgeXuiListViewSetItemRenderer(&pCombo->tList, __xgeXuiComboBoxItemProc, pCombo);
	xgeXuiListViewSetSelect(&pCombo->tList, __xgeXuiComboBoxListSelect, pCombo);
	if ( xgeXuiOverlayAttach(pContext, pCombo->pPopupWidget, pWidget, XGE_XUI_LAYER_POPUP) != XGE_OK ) {
		xgeXuiListViewUnit(&pCombo->tList);
		xgeXuiPopupUnit(&pCombo->tPopup);
		xgeXuiWidgetFree(pCombo->pPopupWidget);
		memset(pCombo, 0, sizeof(*pCombo));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiComboBoxSetState(pCombo, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiComboBoxUnit(xge_xui_combo_box pCombo)
{
	xge_xui_widget pPopupWidget;

	if ( pCombo == NULL ) {
		return;
	}
	if ( pCombo->pWidget != NULL && pCombo->pWidget->pUser == pCombo ) {
		pCombo->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pCombo->pWidget, NULL, NULL);
		pCombo->pWidget->procPaint = NULL;
	}
	pPopupWidget = pCombo->pPopupWidget;
	xgeXuiListViewUnit(&pCombo->tList);
	xgeXuiPopupUnit(&pCombo->tPopup);
	xgeXuiWidgetFree(pPopupWidget);
	memset(pCombo, 0, sizeof(*pCombo));
}

void xgeXuiComboBoxSetItems(xge_xui_combo_box pCombo, const char** arrItems, int iCount)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pCombo->arrItems = arrItems;
	pCombo->arrItemData = NULL;
	pCombo->arrEnabled = NULL;
	pCombo->iItemCount = iCount;
	if ( pCombo->iSelected >= iCount || !__xgeXuiComboBoxItemEnabled(pCombo, pCombo->iSelected) ) {
		pCombo->iSelected = -1;
	}
	pCombo->iHighlight = pCombo->iSelected;
	xgeXuiListViewSetItems(&pCombo->tList, arrItems, iCount);
	xgeXuiListViewSetEnabledItems(&pCombo->tList, NULL, 0);
	xgeXuiListViewSetSelected(&pCombo->tList, pCombo->iSelected);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetItemData(xge_xui_combo_box pCombo, const xge_xui_combo_box_item_t* arrItems, int iCount)
{
	int i;

	if ( pCombo == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_PAGE_COMBO_BOX_ITEM_CAPACITY ) {
		iCount = XGE_XUI_PAGE_COMBO_BOX_ITEM_CAPACITY;
	}
	pCombo->arrItems = NULL;
	pCombo->arrItemData = arrItems;
	pCombo->arrEnabled = NULL;
	pCombo->iItemCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		pCombo->arrListItems[i] = __xgeXuiComboBoxItemText(pCombo, i);
	}
	for ( ; i < XGE_XUI_PAGE_COMBO_BOX_ITEM_CAPACITY; i++ ) {
		pCombo->arrListItems[i] = NULL;
	}
	if ( pCombo->iSelected >= iCount || !__xgeXuiComboBoxItemEnabled(pCombo, pCombo->iSelected) ) {
		pCombo->iSelected = -1;
	}
	pCombo->iHighlight = pCombo->iSelected;
	xgeXuiListViewSetItems(&pCombo->tList, pCombo->arrListItems, iCount);
	xgeXuiListViewSetEnabledItems(&pCombo->tList, NULL, 0);
	xgeXuiListViewSetSelected(&pCombo->tList, pCombo->iSelected);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetFont(xge_xui_combo_box pCombo, xui_font pFont)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->pFont = pFont;
	xgeXuiListViewSetFont(&pCombo->tList, pFont);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetSelect(xge_xui_combo_box pCombo, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->procSelect = procSelect;
	pCombo->pUser = pUser;
}

void xgeXuiComboBoxSetSelected(xge_xui_combo_box pCombo, int iIndex)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pCombo->iItemCount) || !__xgeXuiComboBoxItemEnabled(pCombo, iIndex) ) {
		iIndex = -1;
	}
	if ( pCombo->iSelected != iIndex ) {
		pCombo->iSelected = iIndex;
		pCombo->iHighlight = iIndex;
		pCombo->iChangeCount++;
		xgeXuiListViewSetSelected(&pCombo->tList, iIndex);
		xgeXuiWidgetMarkPaint(pCombo->pWidget);
	}
}

int xgeXuiComboBoxGetSelected(xge_xui_combo_box pCombo)
{
	if ( pCombo == NULL ) {
		return -1;
	}
	return pCombo->iSelected;
}

int xgeXuiComboBoxGetSelectedValue(xge_xui_combo_box pCombo)
{
	return __xgeXuiComboBoxItemValue(pCombo, (pCombo != NULL) ? pCombo->iSelected : -1);
}

void xgeXuiComboBoxSetSelectedValue(xge_xui_combo_box pCombo, int iValue)
{
	int i;

	if ( pCombo == NULL ) {
		return;
	}
	for ( i = 0; i < pCombo->iItemCount; i++ ) {
		if ( __xgeXuiComboBoxItemValue(pCombo, i) == iValue ) {
			xgeXuiComboBoxSetSelected(pCombo, i);
			return;
		}
	}
	xgeXuiComboBoxSetSelected(pCombo, -1);
}

void xgeXuiComboBoxSetEnabledItems(xge_xui_combo_box pCombo, const int* arrEnabled, int iCount)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->arrEnabled = arrEnabled;
	xgeXuiListViewSetEnabledItems(&pCombo->tList, arrEnabled, iCount);
	if ( !__xgeXuiComboBoxItemEnabled(pCombo, pCombo->iSelected) ) {
		xgeXuiComboBoxSetSelected(pCombo, -1);
	}
}

void xgeXuiComboBoxSetPopupHeight(xge_xui_combo_box pCombo, float fHeight)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pCombo->fPopupHeight = fHeight;
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
	}
}

void xgeXuiComboBoxSetPopupMaxHeight(xge_xui_combo_box pCombo, float fHeight)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pCombo->fPopupMaxHeight = fHeight;
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
	}
}

void xgeXuiComboBoxSetPopupPlacement(xge_xui_combo_box pCombo, int iPlacement)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( iPlacement != XGE_XUI_COMBO_POPUP_TOP && iPlacement != XGE_XUI_COMBO_POPUP_BOTTOM ) {
		iPlacement = XGE_XUI_COMBO_POPUP_AUTO;
	}
	pCombo->iPopupPlacement = iPlacement;
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
	}
}

void xgeXuiComboBoxSetMetrics(xge_xui_combo_box pCombo, float fItemHeight)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( fItemHeight < 16.0f ) {
		fItemHeight = 16.0f;
	}
	pCombo->fItemHeight = fItemHeight;
	xgeXuiListViewSetItemHeight(&pCombo->tList, fItemHeight);
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
	}
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetColors(xge_xui_combo_box pCombo, uint32_t iNormal, uint32_t iHover, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iPopup)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->iColorNormal = iNormal;
	pCombo->iColorHover = iHover;
	pCombo->iColorFocus = iFocus;
	pCombo->iColorDisabled = iDisabled;
	pCombo->iTextColor = iText;
	pCombo->iPopupColor = iPopup;
	pCombo->iArrowColor = iText;
	xgeXuiWidgetSetBackground(pCombo->pWidget, iNormal);
	xgeXuiWidgetSetBorder(pCombo->pWidget, 1.0f, pCombo->iBorderColor);
	xgeXuiWidgetSetStateBackground(pCombo->pWidget, XGE_XUI_STATE_HOVER, iHover);
	xgeXuiWidgetSetStateBackground(pCombo->pWidget, XGE_XUI_STATE_DISABLED, iDisabled);
	xgeXuiWidgetSetStateBorder(pCombo->pWidget, XGE_XUI_STATE_FOCUS, 1.0f, iFocus);
	xgeXuiWidgetSetStateBorder(pCombo->pWidget, XGE_XUI_STATE_DISABLED, 1.0f, pCombo->iBorderColor);
	xgeXuiPopupSetBackground(&pCombo->tPopup, iPopup);
	xgeXuiPopupSetBorder(&pCombo->tPopup, pCombo->iBorderColor);
	xgeXuiListViewSetColors(&pCombo->tList, iPopup, XGE_COLOR_RGBA(250, 252, 255, 255), pCombo->iItemSelectedColor, iText, XGE_COLOR_RGBA(218, 232, 244, 210), XGE_COLOR_RGBA(126, 166, 200, 230));
	pCombo->tList.iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetItemColors(xge_xui_combo_box pCombo, uint32_t iHover, uint32_t iSelected, uint32_t iDisabled, uint32_t iDisabledText)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->iItemHoverColor = iHover;
	pCombo->iItemSelectedColor = iSelected;
	pCombo->iItemDisabledColor = iDisabled;
	pCombo->iDisabledTextColor = iDisabledText;
	xgeXuiListViewSetDisabledTextColor(&pCombo->tList, iDisabledText);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

int xgeXuiComboBoxIsOpen(xge_xui_combo_box pCombo)
{
	return (pCombo != NULL) ? xgeXuiPopupIsOpen(&pCombo->tPopup) : 0;
}

int xgeXuiComboBoxGetState(xge_xui_combo_box pCombo)
{
	if ( pCombo == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiComboBoxSetState(pCombo, pCombo->iState & XGE_XUI_STATE_HOVER);
	return pCombo->iState;
}

int xgeXuiComboBoxEvent(xge_xui_combo_box pCombo, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int iTarget;

	if ( (pCombo == NULL) || (pCombo->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pCombo->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pCombo->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiComboBoxSetState(pCombo, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pCombo->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pCombo->iState & XGE_XUI_STATE_HOVER;
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiComboBoxSetState(pCombo, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiComboBoxSetState(pCombo, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pCombo->pContext, pCombo->pWidget);
			__xgeXuiComboBoxSetOpen(pCombo, xgeXuiPopupIsOpen(&pCombo->tPopup) == 0);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( pCombo->pContext == NULL || pCombo->pContext->pFocus != pCombo->pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) || (pEvent->iParam1 == XGE_KEY_DOWN) || (pEvent->iParam1 == XGE_KEY_UP) ) {
				__xgeXuiComboBoxSetOpen(pCombo, 1);
				if ( pEvent->iParam1 == XGE_KEY_UP ) {
					iTarget = __xgeXuiComboBoxNextEnabled(pCombo, (pCombo->iHighlight >= 0) ? pCombo->iHighlight : pCombo->iItemCount, -1);
					__xgeXuiComboBoxSetHighlight(pCombo, iTarget);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiComboBoxSetState(pCombo, pCombo->iState & XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiComboBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiComboBoxEvent((xge_xui_combo_box)pUser, pEvent);
}

void xgeXuiComboBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_combo_box pCombo;
	xge_rect_t tText;
	xge_rect_t tArrow;
	xge_rect_t tButton;
	const char* sText;
	uint32_t iText;
	float fInset;
	float fTextRight;

	pCombo = (xge_xui_combo_box)pUser;
	if ( (pWidget == NULL) || (pCombo == NULL) ) {
		return;
	}
	sText = (pCombo->iSelected >= 0) ? __xgeXuiComboBoxItemText(pCombo, pCombo->iSelected) : "";
	iText = ((pCombo->iState & XGE_XUI_STATE_DISABLED) != 0) ? pCombo->iDisabledTextColor : pCombo->iTextColor;
	tArrow.fW = 8.0f;
	tArrow.fH = 8.0f;
	fInset = (pWidget->tBorderRect.fH - tArrow.fH) * 0.5f;
	if ( fInset < 2.0f ) {
		fInset = 2.0f;
	}
	tButton = pWidget->tBorderRect;
	tButton.fX = tButton.fX + tButton.fW - tButton.fH;
	tButton.fW = tButton.fH;
	tArrow.fX = tButton.fX + (tButton.fW - tArrow.fW) * 0.5f;
	tArrow.fY = pWidget->tBorderRect.fY + fInset;
	tText = pWidget->tContentRect;
	fTextRight = tButton.fX;
	tText.fW = fTextRight - tText.fX;
	if ( tText.fW < 0.0f ) {
		tText.fW = 0.0f;
	}
	if ( pCombo->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pCombo->pFont, sText, tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	__xgeXuiBuiltinAssetDraw(tArrow, xgeXuiPopupIsOpen(&pCombo->tPopup) ? XGE_XUI_ASSET_CHEVRON_DOWN_8 : XGE_XUI_ASSET_CHEVRON_UP_8, ((pCombo->iState & XGE_XUI_STATE_DISABLED) != 0) ? pCombo->iDisabledTextColor : pCombo->iArrowColor);
}
