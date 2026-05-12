static void __xgeXuiScrollBarSetState(xge_xui_scrollbar pScrollBar, int iState)
{
	int iVisualState;

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
		__xgeXuiRenderCacheInvalidate(&pScrollBar->tCache);
		xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
	}
	if ( pScrollBar->pWidget != NULL ) {
		iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED);
		xgeXuiWidgetSetVisualState(pScrollBar->pWidget, iVisualState);
	}
}

static void __xgeXuiScrollBarSetParts(xge_xui_scrollbar pScrollBar, int iHoverPart, int iActivePart)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	if ( pScrollBar->iHoverPart != iHoverPart || pScrollBar->iActivePart != iActivePart ) {
		pScrollBar->iHoverPart = iHoverPart;
		pScrollBar->iActivePart = iActivePart;
		__xgeXuiRenderCacheInvalidate(&pScrollBar->tCache);
		xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
	}
}

static int __xgeXuiScrollBarCacheModeNormalize(int iMode)
{
	if ( (iMode != XGE_XUI_CACHE_AUTO) && (iMode != XGE_XUI_CACHE_OFF) && (iMode != XGE_XUI_CACHE_FORCE) ) {
		return XGE_XUI_CACHE_AUTO;
	}
	return iMode;
}

static void __xgeXuiScrollBarCacheInvalidate(xge_xui_scrollbar pScrollBar, int bLayout)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	__xgeXuiRenderCacheInvalidate(&pScrollBar->tCache);
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pScrollBar->pWidget);
	}
	xgeXuiWidgetMarkPaint(pScrollBar->pWidget);
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
	__xgeXuiRenderCacheInvalidate(&pScrollBar->tCache);
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

static float __xgeXuiScrollBarVisualSize(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tRect;
	float fSize;
	float fCross;

	if ( pScrollBar == NULL ) {
		return 8.0f;
	}
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		fSize = (pScrollBar->fTrackSize > 0.0f) ? pScrollBar->fTrackSize : 8.0f;
		if ( fSize > 8.0f ) {
			fSize = 8.0f;
		}
	} else {
		tRect = (pScrollBar->pWidget != NULL) ? pScrollBar->pWidget->tContentRect : (xge_rect_t){ 0.0f, 0.0f, 18.0f, 18.0f };
		fCross = (pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? tRect.fH : tRect.fW;
		fSize = fCross;
		if ( pScrollBar->fTrackSize > 0.0f && pScrollBar->fTrackSize < fSize ) {
			fSize = pScrollBar->fTrackSize;
		}
	}
	if ( fSize < 1.0f ) {
		fSize = 1.0f;
	}
	return fSize;
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
	if ( pScrollBar->iButtonMode == XGE_XUI_SCROLLBAR_BUTTONS_OFF ) {
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
	if ( pScrollBar->iButtonMode != XGE_XUI_SCROLLBAR_BUTTONS_ON && (fLong < 32.0f || fCross < 8.0f) ) {
		return 0.0f;
	}
	fSize = (pScrollBar->fButtonSize > 0.0f) ? pScrollBar->fButtonSize : fCross;
	if ( fSize > fCross ) {
		fSize = fCross;
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
	if ( fLen < pScrollBar->fMinThumbSize ) {
		fLen = pScrollBar->fMinThumbSize;
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
	float fSize;

	tThumb = __xgeXuiScrollBarThumbRect(pScrollBar);
	fSize = __xgeXuiScrollBarVisualSize(pScrollBar);
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
			tThumb.fY += (tThumb.fH - fSize) * 0.5f;
			tThumb.fH = fSize;
		} else {
			tThumb.fX += (tThumb.fW - fSize) * 0.5f;
			tThumb.fW = fSize;
		}
	} else if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tThumb.fY += (tThumb.fH - fSize) * 0.5f;
		tThumb.fH = fSize;
	} else {
		tThumb.fX += (tThumb.fW - fSize) * 0.5f;
		tThumb.fW = fSize;
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

static int __xgeXuiScrollBarHitPart(xge_xui_scrollbar pScrollBar, float fX, float fY)
{
	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) ) {
		return XGE_XUI_SCROLLBAR_PART_NONE;
	}
	if ( __xgeXuiRectContains(pScrollBar->pWidget->tRect, fX, fY) == 0 ) {
		return XGE_XUI_SCROLLBAR_PART_NONE;
	}
	if ( __xgeXuiRectContains(__xgeXuiScrollBarButtonRect(pScrollBar, 0), fX, fY) ) {
		return XGE_XUI_SCROLLBAR_PART_BUTTON_START;
	}
	if ( __xgeXuiRectContains(__xgeXuiScrollBarButtonRect(pScrollBar, 1), fX, fY) ) {
		return XGE_XUI_SCROLLBAR_PART_BUTTON_END;
	}
	if ( __xgeXuiRectContains(__xgeXuiScrollBarThumbRect(pScrollBar), fX, fY) ) {
		return XGE_XUI_SCROLLBAR_PART_THUMB;
	}
	if ( __xgeXuiRectContains(__xgeXuiScrollBarTrackRect(pScrollBar), fX, fY) ) {
		return XGE_XUI_SCROLLBAR_PART_TRACK;
	}
	return XGE_XUI_SCROLLBAR_PART_NONE;
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
	pScrollBar->iColorButton = XGE_COLOR_RGBA(248, 251, 255, 255);
	pScrollBar->iColorButtonIcon = XGE_COLOR_RGBA(104, 132, 158, 255);
	pScrollBar->iOrientation = XGE_XUI_SEPARATOR_VERTICAL;
	pScrollBar->iMode = XGE_XUI_SCROLLBAR_MODE_FULL;
	pScrollBar->iButtonMode = XGE_XUI_SCROLLBAR_BUTTONS_AUTO;
	pScrollBar->iHoverPart = XGE_XUI_SCROLLBAR_PART_NONE;
	pScrollBar->iActivePart = XGE_XUI_SCROLLBAR_PART_NONE;
	pScrollBar->fTrackSize = 0.0f;
	pScrollBar->fMinThumbSize = 18.0f;
	pScrollBar->fThumbRadius = -1.0f;
	pScrollBar->fButtonSize = 0.0f;
	pScrollBar->iCacheMode = XGE_XUI_CACHE_AUTO;
	pScrollBar->iCacheState = -1;
	__xgeXuiRenderCacheInit(&pScrollBar->tCache);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiScrollBarEventProc, pScrollBar);
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
	__xgeXuiRenderCacheUnit(&pScrollBar->tCache);
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
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 0);
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
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 1);
}

