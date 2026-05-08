int xgeXuiScrollViewBaseInit(xge_xui_scroll_view_base pBase, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pBase == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBase, 0, sizeof(*pBase));
	__xgeXuiViewportWidgetInit(pWidget, 1);
	pBase->pContext = pContext;
	pBase->pWidget = pWidget;
	pBase->fContentW = pWidget->tContentRect.fW;
	pBase->fContentH = pWidget->tContentRect.fH;
	pBase->iBarColor = XGE_COLOR_RGBA(226, 236, 246, 220);
	pBase->iThumbColor = XGE_COLOR_RGBA(158, 176, 196, 235);
	pBase->iScrollbarPolicy = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	pBase->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pBase->iNestedScrollPolicy = XGE_XUI_NESTED_SCROLL_CONSUME;
	pBase->iWheelAxis = XGE_XUI_WHEEL_AXIS_VERTICAL;
	pBase->bScrollbarDragEnabled = 1;
	xgeXuiWidgetSetOverflow(pWidget, XGE_XUI_OVERFLOW_SCROLL);
	pWidget->procEvent = xgeXuiScrollViewBaseEventProc;
	pWidget->procPaint = xgeXuiScrollViewBasePaintProc;
	pWidget->pUser = pBase;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iResult;

	iResult = xgeXuiScrollViewBaseInit((xge_xui_scroll_view_base)pScroll, pContext, pWidget);
	if ( iResult != XGE_OK ) {
		return iResult;
	}
	pWidget->procEvent = xgeXuiScrollViewEventProc;
	pWidget->procPaint = xgeXuiScrollViewPaintProc;
	pWidget->pUser = pScroll;
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

static int __xgeXuiScrollViewShowVerticalBar(xge_xui_scroll_view pScroll)
{
	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0;
	}
	if ( pScroll->iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_HIDDEN ) {
		return 0;
	}
	if ( pScroll->iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_ALWAYS ) {
		return (pScroll->pWidget->tContentRect.fW > 0.0f) && (pScroll->pWidget->tContentRect.fH > 0.0f);
	}
	return pScroll->fContentH > pScroll->pWidget->tContentRect.fH;
}

static int __xgeXuiScrollViewShowHorizontalBar(xge_xui_scroll_view pScroll)
{
	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0;
	}
	if ( pScroll->iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_HIDDEN ) {
		return 0;
	}
	if ( pScroll->iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_ALWAYS ) {
		return (pScroll->pWidget->tContentRect.fW > 0.0f) && (pScroll->pWidget->tContentRect.fH > 0.0f);
	}
	return pScroll->fContentW > pScroll->pWidget->tContentRect.fW;
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

static float __xgeXuiScrollViewBarSize(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL && pScroll->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
}

