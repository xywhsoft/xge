enum {
	XGE_XUI_SPLIT_LAYOUT_MAX_PANES = 16
};

static float __xgeXuiSplitLayoutAxisSize(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		return pSplitLayout->pWidget->tContentRect.fH;
	}
	return pSplitLayout->pWidget->tContentRect.fW;
}

#if XGE_HAS_DEBUGMODE
static void __xgeXuiSplitLayoutDebugSizes(xge_xui_split_layout pSplitLayout, const char* sTag)
{
	int i;

	if ( (pSplitLayout == NULL) || (sTag == NULL) ) {
		return;
	}
	printf("[xui_split_layout] %s orientation=%d panes=%d axis=%.2f divider=%.2f active=%d hover=%d\n",
		sTag,
		pSplitLayout->iOrientation,
		pSplitLayout->iPaneCount,
		pSplitLayout->fResolvedAxis,
		pSplitLayout->fDividerSize,
		pSplitLayout->iActiveDivider,
		pSplitLayout->iHoverDivider);
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		printf("[xui_split_layout]   pane[%d] weight=%.3f min=%.2f size=%.2f rect=(%.2f,%.2f,%.2f,%.2f)\n",
			i,
			pSplitLayout->arrPaneWeights != NULL ? pSplitLayout->arrPaneWeights[i] : 0.0f,
			pSplitLayout->arrPaneMinSizes != NULL ? pSplitLayout->arrPaneMinSizes[i] : 0.0f,
			pSplitLayout->arrPaneResolvedSizes != NULL ? pSplitLayout->arrPaneResolvedSizes[i] : 0.0f,
			(pSplitLayout->arrPaneWidgets != NULL && pSplitLayout->arrPaneWidgets[i] != NULL) ? pSplitLayout->arrPaneWidgets[i]->tRect.fX : 0.0f,
			(pSplitLayout->arrPaneWidgets != NULL && pSplitLayout->arrPaneWidgets[i] != NULL) ? pSplitLayout->arrPaneWidgets[i]->tRect.fY : 0.0f,
			(pSplitLayout->arrPaneWidgets != NULL && pSplitLayout->arrPaneWidgets[i] != NULL) ? pSplitLayout->arrPaneWidgets[i]->tRect.fW : 0.0f,
			(pSplitLayout->arrPaneWidgets != NULL && pSplitLayout->arrPaneWidgets[i] != NULL) ? pSplitLayout->arrPaneWidgets[i]->tRect.fH : 0.0f);
	}
}
#endif

static float __xgeXuiSplitLayoutCrossSize(xge_xui_split_layout pSplitLayout)
{
	if ( (pSplitLayout == NULL) || (pSplitLayout->pWidget == NULL) ) {
		return 0.0f;
	}
	if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		return pSplitLayout->pWidget->tContentRect.fW;
	}
	return pSplitLayout->pWidget->tContentRect.fH;
}

static float __xgeXuiSplitLayoutEventAxis(xge_xui_split_layout pSplitLayout, const xge_event_t* pEvent)
{
	if ( (pSplitLayout == NULL) || (pEvent == NULL) ) {
		return 0.0f;
	}
	if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		return pEvent->fY;
	}
	return pEvent->fX;
}

static int __xgeXuiSplitLayoutDividerIndex(xge_xui_widget pWidget)
{
	if ( pWidget == NULL ) {
		return -1;
	}
	return pWidget->iId;
}

static void __xgeXuiSplitLayoutNormalizeWeights(xge_xui_split_layout pSplitLayout)
{
	float fTotal;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->arrPaneWeights == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
		return;
	}
	fTotal = 0.0f;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		if ( pSplitLayout->arrPaneWeights[i] < 0.0f ) {
			pSplitLayout->arrPaneWeights[i] = 0.0f;
		}
		fTotal += pSplitLayout->arrPaneWeights[i];
	}
	if ( fTotal <= 0.0f ) {
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			pSplitLayout->arrPaneWeights[i] = 1.0f;
		}
	}
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

