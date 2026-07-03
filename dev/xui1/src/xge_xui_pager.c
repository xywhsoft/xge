static int __xgeXuiPagerClampInt(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) {
		return iMin;
	}
	if ( iValue > iMax ) {
		return iMax;
	}
	return iValue;
}

static int __xgeXuiPagerNormalizeWindowSize(int iWindowSize)
{
	int iMax;

	iMax = XGE_XUI_PAGER_ITEM_CAPACITY - 6;
	if ( iMax < 1 ) {
		iMax = 1;
	}
	if ( iWindowSize < 1 ) {
		iWindowSize = 1;
	}
	if ( iWindowSize > iMax ) {
		iWindowSize = iMax;
	}
	if ( (iWindowSize % 2) == 0 ) {
		iWindowSize++;
		if ( iWindowSize > iMax ) {
			iWindowSize -= 2;
		}
	}
	if ( iWindowSize < 1 ) {
		iWindowSize = 1;
	}
	return iWindowSize;
}

static int __xgeXuiPagerClampPage(xge_xui_pager pPager, int iPage)
{
	int iPageCount;

	iPageCount = (pPager != NULL && pPager->iPageCount > 0) ? pPager->iPageCount : 1;
	return __xgeXuiPagerClampInt(iPage, 1, iPageCount);
}

static void __xgeXuiPagerInvalidate(xge_xui_pager pPager, int bLayout)
{
	if ( pPager == NULL ) {
		return;
	}
	if ( bLayout ) {
		xgeXuiWidgetMarkLayout(pPager->pWidget);
	}
	xgeXuiWidgetMarkPaint(pPager->pWidget);
}

static void __xgeXuiPagerSetState(xge_xui_pager pPager, int iState)
{
	int iVisualState;

	if ( pPager == NULL ) {
		return;
	}
	if ( (pPager->pWidget == NULL) || ((pPager->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		iState |= XGE_XUI_STATE_DISABLED;
	}
	if ( pPager->pContext != NULL && pPager->pContext->pFocus == pPager->pWidget ) {
		iState |= XGE_XUI_STATE_FOCUS;
	}
	if ( pPager->iState != iState ) {
		pPager->iState = iState;
		xgeXuiWidgetMarkPaint(pPager->pWidget);
	}
	iVisualState = iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE | XGE_XUI_STATE_FOCUS | XGE_XUI_STATE_DISABLED);
	xgeXuiWidgetSetVisualState(pPager->pWidget, iVisualState);
}

static float __xgeXuiPagerWidthForType(xge_xui_pager pPager, int iType)
{
	if ( pPager == NULL ) {
		return 0.0f;
	}
	if ( iType == XGE_XUI_PAGER_ITEM_PAGE ) {
		return pPager->fPageWidth;
	}
	if ( iType == XGE_XUI_PAGER_ITEM_ELLIPSIS ) {
		return pPager->fEllipsisWidth;
	}
	if ( (iType == XGE_XUI_PAGER_ITEM_PREV) || (iType == XGE_XUI_PAGER_ITEM_NEXT) ) {
		return pPager->fNavWidth;
	}
	return pPager->fTextWidth;
}

static void __xgeXuiPagerAddItem(xge_xui_pager pPager, int iType, int iPage, const char* sText, int bEnabled, xge_rect_t tBase, float* pX)
{
	xge_xui_pager_item_t* pItem;
	float fW;

	if ( (pPager == NULL) || (pX == NULL) || (pPager->iItemCount >= XGE_XUI_PAGER_ITEM_CAPACITY) ) {
		return;
	}
	fW = __xgeXuiPagerWidthForType(pPager, iType);
	if ( fW < 1.0f ) {
		fW = 1.0f;
	}
	pItem = &pPager->arrItem[pPager->iItemCount++];
	memset(pItem, 0, sizeof(*pItem));
	pItem->iType = iType;
	pItem->iPage = iPage;
	pItem->bEnabled = (bEnabled != 0);
	pItem->tRect = (xge_rect_t){ *pX, tBase.fY, fW, tBase.fH };
	snprintf(pItem->sText, sizeof(pItem->sText), "%s", (sText != NULL) ? sText : "");
	pItem->sText[sizeof(pItem->sText) - 1] = 0;
	*pX += fW;
}

static void __xgeXuiPagerAddPageItem(xge_xui_pager pPager, int iPage, xge_rect_t tBase, float* pX)
{
	char sText[16];

	snprintf(sText, sizeof(sText), "%d", iPage);
	sText[sizeof(sText) - 1] = 0;
	__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_PAGE, iPage, sText, iPage != pPager->iCurrentPage, tBase, pX);
}

