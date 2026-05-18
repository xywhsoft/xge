static int __xgeXuiVirtualViewBaseSyncCount(xge_xui_virtual_view_base pList)
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

static int __xgeXuiVirtualViewBaseNormalizeIndex(xge_xui_virtual_view_base pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= __xgeXuiVirtualViewBaseSyncCount(pList)) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiVirtualViewBaseSetHoverInternal(xge_xui_virtual_view_base pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiVirtualViewBaseNormalizeIndex(pList, iIndex);
	if ( pList->iHover == iIndex ) {
		return;
	}
	pList->iHover = iIndex;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

static void __xgeXuiVirtualViewBaseSetFocusIndexInternal(xge_xui_virtual_view_base pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiVirtualViewBaseNormalizeIndex(pList, iIndex);
	if ( pList->iFocus == iIndex ) {
		return;
	}
	pList->iFocus = iIndex;
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

static float __xgeXuiVirtualViewBaseItemHeightAt(xge_xui_virtual_view_base pList, int iIndex)
{
	float fHeight;

	if ( pList == NULL ) {
		return 0.0f;
	}
	fHeight = 0.0f;
	if ( (pList->procHeight != NULL) && (iIndex >= 0) && (iIndex < __xgeXuiVirtualViewBaseSyncCount(pList)) ) {
		fHeight = pList->procHeight(pList->pWidget, iIndex, pList->pUser);
	}
	if ( fHeight <= 0.0f ) {
		fHeight = pList->fItemHeight;
	}
	return (fHeight > 0.0f) ? fHeight : 1.0f;
}

static float __xgeXuiVirtualViewBaseContentHeight(xge_xui_virtual_view_base pList)
{
	float fHeight;
	int i;
	int iCount;

	if ( pList == NULL ) {
		return 0.0f;
	}
	iCount = __xgeXuiVirtualViewBaseSyncCount(pList);
	if ( pList->procHeight == NULL ) {
		return (float)iCount * pList->fItemHeight;
	}
	fHeight = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		fHeight += __xgeXuiVirtualViewBaseItemHeightAt(pList, i);
	}
	return fHeight;
}

static float __xgeXuiVirtualViewBaseItemTop(xge_xui_virtual_view_base pList, int iIndex)
{
	float fTop;
	int i;
	int iCount;

	if ( pList == NULL || iIndex <= 0 ) {
		return 0.0f;
	}
	iCount = __xgeXuiVirtualViewBaseSyncCount(pList);
	if ( iIndex > iCount ) {
		iIndex = iCount;
	}
	if ( pList->procHeight == NULL ) {
		return (float)iIndex * pList->fItemHeight;
	}
	fTop = 0.0f;
	for ( i = 0; i < iIndex; i++ ) {
		fTop += __xgeXuiVirtualViewBaseItemHeightAt(pList, i);
	}
	return fTop;
}

static float __xgeXuiVirtualViewBaseMaxScroll(xge_xui_virtual_view_base pList)
{
	float fMax;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	fMax = __xgeXuiVirtualViewBaseContentHeight(pList) - pList->pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiVirtualViewBaseSyncScrollModel(xge_xui_virtual_view_base pList)
{
	float fContentH;
	xge_rect_t tOuter;

	if ( (pList == NULL) || (pList->pWidget == NULL) ) {
		return;
	}
	fContentH = __xgeXuiVirtualViewBaseContentHeight(pList);
	tOuter = pList->pWidget->tContentRect;
	if ( __xgeXuiScrollModelRectSame(tOuter, pList->tScroll.tViewportRect) &&
		((pList->tScroll.tOuterViewportRect.fW > 0.0f) || (pList->tScroll.tOuterViewportRect.fH > 0.0f)) ) {
		tOuter = pList->tScroll.tOuterViewportRect;
	}
	xgeXuiScrollModelSetViewport(&pList->tScroll, tOuter);
	xgeXuiScrollModelSetContentSize(&pList->tScroll, pList->tScroll.tViewportRect.fW, fContentH);
	xgeXuiScrollModelSetOffset(&pList->tScroll, 0.0f, pList->fScrollY);
	pList->fScrollY = pList->tScroll.fScrollY;
}

static void __xgeXuiVirtualViewBaseClamp(xge_xui_virtual_view_base pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->fScrollY = __xgeXuiClampFloat(pList->fScrollY, 0.0f, __xgeXuiVirtualViewBaseMaxScroll(pList));
	__xgeXuiVirtualViewBaseSyncScrollModel(pList);
}

static int __xgeXuiVirtualViewBaseIndexAt(xge_xui_virtual_view_base pList, float fY)
{
	float fOffset;
	float fContentY;
	float fTop;
	int iIndex;
	int iCount;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return -1;
	}
	__xgeXuiVirtualViewBaseSyncScrollModel(pList);
	xgeXuiScrollModelScreenToContent(&pList->tScroll, pList->pWidget->tContentRect.fX, fY, NULL, &fContentY);
	fOffset = fContentY;
	if ( fOffset < 0.0f ) {
		return -1;
	}
	iCount = __xgeXuiVirtualViewBaseSyncCount(pList);
	if ( pList->procHeight == NULL ) {
		iIndex = (int)(fOffset / pList->fItemHeight);
		return ((iIndex >= 0) && (iIndex < iCount)) ? iIndex : -1;
	}
	fTop = 0.0f;
	for ( iIndex = 0; iIndex < iCount; iIndex++ ) {
		fTop += __xgeXuiVirtualViewBaseItemHeightAt(pList, iIndex);
		if ( fOffset < fTop ) {
			return iIndex;
		}
	}
	return -1;
}

static float __xgeXuiVirtualViewBaseThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiVirtualViewBaseBar(xge_xui_virtual_view_base pList, xge_rect_t* pBar, xge_rect_t* pThumb)
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
	fContentH = __xgeXuiVirtualViewBaseContentHeight(pList);
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
	tThumb.fH = __xgeXuiVirtualViewBaseThumbLen(fTrackH, pList->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiVirtualViewBaseMaxScroll(pList);
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

static void __xgeXuiVirtualViewBaseSetScrollFromThumbDrag(xge_xui_virtual_view_base pList, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pList == NULL) || (__xgeXuiVirtualViewBaseBar(pList, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiVirtualViewBaseMaxScroll(pList);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiVirtualViewBaseSetScroll(pList, pList->fDragScrollY + ((fY - pList->fDragY) / fTravel) * fMaxScroll);
}

static xge_xui_widget __xgeXuiVirtualViewBaseCreateSlot(xge_xui_virtual_view_base pList, int iSlot)
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

static xge_xui_widget __xgeXuiVirtualViewBaseEnsureSlot(xge_xui_virtual_view_base pList, int iSlot)
{
	if ( (pList == NULL) || (iSlot < 0) || (iSlot >= XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY) ) {
		return NULL;
	}
	if ( pList->arrSlotWidget[iSlot] != NULL ) {
		return pList->arrSlotWidget[iSlot];
	}
	pList->arrSlotWidget[iSlot] = __xgeXuiVirtualViewBaseCreateSlot(pList, iSlot);
	if ( pList->arrSlotWidget[iSlot] != NULL ) {
		pList->arrSlotIndex[iSlot] = -1;
		if ( iSlot >= pList->iSlotCount ) {
			pList->iSlotCount = iSlot + 1;
		}
	}
	return pList->arrSlotWidget[iSlot];
}

static void __xgeXuiVirtualViewBaseFreeSlots(xge_xui_virtual_view_base pList)
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

static void __xgeXuiVirtualViewBaseNotifySelect(xge_xui_virtual_view_base pList)
{
	if ( (pList != NULL) && (pList->procSelect != NULL) && (pList->iSelected >= 0) && (pList->iSelected < __xgeXuiVirtualViewBaseSyncCount(pList)) ) {
		pList->procSelect(pList->pWidget, pList->iSelected, pList->pSelectUser);
	}
}

static void __xgeXuiVirtualViewBaseEnsureVisibleInternal(xge_xui_virtual_view_base pList, int iIndex)
{
	float fTop;
	float fBottom;
	float fViewBottom;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (iIndex < 0) || (iIndex >= __xgeXuiVirtualViewBaseSyncCount(pList)) ) {
		return;
	}
	fTop = __xgeXuiVirtualViewBaseItemTop(pList, iIndex);
	fBottom = fTop + __xgeXuiVirtualViewBaseItemHeightAt(pList, iIndex);
	fViewBottom = pList->fScrollY + pList->pWidget->tContentRect.fH;
	if ( fTop < pList->fScrollY ) {
		xgeXuiVirtualViewBaseSetScroll(pList, fTop);
	} else if ( fBottom > fViewBottom ) {
		xgeXuiVirtualViewBaseSetScroll(pList, fBottom - pList->pWidget->tContentRect.fH);
	}
}

int xgeXuiVirtualViewBaseInit(xge_xui_virtual_view_base pBase, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int i;

	if ( (pBase == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pBase, 0, sizeof(*pBase));
	__xgeXuiViewportWidgetInit(pWidget, 1);
	xgeXuiScrollModelInit(&pBase->tScroll, pContext, pWidget);
	xgeXuiScrollModelSetViewport(&pBase->tScroll, pWidget->tContentRect);
	pBase->pContext = pContext;
	pBase->pWidget = pWidget;
	pBase->iSelected = -1;
	pBase->iHover = -1;
	pBase->iFocus = -1;
	pBase->fItemHeight = 24.0f;
	pBase->iBarColor = XGE_COLOR_RGBA(64, 72, 84, 180);
	pBase->iThumbColor = XGE_COLOR_RGBA(160, 172, 188, 220);
	pBase->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pBase->tScroll.iBarColor = pBase->iBarColor;
	pBase->tScroll.iThumbColor = pBase->iThumbColor;
	pBase->tScroll.iScrollbarMode = pBase->iScrollbarMode;
	pBase->tScroll.iScrollbarPolicy = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	for ( i = 0; i < XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY; i++ ) {
		pBase->arrSlotIndex[i] = -1;
	}
	xgeXuiWidgetSetOverflow(pWidget, XGE_XUI_OVERFLOW_SCROLL);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiVirtualViewBaseEventProc, NULL);
	pWidget->procPaint = xgeXuiVirtualViewBasePaintProc;
	pWidget->pUser = pBase;
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiVirtualViewBaseLayoutProc, pBase);
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(24, 28, 34, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

int xgeXuiVirtualListInit(xge_xui_virtual_list pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iResult;

	if ( pList == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	iResult = xgeXuiVirtualViewBaseInit(&pList->tBase, pContext, pWidget);
	if ( iResult != XGE_OK ) {
		return iResult;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiVirtualListEventProc, NULL);
	pWidget->procPaint = xgeXuiVirtualListPaintProc;
	pWidget->pUser = pList;
	xgeXuiWidgetSetLayoutProc(pWidget, xgeXuiVirtualListLayoutProc, pList);
	return XGE_OK;
}

void xgeXuiVirtualViewBaseUnit(xge_xui_virtual_view_base pBase)
{
	if ( pBase == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pBase->pContext, pBase->pWidget);
	__xgeXuiVirtualViewBaseFreeSlots(pBase);
	if ( pBase->pWidget != NULL && pBase->pWidget->pUser == pBase && pBase->pWidget->procEvent == xgeXuiVirtualViewBaseEventProc ) {
		pBase->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pBase->pWidget, NULL, NULL);
		pBase->pWidget->procPaint = NULL;
	}
	if ( pBase->pWidget != NULL && pBase->pWidget->pLayoutUser == pBase && pBase->pWidget->procLayout == xgeXuiVirtualViewBaseLayoutProc ) {
		pBase->pWidget->pLayoutUser = NULL;
		pBase->pWidget->procLayout = NULL;
	}
	memset(pBase, 0, sizeof(*pBase));
}

void xgeXuiVirtualListUnit(xge_xui_virtual_list pList)
{
	xge_xui_widget pWidget;

	if ( pList == NULL ) {
		return;
	}
	pWidget = pList->tBase.pWidget;
	xgeXuiReleaseWidgetCapture(pList->tBase.pContext, pWidget);
	__xgeXuiVirtualViewBaseFreeSlots(&pList->tBase);
	if ( pWidget != NULL && pWidget->pUser == pList && pWidget->procEvent == xgeXuiVirtualListEventProc ) {
		pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		pWidget->procPaint = NULL;
	}
	if ( pWidget != NULL && pWidget->pLayoutUser == pList && pWidget->procLayout == xgeXuiVirtualListLayoutProc ) {
		pWidget->pLayoutUser = NULL;
		pWidget->procLayout = NULL;
	}
	memset(pList, 0, sizeof(*pList));
}

void xgeXuiVirtualViewBaseSetAdapter(xge_xui_virtual_view_base pBase, xge_xui_virtual_view_count_proc procCount, xge_xui_virtual_view_create_proc procCreate, xge_xui_virtual_view_bind_proc procBind, void* pUser)
{
	if ( pBase == NULL ) {
		return;
	}
	__xgeXuiVirtualViewBaseFreeSlots(pBase);
	pBase->procCount = procCount;
	pBase->procCreate = procCreate;
	pBase->procBind = procBind;
	pBase->pUser = pUser;
	__xgeXuiVirtualViewBaseSyncCount(pBase);
	__xgeXuiVirtualViewBaseClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetAdapter(xge_xui_virtual_list pList, xge_xui_virtual_list_count_proc procCount, xge_xui_virtual_list_create_proc procCreate, xge_xui_virtual_list_bind_proc procBind, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	xgeXuiVirtualViewBaseSetAdapter(&pList->tBase, procCount, procCreate, procBind, pUser);
}

void xgeXuiVirtualViewBaseSetItemCount(xge_xui_virtual_view_base pBase, int iCount)
{
	if ( pBase == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pBase->iItemCount = iCount;
	if ( pBase->iSelected >= iCount ) {
		pBase->iSelected = -1;
	}
	if ( pBase->iHover >= iCount ) {
		pBase->iHover = -1;
	}
	if ( pBase->iFocus >= iCount ) {
		pBase->iFocus = -1;
	}
	__xgeXuiVirtualViewBaseClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetItemCount(xge_xui_virtual_list pList, int iCount)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetItemCount(&pList->tBase, iCount);
	}
}

void xgeXuiVirtualViewBaseSetItemHeight(xge_xui_virtual_view_base pBase, float fHeight)
{
	if ( (pBase == NULL) || (fHeight <= 0.0f) ) {
		return;
	}
	pBase->fItemHeight = fHeight;
	__xgeXuiVirtualViewBaseClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetItemHeight(xge_xui_virtual_list pList, float fHeight)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetItemHeight(&pList->tBase, fHeight);
	}
}

void xgeXuiVirtualViewBaseSetItemHeightProc(xge_xui_virtual_view_base pBase, xge_xui_virtual_view_height_proc procHeight)
{
	if ( pBase == NULL ) {
		return;
	}
	pBase->procHeight = procHeight;
	__xgeXuiVirtualViewBaseClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetItemHeightProc(xge_xui_virtual_list pList, xge_xui_virtual_list_height_proc procHeight)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetItemHeightProc(&pList->tBase, procHeight);
	}
}

void xgeXuiVirtualViewBaseSetScroll(xge_xui_virtual_view_base pBase, float fScrollY)
{
	float fOld;

	if ( pBase == NULL ) {
		return;
	}
	fOld = pBase->fScrollY;
	pBase->fScrollY = fScrollY;
	__xgeXuiVirtualViewBaseClamp(pBase);
	if ( pBase->fScrollY != fOld ) {
		xgeXuiWidgetMarkLayout(pBase->pWidget);
		xgeXuiWidgetMarkPaint(pBase->pWidget);
	}
}

void xgeXuiVirtualListSetScroll(xge_xui_virtual_list pList, float fScrollY)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetScroll(&pList->tBase, fScrollY);
	}
}

