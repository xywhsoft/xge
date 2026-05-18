void xgeXuiScrollModelInit(xge_xui_scroll_model pModel, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( pModel == NULL ) {
		return;
	}
	memset(pModel, 0, sizeof(*pModel));
	pModel->pContext = pContext;
	pModel->pWidget = pWidget;
	if ( pWidget != NULL ) {
		pModel->tOuterViewportRect = pWidget->tContentRect;
		pModel->tViewportRect = pWidget->tContentRect;
	} else {
		memset(&pModel->tOuterViewportRect, 0, sizeof(pModel->tOuterViewportRect));
		memset(&pModel->tViewportRect, 0, sizeof(pModel->tViewportRect));
	}
	pModel->fContentW = pModel->tViewportRect.fW;
	pModel->fContentH = pModel->tViewportRect.fH;
	pModel->iBarColor = XGE_COLOR_RGBA(226, 236, 246, 220);
	pModel->iThumbColor = XGE_COLOR_RGBA(158, 176, 196, 235);
	pModel->iScrollbarPolicy = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	pModel->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	pModel->iNestedScrollPolicy = XGE_XUI_NESTED_SCROLL_CONSUME;
	pModel->iWheelAxis = XGE_XUI_WHEEL_AXIS_VERTICAL;
	pModel->bScrollbarDragEnabled = 1;
}

static int __xgeXuiScrollModelRectSame(xge_rect_t tA, xge_rect_t tB)
{
	return (tA.fX == tB.fX) && (tA.fY == tB.fY) && (tA.fW == tB.fW) && (tA.fH == tB.fH);
}

