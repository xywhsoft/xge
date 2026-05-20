static uint32_t __xgeXuiPropertyGridHoverColor(uint32_t iRow)
{
	int iR = (int)XGE_COLOR_GET_R(iRow) + 18;
	int iG = (int)XGE_COLOR_GET_G(iRow) + 18;
	int iB = (int)XGE_COLOR_GET_B(iRow) + 18;
	return XGE_COLOR_RGBA(iR > 255 ? 255 : iR, iG > 255 ? 255 : iG, iB > 255 ? 255 : iB, XGE_COLOR_GET_A(iRow));
}

static void __xgeXuiPropertyGridClamp(xge_xui_property_grid pGrid);
static float __xgeXuiPropertyGridMaxScroll(xge_xui_property_grid pGrid);
static void __xgeXuiPropertyGridLayoutEditInput(xge_xui_property_grid pGrid);
static int __xgeXuiPropertyGridEditInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static xge_rect_t __xgeXuiPropertyGridItemValueRect(xge_xui_property_grid pGrid, int iItem);
static xge_rect_t __xgeXuiPropertyGridItemEditRect(xge_xui_property_grid pGrid, int iItem);
static void xgeXuiPropertyGridPaintAfterProc(xge_xui_widget pWidget, void* pUser);

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

static void __xgeXuiPropertyGridFreeEnumItems(xge_xui_property_grid_item_t* pItem)
{
	if ( pItem == NULL ) {
		return;
	}
	free(pItem->arrEnumItems);
	pItem->arrEnumItems = NULL;
	pItem->iEnumItemCount = 0;
}

static int __xgeXuiPropertyGridValueIsTrue(const char* sValue)
{
	if ( sValue == NULL ) {
		return 0;
	}
	return (strcmp(sValue, "true") == 0) || (strcmp(sValue, "1") == 0) || (strcmp(sValue, "yes") == 0) || (strcmp(sValue, "on") == 0);
}

static int __xgeXuiPropertyGridItemHasAction(xge_xui_property_grid_item_t* pItem)
{
	return (pItem != NULL) && !pItem->iCategory && pItem->bActionEnabled && pItem->sActionText[0] != 0;
}

static float __xgeXuiPropertyGridActionReserve(xge_xui_property_grid_item_t* pItem)
{
	return __xgeXuiPropertyGridItemHasAction(pItem) ? 24.0f : 0.0f;
}

static void __xgeXuiPropertyGridPlaceEditWidget(xge_xui_property_grid pGrid, xge_rect_t tValue)
{
	xge_rect_t tLocal;
	xge_rect_t tContent;

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) || (pGrid->pEditWidget == NULL) ) {
		return;
	}
	tLocal = tValue;
	tLocal.fX -= pGrid->tBase.pWidget->tContentRect.fX;
	tLocal.fY -= pGrid->tBase.pWidget->tContentRect.fY;
	tContent = tValue;
	pGrid->pEditWidget->tLocalRect = tLocal;
	pGrid->pEditWidget->tRect = tValue;
	pGrid->pEditWidget->tContentRect = tContent;
	xgeXuiWidgetMarkPaint(pGrid->pEditWidget);
}

static xge_rect_t __xgeXuiPropertyGridActionRect(xge_xui_property_grid pGrid, int iItem)
{
	xge_xui_property_grid_item_t* pItem;
	xge_rect_t tValue;
	xge_rect_t tAction;
	float fMarkReserve;

	memset(&tAction, 0, sizeof(tAction));
	if ( (pGrid == NULL) || (iItem < 0) || (iItem >= pGrid->iItemCount) ) {
		return tAction;
	}
	pItem = &pGrid->arrItems[iItem];
	if ( !__xgeXuiPropertyGridItemHasAction(pItem) ) {
		return tAction;
	}
	tValue = __xgeXuiPropertyGridItemValueRect(pGrid, iItem);
	fMarkReserve = (pItem->bDefaultChanged || pItem->bError || pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_COLOR) ? 12.0f : 0.0f;
	tAction.fW = 20.0f;
	tAction.fH = (tValue.fH > 4.0f) ? (tValue.fH - 4.0f) : tValue.fH;
	tAction.fX = tValue.fX + tValue.fW - fMarkReserve - tAction.fW - 2.0f;
	tAction.fY = tValue.fY + (tValue.fH - tAction.fH) * 0.5f;
	if ( tAction.fX < tValue.fX ) {
		tAction.fX = tValue.fX;
	}
	if ( tAction.fW < 0.0f ) {
		tAction.fW = 0.0f;
	}
	if ( tAction.fH < 0.0f ) {
		tAction.fH = 0.0f;
	}
	return tAction;
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

static int __xgeXuiPropertyGridEnumIndex(xge_xui_property_grid_item_t* pItem, const char* sValue)
{
	int i;

	if ( pItem == NULL ) {
		return -1;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	for ( i = 0; i < pItem->iEnumItemCount; i++ ) {
		if ( pItem->arrEnumItems[i] != NULL && strcmp(pItem->arrEnumItems[i], sValue) == 0 ) {
			return i;
		}
	}
	return -1;
}

static xge_rect_t __xgeXuiPropertyGridItemValueRect(xge_xui_property_grid pGrid, int iItem)
{
	xge_rect_t tRow;
	int iVisible;

	memset(&tRow, 0, sizeof(tRow));
	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) ) {
		return tRow;
	}
	for ( iVisible = 0; iVisible < pGrid->iVisibleCount; iVisible++ ) {
		if ( pGrid->arrVisible[iVisible] == iItem ) {
			tRow.fX = pGrid->tBase.pWidget->tContentRect.fX;
			tRow.fY = pGrid->tBase.pWidget->tContentRect.fY + (float)iVisible * pGrid->tBase.fItemHeight - pGrid->tBase.fScrollY;
			tRow.fW = pGrid->tBase.pWidget->tContentRect.fW;
			tRow.fH = pGrid->tBase.fItemHeight;
			return __xgeXuiPropertyGridValueRect(pGrid, tRow);
		}
	}
	return tRow;
}