static float __xgeXuiPagerBuildItems(xge_xui_pager pPager, xge_rect_t tContent)
{
	float fX;
	int iWindowSize;
	int iHalf;
	int iStart;
	int iEnd;
	int iPage;

	if ( pPager == NULL ) {
		return 0.0f;
	}
	pPager->iItemCount = 0;
	if ( tContent.fH <= 0.0f ) {
		tContent.fH = pPager->fItemHeight;
	}
	fX = tContent.fX;
	__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_PREV, pPager->iCurrentPage - 1, pPager->sPrevText, pPager->iCurrentPage > 1, tContent, &fX);
	__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_FIRST, 1, pPager->sFirstText, pPager->iCurrentPage > 1, tContent, &fX);
	iWindowSize = __xgeXuiPagerNormalizeWindowSize(pPager->iWindowSize);
	if ( iWindowSize > pPager->iPageCount ) {
		iWindowSize = pPager->iPageCount;
	}
	iHalf = iWindowSize / 2;
	iStart = pPager->iCurrentPage - iHalf;
	iEnd = iStart + iWindowSize - 1;
	if ( iStart < 1 ) {
		iStart = 1;
		iEnd = iWindowSize;
	}
	if ( iEnd > pPager->iPageCount ) {
		iEnd = pPager->iPageCount;
		iStart = iEnd - iWindowSize + 1;
		if ( iStart < 1 ) {
			iStart = 1;
		}
	}
	if ( iStart > 1 ) {
		__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_ELLIPSIS, 0, "...", 0, tContent, &fX);
	}
	for ( iPage = iStart; iPage <= iEnd; iPage++ ) {
		__xgeXuiPagerAddPageItem(pPager, iPage, tContent, &fX);
	}
	if ( iEnd < pPager->iPageCount ) {
		__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_ELLIPSIS, 0, "...", 0, tContent, &fX);
	}
	__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_LAST, pPager->iPageCount, pPager->sLastText, pPager->iCurrentPage < pPager->iPageCount, tContent, &fX);
	__xgeXuiPagerAddItem(pPager, XGE_XUI_PAGER_ITEM_NEXT, pPager->iCurrentPage + 1, pPager->sNextText, pPager->iCurrentPage < pPager->iPageCount, tContent, &fX);
	return fX - tContent.fX;
}