static float __xgeXuiScrollModelBarSize(xge_xui_scroll_model pModel)
{
	return (pModel != NULL && pModel->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
}

static int __xgeXuiScrollModelCanShowBars(xge_xui_scroll_model pModel, xge_rect_t tOuter)
{
	return (pModel != NULL) &&
		(pModel->iScrollbarPolicy != XGE_XUI_SCROLLBAR_POLICY_HIDDEN) &&
		(tOuter.fW > 0.0f) &&
		(tOuter.fH > 0.0f);
}

static void __xgeXuiScrollModelResolveViewport(xge_xui_scroll_model pModel, xge_rect_t tOuter, xge_rect_t* pViewport, int* pShowV, int* pShowH)
{
	xge_rect_t tViewport;
	float fBarSize;
	int bShowV;
	int bShowH;
	int bOldV;
	int bOldH;
	int i;

	tViewport = tOuter;
	bShowV = 0;
	bShowH = 0;
	if ( __xgeXuiScrollModelCanShowBars(pModel, tOuter) ) {
		fBarSize = __xgeXuiScrollModelBarSize(pModel);
		for ( i = 0; i < 3; i++ ) {
			bOldV = bShowV;
			bOldH = bShowH;
			if ( pModel->iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_ALWAYS ) {
				bShowV = 1;
				bShowH = 1;
			} else {
				bShowV = pModel->fContentH > (tOuter.fH - (bShowH ? fBarSize : 0.0f));
				bShowH = pModel->fContentW > (tOuter.fW - (bShowV ? fBarSize : 0.0f));
			}
			if ( (bOldV == bShowV) && (bOldH == bShowH) ) {
				break;
			}
		}
		if ( bShowV && tViewport.fW > fBarSize ) {
			tViewport.fW -= fBarSize;
		}
		if ( bShowH && tViewport.fH > fBarSize ) {
			tViewport.fH -= fBarSize;
		}
	}
	if ( tViewport.fW < 0.0f ) {
		tViewport.fW = 0.0f;
	}
	if ( tViewport.fH < 0.0f ) {
		tViewport.fH = 0.0f;
	}
	if ( pViewport != NULL ) {
		*pViewport = tViewport;
	}
	if ( pShowV != NULL ) {
		*pShowV = bShowV;
	}
	if ( pShowH != NULL ) {
		*pShowH = bShowH;
	}
}

static void __xgeXuiScrollModelSyncViewport(xge_xui_scroll_model pModel)
{
	if ( (pModel == NULL) || (pModel->pWidget == NULL) ) {
		return;
	}
	if ( __xgeXuiScrollModelRectSame(pModel->pWidget->tContentRect, pModel->tViewportRect) &&
		((pModel->tOuterViewportRect.fW > 0.0f) || (pModel->tOuterViewportRect.fH > 0.0f)) ) {
		return;
	}
	xgeXuiScrollModelSetViewport(pModel, pModel->pWidget->tContentRect);
}

void xgeXuiScrollModelGetMaxOffset(xge_xui_scroll_model pModel, float* pX, float* pY)
{
	float fMaxX;
	float fMaxY;

	fMaxX = 0.0f;
	fMaxY = 0.0f;
	if ( pModel != NULL ) {
		fMaxX = pModel->fContentW - pModel->tViewportRect.fW;
		fMaxY = pModel->fContentH - pModel->tViewportRect.fH;
		if ( fMaxX < 0.0f ) {
			fMaxX = 0.0f;
		}
		if ( fMaxY < 0.0f ) {
			fMaxY = 0.0f;
		}
	}
	if ( pX != NULL ) {
		*pX = fMaxX;
	}
	if ( pY != NULL ) {
		*pY = fMaxY;
	}
}

static void __xgeXuiScrollModelClamp(xge_xui_scroll_model pModel)
{
	float fMaxX;
	float fMaxY;

	if ( pModel == NULL ) {
		return;
	}
	xgeXuiScrollModelGetMaxOffset(pModel, &fMaxX, &fMaxY);
	pModel->fScrollX = __xgeXuiClampFloat(pModel->fScrollX, 0.0f, fMaxX);
	pModel->fScrollY = __xgeXuiClampFloat(pModel->fScrollY, 0.0f, fMaxY);
}

static void __xgeXuiScrollModelSetOffsetInternal(xge_xui_scroll_model pModel, float fX, float fY)
{
	float fOldX;
	float fOldY;

	if ( pModel == NULL ) {
		return;
	}
	fOldX = pModel->fScrollX;
	fOldY = pModel->fScrollY;
	xgeXuiScrollModelSetOffset(pModel, fX, fY);
	if ( (fOldX != pModel->fScrollX) || (fOldY != pModel->fScrollY) ) {
		if ( pModel->pWidget != NULL ) {
			xgeXuiWidgetMarkLayout(pModel->pWidget);
			xgeXuiWidgetMarkPaint(pModel->pWidget);
		}
	}
}

void xgeXuiScrollModelSetViewport(xge_xui_scroll_model pModel, xge_rect_t tViewport)
{
	if ( pModel == NULL ) {
		return;
	}
	pModel->tOuterViewportRect = tViewport;
	__xgeXuiScrollModelResolveViewport(pModel, tViewport, &pModel->tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(pModel);
}

void xgeXuiScrollModelSetContentSize(xge_xui_scroll_model pModel, float fWidth, float fHeight)
{
	if ( pModel == NULL ) {
		return;
	}
	pModel->fContentW = (fWidth > 0.0f) ? fWidth : 0.0f;
	pModel->fContentH = (fHeight > 0.0f) ? fHeight : 0.0f;
	__xgeXuiScrollModelResolveViewport(pModel, pModel->tOuterViewportRect, &pModel->tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(pModel);
}

void xgeXuiScrollModelSetOffset(xge_xui_scroll_model pModel, float fX, float fY)
{
	if ( pModel == NULL ) {
		return;
	}
	pModel->fScrollX = fX;
	pModel->fScrollY = fY;
	__xgeXuiScrollModelClamp(pModel);
}

void xgeXuiScrollModelScrollBy(xge_xui_scroll_model pModel, float fDX, float fDY)
{
	if ( pModel == NULL ) {
		return;
	}
	xgeXuiScrollModelSetOffset(pModel, pModel->fScrollX + fDX, pModel->fScrollY + fDY);
}

void xgeXuiScrollModelGetOffset(xge_xui_scroll_model pModel, float* pX, float* pY)
{
	if ( pX != NULL ) {
		*pX = (pModel != NULL) ? pModel->fScrollX : 0.0f;
	}
	if ( pY != NULL ) {
		*pY = (pModel != NULL) ? pModel->fScrollY : 0.0f;
	}
}

void xgeXuiScrollModelScreenToViewport(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pViewportX, float* pViewportY)
{
	__xgeXuiScrollModelSyncViewport(pModel);
	if ( pViewportX != NULL ) {
		*pViewportX = (pModel != NULL) ? (fScreenX - pModel->tViewportRect.fX) : fScreenX;
	}
	if ( pViewportY != NULL ) {
		*pViewportY = (pModel != NULL) ? (fScreenY - pModel->tViewportRect.fY) : fScreenY;
	}
}

void xgeXuiScrollModelViewportToContent(xge_xui_scroll_model pModel, float fViewportX, float fViewportY, float* pContentX, float* pContentY)
{
	if ( pContentX != NULL ) {
		*pContentX = (pModel != NULL) ? (fViewportX + pModel->fScrollX) : fViewportX;
	}
	if ( pContentY != NULL ) {
		*pContentY = (pModel != NULL) ? (fViewportY + pModel->fScrollY) : fViewportY;
	}
}

void xgeXuiScrollModelScreenToContent(xge_xui_scroll_model pModel, float fScreenX, float fScreenY, float* pContentX, float* pContentY)
{
	float fViewportX;
	float fViewportY;

	xgeXuiScrollModelScreenToViewport(pModel, fScreenX, fScreenY, &fViewportX, &fViewportY);
	xgeXuiScrollModelViewportToContent(pModel, fViewportX, fViewportY, pContentX, pContentY);
}

void xgeXuiScrollModelContentToScreen(xge_xui_scroll_model pModel, float fContentX, float fContentY, float* pScreenX, float* pScreenY)
{
	__xgeXuiScrollModelSyncViewport(pModel);
	if ( pScreenX != NULL ) {
		*pScreenX = (pModel != NULL) ? (pModel->tViewportRect.fX + fContentX - pModel->fScrollX) : fContentX;
	}
	if ( pScreenY != NULL ) {
		*pScreenY = (pModel != NULL) ? (pModel->tViewportRect.fY + fContentY - pModel->fScrollY) : fContentY;
	}
}

int xgeXuiScrollViewBaseInit(xge_xui_scroll_view_base pBase, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pBase == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiScrollModelInit(pBase, pContext, pWidget);
	__xgeXuiViewportWidgetInit(pWidget, 1);
	xgeXuiScrollModelSetViewport(pBase, pWidget->tContentRect);
	xgeXuiWidgetSetOverflow(pWidget, XGE_XUI_OVERFLOW_SCROLL);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiScrollViewBaseEventProc, NULL);
	pWidget->procPaintAfter = xgeXuiScrollViewBasePaintProc;
	pWidget->pUser = pBase;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(248, 250, 253, 255));
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

int xgeXuiScrollViewInit(xge_xui_scroll_view pScroll, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iResult;

	iResult = xgeXuiScrollViewBaseInit(&pScroll->tScroll, pContext, pWidget);
	if ( iResult != XGE_OK ) {
		return iResult;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiScrollViewEventProc, NULL);
	pWidget->procPaintAfter = xgeXuiScrollViewPaintProc;
	pWidget->pUser = pScroll;
	return XGE_OK;
}

static float __xgeXuiScrollViewMaxX(xge_xui_scroll_model pScroll)
{
	float fMax;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0.0f;
	}
	__xgeXuiScrollModelSyncViewport(pScroll);
	fMax = pScroll->fContentW - pScroll->tViewportRect.fW;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static float __xgeXuiScrollViewMaxY(xge_xui_scroll_model pScroll)
{
	float fMax;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0.0f;
	}
	__xgeXuiScrollModelSyncViewport(pScroll);
	fMax = pScroll->fContentH - pScroll->tViewportRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static int __xgeXuiScrollViewShowVerticalBar(xge_xui_scroll_model pScroll)
{
	int bShowV;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0;
	}
	__xgeXuiScrollModelSyncViewport(pScroll);
	__xgeXuiScrollModelResolveViewport(pScroll, pScroll->tOuterViewportRect, NULL, &bShowV, NULL);
	return bShowV;
}

static int __xgeXuiScrollViewShowHorizontalBar(xge_xui_scroll_model pScroll)
{
	int bShowH;

	if ( (pScroll == NULL) || (pScroll->pWidget == NULL) ) {
		return 0;
	}
	__xgeXuiScrollModelSyncViewport(pScroll);
	__xgeXuiScrollModelResolveViewport(pScroll, pScroll->tOuterViewportRect, NULL, NULL, &bShowH);
	return bShowH;
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

static float __xgeXuiScrollViewBarSize(xge_xui_scroll_model pScroll)
{
	return __xgeXuiScrollModelBarSize(pScroll);
}

static float __xgeXuiScrollViewButtonSize(xge_xui_scroll_model pScroll, float fBarSize)
{
	return (pScroll != NULL && pScroll->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fBarSize : 0.0f;
}

static int __xgeXuiScrollViewVerticalBar(xge_xui_scroll_model pScroll, xge_rect_t* pBar, xge_rect_t* pThumb)
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
	tBar.fX = pScroll->tOuterViewportRect.fX + pScroll->tOuterViewportRect.fW - fSize;
	tBar.fY = pScroll->tOuterViewportRect.fY;
	tBar.fW = fSize;
	tBar.fH = pScroll->tViewportRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	tThumb.fH = __xgeXuiScrollViewThumbLen(tThumb.fH, pScroll->tViewportRect.fH, pScroll->fContentH);
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

static int __xgeXuiScrollViewHorizontalBar(xge_xui_scroll_model pScroll, xge_rect_t* pBar, xge_rect_t* pThumb)
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
	tBar.fX = pScroll->tOuterViewportRect.fX;
	tBar.fY = pScroll->tOuterViewportRect.fY + pScroll->tOuterViewportRect.fH - fSize;
	tBar.fW = pScroll->tViewportRect.fW;
	tBar.fH = fSize;
	tThumb = tBar;
	tThumb.fX += fButton;
	tThumb.fW -= fButton * 2.0f;
	tThumb.fW = __xgeXuiScrollViewThumbLen(tThumb.fW, pScroll->tViewportRect.fW, pScroll->fContentW);
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

static void __xgeXuiScrollViewSetOffsetFromThumbDrag(xge_xui_scroll_model pScroll, const xge_event_t* pEvent)
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
			__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fDragScrollX, pScroll->fDragScrollY + ((pEvent->fY - pScroll->fDragY) / fTravel) * fMaxScroll);
		}
	} else if ( pScroll->bDragging == 3 && __xgeXuiScrollViewHorizontalBar(pScroll, &tBar, &tThumb) != 0 ) {
		fTravel = tBar.fW - tThumb.fW;
		fMaxScroll = __xgeXuiScrollViewMaxX(pScroll);
		if ( fTravel > 0.0f && fMaxScroll > 0.0f ) {
			__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fDragScrollX + ((pEvent->fX - pScroll->fDragX) / fTravel) * fMaxScroll, pScroll->fDragScrollY);
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
		xgeXuiWidgetSetEvent(pBase->pWidget, NULL, NULL);
		pBase->pWidget->procPaint = NULL;
		pBase->pWidget->procPaintAfter = NULL;
	}
	memset(pBase, 0, sizeof(*pBase));
}

