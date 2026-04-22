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
	pToggle->iColorNormal = pTheme->iStateNormal;
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
			__xgeXuiToggleSetState(pToggle, XGE_XUI_STATE_NORMAL);
			if ( pToggle->pContext != NULL && pToggle->pContext->pCapture == pToggle->pWidget ) {
				xgeXuiSetCapture(pToggle->pContext, NULL);
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
	xge_xui_toggle pToggle;
	xge_rect_t tBox;
	xge_rect_t tText;
	float fBoxSize;
	uint32_t iColor;

	pToggle = (xge_xui_toggle)pUser;
	if ( (pWidget == NULL) || (pToggle == NULL) ) {
		return;
	}
	iColor = __xgeXuiToggleColor(pToggle);
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
	__xgeXuiHostDrawRect(tBox, pToggle->bChecked ? pToggle->iColorChecked : XGE_COLOR_RGBA(180, 186, 196, 255));
	if ( pToggle->bChecked ) {
		tBox.fX += 4.0f;
		tBox.fY += 4.0f;
		tBox.fW -= 8.0f;
		tBox.fH -= 8.0f;
		if ( (tBox.fW > 0.0f) && (tBox.fH > 0.0f) ) {
			__xgeXuiHostDrawRect(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
		}
	}
	if ( (pToggle->pFont != NULL) && (pToggle->sText != NULL) && (pToggle->sText[0] != 0) ) {
		tText = pWidget->tContentRect;
		tText.fX += fBoxSize + 6.0f;
		tText.fW -= fBoxSize + 6.0f;
		if ( tText.fW > 0.0f ) {
			__xgeXuiHostDrawTextRect(pToggle->pFont, pToggle->sText, tText, pToggle->iTextColor, pToggle->iTextFlags);
		}
	}
}

int xgeXuiSliderInit(xge_xui_slider pSlider, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pSlider == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSlider, 0, sizeof(*pSlider));
	pTheme = xgeXuiGetTheme(pContext);
	pSlider->pContext = pContext;
	pSlider->pWidget = pWidget;
	pSlider->fMin = 0.0f;
	pSlider->fMax = 1.0f;
	pSlider->fValue = 0.0f;
	pSlider->iColorTrack = pTheme->iBorderColor;
	pSlider->iColorFill = pTheme->iAccentColor;
	pSlider->iColorKnob = pTheme->iBackgroundColor;
	pSlider->iColorFocus = pTheme->iStateFocus;
	pSlider->iColorDisabled = pTheme->iStateDisabled;
	pWidget->tStyle.fRadius = pTheme->fRadius;
	xgeXuiWidgetSetFocusable(pWidget, 1);
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
			if ( (pSlider->pContext != NULL) && (pSlider->pContext->pCapture == pSlider->pWidget) ) {
				iState |= XGE_XUI_STATE_ACTIVE;
				__xgeXuiSliderSetValueFromPoint(pSlider, pEvent->fX, 1);
				__xgeXuiSliderSetState(pSlider, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiSliderSetState(pSlider, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSlider->pContext, pSlider->pWidget);
			xgeXuiSetCapture(pSlider->pContext, pSlider->pWidget);
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
			if ( pSlider->pContext != NULL && pSlider->pContext->pCapture == pSlider->pWidget ) {
				xgeXuiSetCapture(pSlider->pContext, NULL);
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
			__xgeXuiSliderSetState(pSlider, XGE_XUI_STATE_NORMAL);
			if ( pSlider->pContext != NULL && pSlider->pContext->pCapture == pSlider->pWidget ) {
				xgeXuiSetCapture(pSlider->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

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
	if ( tTrack.fH > 6.0f ) {
		tTrack.fY += (tTrack.fH - 6.0f) * 0.5f;
		tTrack.fH = 6.0f;
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
	if ( fKnobSize > 18.0f ) {
		fKnobSize = 18.0f;
	}
	if ( fKnobSize < 6.0f ) {
		fKnobSize = 6.0f;
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
	__xgeXuiHostDrawRect(tKnob, pSlider->iColorKnob);
}

int xgeXuiProgressInit(xge_xui_progress pProgress, xge_xui_widget pWidget)
{
	if ( (pProgress == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pProgress, 0, sizeof(*pProgress));
	pProgress->pWidget = pWidget;
	pProgress->sText = "";
	pProgress->fMin = 0.0f;
	pProgress->fMax = 1.0f;
	pProgress->fValue = 0.0f;
	pProgress->iColorTrack = XGE_COLOR_RGBA(92, 100, 112, 255);
	pProgress->iColorFill = XGE_COLOR_RGBA(62, 172, 110, 255);
	pProgress->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pProgress->iTextFlags = XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiProgressPaintProc;
	pWidget->pUser = pProgress;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiProgressUnit(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return;
	}
	if ( pProgress->pWidget != NULL && pProgress->pWidget->pUser == pProgress ) {
		pProgress->pWidget->pUser = NULL;
		pProgress->pWidget->procPaint = NULL;
	}
	memset(pProgress, 0, sizeof(*pProgress));
}

void xgeXuiProgressSetRange(xge_xui_progress pProgress, float fMin, float fMax)
{
	if ( pProgress == NULL ) {
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
	pProgress->fMin = fMin;
	pProgress->fMax = fMax;
	__xgeXuiProgressSetValueInternal(pProgress, pProgress->fValue);
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetValue(xge_xui_progress pProgress, float fValue)
{
	__xgeXuiProgressSetValueInternal(pProgress, fValue);
}

float xgeXuiProgressGetValue(xge_xui_progress pProgress)
{
	if ( pProgress == NULL ) {
		return 0.0f;
	}
	return pProgress->fValue;
}

void xgeXuiProgressSetText(xge_xui_progress pProgress, xge_font pFont, const char* sText)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->pFont = pFont;
	pProgress->sText = (sText != NULL) ? sText : "";
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetTextColor(xge_xui_progress pProgress, uint32_t iColor)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iTextColor = iColor;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressSetColors(xge_xui_progress pProgress, uint32_t iTrack, uint32_t iFill)
{
	if ( pProgress == NULL ) {
		return;
	}
	pProgress->iColorTrack = iTrack;
	pProgress->iColorFill = iFill;
	xgeXuiWidgetMarkPaint(pProgress->pWidget);
}

void xgeXuiProgressPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_progress pProgress;
	xge_rect_t tFill;

	pProgress = (xge_xui_progress)pUser;
	if ( (pWidget == NULL) || (pProgress == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pProgress->iColorTrack) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tContentRect, pProgress->iColorTrack);
	}
	tFill = pWidget->tContentRect;
	tFill.fW *= __xgeXuiProgressRate(pProgress);
	if ( (tFill.fW > 0.0f) && (XGE_COLOR_GET_A(pProgress->iColorFill) != 0) ) {
		__xgeXuiHostDrawRect(tFill, pProgress->iColorFill);
	}
	if ( (pProgress->pFont != NULL) && (pProgress->sText != NULL) && (pProgress->sText[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pProgress->pFont, pProgress->sText, pWidget->tContentRect, pProgress->iTextColor, pProgress->iTextFlags);
	}
}

int xgeXuiPanelInit(xge_xui_panel pPanel, xge_xui_widget pWidget)
{
	if ( (pPanel == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPanel, 0, sizeof(*pPanel));
	pPanel->pWidget = pWidget;
	pPanel->sTitle = "";
	pPanel->iBackgroundColor = XGE_COLOR_RGBA(32, 38, 46, 255);
	pPanel->iTitleColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPanel->iTitleFlags = XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_TOP | XGE_TEXT_CLIP;
	pWidget->procPaint = xgeXuiPanelPaintProc;
	pWidget->pUser = pPanel;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPanelUnit(xge_xui_panel pPanel)
{
	if ( pPanel == NULL ) {
		return;
	}
	if ( pPanel->pWidget != NULL && pPanel->pWidget->pUser == pPanel ) {
		pPanel->pWidget->pUser = NULL;
		pPanel->pWidget->procPaint = NULL;
	}
	memset(pPanel, 0, sizeof(*pPanel));
}

void xgeXuiPanelSetBackground(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iBackgroundColor = iColor;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitle(xge_xui_panel pPanel, xge_font pFont, const char* sTitle)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->pFont = pFont;
	pPanel->sTitle = (sTitle != NULL) ? sTitle : "";
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleColor(xge_xui_panel pPanel, uint32_t iColor)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleColor = iColor;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetTitleAlign(xge_xui_panel pPanel, uint32_t iTextFlags)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->iTitleFlags = iTextFlags | XGE_TEXT_CLIP;
	xgeXuiWidgetMarkPaint(pPanel->pWidget);
}

void xgeXuiPanelSetClip(xge_xui_panel pPanel, int bClip)
{
	if ( pPanel == NULL ) {
		return;
	}
	pPanel->bClip = bClip ? 1 : 0;
	xgeXuiWidgetSetClip(pPanel->pWidget, pPanel->bClip);
}

void xgeXuiPanelPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_panel pPanel;

	pPanel = (xge_xui_panel)pUser;
	if ( (pWidget == NULL) || (pPanel == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pPanel->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pPanel->iBackgroundColor);
	}
	if ( (pPanel->pFont != NULL) && (pPanel->sTitle != NULL) && (pPanel->sTitle[0] != 0) ) {
		__xgeXuiHostDrawTextRect(pPanel->pFont, pPanel->sTitle, pWidget->tContentRect, pPanel->iTitleColor, pPanel->iTitleFlags);
	}
}
