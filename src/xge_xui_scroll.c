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

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	if ( pScroll == NULL ) {
		return;
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
	int iInside;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScroll->pWidget->tRect, pEvent->fX, pEvent->fY);
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
			pScroll->bDragging = 1;
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
			xgeXuiSetFocus(pScroll->pContext, pScroll->pWidget);
			xgeXuiSetCapture(pScroll->pContext, pScroll->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pScroll->bDragging == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiScrollViewSetOffsetInternal(pScroll, pScroll->fScrollX + (pScroll->fDragX - pEvent->fX), pScroll->fScrollY + (pScroll->fDragY - pEvent->fY));
			pScroll->fDragX = pEvent->fX;
			pScroll->fDragY = pEvent->fY;
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
	float fVisible;
	float fContent;
	float fMaxScroll;

	pScroll = (xge_xui_scroll_view)pUser;
	if ( (pWidget == NULL) || (pScroll == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pScroll->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pScroll->iBackgroundColor);
	}
	fVisible = pWidget->tContentRect.fH;
	fContent = pScroll->fContentH;
	if ( fContent > fVisible && fVisible > 0.0f ) {
		tBar.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 4.0f;
		tBar.fY = pWidget->tContentRect.fY;
		tBar.fW = 4.0f;
		tBar.fH = pWidget->tContentRect.fH;
		__xgeXuiHostDrawRect(tBar, pScroll->iBarColor);
		tThumb = tBar;
		tThumb.fH = tBar.fH * (fVisible / fContent);
		if ( tThumb.fH < 8.0f ) {
			tThumb.fH = 8.0f;
		}
		if ( tThumb.fH > tBar.fH ) {
			tThumb.fH = tBar.fH;
		}
		fMaxScroll = fContent - fVisible;
		tThumb.fY = tBar.fY;
		if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
			tThumb.fY += (tBar.fH - tThumb.fH) * (pScroll->fScrollY / fMaxScroll);
		}
		__xgeXuiHostDrawRect(tThumb, pScroll->iThumbColor);
	}
	fVisible = pWidget->tContentRect.fW;
	fContent = pScroll->fContentW;
	if ( fContent > fVisible && fVisible > 0.0f ) {
		tBar.fX = pWidget->tContentRect.fX;
		tBar.fY = pWidget->tContentRect.fY + pWidget->tContentRect.fH - 4.0f;
		tBar.fW = pWidget->tContentRect.fW;
		tBar.fH = 4.0f;
		__xgeXuiHostDrawRect(tBar, pScroll->iBarColor);
		tThumb = tBar;
		tThumb.fW = tBar.fW * (fVisible / fContent);
		if ( tThumb.fW < 8.0f ) {
			tThumb.fW = 8.0f;
		}
		if ( tThumb.fW > tBar.fW ) {
			tThumb.fW = tBar.fW;
		}
		fMaxScroll = fContent - fVisible;
		tThumb.fX = tBar.fX;
		if ( fMaxScroll > 0.0f && tBar.fW > tThumb.fW ) {
			tThumb.fX += (tBar.fW - tThumb.fW) * (pScroll->fScrollX / fMaxScroll);
		}
		__xgeXuiHostDrawRect(tThumb, pScroll->iThumbColor);
	}
}

int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pList == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pList, 0, sizeof(*pList));
	pList->pContext = pContext;
	pList->pWidget = pWidget;
	pList->iSelected = -1;
	pList->fItemHeight = 24.0f;
	pList->iBackgroundColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pList->iRowColor = XGE_COLOR_RGBA(36, 42, 50, 255);
	pList->iSelectedColor = XGE_COLOR_RGBA(62, 112, 172, 255);
	pList->iTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pList->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pList->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiListViewEventProc;
	pWidget->procPaint = xgeXuiListViewPaintProc;
	pWidget->pUser = pList;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiListViewUnit(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	if ( pList->pWidget != NULL && pList->pWidget->pUser == pList ) {
		pList->pWidget->pUser = NULL;
		pList->pWidget->procEvent = NULL;
		pList->pWidget->procPaint = NULL;
	}
	memset(pList, 0, sizeof(*pList));
}

void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount)
{
	if ( pList == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pList->arrItems = arrItems;
	pList->iItemCount = iCount;
	if ( pList->iSelected >= iCount ) {
		pList->iSelected = -1;
	}
	__xgeXuiListViewClamp(pList);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont)
{
	if ( pList == NULL ) {
		return;
	}
	pList->pFont = pFont;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight)
{
	if ( pList == NULL ) {
		return;
	}
	if ( fHeight < 1.0f ) {
		fHeight = 1.0f;
	}
	pList->fItemHeight = fHeight;
	__xgeXuiListViewClamp(pList);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		iIndex = -1;
	}
	if ( pList->iSelected != iIndex ) {
		pList->iSelected = iIndex;
		xgeXuiWidgetMarkPaint(pList->pWidget);
	}
}

