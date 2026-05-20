static float __xgeXuiSplitLayoutAxisSize(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		return pSplitLayout->pWidget->tContentRect.fH;
	}
	return pSplitLayout->pWidget->tContentRect.fW;
}

static float __xgeXuiSplitLayoutCrossSize(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		return pSplitLayout->pWidget->tContentRect.fW;
	}
	return pSplitLayout->pWidget->tContentRect.fH;
}

static float __xgeXuiSplitLayoutEventAxis(xge_xui_split_layout pSplitLayout, const xge_event_t* pEvent)
{
	if ( (pSplitLayout == NULL) || (pEvent == NULL) ) {
		return 0.0f;
	}
	return (pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL) ? pEvent->fY : pEvent->fX;
}

static float __xgeXuiSplitLayoutAxisStart(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	return (pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL) ? pSplitLayout->pWidget->tContentRect.fY : pSplitLayout->pWidget->tContentRect.fX;
}

static float __xgeXuiSplitLayoutCrossStart(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	return (pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL) ? pSplitLayout->pWidget->tContentRect.fX : pSplitLayout->pWidget->tContentRect.fY;
}

static int __xgeXuiSplitLayoutDividerIndex(xge_xui_widget pWidget)
{
	return (pWidget != NULL) ? pWidget->iId : -1;
}

static float __xgeXuiSplitLayoutPaneMin(xge_xui_split_layout_pane_t* pPane)
{
	if ( pPane == NULL || pPane->bCollapsed != 0 ) {
		return 0.0f;
	}
	return (pPane->fMinSize > 0.0f) ? pPane->fMinSize : 0.0f;
}

static float __xgeXuiSplitLayoutPaneMax(xge_xui_split_layout_pane_t* pPane)
{
	if ( pPane == NULL || pPane->bCollapsed != 0 ) {
		return 0.0f;
	}
	return (pPane->fMaxSize > 0.0f) ? pPane->fMaxSize : 0.0f;
}

static float __xgeXuiSplitLayoutClampPaneSize(xge_xui_split_layout_pane_t* pPane, float fSize)
{
	float fMin;
	float fMax;

	if ( pPane == NULL ) {
		return fSize;
	}
	fMin = __xgeXuiSplitLayoutPaneMin(pPane);
	fMax = __xgeXuiSplitLayoutPaneMax(pPane);
	if ( fSize < fMin ) {
		fSize = fMin;
	}
	if ( (fMax > 0.0f) && (fSize > fMax) ) {
		fSize = fMax;
	}
	return (fSize > 0.0f) ? fSize : 0.0f;
}

static void __xgeXuiSplitLayoutNormalizeWeights(xge_xui_split_layout pSplitLayout)
{
	float fTotal;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->arrPanes == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
		return;
	}
	fTotal = 0.0f;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		if ( pSplitLayout->arrPanes[i].fWeight < 0.0f ) {
			pSplitLayout->arrPanes[i].fWeight = 0.0f;
		}
		if ( pSplitLayout->arrPanes[i].iMode != XGE_XUI_SPLIT_PANE_FIXED ) {
			fTotal += pSplitLayout->arrPanes[i].fWeight;
		}
	}
	if ( fTotal <= 0.0f ) {
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			if ( pSplitLayout->arrPanes[i].iMode != XGE_XUI_SPLIT_PANE_FIXED ) {
				pSplitLayout->arrPanes[i].fWeight = 1.0f;
			}
		}
	}
}

static void __xgeXuiSplitLayoutInitPaneDefaults(xge_xui_split_layout_pane_t* pPane)
{
	if ( pPane == NULL ) {
		return;
	}
	memset(pPane, 0, sizeof(*pPane));
	pPane->fWeight = 1.0f;
	pPane->fFixedSize = 160.0f;
	pPane->fMinSize = 64.0f;
	pPane->fMaxSize = 0.0f;
	pPane->iMode = XGE_XUI_SPLIT_PANE_GROW;
}

static void __xgeXuiSplitLayoutSetLocalRect(xge_xui_widget pWidget, xge_rect_t tRect)
{
	if ( pWidget == NULL ) {
		return;
	}
	pWidget->tLocalRect = tRect;
	__xgeXuiWidgetArrangeRect(pWidget, tRect);
}

static void __xgeXuiSplitLayoutHideShadow(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pShadowWidget == NULL) ) {
		return;
	}
	xgeXuiWidgetSetVisible(pSplitLayout->pShadowWidget, 0);
}

