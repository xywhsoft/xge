enum {
	XGE_XUI_TREE_ACTIVE_ROW = 0,
	XGE_XUI_TREE_ACTIVE_EXPANDER = 1,
	XGE_XUI_TREE_ACTIVE_CHECK = 2
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

static int __xgeXuiTreeViewNodeEnabled(xge_xui_tree_view pTree, int iNode)
{
	if ( (pTree == NULL) || (iNode < 0) || (iNode >= pTree->iNodeCount) ) {
		return 0;
	}
	return pTree->arrNodes[iNode].bEnabled != 0;
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

static int __xgeXuiTreeViewNormalizeVisible(xge_xui_tree_view pTree, int iVisible)
{
	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return -1;
	}
	return iVisible;
}

static float __xgeXuiTreeViewContentHeight(xge_xui_tree_view pTree)
{
	if ( (pTree == NULL) || (pTree->iVisibleCount <= 0) || (pTree->fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	return (float)pTree->iVisibleCount * pTree->fItemHeight;
}

static void __xgeXuiTreeViewUpdateVisibleWindow(xge_xui_tree_view pTree)
{
	xge_rect_t tViewport;
	int iFirst;
	int iLast;
	int iCount;

	if ( pTree == NULL ) {
		return;
	}
	iFirst = 0;
	iCount = 0;
	if ( (pTree->fItemHeight > 0.0f) && (pTree->iVisibleCount > 0) ) {
		tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
		iFirst = (int)(pTree->tScroll.fScrollY / pTree->fItemHeight);
		if ( iFirst < 0 ) {
			iFirst = 0;
		}
		if ( iFirst > pTree->iVisibleCount ) {
			iFirst = pTree->iVisibleCount;
		}
		iLast = iFirst + (int)(tViewport.fH / pTree->fItemHeight) + 2;
		if ( iLast > pTree->iVisibleCount ) {
			iLast = pTree->iVisibleCount;
		}
		iCount = iLast - iFirst;
		if ( iCount < 0 ) {
			iCount = 0;
		}
	}
	pTree->iFirstVisible = iFirst;
	pTree->iPaintVisibleCount = iCount;
}

static void __xgeXuiTreeViewInvalidate(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return;
	}
	xgeXuiWidgetMarkPaint(xgeXuiScrollFrameGetViewportWidget(&pTree->tFrame));
	xgeXuiWidgetMarkPaint(pTree->pWidget);
}

static void __xgeXuiTreeViewEnsureVisible(xge_xui_tree_view pTree, int iVisible);

static void __xgeXuiTreeViewSyncFrame(xge_xui_tree_view pTree)
{
	if ( pTree == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetContentSize(&pTree->tFrame, 0.0f, __xgeXuiTreeViewContentHeight(pTree));
	xgeXuiScrollFrameSetWheelStep(&pTree->tFrame, ((pTree->fItemHeight > 0.0f) ? pTree->fItemHeight : 24.0f) * 3.0f);
	if ( (pTree->bEnsureSelectedPending != 0) && (pTree->iSelectedId >= 0) ) {
		__xgeXuiTreeViewEnsureVisible(pTree, __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId));
	}
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	__xgeXuiTreeViewInvalidate(pTree);
}

static void __xgeXuiTreeViewSetHoverVisible(xge_xui_tree_view pTree, int iVisible)
{
	if ( pTree == NULL ) {
		return;
	}
	iVisible = __xgeXuiTreeViewNormalizeVisible(pTree, iVisible);
	if ( pTree->iHoverVisible == iVisible ) {
		return;
	}
	pTree->iHoverVisible = iVisible;
	__xgeXuiTreeViewInvalidate(pTree);
}

static void __xgeXuiTreeViewSetFocusVisible(xge_xui_tree_view pTree, int iVisible)
{
	if ( pTree == NULL ) {
		return;
	}
	iVisible = __xgeXuiTreeViewNormalizeVisible(pTree, iVisible);
	if ( pTree->iFocusVisible == iVisible ) {
		return;
	}
	pTree->iFocusVisible = iVisible;
	__xgeXuiTreeViewInvalidate(pTree);
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

static void __xgeXuiTreeViewRebuildVisible(xge_xui_tree_view pTree, int iFallbackNodeId)
{
	int i;
	int iSelectedVisible;

	if ( pTree == NULL ) {
		return;
	}
	pTree->iVisibleCount = 0;
	pTree->iHoverVisible = -1;
	for ( i = 0; (i < pTree->iNodeCount) && (pTree->iVisibleCount < XGE_XUI_TREE_VIEW_VISIBLE_CAPACITY); i++ ) {
		if ( pTree->arrNodes[i].iParent < 0 ) {
			__xgeXuiTreeViewAppendVisible(pTree, i);
		}
	}
	iSelectedVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId);
	if ( (iSelectedVisible < 0) || (__xgeXuiTreeViewNodeEnabled(pTree, pTree->arrVisible[iSelectedVisible]) == 0) ) {
		if ( (iFallbackNodeId >= 0) && (__xgeXuiTreeViewVisibleIndexOfId(pTree, iFallbackNodeId) >= 0) && (__xgeXuiTreeViewNodeEnabled(pTree, __xgeXuiTreeViewFindNode(pTree, iFallbackNodeId)) != 0) ) {
			pTree->iSelectedId = iFallbackNodeId;
		} else {
			pTree->iSelectedId = -1;
		}
	}
	pTree->iFocusVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId);
	__xgeXuiTreeViewSyncFrame(pTree);
}

static int __xgeXuiTreeViewIndexAt(xge_xui_tree_view pTree, float fY)
{
	xge_rect_t tViewport;
	int iIndex;

	if ( (pTree == NULL) || (pTree->fItemHeight <= 0.0f) ) {
		return -1;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
	iIndex = (int)((fY - tViewport.fY + pTree->tScroll.fScrollY) / pTree->fItemHeight);
	if ( (iIndex < 0) || (iIndex >= pTree->iVisibleCount) ) {
		return -1;
	}
	return iIndex;
}

static int __xgeXuiTreeViewNextEnabledVisible(xge_xui_tree_view pTree, int iStart, int iStep)
{
	int i;

	if ( (pTree == NULL) || (pTree->iVisibleCount <= 0) ) {
		return -1;
	}
	if ( iStep == 0 ) {
		iStep = 1;
	}
	if ( iStart < 0 ) {
		iStart = (iStep > 0) ? 0 : (pTree->iVisibleCount - 1);
	}
	if ( iStart >= pTree->iVisibleCount ) {
		iStart = (iStep > 0) ? (pTree->iVisibleCount - 1) : 0;
	}
	for ( i = iStart; (i >= 0) && (i < pTree->iVisibleCount); i += iStep ) {
		if ( __xgeXuiTreeViewNodeEnabled(pTree, pTree->arrVisible[i]) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiTreeViewVisibleRows(xge_xui_tree_view pTree)
{
	xge_rect_t tViewport;
	int iRows;

	if ( (pTree == NULL) || (pTree->fItemHeight <= 0.0f) ) {
		return 1;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
	iRows = (int)(tViewport.fH / pTree->fItemHeight);
	return (iRows > 0) ? iRows : 1;
}

static xge_rect_t __xgeXuiTreeViewRowRect(xge_xui_tree_view pTree, int iVisible)
{
	xge_rect_t tViewport;
	xge_rect_t tRow;

	memset(&tRow, 0, sizeof(tRow));
	if ( (pTree == NULL) || (pTree->fItemHeight <= 0.0f) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return tRow;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
	tRow.fX = tViewport.fX;
	tRow.fY = tViewport.fY + (float)iVisible * pTree->fItemHeight - pTree->tScroll.fScrollY;
	tRow.fW = tViewport.fW;
	tRow.fH = pTree->fItemHeight;
	if ( tRow.fH > 1.0f ) {
		tRow.fH -= 1.0f;
	}
	return tRow;
}

static void __xgeXuiTreeViewNotifySelect(xge_xui_tree_view pTree)
{
	if ( (pTree != NULL) && (pTree->procSelect != NULL) && (pTree->iSelectedId >= 0) ) {
		pTree->iSelectCount++;
		pTree->procSelect(pTree->pWidget, pTree->iSelectedId, pTree->pSelectUser);
	}
}

static void __xgeXuiTreeViewEnsureVisible(xge_xui_tree_view pTree, int iVisible)
{
	xge_rect_t tViewport;
	float fTop;
	float fBottom;
	float fY;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) || (pTree->fItemHeight <= 0.0f) ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
	if ( tViewport.fH <= 0.0f ) {
		return;
	}
	pTree->bEnsureSelectedPending = 0;
	fTop = (float)iVisible * pTree->fItemHeight;
	fBottom = fTop + pTree->fItemHeight;
	fY = pTree->tScroll.fScrollY;
	if ( fTop < fY ) {
		fY = fTop;
	} else if ( fBottom > fY + tViewport.fH ) {
		fY = fBottom - tViewport.fH;
	}
	if ( xgeXuiScrollFrameSetOffset(&pTree->tFrame, 0.0f, fY) ) {
		__xgeXuiTreeViewInvalidate(pTree);
	}
}

static void __xgeXuiTreeViewSelectVisible(xge_xui_tree_view pTree, int iVisible, int bNotify)
{
	int iOld;
	int iNode;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return;
	}
	iNode = pTree->arrVisible[iVisible];
	if ( __xgeXuiTreeViewNodeEnabled(pTree, iNode) == 0 ) {
		return;
	}
	iOld = pTree->iSelectedId;
	pTree->iSelectedId = pTree->arrNodes[iNode].iId;
	__xgeXuiTreeViewSetFocusVisible(pTree, iVisible);
	pTree->bEnsureSelectedPending = 1;
	__xgeXuiTreeViewEnsureVisible(pTree, iVisible);
	__xgeXuiTreeViewInvalidate(pTree);
	if ( bNotify && (iOld != pTree->iSelectedId) ) {
		__xgeXuiTreeViewNotifySelect(pTree);
	}
}

static xge_rect_t __xgeXuiTreeViewExpanderRect(xge_xui_tree_view pTree, xge_xui_tree_view_node_t* pNode, xge_rect_t tRow)
{
	xge_rect_t tRect;

	if ( (pTree == NULL) || (pNode == NULL) ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	tRect.fX = tRow.fX + 6.0f + (float)pNode->iDepth * pTree->fIndent;
	tRect.fY = tRow.fY + (tRow.fH - 12.0f) * 0.5f;
	tRect.fW = 12.0f;
	tRect.fH = 12.0f;
	return tRect;
}

static xge_rect_t __xgeXuiTreeViewCheckRect(xge_xui_tree_view_node_t* pNode, xge_rect_t tExpander)
{
	xge_rect_t tRect;

	if ( (pNode == NULL) || (pNode->bCheckReserved == 0) ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	tRect = tExpander;
	tRect.fX += 17.0f;
	tRect.fY += 1.0f;
	tRect.fW = 10.0f;
	tRect.fH = 10.0f;
	return tRect;
}

static xge_rect_t __xgeXuiTreeViewExpanderHitRect(xge_xui_tree_view pTree, xge_xui_tree_view_node_t* pNode, xge_rect_t tRow)
{
	xge_rect_t tRect;
	float fSlot;

	if ( (pTree == NULL) || (pNode == NULL) ) {
		return (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	fSlot = pTree->fIndent + 4.0f;
	if ( fSlot < 22.0f ) {
		fSlot = 22.0f;
	}
	tRect.fX = tRow.fX + (float)pNode->iDepth * pTree->fIndent;
	tRect.fY = tRow.fY;
	tRect.fW = fSlot;
	tRect.fH = tRow.fH;
	return tRect;
}

static xge_rect_t __xgeXuiTreeViewCheckHitRect(xge_rect_t tCheck, xge_rect_t tRow)
{
	xge_rect_t tRect;

	tRect.fX = tCheck.fX - 3.0f;
	tRect.fY = tRow.fY;
	tRect.fW = tCheck.fW + 6.0f;
	tRect.fH = tRow.fH;
	return tRect;
}

static int __xgeXuiTreeViewHitExpander(xge_xui_tree_view pTree, int iVisible, float fX, float fY)
{
	xge_xui_tree_view_node_t* pNode;
	xge_rect_t tRow;
	xge_rect_t tHit;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return 0;
	}
	pNode = &pTree->arrNodes[pTree->arrVisible[iVisible]];
	if ( pNode->bHasChildren == 0 ) {
		return 0;
	}
	tRow = __xgeXuiTreeViewRowRect(pTree, iVisible);
	tHit = __xgeXuiTreeViewExpanderHitRect(pTree, pNode, tRow);
	return __xgeXuiRectContains(tHit, fX, fY);
}

static int __xgeXuiTreeViewHitCheck(xge_xui_tree_view pTree, int iVisible, float fX, float fY)
{
	xge_xui_tree_view_node_t* pNode;
	xge_rect_t tRow;
	xge_rect_t tExpander;
	xge_rect_t tCheck;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return 0;
	}
	pNode = &pTree->arrNodes[pTree->arrVisible[iVisible]];
	if ( pNode->bCheckReserved == 0 ) {
		return 0;
	}
	tRow = __xgeXuiTreeViewRowRect(pTree, iVisible);
	tExpander = __xgeXuiTreeViewExpanderRect(pTree, pNode, tRow);
	tCheck = __xgeXuiTreeViewCheckRect(pNode, tExpander);
	return __xgeXuiRectContains(__xgeXuiTreeViewCheckHitRect(tCheck, tRow), fX, fY);
}

static int __xgeXuiTreeViewForwardScrollBars(xge_xui_tree_view pTree, const xge_event_t* pEvent)
{
	xge_xui_widget pCapture;
	xge_xui_widget pHWidget;
	xge_xui_widget pVWidget;
	int iRet;

	if ( (pTree == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pHWidget = xgeXuiScrollFrameGetHScrollBarWidget(&pTree->tFrame);
	pVWidget = xgeXuiScrollFrameGetVScrollBarWidget(&pTree->tFrame);
	pCapture = (pTree->pContext != NULL) ? xgeXuiGetPointerCapture(pTree->pContext, pEvent->iPointerId) : NULL;
	if ( (pHWidget != NULL) && ((pCapture == pHWidget) || __xgeXuiRectContains(pHWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pTree->tFrame.tHScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiTreeViewInvalidate(pTree);
			return iRet;
		}
	}
	if ( (pVWidget != NULL) && ((pCapture == pVWidget) || __xgeXuiRectContains(pVWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pTree->tFrame.tVScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiTreeViewInvalidate(pTree);
			return iRet;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void __xgeXuiTreeViewFrameChanged(xge_xui_scroll_frame pFrame, float fX, float fY, void* pUser)
{
	xge_xui_tree_view pTree;

	(void)pFrame;
	(void)fX;
	(void)fY;
	pTree = (xge_xui_tree_view)pUser;
	__xgeXuiTreeViewUpdateVisibleWindow(pTree);
	__xgeXuiTreeViewInvalidate(pTree);
}

static int __xgeXuiTreeViewFloor(float fValue)
{
	int iValue;

	iValue = (int)fValue;
	return (fValue < (float)iValue) ? (iValue - 1) : iValue;
}

static int __xgeXuiTreeViewCeil(float fValue)
{
	int iValue;

	iValue = (int)fValue;
	return (fValue > (float)iValue) ? (iValue + 1) : iValue;
}

static float __xgeXuiTreeViewMin3(float fA, float fB, float fC)
{
	float fMin;

	fMin = (fA < fB) ? fA : fB;
	return (fMin < fC) ? fMin : fC;
}

static float __xgeXuiTreeViewMax3(float fA, float fB, float fC)
{
	float fMax;

	fMax = (fA > fB) ? fA : fB;
	return (fMax > fC) ? fMax : fC;
}

static uint32_t __xgeXuiTreeViewAlphaColor(uint32_t iColor, float fAlphaRate)
{
	int iAlpha;

	if ( fAlphaRate <= 0.0f ) {
		return XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	if ( fAlphaRate > 1.0f ) {
		fAlphaRate = 1.0f;
	}
	iAlpha = (int)((float)XGE_COLOR_GET_A(iColor) * fAlphaRate + 0.5f);
	if ( iAlpha > 255 ) {
		iAlpha = 255;
	}
	return XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), iAlpha);
}

static float __xgeXuiTreeViewTriangleEdge(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tP)
{
	return (tP.fX - tA.fX) * (tB.fY - tA.fY) - (tP.fY - tA.fY) * (tB.fX - tA.fX);
}

static int __xgeXuiTreeViewPointInTriangle(xge_vec2_t tP, xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC)
{
	float fD1;
	float fD2;
	float fD3;
	int bNegative;
	int bPositive;

	fD1 = __xgeXuiTreeViewTriangleEdge(tA, tB, tP);
	fD2 = __xgeXuiTreeViewTriangleEdge(tB, tC, tP);
	fD3 = __xgeXuiTreeViewTriangleEdge(tC, tA, tP);
	bNegative = (fD1 < 0.0f) || (fD2 < 0.0f) || (fD3 < 0.0f);
	bPositive = (fD1 > 0.0f) || (fD2 > 0.0f) || (fD3 > 0.0f);
	return (bNegative && bPositive) ? 0 : 1;
}

static void __xgeXuiTreeViewDrawAaTriangle(xge_vec2_t tA, xge_vec2_t tB, xge_vec2_t tC, uint32_t iColor)
{
	xge_rect_t tPixel;
	xge_vec2_t tSample;
	float fMinX;
	float fMinY;
	float fMaxX;
	float fMaxY;
	float fAlpha;
	int iMinX;
	int iMinY;
	int iMaxX;
	int iMaxY;
	int iX;
	int iY;
	int iSX;
	int iSY;
	int iCoverage;
	const int iSamples = 4;

	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	fMinX = __xgeXuiTreeViewMin3(tA.fX, tB.fX, tC.fX);
	fMinY = __xgeXuiTreeViewMin3(tA.fY, tB.fY, tC.fY);
	fMaxX = __xgeXuiTreeViewMax3(tA.fX, tB.fX, tC.fX);
	fMaxY = __xgeXuiTreeViewMax3(tA.fY, tB.fY, tC.fY);
	iMinX = __xgeXuiTreeViewFloor(fMinX) - 1;
	iMinY = __xgeXuiTreeViewFloor(fMinY) - 1;
	iMaxX = __xgeXuiTreeViewCeil(fMaxX) + 1;
	iMaxY = __xgeXuiTreeViewCeil(fMaxY) + 1;
	for ( iY = iMinY; iY <= iMaxY; iY++ ) {
		for ( iX = iMinX; iX <= iMaxX; iX++ ) {
			iCoverage = 0;
			for ( iSY = 0; iSY < iSamples; iSY++ ) {
				for ( iSX = 0; iSX < iSamples; iSX++ ) {
					tSample.fX = (float)iX + ((float)iSX + 0.5f) / (float)iSamples;
					tSample.fY = (float)iY + ((float)iSY + 0.5f) / (float)iSamples;
					if ( __xgeXuiTreeViewPointInTriangle(tSample, tA, tB, tC) ) {
						iCoverage++;
					}
				}
			}
			if ( iCoverage <= 0 ) {
				continue;
			}
			fAlpha = (float)iCoverage / (float)(iSamples * iSamples);
			tPixel = (xge_rect_t){ (float)iX, (float)iY, 1.0f, 1.0f };
			__xgeXuiHostDrawRect(tPixel, __xgeXuiTreeViewAlphaColor(iColor, fAlpha));
		}
	}
}

static void __xgeXuiTreeViewDrawExpander(xge_rect_t tRect, int bExpanded, uint32_t iColor)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	float fSize;
	float fBase;
	float fDepth;
	float fCX;
	float fCY;

	if ( XGE_COLOR_GET_A(iColor) == 0 ) {
		return;
	}
	fSize = (tRect.fW < tRect.fH) ? tRect.fW : tRect.fH;
	fBase = fSize * 0.68f;
	fDepth = fSize * 0.48f;
	if ( fBase < 7.0f ) {
		fBase = 7.0f;
	}
	if ( fBase > 9.5f ) {
		fBase = 9.5f;
	}
	if ( fDepth < 5.0f ) {
		fDepth = 5.0f;
	}
	if ( fDepth > 6.5f ) {
		fDepth = 6.5f;
	}
	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	if ( bExpanded ) {
		tA = (xge_vec2_t){ fCX, fCY + fDepth * 0.5f };
		tB = (xge_vec2_t){ fCX + fBase * 0.5f, fCY - fDepth * 0.5f };
		tC = (xge_vec2_t){ fCX - fBase * 0.5f, fCY - fDepth * 0.5f };
	} else {
		tA = (xge_vec2_t){ fCX + fDepth * 0.5f, fCY };
		tB = (xge_vec2_t){ fCX - fDepth * 0.5f, fCY - fBase * 0.5f };
		tC = (xge_vec2_t){ fCX - fDepth * 0.5f, fCY + fBase * 0.5f };
	}
	__xgeXuiTreeViewDrawAaTriangle(tA, tB, tC, iColor);
}

static void __xgeXuiTreeViewDrawCheck(xge_rect_t tRect, int bChecked, uint32_t iColor, int bEnabled)
{
	uint32_t iBorder;

	iBorder = bEnabled ? iColor : XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), 110);
	__xgeXuiHostDrawRect(tRect, XGE_COLOR_RGBA(255, 255, 255, bEnabled ? 210 : 80));
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, iBorder);
	if ( bChecked ) {
		__xgeXuiHostDrawLine(tRect.fX + 2.0f, tRect.fY + 5.5f, tRect.fX + 4.0f, tRect.fY + 8.0f, 1.4f, iBorder);
		__xgeXuiHostDrawLine(tRect.fX + 4.0f, tRect.fY + 8.0f, tRect.fX + 8.0f, tRect.fY + 2.5f, 1.4f, iBorder);
	}
}

static void __xgeXuiTreeViewDrawIcon(xge_rect_t tRect, int bFolder, uint32_t iColor, int bEnabled)
{
	xge_rect_t tBody;
	xge_rect_t tTab;
	uint32_t iFill;

	iFill = bEnabled ? iColor : XGE_COLOR_RGBA(XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), 120);
	if ( bFolder ) {
		tTab = (xge_rect_t){ tRect.fX + 1.0f, tRect.fY + 2.0f, 5.0f, 3.0f };
		tBody = (xge_rect_t){ tRect.fX + 1.0f, tRect.fY + 4.0f, 12.0f, 8.0f };
		__xgeXuiHostDrawRect(tTab, iFill);
		__xgeXuiHostDrawRect(tBody, iFill);
		__xgeXuiHostDrawBorderRect(tBody, 1.0f, XGE_COLOR_RGBA(255, 255, 255, bEnabled ? 120 : 70));
	} else {
		tBody = (xge_rect_t){ tRect.fX + 3.0f, tRect.fY + 1.0f, 9.0f, 12.0f };
		__xgeXuiHostDrawRect(tBody, XGE_COLOR_RGBA(255, 255, 255, bEnabled ? 210 : 100));
		__xgeXuiHostDrawBorderRect(tBody, 1.0f, iFill);
		__xgeXuiHostDrawLine(tBody.fX + 2.0f, tBody.fY + 4.0f, tBody.fX + 7.0f, tBody.fY + 4.0f, 1.0f, iFill);
		__xgeXuiHostDrawLine(tBody.fX + 2.0f, tBody.fY + 7.0f, tBody.fX + 7.0f, tBody.fY + 7.0f, 1.0f, iFill);
	}
}

static int __xgeXuiTreeViewItemState(xge_xui_tree_view pTree, int iVisible)
{
	xge_xui_tree_view_node_t* pNode;
	int iState;

	if ( (pTree == NULL) || (iVisible < 0) || (iVisible >= pTree->iVisibleCount) ) {
		return 0;
	}
	pNode = &pTree->arrNodes[pTree->arrVisible[iVisible]];
	iState = 0;
	if ( pNode->iId == pTree->iSelectedId ) {
		iState |= XGE_XUI_TREE_ITEM_SELECTED;
	}
	if ( iVisible == pTree->iHoverVisible ) {
		iState |= XGE_XUI_TREE_ITEM_HOVER;
	}
	if ( pNode->bEnabled == 0 ) {
		iState |= XGE_XUI_TREE_ITEM_DISABLED;
	}
	if ( (iVisible == pTree->iFocusVisible) && (pTree->pContext != NULL) && (pTree->pContext->pFocus == pTree->pWidget) ) {
		iState |= XGE_XUI_TREE_ITEM_FOCUS;
	}
	if ( pNode->bExpanded != 0 ) {
		iState |= XGE_XUI_TREE_ITEM_EXPANDED;
	}
	if ( pNode->bHasChildren != 0 ) {
		iState |= XGE_XUI_TREE_ITEM_HAS_CHILDREN;
	}
	if ( pNode->bChecked != 0 ) {
		iState |= XGE_XUI_TREE_ITEM_CHECKED;
	}
	return iState;
}

static void __xgeXuiTreeViewViewportPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tree_view pTree;
	xge_xui_tree_view_node_t* pNode;
	xge_rect_t tViewport;
	xge_rect_t tRow;
	xge_rect_t tText;
	xge_rect_t tExpander;
	xge_rect_t tCheck;
	xge_rect_t tIcon;
	float fY;
	float fBottom;
	float fX;
	int iVisible;
	int iNode;
	int iState;
	uint32_t iRowColor;
	uint32_t iTextColor;
	uint32_t iExpander;

	pTree = (xge_xui_tree_view)pUser;
	if ( (pWidget == NULL) || (pTree == NULL) || (pTree->fItemHeight <= 0.0f) ) {
		return;
	}
	tViewport = pWidget->tContentRect;
	if ( (tViewport.fW <= 0.0f) || (tViewport.fH <= 0.0f) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pTree->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(tViewport, pTree->iBackgroundColor);
	}
	iVisible = (int)(pTree->tScroll.fScrollY / pTree->fItemHeight);
	if ( iVisible < 0 ) {
		iVisible = 0;
	}
	fY = tViewport.fY + ((float)iVisible * pTree->fItemHeight) - pTree->tScroll.fScrollY;
	fBottom = tViewport.fY + tViewport.fH;
	pTree->iFirstVisible = iVisible;
	pTree->iPaintVisibleCount = 0;
	while ( (iVisible < pTree->iVisibleCount) && (fY < fBottom) ) {
		iNode = pTree->arrVisible[iVisible];
		pNode = &pTree->arrNodes[iNode];
		tRow = __xgeXuiTreeViewRowRect(pTree, iVisible);
		pNode->tRect = tRow;
		iState = __xgeXuiTreeViewItemState(pTree, iVisible);
		if ( (pTree->procItem != NULL) && (pTree->procItem(pTree->pWidget, pNode->iId, iVisible, pNode, tRow, iState, pTree->pItemUser) != 0) ) {
			fY += pTree->fItemHeight;
			iVisible++;
			pTree->iPaintVisibleCount++;
			continue;
		}
		iRowColor = pTree->iRowColor;
		if ( (iState & XGE_XUI_TREE_ITEM_HOVER) != 0 ) {
			iRowColor = pTree->iHoverColor;
		}
		if ( (iState & XGE_XUI_TREE_ITEM_SELECTED) != 0 ) {
			iRowColor = pTree->iSelectedColor;
		}
		__xgeXuiHostDrawRect(tRow, iRowColor);
		if ( (iState & XGE_XUI_TREE_ITEM_FOCUS) != 0 ) {
			__xgeXuiHostDrawBorderRect(tRow, 1.0f, XGE_COLOR_RGBA(74, 142, 210, 190));
		}
		tExpander = __xgeXuiTreeViewExpanderRect(pTree, pNode, tRow);
		iExpander = (pNode->bEnabled != 0) ? pTree->iExpanderColor : XGE_COLOR_RGBA(XGE_COLOR_GET_R(pTree->iExpanderColor), XGE_COLOR_GET_G(pTree->iExpanderColor), XGE_COLOR_GET_B(pTree->iExpanderColor), 120);
		if ( pNode->bHasChildren != 0 ) {
			__xgeXuiTreeViewDrawExpander(tExpander, pNode->bExpanded, iExpander);
		}
		fX = tExpander.fX + 16.0f;
		if ( pNode->bCheckReserved != 0 ) {
			tCheck = __xgeXuiTreeViewCheckRect(pNode, tExpander);
			__xgeXuiTreeViewDrawCheck(tCheck, pNode->bChecked, pTree->iCheckColor, pNode->bEnabled);
			fX = tCheck.fX + tCheck.fW + 5.0f;
		}
		if ( pNode->bIconReserved != 0 ) {
			tIcon = (xge_rect_t){ fX, tRow.fY + (tRow.fH - 14.0f) * 0.5f, 14.0f, 14.0f };
			__xgeXuiTreeViewDrawIcon(tIcon, pNode->bHasChildren, pTree->iIconColor, pNode->bEnabled);
			fX = tIcon.fX + tIcon.fW + 5.0f;
		}
		tText = tRow;
		tText.fX = fX;
		tText.fW = tRow.fX + tRow.fW - tText.fX - 6.0f;
		if ( tText.fW < 1.0f ) {
			tText.fW = 1.0f;
		}
		if ( (pTree->pFont != NULL) && (pNode->sText != NULL) ) {
			iTextColor = (pNode->bEnabled != 0) ? pTree->iTextColor : pTree->iDisabledTextColor;
			__xgeXuiHostDrawTextRect(pTree->pFont, pNode->sText, tText, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		fY += pTree->fItemHeight;
		iVisible++;
		pTree->iPaintVisibleCount++;
	}
}

static void __xgeXuiTreeViewLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tree_view pTree;

	(void)pWidget;
	pTree = (xge_xui_tree_view)pUser;
	__xgeXuiTreeViewSyncFrame(pTree);
}

int xgeXuiTreeViewInit(xge_xui_tree_view pTree, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTree == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTree, 0, sizeof(*pTree));
	pTheme = xgeXuiGetTheme(pContext);
	pTree->pContext = pContext;
	pTree->pWidget = pWidget;
	pTree->pFont = (pTheme != NULL) ? pTheme->pFont : NULL;
	pTree->iSelectedId = -1;
	pTree->iHoverVisible = -1;
	pTree->iFocusVisible = -1;
	pTree->iActiveVisible = -1;
	pTree->iActivePart = XGE_XUI_TREE_ACTIVE_ROW;
	pTree->fItemHeight = 24.0f;
	pTree->fIndent = 18.0f;
	pTree->iBorderColor = XGE_COLOR_RGBA(184, 223, 245, 255);
	pTree->iBackgroundColor = (pTheme != NULL) ? pTheme->iPanelColor : XGE_COLOR_RGBA(236, 246, 253, 255);
	pTree->iRowColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pTree->iHoverColor = XGE_COLOR_RGBA(232, 243, 252, 255);
	pTree->iSelectedColor = XGE_COLOR_RGBA(190, 219, 242, 255);
	pTree->iTextColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(76, 96, 116, 255);
	pTree->iDisabledTextColor = XGE_COLOR_RGBA(142, 152, 166, 190);
	pTree->iExpanderColor = XGE_COLOR_RGBA(55, 118, 176, 255);
	pTree->iIconColor = XGE_COLOR_RGBA(68, 139, 203, 230);
	pTree->iCheckColor = XGE_COLOR_RGBA(62, 126, 184, 230);
	pTree->iBarColor = XGE_COLOR_RGBA(218, 232, 244, 210);
	pTree->iThumbColor = XGE_COLOR_RGBA(126, 166, 200, 230);
	xgeXuiScrollModelInit(&pTree->tScroll);
	__xgeXuiViewportWidgetInit(pWidget, 1);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetBackground(pWidget, pTree->iBackgroundColor);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pTree->iBorderColor);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTreeViewEventProc, pTree);
	xgeXuiWidgetSetLayoutProc(pWidget, __xgeXuiTreeViewLayoutProc, pTree);
	pWidget->pUser = pTree;
	if ( xgeXuiScrollFrameInit(&pTree->tFrame, pContext, pWidget, &pTree->tScroll) != XGE_OK ) {
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
		pWidget->pUser = NULL;
		memset(pTree, 0, sizeof(*pTree));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiScrollFrameSetScrollbarPolicy(&pTree->tFrame, XGE_XUI_SCROLLBAR_POLICY_HIDDEN, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	xgeXuiScrollFrameSetScrollbarMode(&pTree->tFrame, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiScrollFrameSetContentDragEnabled(&pTree->tFrame, 0);
	xgeXuiScrollFrameSetChange(&pTree->tFrame, __xgeXuiTreeViewFrameChanged, pTree);
	xgeXuiScrollFrameSetColors(&pTree->tFrame, pTree->iBarColor, pTree->iThumbColor, pTree->iThumbColor, pTree->iThumbColor, pTree->iThumbColor, pTree->iBarColor);
	xgeXuiScrollFrameSetButtonColors(&pTree->tFrame, pTree->iBarColor, pTree->iThumbColor);
	xgeXuiScrollFrameSetCornerColors(&pTree->tFrame, pTree->iBarColor, pTree->iThumbColor);
	xgeXuiWidgetSetPaint(xgeXuiScrollFrameGetViewportWidget(&pTree->tFrame), __xgeXuiTreeViewViewportPaintProc, pTree);
	__xgeXuiTreeViewSyncFrame(pTree);
	return XGE_OK;
}

void xgeXuiTreeViewUnit(xge_xui_tree_view pTree)
{
	xge_xui_widget pWidget;

	if ( pTree == NULL ) {
		return;
	}
	pWidget = pTree->pWidget;
	xgeXuiReleaseWidgetCapture(pTree->pContext, pWidget);
	if ( pWidget != NULL && pWidget->pUser == pTree ) {
		pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
	}
	xgeXuiScrollFrameUnit(&pTree->tFrame);
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
	pTree->iFocusVisible = -1;
	pTree->iActiveVisible = -1;
	xgeXuiScrollFrameSetOffset(&pTree->tFrame, 0.0f, 0.0f);
	__xgeXuiTreeViewSyncFrame(pTree);
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
	pNode->bEnabled = 1;
	pNode->bIconReserved = 1;
	pNode->sText = (sText != NULL) ? sText : "";
	if ( iParent >= 0 ) {
		pTree->arrNodes[iParent].bHasChildren = 1;
	}
	pTree->iNodeCount++;
	__xgeXuiTreeViewRebuildVisible(pTree, -1);
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
	fScrollY = pTree->tScroll.fScrollY;
	pTree->iNodeCount = 0;
	pTree->iVisibleCount = 0;
	pTree->iSelectedId = iSelectedId;
	pTree->iHoverVisible = -1;
	pTree->iFocusVisible = -1;
	for ( i = 0; i < iCount; i++ ) {
		memset(&tNode, 0, sizeof(tNode));
		tNode.iId = -1;
		tNode.iParent = -1;
		tNode.sText = "";
		tNode.bEnabled = 1;
		tNode.bIconReserved = 1;
		if ( pTree->procNode(pTree->pWidget, i, &tNode, pTree->pAdapterUser) != XGE_OK ) {
			continue;
		}
		iAdded = xgeXuiTreeViewAddNode(pTree, tNode.iId, tNode.iParent, tNode.sText);
		if ( iAdded < 0 ) {
			continue;
		}
		pTree->arrNodes[iAdded].bExpanded = tNode.bExpanded;
		pTree->arrNodes[iAdded].bEnabled = tNode.bEnabled != 0;
		pTree->arrNodes[iAdded].bIconReserved = tNode.bIconReserved;
		pTree->arrNodes[iAdded].bCheckReserved = tNode.bCheckReserved;
		pTree->arrNodes[iAdded].bChecked = tNode.bChecked;
	}
	pTree->iSelectedId = iSelectedId;
	__xgeXuiTreeViewRebuildVisible(pTree, -1);
	xgeXuiTreeViewSetScroll(pTree, fScrollY);
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
	__xgeXuiTreeViewRebuildVisible(pTree, bExpanded ? -1 : iId);
}

int xgeXuiTreeViewGetNodeExpanded(xge_xui_tree_view pTree, int iId)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	return (iNode >= 0) ? pTree->arrNodes[iNode].bExpanded : 0;
}

void xgeXuiTreeViewSetNodeEnabled(xge_xui_tree_view pTree, int iId, int bEnabled)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	if ( iNode < 0 ) {
		return;
	}
	pTree->arrNodes[iNode].bEnabled = (bEnabled != 0);
	__xgeXuiTreeViewRebuildVisible(pTree, -1);
}

int xgeXuiTreeViewGetNodeEnabled(xge_xui_tree_view pTree, int iId)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	return (iNode >= 0) ? pTree->arrNodes[iNode].bEnabled : 0;
}

void xgeXuiTreeViewSetNodeChecked(xge_xui_tree_view pTree, int iId, int bChecked)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	if ( iNode < 0 ) {
		return;
	}
	pTree->arrNodes[iNode].bCheckReserved = 1;
	pTree->arrNodes[iNode].bChecked = (bChecked != 0);
	__xgeXuiTreeViewInvalidate(pTree);
}

int xgeXuiTreeViewGetNodeChecked(xge_xui_tree_view pTree, int iId)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	return (iNode >= 0) ? pTree->arrNodes[iNode].bChecked : 0;
}

void xgeXuiTreeViewSetNodeDecorations(xge_xui_tree_view pTree, int iId, int bIcon, int bCheck)
{
	int iNode;

	iNode = __xgeXuiTreeViewFindNode(pTree, iId);
	if ( iNode < 0 ) {
		return;
	}
	pTree->arrNodes[iNode].bIconReserved = (bIcon != 0);
	pTree->arrNodes[iNode].bCheckReserved = (bCheck != 0);
	__xgeXuiTreeViewInvalidate(pTree);
}

void xgeXuiTreeViewSetSelected(xge_xui_tree_view pTree, int iId)
{
	int iVisible;
	int iNode;

	if ( pTree == NULL ) {
		return;
	}
	iVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, iId);
	iNode = (iVisible >= 0) ? pTree->arrVisible[iVisible] : -1;
	if ( (iVisible < 0) || (__xgeXuiTreeViewNodeEnabled(pTree, iNode) == 0) ) {
		iId = -1;
		iVisible = -1;
	}
	if ( pTree->iSelectedId != iId ) {
		pTree->iSelectedId = iId;
		__xgeXuiTreeViewInvalidate(pTree);
	}
	__xgeXuiTreeViewSetFocusVisible(pTree, iVisible);
	if ( iVisible >= 0 ) {
		pTree->bEnsureSelectedPending = 1;
		__xgeXuiTreeViewEnsureVisible(pTree, iVisible);
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

void xgeXuiTreeViewSetFont(xge_xui_tree_view pTree, xui_font pFont)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->pFont = pFont;
	__xgeXuiTreeViewInvalidate(pTree);
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
	__xgeXuiTreeViewSyncFrame(pTree);
}

void xgeXuiTreeViewSetScroll(xge_xui_tree_view pTree, float fScrollY)
{
	if ( pTree != NULL ) {
		pTree->bEnsureSelectedPending = 0;
		xgeXuiScrollFrameSetOffset(&pTree->tFrame, 0.0f, fScrollY);
		__xgeXuiTreeViewUpdateVisibleWindow(pTree);
		__xgeXuiTreeViewInvalidate(pTree);
	}
}

float xgeXuiTreeViewGetScroll(xge_xui_tree_view pTree)
{
	return (pTree != NULL) ? pTree->tScroll.fScrollY : 0.0f;
}

void xgeXuiTreeViewSetScrollbarMode(xge_xui_tree_view pTree, int iMode)
{
	if ( pTree != NULL ) {
		xgeXuiScrollFrameSetScrollbarMode(&pTree->tFrame, iMode);
		__xgeXuiTreeViewInvalidate(pTree);
	}
}

int xgeXuiTreeViewGetScrollbarMode(xge_xui_tree_view pTree)
{
	return (pTree != NULL) ? xgeXuiScrollFrameGetScrollbarMode(&pTree->tFrame) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiTreeViewSetSelect(xge_xui_tree_view pTree, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->procSelect = procSelect;
	pTree->pSelectUser = pUser;
}

void xgeXuiTreeViewSetItemRenderer(xge_xui_tree_view pTree, xge_xui_tree_view_item_proc procItem, void* pUser)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->procItem = procItem;
	pTree->pItemUser = pUser;
	__xgeXuiTreeViewInvalidate(pTree);
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
	xgeXuiWidgetSetBackground(pTree->pWidget, iBackground);
	xgeXuiScrollFrameSetColors(&pTree->tFrame, iBar, iThumb, iThumb, iThumb, iThumb, iBar);
	xgeXuiScrollFrameSetButtonColors(&pTree->tFrame, iBar, iThumb);
	xgeXuiScrollFrameSetCornerColors(&pTree->tFrame, iBar, iThumb);
	__xgeXuiTreeViewInvalidate(pTree);
}

void xgeXuiTreeViewSetDisabledTextColor(xge_xui_tree_view pTree, uint32_t iColor)
{
	if ( pTree == NULL ) {
		return;
	}
	pTree->iDisabledTextColor = iColor;
	__xgeXuiTreeViewInvalidate(pTree);
}

int xgeXuiTreeViewEvent(xge_xui_tree_view pTree, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;
	xge_xui_widget pCapture;
	int iInsideWidget;
	int iVisible;
	int iSelectedVisible;
	int iTarget;
	int iNode;
	int iRet;

	if ( (pTree == NULL) || (pTree->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pTree->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pTree->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInsideWidget = __xgeXuiRectContains(pTree->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			iRet = __xgeXuiTreeViewForwardScrollBars(pTree, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			iRet = xgeXuiScrollFrameEvent(&pTree->tFrame, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				__xgeXuiTreeViewInvalidate(pTree);
			}
			return iRet;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			iRet = __xgeXuiTreeViewForwardScrollBars(pTree, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
			if ( __xgeXuiRectContains(tViewport, pEvent->fX, pEvent->fY) ) {
				__xgeXuiTreeViewSetHoverVisible(pTree, __xgeXuiTreeViewIndexAt(pTree, pEvent->fY));
			} else {
				__xgeXuiTreeViewSetHoverVisible(pTree, -1);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInsideWidget == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTree->pContext, pTree->pWidget);
			iRet = __xgeXuiTreeViewForwardScrollBars(pTree, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			tViewport = xgeXuiScrollFrameGetViewportRect(&pTree->tFrame);
			if ( __xgeXuiRectContains(tViewport, pEvent->fX, pEvent->fY) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = __xgeXuiTreeViewIndexAt(pTree, pEvent->fY);
			if ( iVisible < 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iNode = pTree->arrVisible[iVisible];
			if ( __xgeXuiTreeViewNodeEnabled(pTree, iNode) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			pTree->iActiveVisible = iVisible;
			pTree->iActivePart = XGE_XUI_TREE_ACTIVE_ROW;
			if ( __xgeXuiTreeViewHitExpander(pTree, iVisible, pEvent->fX, pEvent->fY) ) {
				pTree->iActivePart = XGE_XUI_TREE_ACTIVE_EXPANDER;
			} else if ( __xgeXuiTreeViewHitCheck(pTree, iVisible, pEvent->fX, pEvent->fY) ) {
				pTree->iActivePart = XGE_XUI_TREE_ACTIVE_CHECK;
			}
			__xgeXuiTreeViewSetFocusVisible(pTree, iVisible);
			xgeXuiSetPointerCapture(pTree->pContext, pEvent->iPointerId, pTree->pWidget);
			__xgeXuiTreeViewInvalidate(pTree);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			pCapture = (pTree->pContext != NULL) ? xgeXuiGetPointerCapture(pTree->pContext, pEvent->iPointerId) : NULL;
			if ( (pTree->iActiveVisible >= 0) && (pCapture == pTree->pWidget) ) {
				iVisible = __xgeXuiTreeViewIndexAt(pTree, pEvent->fY);
				if ( iVisible == pTree->iActiveVisible ) {
					iNode = pTree->arrVisible[iVisible];
					if ( pTree->iActivePart == XGE_XUI_TREE_ACTIVE_EXPANDER && pTree->arrNodes[iNode].bHasChildren != 0 ) {
						pTree->arrNodes[iNode].bExpanded = !pTree->arrNodes[iNode].bExpanded;
						__xgeXuiTreeViewRebuildVisible(pTree, pTree->arrNodes[iNode].bExpanded ? -1 : pTree->arrNodes[iNode].iId);
					} else if ( pTree->iActivePart == XGE_XUI_TREE_ACTIVE_CHECK && pTree->arrNodes[iNode].bCheckReserved != 0 ) {
						pTree->arrNodes[iNode].bChecked = !pTree->arrNodes[iNode].bChecked;
						__xgeXuiTreeViewInvalidate(pTree);
					} else {
						__xgeXuiTreeViewSelectVisible(pTree, iVisible, 1);
					}
				}
				pTree->iActiveVisible = -1;
				pTree->iActivePart = XGE_XUI_TREE_ACTIVE_ROW;
				xgeXuiSetPointerCapture(pTree->pContext, pEvent->iPointerId, NULL);
				__xgeXuiTreeViewInvalidate(pTree);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return __xgeXuiTreeViewForwardScrollBars(pTree, pEvent);

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pTree->iActiveVisible = -1;
			pTree->iActivePart = XGE_XUI_TREE_ACTIVE_ROW;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiTreeViewSetHoverVisible(pTree, -1);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pTree->pContext == NULL || pTree->pContext->pFocus != pTree->pWidget || pTree->iVisibleCount <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iSelectedVisible = __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->iSelectedId);
			iTarget = iSelectedVisible;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iTarget = (iSelectedVisible < 0) ? 0 : iSelectedVisible + 1;
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, iTarget, 1);
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iTarget = (iSelectedVisible < 0) ? (pTree->iVisibleCount - 1) : iSelectedVisible - 1;
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, iTarget, -1);
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iTarget = (iSelectedVisible < 0) ? 0 : iSelectedVisible + __xgeXuiTreeViewVisibleRows(pTree);
				if ( iTarget >= pTree->iVisibleCount ) {
					iTarget = pTree->iVisibleCount - 1;
				}
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, iTarget, -1);
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iTarget = (iSelectedVisible < 0) ? 0 : iSelectedVisible - __xgeXuiTreeViewVisibleRows(pTree);
				if ( iTarget < 0 ) {
					iTarget = 0;
				}
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, iTarget, 1);
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, 0, 1);
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, pTree->iVisibleCount - 1, -1);
			} else if ( (pEvent->iParam1 == XGE_KEY_RIGHT) && (iSelectedVisible >= 0) ) {
				iNode = pTree->arrVisible[iSelectedVisible];
				if ( pTree->arrNodes[iNode].bHasChildren != 0 && pTree->arrNodes[iNode].bExpanded == 0 ) {
					pTree->arrNodes[iNode].bExpanded = 1;
					__xgeXuiTreeViewRebuildVisible(pTree, -1);
				} else if ( pTree->arrNodes[iNode].bHasChildren != 0 ) {
					iTarget = __xgeXuiTreeViewNextEnabledVisible(pTree, iSelectedVisible + 1, 1);
					if ( iTarget >= 0 ) {
						__xgeXuiTreeViewSelectVisible(pTree, iTarget, 1);
					}
				}
				return XGE_XUI_EVENT_CONSUMED;
			} else if ( (pEvent->iParam1 == XGE_KEY_LEFT) && (iSelectedVisible >= 0) ) {
				iNode = pTree->arrVisible[iSelectedVisible];
				if ( pTree->arrNodes[iNode].bHasChildren != 0 && pTree->arrNodes[iNode].bExpanded != 0 ) {
					pTree->arrNodes[iNode].bExpanded = 0;
					__xgeXuiTreeViewRebuildVisible(pTree, pTree->arrNodes[iNode].iId);
				} else if ( pTree->arrNodes[iNode].iParent >= 0 ) {
					iTarget = __xgeXuiTreeViewVisibleIndexOfId(pTree, pTree->arrNodes[iNode].iParent);
					if ( iTarget >= 0 ) {
						__xgeXuiTreeViewSelectVisible(pTree, iTarget, 1);
					}
				}
				return XGE_XUI_EVENT_CONSUMED;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiTreeViewNotifySelect(pTree);
				return (pTree->iSelectedId >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iTarget < 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiTreeViewSelectVisible(pTree, iTarget, 1);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTreeViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiTreeViewEvent((xge_xui_tree_view)pUser, pEvent);
}

void xgeXuiTreeViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_tree_view pTree;

	(void)pWidget;
	pTree = (xge_xui_tree_view)pUser;
	if ( pTree == NULL && pWidget != NULL ) {
		pTree = (xge_xui_tree_view)pWidget->pUser;
	}
	if ( pTree == NULL ) {
		return;
	}
	__xgeXuiTreeViewViewportPaintProc(xgeXuiScrollFrameGetViewportWidget(&pTree->tFrame), pTree);
}