static xge_rect_t __xgeXuiPropertyGridItemEditRect(xge_xui_property_grid pGrid, int iItem)
{
	xge_rect_t tRow;
	xge_rect_t tEdit;
	float fBarW;
	int iVisible;

	memset(&tEdit, 0, sizeof(tEdit));
	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) ) {
		return tEdit;
	}
	for ( iVisible = 0; iVisible < pGrid->iVisibleCount; iVisible++ ) {
		if ( pGrid->arrVisible[iVisible] == iItem ) {
			tRow.fX = pGrid->tBase.pWidget->tContentRect.fX;
			tRow.fY = pGrid->tBase.pWidget->tContentRect.fY + (float)iVisible * pGrid->tBase.fItemHeight - pGrid->tBase.fScrollY;
			tRow.fW = pGrid->tBase.pWidget->tContentRect.fW;
			tRow.fH = pGrid->tBase.fItemHeight;
			tEdit = tRow;
			tEdit.fX += pGrid->fNameWidth - 1.0f;
			tEdit.fW -= pGrid->fNameWidth - 1.0f;
			tEdit.fY -= 1.0f;
			tEdit.fH += 1.0f;
			if ( __xgeXuiPropertyGridMaxScroll(pGrid) > 0.0f ) {
				fBarW = (pGrid->tBase.iScrollbarMode == XGE_XUI_SCROLLBAR_MODE_FULL) ? 16.0f : 5.0f;
				tEdit.fW -= fBarW;
			}
			if ( tEdit.fW < 0.0f ) {
				tEdit.fW = 0.0f;
			}
			return tEdit;
		}
	}
	return tEdit;
}

static void __xgeXuiPropertyGridLayoutEditInput(xge_xui_property_grid pGrid)
{
	xge_xui_property_grid_item_t* pItem;
	xge_rect_t tValue;
	float fReserve;

	if ( (pGrid == NULL) || (pGrid->pEditWidget == NULL) ) {
		return;
	}
	if ( (pGrid->iEditing < 0) || (pGrid->iEditing >= pGrid->iItemCount) ) {
		xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
		return;
	}
	pItem = &pGrid->arrItems[pGrid->iEditing];
	tValue = __xgeXuiPropertyGridItemEditRect(pGrid, pGrid->iEditing);
	fReserve = __xgeXuiPropertyGridActionReserve(pItem);
	if ( fReserve > 0.0f ) {
		tValue.fW -= fReserve;
		if ( tValue.fW < 0.0f ) {
			tValue.fW = 0.0f;
		}
	}
	if ( (tValue.fW <= 0.0f) || (tValue.fH <= 0.0f) ) {
		xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
		return;
	}
	__xgeXuiPropertyGridPlaceEditWidget(pGrid, tValue);
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 1);
}

