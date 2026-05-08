int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pSlider == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSlider, 0, sizeof(*pSlider));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pSlider->pContext = pContext;
	pSlider->pWidget = pWidget;
	pSlider->fMin = 0.0f;
	pSlider->fMax = 1.0f;
	pSlider->fValue = 0.0f;
	pSlider->iColorTrack = pTheme->iBorderColor;
	pSlider->iColorFill = pTheme->iAccentColor;
	pSlider->iColorKnob = pTheme->iBackgroundColor;
	pSlider->iColorFocus = XGE_COLOR_RGBA(0, 0, 0, 0);
	pSlider->iColorDisabled = pTheme->iStateDisabled;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	pWidget->procEvent = xgeXuiSliderEventProc;
	pWidget->procPaint = xgeXuiSliderPaintProc;
	pWidget->pUser = pSlider;
	__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiSliderUnit(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return;
	}
	if ( pSlider->pWidget != NULL && pSlider->pWidget->pUser == pSlider ) {
		pSlider->pWidget->pUser = NULL;
		pSlider->pWidget->procEvent = NULL;
		pSlider->pWidget->procPaint = NULL;
	}
	memset(pSlider, 0, sizeof(*pSlider));
}

void xgeXuiSliderSetChange(xge_xui_slider pSlider, xge_xui_slider_proc procChange, void* pUser)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->procChange = procChange;
	pSlider->pUser = pUser;
}

void xgeXuiSliderSetRange(xge_xui_slider pSlider, float fMin, float fMax)
{
	if ( pSlider == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		float fSwap;
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pSlider->fMin = fMin;
	pSlider->fMax = fMax;
	__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue, 0);
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
}

void xgeXuiSliderSetValue(xge_xui_slider pSlider, float fValue)
{
	__xgeXuiSliderSetValueInternal(pSlider, fValue, 0);
}

float xgeXuiSliderGetValue(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return 0.0f;
	}
	return pSlider->fValue;
}

void xgeXuiSliderSetColors(xge_xui_slider pSlider, uint32_t iTrack, uint32_t iFill, uint32_t iKnob, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pSlider == NULL ) {
		return;
	}
	pSlider->iColorTrack = iTrack;
	pSlider->iColorFill = iFill;
	pSlider->iColorKnob = iKnob;
	pSlider->iColorFocus = iFocus;
	pSlider->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pSlider->pWidget);
}

int xgeXuiSliderGetState(xge_xui_slider pSlider)
{
	if ( pSlider == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiSliderSetState(pSlider, pSlider->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pSlider->iState;
}

int xgeXuiSliderEvent(xge_xui_slider pSlider, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;

	if ( (pSlider == NULL) || (pSlider->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pSlider->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pSlider->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pSlider->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pSlider->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			if ( (pSlider->pContext != NULL) && (xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget) ) {
				iState |= XGE_XUI_STATE_ACTIVE;
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
				__xgeXuiSliderSetState(pSlider, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiSliderSetState(pSlider, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiSliderSetState(pSlider, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiSliderSetState(pSlider, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiSliderSetState(pSlider, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSlider->pContext, pSlider->pWidget);
			xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, pSlider->pWidget);
			__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pSlider->iState & XGE_XUI_STATE_ACTIVE) != 0);
			if ( bWasActive ) {
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
			}
			iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			__xgeXuiSliderSetState(pSlider, iState);
			if ( pSlider->pContext != NULL && xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget ) {
				xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, NULL);
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
			if ( pSlider->pContext != NULL && xgeXuiGetPointerCapture(pSlider->pContext, pEvent->iPointerId) == pSlider->pWidget ) {
				xgeXuiSetPointerCapture(pSlider->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pSlider->pContext == NULL) || (pSlider->pContext->pFocus != pSlider->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_LEFT) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue - (pSlider->fMax - pSlider->fMin) * 0.01f, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_RIGHT) || (pEvent->iParam1 == XGE_KEY_UP) ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue + (pSlider->fMax - pSlider->fMin) * 0.01f, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue - (pSlider->fMax - pSlider->fMin) * 0.10f, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fValue + (pSlider->fMax - pSlider->fMin) * 0.10f, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fMin, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_END ) {
				__xgeXuiSliderSetValueInternal(pSlider, pSlider->fMax, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiSliderEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiSliderEvent((xge_xui_slider)pUser, pEvent);
}

void xgeXuiSliderPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_slider pSlider;
	xge_rect_t tTrack;
	xge_rect_t tFill;
	xge_rect_t tKnob;
	float fRate;
	float fKnobSize;

	pSlider = (xge_xui_slider)pUser;
	if ( (pWidget == NULL) || (pSlider == NULL) ) {
		return;
	}
	tTrack = pWidget->tContentRect;
	if ( tTrack.fH > 3.0f ) {
		tTrack.fY += (tTrack.fH - 3.0f) * 0.5f;
		tTrack.fH = 3.0f;
	}
	if ( (pSlider->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		__xgeXuiHostDrawRect(tTrack, pSlider->iColorDisabled);
		return;
	}
	if ( (pSlider->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pSlider->iColorFocus);
	}
	__xgeXuiHostDrawRect(tTrack, pSlider->iColorTrack);
	fRate = __xgeXuiSliderRate(pSlider);
	tFill = tTrack;
	tFill.fW *= fRate;
	if ( tFill.fW > 0.0f ) {
		__xgeXuiHostDrawRect(tFill, pSlider->iColorFill);
	}
	fKnobSize = pWidget->tContentRect.fH;
	if ( fKnobSize > 11.0f ) {
		fKnobSize = 11.0f;
	}
	if ( fKnobSize < 7.0f ) {
		fKnobSize = 7.0f;
	}
	tKnob.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW * fRate - fKnobSize * 0.5f;
	tKnob.fY = pWidget->tContentRect.fY + (pWidget->tContentRect.fH - fKnobSize) * 0.5f;
	tKnob.fW = fKnobSize;
	tKnob.fH = fKnobSize;
	if ( tKnob.fX < pWidget->tContentRect.fX ) {
		tKnob.fX = pWidget->tContentRect.fX;
	}
	if ( (tKnob.fX + tKnob.fW) > (pWidget->tContentRect.fX + pWidget->tContentRect.fW) ) {
		tKnob.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - tKnob.fW;
	}
	xgeShapeCircleFillPx(tKnob.fX + tKnob.fW * 0.5f, tKnob.fY + tKnob.fH * 0.5f, tKnob.fW * 0.5f, pSlider->iColorFill);
	xgeShapeCircleStrokePx(tKnob.fX + tKnob.fW * 0.5f, tKnob.fY + tKnob.fH * 0.5f, tKnob.fW * 0.5f, 1.0f, pSlider->iColorKnob);
}