static void __xgeXuiSplitLayoutResolveOversubscribed(xge_xui_split_layout pSplitLayout, float fAvailable)
{
	float fRequired;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->arrPanes == NULL) ) {
		return;
	}
	fRequired = 0.0f;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		if ( pSplitLayout->arrPanes[i].iMode == XGE_XUI_SPLIT_PANE_FIXED ) {
			fRequired += __xgeXuiSplitLayoutClampPaneSize(&pSplitLayout->arrPanes[i], pSplitLayout->arrPanes[i].fFixedSize);
		} else {
			fRequired += __xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[i]);
		}
	}
	if ( fRequired <= 0.0f ) {
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			pSplitLayout->arrPanes[i].fResolvedSize = 0.0f;
		}
		return;
	}
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		float fBase = (pSplitLayout->arrPanes[i].iMode == XGE_XUI_SPLIT_PANE_FIXED) ?
			__xgeXuiSplitLayoutClampPaneSize(&pSplitLayout->arrPanes[i], pSplitLayout->arrPanes[i].fFixedSize) :
			__xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[i]);
		pSplitLayout->arrPanes[i].fResolvedSize = fAvailable * (fBase / fRequired);
	}
}

static void __xgeXuiSplitLayoutResolveSizes(xge_xui_split_layout pSplitLayout, float fAvailable)
{
	char arrLocked[XGE_XUI_SPLIT_LAYOUT_MAX_PANES];
	float fDividerTotal;
	float fFixedTotal;
	float fGrowMinTotal;
	float fRemaining;
	float fWeightRemaining;
	float fProposed;
	float fMin;
	float fMax;
	int bChanged;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->arrPanes == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
		return;
	}
	if ( fAvailable < 0.0f ) {
		fAvailable = 0.0f;
	}
	fDividerTotal = (float)(pSplitLayout->iPaneCount - 1) * pSplitLayout->fDividerSize;
	if ( fDividerTotal > fAvailable ) {
		fDividerTotal = fAvailable;
	}
	fAvailable -= fDividerTotal;
	if ( fAvailable < 0.0f ) {
		fAvailable = 0.0f;
	}
	pSplitLayout->fResolvedAxis = fAvailable;
	__xgeXuiSplitLayoutNormalizeWeights(pSplitLayout);
	memset(arrLocked, 0, sizeof(arrLocked));
	fFixedTotal = 0.0f;
	fGrowMinTotal = 0.0f;
	fWeightRemaining = 0.0f;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		pSplitLayout->arrPanes[i].fResolvedSize = 0.0f;
		if ( pSplitLayout->arrPanes[i].fMinSize < 0.0f ) {
			pSplitLayout->arrPanes[i].fMinSize = 0.0f;
		}
		if ( pSplitLayout->arrPanes[i].fMaxSize < 0.0f ) {
			pSplitLayout->arrPanes[i].fMaxSize = 0.0f;
		}
		if ( pSplitLayout->arrPanes[i].iMode == XGE_XUI_SPLIT_PANE_FIXED ) {
			pSplitLayout->arrPanes[i].fResolvedSize = __xgeXuiSplitLayoutClampPaneSize(&pSplitLayout->arrPanes[i], pSplitLayout->arrPanes[i].fFixedSize);
			fFixedTotal += pSplitLayout->arrPanes[i].fResolvedSize;
			arrLocked[i] = 1;
		} else {
			fGrowMinTotal += __xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[i]);
			fWeightRemaining += (pSplitLayout->arrPanes[i].fWeight > 0.0f) ? pSplitLayout->arrPanes[i].fWeight : 0.0f;
		}
	}
	if ( (fFixedTotal + fGrowMinTotal) > fAvailable ) {
		__xgeXuiSplitLayoutResolveOversubscribed(pSplitLayout, fAvailable);
		return;
	}
	fRemaining = fAvailable - fFixedTotal;
	if ( fWeightRemaining <= 0.0f ) {
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			if ( arrLocked[i] == 0 ) {
				pSplitLayout->arrPanes[i].fWeight = 1.0f;
				fWeightRemaining += 1.0f;
			}
		}
	}
	do {
		bChanged = 0;
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			if ( arrLocked[i] != 0 ) {
				continue;
			}
			fProposed = (fWeightRemaining > 0.0f) ? (fRemaining * (pSplitLayout->arrPanes[i].fWeight / fWeightRemaining)) : 0.0f;
			fMin = __xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[i]);
			fMax = __xgeXuiSplitLayoutPaneMax(&pSplitLayout->arrPanes[i]);
			if ( fProposed < fMin ) {
				pSplitLayout->arrPanes[i].fResolvedSize = fMin;
				fRemaining -= fMin;
				fWeightRemaining -= pSplitLayout->arrPanes[i].fWeight;
				arrLocked[i] = 1;
				bChanged = 1;
			} else if ( (fMax > 0.0f) && (fProposed > fMax) ) {
				pSplitLayout->arrPanes[i].fResolvedSize = fMax;
				fRemaining -= fMax;
				fWeightRemaining -= pSplitLayout->arrPanes[i].fWeight;
				arrLocked[i] = 1;
				bChanged = 1;
			}
		}
	} while ( bChanged && (fRemaining > 0.0f) );
	if ( fRemaining < 0.0f ) {
		fRemaining = 0.0f;
	}
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		if ( arrLocked[i] != 0 ) {
			continue;
		}
		pSplitLayout->arrPanes[i].fResolvedSize = (fWeightRemaining > 0.0f) ? (fRemaining * (pSplitLayout->arrPanes[i].fWeight / fWeightRemaining)) : 0.0f;
	}
}

