static uint32_t __xgeXuiPropertyGridHoverColor(uint32_t iRow)
{
	int iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	int iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	int iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiPropertyGridClamp(xge_xui_property_grid pGrid);
static float __xgeXuiPropertyGridMaxScroll(xge_xui_property_grid pGrid);

static void __xgeXuiPropertyGridCopyValue(xge_xui_property_grid_item_t* pItem, const char* sValue)
{
	int iLen;

	if ( pItem == NULL ) {
		return;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	iLen = (int)strlen(sValue);
	if ( iLen >= XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY ) {
		iLen = XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY - 1;
	}
	memcpy(pItem->sValueStorage, sValue, (size_t)iLen);
	pItem->sValueStorage[iLen] = 0;
	pItem->sValue = pItem->sValueStorage;
}

static xge_rect_t __xgeXuiPropertyGridValueRect(xge_xui_property_grid pGrid, xge_rect_t tRow)
{
	xge_rect_t tValue;
	float fBarW;

	tValue = tRow;
	tValue.fX += pGrid->fNameWidth;
	tValue.fW -= pGrid->fNameWidth + 6.0f;
	if ( __xgeXuiPropertyGridMaxScroll(pGrid) > 0.0f ) {
		fBarW = (pGrid->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
		tValue.fW -= fBarW;
	}
	if ( tValue.fW < 0.0f ) {
		tValue.fW = 0.0f;
	}
	return tValue;
}

static int __xgeXuiPropertyGridPrevCursor(const char* sText, int iCursor)
{
	if ( sText == NULL || iCursor <= 0 ) {
		return 0;
	}
	iCursor--;
	while ( iCursor > 0 && (((unsigned char)sText[iCursor] & 0xC0) == 0x80) ) {
		iCursor--;
	}
	return iCursor;
}

static int __xgeXuiPropertyGridNextCursor(const char* sText, int iCursor)
{
	int iSize;

	if ( sText == NULL ) {
		return 0;
	}
	iSize = (int)strlen(sText);
	if ( iCursor < 0 ) {
		iCursor = 0;
	}
	if ( iCursor >= iSize ) {
		return iSize;
	}
	iCursor++;
	while ( iCursor < iSize && (((unsigned char)sText[iCursor] & 0xC0) == 0x80) ) {
		iCursor++;
	}
	return iCursor;
}

static int __xgeXuiPropertyGridVisibleIndexOfItem(xge_xui_property_grid pGrid, int iItem)
{
	int i;

	if ( pGrid == NULL ) {
		return -1;
	}
	for ( i = 0; i < pGrid->iVisibleCount; i++ ) {
		if ( pGrid->arrVisible[i] == iItem ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiPropertyGridNormalizeVisible(xge_xui_property_grid pGrid, int iVisible)
{
	if ( (pGrid == NULL) || (iVisible < 0) || (iVisible >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return iVisible;
}

static void __xgeXuiPropertyGridSetHoverVisible(xge_xui_property_grid pGrid, int iVisible)
{
	if ( pGrid == NULL ) {
		return;
	}
	iVisible = __xgeXuiPropertyGridNormalizeVisible(pGrid, iVisible);
	if ( (pGrid->iHover == iVisible) && (pGrid->tBase.iHover == iVisible) ) {
		return;
	}
	pGrid->iHover = iVisible;
	pGrid->tBase.iHover = iVisible;
	pGrid->iState = (iVisible >= 0) ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

static void __xgeXuiPropertyGridSetFocusVisible(xge_xui_property_grid pGrid, int iVisible)
{
	if ( pGrid == NULL ) {
		return;
	}
	iVisible = __xgeXuiPropertyGridNormalizeVisible(pGrid, iVisible);
	if ( pGrid->tBase.iFocus == iVisible ) {
		return;
	}
	pGrid->tBase.iFocus = iVisible;
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

static void __xgeXuiPropertyGridSyncBaseState(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->tBase.iItemCount = pGrid->iVisibleCount;
	pGrid->tBase.iSelected = __xgeXuiPropertyGridVisibleIndexOfItem(pGrid, pGrid->iSelected);
	__xgeXuiPropertyGridSetHoverVisible(pGrid, pGrid->iHover);
	__xgeXuiPropertyGridSetFocusVisible(pGrid, pGrid->tBase.iSelected);
}

static void __xgeXuiPropertyGridRebuildVisible(xge_xui_property_grid pGrid)
{
	int i;
	int iParent;

	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iVisibleCount = 0;
	pGrid->iHover = -1;
	pGrid->tBase.iHover = -1;
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
	__xgeXuiPropertyGridSyncBaseState(pGrid);
	__xgeXuiPropertyGridClamp(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

static int __xgeXuiPropertyGridVisibleAt(xge_xui_property_grid pGrid, float fY)
{
	int iVisible;

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) || (pGrid->tBase.fItemHeight <= 0.0f) ) {
		return -1;
	}
	iVisible = (int)((fY - pGrid->tBase.pWidget->tContentRect.fY + pGrid->tBase.fScrollY) / pGrid->tBase.fItemHeight);
	if ( (iVisible < 0) || (iVisible >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return iVisible;
}

static float __xgeXuiPropertyGridMaxScroll(xge_xui_property_grid pGrid)
{
	float fMax;

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) ) {
		return 0.0f;
	}
	fMax = (float)pGrid->iVisibleCount * pGrid->tBase.fItemHeight - pGrid->tBase.pWidget->tContentRect.fH;
	return (fMax > 0.0f) ? fMax : 0.0f;
}

static void __xgeXuiPropertyGridClamp(xge_xui_property_grid pGrid)
{
	float fMax;

	if ( pGrid == NULL ) {
		return;
	}
	fMax = __xgeXuiPropertyGridMaxScroll(pGrid);
	if ( pGrid->tBase.fScrollY < 0.0f ) {
		pGrid->tBase.fScrollY = 0.0f;
	}
	if ( pGrid->tBase.fScrollY > fMax ) {
		pGrid->tBase.fScrollY = fMax;
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

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) || (__xgeXuiPropertyGridMaxScroll(pGrid) <= 0.0f) ) {
		return 0;
	}
	fSize = (pGrid->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
	fButton = (pGrid->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? fSize : 0.0f;
	tBar.fX = pGrid->tBase.pWidget->tContentRect.fX + pGrid->tBase.pWidget->tContentRect.fW - fSize;
	tBar.fY = pGrid->tBase.pWidget->tContentRect.fY;
	tBar.fW = fSize;
	tBar.fH = pGrid->tBase.pWidget->tContentRect.fH;
	tThumb = tBar;
	tThumb.fY += fButton;
	tThumb.fH -= fButton * 2.0f;
	fTrackH = tThumb.fH;
	if ( fTrackH < 1.0f ) {
		fTrackH = 1.0f;
		tThumb.fH = 1.0f;
	}
	fContentH = (float)pGrid->iVisibleCount * pGrid->tBase.fItemHeight;
	tThumb.fH = __xgeXuiPropertyGridThumbLen(fTrackH, pGrid->tBase.pWidget->tContentRect.fH, fContentH);
	fMaxScroll = __xgeXuiPropertyGridMaxScroll(pGrid);
	if ( fMaxScroll > 0.0f && fTrackH > tThumb.fH ) {
		tThumb.fY += (fTrackH - tThumb.fH) * (pGrid->tBase.fScrollY / fMaxScroll);
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
	xgeXuiPropertyGridSetScroll(pGrid, pGrid->tBase.fDragScrollY + ((fY - pGrid->tBase.fDragY) / fTravel) * fMaxScroll);
}

int xgeXuiPropertyGridInit(xge_xui_property_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iRet;

	if ( (pGrid == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pGrid, 0, sizeof(*pGrid));
	iRet = xgeXuiTextInit(&pGrid->tEditText);
	if ( iRet != XGE_OK ) {
		return iRet;
	}
	iRet = xgeXuiVirtualScrollViewBaseInit(&pGrid->tBase, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		xgeXuiTextUnit(&pGrid->tEditText);
		return iRet;
	}
	if ( pWidget->pLayoutUser == &pGrid->tBase && pWidget->procLayout == xgeXuiVirtualScrollViewBaseLayoutProc ) {
		pWidget->pLayoutUser = NULL;
		pWidget->procLayout = NULL;
	}
	pGrid->iSelected = -1;
	pGrid->iHover = -1;
	pGrid->iEditing = -1;
	pGrid->tBase.fItemHeight = 22.0f;
	pGrid->fNameWidth = 120.0f;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(235, 244, 252, 255));
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
	pGrid->tBase.iBarColor = XGE_COLOR_RGBA(226, 236, 246, 230);
	pGrid->tBase.iThumbColor = XGE_COLOR_RGBA(104, 142, 178, 245);
	pGrid->tBase.iScrollbarMode = XGE_XUI_SCROLLBAR_MODE_COMPACT;
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
	xgeXuiReleaseWidgetCapture(pGrid->tBase.pContext, pGrid->tBase.pWidget);
	if ( pGrid->tBase.pWidget != NULL && pGrid->tBase.pWidget->pUser == pGrid ) {
		pGrid->tBase.pWidget->pUser = NULL;
		pGrid->tBase.pWidget->procEvent = NULL;
		pGrid->tBase.pWidget->procPaint = NULL;
	}
	if ( pGrid->tBase.pWidget != NULL && pGrid->tBase.pWidget->pLayoutUser == &pGrid->tBase && pGrid->tBase.pWidget->procLayout == xgeXuiVirtualScrollViewBaseLayoutProc ) {
		pGrid->tBase.pWidget->pLayoutUser = NULL;
		pGrid->tBase.pWidget->procLayout = NULL;
	}
	xgeXuiTextUnit(&pGrid->tEditText);
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
	pGrid->iEditing = -1;
	pGrid->tBase.fScrollY = 0.0f;
	pGrid->tBase.iItemCount = 0;
	pGrid->tBase.iSelected = -1;
	pGrid->tBase.iHover = -1;
	pGrid->tBase.iFocus = -1;
	xgeXuiTextSet(&pGrid->tEditText, "");
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
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
	__xgeXuiPropertyGridCopyValue(pItem, "");
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
	__xgeXuiPropertyGridCopyValue(pItem, sValue);
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
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
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
	__xgeXuiPropertyGridSyncBaseState(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
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

void xgeXuiPropertyGridSetValue(xge_xui_property_grid pGrid, int iIndex, const char* sValue)
{
	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) || pGrid->arrItems[iIndex].iCategory ) {
		return;
	}
	__xgeXuiPropertyGridCopyValue(&pGrid->arrItems[iIndex], sValue);
	if ( pGrid->iEditing == iIndex ) {
		xgeXuiTextSet(&pGrid->tEditText, pGrid->arrItems[iIndex].sValue);
	}
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

const char* xgeXuiPropertyGridGetValue(xge_xui_property_grid pGrid, int iIndex)
{
	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) || pGrid->arrItems[iIndex].iCategory ) {
		return "";
	}
	return pGrid->arrItems[iIndex].sValue != NULL ? pGrid->arrItems[iIndex].sValue : "";
}

int xgeXuiPropertyGridIsEditing(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iEditing : -1;
}

void xgeXuiPropertyGridBeginEdit(xge_xui_property_grid pGrid, int iIndex)
{
	xge_xui_property_grid_item_t* pItem;

	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) ) {
		return;
	}
	pItem = &pGrid->arrItems[iIndex];
	if ( pItem->iCategory || pItem->bReadonly ) {
		return;
	}
	if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_BOOL ) {
		xgeXuiPropertyGridSetValue(pGrid, iIndex, (strcmp(xgeXuiPropertyGridGetValue(pGrid, iIndex), "true") == 0 || strcmp(xgeXuiPropertyGridGetValue(pGrid, iIndex), "1") == 0) ? "false" : "true");
		if ( pGrid->procChange != NULL ) {
			pGrid->procChange(pGrid->tBase.pWidget, iIndex, xgeXuiPropertyGridGetValue(pGrid, iIndex), pGrid->pChangeUser);
		}
		return;
	}
	pGrid->iEditing = iIndex;
	xgeXuiPropertyGridSetSelected(pGrid, iIndex);
	xgeXuiTextSet(&pGrid->tEditText, xgeXuiPropertyGridGetValue(pGrid, iIndex));
	xgeXuiSetFocus(pGrid->tBase.pContext, pGrid->tBase.pWidget);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridEndEdit(xge_xui_property_grid pGrid, int bCommit)
{
	int iIndex;
	const char* sOld;
	const char* sNew;

	if ( (pGrid == NULL) || (pGrid->iEditing < 0) || (pGrid->iEditing >= pGrid->iItemCount) ) {
		return;
	}
	iIndex = pGrid->iEditing;
	pGrid->iEditing = -1;
	if ( bCommit != 0 && !pGrid->arrItems[iIndex].bReadonly && !pGrid->arrItems[iIndex].iCategory ) {
		sOld = xgeXuiPropertyGridGetValue(pGrid, iIndex);
		sNew = pGrid->tEditText.sText != NULL ? pGrid->tEditText.sText : "";
		if ( strcmp(sOld, sNew) != 0 ) {
			xgeXuiPropertyGridSetValue(pGrid, iIndex, sNew);
			if ( pGrid->procChange != NULL ) {
				pGrid->procChange(pGrid->tBase.pWidget, iIndex, xgeXuiPropertyGridGetValue(pGrid, iIndex), pGrid->pChangeUser);
			}
		}
	}
	xgeXuiTextSet(&pGrid->tEditText, "");
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridSetFont(xge_xui_property_grid pGrid, xge_font pFont)
{
	if ( pGrid != NULL ) {
		pGrid->pFont = pFont;
		xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
	}
}

void xgeXuiPropertyGridSetMetrics(xge_xui_property_grid pGrid, float fRowHeight, float fNameWidth)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->tBase.fItemHeight = (fRowHeight < 1.0f) ? 1.0f : fRowHeight;
	pGrid->fNameWidth = (fNameWidth < 20.0f) ? 20.0f : fNameWidth;
	__xgeXuiPropertyGridClamp(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridSetScroll(xge_xui_property_grid pGrid, float fScrollY)
{
	float fOld;

	if ( pGrid == NULL ) {
		return;
	}
	fOld = pGrid->tBase.fScrollY;
	pGrid->tBase.fScrollY = fScrollY;
	__xgeXuiPropertyGridClamp(pGrid);
	if ( fOld != pGrid->tBase.fScrollY ) {
		xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
	}
}

float xgeXuiPropertyGridGetScroll(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->tBase.fScrollY : 0.0f;
}

void xgeXuiPropertyGridSetScrollbarMode(xge_xui_property_grid pGrid, int iMode)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->tBase.iScrollbarMode = (iMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? XGE_XUI_SCROLLBAR_MODE_FULL : XGE_XUI_SCROLLBAR_MODE_COMPACT;
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

int xgeXuiPropertyGridGetScrollbarMode(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->tBase.iScrollbarMode : XGE_XUI_SCROLLBAR_MODE_COMPACT;
}

void xgeXuiPropertyGridSetSelect(xge_xui_property_grid pGrid, xge_xui_select_proc procSelect, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->tBase.procSelect = procSelect;
		pGrid->tBase.pSelectUser = pUser;
	}
}

void xgeXuiPropertyGridSetChange(xge_xui_property_grid pGrid, xge_xui_property_grid_change_proc procChange, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procChange = procChange;
		pGrid->pChangeUser = pUser;
	}
}

void xgeXuiPropertyGridSetColors(xge_xui_property_grid pGrid, uint32_t iBackground, uint32_t iCategory, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pGrid == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pGrid->tBase.pWidget, iBackground);
	pGrid->iCategoryColor = iCategory;
	pGrid->iRowColor = iRow;
	pGrid->iHoverColor = __xgeXuiPropertyGridHoverColor(iRow);
	pGrid->iSelectedColor = iSelected;
	pGrid->iGridColor = iGrid;
	pGrid->iTextColor = iText;
	pGrid->iValueColor = iText;
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

int xgeXuiPropertyGridEvent(xge_xui_property_grid pGrid, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	int iInside;
	int iVisible;
	int iItem;
	const char* sText;
	int iResult;

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pGrid->tBase.pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_TEXT:
		case XGE_EVENT_IME_START:
		case XGE_EVENT_IME_UPDATE:
		case XGE_EVENT_IME_END:
			if ( pGrid->iEditing < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			iResult = xgeXuiTextInputEvent(&pGrid->tEditText, pEvent);
			xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
			return iResult;
		case XGE_EVENT_KEY_DOWN:
			if ( pGrid->iEditing >= 0 ) {
				if ( pEvent->iParam1 == XGE_KEY_ENTER || pEvent->iParam1 == XGE_KEY_TAB ) {
					xgeXuiPropertyGridEndEdit(pGrid, 1);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
					xgeXuiPropertyGridEndEdit(pGrid, 0);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_BACKSPACE ) {
					xgeXuiTextDeleteBack(&pGrid->tEditText);
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_DELETE ) {
					xgeXuiTextDeleteForward(&pGrid->tEditText);
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_HOME ) {
					xgeXuiTextSetCursor(&pGrid->tEditText, 0);
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_END ) {
					xgeXuiTextSetCursor(&pGrid->tEditText, pGrid->tEditText.iSize);
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				sText = pGrid->tEditText.sText != NULL ? pGrid->tEditText.sText : "";
				if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
					xgeXuiTextSetCursor(&pGrid->tEditText, __xgeXuiPropertyGridPrevCursor(sText, pGrid->tEditText.iCursor));
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
					xgeXuiTextSetCursor(&pGrid->tEditText, __xgeXuiPropertyGridNextCursor(sText, pGrid->tEditText.iCursor));
					xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
					return XGE_XUI_EVENT_CONSUMED;
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_OUT:
			xgeXuiPropertyGridEndEdit(pGrid, 1);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_WHEEL:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiPropertyGridEndEdit(pGrid, 1);
			xgeXuiPropertyGridSetScroll(pGrid, pGrid->tBase.fScrollY - pEvent->fDY * pGrid->tBase.fItemHeight);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			if ( pGrid->tBase.bDraggingThumb != 0 ) {
				if ( xgeXuiGetPointerCapture(pGrid->tBase.pContext, pEvent->iPointerId) != pGrid->tBase.pWidget ) {
					return XGE_XUI_EVENT_CONTINUE;
				}
				__xgeXuiPropertyGridSetScrollFromThumbDrag(pGrid, pEvent->fY);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = iInside ? __xgeXuiPropertyGridVisibleAt(pGrid, pEvent->fY) : -1;
			__xgeXuiPropertyGridSetHoverVisible(pGrid, iVisible);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
			if ( iInside == 0 ) {
				xgeXuiPropertyGridEndEdit(pGrid, 1);
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pGrid->tBase.pContext, pGrid->tBase.pWidget);
			if ( __xgeXuiPropertyGridBar(pGrid, &tBar, &tThumb) != 0 && __xgeXuiRectContains(tBar, pEvent->fX, pEvent->fY) ) {
				if ( __xgeXuiRectContains(tThumb, pEvent->fX, pEvent->fY) ) {
					pGrid->tBase.bDraggingThumb = 1;
					pGrid->tBase.fDragY = pEvent->fY;
					pGrid->tBase.fDragScrollY = pGrid->tBase.fScrollY;
					xgeXuiSetPointerCapture(pGrid->tBase.pContext, pEvent->iPointerId, pGrid->tBase.pWidget);
				} else {
					xgeXuiPropertyGridSetScroll(pGrid, pGrid->tBase.fScrollY + ((pEvent->fY < tThumb.fY) ? -pGrid->tBase.pWidget->tContentRect.fH : pGrid->tBase.pWidget->tContentRect.fH));
				}
				return XGE_XUI_EVENT_CONSUMED;
			}
			iVisible = __xgeXuiPropertyGridVisibleAt(pGrid, pEvent->fY);
			if ( iVisible < 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiPropertyGridSetFocusVisible(pGrid, iVisible);
			iItem = pGrid->arrVisible[iVisible];
			if ( pGrid->arrItems[iItem].iCategory ) {
				xgeXuiPropertyGridEndEdit(pGrid, 1);
				pGrid->arrItems[iItem].bExpanded = !pGrid->arrItems[iItem].bExpanded;
				if ( pGrid->iSelected >= 0 && pGrid->arrItems[pGrid->iSelected].iParentCategory == iItem && !pGrid->arrItems[iItem].bExpanded ) {
					pGrid->iSelected = -1;
					__xgeXuiPropertyGridSyncBaseState(pGrid);
				}
				__xgeXuiPropertyGridRebuildVisible(pGrid);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pGrid->iEditing >= 0 && pGrid->iEditing != iItem ) {
				xgeXuiPropertyGridEndEdit(pGrid, 1);
			}
			xgeXuiPropertyGridSetSelected(pGrid, iItem);
			pGrid->iSelectCount++;
			if ( pGrid->tBase.procSelect != NULL ) {
				pGrid->tBase.procSelect(pGrid->tBase.pWidget, iItem, pGrid->tBase.pSelectUser);
			}
			if ( !pGrid->arrItems[iItem].bReadonly && pEvent->fX >= (pGrid->tBase.pWidget->tContentRect.fX + pGrid->fNameWidth) ) {
				xgeXuiPropertyGridBeginEdit(pGrid, iItem);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			if ( pGrid->tBase.bDraggingThumb == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( xgeXuiGetPointerCapture(pGrid->tBase.pContext, pEvent->iPointerId) != pGrid->tBase.pWidget ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			__xgeXuiPropertyGridSetScrollFromThumbDrag(pGrid, pEvent->fY);
			pGrid->tBase.bDraggingThumb = 0;
			if ( pGrid->tBase.pContext != NULL && xgeXuiGetPointerCapture(pGrid->tBase.pContext, pEvent->iPointerId) == pGrid->tBase.pWidget ) {
				xgeXuiSetPointerCapture(pGrid->tBase.pContext, pEvent->iPointerId, NULL);
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_XUI_CAPTURE_LOST:
		case XGE_EVENT_XUI_CAPTURE_CANCEL:
			pGrid->tBase.bDraggingThumb = 0;
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			__xgeXuiPropertyGridSetHoverVisible(pGrid, -1);
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
	xge_rect_t tEdit;
	xge_rect_t tCursor;
	xge_vec2_t tSize;
	char sPrefix[XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY];
	int iVisible;
	int iItem;
	int iCursor;
	uint32_t iColor;
	uint32_t iText;

	pGrid = (xge_xui_property_grid)pUser;
	if ( (pWidget == NULL) || (pGrid == NULL) ) {
		return;
	}
	for ( iVisible = 0; iVisible < pGrid->iVisibleCount; iVisible++ ) {
		iItem = pGrid->arrVisible[iVisible];
		pItem = &pGrid->arrItems[iItem];
		tRow.fX = pWidget->tContentRect.fX;
		tRow.fY = pWidget->tContentRect.fY + (float)iVisible * pGrid->tBase.fItemHeight - pGrid->tBase.fScrollY;
		tRow.fW = pWidget->tContentRect.fW;
		tRow.fH = pGrid->tBase.fItemHeight;
		if ( (tRow.fY + tRow.fH) < pWidget->tContentRect.fY || tRow.fY > (pWidget->tContentRect.fY + pWidget->tContentRect.fH) ) {
			continue;
		}
		pItem->tRect = tRow;
		iColor = pItem->iCategory ? pGrid->iCategoryColor : pGrid->iRowColor;
		if ( iVisible == pGrid->tBase.iHover && !pItem->iCategory ) {
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
				tValue = __xgeXuiPropertyGridValueRect(pGrid, tRow);
				iText = pItem->bError ? pGrid->iErrorColor : (pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iValueColor);
				if ( pGrid->iEditing == iItem ) {
					tEdit = tValue;
					tEdit.fX += 2.0f;
					tEdit.fY += 2.0f;
					tEdit.fW -= 4.0f;
					tEdit.fH -= 4.0f;
					if ( tEdit.fW < 1.0f ) {
						tEdit.fW = 1.0f;
					}
					if ( tEdit.fH < 1.0f ) {
						tEdit.fH = 1.0f;
					}
					__xgeXuiHostDrawRect(tEdit, XGE_COLOR_RGBA(255, 255, 255, 255));
					__xgeXuiHostDrawBorderRect(tEdit, 1.0f, pGrid->iGridColor);
					tEdit.fX += 4.0f;
					tEdit.fW -= 8.0f;
					__xgeXuiHostDrawTextRect(pGrid->pFont, pGrid->tEditText.sText != NULL ? pGrid->tEditText.sText : "", tEdit, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
					iCursor = xgeXuiTextGetCursor(&pGrid->tEditText);
					if ( iCursor >= XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY ) {
						iCursor = XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY - 1;
					}
					if ( iCursor > 0 && pGrid->tEditText.sText != NULL ) {
						memcpy(sPrefix, pGrid->tEditText.sText, (size_t)iCursor);
						sPrefix[iCursor] = 0;
						tSize = __xgeXuiHostMeasureText(pGrid->pFont, sPrefix);
					} else {
						tSize.fX = 0.0f;
						tSize.fY = 0.0f;
					}
					tCursor.fX = tEdit.fX + tSize.fX + 1.0f;
					if ( tCursor.fX > (tEdit.fX + tEdit.fW - 1.0f) ) {
						tCursor.fX = tEdit.fX + tEdit.fW - 1.0f;
					}
					tCursor.fY = tEdit.fY + 3.0f;
					tCursor.fW = 1.0f;
					tCursor.fH = tEdit.fH - 6.0f;
					__xgeXuiHostDrawRect(tCursor, pGrid->iTextColor);
				} else {
					__xgeXuiHostDrawTextRect(pGrid->pFont, pItem->sValue != NULL ? pItem->sValue : "", tValue, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
				}
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
		if ( pGrid->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_COMPACT ) {
			tThumb.fX += (tThumb.fW - 4.0f) * 0.5f;
			tThumb.fW = 4.0f;
			__xgeXuiHostDrawRoundedRect(tThumb, pGrid->tBase.iThumbColor, 2.0f);
		} else {
			__xgeXuiHostDrawRect(tBar, XGE_COLOR_RGBA(255, 255, 255, 255));
			__xgeXuiHostDrawBorderRect(tBar, 1.0f, pGrid->iGridColor);
			__xgeXuiHostDrawRect(tThumb, pGrid->tBase.iThumbColor);
		}
	}
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pGrid->iGridColor);
}
