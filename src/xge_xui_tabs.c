static void __xgeXuiTabsSetState(xge_xui_tabs pTabs, int iState)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( (pTabs->pWidget == NULL) || ((pTabs->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pTabs->pContext != NULL && pTabs->pContext->pFocus == pTabs->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pTabs->iState != iState ) {
		pTabs->iState = iState;
		xgeXuiWidgetMarkPaint(pTabs->pWidget);
	}
}

static float __xgeXuiTabsMaxScroll(xge_xui_tabs pTabs)
{
	float fMax;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = (float)pTabs->iItemCount * pTabs->fTabWidth - pTabs->pWidget->tContentRect.fW;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiTabsClampScroll(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->fScrollX = __xgeXuiClampFloat(pTabs->fScrollX, 0.0f, __xgeXuiTabsMaxScroll(pTabs));
}

static int __xgeXuiTabsItemEnabled(xge_xui_tabs pTabs, int iIndex)
{
	if ( (pTabs == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return 0;
	}
	if ( (pTabs->arrEnabled == NULL) || (iIndex >= pTabs->iEnabledCount) ) {
		return 1;
	}
	return pTabs->arrEnabled[iIndex] != 0;
}

static int __xgeXuiTabsItemDirty(xge_xui_tabs pTabs, int iIndex)
{
	if ( (pTabs == NULL) || (pTabs->arrDirty == NULL) || (iIndex < 0) || (iIndex >= pTabs->iDirtyCount) ) {
		return 0;
	}
	return pTabs->arrDirty[iIndex] != 0;
}

static xge_rect_t __xgeXuiTabsTabRect(xge_xui_tabs pTabs, int iIndex)
{
	xge_rect_t tTab;

	memset(&tTab, 0, sizeof(tTab));
	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) ) {
		return tTab;
	}
	tTab.fX = pTabs->pWidget->tContentRect.fX - pTabs->fScrollX + (float)iIndex * pTabs->fTabWidth;
	tTab.fY = pTabs->pWidget->tContentRect.fY;
	tTab.fW = pTabs->fTabWidth;
	tTab.fH = pTabs->fTabHeight;
	return tTab;
}

static xge_rect_t __xgeXuiTabsCloseRect(xge_xui_tabs pTabs, int iIndex)
{
	xge_rect_t tRect;
	xge_rect_t tTab;
	float fSize;

	memset(&tRect, 0, sizeof(tRect));
	if ( (pTabs == NULL) || (pTabs->bCloseButtons == 0) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return tRect;
	}
	tTab = __xgeXuiTabsTabRect(pTabs, iIndex);
	fSize = (pTabs->fTabHeight < 18.0f) ? pTabs->fTabHeight - 8.0f : 10.0f;
	if ( fSize < 6.0f ) {
		fSize = 6.0f;
	}
	tRect.fX = tTab.fX + tTab.fW - fSize - 7.0f;
	tRect.fY = tTab.fY + (tTab.fH - fSize) * 0.5f;
	tRect.fW = fSize;
	tRect.fH = fSize;
	return tRect;
}

static int __xgeXuiTabsIndexAt(xge_xui_tabs pTabs, float fX, float fY)
{
	xge_rect_t tContent;
	xge_rect_t tTab;
	int iIndex;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) || (pTabs->fTabWidth <= 0.0f) || (pTabs->fTabHeight <= 0.0f) ) {
		return -1;
	}
	tContent = pTabs->pWidget->tContentRect;
	if ( (fX < tContent.fX) || (fY < tContent.fY) || (fY >= (tContent.fY + pTabs->fTabHeight)) ) {
		return -1;
	}
	iIndex = (int)((fX - tContent.fX + pTabs->fScrollX) / pTabs->fTabWidth);
	if ( (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return -1;
	}
	tTab = __xgeXuiTabsTabRect(pTabs, iIndex);
	if ( (fX < tTab.fX) || (fX >= tTab.fX + tTab.fW) || (tTab.fX >= tContent.fX + tContent.fW) || (tTab.fX + tTab.fW <= tContent.fX) ) {
		return -1;
	}
	return iIndex;
}

