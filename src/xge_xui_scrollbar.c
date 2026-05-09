static void __xgeXuiScrollBarSetState(xge_xui_scrollbar pScrollBar, int iState)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	if ( (pScrollBar->pWidget == NULL) || ((pScrollBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pScrollBar->pContext != NULL && pScrollBar->pContext->pFocus == pScrollBar->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pScrollBar->iState != iState ) {
		pScrollBar->iState = iState;
		xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
	}
}

static float __xgeXuiScrollBarRange(xge_xui_scrollbar pScrollBar)
{
	if ( pScrollBar == NULL ) {
		return 0.0f;
	}
	return (pScrollBar->fMax > pScrollBar->fMin) ? (pScrollBar->fMax - pScrollBar->fMin) : 0.0f;
}

static float __xgeXuiScrollBarStep(xge_xui_scrollbar pScrollBar)
{
	if ( (pScrollBar != NULL) && (pScrollBar->fPage > 0.0f) ) {
		return pScrollBar->fPage;
	}
	return 1.0f;
}

static int __xgeXuiScrollBarSetValueInternal(xge_xui_scrollbar pScrollBar, float fValue, int bNotify)
{
	if ( pScrollBar == NULL ) {
		return 0;
	}
	fValue = __xgeXuiClampFloat(fValue, pScrollBar->fMin, pScrollBar->fMax);
	if ( pScrollBar->fValue == fValue ) {
		return 0;
	}
	pScrollBar->fValue = fValue;
	pScrollBar->iChangeCount++;
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
	if ( bNotify && pScrollBar->procChange != NULL ) {
		pScrollBar->procChange(pScrollBar->pWidget, pScrollBar->fValue, pScrollBar->pUser);
	}
	return 1;
}

static float __xgeXuiScrollBarAxisPos(xge_xui_scrollbar pScrollBar, const xge_event_t* pEvent)
{
	if ( (pScrollBar != NULL) && (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ) {
		return pEvent->fX;
	}
	return pEvent->fY;
}

static float __xgeXuiScrollBarTrackLen(xge_xui_scrollbar pScrollBar, xge_rect_t tTrack)
{
	if ( (pScrollBar != NULL) && (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ) {
		return tTrack.fW;
	}
	return tTrack.fH;
}

static float __xgeXuiScrollBarButtonSize(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tRect;
	float fLong;
	float fCross;
	float fSize;

	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		return 0.0f;
	}
	tRect = pScrollBar->pWidget->tContentRect;
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		fLong = tRect.fW;
		fCross = tRect.fH;
	} else {
		fLong = tRect.fH;
		fCross = tRect.fW;
	}
	if ( fLong < 32.0f || fCross < 8.0f ) {
		return 0.0f;
	}
	fSize = fCross;
	if ( fSize > 18.0f ) {
		fSize = 18.0f;
	}
	return fSize;
}

static xge_rect_t __xgeXuiScrollBarTrackRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	float fButton;

	tTrack = pScrollBar->pWidget->tContentRect;
	fButton = __xgeXuiScrollBarButtonSize(pScrollBar);
	if ( fButton <= 0.0f ) {
		return tTrack;
	}
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tTrack.fX += fButton;
		tTrack.fW -= fButton * 2.0f;
	} else {
		tTrack.fY += fButton;
		tTrack.fH -= fButton * 2.0f;
	}
	if ( tTrack.fW < 0.0f ) {
		tTrack.fW = 0.0f;
	}
	if ( tTrack.fH < 0.0f ) {
		tTrack.fH = 0.0f;
	}
	return tTrack;
}

static xge_rect_t __xgeXuiScrollBarButtonRect(xge_xui_scrollbar pScrollBar, int bEnd)
{
	xge_rect_t tRect;
	float fButton;

	tRect = pScrollBar->pWidget->tContentRect;
	fButton = __xgeXuiScrollBarButtonSize(pScrollBar);
	if ( fButton <= 0.0f ) {
		tRect.fW = 0.0f;
		tRect.fH = 0.0f;
		return tRect;
	}
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tRect.fW = fButton;
		if ( bEnd ) {
			tRect.fX = pScrollBar->pWidget->tContentRect.fX + pScrollBar->pWidget->tContentRect.fW - fButton;
		}
	} else {
		tRect.fH = fButton;
		if ( bEnd ) {
			tRect.fY = pScrollBar->pWidget->tContentRect.fY + pScrollBar->pWidget->tContentRect.fH - fButton;
		}
	}
	return tRect;
}