static void __xgeXuiSplitLayoutResolveSizes(xge_xui_split_layout pSplitLayout, float fAvailable)
{
	float fDividerTotal;
	float fWeightTotal;
	float fWeightRemaining;
	float fAvailableRemaining;
	float fSumMin;
	float fProposed;
	char arrLocked[XGE_XUI_SPLIT_LAYOUT_MAX_PANES];
	int bChanged;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->arrPaneResolvedSizes == NULL) || (pSplitLayout->arrPaneWeights == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
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
	fSumMin = 0.0f;
	fWeightTotal = 0.0f;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		arrLocked[i] = 0;
		if ( pSplitLayout->arrPaneMinSizes[i] < 0.0f ) {
			pSplitLayout->arrPaneMinSizes[i] = 0.0f;
		}
		fSumMin += pSplitLayout->arrPaneMinSizes[i];
		fWeightTotal += (pSplitLayout->arrPaneWeights[i] > 0.0f) ? pSplitLayout->arrPaneWeights[i] : 0.0f;
	}
	if ( fWeightTotal <= 0.0f ) {
		fWeightTotal = (float)pSplitLayout->iPaneCount;
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			pSplitLayout->arrPaneWeights[i] = 1.0f;
		}
	}
	if ( fSumMin >= fAvailable && fSumMin > 0.0f ) {
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			pSplitLayout->arrPaneResolvedSizes[i] = fAvailable * (pSplitLayout->arrPaneMinSizes[i] / fSumMin);
		}
		return;
	}
	fAvailableRemaining = fAvailable;
	fWeightRemaining = fWeightTotal;
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		pSplitLayout->arrPaneResolvedSizes[i] = 0.0f;
	}
	do {
		bChanged = 0;
		for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
			if ( arrLocked[i] != 0 ) {
				continue;
			}
			if ( fWeightRemaining > 0.0f ) {
				fProposed = fAvailableRemaining * (pSplitLayout->arrPaneWeights[i] / fWeightRemaining);
			} else {
				fProposed = fAvailableRemaining;
			}
			if ( fProposed < pSplitLayout->arrPaneMinSizes[i] ) {
				pSplitLayout->arrPaneResolvedSizes[i] = pSplitLayout->arrPaneMinSizes[i];
				fAvailableRemaining -= pSplitLayout->arrPaneResolvedSizes[i];
				fWeightRemaining -= pSplitLayout->arrPaneWeights[i];
				arrLocked[i] = 1;
				bChanged = 1;
			}
		}
	} while ( bChanged && (fAvailableRemaining > 0.0f) );
	if ( fAvailableRemaining < 0.0f ) {
		fAvailableRemaining = 0.0f;
	}
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		if ( arrLocked[i] != 0 ) {
			continue;
		}
		if ( fWeightRemaining > 0.0f ) {
			pSplitLayout->arrPaneResolvedSizes[i] = fAvailableRemaining * (pSplitLayout->arrPaneWeights[i] / fWeightRemaining);
		} else {
			pSplitLayout->arrPaneResolvedSizes[i] = 0.0f;
		}
	}
}