static float __xgeXuiScrollViewButtonSize(xge_xui_scroll_view pScroll, float fBarSize)
{
	return (pScroll != NULL && pScroll->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fBarSize : 0.0f;
}

static int __xgeXuiScrollViewVerticalBar(xge_xui_scroll_view pScroll, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fMaxScroll;
	float fSize;
	float fButton;

	if ( __xgeXuiScrollViewShowVerticalBar(pScroll) == 0 ) {
		return 0;
	}
	fSize = __xgeXuiScrollViewBarSize(pScroll);
	fButton = __xgeXuiScrollViewButtonSize(pScroll, fSize);
	tBar.fX = pScroll->pWidget->tContentRect.fX + pScroll->pWidget->tContentRect.fW - fSize;
	tBar.fY = pScroll->pWidget->tContentRect.fY;
	tBar.fW = fSize;
	tBar.fH = pScroll->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	tThumb.fH = __xgeXuiScrollViewThumbLen(tThumb.fH, pScroll->pWidget->tContentRect.fH, pScroll->fContentH);
	fMaxScroll = __xgeXuiScrollViewMaxY(pScroll);
	if ( fMaxScroll > 0.0f && (tBar.fH - fButton * 2.0f) > tThumb.fH ) {
		tThumb.fY += ((tBar.fH - fButton * 2.0f) - tThumb.fH) * (pScroll->fScrollY / fMaxScroll);
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
	float fSize;
	float fButton;

	if ( __xgeXuiScrollViewShowHorizontalBar(pScroll) == 0 ) {
		return 0;
	}
	fSize = __xgeXuiScrollViewBarSize(pScroll);
	fButton = __xgeXuiScrollViewButtonSize(pScroll, fSize);
	tBar.fX = pScroll->pWidget->tContentRect.fX;
	tBar.fY = pScroll->pWidget->tContentRect.fY + pScroll->pWidget->tContentRect.fH - fSize;
	tBar.fW = pScroll->pWidget->tContentRect.fW;
	tBar.fH = fSize;
	tThumb = tBar;
	tThumb.fX += fButton;
	tThumb.fW -= fButton * 2.0f;
	tThumb.fW = __xgeXuiScrollViewThumbLen(tThumb.fW, pScroll->pWidget->tContentRect.fW, pScroll->fContentW);
	fMaxScroll = __xgeXuiScrollViewMaxX(pScroll);
	if ( fMaxScroll > 0.0f && (tBar.fW - fButton * 2.0f) > tThumb.fW ) {
		tThumb.fX += ((tBar.fW - fButton * 2.0f) - tThumb.fW) * (pScroll->fScrollX / fMaxScroll);
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

void xgeXuiScrollViewBaseUnit(xge_xui_scroll_view_base pBase)
{
	if ( pBase == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pBase->pContext, pBase->pWidget);
	if ( pBase->pWidget != NULL && pBase->pWidget->pUser == pBase && ((pBase->pWidget->procEvent == xgeXuiScrollViewBaseEventProc) || (pBase->pWidget->procEvent == xgeXuiScrollViewEventProc)) ) {
		pBase->pWidget->pUser = NULL;
		pBase->pWidget->procEvent = NULL;
		pBase->pWidget->procPaint = NULL;
	}
	memset(pBase, 0, sizeof(*pBase));
}

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	xgeXuiScrollViewBaseUnit((xge_xui_scroll_view_base)pScroll);
}

void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight)
{
	float fOldW;
	float fOldH;
	float fOldX;
	float fOldY;

	if ( pScroll == NULL ) {
		return;
	}
	if ( fWidth < 0.0f ) {
		fWidth = 0.0f;
	}
	if ( fHeight < 0.0f ) {
		fHeight = 0.0f;
	}
	fOldW = pScroll->fContentW;
	fOldH = pScroll->fContentH;
	fOldX = pScroll->fScrollX;
	fOldY = pScroll->fScrollY;
	pScroll->fContentW = fWidth;
	pScroll->fContentH = fHeight;
	__xgeXuiScrollViewClamp(pScroll);
	if ( (fOldW != pScroll->fContentW) || (fOldH != pScroll->fContentH) || (fOldX != pScroll->fScrollX) || (fOldY != pScroll->fScrollY) ) {
		xgeXuiWidgetMarkLayout(pScroll->pWidget);
	}
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY)
{
	__xgeXuiScrollViewSetOffsetInternal(pScroll, fX, fY);
}

void xgeXuiScrollViewScrollBy(xge_xui_scroll_view pScroll, float fDX, float fDY)
{
	if ( pScroll == NULL ) {
		return;
	}
	__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + fDX, pScroll->fScrollY + fDY);
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

void xgeXuiScrollViewEnsureRectVisible(xge_xui_scroll_view pScroll, xge_rect_t tRect)
{
	float fX;
	float fY;
	float fVisibleW;
	float fVisibleH;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return;
	}
	fX = pScroll->fScrollX;
	fY = pScroll->fScrollY;
	fVisibleW = pScroll->pWidget->tContentRect.fW;
	fVisibleH = pScroll->pWidget->tContentRect.fH;
	if ( tRect.fW > fVisibleW ) {
		tRect.fW = fVisibleW;
	}
	if ( tRect.fH > fVisibleH ) {
		tRect.fH = fVisibleH;
	}
	if ( tRect.fX < fX ) {
		fX = tRect.fX;
	} else if ( tRect.fX + tRect.fW > fX + fVisibleW ) {
		fX = tRect.fX + tRect.fW - fVisibleW;
	}
	if ( tRect.fY < fY ) {
		fY = tRect.fY;
	} else if ( tRect.fY + tRect.fH > fY + fVisibleH ) {
		fY = tRect.fY + tRect.fH - fVisibleH;
	}
	__xgeXuiScrollViewSetOffsetInternal(pScroll, fX, fY);
}

void xgeXuiScrollViewEnsureChildVisible(xge_xui_scroll_view pScroll, xge_xui_widget pChild)
{
	xge_rect_t tRect;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pChild == NULL) ) {
		return;
	}
	tRect = pChild->tRect;
	tRect.fX = tRect.fX - pScroll->pWidget->tContentRect.fX + pScroll->fScrollX;
	tRect.fY = tRect.fY - pScroll->pWidget->tContentRect.fY + pScroll->fScrollY;
	xgeXuiScrollViewEnsureRectVisible(pScroll, tRect);
}

