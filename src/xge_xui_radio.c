static void __xgeXuiRadioSetState(xge_xui_radio pRadio, int iState)
{
	if ( pRadio == NULL ) {
		return;
	}
	if ( (pRadio->pWidget == NULL) || ((pRadio->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pRadio->pContext != NULL && pRadio->pContext->pFocus == pRadio->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pRadio->iState != iState ) {
		pRadio->iState = iState;
		xgeXuiWidgetMarkPaint(pRadio->pWidget);
	}
}

static uint32_t __xgeXuiRadioColor(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pRadio->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pRadio->iColorDisabled;
	}
	if ( (pRadio->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pRadio->iColorActive;
	}
	if ( (pRadio->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pRadio->iColorHover;
	}
	if ( (pRadio->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pRadio->iColorFocus;
	}
	return pRadio->iColorNormal;
}

static void __xgeXuiRadioGroupUnlink(xge_xui_radio pRadio)
{
	xge_xui_radio pPrev;
	xge_xui_radio pIt;

	if ( (pRadio == NULL) || (pRadio->pGroup == NULL) ) {
		return;
	}
	pPrev = NULL;
	pIt = pRadio->pGroup->pFirst;
	while ( pIt != NULL ) {
		if ( pIt == pRadio ) {
			if ( pPrev != NULL ) {
				pPrev->pNextInGroup = pIt->pNextInGroup;
			} else {
				pRadio->pGroup->pFirst = pIt->pNextInGroup;
			}
			break;
		}
		pPrev = pIt;
		pIt = pIt->pNextInGroup;
	}
	pRadio->pGroup = NULL;
	pRadio->pNextInGroup = NULL;
}

static void __xgeXuiRadioSetCheckedInternal(xge_xui_radio pRadio, int bChecked, int bNotify)
{
	if ( pRadio == NULL ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( pRadio->bChecked == bChecked ) {
		return;
	}
	pRadio->bChecked = bChecked;
	pRadio->iChangeCount++;
	xgeXuiWidgetMarkPaint(pRadio->pWidget);
	if ( bNotify && pRadio->procChange != NULL ) {
		pRadio->procChange(pRadio->pWidget, pRadio->bChecked, pRadio->pUser);
	}
}

void xgeXuiRadioGroupInit(xge_xui_radio_group pGroup)
{
	if ( pGroup == NULL ) {
		return;
	}
	memset(pGroup, 0, sizeof(*pGroup));
	pGroup->iSelectedValue = -1;
}

void xgeXuiRadioGroupUnit(xge_xui_radio_group pGroup)
{
	xge_xui_radio pIt;
	xge_xui_radio pNext;

	if ( pGroup == NULL ) {
		return;
	}
	pIt = pGroup->pFirst;
	while ( pIt != NULL ) {
		pNext = pIt->pNextInGroup;
		pIt->pGroup = NULL;
		pIt->pNextInGroup = NULL;
		pIt = pNext;
	}
	memset(pGroup, 0, sizeof(*pGroup));
}

void xgeXuiRadioGroupSetChange(xge_xui_radio_group pGroup, xge_xui_select_proc procChange, void* pUser)
{
	if ( pGroup == NULL ) {
		return;
	}
	pGroup->procChange = procChange;
	pGroup->pUser = pUser;
}

void xgeXuiRadioGroupSetSelected(xge_xui_radio_group pGroup, int iValue)
{
	xge_xui_radio pIt;
	int bChanged;

	if ( pGroup == NULL ) {
		return;
	}
	bChanged = (pGroup->iSelectedValue != iValue);
	pGroup->iSelectedValue = iValue;
	pIt = pGroup->pFirst;
	while ( pIt != NULL ) {
		__xgeXuiRadioSetCheckedInternal(pIt, pIt->iValue == iValue, bChanged);
		pIt = pIt->pNextInGroup;
	}
	if ( bChanged ) {
		pGroup->iChangeCount++;
		if ( pGroup->procChange != NULL ) {
			pGroup->procChange(NULL, iValue, pGroup->pUser);
		}
	}
}

int xgeXuiRadioGroupGetSelected(xge_xui_radio_group pGroup)
{
	if ( pGroup == NULL ) {
		return -1;
	}
	return pGroup->iSelectedValue;
}

int xgeXuiRadioInit(xge_xui_radio pRadio, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pRadio == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pRadio, 0, sizeof(*pRadio));
	pTheme = xgeXuiGetTheme(pContext);
	pRadio->pContext = pContext;
	pRadio->pWidget = pWidget;
	pRadio->pFont = pTheme->pFont;
	pRadio->sText = "";
	pRadio->iValue = -1;
	pRadio->iTextColor = pTheme->iTextColor;
	pRadio->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pRadio->iColorNormal = XGE_COLOR_RGBA(0, 0, 0, 0);
	pRadio->iColorHover = pTheme->iStateHover;
	pRadio->iColorActive = pTheme->iStateActive;
	pRadio->iColorFocus = pTheme->iStateFocus;
	pRadio->iColorDisabled = pTheme->iStateDisabled;
	pRadio->iColorRing = XGE_COLOR_RGBA(180, 186, 196, 255);
	pRadio->iColorChecked = pTheme->iAccentColor;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiRadioEventProc;
	pWidget->procPaint = xgeXuiRadioPaintProc;
	pWidget->pUser = pRadio;
	__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiRadioUnit(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return;
	}
	__xgeXuiRadioGroupUnlink(pRadio);
	if ( pRadio->pWidget != NULL && pRadio->pWidget->pUser == pRadio ) {
		pRadio->pWidget->pUser = NULL;
		pRadio->pWidget->procEvent = NULL;
		pRadio->pWidget->procPaint = NULL;
	}
	memset(pRadio, 0, sizeof(*pRadio));
}

void xgeXuiRadioSetGroup(xge_xui_radio pRadio, xge_xui_radio_group pGroup, int iValue)
{
	if ( pRadio == NULL ) {
		return;
	}
	__xgeXuiRadioGroupUnlink(pRadio);
	pRadio->iValue = iValue;
	if ( pGroup != NULL ) {
		pRadio->pGroup = pGroup;
		pRadio->pNextInGroup = pGroup->pFirst;
		pGroup->pFirst = pRadio;
		__xgeXuiRadioSetCheckedInternal(pRadio, pGroup->iSelectedValue == iValue, 0);
	}
}

void xgeXuiRadioSetChange(xge_xui_radio pRadio, xge_xui_toggle_proc procChange, void* pUser)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->procChange = procChange;
	pRadio->pUser = pUser;
}

void xgeXuiRadioSetText(xge_xui_radio pRadio, xge_font pFont, const char* sText)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->pFont = pFont;
	pRadio->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pRadio->pWidget);
	xgeXuiWidgetMarkPaint(pRadio->pWidget);
}

void xgeXuiRadioSetChecked(xge_xui_radio pRadio, int bChecked)
{
	if ( pRadio == NULL ) {
		return;
	}
	if ( bChecked && pRadio->pGroup != NULL ) {
		xgeXuiRadioGroupSetSelected(pRadio->pGroup, pRadio->iValue);
	} else {
		__xgeXuiRadioSetCheckedInternal(pRadio, bChecked, 0);
	}
}

int xgeXuiRadioGetChecked(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return 0;
	}
	return pRadio->bChecked;
}