static float __xgeXuiSplitLayoutClampDragAxis(xge_xui_split_layout pSplitLayout, int iDivider, float fAxis)
{
	float fStart;
	float fPairTotal;
	float fMinBefore;
	float fMinAfter;
	float fMaxBefore;
	float fMaxAfter;
	float fMinAxis;
	float fMaxAxis;
	int i;

	if ( (pSplitLayout == NULL) || (iDivider < 0) || (iDivider >= (pSplitLayout->iPaneCount - 1)) ) {
		return fAxis;
	}
	fStart = __xgeXuiSplitLayoutAxisStart(pSplitLayout);
	for ( i = 0; i < iDivider; i++ ) {
		fStart += pSplitLayout->arrPanes[i].fResolvedSize + pSplitLayout->fDividerSize;
	}
	fPairTotal = pSplitLayout->arrPanes[iDivider].fResolvedSize + pSplitLayout->arrPanes[iDivider + 1].fResolvedSize;
	fMinBefore = __xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[iDivider]);
	fMinAfter = __xgeXuiSplitLayoutPaneMin(&pSplitLayout->arrPanes[iDivider + 1]);
	fMaxBefore = __xgeXuiSplitLayoutPaneMax(&pSplitLayout->arrPanes[iDivider]);
	fMaxAfter = __xgeXuiSplitLayoutPaneMax(&pSplitLayout->arrPanes[iDivider + 1]);
	fMinAxis = fStart + fMinBefore;
	fMaxAxis = fStart + fPairTotal - fMinAfter;
	if ( fMaxBefore > 0.0f && (fStart + fMaxBefore) < fMaxAxis ) {
		fMaxAxis = fStart + fMaxBefore;
	}
	if ( fMaxAfter > 0.0f && (fStart + fPairTotal - fMaxAfter) > fMinAxis ) {
		fMinAxis = fStart + fPairTotal - fMaxAfter;
	}
	if ( fMaxAxis < fMinAxis ) {
		fMaxAxis = fMinAxis;
	}
	return __xgeXuiClampFloat(fAxis, fMinAxis, fMaxAxis);
}

static void __xgeXuiSplitLayoutUpdateShadow(xge_xui_split_layout pSplitLayout)
{
	xge_rect_t tRect;
	float fAxis;
	float fCross;
	float fCrossSize;
	float fVisual;

	if ( (pSplitLayout == NULL) || (pSplitLayout->pShadowWidget == NULL) || (pSplitLayout->iActiveDivider < 0) ) {
		return;
	}
	fAxis = __xgeXuiSplitLayoutClampDragAxis(pSplitLayout, pSplitLayout->iActiveDivider, pSplitLayout->fDragCurrentMouse);
	fCross = __xgeXuiSplitLayoutCrossStart(pSplitLayout);
	fCrossSize = __xgeXuiSplitLayoutCrossSize(pSplitLayout);
	fVisual = (pSplitLayout->fDividerVisualSize > 0.0f) ? pSplitLayout->fDividerVisualSize : pSplitLayout->fDividerSize;
	memset(&tRect, 0, sizeof(tRect));
	if ( pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL ) {
		tRect.fX = fCross;
		tRect.fY = fAxis - fVisual * 0.5f;
		tRect.fW = fCrossSize;
		tRect.fH = fVisual;
	} else {
		tRect.fX = fAxis - fVisual * 0.5f;
		tRect.fY = fCross;
		tRect.fW = fVisual;
		tRect.fH = fCrossSize;
	}
	xgeXuiWidgetSetRect(pSplitLayout->pShadowWidget, tRect);
	xgeXuiWidgetSetVisible(pSplitLayout->pShadowWidget, 1);
}

