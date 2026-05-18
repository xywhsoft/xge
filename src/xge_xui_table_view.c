static uint32_t __xgeXuiTableViewHoverColor(uint32_t iRow)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static int __xgeXuiTableViewSyncCount(xge_xui_table_view pTable)
{
	int iCount;

	if ( pTable == NULL ) {
		return 0;
	}
	if ( pTable->procCount != NULL ) {
		iCount = pTable->procCount(pTable->tBase.pWidget, pTable->pAdapterUser);
		if ( iCount < 0 ) {
			iCount = 0;
		}
		pTable->tBase.iItemCount = iCount;
	}
	if ( pTable->tBase.iSelected >= pTable->tBase.iItemCount ) {
		pTable->tBase.iSelected = -1;
	}
	if ( pTable->iHoverRow >= pTable->tBase.iItemCount ) {
		pTable->iHoverRow = -1;
		pTable->tBase.iHover = -1;
	}
	if ( pTable->tBase.iFocus >= pTable->tBase.iItemCount ) {
		pTable->tBase.iFocus = -1;
	}
	return pTable->tBase.iItemCount;
}

static int __xgeXuiTableViewNormalizeRow(xge_xui_table_view pTable, int iRow)
{
	if ( (pTable == NULL) || (iRow < 0) || (iRow >= __xgeXuiTableViewSyncCount(pTable)) ) {
		return -1;
	}
	return iRow;
}