float xgeXuiVirtualViewBaseGetScroll(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->fScrollY : 0.0f;
}

float xgeXuiVirtualListGetScroll(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetScroll(&pList->tBase) : 0.0f;
}

void xgeXuiVirtualViewBaseSetScrollbarMode(xge_xui_virtual_view_base pBase, int iMode)
{
	if ( pBase == NULL ) {
		return;
	}
	pBase->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pBase->tScroll.iScrollbarMode = pBase->iScrollbarMode;
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetScrollbarMode(xge_xui_virtual_list pList, int iMode)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetScrollbarMode(&pList->tBase, iMode);
	}
}

int xgeXuiVirtualViewBaseGetScrollbarMode(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

int xgeXuiVirtualListGetScrollbarMode(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetScrollbarMode(&pList->tBase) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiVirtualViewBaseEnsureIndexVisible(xge_xui_virtual_view_base pBase, int iIndex)
{
	__xgeXuiVirtualViewBaseEnsureVisibleInternal(pBase, iIndex);
}

void xgeXuiVirtualListEnsureVisible(xge_xui_virtual_list pList, int iIndex)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseEnsureIndexVisible(&pList->tBase, iIndex);
	}
}

void xgeXuiVirtualViewBaseRefresh(xge_xui_virtual_view_base pBase)
{
	int i;

	if ( pBase == NULL ) {
		return;
	}
	for ( i = 0; i < pBase->iSlotCount; i++ ) {
		pBase->arrSlotIndex[i] = -1;
	}
	__xgeXuiVirtualViewBaseSyncCount(pBase);
	__xgeXuiVirtualViewBaseClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListRefresh(xge_xui_virtual_list pList)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseRefresh(&pList->tBase);
	}
}

