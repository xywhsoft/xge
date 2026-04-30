static uint32_t __xgeXuiPropertyGridHoverColor(uint32_t iRow)
{
	int iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	int iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	int iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiPropertyGridClamp(xge_xui_property_grid pGrid);

static void __xgeXuiPropertyGridRebuildVisible(xge_xui_property_grid pGrid)
{
	int i;
	int iParent;

	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iVisibleCount = 0;
	for ( i = 0; i < pGrid->iItemCount && pGrid->iVisibleCount < XGE_XUI_PROPERTY_GRID_ITEM_CAPACITY; i++ ) {
		if ( pGrid->arrItems[i].iCategory ) {
			pGrid->arrVisible[pGrid->iVisibleCount++] = i;
			continue;
		}
		iParent = pGrid->arrItems[i].iParentCategory;
		if ( (iParent < 0) || (iParent >= pGrid->iItemCount) || pGrid->arrItems[iParent].bExpanded ) {
			pGrid->arrVisible[pGrid->iVisibleCount++] = i;
		}
	}
	__xgeXuiPropertyGridClamp(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

static int __xgeXuiPropertyGridVisibleAt(xge_xui_property_grid pGrid, float fY)
{
	int iVisible;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) || (pGrid->fRowHeight <= 0.0f) ) {
		return -1;
	}
	iVisible = (int)((fY - pGrid->pWidget->tContentRect.fY + pGrid->fScrollY) / pGrid->fRowHeight);
	if ( (iVisible < 0) || (iVisible >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return iVisible;
}

static float __xgeXuiPropertyGridMaxScroll(xge_xui_property_grid pGrid)
{
	float fMax;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = (float)pGrid->iVisibleCount * pGrid->fRowHeight - pGrid->pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiPropertyGridClamp(xge_xui_property_grid pGrid)
{
	float fMax;

	if ( pGrid == NULL ) {
		return;
	}
	fMax = __xgeXuiPropertyGridMaxScroll(pGrid);
	if ( pGrid->fScrollY < 0.0f ) {
		pGrid->fScrollY = 0.0f;
	}
	if ( pGrid->fScrollY > fMax ) {
		pGrid->fScrollY = fMax;
	}
}

static float __xgeXuiPropertyGridThumbLen(float fTrackLen, float fVisible, float fContent)
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

static int __xgeXuiPropertyGridBar(xge_xui_property_grid pGrid, xge_rect_t* pBar, xge_rect_t* pThumb)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fContentH;
	float fMaxScroll;
	float fSize;
	float fButton;
	float fTrackH;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) || (__xgeXuiPropertyGridMaxScroll(pGrid) <= 0.0f) ) {
		return 0;
	}
	fSize = (pGrid->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pGrid->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pGrid->pWidget->tContentRect.fX + pGrid->pWidget->tContentRect.fW - fSize;
	tBar.fY = pGrid->pWidget->tContentRect.fY;
	tBar.fW = fSize;
	tBar.fH = pGrid->pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	fTrackH = tThumb.fH;
	if ( fTrackH < 1.0f ) {
		fTrackH = 1.0f;
		tThumb.fH = 1.0f;
	}
	fContentH = (float)pGrid->iVisibleCount * pGrid->fRowHeight;
	tThumb.fH = __xgeXuiPropertyGridThumbLen(fTrackH, pGrid->pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiPropertyGridMaxScroll(pGrid);
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pGrid->fScrollY / fMaxScroll);
	}
	if ( pBar != NULL ) {
		*pBar = tBar;
	}
	if ( pThumb != NULL ) {
		*pThumb = tThumb;
	}
	return 1;
}

static void __xgeXuiPropertyGridSetScrollFromThumbDrag(xge_xui_property_grid pGrid, float fY)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	float fTravel;
	float fMaxScroll;

	if ( (pGrid == NULL) || (__xgeXuiPropertyGridBar(pGrid, &tBar, &tThumb) == 0) ) {
		return;
	}
	fTravel = tBar.fH - tThumb.fH;
	fMaxScroll = __xgeXuiPropertyGridMaxScroll(pGrid);
	if ( (fTravel <= 0.0f) || (fMaxScroll <= 0.0f) ) {
		return;
	}
	xgeXuiPropertyGridSetScroll(pGrid, pGrid->fDragScrollY + ((fY - pGrid->fDragY) / fTravel) * fMaxScroll);
}