static void __xgeXuiSplitLayoutCommitDrag(xge_xui_split_layout pSplitLayout)
{
	float fAxis;
	float fStart;
	float fPairTotal;
	float fNewBefore;
	float fNewAfter;
	float fPairWeight;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->iActiveDivider < 0) ) {
		return;
	}
	fAxis = __xgeXuiSplitLayoutClampDragAxis(pSplitLayout, pSplitLayout->iActiveDivider, pSplitLayout->fDragCurrentMouse);
	fStart = __xgeXuiSplitLayoutAxisStart(pSplitLayout);
	for ( i = 0; i < pSplitLayout->iActiveDivider; i++ ) {
		fStart += pSplitLayout->arrPanes[i].fResolvedSize + pSplitLayout->fDividerSize;
	}
	fNewBefore = fAxis - fStart;
	fPairTotal = pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].fResolvedSize + pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].fResolvedSize;
	fNewBefore = __xgeXuiSplitLayoutClampPaneSize(&pSplitLayout->arrPanes[pSplitLayout->iActiveDivider], fNewBefore);
	if ( fNewBefore > fPairTotal ) {
		fNewBefore = fPairTotal;
	}
	fNewAfter = fPairTotal - fNewBefore;
	fNewAfter = __xgeXuiSplitLayoutClampPaneSize(&pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1], fNewAfter);
	if ( fNewAfter > fPairTotal ) {
		fNewAfter = fPairTotal;
	}
	fNewBefore = fPairTotal - fNewAfter;
	if ( pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].iMode == XGE_XUI_SPLIT_PANE_FIXED ) {
		pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].fFixedSize = fNewBefore;
	}
	if ( pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].iMode == XGE_XUI_SPLIT_PANE_FIXED ) {
		pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].fFixedSize = fNewAfter;
	}
	if ( (pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].iMode != XGE_XUI_SPLIT_PANE_FIXED) &&
		(pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].iMode != XGE_XUI_SPLIT_PANE_FIXED) ) {
		fPairWeight = pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].fWeight + pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].fWeight;
		if ( fPairWeight <= 0.0f ) {
			fPairWeight = 1.0f;
		}
		if ( fPairTotal > 0.0f ) {
			pSplitLayout->arrPanes[pSplitLayout->iActiveDivider].fWeight = fPairWeight * (fNewBefore / fPairTotal);
			pSplitLayout->arrPanes[pSplitLayout->iActiveDivider + 1].fWeight = fPairWeight * (fNewAfter / fPairTotal);
		}
	}
	if ( pSplitLayout->procChange != NULL ) {
		pSplitLayout->procChange(pSplitLayout->pWidget, pSplitLayout->iActiveDivider, pSplitLayout->pChangeUser);
	}
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

static void __xgeXuiSplitLayoutEnsureShadow(xge_xui_split_layout pSplitLayout)
{
	xge_xui_widget pOverlayRoot;

	if ( (pSplitLayout == NULL) || (pSplitLayout->pContext == NULL) || (pSplitLayout->pShadowWidget != NULL) ) {
		return;
	}
	pOverlayRoot = xgeXuiOverlayRoot(pSplitLayout->pContext);
	if ( pOverlayRoot == NULL ) {
		return;
	}
	pSplitLayout->pShadowWidget = xgeXuiWidgetCreate();
	if ( pSplitLayout->pShadowWidget == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pSplitLayout->pShadowWidget, pSplitLayout->iShadowColor);
	xgeXuiWidgetSetZ(pSplitLayout->pShadowWidget, 1000);
	xgeXuiWidgetSetVisible(pSplitLayout->pShadowWidget, 0);
	(void)xgeXuiWidgetAddInternal(pOverlayRoot, pSplitLayout->pShadowWidget);
}

static void __xgeXuiSplitLayoutFreeArrays(xge_xui_split_layout pSplitLayout)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->arrPanes = (xge_xui_split_layout_pane_t*)xrtRealloc(pSplitLayout->arrPanes, 0);
	pSplitLayout->arrDividers = (xge_xui_split_layout_divider_t*)xrtRealloc(pSplitLayout->arrDividers, 0);
}

