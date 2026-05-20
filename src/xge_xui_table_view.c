static uint32_t __xgeXuiTableViewHoverColor(uint32_t iRow)
{
	int iR;
	int iG;
	int iB;

	iR = (int)XGE_COLOR_GET_R(iRow) + 14;
	iG = (int)XGE_COLOR_GET_G(iRow) + 14;
	iB = (int)XGE_COLOR_GET_B(iRow) + 14;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiTableViewInvalidate(xge_xui_table_view pTable)
{
	if ( pTable == NULL ) {
		return;
	}
	xgeXuiWidgetMarkPaint(pTable->pWidget);
	if ( pTable->pFrame != NULL ) {
		xgeXuiWidgetMarkPaint(xgeXuiScrollFrameGetViewportWidget(pTable->pFrame));
	}
}

static float __xgeXuiTableViewColumnWidth(xge_xui_table_view pTable, int iColumn)
{
	float fWidth;

	if ( (pTable == NULL) || (iColumn < 0) || (iColumn >= pTable->iColumnCount) ) {
		return 0.0f;
	}
	if ( pTable->arrColumns[iColumn].bVisible == 0 ) {
		return 0.0f;
	}
	fWidth = pTable->arrColumns[iColumn].fWidth;
	if ( fWidth <= 0.0f ) {
		fWidth = pTable->fDefaultColumnWidth;
	}
	if ( fWidth <= 0.0f ) {
		fWidth = 80.0f;
	}
	return fWidth;
}

static float __xgeXuiTableViewRowHeight(xge_xui_table_view pTable, int iRow)
{
	float fHeight;

	if ( pTable == NULL ) {
		return 0.0f;
	}
	fHeight = 0.0f;
	if ( (pTable->arrRows != NULL) && (iRow >= 0) && (iRow < pTable->iRowStateCount) ) {
		fHeight = pTable->arrRows[iRow].fHeight;
	}
	if ( fHeight <= 0.0f ) {
		fHeight = pTable->fDefaultRowHeight;
	}
	if ( fHeight <= 0.0f ) {
		fHeight = 20.0f;
	}
	return fHeight;
}

static int __xgeXuiTableViewRowDisabled(xge_xui_table_view pTable, int iRow)
{
	if ( (pTable != NULL) && (pTable->arrRows != NULL) && (iRow >= 0) && (iRow < pTable->iRowStateCount) ) {
		return pTable->arrRows[iRow].bDisabled != 0;
	}
	return 0;
}

static int __xgeXuiTableViewSyncRowCount(xge_xui_table_view pTable)
{
	int iCount;

	if ( pTable == NULL ) {
		return 0;
	}
	if ( pTable->procCount != NULL ) {
		iCount = pTable->procCount(pTable->pWidget, pTable->pAdapterUser);
		if ( iCount < 0 ) {
			iCount = 0;
		}
		pTable->iRowCount = iCount;
	}
	if ( pTable->iSelectedRow >= pTable->iRowCount ) {
		pTable->iSelectedRow = -1;
		pTable->iSelectedColumn = -1;
	}
	if ( pTable->iHoverRow >= pTable->iRowCount ) {
		pTable->iHoverRow = -1;
		pTable->iHoverColumn = -1;
	}
	if ( pTable->iFocusRow >= pTable->iRowCount ) {
		pTable->iFocusRow = -1;
		pTable->iFocusColumn = -1;
	}
	return pTable->iRowCount;
}

static float __xgeXuiTableViewContentWidth(xge_xui_table_view pTable)
{
	float fW;
	int i;

	fW = 0.0f;
	if ( pTable != NULL ) {
		for ( i = 0; i < pTable->iColumnCount; i++ ) {
			fW += __xgeXuiTableViewColumnWidth(pTable, i);
		}
	}
	return fW;
}

static float __xgeXuiTableViewRowTop(xge_xui_table_view pTable, int iRow)
{
	float fY;
	int i;

	fY = 0.0f;
	if ( pTable == NULL ) {
		return 0.0f;
	}
	for ( i = 0; (i < iRow) && (i < pTable->iRowCount); i++ ) {
		fY += __xgeXuiTableViewRowHeight(pTable, i);
	}
	return fY;
}

static float __xgeXuiTableViewContentHeight(xge_xui_table_view pTable)
{
	float fH;
	int i;
	int iCount;

	fH = 0.0f;
	iCount = __xgeXuiTableViewSyncRowCount(pTable);
	for ( i = 0; i < iCount; i++ ) {
		fH += __xgeXuiTableViewRowHeight(pTable, i);
	}
	return fH;
}

static int __xgeXuiTableViewColumnAtContentX(xge_xui_table_view pTable, float fContentX)
{
	float fX;
	float fW;
	int i;

	if ( pTable == NULL ) {
		return -1;
	}
	fX = 0.0f;
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		fW = __xgeXuiTableViewColumnWidth(pTable, i);
		if ( fW <= 0.0f ) {
			continue;
		}
		if ( (fContentX >= fX) && (fContentX < fX + fW) ) {
			return i;
		}
		fX += fW;
	}
	return -1;
}

static int __xgeXuiTableViewRowAtContentY(xge_xui_table_view pTable, float fContentY)
{
	float fY;
	float fH;
	int i;
	int iCount;

	iCount = __xgeXuiTableViewSyncRowCount(pTable);
	fY = 0.0f;
	for ( i = 0; i < iCount; i++ ) {
		fH = __xgeXuiTableViewRowHeight(pTable, i);
		if ( (fContentY >= fY) && (fContentY < fY + fH) ) {
			return i;
		}
		fY += fH;
	}
	return -1;
}

static float __xgeXuiTableViewColumnLeft(xge_xui_table_view pTable, int iColumn)
{
	float fX;
	int i;

	fX = 0.0f;
	if ( pTable == NULL ) {
		return 0.0f;
	}
	for ( i = 0; (i < iColumn) && (i < pTable->iColumnCount); i++ ) {
		fX += __xgeXuiTableViewColumnWidth(pTable, i);
	}
	return fX;
}

static int __xgeXuiTableViewNextVisibleColumn(xge_xui_table_view pTable, int iColumn, int iStep)
{
	int i;

	if ( (pTable == NULL) || (iStep == 0) ) {
		return -1;
	}
	i = iColumn;
	while ( (i >= 0) && (i < pTable->iColumnCount) ) {
		if ( __xgeXuiTableViewColumnWidth(pTable, i) > 0.0f ) {
			return i;
		}
		i += iStep;
	}
	return -1;
}

static int __xgeXuiTableViewFirstVisibleColumn(xge_xui_table_view pTable)
{
	return __xgeXuiTableViewNextVisibleColumn(pTable, 0, 1);
}

static int __xgeXuiTableViewLastVisibleColumn(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? __xgeXuiTableViewNextVisibleColumn(pTable, pTable->iColumnCount - 1, -1) : -1;
}

static void __xgeXuiTableViewDefaultCell(xge_xui_table_view pTable, int iColumn, xge_xui_table_view_cell_t* pCell)
{
	if ( pCell == NULL ) {
		return;
	}
	memset(pCell, 0, sizeof(*pCell));
	pCell->iRowSpan = 1;
	pCell->iColSpan = 1;
	pCell->iType = ((pTable != NULL) && (iColumn >= 0) && (iColumn < pTable->iColumnCount)) ? pTable->arrColumns[iColumn].iType : XGE_XUI_TABLE_CELL_TYPE_TEXT;
}