static float __xgeXuiScrollBarTrackStart(xge_xui_scrollbar pScrollBar, xge_rect_t tTrack)
{
	if ( (pScrollBar != NULL) && (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ) {
		return tTrack.fX;
	}
	return tTrack.fY;
}

static float __xgeXuiScrollBarThumbLen(xge_xui_scrollbar pScrollBar, float fTrackLen)
{
	float fRange;
	float fTotal;
	float fLen;

	if ( (pScrollBar == NULL) || (fTrackLen <= 0.0f) ) {
		return 0.0f;
	}
	fRange = __xgeXuiScrollBarRange(pScrollBar);
	if ( fRange <= 0.0f ) {
		return fTrackLen;
	}
	fTotal = fRange + ((pScrollBar->fPage > 0.0f) ? pScrollBar->fPage : 0.0f);
	fLen = (fTotal > 0.0f) ? (fTrackLen * (pScrollBar->fPage / fTotal)) : 8.0f;
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	return fLen;
}

static xge_rect_t __xgeXuiScrollBarThumbRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	xge_rect_t tThumb;
	float fTrackLen;
	float fThumbLen;
	float fTravel;
	float fRate;
	float fRange;

	tTrack = __xgeXuiScrollBarTrackRect(pScrollBar);
	tThumb = tTrack;
	fTrackLen = __xgeXuiScrollBarTrackLen(pScrollBar, tTrack);
	fThumbLen = __xgeXuiScrollBarThumbLen(pScrollBar, fTrackLen);
	fTravel = fTrackLen - fThumbLen;
	fRange = __xgeXuiScrollBarRange(pScrollBar);
	fRate = (fRange > 0.0f) ? ((pScrollBar->fValue - pScrollBar->fMin) / fRange) : 0.0f;
	fRate = __xgeXuiClampFloat(fRate, 0.0f, 1.0f);
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tThumb.fX = tTrack.fX + fTravel * fRate;
		tThumb.fW = fThumbLen;
	} else {
		tThumb.fY = tTrack.fY + fTravel * fRate;
		tThumb.fH = fThumbLen;
	}
	return tThumb;
}

static xge_rect_t __xgeXuiScrollBarVisualThumbRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tThumb;

	tThumb = __xgeXuiScrollBarThumbRect(pScrollBar);
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
			tThumb.fX += 1.0f;
			tThumb.fW -= 2.0f;
			tThumb.fY += (tThumb.fH - 4.0f) * 0.5f;
			tThumb.fH = 4.0f;
		} else {
			tThumb.fY += 1.0f;
			tThumb.fH -= 2.0f;
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
		}
	} else if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tThumb.fX += 1.0f;
		tThumb.fW -= 2.0f;
		tThumb.fY += 2.0f;
		tThumb.fH -= 4.0f;
	} else {
		tThumb.fY += 1.0f;
		tThumb.fH -= 2.0f;
		tThumb.fX += 2.0f;
		tThumb.fW -= 4.0f;
	}
	if ( tThumb.fW < 1.0f ) {
		tThumb.fW = 1.0f;
	}
	if ( tThumb.fH < 1.0f ) {
		tThumb.fH = 1.0f;
	}
	return tThumb;
}

static void __xgeXuiScrollBarSetValueFromDrag(xge_xui_scrollbar pScrollBar, float fMouse, int bNotify)
{
	xge_rect_t tTrack;
	float fTrackLen;
	float fThumbLen;
	float fTravel;
	float fRange;

	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) ) {
		return;
	}
	tTrack = __xgeXuiScrollBarTrackRect(pScrollBar);
	fTrackLen = __xgeXuiScrollBarTrackLen(pScrollBar, tTrack);
	fThumbLen = __xgeXuiScrollBarThumbLen(pScrollBar, fTrackLen);
	fTravel = fTrackLen - fThumbLen;
	fRange = __xgeXuiScrollBarRange(pScrollBar);
	if ( (fTravel <= 0.0f) || (fRange <= 0.0f) ) {
		return;
	}
	__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fDragStartValue + ((fMouse - pScrollBar->fDragStartMouse) / fTravel) * fRange, bNotify);
}