static int __xgeXuiSplitLayoutResizeArrays(xge_xui_split_layout pSplitLayout, int iPaneCount)
{
	xge_xui_split_layout_pane_t* arrPanes;
	xge_xui_split_layout_divider_t* arrDividers;
	int iOldCount;
	int iCopyCount;
	int i;

	if ( (pSplitLayout == NULL) || (iPaneCount < 2) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	arrPanes = (xge_xui_split_layout_pane_t*)xrtRealloc(NULL, sizeof(*arrPanes) * (size_t)iPaneCount);
	arrDividers = (xge_xui_split_layout_divider_t*)xrtRealloc(NULL, sizeof(*arrDividers) * (size_t)(iPaneCount - 1));
	if ( (arrPanes == NULL) || (arrDividers == NULL) ) {
		arrPanes = (xge_xui_split_layout_pane_t*)xrtRealloc(arrPanes, 0);
		arrDividers = (xge_xui_split_layout_divider_t*)xrtRealloc(arrDividers, 0);
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	memset(arrPanes, 0, sizeof(*arrPanes) * (size_t)iPaneCount);
	memset(arrDividers, 0, sizeof(*arrDividers) * (size_t)(iPaneCount - 1));
	iOldCount = pSplitLayout->iPaneCount;
	iCopyCount = (iOldCount < iPaneCount) ? iOldCount : iPaneCount;
	for ( i = 0; i < iCopyCount; i++ ) {
		arrPanes[i] = pSplitLayout->arrPanes[i];
	}
	for ( i = iCopyCount; i < iPaneCount; i++ ) {
		__xgeXuiSplitLayoutInitPaneDefaults(&arrPanes[i]);
	}
	iCopyCount = ((iOldCount - 1) < (iPaneCount - 1)) ? (iOldCount - 1) : (iPaneCount - 1);
	for ( i = 0; i < iCopyCount; i++ ) {
		arrDividers[i] = pSplitLayout->arrDividers[i];
	}
	if ( iPaneCount < iOldCount ) {
		for ( i = iPaneCount; i < iOldCount; i++ ) {
			xgeXuiWidgetFree(pSplitLayout->arrPanes[i].pWidget);
		}
		for ( i = iPaneCount - 1; i < iOldCount - 1; i++ ) {
			xgeXuiWidgetFree(pSplitLayout->arrDividers[i].pWidget);
		}
	}
	__xgeXuiSplitLayoutFreeArrays(pSplitLayout);
	pSplitLayout->arrPanes = arrPanes;
	pSplitLayout->arrDividers = arrDividers;
	return XGE_OK;
}

int xgeXuiSplitLayoutInit(xge_xui_split_layout pSplitLayout, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pSplitLayout == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pSplitLayout, 0, sizeof(*pSplitLayout));
	xgeXuiWidgetSetRole(pWidget, XGE_XUI_WIDGET_ROLE_CONTAINER);
	pSplitLayout->pContext = pContext;
	pSplitLayout->pWidget = pWidget;
	pSplitLayout->iOrientation = XGE_XUI_ORIENTATION_VERTICAL;
	pSplitLayout->fDividerSize = 8.0f;
	pSplitLayout->fDividerVisualSize = 4.0f;
	pSplitLayout->fDividerHitSize = 12.0f;
	pSplitLayout->bShadowDrag = 1;
	pSplitLayout->iActiveDivider = -1;
	pSplitLayout->iHoverDivider = -1;
	pSplitLayout->iDividerColor = XGE_COLOR_RGBA(149, 176, 206, 220);
	pSplitLayout->iDividerHoverColor = XGE_COLOR_RGBA(93, 154, 220, 235);
	pSplitLayout->iDividerActiveColor = XGE_COLOR_RGBA(46, 124, 214, 245);
	pSplitLayout->iShadowColor = XGE_COLOR_RGBA(46, 124, 214, 110);
	pWidget->tStyle.iLayout = XGE_XUI_LAYOUT_ABSOLUTE;
	pWidget->procLayout = xgeXuiSplitLayoutLayoutProc;
	pWidget->pLayoutUser = pSplitLayout;
	pWidget->pUser = pSplitLayout;
	__xgeXuiSplitLayoutEnsureShadow(pSplitLayout);
	xgeXuiSplitLayoutSetPaneCount(pSplitLayout, 2);
	return XGE_OK;
}

void xgeXuiSplitLayoutUnit(xge_xui_split_layout pSplitLayout)
{
	int i;

	if ( pSplitLayout == NULL ) {
		return;
	}
	if ( pSplitLayout->pShadowWidget != NULL ) {
		xgeXuiWidgetFree(pSplitLayout->pShadowWidget);
		pSplitLayout->pShadowWidget = NULL;
	}
	for ( i = 0; i < pSplitLayout->iPaneCount - 1; i++ ) {
		xgeXuiWidgetFree(pSplitLayout->arrDividers[i].pWidget);
	}
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		xgeXuiWidgetFree(pSplitLayout->arrPanes[i].pWidget);
	}
	__xgeXuiSplitLayoutFreeArrays(pSplitLayout);
	if ( pSplitLayout->pWidget != NULL ) {
		pSplitLayout->pWidget->procLayout = NULL;
		pSplitLayout->pWidget->pLayoutUser = NULL;
		if ( pSplitLayout->pWidget->pUser == pSplitLayout ) {
			pSplitLayout->pWidget->pUser = NULL;
		}
	}
	memset(pSplitLayout, 0, sizeof(*pSplitLayout));
}