static int __xgeXuiTabsCloseAt(xge_xui_tabs pTabs, float fX, float fY)
{
	int iIndex;
	xge_rect_t tClose;

	iIndex = __xgeXuiTabsIndexAt(pTabs, fX, fY);
	if ( iIndex < 0 ) {
		return -1;
	}
	tClose = __xgeXuiTabsCloseRect(pTabs, iIndex);
	if ( __xgeXuiRectContains(tClose, fX, fY) ) {
		return iIndex;
	}
	return -1;
}

static void __xgeXuiTabsEnsureVisible(xge_xui_tabs pTabs, int iIndex)
{
	float fLeft;
	float fRight;
	float fViewW;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) || (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return;
	}
	fViewW = pTabs->pWidget->tContentRect.fW;
	fLeft = (float)iIndex * pTabs->fTabWidth;
	fRight = fLeft + pTabs->fTabWidth;
	if ( fRight - pTabs->fScrollX > fViewW ) {
		pTabs->fScrollX = fRight - fViewW;
	}
	if ( fLeft < pTabs->fScrollX ) {
		pTabs->fScrollX = fLeft;
	}
	__xgeXuiTabsClampScroll(pTabs);
}

static void __xgeXuiTabsLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	__xgeXuiTabsClampScroll((xge_xui_tabs)pUser);
}

static int __xgeXuiTabsStepEnabled(xge_xui_tabs pTabs, int iStart, int iDelta)
{
	int i;
	int iIndex;

	if ( (pTabs == NULL) || (pTabs->iItemCount <= 0) ) {
		return -1;
	}
	iIndex = iStart;
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		iIndex += iDelta;
		if ( iIndex >= pTabs->iItemCount ) {
			iIndex = 0;
		} else if ( iIndex < 0 ) {
			iIndex = pTabs->iItemCount - 1;
		}
		if ( __xgeXuiTabsItemEnabled(pTabs, iIndex) ) {
			return iIndex;
		}
	}
	return -1;
}

static void __xgeXuiTabsSelectInternal(xge_xui_tabs pTabs, int iIndex, int bNotify)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		iIndex = -1;
	}
	if ( (iIndex >= 0) && (__xgeXuiTabsItemEnabled(pTabs, iIndex) == 0) ) {
		return;
	}
	if ( pTabs->iSelected != iIndex ) {
		pTabs->iSelected = iIndex;
		pTabs->iChangeCount++;
		__xgeXuiTabsEnsureVisible(pTabs, iIndex);
		xgeXuiWidgetMarkPaint(pTabs->pWidget);
		if ( bNotify && pTabs->procSelect != NULL ) {
			pTabs->procSelect(pTabs->pWidget, pTabs->iSelected, pTabs->pUser);
		}
	}
}

int xgeXuiTabsInit(xge_xui_tabs pTabs, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTabs == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTabs, 0, sizeof(*pTabs));
	pTheme = xgeXuiGetTheme(pContext);
	pTabs->pContext = pContext;
	pTabs->pWidget = pWidget;
	pTabs->pFont = pTheme->pFont;
	pTabs->iSelected = -1;
	pTabs->iHover = -1;
	pTabs->fTabWidth = 86.0f;
	pTabs->fTabHeight = 28.0f;
	pTabs->iBackgroundColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	pTabs->iTabColor = pTheme->iPanelColor;
	pTabs->iHoverColor = pTheme->iStateHover;
	pTabs->iActiveColor = pTheme->iAccentColor;
	pTabs->iFocusColor = pTheme->iStateFocus;
	pTabs->iDisabledColor = pTheme->iStateDisabled;
	pTabs->iTextColor = pTheme->iTextColor;
	pTabs->iActiveTextColor = pTheme->iTextColor;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procLayout = __xgeXuiTabsLayoutProc;
	pWidget->pLayoutUser = pTabs;
	pWidget->procEvent = xgeXuiTabsEventProc;
	pWidget->procPaint = xgeXuiTabsPaintProc;
	pWidget->pUser = pTabs;
	__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiTabsUnit(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( pTabs->pWidget != NULL && pTabs->pWidget->pUser == pTabs ) {
		pTabs->pWidget->pUser = NULL;
		pTabs->pWidget->procEvent = NULL;
		pTabs->pWidget->procPaint = NULL;
	}
	if ( pTabs->pWidget != NULL && pTabs->pWidget->pLayoutUser == pTabs ) {
		pTabs->pWidget->pLayoutUser = NULL;
		pTabs->pWidget->procLayout = NULL;
	}
	memset(pTabs, 0, sizeof(*pTabs));
}

