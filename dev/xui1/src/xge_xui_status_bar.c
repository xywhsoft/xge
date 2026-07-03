static int __xgeXuiStatusBarSection(int iSection)
{
	if ( iSection == XGE_XUI_STATUS_BAR_SECTION_CENTER || iSection == XGE_XUI_STATUS_BAR_SECTION_RIGHT ) {
		return iSection;
	}
	return XGE_XUI_STATUS_BAR_SECTION_LEFT;
}

static int __xgeXuiStatusBarItemInteractive(xge_xui_status_bar pStatusBar, int iIndex)
{
	xge_xui_status_bar_item_t* pItem;

	if ( (pStatusBar == NULL) || (iIndex < 0) || (iIndex >= pStatusBar->iItemCount) ) {
		return 0;
	}
	pItem = &pStatusBar->arrItems[iIndex];
	return (pItem->bEnabled != 0) && (pItem->bClickable != 0) && (pItem->iType != XGE_XUI_STATUS_BAR_ITEM_SPACER);
}

static xge_rect_t __xgeXuiStatusBarInsetRect(xge_rect_t tRect, float fLeft, float fTop, float fRight, float fBottom)
{
	tRect.fX += fLeft;
	tRect.fY += fTop;
	tRect.fW -= fLeft + fRight;
	tRect.fH -= fTop + fBottom;
	if ( tRect.fW < 0.0f ) {
		tRect.fW = 0.0f;
	}
	if ( tRect.fH < 0.0f ) {
		tRect.fH = 0.0f;
	}
	return tRect;
}

static uint32_t __xgeXuiStatusBarColorAlpha(uint32_t iColor, uint32_t iAlpha)
{
	if ( iAlpha > 255u ) {
		iAlpha = 255u;
	}
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iAlpha);
}

static void __xgeXuiStatusBarDrawRoundedFill(xge_rect_t tRect, float fRadius, uint32_t iColor)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (XGE_COLOR_GET_A(iColor) == 0) ) {
		return;
	}
	if ( fRadius <= 0.5f ) {
		__xgeXuiHostDrawRect(tRect, iColor);
		return;
	}
	if ( fRadius > tRect.fW * 0.5f ) {
		fRadius = tRect.fW * 0.5f;
	}
	if ( fRadius > tRect.fH * 0.5f ) {
		fRadius = tRect.fH * 0.5f;
	}
	if ( fRadius <= 0.5f ) {
		__xgeXuiHostDrawRect(tRect, iColor);
		return;
	}
	__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX + fRadius, tRect.fY, tRect.fW - fRadius * 2.0f, tRect.fH }, iColor);
	__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX, tRect.fY + fRadius, tRect.fW, tRect.fH - fRadius * 2.0f }, iColor);
	__xgeXuiHostDrawCircle(tRect.fX + fRadius, tRect.fY + fRadius, fRadius, iColor);
	__xgeXuiHostDrawCircle(tRect.fX + tRect.fW - fRadius, tRect.fY + fRadius, fRadius, iColor);
	__xgeXuiHostDrawCircle(tRect.fX + fRadius, tRect.fY + tRect.fH - fRadius, fRadius, iColor);
	__xgeXuiHostDrawCircle(tRect.fX + tRect.fW - fRadius, tRect.fY + tRect.fH - fRadius, fRadius, iColor);
}

static void __xgeXuiStatusBarDrawSoftCell(xge_rect_t tRect, uint32_t iFill, uint32_t iBorder, int bPressed)
{
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		return;
	}
	__xgeXuiStatusBarDrawRoundedFill(tRect, 3.0f, iFill);
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, iBorder);
	if ( bPressed != 0 ) {
		__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX + 1.0f, tRect.fY + tRect.fH - 2.0f, tRect.fW - 2.0f, 1.0f }, XGE_COLOR_RGBA(46, 96, 136, 36));
	} else {
		__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX + 1.0f, tRect.fY + 1.0f, tRect.fW - 2.0f, 1.0f }, XGE_COLOR_RGBA(255, 255, 255, 112));
	}
}