int xgeXuiListViewGetSelected(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return -1;
	}
	return pList->iSelected;
}

void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY)
{
	float fOld;

	if ( pList == NULL ) {
		return;
	}
	fOld = pList->fScrollY;
	pList->fScrollY = fScrollY;
	__xgeXuiListViewClamp(pList);
	if ( fOld != pList->fScrollY ) {
		xgeXuiWidgetMarkPaint(pList->pWidget);
	}
}

float xgeXuiListViewGetScroll(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return 0.0f;
	}
	return pList->fScrollY;
}

void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procSelect = procSelect;
	pList->pUser = pUser;
}

void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iBackgroundColor = iBackground;
	pList->iRowColor = iRow;
	pList->iSelectedColor = iSelected;
	pList->iTextColor = iText;
	pList->iBarColor = iBar;
	pList->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent)
{
	int iIndex;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pList->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pList->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( __xgeXuiRectContains(pList->pWidget->tRect, pEvent->fX, pEvent->fY) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			xgeXuiListViewSetScroll(pList, pList->fScrollY - pEvent->fDY * pList->fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			xgeXuiSetFocus(pList->pContext, pList->pWidget);
			iIndex = __xgeXuiListViewIndexAt(pList, pEvent->fY);
			if ( iIndex >= 0 ) {
				xgeXuiListViewSetSelected(pList, iIndex);
				if ( pList->procSelect != NULL ) {
					pList->procSelect(pList->pWidget, pList->iSelected, pList->pUser);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiListViewEvent((xge_xui_list_view)pUser, pEvent);
}

void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_list_view pList;
	xge_rect_t tRow;
	xge_rect_t tText;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iFirst;
	int iLast;
	int i;
	float fContentH;
	float fMaxScroll;

	pList = (xge_xui_list_view)pUser;
	if ( (pWidget == NULL) || (pList == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pList->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pList->iBackgroundColor);
	}
	if ( (pList->fItemHeight <= 0.0f) || (pList->iItemCount <= 0) ) {
		return;
	}
	iFirst = (int)(pList->fScrollY / pList->fItemHeight);
	if ( iFirst < 0 ) {
		iFirst = 0;
	}
	iLast = iFirst + (int)(pWidget->tContentRect.fH / pList->fItemHeight) + 2;
	if ( iLast > pList->iItemCount ) {
		iLast = pList->iItemCount;
	}
	for ( i = iFirst; i < iLast; i++ ) {
		tRow.fX = pWidget->tContentRect.fX;
		tRow.fY = pWidget->tContentRect.fY + (float)i * pList->fItemHeight - pList->fScrollY;
		tRow.fW = pWidget->tContentRect.fW;
		tRow.fH = pList->fItemHeight;
		__xgeXuiHostDrawRect(tRow, (i == pList->iSelected) ? pList->iSelectedColor : pList->iRowColor);
		if ( (pList->pFont != NULL) && (pList->arrItems != NULL) && (pList->arrItems[i] != NULL) ) {
			tText = tRow;
			tText.fX += 4.0f;
			tText.fW -= 8.0f;
			__xgeXuiHostDrawTextRect(pList->pFont, pList->arrItems[i], tText, pList->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	fContentH = (float)pList->iItemCount * pList->fItemHeight;
	if ( fContentH > pWidget->tContentRect.fH && pWidget->tContentRect.fH > 0.0f ) {
		tBar.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 4.0f;
		tBar.fY = pWidget->tContentRect.fY;
		tBar.fW = 4.0f;
		tBar.fH = pWidget->tContentRect.fH;
		__xgeXuiHostDrawRect(tBar, pList->iBarColor);
		tThumb = tBar;
		tThumb.fH = tBar.fH * (pWidget->tContentRect.fH / fContentH);
		if ( tThumb.fH < 8.0f ) {
			tThumb.fH = 8.0f;
		}
		if ( tThumb.fH > tBar.fH ) {
			tThumb.fH = tBar.fH;
		}
		fMaxScroll = fContentH - pWidget->tContentRect.fH;
		if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
			tThumb.fY = tBar.fY + (tBar.fH - tThumb.fH) * (pList->fScrollY / fMaxScroll);
		}
		__xgeXuiHostDrawRect(tThumb, pList->iThumbColor);
	}
}
