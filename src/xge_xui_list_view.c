static uint32_t __xgeXuiListViewHoverColor(uint32_t iRow)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiListViewNotifySelect(xge_xui_list_view pList)
{
	if ( (pList != NULL) && (pList->procSelect != NULL) && (pList->iSelected >= 0) ) {
		pList->procSelect(pList->pWidget, pList->iSelected, pList->pUser);
	}
}

static void __xgeXuiListViewEnsureVisible(xge_xui_list_view pList, int iIndex)
{
	float fTop;
	float fBottom;
	float fViewTop;
	float fViewBottom;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return;
	}
	fTop = (float)iIndex * pList->fItemHeight;
	fBottom = fTop + pList->fItemHeight;
	fViewTop = pList->fScrollY;
	fViewBottom = fViewTop + pList->pWidget->tContentRect.fH;
	if ( fTop < fViewTop ) {
		xgeXuiListViewSetScroll(pList, fTop);
	} else if ( fBottom > fViewBottom ) {
		xgeXuiListViewSetScroll(pList, fBottom - pList->pWidget->tContentRect.fH);
	}
}

static void __xgeXuiListViewSelectInternal(xge_xui_list_view pList, int iIndex, int bNotify)
{
	int iOld;

	if ( pList == NULL ) {
		return;
	}
	iOld = pList->iSelected;
	xgeXuiListViewSetSelected(pList, iIndex);
	if ( pList->iSelected >= 0 ) {
		__xgeXuiListViewEnsureVisible(pList, pList->iSelected);
	}
	if ( bNotify && (iOld != pList->iSelected) ) {
		__xgeXuiListViewNotifySelect(pList);
	}
}

static int __xgeXuiListViewVisibleRows(xge_xui_list_view pList)
{
	int iRows;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return 1;
	}
	iRows = (int)(pList->pWidget->tContentRect.fH / pList->fItemHeight);
	return (iRows > 0) ? iRows : 1;
}

static float __xgeXuiListViewThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiListViewBar(xge_xui_list_view pList, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fMaxScroll;

	if ( (pList == NULL) || (pList->pWidget == NULL) ) {
		return 0;
	}
	fContentH = (float)pList->iItemCount * pList->fItemHeight;
	if ( (fContentH <= pList->pWidget->tContentRect.fH) || (pList->pWidget->tContentRect.fH <= 0.0f) ) {
		return 0;
	}
	tBar.fX = pList->pWidget->tContentRect.fX + pList->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pList->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pList->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fH = __xgeXuiListViewThumbLen(tBar.fH, pList->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiListViewMaxScroll(pList);
	if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
		tThumb.fY += (tBar.fH - tThumb.fH) * (pList->fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static void __xgeXuiListViewSetScrollFromThumbDrag(xge_xui_list_view pList, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pList == NULL) || (__xgeXuiListViewBar(pList, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiListViewMaxScroll(pList);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiListViewSetScroll(pList, pList->fDragScrollY + ((fY - pList->fDragY) / fTravel) * fMaxScroll);
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
	pList->iHover = -1;
	pList->fItemHeight = 24.0f;
	pList->iBackgroundColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pList->iRowColor = XGE_COLOR_RGBA(36, 42, 50, 255);
	pList->iHoverColor = XGE_COLOR_RGBA(52, 62, 76, 255);
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
	if ( pList->pContext != NULL && pList->pContext->pCapture == pList->pWidget ) {
		xgeXuiSetCapture(pList->pContext, NULL);
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
	if ( pList->iHover >= iCount ) {
		pList->iHover = -1;
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
	pList->iHoverColor = __xgeXuiListViewHoverColor(iRow);
	pList->iSelectedColor = iSelected;
	pList->iTextColor = iText;
	pList->iBarColor = iBar;
	pList->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iIndex;
	int iInside;
	int iTarget;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pList->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pList->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pList->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiListViewSetScroll(pList, pList->fScrollY - pEvent->fDY * pList->fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pList->bDraggingThumb != 0 ) {
				__xgeXuiListViewSetScrollFromThumbDrag(pList, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = iInside ? __xgeXuiListViewIndexAt(pList, pEvent->fY) : -1;
			if ( pList->iHover != iIndex ) {
				pList->iHover = iIndex;
				xgeXuiWidgetMarkPaint(pList->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pList->pContext, pList->pWidget);
			if ( __xgeXuiListViewBar(pList, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pList->bDraggingThumb = 1;
					pList->fDragY = pEvent->fY;
					pList->fDragScrollY = pList->fScrollY;
					xgeXuiSetCapture(pList->pContext, pList->pWidget);
				} else {
					xgeXuiListViewSetScroll(pList, pList->fScrollY + ((pEvent->fY < tThumb.fY) ? -pList->pWidget->tContentRect.fH : pList->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiListViewIndexAt(pList, pEvent->fY);
			if ( iIndex >= 0 ) {
				__xgeXuiListViewSelectInternal(pList, iIndex, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pList->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) ) {
				__xgeXuiListViewSetScrollFromThumbDrag(pList, pEvent->fY);
			}
			pList->bDraggingThumb = 0;
			if ( pList->pContext != NULL && pList->pContext->pCapture == pList->pWidget ) {
				xgeXuiSetCapture(pList->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
			pList->bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( pList->iHover != -1 ) {
				pList->iHover = -1;
				xgeXuiWidgetMarkPaint(pList->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pList->pContext == NULL || pList->pContext->pFocus != pList->pWidget || pList->iItemCount <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iTarget = pList->iSelected;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + 1;
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iTarget = (iTarget < 0) ? (pList->iItemCount - 1) : iTarget - 1;
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iTarget = (iTarget < 0) ? 0 : iTarget - __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iTarget = 0;
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iTarget = pList->iItemCount - 1;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiListViewNotifySelect(pList);
				return (pList->iSelected >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iTarget < 0 ) {
				iTarget = 0;
			}
			if ( iTarget >= pList->iItemCount ) {
				iTarget = pList->iItemCount - 1;
			}
			__xgeXuiListViewSelectInternal(pList, iTarget, 1);
			return XGE_XUI_EVENT_CONSUMED;

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
	uint32_t iRowColor;

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
		iRowColor = pList->iRowColor;
		if ( i == pList->iHover ) {
			iRowColor = pList->iHoverColor;
		}
		if ( i == pList->iSelected ) {
			iRowColor = pList->iSelectedColor;
		}
		__xgeXuiHostDrawRect(tRow, iRowColor);
		if ( (pList->pFont != NULL) && (pList->arrItems != NULL) && (pList->arrItems[i] != NULL) ) {
			tText = tRow;
			tText.fX += 4.0f;
			tText.fW -= 8.0f;
			__xgeXuiHostDrawTextRect(pList->pFont, pList->arrItems[i], tText, pList->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	if ( __xgeXuiListViewBar(pList, &tBar, &tThumb) != 0 ) {
		__xgeXuiHostDrawRect(tBar, pList->iBarColor);
		__xgeXuiHostDrawRect(tThumb, pList->iThumbColor);
	}
}