void xgeXuiScrollViewUnit(xge_xui_scroll_view pScroll)
{
	if ( pScroll != NULL ) {
		xgeXuiScrollViewBaseUnit(&pScroll->tScroll);
	}
}

void xgeXuiScrollViewSetContentSize(xge_xui_scroll_view pScroll, float fWidth, float fHeight)
{
	xge_xui_scroll_model pModel;
	float fOldW;
	float fOldH;
	float fOldX;
	float fOldY;

	if ( pScroll == NULL ) {
		return;
	}
	pModel = &pScroll->tScroll;
	if ( pModel->pWidget != NULL ) {
		__xgeXuiScrollModelSyncViewport(pModel);
	}
	fOldW = pModel->fContentW;
	fOldH = pModel->fContentH;
	fOldX = pModel->fScrollX;
	fOldY = pModel->fScrollY;
	xgeXuiScrollModelSetContentSize(pModel, fWidth, fHeight);
	if ( (fOldW != pModel->fContentW) || (fOldH != pModel->fContentH) || (fOldX != pModel->fScrollX) || (fOldY != pModel->fScrollY) ) {
		xgeXuiWidgetMarkLayout(pModel->pWidget);
	}
	xgeXuiWidgetMarkPaint(pModel->pWidget);
}

void xgeXuiScrollViewSetOffset(xge_xui_scroll_view pScroll, float fX, float fY)
{
	if ( pScroll == NULL ) {
		return;
	}
	__xgeXuiScrollModelSetOffsetInternal(&pScroll->tScroll, fX, fY);
}