static const char* __xgeXuiTableViewCellText(xge_xui_table_view pTable, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, char* sBuffer, int iSize)
{
	xge_xui_table_view_format_proc procFormatter;
	void* pUser;

	if ( (pCell == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return NULL;
	}
	sBuffer[0] = 0;
	if ( (pCell->sText != NULL) && (pCell->sText[0] != 0) ) {
		return pCell->sText;
	}
	procFormatter = pCell->procFormatter;
	pUser = pCell->pFormatterUser;
	if ( (procFormatter == NULL) && (pTable != NULL) && (iColumn >= 0) && (iColumn < pTable->iColumnCount) ) {
		procFormatter = pTable->arrColumns[iColumn].procFormatter;
		pUser = pTable->arrColumns[iColumn].pFormatterUser;
	}
	if ( procFormatter != NULL ) {
		if ( procFormatter((pTable != NULL) ? pTable->pWidget : NULL, iRow, iColumn, pCell, sBuffer, iSize, pUser) != 0 ) {
			sBuffer[iSize - 1] = 0;
			return sBuffer;
		}
		sBuffer[0] = 0;
	}
	return NULL;
}

static void __xgeXuiTableViewGetCell(xge_xui_table_view pTable, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell)
{
	int iRowSpan;
	int iColSpan;

	__xgeXuiTableViewDefaultCell(pTable, iColumn, pCell);
	if ( (pTable == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) ) {
		return;
	}
	if ( pTable->procCell != NULL ) {
		(void)pTable->procCell(pTable->pWidget, iRow, iColumn, pCell, pTable->pAdapterUser);
	}
	if ( pTable->procMerge != NULL ) {
		iRowSpan = pCell->iRowSpan;
		iColSpan = pCell->iColSpan;
		if ( pTable->procMerge(pTable->pWidget, iRow, iColumn, &iRowSpan, &iColSpan, pTable->pMergeUser) != 0 ) {
			pCell->iRowSpan = iRowSpan;
			pCell->iColSpan = iColSpan;
		}
	}
	if ( pCell->iRowSpan <= 0 ) {
		pCell->iRowSpan = 1;
	}
	if ( pCell->iColSpan <= 0 ) {
		pCell->iColSpan = 1;
	}
	if ( iRow + pCell->iRowSpan > pTable->iRowCount ) {
		pCell->iRowSpan = pTable->iRowCount - iRow;
	}
	if ( iColumn + pCell->iColSpan > pTable->iColumnCount ) {
		pCell->iColSpan = pTable->iColumnCount - iColumn;
	}
	if ( pCell->iRowSpan <= 0 ) {
		pCell->iRowSpan = 1;
	}
	if ( pCell->iColSpan <= 0 ) {
		pCell->iColSpan = 1;
	}
}

static int __xgeXuiTableViewMergeOwner(xge_xui_table_view pTable, int iRow, int iColumn, int* pOwnerRow, int* pOwnerColumn)
{
	xge_xui_table_view_cell_t tCell;
	int r;
	int c;

	if ( pOwnerRow != NULL ) {
		*pOwnerRow = iRow;
	}
	if ( pOwnerColumn != NULL ) {
		*pOwnerColumn = iColumn;
	}
	if ( (pTable == NULL) || (iRow < 0) || (iColumn < 0) ) {
		return 0;
	}
	for ( r = 0; r <= iRow; r++ ) {
		for ( c = 0; c <= iColumn; c++ ) {
			__xgeXuiTableViewGetCell(pTable, r, c, &tCell);
			if ( (tCell.iRowSpan <= 1) && (tCell.iColSpan <= 1) ) {
				continue;
			}
			if ( (iRow >= r) && (iRow < r + tCell.iRowSpan) && (iColumn >= c) && (iColumn < c + tCell.iColSpan) ) {
				if ( pOwnerRow != NULL ) {
					*pOwnerRow = r;
				}
				if ( pOwnerColumn != NULL ) {
					*pOwnerColumn = c;
				}
				return (r != iRow) || (c != iColumn);
			}
		}
	}
	return 0;
}

static float __xgeXuiTableViewSpanWidth(xge_xui_table_view pTable, int iColumn, int iColSpan)
{
	float fW;
	int i;

	fW = 0.0f;
	if ( iColSpan <= 0 ) {
		iColSpan = 1;
	}
	for ( i = 0; (i < iColSpan) && (iColumn + i < pTable->iColumnCount); i++ ) {
		fW += __xgeXuiTableViewColumnWidth(pTable, iColumn + i);
	}
	return fW;
}

static float __xgeXuiTableViewSpanHeight(xge_xui_table_view pTable, int iRow, int iRowSpan)
{
	float fH;
	int i;

	fH = 0.0f;
	if ( iRowSpan <= 0 ) {
		iRowSpan = 1;
	}
	for ( i = 0; (i < iRowSpan) && (iRow + i < pTable->iRowCount); i++ ) {
		fH += __xgeXuiTableViewRowHeight(pTable, iRow + i);
	}
	return fH;
}

static xge_rect_t __xgeXuiTableViewCellContentRect(xge_xui_table_view pTable, int iRow, int iColumn, int iRowSpan, int iColSpan)
{
	xge_rect_t tRect;

	memset(&tRect, 0, sizeof(tRect));
	if ( pTable == NULL ) {
		return tRect;
	}
	tRect.fX = __xgeXuiTableViewColumnLeft(pTable, iColumn);
	tRect.fY = __xgeXuiTableViewRowTop(pTable, iRow);
	tRect.fW = __xgeXuiTableViewSpanWidth(pTable, iColumn, iColSpan);
	tRect.fH = __xgeXuiTableViewSpanHeight(pTable, iRow, iRowSpan);
	return tRect;
}

static int __xgeXuiTableViewRectIntersects(xge_rect_t tA, xge_rect_t tB)
{
	return (tA.fX < tB.fX + tB.fW) && (tA.fX + tA.fW > tB.fX) && (tA.fY < tB.fY + tB.fH) && (tA.fY + tA.fH > tB.fY);
}

static xge_rect_t __xgeXuiTableViewContentToScreenRect(xge_xui_table_view pTable, xge_rect_t tContent)
{
	xge_rect_t tViewport;

	tViewport = xgeXuiScrollFrameGetViewportRect(pTable->pFrame);
	tContent.fX = tViewport.fX + tContent.fX - pTable->tScroll.fScrollX;
	tContent.fY = tViewport.fY + tContent.fY - pTable->tScroll.fScrollY;
	return tContent;
}

static int __xgeXuiTableViewVisibleRowStart(xge_xui_table_view pTable, float* pY)
{
	float fY;
	float fScrollY;
	int i;
	int iCount;

	fY = 0.0f;
	fScrollY = (pTable != NULL) ? pTable->tScroll.fScrollY : 0.0f;
	iCount = __xgeXuiTableViewSyncRowCount(pTable);
	for ( i = 0; i < iCount; i++ ) {
		if ( fY + __xgeXuiTableViewRowHeight(pTable, i) > fScrollY ) {
			if ( pY != NULL ) {
				*pY = fY;
			}
			return i;
		}
		fY += __xgeXuiTableViewRowHeight(pTable, i);
	}
	if ( pY != NULL ) {
		*pY = fY;
	}
	return iCount;
}

static void __xgeXuiTableViewSyncFrame(xge_xui_table_view pTable)
{
	if ( pTable == NULL ) {
		return;
	}
	__xgeXuiTableViewSyncRowCount(pTable);
	xgeXuiScrollFrameSetContentSize(pTable->pFrame, __xgeXuiTableViewContentWidth(pTable), __xgeXuiTableViewContentHeight(pTable));
	xgeXuiScrollFrameSetWheelStep(pTable->pFrame, ((pTable->fDefaultRowHeight > 0.0f) ? pTable->fDefaultRowHeight : 20.0f) * 3.0f);
}

static void __xgeXuiTableViewFrameChanged(xge_xui_scroll_frame pFrame, float fX, float fY, void* pUser)
{
	xge_xui_table_view pTable;

	(void)pFrame;
	(void)fX;
	(void)fY;
	pTable = (xge_xui_table_view)pUser;
	__xgeXuiTableViewInvalidate(pTable);
}

static void __xgeXuiTableViewLayoutProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_view pTable;
	xge_rect_t tBody;

	(void)pWidget;
	pTable = (xge_xui_table_view)pUser;
	if ( (pTable == NULL) || (pTable->pWidget == NULL) || (pTable->pBodyWidget == NULL) ) {
		return;
	}
	tBody = pTable->pWidget->tContentRect;
	tBody.fY += pTable->fHeaderHeight;
	tBody.fH -= pTable->fHeaderHeight;
	if ( tBody.fH < 0.0f ) {
		tBody.fH = 0.0f;
	}
	pTable->pBodyWidget->tLocalRect = tBody;
	__xgeXuiWidgetArrangeRect(pTable->pBodyWidget, tBody);
	__xgeXuiTableViewSyncFrame(pTable);
	xgeXuiScrollFrameLayout(pTable->pFrame);
}

