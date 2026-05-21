static int __xgeXuiToastNormalizeType(int iType)
{
	if ( iType < XGE_XUI_TOAST_TYPE_INFO || iType > XGE_XUI_TOAST_TYPE_ERROR ) {
		return XGE_XUI_TOAST_TYPE_INFO;
	}
	return iType;
}

static int __xgeXuiToastNormalizePlacement(int iPlacement)
{
	if ( iPlacement < XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT || iPlacement > XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) {
		return XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
	}
	return iPlacement;
}

static int __xgeXuiToastNormalizeDirection(int iDirection)
{
	if ( (iDirection != XGE_XUI_TOAST_DIRECTION_DOWN) && (iDirection != XGE_XUI_TOAST_DIRECTION_UP) ) {
		return XGE_XUI_TOAST_DIRECTION_AUTO;
	}
	return iDirection;
}

static xge_font __xgeXuiToastFont(xge_xui_toast pToast)
{
	const xge_xui_theme_t* pTheme;

	if ( pToast == NULL ) {
		return NULL;
	}
	if ( pToast->pFont != NULL ) {
		return pToast->pFont;
	}
	pTheme = xgeXuiGetTheme(pToast->pContext);
	return (pTheme != NULL) ? pTheme->pFont : NULL;
}

static void __xgeXuiToastDefaults(xge_xui_context pContext)
{
	xge_xui_toast pToast;

	if ( pContext == NULL ) {
		return;
	}
	pToast = &pContext->tToast;
	if ( pToast->bDefaultsReady != 0 ) {
		return;
	}
	memset(pToast, 0, sizeof(*pToast));
	pToast->pContext = pContext;
	pToast->iNextId = 1;
	pToast->iPlacement = XGE_XUI_TOAST_PLACEMENT_TOP_RIGHT;
	pToast->iDirection = XGE_XUI_TOAST_DIRECTION_AUTO;
	pToast->iMaxVisible = 0;
	pToast->fWidth = 300.0f;
	pToast->fMargin = 18.0f;
	pToast->fGap = 8.0f;
	pToast->fMinHeight = 54.0f;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	memset(&pToast->tItemStyle, 0, sizeof(pToast->tItemStyle));
	pToast->tItemStyle.iBackgroundColor = XGE_COLOR_RGBA(247, 252, 255, 245);
	pToast->tItemStyle.iBorderColor = XGE_COLOR_RGBA(129, 174, 207, 255);
	pToast->tItemStyle.fBorderWidth = 1.0f;
	pToast->tItemStyle.fRadius = 4.0f;
	pToast->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pToast->iMutedTextColor = XGE_COLOR_RGBA(92, 112, 130, 255);
	pToast->iInfoColor = XGE_COLOR_RGBA(78, 159, 220, 255);
	pToast->iSuccessColor = XGE_COLOR_RGBA(43, 184, 150, 255);
	pToast->iWarningColor = XGE_COLOR_RGBA(244, 187, 68, 255);
	pToast->iErrorColor = XGE_COLOR_RGBA(224, 92, 92, 255);
	pToast->iCloseColor = XGE_COLOR_RGBA(96, 126, 148, 255);
	pToast->iCloseHoverColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pToast->bDefaultsReady = 1;
}

static uint32_t __xgeXuiToastTypeColor(xge_xui_toast pToast, int iType)
{
	if ( pToast == NULL ) {
		return 0;
	}
	switch ( iType ) {
		case XGE_XUI_TOAST_TYPE_SUCCESS:
			return pToast->iSuccessColor;
		case XGE_XUI_TOAST_TYPE_WARNING:
			return pToast->iWarningColor;
		case XGE_XUI_TOAST_TYPE_ERROR:
			return pToast->iErrorColor;
		case XGE_XUI_TOAST_TYPE_INFO:
		default:
			return pToast->iInfoColor;
	}
}

static int __xgeXuiToastTypeAsset(int iType)
{
	switch ( iType ) {
		case XGE_XUI_TOAST_TYPE_SUCCESS:
			return XGE_XUI_ASSET_PROPERTY_CHECK_12;
		case XGE_XUI_TOAST_TYPE_WARNING:
			return XGE_XUI_ASSET_MSGBOX_WAR;
		case XGE_XUI_TOAST_TYPE_ERROR:
			return XGE_XUI_ASSET_MSGBOX_ERROR;
		case XGE_XUI_TOAST_TYPE_INFO:
		default:
			return XGE_XUI_ASSET_MSGBOX_INFO;
	}
}