void xgeXuiScrollViewSetScrollbarPolicy(xge_xui_scroll_view pScroll, int iPolicy)
{
	if ( pScroll == NULL ) {
		return;
	}
	if ( (iPolicy != XGE_XUI_SCROLLBAR_POLICY_ALWAYS) && (iPolicy != XGE_XUI_SCROLLBAR_POLICY_HIDDEN) ) {
		iPolicy = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	}
	pScroll->iScrollbarPolicy = iPolicy;
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

void xgeXuiScrollViewSetScrollbarMode(xge_xui_scroll_view pScroll, int iMode)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

int xgeXuiScrollViewGetScrollbarMode(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

static int __xgeXuiScrollViewWheelAxisClamp(int iAxis)
{
	if ( (iAxis == XGE_XUI_WHEEL_AXIS_HORIZONTAL) || (iAxis == XGE_XUI_WHEEL_AXIS_BOTH) ) {
		return iAxis;
	}
	return XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollViewSetNestedScrollPolicy(xge_xui_scroll_view pScroll, int iPolicy)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->iNestedScrollPolicy = (iPolicy == XGE_XUI_NESTED_SCROLL_PASS_EDGE) ? XGE_XUI_NESTED_SCROLL_PASS_EDGE : XGE_XUI_NESTED_SCROLL_CONSUME;
}

void xgeXuiScrollViewSetWheelAxis(xge_xui_scroll_view pScroll, int iAxis)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->iWheelAxis = __xgeXuiScrollViewWheelAxisClamp(iAxis);
}

int xgeXuiScrollViewGetWheelAxis(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? __xgeXuiScrollViewWheelAxisClamp(pScroll->iWheelAxis) : XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollViewSetContentDragEnabled(xge_xui_scroll_view pScroll, int bEnabled)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->bContentDragEnabled = bEnabled ? 1 : 0;
}

int xgeXuiScrollViewIsContentDragEnabled(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->bContentDragEnabled : 0;
}

void xgeXuiScrollViewSetScrollbarDragEnabled(xge_xui_scroll_view pScroll, int bEnabled)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->bScrollbarDragEnabled = bEnabled ? 1 : 0;
}

int xgeXuiScrollViewIsScrollbarDragEnabled(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->bScrollbarDragEnabled : 0;
}

void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pScroll->pWidget, iBackground);
	pScroll->iBarColor = iBar;
	pScroll->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pScroll->pWidget);
}

