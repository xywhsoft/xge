int xgeXuiButtonInit(xge_xui_button pButton, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pButton == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pButton, 0, sizeof(*pButton));
	pTheme = xgeXuiGetTheme(pContext);
	pButton->pContext = pContext;
	pButton->pWidget = pWidget;
	pButton->pFont = pTheme->pFont;
	pButton->sText = "";
	pButton->iTextColor = pTheme->iTextColor;
	pButton->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pButton->iColorNormal = pTheme->iStateNormal;
	pButton->iColorHover = pTheme->iStateHover;
	pButton->iColorActive = pTheme->iStateActive;
	pButton->iColorFocus = pTheme->iStateFocus;
	pButton->iColorDisabled = pTheme->iStateDisabled;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiButtonEventProc;
	pWidget->procPaint = xgeXuiButtonPaintProc;
	pWidget->pUser = pButton;
	__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiButtonUnit(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return;
	}
	if ( pButton->pWidget != NULL && pButton->pWidget->pUser == pButton ) {
		pButton->pWidget->pUser = NULL;
		pButton->pWidget->procEvent = NULL;
		pButton->pWidget->procPaint = NULL;
	}
	memset(pButton, 0, sizeof(*pButton));
}

void xgeXuiButtonSetClick(xge_xui_button pButton, xge_xui_click_proc procClick, void* pUser)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->procClick = procClick;
	pButton->pUser = pUser;
}

void xgeXuiButtonSetText(xge_xui_button pButton, xge_font pFont, const char* sText)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->pFont = pFont;
	pButton->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pButton->pWidget);
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetTextColor(xge_xui_button pButton, uint32_t iColor)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

void xgeXuiButtonSetColors(xge_xui_button pButton, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pButton == NULL ) {
		return;
	}
	pButton->iColorNormal = iNormal;
	pButton->iColorHover = iHover;
	pButton->iColorActive = iActive;
	pButton->iColorFocus = iFocus;
	pButton->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pButton->pWidget);
}

int xgeXuiButtonGetState(xge_xui_button pButton)
{
	if ( pButton == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiButtonSetState(pButton, pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pButton->iState;
}

int xgeXuiButtonEvent(xge_xui_button pButton, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pButton == NULL) || (pButton->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pButton->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pButton->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pButton->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pButton->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiButtonSetState(pButton, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiButtonSetState(pButton, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiButtonSetState(pButton, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiButtonSetState(pButton, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pButton->pContext, pButton->pWidget);
			xgeXuiSetCapture(pButton->pContext, pButton->pWidget);
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pButton->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiButtonSetState(pButton, iState);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pButton->iClickCount++;
				if ( pButton->procClick != NULL ) {
					pButton->procClick(pButton->pWidget, pButton->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			__xgeXuiButtonSetState(pButton, XGE_XUI_STATE_NORMAL);
			if ( pButton->pContext != NULL && pButton->pContext->pCapture == pButton->pWidget ) {
				xgeXuiSetCapture(pButton->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pButton->pContext == NULL) || (pButton->pContext->pFocus != pButton->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pButton->iClickCount++;
			if ( pButton->procClick != NULL ) {
				pButton->procClick(pButton->pWidget, pButton->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiButtonEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiButtonEvent((xge_xui_button)pUser, pEvent);
}

void xgeXuiButtonPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_button pButton;
	uint32_t iColor;

	pButton = (xge_xui_button)pUser;
	if ( (pWidget == NULL) || (pButton == NULL) ) {
		return;
	}
	iColor = __xgeXuiButtonColor(pButton);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	if ( (pButton->pFont != NULL) && (pButton->sText != NULL) && (pButton->sText[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pButton->pFont, pButton->sText, pWidget->tContentRect, pButton->iTextColor, pButton->iTextFlags);
	}
}
