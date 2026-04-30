static const uint16_t arrXgeXuiTreeChevronRight8[8] = {
	0x0000, 0x0020, 0x0030, 0x0038, 0x0038, 0x0030, 0x0020, 0x0000
};

static const uint16_t arrXgeXuiTreeChevronDown8[8] = {
	0x0000, 0x0000, 0x0070, 0x0038, 0x001c, 0x0008, 0x0000, 0x0000
};

static uint32_t __xgeXuiTreeViewHoverColor(uint32_t iRow)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static int __xgeXuiTreeViewFindNode(xge_xui_tree_view pTree, int iId)
{
	int i;

	if ( pTree == NULL ) {
		return -1;
	}
	for ( i = 0; i < pTree->iNodeCount; i++ ) {
		if ( pTree->arrNodes[i].iId == iId ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiTreeViewVisibleIndexOfId(xge_xui_tree_view pTree, int iId)
{
	int i;

	if ( pTree == NULL ) {
		return -1;
	}
	for ( i = 0; i < pTree->iVisibleCount; i++ ) {
		if ( pTree->arrNodes[pTree->arrVisible[i]].iId == iId ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiTreeViewAncestorExpanded(xge_xui_tree_view pTree, int iNode)
{
	int iParent;

	if ( (pTree == NULL) || (iNode < 0) || (iNode >= pTree->iNodeCount) ) {
		return 0;
	}
	iParent = __xgeXuiTreeViewFindNode(pTree, pTree->arrNodes[iNode].iParent);
	while ( iParent >= 0 ) {
		if ( pTree->arrNodes[iParent].bExpanded == 0 ) {
			return 0;
		}
		iParent = __xgeXuiTreeViewFindNode(pTree, pTree->arrNodes[iParent].iParent);
	}
	return 1;
}

static float __xgeXuiTreeViewMaxScroll(xge_xui_tree_view pTree)
{
	float fContent;
	float fView;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (pTree->fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	fContent = (float)pTree->iVisibleCount * pTree->fItemHeight;
	fView = pTree->pWidget->tContentRect.fH;
	return (fContent > fView) ? (fContent - fView) : 0.0f;
}

static void __xgeXuiTreeViewClamp(xge_xui_tree_view pTree)
{
	float fMax;

	if ( pTree == NULL ) {
		return;
	}
	fMax = __xgeXuiTreeViewMaxScroll(pTree);
	if ( pTree->fScrollY < 0.0f ) {
		pTree->fScrollY = 0.0f;
	}
	if ( pTree->fScrollY > fMax ) {
		pTree->fScrollY = fMax;
	}
}

static void __xgeXuiTreeViewAppendVisible(xge_xui_tree_view pTree, int iNode);
static void __xgeXuiTreeViewUpdateVisibleWindow(xge_xui_tree_view pTree);

static void __xgeXuiTreeViewRebuildVisible(xge_xui_tree_view pTree)
{
	int i;

	if ( pTree == NULL ) {
		return;
	}
	pTree->iVisibleCount = 0;
	for ( i = 0; i < pTree->iNodeCount && pTree->iVisibleCount < XGE_XUI_TREE_VIEW_VISIBLE_CAPACITY; i++ ) {
		if ( pTree->arrNodes[i].iParent < 0 ) {
			__xgeXuiTreeViewAppendVisible(pTree, i);
		}
	}
	if ( __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId) < 0 ) {
		pTree->iSelectedId = -1;
	}
	__xgeXuiTreeViewClamp(pTree);
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

static void __xgeXuiTreeViewAppendVisible(xge_xui_tree_view pTree, int iNode)
{
	int i;
	int iId;

	if ( (pTree == NULL) || (iNode < 0) || (iNode >= pTree->iNodeCount) || (pTree->iVisibleCount >= XGE_XUI_TREE_VIEW_VISIBLE_CAPACITY) ) {
		return;
	}
	pTree->arrVisible[pTree->iVisibleCount++] = iNode;
	if ( pTree->arrNodes[iNode].bExpanded == 0 ) {
		return;
	}
	iId = pTree->arrNodes[iNode].iId;
	for ( i = 0; i < pTree->iNodeCount; i++ ) {
		if ( pTree->arrNodes[i].iParent == iId ) {
			__xgeXuiTreeViewAppendVisible(pTree, i);
		}
	}
}

static int __xgeXuiTreeViewIndexAt(xge_xui_tree_view pTree, float fY)
{
	int iIndex;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (pTree->fItemHeight <= 0.0f) ) {
		return -1;
	}
	iIndex = (int)((fY - pTree->pWidget->tContentRect.fY + pTree->fScrollY) / pTree->fItemHeight);
	if ( (iIndex < 0) || (iIndex >= pTree->iVisibleCount) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiTreeViewVisibleWindow(xge_xui_tree_view pTree, int* pFirst, int* pCount)
{
	int iFirst;
	int iLast;
	int iCount;

	iFirst = 0;
	iCount = 0;
	if ( (pTree != NULL) && (pTree->pWidget != NULL) && (pTree->fItemHeight > 0.0f) && (pTree->iVisibleCount > 0) ) {
		iFirst = (int)(pTree->fScrollY / pTree->fItemHeight);
		if ( iFirst < 0 ) {
			iFirst = 0;
		}
		if ( iFirst > pTree->iVisibleCount ) {
			iFirst = pTree->iVisibleCount;
		}
		iLast = iFirst + (int)(pTree->pWidget->tContentRect.fH / pTree->fItemHeight) + 2;
		if ( iLast > pTree->iVisibleCount ) {
			iLast = pTree->iVisibleCount;
		}
		iCount = iLast - iFirst;
		if ( iCount < 0 ) {
			iCount = 0;
		}
	}
	if ( pFirst != NULL ) {
		*pFirst = iFirst;
	}
	if ( pCount != NULL ) {
		*pCount = iCount;
	}
}

static void __xgeXuiTreeViewUpdateVisibleWindow(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return;
	}
	__xgeXuiTreeViewVisibleWindow(pTree, &pTree->iFirstVisible, &pTree->iPaintVisibleCount);
}

static void __xgeXuiTreeViewNotifySelect(xge_xui_tree_view pTree)
{
	if ( (pTree != NULL) && (pTree->procSelect != NULL) && (pTree->iSelectedId >= 0) ) {
		pTree->procSelect(pTree->pWidget, pTree->iSelectedId, pTree->pUser);
	}
}

static void __xgeXuiTreeViewEnsureVisible(xge_xui_tree_view pTree, int iVisible)
{
	float fTop;
	float fBottom;
	float fViewTop;
	float fViewBottom;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return;
	}
	fTop = (float)iVisible * pTree->fItemHeight;
	fBottom = fTop + pTree->fItemHeight;
	fViewTop = pTree->fScrollY;
	fViewBottom = fViewTop + pTree->pWidget->tContentRect.fH;
	if ( fTop < fViewTop ) {
		xgeXuiTreeViewSetScroll(pTree, fTop);
	} else if ( fBottom > fViewBottom ) {
		xgeXuiTreeViewSetScroll(pTree, fBottom - pTree->pWidget->tContentRect.fH);
	}
}

static void __xgeXuiTreeViewSelectVisible(xge_xui_tree_view pTree, int iVisible, int bNotify)
{
	int iOld;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return;
	}
	iOld = pTree->iSelectedId;
	pTree->iSelectedId = pTree->arrNodes[pTree->arrVisible[iVisible]].iId;
	__xgeXuiTreeViewEnsureVisible(pTree, iVisible);
	xgeXuiWidgetMarkPaint(pTree->pWidget);
	if ( bNotify && (iOld != pTree->iSelectedId) ) {
		pTree->iSelectCount++;
		__xgeXuiTreeViewNotifySelect(pTree);
	}
}

static float __xgeXuiTreeViewThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiTreeViewBar(xge_xui_tree_view pTree, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fMaxScroll;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) ) {
		return 0;
	}
	fContentH = (float)pTree->iVisibleCount * pTree->fItemHeight;
	if ( (fContentH <= pTree->pWidget->tContentRect.fH) || (pTree->pWidget->tContentRect.fH <= 0.0f) ) {
		return 0;
	}
	tBar.fX = pTree->pWidget->tContentRect.fX + pTree->pWidget->tContentRect.fW - 4.0f;
	tBar.fY = pTree->pWidget->tContentRect.fY;
	tBar.fW = 4.0f;
	tBar.fH = pTree->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fH = __xgeXuiTreeViewThumbLen(tBar.fH, pTree->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiTreeViewMaxScroll(pTree);
	if ( fMaxScroll > 0.0f && tBar.fH > tThumb.fH ) {
		tThumb.fY += (tBar.fH - tThumb.fH) * (pTree->fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static void __xgeXuiTreeViewSetScrollFromThumbDrag(xge_xui_tree_view pTree, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pTree == NULL) || (__xgeXuiTreeViewBar(pTree, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiTreeViewMaxScroll(pTree);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiTreeViewSetScroll(pTree, pTree->fDragScrollY + ((fY - pTree->fDragY) / fTravel) * fMaxScroll);
}

static int __xgeXuiTreeViewHitExpander(xge_xui_tree_view pTree, int iVisible, float fX)
{
	xge_xui_tree_view_node_t* pNode;
	float fLeft;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return 0;
	}
	pNode = &pTree->arrNodes[pTree->arrVisible[iVisible]];
	if ( pNode->bHasChildren == 0 ) {
		return 0;
	}
	fLeft = pTree->pWidget->tContentRect.fX + 4.0f + (float)pNode->iDepth * pTree->fIndent;
	return (fX >= fLeft && fX <= fLeft + 14.0f);
}

int xgeXuiTreeViewInit(xge_xui_tree_view pTree, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pTree == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTree, 0, sizeof(*pTree));
	pTree->pContext = pContext;
	pTree->pWidget = pWidget;
	pTree->iSelectedId = -1;
	pTree->iHoverVisible = -1;
	pTree->iActiveVisible = -1;
	pTree->fItemHeight = 22.0f;
	pTree->fIndent = 16.0f;
	pTree->iBackgroundColor = XGE_COLOR_RGBA(235, 244, 252, 255);
	pTree->iRowColor = XGE_COLOR_RGBA(245, 250, 255, 255);
	pTree->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	pTree->iSelectedColor = XGE_COLOR_RGBA(187, 220, 248, 255);
	pTree->iTextColor = XGE_COLOR_RGBA(34, 48, 64, 255);
	pTree->iDisabledTextColor = XGE_COLOR_RGBA(124, 138, 150, 220);
	pTree->iExpanderColor = XGE_COLOR_RGBA(55, 118, 176, 255);
	pTree->iBarColor = XGE_COLOR_RGBA(185, 208, 226, 170);
	pTree->iThumbColor = XGE_COLOR_RGBA(91, 151, 205, 220);
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiTreeViewEventProc;
	pWidget->procPaint = xgeXuiTreeViewPaintProc;
	pWidget->pUser = pTree;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiTreeViewUnit(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return;
	}
	if ( pTree->pContext != NULL && pTree->pContext->pCapture == pTree->pWidget ) {
		xgeXuiSetCapture(pTree->pContext, NULL);
	}
	if ( pTree->pWidget != NULL && pTree->pWidget->pUser == pTree ) {
		pTree->pWidget->pUser = NULL;
		pTree->pWidget->procEvent = NULL;
		pTree->pWidget->procPaint = NULL;
	}
	memset(pTree, 0, sizeof(*pTree));
}

void xgeXuiTreeViewClear(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->iNodeCount = 0;
	pTree->iVisibleCount = 0;
	pTree->iFirstVisible = 0;
	pTree->iPaintVisibleCount = 0;
	pTree->iSelectedId = -1;
	pTree->iHoverVisible = -1;
	pTree->iActiveVisible = -1;
	pTree->fScrollY = 0.0f;
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

int xgeXuiTreeViewAddNode(xge_xui_tree_view pTree, int iId, int iParentId, const char* sText)
{
	xge_xui_tree_view_node_t* pNode;
	int iParent;

	if ( (pTree == NULL) || (iId < 0) || (__xgeXuiTreeViewFindNode(pTree, iId) >= 0) ) {
		return -1;
	}
	if ( pTree->iNodeCount >= XGE_XUI_TREE_VIEW_NODE_CAPACITY ) {
		return -1;
	}
	iParent = __xgeXuiTreeViewFindNode(pTree, iParentId);
	if ( (iParentId >= 0) && (iParent < 0) ) {
		return -1;
	}
	pNode = &pTree->arrNodes[pTree->iNodeCount];
	memset(pNode, 0, sizeof(*pNode));
	pNode->iId = iId;
	pNode->iParent = iParentId;
	pNode->iDepth = (iParent >= 0) ? (pTree->arrNodes[iParent].iDepth + 1) : 0;
	pNode->sText = sText;
	pNode->bIconReserved = 1;
	pNode->bCheckReserved = 0;
	if ( iParent >= 0 ) {
		pTree->arrNodes[iParent].bHasChildren = 1;
	}
	pTree->iNodeCount++;
	__xgeXuiTreeViewRebuildVisible(pTree);
	return pTree->iNodeCount - 1;
}

void xgeXuiTreeViewSetAdapter(xge_xui_tree_view pTree, xge_xui_tree_view_count_proc procCount, xge_xui_tree_view_node_proc procNode, void* pUser)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->procCount = procCount;
	pTree->procNode = procNode;
	pTree->pAdapterUser = pUser;
	(void)xgeXuiTreeViewRefreshAdapter(pTree);
}

int xgeXuiTreeViewRefreshAdapter(xge_xui_tree_view pTree)
{
	xge_xui_tree_view_node_t tNode;
	int i;
	int iCount;
	int iAdded;
	int iSelectedId;
	float fScrollY;

	if ( pTree == NULL ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( (pTree->procCount == NULL) || (pTree->procNode == NULL) ) {
		return XGE_OK;
	}
	iCount = pTree->procCount(pTree->pWidget, pTree->pAdapterUser);
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_TREE_VIEW_NODE_CAPACITY ) {
		iCount = XGE_XUI_TREE_VIEW_NODE_CAPACITY;
	}
	iSelectedId = pTree->iSelectedId;
	fScrollY = pTree->fScrollY;
	pTree->iNodeCount = 0;
	pTree->iVisibleCount = 0;
	pTree->iSelectedId = iSelectedId;
	pTree->iHoverVisible = -1;
	pTree->iActiveVisible = -1;
	pTree->fScrollY = fScrollY;
	for ( i = 0; i < iCount; i++ ) {
		memset(&tNode, 0, sizeof(tNode));
		tNode.iId = -1;
		tNode.iParent = -1;
		tNode.sText = "";
		tNode.bIconReserved = 1;
		if ( pTree->procNode(pTree->pWidget, i, &tNode, pTree->pAdapterUser) != XGE_OK ) {
			continue;
		}
		iAdded = xgeXuiTreeViewAddNode(pTree, tNode.iId, tNode.iParent, tNode.sText);
		if ( iAdded < 0 ) {
			continue;
		}
		pTree->arrNodes[iAdded].bExpanded = tNode.bExpanded;
		pTree->arrNodes[iAdded].bIconReserved = tNode.bIconReserved;
		pTree->arrNodes[iAdded].bCheckReserved = tNode.bCheckReserved;
	}
	pTree->iSelectedId = iSelectedId;
	__xgeXuiTreeViewRebuildVisible(pTree);
	return XGE_OK;
}

void xgeXuiTreeViewSetNodeExpanded(xge_xui_tree_view pTree, int iId, int bExpanded)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	if ( iNode < 0 ) {
		return;
	}
	pTree->arrNodes[iNode].bExpanded = (bExpanded != 0);
	__xgeXuiTreeViewRebuildVisible(pTree);
}

int xgeXuiTreeViewGetNodeExpanded(xge_xui_tree_view pTree, int iId)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	return (iNode >= 0) ? pTree->arrNodes[iNode].bExpanded : 0;
}

void xgeXuiTreeViewSetSelected(xge_xui_tree_view pTree, int iId)
{
	int iVisible;

	if ( pTree == NULL ) {
		return;
	}
	iVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, iId);
	if ( iVisible < 0 ) {
		iId = -1;
	}
	if ( pTree->iSelectedId != iId ) {
		pTree->iSelectedId = iId;
		if ( iId >= 0 ) {
			__xgeXuiTreeViewEnsureVisible(pTree, iVisible);
		}
		xgeXuiWidgetMarkPaint(pTree->pWidget);
	}
}

int xgeXuiTreeViewGetSelected(xge_xui_tree_view pTree)
{
	return (pTree != NULL) ? pTree->iSelectedId : -1;
}

int xgeXuiTreeViewGetVisibleCount(xge_xui_tree_view pTree)
{
	return (pTree != NULL) ? pTree->iVisibleCount : 0;
}

int xgeXuiTreeViewGetVisibleNodeId(xge_xui_tree_view pTree, int iVisible)
{
	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return -1;
	}
	return pTree->arrNodes[pTree->arrVisible[iVisible]].iId;
}

int xgeXuiTreeViewGetFirstVisible(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return 0;
	}
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	return pTree->iFirstVisible;
}

int xgeXuiTreeViewGetPaintVisibleCount(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return 0;
	}
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	return pTree->iPaintVisibleCount;
}