static void __xgeXuiToastItemFree(xge_xui_toast_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	if ( pItem->sTitle != NULL ) {
		xrtFree(pItem->sTitle);
	}
	if ( pItem->sMessage != NULL ) {
		xrtFree(pItem->sMessage);
	}
	memset(pItem, 0, sizeof(*pItem));
}

static void __xgeXuiToastItemMove(xge_xui_toast_item_t* pDst, xge_xui_toast_item_t* pSrc)
{
	if ( (pDst == NULL) || (pSrc == NULL) ) {
		return;
	}
	*pDst = *pSrc;
	memset(pSrc, 0, sizeof(*pSrc));
}

static int __xgeXuiToastDirection(xge_xui_toast pToast)
{
	if ( pToast == NULL ) {
		return XGE_XUI_TOAST_DIRECTION_DOWN;
	}
	if ( pToast->iDirection == XGE_XUI_TOAST_DIRECTION_DOWN || pToast->iDirection == XGE_XUI_TOAST_DIRECTION_UP ) {
		return pToast->iDirection;
	}
	if ( pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_RIGHT ) {
		return XGE_XUI_TOAST_DIRECTION_UP;
	}
	return XGE_XUI_TOAST_DIRECTION_DOWN;
}

static int __xgeXuiToastVisibleLimit(xge_xui_toast pToast)
{
	xge_rect_t tBounds;
	float fUsable;
	int iLimit;

	if ( (pToast == NULL) || (pToast->pContext == NULL) ) {
		return 0;
	}
	if ( pToast->iMaxVisible > 0 ) {
		iLimit = pToast->iMaxVisible;
	} else {
		tBounds = (pToast->pContext->pOverlayRoot != NULL) ? pToast->pContext->pOverlayRoot->tContentRect : pToast->pContext->pRoot->tContentRect;
		if ( tBounds.fW <= 0.0f || tBounds.fH <= 0.0f ) {
			tBounds = (pToast->pContext->pRoot != NULL) ? pToast->pContext->pRoot->tRect : (xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() };
		}
		fUsable = tBounds.fH - pToast->fMargin * 2.0f + pToast->fGap;
		iLimit = (int)(fUsable / (pToast->fMinHeight + pToast->fGap));
	}
	if ( iLimit < 1 ) {
		iLimit = 1;
	}
	if ( iLimit > XGE_XUI_TOAST_VISIBLE_CAPACITY ) {
		iLimit = XGE_XUI_TOAST_VISIBLE_CAPACITY;
	}
	return iLimit;
}

