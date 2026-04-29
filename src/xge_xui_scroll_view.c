int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pScroll == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pScroll, 0, sizeof(*pScroll));
	pScroll->pContext = pContext;
	pScroll->pWidget = pWidget;
	pScroll->fContentW = pWidget->tContentRect.fW;
	pScroll->fContentH = pWidget->tContentRect.fH;
	pScroll->iBackgroundColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pScroll->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pScroll->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiScrollViewEventProc;
	pWidget->procPaint = xgeXuiScrollViewPaintProc;
	pWidget->pUser = pScroll;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

static float __xgeXuiScrollViewMaxX(xge_xui_scroll_view pScroll)
{
	float fMax;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = pScroll->fContentW - pScroll->pWidget->tContentRect.fW;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static float __xgeXuiScrollViewMaxY(xge_xui_scroll_view pScroll)
{
	float fMax;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = pScroll->fContentH - pScroll->pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static float __xgeXuiScrollViewThumbLen(float fTrackLen, float fVisible, float fContent)
{
	float fLen;

	if ( (fTrackLen <= 0.0f) || (fVisible <= 0.0f) || (fContent <= fVisible) ) {
		return fTrackLen;
	}
	fLen = fTrackLen * (fVisible / fContent);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	if ( fLen > fTrackLen ) {
		fLen = fTrackLen;
	}
	return fLen;
}

static int __xgeXuiScrollViewVerticalBar(xge_xui_scroll_view pScroll, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fMaxScroll;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pScroll->fContentH <= pScroll->pWidget->tContentRect.fH) ) {
		return 0;
	}
	tBar.fX = pScroll->pWidget->tContentRect.fX + pScroll->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pScroll->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pScroll->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fH = __xgeXuiScrollViewThumbLen(tBar.fH, pScroll->pWidget->tContentRect.fH, pScroll->fContentH);
	fMaxScroll = __xgeXuiScrollViewMaxY(pScroll);
	if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
		tThumb.fY += (tBar.fH - tThumb.fH) * (pScroll->fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static int __xgeXuiScrollViewHorizontalBar(xge_xui_scroll_view pScroll, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fMaxScroll;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pScroll->fContentW <= pScroll->pWidget->tContentRect.fW) ) {
		return 0;
	}
	tBar.fX = pScroll->pWidget->tContentRect.fX;
	tBar.fY = pScroll->pWidget->tContentRect.fY + pScroll->pWidget->tContentRect.fH - 4.0f;
	tBar.fW = pScroll->pWidget->tContentRect.fW;
	tBar.fH = 4.0f;
	tThumb = tBar;
	tThumb.fW = __xgeXuiScrollViewThumbLen(tBar.fW, pScroll->pWidget->tContentRect.fW, pScroll->fContentW);
	fMaxScroll = __xgeXuiScrollViewMaxX(pScroll);
	if ( fMaxScroll > 0.0f && tBar.fW > tThumb.fW ) {
		tThumb.fX += (tBar.fW - tThumb.fW) * (pScroll->fScrollX / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static void __xgeXuiScrollViewSetOffsetFromThumbDrag(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pScroll == NULL) || (pEvent == NULL) ) {
		return;
	}
	if ( pScroll->bDragging == 2 && __xgeXuiScrollViewVerticalBar(pScroll, &tBar, &tThumb) != 0 ) {
		fTravel = tBar.fH - tThumb.fH;
		fMaxScroll = __xgeXuiScrollViewMaxY(pScroll);
		if ( fTravel > 0.0f && fMaxScroll > 0.0f ) {
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fDragScrollX, pScroll->fDragScrollY + ((pEvent->fY - pScroll->fDragY) / fTravel) * fMaxScroll);
		}
	} else if ( pScroll->bDragging == 3 && __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 ) {
		fTravel = tBar.fW - tThumb.fW;
		fMaxScroll = __xgeXuiScrollViewMaxX(pScroll);
		if ( fTravel > 0.0f && fMaxScroll > 0.0f ) {
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fDragScrollX + ((pEvent->fX - pScroll->fDragX) / fTravel) * fMaxScroll, pScroll->fDragScrollY);
		}
	}
}

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( pScroll->pContext != NULL && pScroll->pContext->pCapture == pScroll->pWidget ) {
		xgeXuiSetCapture(pScroll->pContext, NULL);
	}
	if ( pScroll->pWidget != NULL && pScroll->pWidget->pUser == pScroll ) {
		pScroll->pWidget->pUser = NULL;
		pScroll->pWidget->procEvent = NULL;
		pScroll->pWidget->procPaint = NULL;
	}
	memset(pScroll, 0, sizeof(*pScroll));
}

