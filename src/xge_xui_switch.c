static void __xgeXuiSwitchSetState(xge_xui_switch pSwitch, int iState)
{
	if ( pSwitch == NULL ) {
		return;
	}
	if ( (pSwitch->pWidget == NULL) || ((pSwitch->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pSwitch->pContext != NULL && pSwitch->pContext->pFocus == pSwitch->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pSwitch->iState != iState ) {
		pSwitch->iState = iState;
		xgeXuiWidgetMarkPaint(pSwitch->pWidget);
	}
}

static uint32_t __xgeXuiSwitchColor(xge_xui_switch pSwitch)
{
	if ( pSwitch == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pSwitch->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pSwitch->iColorDisabled;
	}
	if ( (pSwitch->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pSwitch->iColorActive;
	}
	if ( (pSwitch->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pSwitch->iColorHover;
	}
	if ( (pSwitch->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pSwitch->iColorFocus;
	}
	return pSwitch->iColorNormal;
}

static void __xgeXuiSwitchToggle(xge_xui_switch pSwitch)
{
	if ( pSwitch == NULL ) {
		return;
	}
	pSwitch->bChecked = pSwitch->bChecked ? 0 : 1;
	pSwitch->iChangeCount++;
	xgeXuiWidgetMarkPaint(pSwitch->pWidget);
	if ( pSwitch->procChange != NULL ) {
		pSwitch->procChange(pSwitch->pWidget, pSwitch->bChecked, pSwitch->pUser);
	}
}

int xgeXuiSwitchInit(xge_xui_switch pSwitch, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pSwitch == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSwitch, 0, sizeof(*pSwitch));
	pTheme = xgeXuiGetTheme(pContext);
	pSwitch->pContext = pContext;
	pSwitch->pWidget = pWidget;
	pSwitch->pFont = pTheme->pFont;
	pSwitch->sText = "";
	pSwitch->iTextColor = pTheme->iTextColor;
	pSwitch->iTextFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pSwitch->iColorNormal = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSwitch->iColorHover = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSwitch->iColorActive = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSwitch->iColorFocus = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSwitch->iColorDisabled = pTheme->iStateDisabled;
	pSwitch->iColorTrack = XGE_COLOR_RGBA(220, 231, 240, 255);
	pSwitch->iColorChecked = pTheme->iAccentColor;
	pSwitch->iColorKnob = XGE_COLOR_RGBA(255, 255, 255, 255);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	pWidget->procEvent = xgeXuiSwitchEventProc;
	pWidget->procPaint = xgeXuiSwitchPaintProc;
	pWidget->pUser = pSwitch;
	__xgeXuiSwitchSetState(pSwitch, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiSwitchUnit(xge_xui_switch pSwitch)
{
	if ( pSwitch == NULL ) {
		return;
	}
	if ( pSwitch->pWidget != NULL && pSwitch->pWidget->pUser == pSwitch ) {
		pSwitch->pWidget->pUser = NULL;
		pSwitch->pWidget->procEvent = NULL;
		pSwitch->pWidget->procPaint = NULL;
	}
	memset(pSwitch, 0, sizeof(*pSwitch));
}

void xgeXuiSwitchSetChange(xge_xui_switch pSwitch, xge_xui_toggle_proc procChange, void* pUser)
{
	if ( pSwitch == NULL ) {
		return;
	}
	pSwitch->procChange = procChange;
	pSwitch->pUser = pUser;
}

void xgeXuiSwitchSetText(xge_xui_switch pSwitch, xge_font pFont, const char* sText)
{
	if ( pSwitch == NULL ) {
		return;
	}
	pSwitch->pFont = pFont;
	pSwitch->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkLayout(pSwitch->pWidget);
	xgeXuiWidgetMarkPaint(pSwitch->pWidget);
}

void xgeXuiSwitchSetChecked(xge_xui_switch pSwitch, int bChecked)
{
	if ( pSwitch == NULL ) {
		return;
	}
	bChecked = bChecked ? 1 : 0;
	if ( pSwitch->bChecked != bChecked ) {
		pSwitch->bChecked = bChecked;
		xgeXuiWidgetMarkPaint(pSwitch->pWidget);
	}
}

int xgeXuiSwitchGetChecked(xge_xui_switch pSwitch)
{
	if ( pSwitch == NULL ) {
		return 0;
	}
	return pSwitch->bChecked;
}

void xgeXuiSwitchSetTextColor(xge_xui_switch pSwitch, uint32_t iColor)
{
	if ( pSwitch == NULL ) {
		return;
	}
	pSwitch->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pSwitch->pWidget);
}

void xgeXuiSwitchSetColors(xge_xui_switch pSwitch, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled, uint32_t iTrack, uint32_t iChecked, uint32_t iKnob)
{
	if ( pSwitch == NULL ) {
		return;
	}
	pSwitch->iColorNormal = iNormal;
	pSwitch->iColorHover = iHover;
	pSwitch->iColorActive = iActive;
	pSwitch->iColorFocus = iFocus;
	pSwitch->iColorDisabled = iDisabled;
	pSwitch->iColorTrack = iTrack;
	pSwitch->iColorChecked = iChecked;
	pSwitch->iColorKnob = iKnob;
	xgeXuiWidgetMarkPaint(pSwitch->pWidget);
}

int xgeXuiSwitchGetState(xge_xui_switch pSwitch)
{
	if ( pSwitch == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiSwitchSetState(pSwitch, pSwitch->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pSwitch->iState;
}

int xgeXuiSwitchEvent(xge_xui_switch pSwitch, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pSwitch == NULL) || (pSwitch->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pSwitch->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pSwitch->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiSwitchSetState(pSwitch, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pSwitch->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pSwitch->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			__xgeXuiSwitchSetState(pSwitch, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			return ((pSwitch->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiSwitchSetState(pSwitch, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiSwitchSetState(pSwitch, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiSwitchSetState(pSwitch, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSwitch->pContext, pSwitch->pWidget);
			xgeXuiSetCapture(pSwitch->pContext, pSwitch->pWidget);
			__xgeXuiSwitchSetState(pSwitch, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pSwitch->iState & XGE_XUI_STATE_ACTIVE) != 0);
			__xgeXuiSwitchSetState(pSwitch, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pSwitch->pContext != NULL && pSwitch->pContext->pCapture == pSwitch->pWidget ) {
				xgeXuiSetCapture(pSwitch->pContext, NULL);
			}
			if ( bWasActive && iInside ) {
				__xgeXuiSwitchToggle(pSwitch);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			__xgeXuiSwitchSetState(pSwitch, XGE_XUI_STATE_NORMAL);
			if ( pSwitch->pContext != NULL && pSwitch->pContext->pCapture == pSwitch->pWidget ) {
				xgeXuiSetCapture(pSwitch->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pSwitch->pContext == NULL) || (pSwitch->pContext->pFocus != pSwitch->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 != XGE_KEY_ENTER) && (pEvent->iParam1 != XGE_KEY_SPACE) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiSwitchToggle(pSwitch);
			return XGE_XUI_EVENT_CONSUMED;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiSwitchEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiSwitchEvent((xge_xui_switch)pUser, pEvent);
}

void xgeXuiSwitchPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_switch pSwitch;
	xge_rect_t tTrack;
	xge_rect_t tTrackMid;
	xge_rect_t tText;
	float fTrackW;
	float fTrackH;
	float fRadius;
	float fCenterY;
	float fKnobRadius;
	float fKnobMargin;
	float fKnobX;
	uint32_t iColor;
	uint32_t iTrackColor;

	pSwitch = (xge_xui_switch)pUser;
	if ( (pWidget == NULL) || (pSwitch == NULL) ) {
		return;
	}
	iColor = __xgeXuiSwitchColor(pSwitch);
	if ( XGE_COLOR_GET_A(iColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, iColor);
	}
	fTrackH = pWidget->tContentRect.fH * 0.72f;
	if ( fTrackH > 28.0f ) {
		fTrackH = 28.0f;
	}
	if ( fTrackH < 14.0f ) {
		fTrackH = 14.0f;
	}
	fTrackW = fTrackH * 2.36f;
	tTrack.fX = pWidget->tContentRect.fX;
	tTrack.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fTrackH) * 0.5f;
	tTrack.fW = fTrackW;
	tTrack.fH = fTrackH;
	fRadius = fTrackH * 0.5f;
	fCenterY = tTrack.fY + fRadius;
	iTrackColor = pSwitch->bChecked ? pSwitch->iColorChecked : pSwitch->iColorTrack;
	tTrackMid.fX = tTrack.fX + fRadius;
	tTrackMid.fY = tTrack.fY;
	tTrackMid.fW = tTrack.fW - fRadius * 2.0f;
	tTrackMid.fH = tTrack.fH;
	if ( tTrackMid.fW > 0.0f ) {
		__xgeXuiHostDrawRect(tTrackMid, iTrackColor);
	}
	xgeShapeCircleFillPx(tTrack.fX + fRadius, fCenterY, fRadius, iTrackColor);
	xgeShapeCircleFillPx(tTrack.fX + tTrack.fW - fRadius, fCenterY, fRadius, iTrackColor);
	xgeShapeLinePx(tTrack.fX + fRadius, tTrack.fY + 1.0f, tTrack.fX + tTrack.fW - fRadius, tTrack.fY + 1.0f, 1.0f, pSwitch->bChecked ? pSwitch->iColorChecked : XGE_COLOR_RGBA(176, 198, 216, 220));
	xgeShapeLinePx(tTrack.fX + fRadius, tTrack.fY + tTrack.fH - 1.0f, tTrack.fX + tTrack.fW - fRadius, tTrack.fY + tTrack.fH - 1.0f, 1.0f, pSwitch->bChecked ? pSwitch->iColorChecked : XGE_COLOR_RGBA(176, 198, 216, 220));
	xgeShapeCircleStrokePx(tTrack.fX + fRadius, fCenterY, fRadius - 0.5f, 1.0f, pSwitch->bChecked ? pSwitch->iColorChecked : XGE_COLOR_RGBA(176, 198, 216, 220));
	xgeShapeCircleStrokePx(tTrack.fX + tTrack.fW - fRadius, fCenterY, fRadius - 0.5f, 1.0f, pSwitch->bChecked ? pSwitch->iColorChecked : XGE_COLOR_RGBA(176, 198, 216, 220));
	fKnobRadius = fTrackH * 0.39f - 1.0f;
	if ( fKnobRadius < 4.0f ) {
		fKnobRadius = 4.0f;
	}
	fKnobMargin = (fTrackH - fKnobRadius * 2.0f) * 0.5f;
	if ( fKnobMargin < 2.0f ) {
		fKnobMargin = 2.0f;
	}
	fKnobX = pSwitch->bChecked ? (tTrack.fX + tTrack.fW - fKnobMargin - fKnobRadius) : (tTrack.fX + fKnobMargin + fKnobRadius);
	xgeShapeCircleFillPx(fKnobX, fCenterY, fKnobRadius, pSwitch->iColorKnob);
	xgeShapeCircleStrokePx(fKnobX, fCenterY, fKnobRadius, 1.0f, pSwitch->bChecked ? XGE_COLOR_RGBA(127, 196, 229, 180) : XGE_COLOR_RGBA(172, 194, 212, 220));
	if ( (pSwitch->pFont != NULL) && (pSwitch->sText != NULL) && (pSwitch->sText[0] != 0) ) {
		tText = pWidget->tContentRect;
		tText.fX += fTrackW + 8.0f;
		tText.fW -= fTrackW + 8.0f;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pSwitch->pFont, pSwitch->sText, tText, pSwitch->iTextColor, pSwitch->iTextFlags);
		}
	}
}