static void __xgeXuiToastLayout(xge_xui_toast pToast)
{
	xge_rect_t tBounds;
	xge_xui_toast_item_t* pItem;
	xge_font pFont;
	float fWidth;
	float fTextW;
	float fLineH;
	float fTitleH;
	float fMessageH;
	float fStackH;
	float fY;
	float fX;
	int iMessageLines;
	int iDirection;
	int i;

	if ( (pToast == NULL) || (pToast->pContext == NULL) ) {
		return;
	}
	tBounds = (pToast->pContext->pOverlayRoot != NULL) ? pToast->pContext->pOverlayRoot->tContentRect : (xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() };
	if ( tBounds.fW <= 0.0f || tBounds.fH <= 0.0f ) {
		tBounds = (pToast->pContext->pRoot != NULL) ? pToast->pContext->pRoot->tRect : (xge_rect_t){ 0.0f, 0.0f, (float)xgeGetWidth(), (float)xgeGetHeight() };
	}
	if ( pToast->pWidget != NULL ) {
		xgeXuiWidgetSetRect(pToast->pWidget, tBounds);
	}
	pFont = __xgeXuiToastFont(pToast);
	fWidth = pToast->fWidth;
	if ( fWidth > tBounds.fW - pToast->fMargin * 2.0f ) {
		fWidth = tBounds.fW - pToast->fMargin * 2.0f;
	}
	if ( fWidth < 140.0f ) {
		fWidth = (tBounds.fW > 140.0f) ? 140.0f : tBounds.fW;
	}
	fLineH = __xgeXuiBoxLineHeight(pFont);
	fTextW = fWidth - 78.0f;
	if ( fTextW < 48.0f ) {
		fTextW = 48.0f;
	}
	fStackH = 0.0f;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		pItem = &pToast->arrActive[i];
		fTitleH = ((pItem->sTitle != NULL) && (pItem->sTitle[0] != 0)) ? fLineH : 0.0f;
		iMessageLines = __xgeXuiBoxWrapMeasure(pFont, (pItem->sMessage != NULL) ? pItem->sMessage : "", fTextW, NULL);
		fMessageH = ((pItem->sMessage != NULL) && (pItem->sMessage[0] != 0)) ? (float)iMessageLines * fLineH : 0.0f;
		pItem->fHeight = fTitleH + fMessageH + 18.0f;
		if ( (fTitleH > 0.0f) && (fMessageH > 0.0f) ) {
			pItem->fHeight += 2.0f;
		}
		if ( pItem->fHeight < pToast->fMinHeight ) {
			pItem->fHeight = pToast->fMinHeight;
		}
		fStackH += pItem->fHeight;
		if ( i + 1 < pToast->iActiveCount ) {
			fStackH += pToast->fGap;
		}
	}
	if ( pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_TOP_LEFT || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_LEFT ) {
		fX = tBounds.fX + pToast->fMargin;
	} else if ( pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_TOP_CENTER || pToast->iPlacement == XGE_XUI_TOAST_PLACEMENT_BOTTOM_CENTER ) {
		fX = tBounds.fX + (tBounds.fW - fWidth) * 0.5f;
	} else {
		fX = tBounds.fX + tBounds.fW - pToast->fMargin - fWidth;
	}
	if ( fX < tBounds.fX ) {
		fX = tBounds.fX;
	}
	if ( fX + fWidth > tBounds.fX + tBounds.fW ) {
		fX = tBounds.fX + tBounds.fW - fWidth;
	}
	iDirection = __xgeXuiToastDirection(pToast);
	if ( iDirection == XGE_XUI_TOAST_DIRECTION_UP ) {
		fY = tBounds.fY + tBounds.fH - pToast->fMargin - fStackH;
	} else {
		fY = tBounds.fY + pToast->fMargin;
	}
	if ( fY < tBounds.fY ) {
		fY = tBounds.fY;
	}
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		pItem = &pToast->arrActive[i];
		pItem->tRect = (xge_rect_t){ fX, fY, fWidth, pItem->fHeight };
		pItem->tCloseRect = (xge_rect_t){ fX + fWidth - 24.0f, fY + 8.0f, 16.0f, 16.0f };
		pItem->tTitleRect = (xge_rect_t){ fX + 48.0f, fY + 8.0f, fWidth - 78.0f, fLineH };
		if ( (pItem->sTitle == NULL) || (pItem->sTitle[0] == 0) ) {
			pItem->tMessageRect = (xge_rect_t){ fX + 48.0f, fY + (pItem->fHeight - fLineH) * 0.5f, fWidth - 60.0f, pItem->fHeight - 16.0f };
		} else {
			pItem->tMessageRect = (xge_rect_t){ fX + 48.0f, fY + 8.0f + fLineH + 2.0f, fWidth - 60.0f, pItem->fHeight - 18.0f - fLineH };
		}
		if ( i < XGE_XUI_TOAST_VISIBLE_CAPACITY && pToast->arrHitWidget[i] != NULL ) {
			xgeXuiWidgetSetRect(pToast->arrHitWidget[i], pItem->tRect);
			xgeXuiWidgetSetVisible(pToast->arrHitWidget[i], 1);
			xgeXuiWidgetSetHitTestVisible(pToast->arrHitWidget[i], 1);
		}
		fY += pItem->fHeight + pToast->fGap;
	}
	for ( i = pToast->iActiveCount; i < XGE_XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		if ( pToast->arrHitWidget[i] != NULL ) {
			xgeXuiWidgetSetVisible(pToast->arrHitWidget[i], 0);
			xgeXuiWidgetSetHitTestVisible(pToast->arrHitWidget[i], 0);
			xgeXuiWidgetSetRect(pToast->arrHitWidget[i], (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f });
		}
	}
}

