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
	if ( pList->iHover >= pList->iItemCount ) {
		pList->iHover = -1;
	}
	if ( pList->iFocus >= pList->iItemCount ) {
		pList->iFocus = -1;
	}
	return pList->iItemCount;
}

static int __xgeXuiVirtualListNormalizeIndex(xge_xui_virtual_list pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= __xgeXuiVirtualListSyncCount(pList)) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiVirtualListSetHover(xge_xui_virtual_list pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiVirtualListNormalizeIndex(pList, iIndex);
	if ( pList->iHover == iIndex ) {
		return;
	}
	pList->iHover = iIndex;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

static void __xgeXuiVirtualListSetFocusIndex(xge_xui_virtual_list pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiVirtualListNormalizeIndex(pList, iIndex);
	if ( pList->iFocus == iIndex ) {
		return;
	}
	pList->iFocus = iIndex;
	xgeXuiWidgetMarkPaint(pList->pWidget);
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
	float fSize;
	float fButton;
	float fTrackH;

	if ( (pList == NULL) || (pList->pWidget == NULL) ) {
		return 0;
	}
	fContentH = __xgeXuiVirtualListContentHeight(pList);
	if ( (fContentH <= pList->pWidget->tContentRect.fH) || (pList->pWidget->tContentRect.fH <= 0.0f) ) {
		return 0;
	}
	fSize = (pList->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pList->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pList->pWidget->tContentRect.fX + pList->pWidget->tContentRect.fW - fSize;
	tBar.fY = pList->pWidget->tContentRect.fY;
	tBar.fW = fSize;
	tBar.fH = pList->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	fTrackH = tThumb.fH;
	if ( fTrackH < 1.0f ) {
		fTrackH = 1.0f;
		tThumb.fH = 1.0f;
	}
	tThumb.fH = __xgeXuiVirtualListThumbLen(fTrackH, pList->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiVirtualListMaxScroll(pList);
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pList->fScrollY / fMaxScroll);
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
		xgeXuiWidgetAddInternal(pList->pWidget, pSlot);
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

int xgeXuiVirtualScrollViewBaseInit(xge_xui_virtual_scroll_view_base pBase, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pBase == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBase, 0, sizeof(*pBase));
	__xgeXuiViewportWidgetInit(pWidget, 1);
	pBase->pContext = pContext;
	pBase->pWidget = pWidget;
	pBase->iSelected = -1;
	pBase->iHover = -1;
	pBase->iFocus = -1;
	pBase->fItemHeight = 24.0f;
	pBase->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pBase->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	pBase->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	for ( i = 0; i < XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY; i++ ) {
		pBase->arrSlotIndex[i] = -1;
	}
	xgeXuiWidgetSetOverflow(pWidget, XGE_XUI_OVERFLOW_SCROLL);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiVirtualScrollViewBaseEventProc, NULL);
	pWidget->procPaint = xgeXuiVirtualScrollViewBasePaintProc;
	pWidget->pUser = pBase;
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiVirtualScrollViewBaseLayoutProc, pBase);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(24, 28, 34, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

int xgeXuiVirtualListInit(xge_xui_virtual_list pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iResult;

	iResult = xgeXuiVirtualScrollViewBaseInit((xge_xui_virtual_scroll_view_base)pList, pContext, pWidget);
	if ( iResult != XGE_OK ) {
		return iResult;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiVirtualListEventProc, NULL);
	pWidget->procPaint = xgeXuiVirtualListPaintProc;
	pWidget->pUser = pList;
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiVirtualListLayoutProc, pList);
	return XGE_OK;
}

void xgeXuiVirtualScrollViewBaseUnit(xge_xui_virtual_scroll_view_base pBase)
{
	if ( pBase == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pBase->pContext, pBase->pWidget);
	__xgeXuiVirtualListFreeSlots((xge_xui_virtual_list)pBase);
	if ( pBase->pWidget != NULL && pBase->pWidget->pUser == pBase && ((pBase->pWidget->procEvent == xgeXuiVirtualScrollViewBaseEventProc) || (pBase->pWidget->procEvent == xgeXuiVirtualListEventProc)) ) {
		pBase->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pBase->pWidget, NULL, NULL);
		pBase->pWidget->procPaint = NULL;
	}
	if ( pBase->pWidget != NULL && pBase->pWidget->pLayoutUser == pBase && ((pBase->pWidget->procLayout == xgeXuiVirtualScrollViewBaseLayoutProc) || (pBase->pWidget->procLayout == xgeXuiVirtualListLayoutProc)) ) {
		pBase->pWidget->pLayoutUser = NULL;
		pBase->pWidget->procLayout = NULL;
	}
	memset(pBase, 0, sizeof(*pBase));
}