int xgeXuiPropertyGridInit(xge_xui_property_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget)
{
	if ( (pGrid == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pGrid, 0, sizeof(*pGrid));
	pGrid->pContext = pContext;
	pGrid->pWidget = pWidget;
	pGrid->iSelected = -1;
	pGrid->iHover = -1;
	pGrid->fRowHeight = 22.0f;
	pGrid->fNameWidth = 120.0f;
	pGrid->iBackgroundColor = XGE_COLOR_RGBA(235, 244, 252, 255);
	pGrid->iCategoryColor = XGE_COLOR_RGBA(210, 231, 247, 255);
	pGrid->iRowColor = XGE_COLOR_RGBA(245, 250, 255, 255);
	pGrid->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	pGrid->iSelectedColor = XGE_COLOR_RGBA(187, 220, 248, 255);
	pGrid->iGridColor = XGE_COLOR_RGBA(151, 187, 215, 255);
	pGrid->iTextColor = XGE_COLOR_RGBA(34, 48, 64, 255);
	pGrid->iValueColor = XGE_COLOR_RGBA(25, 84, 132, 255);
	pGrid->iReadonlyColor = XGE_COLOR_RGBA(128, 138, 150, 220);
	pGrid->iChangedColor = XGE_COLOR_RGBA(0, 128, 192, 255);
	pGrid->iErrorColor = XGE_COLOR_RGBA(210, 72, 72, 255);
	pGrid->iBarColor = XGE_COLOR_RGBA(226, 236, 246, 230);
	pGrid->iThumbColor = XGE_COLOR_RGBA(104, 142, 178, 245);
	pGrid->iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetSetFocusable(pWidget, 1);
	xgeXuiWidgetSetClip(pWidget, 1);
	pWidget->procEvent = xgeXuiPropertyGridEventProc;
	pWidget->procPaint = xgeXuiPropertyGridPaintProc;
	pWidget->pUser = pGrid;
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPropertyGridUnit(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( pGrid->pWidget != NULL && pGrid->pWidget->pUser == pGrid ) {
		pGrid->pWidget->pUser = NULL;
		pGrid->pWidget->procEvent = NULL;
		pGrid->pWidget->procPaint = NULL;
	}
	memset(pGrid, 0, sizeof(*pGrid));
}

void xgeXuiPropertyGridClear(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iItemCount = 0;
	pGrid->iVisibleCount = 0;
	pGrid->iSelected = -1;
	pGrid->iHover = -1;
	pGrid->fScrollY = 0.0f;
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

int xgeXuiPropertyGridAddCategory(xge_xui_property_grid pGrid, const char* sName, int bExpanded)
{
	xge_xui_property_grid_item_t* pItem;

	if ( (pGrid == NULL) || (pGrid->iItemCount >= XGE_XUI_PROPERTY_GRID_ITEM_CAPACITY) ) {
		return -1;
	}
	pItem = &pGrid->arrItems[pGrid->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->sName = sName;
	pItem->iCategory = 1;
	pItem->iParentCategory = -1;
	pItem->bExpanded = (bExpanded != 0);
	pGrid->iItemCount++;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
	return pGrid->iItemCount - 1;
}

int xgeXuiPropertyGridAddProperty(xge_xui_property_grid pGrid, int iCategory, const char* sName, const char* sValue, int iEditor)
{
	xge_xui_property_grid_item_t* pItem;

	if ( (pGrid == NULL) || (pGrid->iItemCount >= XGE_XUI_PROPERTY_GRID_ITEM_CAPACITY) ) {
		return -1;
	}
	pItem = &pGrid->arrItems[pGrid->iItemCount];
	memset(pItem, 0, sizeof(*pItem));
	pItem->sName = sName;
	pItem->sValue = sValue;
	pItem->iEditor = iEditor;
	pItem->iParentCategory = iCategory;
	pGrid->iItemCount++;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
	return pGrid->iItemCount - 1;
}

void xgeXuiPropertyGridSetPropertyFlags(xge_xui_property_grid pGrid, int iIndex, int bReadonly, int bDefaultChanged, int bError)
{
	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) ) {
		return;
	}
	pGrid->arrItems[iIndex].bReadonly = (bReadonly != 0);
	pGrid->arrItems[iIndex].bDefaultChanged = (bDefaultChanged != 0);
	pGrid->arrItems[iIndex].bError = (bError != 0);
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

void xgeXuiPropertyGridSetSelected(xge_xui_property_grid pGrid, int iIndex)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( (iIndex < 0) || (iIndex >= pGrid->iItemCount) || pGrid->arrItems[iIndex].iCategory ) {
		iIndex = -1;
	}
	pGrid->iSelected = iIndex;
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

int xgeXuiPropertyGridGetSelected(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iSelected : -1;
}

int xgeXuiPropertyGridGetVisibleCount(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iVisibleCount : 0;
}

int xgeXuiPropertyGridGetVisibleItem(xge_xui_property_grid pGrid, int iVisible)
{
	if ( (pGrid == NULL) || (iVisible < 0) || (iVisible >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return pGrid->arrVisible[iVisible];
}

void xgeXuiPropertyGridSetFont(xge_xui_property_grid pGrid, xge_font pFont)
{
	if ( pGrid != NULL ) {
		pGrid->pFont = pFont;
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

void xgeXuiPropertyGridSetMetrics(xge_xui_property_grid pGrid, float fRowHeight, float fNameWidth)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->fRowHeight = (fRowHeight < 1.0f) ? 1.0f : fRowHeight;
	pGrid->fNameWidth = (fNameWidth < 20.0f) ? 20.0f : fNameWidth;
	__xgeXuiPropertyGridClamp(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

void xgeXuiPropertyGridSetScroll(xge_xui_property_grid pGrid, float fScrollY)
{
	float fOld;

	if ( pGrid == NULL ) {
		return;
	}
	fOld = pGrid->fScrollY;
	pGrid->fScrollY = fScrollY;
	__xgeXuiPropertyGridClamp(pGrid);
	if ( fOld != pGrid->fScrollY ) {
		xgeXuiWidgetMarkPaint(pGrid->pWidget);
	}
}

float xgeXuiPropertyGridGetScroll(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->fScrollY : 0.0f;
}

void xgeXuiPropertyGridSetScrollbarMode(xge_xui_property_grid pGrid, int iMode)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

int xgeXuiPropertyGridGetScrollbarMode(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiPropertyGridSetSelect(xge_xui_property_grid pGrid, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procSelect = procSelect;
		pGrid->pUser = pUser;
	}
}

void xgeXuiPropertyGridSetColors(xge_xui_property_grid pGrid, uint32_t iBackground, uint32_t iCategory, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iBackgroundColor = iBackground;
	pGrid->iCategoryColor = iCategory;
	pGrid->iRowColor = iRow;
	pGrid->iHoverColor = __xgeXuiPropertyGridHoverColor(iRow);
	pGrid->iSelectedColor = iSelected;
	pGrid->iGridColor = iGrid;
	pGrid->iTextColor = iText;
	pGrid->iValueColor = iText;
	xgeXuiWidgetMarkPaint(pGrid->pWidget);
}

int xgeXuiPropertyGridEvent(xge_xui_property_grid pGrid, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iVisible;
	int iItem;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pGrid->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiPropertyGridSetScroll(pGrid, pGrid->fScrollY - pEvent->fDY * pGrid->fRowHeight);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pGrid->bDraggingThumb != 0 ) {
				__xgeXuiPropertyGridSetScrollFromThumbDrag(pGrid, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = iInside ? __xgeXuiPropertyGridVisibleAt(pGrid, pEvent->fY) : -1;
			if ( pGrid->iHover != iVisible ) {
				pGrid->iHover = iVisible;
				pGrid->iState = (iVisible >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
				xgeXuiWidgetMarkPaint(pGrid->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pGrid->pContext, pGrid->pWidget);
			if ( __xgeXuiPropertyGridBar(pGrid, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pGrid->bDraggingThumb = 1;
					pGrid->fDragY = pEvent->fY;
					pGrid->fDragScrollY = pGrid->fScrollY;
					xgeXuiSetCapture(pGrid->pContext, pGrid->pWidget);
				} else {
					xgeXuiPropertyGridSetScroll(pGrid, pGrid->fScrollY + ((pEvent->fY < tThumb.fY) ? -pGrid->pWidget->tContentRect.fH : pGrid->pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = __xgeXuiPropertyGridVisibleAt(pGrid, pEvent->fY);
			if ( iVisible < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iItem = pGrid->arrVisible[iVisible];
			if ( pGrid->arrItems[iItem].iCategory ) {
				pGrid->arrItems[iItem].bExpanded = !pGrid->arrItems[iItem].bExpanded;
				if ( pGrid->iSelected >= 0 && pGrid->arrItems[pGrid->iSelected].iParentCategory == iItem && !pGrid->arrItems[iItem].bExpanded ) {
					pGrid->iSelected = -1;
				}
				__xgeXuiPropertyGridRebuildVisible(pGrid);
				return XGE_XUI_EVENT_CONSUMED;
			}
			xgeXuiPropertyGridSetSelected(pGrid, iItem);
			pGrid->iSelectCount++;
			if ( pGrid->procSelect != NULL ) {
				pGrid->procSelect(pGrid->pWidget, iItem, pGrid->pUser);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pGrid->bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiPropertyGridSetScrollFromThumbDrag(pGrid, pEvent->fY);
			pGrid->bDraggingThumb = 0;
			if ( pGrid->pContext != NULL && pGrid->pContext->pCapture == pGrid->pWidget ) {
				xgeXuiSetCapture(pGrid->pContext, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_XUI_CAPTURE_LOST:
			pGrid->bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiPropertyGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiPropertyGridEvent((xge_xui_property_grid)pUser, pEvent);
}

void xgeXuiPropertyGridPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_item_t* pItem;
	xge_rect_t tRow;
	xge_rect_t tName;
	xge_rect_t tValue;
	xge_rect_t tMark;
	xge_rect_t tLine;
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iVisible;
	int iItem;
	uint32_t iColor;
	uint32_t iText;

	pGrid = (xge_xui_property_grid)pUser;
	if ( (pWidget == NULL) || (pGrid == NULL) ) {
		return;
	}
	__xgeXuiHostDrawRect(pWidget->tRect, pGrid->iBackgroundColor);
	for ( iVisible = 0; iVisible < pGrid->iVisibleCount; iVisible++ ) {
		iItem = pGrid->arrVisible[iVisible];
		pItem = &pGrid->arrItems[iItem];
		tRow.fX = pWidget->tContentRect.fX;
		tRow.fY = pWidget->tContentRect.fY + (float)iVisible * pGrid->fRowHeight - pGrid->fScrollY;
		tRow.fW = pWidget->tContentRect.fW;
		tRow.fH = pGrid->fRowHeight;
		if ( (tRow.fY + tRow.fH) < pWidget->tContentRect.fY || tRow.fY > (pWidget->tContentRect.fY + pWidget->tContentRect.fH) ) {
			continue;
		}
		pItem->tRect = tRow;
		iColor = pItem->iCategory ? pGrid->iCategoryColor : pGrid->iRowColor;
		if ( iVisible == pGrid->iHover && !pItem->iCategory ) {
			iColor = pGrid->iHoverColor;
		}
		if ( iItem == pGrid->iSelected ) {
			iColor = pGrid->iSelectedColor;
		}
		__xgeXuiHostDrawRect(tRow, iColor);
		tLine.fX = tRow.fX;
		tLine.fY = tRow.fY + tRow.fH - 1.0f;
		tLine.fW = tRow.fW;
		tLine.fH = 1.0f;
		__xgeXuiHostDrawRect(tLine, pGrid->iGridColor);
		if ( !pItem->iCategory ) {
			tLine.fX = tRow.fX + pGrid->fNameWidth - 1.0f;
			tLine.fY = tRow.fY;
			tLine.fW = 1.0f;
			tLine.fH = tRow.fH;
			__xgeXuiHostDrawRect(tLine, pGrid->iGridColor);
		}
		if ( pGrid->pFont != NULL ) {
			tName = tRow;
			tName.fX += pItem->iCategory ? 18.0f : 6.0f;
			tName.fW = pItem->iCategory ? (tRow.fW - 22.0f) : (pGrid->fNameWidth - 8.0f);
			iText = pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iTextColor;
			__xgeXuiHostDrawTextRect(pGrid->pFont, pItem->sName != NULL ? pItem->sName : "", tName, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
			if ( !pItem->iCategory ) {
				tValue = tRow;
				tValue.fX += pGrid->fNameWidth;
				tValue.fW -= pGrid->fNameWidth + 6.0f;
				iText = pItem->bError ? pGrid->iErrorColor : (pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iValueColor);
				__xgeXuiHostDrawTextRect(pGrid->pFont, pItem->sValue != NULL ? pItem->sValue : "", tValue, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
				if ( pItem->bDefaultChanged || pItem->bError || pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_COLOR ) {
					tMark.fX = tRow.fX + tRow.fW - 10.0f;
					tMark.fY = tRow.fY + 7.0f;
					tMark.fW = 5.0f;
					tMark.fH = 5.0f;
					__xgeXuiHostDrawRect(tMark, pItem->bError ? pGrid->iErrorColor : pGrid->iChangedColor);
				}
			}
		}
	}
	if ( __xgeXuiPropertyGridBar(pGrid, &tBar, &tThumb) != 0 ) {
		if ( pGrid->iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pGrid->iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, pGrid->iGridColor);
			__xgeXuiHostDrawRect(tThumb, pGrid->iThumbColor);
		}
	}
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pGrid->iGridColor);
}