static void __xgeXuiPropertyGridLayoutEnumPopup(xge_xui_property_grid pGrid)
{
	xge_rect_t tValue;
	xge_rect_t tPopup;
	float fHeight;
	int iIndex;

	if ( (pGrid == NULL) || (pGrid->pEnumPopupWidget == NULL) || (pGrid->pEnumListWidget == NULL) || (pGrid->pEnumPopup == NULL) || (pGrid->pEnumList == NULL) ) {
		return;
	}
	iIndex = pGrid->iEnumEditing;
	if ( (iIndex < 0) || (iIndex >= pGrid->iItemCount) ) {
		return;
	}
	tValue = __xgeXuiPropertyGridItemValueRect(pGrid, iIndex);
	fHeight = (float)pGrid->arrItems[iIndex].iEnumItemCount * pGrid->pEnumList->tBase.fItemHeight + 4.0f;
	if ( fHeight > 180.0f ) {
		fHeight = 180.0f;
	}
	if ( fHeight < pGrid->pEnumList->tBase.fItemHeight + 4.0f ) {
		fHeight = pGrid->pEnumList->tBase.fItemHeight + 4.0f;
	}
	tPopup.fX = tValue.fX;
	tPopup.fY = tValue.fY + tValue.fH + 1.0f;
	tPopup.fW = tValue.fW;
	tPopup.fH = fHeight;
	if ( tPopup.fW < 96.0f ) {
		tPopup.fW = 96.0f;
	}
	xgeXuiWidgetSetRect(pGrid->pEnumPopupWidget, tPopup);
	xgeXuiPopupSetAnchorRect(pGrid->pEnumPopup, tValue);
	xgeXuiPopupSetOffset(pGrid->pEnumPopup, 0.0f, 1.0f);
	xgeXuiPopupApplyPlacement(pGrid->pEnumPopup);
	xgeXuiWidgetSetRect(pGrid->pEnumListWidget, (xge_rect_t){ 2.0f, 2.0f, tPopup.fW - 4.0f, tPopup.fH - 4.0f });
}

static void __xgeXuiPropertyGridCloseEnum(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( pGrid->pEnumPopup != NULL ) {
		xgeXuiPopupSetOpen(pGrid->pEnumPopup, 0);
	}
	pGrid->iEnumEditing = -1;
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

static void __xgeXuiPropertyGridEnumPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_property_grid pGrid;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	if ( pGrid != NULL ) {
		pGrid->iEnumEditing = -1;
		xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
	}
}

static void __xgeXuiPropertyGridEnumSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_item_t* pItem;
	int iProp;
	const char* sOld;
	const char* sNew;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	if ( (pGrid == NULL) || (pGrid->iEnumEditing < 0) || (pGrid->iEnumEditing >= pGrid->iItemCount) ) {
		return;
	}
	iProp = pGrid->iEnumEditing;
	pItem = &pGrid->arrItems[iProp];
	if ( (iIndex < 0) || (iIndex >= pItem->iEnumItemCount) || (pItem->arrEnumItems[iIndex] == NULL) ) {
		__xgeXuiPropertyGridCloseEnum(pGrid);
		return;
	}
	sOld = xgeXuiPropertyGridGetValue(pGrid, iProp);
	sNew = pItem->arrEnumItems[iIndex];
	if ( strcmp(sOld, sNew) != 0 ) {
		__xgeXuiPropertyGridCopyValue(pItem, sNew);
		if ( pGrid->procChange != NULL ) {
			pGrid->procChange(pGrid->tBase.pWidget, iProp, xgeXuiPropertyGridGetValue(pGrid, iProp), pGrid->pChangeUser);
		}
	}
	__xgeXuiPropertyGridCloseEnum(pGrid);
}

static int __xgeXuiPropertyGridEditInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_property_grid pGrid;
	int iResult;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	if ( (pGrid == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ENTER) ) {
		xgeXuiPropertyGridEndEdit(pGrid, 1);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		xgeXuiPropertyGridEndEdit(pGrid, 0);
		return XGE_XUI_EVENT_CONSUMED;
	}
	iResult = xgeXuiInputEvent(&pGrid->tEditInput, pEvent);
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		if ( (pGrid->tEditInput.pDefaultMenu != NULL) && (xgeXuiMenuIsOpen(pGrid->tEditInput.pDefaultMenu) != 0) ) {
			return iResult;
		}
		xgeXuiPropertyGridEndEdit(pGrid, 1);
	}
	return iResult;
}