static int __xgeXuiPagerHitTest(xge_xui_pager pPager, float fX, float fY)
{
	int i;

	if ( pPager == NULL ) {
		return -1;
	}
	__xgeXuiPagerBuildItems(pPager, pPager->pWidget->tContentRect);
	for ( i = 0; i < pPager->iItemCount; i++ ) {
		if ( __xgeXuiRectContains(pPager->arrItem[i].tRect, fX, fY) ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiPagerActivateItem(xge_xui_pager pPager, int iIndex)
{
	xge_xui_pager_item_t* pItem;
	int iTarget;

	if ( (pPager == NULL) || (iIndex < 0) || (iIndex >= pPager->iItemCount) ) {
		return;
	}
	pItem = &pPager->arrItem[iIndex];
	if ( pItem->bEnabled == 0 ) {
		return;
	}
	iTarget = pPager->iCurrentPage;
	if ( pItem->iType == XGE_XUI_PAGER_ITEM_PREV ) {
		iTarget = pPager->iCurrentPage - 1;
	} else if ( pItem->iType == XGE_XUI_PAGER_ITEM_NEXT ) {
		iTarget = pPager->iCurrentPage + 1;
	} else if ( pItem->iType == XGE_XUI_PAGER_ITEM_FIRST ) {
		iTarget = 1;
	} else if ( pItem->iType == XGE_XUI_PAGER_ITEM_LAST ) {
		iTarget = pPager->iPageCount;
	} else if ( pItem->iType == XGE_XUI_PAGER_ITEM_PAGE ) {
		iTarget = pItem->iPage;
	}
	xgeXuiPagerSetCurrent(pPager, iTarget, 1);
}

static uint32_t __xgeXuiPagerItemBackground(xge_xui_pager pPager, int iIndex)
{
	xge_xui_pager_item_t* pItem;

	if ( pPager == NULL ) {
		return 0;
	}
	if ( (pPager->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pPager->iBackgroundColor;
	}
	if ( (iIndex < 0) || (iIndex >= pPager->iItemCount) ) {
		return pPager->iBackgroundColor;
	}
	pItem = &pPager->arrItem[iIndex];
	if ( (pItem->iType == XGE_XUI_PAGER_ITEM_PAGE) && (pItem->iPage == pPager->iCurrentPage) ) {
		return pPager->iCurrentColor;
	}
	if ( (pItem->bEnabled != 0) && (iIndex == pPager->iActive) ) {
		return pPager->iActiveColor;
	}
	if ( (pItem->bEnabled != 0) && (iIndex == pPager->iHover) ) {
		return pPager->iHoverColor;
	}
	return pPager->iBackgroundColor;
}

static uint32_t __xgeXuiPagerItemTextColor(xge_xui_pager pPager, int iIndex)
{
	xge_xui_pager_item_t* pItem;

	if ( pPager == NULL ) {
		return 0;
	}
	if ( (pPager->iState & XGE_XUI_STATE_DISABLED) != 0 ) {
		return pPager->iDisabledTextColor;
	}
	if ( (iIndex < 0) || (iIndex >= pPager->iItemCount) ) {
		return pPager->iTextColor;
	}
	pItem = &pPager->arrItem[iIndex];
	if ( (pItem->iType == XGE_XUI_PAGER_ITEM_PAGE) && (pItem->iPage == pPager->iCurrentPage) ) {
		return pPager->iCurrentTextColor;
	}
	return pItem->bEnabled ? pPager->iTextColor : pPager->iDisabledTextColor;
}

int xgeXuiPagerInit(xge_xui_pager pPager, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pPager == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPager, 0, sizeof(*pPager));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pPager->pContext = pContext;
	pPager->pWidget = pWidget;
	pPager->pFont = (pTheme != NULL) ? pTheme->pFont : NULL;
	pPager->sFirstText = "First";
	pPager->sLastText = "Last";
	pPager->sPrevText = "<";
	pPager->sNextText = ">";
	pPager->iPageCount = 1;
	pPager->iCurrentPage = 1;
	pPager->iWindowSize = 5;
	pPager->iHover = -1;
	pPager->iActive = -1;
	pPager->fItemHeight = 28.0f;
	pPager->fPageWidth = 38.0f;
	pPager->fTextWidth = 56.0f;
	pPager->fNavWidth = 46.0f;
	pPager->fEllipsisWidth = 32.0f;
	pPager->iBackgroundColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPager->iBorderColor = XGE_COLOR_RGBA(214, 226, 236, 255);
	pPager->iTextColor = XGE_COLOR_RGBA(36, 54, 72, 255);
	pPager->iHoverColor = XGE_COLOR_RGBA(239, 247, 252, 255);
	pPager->iActiveColor = XGE_COLOR_RGBA(228, 240, 248, 255);
	pPager->iCurrentColor = XGE_COLOR_RGBA(34, 184, 170, 255);
	pPager->iCurrentTextColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPager->iDisabledTextColor = XGE_COLOR_RGBA(160, 172, 184, 255);
	pPager->iFocusColor = (pTheme != NULL) ? pTheme->iStateFocus : XGE_COLOR_RGBA(150, 198, 232, 255);
	xgeXuiWidgetSetBackground(pWidget, 0);
	xgeXuiWidgetSetBorder(pWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPagerEventProc, pPager);
	pWidget->procMeasure = xgeXuiPagerMeasureProc;
	pWidget->procPaint = xgeXuiPagerPaintProc;
	pWidget->pUser = pPager;
	__xgeXuiPagerSetState(pPager, XGE_XUI_STATE_NORMAL);
	return XGE_OK;
}

void xgeXuiPagerUnit(xge_xui_pager pPager)
{
	if ( pPager == NULL ) {
		return;
	}
	if ( pPager->pWidget != NULL && pPager->pWidget->pUser == pPager ) {
		pPager->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPager->pWidget, NULL, NULL);
		pPager->pWidget->procMeasure = NULL;
		pPager->pWidget->procPaint = NULL;
	}
	memset(pPager, 0, sizeof(*pPager));
}

void xgeXuiPagerSetPageCount(xge_xui_pager pPager, int iPageCount)
{
	if ( pPager == NULL ) {
		return;
	}
	if ( iPageCount < 1 ) {
		iPageCount = 1;
	}
	pPager->iPageCount = iPageCount;
	pPager->iCurrentPage = __xgeXuiPagerClampPage(pPager, pPager->iCurrentPage);
	pPager->iHover = -1;
	pPager->iActive = -1;
	__xgeXuiPagerInvalidate(pPager, 1);
}

void xgeXuiPagerSetCurrent(xge_xui_pager pPager, int iPage, int bNotify)
{
	int iOldPage;

	if ( pPager == NULL ) {
		return;
	}
	iOldPage = pPager->iCurrentPage;
	iPage = __xgeXuiPagerClampPage(pPager, iPage);
	if ( iPage == iOldPage ) {
		return;
	}
	pPager->iCurrentPage = iPage;
	pPager->iHover = -1;
	pPager->iActive = -1;
	pPager->iChangeCount++;
	__xgeXuiPagerInvalidate(pPager, 1);
	if ( (bNotify != 0) && (pPager->procChange != NULL) ) {
		pPager->procChange(pPager->pWidget, iOldPage, pPager->iCurrentPage, pPager->pUser);
	}
}

void xgeXuiPagerSetTotal(xge_xui_pager pPager, int iTotalCount, int iPageSize)
{
	int iPageCount;

	if ( pPager == NULL ) {
		return;
	}
	if ( iTotalCount < 0 ) {
		iTotalCount = 0;
	}
	if ( iPageSize <= 0 ) {
		iPageSize = 1;
	}
	iPageCount = (iTotalCount + iPageSize - 1) / iPageSize;
	if ( iPageCount < 1 ) {
		iPageCount = 1;
	}
	xgeXuiPagerSetPageCount(pPager, iPageCount);
}

int xgeXuiPagerGetCurrent(xge_xui_pager pPager)
{
	return (pPager != NULL) ? pPager->iCurrentPage : 0;
}

int xgeXuiPagerGetPageCount(xge_xui_pager pPager)
{
	return (pPager != NULL) ? pPager->iPageCount : 0;
}

void xgeXuiPagerSetWindowSize(xge_xui_pager pPager, int iWindowSize)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->iWindowSize = __xgeXuiPagerNormalizeWindowSize(iWindowSize);
	__xgeXuiPagerInvalidate(pPager, 1);
}

void xgeXuiPagerSetText(xge_xui_pager pPager, const char* sFirst, const char* sLast, const char* sPrev, const char* sNext)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->sFirstText = (sFirst != NULL) ? sFirst : "First";
	pPager->sLastText = (sLast != NULL) ? sLast : "Last";
	pPager->sPrevText = (sPrev != NULL) ? sPrev : "<";
	pPager->sNextText = (sNext != NULL) ? sNext : ">";
	__xgeXuiPagerInvalidate(pPager, 1);
}

