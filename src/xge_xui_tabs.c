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

static int __xgeXuiTabsIndexAt(xge_xui_tabs pTabs, float fX, float fY)
{
	xge_rect_t tContent;
	int iIndex;

	if ( (pTabs == NULL) || (pTabs->pWidget == NULL) || (pTabs->fTabWidth <= 0.0f) || (pTabs->fTabHeight <= 0.0f) ) {
		return -1;
	}
	tContent = pTabs->pWidget->tContentRect;
	if ( (fX < tContent.fX) || (fY < tContent.fY) || (fY >= (tContent.fY + pTabs->fTabHeight)) ) {
		return -1;
	}
	iIndex = (int)((fX - tContent.fX) / pTabs->fTabWidth);
	if ( (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		return -1;
	}
	if ( fX >= (tContent.fX + (float)(iIndex + 1) * pTabs->fTabWidth) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiTabsSelectInternal(xge_xui_tabs pTabs, int iIndex, int bNotify)
{
	if ( pTabs == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pTabs->iItemCount) ) {
		iIndex = -1;
	}
	if ( pTabs->iSelected != iIndex ) {
		pTabs->iSelected = iIndex;
		pTabs->iChangeCount++;
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
	xgeXuiWidgetMarkPaint(pTabs->pWidget);
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
			pTabs->iHover = iIndex;
			if ( iIndex >= 0 ) {
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
			if ( iIndex < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTabs->pContext, pTabs->pWidget);
			xgeXuiSetCapture(pTabs->pContext, pTabs->pWidget);
			pTabs->iHover = iIndex;
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
			pTabs->iHover = iIndex;
			__xgeXuiTabsSetState(pTabs, iIndex >= 0 ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( iIndex >= 0 ) {
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			pTabs->iHover = -1;
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
				iIndex = pTabs->iSelected + 1;
				if ( iIndex >= pTabs->iItemCount ) {
					iIndex = 0;
				}
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
				iIndex = pTabs->iSelected - 1;
				if ( iIndex < 0 ) {
					iIndex = pTabs->iItemCount - 1;
				}
				__xgeXuiTabsSelectInternal(pTabs, iIndex, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

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
		tTab.fX = pWidget->tContentRect.fX + (float)i * pTabs->fTabWidth;
		tTab.fY = pWidget->tContentRect.fY;
		tTab.fW = pTabs->fTabWidth;
		tTab.fH = pTabs->fTabHeight;
		if ( tTab.fX >= (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			break;
		}
		if ( (tTab.fX + tTab.fW) > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
			tTab.fW = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tTab.fX;
		}
		iColor = pTabs->iTabColor;
		iTextColor = pTabs->iTextColor;
		if ( (pTabs->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
			iColor = pTabs->iDisabledColor;
		} else if ( i == pTabs->iSelected ) {
			iColor = pTabs->iActiveColor;
			iTextColor = pTabs->iActiveTextColor;
		} else if ( i == pTabs->iHover ) {
			iColor = pTabs->iHoverColor;
		}
		if ( XGE_COLOR_GET_A(iColor) != 0 ) {
			__xgeXuiHostDrawRect(tTab, iColor);
		}
		if ( (pTabs->pFont != NULL) && (pTabs->arrItems != NULL) && (pTabs->arrItems[i] != NULL) ) {
			tText = tTab;
			tText.fX += 6.0f;
			tText.fW -= 12.0f;
			if ( tText.fW > 0.0f ) {
				__xgeXuiHostDrawTextRect(pTabs->pFont, pTabs->arrItems[i], tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
		}
	}
}