void xgeXuiSplitLayoutSetOrientation(xge_xui_split_layout pSplitLayout, int iOrientation)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->iOrientation = (iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL) ? XGE_XUI_ORIENTATION_HORIZONTAL : XGE_XUI_ORIENTATION_VERTICAL;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetPaneCount(xge_xui_split_layout pSplitLayout, int iCount)
{
	int i;

	if ( pSplitLayout == NULL ) {
		return;
	}
	if ( iCount < 2 ) {
		iCount = 2;
	}
	if ( iCount > XGE_XUI_SPLIT_LAYOUT_MAX_PANES ) {
		iCount = XGE_XUI_SPLIT_LAYOUT_MAX_PANES;
	}
	if ( pSplitLayout->iPaneCount == iCount ) {
		return;
	}
	if ( __xgeXuiSplitLayoutResizeArrays(pSplitLayout, iCount) != XGE_OK ) {
		return;
	}
	pSplitLayout->iPaneCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		if ( pSplitLayout->arrPanes[i].pWidget == NULL ) {
			pSplitLayout->arrPanes[i].pWidget = xgeXuiWidgetCreate();
			if ( pSplitLayout->arrPanes[i].pWidget != NULL ) {
				xgeXuiWidgetSetLayout(pSplitLayout->arrPanes[i].pWidget, XGE_XUI_LAYOUT_COLUMN);
				xgeXuiWidgetSetGap(pSplitLayout->arrPanes[i].pWidget, 8.0f);
				xgeXuiWidgetSetPaddingPx(pSplitLayout->arrPanes[i].pWidget, 12.0f, 12.0f, 12.0f, 12.0f);
				(void)xgeXuiWidgetAddInternal(pSplitLayout->pWidget, pSplitLayout->arrPanes[i].pWidget);
			}
		}
	}
	for ( i = 0; i < iCount - 1; i++ ) {
		if ( pSplitLayout->arrDividers[i].pWidget == NULL ) {
			pSplitLayout->arrDividers[i].pWidget = xgeXuiWidgetCreate();
			if ( pSplitLayout->arrDividers[i].pWidget != NULL ) {
				xgeXuiWidgetSetEvent(pSplitLayout->arrDividers[i].pWidget, xgeXuiSplitLayoutDividerEventProc, NULL);
				pSplitLayout->arrDividers[i].pWidget->procPaint = xgeXuiSplitLayoutDividerPaintProc;
				pSplitLayout->arrDividers[i].pWidget->pUser = pSplitLayout;
				(void)xgeXuiWidgetAddInternal(pSplitLayout->pWidget, pSplitLayout->arrDividers[i].pWidget);
				xgeXuiWidgetSetFocusable(pSplitLayout->arrDividers[i].pWidget, 1);
				xgeXuiWidgetSetClip(pSplitLayout->arrDividers[i].pWidget, 0);
				xgeXuiWidgetSetZ(pSplitLayout->arrDividers[i].pWidget, 10);
			}
		}
		xgeXuiWidgetSetId(pSplitLayout->arrDividers[i].pWidget, i);
	}
	__xgeXuiSplitLayoutNormalizeWeights(pSplitLayout);
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

int xgeXuiSplitLayoutGetPaneCount(xge_xui_split_layout pSplitLayout)
{
	return (pSplitLayout != NULL) ? pSplitLayout->iPaneCount : 0;
}

xge_xui_widget xgeXuiSplitLayoutGetPaneWidget(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return NULL;
	}
	return pSplitLayout->arrPanes[iIndex].pWidget;
}

void xgeXuiSplitLayoutSetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex, float fWeight)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPanes[iIndex].fWeight = (fWeight > 0.0f) ? fWeight : 0.0f;
	if ( pSplitLayout->arrPanes[iIndex].iMode != XGE_XUI_SPLIT_PANE_FIXED ) {
		__xgeXuiSplitLayoutNormalizeWeights(pSplitLayout);
	}
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

float xgeXuiSplitLayoutGetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return 0.0f;
	}
	return pSplitLayout->arrPanes[iIndex].fWeight;
}

void xgeXuiSplitLayoutSetPaneMode(xge_xui_split_layout pSplitLayout, int iIndex, int iMode)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPanes[iIndex].iMode = (iMode == XGE_XUI_SPLIT_PANE_FIXED) ? XGE_XUI_SPLIT_PANE_FIXED : XGE_XUI_SPLIT_PANE_GROW;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

int xgeXuiSplitLayoutGetPaneMode(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return XGE_XUI_SPLIT_PANE_GROW;
	}
	return pSplitLayout->arrPanes[iIndex].iMode;
}

void xgeXuiSplitLayoutSetPaneFixedSize(xge_xui_split_layout pSplitLayout, int iIndex, float fSize)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPanes[iIndex].fFixedSize = (fSize > 0.0f) ? fSize : 0.0f;
	pSplitLayout->arrPanes[iIndex].iMode = XGE_XUI_SPLIT_PANE_FIXED;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

float xgeXuiSplitLayoutGetPaneFixedSize(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return 0.0f;
	}
	return pSplitLayout->arrPanes[iIndex].fFixedSize;
}

void xgeXuiSplitLayoutSetPaneMinSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMinSize)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPanes[iIndex].fMinSize = (fMinSize > 0.0f) ? fMinSize : 0.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetPaneMaxSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMaxSize)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPanes[iIndex].fMaxSize = (fMaxSize > 0.0f) ? fMaxSize : 0.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

float xgeXuiSplitLayoutGetPaneSize(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return 0.0f;
	}
	return pSplitLayout->arrPanes[iIndex].fResolvedSize;
}