int xgeXuiVirtualViewBaseGetFirstVisible(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iVisibleStart : 0;
}

int xgeXuiVirtualListGetFirstVisible(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetFirstVisible(&pList->tBase) : 0;
}

int xgeXuiVirtualViewBaseGetVisibleCount(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iVisibleCount : 0;
}

int xgeXuiVirtualListGetVisibleCount(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetVisibleCount(&pList->tBase) : 0;
}

xge_xui_widget xgeXuiVirtualViewBaseGetSlotWidget(xge_xui_virtual_view_base pBase, int iSlot)
{
	if ( (pBase == NULL) || (iSlot < 0) || (iSlot >= pBase->iSlotCount) ) {
		return NULL;
	}
	return pBase->arrSlotWidget[iSlot];
}

xge_xui_widget xgeXuiVirtualListGetSlotWidget(xge_xui_virtual_list pList, int iSlot)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetSlotWidget(&pList->tBase, iSlot) : NULL;
}

void xgeXuiVirtualViewBaseSetSelect(xge_xui_virtual_view_base pBase, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pBase == NULL ) {
		return;
	}
	pBase->procSelect = procSelect;
	pBase->pSelectUser = pUser;
}

void xgeXuiVirtualListSetSelect(xge_xui_virtual_list pList, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetSelect(&pList->tBase, procSelect, pUser);
	}
}