static int __xgeXuiToastHitEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static void __xgeXuiToastUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
static void __xgeXuiToastPaintProc(xge_xui_widget pWidget, void* pUser);

static int __xgeXuiToastEnsureWidget(xge_xui_context pContext)
{
	xge_xui_toast pToast;
	int i;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiToastDefaults(pContext);
	pToast = &pContext->tToast;
	if ( pToast->pWidget != NULL ) {
		return XGE_OK;
	}
	pToast->pWidget = xgeXuiWidgetCreate();
	if ( pToast->pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiOverlayWidgetInit(pToast->pWidget, 0);
	xgeXuiWidgetSetBackground(pToast->pWidget, 0);
	xgeXuiWidgetSetBorder(pToast->pWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pToast->pWidget, 0);
	xgeXuiWidgetSetInputTransparent(pToast->pWidget, 1);
	pToast->pWidget->procUpdate = __xgeXuiToastUpdateProc;
	pToast->pWidget->procPaint = __xgeXuiToastPaintProc;
	pToast->pWidget->pUser = pToast;
	for ( i = 0; i < XGE_XUI_TOAST_VISIBLE_CAPACITY; i++ ) {
		pToast->arrHitWidget[i] = xgeXuiWidgetCreate();
		if ( pToast->arrHitWidget[i] == NULL ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		xgeXuiWidgetSetBackground(pToast->arrHitWidget[i], 0);
		xgeXuiWidgetSetBorder(pToast->arrHitWidget[i], 0.0f, 0);
		xgeXuiWidgetSetVisible(pToast->arrHitWidget[i], 0);
		xgeXuiWidgetSetHitTestVisible(pToast->arrHitWidget[i], 0);
		xgeXuiWidgetSetEvent(pToast->arrHitWidget[i], __xgeXuiToastHitEventProc, pToast);
		xgeXuiWidgetAdd(pToast->pWidget, pToast->arrHitWidget[i]);
	}
	if ( xgeXuiOverlayAttach(pContext, pToast->pWidget, NULL, XGE_XUI_LAYER_TOOLTIP) != XGE_OK ) {
		return XGE_ERROR;
	}
	__xgeXuiToastLayout(pToast);
	return XGE_OK;
}

static void __xgeXuiToastNotifyCloseId(xge_xui_toast pToast, int iToastId, int iReason)
{
	if ( (pToast != NULL) && (pToast->procClose != NULL) && (iToastId > 0) ) {
		pToast->procClose(pToast->pContext, iToastId, iReason, pToast->pCloseUser);
	}
}

static int __xgeXuiToastFindActiveById(xge_xui_toast pToast, int iToastId)
{
	int i;

	if ( (pToast == NULL) || (iToastId <= 0) ) {
		return -1;
	}
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		if ( pToast->arrActive[i].iId == iToastId ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiToastActivatePending(xge_xui_toast pToast)
{
	int iLimit;
	int i;

	if ( pToast == NULL ) {
		return;
	}
	iLimit = __xgeXuiToastVisibleLimit(pToast);
	while ( (pToast->iActiveCount < iLimit) && (pToast->iPendingCount > 0) ) {
		__xgeXuiToastItemMove(&pToast->arrActive[pToast->iActiveCount], &pToast->arrPending[0]);
		pToast->arrActive[pToast->iActiveCount].fElapsed = 0.0f;
		pToast->iActiveCount++;
		for ( i = 0; i + 1 < pToast->iPendingCount; i++ ) {
			__xgeXuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
		}
		pToast->iPendingCount--;
	}
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

static void __xgeXuiToastRemoveActiveAt(xge_xui_toast pToast, int iIndex, int iReason)
{
	int iToastId;
	int i;

	if ( (pToast == NULL) || (iIndex < 0) || (iIndex >= pToast->iActiveCount) ) {
		return;
	}
	iToastId = pToast->arrActive[iIndex].iId;
	__xgeXuiToastItemFree(&pToast->arrActive[iIndex]);
	for ( i = iIndex; i + 1 < pToast->iActiveCount; i++ ) {
		__xgeXuiToastItemMove(&pToast->arrActive[i], &pToast->arrActive[i + 1]);
	}
	pToast->iActiveCount--;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	if ( iReason == XGE_XUI_TOAST_CLOSE_TIMEOUT ) {
		pToast->iExpireCount++;
	} else {
		pToast->iCloseCount++;
	}
	__xgeXuiToastActivatePending(pToast);
	__xgeXuiToastNotifyCloseId(pToast, iToastId, iReason);
}

static void __xgeXuiToastRequestActiveClose(xge_xui_toast pToast, int iIndex, int iReason)
{
	xge_xui_toast_item_t* pItem;

	if ( (pToast == NULL) || (iIndex < 0) || (iIndex >= pToast->iActiveCount) ) {
		return;
	}
	pItem = &pToast->arrActive[iIndex];
	if ( pItem->bClosing != 0 ) {
		return;
	}
	pItem->bClosing = 1;
	pItem->iCloseReason = iReason;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

static void __xgeXuiToastProcessClosing(xge_xui_toast pToast)
{
	xge_xui_toast_click_proc procClick;
	void* pUser;
	int iId;
	int iReason;
	int iIndex;
	int i;

	if ( pToast == NULL ) {
		return;
	}
	i = 0;
	while ( i < pToast->iActiveCount ) {
		if ( pToast->arrActive[i].bClosing == 0 ) {
			i++;
			continue;
		}
		iId = pToast->arrActive[i].iId;
		iReason = pToast->arrActive[i].iCloseReason;
		procClick = (iReason == XGE_XUI_TOAST_CLOSE_CLICK) ? pToast->arrActive[i].procClick : NULL;
		pUser = pToast->arrActive[i].pUser;
		if ( procClick != NULL ) {
			procClick(pToast->pContext, iId, pUser);
		}
		iIndex = __xgeXuiToastFindActiveById(pToast, iId);
		if ( iIndex >= 0 ) {
			__xgeXuiToastRemoveActiveAt(pToast, iIndex, iReason);
			i = 0;
			continue;
		}
		i = 0;
	}
}

static int __xgeXuiToastDropPendingAt(xge_xui_toast pToast, int iIndex)
{
	int iToastId;
	int i;

	if ( (pToast == NULL) || (iIndex < 0) || (iIndex >= pToast->iPendingCount) ) {
		return 0;
	}
	iToastId = pToast->arrPending[iIndex].iId;
	__xgeXuiToastItemFree(&pToast->arrPending[iIndex]);
	for ( i = iIndex; i + 1 < pToast->iPendingCount; i++ ) {
		__xgeXuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
	}
	pToast->iPendingCount--;
	pToast->iDropCount++;
	return iToastId;
}

int xgeXuiToastShow(xge_xui_context pContext, int iType, const char* sTitle, const char* sMessage, float fDuration, xge_xui_toast_click_proc procClick, void* pUser)
{
	xge_xui_toast pToast;
	xge_xui_toast_item_t tItem;
	int iId;
	int iDroppedId;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) ) {
		return -1;
	}
	if ( __xgeXuiToastEnsureWidget(pContext) != XGE_OK ) {
		return -1;
	}
	pToast = &pContext->tToast;
	memset(&tItem, 0, sizeof(tItem));
	tItem.sTitle = __xgeXuiBoxCopyText(sTitle);
	tItem.sMessage = __xgeXuiBoxCopyText(sMessage);
	if ( (tItem.sTitle == NULL) || (tItem.sMessage == NULL) ) {
		__xgeXuiToastItemFree(&tItem);
		return -1;
	}
	iId = pToast->iNextId++;
	if ( pToast->iNextId <= 0 ) {
		pToast->iNextId = 1;
	}
	tItem.iId = iId;
	tItem.iType = __xgeXuiToastNormalizeType(iType);
	tItem.fDuration = (fDuration <= 0.0f) ? 3.0f : fDuration;
	tItem.procClick = procClick;
	tItem.pUser = pUser;
	iDroppedId = 0;
	if ( pToast->iPendingCount >= XGE_XUI_TOAST_QUEUE_CAPACITY ) {
		iDroppedId = __xgeXuiToastDropPendingAt(pToast, 0);
	}
	__xgeXuiToastItemMove(&pToast->arrPending[pToast->iPendingCount], &tItem);
	pToast->iPendingCount++;
	pToast->iShowCount++;
	__xgeXuiToastActivatePending(pToast);
	if ( iDroppedId > 0 ) {
		__xgeXuiToastNotifyCloseId(pToast, iDroppedId, XGE_XUI_TOAST_CLOSE_CAPACITY_DROP);
	}
	return iId;
}

int xgeXuiToastClose(xge_xui_context pContext, int iToastId)
{
	xge_xui_toast pToast;
	int i;

	if ( (pContext == NULL) || (pContext->bInitialized == 0) || (iToastId <= 0) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiToastDefaults(pContext);
	pToast = &pContext->tToast;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		if ( pToast->arrActive[i].iId == iToastId ) {
			__xgeXuiToastRemoveActiveAt(pToast, i, XGE_XUI_TOAST_CLOSE_API);
			return XGE_OK;
		}
	}
	for ( i = 0; i < pToast->iPendingCount; i++ ) {
		if ( pToast->arrPending[i].iId == iToastId ) {
			__xgeXuiToastItemFree(&pToast->arrPending[i]);
			for ( ; i + 1 < pToast->iPendingCount; i++ ) {
				__xgeXuiToastItemMove(&pToast->arrPending[i], &pToast->arrPending[i + 1]);
			}
			pToast->iPendingCount--;
			pToast->iCloseCount++;
			xgeXuiWidgetMarkPaint(pToast->pWidget);
			__xgeXuiToastNotifyCloseId(pToast, iToastId, XGE_XUI_TOAST_CLOSE_API);
			return XGE_OK;
		}
	}
	return XGE_ERROR;
}

void xgeXuiToastClear(xge_xui_context pContext)
{
	xge_xui_toast pToast;
	int arrNotifyId[XGE_XUI_TOAST_VISIBLE_CAPACITY + XGE_XUI_TOAST_QUEUE_CAPACITY];
	int iNotifyCount;
	int i;

	if ( pContext == NULL ) {
		return;
	}
	pToast = &pContext->tToast;
	if ( pToast->bDefaultsReady == 0 ) {
		return;
	}
	iNotifyCount = 0;
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		arrNotifyId[iNotifyCount++] = pToast->arrActive[i].iId;
		__xgeXuiToastItemFree(&pToast->arrActive[i]);
	}
	for ( i = 0; i < pToast->iPendingCount; i++ ) {
		arrNotifyId[iNotifyCount++] = pToast->arrPending[i].iId;
		__xgeXuiToastItemFree(&pToast->arrPending[i]);
	}
	pToast->iActiveCount = 0;
	pToast->iPendingCount = 0;
	pToast->iHoverItem = -1;
	pToast->iHoverClose = -1;
	__xgeXuiToastLayout(pToast);
	xgeXuiWidgetMarkPaint(pToast->pWidget);
	for ( i = 0; i < iNotifyCount; i++ ) {
		__xgeXuiToastNotifyCloseId(pToast, arrNotifyId[i], XGE_XUI_TOAST_CLOSE_CLEAR);
	}
}

int xgeXuiToastGetActiveCount(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return 0;
	}
	__xgeXuiToastDefaults(pContext);
	return pContext->tToast.iActiveCount;
}

int xgeXuiToastGetPendingCount(xge_xui_context pContext)
{
	if ( pContext == NULL ) {
		return 0;
	}
	__xgeXuiToastDefaults(pContext);
	return pContext->tToast.iPendingCount;
}

void xgeXuiToastSetPlacement(xge_xui_context pContext, int iPlacement)
{
	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pContext->tToast.iPlacement = __xgeXuiToastNormalizePlacement(iPlacement);
	__xgeXuiToastLayout(&pContext->tToast);
	xgeXuiWidgetMarkPaint(pContext->tToast.pWidget);
}

void xgeXuiToastSetDirection(xge_xui_context pContext, int iDirection)
{
	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pContext->tToast.iDirection = __xgeXuiToastNormalizeDirection(iDirection);
	__xgeXuiToastLayout(&pContext->tToast);
	xgeXuiWidgetMarkPaint(pContext->tToast.pWidget);
}

void xgeXuiToastSetMetrics(xge_xui_context pContext, float fWidth, float fMargin, float fGap, int iMaxVisible)
{
	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pContext->tToast.fWidth = (fWidth < 140.0f) ? 140.0f : fWidth;
	pContext->tToast.fMargin = (fMargin < 0.0f) ? 0.0f : fMargin;
	pContext->tToast.fGap = (fGap < 0.0f) ? 0.0f : fGap;
	pContext->tToast.iMaxVisible = (iMaxVisible < 0) ? 0 : iMaxVisible;
	__xgeXuiToastActivatePending(&pContext->tToast);
}

void xgeXuiToastSetFont(xge_xui_context pContext, xge_font pFont)
{
	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pContext->tToast.pFont = pFont;
	__xgeXuiToastLayout(&pContext->tToast);
	xgeXuiWidgetMarkPaint(pContext->tToast.pWidget);
}

void xgeXuiToastSetColors(xge_xui_context pContext, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iMutedText, uint32_t iInfo, uint32_t iSuccess, uint32_t iWarning, uint32_t iError)
{
	xge_xui_toast pToast;

	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pToast = &pContext->tToast;
	pToast->tItemStyle.iBackgroundColor = iBackground;
	pToast->tItemStyle.iBorderColor = iBorder;
	pToast->iTextColor = iText;
	pToast->iMutedTextColor = iMutedText;
	pToast->iInfoColor = iInfo;
	pToast->iSuccessColor = iSuccess;
	pToast->iWarningColor = iWarning;
	pToast->iErrorColor = iError;
	xgeXuiWidgetMarkPaint(pToast->pWidget);
}

void xgeXuiToastSetClose(xge_xui_context pContext, xge_xui_toast_close_proc procClose, void* pUser)
{
	if ( pContext == NULL ) {
		return;
	}
	__xgeXuiToastDefaults(pContext);
	pContext->tToast.procClose = procClose;
	pContext->tToast.pCloseUser = pUser;
}

static int __xgeXuiToastHitIndex(xge_xui_toast pToast, float fX, float fY)
{
	int i;

	if ( pToast == NULL ) {
		return -1;
	}
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		if ( __xgeXuiRectContains(pToast->arrActive[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiToastHitEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_toast pToast;
	xge_xui_toast_item_t* pItem;
	int iIndex;
	int iClose;

	(void)pWidget;
	pToast = (xge_xui_toast)pUser;
	if ( (pToast == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iIndex = __xgeXuiToastHitIndex(pToast, pEvent->fX, pEvent->fY);
	if ( pEvent->iType == XGE_EVENT_MOUSE_MOVE ) {
		iClose = (iIndex >= 0 && __xgeXuiRectContains(pToast->arrActive[iIndex].tCloseRect, pEvent->fX, pEvent->fY)) ? iIndex : -1;
		if ( pToast->iHoverItem != iIndex || pToast->iHoverClose != iClose ) {
			pToast->iHoverItem = iIndex;
			pToast->iHoverClose = iClose;
			xgeXuiWidgetMarkPaint(pToast->pWidget);
		}
		return (iIndex >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pEvent->iType == XGE_EVENT_MOUSE_DOWN && pEvent->iParam1 == XGE_MOUSE_LEFT) || pEvent->iType == XGE_EVENT_TOUCH_BEGIN) && iIndex >= 0 ) {
		pItem = &pToast->arrActive[iIndex];
		if ( pItem->bClosing != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
		if ( __xgeXuiRectContains(pItem->tCloseRect, pEvent->fX, pEvent->fY) ) {
			__xgeXuiToastRequestActiveClose(pToast, iIndex, XGE_XUI_TOAST_CLOSE_BUTTON);
			return XGE_XUI_EVENT_CONSUMED;
		}
		__xgeXuiToastRequestActiveClose(pToast, iIndex, XGE_XUI_TOAST_CLOSE_CLICK);
		return XGE_XUI_EVENT_CONSUMED;
	}
	return (iIndex >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
}

static void __xgeXuiToastUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_toast pToast;
	int i;

	(void)pWidget;
	pToast = (xge_xui_toast)pUser;
	if ( pToast == NULL ) {
		return;
	}
	__xgeXuiToastProcessClosing(pToast);
	if ( fDelta <= 0.0f ) {
		__xgeXuiToastActivatePending(pToast);
		return;
	}
	i = 0;
	while ( i < pToast->iActiveCount ) {
		if ( pToast->arrActive[i].bClosing != 0 ) {
			i++;
			continue;
		}
		pToast->arrActive[i].fElapsed += fDelta;
		if ( pToast->arrActive[i].fElapsed >= pToast->arrActive[i].fDuration ) {
			__xgeXuiToastRemoveActiveAt(pToast, i, XGE_XUI_TOAST_CLOSE_TIMEOUT);
			continue;
		}
		i++;
	}
	__xgeXuiToastActivatePending(pToast);
}

static void __xgeXuiToastPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_toast pToast;
	xge_xui_toast_item_t* pItem;
	xge_font pFont;
	xge_rect_t tRect;
	xge_rect_t tBand;
	xge_rect_t tIcon;
	xge_rect_t tCloseIcon;
	xge_rect_t tProgress;
	uint32_t iTypeColor;
	uint32_t iCloseColor;
	float fProgress;
	int i;

	(void)pWidget;
	pToast = (xge_xui_toast)pUser;
	if ( pToast == NULL ) {
		return;
	}
	__xgeXuiToastLayout(pToast);
	pFont = __xgeXuiToastFont(pToast);
	for ( i = 0; i < pToast->iActiveCount; i++ ) {
		pItem = &pToast->arrActive[i];
		tRect = pItem->tRect;
		iTypeColor = __xgeXuiToastTypeColor(pToast, pItem->iType);
		__xgeXuiHostDrawSurface(tRect, &pToast->tItemStyle);
		tBand = tRect;
		tBand.fW = 4.0f;
		__xgeXuiHostDrawRect(tBand, iTypeColor);
		tIcon = (xge_rect_t){ tRect.fX + 14.0f, tRect.fY + 14.0f, 22.0f, 22.0f };
		__xgeXuiHostDrawRoundedRect(tIcon, XGE_COLOR_RGBA(XGE_COLOR_GET_R(iTypeColor), XGE_COLOR_GET_G(iTypeColor), XGE_COLOR_GET_B(iTypeColor), 30), 3.0f);
		__xgeXuiBuiltinAssetDraw((xge_rect_t){ tIcon.fX + 4.0f, tIcon.fY + 4.0f, 14.0f, 14.0f }, __xgeXuiToastTypeAsset(pItem->iType), iTypeColor);
		if ( pFont != NULL ) {
			if ( (pItem->sTitle != NULL) && (pItem->sTitle[0] != 0) ) {
				__xgeXuiHostDrawTextRect(pFont, pItem->sTitle, pItem->tTitleRect, pToast->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
			__xgeXuiBoxDrawWrappedText(pFont, (pItem->sMessage != NULL) ? pItem->sMessage : "", pItem->tMessageRect, pToast->iMutedTextColor);
		}
		iCloseColor = (i == pToast->iHoverClose) ? pToast->iCloseHoverColor : pToast->iCloseColor;
		tCloseIcon = (xge_rect_t){ pItem->tCloseRect.fX + 3.0f, pItem->tCloseRect.fY + 3.0f, 10.0f, 10.0f };
		__xgeXuiBuiltinAssetDraw(tCloseIcon, XGE_XUI_ASSET_CLEAR_10, iCloseColor);
		fProgress = (pItem->fDuration > 0.0f) ? (1.0f - pItem->fElapsed / pItem->fDuration) : 0.0f;
		if ( fProgress < 0.0f ) {
			fProgress = 0.0f;
		}
		if ( fProgress > 1.0f ) {
			fProgress = 1.0f;
		}
		tProgress = (xge_rect_t){ tRect.fX + 4.0f, tRect.fY + tRect.fH - 2.0f, (tRect.fW - 4.0f) * fProgress, 2.0f };
		__xgeXuiHostDrawRect(tProgress, XGE_COLOR_RGBA(XGE_COLOR_GET_R(iTypeColor), XGE_COLOR_GET_G(iTypeColor), XGE_COLOR_GET_B(iTypeColor), 90));
	}
}