void xgeXuiScrollBarSetMode(xge_xui_scrollbar pScrollBar, int iMode)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iMode = (iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) ? XGE_XUI_SCROLLBAR_MODE_COMPACT : XGE_XUI_SCROLLBAR_MODE_FULL;
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 1);
}

int xgeXuiScrollBarGetMode(xge_xui_scrollbar pScrollBar)
{
	return (pScrollBar != NULL) ? pScrollBar->iMode : XGE_XUI_SCROLLBAR_MODE_FULL;
}

void xgeXuiScrollBarSetButtonMode(xge_xui_scrollbar pScrollBar, int iMode)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	if ( (iMode != XGE_XUI_SCROLLBAR_BUTTONS_AUTO) && (iMode != XGE_XUI_SCROLLBAR_BUTTONS_OFF) && (iMode != XGE_XUI_SCROLLBAR_BUTTONS_ON) ) {
		iMode = XGE_XUI_SCROLLBAR_BUTTONS_AUTO;
	}
	pScrollBar->iButtonMode = iMode;
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 1);
}

void xgeXuiScrollBarSetMetrics(xge_xui_scrollbar pScrollBar, float fTrackSize, float fMinThumbSize, float fThumbRadius, float fButtonSize)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->fTrackSize = (fTrackSize > 0.0f) ? fTrackSize : 0.0f;
	pScrollBar->fMinThumbSize = (fMinThumbSize > 0.0f) ? fMinThumbSize : 18.0f;
	pScrollBar->fThumbRadius = (fThumbRadius >= 0.0f) ? fThumbRadius : -1.0f;
	pScrollBar->fButtonSize = (fButtonSize > 0.0f) ? fButtonSize : 0.0f;
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 1);
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
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 0);
}

void xgeXuiScrollBarSetButtonColors(xge_xui_scrollbar pScrollBar, uint32_t iButton, uint32_t iIcon)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iColorButton = iButton;
	pScrollBar->iColorButtonIcon = iIcon;
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 0);
}

