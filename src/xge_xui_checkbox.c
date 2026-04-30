int xgeXuiCheckBoxInit(xge_xui_checkbox pCheckBox, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pCheckBox == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pCheckBox, 0, sizeof(*pCheckBox));
	pTheme = xgeXuiGetTheme(pContext);
	pCheckBox->pContext = pContext;
	pCheckBox->pWidget = pWidget;
	pCheckBox->pFont = pTheme->pFont;
	pCheckBox->sText = "";
	pCheckBox->iTextColor = pTheme->iTextColor;
	pCheckBox->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pCheckBox->iColorNormal = XGE_COLOR_RGBA(0, 0, 0, 0);
	pCheckBox->iColorHover = pTheme->iStateHover;
	pCheckBox->iColorActive = pTheme->iStateActive;
	pCheckBox->iColorFocus = pTheme->iStateFocus;
	pCheckBox->iColorDisabled = pTheme->iStateDisabled;
	pCheckBox->iColorBox = pTheme->iBorderColor;
	pCheckBox->iColorChecked = pTheme->iAccentColor;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiCheckBoxEventProc;
	pWidget->procPaint = xgeXuiCheckBoxPaintProc;
	pWidget->pUser = pCheckBox;
	__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiCheckBoxUnit(xge_xui_checkbox pCheckBox)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	if ( pCheckBox->pWidget != NULL && pCheckBox->pWidget->pUser == pCheckBox ) {
		pCheckBox->pWidget->pUser = NULL;
		pCheckBox->pWidget->procEvent = NULL;
		pCheckBox->pWidget->procPaint = NULL;
	}
	memset(pCheckBox, 0, sizeof(*pCheckBox));
}

void xgeXuiCheckBoxSetChange(xge_xui_checkbox pCheckBox, xge_xui_toggle_proc procChange, void* pUser)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->procChange = procChange;
	pCheckBox->pUser = pUser;
}

void xgeXuiCheckBoxSetText(xge_xui_checkbox pCheckBox, xge_font pFont, const char* sText)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->pFont = pFont;
	pCheckBox->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pCheckBox->pWidget);
	xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
}

void xgeXuiCheckBoxSetChecked(xge_xui_checkbox pCheckBox, int bChecked)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( pCheckBox->bChecked != bChecked ) {
		pCheckBox->bChecked = bChecked;
		xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
	}
}

int xgeXuiCheckBoxGetChecked(xge_xui_checkbox pCheckBox)
{
	if ( pCheckBox == NULL ) {
		return 0;
	}
	return pCheckBox->bChecked;
}

void xgeXuiCheckBoxSetTextColor(xge_xui_checkbox pCheckBox, uint32_t iColor)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
}

void xgeXuiCheckBoxSetColors(xge_xui_checkbox pCheckBox, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iBox, uint32_t iChecked)
{
	if ( pCheckBox == NULL ) {
		return;
	}
	pCheckBox->iColorNormal = iNormal;
	pCheckBox->iColorHover = iHover;
	pCheckBox->iColorActive = iActive;
	pCheckBox->iColorFocus = iFocus;
	pCheckBox->iColorDisabled = iDisabled;
	pCheckBox->iColorBox = iBox;
	pCheckBox->iColorChecked = iChecked;
	xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
}