void xgeXuiTreeViewSetFont(xge_xui_tree_view pTree, xge_font pFont)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->pFont = pFont;
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

void xgeXuiTreeViewSetMetrics(xge_xui_tree_view pTree, float fItemHeight, float fIndent)
{
	if ( pTree == NULL ) {
		return;
	}
	if ( fItemHeight < 1.0f ) {
		fItemHeight = 1.0f;
	}
	if ( fIndent < 0.0f ) {
		fIndent = 0.0f;
	}
	pTree->fItemHeight = fItemHeight;
	pTree->fIndent = fIndent;
	__xgeXuiTreeViewClamp(pTree);
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

void xgeXuiTreeViewSetScroll(xge_xui_tree_view pTree, float fScrollY)
{
	float fOld;

	if ( pTree == NULL ) {
		return;
	}
	fOld = pTree->fScrollY;
	pTree->fScrollY = fScrollY;
	__xgeXuiTreeViewClamp(pTree);
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	if ( fOld != pTree->fScrollY ) {
		xgeXuiWidgetMarkPaint(pTree->pWidget);
	}
}

float xgeXuiTreeViewGetScroll(xge_xui_tree_view pTree)
{
	return (pTree != NULL) ? pTree->fScrollY : 0.0f;
}

void xgeXuiTreeViewSetSelect(xge_xui_tree_view pTree, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->procSelect = procSelect;
	pTree->pUser = pUser;
}

void xgeXuiTreeViewSetColors(xge_xui_tree_view pTree, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->iBackgroundColor = iBackground;
	pTree->iRowColor = iRow;
	pTree->iHoverColor = __xgeXuiTreeViewHoverColor(iRow);
	pTree->iSelectedColor = iSelected;
	pTree->iTextColor = iText;
	pTree->iBarColor = iBar;
	pTree->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

void xgeXuiTreeViewSetDisabledTextColor(xge_xui_tree_view pTree, uint32_t iColor)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->iDisabledTextColor = iColor;
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

int xgeXuiTreeViewEvent(xge_xui_tree_view pTree, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iVisible;
	int iSelectedVisible;
	int iNode;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pTree->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pTree->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pTree->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiTreeViewSetScroll(pTree, pTree->fScrollY - pEvent->fDY * pTree->fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pTree->bDraggingThumb != 0 ) {
				__xgeXuiTreeViewSetScrollFromThumbDrag(pTree, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = iInside ? __xgeXuiTreeViewIndexAt(pTree, pEvent->fY) : -1;
			if ( pTree->iHoverVisible != iVisible ) {
				pTree->iHoverVisible = iVisible;
				pTree->iState = (iVisible >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
				xgeXuiWidgetMarkPaint(pTree->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTree->pContext, pTree->pWidget);
			if ( __xgeXuiTreeViewBar(pTree, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pTree->bDraggingThumb = 1;
					pTree->fDragY = pEvent->fY;
					pTree->fDragScrollY = pTree->fScrollY;
					xgeXuiSetCapture(pTree->pContext, pTree->pWidget);
				} else {
					xgeXuiTreeViewSetScroll(pTree, pTree->fScrollY + ((pEvent->fY < tThumb.fY) ? -pTree->pWidget->tContentRect.fH : pTree->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = __xgeXuiTreeViewIndexAt(pTree, pEvent->fY);
			if ( iVisible < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pTree->iActiveVisible = iVisible;
			pTree->bActiveExpander = __xgeXuiTreeViewHitExpander(pTree, iVisible, pEvent->fX);
			pTree->iState = XGE_XUI_STATE_ACTIVE;
			xgeXuiSetCapture(pTree->pContext, pTree->pWidget);
			xgeXuiWidgetMarkPaint(pTree->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( pTree->bDraggingThumb != 0 ) {
				__xgeXuiTreeViewSetScrollFromThumbDrag(pTree, pEvent->fY);
				pTree->bDraggingThumb = 0;
				xgeXuiSetCapture(pTree->pContext, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTree->iActiveVisible < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iVisible = __xgeXuiTreeViewIndexAt(pTree, pEvent->fY);
			if ( iVisible == pTree->iActiveVisible ) {
				iNode = pTree->arrVisible[iVisible];
				if ( pTree->bActiveExpander != 0 && pTree->arrNodes[iNode].bHasChildren != 0 ) {
					pTree->arrNodes[iNode].bExpanded = !pTree->arrNodes[iNode].bExpanded;
					__xgeXuiTreeViewRebuildVisible(pTree);
				} else {
					__xgeXuiTreeViewSelectVisible(pTree, iVisible, 1);
				}
			}
			pTree->iActiveVisible = -1;
			pTree->bActiveExpander = 0;
			pTree->iState = (pTree->iHoverVisible >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			xgeXuiSetCapture(pTree->pContext, NULL);
			xgeXuiWidgetMarkPaint(pTree->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
			pTree->bDraggingThumb = 0;
			pTree->iActiveVisible = -1;
			pTree->bActiveExpander = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( pTree->iHoverVisible != -1 ) {
				pTree->iHoverVisible = -1;
				pTree->iState = XGE_XUI_STATE_NORMAL;
				xgeXuiWidgetMarkPaint(pTree->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pTree->pContext == NULL || pTree->pContext->pFocus != pTree->pWidget || pTree->iVisibleCount <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iSelectedVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId);
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iVisible = (iSelectedVisible < 0) ? 0 : iSelectedVisible + 1;
				if ( iVisible >= pTree->iVisibleCount ) {
					iVisible = pTree->iVisibleCount - 1;
				}
				__xgeXuiTreeViewSelectVisible(pTree, iVisible, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iVisible = (iSelectedVisible < 0) ? (pTree->iVisibleCount - 1) : iSelectedVisible - 1;
				if ( iVisible < 0 ) {
					iVisible = 0;
				}
				__xgeXuiTreeViewSelectVisible(pTree, iVisible, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_RIGHT) && (iSelectedVisible >= 0) ) {
				iNode = pTree->arrVisible[iSelectedVisible];
				if ( pTree->arrNodes[iNode].bHasChildren != 0 && pTree->arrNodes[iNode].bExpanded == 0 ) {
					pTree->arrNodes[iNode].bExpanded = 1;
					__xgeXuiTreeViewRebuildVisible(pTree);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_LEFT) && (iSelectedVisible >= 0) ) {
				iNode = pTree->arrVisible[iSelectedVisible];
				if ( pTree->arrNodes[iNode].bHasChildren != 0 && pTree->arrNodes[iNode].bExpanded != 0 ) {
					pTree->arrNodes[iNode].bExpanded = 0;
					__xgeXuiTreeViewRebuildVisible(pTree);
				} else if ( pTree->arrNodes[iNode].iParent >= 0 ) {
					xgeXuiTreeViewSetSelected(pTree, pTree->arrNodes[iNode].iParent);
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiTreeViewNotifySelect(pTree);
				return (pTree->iSelectedId >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			}
			return XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTreeViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTreeViewEvent((xge_xui_tree_view)pUser, pEvent);
}

void xgeXuiTreeViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tree_view pTree;
	xge_xui_tree_view_node_t* pNode;
	xge_rect_t tRow;
	xge_rect_t tText;
	xge_rect_t tIcon;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iFirst;
	int iLast;
	int iCount;
	int i;
	int iVisible;
	uint32_t iRowColor;

	pTree = (xge_xui_tree_view)pUser;
	if ( (pWidget == NULL) || (pTree == NULL) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pTree->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(pWidget->tRect, pTree->iBackgroundColor);
	}
	if ( (pTree->fItemHeight <= 0.0f) || (pTree->iVisibleCount <= 0) ) {
		return;
	}
	__xgeXuiTreeViewVisibleWindow(pTree, &iFirst, &iCount);
	pTree->iFirstVisible = iFirst;
	pTree->iPaintVisibleCount = iCount;
	iLast = iFirst + iCount;
	for ( iVisible = iFirst; iVisible < iLast; iVisible++ ) {
		i = pTree->arrVisible[iVisible];
		pNode = &pTree->arrNodes[i];
		tRow.fX = pWidget->tContentRect.fX;
		tRow.fY = pWidget->tContentRect.fY + (float)iVisible * pTree->fItemHeight - pTree->fScrollY;
		tRow.fW = pWidget->tContentRect.fW;
		tRow.fH = pTree->fItemHeight;
		pNode->tRect = tRow;
		iRowColor = pTree->iRowColor;
		if ( iVisible == pTree->iHoverVisible ) {
			iRowColor = pTree->iHoverColor;
		}
		if ( pNode->iId == pTree->iSelectedId ) {
			iRowColor = pTree->iSelectedColor;
		}
		__xgeXuiHostDrawRect(tRow, iRowColor);
		tIcon.fX = tRow.fX + 4.0f + (float)pNode->iDepth * pTree->fIndent;
		tIcon.fY = tRow.fY + (tRow.fH - 8.0f) * 0.5f;
		tIcon.fW = 8.0f;
		tIcon.fH = 8.0f;
		if ( pNode->bHasChildren != 0 ) {
			__xgeXuiHostDrawBitmapMask(tIcon, pNode->bExpanded ? arrXgeXuiTreeChevronDown8 : arrXgeXuiTreeChevronRight8, 8, 8, pTree->iExpanderColor);
		}
		tText = tRow;
		tText.fX = tIcon.fX + 14.0f + (pNode->bCheckReserved ? 14.0f : 0.0f) + (pNode->bIconReserved ? 14.0f : 0.0f);
		tText.fW = tRow.fX + tRow.fW - tText.fX - 6.0f;
		if ( (pTree->pFont != NULL) && (pNode->sText != NULL) && (tText.fW > 0.0f) ) {
			__xgeXuiHostDrawTextRect(pTree->pFont, pNode->sText, tText, pTree->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	if ( __xgeXuiTreeViewBar(pTree, &tBar, &tThumb) != 0 ) {
		__xgeXuiHostDrawRect(tBar, pTree->iBarColor);
		__xgeXuiHostDrawRect(tThumb, pTree->iThumbColor);
	}
}