void xgeXuiScrollBarSetCacheMode(xge_xui_scrollbar pScrollBar, int iMode)
{
	if ( pScrollBar == NULL ) {
		return;
	}
	pScrollBar->iCacheMode = __xgeXuiScrollBarCacheModeNormalize(iMode);
	__xgeXuiScrollBarCacheInvalidate(pScrollBar, 0);
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
	int iPart;

	if ( (pScrollBar == NULL) || (pScrollBar->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pScrollBar->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScrollBar->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiScrollBarSetParts(pScrollBar, XGE_XUI_SCROLLBAR_PART_NONE, XGE_XUI_SCROLLBAR_PART_NONE);
		__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScrollBar->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pScrollBar->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	fMouse = __xgeXuiScrollBarAxisPos(pScrollBar, pEvent);
	iPart = __xgeXuiScrollBarHitPart(pScrollBar, pEvent->fX, pEvent->fY);
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
				__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_THUMB);
				__xgeXuiScrollBarSetValueFromDrag(pScrollBar, fMouse, 1);
				__xgeXuiScrollBarSetState(pScrollBar, iState);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_NONE);
			__xgeXuiScrollBarSetState(pScrollBar, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiScrollBarSetState(pScrollBar, iState);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_NONE);
			__xgeXuiScrollBarSetState(pScrollBar, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiScrollBarSetParts(pScrollBar, XGE_XUI_SCROLLBAR_PART_NONE, XGE_XUI_SCROLLBAR_PART_NONE);
			__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pScrollBar->pContext, pScrollBar->pWidget);
			xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, pScrollBar->pWidget);
			if ( iPart == XGE_XUI_SCROLLBAR_PART_BUTTON_START ) {
				__xgeXuiScrollBarSetParts(pScrollBar, iPart, iPart);
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue - __xgeXuiScrollBarStep(pScrollBar), 1);
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( iPart == XGE_XUI_SCROLLBAR_PART_BUTTON_END ) {
				__xgeXuiScrollBarSetParts(pScrollBar, iPart, iPart);
				__xgeXuiScrollBarSetValueInternal(pScrollBar, pScrollBar->fValue + __xgeXuiScrollBarStep(pScrollBar), 1);
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
				return XGE_XUI_EVENT_CONSUMED;
			}
			tThumb = __xgeXuiScrollBarThumbRect(pScrollBar);
			if ( iPart == XGE_XUI_SCROLLBAR_PART_THUMB ) {
				xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, pScrollBar->pWidget);
				pScrollBar->bDraggingThumb = 1;
				pScrollBar->fDragStartMouse = fMouse;
				pScrollBar->fDragStartValue = pScrollBar->fValue;
				__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_THUMB);
				__xgeXuiScrollBarSetState(pScrollBar, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			} else {
				__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_TRACK);
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
			__xgeXuiScrollBarSetParts(pScrollBar, iPart, XGE_XUI_SCROLLBAR_PART_NONE);
			if ( pScrollBar->pContext != NULL && xgeXuiGetPointerCapture(pScrollBar->pContext, pEvent->iPointerId) == pScrollBar->pWidget ) {
				xgeXuiSetPointerCapture(pScrollBar->pContext, pEvent->iPointerId, NULL);
			}
			__xgeXuiScrollBarSetState(pScrollBar, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			return bWasActive ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pScrollBar->bDraggingThumb = 0;
			__xgeXuiScrollBarSetParts(pScrollBar, XGE_XUI_SCROLLBAR_PART_NONE, XGE_XUI_SCROLLBAR_PART_NONE);
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
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiScrollBarEvent((xge_xui_scrollbar)pUser, pEvent);
}