static float __xgeXuiSplitLayoutClampDragAxis(xge_xui_split_layout pSplitLayout, int iDivider, float fAxis)
{
	float fAxisStart;
	float fCurrentStart;
	float fMinAxis;
	float fMaxAxis;
	int i;

	if ( (pSplitLayout == NULL) || (iDivider < 0) || (iDivider >= (pSplitLayout->iPaneCount - 1)) ) {
		return fAxis;
	}
	fAxisStart = (pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? pSplitLayout->pWidget->tContentRect.fY : pSplitLayout->pWidget->tContentRect.fX;
	fCurrentStart = fAxisStart;
	for ( i = 0; i < iDivider; i++ ) {
		fCurrentStart += pSplitLayout->arrPaneResolvedSizes[i] + pSplitLayout->fDividerSize;
	}
	fMinAxis = fCurrentStart + pSplitLayout->arrPaneMinSizes[iDivider];
	fMaxAxis = fCurrentStart + pSplitLayout->arrPaneResolvedSizes[iDivider] + pSplitLayout->fDividerSize + pSplitLayout->arrPaneResolvedSizes[iDivider + 1] - pSplitLayout->arrPaneMinSizes[iDivider + 1];
	return __xgeXuiClampFloat(fAxis, fMinAxis, fMaxAxis);
}

static void __xgeXuiSplitLayoutUpdateShadow(xge_xui_split_layout pSplitLayout)
{
	xge_rect_t tRect;
	float fAxis;
	float fCross;
	float fCrossSize;

	if ( (pSplitLayout == NULL) || (pSplitLayout->pShadowWidget == NULL) || (pSplitLayout->iActiveDivider < 0) ) {
		return;
	}
	fAxis = __xgeXuiSplitLayoutClampDragAxis(pSplitLayout, pSplitLayout->iActiveDivider, pSplitLayout->fDragCurrentMouse);
	fCross = (pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? pSplitLayout->pWidget->tContentRect.fX : pSplitLayout->pWidget->tContentRect.fY;
	fCrossSize = __xgeXuiSplitLayoutCrossSize(pSplitLayout);
	memset(&tRect, 0, sizeof(tRect));
	if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
		tRect.fX = fCross;
		tRect.fY = fAxis;
		tRect.fW = fCrossSize;
		tRect.fH = pSplitLayout->fDividerSize;
	} else {
		tRect.fX = fAxis;
		tRect.fY = fCross;
		tRect.fW = pSplitLayout->fDividerSize;
		tRect.fH = fCrossSize;
	}
	xgeXuiWidgetSetRect(pSplitLayout->pShadowWidget, tRect);
	xgeXuiWidgetSetVisible(pSplitLayout->pShadowWidget, 1);
}

static void __xgeXuiSplitLayoutCommitDrag(xge_xui_split_layout pSplitLayout)
{
	float fAxis;
	float fCurrentStart;
	float fPairTotal;
	float fNewBefore;
	float fNewAfter;
	float fPairWeight;
	int i;

	if ( (pSplitLayout == NULL) || (pSplitLayout->iActiveDivider < 0) ) {
		return;
	}
	fAxis = __xgeXuiSplitLayoutClampDragAxis(pSplitLayout, pSplitLayout->iActiveDivider, pSplitLayout->fDragCurrentMouse);
	fCurrentStart = (pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? pSplitLayout->pWidget->tContentRect.fY : pSplitLayout->pWidget->tContentRect.fX;
	for ( i = 0; i < pSplitLayout->iActiveDivider; i++ ) {
		fCurrentStart += pSplitLayout->arrPaneResolvedSizes[i] + pSplitLayout->fDividerSize;
	}
	fNewBefore = fAxis - fCurrentStart;
	fPairTotal = pSplitLayout->arrPaneResolvedSizes[pSplitLayout->iActiveDivider] + pSplitLayout->arrPaneResolvedSizes[pSplitLayout->iActiveDivider + 1];
	if ( fNewBefore < pSplitLayout->arrPaneMinSizes[pSplitLayout->iActiveDivider] ) {
		fNewBefore = pSplitLayout->arrPaneMinSizes[pSplitLayout->iActiveDivider];
	}
	if ( fNewBefore > (fPairTotal - pSplitLayout->arrPaneMinSizes[pSplitLayout->iActiveDivider + 1]) ) {
		fNewBefore = fPairTotal - pSplitLayout->arrPaneMinSizes[pSplitLayout->iActiveDivider + 1];
	}
	if ( fNewBefore < 0.0f ) {
		fNewBefore = 0.0f;
	}
	fNewAfter = fPairTotal - fNewBefore;
	if ( fNewAfter < 0.0f ) {
		fNewAfter = 0.0f;
	}
	fPairWeight = pSplitLayout->arrPaneWeights[pSplitLayout->iActiveDivider] + pSplitLayout->arrPaneWeights[pSplitLayout->iActiveDivider + 1];
	if ( fPairWeight <= 0.0f ) {
		fPairWeight = 1.0f;
	}
	if ( fPairTotal > 0.0f ) {
		pSplitLayout->arrPaneWeights[pSplitLayout->iActiveDivider] = fPairWeight * (fNewBefore / fPairTotal);
		pSplitLayout->arrPaneWeights[pSplitLayout->iActiveDivider + 1] = fPairWeight * (fNewAfter / fPairTotal);
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
	pSplitLayout->arrPaneWidgets = (xge_xui_widget*)xrtRealloc(pSplitLayout->arrPaneWidgets, 0);
	pSplitLayout->arrDividerWidgets = (xge_xui_widget*)xrtRealloc(pSplitLayout->arrDividerWidgets, 0);
	pSplitLayout->arrPaneWeights = (float*)xrtRealloc(pSplitLayout->arrPaneWeights, 0);
	pSplitLayout->arrPaneResolvedSizes = (float*)xrtRealloc(pSplitLayout->arrPaneResolvedSizes, 0);
	pSplitLayout->arrPaneMinSizes = (float*)xrtRealloc(pSplitLayout->arrPaneMinSizes, 0);
}

static int __xgeXuiSplitLayoutResizeArrays(xge_xui_split_layout pSplitLayout, int iPaneCount)
{
	xge_xui_widget* arrPaneWidgets;
	xge_xui_widget* arrDividerWidgets;
	float* arrPaneWeights;
	float* arrPaneResolvedSizes;
	float* arrPaneMinSizes;

	if ( (pSplitLayout == NULL) || (iPaneCount < 2) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	arrPaneWidgets = (xge_xui_widget*)xrtRealloc(pSplitLayout->arrPaneWidgets, sizeof(*arrPaneWidgets) * (size_t)iPaneCount);
	arrDividerWidgets = (xge_xui_widget*)xrtRealloc(pSplitLayout->arrDividerWidgets, sizeof(*arrDividerWidgets) * (size_t)(iPaneCount - 1));
	arrPaneWeights = (float*)xrtRealloc(pSplitLayout->arrPaneWeights, sizeof(*arrPaneWeights) * (size_t)iPaneCount);
	arrPaneResolvedSizes = (float*)xrtRealloc(pSplitLayout->arrPaneResolvedSizes, sizeof(*arrPaneResolvedSizes) * (size_t)iPaneCount);
	arrPaneMinSizes = (float*)xrtRealloc(pSplitLayout->arrPaneMinSizes, sizeof(*arrPaneMinSizes) * (size_t)iPaneCount);
	if ( (arrPaneWidgets == NULL) || (arrDividerWidgets == NULL) || (arrPaneWeights == NULL) || (arrPaneResolvedSizes == NULL) || (arrPaneMinSizes == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pSplitLayout->arrPaneWidgets = arrPaneWidgets;
	pSplitLayout->arrDividerWidgets = arrDividerWidgets;
	pSplitLayout->arrPaneWeights = arrPaneWeights;
	pSplitLayout->arrPaneResolvedSizes = arrPaneResolvedSizes;
	pSplitLayout->arrPaneMinSizes = arrPaneMinSizes;
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
	pSplitLayout->iOrientation = XGE_XUI_SEPARATOR_VERTICAL;
	pSplitLayout->fDividerSize = 3.0f;
	pSplitLayout->bShadowDrag = 1;
	pSplitLayout->iActiveDivider = -1;
	pSplitLayout->iHoverDivider = -1;
	pSplitLayout->iDividerColor = XGE_COLOR_RGBA(206, 216, 228, 220);
	pSplitLayout->iDividerHoverColor = XGE_COLOR_RGBA(184, 198, 214, 235);
	pSplitLayout->iDividerActiveColor = XGE_COLOR_RGBA(150, 170, 190, 245);
	pSplitLayout->iShadowColor = XGE_COLOR_RGBA(116, 184, 255, 120);
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
		xgeXuiWidgetFree(pSplitLayout->arrDividerWidgets[i]);
	}
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		xgeXuiWidgetFree(pSplitLayout->arrPaneWidgets[i]);
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
	pSplitLayout->iOrientation = (iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? XGE_XUI_SEPARATOR_HORIZONTAL : XGE_XUI_SEPARATOR_VERTICAL;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

void xgeXuiSplitLayoutSetPaneCount(xge_xui_split_layout pSplitLayout, int iCount)
{
	int iOldCount;
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
	iOldCount = pSplitLayout->iPaneCount;
	if ( iCount < iOldCount ) {
		for ( i = iCount; i < iOldCount; i++ ) {
			xgeXuiWidgetFree(pSplitLayout->arrPaneWidgets[i]);
		}
		for ( i = iCount - 1; i < iOldCount - 1; i++ ) {
			xgeXuiWidgetFree(pSplitLayout->arrDividerWidgets[i]);
		}
	}
	if ( __xgeXuiSplitLayoutResizeArrays(pSplitLayout, iCount) != XGE_OK ) {
		return;
	}
	for ( i = iOldCount; i < iCount; i++ ) {
		pSplitLayout->arrPaneWidgets[i] = NULL;
		pSplitLayout->arrPaneWeights[i] = 1.0f;
		pSplitLayout->arrPaneResolvedSizes[i] = 0.0f;
		pSplitLayout->arrPaneMinSizes[i] = 64.0f;
	}
	for ( i = (iOldCount > 0) ? (iOldCount - 1) : 0; i < (iCount - 1); i++ ) {
		pSplitLayout->arrDividerWidgets[i] = NULL;
	}
	for ( i = iOldCount; i < iCount; i++ ) {
		pSplitLayout->arrPaneWidgets[i] = xgeXuiWidgetCreate();
		if ( pSplitLayout->arrPaneWidgets[i] != NULL ) {
			xgeXuiWidgetSetLayout(pSplitLayout->arrPaneWidgets[i], XGE_XUI_LAYOUT_COLUMN);
			xgeXuiWidgetSetGap(pSplitLayout->arrPaneWidgets[i], 8.0f);
			xgeXuiWidgetSetPaddingPx(pSplitLayout->arrPaneWidgets[i], 12.0f, 12.0f, 12.0f, 12.0f);
			(void)xgeXuiWidgetAddInternal(pSplitLayout->pWidget, pSplitLayout->arrPaneWidgets[i]);
		}
		pSplitLayout->arrPaneWeights[i] = 1.0f;
		pSplitLayout->arrPaneResolvedSizes[i] = 0.0f;
		pSplitLayout->arrPaneMinSizes[i] = 64.0f;
	}
	for ( i = (iOldCount > 0) ? (iOldCount - 1) : 0; i < (iCount - 1); i++ ) {
		if ( pSplitLayout->arrDividerWidgets[i] == NULL ) {
			pSplitLayout->arrDividerWidgets[i] = xgeXuiWidgetCreate();
			if ( pSplitLayout->arrDividerWidgets[i] != NULL ) {
				xgeXuiWidgetSetEvent(pSplitLayout->arrDividerWidgets[i], xgeXuiSplitLayoutDividerEventProc, NULL);
				pSplitLayout->arrDividerWidgets[i]->procPaint = xgeXuiSplitLayoutDividerPaintProc;
				pSplitLayout->arrDividerWidgets[i]->pUser = pSplitLayout;
				(void)xgeXuiWidgetAddInternal(pSplitLayout->pWidget, pSplitLayout->arrDividerWidgets[i]);
				xgeXuiWidgetSetId(pSplitLayout->arrDividerWidgets[i], i);
				xgeXuiWidgetSetFocusable(pSplitLayout->arrDividerWidgets[i], 1);
				xgeXuiWidgetSetClip(pSplitLayout->arrDividerWidgets[i], 0);
				xgeXuiWidgetSetZ(pSplitLayout->arrDividerWidgets[i], 10);
			}
		} else {
			xgeXuiWidgetSetId(pSplitLayout->arrDividerWidgets[i], i);
		}
	}
	pSplitLayout->iPaneCount = iCount;
	__xgeXuiSplitLayoutNormalizeWeights(pSplitLayout);
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
	xgeXuiWidgetMarkPaint(pSplitLayout->pWidget);
}

int xgeXuiSplitLayoutGetPaneCount(xge_xui_split_layout pSplitLayout)
{
	if ( pSplitLayout == NULL ) {
		return 0;
	}
	return pSplitLayout->iPaneCount;
}

xge_xui_widget xgeXuiSplitLayoutGetPaneWidget(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return NULL;
	}
	return pSplitLayout->arrPaneWidgets[iIndex];
}

void xgeXuiSplitLayoutSetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex, float fWeight)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPaneWeights[iIndex] = (fWeight > 0.0f) ? fWeight : 0.0f;
	__xgeXuiSplitLayoutNormalizeWeights(pSplitLayout);
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

float xgeXuiSplitLayoutGetPaneWeight(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return 0.0f;
	}
	return pSplitLayout->arrPaneWeights[iIndex];
}

void xgeXuiSplitLayoutSetPaneMinSize(xge_xui_split_layout pSplitLayout, int iIndex, float fMinSize)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return;
	}
	pSplitLayout->arrPaneMinSizes[iIndex] = (fMinSize > 0.0f) ? fMinSize : 0.0f;
	xgeXuiWidgetMarkLayout(pSplitLayout->pWidget);
}

float xgeXuiSplitLayoutGetPaneSize(xge_xui_split_layout pSplitLayout, int iIndex)
{
	if ( (pSplitLayout == NULL) || (iIndex < 0) || (iIndex >= pSplitLayout->iPaneCount) ) {
		return 0.0f;
	}
	return pSplitLayout->arrPaneResolvedSizes[iIndex];
}

void xgeXuiSplitLayoutSetDividerSize(xge_xui_split_layout pSplitLayout, float fSize)
{
	if ( pSplitLayout == NULL ) {
		return;
	}
	pSplitLayout->fDividerSize = (fSize > 2.0f) ? fSize : 2.0f;
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

void xgeXuiSplitLayoutLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_split_layout pSplitLayout;
	xge_rect_t tRect;
	float fAxis;
	float fCrossSize;
	int i;

	pSplitLayout = (xge_xui_split_layout)pUser;
	if ( (pWidget == NULL) || (pSplitLayout == NULL) || (pSplitLayout->iPaneCount <= 0) ) {
		return;
	}
	__xgeXuiSplitLayoutResolveSizes(pSplitLayout, __xgeXuiSplitLayoutAxisSize(pSplitLayout));
	fAxis = (pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL) ? pWidget->tContentRect.fY : pWidget->tContentRect.fX;
	fCrossSize = __xgeXuiSplitLayoutCrossSize(pSplitLayout);
	for ( i = 0; i < pSplitLayout->iPaneCount; i++ ) {
		memset(&tRect, 0, sizeof(tRect));
		if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
			tRect.fX = pWidget->tContentRect.fX;
			tRect.fY = fAxis;
			tRect.fW = fCrossSize;
			tRect.fH = pSplitLayout->arrPaneResolvedSizes[i];
		} else {
			tRect.fX = fAxis;
			tRect.fY = pWidget->tContentRect.fY;
			tRect.fW = pSplitLayout->arrPaneResolvedSizes[i];
			tRect.fH = fCrossSize;
		}
		__xgeXuiSplitLayoutSetLocalRect(pSplitLayout->arrPaneWidgets[i], tRect);
		fAxis += pSplitLayout->arrPaneResolvedSizes[i];
		if ( i < (pSplitLayout->iPaneCount - 1) ) {
			memset(&tRect, 0, sizeof(tRect));
			if ( pSplitLayout->iOrientation == XGE_XUI_SEPARATOR_HORIZONTAL ) {
				tRect.fX = pWidget->tContentRect.fX;
				tRect.fY = fAxis;
				tRect.fW = fCrossSize;
				tRect.fH = pSplitLayout->fDividerSize;
			} else {
				tRect.fX = fAxis;
				tRect.fY = pWidget->tContentRect.fY;
				tRect.fW = pSplitLayout->fDividerSize;
				tRect.fH = fCrossSize;
			}
			__xgeXuiSplitLayoutSetLocalRect(pSplitLayout->arrDividerWidgets[i], tRect);
			fAxis += pSplitLayout->fDividerSize;
		}
	}
	if ( pSplitLayout->iActiveDivider >= 0 && pSplitLayout->bShadowDrag != 0 ) {
		__xgeXuiSplitLayoutUpdateShadow(pSplitLayout);
	}
#if XGE_HAS_DEBUGMODE
	__xgeXuiSplitLayoutDebugSizes(pSplitLayout, "layout");
#endif
}

int xgeXuiSplitLayoutDividerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_split_layout pSplitLayout;
	int iIndex;
	int bInside;

	pSplitLayout = (xge_xui_split_layout)pUser;
	iIndex = __xgeXuiSplitLayoutDividerIndex(pWidget);
	if ( (pSplitLayout == NULL) || (pWidget == NULL) || (pEvent == NULL) || (iIndex < 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	bInside = __xgeXuiRectContains(pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_POINTER_ENTER:
			pSplitLayout->iHoverDivider = iIndex;
#if XGE_HAS_DEBUGMODE
			printf("[xui_split_layout] divider-enter index=%d\n", iIndex);
#endif
			xgeXuiWidgetMarkPaint(pWidget);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			if ( pSplitLayout->iHoverDivider == iIndex ) {
#if XGE_HAS_DEBUGMODE
				printf("[xui_split_layout] divider-leave index=%d\n", iIndex);
#endif
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
			pSplitLayout->fDragStartBefore = pSplitLayout->arrPaneResolvedSizes[iIndex];
			pSplitLayout->fDragStartAfter = pSplitLayout->arrPaneResolvedSizes[iIndex + 1];
#if XGE_HAS_DEBUGMODE
			printf("[xui_split_layout] drag-start index=%d axis=%.2f before=%.2f after=%.2f inside=%d\n",
				iIndex,
				pSplitLayout->fDragStartMouse,
				pSplitLayout->fDragStartBefore,
				pSplitLayout->fDragStartAfter,
				bInside);
#endif
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
#if XGE_HAS_DEBUGMODE
			printf("[xui_split_layout] drag-move index=%d axis=%.2f shadow=%d\n",
				iIndex,
				pSplitLayout->fDragCurrentMouse,
				pSplitLayout->bShadowDrag);
#endif
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
#if XGE_HAS_DEBUGMODE
			printf("[xui_split_layout] drag-end index=%d axis=%.2f\n", iIndex, pSplitLayout->fDragCurrentMouse);
#endif
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
#if XGE_HAS_DEBUGMODE
				printf("[xui_split_layout] drag-cancel index=%d\n", iIndex);
#endif
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
	int iIndex;
	uint32_t iColor;

	pSplitLayout = (xge_xui_split_layout)pUser;
	iIndex = __xgeXuiSplitLayoutDividerIndex(pWidget);
	if ( (pSplitLayout == NULL) || (pWidget == NULL) || (iIndex < 0) ) {
		return;
	}
	iColor = pSplitLayout->iDividerColor;
	if ( pSplitLayout->iActiveDivider == iIndex ) {
		iColor = pSplitLayout->iDividerActiveColor;
	} else if ( pSplitLayout->iHoverDivider == iIndex ) {
		iColor = pSplitLayout->iDividerHoverColor;
	}
	__xgeXuiHostDrawRect(pWidget->tRect, iColor);
}