static int __xgeXuiTableViewForwardScrollBars(xge_xui_table_view pTable, const xge_event_t* pEvent)
{
	xge_xui_widget pCapture;
	xge_xui_widget pHWidget;
	xge_xui_widget pVWidget;
	int iRet;

	if ( (pTable == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	pHWidget = xgeXuiScrollFrameGetHScrollBarWidget(pTable->pFrame);
	pVWidget = xgeXuiScrollFrameGetVScrollBarWidget(pTable->pFrame);
	pCapture = (pTable->pContext != NULL) ? xgeXuiGetPointerCapture(pTable->pContext, pEvent->iPointerId) : NULL;
	if ( (pHWidget != NULL) && ((pCapture == pHWidget) || __xgeXuiRectContains(pHWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pTable->pFrame->tHScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiTableViewInvalidate(pTable);
			return iRet;
		}
	}
	if ( (pVWidget != NULL) && ((pCapture == pVWidget) || __xgeXuiRectContains(pVWidget->tRect, pEvent->fX, pEvent->fY)) ) {
		iRet = xgeXuiScrollBarEvent(&pTable->pFrame->tVScrollBar, pEvent);
		if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
			__xgeXuiTableViewInvalidate(pTable);
			return iRet;
		}
	}
	return XGE_XUI_EVENT_CONTINUE;
}

static xge_rect_t __xgeXuiTableViewHeaderRect(xge_xui_table_view pTable)
{
	xge_rect_t tViewport;
	xge_rect_t tHeader;

	memset(&tHeader, 0, sizeof(tHeader));
	if ( (pTable == NULL) || (pTable->pWidget == NULL) ) {
		return tHeader;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pTable->pFrame);
	if ( tViewport.fW <= 0.0f && tViewport.fH <= 0.0f ) {
		tViewport = pTable->pWidget->tContentRect;
	}
	tHeader = tViewport;
	tHeader.fY = pTable->pWidget->tContentRect.fY;
	tHeader.fH = pTable->fHeaderHeight;
	return tHeader;
}

static int __xgeXuiTableViewHeaderColumnAt(xge_xui_table_view pTable, float fX, float fY)
{
	xge_rect_t tHeader;
	float fContentX;

	tHeader = __xgeXuiTableViewHeaderRect(pTable);
	if ( __xgeXuiRectContains(tHeader, fX, fY) == 0 ) {
		return -1;
	}
	fContentX = fX - tHeader.fX + pTable->tScroll.fScrollX;
	return __xgeXuiTableViewColumnAtContentX(pTable, fContentX);
}

static int __xgeXuiTableViewHeaderResizeAt(xge_xui_table_view pTable, float fX, float fY)
{
	xge_rect_t tHeader;
	float fLeft;
	float fRight;
	int i;

	if ( pTable == NULL ) {
		return -1;
	}
	tHeader = __xgeXuiTableViewHeaderRect(pTable);
	if ( __xgeXuiRectContains(tHeader, fX, fY) == 0 ) {
		return -1;
	}
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		if ( (pTable->arrColumns[i].bVisible == 0) || (pTable->arrColumns[i].bResizable == 0) ) {
			continue;
		}
		fLeft = tHeader.fX + __xgeXuiTableViewColumnLeft(pTable, i) - pTable->tScroll.fScrollX;
		fRight = fLeft + __xgeXuiTableViewColumnWidth(pTable, i);
		if ( (fX >= fRight - 4.0f) && (fX <= fRight + 4.0f) ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiTableViewHitCell(xge_xui_table_view pTable, float fX, float fY, int* pRow, int* pColumn)
{
	xge_rect_t tViewport;
	float fContentX;
	float fContentY;
	int iRow;
	int iColumn;
	int iOwnerRow;
	int iOwnerColumn;

	if ( pRow != NULL ) {
		*pRow = -1;
	}
	if ( pColumn != NULL ) {
		*pColumn = -1;
	}
	if ( pTable == NULL ) {
		return 0;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pTable->pFrame);
	if ( __xgeXuiRectContains(tViewport, fX, fY) == 0 ) {
		return 0;
	}
	fContentX = fX - tViewport.fX + pTable->tScroll.fScrollX;
	fContentY = fY - tViewport.fY + pTable->tScroll.fScrollY;
	iColumn = __xgeXuiTableViewColumnAtContentX(pTable, fContentX);
	iRow = __xgeXuiTableViewRowAtContentY(pTable, fContentY);
	if ( (iRow < 0) || (iColumn < 0) ) {
		return 0;
	}
	(void)__xgeXuiTableViewMergeOwner(pTable, iRow, iColumn, &iOwnerRow, &iOwnerColumn);
	if ( pRow != NULL ) {
		*pRow = iOwnerRow;
	}
	if ( pColumn != NULL ) {
		*pColumn = iOwnerColumn;
	}
	return 1;
}

static int __xgeXuiTableViewCellDisabled(xge_xui_table_view pTable, int iRow, int iColumn)
{
	xge_xui_table_view_cell_t tCell;

	if ( __xgeXuiTableViewRowDisabled(pTable, iRow) ) {
		return 1;
	}
	__xgeXuiTableViewGetCell(pTable, iRow, iColumn, &tCell);
	return tCell.bDisabled != 0;
}

static int __xgeXuiTableViewTooltipResolve(xge_xui_context pContext, xge_xui_widget pWidget, xge_xui_tooltip_desc pDesc, void* pUser)
{
	xge_xui_table_view pTable;
	xge_xui_table_view_cell_t tCell;
	int iRow;
	int iColumn;

	(void)pWidget;
	pTable = (xge_xui_table_view)pUser;
	if ( (pContext == NULL) || (pTable == NULL) || (pDesc == NULL) ) {
		return 0;
	}
	if ( __xgeXuiTableViewHitCell(pTable, pContext->fTooltipMouseX, pContext->fTooltipMouseY, &iRow, &iColumn) == 0 ) {
		return 0;
	}
	__xgeXuiTableViewGetCell(pTable, iRow, iColumn, &tCell);
	if ( (tCell.sTooltip == NULL) || (tCell.sTooltip[0] == 0) ) {
		return 0;
	}
	memset(pDesc, 0, sizeof(*pDesc));
	pDesc->iType = XGE_XUI_TOOLTIP_TEXT;
	pDesc->sText = tCell.sTooltip;
	pDesc->iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	pDesc->fOffsetX = 12.0f;
	pDesc->fOffsetY = 18.0f;
	pDesc->fDelay = 0.35f;
	pDesc->bFollowCursor = 1;
	return 1;
}

static void __xgeXuiTableViewNotifySelect(xge_xui_table_view pTable)
{
	if ( (pTable == NULL) || (pTable->procSelect == NULL) ) {
		return;
	}
	pTable->iSelectCount++;
	pTable->procSelect(pTable->pWidget, pTable->iSelectedRow, pTable->iSelectedColumn, pTable->iSelectionMode, pTable->pSelectUser);
}

static void __xgeXuiTableViewSetHover(xge_xui_table_view pTable, int iRow, int iColumn)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( (pTable->iHoverRow == iRow) && (pTable->iHoverColumn == iColumn) ) {
		return;
	}
	pTable->iHoverRow = iRow;
	pTable->iHoverColumn = iColumn;
	if ( pTable->procHover != NULL ) {
		pTable->procHover(pTable->pWidget, iRow, iColumn, pTable->iSelectionMode, pTable->pHoverUser);
	}
	__xgeXuiTableViewInvalidate(pTable);
}

static void __xgeXuiTableViewSetFocusCell(xge_xui_table_view pTable, int iRow, int iColumn)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( (pTable->iFocusRow == iRow) && (pTable->iFocusColumn == iColumn) ) {
		return;
	}
	pTable->iFocusRow = iRow;
	pTable->iFocusColumn = iColumn;
	__xgeXuiTableViewInvalidate(pTable);
}

static void __xgeXuiTableViewSelect(xge_xui_table_view pTable, int iRow, int iColumn, int bNotify)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( (iRow < 0) || (iRow >= __xgeXuiTableViewSyncRowCount(pTable)) ) {
		iRow = -1;
		iColumn = -1;
	}
	if ( iRow >= 0 ) {
		if ( pTable->iSelectionMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW ) {
			iColumn = -1;
			if ( __xgeXuiTableViewRowDisabled(pTable, iRow) ) {
				return;
			}
		} else {
			if ( (iColumn < 0) || (iColumn >= pTable->iColumnCount) || __xgeXuiTableViewCellDisabled(pTable, iRow, iColumn) ) {
				return;
			}
		}
	}
	if ( (pTable->iSelectedRow == iRow) && (pTable->iSelectedColumn == iColumn) ) {
		if ( bNotify ) {
			__xgeXuiTableViewNotifySelect(pTable);
		}
		return;
	}
	pTable->iSelectedRow = iRow;
	pTable->iSelectedColumn = iColumn;
	pTable->iActiveRow = iRow;
	pTable->iActiveColumn = iColumn;
	__xgeXuiTableViewSetFocusCell(pTable, iRow, iColumn);
	xgeXuiTableViewEnsureCellVisible(pTable, iRow, (iColumn >= 0) ? iColumn : __xgeXuiTableViewFirstVisibleColumn(pTable));
	__xgeXuiTableViewInvalidate(pTable);
	if ( bNotify ) {
		__xgeXuiTableViewNotifySelect(pTable);
	}
}

static int __xgeXuiTableViewVisibleRows(xge_xui_table_view pTable)
{
	xge_rect_t tViewport;
	float fRow;

	if ( pTable == NULL ) {
		return 1;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pTable->pFrame);
	fRow = (pTable->fDefaultRowHeight > 0.0f) ? pTable->fDefaultRowHeight : 20.0f;
	if ( fRow <= 0.0f ) {
		return 1;
	}
	return (int)(tViewport.fH / fRow) > 0 ? (int)(tViewport.fH / fRow) : 1;
}

static int __xgeXuiTableViewMoveSelection(xge_xui_table_view pTable, int iKey)
{
	int iRow;
	int iColumn;
	int iStep;

	if ( (pTable == NULL) || (__xgeXuiTableViewSyncRowCount(pTable) <= 0) ) {
		return 0;
	}
	iRow = pTable->iSelectedRow;
	iColumn = pTable->iSelectedColumn;
	if ( iRow < 0 ) {
		iRow = 0;
	}
	if ( pTable->iSelectionMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW ) {
		iColumn = -1;
		if ( iKey == XGE_KEY_DOWN ) {
			iRow++;
		} else if ( iKey == XGE_KEY_UP ) {
			iRow--;
		} else if ( iKey == XGE_KEY_PAGE_DOWN ) {
			iRow += __xgeXuiTableViewVisibleRows(pTable);
		} else if ( iKey == XGE_KEY_PAGE_UP ) {
			iRow -= __xgeXuiTableViewVisibleRows(pTable);
		} else if ( iKey == XGE_KEY_HOME ) {
			iRow = 0;
		} else if ( iKey == XGE_KEY_END ) {
			iRow = pTable->iRowCount - 1;
		} else {
			return 0;
		}
		if ( iRow < 0 ) {
			iRow = 0;
		}
		if ( iRow >= pTable->iRowCount ) {
			iRow = pTable->iRowCount - 1;
		}
		__xgeXuiTableViewSelect(pTable, iRow, -1, 1);
		return 1;
	}
	if ( iColumn < 0 ) {
		iColumn = __xgeXuiTableViewFirstVisibleColumn(pTable);
	}
	if ( iKey == XGE_KEY_DOWN ) {
		iRow++;
	} else if ( iKey == XGE_KEY_UP ) {
		iRow--;
	} else if ( iKey == XGE_KEY_RIGHT ) {
		iStep = iColumn + 1;
		iColumn = __xgeXuiTableViewNextVisibleColumn(pTable, iStep, 1);
	} else if ( iKey == XGE_KEY_LEFT ) {
		iStep = iColumn - 1;
		iColumn = __xgeXuiTableViewNextVisibleColumn(pTable, iStep, -1);
	} else if ( iKey == XGE_KEY_PAGE_DOWN ) {
		iRow += __xgeXuiTableViewVisibleRows(pTable);
	} else if ( iKey == XGE_KEY_PAGE_UP ) {
		iRow -= __xgeXuiTableViewVisibleRows(pTable);
	} else if ( iKey == XGE_KEY_HOME ) {
		iColumn = __xgeXuiTableViewFirstVisibleColumn(pTable);
	} else if ( iKey == XGE_KEY_END ) {
		iColumn = __xgeXuiTableViewLastVisibleColumn(pTable);
	} else {
		return 0;
	}
	if ( iRow < 0 ) {
		iRow = 0;
	}
	if ( iRow >= pTable->iRowCount ) {
		iRow = pTable->iRowCount - 1;
	}
	if ( iColumn < 0 ) {
		iColumn = __xgeXuiTableViewFirstVisibleColumn(pTable);
	}
	__xgeXuiTableViewSelect(pTable, iRow, iColumn, 1);
	return 1;
}

static uint32_t __xgeXuiTableViewCellBackground(xge_xui_table_view pTable, int iRow, int iColumn, int iState)
{
	(void)iColumn;
	if ( (iState & XGE_XUI_TABLE_CELL_DISABLED) != 0 ) {
		return pTable->iDisabledColor;
	}
	if ( (iState & XGE_XUI_TABLE_CELL_SELECTED) != 0 ) {
		return pTable->iSelectedColor;
	}
	if ( (iState & XGE_XUI_TABLE_CELL_HOVER) != 0 ) {
		return pTable->iHoverColor;
	}
	return (iRow & 1) ? pTable->iAltRowColor : pTable->iRowColor;
}

static void __xgeXuiTableViewResolveCellColors(xge_xui_table_view pTable, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, int iState, uint32_t* pBackground, uint32_t* pText, uint32_t* pGrid)
{
	uint32_t iBackground;
	uint32_t iText;
	uint32_t iGrid;

	iBackground = (pTable != NULL) ? ((iRow & 1) ? pTable->iAltRowColor : pTable->iRowColor) : 0;
	iText = (pTable != NULL) ? pTable->iTextColor : 0;
	iGrid = (pTable != NULL) ? pTable->iGridColor : 0;
	if ( (pTable != NULL) && (iColumn >= 0) && (iColumn < pTable->iColumnCount) && (pTable->arrColumns[iColumn].bHasStyle != 0) ) {
		if ( XGE_COLOR_GET_A(pTable->arrColumns[iColumn].tStyle.iBackgroundColor) != 0 ) {
			iBackground = pTable->arrColumns[iColumn].tStyle.iBackgroundColor;
		}
		if ( XGE_COLOR_GET_A(pTable->arrColumns[iColumn].tStyle.iBorderColor) != 0 ) {
			iGrid = pTable->arrColumns[iColumn].tStyle.iBorderColor;
		}
	}
	if ( (pTable != NULL) && (pTable->arrRows != NULL) && (iRow >= 0) && (iRow < pTable->iRowStateCount) && (pTable->arrRows[iRow].bHasStyle != 0) ) {
		if ( XGE_COLOR_GET_A(pTable->arrRows[iRow].tStyle.iBackgroundColor) != 0 ) {
			iBackground = pTable->arrRows[iRow].tStyle.iBackgroundColor;
		}
		if ( XGE_COLOR_GET_A(pTable->arrRows[iRow].tStyle.iBorderColor) != 0 ) {
			iGrid = pTable->arrRows[iRow].tStyle.iBorderColor;
		}
	}
	if ( (pCell != NULL) && (pCell->bHasStyle != 0) ) {
		if ( XGE_COLOR_GET_A(pCell->tStyle.iBackgroundColor) != 0 ) {
			iBackground = pCell->tStyle.iBackgroundColor;
		}
		if ( XGE_COLOR_GET_A(pCell->tStyle.iBorderColor) != 0 ) {
			iGrid = pCell->tStyle.iBorderColor;
		}
	}
	if ( (pTable != NULL) && ((iState & XGE_XUI_TABLE_CELL_DISABLED) != 0) ) {
		iBackground = pTable->iDisabledColor;
		iText = pTable->iDisabledTextColor;
	} else if ( (pTable != NULL) && ((iState & XGE_XUI_TABLE_CELL_SELECTED) != 0) ) {
		iBackground = pTable->iSelectedColor;
	} else if ( (pTable != NULL) && ((iState & XGE_XUI_TABLE_CELL_HOVER) != 0) ) {
		iBackground = pTable->iHoverColor;
	}
	if ( pBackground != NULL ) {
		*pBackground = iBackground;
	}
	if ( pText != NULL ) {
		*pText = iText;
	}
	if ( pGrid != NULL ) {
		*pGrid = iGrid;
	}
}

static int __xgeXuiTableViewCellState(xge_xui_table_view pTable, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell)
{
	int iState;

	iState = 0;
	if ( pTable->iSelectionMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW ) {
		if ( iRow == pTable->iSelectedRow ) {
			iState |= XGE_XUI_TABLE_CELL_SELECTED;
		}
		if ( iRow == pTable->iHoverRow ) {
			iState |= XGE_XUI_TABLE_CELL_HOVER;
		}
		if ( (iRow == pTable->iFocusRow) && (pTable->pContext != NULL) && (pTable->pContext->pFocus == pTable->pWidget) ) {
			iState |= XGE_XUI_TABLE_CELL_FOCUS;
		}
	} else {
		if ( (iRow == pTable->iSelectedRow) && (iColumn == pTable->iSelectedColumn) ) {
			iState |= XGE_XUI_TABLE_CELL_SELECTED;
		}
		if ( (iRow == pTable->iHoverRow) && (iColumn == pTable->iHoverColumn) ) {
			iState |= XGE_XUI_TABLE_CELL_HOVER;
		}
		if ( (iRow == pTable->iFocusRow) && (iColumn == pTable->iFocusColumn) && (pTable->pContext != NULL) && (pTable->pContext->pFocus == pTable->pWidget) ) {
			iState |= XGE_XUI_TABLE_CELL_FOCUS;
		}
	}
	if ( __xgeXuiTableViewRowDisabled(pTable, iRow) || ((pCell != NULL) && (pCell->bDisabled != 0)) ) {
		iState |= XGE_XUI_TABLE_CELL_DISABLED;
	}
	if ( (pCell != NULL) && (pCell->bEditing != 0) ) {
		iState |= XGE_XUI_TABLE_CELL_EDITING;
	}
	if ( (pCell != NULL) && (pCell->bInvalid != 0) ) {
		iState |= XGE_XUI_TABLE_CELL_INVALID;
	}
	if ( (pCell != NULL) && (pCell->bDirty != 0) ) {
		iState |= XGE_XUI_TABLE_CELL_DIRTY;
	}
	if ( (pTable != NULL) && (pTable->arrRows != NULL) && (iRow >= 0) && (iRow < pTable->iRowStateCount) && (pTable->arrRows[iRow].bSelected != 0) ) {
		iState |= XGE_XUI_TABLE_CELL_SELECTED;
	}
	return iState;
}

static void __xgeXuiTableViewPaintCellText(xge_xui_table_view pTable, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, uint32_t iTextColor)
{
	xge_rect_t tText;
	const char* sText;
	char sBuffer[128];
	int iAlign;

	if ( (pTable == NULL) || (pTable->pFont == NULL) || (pCell == NULL) ) {
		return;
	}
	sText = __xgeXuiTableViewCellText(pTable, iRow, iColumn, pCell, sBuffer, (int)sizeof(sBuffer));
	if ( (sText == NULL) || (sText[0] == 0) ) {
		return;
	}
	tText = tRect;
	tText.fX += 5.0f;
	tText.fW -= 10.0f;
	if ( tText.fW < 1.0f ) {
		tText.fW = 1.0f;
	}
	iAlign = ((iColumn >= 0) && (iColumn < pTable->iColumnCount)) ? pTable->arrColumns[iColumn].iAlign : XGE_TEXT_ALIGN_LEFT;
	(void)iState;
	__xgeXuiHostDrawTextRect(pTable->pFont, sText, tText, iTextColor, iAlign | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
}

static void __xgeXuiTableViewViewportPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_view pTable;
	xge_xui_table_view_cell_t tCell;
	xge_rect_t tViewport;
	xge_rect_t tCellContent;
	xge_rect_t tCellScreen;
	xge_rect_t tVisibleContent;
	xge_rect_t tLine;
	float fContentY;
	float fY;
	float fBottom;
	int iRow;
	int iColumn;
	int iOwnerRow;
	int iOwnerColumn;
	int iState;
	int iHandled;
	uint32_t iBackground;
	uint32_t iText;
	uint32_t iGrid;

	pTable = (xge_xui_table_view)pUser;
	if ( (pWidget == NULL) || (pTable == NULL) ) {
		return;
	}
	tViewport = pWidget->tContentRect;
	if ( (tViewport.fW <= 0.0f) || (tViewport.fH <= 0.0f) ) {
		return;
	}
	__xgeXuiTableViewSyncRowCount(pTable);
	if ( XGE_COLOR_GET_A(pTable->iHeaderColor) != 0 ) {
		__xgeXuiHostDrawRect(tViewport, pTable->iBackgroundColor);
	}
	iRow = __xgeXuiTableViewVisibleRowStart(pTable, &fContentY);
	fY = tViewport.fY + fContentY - pTable->tScroll.fScrollY;
	fBottom = tViewport.fY + tViewport.fH;
	pTable->iFirstVisible = iRow;
	pTable->iPaintVisibleCount = 0;
	tVisibleContent = (xge_rect_t){ pTable->tScroll.fScrollX, pTable->tScroll.fScrollY, tViewport.fW, tViewport.fH };
	while ( (iRow < pTable->iRowCount) && (fY < fBottom) ) {
		for ( iColumn = 0; iColumn < pTable->iColumnCount; iColumn++ ) {
			if ( __xgeXuiTableViewColumnWidth(pTable, iColumn) <= 0.0f ) {
				continue;
			}
			if ( __xgeXuiTableViewMergeOwner(pTable, iRow, iColumn, &iOwnerRow, &iOwnerColumn) ) {
				continue;
			}
			__xgeXuiTableViewGetCell(pTable, iRow, iColumn, &tCell);
			tCellContent = __xgeXuiTableViewCellContentRect(pTable, iRow, iColumn, tCell.iRowSpan, tCell.iColSpan);
			if ( __xgeXuiTableViewRectIntersects(tCellContent, tVisibleContent) == 0 ) {
				continue;
			}
			tCellScreen = __xgeXuiTableViewContentToScreenRect(pTable, tCellContent);
			iState = __xgeXuiTableViewCellState(pTable, iRow, iColumn, &tCell);
			if ( (tCell.iRowSpan > 1) || (tCell.iColSpan > 1) ) {
				iState |= XGE_XUI_TABLE_CELL_MERGED;
			}
			__xgeXuiTableViewResolveCellColors(pTable, iRow, iColumn, &tCell, iState, &iBackground, &iText, &iGrid);
			__xgeXuiHostDrawRect(tCellScreen, iBackground);
			iHandled = 0;
			if ( tCell.procRenderer != NULL ) {
				iHandled = tCell.procRenderer(pTable->pWidget, iRow, iColumn, &tCell, tCellScreen, iState, tCell.pRendererUser);
			} else if ( pTable->arrColumns[iColumn].procRenderer != NULL ) {
				iHandled = pTable->arrColumns[iColumn].procRenderer(pTable->pWidget, iRow, iColumn, &tCell, tCellScreen, iState, pTable->arrColumns[iColumn].pRendererUser);
			} else if ( pTable->procCellRenderer != NULL ) {
				iHandled = pTable->procCellRenderer(pTable->pWidget, iRow, iColumn, &tCell, tCellScreen, iState, pTable->pCellRendererUser);
			}
			if ( iHandled == 0 ) {
				__xgeXuiTableViewPaintCellText(pTable, iRow, iColumn, &tCell, tCellScreen, iState, iText);
			}
			tLine = (xge_rect_t){ tCellScreen.fX + tCellScreen.fW - 1.0f, tCellScreen.fY, 1.0f, tCellScreen.fH };
			__xgeXuiHostDrawRect(tLine, iGrid);
			tLine = (xge_rect_t){ tCellScreen.fX, tCellScreen.fY + tCellScreen.fH - 1.0f, tCellScreen.fW, 1.0f };
			__xgeXuiHostDrawRect(tLine, iGrid);
			if ( (iState & XGE_XUI_TABLE_CELL_FOCUS) != 0 ) {
				__xgeXuiHostDrawBorderRect(tCellScreen, 1.0f, pTable->iFocusRingColor);
			}
			if ( (iState & XGE_XUI_TABLE_CELL_INVALID) != 0 ) {
				__xgeXuiHostDrawBorderRect(tCellScreen, 1.0f, XGE_COLOR_RGBA(220, 64, 64, 230));
			} else if ( (iState & XGE_XUI_TABLE_CELL_EDITING) != 0 ) {
				__xgeXuiHostDrawBorderRect(tCellScreen, 1.0f, XGE_COLOR_RGBA(74, 142, 210, 230));
			}
			if ( (iState & XGE_XUI_TABLE_CELL_DIRTY) != 0 ) {
				tLine = (xge_rect_t){ tCellScreen.fX + tCellScreen.fW - 5.0f, tCellScreen.fY + 1.0f, 4.0f, 4.0f };
				__xgeXuiHostDrawRect(tLine, XGE_COLOR_RGBA(240, 166, 52, 230));
			}
		}
		fContentY += __xgeXuiTableViewRowHeight(pTable, iRow);
		fY = tViewport.fY + fContentY - pTable->tScroll.fScrollY;
		iRow++;
		pTable->iPaintVisibleCount++;
	}
}

int xgeXuiTableViewInit(xge_xui_table_view pTable, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;

	if ( (pTable == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pTable, 0, sizeof(*pTable));
	pTheme = xgeXuiGetTheme(pContext);
	pTable->pContext = pContext;
	pTable->pWidget = pWidget;
	pTable->iSelectionMode = XGE_XUI_TABLE_VIEW_SELECTION_CELL;
	pTable->iSelectedRow = -1;
	pTable->iSelectedColumn = -1;
	pTable->iHoverRow = -1;
	pTable->iHoverColumn = -1;
	pTable->iFocusRow = -1;
	pTable->iFocusColumn = -1;
	pTable->iActiveRow = -1;
	pTable->iActiveColumn = -1;
	pTable->iSortColumn = -1;
	pTable->iResizeColumn = -1;
	pTable->fDefaultColumnWidth = 80.0f;
	pTable->fDefaultRowHeight = 20.0f;
	pTable->fHeaderHeight = 24.0f;
	pTable->iBackgroundColor = (pTheme != NULL) ? pTheme->iPanelColor : XGE_COLOR_RGBA(236, 246, 253, 255);
	pTable->iHeaderColor = XGE_COLOR_RGBA(224, 240, 251, 255);
	pTable->iHeaderTextColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(54, 76, 98, 255);
	pTable->iRowColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pTable->iAltRowColor = XGE_COLOR_RGBA(242, 247, 252, 255);
	pTable->iHoverColor = XGE_COLOR_RGBA(232, 243, 252, 255);
	pTable->iSelectedColor = XGE_COLOR_RGBA(190, 219, 242, 255);
	pTable->iDisabledColor = XGE_COLOR_RGBA(238, 242, 246, 180);
	pTable->iGridColor = XGE_COLOR_RGBA(184, 211, 232, 255);
	pTable->iTextColor = (pTheme != NULL) ? pTheme->iTextColor : XGE_COLOR_RGBA(76, 96, 116, 255);
	pTable->iDisabledTextColor = XGE_COLOR_RGBA(142, 152, 166, 190);
	pTable->iFocusRingColor = XGE_COLOR_RGBA(74, 142, 210, 210);
	pTable->iBarColor = XGE_COLOR_RGBA(218, 232, 244, 210);
	pTable->iThumbColor = XGE_COLOR_RGBA(126, 166, 200, 230);
	pTable->pBodyWidget = xgeXuiWidgetCreate();
	if ( pTable->pBodyWidget == NULL ) {
		memset(pTable, 0, sizeof(*pTable));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pTable->pFrame = (xge_xui_scroll_frame_t*)calloc(1, sizeof(xge_xui_scroll_frame_t));
	if ( pTable->pFrame == NULL ) {
		xgeXuiWidgetFree(pTable->pBodyWidget);
		memset(pTable, 0, sizeof(*pTable));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiScrollModelInit(&pTable->tScroll);
	__xgeXuiViewportWidgetInit(pWidget, 1);
	xgeXuiWidgetSetLayout(pWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetBackground(pWidget, pTable->iBackgroundColor);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pTable->iGridColor);
	xgeXuiWidgetSetPaddingPx(pWidget, 2.0f, 2.0f, 2.0f, 2.0f);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTableViewEventProc, pTable);
	xgeXuiWidgetSetLayoutProc(pWidget, __xgeXuiTableViewLayoutProc, pTable);
	xgeXuiWidgetSetPaint(pWidget, xgeXuiTableViewPaintProc, pTable);
	pWidget->pUser = pTable;
	__xgeXuiViewportWidgetInit(pTable->pBodyWidget, 0);
	xgeXuiWidgetSetLayout(pTable->pBodyWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetClip(pTable->pBodyWidget, 1);
	xgeXuiWidgetSetBackground(pTable->pBodyWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBorder(pTable->pBodyWidget, 0.0f, 0);
	xgeXuiWidgetSetTooltipResolver(pTable->pBodyWidget, __xgeXuiTableViewTooltipResolve, pTable);
	if ( xgeXuiWidgetAddInternal(pWidget, pTable->pBodyWidget) != XGE_OK ) {
		free(pTable->pFrame);
		xgeXuiWidgetFree(pTable->pBodyWidget);
		memset(pTable, 0, sizeof(*pTable));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( xgeXuiScrollFrameInit(pTable->pFrame, pContext, pTable->pBodyWidget, &pTable->tScroll) != XGE_OK ) {
		free(pTable->pFrame);
		xgeXuiWidgetFree(pTable->pBodyWidget);
		memset(pTable, 0, sizeof(*pTable));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetLayoutProc(pTable->pBodyWidget, xgeXuiScrollFrameLayoutProc, pTable->pFrame);
	xgeXuiScrollFrameSetScrollbarPolicy(pTable->pFrame, XGE_XUI_SCROLLBAR_POLICY_AUTO, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	xgeXuiScrollFrameSetScrollbarMode(pTable->pFrame, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiScrollFrameSetContentDragEnabled(pTable->pFrame, 0);
	xgeXuiScrollFrameSetChange(pTable->pFrame, __xgeXuiTableViewFrameChanged, pTable);
	xgeXuiScrollFrameSetColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iBarColor);
	xgeXuiScrollFrameSetButtonColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor);
	xgeXuiScrollFrameSetCornerColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor);
	xgeXuiWidgetSetPaint(xgeXuiScrollFrameGetViewportWidget(pTable->pFrame), __xgeXuiTableViewViewportPaintProc, pTable);
	__xgeXuiTableViewSyncFrame(pTable);
	return XGE_OK;
}

void xgeXuiTableViewUnit(xge_xui_table_view pTable)
{
	xge_xui_widget pWidget;
	xge_xui_widget pBody;

	if ( pTable == NULL ) {
		return;
	}
	pWidget = pTable->pWidget;
	pBody = pTable->pBodyWidget;
	if ( pTable->pContext != NULL ) {
		xgeXuiReleaseWidgetCapture(pTable->pContext, pWidget);
		xgeXuiReleaseWidgetCapture(pTable->pContext, pBody);
	}
	if ( pWidget != NULL && pWidget->pUser == pTable ) {
		pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		xgeXuiWidgetSetLayoutProc(pWidget, NULL, NULL);
		xgeXuiWidgetSetPaint(pWidget, NULL, NULL);
	}
	xgeXuiScrollFrameUnit(pTable->pFrame);
	free(pTable->pFrame);
	xgeXuiWidgetFree(pBody);
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
			pTable->arrColumns[i].fWidth = pTable->fDefaultColumnWidth;
		}
		if ( pTable->arrColumns[i].fMinWidth <= 0.0f ) {
			pTable->arrColumns[i].fMinWidth = 30.0f;
		}
		if ( pTable->arrColumns[i].bVisibleSet == 0 ) {
			pTable->arrColumns[i].bVisible = 1;
		}
		if ( pTable->arrColumns[i].bResizableSet == 0 ) {
			pTable->arrColumns[i].bResizable = 1;
		}
	}
	if ( pTable->iSelectedColumn >= iCount ) {
		pTable->iSelectedColumn = -1;
	}
	__xgeXuiTableViewSyncFrame(pTable);
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetRows(xge_xui_table_view pTable, const xge_xui_table_view_row_t* arrRows, int iCount)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	pTable->arrRows = arrRows;
	pTable->iRowStateCount = iCount;
	if ( pTable->procCount == NULL ) {
		pTable->iRowCount = iCount;
	}
	__xgeXuiTableViewSyncFrame(pTable);
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetAdapter(xge_xui_table_view pTable, xge_xui_table_view_count_proc procCount, xge_xui_table_view_cell_proc procCell, void* pUser)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->procCount = procCount;
	pTable->procCell = procCell;
	pTable->pAdapterUser = pUser;
	__xgeXuiTableViewSyncFrame(pTable);
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetSort(xge_xui_table_view pTable, xge_xui_table_view_sort_proc procSort, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procSort = procSort;
		pTable->pSortUser = pUser;
	}
}

void xgeXuiTableViewSetSelect(xge_xui_table_view pTable, xge_xui_table_view_select_proc procSelect, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procSelect = procSelect;
		pTable->pSelectUser = pUser;
	}
}

void xgeXuiTableViewSetColumnResize(xge_xui_table_view pTable, xge_xui_table_view_column_resize_proc procResize, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procColumnResize = procResize;
		pTable->pColumnResizeUser = pUser;
	}
}

void xgeXuiTableViewSetHover(xge_xui_table_view pTable, xge_xui_table_view_hover_proc procHover, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procHover = procHover;
		pTable->pHoverUser = pUser;
	}
}

void xgeXuiTableViewSetMergeProvider(xge_xui_table_view pTable, xge_xui_table_view_merge_proc procMerge, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procMerge = procMerge;
		pTable->pMergeUser = pUser;
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewSetHeaderRenderer(xge_xui_table_view pTable, xge_xui_table_view_header_renderer_proc procRenderer, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procHeaderRenderer = procRenderer;
		pTable->pHeaderRendererUser = pUser;
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewSetCellRenderer(xge_xui_table_view pTable, xge_xui_table_view_cell_renderer_proc procRenderer, void* pUser)
{
	if ( pTable != NULL ) {
		pTable->procCellRenderer = procRenderer;
		pTable->pCellRendererUser = pUser;
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewSetColumnFormatter(xge_xui_table_view pTable, int iColumn, xge_xui_table_view_format_proc procFormatter, void* pUser)
{
	if ( (pTable == NULL) || (iColumn < 0) || (iColumn >= pTable->iColumnCount) ) {
		return;
	}
	pTable->arrColumns[iColumn].procFormatter = procFormatter;
	pTable->arrColumns[iColumn].pFormatterUser = pUser;
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetFont(xge_xui_table_view pTable, xge_font pFont)
{
	if ( pTable != NULL ) {
		pTable->pFont = pFont;
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewSetDefaultMetrics(xge_xui_table_view pTable, float fColumnWidth, float fRowHeight, float fHeaderHeight)
{
	if ( pTable == NULL ) {
		return;
	}
	if ( fColumnWidth > 0.0f ) {
		pTable->fDefaultColumnWidth = fColumnWidth;
	}
	if ( fRowHeight > 0.0f ) {
		pTable->fDefaultRowHeight = fRowHeight;
	}
	if ( fHeaderHeight > 0.0f ) {
		pTable->fHeaderHeight = fHeaderHeight;
	}
	__xgeXuiTableViewLayoutProc(pTable->pWidget, pTable);
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetSelectionMode(xge_xui_table_view pTable, int iMode)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->iSelectionMode = (iMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW) ? XGE_XUI_TABLE_VIEW_SELECTION_ROW : XGE_XUI_TABLE_VIEW_SELECTION_CELL;
	if ( pTable->iSelectionMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW ) {
		pTable->iSelectedColumn = -1;
		pTable->iFocusColumn = -1;
	}
	__xgeXuiTableViewInvalidate(pTable);
}

int xgeXuiTableViewGetSelectionMode(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->iSelectionMode : XGE_XUI_TABLE_VIEW_SELECTION_CELL;
}

void xgeXuiTableViewSetSelectedRow(xge_xui_table_view pTable, int iRow)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->iSelectionMode = XGE_XUI_TABLE_VIEW_SELECTION_ROW;
	__xgeXuiTableViewSelect(pTable, iRow, -1, 0);
}

int xgeXuiTableViewGetSelectedRow(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->iSelectedRow : -1;
}

void xgeXuiTableViewSetSelectedCell(xge_xui_table_view pTable, int iRow, int iColumn)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->iSelectionMode = XGE_XUI_TABLE_VIEW_SELECTION_CELL;
	__xgeXuiTableViewSelect(pTable, iRow, iColumn, 0);
}

void xgeXuiTableViewGetSelectedCell(xge_xui_table_view pTable, int* pRow, int* pColumn)
{
	if ( pRow != NULL ) {
		*pRow = (pTable != NULL) ? pTable->iSelectedRow : -1;
	}
	if ( pColumn != NULL ) {
		*pColumn = (pTable != NULL) ? pTable->iSelectedColumn : -1;
	}
}

void xgeXuiTableViewSetOffset(xge_xui_table_view pTable, float fScrollX, float fScrollY)
{
	if ( pTable != NULL ) {
		xgeXuiScrollFrameSetOffset(pTable->pFrame, fScrollX, fScrollY);
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewGetOffset(xge_xui_table_view pTable, float* pScrollX, float* pScrollY)
{
	xgeXuiScrollFrameGetOffset((pTable != NULL) ? pTable->pFrame : NULL, pScrollX, pScrollY);
}

void xgeXuiTableViewSetScrollbarMode(xge_xui_table_view pTable, int iMode)
{
	if ( pTable != NULL ) {
		xgeXuiScrollFrameSetScrollbarMode(pTable->pFrame, iMode);
		__xgeXuiTableViewInvalidate(pTable);
	}
}

int xgeXuiTableViewGetScrollbarMode(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? xgeXuiScrollFrameGetScrollbarMode(pTable->pFrame) : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

int xgeXuiTableViewGetRowCount(xge_xui_table_view pTable)
{
	return __xgeXuiTableViewSyncRowCount(pTable);
}

int xgeXuiTableViewGetFirstVisible(xge_xui_table_view pTable)
{
	float fY;
	int iFirst;

	iFirst = __xgeXuiTableViewVisibleRowStart(pTable, &fY);
	if ( pTable != NULL ) {
		pTable->iFirstVisible = iFirst;
	}
	return iFirst;
}

int xgeXuiTableViewGetPaintVisibleCount(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->iPaintVisibleCount : 0;
}

void xgeXuiTableViewGetActiveCell(xge_xui_table_view pTable, int* pRow, int* pColumn)
{
	if ( pRow != NULL ) {
		*pRow = (pTable != NULL) ? pTable->iActiveRow : -1;
	}
	if ( pColumn != NULL ) {
		*pColumn = (pTable != NULL) ? pTable->iActiveColumn : -1;
	}
}

int xgeXuiTableViewGetCellContentRect(xge_xui_table_view pTable, int iRow, int iColumn, xge_rect_t* pRect)
{
	xge_xui_table_view_cell_t tCell;
	int iOwnerRow;
	int iOwnerColumn;

	if ( pRect != NULL ) {
		memset(pRect, 0, sizeof(*pRect));
	}
	if ( (pTable == NULL) || (pRect == NULL) || (iRow < 0) || (iColumn < 0) || (iRow >= __xgeXuiTableViewSyncRowCount(pTable)) || (iColumn >= pTable->iColumnCount) ) {
		return 0;
	}
	(void)__xgeXuiTableViewMergeOwner(pTable, iRow, iColumn, &iOwnerRow, &iOwnerColumn);
	__xgeXuiTableViewGetCell(pTable, iOwnerRow, iOwnerColumn, &tCell);
	*pRect = __xgeXuiTableViewCellContentRect(pTable, iOwnerRow, iOwnerColumn, tCell.iRowSpan, tCell.iColSpan);
	return 1;
}

int xgeXuiTableViewGetCellRect(xge_xui_table_view pTable, int iRow, int iColumn, xge_rect_t* pRect)
{
	xge_rect_t tContent;

	if ( pRect != NULL ) {
		memset(pRect, 0, sizeof(*pRect));
	}
	if ( (pTable == NULL) || (pRect == NULL) || (xgeXuiTableViewGetCellContentRect(pTable, iRow, iColumn, &tContent) == 0) ) {
		return 0;
	}
	*pRect = __xgeXuiTableViewContentToScreenRect(pTable, tContent);
	return 1;
}

xge_xui_widget xgeXuiTableViewGetBodyWidget(xge_xui_table_view pTable)
{
	return (pTable != NULL) ? pTable->pBodyWidget : NULL;
}

xge_xui_widget xgeXuiTableViewGetViewportWidget(xge_xui_table_view pTable)
{
	return ((pTable != NULL) && (pTable->pFrame != NULL)) ? xgeXuiScrollFrameGetViewportWidget(pTable->pFrame) : NULL;
}

void xgeXuiTableViewSetColors(xge_xui_table_view pTable, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pTable == NULL ) {
		return;
	}
	pTable->iBackgroundColor = iBackground;
	pTable->iHeaderColor = iHeader;
	pTable->iHeaderTextColor = iText;
	pTable->iRowColor = iRow;
	pTable->iAltRowColor = __xgeXuiTableViewHoverColor(iRow);
	pTable->iHoverColor = __xgeXuiTableViewHoverColor(iRow);
	pTable->iSelectedColor = iSelected;
	pTable->iGridColor = iGrid;
	pTable->iTextColor = iText;
	xgeXuiWidgetSetBackground(pTable->pWidget, iBackground);
	xgeXuiWidgetSetBorder(pTable->pWidget, 1.0f, iGrid);
	xgeXuiScrollFrameSetColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iThumbColor, pTable->iBarColor);
	xgeXuiScrollFrameSetButtonColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor);
	xgeXuiScrollFrameSetCornerColors(pTable->pFrame, pTable->iBarColor, pTable->iThumbColor);
	__xgeXuiTableViewInvalidate(pTable);
}

void xgeXuiTableViewSetDisabledTextColor(xge_xui_table_view pTable, uint32_t iColor)
{
	if ( pTable != NULL ) {
		pTable->iDisabledTextColor = iColor;
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewRefresh(xge_xui_table_view pTable)
{
	if ( pTable != NULL ) {
		__xgeXuiTableViewSyncFrame(pTable);
		__xgeXuiTableViewInvalidate(pTable);
	}
}

void xgeXuiTableViewEnsureCellVisible(xge_xui_table_view pTable, int iRow, int iColumn)
{
	xge_xui_table_view_cell_t tCell;
	xge_rect_t tRect;

	if ( (pTable == NULL) || (iRow < 0) || (iColumn < 0) ) {
		return;
	}
	__xgeXuiTableViewGetCell(pTable, iRow, iColumn, &tCell);
	tRect = __xgeXuiTableViewCellContentRect(pTable, iRow, iColumn, tCell.iRowSpan, tCell.iColSpan);
	if ( xgeXuiScrollModelEnsureRectVisible(&pTable->tScroll, tRect) ) {
		xgeXuiScrollFrameSetOffset(pTable->pFrame, pTable->tScroll.fScrollX, pTable->tScroll.fScrollY);
		__xgeXuiTableViewInvalidate(pTable);
	}
}

int xgeXuiTableViewEvent(xge_xui_table_view pTable, const xge_event_t* pEvent)
{
	xge_rect_t tHeader;
	int iInsideWidget;
	int iRet;
	int iRow;
	int iColumn;
	int iResize;
	float fWidth;

	if ( (pTable == NULL) || (pTable->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( ((pTable->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0) || ((pTable->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInsideWidget = __xgeXuiRectContains(pTable->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			iRet = __xgeXuiTableViewForwardScrollBars(pTable, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			iRet = xgeXuiScrollFrameEvent(pTable->pFrame, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				__xgeXuiTableViewInvalidate(pTable);
			}
			return iRet;

		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pTable->iResizeColumn >= 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->pContext, pEvent->iPointerId) != pTable->pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				fWidth = pTable->fDragWidth + (pEvent->fX - pTable->fDragX);
				if ( fWidth < pTable->arrColumns[pTable->iResizeColumn].fMinWidth ) {
					fWidth = pTable->arrColumns[pTable->iResizeColumn].fMinWidth;
				}
				if ( (pTable->arrColumns[pTable->iResizeColumn].fMaxWidth > 0.0f) && (fWidth > pTable->arrColumns[pTable->iResizeColumn].fMaxWidth) ) {
					fWidth = pTable->arrColumns[pTable->iResizeColumn].fMaxWidth;
				}
				pTable->arrColumns[pTable->iResizeColumn].fWidth = fWidth;
				pTable->iColumnResizeCount++;
				if ( pTable->procColumnResize != NULL ) {
					pTable->procColumnResize(pTable->pWidget, pTable->iResizeColumn, fWidth, pTable->pColumnResizeUser);
				}
				__xgeXuiTableViewSyncFrame(pTable);
				__xgeXuiTableViewInvalidate(pTable);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRet = __xgeXuiTableViewForwardScrollBars(pTable, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			if ( __xgeXuiTableViewHitCell(pTable, pEvent->fX, pEvent->fY, &iRow, &iColumn) ) {
				__xgeXuiTableViewSetHover(pTable, iRow, iColumn);
			} else {
				__xgeXuiTableViewSetHover(pTable, -1, -1);
			}
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInsideWidget == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pTable->pContext, pTable->pWidget);
			iResize = __xgeXuiTableViewHeaderResizeAt(pTable, pEvent->fX, pEvent->fY);
			if ( (iResize >= 0) && ((pEvent->iType != XGE_EVENT_MOUSE_DOWN) || (pEvent->iParam1 == XGE_MOUSE_LEFT)) ) {
				pTable->iResizeColumn = iResize;
				pTable->fDragX = pEvent->fX;
				pTable->fDragWidth = pTable->arrColumns[iResize].fWidth;
				xgeXuiSetPointerCapture(pTable->pContext, pEvent->iPointerId, pTable->pWidget);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRet = __xgeXuiTableViewForwardScrollBars(pTable, pEvent);
			if ( iRet == XGE_XUI_EVENT_CONSUMED ) {
				return iRet;
			}
			tHeader = __xgeXuiTableViewHeaderRect(pTable);
			if ( __xgeXuiRectContains(tHeader, pEvent->fX, pEvent->fY) ) {
				iColumn = __xgeXuiTableViewHeaderColumnAt(pTable, pEvent->fX, pEvent->fY);
				if ( iColumn >= 0 ) {
					pTable->bSortDescending = (pTable->iSortColumn == iColumn) ? !pTable->bSortDescending : 0;
					pTable->iSortColumn = iColumn;
					pTable->iSortCount++;
					if ( pTable->procSort != NULL ) {
						pTable->procSort(pTable->pWidget, iColumn, pTable->bSortDescending, pTable->pSortUser);
					}
					__xgeXuiTableViewInvalidate(pTable);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			if ( __xgeXuiTableViewHitCell(pTable, pEvent->fX, pEvent->fY, &iRow, &iColumn) ) {
				pTable->iActiveRow = iRow;
				pTable->iActiveColumn = iColumn;
				__xgeXuiTableViewSelect(pTable, iRow, (pTable->iSelectionMode == XGE_XUI_TABLE_VIEW_SELECTION_ROW) ? -1 : iColumn, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pTable->iResizeColumn >= 0 ) {
				if ( xgeXuiGetPointerCapture(pTable->pContext, pEvent->iPointerId) == pTable->pWidget ) {
					xgeXuiSetPointerCapture(pTable->pContext, pEvent->iPointerId, NULL);
				}
				pTable->iResizeColumn = -1;
				return XGE_XUI_EVENT_CONSUMED;
			}
			iRet = __xgeXuiTableViewForwardScrollBars(pTable, pEvent);
			return iRet;

		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pTable->iResizeColumn = -1;
			pTable->iActiveRow = -1;
			pTable->iActiveColumn = -1;
			return XGE_XUI_EVENT_CONSUMED;

		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiTableViewSetHover(pTable, -1, -1);
			return XGE_XUI_EVENT_CONTINUE;

		case XGE_EVENT_KEY_DOWN:
			if ( (pTable->pContext == NULL) || (pTable->pContext->pFocus != pTable->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			return __xgeXuiTableViewMoveSelection(pTable, pEvent->iParam1) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;

		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiTableViewEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiTableViewEvent((xge_xui_table_view)pUser, pEvent);
}

void xgeXuiTableViewPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_view pTable;
	xge_rect_t tHeader;
	xge_rect_t tCell;
	xge_rect_t tText;
	xge_rect_t tLine;
	float fX;
	float fW;
	int i;
	int iState;
	int iHandled;

	pTable = (xge_xui_table_view)pUser;
	if ( (pWidget == NULL) || (pTable == NULL) ) {
		return;
	}
	tHeader = __xgeXuiTableViewHeaderRect(pTable);
	if ( tHeader.fW <= 0.0f || tHeader.fH <= 0.0f ) {
		return;
	}
	__xgeXuiHostDrawRect(tHeader, pTable->iHeaderColor);
	(void)xgeFlush();
	__xgeXuiHostClipSet(tHeader);
	fX = tHeader.fX - pTable->tScroll.fScrollX;
	for ( i = 0; i < pTable->iColumnCount; i++ ) {
		fW = __xgeXuiTableViewColumnWidth(pTable, i);
		if ( fW <= 0.0f ) {
			continue;
		}
		tCell = (xge_rect_t){ fX, tHeader.fY, fW, tHeader.fH };
		pTable->arrColumns[i].tRect = tCell;
		iState = XGE_XUI_TABLE_CELL_HEADER;
		if ( i == pTable->iSortColumn ) {
			iState |= XGE_XUI_TABLE_CELL_SELECTED;
		}
		iHandled = 0;
		if ( pTable->procHeaderRenderer != NULL ) {
			iHandled = pTable->procHeaderRenderer(pTable->pWidget, i, &pTable->arrColumns[i], tCell, iState, pTable->pHeaderRendererUser);
		}
		if ( iHandled == 0 ) {
			__xgeXuiHostDrawRect(tCell, pTable->iHeaderColor);
			if ( (pTable->pFont != NULL) && (pTable->arrColumns[i].sTitle != NULL) ) {
				tText = tCell;
				tText.fX += 5.0f;
				tText.fW -= 10.0f;
				if ( tText.fW < 1.0f ) {
					tText.fW = 1.0f;
				}
				__xgeXuiHostDrawTextRect(pTable->pFont, pTable->arrColumns[i].sTitle, tText, pTable->iHeaderTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			}
			if ( i == pTable->iSortColumn ) {
				tLine = (xge_rect_t){ tCell.fX + tCell.fW - 12.0f, tCell.fY + tCell.fH * 0.5f - 1.0f, 6.0f, 2.0f };
				__xgeXuiHostDrawRect(tLine, pTable->iHeaderTextColor);
			}
		}
		tLine = (xge_rect_t){ tCell.fX + tCell.fW - 1.0f, tCell.fY, 1.0f, tCell.fH };
		__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
		fX += fW;
	}
	(void)xgeFlush();
	__xgeXuiHostClipClear();
	tLine = (xge_rect_t){ tHeader.fX, tHeader.fY + tHeader.fH - 1.0f, tHeader.fW, 1.0f };
	__xgeXuiHostDrawRect(tLine, pTable->iGridColor);
	__xgeXuiHostDrawBorderRect(pWidget->tBorderRect, 1.0f, pTable->iGridColor);
}
