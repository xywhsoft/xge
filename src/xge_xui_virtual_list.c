static int __xgeXuiVirtualListSyncCount(xge_xui_virtual_list pList)
{
	int iCount;

	if ( pList == NULL ) {
		return 0;
	}
	if ( pList->procCount != NULL ) {
		iCount = pList->procCount(pList->pWidget, pList->pUser);
		if ( iCount < 0 ) {
			iCount = 0;
		}
		pList->iItemCount = iCount;
	}
	if ( pList->iSelected >= pList->iItemCount ) {
		pList->iSelected = -1;
	}
	return pList->iItemCount;
}

static float __xgeXuiVirtualListItemHeightAt(xge_xui_virtual_list pList, int iIndex)
{
	float fHeight;

	if ( pList == NULL ) {
		return 0.0f;
	}
	fHeight = 0.0f;
	if ( (pList->procHeight != NULL) && (iIndex >= 0) && (iIndex < __xgeXuiVirtualListSyncCount(pList)) ) {
		fHeight = pList->procHeight(pList->pWidget, iIndex, pList->pUser);
	}
	if ( fHeight <= 0.0f ) {
		fHeight = pList->fItemHeight;
	}
	return (fHeight > 0.0f) ? fHeight : 1.0f;
}

static float __xgeXuiVirtualListContentHeight(xge_xui_virtual_list pList)
{
	float fHeight;
	int i;
	int iCount;

	if ( pList == NULL ) {
		return 0.0f;
	}
	iCount = __xgeXuiVirtualListSyncCount(pList);
	if ( pList->procHeight == NULL ) {
		return (float)iCount * pList->fItemHeight;
	}
	fHeight = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		fHeight += __xgeXuiVirtualListItemHeightAt(pList, i);
	}
	return fHeight;
}

static float __xgeXuiVirtualListItemTop(xge_xui_virtual_list pList, int iIndex)
{
	float fTop;
	int i;
	int iCount;

	if ( pList == NULL || iIndex <= 0 ) {
		return 0.0f;
	}
	iCount = __xgeXuiVirtualListSyncCount(pList);
	if ( iIndex > iCount ) {
		iIndex = iCount;
	}
	if ( pList->procHeight == NULL ) {
		return (float)iIndex * pList->fItemHeight;
	}
	fTop = 0.0f;
	for ( i = 0; i < iIndex; i++ ) {
		fTop += __xgeXuiVirtualListItemHeightAt(pList, i);
	}
	return fTop;
}

static float __xgeXuiVirtualListMaxScroll(xge_xui_virtual_list pList)
{
	float fMax;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	fMax = __xgeXuiVirtualListContentHeight(pList) - pList->pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiVirtualListClamp(xge_xui_virtual_list pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->fScrollY = __xgeXuiClampFloat(pList->fScrollY, 0.0f, __xgeXuiVirtualListMaxScroll(pList));
}

static int __xgeXuiVirtualListIndexAt(xge_xui_virtual_list pList, float fY)
{
	float fOffset;
	float fTop;
	int iIndex;
	int iCount;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return -1;
	}
	fOffset = fY - pList->pWidget->tContentRect.fY + pList->fScrollY;
	if ( fOffset < 0.0f ) {
		return -1;
	}
	iCount = __xgeXuiVirtualListSyncCount(pList);
	if ( pList->procHeight == NULL ) {
		iIndex = (int)(fOffset / pList->fItemHeight);
		return ((iIndex >= 0) && (iIndex < iCount)) ? iIndex : -1;
	}
	fTop = 0.0f;
	for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
		fTop += __xgeXuiVirtualListItemHeightAt(pList, iIndex);
		if ( fOffset < fTop ) {
			return iIndex;
		}
	}
	return -1;
}