void xgeXuiPagerSetFont(xge_xui_pager pPager, xui_font pFont)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->pFont = pFont;
	__xgeXuiPagerInvalidate(pPager, 1);
}

void xgeXuiPagerSetMetrics(xge_xui_pager pPager, float fItemHeight, float fPageWidth, float fTextWidth, float fNavWidth, float fEllipsisWidth)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->fItemHeight = (fItemHeight > 0.0f) ? fItemHeight : 28.0f;
	pPager->fPageWidth = (fPageWidth > 0.0f) ? fPageWidth : 38.0f;
	pPager->fTextWidth = (fTextWidth > 0.0f) ? fTextWidth : 56.0f;
	pPager->fNavWidth = (fNavWidth > 0.0f) ? fNavWidth : 46.0f;
	pPager->fEllipsisWidth = (fEllipsisWidth > 0.0f) ? fEllipsisWidth : 32.0f;
	__xgeXuiPagerInvalidate(pPager, 1);
}

void xgeXuiPagerSetColors(xge_xui_pager pPager, uint32_t iBackground, uint32_t iBorder, uint32_t iText, uint32_t iHover, uint32_t iActive, uint32_t iCurrent, uint32_t iCurrentText, uint32_t iDisabledText)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->iBackgroundColor = iBackground;
	pPager->iBorderColor = iBorder;
	pPager->iTextColor = iText;
	pPager->iHoverColor = iHover;
	pPager->iActiveColor = iActive;
	pPager->iCurrentColor = iCurrent;
	pPager->iCurrentTextColor = iCurrentText;
	pPager->iDisabledTextColor = iDisabledText;
	__xgeXuiPagerInvalidate(pPager, 0);
}