static void __xgeXuiTableViewSetHoverRow(xge_xui_table_view pTable, int iRow)
{
	if ( pTable == NULL ) {
		return;
	}
	iRow = __xgeXuiTableViewNormalizeRow(pTable, iRow);
	if ( (pTable->iHoverRow == iRow) && (pTable->tBase.iHover == iRow) ) {
		return;
	}
	pTable->iHoverRow = iRow;
	pTable->tBase.iHover = iRow;
	pTable->iState = (iRow >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

static void __xgeXuiTableViewSetFocusRow(xge_xui_table_view pTable, int iRow)
{
	if ( pTable == NULL ) {
		return;
	}
	iRow = __xgeXuiTableViewNormalizeRow(pTable, iRow);
	if ( pTable->tBase.iFocus == iRow ) {
		return;
	}
	pTable->tBase.iFocus = iRow;
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

static float __xgeXuiTableViewBodyHeight(xge_xui_table_view pTable)
{
	if ( (pTable == NULL) || (pTable->tBase.pWidget == NULL) ) {
		return 0.0f;
	}
	return pTable->tBase.pWidget->tContentRect.fH - pTable->fHeaderHeight;
}

static float __xgeXuiTableViewMaxScroll(xge_xui_table_view pTable)
{
	float fContent;
	float fView;

	if ( (pTable == NULL) || (pTable->tBase.fItemHeight <= 0.0f) ) {
		return 0.0f;
	}
	fContent = (float)__xgeXuiTableViewSyncCount(pTable) * pTable->tBase.fItemHeight;
	fView = __xgeXuiTableViewBodyHeight(pTable);
	return (fContent > fView) ? (fContent - fView) : 0.0f;
}

static void __xgeXuiTableViewClamp(xge_xui_table_view pTable)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->tBase.fScrollY = __xgeXuiClampFloat(pTable->tBase.fScrollY, 0.0f, __xgeXuiTableViewMaxScroll(pTable));
}

static void __xgeXuiTableViewWindow(xge_xui_table_view pTable, int* pFirst, int* pCount)
{
	int iFirst;
	int iLast;
	int iCount;
	int iRows;
	float fBodyH;

	iFirst = 0;
	iCount = 0;
	iRows = __xgeXuiTableViewSyncCount(pTable);
	if ( (pTable != NULL) && (pTable->tBase.fItemHeight > 0.0f) && (iRows > 0) ) {
		fBodyH = __xgeXuiTableViewBodyHeight(pTable);
		iFirst = (int)(pTable->tBase.fScrollY / pTable->tBase.fItemHeight);
		if ( iFirst < 0 ) {
			iFirst = 0;
		}
		if ( iFirst > iRows ) {
			iFirst = iRows;
		}
		iLast = iFirst + (int)(fBodyH / pTable->tBase.fItemHeight) + 2;
		if ( iLast > iRows ) {
			iLast = iRows;
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

static void __xgeXuiTableViewUpdateWindow(xge_xui_table_view pTable)
{
	if ( pTable != NULL ) {
		__xgeXuiTableViewWindow(pTable, &pTable->iFirstVisible, &pTable->iPaintVisibleCount);
	}
}

static int __xgeXuiTableViewRowAt(xge_xui_table_view pTable, float fY)
{
	int iRow;

	if ( (pTable == NULL) || (pTable->tBase.pWidget == NULL) || (pTable->tBase.fItemHeight <= 0.0f) ) {
		return -1;
	}
	iRow = (int)((fY - pTable->tBase.pWidget->tContentRect.fY - pTable->fHeaderHeight + pTable->tBase.fScrollY) / pTable->tBase.fItemHeight);
	if ( (iRow < 0) || (iRow >= __xgeXuiTableViewSyncCount(pTable)) ) {
		return -1;
	}
	return iRow;
}

static int __xgeXuiTableViewColumnAt(xge_xui_table_view pTable, float fX)
{
	int i;

	if ( pTable == NULL ) {
		return -1;
	}
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		if ( __xgeXuiRectContains(pTable->arrColumns[i].tRect, fX, pTable->arrColumns[i].tRect.fY + 1.0f) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiTableViewResizeColumnAt(xge_xui_table_view pTable, float fX, float fY)
{
	int i;
	float fEdge;

	if ( (pTable == NULL) || (pTable->tBase.pWidget == NULL) || (fY < pTable->tBase.pWidget->tContentRect.fY) || (fY > pTable->tBase.pWidget->tContentRect.fY + pTable->fHeaderHeight) ) {
		return -1;
	}
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		fEdge = pTable->arrColumns[i].tRect.fX + pTable->arrColumns[i].tRect.fW;
		if ( (fX >= fEdge - 3.0f) && (fX <= fEdge + 3.0f) ) {
			return i;
		}
	}
	return -1;
}

static float __xgeXuiTableViewContentWidth(xge_xui_table_view pTable)
{
	float fW;
	int i;

	fW = 0.0f;
	if ( pTable != NULL ) {
		for ( i = 0; i < pTable->iColumnCount; i++ ) {
			fW += pTable->arrColumns[i].fWidth;
		}
	}
	return fW;
}

static float __xgeXuiTableViewThumbLen(float fTrackLen, float fVisible, float fContent)
{
	float fLen;

	if ( (fTrackLen <= 0.0f) || (fVisible <= 0.0f) || (fContent <= fVisible) ) {
		return fTrackLen;
	}
	fLen = fTrackLen * (fVisible / fContent);
	if ( fLen < 8.0f ) {
		fLen = 8.0f;
	}
	return (fLen > fTrackLen) ? fTrackLen : fLen;
}

static int __xgeXuiTableViewBar(xge_xui_table_view pTable, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fBodyH;
	float fMaxScroll;
	float fSize;
	float fButton;
	float fTrackH;

	if ( (pTable == NULL) || (pTable->tBase.pWidget == NULL) ) {
		return 0;
	}
	fBodyH = __xgeXuiTableViewBodyHeight(pTable);
	fContentH = (float)__xgeXuiTableViewSyncCount(pTable) * pTable->tBase.fItemHeight;
	if ( (fBodyH <= 0.0f) || (fContentH <= fBodyH) ) {
		return 0;
	}
	fSize = (pTable->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pTable->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pTable->tBase.pWidget->tContentRect.fX + pTable->tBase.pWidget->tContentRect.fW - fSize;
	tBar.fY = pTable->tBase.pWidget->tContentRect.fY + pTable->fHeaderHeight;
	tBar.fW = fSize;
	tBar.fH = fBodyH;
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
	tThumb.fH = __xgeXuiTableViewThumbLen(fTrackH, fBodyH, fContentH);
	fMaxScroll = __xgeXuiTableViewMaxScroll(pTable);
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pTable->tBase.fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static xge_rect_t __xgeXuiTableViewVisualThumbRect(xge_rect_t tThumb)
{
	return tThumb;
}

static void __xgeXuiTableViewSetScrollFromThumbDrag(xge_xui_table_view pTable, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pTable == NULL) || (__xgeXuiTableViewBar(pTable, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiTableViewMaxScroll(pTable);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiTableViewSetScroll(pTable, pTable->tBase.fDragScrollY + ((fY - pTable->tBase.fDragY) / fTravel) * fMaxScroll);
}

int xgeXuiTableViewInit(xge_xui_table_view pTable, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pTable == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTable, 0, sizeof(*pTable));
	if ( xgeXuiVirtualViewBaseInit(&pTable->tBase, pContext, pWidget) != XGE_OK ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pWidget->pLayoutUser == &pTable->tBase ) {
		pWidget->pLayoutUser = NULL;
		pWidget->procLayout = NULL;
	}
	pTable->tBase.iSelected = -1;
	pTable->iHoverRow = -1;
	pTable->iActiveRow = -1;
	pTable->iSortColumn = -1;
	pTable->iResizeColumn = -1;
	pTable->fHeaderHeight = 24.0f;
	pTable->tBase.fItemHeight = 22.0f;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(235, 244, 252, 255));
	pTable->iHeaderColor = XGE_COLOR_RGBA(210, 231, 247, 255);
	pTable->iHeaderTextColor = XGE_COLOR_RGBA(26, 52, 76, 255);
	pTable->iRowColor = XGE_COLOR_RGBA(245, 250, 255, 255);
	pTable->iAltRowColor = XGE_COLOR_RGBA(238, 247, 254, 255);
	pTable->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	pTable->iSelectedColor = XGE_COLOR_RGBA(187, 220, 248, 255);
	pTable->iGridColor = XGE_COLOR_RGBA(151, 187, 215, 255);
	pTable->iTextColor = XGE_COLOR_RGBA(34, 48, 64, 255);
	pTable->tBase.iBarColor = XGE_COLOR_RGBA(226, 236, 246, 230);
	pTable->tBase.iThumbColor = XGE_COLOR_RGBA(104, 142, 178, 245);
	pTable->tBase.iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTableViewEventProc, NULL);
	pWidget->procPaint = xgeXuiTableViewPaintProc;
	pWidget->pUser = pTable;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiTableViewUnit(xge_xui_table_view pTable)
{
	if ( pTable == NULL ) {
		return;
	}
	xgeXuiReleaseWidgetCapture(pTable->tBase.pContext, pTable->tBase.pWidget);
	if ( pTable->tBase.pWidget != NULL && pTable->tBase.pWidget->pUser == pTable ) {
		pTable->tBase.pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pTable->tBase.pWidget, NULL, NULL);
		pTable->tBase.pWidget->procPaint = NULL;
	}
	memset(pTable, 0, sizeof(*pTable));
}

void xgeXuiTableViewSetColumns(xge_xui_table_view pTable, const xge_xui_table_view_column_t* arrColumns, int iCount)
{
	int i;

	if ( pTable == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY ) {
		iCount = XGE_XUI_TABLE_VIEW_COLUMN_CAPACITY;
	}
	pTable->iColumnCount = iCount;
	for ( i = 0; i < iCount; i++ ) {
		pTable->arrColumns[i] = arrColumns[i];
		if ( pTable->arrColumns[i].fWidth <= 0.0f ) {
			pTable->arrColumns[i].fWidth = 80.0f;
		}
		if ( pTable->arrColumns[i].fMinWidth <= 0.0f ) {
			pTable->arrColumns[i].fMinWidth = 30.0f;
		}
	}
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

void xgeXuiTableViewSetAdapter(xge_xui_table_view pTable, xge_xui_table_view_count_proc procCount, xge_xui_table_view_cell_proc procCell, void* pUser)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->procCount = procCount;
	pTable->procCell = procCell;
	pTable->pAdapterUser = pUser;
	__xgeXuiTableViewSyncCount(pTable);
	__xgeXuiTableViewClamp(pTable);
	__xgeXuiTableViewUpdateWindow(pTable);
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

void xgeXuiTableViewSetSort(xge_xui_table_view pTable, xge_xui_table_view_sort_proc procSort, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procSort = procSort;
		pTable->pSortUser = pUser;
	}
}

void xgeXuiTableViewSetSelect(xge_xui_table_view pTable, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->tBase.procSelect = procSelect;
		pTable->tBase.pSelectUser = pUser;
	}
}

void xgeXuiTableViewSetFont(xge_xui_table_view pTable, xge_font pFont)
{
	if ( pTable != NULL ) {
		pTable->pFont = pFont;
		xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
	}
}

void xgeXuiTableViewSetMetrics(xge_xui_table_view pTable, float fHeaderHeight, float fRowHeight)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->fHeaderHeight = (fHeaderHeight < 1.0f) ? 1.0f : fHeaderHeight;
	pTable->tBase.fItemHeight = (fRowHeight < 1.0f) ? 1.0f : fRowHeight;
	__xgeXuiTableViewClamp(pTable);
	__xgeXuiTableViewUpdateWindow(pTable);
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

void xgeXuiTableViewSetScroll(xge_xui_table_view pTable, float fScrollY)
{
	float fOld;

	if ( pTable == NULL ) {
		return;
	}
	fOld = pTable->tBase.fScrollY;
	pTable->tBase.fScrollY = fScrollY;
	__xgeXuiTableViewClamp(pTable);
	__xgeXuiTableViewUpdateWindow(pTable);
	if ( fOld != pTable->tBase.fScrollY ) {
		xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
	}
}

float xgeXuiTableViewGetScroll(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->tBase.fScrollY : 0.0f;
}

void xgeXuiTableViewSetScrollbarMode(xge_xui_table_view pTable, int iMode)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->tBase.iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

int xgeXuiTableViewGetScrollbarMode(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->tBase.iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiTableViewSetSelected(xge_xui_table_view pTable, int iRow)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( (iRow < 0) || (iRow >= __xgeXuiTableViewSyncCount(pTable)) ) {
		iRow = -1;
	}
	pTable->tBase.iSelected = iRow;
	__xgeXuiTableViewSetFocusRow(pTable, iRow);
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

int xgeXuiTableViewGetSelected(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->tBase.iSelected : -1;
}

int xgeXuiTableViewGetRowCount(xge_xui_table_view pTable)
{
	return __xgeXuiTableViewSyncCount(pTable);
}

int xgeXuiTableViewGetFirstVisible(xge_xui_table_view pTable)
{
	__xgeXuiTableViewUpdateWindow(pTable);
	return (pTable != NULL) ? pTable->iFirstVisible : 0;
}

int xgeXuiTableViewGetPaintVisibleCount(xge_xui_table_view pTable)
{
	__xgeXuiTableViewUpdateWindow(pTable);
	return (pTable != NULL) ? pTable->iPaintVisibleCount : 0;
}

void xgeXuiTableViewSetColors(xge_xui_table_view pTable, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pTable == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pTable->tBase.pWidget, iBackground);
	pTable->iHeaderColor = iHeader;
	pTable->iHeaderTextColor = iText;
	pTable->iRowColor = iRow;
	pTable->iAltRowColor = __xgeXuiTableViewHoverColor(iRow);
	pTable->iHoverColor = __xgeXuiTableViewHoverColor(iRow);
	pTable->iSelectedColor = iSelected;
	pTable->iGridColor = iGrid;
	pTable->iTextColor = iText;
	xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
}

int xgeXuiTableViewEvent(xge_xui_table_view pTable, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iRow;
	int iColumn;
	int iResize;

	if ( (pTable == NULL) || (pTable->tBase.pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pTable->tBase.pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pTable->tBase.pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pTable->tBase.pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiTableViewSetScroll(pTable, pTable->tBase.fScrollY - pEvent->fDY * pTable->tBase.fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_MOVE:
			if ( pTable->tBase.bDraggingThumb != 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId) != pTable->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiTableViewSetScrollFromThumbDrag(pTable, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTable->iResizeColumn >= 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId) != pTable->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				pTable->arrColumns[pTable->iResizeColumn].fWidth = pTable->fDragWidth + (pEvent->fX - pTable->fDragX);
				if ( pTable->arrColumns[pTable->iResizeColumn].fWidth < pTable->arrColumns[pTable->iResizeColumn].fMinWidth ) {
					pTable->arrColumns[pTable->iResizeColumn].fWidth = pTable->arrColumns[pTable->iResizeColumn].fMinWidth;
				}
				xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRow = iInside ? __xgeXuiTableViewRowAt(pTable, pEvent->fY) : -1;
			__xgeXuiTableViewSetHoverRow(pTable, iRow);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTable->tBase.pContext, pTable->tBase.pWidget);
			iResize = __xgeXuiTableViewResizeColumnAt(pTable, pEvent->fX, pEvent->fY);
			if ( iResize >= 0 ) {
				pTable->iResizeColumn = iResize;
				pTable->fDragX = pEvent->fX;
				pTable->fDragWidth = pTable->arrColumns[iResize].fWidth;
				xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, pTable->tBase.pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiTableViewBar(pTable, &tBar, &tThumb) && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pTable->tBase.bDraggingThumb = 1;
					pTable->tBase.fDragY = pEvent->fY;
					pTable->tBase.fDragScrollY = pTable->tBase.fScrollY;
					xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, pTable->tBase.pWidget);
				} else {
					xgeXuiTableViewSetScroll(pTable, pTable->tBase.fScrollY + ((pEvent->fY < tThumb.fY) ? -__xgeXuiTableViewBodyHeight(pTable) : __xgeXuiTableViewBodyHeight(pTable)));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->fY <= pTable->tBase.pWidget->tContentRect.fY + pTable->fHeaderHeight ) {
				iColumn = __xgeXuiTableViewColumnAt(pTable, pEvent->fX);
				if ( iColumn >= 0 ) {
					pTable->bSortDescending = (pTable->iSortColumn == iColumn) ? !pTable->bSortDescending : 0;
					pTable->iSortColumn = iColumn;
					pTable->iSortCount++;
					if ( pTable->procSort != NULL ) {
						pTable->procSort(pTable->tBase.pWidget, iColumn, pTable->bSortDescending, pTable->pSortUser);
					}
					xgeXuiWidgetMarkPaint(pTable->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			iRow = __xgeXuiTableViewRowAt(pTable, pEvent->fY);
			if ( iRow >= 0 ) {
				pTable->iActiveRow = iRow;
				__xgeXuiTableViewSetFocusRow(pTable, iRow);
				xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, pTable->tBase.pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_UP:
			if ( pTable->tBase.bDraggingThumb != 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId) != pTable->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiTableViewSetScrollFromThumbDrag(pTable, pEvent->fY);
				pTable->tBase.bDraggingThumb = 0;
				xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTable->iResizeColumn >= 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId) != pTable->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				pTable->iResizeColumn = -1;
				xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pTable->iActiveRow >= 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId) != pTable->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				iRow = __xgeXuiTableViewRowAt(pTable, pEvent->fY);
				if ( iRow == pTable->iActiveRow ) {
					xgeXuiTableViewSetSelected(pTable, iRow);
					pTable->iSelectCount++;
					if ( pTable->tBase.procSelect != NULL ) {
						pTable->tBase.procSelect(pTable->tBase.pWidget, iRow, pTable->tBase.pSelectUser);
					}
				}
				pTable->iActiveRow = -1;
				xgeXuiSetPointerCapture(pTable->tBase.pContext, pEvent->iPointerId, NULL);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pTable->iResizeColumn = -1;
			pTable->iActiveRow = -1;
			pTable->tBase.bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiTableViewSetHoverRow(pTable, -1);
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTableViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiTableViewEvent((xge_xui_table_view)pUser, pEvent);
}

void xgeXuiTableViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_view pTable;
	xge_rect_t tRect;
	xge_rect_t tText;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	xge_rect_t tLine;
	xge_rect_t tBodyClip;
	char sCell[128];
	float fX;
	float fGridBottom;
	float fFillW;
	float fBodyW;
	float fScrollW;
	int i;
	int iRow;
	int iFirst;
	int iCount;
	int iLast;
	uint32_t iRowColor;

	pTable = (xge_xui_table_view)pUser;
	if ( (pWidget == NULL) || (pTable == NULL) ) {
		return;
	}
	fGridBottom = pWidget->tContentRect.fY + pTable->fHeaderHeight;
	fScrollW = __xgeXuiTableViewBar(pTable, &tBar, &tThumb) ? tBar.fW : 0.0f;
	fFillW = pWidget->tContentRect.fW - fScrollW;
	if ( fFillW < 0.0f ) {
		fFillW = 0.0f;
	}
	fBodyW = __xgeXuiTableViewContentWidth(pTable);
	if ( fBodyW > fFillW ) {
		fBodyW = fFillW;
	}
	if ( fBodyW < 0.0f ) {
		fBodyW = 0.0f;
	}
	tRect.fX = pWidget->tContentRect.fX;
	tRect.fY = pWidget->tContentRect.fY;
	tRect.fW = fFillW;
	tRect.fH = pTable->fHeaderHeight;
	fX = pWidget->tContentRect.fX;
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		tRect.fX = fX;
		tRect.fY = pWidget->tContentRect.fY;
		tRect.fW = pTable->arrColumns[i].fWidth;
		tRect.fH = pTable->fHeaderHeight;
		pTable->arrColumns[i].tRect = tRect;
		fX += pTable->arrColumns[i].fWidth;
	}
	__xgeXuiTableViewWindow(pTable, &iFirst, &iCount);
	pTable->iFirstVisible = iFirst;
	pTable->iPaintVisibleCount = iCount;
	iLast = iFirst + iCount;
	tBodyClip.fX = pWidget->tContentRect.fX;
	tBodyClip.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight;
	tBodyClip.fW = pWidget->tContentRect.fW;
	tBodyClip.fH = pWidget->tContentRect.fH - pTable->fHeaderHeight;
	if ( tBodyClip.fH > 0.0f ) {
		(void)xgeFlush();
		__xgeXuiHostClipSet(tBodyClip);
	}
	for ( iRow = iFirst; iRow < iLast; iRow++ ) {
		iRowColor = (iRow & 1) ? pTable->iAltRowColor : pTable->iRowColor;
		if ( iRow == pTable->tBase.iHover ) {
			iRowColor = pTable->iHoverColor;
		}
		if ( iRow == pTable->tBase.iSelected ) {
			iRowColor = pTable->iSelectedColor;
		}
		tRect.fX = pWidget->tContentRect.fX;
		tRect.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight + (float)iRow * pTable->tBase.fItemHeight - pTable->tBase.fScrollY;
		tRect.fW = fBodyW;
		tRect.fH = pTable->tBase.fItemHeight;
		__xgeXuiHostDrawRect(tRect, iRowColor);
		fX = pWidget->tContentRect.fX;
		for ( i = 0; i < pTable->iColumnCount; i++ ) {
			tRect.fX = fX;
			tRect.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight + (float)iRow * pTable->tBase.fItemHeight - pTable->tBase.fScrollY;
			tRect.fW = pTable->arrColumns[i].fWidth;
			tRect.fH = pTable->tBase.fItemHeight;
			tLine.fX = tRect.fX + tRect.fW - 1.0f;
			tLine.fY = tRect.fY;
			tLine.fW = 1.0f;
			tLine.fH = tRect.fH;
			__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
			sCell[0] = 0;
			if ( pTable->procCell != NULL ) {
				(void)pTable->procCell(pTable->tBase.pWidget, iRow, i, sCell, (int)sizeof(sCell), pTable->pAdapterUser);
			}
			if ( pTable->pFont != NULL && sCell[0] != 0 ) {
				tText = tRect;
				tText.fX += 4.0f;
				tText.fW -= 8.0f;
				__xgeXuiHostDrawTextRect(pTable->pFont, sCell, tText, pTable->iTextColor, pTable->arrColumns[i].iAlign | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
			fX += pTable->arrColumns[i].fWidth;
		}
		tLine.fX = pWidget->tContentRect.fX;
		tLine.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight + (float)(iRow + 1) * pTable->tBase.fItemHeight - pTable->tBase.fScrollY - 1.0f;
		tLine.fW = __xgeXuiTableViewContentWidth(pTable);
		tLine.fH = 1.0f;
		__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
		if ( (tLine.fY + 1.0f) > fGridBottom ) {
			fGridBottom = tLine.fY + 1.0f;
		}
	}
	if ( tBodyClip.fH > 0.0f ) {
		(void)xgeFlush();
		__xgeXuiHostClipClear();
	}
	(void)xgeFlush();
	if ( __xgeXuiTableViewBar(pTable, &tBar, &tThumb) ) {
		if ( pTable->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb = __xgeXuiTableViewVisualThumbRect(tThumb);
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pTable->tBase.iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, pTable->iGridColor);
			__xgeXuiHostDrawRect(__xgeXuiTableViewVisualThumbRect(tThumb), pTable->tBase.iThumbColor);
		}
	}
	tLine.fX = pWidget->tContentRect.fX;
	tLine.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight - 1.0f;
	tLine.fW = __xgeXuiTableViewContentWidth(pTable);
	tLine.fH = 1.0f;
	__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
	tRect.fX = pWidget->tContentRect.fX;
	tRect.fY = pWidget->tContentRect.fY;
	tRect.fW = pWidget->tContentRect.fW;
	tRect.fH = pTable->fHeaderHeight;
	__xgeXuiHostDrawRect(tRect, pTable->iHeaderColor);
	(void)xgeFlush();
	fX = pWidget->tContentRect.fX;
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		tRect.fX = fX;
		tRect.fY = pWidget->tContentRect.fY;
		tRect.fW = pTable->arrColumns[i].fWidth;
		tRect.fH = pTable->fHeaderHeight;
		__xgeXuiHostDrawRect(tRect, pTable->iHeaderColor);
		tLine.fX = tRect.fX + tRect.fW - 1.0f;
		tLine.fY = tRect.fY;
		tLine.fW = 1.0f;
		tLine.fH = tRect.fH;
		__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
		if ( pTable->pFont != NULL && pTable->arrColumns[i].sTitle != NULL ) {
			tText = tRect;
			tText.fX += 4.0f;
			tText.fW -= 8.0f;
			__xgeXuiHostDrawTextRect(pTable->pFont, pTable->arrColumns[i].sTitle, tText, pTable->iHeaderTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		fX += pTable->arrColumns[i].fWidth;
	}
	tLine.fX = pWidget->tContentRect.fX;
	tLine.fY = pWidget->tContentRect.fY + pTable->fHeaderHeight - 1.0f;
	tLine.fW = __xgeXuiTableViewContentWidth(pTable);
	tLine.fH = 1.0f;
	__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pTable->iGridColor);
	(void)__xgeXuiTableViewContentWidth(pTable);
}