void xgeXuiRadioSetTextColor(xge_xui_radio pRadio, uint32_t iColor)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pRadio->pWidget);
}

void xgeXuiRadioSetColors(xge_xui_radio pRadio, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iRing, uint32_t iChecked)
{
	if ( pRadio == NULL ) {
		return;
	}
	pRadio->iColorNormal = iNormal;
	pRadio->iColorHover = iHover;
	pRadio->iColorActive = iActive;
	pRadio->iColorFocus = iFocus;
	pRadio->iColorDisabled = iDisabled;
	pRadio->iColorRing = iRing;
	pRadio->iColorChecked = iChecked;
	xgeXuiWidgetMarkPaint(pRadio->pWidget);
}

int xgeXuiRadioGetState(xge_xui_radio pRadio)
{
	if ( pRadio == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiRadioSetState(pRadio, pRadio->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pRadio->iState;
}

int xgeXuiRadioEvent(xge_xui_radio pRadio, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pRadio == NULL) || (pRadio->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pRadio->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pRadio->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pRadio->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pRadio->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiRadioSetState(pRadio, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pRadio->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiRadioSetState(pRadio, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiRadioSetState(pRadio, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiRadioSetState(pRadio, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pRadio->pContext, pRadio->pWidget);
			xgeXuiSetCapture(pRadio->pContext, pRadio->pWidget);
			__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pRadio->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiRadioSetState(pRadio, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pRadio->pContext != NULL && pRadio->pContext->pCapture == pRadio->pWidget ) {
				xgeXuiSetCapture(pRadio->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				xgeXuiRadioSetChecked(pRadio, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			__xgeXuiRadioSetState(pRadio, XGE_XUI_STATE_NORMAL);
			if ( pRadio->pContext != NULL && pRadio->pContext->pCapture == pRadio->pWidget ) {
				xgeXuiSetCapture(pRadio->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pRadio->pContext == NULL) || (pRadio->pContext->pFocus != pRadio->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiRadioSetChecked(pRadio, 1);
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiRadioEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiRadioEvent((xge_xui_radio)pUser, pEvent);
}

void xgeXuiRadioPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_radio pRadio;
	xge_rect_t tBox;
	xge_rect_t tMark;
	xge_rect_t tText;
	float fSize;
	uint32_t iColor;

	pRadio = (xge_xui_radio)pUser;
	if ( (pWidget == NULL) || (pRadio == NULL) ) {
		return;
	}
	iColor = __xgeXuiRadioColor(pRadio);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	fSize = pWidget->tContentRect.fH;
	if ( fSize > 18.0f ) {
		fSize = 18.0f;
	}
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	tBox.fX = pWidget->tContentRect.fX;
	tBox.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fSize) * 0.5f;
	tBox.fW = fSize;
	tBox.fH = fSize;
	__xgeXuiHostDrawRect(tBox, pRadio->iColorRing);
	if ( pRadio->bChecked ) {
		tMark = tBox;
		tMark.fX += 5.0f;
		tMark.fY += 5.0f;
		tMark.fW -= 10.0f;
		tMark.fH -= 10.0f;
		if ( (tMark.fW > 0.0f) && (tMark.fH > 0.0f) ) {
			__xgeXuiHostDrawRect(tMark, pRadio->iColorChecked);
		}
	}
	if ( (pRadio->pFont != NULL) && (pRadio->sText != NULL) && (pRadio->sText[0] != 0) ) {
		tText = pWidget->tContentRect;
		tText.fX += fSize + 6.0f;
		tText.fW -= fSize + 6.0f;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pRadio->pFont, pRadio->sText, tText, pRadio->iTextColor, pRadio->iTextFlags);
		}
	}
}