int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iWheelAxis;
	float fWheelX;
	float fWheelY;
	float fOldX;
	float fOldY;

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
			fOldX = pScroll->fScrollX;
			fOldY = pScroll->fScrollY;
			iWheelAxis = __xgeXuiScrollViewWheelAxisClamp(pScroll->iWheelAxis);
			fWheelX = 0.0f;
			fWheelY = 0.0f;
			if ( iWheelAxis == XGE_XUI_WHEEL_AXIS_HORIZONTAL ) {
				fWheelX = (pEvent->fDX != 0.0f) ? pEvent->fDX : pEvent->fDY;
			} else if ( iWheelAxis == XGE_XUI_WHEEL_AXIS_BOTH ) {
				fWheelX = pEvent->fDX;
				fWheelY = pEvent->fDY;
			} else {
				fWheelY = pEvent->fDY;
			}
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX - fWheelX * 32.0f, pScroll->fScrollY - fWheelY * 32.0f);
			if ( (pScroll->iNestedScrollPolicy == XGE_XUI_NESTED_SCROLL_PASS_EDGE) && (fOldX == pScroll->fScrollX) && (fOldY == pScroll->fScrollY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
			pScroll->fDragScrollX = pScroll->fScrollX;
			pScroll->fDragScrollY = pScroll->fScrollY;
			if ( __xgeXuiScrollViewVerticalBar(pScroll, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
				xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, pScroll->pWidget);
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					if ( pScroll->bScrollbarDragEnabled == 0 ) {
						xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, NULL);
						return XGE_XUI_EVENT_CONTINUE;
					}
					pScroll->bDragging = 2;
				} else {
					__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX, pScroll->fScrollY + ((pEvent->fY < tThumb.fY) ? -pScroll->pWidget->tContentRect.fH : pScroll->pWidget->tContentRect.fH));
					pScroll->bDragging = 0;
					xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, NULL);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
				xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, pScroll->pWidget);
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					if ( pScroll->bScrollbarDragEnabled == 0 ) {
						xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, NULL);
						return XGE_XUI_EVENT_CONTINUE;
					}
					pScroll->bDragging = 3;
				} else {
					__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + ((pEvent->fX < tThumb.fX) ? -pScroll->pWidget->tContentRect.fW : pScroll->pWidget->tContentRect.fW), pScroll->fScrollY);
					pScroll->bDragging = 0;
					xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, NULL);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pScroll->bContentDragEnabled == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
			xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, pScroll->pWidget);
			pScroll->bDragging = 1;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pScroll->pContext, pEvent->iPointerId) != pScroll->pWidget ) {
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
			if ( xgeXuiGetPointerCapture(pScroll->pContext, pEvent->iPointerId) != pScroll->pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->bDragging = 0;
			if ( pScroll->pContext != NULL && xgeXuiGetPointerCapture(pScroll->pContext, pEvent->iPointerId) == pScroll->pWidget ) {
				xgeXuiSetPointerCapture(pScroll->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pScroll->bDragging = 0;
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

static void __xgeXuiScrollViewPaintBar(xge_xui_scroll_view pScroll, xge_rect_t tBar, xge_rect_t tThumb)
{
	xge_rect_t tVisual;

	if ( pScroll == NULL ) {
		return;
	}
	if ( pScroll->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
		tVisual = tThumb;
		if ( tBar.fW <= tBar.fH ) {
			tVisual.fX += (tVisual.fW - 4.0f) * 0.5f;
			tVisual.fW = 4.0f;
		} else {
			tVisual.fY += (tVisual.fH - 4.0f) * 0.5f;
			tVisual.fH = 4.0f;
		}
		__xgeXuiHostDrawRoundedRect(tVisual, pScroll->iThumbColor, 2.0f);
		return;
	}
	__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
	__xgeXuiHostDrawBorderRect(tBar, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
	__xgeXuiHostDrawRect(tThumb, pScroll->iThumbColor);
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
	if ( __xgeXuiScrollViewVerticalBar(pScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(pScroll, tBar, tThumb);
	}
	if ( __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(pScroll, tBar, tThumb);
	}
}

void xgeXuiScrollViewBaseSetContentSize(xge_xui_scroll_view_base pBase, float fWidth, float fHeight)
{
	xgeXuiScrollViewSetContentSize((xge_xui_scroll_view)pBase, fWidth, fHeight);
}

void xgeXuiScrollViewBaseSetOffset(xge_xui_scroll_view_base pBase, float fX, float fY)
{
	xgeXuiScrollViewSetOffset((xge_xui_scroll_view)pBase, fX, fY);
}

void xgeXuiScrollViewBaseScrollBy(xge_xui_scroll_view_base pBase, float fDX, float fDY)
{
	xgeXuiScrollViewScrollBy((xge_xui_scroll_view)pBase, fDX, fDY);
}

void xgeXuiScrollViewBaseGetOffset(xge_xui_scroll_view_base pBase, float* pX, float* pY)
{
	xgeXuiScrollViewGetOffset((xge_xui_scroll_view)pBase, pX, pY);
}

void xgeXuiScrollViewBaseEnsureRectVisible(xge_xui_scroll_view_base pBase, xge_rect_t tRect)
{
	xgeXuiScrollViewEnsureRectVisible((xge_xui_scroll_view)pBase, tRect);
}

void xgeXuiScrollViewBaseEnsureChildVisible(xge_xui_scroll_view_base pBase, xge_xui_widget pChild)
{
	xgeXuiScrollViewEnsureChildVisible((xge_xui_scroll_view)pBase, pChild);
}

void xgeXuiScrollViewBaseSetScrollbarPolicy(xge_xui_scroll_view_base pBase, int iPolicy)
{
	xgeXuiScrollViewSetScrollbarPolicy((xge_xui_scroll_view)pBase, iPolicy);
}

void xgeXuiScrollViewBaseSetScrollbarMode(xge_xui_scroll_view_base pBase, int iMode)
{
	xgeXuiScrollViewSetScrollbarMode((xge_xui_scroll_view)pBase, iMode);
}

int xgeXuiScrollViewBaseGetScrollbarMode(xge_xui_scroll_view_base pBase)
{
	return xgeXuiScrollViewGetScrollbarMode((xge_xui_scroll_view)pBase);
}

void xgeXuiScrollViewBaseSetNestedScrollPolicy(xge_xui_scroll_view_base pBase, int iPolicy)
{
	xgeXuiScrollViewSetNestedScrollPolicy((xge_xui_scroll_view)pBase, iPolicy);
}

void xgeXuiScrollViewBaseSetWheelAxis(xge_xui_scroll_view_base pBase, int iAxis)
{
	xgeXuiScrollViewSetWheelAxis((xge_xui_scroll_view)pBase, iAxis);
}

int xgeXuiScrollViewBaseGetWheelAxis(xge_xui_scroll_view_base pBase)
{
	return xgeXuiScrollViewGetWheelAxis((xge_xui_scroll_view)pBase);
}

void xgeXuiScrollViewBaseSetContentDragEnabled(xge_xui_scroll_view_base pBase, int bEnabled)
{
	xgeXuiScrollViewSetContentDragEnabled((xge_xui_scroll_view)pBase, bEnabled);
}

int xgeXuiScrollViewBaseIsContentDragEnabled(xge_xui_scroll_view_base pBase)
{
	return xgeXuiScrollViewIsContentDragEnabled((xge_xui_scroll_view)pBase);
}

void xgeXuiScrollViewBaseSetScrollbarDragEnabled(xge_xui_scroll_view_base pBase, int bEnabled)
{
	xgeXuiScrollViewSetScrollbarDragEnabled((xge_xui_scroll_view)pBase, bEnabled);
}

int xgeXuiScrollViewBaseIsScrollbarDragEnabled(xge_xui_scroll_view_base pBase)
{
	return xgeXuiScrollViewIsScrollbarDragEnabled((xge_xui_scroll_view)pBase);
}

void xgeXuiScrollViewBaseSetColors(xge_xui_scroll_view_base pBase, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	xgeXuiScrollViewSetColors((xge_xui_scroll_view)pBase, iBackground, iBar, iThumb);
}

int xgeXuiScrollViewBaseEvent(xge_xui_scroll_view_base pBase, const xge_event_t* pEvent)
{
	return xgeXuiScrollViewEvent((xge_xui_scroll_view)pBase, pEvent);
}

int xgeXuiScrollViewBaseEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollViewBaseEvent((xge_xui_scroll_view_base)pUser, pEvent);
}

void xgeXuiScrollViewBasePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xgeXuiScrollViewPaintProc(pWidget, pUser);
}