void xgeXuiScrollViewScrollBy(xge_xui_scroll_view pScroll, float fDX, float fDY)
{
	if ( pScroll == NULL ) {
		return;
	}
	__xgeXuiScrollModelSetOffsetInternal(&pScroll->tScroll, pScroll->tScroll.fScrollX + fDX, pScroll->tScroll.fScrollY + fDY);
}

void xgeXuiScrollViewGetOffset(xge_xui_scroll_view pScroll, float* pX, float* pY)
{
	xgeXuiScrollModelGetOffset((pScroll != NULL) ? &pScroll->tScroll : NULL, pX, pY);
}

void xgeXuiScrollViewEnsureRectVisible(xge_xui_scroll_view pScroll, xge_rect_t tRect)
{
	float fX;
	float fY;
	float fVisibleW;
	float fVisibleH;

	if ( (pScroll == NULL) || (pScroll->tScroll.pWidget == NULL) ) {
		return;
	}
	__xgeXuiScrollModelSyncViewport(&pScroll->tScroll);
	fX = pScroll->tScroll.fScrollX;
	fY = pScroll->tScroll.fScrollY;
	fVisibleW = pScroll->tScroll.tViewportRect.fW;
	fVisibleH = pScroll->tScroll.tViewportRect.fH;
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
	__xgeXuiScrollModelSetOffsetInternal(&pScroll->tScroll, fX, fY);
}

