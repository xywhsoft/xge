static uint32_t __xgeXuiListViewHoverColor(uint32_t iRow)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiListViewEnsureVisible(xge_xui_list_view pList, int iIndex);

static float __xgeXuiListViewContentHeight(xge_xui_list_view pList)
{
	if ( (pList == NULL) || (pList->iItemCount <= 0) || (pList->fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	return (float)pList->iItemCount * pList->fItemHeight;
}

static int __xgeXuiListViewNormalizeIndex(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiListViewInvalidate(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	xgeXuiWidgetMarkPaint(xgeXuiScrollFrameGetViewportWidget(&pList->tFrame));
	xgeXuiWidgetMarkPaint(pList->pWidget);
}

static void __xgeXuiListViewSyncFrame(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	xgeXuiScrollFrameSetContentSize(&pList->tFrame, 0.0f, __xgeXuiListViewContentHeight(pList));
	xgeXuiScrollFrameSetWheelStep(&pList->tFrame, ((pList->fItemHeight > 0.0f) ? pList->fItemHeight : 24.0f) * 3.0f);
	__xgeXuiListViewClamp(pList);
	if ( (pList->bEnsureSelectedPending != 0) && (pList->iSelected >= 0) ) {
		__xgeXuiListViewEnsureVisible(pList, pList->iSelected);
	}
	__xgeXuiListViewInvalidate(pList);
}

static int __xgeXuiListViewItemEnabled(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return 0;
	}
	if ( (pList->arrEnabled == NULL) || (iIndex >= pList->iEnabledCount) ) {
		return 1;
	}
	return (pList->arrEnabled[iIndex] != 0);
}

static int __xgeXuiListViewHasSelectionBuffer(xge_xui_list_view pList)
{
	return (pList != NULL) && (pList->arrSelected != NULL) && (pList->iSelectionCount > 0);
}

static void __xgeXuiListViewClearSelectionBuffer(xge_xui_list_view pList)
{
	int i;

	if ( __xgeXuiListViewHasSelectionBuffer(pList) == 0 ) {
		return;
	}
	for ( i = 0; i < pList->iSelectionCount; i++ ) {
		pList->arrSelected[i] = 0;
	}
}

static void __xgeXuiListViewSyncSelectionBufferSingle(xge_xui_list_view pList)
{
	if ( __xgeXuiListViewHasSelectionBuffer(pList) == 0 ) {
		return;
	}
	__xgeXuiListViewClearSelectionBuffer(pList);
	if ( (pList->iSelected >= 0) && (pList->iSelected < pList->iSelectionCount) ) {
		pList->arrSelected[pList->iSelected] = 1;
	}
}

static void __xgeXuiListViewApplyRangeSelection(xge_xui_list_view pList, int iFrom, int iTo)
{
	int iBegin;
	int iEnd;
	int i;

	if ( __xgeXuiListViewHasSelectionBuffer(pList) == 0 ) {
		return;
	}
	if ( iFrom > iTo ) {
		iBegin = iTo;
		iEnd = iFrom;
	} else {
		iBegin = iFrom;
		iEnd = iTo;
	}
	if ( iBegin < 0 ) {
		iBegin = 0;
	}
	if ( iEnd >= pList->iSelectionCount ) {
		iEnd = pList->iSelectionCount - 1;
	}
	__xgeXuiListViewClearSelectionBuffer(pList);
	for ( i = iBegin; i <= iEnd; i++ ) {
		if ( __xgeXuiListViewItemEnabled(pList, i) ) {
			pList->arrSelected[i] = 1;
		}
	}
}

static int __xgeXuiListViewIsItemSelected(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return 0;
	}
	if ( (pList->iSelectionMode != XGE_XUI_SELECTION_SINGLE) && __xgeXuiListViewHasSelectionBuffer(pList) && (iIndex < pList->iSelectionCount) ) {
		return pList->arrSelected[iIndex] != 0;
	}
	return iIndex == pList->iSelected;
}

static void __xgeXuiListViewSetHover(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiListViewNormalizeIndex(pList, iIndex);
	if ( pList->iHover == iIndex ) {
		return;
	}
	pList->iHover = iIndex;
	__xgeXuiListViewInvalidate(pList);
}

static void __xgeXuiListViewSetFocusIndex(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiListViewNormalizeIndex(pList, iIndex);
	if ( pList->iFocus == iIndex ) {
		return;
	}
	pList->iFocus = iIndex;
	__xgeXuiListViewInvalidate(pList);
}

static int __xgeXuiListViewNextEnabled(xge_xui_list_view pList, int iStart, int iStep)
{
	int i;

	if ( (pList == NULL) || (pList->iItemCount <= 0) ) {
		return -1;
	}
	if ( iStep == 0 ) {
		iStep = 1;
	}
	if ( iStart < 0 ) {
		iStart = (iStep > 0) ? 0 : (pList->iItemCount - 1);
	}
	if ( iStart >= pList->iItemCount ) {
		iStart = (iStep > 0) ? (pList->iItemCount - 1) : 0;
	}
	for ( i = iStart; (i >= 0) && (i < pList->iItemCount); i += iStep ) {
		if ( __xgeXuiListViewItemEnabled(pList, i) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiListViewVisibleRows(xge_xui_list_view pList)
{
	xge_rect_t tViewport;
	int iRows;

	if ( (pList == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return 1;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pList->tFrame);
	iRows = (int)(tViewport.fH / pList->fItemHeight);
	return (iRows > 0) ? iRows : 1;
}

static void __xgeXuiListViewNotifySelect(xge_xui_list_view pList)
{
	if ( (pList != NULL) && (pList->procSelect != NULL) && (pList->iSelected >= 0) && __xgeXuiListViewItemEnabled(pList, pList->iSelected) ) {
		pList->procSelect(pList->pWidget, pList->iSelected, pList->pSelectUser);
	}
}

static void __xgeXuiListViewEnsureVisible(xge_xui_list_view pList, int iIndex)
{
	xge_rect_t tViewport;
	float fTop;
	float fBottom;
	float fY;

	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) || (pList->fItemHeight <= 0.0f) ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(&pList->tFrame);
	if ( tViewport.fH <= 0.0f ) {
		return;
	}
	pList->bEnsureSelectedPending = 0;
	fTop = (float)iIndex * pList->fItemHeight;
	fBottom = fTop + pList->fItemHeight;
	fY = pList->tScroll.fScrollY;
	if ( fTop < fY ) {
		fY = fTop;
	} else if ( fBottom > fY + tViewport.fH ) {
		fY = fBottom - tViewport.fH;
	}
	if ( xgeXuiScrollFrameSetOffset(&pList->tFrame, 0.0f, fY) ) {
		__xgeXuiListViewInvalidate(pList);
	}
}

static void __xgeXuiListViewSelectWithMode(xge_xui_list_view pList, int iIndex, int iModifiers, int bNotify)
{
	int iOld;
	int bShift;
	int bCtrl;
	int iAnchor;
	int bSelectionChanged;
	int bNewSelected;

	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) || (__xgeXuiListViewItemEnabled(pList, iIndex) == 0) ) {
		return;
	}
	iOld = pList->iSelected;
	bSelectionChanged = 0;
	bShift = (iModifiers & XGE_KEY_MOD_SHIFT) != 0;
	bCtrl = (iModifiers & XGE_KEY_MOD_CTRL) != 0;
	if ( (pList->iSelectionMode == XGE_XUI_SELECTION_RANGE) && bShift && __xgeXuiListViewHasSelectionBuffer(pList) ) {
		iAnchor = pList->iSelectionAnchor;
		if ( iAnchor < 0 ) {
			iAnchor = (pList->iSelected >= 0) ? pList->iSelected : iIndex;
		}
		pList->iSelected = iIndex;
		__xgeXuiListViewApplyRangeSelection(pList, iAnchor, iIndex);
		bSelectionChanged = 1;
	} else if ( (pList->iSelectionMode != XGE_XUI_SELECTION_SINGLE) && bCtrl && __xgeXuiListViewHasSelectionBuffer(pList) ) {
		bNewSelected = pList->arrSelected[iIndex] ? 0 : 1;
		if ( pList->arrSelected[iIndex] != bNewSelected ) {
			bSelectionChanged = 1;
		}
		pList->arrSelected[iIndex] = bNewSelected;
		pList->iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
	} else {
		pList->iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
		__xgeXuiListViewSyncSelectionBufferSingle(pList);
		bSelectionChanged = 1;
	}
	__xgeXuiListViewSetFocusIndex(pList, iIndex);
	pList->bEnsureSelectedPending = 1;
	__xgeXuiListViewEnsureVisible(pList, iIndex);
	__xgeXuiListViewInvalidate(pList);
	if ( bNotify && ((iOld != pList->iSelected) || bSelectionChanged || (pList->bNotifyRepeatSelect != 0)) ) {
		__xgeXuiListViewNotifySelect(pList);
	}
}

static int __xgeXuiListViewForwardScrollBars(xge_xui_list_view pList, const xge_event_t* pEvent)
{
	xge_xui_widget pCapture;
	xge_xui_widget pHWidget;
	xge_xui_widget pVWidget;
	int iRet;

	if ( (pList == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pHWidget = xgeXuiScrollFrameGetHScrollBarWidget(&pList->tFrame);
	pVWidget = xgeXuiScrollFrameGetVScrollBarWidget(&pList->tFrame);
	pCapture = (pList->pContext != NULL) ? xgeXuiGetPointerCapture(pList->pContext, pEvent->iPointerId) : NULL;
	if ( (pHWidget != NULL) && ((pCapture == pHWidget) || __xgeXuiRectContains(pHWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pList->tFrame.tHScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiListViewInvalidate(pList);
			return iRet;
		}
	}
	if ( (pVWidget != NULL) && ((pCapture == pVWidget) || __xgeXuiRectContains(pVWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pList->tFrame.tVScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiListViewInvalidate(pList);
			return iRet;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static void __xgeXuiListViewFrameChanged(xge_xui_scroll_frame pFrame, float fX, float fY, void* pUser)
{
	xge_xui_list_view pList;

	(void)pFrame;
	(void)fX;
	(void)fY;
	pList = (xge_xui_list_view)pUser;
	__xgeXuiListViewInvalidate(pList);
}

static void __xgeXuiListViewViewportPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_list_view pList;
	xge_rect_t tViewport;
	xge_rect_t tRow;
	xge_rect_t tText;
	float fY;
	float fBottom;
	int iIndex;
	int iState;
	uint32_t iRowColor;
	uint32_t iTextColor;

	pList = (xge_xui_list_view)pUser;
	if ( (pWidget == NULL) || (pList == NULL) || (pList->fItemHeight <= 0.0f) ) {
		return;
	}
	tViewport = pWidget->tContentRect;
	if ( (tViewport.fW <= 0.0f) || (tViewport.fH <= 0.0f) ) {
		return;
	}
	if ( XGE_COLOR_GET_A(pList->iBackgroundColor) != 0 ) {
		__xgeXuiHostDrawRect(tViewport, pList->iBackgroundColor);
	}
	iIndex = (int)(pList->tScroll.fScrollY / pList->fItemHeight);
	if ( iIndex < 0 ) {
		iIndex = 0;
	}
	fY = tViewport.fY + ((float)iIndex * pList->fItemHeight) - pList->tScroll.fScrollY;
	fBottom = tViewport.fY + tViewport.fH;
	while ( (iIndex < pList->iItemCount) && (fY < fBottom) ) {
		tRow = (xge_rect_t){ tViewport.fX, fY, tViewport.fW, pList->fItemHeight };
		if ( tRow.fH > 1.0f ) {
			tRow.fH -= 1.0f;
		}
		iRowColor = pList->iRowColor;
		if ( (iIndex == pList->iHover) && __xgeXuiListViewItemEnabled(pList, iIndex) ) {
			iRowColor = pList->iHoverColor;
		}
		if ( __xgeXuiListViewIsItemSelected(pList, iIndex) ) {
			iRowColor = pList->iSelectedColor;
		}
		iState = __xgeXuiListViewIsItemSelected(pList, iIndex) ? XGE_XUI_LIST_ITEM_SELECTED : 0;
		if ( iIndex == pList->iHover ) {
			iState |= XGE_XUI_LIST_ITEM_HOVER;
		}
		if ( __xgeXuiListViewItemEnabled(pList, iIndex) == 0 ) {
			iState |= XGE_XUI_LIST_ITEM_DISABLED;
		}
		if ( (iIndex == pList->iFocus) && (pList->pContext != NULL) && (pList->pContext->pFocus == pList->pWidget) ) {
			iState |= XGE_XUI_LIST_ITEM_FOCUS;
		}
		if ( (pList->procItem != NULL) && (pList->procItem(pList->pWidget, iIndex, tRow, iState, pList->pItemUser) != 0) ) {
			fY += pList->fItemHeight;
			iIndex++;
			continue;
		}
		__xgeXuiHostDrawRect(tRow, iRowColor);
		if ( (pList->pFont != NULL) && (pList->arrItems != NULL) && (pList->arrItems[iIndex] != NULL) ) {
			tText = tRow;
			tText.fX += 4.0f;
			tText.fW -= 8.0f;
			if ( tText.fW < 1.0f ) {
				tText.fW = 1.0f;
			}
			iTextColor = __xgeXuiListViewItemEnabled(pList, iIndex) ? pList->iTextColor : pList->iDisabledTextColor;
			__xgeXuiHostDrawTextRect(pList->pFont, pList->arrItems[iIndex], tText, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		fY += pList->fItemHeight;
		iIndex++;
	}
}

static void __xgeXuiListViewLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_list_view pList;

	(void)pWidget;
	pList = (xge_xui_list_view)pUser;
	__xgeXuiListViewSyncFrame(pList);
}

int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pList == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pList, 0, sizeof(*pList));
	pTheme = xgeXuiGetTheme(pContext);
	pList->pContext = pContext;
	pList->pWidget = pWidget;
	pList->iSelected = -1;
	pList->iHover = -1;
	pList->iFocus = -1;
	pList->iSelectionMode = XGE_XUI_SELECTION_SINGLE;
	pList->iSelectionAnchor = -1;
	pList->fItemHeight = 24.0f;
	pList->iBorderColor = XGE_COLOR_RGBA(184, 223, 245, 255);
	pList->iBackgroundColor = (pTheme != NULL) ? pTheme->iPanelColor : XGE_COLOR_RGBA(236, 246, 253, 255);
	pList->iRowColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pList->iHoverColor = XGE_COLOR_RGBA(232, 243, 252, 255);
	pList->iSelectedColor = XGE_COLOR_RGBA(190, 219, 242, 255);
	pList->iTextColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(76, 96, 116, 255);
	pList->iDisabledTextColor = XGE_COLOR_RGBA(142, 152, 166, 190);
	pList->iBarColor = XGE_COLOR_RGBA(218, 232, 244, 210);
	pList->iThumbColor = XGE_COLOR_RGBA(126, 166, 200, 230);
	xgeXuiScrollModelInit(&pList->tScroll);
	__xgeXuiViewportWidgetInit(pWidget, 1);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetBackground(pWidget, pList->iBackgroundColor);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pList->iBorderColor);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiListViewEventProc, pList);
	xgeXuiWidgetSetLayoutProc(pWidget, __xgeXuiListViewLayoutProc, pList);
	pWidget->pUser = pList;
	if ( xgeXuiScrollFrameInit(&pList->tFrame, pContext, pWidget, &pList->tScroll) != XGE_OK ) {
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
		pWidget->pUser = NULL;
		memset(pList, 0, sizeof(*pList));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiScrollFrameSetScrollbarPolicy(&pList->tFrame, XGE_XUI_SCROLLBAR_POLICY_HIDDEN, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	xgeXuiScrollFrameSetScrollbarMode(&pList->tFrame, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiScrollFrameSetContentDragEnabled(&pList->tFrame, 0);
	xgeXuiScrollFrameSetChange(&pList->tFrame, __xgeXuiListViewFrameChanged, pList);
	xgeXuiScrollFrameSetColors(&pList->tFrame, pList->iBarColor, pList->iThumbColor, pList->iThumbColor, pList->iThumbColor, pList->iThumbColor, pList->iBarColor);
	xgeXuiScrollFrameSetButtonColors(&pList->tFrame, pList->iBarColor, pList->iThumbColor);
	xgeXuiScrollFrameSetCornerColors(&pList->tFrame, pList->iBarColor, pList->iThumbColor);
	xgeXuiWidgetSetPaint(xgeXuiScrollFrameGetViewportWidget(&pList->tFrame), __xgeXuiListViewViewportPaintProc, pList);
	__xgeXuiListViewSyncFrame(pList);
	return XGE_OK;
}

void xgeXuiListViewUnit(xge_xui_list_view pList)
{
	xge_xui_widget pWidget;

	if ( pList == NULL ) {
		return;
	}
	pWidget = pList->pWidget;
	if ( pWidget != NULL && pWidget->pUser == pList ) {
		pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
	}
	xgeXuiScrollFrameUnit(&pList->tFrame);
	memset(pList, 0, sizeof(*pList));
}

void xgeXuiListViewSetItems(xge_xui_list_view pList, const char** arrItems, int iCount)
{
	if ( pList == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pList->arrItems = arrItems;
	pList->iItemCount = iCount;
	if ( pList->iSelected >= iCount ) {
		pList->iSelected = -1;
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	if ( pList->iSelectionAnchor >= iCount ) {
		pList->iSelectionAnchor = -1;
	}
	if ( pList->iHover >= iCount ) {
		__xgeXuiListViewSetHover(pList, -1);
	}
	if ( pList->iFocus >= iCount ) {
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	__xgeXuiListViewSyncSelectionBufferSingle(pList);
	__xgeXuiListViewSyncFrame(pList);
}

void xgeXuiListViewSetEnabledItems(xge_xui_list_view pList, const int* arrEnabled, int iCount)
{
	if ( pList == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pList->arrEnabled = arrEnabled;
	pList->iEnabledCount = iCount;
	if ( (pList->iSelected >= 0) && (__xgeXuiListViewItemEnabled(pList, pList->iSelected) == 0) ) {
		pList->iSelected = -1;
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	__xgeXuiListViewSyncSelectionBufferSingle(pList);
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont)
{
	if ( pList == NULL ) {
		return;
	}
	pList->pFont = pFont;
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight)
{
	if ( pList == NULL ) {
		return;
	}
	if ( fHeight <= 1.0f ) {
		fHeight = 1.0f;
	}
	pList->fItemHeight = fHeight;
	__xgeXuiListViewSyncFrame(pList);
}

void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pList->iItemCount) || ((iIndex >= 0) && (__xgeXuiListViewItemEnabled(pList, iIndex) == 0)) ) {
		iIndex = -1;
	}
	if ( pList->iSelected != iIndex ) {
		pList->iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
		__xgeXuiListViewSyncSelectionBufferSingle(pList);
		__xgeXuiListViewInvalidate(pList);
	}
	__xgeXuiListViewSetFocusIndex(pList, iIndex);
	pList->bEnsureSelectedPending = 1;
	__xgeXuiListViewEnsureVisible(pList, iIndex);
}

int xgeXuiListViewGetSelected(xge_xui_list_view pList)
{
	return (pList != NULL) ? pList->iSelected : -1;
}

void xgeXuiListViewSetSelectionMode(xge_xui_list_view pList, int iMode)
{
	if ( pList == NULL ) {
		return;
	}
	if ( (iMode != XGE_XUI_SELECTION_MULTI) && (iMode != XGE_XUI_SELECTION_RANGE) ) {
		iMode = XGE_XUI_SELECTION_SINGLE;
	}
	pList->iSelectionMode = iMode;
	if ( iMode == XGE_XUI_SELECTION_SINGLE ) {
		__xgeXuiListViewSyncSelectionBufferSingle(pList);
	}
	__xgeXuiListViewInvalidate(pList);
}

int xgeXuiListViewGetSelectionMode(xge_xui_list_view pList)
{
	return (pList != NULL) ? pList->iSelectionMode : XGE_XUI_SELECTION_SINGLE;
}

void xgeXuiListViewSetSelectionBuffer(xge_xui_list_view pList, int* arrSelected, int iCount)
{
	if ( pList == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pList->arrSelected = arrSelected;
	pList->iSelectionCount = iCount;
	__xgeXuiListViewSyncSelectionBufferSingle(pList);
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewClearSelection(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iSelected = -1;
	pList->iSelectionAnchor = -1;
	__xgeXuiListViewSetFocusIndex(pList, -1);
	__xgeXuiListViewClearSelectionBuffer(pList);
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewSetItemSelected(xge_xui_list_view pList, int iIndex, int bSelected)
{
	int i;

	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->iItemCount) ) {
		return;
	}
	if ( (pList->iSelectionMode == XGE_XUI_SELECTION_SINGLE) || (__xgeXuiListViewHasSelectionBuffer(pList) == 0) ) {
		xgeXuiListViewSetSelected(pList, bSelected ? iIndex : -1);
		return;
	}
	if ( iIndex < pList->iSelectionCount ) {
		pList->arrSelected[iIndex] = bSelected ? 1 : 0;
	}
	if ( bSelected ) {
		pList->iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
	} else if ( pList->iSelected == iIndex ) {
		pList->iSelected = -1;
		for ( i = 0; (i < pList->iSelectionCount) && (i < pList->iItemCount); i++ ) {
			if ( pList->arrSelected[i] != 0 ) {
				pList->iSelected = i;
				break;
			}
		}
	}
	__xgeXuiListViewSetFocusIndex(pList, pList->iSelected);
	__xgeXuiListViewInvalidate(pList);
}

int xgeXuiListViewIsItemSelected(xge_xui_list_view pList, int iIndex)
{
	return __xgeXuiListViewIsItemSelected(pList, iIndex);
}

void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY)
{
	if ( pList != NULL ) {
		pList->bEnsureSelectedPending = 0;
		xgeXuiScrollFrameSetOffset(&pList->tFrame, 0.0f, fScrollY);
		__xgeXuiListViewInvalidate(pList);
	}
}

float xgeXuiListViewGetScroll(xge_xui_list_view pList)
{
	return (pList != NULL) ? pList->tScroll.fScrollY : 0.0f;
}

void xgeXuiListViewSetScrollbarMode(xge_xui_list_view pList, int iMode)
{
	if ( pList != NULL ) {
		xgeXuiScrollFrameSetScrollbarMode(&pList->tFrame, iMode);
		__xgeXuiListViewInvalidate(pList);
	}
}

int xgeXuiListViewGetScrollbarMode(xge_xui_list_view pList)
{
	return (pList != NULL) ? xgeXuiScrollFrameGetScrollbarMode(&pList->tFrame) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procSelect = procSelect;
	pList->pSelectUser = pUser;
}

void xgeXuiListViewSetItemRenderer(xge_xui_list_view pList, xge_xui_list_view_item_proc procItem, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procItem = procItem;
	pList->pItemUser = pUser;
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iBackgroundColor = iBackground;
	pList->iRowColor = iRow;
	pList->iHoverColor = __xgeXuiListViewHoverColor(iRow);
	pList->iSelectedColor = iSelected;
	pList->iTextColor = iText;
	pList->iBarColor = iBar;
	pList->iThumbColor = iThumb;
	xgeXuiWidgetSetBackground(pList->pWidget, iBackground);
	xgeXuiScrollFrameSetColors(&pList->tFrame, iBar, iThumb, iThumb, iThumb, iThumb, iBar);
	xgeXuiScrollFrameSetButtonColors(&pList->tFrame, iBar, iThumb);
	xgeXuiScrollFrameSetCornerColors(&pList->tFrame, iBar, iThumb);
	__xgeXuiListViewInvalidate(pList);
}

void xgeXuiListViewSetDisabledTextColor(xge_xui_list_view pList, uint32_t iColor)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iDisabledTextColor = iColor;
	__xgeXuiListViewInvalidate(pList);
}

int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent)
{
	xge_rect_t tViewport;
	int iIndex;
	int iInsideWidget;
	int iRet;
	int iTarget;

	if ( (pList == NULL) || (pList->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pList->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pList->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInsideWidget = __xgeXuiRectContains(pList->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			iRet = __xgeXuiListViewForwardScrollBars(pList, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			iRet = xgeXuiScrollFrameEvent(&pList->tFrame, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				__xgeXuiListViewInvalidate(pList);
			}
			return iRet;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			iRet = __xgeXuiListViewForwardScrollBars(pList, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			tViewport = xgeXuiScrollFrameGetViewportRect(&pList->tFrame);
			if ( __xgeXuiRectContains(tViewport, pEvent->fX, pEvent->fY) ) {
				__xgeXuiListViewSetHover(pList, __xgeXuiListViewIndexAt(pList, pEvent->fY));
			} else {
				__xgeXuiListViewSetHover(pList, -1);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInsideWidget == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pList->pContext, pList->pWidget);
			iRet = __xgeXuiListViewForwardScrollBars(pList, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			tViewport = xgeXuiScrollFrameGetViewportRect(&pList->tFrame);
			if ( __xgeXuiRectContains(tViewport, pEvent->fX, pEvent->fY) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiListViewIndexAt(pList, pEvent->fY);
			if ( iIndex < 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiListViewItemEnabled(pList, iIndex) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiListViewSelectWithMode(pList, iIndex, pEvent->iParam2, 1);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			iRet = __xgeXuiListViewForwardScrollBars(pList, pEvent);
			return iRet;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiListViewSetHover(pList, -1);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pList->pContext == NULL || pList->pContext->pFocus != pList->pWidget || pList->iItemCount <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iTarget = pList->iSelected;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + 1;
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iTarget = (iTarget < 0) ? (pList->iItemCount - 1) : iTarget - 1;
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iTarget = (iTarget < 0) ? 0 : iTarget - __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iTarget = 0;
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iTarget = pList->iItemCount - 1;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiListViewNotifySelect(pList);
				return ((pList->iSelected >= 0) && __xgeXuiListViewItemEnabled(pList, pList->iSelected)) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iTarget < 0 ) {
				iTarget = 0;
			}
			if ( iTarget >= pList->iItemCount ) {
				iTarget = pList->iItemCount - 1;
			}
			iTarget = __xgeXuiListViewNextEnabled(pList, iTarget, (pEvent->iParam1 == XGE_KEY_UP || pEvent->iParam1 == XGE_KEY_PAGE_UP || pEvent->iParam1 == XGE_KEY_END) ? -1 : 1);
			if ( iTarget < 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			__xgeXuiListViewSelectWithMode(pList, iTarget, pEvent->iParam2, 1);
			return XGE_XUI_EVENT_CONSUMED;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiListViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiListViewEvent((xge_xui_list_view)pUser, pEvent);
}

void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_list_view pList;

	(void)pWidget;
	pList = (xge_xui_list_view)pUser;
	if ( pList == NULL ) {
		return;
	}
	__xgeXuiListViewViewportPaintProc(xgeXuiScrollFrameGetViewportWidget(&pList->tFrame), pList);
}
