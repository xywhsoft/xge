static void __xgeXuiSplitterSetState(xge_xui_splitter pSplitter, int iState)
{
	if ( pSplitter == NULL ) {
		return;
	}
	if ( (pSplitter->pWidget == NULL) || ((pSplitter->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pSplitter->pContext != NULL && pSplitter->pContext->pFocus == pSplitter->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pSplitter->iState != iState ) {
		pSplitter->iState = iState;
		xgeXuiWidgetMarkPaint(pSplitter->pWidget);
	}
}

static uint32_t __xgeXuiSplitterColor(xge_xui_splitter pSplitter)
{
	if ( pSplitter == NULL ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( (pSplitter->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pSplitter->iColorDisabled;
	}
	if ( (pSplitter->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		return pSplitter->iColorActive;
	}
	if ( (pSplitter->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		return pSplitter->iColorHover;
	}
	if ( (pSplitter->iState & XGE_XUI_STATE_FOCUS) != 0 ) {
		return pSplitter->iColorFocus;
	}
	return pSplitter->iColorNormal;
}

static float __xgeXuiSplitterClamp(xge_xui_splitter pSplitter, float fValue)
{
	if ( pSplitter == NULL ) {
		return fValue;
	}
	if ( fValue < pSplitter->fMin ) {
		return pSplitter->fMin;
	}
	if ( fValue > pSplitter->fMax ) {
		return pSplitter->fMax;
	}
	return fValue;
}

static void __xgeXuiSplitterSetValueInternal(xge_xui_splitter pSplitter, float fValue, int bNotify)
{
	if ( pSplitter == NULL ) {
		return;
	}
	fValue = __xgeXuiSplitterClamp(pSplitter, fValue);
	if ( pSplitter->fValue != fValue ) {
		pSplitter->fValue = fValue;
		pSplitter->iChangeCount++;
		xgeXuiWidgetMarkPaint(pSplitter->pWidget);
		if ( bNotify && pSplitter->procChange != NULL ) {
			pSplitter->procChange(pSplitter->pWidget, pSplitter->fValue, pSplitter->pUser);
		}
	}
}

int xgeXuiSplitterInit(xge_xui_splitter pSplitter, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pSplitter == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSplitter, 0, sizeof(*pSplitter));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pSplitter->pContext = pContext;
	pSplitter->pWidget = pWidget;
	pSplitter->fMin = -10000.0f;
	pSplitter->fMax = 10000.0f;
	pSplitter->iColorNormal = pTheme->iBorderColor;
	pSplitter->iColorHover = pTheme->iStateHover;
	pSplitter->iColorActive = pTheme->iStateActive;
	pSplitter->iColorFocus = pTheme->iStateFocus;
	pSplitter->iColorDisabled = pTheme->iStateDisabled;
	pSplitter->iOrientation = XGE_XUI_SEPARATOR_VERTICAL;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiSplitterEventProc, NULL);
	pWidget->procPaint = xgeXuiSplitterPaintProc;
	pWidget->pUser = pSplitter;
	__xgeXuiSplitterSetState(pSplitter, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiSplitterUnit(xge_xui_splitter pSplitter)
{
	if ( pSplitter == NULL ) {
		return;
	}
	if ( pSplitter->pWidget != NULL && pSplitter->pWidget->pUser == pSplitter ) {
		pSplitter->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pSplitter->pWidget, NULL, NULL);
		pSplitter->pWidget->procPaint = NULL;
	}
	memset(pSplitter, 0, sizeof(*pSplitter));
}

void xgeXuiSplitterSetChange(xge_xui_splitter pSplitter, xge_xui_slider_proc procChange, void* pUser)
{
	if ( pSplitter == NULL ) {
		return;
	}
	pSplitter->procChange = procChange;
	pSplitter->pUser = pUser;
}

void xgeXuiSplitterSetRange(xge_xui_splitter pSplitter, float fMin, float fMax)
{
	float fSwap;

	if ( pSplitter == NULL ) {
		return;
	}
	if ( fMax < fMin ) {
		fSwap = fMin;
		fMin = fMax;
		fMax = fSwap;
	}
	if ( fMax == fMin ) {
		fMax = fMin + 1.0f;
	}
	pSplitter->fMin = fMin;
	pSplitter->fMax = fMax;
	__xgeXuiSplitterSetValueInternal(pSplitter, pSplitter->fValue, 0);
	xgeXuiWidgetMarkPaint(pSplitter->pWidget);
}

void xgeXuiSplitterSetValue(xge_xui_splitter pSplitter, float fValue)
{
	__xgeXuiSplitterSetValueInternal(pSplitter, fValue, 0);
}

float xgeXuiSplitterGetValue(xge_xui_splitter pSplitter)
{
	if ( pSplitter == NULL ) {
		return 0.0f;
	}
	return pSplitter->fValue;
}

void xgeXuiSplitterSetOrientation(xge_xui_splitter pSplitter, int iOrientation)
{
	if ( pSplitter == NULL ) {
		return;
	}
	pSplitter->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? XGE_XUI_SEPARATOR_HORIZONTAL : XGE_XUI_SEPARATOR_VERTICAL;
	xgeXuiWidgetMarkPaint(pSplitter->pWidget);
}

void xgeXuiSplitterSetColors(xge_xui_splitter pSplitter, uint32_t iNormal, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pSplitter == NULL ) {
		return;
	}
	pSplitter->iColorNormal = iNormal;
	pSplitter->iColorHover = iHover;
	pSplitter->iColorActive = iActive;
	pSplitter->iColorFocus = iFocus;
	pSplitter->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pSplitter->pWidget);
}

int xgeXuiSplitterGetState(xge_xui_splitter pSplitter)
{
	if ( pSplitter == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiSplitterSetState(pSplitter, pSplitter->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pSplitter->iState;
}

int xgeXuiSplitterEvent(xge_xui_splitter pSplitter, const xge_event_t* pEvent)
{
	int iInside;
	int iState;
	int bWasActive;
	float fMouse;

	if ( (pSplitter == NULL) || (pSplitter->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pSplitter->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pSplitter->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiSplitterSetState(pSplitter, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pSplitter->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pSplitter->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	fMouse = (pSplitter->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? pEvent->fY : pEvent->fX;

	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			if ( (pSplitter->pContext != NULL) && (xgeXuiGetPointerCapture(pSplitter->pContext, pEvent->iPointerId) == pSplitter->pWidget) ) {
				iState |= XGE_XUI_STATE_ACTIVE;
				__xgeXuiSplitterSetValueInternal(pSplitter, pSplitter->fDragStartValue + (fMouse - pSplitter->fDragStartMouse), 1);
				__xgeXuiSplitterSetState(pSplitter, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiSplitterSetState(pSplitter, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiSplitterSetState(pSplitter, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( (pSplitter->pContext != NULL) && (xgeXuiWidgetHasCapture(pSplitter->pContext, pSplitter->pWidget) != 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiSplitterSetState(pSplitter, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiSplitterSetState(pSplitter, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pSplitter->pContext, pSplitter->pWidget);
			xgeXuiSetPointerCapture(pSplitter->pContext, pEvent->iPointerId, pSplitter->pWidget);
			pSplitter->fDragStartMouse = fMouse;
			pSplitter->fDragStartValue = pSplitter->fValue;
			__xgeXuiSplitterSetState(pSplitter, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pSplitter->iState & XGE_XUI_STATE_ACTIVE) != 0);
			if ( bWasActive ) {
				__xgeXuiSplitterSetValueInternal(pSplitter, pSplitter->fDragStartValue + (fMouse - pSplitter->fDragStartMouse), 1);
			}
			__xgeXuiSplitterSetState(pSplitter, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pSplitter->pContext != NULL && xgeXuiGetPointerCapture(pSplitter->pContext, pEvent->iPointerId) == pSplitter->pWidget ) {
				xgeXuiSetPointerCapture(pSplitter->pContext, pEvent->iPointerId, NULL);
			}
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			__xgeXuiSplitterSetState(pSplitter, XGE_XUI_STATE_NORMAL);
			if ( pSplitter->pContext != NULL && xgeXuiGetPointerCapture(pSplitter->pContext, pEvent->iPointerId) == pSplitter->pWidget ) {
				xgeXuiSetPointerCapture(pSplitter->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiSplitterEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiSplitterEvent((xge_xui_splitter)pUser, pEvent);
}

void xgeXuiSplitterPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_splitter pSplitter;
	xge_rect_t tBar;
	uint32_t iColor;

	pSplitter = (xge_xui_splitter)pUser;
	if ( (pWidget == NULL) || (pSplitter == NULL) ) {
		return;
	}
	iColor = __xgeXuiSplitterColor(pSplitter);
	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	tBar = pWidget->tContentRect;
	if ( pSplitter->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		if ( tBar.fH > 4.0f ) {
			tBar.fY += (tBar.fH - 4.0f) * 0.5f;
			tBar.fH = 4.0f;
		}
	} else {
		if ( tBar.fW > 4.0f ) {
			tBar.fX += (tBar.fW - 4.0f) * 0.5f;
			tBar.fW = 4.0f;
		}
	}
	if ( (tBar.fW > 0.0f) && (tBar.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(tBar, iColor);
	}
}