void xgeXuiScrollViewEnsureChildVisible(xge_xui_scroll_view pScroll, xge_xui_widget pChild)
{
	xge_rect_t tRect;

	if ( (pScroll == NULL) || (pScroll->tScroll.pWidget == NULL) || (pChild == NULL) ) {
		return;
	}
	tRect = pChild->tRect;
	tRect.fX = tRect.fX - pScroll->tScroll.pWidget->tContentRect.fX + pScroll->tScroll.fScrollX;
	tRect.fY = tRect.fY - pScroll->tScroll.pWidget->tContentRect.fY + pScroll->tScroll.fScrollY;
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
	pScroll->tScroll.iScrollbarPolicy = iPolicy;
	__xgeXuiScrollModelResolveViewport(&pScroll->tScroll, pScroll->tScroll.tOuterViewportRect, &pScroll->tScroll.tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(&pScroll->tScroll);
	xgeXuiWidgetMarkLayout(pScroll->tScroll.pWidget);
	xgeXuiWidgetMarkPaint(pScroll->tScroll.pWidget);
}

void xgeXuiScrollViewSetScrollbarMode(xge_xui_scroll_view pScroll, int iMode)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->tScroll.iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	__xgeXuiScrollModelResolveViewport(&pScroll->tScroll, pScroll->tScroll.tOuterViewportRect, &pScroll->tScroll.tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(&pScroll->tScroll);
	xgeXuiWidgetMarkLayout(pScroll->tScroll.pWidget);
	xgeXuiWidgetMarkPaint(pScroll->tScroll.pWidget);
}

int xgeXuiScrollViewGetScrollbarMode(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->tScroll.iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
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
	pScroll->tScroll.iNestedScrollPolicy = (iPolicy == XGE_XUI_NESTED_SCROLL_PASS_EDGE) ? XGE_XUI_NESTED_SCROLL_PASS_EDGE : XGE_XUI_NESTED_SCROLL_CONSUME;
}

void xgeXuiScrollViewSetWheelAxis(xge_xui_scroll_view pScroll, int iAxis)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->tScroll.iWheelAxis = __xgeXuiScrollViewWheelAxisClamp(iAxis);
}

