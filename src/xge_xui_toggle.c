int xgeXuiToggleInit(xge_xui_toggle pToggle, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pToggle == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pToggle, 0, sizeof(*pToggle));
	pTheme = xgeXuiGetTheme(pContext);
	pToggle->pContext = pContext;
	pToggle->pWidget = pWidget;
	pToggle->pFont = pTheme->pFont;
	pToggle->sText = "";
	pToggle->iTextColor = pTheme->iTextColor;
	pToggle->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pToggle->iColorNormal = XGE_COLOR_RGBA(0, 0, 0, 0);
	pToggle->iColorHover = pTheme->iStateHover;
	pToggle->iColorActive = pTheme->iStateActive;
	pToggle->iColorFocus = pTheme->iStateFocus;
	pToggle->iColorDisabled = pTheme->iStateDisabled;
	pToggle->iColorChecked = pTheme->iAccentColor;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiToggleEventProc;
	pWidget->procPaint = xgeXuiTogglePaintProc;
	pWidget->pUser = pToggle;
	__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiToggleUnit(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return;
	}
	if ( pToggle->pWidget != NULL && pToggle->pWidget->pUser == pToggle ) {
		pToggle->pWidget->pUser = NULL;
		pToggle->pWidget->procEvent = NULL;
		pToggle->pWidget->procPaint = NULL;
	}
	memset(pToggle, 0, sizeof(*pToggle));
}

void xgeXuiToggleSetChange(xge_xui_toggle pToggle, xge_xui_toggle_proc procChange, void* pUser)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->procChange = procChange;
	pToggle->pUser = pUser;
}

void xgeXuiToggleSetText(xge_xui_toggle pToggle, xge_font pFont, const char* sText)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->pFont = pFont;
	pToggle->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pToggle->pWidget);
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

void xgeXuiToggleSetChecked(xge_xui_toggle pToggle, int bChecked)
{
	if ( pToggle == NULL ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( pToggle->bChecked != bChecked ) {
		pToggle->bChecked = bChecked;
		xgeXuiWidgetMarkPaint(pToggle->pWidget);
	}
}

int xgeXuiToggleGetChecked(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return 0;
	}
	return pToggle->bChecked;
}

void xgeXuiToggleSetTextColor(xge_xui_toggle pToggle, uint32_t iColor)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

void xgeXuiToggleSetColors(xge_xui_toggle pToggle, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iChecked)
{
	if ( pToggle == NULL ) {
		return;
	}
	pToggle->iColorNormal = iNormal;
	pToggle->iColorHover = iHover;
	pToggle->iColorActive = iActive;
	pToggle->iColorFocus = iFocus;
	pToggle->iColorDisabled = iDisabled;
	pToggle->iColorChecked = iChecked;
	xgeXuiWidgetMarkPaint(pToggle->pWidget);
}

int xgeXuiToggleGetState(xge_xui_toggle pToggle)
{
	if ( pToggle == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiToggleSetState(pToggle, pToggle->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pToggle->iState;
}

int xgeXuiToggleEvent(xge_xui_toggle pToggle, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pToggle == NULL) || (pToggle->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pToggle->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pToggle->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pToggle->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pToggle->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiToggleSetState(pToggle, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiToggleSetState(pToggle, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiToggleSetState(pToggle, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiToggleSetState(pToggle, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pToggle->pContext, pToggle->pWidget);
			xgeXuiSetCapture(pToggle->pContext, pToggle->pWidget);
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pToggle->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiToggleSetState(pToggle, iState);
			if ( pToggle->pContext != NULL && pToggle->pContext->pCapture == pToggle->pWidget ) {
				xgeXuiSetCapture(pToggle->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pToggle->bChecked = pToggle->bChecked ? 0 : 1;
				pToggle->iChangeCount++;
				xgeXuiWidgetMarkPaint(pToggle->pWidget);
				if ( pToggle->procChange != NULL ) {
					pToggle->procChange(pToggle->pWidget, pToggle->bChecked, pToggle->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
			if ( pToggle->pContext != NULL && pToggle->pContext->pCapture == pToggle->pWidget ) {
				xgeXuiSetCapture(pToggle->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pToggle->pContext == NULL) || (pToggle->pContext->pFocus != pToggle->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pToggle->bChecked = pToggle->bChecked ? 0 : 1;
			pToggle->iChangeCount++;
			xgeXuiWidgetMarkPaint(pToggle->pWidget);
			if ( pToggle->procChange != NULL ) {
				pToggle->procChange(pToggle->pWidget, pToggle->bChecked, pToggle->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiToggleEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiToggleEvent((xge_xui_toggle)pUser, pEvent);
}

void xgeXuiTogglePaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrCheck12[12] = {
		0x000, 0x000, 0x006, 0x00c,
		0x018, 0x830, 0xc60, 0x6c0,
		0x380, 0x100, 0x000, 0x000
	};
	xge_xui_toggle pToggle;
	xge_rect_t tBox;
	xge_rect_t tMark;
	xge_rect_t tText;
	xge_rect_t tContent;
	float fBoxSize;
	uint32_t iColor;
	uint32_t iBoxColor;

	pToggle = (xge_xui_toggle)pUser;
	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return;
	}
	tContent = pWidget->tContentRect;
	if ( (tContent.fW <= 0.0f) || (tContent.fH <= 0.0f) ) {
		tContent = pWidget->tRect;
	}
	iColor = __xgeXuiToggleColor(pToggle);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	fBoxSize = tContent.fH;
	if ( fBoxSize > 18.0f ) {
		fBoxSize = 18.0f;
	}
	if ( fBoxSize < 1.0f ) {
		fBoxSize = 1.0f;
	}
	tBox.fX = tContent.fX;
	tBox.fY = tContent.fY + (tContent.fH - fBoxSize) * 0.5f;
	tBox.fW = fBoxSize;
	tBox.fH = fBoxSize;
	iBoxColor = ((pToggle->iState & XGE_XUI_STATE_DISABLED) != 0) ? XGE_COLOR_RGBA(146, 158, 170, 255) : XGE_COLOR_RGBA(79, 149, 196, 255);
	__xgeXuiHostDrawRect(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawBorderRect(tBox, 1.5f, iBoxColor);
	if ( pToggle->bChecked ) {
		tMark = tBox;
		tMark.fX += 2.0f;
		tMark.fY += 2.0f;
		tMark.fW -= 4.0f;
		tMark.fH -= 4.0f;
		if ( (tMark.fW > 0.0f) && (tMark.fH > 0.0f) ) {
			__xgeXuiHostDrawBitmapMask(tMark, arrCheck12, 12, 12, pToggle->iColorChecked);
		}
	}
	if ( (pToggle->pFont != NULL) && (pToggle->sText != NULL) && (pToggle->sText[0] != 0) ) {
		tText = tContent;
		tText.fX += fBoxSize + 6.0f;
		tText.fW -= fBoxSize + 6.0f;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pToggle->pFont, pToggle->sText, tText, pToggle->iTextColor, pToggle->iTextFlags);
		}
	}
}