int xgeXuiCheckBoxGetState(xge_xui_checkbox pCheckBox)
{
	if ( pCheckBox == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiCheckBoxSetState(pCheckBox, pCheckBox->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pCheckBox->iState;
}

int xgeXuiCheckBoxEvent(xge_xui_checkbox pCheckBox, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pCheckBox == NULL) || (pCheckBox->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pCheckBox->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pCheckBox->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pCheckBox->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pCheckBox->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			__xgeXuiCheckBoxSetState(pCheckBox, iState);
			return ((iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiCheckBoxSetState(pCheckBox, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiCheckBoxSetState(pCheckBox, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiCheckBoxSetState(pCheckBox, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pCheckBox->pContext, pCheckBox->pWidget);
			xgeXuiSetCapture(pCheckBox->pContext, pCheckBox->pWidget);
			__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pCheckBox->iState & XGE_XUI_STATE_ACTIVE) != 0);
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiCheckBoxSetState(pCheckBox, iState);
			if ( pCheckBox->pContext != NULL && pCheckBox->pContext->pCapture == pCheckBox->pWidget ) {
				xgeXuiSetCapture(pCheckBox->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				pCheckBox->bChecked = pCheckBox->bChecked ? 0 : 1;
				pCheckBox->iChangeCount++;
				xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
				if ( pCheckBox->procChange != NULL ) {
					pCheckBox->procChange(pCheckBox->pWidget, pCheckBox->bChecked, pCheckBox->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			__xgeXuiCheckBoxSetState(pCheckBox, XGE_XUI_STATE_NORMAL);
			if ( pCheckBox->pContext != NULL && pCheckBox->pContext->pCapture == pCheckBox->pWidget ) {
				xgeXuiSetCapture(pCheckBox->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pCheckBox->pContext == NULL) || (pCheckBox->pContext->pFocus != pCheckBox->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pCheckBox->bChecked = pCheckBox->bChecked ? 0 : 1;
			pCheckBox->iChangeCount++;
			xgeXuiWidgetMarkPaint(pCheckBox->pWidget);
			if ( pCheckBox->procChange != NULL ) {
				pCheckBox->procChange(pCheckBox->pWidget, pCheckBox->bChecked, pCheckBox->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiCheckBoxEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiCheckBoxEvent((xge_xui_checkbox)pUser, pEvent);
}

void xgeXuiCheckBoxPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrCheck12[12] = {
		0x000, 0x000, 0x006, 0x00c,
		0x018, 0x830, 0xc60, 0x6c0,
		0x380, 0x100, 0x000, 0x000
	};
	xge_xui_checkbox pCheckBox;
	xge_rect_t tBox;
	xge_rect_t tMark;
	xge_rect_t tText;
	float fBoxSize;
	uint32_t iColor;

	pCheckBox = (xge_xui_checkbox)pUser;
	if ( (pWidget == NULL) || (pCheckBox == NULL) ) {
		return;
	}
	iColor = __xgeXuiCheckBoxColor(pCheckBox);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	fBoxSize = pWidget->tContentRect.fH;
	if ( fBoxSize > 18.0f ) {
		fBoxSize = 18.0f;
	}
	if ( fBoxSize < 1.0f ) {
		fBoxSize = 1.0f;
	}
	tBox.fX = pWidget->tContentRect.fX;
	tBox.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fBoxSize) * 0.5f;
	tBox.fW = fBoxSize;
	tBox.fH = fBoxSize;
	__xgeXuiHostDrawRect(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawBorderRect(tBox, 1.5f, pCheckBox->iColorBox);
	if ( pCheckBox->bChecked ) {
		tMark = tBox;
		tMark.fX += 2.0f;
		tMark.fY += 2.0f;
		tMark.fW -= 4.0f;
		tMark.fH -= 4.0f;
		if ( (tMark.fW > 0.0f) && (tMark.fH > 0.0f) ) {
			__xgeXuiHostDrawBitmapMask(tMark, arrCheck12, 12, 12, pCheckBox->iColorChecked);
		}
	}
	if ( (pCheckBox->pFont != NULL) && (pCheckBox->sText != NULL) && (pCheckBox->sText[0] != 0) ) {
		tText = pWidget->tContentRect;
		tText.fX += fBoxSize + 6.0f;
		tText.fW -= fBoxSize + 6.0f;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pCheckBox->pFont, pCheckBox->sText, tText, pCheckBox->iTextColor, pCheckBox->iTextFlags);
		}
	}
}