static void __xgeXuiStatusBarSetState(xge_xui_status_bar pStatusBar, int iState)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	if ( (pStatusBar->pWidget == NULL) || ((pStatusBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pStatusBar->pContext != NULL && pStatusBar->pContext->pFocus == pStatusBar->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pStatusBar->iState != iState ) {
		pStatusBar->iState = iState;
		xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
	}
}

static float __xgeXuiStatusBarItemWidth(xge_xui_status_bar pStatusBar, xge_xui_status_bar_item_t* pItem)
{
	xge_vec2_t tSize;
	float fWidth;

	if ( (pStatusBar == NULL) || (pItem == NULL) ) {
		return 0.0f;
	}
	if ( pItem->fWidth > 0.0f ) {
		return pItem->fWidth;
	}
	if ( (pItem->iType == XGE_XUI_STATUS_BAR_ITEM_SPACER) && (pItem->fFlex > 0.0f) ) {
		return 0.0f;
	}
	if ( pItem->iType == XGE_XUI_STATUS_BAR_ITEM_PROGRESS ) {
		return 90.0f;
	}
	if ( pItem->iType == XGE_XUI_STATUS_BAR_ITEM_SPACER ) {
		return 12.0f;
	}
	fWidth = 36.0f;
	if ( (pStatusBar->pFont != NULL) && (pItem->sText != NULL) && (pItem->sText[0] != 0) ) {
		tSize = __xgeXuiHostMeasureText(pStatusBar->pFont, pItem->sText);
		fWidth = tSize.fX + pStatusBar->fItemPadding * 2.0f;
	}
	return (fWidth < 12.0f) ? 12.0f : fWidth;
}

static void __xgeXuiStatusBarLayoutSection(xge_xui_status_bar pStatusBar, int iSection, float fStart, float fEnd, int iAlign)
{
	xge_rect_t tRect;
	float fTotal;
	float fFlexTotal;
	float fFlexExtra;
	float fCursor;
	float fWidth;
	int i;

	if ( (pStatusBar == NULL) || (pStatusBar->pWidget == NULL) ) {
		return;
	}
	fTotal = 0.0f;
	fFlexTotal = 0.0f;
	for ( i = 0; i < pStatusBar->iItemCount; i++ ) {
		if ( pStatusBar->arrItems[i].iSection == iSection ) {
			if ( fTotal > 0.0f ) {
				fTotal += pStatusBar->fGap;
			}
			if ( (pStatusBar->arrItems[i].iType == XGE_XUI_STATUS_BAR_ITEM_SPACER) && (pStatusBar->arrItems[i].fFlex > 0.0f) ) {
				fFlexTotal += pStatusBar->arrItems[i].fFlex;
			} else {
				fTotal += __xgeXuiStatusBarItemWidth(pStatusBar, &pStatusBar->arrItems[i]);
			}
		}
	}
	fFlexExtra = ((fEnd - fStart) > fTotal) ? ((fEnd - fStart) - fTotal) : 0.0f;
	if ( (fFlexTotal > 0.0f) && (fFlexExtra > 0.0f) ) {
		fTotal = fEnd - fStart;
	}
	if ( iAlign == XGE_XUI_STATUS_BAR_SECTION_RIGHT ) {
		fCursor = fEnd - fTotal;
	} else if ( iAlign == XGE_XUI_STATUS_BAR_SECTION_CENTER ) {
		fCursor = (fStart + fEnd - fTotal) * 0.5f;
	} else {
		fCursor = fStart;
	}
	if ( fCursor < fStart ) {
		fCursor = fStart;
	}
	tRect.fY = pStatusBar->pWidget->tContentRect.fY;
	tRect.fH = pStatusBar->pWidget->tContentRect.fH;
	for ( i = 0; i < pStatusBar->iItemCount; i++ ) {
		if ( pStatusBar->arrItems[i].iSection != iSection ) {
			continue;
		}
		if ( (pStatusBar->arrItems[i].iType == XGE_XUI_STATUS_BAR_ITEM_SPACER) && (pStatusBar->arrItems[i].fFlex > 0.0f) && (fFlexTotal > 0.0f) ) {
			fWidth = fFlexExtra * (pStatusBar->arrItems[i].fFlex / fFlexTotal);
		} else {
			fWidth = __xgeXuiStatusBarItemWidth(pStatusBar, &pStatusBar->arrItems[i]);
		}
		tRect.fX = fCursor;
		tRect.fW = fWidth;
		if ( tRect.fX < fStart ) {
			tRect.fW -= fStart - tRect.fX;
			tRect.fX = fStart;
		}
		if ( (tRect.fX + tRect.fW) > fEnd ) {
			tRect.fW = fEnd - tRect.fX;
		}
		if ( tRect.fW < 0.0f ) {
			tRect.fW = 0.0f;
		}
		pStatusBar->arrItems[i].tRect = tRect;
		fCursor += fWidth + pStatusBar->fGap;
	}
}

static void __xgeXuiStatusBarLayout(xge_xui_status_bar pStatusBar)
{
	xge_rect_t tContent;
	int i;

	if ( (pStatusBar == NULL) || (pStatusBar->pWidget == NULL) ) {
		return;
	}
	for ( i = 0; i < pStatusBar->iItemCount; i++ ) {
		memset(&pStatusBar->arrItems[i].tRect, 0, sizeof(pStatusBar->arrItems[i].tRect));
	}
	tContent = pStatusBar->pWidget->tContentRect;
	__xgeXuiStatusBarLayoutSection(pStatusBar, XGE_XUI_STATUS_BAR_SECTION_LEFT, tContent.fX, tContent.fX + tContent.fW, XGE_XUI_STATUS_BAR_SECTION_LEFT);
	__xgeXuiStatusBarLayoutSection(pStatusBar, XGE_XUI_STATUS_BAR_SECTION_CENTER, tContent.fX, tContent.fX + tContent.fW, XGE_XUI_STATUS_BAR_SECTION_CENTER);
	__xgeXuiStatusBarLayoutSection(pStatusBar, XGE_XUI_STATUS_BAR_SECTION_RIGHT, tContent.fX, tContent.fX + tContent.fW, XGE_XUI_STATUS_BAR_SECTION_RIGHT);
}

static int __xgeXuiStatusBarIndexAt(xge_xui_status_bar pStatusBar, float fX, float fY)
{
	int i;

	if ( pStatusBar == NULL ) {
		return -1;
	}
	__xgeXuiStatusBarLayout(pStatusBar);
	for ( i = 0; i < pStatusBar->iItemCount; i++ ) {
		if ( __xgeXuiStatusBarItemInteractive(pStatusBar, i) && __xgeXuiRectContains(pStatusBar->arrItems[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiStatusBarInit(xge_xui_status_bar pStatusBar, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	const xge_xui_chrome_style_t* pChrome;

	if ( (pStatusBar == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pStatusBar, 0, sizeof(*pStatusBar));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pChrome = xgeXuiGetChromeStyle(pContext);
	pStatusBar->pContext = pContext;
	pStatusBar->pWidget = pWidget;
	pStatusBar->pFont = pTheme->pFont;
	pStatusBar->iHover = -1;
	pStatusBar->iActive = -1;
	pStatusBar->fHeight = pChrome->tStatusBarMetrics.fHeight;
	pStatusBar->fGap = pChrome->tStatusBarMetrics.fItemGap;
	pStatusBar->fItemPadding = pChrome->tStatusBarMetrics.fItemPaddingX;
	xgeXuiWidgetSetBackground(pWidget, pChrome->tBarColors.iBackground);
	pStatusBar->iBorderColor = pChrome->tBarColors.iBorder;
	pStatusBar->iItemColor = pChrome->tBarColors.iItem;
	pStatusBar->iHoverColor = pChrome->tBarColors.iHover;
	pStatusBar->iActiveColor = pChrome->tBarColors.iActive;
	pStatusBar->iTextColor = pChrome->tBarColors.iText;
	pStatusBar->iDisabledTextColor = pChrome->tBarColors.iDisabledText;
	pStatusBar->iProgressTrackColor = XGE_COLOR_RGBA(216, 236, 248, 255);
	pStatusBar->iProgressFillColor = pChrome->tBarColors.iAccent;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiStatusBarEventProc, NULL);
	pWidget->procPaint = xgeXuiStatusBarPaintProc;
	pWidget->pUser = pStatusBar;
	__xgeXuiStatusBarSetState(pStatusBar, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiStatusBarUnit(xge_xui_status_bar pStatusBar)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	if ( pStatusBar->pWidget != NULL && pStatusBar->pWidget->pUser == pStatusBar ) {
		pStatusBar->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pStatusBar->pWidget, NULL, NULL);
		pStatusBar->pWidget->procPaint = NULL;
	}
	memset(pStatusBar, 0, sizeof(*pStatusBar));
}

void xgeXuiStatusBarClear(xge_xui_status_bar pStatusBar)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	memset(pStatusBar->arrItems, 0, sizeof(pStatusBar->arrItems));
	pStatusBar->iItemCount = 0;
	pStatusBar->iHover = -1;
	pStatusBar->iActive = -1;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

static int __xgeXuiStatusBarAddItem(xge_xui_status_bar pStatusBar, int iSection, int iType, const char* sText, float fWidth, int bClickable)
{
	xge_xui_status_bar_item_t* pItem;
	int iIndex;

	if ( pStatusBar == NULL ) {
		return -1;
	}
	if ( pStatusBar->iItemCount >= XGE_XUI_STATUS_BAR_CAPACITY ) {
		return -1;
	}
	iIndex = pStatusBar->iItemCount++;
	pItem = &pStatusBar->arrItems[iIndex];
	memset(pItem, 0, sizeof(*pItem));
	pItem->sText = (sText != NULL) ? sText : "";
	pItem->iType = iType;
	pItem->iSection = __xgeXuiStatusBarSection(iSection);
	pItem->bClickable = bClickable != 0;
	pItem->bEnabled = 1;
	pItem->fWidth = fWidth;
	pItem->fFlex = 0.0f;
	pItem->fMin = 0.0f;
	pItem->fMax = 1.0f;
	pItem->fValue = 0.0f;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
	return iIndex;
}

int xgeXuiStatusBarAddText(xge_xui_status_bar pStatusBar, int iSection, const char* sText, float fWidth, int bClickable)
{
	return __xgeXuiStatusBarAddItem(pStatusBar, iSection, XGE_XUI_STATUS_BAR_ITEM_TEXT, sText, fWidth, bClickable);
}

int xgeXuiStatusBarAddProgress(xge_xui_status_bar pStatusBar, int iSection, float fMin, float fMax, float fValue, float fWidth)
{
	int iIndex;
	xge_xui_status_bar_item_t* pItem;

	iIndex = __xgeXuiStatusBarAddItem(pStatusBar, iSection, XGE_XUI_STATUS_BAR_ITEM_PROGRESS, "", fWidth, 0);
	if ( iIndex < 0 ) {
		return -1;
	}
	pItem = &pStatusBar->arrItems[iIndex];
	if ( fMax < fMin ) {
		fMax = fMin;
	}
	pItem->fMin = fMin;
	pItem->fMax = fMax;
	pItem->fValue = fValue;
	return iIndex;
}

int xgeXuiStatusBarAddSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWidth)
{
	return __xgeXuiStatusBarAddItem(pStatusBar, iSection, XGE_XUI_STATUS_BAR_ITEM_SPACER, "", fWidth, 0);
}

int xgeXuiStatusBarAddFlexibleSpacer(xge_xui_status_bar pStatusBar, int iSection, float fWeight)
{
	int iIndex;

	iIndex = __xgeXuiStatusBarAddItem(pStatusBar, iSection, XGE_XUI_STATUS_BAR_ITEM_SPACER, "", 0.0f, 0);
	if ( iIndex < 0 ) {
		return -1;
	}
	if ( fWeight <= 0.0f ) {
		fWeight = 1.0f;
	}
	pStatusBar->arrItems[iIndex].fFlex = fWeight;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
	return iIndex;
}

void xgeXuiStatusBarSetFont(xge_xui_status_bar pStatusBar, xui_font pFont)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	pStatusBar->pFont = pFont;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

void xgeXuiStatusBarSetSelect(xge_xui_status_bar pStatusBar, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	pStatusBar->procSelect = procSelect;
	pStatusBar->pUser = pUser;
}

void xgeXuiStatusBarSetItemEnabled(xge_xui_status_bar pStatusBar, int iIndex, int bEnabled)
{
	if ( (pStatusBar == NULL) || (iIndex < 0) || (iIndex >= pStatusBar->iItemCount) ) {
		return;
	}
	pStatusBar->arrItems[iIndex].bEnabled = (bEnabled != 0);
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

void xgeXuiStatusBarSetItemText(xge_xui_status_bar pStatusBar, int iIndex, const char* sText)
{
	if ( (pStatusBar == NULL) || (iIndex < 0) || (iIndex >= pStatusBar->iItemCount) ) {
		return;
	}
	pStatusBar->arrItems[iIndex].sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

void xgeXuiStatusBarSetProgress(xge_xui_status_bar pStatusBar, int iIndex, float fValue)
{
	xge_xui_status_bar_item_t* pItem;

	if ( (pStatusBar == NULL) || (iIndex < 0) || (iIndex >= pStatusBar->iItemCount) ) {
		return;
	}
	pItem = &pStatusBar->arrItems[iIndex];
	if ( fValue < pItem->fMin ) {
		fValue = pItem->fMin;
	}
	if ( fValue > pItem->fMax ) {
		fValue = pItem->fMax;
	}
	pItem->fValue = fValue;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

void xgeXuiStatusBarSetMetrics(xge_xui_status_bar pStatusBar, float fHeight, float fGap, float fItemPadding)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	if ( fHeight < 1.0f ) {
		fHeight = 1.0f;
	}
	if ( fGap < 0.0f ) {
		fGap = 0.0f;
	}
	if ( fItemPadding < 0.0f ) {
		fItemPadding = 0.0f;
	}
	pStatusBar->fHeight = fHeight;
	pStatusBar->fGap = fGap;
	pStatusBar->fItemPadding = fItemPadding;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

void xgeXuiStatusBarSetColors(xge_xui_status_bar pStatusBar, uint32_t iBackground, uint32_t iBorder, uint32_t iItem, uint32_t iHover, uint32_t iActive, uint32_t iText, uint32_t iDisabledText, uint32_t iProgressTrack, uint32_t iProgressFill)
{
	if ( pStatusBar == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pStatusBar->pWidget, iBackground);
	pStatusBar->iBorderColor = iBorder;
	pStatusBar->iItemColor = iItem;
	pStatusBar->iHoverColor = iHover;
	pStatusBar->iActiveColor = iActive;
	pStatusBar->iTextColor = iText;
	pStatusBar->iDisabledTextColor = iDisabledText;
	pStatusBar->iProgressTrackColor = iProgressTrack;
	pStatusBar->iProgressFillColor = iProgressFill;
	xgeXuiWidgetMarkPaint(pStatusBar->pWidget);
}

int xgeXuiStatusBarGetState(xge_xui_status_bar pStatusBar)
{
	if ( pStatusBar == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiStatusBarSetState(pStatusBar, pStatusBar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pStatusBar->iState;
}

int xgeXuiStatusBarEvent(xge_xui_status_bar pStatusBar, const xge_event_t* pEvent)
{
	int iIndex;
	int iState;

	if ( (pStatusBar == NULL) || (pStatusBar->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pStatusBar->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pStatusBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiStatusBarSetState(pStatusBar, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = __xgeXuiStatusBarIndexAt(pStatusBar, pEvent->fX, pEvent->fY);
	iState = pStatusBar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pStatusBar->iHover = iIndex;
			__xgeXuiStatusBarSetState(pStatusBar, (iIndex >= 0) ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pStatusBar->iHover = -1;
			__xgeXuiStatusBarSetState(pStatusBar, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iIndex < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pStatusBar->pContext, pStatusBar->pWidget);
			xgeXuiSetPointerCapture(pStatusBar->pContext, pEvent->iPointerId, pStatusBar->pWidget);
			pStatusBar->iHover = iIndex;
			pStatusBar->iActive = iIndex;
			__xgeXuiStatusBarSetState(pStatusBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( (pStatusBar->iState & XGE_XUI_STATE_ACTIVE) == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pStatusBar->pContext != NULL && xgeXuiGetPointerCapture(pStatusBar->pContext, pEvent->iPointerId) == pStatusBar->pWidget ) {
				xgeXuiSetPointerCapture(pStatusBar->pContext, pEvent->iPointerId, NULL);
			}
			if ( iIndex == pStatusBar->iActive ) {
				pStatusBar->iSelectCount++;
				if ( pStatusBar->procSelect != NULL ) {
					pStatusBar->procSelect(pStatusBar->pWidget, iIndex, pStatusBar->pUser);
				}
			}
			pStatusBar->iHover = iIndex;
			pStatusBar->iActive = -1;
			__xgeXuiStatusBarSetState(pStatusBar, (iIndex >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pStatusBar->iHover = -1;
			pStatusBar->iActive = -1;
			__xgeXuiStatusBarSetState(pStatusBar, XGE_XUI_STATE_NORMAL);
			if ( pStatusBar->pContext != NULL && xgeXuiGetPointerCapture(pStatusBar->pContext, pEvent->iPointerId) == pStatusBar->pWidget ) {
				xgeXuiSetPointerCapture(pStatusBar->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiStatusBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiStatusBarEvent((xge_xui_status_bar)pUser, pEvent);
}

void xgeXuiStatusBarPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_status_bar pStatusBar;
	xge_xui_status_bar_item_t* pItem;
	xge_rect_t tRect;
	xge_rect_t tCell;
	xge_rect_t tText;
	xge_rect_t tTrack;
	xge_rect_t tFill;
	uint32_t iColor;
	uint32_t iBorder;
	uint32_t iTextColor;
	int bPressed;
	float fRange;
	float fRatio;
	int i;

	pStatusBar = (xge_xui_status_bar)pUser;
	if ( (pWidget == NULL) || (pStatusBar == NULL) ) {
		return;
	}
	__xgeXuiStatusBarLayout(pStatusBar);
	if ( XGE_COLOR_GET_A(pStatusBar->iBorderColor) != 0 ) {
		__xgeXuiHostDrawRect((xge_rect_t){ pWidget->tRect.fX, pWidget->tRect.fY, pWidget->tRect.fW, 1.0f }, pStatusBar->iBorderColor);
		__xgeXuiHostDrawRect((xge_rect_t){ pWidget->tRect.fX, pWidget->tRect.fY + 1.0f, pWidget->tRect.fW, 1.0f }, XGE_COLOR_RGBA(255, 255, 255, 108));
	}
	for ( i = 0; i < pStatusBar->iItemCount; i++ ) {
		pItem = &pStatusBar->arrItems[i];
		tRect = pItem->tRect;
		if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) || (pItem->iType == XGE_XUI_STATUS_BAR_ITEM_SPACER) ) {
			continue;
		}
		iColor = pStatusBar->iItemColor;
		iBorder = __xgeXuiStatusBarColorAlpha(pStatusBar->iBorderColor, 82);
		iTextColor = (pItem->bEnabled != 0) ? pStatusBar->iTextColor : pStatusBar->iDisabledTextColor;
		bPressed = 0;
		if ( i == pStatusBar->iActive ) {
			iColor = pStatusBar->iActiveColor;
			iBorder = XGE_COLOR_RGBA(86, 151, 206, 190);
			bPressed = 1;
		} else if ( i == pStatusBar->iHover ) {
			iColor = pStatusBar->iHoverColor;
			iBorder = XGE_COLOR_RGBA(112, 174, 220, 160);
		}
		if ( pItem->bClickable ) {
			tCell = __xgeXuiStatusBarInsetRect(tRect, 1.0f, 3.0f, 1.0f, 3.0f);
			if ( bPressed != 0 ) {
				tCell.fY += 1.0f;
			}
			if ( XGE_COLOR_GET_A(iColor) == 0 ) {
				iColor = XGE_COLOR_RGBA(255, 255, 255, 72);
			}
			if ( (tCell.fW > 0.0f) && (tCell.fH > 0.0f) ) {
				__xgeXuiStatusBarDrawSoftCell(tCell, iColor, iBorder, bPressed);
			}
		}
		if ( pItem->iType == XGE_XUI_STATUS_BAR_ITEM_PROGRESS ) {
			tTrack = tRect;
			tTrack.fX += pStatusBar->fItemPadding;
			tTrack.fW -= pStatusBar->fItemPadding * 2.0f;
			tTrack.fY += (tTrack.fH - 9.0f) * 0.5f;
			tTrack.fH = 9.0f;
			if ( (tTrack.fW > 0.0f) && XGE_COLOR_GET_A(pStatusBar->iProgressTrackColor) != 0 ) {
				__xgeXuiStatusBarDrawRoundedFill(tTrack, 4.0f, pStatusBar->iProgressTrackColor);
				fRange = pItem->fMax - pItem->fMin;
				fRatio = (fRange > 0.0f) ? ((pItem->fValue - pItem->fMin) / fRange) : 0.0f;
				if ( fRatio < 0.0f ) {
					fRatio = 0.0f;
				}
				if ( fRatio > 1.0f ) {
					fRatio = 1.0f;
				}
				tFill = __xgeXuiStatusBarInsetRect(tTrack, 1.0f, 1.0f, 1.0f, 1.0f);
				tFill.fW *= fRatio;
				if ( (tFill.fW > 0.0f) && XGE_COLOR_GET_A(pStatusBar->iProgressFillColor) != 0 ) {
					__xgeXuiStatusBarDrawRoundedFill(tFill, 3.0f, pStatusBar->iProgressFillColor);
					__xgeXuiHostDrawRect((xge_rect_t){ tFill.fX + 1.0f, tFill.fY + 1.0f, tFill.fW - 2.0f, 1.0f }, XGE_COLOR_RGBA(255, 255, 255, 86));
				}
				__xgeXuiHostDrawBorderRect(tTrack, 1.0f, __xgeXuiStatusBarColorAlpha(pStatusBar->iBorderColor, 130));
			}
		} else if ( (pStatusBar->pFont != NULL) && (pItem->sText != NULL) && (pItem->sText[0] != 0) ) {
			tText = tRect;
			tText.fX += pStatusBar->fItemPadding;
			tText.fW -= pStatusBar->fItemPadding * 2.0f;
			if ( bPressed != 0 ) {
				tText.fY += 1.0f;
			}
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pStatusBar->pFont, pItem->sText, tText, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
}
