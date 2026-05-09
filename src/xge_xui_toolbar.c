static void __xgeXuiToolbarSetState(xge_xui_toolbar pToolbar, int iState)
{
	if ( pToolbar == NULL ) {
		return;
	}
	if ( (pToolbar->pWidget == NULL) || ((pToolbar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pToolbar->pContext != NULL && pToolbar->pContext->pFocus == pToolbar->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pToolbar->iState != iState ) {
		pToolbar->iState = iState;
		xgeXuiWidgetMarkPaint(pToolbar->pWidget);
	}
}

static int __xgeXuiToolbarItemInteractive(xge_xui_toolbar pToolbar, int iIndex)
{
	xge_xui_toolbar_item_t* pItem;

	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return 0;
	}
	pItem = &pToolbar->arrItems[iIndex];
	return (pItem->bEnabled != 0) && (pItem->iType != XGE_XUI_TOOLBAR_ITEM_SEPARATOR);
}

static void __xgeXuiToolbarLayout(xge_xui_toolbar pToolbar)
{
	xge_rect_t tContent;
	xge_rect_t tItem;
	float fLimit;
	float fCursor;
	float fSize;
	int iPrevGroup;
	int i;

	if ( (pToolbar == NULL) || (pToolbar->pWidget == NULL) ) {
		return;
	}
	tContent = pToolbar->pWidget->tContentRect;
	memset(&pToolbar->tOverflowRect, 0, sizeof(pToolbar->tOverflowRect));
	pToolbar->iOverflowFirst = -1;
	pToolbar->iOverflowCount = 0;
	for ( i = 0; i < pToolbar->iItemCount; i++ ) {
		memset(&pToolbar->arrItems[i].tRect, 0, sizeof(pToolbar->arrItems[i].tRect));
	}
	if ( pToolbar->bOverflowEnabled != 0 ) {
		pToolbar->tOverflowRect = tContent;
		if ( pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
			pToolbar->tOverflowRect.fY = tContent.fY + tContent.fH - pToolbar->fOverflowSize;
			pToolbar->tOverflowRect.fH = pToolbar->fOverflowSize;
			if ( pToolbar->tOverflowRect.fH < 1.0f ) {
				pToolbar->tOverflowRect.fH = 1.0f;
			}
		} else {
			pToolbar->tOverflowRect.fX = tContent.fX + tContent.fW - pToolbar->fOverflowSize;
			pToolbar->tOverflowRect.fW = pToolbar->fOverflowSize;
			if ( pToolbar->tOverflowRect.fW < 1.0f ) {
				pToolbar->tOverflowRect.fW = 1.0f;
			}
		}
	}
	fLimit = (pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? (tContent.fY + tContent.fH) : (tContent.fX + tContent.fW);
	if ( pToolbar->bOverflowEnabled != 0 ) {
		fLimit -= pToolbar->fOverflowSize;
	}
	fCursor = (pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? tContent.fY : tContent.fX;
	iPrevGroup = 0;
	for ( i = 0; i < pToolbar->iItemCount; i++ ) {
		if ( (i > 0) && (pToolbar->arrItems[i].iType != XGE_XUI_TOOLBAR_ITEM_SEPARATOR) && (pToolbar->arrItems[i].iGroup != iPrevGroup) ) {
			fCursor += pToolbar->fGroupGap;
		}
		fSize = (pToolbar->arrItems[i].iType == XGE_XUI_TOOLBAR_ITEM_SEPARATOR) ? pToolbar->fSeparatorSize : ((pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? pToolbar->fItemHeight : pToolbar->fItemWidth);
		if ( fSize < 1.0f ) {
			fSize = 1.0f;
		}
		if ( (pToolbar->bOverflowEnabled != 0) && ((fCursor + fSize) > fLimit) ) {
			pToolbar->iOverflowFirst = i;
			pToolbar->iOverflowCount = pToolbar->iItemCount - i;
			break;
		}
		if ( pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
			tItem.fX = tContent.fX;
			tItem.fY = fCursor;
			tItem.fW = tContent.fW;
			tItem.fH = fSize;
			if ( (tItem.fY + tItem.fH) > (tContent.fY + tContent.fH) ) {
				tItem.fH = tContent.fY + tContent.fH - tItem.fY;
			}
		} else {
			tItem.fX = fCursor;
			tItem.fY = tContent.fY;
			tItem.fW = fSize;
			tItem.fH = tContent.fH;
			if ( (tItem.fX + tItem.fW) > (tContent.fX + tContent.fW) ) {
				tItem.fW = tContent.fX + tContent.fW - tItem.fX;
			}
		}
		if ( (tItem.fW < 0.0f) || (tItem.fH < 0.0f) ) {
			tItem.fW = 0.0f;
			tItem.fH = 0.0f;
		}
		pToolbar->arrItems[i].tRect = tItem;
		fCursor += fSize;
		if ( pToolbar->arrItems[i].iType != XGE_XUI_TOOLBAR_ITEM_SEPARATOR ) {
			iPrevGroup = pToolbar->arrItems[i].iGroup;
		}
	}
	if ( pToolbar->iOverflowCount == 0 ) {
		memset(&pToolbar->tOverflowRect, 0, sizeof(pToolbar->tOverflowRect));
	}
}

static int __xgeXuiToolbarIndexAt(xge_xui_toolbar pToolbar, float fX, float fY)
{
	int i;

	if ( pToolbar == NULL ) {
		return -1;
	}
	__xgeXuiToolbarLayout(pToolbar);
	for ( i = 0; i < pToolbar->iItemCount; i++ ) {
		if ( __xgeXuiToolbarItemInteractive(pToolbar, i) && __xgeXuiRectContains(pToolbar->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiToolbarFirstEnabled(xge_xui_toolbar pToolbar)
{
	int i;

	if ( pToolbar == NULL ) {
		return -1;
	}
	for ( i = 0; i < pToolbar->iItemCount; i++ ) {
		if ( __xgeXuiToolbarItemInteractive(pToolbar, i) ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiToolbarSelect(xge_xui_toolbar pToolbar, int iIndex)
{
	xge_xui_toolbar_item_t* pItem;

	if ( !__xgeXuiToolbarItemInteractive(pToolbar, iIndex) ) {
		return;
	}
	pItem = &pToolbar->arrItems[iIndex];
	if ( pItem->iType == XGE_XUI_TOOLBAR_ITEM_TOGGLE ) {
		pItem->bChecked = (pItem->bChecked == 0);
	}
	pToolbar->iSelectCount++;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
	if ( pToolbar->procSelect != NULL ) {
		pToolbar->procSelect(pToolbar->pWidget, iIndex, pToolbar->pUser);
	}
}

static int __xgeXuiToolbarTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	xge_xui_toolbar pToolbar;
	const char* sText;
	int iIndex;

	(void)pWidget;
	pToolbar = (xge_xui_toolbar)pUser;
	if ( (pToolbar == NULL) || (pDesc == NULL) ) {
		return 0;
	}
	if ( pContext != NULL ) {
		iIndex = __xgeXuiToolbarIndexAt(pToolbar, pContext->fTooltipMouseX, pContext->fTooltipMouseY);
	} else {
		iIndex = pToolbar->iHover;
	}
	if ( !__xgeXuiToolbarItemInteractive(pToolbar, iIndex) ) {
		return 0;
	}
	sText = pToolbar->arrItems[iIndex].sTooltip;
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return 0;
	}
	pDesc->iType = XGE_XUI_TOOLTIP_TEXT;
	pDesc->sText = sText;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 12.0f;
	pDesc->fOffsetY = 16.0f;
	pDesc->bFollowCursor = 1;
	return 1;
}

int xgeXuiToolbarInit(xge_xui_toolbar pToolbar, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pToolbar == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pToolbar, 0, sizeof(*pToolbar));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pToolbar->pContext = pContext;
	pToolbar->pWidget = pWidget;
	pToolbar->pFont = pTheme->pFont;
	pToolbar->iOrientation = XGE_XUI_SEPARATOR_HORIZONTAL;
	pToolbar->iHover = -1;
	pToolbar->iActive = -1;
	pToolbar->iOverflowFirst = -1;
	pToolbar->fItemWidth = 64.0f;
	pToolbar->fItemHeight = 26.0f;
	pToolbar->fSeparatorSize = 8.0f;
	pToolbar->fGroupGap = 6.0f;
	pToolbar->fOverflowSize = 24.0f;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(232, 246, 255, 255));
	pToolbar->iItemColor = XGE_COLOR_RGBA(247, 252, 255, 255);
	pToolbar->iHoverColor = pTheme->iStateHover;
	pToolbar->iActiveColor = XGE_COLOR_RGBA(190, 231, 252, 255);
	pToolbar->iCheckedColor = XGE_COLOR_RGBA(47, 145, 215, 255);
	pToolbar->iFocusColor = pTheme->iStateFocus;
	pToolbar->iDisabledColor = pTheme->iStateDisabled;
	pToolbar->iSeparatorColor = XGE_COLOR_RGBA(127, 196, 229, 220);
	pToolbar->iTextColor = pTheme->iTextColor;
	pToolbar->iDisabledTextColor = XGE_COLOR_RGBA(118, 132, 148, 255);
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiToolbarEventProc, NULL);
	pWidget->procPaint = xgeXuiToolbarPaintProc;
	pWidget->pUser = pToolbar;
	xgeXuiWidgetSetTooltipResolver(pWidget, __xgeXuiToolbarTooltipResolve, pToolbar);
	__xgeXuiToolbarSetState(pToolbar, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiToolbarUnit(xge_xui_toolbar pToolbar)
{
	if ( pToolbar == NULL ) {
		return;
	}
	if ( pToolbar->pWidget != NULL && pToolbar->pWidget->pUser == pToolbar ) {
		pToolbar->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pToolbar->pWidget, NULL, NULL);
		pToolbar->pWidget->procPaint = NULL;
		xgeXuiWidgetClearTooltip(pToolbar->pWidget);
	}
	memset(pToolbar, 0, sizeof(*pToolbar));
}

void xgeXuiToolbarSetItems(xge_xui_toolbar pToolbar, const char** arrText, const int* arrTypes, int iCount)
{
	int i;
	int iType;

	if ( pToolbar == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_TOOLBAR_CAPACITY ) {
		iCount = XGE_XUI_TOOLBAR_CAPACITY;
	}
	memset(pToolbar->arrItems, 0, sizeof(pToolbar->arrItems));
	for ( i = 0; i < iCount; i++ ) {
		iType = (arrTypes != NULL) ? arrTypes[i] : XGE_XUI_TOOLBAR_ITEM_BUTTON;
		if ( (iType != XGE_XUI_TOOLBAR_ITEM_TOGGLE) && (iType != XGE_XUI_TOOLBAR_ITEM_SEPARATOR) ) {
			iType = XGE_XUI_TOOLBAR_ITEM_BUTTON;
		}
		pToolbar->arrItems[i].sText = (arrText != NULL && arrText[i] != NULL) ? arrText[i] : "";
		pToolbar->arrItems[i].sTooltip = "";
		pToolbar->arrItems[i].iType = iType;
		pToolbar->arrItems[i].bEnabled = 1;
		pToolbar->arrItems[i].bChecked = 0;
		pToolbar->arrItems[i].iGroup = 0;
	}
	pToolbar->iItemCount = iCount;
	pToolbar->iHover = -1;
	pToolbar->iActive = -1;
	pToolbar->iOverflowFirst = -1;
	pToolbar->iOverflowCount = 0;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetFont(xge_xui_toolbar pToolbar, xge_font pFont)
{
	if ( pToolbar == NULL ) {
		return;
	}
	pToolbar->pFont = pFont;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetSelect(xge_xui_toolbar pToolbar, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pToolbar == NULL ) {
		return;
	}
	pToolbar->procSelect = procSelect;
	pToolbar->pUser = pUser;
}

void xgeXuiToolbarSetOrientation(xge_xui_toolbar pToolbar, int iOrientation)
{
	if ( pToolbar == NULL ) {
		return;
	}
	pToolbar->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_VERTICAL) ? XGE_XUI_SEPARATOR_VERTICAL : XGE_XUI_SEPARATOR_HORIZONTAL;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetItemSize(xge_xui_toolbar pToolbar, float fWidth, float fHeight, float fSeparatorSize)
{
	if ( pToolbar == NULL ) {
		return;
	}
	if ( fWidth < 1.0f ) {
		fWidth = 1.0f;
	}
	if ( fHeight < 1.0f ) {
		fHeight = 1.0f;
	}
	if ( fSeparatorSize < 1.0f ) {
		fSeparatorSize = 1.0f;
	}
	pToolbar->fItemWidth = fWidth;
	pToolbar->fItemHeight = fHeight;
	pToolbar->fSeparatorSize = fSeparatorSize;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetGroupGap(xge_xui_toolbar pToolbar, float fGap)
{
	if ( pToolbar == NULL ) {
		return;
	}
	if ( fGap < 0.0f ) {
		fGap = 0.0f;
	}
	pToolbar->fGroupGap = fGap;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetItemGroup(xge_xui_toolbar pToolbar, int iIndex, int iGroup)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return;
	}
	if ( iGroup < 0 ) {
		iGroup = 0;
	}
	pToolbar->arrItems[iIndex].iGroup = iGroup;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

int xgeXuiToolbarGetItemGroup(xge_xui_toolbar pToolbar, int iIndex)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return 0;
	}
	return pToolbar->arrItems[iIndex].iGroup;
}

void xgeXuiToolbarSetItemTooltip(xge_xui_toolbar pToolbar, int iIndex, const char* sText)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return;
	}
	pToolbar->arrItems[iIndex].sTooltip = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

const char* xgeXuiToolbarGetItemTooltip(xge_xui_toolbar pToolbar, int iIndex)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return "";
	}
	return (pToolbar->arrItems[iIndex].sTooltip != NULL) ? pToolbar->arrItems[iIndex].sTooltip : "";
}

const char* xgeXuiToolbarGetHoverTooltip(xge_xui_toolbar pToolbar)
{
	if ( (pToolbar == NULL) || !__xgeXuiToolbarItemInteractive(pToolbar, pToolbar->iHover) ) {
		return "";
	}
	return xgeXuiToolbarGetItemTooltip(pToolbar, pToolbar->iHover);
}

void xgeXuiToolbarSetOverflow(xge_xui_toolbar pToolbar, int bEnabled, float fButtonSize, xge_xui_click_proc procOverflow, void* pUser)
{
	if ( pToolbar == NULL ) {
		return;
	}
	if ( fButtonSize < 1.0f ) {
		fButtonSize = 1.0f;
	}
	pToolbar->bOverflowEnabled = (bEnabled != 0);
	pToolbar->fOverflowSize = fButtonSize;
	pToolbar->procOverflow = procOverflow;
	pToolbar->pOverflowUser = pUser;
	if ( pToolbar->bOverflowEnabled == 0 ) {
		pToolbar->iOverflowFirst = -1;
		pToolbar->iOverflowCount = 0;
		memset(&pToolbar->tOverflowRect, 0, sizeof(pToolbar->tOverflowRect));
	}
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

int xgeXuiToolbarGetOverflowFirst(xge_xui_toolbar pToolbar)
{
	if ( pToolbar == NULL ) {
		return -1;
	}
	__xgeXuiToolbarLayout(pToolbar);
	return pToolbar->iOverflowFirst;
}

int xgeXuiToolbarGetOverflowCount(xge_xui_toolbar pToolbar)
{
	if ( pToolbar == NULL ) {
		return 0;
	}
	__xgeXuiToolbarLayout(pToolbar);
	return pToolbar->iOverflowCount;
}

xge_rect_t xgeXuiToolbarGetOverflowRect(xge_xui_toolbar pToolbar)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pToolbar == NULL ) {
		return tRect;
	}
	__xgeXuiToolbarLayout(pToolbar);
	return pToolbar->tOverflowRect;
}

void xgeXuiToolbarSetItemEnabled(xge_xui_toolbar pToolbar, int iIndex, int bEnabled)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return;
	}
	pToolbar->arrItems[iIndex].bEnabled = (bEnabled != 0);
	if ( (pToolbar->iHover == iIndex) && (bEnabled == 0) ) {
		pToolbar->iHover = -1;
	}
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

void xgeXuiToolbarSetItemChecked(xge_xui_toolbar pToolbar, int iIndex, int bChecked)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return;
	}
	pToolbar->arrItems[iIndex].bChecked = (bChecked != 0);
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

int xgeXuiToolbarGetItemChecked(xge_xui_toolbar pToolbar, int iIndex)
{
	if ( (pToolbar == NULL) || (iIndex < 0) || (iIndex >= pToolbar->iItemCount) ) {
		return 0;
	}
	return pToolbar->arrItems[iIndex].bChecked;
}

void xgeXuiToolbarSetColors(xge_xui_toolbar pToolbar, uint32_t iBackground, uint32_t iItem, uint32_t iHover, uint32_t iActive, uint32_t iChecked, uint32_t iFocus, uint32_t iDisabled, uint32_t iSeparator, uint32_t iText, uint32_t iDisabledText)
{
	if ( pToolbar == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pToolbar->pWidget, iBackground);
	pToolbar->iItemColor = iItem;
	pToolbar->iHoverColor = iHover;
	pToolbar->iActiveColor = iActive;
	pToolbar->iCheckedColor = iChecked;
	pToolbar->iFocusColor = iFocus;
	pToolbar->iDisabledColor = iDisabled;
	pToolbar->iSeparatorColor = iSeparator;
	pToolbar->iTextColor = iText;
	pToolbar->iDisabledTextColor = iDisabledText;
	xgeXuiWidgetMarkPaint(pToolbar->pWidget);
}

int xgeXuiToolbarGetState(xge_xui_toolbar pToolbar)
{
	if ( pToolbar == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiToolbarSetState(pToolbar, pToolbar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pToolbar->iState;
}

int xgeXuiToolbarEvent(xge_xui_toolbar pToolbar, const xge_event_t* pEvent)
{
	int iIndex;
	int iState;

	if ( (pToolbar == NULL) || (pToolbar->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pToolbar->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pToolbar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiToolbarSetState(pToolbar, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = __xgeXuiToolbarIndexAt(pToolbar, pEvent->fX, pEvent->fY);
	iState = pToolbar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pToolbar->bOverflowActive = 0;
			pToolbar->iHover = iIndex;
			__xgeXuiToolbarSetState(pToolbar, (iIndex >= 0) ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pToolbar->iHover = -1;
			pToolbar->bOverflowActive = 0;
			__xgeXuiToolbarSetState(pToolbar, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( (pToolbar->iOverflowCount > 0) && __xgeXuiRectContains(pToolbar->tOverflowRect, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pToolbar->pContext, pToolbar->pWidget);
				xgeXuiSetPointerCapture(pToolbar->pContext, pEvent->iPointerId, pToolbar->pWidget);
				pToolbar->iHover = -1;
				pToolbar->iActive = -1;
				pToolbar->bOverflowActive = 1;
				__xgeXuiToolbarSetState(pToolbar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( iIndex < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pToolbar->pContext, pToolbar->pWidget);
			xgeXuiSetPointerCapture(pToolbar->pContext, pEvent->iPointerId, pToolbar->pWidget);
			pToolbar->iHover = iIndex;
			pToolbar->iActive = iIndex;
			__xgeXuiToolbarSetState(pToolbar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( (pToolbar->iState & XGE_XUI_STATE_ACTIVE) == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pToolbar->pContext != NULL && xgeXuiGetPointerCapture(pToolbar->pContext, pEvent->iPointerId) == pToolbar->pWidget ) {
				xgeXuiSetPointerCapture(pToolbar->pContext, pEvent->iPointerId, NULL);
			}
			if ( pToolbar->bOverflowActive != 0 ) {
				if ( (pToolbar->iOverflowCount > 0) && __xgeXuiRectContains(pToolbar->tOverflowRect, pEvent->fX, pEvent->fY) && (pToolbar->procOverflow != NULL) ) {
					pToolbar->procOverflow(pToolbar->pWidget, pToolbar->pOverflowUser);
				}
			} else if ( iIndex == pToolbar->iActive ) {
				__xgeXuiToolbarSelect(pToolbar, iIndex);
			}
			pToolbar->iHover = iIndex;
			pToolbar->iActive = -1;
			pToolbar->bOverflowActive = 0;
			__xgeXuiToolbarSetState(pToolbar, (iIndex >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pToolbar->iHover = -1;
			pToolbar->iActive = -1;
			pToolbar->bOverflowActive = 0;
			__xgeXuiToolbarSetState(pToolbar, XGE_XUI_STATE_NORMAL);
			if ( pToolbar->pContext != NULL && xgeXuiGetPointerCapture(pToolbar->pContext, pEvent->iPointerId) == pToolbar->pWidget ) {
				xgeXuiSetPointerCapture(pToolbar->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pToolbar->pContext == NULL) || (pToolbar->pContext->pFocus != pToolbar->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iIndex = __xgeXuiToolbarItemInteractive(pToolbar, pToolbar->iHover) ? pToolbar->iHover : __xgeXuiToolbarFirstEnabled(pToolbar);
			if ( iIndex < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiToolbarSelect(pToolbar, iIndex);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiToolbarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiToolbarEvent((xge_xui_toolbar)pUser, pEvent);
}

void xgeXuiToolbarPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_toolbar pToolbar;
	xge_xui_toolbar_item_t* pItem;
	xge_rect_t tRect;
	xge_rect_t tLine;
	xge_rect_t tText;
	xge_rect_t tDot;
	uint32_t iColor;
	uint32_t iTextColor;
	int i;

	pToolbar = (xge_xui_toolbar)pUser;
	if ( (pWidget == NULL) || (pToolbar == NULL) ) {
		return;
	}
	__xgeXuiToolbarLayout(pToolbar);
	for ( i = 0; i < pToolbar->iItemCount; i++ ) {
		pItem = &pToolbar->arrItems[i];
		tRect = pItem->tRect;
		if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
			continue;
		}
		if ( pItem->iType == XGE_XUI_TOOLBAR_ITEM_SEPARATOR ) {
			tLine = tRect;
			if ( pToolbar->iOrientation == XGE_XUI_SEPARATOR_VERTICAL ) {
				tLine.fY += (tLine.fH - 1.0f) * 0.5f;
				tLine.fH = 1.0f;
				tLine.fX += 6.0f;
				tLine.fW -= 12.0f;
			} else {
				tLine.fX += (tLine.fW - 1.0f) * 0.5f;
				tLine.fW = 1.0f;
				tLine.fY += 5.0f;
				tLine.fH -= 10.0f;
			}
			if ( (tLine.fW > 0.0f) && (tLine.fH > 0.0f) && XGE_COLOR_GET_A(pToolbar->iSeparatorColor) != 0 ) {
				__xgeXuiHostDrawRect(tLine, pToolbar->iSeparatorColor);
			}
			continue;
		}
		iColor = pToolbar->iItemColor;
		iTextColor = pToolbar->iTextColor;
		if ( (pToolbar->iState & XGE_XUI_STATE_DISABLED) != 0 || pItem->bEnabled == 0 ) {
			iColor = pToolbar->iDisabledColor;
			iTextColor = pToolbar->iDisabledTextColor;
		} else if ( i == pToolbar->iActive ) {
			iColor = pToolbar->iActiveColor;
		} else if ( pItem->bChecked != 0 ) {
			iColor = pToolbar->iCheckedColor;
			iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
		} else if ( i == pToolbar->iHover ) {
			iColor = pToolbar->iHoverColor;
		}
		if ( XGE_COLOR_GET_A(iColor) != 0 ) {
			__xgeXuiHostDrawRect(tRect, iColor);
		}
		__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
		if ( (pToolbar->pFont != NULL) && (pItem->sText != NULL) && (pItem->sText[0] != 0) ) {
			tText = tRect;
			tText.fX += 5.0f;
			tText.fW -= 10.0f;
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pToolbar->pFont, pItem->sText, tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
	if ( pToolbar->iOverflowCount > 0 && pToolbar->tOverflowRect.fW > 0.0f && pToolbar->tOverflowRect.fH > 0.0f ) {
		iColor = pToolbar->bOverflowActive ? pToolbar->iActiveColor : pToolbar->iHoverColor;
		__xgeXuiHostDrawRect(pToolbar->tOverflowRect, iColor);
		__xgeXuiHostDrawBorderRect(pToolbar->tOverflowRect, 1.0f, XGE_COLOR_RGBA(127, 196, 229, 255));
		tDot = pToolbar->tOverflowRect;
		tDot.fW = 2.0f;
		tDot.fH = 2.0f;
		tDot.fX = pToolbar->tOverflowRect.fX + (pToolbar->tOverflowRect.fW - 10.0f) * 0.5f;
		tDot.fY = pToolbar->tOverflowRect.fY + (pToolbar->tOverflowRect.fH - 2.0f) * 0.5f;
		__xgeXuiHostDrawRect(tDot, pToolbar->iTextColor);
		tDot.fX += 4.0f;
		__xgeXuiHostDrawRect(tDot, pToolbar->iTextColor);
		tDot.fX += 4.0f;
		__xgeXuiHostDrawRect(tDot, pToolbar->iTextColor);
	}
}
