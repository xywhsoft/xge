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

static void __xgeXuiListViewNotifySelect(xge_xui_list_view pList);
static void __xgeXuiListViewEnsureVisible(xge_xui_list_view pList, int iIndex);

static int __xgeXuiListViewNormalizeIndex(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
		return -1;
	}
	return iIndex;
}

static void __xgeXuiListViewSetHover(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiListViewNormalizeIndex(pList, iIndex);
	if ( (pList->iHover == iIndex) && (pList->tBase.iHover == iIndex) ) {
		return;
	}
	pList->iHover = iIndex;
	pList->tBase.iHover = iIndex;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

static void __xgeXuiListViewSetFocusIndex(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	iIndex = __xgeXuiListViewNormalizeIndex(pList, iIndex);
	if ( pList->tBase.iFocus == iIndex ) {
		return;
	}
	pList->tBase.iFocus = iIndex;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

static int __xgeXuiListViewItemEnabled(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
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
	if ( (pList->tBase.iSelected >= 0) && (pList->tBase.iSelected < pList->iSelectionCount) ) {
		pList->arrSelected[pList->tBase.iSelected] = 1;
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

static void __xgeXuiListViewSelectWithMode(xge_xui_list_view pList, int iIndex, int iModifiers, int bNotify)
{
	int iOld;
	int bShift;
	int bCtrl;
	int iAnchor;

	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) || (__xgeXuiListViewItemEnabled(pList, iIndex) == 0) ) {
		return;
	}
	iOld = pList->tBase.iSelected;
	bShift = (iModifiers & XGE_KEY_MOD_SHIFT) != 0;
	bCtrl = (iModifiers & XGE_KEY_MOD_CTRL) != 0;
	if ( (pList->iSelectionMode == XGE_XUI_SELECTION_RANGE) && bShift && __xgeXuiListViewHasSelectionBuffer(pList) ) {
		iAnchor = pList->iSelectionAnchor;
		if ( iAnchor < 0 ) {
			iAnchor = (pList->tBase.iSelected >= 0) ? pList->tBase.iSelected : iIndex;
		}
		pList->tBase.iSelected = iIndex;
		__xgeXuiListViewApplyRangeSelection(pList, iAnchor, iIndex);
	} else if ( (pList->iSelectionMode == XGE_XUI_SELECTION_MULTI) && bCtrl && __xgeXuiListViewHasSelectionBuffer(pList) ) {
		pList->arrSelected[iIndex] = pList->arrSelected[iIndex] ? 0 : 1;
		pList->tBase.iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
	} else {
		pList->tBase.iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
		__xgeXuiListViewSyncSelectionBufferSingle(pList);
	}
	__xgeXuiListViewSetFocusIndex(pList, iIndex);
	__xgeXuiListViewEnsureVisible(pList, iIndex);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
	if ( bNotify && (iOld != pList->tBase.iSelected) ) {
		__xgeXuiListViewNotifySelect(pList);
	}
}

static int __xgeXuiListViewIsItemSelected(xge_xui_list_view pList, int iIndex)
{
	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
		return 0;
	}
	if ( (pList->iSelectionMode != XGE_XUI_SELECTION_SINGLE) && __xgeXuiListViewHasSelectionBuffer(pList) && (iIndex < pList->iSelectionCount) ) {
		return pList->arrSelected[iIndex] != 0;
	}
	return iIndex == pList->tBase.iSelected;
}

static int __xgeXuiListViewNextEnabled(xge_xui_list_view pList, int iStart, int iStep)
{
	int i;

	if ( (pList == NULL) || (pList->tBase.iItemCount <= 0) ) {
		return -1;
	}
	if ( iStep == 0 ) {
		iStep = 1;
	}
	if ( iStart < 0 ) {
		iStart = (iStep > 0) ? 0 : (pList->tBase.iItemCount - 1);
	}
	if ( iStart >= pList->tBase.iItemCount ) {
		iStart = (iStep > 0) ? (pList->tBase.iItemCount - 1) : 0;
	}
	for ( i = iStart; (i >= 0) && (i < pList->tBase.iItemCount); i += iStep ) {
		if ( __xgeXuiListViewItemEnabled(pList, i) ) {
			return i;
		}
	}
	return -1;
}

static void __xgeXuiListViewNotifySelect(xge_xui_list_view pList)
{
	if ( (pList != NULL) && (pList->tBase.procSelect != NULL) && (pList->tBase.iSelected >= 0) && __xgeXuiListViewItemEnabled(pList, pList->tBase.iSelected) ) {
		pList->tBase.procSelect(pList->tBase.pWidget, pList->tBase.iSelected, pList->tBase.pSelectUser);
	}
}

static void __xgeXuiListViewEnsureVisible(xge_xui_list_view pList, int iIndex)
{
	float fTop;
	float fBottom;
	float fViewTop;
	float fViewBottom;

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
		return;
	}
	fTop = (float)iIndex * pList->tBase.fItemHeight;
	fBottom = fTop + pList->tBase.fItemHeight;
	fViewTop = pList->tBase.fScrollY;
	fViewBottom = fViewTop + pList->tBase.pWidget->tContentRect.fH;
	if ( fTop < fViewTop ) {
		xgeXuiListViewSetScroll(pList, fTop);
	} else if ( fBottom > fViewBottom ) {
		xgeXuiListViewSetScroll(pList, fBottom - pList->tBase.pWidget->tContentRect.fH);
	}
}