void xgeXuiTabsSetItems(xge_xui_tabs pTabs, const char** arrItems, int iCount)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pTabs->arrItems = arrItems;
	pTabs->iItemCount = iCount;
	if ( pTabs->iSelected >= iCount ) {
		pTabs->iSelected = -1;
	}
	if ( pTabs->iHover >= iCount ) {
		pTabs->iHover = -1;
	}
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetEnabledItems(xge_xui_tabs pTabs, const int* arrEnabled, int iCount)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrEnabled = arrEnabled;
	pTabs->iEnabledCount = (iCount > 0) ? iCount : 0;
	if ( (pTabs->iSelected >= 0) && (__xgeXuiTabsItemEnabled(pTabs, pTabs->iSelected) == 0) ) {
		pTabs->iSelected = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, 1);
	}
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetDirtyItems(xge_xui_tabs pTabs, const int* arrDirty, int iCount)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrDirty = arrDirty;
	pTabs->iDirtyCount = (iCount > 0) ? iCount : 0;
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetIcons(xge_xui_tabs pTabs, const xge_texture* arrIcons, const xge_rect_t* arrSrc, int iCount)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->arrIcons = arrIcons;
	pTabs->arrIconSrc = arrSrc;
	pTabs->iIconCount = (iCount > 0) ? iCount : 0;
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetFont(xge_xui_tabs pTabs, xge_font pFont)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->pFont = pFont;
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetSelect(xge_xui_tabs pTabs, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->procSelect = procSelect;
	pTabs->pUser = pUser;
}

void xgeXuiTabsSetClose(xge_xui_tabs pTabs, xge_xui_select_proc procClose, int bCloseButtons, void* pUser)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->procClose = procClose;
	pTabs->bCloseButtons = (bCloseButtons != 0);
	pTabs->pUser = pUser;
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetSelected(xge_xui_tabs pTabs, int iIndex)
{
	__xgeXuiTabsSelectInternal(pTabs, iIndex, 0);
}

int xgeXuiTabsGetSelected(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return -1;
	}
	return pTabs->iSelected;
}

void xgeXuiTabsSetTabSize(xge_xui_tabs pTabs, float fWidth, float fHeight)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( fWidth < 1.0f ) {
		fWidth = 1.0f;
	}
	if ( fHeight < 1.0f ) {
		fHeight = 1.0f;
	}
	pTabs->fTabWidth = fWidth;
	pTabs->fTabHeight = fHeight;
	__xgeXuiTabsClampScroll(pTabs);
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetScrollable(xge_xui_tabs pTabs, int bScrollable)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->bScrollable = (bScrollable != 0);
	if ( pTabs->bScrollable == 0 ) {
		pTabs->fScrollX = 0.0f;
	}
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

void xgeXuiTabsSetScroll(xge_xui_tabs pTabs, float fScrollX)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->fScrollX = fScrollX;
	__xgeXuiTabsClampScroll(pTabs);
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

float xgeXuiTabsGetScroll(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return 0.0f;
	}
	__xgeXuiTabsClampScroll(pTabs);
	return pTabs->fScrollX;
}

void xgeXuiTabsSetColors(xge_xui_tabs pTabs, uint32_t iBackground, uint32_t iTab, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iText, uint32_t iActiveText)
{
	if ( pTabs == NULL ) {
		return;
	}
	pTabs->iBackgroundColor = iBackground;
	pTabs->iTabColor = iTab;
	pTabs->iHoverColor = iHover;
	pTabs->iActiveColor = iActive;
	pTabs->iFocusColor = iFocus;
	pTabs->iDisabledColor = iDisabled;
	pTabs->iTextColor = iText;
	pTabs->iActiveTextColor = iActiveText;
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
}