static uint32_t __xgeXuiScrollBarThumbColor(xge_xui_scrollbar pScrollBar)
{
	if ( pScrollBar == NULL ) {
		return 0;
	}
	if ( (pScrollBar->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pScrollBar->iColorDisabled;
	}
	if ( pScrollBar->iActivePart == XGE_XUI_SCROLLBAR_PART_THUMB ) {
		return pScrollBar->iColorActive;
	}
	if ( pScrollBar->iHoverPart == XGE_XUI_SCROLLBAR_PART_THUMB ) {
		return pScrollBar->iColorHover;
	}
	return pScrollBar->iColorThumb;
}

static uint32_t __xgeXuiScrollBarMixColor(uint32_t iBase, uint32_t iTarget, float fRate)
{
	int r;
	int g;
	int b;
	int a;

	if ( fRate < 0.0f ) {
		fRate = 0.0f;
	} else if ( fRate > 1.0f ) {
		fRate = 1.0f;
	}
	r = (int)((float)XGE_COLOR_GET_R(iBase) + ((float)XGE_COLOR_GET_R(iTarget) - (float)XGE_COLOR_GET_R(iBase)) * fRate);
	g = (int)((float)XGE_COLOR_GET_G(iBase) + ((float)XGE_COLOR_GET_G(iTarget) - (float)XGE_COLOR_GET_G(iBase)) * fRate);
	b = (int)((float)XGE_COLOR_GET_B(iBase) + ((float)XGE_COLOR_GET_B(iTarget) - (float)XGE_COLOR_GET_B(iBase)) * fRate);
	a = (int)((float)XGE_COLOR_GET_A(iBase) + ((float)XGE_COLOR_GET_A(iTarget) - (float)XGE_COLOR_GET_A(iBase)) * fRate);
	return XGE_COLOR_RGBA(r, g, b, a);
}

static uint32_t __xgeXuiScrollBarButtonColor(xge_xui_scrollbar pScrollBar, int bEnd)
{
	int iPart;

	if ( pScrollBar == NULL ) {
		return 0;
	}
	if ( (pScrollBar->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return __xgeXuiScrollBarMixColor(pScrollBar->iColorButton, pScrollBar->iColorDisabled, 0.32f);
	}
	iPart = bEnd ? XGE_XUI_SCROLLBAR_PART_BUTTON_END : XGE_XUI_SCROLLBAR_PART_BUTTON_START;
	if ( pScrollBar->iActivePart == iPart ) {
		return __xgeXuiScrollBarMixColor(pScrollBar->iColorButton, pScrollBar->iColorActive, 0.42f);
	}
	if ( pScrollBar->iHoverPart == iPart ) {
		return __xgeXuiScrollBarMixColor(pScrollBar->iColorButton, pScrollBar->iColorHover, 0.28f);
	}
	return pScrollBar->iColorButton;
}

static xge_rect_t __xgeXuiScrollBarVisualTrackRect(xge_xui_scrollbar pScrollBar)
{
	xge_rect_t tTrack;
	float fSize;

	tTrack = __xgeXuiScrollBarTrackRect(pScrollBar);
	if ( pScrollBar == NULL ) {
		return tTrack;
	}
	fSize = __xgeXuiScrollBarVisualSize(pScrollBar);
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tTrack.fY += (tTrack.fH - fSize) * 0.5f;
		tTrack.fH = fSize;
	} else {
		tTrack.fX += (tTrack.fW - fSize) * 0.5f;
		tTrack.fW = fSize;
	}
	if ( tTrack.fW < 1.0f ) {
		tTrack.fW = 1.0f;
	}
	if ( tTrack.fH < 1.0f ) {
		tTrack.fH = 1.0f;
	}
	return tTrack;
}

static void __xgeXuiScrollBarDrawButtonIcon(xge_xui_scrollbar pScrollBar, xge_rect_t tButton, int bEnd)
{
	static const uint16_t arrTriangleLeft12[12] = {
		0x000, 0x010, 0x030, 0x070, 0x0F0, 0x1F0, 0x0F0, 0x070, 0x030, 0x010, 0x000, 0x000
	};
	static const uint16_t arrTriangleRight12[12] = {
		0x000, 0x080, 0x0C0, 0x0E0, 0x0F0, 0x0F8, 0x0F0, 0x0E0, 0x0C0, 0x080, 0x000, 0x000
	};
	static const uint16_t arrTriangleUp12[12] = {
		0x000, 0x000, 0x060, 0x0F0, 0x1F8, 0x3FC, 0x7FE, 0x000, 0x000, 0x000, 0x000, 0x000
	};
	static const uint16_t arrTriangleDown12[12] = {
		0x000, 0x000, 0x000, 0x000, 0x000, 0x7FE, 0x3FC, 0x1F8, 0x0F0, 0x060, 0x000, 0x000
	};
	const uint16_t* arrIcon;
	xge_rect_t tIcon;
	float fSize;

	if ( (pScrollBar == NULL) || (XGE_COLOR_GET_A(pScrollBar->iColorButtonIcon) == 0) || (tButton.fW <= 0.0f) || (tButton.fH <= 0.0f) ) {
		return;
	}
	fSize = ((tButton.fW < tButton.fH) ? tButton.fW : tButton.fH) * 0.58f;
	if ( fSize < 7.0f ) {
		fSize = 7.0f;
	}
	if ( fSize > 11.0f ) {
		fSize = 11.0f;
	}
	if ( pScrollBar->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		arrIcon = bEnd ? arrTriangleRight12 : arrTriangleLeft12;
	} else {
		arrIcon = bEnd ? arrTriangleDown12 : arrTriangleUp12;
	}
	tIcon.fX = tButton.fX + (tButton.fW - fSize) * 0.5f;
	tIcon.fY = tButton.fY + (tButton.fH - fSize) * 0.5f;
	tIcon.fW = fSize;
	tIcon.fH = fSize;
	__xgeXuiHostDrawBitmapMask(tIcon, arrIcon, 12, 12, pScrollBar->iColorButtonIcon);
}

static void __xgeXuiScrollBarDrawDirect(xge_xui_widget pWidget, xge_xui_scrollbar pScrollBar, xge_rect_t tContent)
{
	xge_rect_t tButtonA;
	xge_rect_t tButtonB;
	xge_rect_t tTrack;
	xge_rect_t tThumb;
	float fRadius;
	uint32_t iThumbColor;
	uint32_t iButtonAColor;
	uint32_t iButtonBColor;
	uint32_t iBorderColor;

	if ( (pWidget == NULL) || (pScrollBar == NULL) ) {
		return;
	}
	if ( (pScrollBar->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pScrollBar->iColorFocus) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pScrollBar->iColorFocus);
	}
	iBorderColor = XGE_COLOR_RGBA(144, 170, 198, 255);
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_FULL ) {
		if ( XGE_COLOR_GET_A(pScrollBar->iColorTrack) != 0 ) {
			__xgeXuiHostDrawRect(pWidget->tContentRect, pScrollBar->iColorTrack);
		}
		tButtonA = __xgeXuiScrollBarButtonRect(pScrollBar, 0);
		tButtonB = __xgeXuiScrollBarButtonRect(pScrollBar, 1);
		iButtonAColor = __xgeXuiScrollBarButtonColor(pScrollBar, 0);
		iButtonBColor = __xgeXuiScrollBarButtonColor(pScrollBar, 1);
		if ( (tButtonA.fW > 0.0f) && (tButtonA.fH > 0.0f) && XGE_COLOR_GET_A(iButtonAColor) != 0 ) {
			__xgeXuiHostDrawRect(tButtonA, iButtonAColor);
			__xgeXuiHostDrawRect(tButtonB, iButtonBColor);
			__xgeXuiScrollBarDrawButtonIcon(pScrollBar, tButtonA, 0);
			__xgeXuiScrollBarDrawButtonIcon(pScrollBar, tButtonB, 1);
		}
	}
	tTrack = (pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT) ? tContent : __xgeXuiScrollBarVisualTrackRect(pScrollBar);
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		tTrack = __xgeXuiScrollBarVisualTrackRect(pScrollBar);
	}
	if ( XGE_COLOR_GET_A(pScrollBar->iColorTrack) != 0 && (tTrack.fW > 0.0f) && (tTrack.fH > 0.0f) ) {
		if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_FULL ) {
			__xgeXuiHostDrawRect(tTrack, pScrollBar->iColorTrack);
		} else {
			__xgeXuiHostDrawRoundedRect(tTrack, pScrollBar->iColorTrack, (tTrack.fW < tTrack.fH ? tTrack.fW : tTrack.fH) * 0.5f);
		}
	}
	tThumb = __xgeXuiScrollBarVisualThumbRect(pScrollBar);
	iThumbColor = __xgeXuiScrollBarThumbColor(pScrollBar);
	if ( XGE_COLOR_GET_A(iThumbColor) != 0 ) {
		fRadius = (pScrollBar->fThumbRadius >= 0.0f) ? pScrollBar->fThumbRadius : ((tThumb.fW < tThumb.fH ? tThumb.fW : tThumb.fH) * 0.5f);
		if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_FULL ) {
			__xgeXuiHostDrawRect(tThumb, iThumbColor);
		} else {
			__xgeXuiHostDrawRoundedRect(tThumb, iThumbColor, fRadius);
		}
	}
	if ( pScrollBar->iMode == XGE_XUI_SCROLLBAR_MODE_FULL ) {
		__xgeXuiHostDrawBorderRect(pWidget->tContentRect, 1.0f, iBorderColor);
	}
}

void xgeXuiScrollBarPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_scrollbar pScrollBar;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pScrollBar = (xge_xui_scrollbar)pUser;
	if ( (pWidget == NULL) || (pScrollBar == NULL) ) {
		return;
	}
	/* The render-cache path is disabled for ScrollBar until shape primitives can be cached in local coordinates. */
	__xgeXuiScrollBarDrawDirect(pWidget, pScrollBar, pWidget->tContentRect);
}
