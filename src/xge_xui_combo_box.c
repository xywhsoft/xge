static void __xgeXuiComboBoxSetState(xge_xui_combo_box pCombo, int iState)
{
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
}

static uint32_t __xgeXuiComboBoxColor(xge_xui_combo_box pCombo)
{
	if ( pCombo == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pCombo->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pCombo->iColorDisabled;
	}
	if ( (pCombo->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pCombo->iColorActive;
	}
	if ( (pCombo->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pCombo->iColorHover;
	}
	return pCombo->iColorNormal;
}

static void __xgeXuiComboBoxLayoutPopup(xge_xui_combo_box pCombo)
{
	xge_rect_t tRect;
	float fHeight;

	if ( (pCombo == NULL) || (pCombo->pWidget == NULL) || (pCombo->pPopupWidget == NULL) || (pCombo->pListWidget == NULL) ) {
		return;
	}
	fHeight = pCombo->fDropDownHeight;
	if ( fHeight <= 0.0f ) {
		fHeight = (float)pCombo->iItemCount * pCombo->tList.tBase.fItemHeight;
		if ( fHeight > 160.0f ) {
			fHeight = 160.0f;
		}
		if ( fHeight < pCombo->tList.tBase.fItemHeight ) {
			fHeight = pCombo->tList.tBase.fItemHeight;
		}
		fHeight += 4.0f;
	}
	tRect.fX = pCombo->pWidget->tRect.fX;
	tRect.fY = pCombo->pWidget->tRect.fY + pCombo->pWidget->tRect.fH + 2.0f;
	tRect.fW = pCombo->pWidget->tRect.fW;
	tRect.fH = fHeight;
	xgeXuiWidgetSetRect(pCombo->pPopupWidget, tRect);
	xgeXuiPopupSetAnchorRect(&pCombo->tPopup, pCombo->pWidget->tRect);
	xgeXuiPopupSetOffset(&pCombo->tPopup, 0.0f, 2.0f);
	xgeXuiPopupApplyPlacement(&pCombo->tPopup);
	xgeXuiWidgetSetRect(pCombo->pListWidget, (xge_rect_t){ 2.0f, 2.0f, tRect.fW - 4.0f, tRect.fH - 4.0f });
}

static void __xgeXuiComboBoxSetOpen(xge_xui_combo_box pCombo, int bOpen)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( bOpen ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
		xgeXuiListViewSetSelected(&pCombo->tList, pCombo->iSelected);
		xgeXuiPopupSetOpen(&pCombo->tPopup, 1);
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
	__xgeXuiComboBoxSetState(pCombo, pCombo->iState & XGE_XUI_STATE_HOVER);
}

static void __xgeXuiComboBoxListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_combo_box pCombo;

	(void)pWidget;
	pCombo = (xge_xui_combo_box)pUser;
	if ( pCombo == NULL ) {
		return;
	}
	xgeXuiComboBoxSetSelected(pCombo, iIndex);
	__xgeXuiComboBoxSetOpen(pCombo, 0);
	if ( pCombo->procSelect != NULL ) {
		pCombo->procSelect(pCombo->pWidget, pCombo->iSelected, pCombo->pUser);
	}
}

static int __xgeXuiComboBoxItemProc(xge_xui_widget pWidget, int iIndex, xge_rect_t tRect, int iState, void* pUser)
{
	xge_xui_combo_box pCombo;
	xge_rect_t tText;
	uint32_t iRow;

	(void)pWidget;
	pCombo = (xge_xui_combo_box)pUser;
	if ( pCombo == NULL ) {
		return 0;
	}
	tRect.fH -= 1.0f;
	if ( tRect.fH < 1.0f ) {
		tRect.fH = 1.0f;
	}
	if ( (iState & XGE_XUI_LIST_ITEM_HOVER) != 0 ) {
		iRow = XGE_COLOR_RGBA(255, 246, 194, 255);
	} else if ( (iState & XGE_XUI_LIST_ITEM_SELECTED) != 0 ) {
		iRow = pCombo->tList.iSelectedColor;
	} else {
		iRow = pCombo->tList.iRowColor;
	}
	__xgeXuiHostDrawRect(tRect, iRow);
	if ( (pCombo->pFont != NULL) && (pCombo->arrItems != NULL) && (iIndex >= 0) && (iIndex < pCombo->iItemCount) && (pCombo->arrItems[iIndex] != NULL) ) {
		tText = tRect;
		tText.fX += 7.0f;
		tText.fW -= 14.0f;
		__xgeXuiHostDrawTextRect(pCombo->pFont, pCombo->arrItems[iIndex], tText, pCombo->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
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
	pCombo->fDropDownHeight = 0.0f;
	pCombo->iTextColor = pTheme->iTextColor;
	pCombo->iColorNormal = pTheme->iStateNormal;
	pCombo->iColorHover = pTheme->iStateHover;
	pCombo->iColorActive = pTheme->iStateActive;
	pCombo->iColorFocus = pTheme->iStateFocus;
	pCombo->iColorDisabled = pTheme->iStateDisabled;
	pCombo->iPopupColor = pTheme->iPanelColor;
	pCombo->pPopupWidget = xgeXuiWidgetCreate();
	pCombo->pListWidget = xgeXuiWidgetCreate();
	if ( (pCombo->pPopupWidget == NULL) || (pCombo->pListWidget == NULL) ) {
		xgeXuiWidgetFree(pCombo->pPopupWidget);
		xgeXuiWidgetFree(pCombo->pListWidget);
		memset(pCombo, 0, sizeof(*pCombo));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pWidget->procEvent = xgeXuiComboBoxEventProc;
	pWidget->procPaint = xgeXuiComboBoxPaintProc;
	pWidget->pUser = pCombo;
	xgeXuiPopupInit(&pCombo->tPopup, pContext, pCombo->pPopupWidget);
	xgeXuiPopupSetOwner(&pCombo->tPopup, pWidget);
	xgeXuiPopupSetFocusRestore(&pCombo->tPopup, pWidget);
	xgeXuiPopupSetPlacement(&pCombo->tPopup, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
	xgeXuiPopupSetClose(&pCombo->tPopup, __xgeXuiComboBoxPopupClose, pCombo);
	xgeXuiPopupSetBackground(&pCombo->tPopup, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiPopupSetBorder(&pCombo->tPopup, XGE_COLOR_RGBA(184, 223, 245, 255));
	xgeXuiListViewInit(&pCombo->tList, pContext, pCombo->pListWidget);
	xgeXuiWidgetSetPaddingPx(pCombo->pListWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiListViewSetFont(&pCombo->tList, pCombo->pFont);
	xgeXuiListViewSetColors(&pCombo->tList,
		pTheme->iPanelColor,
		XGE_COLOR_RGBA(248, 250, 253, 255),
		XGE_COLOR_RGBA(190, 219, 242, 255),
		pTheme->iTextColor,
		XGE_COLOR_RGBA(218, 232, 244, 210),
		XGE_COLOR_RGBA(126, 166, 200, 230));
	pCombo->tList.iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeXuiListViewSetDisabledTextColor(&pCombo->tList, XGE_COLOR_RGBA(142, 152, 166, 190));
	xgeXuiListViewSetItemRenderer(&pCombo->tList, __xgeXuiComboBoxItemProc, pCombo);
	xgeXuiListViewSetSelect(&pCombo->tList, __xgeXuiComboBoxListSelect, pCombo);
	xgeXuiWidgetAddInternal(pCombo->pPopupWidget, pCombo->pListWidget);
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
		pCombo->pWidget->procEvent = NULL;
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
	pCombo->iItemCount = iCount;
	if ( pCombo->iSelected >= iCount ) {
		pCombo->iSelected = -1;
	}
	xgeXuiListViewSetItems(&pCombo->tList, arrItems, iCount);
	xgeXuiListViewSetSelected(&pCombo->tList, pCombo->iSelected);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

void xgeXuiComboBoxSetFont(xge_xui_combo_box pCombo, xge_font pFont)
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
	if ( (iIndex < 0) || (iIndex >= pCombo->iItemCount) ) {
		iIndex = -1;
	}
	if ( pCombo->iSelected != iIndex ) {
		pCombo->iSelected = iIndex;
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

void xgeXuiComboBoxSetDropDownHeight(xge_xui_combo_box pCombo, float fHeight)
{
	if ( pCombo == NULL ) {
		return;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pCombo->fDropDownHeight = fHeight;
	if ( xgeXuiPopupIsOpen(&pCombo->tPopup) ) {
		__xgeXuiComboBoxLayoutPopup(pCombo);
	}
}

void xgeXuiComboBoxSetColors(xge_xui_combo_box pCombo, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iPopup)
{
	if ( pCombo == NULL ) {
		return;
	}
	pCombo->iColorNormal = iNormal;
	pCombo->iColorHover = iHover;
	pCombo->iColorActive = iActive;
	pCombo->iColorFocus = iFocus;
	pCombo->iColorDisabled = iDisabled;
	pCombo->iTextColor = iText;
	pCombo->iPopupColor = iPopup;
	xgeXuiPopupSetBackground(&pCombo->tPopup, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiPopupSetBorder(&pCombo->tPopup, XGE_COLOR_RGBA(184, 223, 245, 255));
	xgeXuiListViewSetColors(&pCombo->tList, iPopup, iNormal, iActive, iText, XGE_COLOR_RGBA(218, 232, 244, 210), XGE_COLOR_RGBA(126, 166, 200, 230));
	pCombo->tList.iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeXuiWidgetMarkPaint(pCombo->pWidget);
}

int xgeXuiComboBoxIsOpen(xge_xui_combo_box pCombo)
{
	if ( pCombo == NULL ) {
		return 0;
	}
	return xgeXuiPopupIsOpen(&pCombo->tPopup);
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
			if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
				__xgeXuiComboBoxSetOpen(pCombo, 1);
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
	static const uint16_t arrChevronUp8[8] = {
		0x00, 0x00, 0x42, 0x66, 0x3c, 0x18, 0x00, 0x00
	};
	static const uint16_t arrChevronDown8[8] = {
		0x00, 0x00, 0x18, 0x3c, 0x66, 0x42, 0x00, 0x00
	};
	xge_xui_combo_box pCombo;
	xge_rect_t tText;
	xge_rect_t tArrow;
	const char* sText;

	pCombo = (xge_xui_combo_box)pUser;
	if ( (pWidget == NULL) || (pCombo == NULL) ) {
		return;
	}
	if ( (pCombo->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pCombo->iColorFocus) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pCombo->iColorFocus);
	}
	if ( XGE_COLOR_GET_A(__xgeXuiComboBoxColor(pCombo)) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tContentRect, __xgeXuiComboBoxColor(pCombo));
		__xgeXuiHostDrawBorderRect(pWidget->tContentRect, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
	}
	sText = "";
	if ( (pCombo->arrItems != NULL) && (pCombo->iSelected >= 0) && (pCombo->iSelected < pCombo->iItemCount) && (pCombo->arrItems[pCombo->iSelected] != NULL) ) {
		sText = pCombo->arrItems[pCombo->iSelected];
	}
	tText = pWidget->tContentRect;
	tText.fX += 8.0f;
	tText.fW -= 28.0f;
	if ( pCombo->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pCombo->pFont, sText, tText, pCombo->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	tArrow = pWidget->tContentRect;
	tArrow.fW = 8.0f;
	tArrow.fH = 8.0f;
	tArrow.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 15.0f;
	tArrow.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - tArrow.fH) * 0.5f;
	__xgeXuiHostDrawBitmapMask(tArrow, xgeXuiPopupIsOpen(&pCombo->tPopup) ? arrChevronDown8 : arrChevronUp8, 8, 8, pCombo->iTextColor);
}