void xgeXuiPagerSetChange(xge_xui_pager pPager, xge_xui_pager_change_proc procChange, void* pUser)
{
	if ( pPager == NULL ) {
		return;
	}
	pPager->procChange = procChange;
	pPager->pUser = pUser;
}

int xgeXuiPagerEvent(xge_xui_pager pPager, const xge_event_t* pEvent)
{
	int iInside;
	int iHit;
	int iState;
	int iOldActive;

	if ( (pPager == NULL) || (pPager->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pPager->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pPager->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		__xgeXuiPagerSetState(pPager, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pPager->pWidget->tRect, pEvent->fX, pEvent->fY);
	iState = pPager->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			iHit = iInside ? __xgeXuiPagerHitTest(pPager, pEvent->fX, pEvent->fY) : -1;
			pPager->iHover = (iHit >= 0 && pPager->arrItem[iHit].bEnabled) ? iHit : -1;
			__xgeXuiPagerSetState(pPager, iInside ? (iState | XGE_XUI_STATE_HOVER) : (iState & ~XGE_XUI_STATE_HOVER));
			__xgeXuiPagerInvalidate(pPager, 0);
			return ((pPager->iState & XGE_XUI_STATE_ACTIVE) != 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			__xgeXuiPagerSetState(pPager, iState | XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			pPager->iHover = -1;
			__xgeXuiPagerSetState(pPager, iState & ~(XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
			__xgeXuiPagerInvalidate(pPager, 0);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			__xgeXuiPagerSetState(pPager, iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iHit = __xgeXuiPagerHitTest(pPager, pEvent->fX, pEvent->fY);
			if ( (iHit < 0) || (pPager->arrItem[iHit].bEnabled == 0) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPager->pContext, pPager->pWidget);
			xgeXuiSetPointerCapture(pPager->pContext, pEvent->iPointerId, pPager->pWidget);
			pPager->iActive = iHit;
			pPager->iHover = iHit;
			__xgeXuiPagerSetState(pPager, XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE);
			__xgeXuiPagerInvalidate(pPager, 0);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
			iOldActive = pPager->iActive;
			iHit = iInside ? __xgeXuiPagerHitTest(pPager, pEvent->fX, pEvent->fY) : -1;
			pPager->iActive = -1;
			pPager->iHover = (iHit >= 0 && pPager->arrItem[iHit].bEnabled) ? iHit : -1;
			__xgeXuiPagerSetState(pPager, iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL);
			if ( pPager->pContext != NULL && xgeXuiGetPointerCapture(pPager->pContext, pEvent->iPointerId) == pPager->pWidget ) {
				xgeXuiSetPointerCapture(pPager->pContext, pEvent->iPointerId, NULL);
			}
			if ( (iOldActive >= 0) && (iOldActive == iHit) ) {
				__xgeXuiPagerActivateItem(pPager, iHit);
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiPagerInvalidate(pPager, 0);
			return (iOldActive >= 0) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pPager->iActive = -1;
			__xgeXuiPagerSetState(pPager, XGE_XUI_STATE_NORMAL);
			if ( pPager->pContext != NULL && xgeXuiGetPointerCapture(pPager->pContext, pEvent->iPointerId) == pPager->pWidget ) {
				xgeXuiSetPointerCapture(pPager->pContext, pEvent->iPointerId, NULL);
			}
			__xgeXuiPagerInvalidate(pPager, 0);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pPager->pContext == NULL) || (pPager->pContext->pFocus != pPager->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
				xgeXuiPagerSetCurrent(pPager, pPager->iCurrentPage - 1, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
				xgeXuiPagerSetCurrent(pPager, pPager->iCurrentPage + 1, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				xgeXuiPagerSetCurrent(pPager, 1, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_END ) {
				xgeXuiPagerSetCurrent(pPager, pPager->iPageCount, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiPagerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiPagerEvent((xge_xui_pager)pUser, pEvent);
}

xge_vec2_t xgeXuiPagerMeasureProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_pager pPager;
	xge_vec2_t tSize;
	xge_rect_t tMeasure;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPager = (xge_xui_pager)pUser;
	tSize = (xge_vec2_t){ 0.0f, 0.0f };
	if ( pPager == NULL ) {
		return tSize;
	}
	tMeasure = (xge_rect_t){ 0.0f, 0.0f, 0.0f, pPager->fItemHeight };
	tSize.fX = __xgeXuiPagerBuildItems(pPager, tMeasure);
	tSize.fY = pPager->fItemHeight;
	return tSize;
}

void xgeXuiPagerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_pager pPager;
	xge_xui_pager_item_t* pItem;
	xge_rect_t tGroup;
	float fRight;
	int bDisabled;
	int i;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPager = (xge_xui_pager)pUser;
	if ( (pWidget == NULL) || (pPager == NULL) ) {
		return;
	}
	__xgeXuiPagerSetState(pPager, pPager->iState & (XGE_XUI_STATE_HOVER | XGE_XUI_STATE_ACTIVE));
	__xgeXuiPagerBuildItems(pPager, pWidget->tContentRect);
	if ( pPager->iItemCount <= 0 ) {
		return;
	}
	bDisabled = ((pPager->iState & XGE_XUI_STATE_DISABLED) != 0);
	for ( i = 0; i < pPager->iItemCount; i++ ) {
		pItem = &pPager->arrItem[i];
		__xgeXuiHostDrawRect(pItem->tRect, __xgeXuiPagerItemBackground(pPager, i));
		if ( pPager->pFont != NULL && pItem->sText[0] != 0 ) {
			__xgeXuiHostDrawTextRect(pPager->pFont, pItem->sText, pItem->tRect, __xgeXuiPagerItemTextColor(pPager, i), XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	tGroup = pPager->arrItem[0].tRect;
	fRight = pPager->arrItem[pPager->iItemCount - 1].tRect.fX + pPager->arrItem[pPager->iItemCount - 1].tRect.fW;
	tGroup.fW = fRight - tGroup.fX;
	if ( XGE_COLOR_GET_A(pPager->iBorderColor) != 0 ) {
		__xgeXuiHostDrawBorderRect(tGroup, 1.0f, pPager->iBorderColor);
		for ( i = 1; i < pPager->iItemCount; i++ ) {
			if ( (pPager->arrItem[i - 1].iType == XGE_XUI_PAGER_ITEM_PAGE && pPager->arrItem[i - 1].iPage == pPager->iCurrentPage) ||
			     (pPager->arrItem[i].iType == XGE_XUI_PAGER_ITEM_PAGE && pPager->arrItem[i].iPage == pPager->iCurrentPage) ) {
				continue;
			}
			__xgeXuiHostDrawRect((xge_rect_t){ pPager->arrItem[i].tRect.fX, tGroup.fY, 1.0f, tGroup.fH }, pPager->iBorderColor);
		}
	}
	if ( !bDisabled && (pPager->iState & XGE_XUI_STATE_FOCUS) != 0 && XGE_COLOR_GET_A(pPager->iFocusColor) != 0 ) {
		__xgeXuiHostDrawBorderRect(tGroup, 1.0f, pPager->iFocusColor);
	}
}