int xgeXuiScrollBarInit(xge_xui_scrollbar pScrollBar, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pScrollBar == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pScrollBar, 0, sizeof(*pScrollBar));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pScrollBar->pContext = pContext;
	pScrollBar->pWidget = pWidget;
	pScrollBar->fMin = 0.0f;
	pScrollBar->fMax = 1.0f;
	pScrollBar->fPage = 0.2f;
	pScrollBar->iColorTrack = XGE_COLOR_RGBA(255, 255, 255, 255);
	pScrollBar->iColorThumb = XGE_COLOR_RGBA(185, 208, 226, 245);
	pScrollBar->iColorHover = XGE_COLOR_RGBA(158, 185, 208, 250);
	pScrollBar->iColorActive = XGE_COLOR_RGBA(132, 160, 188, 255);
	pScrollBar->iColorFocus = XGE_COLOR_RGBA(0, 0, 0, 0);
	pScrollBar->iColorDisabled = pTheme->iStateDisabled;
	pScrollBar->iOrientation = XGE_XUI_SEPARATOR_VERTICAL;
	pScrollBar->iMode = XGE_XUI_SCROLLBAR_MODE_FULL;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiScrollBarEventProc, NULL);
	pWidget->procPaint = xgeXuiScrollBarPaintProc;
	pWidget->pUser = pScrollBar;
	__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiScrollBarUnit(xge_xui_scrollbar pScrollBar)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pScrollBar->pContext, pScrollBar->pWidget);
	if ( pScrollBar->pWidget != NULL && pScrollBar->pWidget->pUser == pScrollBar ) {
		pScrollBar->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pScrollBar->pWidget, NULL, NULL);
		pScrollBar->pWidget->procPaint = NULL;
	}
	memset(pScrollBar, 0, sizeof(*pScrollBar));
}

void xgeXuiScrollBarSetChange(xge_xui_scrollbar pScrollBar, xge_xui_slider_proc procChange, void* pUser)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->procChange = procChange;
	pScrollBar->pUser = pUser;
}

void xgeXuiScrollBarSetRange(xge_xui_scrollbar pScrollBar, float fMin, float fMax, float fPage)
{
	float fSwap;

	if ( pScrollBar == NULL ) {
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
	pScrollBar->fMin = fMin;
	pScrollBar->fMax = fMax;
	xgeXuiScrollBarSetPage(pScrollBar, fPage);
	__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue, 0);
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
}

void xgeXuiScrollBarSetPage(xge_xui_scrollbar pScrollBar, float fPage)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	if ( fPage < 0.0f ) {
		fPage = 0.0f;
	}
	pScrollBar->fPage = fPage;
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
}

void xgeXuiScrollBarSetValue(xge_xui_scrollbar pScrollBar, float fValue)
{
	__xgeXuiScrollBarSetValueInternal(pScrollBar, fValue, 0);
}

float xgeXuiScrollBarGetValue(xge_xui_scrollbar pScrollBar)
{
	if ( pScrollBar == NULL ) {
		return 0.0f;
	}
	return pScrollBar->fValue;
}

void xgeXuiScrollBarSetOrientation(xge_xui_scrollbar pScrollBar, int iOrientation)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? XGE_XUI_SEPARATOR_HORIZONTAL : XGE_XUI_SEPARATOR_VERTICAL;
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
}

void xgeXuiScrollBarSetMode(xge_xui_scrollbar pScrollBar, int iMode)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iMode = (iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) ? XGE_XUI_SCROLLBAR_MODE_COMPACT : XGE_XUI_SCROLLBAR_MODE_FULL;
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
}

int xgeXuiScrollBarGetMode(xge_xui_scrollbar pScrollBar)
{
	return (pScrollBar != NULL) ? pScrollBar->iMode : XGE_XUI_SCROLLBAR_MODE_FULL;
}

void xgeXuiScrollBarSetColors(xge_xui_scrollbar pScrollBar, uint32_t iTrack, uint32_t iThumb, uint32_t iHover, uint32_t iActive, uint32_t iFocus, uint32_t iDisabled)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iColorTrack = iTrack;
	pScrollBar->iColorThumb = iThumb;
	pScrollBar->iColorHover = iHover;
	pScrollBar->iColorActive = iActive;
	pScrollBar->iColorFocus = iFocus;
	pScrollBar->iColorDisabled = iDisabled;
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
}