static float __xgeXuiVirtualListThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiVirtualListBar(xge_xui_virtual_list pList, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fMaxScroll;

	if ( (pList == NULL) || (pList->pWidget == NULL) ) {
		return 0;
	}
	fContentH = __xgeXuiVirtualListContentHeight(pList);
	if ( (fContentH <= pList->pWidget->tContentRect.fH) || (pList->pWidget->tContentRect.fH <= 0.0f) ) {
		return 0;
	}
	tBar.fX = pList->pWidget->tContentRect.fX + pList->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pList->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pList->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fH = __xgeXuiVirtualListThumbLen(tBar.fH, pList->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiVirtualListMaxScroll(pList);
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

static void __xgeXuiVirtualListSetScrollFromThumbDrag(xge_xui_virtual_list pList, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pList == NULL) || (__xgeXuiVirtualListBar(pList, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiVirtualListMaxScroll(pList);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiVirtualListSetScroll(pList, pList->fDragScrollY + ((fY - pList->fDragY) / fTravel) * fMaxScroll);
}

static xge_xui_widget __xgeXuiVirtualListCreateSlot(xge_xui_virtual_list pList, int iSlot)
{
	xge_xui_widget pSlot;

	if ( pList == NULL ) {
		return NULL;
	}
	if ( pList->procCreate != NULL ) {
		pSlot = pList->procCreate(pList->pWidget, iSlot, pList->pUser);
	} else {
		pSlot = xgeXuiWidgetCreate();
	}
	if ( pSlot != NULL ) {
		xgeXuiWidgetSetVisible(pSlot, 1);
		xgeXuiWidgetAdd(pList->pWidget, pSlot);
	}
	return pSlot;
}

static xge_xui_widget __xgeXuiVirtualListEnsureSlot(xge_xui_virtual_list pList, int iSlot)
{
	if ( (pList == NULL) || (iSlot < 0) || (iSlot >= XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY) ) {
		return NULL;
	}
	if ( pList->arrSlotWidget[iSlot] != NULL ) {
		return pList->arrSlotWidget[iSlot];
	}
	pList->arrSlotWidget[iSlot] = __xgeXuiVirtualListCreateSlot(pList, iSlot);
	if ( pList->arrSlotWidget[iSlot] != NULL ) {
		pList->arrSlotIndex[iSlot] = -1;
		if ( iSlot >= pList->iSlotCount ) {
			pList->iSlotCount = iSlot + 1;
		}
	}
	return pList->arrSlotWidget[iSlot];
}

static void __xgeXuiVirtualListFreeSlots(xge_xui_virtual_list pList)
{
	int i;

	if ( pList == NULL ) {
		return;
	}
	for ( i = 0; i < pList->iSlotCount; i++ ) {
		if ( pList->arrSlotWidget[i] != NULL ) {
			xgeXuiWidgetFree(pList->arrSlotWidget[i]);
			pList->arrSlotWidget[i] = NULL;
		}
		pList->arrSlotIndex[i] = -1;
	}
	pList->iSlotCount = 0;
	pList->iVisibleStart = 0;
	pList->iVisibleCount = 0;
}

static void __xgeXuiVirtualListNotifySelect(xge_xui_virtual_list pList)
{
	if ( (pList != NULL) && (pList->procSelect != NULL) && (pList->iSelected >= 0) && (pList->iSelected < __xgeXuiVirtualListSyncCount(pList)) ) {
		pList->procSelect(pList->pWidget, pList->iSelected, pList->pSelectUser);
	}
}

static void __xgeXuiVirtualListEnsureVisible(xge_xui_virtual_list pList, int iIndex)
{
	float fTop;
	float fBottom;
	float fViewBottom;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (iIndex < 0) || (iIndex >= __xgeXuiVirtualListSyncCount(pList)) ) {
		return;
	}
	fTop = __xgeXuiVirtualListItemTop(pList, iIndex);
	fBottom = fTop + __xgeXuiVirtualListItemHeightAt(pList, iIndex);
	fViewBottom = pList->fScrollY + pList->pWidget->tContentRect.fH;
	if ( fTop < pList->fScrollY ) {
		xgeXuiVirtualListSetScroll(pList, fTop);
	} else if ( fBottom > fViewBottom ) {
		xgeXuiVirtualListSetScroll(pList, fBottom - pList->pWidget->tContentRect.fH);
	}
}

int xgeXuiVirtualListInit(xge_xui_virtual_list pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pList == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pList, 0, sizeof(*pList));
	pList->pContext = pContext;
	pList->pWidget = pWidget;
	pList->iSelected = -1;
	pList->fItemHeight = 24.0f;
	pList->iBackgroundColor = XGE_COLOR_RGBA(24, 28, 34, 255);
	pList->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pList->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	for ( i = 0; i < XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY; i++ ) {
		pList->arrSlotIndex[i] = -1;
	}
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiVirtualListEventProc;
	pWidget->procPaint = xgeXuiVirtualListPaintProc;
	pWidget->pUser = pList;
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiVirtualListLayoutProc, pList);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiVirtualListUnit(xge_xui_virtual_list pList)
{
	if ( pList == NULL ) {
		return;
	}
	if ( pList->pContext != NULL && pList->pContext->pCapture == pList->pWidget ) {
		xgeXuiSetCapture(pList->pContext, NULL);
	}
	__xgeXuiVirtualListFreeSlots(pList);
	if ( pList->pWidget != NULL && pList->pWidget->pUser == pList ) {
		pList->pWidget->pUser = NULL;
		pList->pWidget->procEvent = NULL;
		pList->pWidget->procPaint = NULL;
	}
	if ( pList->pWidget != NULL && pList->pWidget->pLayoutUser == pList ) {
		pList->pWidget->pLayoutUser = NULL;
		pList->pWidget->procLayout = NULL;
	}
	memset(pList, 0, sizeof(*pList));
}

void xgeXuiVirtualListSetAdapter(xge_xui_virtual_list pList, xge_xui_virtual_list_count_proc procCount, xge_xui_virtual_list_create_proc procCreate, xge_xui_virtual_list_bind_proc procBind, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	__xgeXuiVirtualListFreeSlots(pList);
	pList->procCount = procCount;
	pList->procCreate = procCreate;
	pList->procBind = procBind;
	pList->pUser = pUser;
	__xgeXuiVirtualListSyncCount(pList);
	__xgeXuiVirtualListClamp(pList);
	xgeXuiWidgetMarkLayout(pList->pWidget);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiVirtualListSetItemCount(xge_xui_virtual_list pList, int iCount)
{
	if ( pList == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pList->iItemCount = iCount;
	if ( pList->iSelected >= iCount ) {
		pList->iSelected = -1;
	}
	__xgeXuiVirtualListClamp(pList);
	xgeXuiWidgetMarkLayout(pList->pWidget);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiVirtualListSetItemHeight(xge_xui_virtual_list pList, float fHeight)
{
	if ( (pList == NULL) || (fHeight <= 0.0f) ) {
		return;
	}
	pList->fItemHeight = fHeight;
	__xgeXuiVirtualListClamp(pList);
	xgeXuiWidgetMarkLayout(pList->pWidget);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiVirtualListSetItemHeightProc(xge_xui_virtual_list pList, xge_xui_virtual_list_height_proc procHeight)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procHeight = procHeight;
	__xgeXuiVirtualListClamp(pList);
	xgeXuiWidgetMarkLayout(pList->pWidget);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

void xgeXuiVirtualListSetScroll(xge_xui_virtual_list pList, float fScrollY)
{
	float fOld;

	if ( pList == NULL ) {
		return;
	}
	fOld = pList->fScrollY;
	pList->fScrollY = fScrollY;
	__xgeXuiVirtualListClamp(pList);
	if ( pList->fScrollY != fOld ) {
		xgeXuiWidgetMarkLayout(pList->pWidget);
		xgeXuiWidgetMarkPaint(pList->pWidget);
	}
}

float xgeXuiVirtualListGetScroll(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? pList->fScrollY : 0.0f;
}

void xgeXuiVirtualListEnsureVisible(xge_xui_virtual_list pList, int iIndex)
{
	__xgeXuiVirtualListEnsureVisible(pList, iIndex);
}

void xgeXuiVirtualListRefresh(xge_xui_virtual_list pList)
{
	int i;

	if ( pList == NULL ) {
		return;
	}
	for ( i = 0; i < pList->iSlotCount; i++ ) {
		pList->arrSlotIndex[i] = -1;
	}
	__xgeXuiVirtualListSyncCount(pList);
	__xgeXuiVirtualListClamp(pList);
	xgeXuiWidgetMarkLayout(pList->pWidget);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiVirtualListGetFirstVisible(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? pList->iVisibleStart : 0;
}

int xgeXuiVirtualListGetVisibleCount(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? pList->iVisibleCount : 0;
}

xge_xui_widget xgeXuiVirtualListGetSlotWidget(xge_xui_virtual_list pList, int iSlot)
{
	if ( (pList == NULL) || (iSlot < 0) || (iSlot >= pList->iSlotCount) ) {
		return NULL;
	}
	return pList->arrSlotWidget[iSlot];
}

void xgeXuiVirtualListSetSelect(xge_xui_virtual_list pList, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procSelect = procSelect;
	pList->pSelectUser = pUser;
}

void xgeXuiVirtualListSetSelected(xge_xui_virtual_list pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= __xgeXuiVirtualListSyncCount(pList)) ) {
		iIndex = -1;
	}
	if ( pList->iSelected == iIndex ) {
		return;
	}
	pList->iSelected = iIndex;
	__xgeXuiVirtualListEnsureVisible(pList, iIndex);
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiVirtualListGetSelected(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? pList->iSelected : -1;
}

void xgeXuiVirtualListSetColors(xge_xui_virtual_list pList, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iBackgroundColor = iBackground;
	pList->iBarColor = iBar;
	pList->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiVirtualListEvent(xge_xui_virtual_list pList, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iIndex;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pList->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pList->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pList->pWidget->tContentRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiVirtualListSetScroll(pList, pList->fScrollY - pEvent->fDY * __xgeXuiVirtualListItemHeightAt(pList, pList->iSelected >= 0 ? pList->iSelected : 0));
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pList->pContext, pList->pWidget);
			if ( __xgeXuiVirtualListBar(pList, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pList->bDraggingThumb = 1;
					pList->fDragY = pEvent->fY;
					pList->fDragScrollY = pList->fScrollY;
					xgeXuiSetCapture(pList->pContext, pList->pWidget);
				} else {
					xgeXuiVirtualListSetScroll(pList, pList->fScrollY + ((pEvent->fY < tThumb.fY) ? -pList->pWidget->tContentRect.fH : pList->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiVirtualListIndexAt(pList, pEvent->fY);
			if ( iIndex >= 0 ) {
				xgeXuiVirtualListSetSelected(pList, iIndex);
				__xgeXuiVirtualListNotifySelect(pList);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pList->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiVirtualListSetScrollFromThumbDrag(pList, pEvent->fY);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pList->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pList->bDraggingThumb = 0;
			if ( pList->pContext != NULL && pList->pContext->pCapture == pList->pWidget ) {
				xgeXuiSetCapture(pList->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_KEY_DOWN:
			if ( pList->pContext == NULL || pList->pContext->pFocus != pList->pWidget || __xgeXuiVirtualListSyncCount(pList) <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iIndex = pList->iSelected;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iIndex = (iIndex < 0) ? 0 : iIndex + 1;
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iIndex = (iIndex < 0) ? (pList->iItemCount - 1) : iIndex - 1;
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iIndex = (iIndex < 0) ? 0 : iIndex + ((int)(pList->pWidget->tContentRect.fH / pList->fItemHeight));
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iIndex = (iIndex < 0) ? 0 : iIndex - ((int)(pList->pWidget->tContentRect.fH / pList->fItemHeight));
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iIndex = 0;
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iIndex = pList->iItemCount - 1;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiVirtualListNotifySelect(pList);
				return (pList->iSelected >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iIndex < 0 ) {
				iIndex = 0;
			}
			if ( iIndex >= pList->iItemCount ) {
				iIndex = pList->iItemCount - 1;
			}
			xgeXuiVirtualListSetSelected(pList, iIndex);
			__xgeXuiVirtualListNotifySelect(pList);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiVirtualListEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiVirtualListEvent((xge_xui_virtual_list)pUser, pEvent);
}

void xgeXuiVirtualListLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_list pList;
	xge_xui_widget pSlot;
	xge_rect_t tRect;
	float fY;
	float fBottom;
	int iCount;
	int iFirst;
	int iVisible;
	int iSlot;
	int iIndex;

	(void)pWidget;
	pList = (xge_xui_virtual_list)pUser;
	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return;
	}
	iCount = __xgeXuiVirtualListSyncCount(pList);
	__xgeXuiVirtualListClamp(pList);
	if ( pList->procHeight == NULL ) {
		iFirst = (int)(pList->fScrollY / pList->fItemHeight);
	} else {
		iFirst = __xgeXuiVirtualListIndexAt(pList, pList->pWidget->tContentRect.fY);
	}
	if ( iFirst < 0 ) {
		iFirst = 0;
	}
	if ( iFirst > iCount ) {
		iFirst = iCount;
	}
	if ( pList->procHeight == NULL ) {
		iVisible = (int)(pList->pWidget->tContentRect.fH / pList->fItemHeight) + 2;
		if ( iVisible < 0 ) {
			iVisible = 0;
		}
		if ( iVisible > XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY ) {
			iVisible = XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY;
		}
		if ( iVisible > (iCount - iFirst) ) {
			iVisible = iCount - iFirst;
		}
	} else {
		iVisible = 0;
		fY = __xgeXuiVirtualListItemTop(pList, iFirst);
		fBottom = pList->fScrollY + pList->pWidget->tContentRect.fH;
		while ( (iFirst + iVisible < iCount) && (iVisible < XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY) && (fY < fBottom + pList->fItemHeight * 2.0f) ) {
			fY += __xgeXuiVirtualListItemHeightAt(pList, iFirst + iVisible);
			iVisible++;
		}
	}
	pList->iVisibleStart = iFirst;
	pList->iVisibleCount = iVisible;
	for ( iSlot = 0; iSlot < iVisible; iSlot++ ) {
		iIndex = iFirst + iSlot;
		pSlot = __xgeXuiVirtualListEnsureSlot(pList, iSlot);
		if ( pSlot == NULL ) {
			continue;
		}
		tRect.fX = pList->pWidget->tContentRect.fX;
		tRect.fY = pList->pWidget->tContentRect.fY + __xgeXuiVirtualListItemTop(pList, iIndex) - pList->fScrollY;
		tRect.fW = pList->pWidget->tContentRect.fW;
		tRect.fH = __xgeXuiVirtualListItemHeightAt(pList, iIndex);
		xgeXuiWidgetSetRect(pSlot, tRect);
		xgeXuiWidgetSetVisible(pSlot, 1);
		if ( pList->arrSlotIndex[iSlot] != iIndex ) {
			pList->arrSlotIndex[iSlot] = iIndex;
			if ( pList->procBind != NULL ) {
				pList->procBind(pSlot, iIndex, pList->pUser);
			}
		}
	}
	for ( iSlot = iVisible; iSlot < pList->iSlotCount; iSlot++ ) {
		if ( pList->arrSlotWidget[iSlot] != NULL ) {
			xgeXuiWidgetSetVisible(pList->arrSlotWidget[iSlot], 0);
		}
		pList->arrSlotIndex[iSlot] = -1;
	}
}

void xgeXuiVirtualListPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_list pList;
	xge_rect_t tBar;
	xge_rect_t tThumb;

	pList = (xge_xui_virtual_list)pUser;
	if ( (pWidget == NULL) || (pList == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pList->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pList->iBackgroundColor);
	}
	if ( __xgeXuiVirtualListBar(pList, &tBar, &tThumb) != 0 ) {
		__xgeXuiHostDrawRect(tBar, pList->iBarColor);
		__xgeXuiHostDrawRect(tThumb, pList->iThumbColor);
	}
}