int xgeXuiScrollViewGetWheelAxis(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? __xgeXuiScrollViewWheelAxisClamp(pScroll->tScroll.iWheelAxis) : XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollViewSetContentDragEnabled(xge_xui_scroll_view pScroll, int bEnabled)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->tScroll.bContentDragEnabled = bEnabled ? 1 : 0;
}

int xgeXuiScrollViewIsContentDragEnabled(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->tScroll.bContentDragEnabled : 0;
}

void xgeXuiScrollViewSetScrollbarDragEnabled(xge_xui_scroll_view pScroll, int bEnabled)
{
	if ( pScroll == NULL ) {
		return;
	}
	pScroll->tScroll.bScrollbarDragEnabled = bEnabled ? 1 : 0;
}

int xgeXuiScrollViewIsScrollbarDragEnabled(xge_xui_scroll_view pScroll)
{
	return (pScroll != NULL) ? pScroll->tScroll.bScrollbarDragEnabled : 0;
}

void xgeXuiScrollViewSetColors(xge_xui_scroll_view pScroll, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pScroll == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pScroll->tScroll.pWidget, iBackground);
	pScroll->tScroll.iBarColor = iBar;
	pScroll->tScroll.iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pScroll->tScroll.pWidget);
}

static int __xgeXuiScrollModelEvent(xge_xui_scroll_model pScroll, const xge_event_t* pEvent)
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
	__xgeXuiScrollModelSyncViewport(pScroll);
	if ( (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pScroll->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pScroll->tOuterViewportRect, pEvent->fX, pEvent->fY);
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
			__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fScrollX - fWheelX * 32.0f, pScroll->fScrollY - fWheelY * 32.0f);
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
					__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fScrollX, pScroll->fScrollY + ((pEvent->fY < tThumb.fY) ? -pScroll->tViewportRect.fH : pScroll->tViewportRect.fH));
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
					__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fScrollX + ((pEvent->fX < tThumb.fX) ? -pScroll->tViewportRect.fW : pScroll->tViewportRect.fW), pScroll->fScrollY);
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
				__xgeXuiScrollModelSetOffsetInternal(pScroll, pScroll->fScrollX + (pScroll->fDragX - pEvent->fX), pScroll->fScrollY + (pScroll->fDragY - pEvent->fY));
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

int xgeXuiScrollViewEvent(xge_xui_scroll_view pScroll, const xge_event_t* pEvent)
{
	return __xgeXuiScrollModelEvent((pScroll != NULL) ? &pScroll->tScroll : NULL, pEvent);
}

int xgeXuiScrollViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollViewEvent((xge_xui_scroll_view)pUser, pEvent);
}

static void __xgeXuiScrollViewPaintBar(xge_xui_scroll_model pScroll, xge_rect_t tBar, xge_rect_t tThumb)
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
	if ( __xgeXuiScrollViewVerticalBar(&pScroll->tScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(&pScroll->tScroll, tBar, tThumb);
	}
	if ( __xgeXuiScrollViewHorizontalBar(&pScroll->tScroll, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(&pScroll->tScroll, tBar, tThumb);
	}
}

void xgeXuiScrollViewBaseSetContentSize(xge_xui_scroll_view_base pBase, float fWidth, float fHeight)
{
	if ( pBase != NULL && pBase->pWidget != NULL ) {
		__xgeXuiScrollModelSyncViewport(pBase);
	}
	xgeXuiScrollModelSetContentSize(pBase, fWidth, fHeight);
	if ( pBase != NULL && pBase->pWidget != NULL ) {
		xgeXuiWidgetMarkLayout(pBase->pWidget);
		xgeXuiWidgetMarkPaint(pBase->pWidget);
	}
}

void xgeXuiScrollViewBaseSetOffset(xge_xui_scroll_view_base pBase, float fX, float fY)
{
	__xgeXuiScrollModelSetOffsetInternal(pBase, fX, fY);
}

void xgeXuiScrollViewBaseScrollBy(xge_xui_scroll_view_base pBase, float fDX, float fDY)
{
	if ( pBase != NULL ) {
		__xgeXuiScrollModelSetOffsetInternal(pBase, pBase->fScrollX + fDX, pBase->fScrollY + fDY);
	}
}

void xgeXuiScrollViewBaseGetOffset(xge_xui_scroll_view_base pBase, float* pX, float* pY)
{
	xgeXuiScrollModelGetOffset(pBase, pX, pY);
}

void xgeXuiScrollViewBaseEnsureRectVisible(xge_xui_scroll_view_base pBase, xge_rect_t tRect)
{
	float fX;
	float fY;
	float fVisibleW;
	float fVisibleH;

	if ( (pBase == NULL) || (pBase->pWidget == NULL) ) {
		return;
	}
	__xgeXuiScrollModelSyncViewport(pBase);
	fX = pBase->fScrollX;
	fY = pBase->fScrollY;
	fVisibleW = pBase->tViewportRect.fW;
	fVisibleH = pBase->tViewportRect.fH;
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
	__xgeXuiScrollModelSetOffsetInternal(pBase, fX, fY);
}

void xgeXuiScrollViewBaseEnsureChildVisible(xge_xui_scroll_view_base pBase, xge_xui_widget pChild)
{
	xge_rect_t tRect;

	if ( (pBase == NULL) || (pBase->pWidget == NULL) || (pChild == NULL) ) {
		return;
	}
	tRect = pChild->tRect;
	tRect.fX = tRect.fX - pBase->pWidget->tContentRect.fX + pBase->fScrollX;
	tRect.fY = tRect.fY - pBase->pWidget->tContentRect.fY + pBase->fScrollY;
	xgeXuiScrollViewBaseEnsureRectVisible(pBase, tRect);
}

void xgeXuiScrollViewBaseSetScrollbarPolicy(xge_xui_scroll_view_base pBase, int iPolicy)
{
	if ( pBase == NULL ) {
		return;
	}
	if ( (iPolicy != XGE_XUI_SCROLLBAR_POLICY_ALWAYS) && (iPolicy != XGE_XUI_SCROLLBAR_POLICY_HIDDEN) ) {
		iPolicy = XGE_XUI_SCROLLBAR_POLICY_AUTO;
	}
	pBase->iScrollbarPolicy = iPolicy;
	__xgeXuiScrollModelResolveViewport(pBase, pBase->tOuterViewportRect, &pBase->tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

void xgeXuiScrollViewBaseSetScrollbarMode(xge_xui_scroll_view_base pBase, int iMode)
{
	if ( pBase == NULL ) {
		return;
	}
	pBase->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	__xgeXuiScrollModelResolveViewport(pBase, pBase->tOuterViewportRect, &pBase->tViewportRect, NULL, NULL);
	__xgeXuiScrollModelClamp(pBase);
	xgeXuiWidgetMarkLayout(pBase->pWidget);
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

int xgeXuiScrollViewBaseGetScrollbarMode(xge_xui_scroll_view_base pBase)
{
	return (pBase != NULL) ? pBase->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiScrollViewBaseSetNestedScrollPolicy(xge_xui_scroll_view_base pBase, int iPolicy)
{
	if ( pBase != NULL ) {
		pBase->iNestedScrollPolicy = (iPolicy == XGE_XUI_NESTED_SCROLL_PASS_EDGE) ? XGE_XUI_NESTED_SCROLL_PASS_EDGE : XGE_XUI_NESTED_SCROLL_CONSUME;
	}
}

void xgeXuiScrollViewBaseSetWheelAxis(xge_xui_scroll_view_base pBase, int iAxis)
{
	if ( pBase != NULL ) {
		pBase->iWheelAxis = __xgeXuiScrollViewWheelAxisClamp(iAxis);
	}
}

int xgeXuiScrollViewBaseGetWheelAxis(xge_xui_scroll_view_base pBase)
{
	return (pBase != NULL) ? __xgeXuiScrollViewWheelAxisClamp(pBase->iWheelAxis) : XGE_XUI_WHEEL_AXIS_VERTICAL;
}

void xgeXuiScrollViewBaseSetContentDragEnabled(xge_xui_scroll_view_base pBase, int bEnabled)
{
	if ( pBase != NULL ) {
		pBase->bContentDragEnabled = bEnabled ? 1 : 0;
	}
}

int xgeXuiScrollViewBaseIsContentDragEnabled(xge_xui_scroll_view_base pBase)
{
	return (pBase != NULL) ? pBase->bContentDragEnabled : 0;
}

void xgeXuiScrollViewBaseSetScrollbarDragEnabled(xge_xui_scroll_view_base pBase, int bEnabled)
{
	if ( pBase != NULL ) {
		pBase->bScrollbarDragEnabled = bEnabled ? 1 : 0;
	}
}

int xgeXuiScrollViewBaseIsScrollbarDragEnabled(xge_xui_scroll_view_base pBase)
{
	return (pBase != NULL) ? pBase->bScrollbarDragEnabled : 0;
}

void xgeXuiScrollViewBaseSetColors(xge_xui_scroll_view_base pBase, uint32_t iBackground, uint32_t iBar, uint32_t iThumb)
{
	if ( pBase == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pBase->pWidget, iBackground);
	pBase->iBarColor = iBar;
	pBase->iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pBase->pWidget);
}

int xgeXuiScrollViewBaseEvent(xge_xui_scroll_view_base pBase, const xge_event_t* pEvent)
{
	return __xgeXuiScrollModelEvent(pBase, pEvent);
}

int xgeXuiScrollViewBaseEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiScrollViewBaseEvent((xge_xui_scroll_view_base)pUser, pEvent);
}

void xgeXuiScrollViewBasePaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_scroll_model pBase;
	xge_rect_t tBar;
	xge_rect_t tThumb;

	(void)pWidget;
	pBase = (xge_xui_scroll_model)pUser;
	if ( pBase == NULL ) {
		return;
	}
	if ( __xgeXuiScrollViewVerticalBar(pBase, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(pBase, tBar, tThumb);
	}
	if ( __xgeXuiScrollViewHorizontalBar(pBase, &tBar, &tThumb) != 0 ) {
		__xgeXuiScrollViewPaintBar(pBase, tBar, tThumb);
	}
}