void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	pScroll->fContentW = fWidth;
	pScroll->fContentH = fHeight;
	__xgeXuiScrollViewClamp(pScroll);
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY)
{
	__xgeXuiScrollViewSetOffsetInternal(pScroll, fX, fY);
}

void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = (pScroll != NULL) ? pScroll->fScrollX : 0.0f;
	}
	if ( pY != NULL ) {
		*pY = (pScroll != NULL) ? pScroll->fScrollY : 0.0f;
	}
}

void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->iBackgroundColor = iBackground;
	pScroll->iBarColor = iBar;
	pScroll->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScroll->pWidget->tContentRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX - pEvent->fDX * 32.0f, pScroll->fScrollY - pEvent->fDY * 32.0f);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
			xgeXuiSetCapture(pScroll->pContext, pScroll->pWidget);
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
			pScroll->fDragScrollX = pScroll->fScrollX;
			pScroll->fDragScrollY = pScroll->fScrollY;
			if ( __xgeXuiScrollViewVerticalBar(pScroll, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pScroll->bDragging = 2;
				} else {
					__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX, pScroll->fScrollY + ((pEvent->fY < tThumb.fY) ? -pScroll->pWidget->tContentRect.fH : pScroll->pWidget->tContentRect.fH));
					pScroll->bDragging = 0;
					xgeXuiSetCapture(pScroll->pContext, NULL);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pScroll->bDragging = 3;
				} else {
					__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + ((pEvent->fX < tThumb.fX) ? -pScroll->pWidget->tContentRect.fW : pScroll->pWidget->tContentRect.fW), pScroll->fScrollY);
					pScroll->bDragging = 0;
					xgeXuiSetCapture(pScroll->pContext, NULL);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			pScroll->bDragging = 1;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pScroll->bDragging == 1 ) {
				__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + (pScroll->fDragX - pEvent->fX), pScroll->fScrollY + (pScroll->fDragY - pEvent->fY));
				pScroll->fDragX = pEvent->fX;
				pScroll->fDragY = pEvent->fY;
			} else {
				__xgeXuiScrollViewSetOffsetFromThumbDrag(pScroll, pEvent);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->bDragging = 0;
			if ( pScroll->pContext != NULL && pScroll->pContext->pCapture == pScroll->pWidget ) {
				xgeXuiSetCapture(pScroll->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollViewEvent((xge_xui_scroll_view)pUser, pEvent);
}

void xgeXuiScrollViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_scroll_view pScroll;
	xge_rect_t tBar;
	xge_rect_t tThumb;

	pScroll = (xge_xui_scroll_view)pUser;
	if ( (pWidget == NULL) || (pScroll == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pScroll->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pScroll->iBackgroundColor);
	}
	if ( __xgeXuiScrollViewVerticalBar(pScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiHostDrawRect(tBar, pScroll->iBarColor);
		__xgeXuiHostDrawRect(tThumb, pScroll->iThumbColor);
	}
	if ( __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiHostDrawRect(tBar, pScroll->iBarColor);
		__xgeXuiHostDrawRect(tThumb, pScroll->iThumbColor);
	}
}