void xgeXuiVirtualViewBaseSetSelected(xge_xui_virtual_view_base pBase, int iIndex)
{
	if ( pBase == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= __xgeXuiVirtualViewBaseSyncCount(pBase)) ) {
		iIndex = -1;
	}
	if ( pBase->iSelected == iIndex ) {
		if ( pBase->iFocus != iIndex ) {
			pBase->iFocus = iIndex;
			xgeXuiWidgetMarkPaint(pBase->pWidget);
		}
		return;
	}
	pBase->iSelected = iIndex;
	pBase->iFocus = iIndex;
	__xgeXuiVirtualViewBaseEnsureVisibleInternal(pBase, iIndex);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetSelected(xge_xui_virtual_list pList, int iIndex)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetSelected(&pList->tBase, iIndex);
	}
}

int xgeXuiVirtualViewBaseGetSelected(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iSelected : -1;
}

int xgeXuiVirtualListGetSelected(xge_xui_virtual_list pList)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseGetSelected(&pList->tBase) : -1;
}

void xgeXuiVirtualViewBaseSetHover(xge_xui_virtual_view_base pBase, int iIndex)
{
	__xgeXuiVirtualViewBaseSetHoverInternal(pBase, iIndex);
}

int xgeXuiVirtualViewBaseGetHover(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iHover : -1;
}