static void __xgeXuiListViewSelectInternal(xge_xui_list_view pList, int iIndex, int bNotify)
{
	int iOld;

	if ( pList == NULL ) {
		return;
	}
	iOld = pList->tBase.iSelected;
	xgeXuiListViewSetSelected(pList, iIndex);
	if ( pList->tBase.iSelected >= 0 ) {
		__xgeXuiListViewEnsureVisible(pList, pList->tBase.iSelected);
	}
	if ( bNotify && (iOld != pList->tBase.iSelected) ) {
		__xgeXuiListViewNotifySelect(pList);
	}
}

static int __xgeXuiListViewVisibleRows(xge_xui_list_view pList)
{
	int iRows;

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) || (pList->tBase.fItemHeight <= 0.0f) ) {
		return 1;
	}
	iRows = (int)(pList->tBase.pWidget->tContentRect.fH / pList->tBase.fItemHeight);
	return (iRows > 0) ? iRows : 1;
}

static float __xgeXuiListViewThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiListViewBar(xge_xui_list_view pList, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fMaxScroll;
	float fSize;
	float fButton;
	float fTrackH;

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) ) {
		return 0;
	}
	fContentH = (float)pList->tBase.iItemCount * pList->tBase.fItemHeight;
	if ( (fContentH <= pList->tBase.pWidget->tContentRect.fH) || (pList->tBase.pWidget->tContentRect.fH <= 0.0f) ) {
		return 0;
	}
	fSize = (pList->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pList->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pList->tBase.pWidget->tContentRect.fX + pList->tBase.pWidget->tContentRect.fW - fSize;
	tBar.fY = pList->tBase.pWidget->tContentRect.fY + 1.0f;
	tBar.fW = fSize;
	tBar.fH = pList->tBase.pWidget->tContentRect.fH - 2.0f;
	if ( tBar.fH < 1.0f ) {
		tBar.fH = 1.0f;
	}
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	fTrackH = tThumb.fH;
	if ( fTrackH < 1.0f ) {
		fTrackH = 1.0f;
		tThumb.fH = 1.0f;
	}
	tThumb.fH = __xgeXuiListViewThumbLen(fTrackH, pList->tBase.pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiListViewMaxScroll(pList);
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pList->tBase.fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static void __xgeXuiListViewSetScrollFromThumbDrag(xge_xui_list_view pList, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pList == NULL) || (__xgeXuiListViewBar(pList, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiListViewMaxScroll(pList);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiListViewSetScroll(pList, pList->tBase.fDragScrollY + ((fY - pList->tBase.fDragY) / fTravel) * fMaxScroll);
}

int xgeXuiListViewInit(xge_xui_list_view pList, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pList == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pList, 0, sizeof(*pList));
	if ( xgeXuiVirtualScrollViewBaseInit(&pList->tBase, pContext, pWidget) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pWidget->pLayoutUser == &pList->tBase ) {
		pWidget->pLayoutUser = NULL;
		pWidget->procLayout = NULL;
	}
	pTheme = xgeXuiGetTheme(pContext);
	pList->iSelectionMode = XGE_XUI_SELECTION_SINGLE;
	pList->iSelectionAnchor = -1;
	pList->iHover = -1;
	xgeXuiWidgetSetBackground(pWidget, pTheme->iPanelColor);
	pList->iBorderColor = XGE_COLOR_RGBA(184, 223, 245, 255);
	pList->iRowColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pList->iHoverColor = XGE_COLOR_RGBA(232, 243, 252, 255);
	pList->iSelectedColor = XGE_COLOR_RGBA(190, 219, 242, 255);
	pList->iTextColor = pTheme->iTextColor;
	pList->iDisabledTextColor = XGE_COLOR_RGBA(142, 152, 166, 190);
	pList->tBase.iBarColor = XGE_COLOR_RGBA(218, 232, 244, 210);
	pList->tBase.iThumbColor = XGE_COLOR_RGBA(126, 166, 200, 230);
	pList->tBase.iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	pWidget->procEvent = xgeXuiListViewEventProc;
	pWidget->procPaint = xgeXuiListViewPaintProc;
	pWidget->pUser = pList;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiListViewUnit(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pList->tBase.pContext, pList->tBase.pWidget);
	if ( pList->tBase.pWidget != NULL && pList->tBase.pWidget->pUser == pList ) {
		pList->tBase.pWidget->pUser = NULL;
		pList->tBase.pWidget->procEvent = NULL;
		pList->tBase.pWidget->procPaint = NULL;
	}
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
	pList->tBase.iItemCount = iCount;
	if ( pList->tBase.iSelected >= iCount ) {
		pList->tBase.iSelected = -1;
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	if ( pList->iSelectionAnchor >= iCount ) {
		pList->iSelectionAnchor = -1;
	}
	if ( pList->iHover >= iCount ) {
		__xgeXuiListViewSetHover(pList, -1);
	}
	if ( pList->tBase.iFocus >= iCount ) {
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	__xgeXuiListViewSyncSelectionBufferSingle(pList);
	__xgeXuiListViewClamp(pList);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
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
	if ( (pList->tBase.iSelected >= 0) && (__xgeXuiListViewItemEnabled(pList, pList->tBase.iSelected) == 0) ) {
		pList->tBase.iSelected = -1;
		__xgeXuiListViewSetFocusIndex(pList, -1);
	}
	__xgeXuiListViewSyncSelectionBufferSingle(pList);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetFont(xge_xui_list_view pList, xge_font pFont)
{
	if ( pList == NULL ) {
		return;
	}
	pList->pFont = pFont;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetItemHeight(xge_xui_list_view pList, float fHeight)
{
	if ( pList == NULL ) {
		return;
	}
	if ( fHeight < 1.0f ) {
		fHeight = 1.0f;
	}
	pList->tBase.fItemHeight = fHeight;
	__xgeXuiListViewClamp(pList);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetSelected(xge_xui_list_view pList, int iIndex)
{
	if ( pList == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
		iIndex = -1;
	}
	if ( pList->tBase.iSelected != iIndex ) {
		pList->tBase.iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
		__xgeXuiListViewSyncSelectionBufferSingle(pList);
		xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
	}
	__xgeXuiListViewSetFocusIndex(pList, iIndex);
}

int xgeXuiListViewGetSelected(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return -1;
	}
	return pList->tBase.iSelected;
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
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

int xgeXuiListViewGetSelectionMode(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return XGE_XUI_SELECTION_SINGLE;
	}
	return pList->iSelectionMode;
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
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewClearSelection(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return;
	}
	pList->tBase.iSelected = -1;
	pList->iSelectionAnchor = -1;
	__xgeXuiListViewSetFocusIndex(pList, -1);
	__xgeXuiListViewClearSelectionBuffer(pList);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetItemSelected(xge_xui_list_view pList, int iIndex, int bSelected)
{
	int i;

	if ( (pList == NULL) || (iIndex < 0) || (iIndex >= pList->tBase.iItemCount) ) {
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
		pList->tBase.iSelected = iIndex;
		pList->iSelectionAnchor = iIndex;
	} else if ( pList->tBase.iSelected == iIndex ) {
		pList->tBase.iSelected = -1;
		for ( i = 0; (i < pList->iSelectionCount) && (i < pList->tBase.iItemCount); i++ ) {
			if ( pList->arrSelected[i] != 0 ) {
				pList->tBase.iSelected = i;
				break;
			}
		}
	}
	__xgeXuiListViewSetFocusIndex(pList, pList->tBase.iSelected);
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

int xgeXuiListViewIsItemSelected(xge_xui_list_view pList, int iIndex)
{
	return __xgeXuiListViewIsItemSelected(pList, iIndex);
}

void xgeXuiListViewSetScroll(xge_xui_list_view pList, float fScrollY)
{
	float fOld;

	if ( pList == NULL ) {
		return;
	}
	fOld = pList->tBase.fScrollY;
	pList->tBase.fScrollY = fScrollY;
	__xgeXuiListViewClamp(pList);
	if ( fOld != pList->tBase.fScrollY ) {
		xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
	}
}

float xgeXuiListViewGetScroll(xge_xui_list_view pList)
{
	if ( pList == NULL ) {
		return 0.0f;
	}
	return pList->tBase.fScrollY;
}

void xgeXuiListViewSetScrollbarMode(xge_xui_list_view pList, int iMode)
{
	if ( pList == NULL ) {
		return;
	}
	pList->tBase.iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

int xgeXuiListViewGetScrollbarMode(xge_xui_list_view pList)
{
	return (pList != NULL) ? pList->tBase.iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiListViewSetSelect(xge_xui_list_view pList, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->tBase.procSelect = procSelect;
	pList->tBase.pSelectUser = pUser;
}

void xgeXuiListViewSetItemRenderer(xge_xui_list_view pList, xge_xui_list_view_item_proc procItem, void* pUser)
{
	if ( pList == NULL ) {
		return;
	}
	pList->procItem = procItem;
	pList->pItemUser = pUser;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetColors(xge_xui_list_view pList, uint32_t iBackground, uint32_t iRow, uint32_t iSelected, uint32_t iText, uint32_t iBar, uint32_t iThumb)
{
	if ( pList == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pList->tBase.pWidget, iBackground);
	pList->iRowColor = iRow;
	pList->iHoverColor = __xgeXuiListViewHoverColor(iRow);
	pList->iSelectedColor = iSelected;
	pList->iTextColor = iText;
	pList->tBase.iBarColor = iBar;
	pList->tBase.iThumbColor = iThumb;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

void xgeXuiListViewSetDisabledTextColor(xge_xui_list_view pList, uint32_t iColor)
{
	if ( pList == NULL ) {
		return;
	}
	pList->iDisabledTextColor = iColor;
	xgeXuiWidgetMarkPaint(pList->tBase.pWidget);
}

int xgeXuiListViewEvent(xge_xui_list_view pList, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iIndex;
	int iInside;
	int iTarget;

	if ( (pList == NULL) || (pList->tBase.pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pList->tBase.pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pList->tBase.pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pList->tBase.pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiListViewSetScroll(pList, pList->tBase.fScrollY - pEvent->fDY * pList->tBase.fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pList->tBase.bDraggingThumb != 0 ) {
				if ( xgeXuiGetPointerCapture(pList->tBase.pContext, pEvent->iPointerId) != pList->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiListViewSetScrollFromThumbDrag(pList, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = iInside ? __xgeXuiListViewIndexAt(pList, pEvent->fY) : -1;
			__xgeXuiListViewSetHover(pList, iIndex);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pList->tBase.pContext, pList->tBase.pWidget);
			if ( __xgeXuiListViewBar(pList, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pList->tBase.bDraggingThumb = 1;
					pList->tBase.fDragY = pEvent->fY;
					pList->tBase.fDragScrollY = pList->tBase.fScrollY;
					xgeXuiSetPointerCapture(pList->tBase.pContext, pEvent->iPointerId, pList->tBase.pWidget);
				} else {
					xgeXuiListViewSetScroll(pList, pList->tBase.fScrollY + ((pEvent->fY < tThumb.fY) ? -pList->tBase.pWidget->tContentRect.fH : pList->tBase.pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iIndex = __xgeXuiListViewIndexAt(pList, pEvent->fY);
			if ( iIndex >= 0 ) {
				if ( __xgeXuiListViewItemEnabled(pList, iIndex) == 0 ) {
					return XGE_XUI_EVENT_CONSUMED;
				}
				__xgeXuiListViewSelectWithMode(pList, iIndex, pEvent->iParam2, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pList->tBase.bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pList->tBase.pContext, pEvent->iPointerId) != pList->tBase.pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iType == XGE_EVENT_MOUSE_UP) || (pEvent->iType == XGE_EVENT_TOUCH_END) ) {
				__xgeXuiListViewSetScrollFromThumbDrag(pList, pEvent->fY);
			}
			pList->tBase.bDraggingThumb = 0;
			if ( pList->tBase.pContext != NULL && xgeXuiGetPointerCapture(pList->tBase.pContext, pEvent->iPointerId) == pList->tBase.pWidget ) {
				xgeXuiSetPointerCapture(pList->tBase.pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pList->tBase.bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiListViewSetHover(pList, -1);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( pList->tBase.pContext == NULL || pList->tBase.pContext->pFocus != pList->tBase.pWidget || pList->tBase.iItemCount <= 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iTarget = pList->tBase.iSelected;
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + 1;
			} else if ( pEvent->iParam1 == XGE_KEY_UP ) {
				iTarget = (iTarget < 0) ? (pList->tBase.iItemCount - 1) : iTarget - 1;
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				iTarget = (iTarget < 0) ? 0 : iTarget + __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				iTarget = (iTarget < 0) ? 0 : iTarget - __xgeXuiListViewVisibleRows(pList);
			} else if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				iTarget = 0;
			} else if ( pEvent->iParam1 == XGE_KEY_END ) {
				iTarget = pList->tBase.iItemCount - 1;
			} else if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
				__xgeXuiListViewNotifySelect(pList);
				return ((pList->tBase.iSelected >= 0) && __xgeXuiListViewItemEnabled(pList, pList->tBase.iSelected)) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
			} else {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iTarget < 0 ) {
				iTarget = 0;
			}
			if ( iTarget >= pList->tBase.iItemCount ) {
				iTarget = pList->tBase.iItemCount - 1;
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
	(void)pWidget;
	return xgeXuiListViewEvent((xge_xui_list_view)pUser, pEvent);
}

void xgeXuiListViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_list_view pList;
	xge_rect_t tRow;
	xge_rect_t tText;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	xge_rect_t tBorder;
	int iFirst;
	int iLast;
	int i;
	int iState;
	uint32_t iRowColor;
	uint32_t iTextColor;

	pList = (xge_xui_list_view)pUser;
	if ( (pWidget == NULL) || (pList == NULL) ) {
		return;
	}
	if ( (pList->tBase.fItemHeight <= 0.0f) || (pList->tBase.iItemCount <= 0) ) {
		if ( XGE_COLOR_GET_A(pList->iBorderColor) != 0 ) {
			__xgeXuiHostDrawBorderRect(pWidget->tContentRect, 1.0f, pList->iBorderColor);
		}
		return;
	}
	iFirst = (int)(pList->tBase.fScrollY / pList->tBase.fItemHeight);
	if ( iFirst < 0 ) {
		iFirst = 0;
	}
	iLast = iFirst + (int)(pWidget->tContentRect.fH / pList->tBase.fItemHeight) + 2;
	if ( iLast > pList->tBase.iItemCount ) {
		iLast = pList->tBase.iItemCount;
	}
	for ( i = iFirst; i < iLast; i++ ) {
		tRow.fX = pWidget->tContentRect.fX;
		tRow.fY = pWidget->tContentRect.fY + (float)i * pList->tBase.fItemHeight - pList->tBase.fScrollY;
		tRow.fW = pWidget->tContentRect.fW - 6.0f;
		tRow.fH = pList->tBase.fItemHeight - 1.0f;
		if ( tRow.fW < 1.0f ) {
			tRow.fW = 1.0f;
		}
		if ( tRow.fH < 1.0f ) {
			tRow.fH = 1.0f;
		}
		iRowColor = pList->iRowColor;
		if ( (i == pList->tBase.iHover) && __xgeXuiListViewItemEnabled(pList, i) ) {
			iRowColor = pList->iHoverColor;
		}
		if ( __xgeXuiListViewIsItemSelected(pList, i) ) {
			iRowColor = pList->iSelectedColor;
		}
		iState = __xgeXuiListViewIsItemSelected(pList, i) ? XGE_XUI_LIST_ITEM_SELECTED : 0;
		if ( i == pList->tBase.iHover ) {
			iState |= XGE_XUI_LIST_ITEM_HOVER;
		}
		if ( __xgeXuiListViewItemEnabled(pList, i) == 0 ) {
			iState |= XGE_XUI_LIST_ITEM_DISABLED;
		}
		if ( (i == pList->tBase.iFocus) && (pList->tBase.pContext != NULL) && (pList->tBase.pContext->pFocus == pList->tBase.pWidget) ) {
			iState |= XGE_XUI_LIST_ITEM_FOCUS;
		}
		if ( (pList->procItem != NULL) && (pList->procItem(pWidget, i, tRow, iState, pList->pItemUser) != 0) ) {
			continue;
		}
		__xgeXuiHostDrawRect(tRow, iRowColor);
		if ( (pList->pFont != NULL) && (pList->arrItems != NULL) && (pList->arrItems[i] != NULL) ) {
			tText = tRow;
			tText.fX += 4.0f;
			tText.fW -= 8.0f;
			iTextColor = __xgeXuiListViewItemEnabled(pList, i) ? pList->iTextColor : pList->iDisabledTextColor;
			__xgeXuiHostDrawTextRect(pList->pFont, pList->arrItems[i], tText, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	if ( __xgeXuiListViewBar(pList, &tBar, &tThumb) != 0 ) {
		if ( pList->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pList->tBase.iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, pList->iBorderColor);
			__xgeXuiHostDrawRect(tThumb, pList->tBase.iThumbColor);
		}
	}
	if ( XGE_COLOR_GET_A(pList->iBorderColor) != 0 ) {
		tBorder = pWidget->tContentRect;
		if ( tBorder.fW > 0.0f && tBorder.fH > 0.0f ) {
			__xgeXuiHostDrawBorderRect(tBorder, 1.0f, pList->iBorderColor);
		}
	}
}