int xgeXuiScrollBarGetState(xge_xui_scrollbar pScrollBar)
{
	if ( pScrollBar == NULL ) {
		return XGE_XUI_STATE_DISABLED;
	}
	__xgeXuiScrollBarSetState(pScrollBar, pScrollBar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	return pScrollBar->iState;
}

int xgeXuiScrollBarEvent(xge_xui_scrollbar pScrollBar, const xge_event_t* pEvent)
{
	xge_rect_t tThumb;
	float fMouse;
	int iInside;
	int iState;
	int bWasActive;

	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pScrollBar->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScrollBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScrollBar->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pScrollBar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	fMouse = __xgeXuiScrollBarAxisPos(pScrollBar, pEvent);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( iInside ) {
				iState |= XGE_XUI_STATE_HOVER;
			} else {
				iState &= ~XGE_XUI_STATE_HOVER;
			}
			if ( pScrollBar->bDraggingThumb != 0 && xgeXuiGetPointerCapture(pScrollBar->pContext, pEvent->iPointerId) == pScrollBar->pWidget ) {
				iState |= XGE_XUI_STATE_ACTIVE;
				__xgeXuiScrollBarSetValueFromDrag(pScrollBar, fMouse, 1);
				__xgeXuiScrollBarSetState(pScrollBar, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiScrollBarSetState(pScrollBar, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiScrollBarSetState(pScrollBar, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiScrollBarSetState(pScrollBar, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pScrollBar->pContext, pScrollBar->pWidget);
			xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, pScrollBar->pWidget);
			if ( __xgeXuiRectContains(__xgeXuiScrollBarButtonRect(pScrollBar, 0), pEvent->fX, pEvent->fY) ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue - __xgeXuiScrollBarStep(pScrollBar), 1);
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(__xgeXuiScrollBarButtonRect(pScrollBar, 1), pEvent->fX, pEvent->fY) ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue + __xgeXuiScrollBarStep(pScrollBar), 1);
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			tThumb = __xgeXuiScrollBarThumbRect(pScrollBar);
			if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, pScrollBar->pWidget);
				pScrollBar->bDraggingThumb = 1;
				pScrollBar->fDragStartMouse = fMouse;
				pScrollBar->fDragStartValue = pScrollBar->fValue;
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			} else {
				if ( fMouse < __xgeXuiScrollBarTrackStart(pScrollBar, tThumb) ) {
					__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue - pScrollBar->fPage, 1);
				} else {
					__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue + pScrollBar->fPage, 1);
				}
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue - pEvent->fDY * __xgeXuiScrollBarStep(pScrollBar), 1);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			bWasActive = ((pScrollBar->iState & XGE_XUI_STATE_ACTIVE) != 0);
			if ( pScrollBar->bDraggingThumb != 0 ) {
				if ( xgeXuiGetPointerCapture(pScrollBar->pContext, pEvent->iPointerId) != pScrollBar->pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiScrollBarSetValueFromDrag(pScrollBar, fMouse, 1);
			}
			pScrollBar->bDraggingThumb = 0;
			if ( pScrollBar->pContext != NULL && xgeXuiGetPointerCapture(pScrollBar->pContext, pEvent->iPointerId) == pScrollBar->pWidget ) {
				xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, NULL);
			}
			__xgeXuiScrollBarSetState(pScrollBar, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pScrollBar->bDraggingThumb = 0;
			if ( pScrollBar->pContext != NULL && xgeXuiGetPointerCapture(pScrollBar->pContext, pEvent->iPointerId) == pScrollBar->pWidget ) {
				xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, NULL);
			}
			__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( (pScrollBar->pContext == NULL) || (pScrollBar->pContext->pFocus != pScrollBar->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_LEFT) || (pEvent->iParam1 == XGE_KEY_UP) || (pEvent->iParam1 == XGE_KEY_PAGE_UP) ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue - __xgeXuiScrollBarStep(pScrollBar), 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_RIGHT) || (pEvent->iParam1 == XGE_KEY_DOWN) || (pEvent->iParam1 == XGE_KEY_PAGE_DOWN) ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue + __xgeXuiScrollBarStep(pScrollBar), 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fMin, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_END ) {
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fMax, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiScrollBarEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollBarEvent((xge_xui_scrollbar)pUser, pEvent);
}

void xgeXuiScrollBarPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const uint16_t arrTriangleUp8[8] = {
		0x00, 0x18, 0x3c, 0x7e, 0xff, 0x00, 0x00, 0x00
	};
	static const uint16_t arrTriangleDown8[8] = {
		0x00, 0x00, 0x00, 0xff, 0x7e, 0x3c, 0x18, 0x00
	};
	static const uint16_t arrTriangleLeft8[8] = {
		0x08, 0x18, 0x38, 0x78, 0x78, 0x38, 0x18, 0x08
	};
	static const uint16_t arrTriangleRight8[8] = {
		0x10, 0x18, 0x1c, 0x1e, 0x1e, 0x1c, 0x18, 0x10
	};
	xge_xui_scrollbar pScrollBar;
	xge_rect_t tButtonA;
	xge_rect_t tButtonB;
	xge_rect_t tIcon;
	xge_rect_t tTrack;
	xge_rect_t tThumb;
	uint32_t iThumbColor;
	float fIcon;

	pScrollBar = (xge_xui_scrollbar)pUser;
	if ( (pWidget == NULL) || (pScrollBar == NULL) ) {
		return;
	}
	if ( (pScrollBar->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pScrollBar->iColorFocus) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pScrollBar->iColorFocus);
	}
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		tThumb = __xgeXuiScrollBarVisualThumbRect(pScrollBar);
		iThumbColor = pScrollBar->iColorThumb;
		if ( (pScrollBar->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
			iThumbColor = pScrollBar->iColorDisabled;
		} else if ( (pScrollBar->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
			iThumbColor = pScrollBar->iColorActive;
		} else if ( (pScrollBar->iState & XGE_XUI_STATE_HOVER) != 0 ) {
			iThumbColor = pScrollBar->iColorHover;
		}
		if ( XGE_COLOR_GET_A(iThumbColor) != 0 ) {
			__xgeXuiHostDrawRoundedRect(tThumb, iThumbColor, (tThumb.fW < tThumb.fH ? tThumb.fW : tThumb.fH) * 0.5f);
		}
		return;
	}
	if ( XGE_COLOR_GET_A(pScrollBar->iColorTrack) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tContentRect, pScrollBar->iColorTrack);
		__xgeXuiHostDrawBorderRect(pWidget->tContentRect, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
	}
	tButtonA = __xgeXuiScrollBarButtonRect(pScrollBar, 0);
	tButtonB = __xgeXuiScrollBarButtonRect(pScrollBar, 1);
	if ( (tButtonA.fW > 0.0f) && (tButtonA.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(tButtonA, XGE_COLOR_RGBA(255, 255, 255, 255));
		__xgeXuiHostDrawBorderRect(tButtonA, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
		__xgeXuiHostDrawRect(tButtonB, XGE_COLOR_RGBA(255, 255, 255, 255));
		__xgeXuiHostDrawBorderRect(tButtonB, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
		fIcon = (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? tButtonA.fH : tButtonA.fW;
		if ( fIcon > 8.0f ) {
			fIcon = 8.0f;
		}
		tIcon.fW = fIcon;
		tIcon.fH = fIcon;
		tIcon.fX = tButtonA.fX + (tButtonA.fW - fIcon) * 0.5f;
		tIcon.fY = tButtonA.fY + (tButtonA.fH - fIcon) * 0.5f;
		__xgeXuiHostDrawBitmapMask(tIcon, (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? arrTriangleLeft8 : arrTriangleUp8, 8, 8, pScrollBar->iColorThumb);
		tIcon.fX = tButtonB.fX + (tButtonB.fW - fIcon) * 0.5f;
		tIcon.fY = tButtonB.fY + (tButtonB.fH - fIcon) * 0.5f;
		__xgeXuiHostDrawBitmapMask(tIcon, (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? arrTriangleRight8 : arrTriangleDown8, 8, 8, pScrollBar->iColorThumb);
	}
	tTrack = __xgeXuiScrollBarTrackRect(pScrollBar);
	if ( XGE_COLOR_GET_A(pScrollBar->iColorTrack) != 0 && (tTrack.fW > 0.0f) && (tTrack.fH > 0.0f) ) {
		__xgeXuiHostDrawRect(tTrack, pScrollBar->iColorTrack);
	}
	__xgeXuiHostDrawBorderRect(pWidget->tContentRect, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
	tThumb = __xgeXuiScrollBarVisualThumbRect(pScrollBar);
	iThumbColor = pScrollBar->iColorThumb;
	if ( (pScrollBar->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		iThumbColor = pScrollBar->iColorDisabled;
	} else if ( (pScrollBar->iState & XGE_XUI_STATE_ACTIVE) != 0 ) {
		iThumbColor = pScrollBar->iColorActive;
	} else if ( (pScrollBar->iState & XGE_XUI_STATE_HOVER) != 0 ) {
		iThumbColor = pScrollBar->iColorHover;
	}
	if ( XGE_COLOR_GET_A(iThumbColor) != 0 ) {
		__xgeXuiHostDrawRect(tThumb, iThumbColor);
	}
}