void xgeXuiVirtualViewBaseSetFocusIndex(xge_xui_virtual_view_base pBase, int iIndex)
{
	__xgeXuiVirtualViewBaseSetFocusIndexInternal(pBase, iIndex);
}

int xgeXuiVirtualViewBaseGetFocusIndex(xge_xui_virtual_view_base pBase)
{
	return (pBase != NULL) ? pBase->iFocus : -1;
}

void xgeXuiVirtualViewBaseSetColors(xge_xui_virtual_view_base pBase, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pBase == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pBase->pWidget, iBackground);
	pBase->iBarColor = iBar;
	pBase->iThumbColor = iThumb;
	pBase->tScroll.iBarColor = iBar;
	pBase->tScroll.iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiVirtualListSetColors(xge_xui_virtual_list pList, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseSetColors(&pList->tBase, iBackground, iBar, iThumb);
	}
}

int xgeXuiVirtualViewBaseEvent(xge_xui_virtual_view_base pBase, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iIndex;

	if ( (pBase == NULL) || (pBase->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pBase->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pBase->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pBase->pWidget->tContentRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiVirtualViewBaseSetScroll(pBase, pBase->fScrollY - pEvent->fDY * __xgeXuiVirtualViewBaseItemHeightAt(pBase, pBase->iSelected >= 0 ? pBase->iSelected : 0));
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pBase->pContext, pBase->pWidget);
			if ( __xgeXuiVirtualViewBaseBar(pBase, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pBase->bDraggingThumb = 1;
					pBase->fDragY = pEvent->fY;
					pBase->fDragScrollY = pBase->fScrollY;
					xgeXuiSetPointerCapture(pBase->pContext, pEvent->iPointerId, pBase->pWidget);
				} else {
					xgeXuiVirtualViewBaseSetScroll(pBase, pBase->fScrollY + ((pEvent->fY < tThumb.fY) ? -pBase->pWidget->tContentRect.fH : pBase->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiVirtualViewBaseIndexAt(pBase, pEvent->fY);
			if ( iIndex >= 0 ) {
				__xgeXuiVirtualViewBaseSetFocusIndexInternal(pBase, iIndex);
				xgeXuiVirtualViewBaseSetSelected(pBase, iIndex);
				__xgeXuiVirtualViewBaseNotifySelect(pBase);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pBase->bDraggingThumb == 0 ) {
				__xgeXuiVirtualViewBaseSetHoverInternal(pBase, iInside ? __xgeXuiVirtualViewBaseIndexAt(pBase, pEvent->fY) : -1);
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pBase->pContext, pEvent->iPointerId) != pBase->pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiVirtualViewBaseSetScrollFromThumbDrag(pBase, pEvent->fY);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pBase->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pBase->pContext, pEvent->iPointerId) != pBase->pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pBase->bDraggingThumb = 0;
			if ( pBase->pContext != NULL && xgeXuiGetPointerCapture(pBase->pContext, pEvent->iPointerId) == pBase->pWidget ) {
				xgeXuiSetPointerCapture(pBase->pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pBase->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pBase->bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiVirtualViewBaseSetHoverInternal(pBase, -1);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pBase->pContext == NULL || pBase->pContext->pFocus != pBase->pWidget || __xgeXuiVirtualViewBaseSyncCount(pBase) <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iIndex = pBase->iSelected;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iIndex = (iIndex < 0) ? 0 : iIndex + 1;
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iIndex = (iIndex < 0) ? (pBase->iItemCount - 1) : iIndex - 1;
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iIndex = (iIndex < 0) ? 0 : iIndex + ((int)(pBase->pWidget->tContentRect.fH / pBase->fItemHeight));
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iIndex = (iIndex < 0) ? 0 : iIndex - ((int)(pBase->pWidget->tContentRect.fH / pBase->fItemHeight));
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iIndex = 0;
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iIndex = pBase->iItemCount - 1;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiVirtualViewBaseNotifySelect(pBase);
				return (pBase->iSelected >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iIndex < 0 ) {
				iIndex = 0;
			}
			if ( iIndex >= pBase->iItemCount ) {
				iIndex = pBase->iItemCount - 1;
			}
			xgeXuiVirtualViewBaseSetSelected(pBase, iIndex);
			__xgeXuiVirtualViewBaseNotifySelect(pBase);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiVirtualListEvent(xge_xui_virtual_list pList, const xge_event_t* pEvent)
{
	return (pList != NULL) ? xgeXuiVirtualViewBaseEvent(&pList->tBase, pEvent) : XGE_XUI_EVENT_CONTINUE;
}

int xgeXuiVirtualViewBaseEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiVirtualViewBaseEvent((xge_xui_virtual_view_base)pUser, pEvent);
}

int xgeXuiVirtualListEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiVirtualListEvent((xge_xui_virtual_list)pUser, pEvent);
}

void xgeXuiVirtualViewBaseLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_view_base pBase;
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
	pBase = (xge_xui_virtual_view_base)pUser;
	if ( (pBase == NULL) || (pBase->pWidget == NULL) || (pBase->fItemHeight <= 0.0f) ) {
		return;
	}
	iCount = __xgeXuiVirtualViewBaseSyncCount(pBase);
	__xgeXuiVirtualViewBaseClamp(pBase);
	if ( pBase->procHeight == NULL ) {
		iFirst = (int)(pBase->fScrollY / pBase->fItemHeight);
	} else {
		iFirst = __xgeXuiVirtualViewBaseIndexAt(pBase, pBase->pWidget->tContentRect.fY);
	}
	if ( iFirst < 0 ) {
		iFirst = 0;
	}
	if ( iFirst > iCount ) {
		iFirst = iCount;
	}
	if ( pBase->procHeight == NULL ) {
		iVisible = (int)(pBase->pWidget->tContentRect.fH / pBase->fItemHeight) + 2;
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
		fY = __xgeXuiVirtualViewBaseItemTop(pBase, iFirst);
		fBottom = pBase->fScrollY + pBase->pWidget->tContentRect.fH;
		while ( (iFirst + iVisible < iCount) && (iVisible < XGE_XUI_VIRTUAL_LIST_SLOT_CAPACITY) && (fY < fBottom + pBase->fItemHeight * 2.0f) ) {
			fY += __xgeXuiVirtualViewBaseItemHeightAt(pBase, iFirst + iVisible);
			iVisible++;
		}
	}
	pBase->iVisibleStart = iFirst;
	pBase->iVisibleCount = iVisible;
	for ( iSlot = 0; iSlot < iVisible; iSlot++ ) {
		iIndex = iFirst + iSlot;
		pSlot = __xgeXuiVirtualViewBaseEnsureSlot(pBase, iSlot);
		if ( pSlot == NULL ) {
			continue;
		}
		tRect.fX = pBase->pWidget->tContentRect.fX;
		tRect.fY = pBase->pWidget->tContentRect.fY + __xgeXuiVirtualViewBaseItemTop(pBase, iIndex) - pBase->fScrollY;
		tRect.fW = pBase->pWidget->tContentRect.fW;
		tRect.fH = __xgeXuiVirtualViewBaseItemHeightAt(pBase, iIndex);
		xgeXuiWidgetSetRect(pSlot, tRect);
		xgeXuiWidgetSetVisible(pSlot, 1);
		if ( pBase->arrSlotIndex[iSlot] != iIndex ) {
			pBase->arrSlotIndex[iSlot] = iIndex;
			if ( pBase->procBind != NULL ) {
				pBase->procBind(pSlot, iIndex, pBase->pUser);
			}
		}
	}
	for ( iSlot = iVisible; iSlot < pBase->iSlotCount; iSlot++ ) {
		if ( pBase->arrSlotWidget[iSlot] != NULL ) {
			xgeXuiWidgetSetVisible(pBase->arrSlotWidget[iSlot], 0);
		}
		pBase->arrSlotIndex[iSlot] = -1;
	}
}

void xgeXuiVirtualListLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_list pList;

	pList = (xge_xui_virtual_list)pUser;
	if ( pList != NULL ) {
		xgeXuiVirtualViewBaseLayoutProc(pWidget, &pList->tBase);
	}
}

void xgeXuiVirtualViewBasePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_view_base pBase;
	xge_rect_t tBar;
	xge_rect_t tThumb;

	pBase = (xge_xui_virtual_view_base)pUser;
	if ( (pWidget == NULL) || (pBase == NULL) ) {
		return;
	}
	if ( __xgeXuiVirtualViewBaseBar(pBase, &tBar, &tThumb) != 0 ) {
		if ( pBase->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pBase->iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, XGE_COLOR_RGBA(184, 223, 245, 255));
			__xgeXuiHostDrawRect(tThumb, pBase->iThumbColor);
		}
	}
}

void xgeXuiVirtualListPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_virtual_list pList;

	pList = (xge_xui_virtual_list)pUser;
	if ( pList != NULL ) {
		xgeXuiVirtualViewBasePaintProc(pWidget, &pList->tBase);
	}
}