void xgeXuiSplitLayoutSetDividerSize(xge_xui_split_layout pSplitLayout, float fSize)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->fDividerSize = (fSize > 1.0f) ? fSize : 1.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetDividerVisualSize(xge_xui_split_layout pSplitLayout, float fSize)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->fDividerVisualSize = (fSize > 1.0f) ? fSize : 1.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetDividerHitSize(xge_xui_split_layout pSplitLayout, float fSize)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->fDividerHitSize = (fSize > 1.0f) ? fSize : 1.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetShadowDrag(xge_xui_split_layout pSplitLayout, int bEnabled)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->bShadowDrag = (bEnabled != 0);
	if ( pSplitLayout->bShadowDrag == 0 ) {
		__xgeXuiSplitLayoutHideShadow(pSplitLayout);
	}
}

void xgeXuiSplitLayoutSetColors(xge_xui_split_layout pSplitLayout, uint32_t iDivider, uint32_t iHover, uint32_t iActive, uint32_t iShadow)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->iDividerColor = iDivider;
	pSplitLayout->iDividerHoverColor = iHover;
	pSplitLayout->iDividerActiveColor = iActive;
	pSplitLayout->iShadowColor = iShadow;
	if ( pSplitLayout->pShadowWidget != NULL ) {
		xgeXuiWidgetSetBackground(pSplitLayout->pShadowWidget, iShadow);
	}
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetChange(xge_xui_split_layout pSplitLayout, xge_xui_split_layout_change_proc procChange, void* pUser)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->procChange = procChange;
	pSplitLayout->pChangeUser = pUser;
}

void xgeXuiSplitLayoutLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_split_layout pSplitLayout;
	xge_rect_t tPaneRect;
	xge_rect_t tLayoutRect;
	xge_rect_t tVisualRect;
	xge_rect_t tHitRect;
	float fAxis;
	float fCrossSize;
	float fVisual;
	float fHit;
	int i;

	pSplitLayout = (xge_xui_split_layout)pUser;
	if ( (pWidget == NULL) || (pSplitLayout == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
		return;
	}
	__xgeXuiSplitLayoutResolveSizes(pSplitLayout, __xgeXuiSplitLayoutAxisSize(pSplitLayout));
	fAxis = __xgeXuiSplitLayoutAxisStart(pSplitLayout);
	fCrossSize = __xgeXuiSplitLayoutCrossSize(pSplitLayout);
	fVisual = (pSplitLayout->fDividerVisualSize > 0.0f) ? pSplitLayout->fDividerVisualSize : pSplitLayout->fDividerSize;
	fHit = (pSplitLayout->fDividerHitSize > 0.0f) ? pSplitLayout->fDividerHitSize : pSplitLayout->fDividerSize;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		memset(&tPaneRect, 0, sizeof(tPaneRect));
		if ( pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL ) {
			tPaneRect.fX = pWidget->tContentRect.fX;
			tPaneRect.fY = fAxis;
			tPaneRect.fW = fCrossSize;
			tPaneRect.fH = pSplitLayout->arrPanes[i].fResolvedSize;
		} else {
			tPaneRect.fX = fAxis;
			tPaneRect.fY = pWidget->tContentRect.fY;
			tPaneRect.fW = pSplitLayout->arrPanes[i].fResolvedSize;
			tPaneRect.fH = fCrossSize;
		}
		__xgeXuiSplitLayoutSetLocalRect(pSplitLayout->arrPanes[i].pWidget, tPaneRect);
		fAxis += pSplitLayout->arrPanes[i].fResolvedSize;
		if ( i < (pSplitLayout->iPaneCount - 1) ) {
			memset(&tLayoutRect, 0, sizeof(tLayoutRect));
			memset(&tVisualRect, 0, sizeof(tVisualRect));
			memset(&tHitRect, 0, sizeof(tHitRect));
			if ( pSplitLayout->iOrientation == XGE_XUI_ORIENTATION_HORIZONTAL ) {
				tLayoutRect.fX = pWidget->tContentRect.fX;
				tLayoutRect.fY = fAxis;
				tLayoutRect.fW = fCrossSize;
				tLayoutRect.fH = pSplitLayout->fDividerSize;
				tVisualRect.fX = tLayoutRect.fX;
				tVisualRect.fY = tLayoutRect.fY + (tLayoutRect.fH - fVisual) * 0.5f;
				tVisualRect.fW = tLayoutRect.fW;
				tVisualRect.fH = fVisual;
				tHitRect.fX = tLayoutRect.fX;
				tHitRect.fY = tLayoutRect.fY + (tLayoutRect.fH - fHit) * 0.5f;
				tHitRect.fW = tLayoutRect.fW;
				tHitRect.fH = fHit;
			} else {
				tLayoutRect.fX = fAxis;
				tLayoutRect.fY = pWidget->tContentRect.fY;
				tLayoutRect.fW = pSplitLayout->fDividerSize;
				tLayoutRect.fH = fCrossSize;
				tVisualRect.fX = tLayoutRect.fX + (tLayoutRect.fW - fVisual) * 0.5f;
				tVisualRect.fY = tLayoutRect.fY;
				tVisualRect.fW = fVisual;
				tVisualRect.fH = tLayoutRect.fH;
				tHitRect.fX = tLayoutRect.fX + (tLayoutRect.fW - fHit) * 0.5f;
				tHitRect.fY = tLayoutRect.fY;
				tHitRect.fW = fHit;
				tHitRect.fH = tLayoutRect.fH;
			}
			pSplitLayout->arrDividers[i].tLayoutRect = tLayoutRect;
			pSplitLayout->arrDividers[i].tVisualRect = tVisualRect;
			pSplitLayout->arrDividers[i].tHitRect = tHitRect;
			__xgeXuiSplitLayoutSetLocalRect(pSplitLayout->arrDividers[i].pWidget, tHitRect);
			fAxis += pSplitLayout->fDividerSize;
		}
	}
	if ( pSplitLayout->iActiveDivider >= 0 && pSplitLayout->bShadowDrag != 0 ) {
		__xgeXuiSplitLayoutUpdateShadow(pSplitLayout);
	}
}

int xgeXuiSplitLayoutDividerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_split_layout pSplitLayout;
	int iIndex;
	int bInside;

	pSplitLayout = (xge_xui_split_layout)pUser;
	iIndex = __xgeXuiSplitLayoutDividerIndex(pWidget);
	if ( (pSplitLayout == NULL) || (pWidget == NULL) || (pEvent == NULL) || (iIndex < 0) || (iIndex >= (pSplitLayout->iPaneCount - 1)) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	bInside = __xgeXuiRectContains(pSplitLayout->arrDividers[iIndex].tHitRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_POINTER_ENTER:
			pSplitLayout->iHoverDivider = iIndex;
			xgeXuiWidgetMarkPaint(pWidget);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( pSplitLayout->iHoverDivider == iIndex ) {
				pSplitLayout->iHoverDivider = -1;
				xgeXuiWidgetMarkPaint(pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( bInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pSplitLayout->iActiveDivider = iIndex;
			pSplitLayout->fDragStartMouse = __xgeXuiSplitLayoutEventAxis(pSplitLayout, pEvent);
			pSplitLayout->fDragCurrentMouse = pSplitLayout->fDragStartMouse;
			pSplitLayout->fDragStartBefore = pSplitLayout->arrPanes[iIndex].fResolvedSize;
			pSplitLayout->fDragStartAfter = pSplitLayout->arrPanes[iIndex + 1].fResolvedSize;
			xgeXuiSetFocus(pSplitLayout->pContext, pWidget);
			xgeXuiSetPointerCapture(pSplitLayout->pContext, pEvent->iPointerId, pWidget);
			if ( pSplitLayout->bShadowDrag != 0 ) {
				__xgeXuiSplitLayoutUpdateShadow(pSplitLayout);
			}
			xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pSplitLayout->iActiveDivider != iIndex || xgeXuiGetPointerCapture(pSplitLayout->pContext, pEvent->iPointerId) != pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pSplitLayout->fDragCurrentMouse = __xgeXuiSplitLayoutEventAxis(pSplitLayout, pEvent);
			if ( pSplitLayout->bShadowDrag != 0 ) {
				__xgeXuiSplitLayoutUpdateShadow(pSplitLayout);
			} else {
				__xgeXuiSplitLayoutCommitDrag(pSplitLayout);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			if ( pSplitLayout->iActiveDivider != iIndex ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			pSplitLayout->fDragCurrentMouse = __xgeXuiSplitLayoutEventAxis(pSplitLayout, pEvent);
			__xgeXuiSplitLayoutCommitDrag(pSplitLayout);
			__xgeXuiSplitLayoutHideShadow(pSplitLayout);
			pSplitLayout->iActiveDivider = -1;
			xgeXuiSetPointerCapture(pSplitLayout->pContext, pEvent->iPointerId, NULL);
			xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			if ( pSplitLayout->iActiveDivider == iIndex ) {
				pSplitLayout->iActiveDivider = -1;
				__xgeXuiSplitLayoutHideShadow(pSplitLayout);
				xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
			}
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

void xgeXuiSplitLayoutDividerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_split_layout pSplitLayout;
	xge_rect_t tRect;
	int iIndex;
	uint32_t iColor;

	pSplitLayout = (xge_xui_split_layout)pUser;
	iIndex = __xgeXuiSplitLayoutDividerIndex(pWidget);
	if ( (pSplitLayout == NULL) || (pWidget == NULL) || (iIndex < 0) || (iIndex >= (pSplitLayout->iPaneCount - 1)) ) {
		return;
	}
	iColor = pSplitLayout->iDividerColor;
	if ( pSplitLayout->iActiveDivider == iIndex ) {
		iColor = pSplitLayout->iDividerActiveColor;
	} else if ( pSplitLayout->iHoverDivider == iIndex ) {
		iColor = pSplitLayout->iDividerHoverColor;
	}
	tRect = pSplitLayout->arrDividers[iIndex].tVisualRect;
	__xgeXuiHostDrawRect(tRect, iColor);
}