void xgeXuiVirtualListUnit(xge_xui_virtual_list pList)
{
	xgeXuiVirtualScrollViewBaseUnit((xge_xui_virtual_scroll_view_base)pList);
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
	if ( pList->iHover >= iCount ) {
		pList->iHover = -1;
	}
	if ( pList->iFocus >= iCount ) {
		pList->iFocus = -1;
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

void xgeXuiVirtualListSetScrollbarMode(xge_xui_virtual_list pList, int iMode)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

int xgeXuiVirtualListGetScrollbarMode(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? pList->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
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
		if ( pList->iFocus != iIndex ) {
			pList->iFocus = iIndex;
			xgeXuiWidgetMarkPaint(pList->pWidget);
		}
		return;
	}
	pList->iSelected = iIndex;
	pList->iFocus = iIndex;
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
	xgeXuiWidgetSetBackground(pList->pWidget, iBackground);
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
					xgeXuiSetPointerCapture(pList->pContext, pEvent->iPointerId, pList->pWidget);
				} else {
					xgeXuiVirtualListSetScroll(pList, pList->fScrollY + ((pEvent->fY < tThumb.fY) ? -pList->pWidget->tContentRect.fH : pList->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiVirtualListIndexAt(pList, pEvent->fY);
			if ( iIndex >= 0 ) {
				__xgeXuiVirtualListSetFocusIndex(pList, iIndex);
				xgeXuiVirtualListSetSelected(pList, iIndex);
				__xgeXuiVirtualListNotifySelect(pList);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pList->bDraggingThumb == 0 ) {
				__xgeXuiVirtualListSetHover(pList, iInside ? __xgeXuiVirtualListIndexAt(pList, pEvent->fY) : -1);
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pList->pContext, pEvent->iPointerId) != pList->pWidget ) {
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
			if ( xgeXuiGetPointerCapture(pList->pContext, pEvent->iPointerId) != pList->pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pList->bDraggingThumb = 0;
			if ( pList->pContext != NULL && xgeXuiGetPointerCapture(pList->pContext, pEvent->iPointerId) == pList->pWidget ) {
				xgeXuiSetPointerCapture(pList->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pList->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pList->bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiVirtualListSetHover(pList, -1);
			return XGE_XUI_EVENT_CONTINUE;

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
	if ( __xgeXuiVirtualListBar(pList, &tBar, &tThumb) != 0 ) {
		if ( pList->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pList->iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
			__xgeXuiHostDrawRect(tThumb, pList->iThumbColor);
		}
	}
}

void xgeXuiVirtualScrollViewBaseSetAdapter(xge_xui_virtual_scroll_view_base pBase, xge_xui_virtual_scroll_count_proc procCount, xge_xui_virtual_scroll_create_proc procCreate, xge_xui_virtual_scroll_bind_proc procBind, void* pUser)
{
	xgeXuiVirtualListSetAdapter((xge_xui_virtual_list)pBase, procCount, procCreate, procBind, pUser);
}

void xgeXuiVirtualScrollViewBaseSetItemCount(xge_xui_virtual_scroll_view_base pBase, int iCount)
{
	xgeXuiVirtualListSetItemCount((xge_xui_virtual_list)pBase, iCount);
}

void xgeXuiVirtualScrollViewBaseSetItemHeight(xge_xui_virtual_scroll_view_base pBase, float fHeight)
{
	xgeXuiVirtualListSetItemHeight((xge_xui_virtual_list)pBase, fHeight);
}

void xgeXuiVirtualScrollViewBaseSetItemHeightProc(xge_xui_virtual_scroll_view_base pBase, xge_xui_virtual_scroll_height_proc procHeight)
{
	xgeXuiVirtualListSetItemHeightProc((xge_xui_virtual_list)pBase, procHeight);
}

void xgeXuiVirtualScrollViewBaseSetScroll(xge_xui_virtual_scroll_view_base pBase, float fScrollY)
{
	xgeXuiVirtualListSetScroll((xge_xui_virtual_list)pBase, fScrollY);
}

float xgeXuiVirtualScrollViewBaseGetScroll(xge_xui_virtual_scroll_view_base pBase)
{
	return xgeXuiVirtualListGetScroll((xge_xui_virtual_list)pBase);
}

void xgeXuiVirtualScrollViewBaseSetScrollbarMode(xge_xui_virtual_scroll_view_base pBase, int iMode)
{
	xgeXuiVirtualListSetScrollbarMode((xge_xui_virtual_list)pBase, iMode);
}

int xgeXuiVirtualScrollViewBaseGetScrollbarMode(xge_xui_virtual_scroll_view_base pBase)
{
	return xgeXuiVirtualListGetScrollbarMode((xge_xui_virtual_list)pBase);
}

void xgeXuiVirtualScrollViewBaseEnsureIndexVisible(xge_xui_virtual_scroll_view_base pBase, int iIndex)
{
	xgeXuiVirtualListEnsureVisible((xge_xui_virtual_list)pBase, iIndex);
}

void xgeXuiVirtualScrollViewBaseRefresh(xge_xui_virtual_scroll_view_base pBase)
{
	xgeXuiVirtualListRefresh((xge_xui_virtual_list)pBase);
}

int xgeXuiVirtualScrollViewBaseGetFirstVisible(xge_xui_virtual_scroll_view_base pBase)
{
	return xgeXuiVirtualListGetFirstVisible((xge_xui_virtual_list)pBase);
}

int xgeXuiVirtualScrollViewBaseGetVisibleCount(xge_xui_virtual_scroll_view_base pBase)
{
	return xgeXuiVirtualListGetVisibleCount((xge_xui_virtual_list)pBase);
}

xge_xui_widget xgeXuiVirtualScrollViewBaseGetSlotWidget(xge_xui_virtual_scroll_view_base pBase, int iSlot)
{
	return xgeXuiVirtualListGetSlotWidget((xge_xui_virtual_list)pBase, iSlot);
}

void xgeXuiVirtualScrollViewBaseSetSelect(xge_xui_virtual_scroll_view_base pBase, xge_xui_select_proc procSelect, void* pUser)
{
	xgeXuiVirtualListSetSelect((xge_xui_virtual_list)pBase, procSelect, pUser);
}

void xgeXuiVirtualScrollViewBaseSetSelected(xge_xui_virtual_scroll_view_base pBase, int iIndex)
{
	xgeXuiVirtualListSetSelected((xge_xui_virtual_list)pBase, iIndex);
}

int xgeXuiVirtualScrollViewBaseGetSelected(xge_xui_virtual_scroll_view_base pBase)
{
	return xgeXuiVirtualListGetSelected((xge_xui_virtual_list)pBase);
}

void xgeXuiVirtualScrollViewBaseSetHover(xge_xui_virtual_scroll_view_base pBase, int iIndex)
{
	__xgeXuiVirtualListSetHover((xge_xui_virtual_list)pBase, iIndex);
}

int xgeXuiVirtualScrollViewBaseGetHover(xge_xui_virtual_scroll_view_base pBase)
{
	return (pBase != NULL) ? pBase->iHover : -1;
}

void xgeXuiVirtualScrollViewBaseSetFocusIndex(xge_xui_virtual_scroll_view_base pBase, int iIndex)
{
	__xgeXuiVirtualListSetFocusIndex((xge_xui_virtual_list)pBase, iIndex);
}

int xgeXuiVirtualScrollViewBaseGetFocusIndex(xge_xui_virtual_scroll_view_base pBase)
{
	return (pBase != NULL) ? pBase->iFocus : -1;
}

void xgeXuiVirtualScrollViewBaseSetColors(xge_xui_virtual_scroll_view_base pBase, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	xgeXuiVirtualListSetColors((xge_xui_virtual_list)pBase, iBackground, iBar, iThumb);
}

int xgeXuiVirtualScrollViewBaseEvent(xge_xui_virtual_scroll_view_base pBase, const xge_event_t* pEvent)
{
	return xgeXuiVirtualListEvent((xge_xui_virtual_list)pBase, pEvent);
}

int xgeXuiVirtualScrollViewBaseEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiVirtualScrollViewBaseEvent((xge_xui_virtual_scroll_view_base)pUser, pEvent);
}

void xgeXuiVirtualScrollViewBaseLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xgeXuiVirtualListLayoutProc(pWidget, pUser);
}

void xgeXuiVirtualScrollViewBasePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xgeXuiVirtualListPaintProc(pWidget, pUser);
}