int xgeXuiPropertyGridInit(xge_xui_property_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget)
{
	int iRet;
	const xge_xui_theme_t* pTheme;

	if ( (pGrid == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pGrid, 0, sizeof(*pGrid));
	pTheme = xgeXuiGetTheme(pContext);
	pGrid->pEditWidget = xgeXuiWidgetCreate();
	pGrid->pEnumPopupWidget = xgeXuiWidgetCreate();
	pGrid->pEnumListWidget = xgeXuiWidgetCreate();
	pGrid->pEnumPopup = (xge_xui_popup)malloc(sizeof(xge_xui_popup_t));
	pGrid->pEnumList = (xge_xui_list_view)malloc(sizeof(xge_xui_list_view_t));
	if ( (pGrid->pEditWidget == NULL) || (pGrid->pEnumPopupWidget == NULL) || (pGrid->pEnumListWidget == NULL) || (pGrid->pEnumPopup == NULL) || (pGrid->pEnumList == NULL) ) {
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pEnumPopupWidget);
		xgeXuiWidgetFree(pGrid->pEnumListWidget);
		free(pGrid->pEnumPopup);
		free(pGrid->pEnumList);
		memset(pGrid, 0, sizeof(*pGrid));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeXuiVirtualViewBaseInit(&pGrid->tBase, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pEnumPopupWidget);
		xgeXuiWidgetFree(pGrid->pEnumListWidget);
		free(pGrid->pEnumPopup);
		free(pGrid->pEnumList);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	if ( pWidget->pLayoutUser == &pGrid->tBase && pWidget->procLayout == xgeXuiVirtualViewBaseLayoutProc ) {
		pWidget->pLayoutUser = NULL;
		pWidget->procLayout = NULL;
	}
	pGrid->iSelected = -1;
	pGrid->iHover = -1;
	pGrid->iEditing = -1;
	pGrid->iEnumEditing = -1;
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
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPropertyGridEventProc, NULL);
	pWidget->procPaint = xgeXuiPropertyGridPaintProc;
	pWidget->procPaintAfter = xgeXuiPropertyGridPaintAfterProc;
	pWidget->pUser = pGrid;
	iRet = xgeXuiInputInit(&pGrid->tEditInput, pContext, pGrid->pEditWidget, pTheme != NULL ? pTheme->pFont : NULL);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pEnumPopupWidget);
		xgeXuiWidgetFree(pGrid->pEnumListWidget);
		free(pGrid->pEnumPopup);
		free(pGrid->pEnumList);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	xgeXuiWidgetSetEvent(pGrid->pEditWidget, __xgeXuiPropertyGridEditInputEventProc, pGrid);
	xgeXuiWidgetSetPaddingPx(pGrid->pEditWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
	xgeXuiInputSetMaxLength(&pGrid->tEditInput, XGE_XUI_PROPERTY_GRID_VALUE_CAPACITY - 1);
	xgeXuiInputSetClearButton(&pGrid->tEditInput, 0);
	xgeXuiInputSetColors(&pGrid->tEditInput, pGrid->iValueColor, pGrid->iRowColor, pGrid->iSelectedColor, pGrid->iTextColor);
	xgeXuiWidgetAdd(pWidget, pGrid->pEditWidget);
	xgeXuiPopupInit(pGrid->pEnumPopup, pContext, pGrid->pEnumPopupWidget);
	xgeXuiPopupSetOwner(pGrid->pEnumPopup, pWidget);
	xgeXuiPopupSetFocusRestore(pGrid->pEnumPopup, pWidget);
	xgeXuiPopupSetPlacement(pGrid->pEnumPopup, XGE_XUI_OVERLAY_PLACEMENT_BOTTOM_LEFT);
	xgeXuiPopupSetClose(pGrid->pEnumPopup, __xgeXuiPropertyGridEnumPopupClose, pGrid);
	xgeXuiPopupSetBackground(pGrid->pEnumPopup, XGE_COLOR_RGBA(255, 255, 255, 255));
	xgeXuiPopupSetBorder(pGrid->pEnumPopup, pGrid->iGridColor);
	xgeXuiListViewInit(pGrid->pEnumList, pContext, pGrid->pEnumListWidget);
	xgeXuiWidgetSetPaddingPx(pGrid->pEnumListWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiListViewSetFont(pGrid->pEnumList, pTheme != NULL ? pTheme->pFont : NULL);
	xgeXuiListViewSetItemHeight(pGrid->pEnumList, pGrid->tBase.fItemHeight);
	xgeXuiListViewSetScrollbarMode(pGrid->pEnumList, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiListViewSetColors(pGrid->pEnumList, XGE_COLOR_RGBA(255, 255, 255, 255), pGrid->iRowColor, pGrid->iSelectedColor, pGrid->iTextColor, pGrid->tBase.iBarColor, pGrid->tBase.iThumbColor);
	pGrid->pEnumList->iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	xgeXuiListViewSetSelect(pGrid->pEnumList, __xgeXuiPropertyGridEnumSelect, pGrid);
	xgeXuiWidgetAddInternal(pGrid->pEnumPopupWidget, pGrid->pEnumListWidget);
	if ( xgeXuiOverlayAttach(pContext, pGrid->pEnumPopupWidget, pWidget, XGE_XUI_LAYER_POPUP) != XGE_OK ) {
		xgeXuiListViewUnit(pGrid->pEnumList);
		xgeXuiPopupUnit(pGrid->pEnumPopup);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pEnumPopupWidget);
		free(pGrid->pEnumPopup);
		free(pGrid->pEnumList);
		memset(pGrid, 0, sizeof(*pGrid));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiPropertyGridUnit(xge_xui_property_grid pGrid)
{
	xge_xui_widget pEditWidget;
	xge_xui_widget pPopupWidget;
	xge_xui_popup pPopup;
	xge_xui_list_view pList;
	int i;

	if ( pGrid == NULL ) {
		return;
	}
	for ( i = 0; i < pGrid->iItemCount; i++ ) {
		__xgeXuiPropertyGridFreeEnumItems(&pGrid->arrItems[i]);
	}
	xgeXuiReleaseWidgetCapture(pGrid->tBase.pContext, pGrid->tBase.pWidget);
	if ( pGrid->tBase.pWidget != NULL && pGrid->tBase.pWidget->pUser == pGrid ) {
		pGrid->tBase.pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pGrid->tBase.pWidget, NULL, NULL);
		pGrid->tBase.pWidget->procPaint = NULL;
		pGrid->tBase.pWidget->procPaintAfter = NULL;
	}
	if ( pGrid->tBase.pWidget != NULL && pGrid->tBase.pWidget->pLayoutUser == &pGrid->tBase && pGrid->tBase.pWidget->procLayout == xgeXuiVirtualViewBaseLayoutProc ) {
		pGrid->tBase.pWidget->pLayoutUser = NULL;
		pGrid->tBase.pWidget->procLayout = NULL;
	}
	pEditWidget = pGrid->pEditWidget;
	pPopupWidget = pGrid->pEnumPopupWidget;
	pPopup = pGrid->pEnumPopup;
	pList = pGrid->pEnumList;
	xgeXuiInputUnit(&pGrid->tEditInput);
	if ( pList != NULL ) {
		xgeXuiListViewUnit(pList);
	}
	if ( pPopup != NULL ) {
		xgeXuiPopupUnit(pPopup);
	}
	xgeXuiWidgetFree(pEditWidget);
	xgeXuiWidgetFree(pPopupWidget);
	free(pPopup);
	free(pList);
	memset(pGrid, 0, sizeof(*pGrid));
}

void xgeXuiPropertyGridClear(xge_xui_property_grid pGrid)
{
	int i;

	if ( pGrid == NULL ) {
		return;
	}
	for ( i = 0; i < pGrid->iItemCount; i++ ) {
		__xgeXuiPropertyGridFreeEnumItems(&pGrid->arrItems[i]);
	}
	pGrid->iItemCount = 0;
	pGrid->iVisibleCount = 0;
	pGrid->iSelected = -1;
	pGrid->iHover = -1;
	pGrid->iEditing = -1;
	__xgeXuiPropertyGridCloseEnum(pGrid);
	pGrid->tBase.fScrollY = 0.0f;
	pGrid->tBase.iItemCount = 0;
	pGrid->tBase.iSelected = -1;
	pGrid->tBase.iHover = -1;
	pGrid->tBase.iFocus = -1;
	if ( pGrid->pEnumList != NULL ) {
		xgeXuiListViewSetItems(pGrid->pEnumList, NULL, 0);
	}
	xgeXuiInputSetText(&pGrid->tEditInput, "");
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
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

void xgeXuiPropertyGridSetEnumItems(xge_xui_property_grid pGrid, int iIndex, const char** arrItems, int iCount)
{
	xge_xui_property_grid_item_t* pItem;
	const char** arrCopy;
	int i;

	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) || pGrid->arrItems[iIndex].iCategory ) {
		return;
	}
	if ( iCount < 0 ) {
		iCount = 0;
	}
	if ( iCount > XGE_XUI_PROPERTY_GRID_ENUM_CAPACITY ) {
		iCount = XGE_XUI_PROPERTY_GRID_ENUM_CAPACITY;
	}
	pItem = &pGrid->arrItems[iIndex];
	__xgeXuiPropertyGridFreeEnumItems(pItem);
	arrCopy = NULL;
	if ( iCount > 0 ) {
		arrCopy = (const char**)malloc(sizeof(const char*) * (size_t)iCount);
		if ( arrCopy == NULL ) {
			xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
			return;
		}
	}
	for ( i = 0; i < iCount; i++ ) {
		arrCopy[i] = (arrItems != NULL) ? arrItems[i] : NULL;
	}
	pItem->arrEnumItems = arrCopy;
	pItem->iEnumItemCount = iCount;
	if ( pGrid->iEnumEditing == iIndex && pGrid->pEnumList != NULL ) {
		xgeXuiListViewSetItems(pGrid->pEnumList, pItem->arrEnumItems, pItem->iEnumItemCount);
		xgeXuiListViewSetSelected(pGrid->pEnumList, __xgeXuiPropertyGridEnumIndex(pItem, xgeXuiPropertyGridGetValue(pGrid, iIndex)));
		__xgeXuiPropertyGridLayoutEnumPopup(pGrid);
	}
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridSetActionButton(xge_xui_property_grid pGrid, int iIndex, const char* sText, int iAction, int bEnabled)
{
	xge_xui_property_grid_item_t* pItem;
	int iLen;

	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) || pGrid->arrItems[iIndex].iCategory ) {
		return;
	}
	if ( sText == NULL ) {
		sText = "";
	}
	pItem = &pGrid->arrItems[iIndex];
	iLen = (int)strlen(sText);
	if ( iLen >= (int)sizeof(pItem->sActionText) ) {
		iLen = (int)sizeof(pItem->sActionText) - 1;
	}
	memcpy(pItem->sActionText, sText, (size_t)iLen);
	pItem->sActionText[iLen] = 0;
	pItem->iAction = iAction;
	pItem->bActionEnabled = bEnabled ? 1 : 0;
	__xgeXuiPropertyGridLayoutEditInput(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
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
		xgeXuiInputSetText(&pGrid->tEditInput, pGrid->arrItems[iIndex].sValue);
	}
	if ( pGrid->iEnumEditing == iIndex && pGrid->pEnumList != NULL ) {
		xgeXuiListViewSetSelected(pGrid->pEnumList, __xgeXuiPropertyGridEnumIndex(&pGrid->arrItems[iIndex], pGrid->arrItems[iIndex].sValue));
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
	xge_rect_t tValue;

	if ( (pGrid == NULL) || (iIndex < 0) || (iIndex >= pGrid->iItemCount) ) {
		return;
	}
	pItem = &pGrid->arrItems[iIndex];
	if ( pItem->iCategory || pItem->bReadonly ) {
		return;
	}
	__xgeXuiPropertyGridCloseEnum(pGrid);
	if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_BOOL ) {
		xgeXuiPropertyGridSetValue(pGrid, iIndex, __xgeXuiPropertyGridValueIsTrue(xgeXuiPropertyGridGetValue(pGrid, iIndex)) ? "false" : "true");
		if ( pGrid->procChange != NULL ) {
			pGrid->procChange(pGrid->tBase.pWidget, iIndex, xgeXuiPropertyGridGetValue(pGrid, iIndex), pGrid->pChangeUser);
		}
		return;
	}
	if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM && pItem->iEnumItemCount > 0 && pGrid->pEnumPopup != NULL && pGrid->pEnumList != NULL ) {
		pGrid->iEditing = -1;
		pGrid->iEnumEditing = iIndex;
		xgeXuiPropertyGridSetSelected(pGrid, iIndex);
		xgeXuiListViewSetItems(pGrid->pEnumList, pItem->arrEnumItems, pItem->iEnumItemCount);
		xgeXuiListViewSetItemHeight(pGrid->pEnumList, pGrid->tBase.fItemHeight);
		xgeXuiListViewSetSelected(pGrid->pEnumList, __xgeXuiPropertyGridEnumIndex(pItem, xgeXuiPropertyGridGetValue(pGrid, iIndex)));
		__xgeXuiPropertyGridLayoutEnumPopup(pGrid);
		xgeXuiPopupSetOpen(pGrid->pEnumPopup, 1);
		__xgeXuiLayoutWidget(pGrid->pEnumPopupWidget, pGrid->pEnumPopupWidget->tRect);
		xgeXuiSetFocus(pGrid->tBase.pContext, pGrid->pEnumListWidget);
		xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
		return;
	}
	xgeXuiPropertyGridSetSelected(pGrid, iIndex);
	xgeXuiInputSetText(&pGrid->tEditInput, xgeXuiPropertyGridGetValue(pGrid, iIndex));
	xgeXuiInputSetSelection(&pGrid->tEditInput, 0, pGrid->tEditInput.tText.iSize);
	tValue = __xgeXuiPropertyGridItemEditRect(pGrid, iIndex);
	if ( (tValue.fW > 0.0f) && (tValue.fH > 0.0f) ) {
		__xgeXuiPropertyGridPlaceEditWidget(pGrid, tValue);
		xgeXuiWidgetSetVisible(pGrid->pEditWidget, 1);
	}
	xgeXuiSetFocus(pGrid->tBase.pContext, pGrid->pEditWidget);
	pGrid->iEditing = iIndex;
	__xgeXuiPropertyGridLayoutEditInput(pGrid);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridEndEdit(xge_xui_property_grid pGrid, int bCommit)
{
	int iIndex;
	const char* sOld;
	const char* sNew;

	if ( pGrid == NULL ) {
		return;
	}
	if ( pGrid->iEnumEditing >= 0 ) {
		__xgeXuiPropertyGridCloseEnum(pGrid);
	}
	if ( (pGrid->iEditing < 0) || (pGrid->iEditing >= pGrid->iItemCount) ) {
		return;
	}
	iIndex = pGrid->iEditing;
	pGrid->iEditing = -1;
	if ( bCommit != 0 && !pGrid->arrItems[iIndex].bReadonly && !pGrid->arrItems[iIndex].iCategory ) {
		sOld = xgeXuiPropertyGridGetValue(pGrid, iIndex);
		sNew = xgeXuiInputGetText(&pGrid->tEditInput);
		if ( strcmp(sOld, sNew) != 0 ) {
			xgeXuiPropertyGridSetValue(pGrid, iIndex, sNew);
			if ( pGrid->procChange != NULL ) {
				pGrid->procChange(pGrid->tBase.pWidget, iIndex, xgeXuiPropertyGridGetValue(pGrid, iIndex), pGrid->pChangeUser);
			}
		}
	}
	xgeXuiInputSetText(&pGrid->tEditInput, "");
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

void xgeXuiPropertyGridSetFont(xge_xui_property_grid pGrid, xge_font pFont)
{
	if ( pGrid != NULL ) {
		pGrid->pFont = pFont;
		xgeXuiInputSetFont(&pGrid->tEditInput, pFont);
		if ( pGrid->pEnumList != NULL ) {
			xgeXuiListViewSetFont(pGrid->pEnumList, pFont);
		}
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
	if ( pGrid->pEnumList != NULL ) {
		xgeXuiListViewSetItemHeight(pGrid->pEnumList, pGrid->tBase.fItemHeight);
	}
	__xgeXuiPropertyGridLayoutEditInput(pGrid);
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

void xgeXuiPropertyGridSetAction(xge_xui_property_grid pGrid, xge_xui_property_grid_action_proc procAction, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procAction = procAction;
		pGrid->pActionUser = pUser;
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
	xgeXuiInputSetColors(&pGrid->tEditInput, iText, iRow, iSelected, iText);
	if ( pGrid->pEnumPopup != NULL ) {
		xgeXuiPopupSetBorder(pGrid->pEnumPopup, iGrid);
	}
	if ( pGrid->pEnumList != NULL ) {
		xgeXuiListViewSetColors(pGrid->pEnumList, XGE_COLOR_RGBA(255, 255, 255, 255), iRow, iSelected, iText, pGrid->tBase.iBarColor, pGrid->tBase.iThumbColor);
		pGrid->pEnumList->iBorderColor = XGE_COLOR_RGBA(0, 0, 0, 0);
	}
	xgeXuiWidgetMarkPaint(pGrid->tBase.pWidget);
}

int xgeXuiPropertyGridEvent(xge_xui_property_grid pGrid, const xge_event_t* pEvent)
{
	xge_rect_t tBar;
	xge_rect_t tThumb;
	xge_rect_t tValue;
	xge_rect_t tAction;
	int iInside;
	int iVisible;
	int iItem;

	if ( (pGrid == NULL) || (pGrid->tBase.pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pGrid->tBase.pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_CONTEXT_BEGIN:
		case XGE_EVENT_XUI_CONTEXT_UPDATE:
		case XGE_EVENT_XUI_CONTEXT_END:
		case XGE_EVENT_XUI_CONTEXT_CANCEL:
			if ( (pGrid->iEditing >= 0) && (pGrid->pEditWidget != NULL) && __xgeXuiRectContains(pGrid->pEditWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_TEXT:
		case XGE_EVENT_IME_START:
		case XGE_EVENT_IME_UPDATE:
		case XGE_EVENT_IME_END:
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_KEY_DOWN:
			if ( pGrid->tBase.pContext != NULL && pGrid->tBase.pContext->pFocus == pGrid->tBase.pWidget && pGrid->iSelected >= 0 && pGrid->iSelected < pGrid->iItemCount ) {
				if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
					if ( (pGrid->arrItems[pGrid->iSelected].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_BOOL) || (pGrid->arrItems[pGrid->iSelected].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM) ) {
						xgeXuiPropertyGridBeginEdit(pGrid, pGrid->iSelected);
						return XGE_XUI_EVENT_CONSUMED;
					}
				}
				if ( pEvent->iParam1 == XGE_KEY_DOWN && pGrid->arrItems[pGrid->iSelected].iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM ) {
					xgeXuiPropertyGridBeginEdit(pGrid, pGrid->iSelected);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_OUT:
			if ( pGrid->pEnumPopup != NULL && xgeXuiPopupIsOpen(pGrid->pEnumPopup) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
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
			if ( (pGrid->iEditing >= 0) && (pGrid->pEditWidget != NULL) && __xgeXuiRectContains(pGrid->pEditWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( iInside == 0 ) {
				xgeXuiPropertyGridEndEdit(pGrid, 1);
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pGrid->iEnumEditing >= 0 ) {
				tValue = __xgeXuiPropertyGridItemValueRect(pGrid, pGrid->iEnumEditing);
				if ( __xgeXuiRectContains(tValue, pEvent->fX, pEvent->fY) ) {
					__xgeXuiPropertyGridCloseEnum(pGrid);
					return XGE_XUI_EVENT_CONSUMED;
				}
				__xgeXuiPropertyGridCloseEnum(pGrid);
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
			tAction = __xgeXuiPropertyGridActionRect(pGrid, iItem);
			if ( tAction.fW > 0.0f && __xgeXuiRectContains(tAction, pEvent->fX, pEvent->fY) ) {
				xgeXuiPropertyGridEndEdit(pGrid, 1);
				xgeXuiPropertyGridSetSelected(pGrid, iItem);
				pGrid->iSelectCount++;
				if ( pGrid->tBase.procSelect != NULL ) {
					pGrid->tBase.procSelect(pGrid->tBase.pWidget, iItem, pGrid->tBase.pSelectUser);
				}
				if ( pGrid->procAction != NULL ) {
					pGrid->procAction(pGrid->tBase.pWidget, iItem, pGrid->arrItems[iItem].iAction, pGrid->pActionUser);
				}
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
	xge_rect_t tText;
	xge_rect_t tArrow;
	xge_rect_t tBox;
	xge_rect_t tCheck;
	xge_rect_t tAction;
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
	__xgeXuiPropertyGridLayoutEditInput(pGrid);
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
				} else {
					tText = tValue;
					if ( __xgeXuiPropertyGridItemHasAction(pItem) ) {
						tText.fW -= __xgeXuiPropertyGridActionReserve(pItem);
						if ( tText.fW < 0.0f ) {
							tText.fW = 0.0f;
						}
					}
					if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_BOOL ) {
						tBox.fW = 13.0f;
						tBox.fH = 13.0f;
						tBox.fX = tValue.fX + 5.0f;
						tBox.fY = tValue.fY + (tValue.fH - tBox.fH) * 0.5f;
						__xgeXuiHostDrawRect(tBox, XGE_COLOR_RGBA(255, 255, 255, 255));
						__xgeXuiHostDrawBorderRect(tBox, 1.0f, pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iGridColor);
						if ( __xgeXuiPropertyGridValueIsTrue(pItem->sValue) ) {
							tCheck = tBox;
							tCheck.fX += 1.0f;
							tCheck.fY += 1.0f;
							tCheck.fW -= 2.0f;
							tCheck.fH -= 2.0f;
							__xgeXuiBuiltinAssetDraw(tCheck, XGE_XUI_ASSET_PROPERTY_CHECK_12, pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iValueColor);
						}
						tText.fX += 24.0f;
						tText.fW -= 24.0f;
						if ( tText.fW < 0.0f ) {
							tText.fW = 0.0f;
						}
					} else if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM ) {
						tText.fW -= 18.0f;
						if ( tText.fW < 0.0f ) {
							tText.fW = 0.0f;
						}
					}
					__xgeXuiHostDrawTextRect(pGrid->pFont, pItem->sValue != NULL ? pItem->sValue : "", tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
					if ( pItem->iEditor == XGE_XUI_PROPERTY_GRID_EDITOR_ENUM && !pItem->bReadonly ) {
						tArrow.fW = 8.0f;
						tArrow.fH = 8.0f;
						tArrow.fX = tValue.fX + tValue.fW - 13.0f;
						tArrow.fY = tValue.fY + (tValue.fH - tArrow.fH) * 0.5f;
						__xgeXuiBuiltinAssetDraw(tArrow, XGE_XUI_ASSET_CHEVRON_UP_8, iText);
					}
					tAction = __xgeXuiPropertyGridActionRect(pGrid, iItem);
					if ( tAction.fW > 0.0f && tAction.fH > 0.0f ) {
						__xgeXuiHostDrawRect(tAction, (iVisible == pGrid->iHover) ? pGrid->iHoverColor : XGE_COLOR_RGBA(238, 247, 254, 255));
						__xgeXuiHostDrawBorderRect(tAction, 1.0f, pGrid->iGridColor);
						__xgeXuiHostDrawTextRect(pGrid->pFont, pItem->sActionText, tAction, pItem->bReadonly ? pGrid->iReadonlyColor : pGrid->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
					}
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
	tLine.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
	tLine.fY = pWidget->tContentRect.fY;
	tLine.fW = 1.0f;
	tLine.fH = pWidget->tContentRect.fH;
	__xgeXuiHostDrawRect(tLine, pGrid->iGridColor);
}

static void xgeXuiPropertyGridPaintAfterProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_rect_t tLine;

	pGrid = (xge_xui_property_grid)pUser;
	if ( (pWidget == NULL) || (pGrid == NULL) ) {
		return;
	}
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pGrid->iGridColor);
	tLine.fX = pWidget->tContentRect.fX + pWidget->tContentRect.fW - 1.0f;
	tLine.fY = pWidget->tContentRect.fY;
	tLine.fW = 1.0f;
	tLine.fH = pWidget->tContentRect.fH;
	__xgeXuiHostDrawRect(tLine, pGrid->iGridColor);
}