int xgeXuiTabsGetState(xge_xui_tabs pTabs)
{
	if ( pTabs == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiTabsSetState(pTabs, pTabs->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pTabs->iState;
}

int xgeXuiTabsEvent(xge_xui_tabs pTabs, const xge_event_t* pEvent)
{
	int iIndex;
	int iClose;
	int iState;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pTabs->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pTabs->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = __xgeXuiTabsIndexAt(pTabs, pEvent->fX, pEvent->fY);
	iState = pTabs->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			pTabs->iHover = (iIndex >= 0 && __xgeXuiTabsItemEnabled(pTabs, iIndex)) ? iIndex : -1;
			if ( pTabs->iHover >= 0 ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiTabsSetState(pTabs, iState);
			xgeXuiWidgetMarkPaint(pTabs->pWidget);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			pTabs->iHover = -1;
			__xgeXuiTabsSetState(pTabs, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( (iIndex < 0) || (__xgeXuiTabsItemEnabled(pTabs, iIndex) == 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTabs->pContext, pTabs->pWidget);
			xgeXuiSetCapture(pTabs->pContext, pTabs->pWidget);
			pTabs->iHover = iIndex;
			pTabs->iActive = iIndex;
			pTabs->bActiveClose = (__xgeXuiTabsCloseAt(pTabs, pEvent->fX, pEvent->fY) == iIndex);
			__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( (pTabs->iState & XGE_XUI_STATE_ACTIVE) == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pTabs->pContext != NULL && pTabs->pContext->pCapture == pTabs->pWidget ) {
				xgeXuiSetCapture(pTabs->pContext, NULL);
			}
			iClose = __xgeXuiTabsCloseAt(pTabs, pEvent->fX, pEvent->fY);
			pTabs->iHover = (iIndex >= 0 && __xgeXuiTabsItemEnabled(pTabs, iIndex)) ? iIndex : -1;
			__xgeXuiTabsSetState(pTabs, pTabs->iHover >= 0 ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( (pTabs->bActiveClose != 0) && (iClose == pTabs->iActive) && (pTabs->procClose != NULL) ) {
				pTabs->procClose(pTabs->pWidget, iClose, pTabs->pUser);
			} else if ( pTabs->iHover >= 0 ) {
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
			}
			pTabs->iActive = -1;
			pTabs->bActiveClose = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			pTabs->iHover = -1;
			pTabs->iActive = -1;
			pTabs->bActiveClose = 0;
			__xgeXuiTabsSetState(pTabs, XGE_XUI_STATE_NORMAL);
			if ( pTabs->pContext != NULL && pTabs->pContext->pCapture == pTabs->pWidget ) {
				xgeXuiSetCapture(pTabs->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pTabs->pContext == NULL) || (pTabs->pContext->pFocus != pTabs->pWidget) || (pTabs->iItemCount <= 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
				iIndex = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, 1);
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
				iIndex = __xgeXuiTabsStepEnabled(pTabs, pTabs->iSelected, -1);
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_WHEEL:
			if ( (pTabs->bScrollable == 0) || (__xgeXuiRectContains(pTabs->pWidget->tContentRect, pEvent->fX, pEvent->fY) == 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiTabsSetScroll(pTabs, pTabs->fScrollX - (pEvent->fDX != 0.0f ? pEvent->fDX : pEvent->fDY) * 32.0f);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTabsEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTabsEvent((xge_xui_tabs)pUser, pEvent);
}

void xgeXuiTabsPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tabs pTabs;
	xge_rect_t tTab;
	xge_rect_t tText;
	xge_rect_t tIcon;
	xge_rect_t tClose;
	xge_rect_t tMark;
	xge_draw_t tDraw;
	uint32_t iColor;
	uint32_t iTextColor;
	int i;

	pTabs = (xge_xui_tabs)pUser;
	if ( (pWidget == NULL) || (pTabs == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pTabs->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pTabs->iBackgroundColor);
	}
	if ( (pTabs->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pTabs->iFocusColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pTabs->iFocusColor);
	}
	for ( i = 0; i < pTabs->iItemCount; i++ ) {
		tTab = __xgeXuiTabsTabRect(pTabs, i);
		if ( tTab.fX >= (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			break;
		}
		if ( (tTab.fX + tTab.fW) <= pWidget->tContentRect.fX ) {
			continue;
		}
		if ( tTab.fX < pWidget->tContentRect.fX ) {
			tTab.fW -= pWidget->tContentRect.fX - tTab.fX;
			tTab.fX = pWidget->tContentRect.fX;
		}
		if ( (tTab.fX + tTab.fW) > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			tTab.fW = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tTab.fX;
		}
		iColor = pTabs->iTabColor;
		iTextColor = pTabs->iTextColor;
		if ( (pTabs->iState & XGE_XUI_STATE_DISABLED) != 0 || __xgeXuiTabsItemEnabled(pTabs, i) == 0 ) {
			iColor = pTabs->iDisabledColor;
		} else if ( i == pTabs->iSelected ) {
			iColor = pTabs->iActiveColor;
			iTextColor = pTabs->iActiveTextColor;
		} else if ( i == pTabs->iHover ) {
			iColor = pTabs->iHoverColor;
		}
		if ( XGE_COLOR_GET_A(iColor) != 0 ) {
			__xgeXuiHostDrawRect(tTab, iColor);
			__xgeXuiHostDrawBorderRect(tTab, 1.0f, XGE_COLOR_RGBA(156, 199, 231, 255));
		}
		tText = tTab;
		tText.fX += 6.0f;
		tText.fW -= 12.0f;
		if ( (pTabs->arrIcons != NULL) && (i < pTabs->iIconCount) ) {
			tIcon.fX = tText.fX;
			tIcon.fY = tTab.fY + (tTab.fH - 14.0f) * 0.5f;
			tIcon.fW = 14.0f;
			tIcon.fH = 14.0f;
			if ( pTabs->arrIcons[i] != NULL ) {
				memset(&tDraw, 0, sizeof(tDraw));
				tDraw.pTexture = pTabs->arrIcons[i];
				tDraw.tDst = tIcon;
				tDraw.tSrc = (pTabs->arrIconSrc != NULL) ? pTabs->arrIconSrc[i] : (xge_rect_t){ 0.0f, 0.0f, 1.0f, 1.0f };
				tDraw.iColor = XGE_COLOR_RGBA(255, 255, 255, 255);
				tDraw.iFlags = XGE_DRAW_SCREEN_SPACE;
				__xgeXuiHostDrawImage(&tDraw);
			}
			tText.fX += 18.0f;
			tText.fW -= 18.0f;
		}
		if ( __xgeXuiTabsItemDirty(pTabs, i) ) {
			tMark.fX = tTab.fX + 6.0f;
			tMark.fY = tTab.fY + 5.0f;
			tMark.fW = 5.0f;
			tMark.fH = 5.0f;
			__xgeXuiHostDrawRect(tMark, XGE_COLOR_RGBA(41, 151, 255, 255));
			tText.fX += 8.0f;
			tText.fW -= 8.0f;
		}
		if ( pTabs->bCloseButtons != 0 ) {
			tClose = __xgeXuiTabsCloseRect(pTabs, i);
			if ( (tClose.fW > 0.0f) && (tClose.fH > 0.0f) ) {
				__xgeXuiHostDrawBorderRect(tClose, 1.0f, XGE_COLOR_RGBA(96, 143, 186, 255));
			}
			tText.fW -= tClose.fW + 8.0f;
		}
		if ( (pTabs->pFont != NULL) && (pTabs->arrItems != NULL) && (pTabs->arrItems[i] != NULL) ) {
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pTabs->pFont, pTabs->arrItems[i], tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
}
