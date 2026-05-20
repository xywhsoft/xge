static int __xgeXuiPropertyGridCountProc(xge_xui_widget pWidget, void* pUser);
static int __xgeXuiPropertyGridCellProc(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser);
static int __xgeXuiPropertyGridMergeProc(xge_xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser);
static void __xgeXuiPropertyGridSetProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
static int __xgeXuiPropertyGridValidateProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
static void __xgeXuiPropertyGridChangeProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser);
static int __xgeXuiPropertyGridEditorProc(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, void* pUser);
static int __xgeXuiPropertyGridEditorConfigProc(xge_xui_widget pWidget, int iRow, int iColumn, int iType, xge_xui_table_grid_editor_config_t* pConfig, void* pUser);
static void __xgeXuiPropertyGridSelectProc(xge_xui_widget pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser);
static int __xgeXuiPropertyGridCategoryRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser);
static int __xgeXuiPropertyGridNameRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser);
static int __xgeXuiPropertyGridValueRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser);
static void __xgeXuiPropertyGridUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);

#define XGE_XUI_PROPERTY_GRID_VISIBLE_CATEGORY 1
#define XGE_XUI_PROPERTY_GRID_VISIBLE_PROPERTY 2

static void __xgeXuiPropertyGridCopy(char* sDst, int iDstSize, const char* sSrc)
{
	if ( (sDst == NULL) || (iDstSize <= 0) ) {
		return;
	}
	if ( sSrc == NULL ) {
		sSrc = "";
	}
	snprintf(sDst, (size_t)iDstSize, "%s", sSrc);
	sDst[iDstSize - 1] = 0;
}

static int __xgeXuiPropertyGridTextEqualNoCase(const char* sA, const char* sB)
{
	unsigned char cA;
	unsigned char cB;

	if ( (sA == NULL) || (sB == NULL) ) {
		return 0;
	}
	while ( (*sA != 0) && (*sB != 0) ) {
		cA = (unsigned char)*sA++;
		cB = (unsigned char)*sB++;
		if ( cA >= 'A' && cA <= 'Z' ) {
			cA = (unsigned char)(cA - 'A' + 'a');
		}
		if ( cB >= 'A' && cB <= 'Z' ) {
			cB = (unsigned char)(cB - 'A' + 'a');
		}
		if ( cA != cB ) {
			return 0;
		}
	}
	return (*sA == 0) && (*sB == 0);
}

static int __xgeXuiPropertyGridBoolText(const char* sText)
{
	if ( sText == NULL ) {
		return 0;
	}
	return (strcmp(sText, "1") == 0) ||
	       __xgeXuiPropertyGridTextEqualNoCase(sText, "true") ||
	       __xgeXuiPropertyGridTextEqualNoCase(sText, "yes") ||
	       __xgeXuiPropertyGridTextEqualNoCase(sText, "on") ||
	       __xgeXuiPropertyGridTextEqualNoCase(sText, "enabled") ||
	       __xgeXuiPropertyGridTextEqualNoCase(sText, "checked");
}

static void __xgeXuiPropertyGridSetDefaultStyle(xge_xui_property_grid_style_t* pStyle)
{
	if ( pStyle == NULL ) {
		return;
	}
	memset(pStyle, 0, sizeof(*pStyle));
	pStyle->iBackgroundColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pStyle->iGridColor = XGE_COLOR_RGBA(184, 211, 232, 255);
	pStyle->iCategoryBackgroundColor = XGE_COLOR_RGBA(218, 235, 249, 255);
	pStyle->iCategoryHoverColor = XGE_COLOR_RGBA(203, 226, 245, 255);
	pStyle->iCategoryTextColor = XGE_COLOR_RGBA(40, 84, 126, 255);
	pStyle->iCategoryIconColor = XGE_COLOR_RGBA(48, 122, 190, 255);
	pStyle->iNameBackgroundColor = XGE_COLOR_RGBA(235, 242, 249, 255);
	pStyle->iNameTextColor = XGE_COLOR_RGBA(58, 77, 96, 255);
	pStyle->iNameHoverColor = XGE_COLOR_RGBA(225, 236, 246, 255);
	pStyle->iValueBackgroundColor = XGE_COLOR_RGBA(248, 250, 253, 255);
	pStyle->iValueTextColor = XGE_COLOR_RGBA(42, 52, 66, 255);
	pStyle->iSelectedColor = XGE_COLOR_RGBA(190, 219, 242, 255);
	pStyle->iReadonlyTextColor = XGE_COLOR_RGBA(116, 128, 142, 255);
	pStyle->iInvalidColor = XGE_COLOR_RGBA(220, 64, 64, 230);
	pStyle->iDirtyColor = XGE_COLOR_RGBA(240, 166, 52, 230);
}

static int __xgeXuiPropertyGridValidCategory(xge_xui_property_grid pGrid, int iCategory)
{
	return (pGrid != NULL) && (iCategory >= 0) && (iCategory < pGrid->iCategoryCount);
}

static int __xgeXuiPropertyGridValidProperty(xge_xui_property_grid pGrid, int iProperty)
{
	return (pGrid != NULL) && (iProperty >= 0) && (iProperty < pGrid->iPropertyCount);
}

static int __xgeXuiPropertyGridVisiblePropertyRow(xge_xui_property_grid pGrid, int iProperty)
{
	int i;

	if ( pGrid == NULL ) {
		return -1;
	}
	for ( i = 0; i < pGrid->iVisibleCount; i++ ) {
		if ( pGrid->arrVisibleKind[i] == XGE_XUI_PROPERTY_GRID_VISIBLE_PROPERTY && pGrid->arrVisibleIndex[i] == iProperty ) {
			return i;
		}
	}
	return -1;
}

static int __xgeXuiPropertyGridVisibleRowProperty(xge_xui_property_grid pGrid, int iRow)
{
	if ( (pGrid == NULL) || (iRow < 0) || (iRow >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return (pGrid->arrVisibleKind[iRow] == XGE_XUI_PROPERTY_GRID_VISIBLE_PROPERTY) ? pGrid->arrVisibleIndex[iRow] : -1;
}

static int __xgeXuiPropertyGridVisibleRowCategory(xge_xui_property_grid pGrid, int iRow)
{
	if ( (pGrid == NULL) || (iRow < 0) || (iRow >= pGrid->iVisibleCount) ) {
		return -1;
	}
	return (pGrid->arrVisibleKind[iRow] == XGE_XUI_PROPERTY_GRID_VISIBLE_CATEGORY) ? pGrid->arrVisibleIndex[iRow] : -1;
}

static int __xgeXuiPropertyGridPropertyDirty(xge_xui_property_grid_property_t* pProp)
{
	if ( pProp == NULL ) {
		return 0;
	}
	if ( (pProp->iFlags & XGE_XUI_PROPERTY_FLAG_DIRTY) != 0 ) {
		return 1;
	}
	return (pProp->bAutoDirty != 0) && (pProp->sDefaultValue[0] != 0) && (strcmp(pProp->sValue, pProp->sDefaultValue) != 0);
}

static void __xgeXuiPropertyGridSyncColumns(xge_xui_property_grid pGrid)
{
	xge_rect_t tViewport;
	float fAvailableWidth;
	float fNameWidth;
	float fValueWidth;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) ) {
		return;
	}
	tViewport = xgeXuiScrollFrameGetViewportRect(pGrid->tGrid.tTable.pFrame);
	fAvailableWidth = (tViewport.fW > 1.0f) ? tViewport.fW : pGrid->pWidget->tContentRect.fW;
	if ( fAvailableWidth > 1.0f ) {
		fAvailableWidth -= 1.0f;
	}
	if ( fAvailableWidth < 40.0f ) {
		fAvailableWidth = 40.0f;
	}
	fNameWidth = pGrid->fNameWidth;
	if ( fNameWidth > fAvailableWidth - 40.0f ) {
		fNameWidth = fAvailableWidth - 40.0f;
	}
	if ( fNameWidth < 40.0f ) {
		fNameWidth = (fAvailableWidth > 80.0f) ? 40.0f : (fAvailableWidth * 0.5f);
	}
	fValueWidth = fAvailableWidth - fNameWidth;
	if ( fValueWidth < 1.0f ) {
		fValueWidth = 1.0f;
	}
	memset(pGrid->arrColumns, 0, sizeof(pGrid->arrColumns));
	pGrid->arrColumns[0].iId = 0;
	pGrid->arrColumns[0].sId = "name";
	pGrid->arrColumns[0].sTitle = "";
	pGrid->arrColumns[0].fWidth = fNameWidth;
	pGrid->arrColumns[0].fMinWidth = 60.0f;
	pGrid->arrColumns[0].bVisible = 1;
	pGrid->arrColumns[0].bVisibleSet = 1;
	pGrid->arrColumns[0].bResizable = 0;
	pGrid->arrColumns[0].bResizableSet = 1;
	pGrid->arrColumns[0].iAlign = XGE_TEXT_ALIGN_LEFT;
	pGrid->arrColumns[0].iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	pGrid->arrColumns[1] = pGrid->arrColumns[0];
	pGrid->arrColumns[1].iId = 1;
	pGrid->arrColumns[1].sId = "value";
	pGrid->arrColumns[1].fWidth = fValueWidth;
	pGrid->arrColumns[1].fMinWidth = 1.0f;
	xgeXuiTableGridSetColumns(&pGrid->tGrid, pGrid->arrColumns, 2);
}

static void __xgeXuiPropertyGridApplyStyle(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	xgeXuiTableGridSetColors(&pGrid->tGrid, pGrid->tStyle.iBackgroundColor, pGrid->tStyle.iCategoryBackgroundColor, pGrid->tStyle.iValueBackgroundColor, pGrid->tStyle.iSelectedColor, pGrid->tStyle.iGridColor, pGrid->tStyle.iValueTextColor);
	xgeXuiTableViewSetDisabledTextColor(&pGrid->tGrid.tTable, pGrid->tStyle.iReadonlyTextColor);
	if ( pGrid->pWidget != NULL ) {
		xgeXuiWidgetSetBackground(pGrid->pWidget, pGrid->tStyle.iBackgroundColor);
	}
}

static void __xgeXuiPropertyGridRebuildVisible(xge_xui_property_grid pGrid)
{
	int i;
	int iCategory;

	if ( pGrid == NULL ) {
		return;
	}
	pGrid->iVisibleCount = 0;
	for ( iCategory = 0; iCategory < pGrid->iCategoryCount && pGrid->iVisibleCount < XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY; iCategory++ ) {
		pGrid->arrVisibleKind[pGrid->iVisibleCount] = XGE_XUI_PROPERTY_GRID_VISIBLE_CATEGORY;
		pGrid->arrVisibleIndex[pGrid->iVisibleCount] = iCategory;
		pGrid->arrRows[pGrid->iVisibleCount].fHeight = pGrid->fCategoryHeight;
		pGrid->arrRows[pGrid->iVisibleCount].bSelected = 0;
		pGrid->arrRows[pGrid->iVisibleCount].bDisabled = 0;
		pGrid->arrRows[pGrid->iVisibleCount].bHasStyle = 1;
		pGrid->arrRows[pGrid->iVisibleCount].tStyle.iBackgroundColor = pGrid->tStyle.iCategoryBackgroundColor;
		pGrid->arrRows[pGrid->iVisibleCount].tStyle.iBorderColor = pGrid->tStyle.iGridColor;
		pGrid->iVisibleCount++;
		if ( pGrid->arrCategories[iCategory].bExpanded == 0 ) {
			continue;
		}
		for ( i = 0; i < pGrid->iPropertyCount && pGrid->iVisibleCount < XGE_XUI_PROPERTY_GRID_VISIBLE_CAPACITY; i++ ) {
			if ( pGrid->arrProperties[i].iCategory != iCategory ) {
				continue;
			}
			if ( (pGrid->arrProperties[i].iFlags & XGE_XUI_PROPERTY_FLAG_HIDDEN) != 0 ) {
				continue;
			}
			pGrid->arrVisibleKind[pGrid->iVisibleCount] = XGE_XUI_PROPERTY_GRID_VISIBLE_PROPERTY;
			pGrid->arrVisibleIndex[pGrid->iVisibleCount] = i;
			pGrid->arrRows[pGrid->iVisibleCount].fHeight = pGrid->fRowHeight;
			pGrid->arrRows[pGrid->iVisibleCount].bSelected = 0;
			pGrid->arrRows[pGrid->iVisibleCount].bDisabled = 0;
			pGrid->arrRows[pGrid->iVisibleCount].bHasStyle = 0;
			pGrid->iVisibleCount++;
		}
	}
	xgeXuiTableGridSetRows(&pGrid->tGrid, pGrid->arrRows, pGrid->iVisibleCount);
	xgeXuiTableViewRefresh(&pGrid->tGrid.tTable);
}

static int __xgeXuiPropertyGridCountProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	return (pUser != NULL) ? ((xge_xui_property_grid)pUser)->iVisibleCount : 0;
}

static int __xgeXuiPropertyGridCellProc(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	xge_xui_property_grid_category_t* pCategory;
	int iIndex;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	if ( (pGrid == NULL) || (pCell == NULL) || (iRow < 0) || (iRow >= pGrid->iVisibleCount) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	if ( pGrid->arrVisibleKind[iRow] == XGE_XUI_PROPERTY_GRID_VISIBLE_CATEGORY ) {
		iIndex = pGrid->arrVisibleIndex[iRow];
		pCategory = &pGrid->arrCategories[iIndex];
		pCell->sText = pCategory->sName;
		pCell->iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
		pCell->iColSpan = 2;
		pCell->procRenderer = __xgeXuiPropertyGridCategoryRenderer;
		pCell->pRendererUser = pGrid;
		pCell->bHasStyle = 1;
		pCell->tStyle.iBackgroundColor = pGrid->tStyle.iCategoryBackgroundColor;
		pCell->tStyle.iBorderColor = pGrid->tStyle.iGridColor;
		if ( iColumn > 0 ) {
			pCell->sText = "";
		}
		return XGE_OK;
	}
	iIndex = pGrid->arrVisibleIndex[iRow];
	pProp = &pGrid->arrProperties[iIndex];
	pCell->sText = (iColumn == 0) ? pProp->sName : pProp->sValue;
	pCell->sTooltip = ((pGrid->iDescriptionMode == XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP || pGrid->iDescriptionMode == XGE_XUI_PROPERTY_GRID_DESCRIPTION_BOTH) && pProp->sDescription[0] != 0) ? pProp->sDescription : NULL;
	pCell->iType = (iColumn == 0) ? XGE_XUI_TABLE_CELL_TYPE_TEXT : pProp->iType;
	pCell->bInvalid = (iColumn == 1) && ((pProp->iFlags & XGE_XUI_PROPERTY_FLAG_INVALID) != 0);
	pCell->bDirty = (iColumn == 1) && __xgeXuiPropertyGridPropertyDirty(pProp);
	pCell->bDisabled = (iColumn == 0) || ((pProp->iFlags & XGE_XUI_PROPERTY_FLAG_DISABLED) != 0) || ((iColumn == 1) && ((pProp->iFlags & XGE_XUI_PROPERTY_FLAG_READONLY) != 0));
	pCell->bHasStyle = 1;
	pCell->tStyle.iBackgroundColor = (iColumn == 0) ? pGrid->tStyle.iNameBackgroundColor : pGrid->tStyle.iValueBackgroundColor;
	pCell->tStyle.iBorderColor = pGrid->tStyle.iGridColor;
	if ( iColumn == 0 ) {
		pCell->procRenderer = __xgeXuiPropertyGridNameRenderer;
		pCell->pRendererUser = pGrid;
	} else if ( pProp->procRenderer != NULL || pGrid->procRenderer != NULL ) {
		pCell->procRenderer = __xgeXuiPropertyGridValueRenderer;
		pCell->pRendererUser = pGrid;
	}
	return XGE_OK;
}

static int __xgeXuiPropertyGridMergeProc(xge_xui_widget pWidget, int iRow, int iColumn, int* pRowSpan, int* pColSpan, void* pUser)
{
	xge_xui_property_grid pGrid;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	if ( (pGrid == NULL) || (iRow < 0) || (iRow >= pGrid->iVisibleCount) ) {
		return 0;
	}
	if ( pGrid->arrVisibleKind[iRow] == XGE_XUI_PROPERTY_GRID_VISIBLE_CATEGORY && iColumn == 0 ) {
		if ( pRowSpan != NULL ) {
			*pRowSpan = 1;
		}
		if ( pColSpan != NULL ) {
			*pColSpan = 2;
		}
		return 1;
	}
	return 0;
}

static void __xgeXuiPropertyGridSetProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xge_xui_property_grid pGrid;
	int iProperty;

	(void)pWidget;
	(void)iType;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( (iColumn != 1) || !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	(void)xgeXuiPropertyGridSetValue(pGrid, iProperty, sValue);
}

static int __xgeXuiPropertyGridValidateProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	int iProperty;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( (iColumn != 1) || !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return 0;
	}
	pProp = &pGrid->arrProperties[iProperty];
	if ( pGrid->procValidate == NULL ) {
		return 1;
	}
	return pGrid->procValidate(pGrid->pWidget, iProperty, pProp->sId, sValue, iType, pGrid->pValidateUser);
}

static void __xgeXuiPropertyGridChangeProc(xge_xui_widget pWidget, int iRow, int iColumn, const char* sValue, int iType, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	int iProperty;

	(void)pWidget;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( (iColumn != 1) || !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	pProp = &pGrid->arrProperties[iProperty];
	if ( pGrid->procChange != NULL ) {
		pGrid->procChange(pGrid->pWidget, iProperty, pProp->sId, sValue, iType, pGrid->pChangeUser);
	}
}

static int __xgeXuiPropertyGridEditorProc(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	xge_xui_property_grid_action_proc procAction;
	void* pActionUser;
	int iProperty;

	(void)pWidget;
	(void)pCell;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( (iColumn != 1) || !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return 0;
	}
	pProp = &pGrid->arrProperties[iProperty];
	procAction = (pProp->procAction != NULL) ? pProp->procAction : pGrid->procAction;
	pActionUser = (pProp->procAction != NULL) ? pProp->pActionUser : pGrid->pActionUser;
	if ( procAction == NULL ) {
		return 0;
	}
	return procAction(pGrid->pWidget, iProperty, pProp->sId, tRect, pActionUser);
}

static int __xgeXuiPropertyGridEditorConfigProc(xge_xui_widget pWidget, int iRow, int iColumn, int iType, xge_xui_table_grid_editor_config_t* pConfig, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	int iProperty;

	(void)pWidget;
	(void)iType;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( (iColumn != 1) || !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) || (pConfig == NULL) ) {
		return 0;
	}
	pProp = &pGrid->arrProperties[iProperty];
	if ( pProp->bHasEditorConfig == 0 ) {
		return 0;
	}
	*pConfig = pProp->tEditorConfig;
	return 1;
}

static void __xgeXuiPropertyGridSelectProc(xge_xui_widget pWidget, int iRow, int iColumn, int iSelectionMode, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	int iProperty;

	(void)pWidget;
	(void)iColumn;
	(void)iSelectionMode;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	pGrid->iSelectedProperty = iProperty;
	pGrid->iSelectCount++;
	pProp = &pGrid->arrProperties[iProperty];
	if ( pGrid->procSelect != NULL ) {
		pGrid->procSelect(pGrid->pWidget, iProperty, pProp->sId, pGrid->pSelectUser);
	}
}

static void __xgeXuiPropertyGridDrawCategoryArrow(xge_xui_property_grid pGrid, xge_rect_t tRect, int bExpanded)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	float fCX;
	float fCY;
	float fS;

	if ( pGrid == NULL ) {
		return;
	}
	fCX = tRect.fX + 12.0f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	fS = 4.0f;
	if ( bExpanded ) {
		tA = (xge_vec2_t){ fCX - fS, fCY - 2.0f };
		tB = (xge_vec2_t){ fCX + fS, fCY - 2.0f };
		tC = (xge_vec2_t){ fCX, fCY + 4.0f };
	} else {
		tA = (xge_vec2_t){ fCX - 2.0f, fCY - fS };
		tB = (xge_vec2_t){ fCX - 2.0f, fCY + fS };
		tC = (xge_vec2_t){ fCX + 4.0f, fCY };
	}
	xgeShapeTriangleFillPx(tA, tB, tC, pGrid->tStyle.iCategoryIconColor);
}

static int __xgeXuiPropertyGridCategoryRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_category_t* pCategory;
	xge_rect_t tText;
	uint32_t iBackground;
	int iCategory;

	(void)pWidget;
	(void)iColumn;
	(void)pCell;
	pGrid = (xge_xui_property_grid)pUser;
	iCategory = __xgeXuiPropertyGridVisibleRowCategory(pGrid, iRow);
	if ( !__xgeXuiPropertyGridValidCategory(pGrid, iCategory) ) {
		return 0;
	}
	pCategory = &pGrid->arrCategories[iCategory];
	iBackground = ((iState & XGE_XUI_TABLE_CELL_HOVER) != 0) ? pGrid->tStyle.iCategoryHoverColor : pGrid->tStyle.iCategoryBackgroundColor;
	__xgeXuiHostDrawRect(tRect, iBackground);
	__xgeXuiPropertyGridDrawCategoryArrow(pGrid, tRect, pCategory->bExpanded);
	tText = tRect;
	tText.fX += 24.0f;
	tText.fW -= 28.0f;
	__xgeXuiHostDrawTextRect(pGrid->pFont, pCategory->sName, tText, pGrid->tStyle.iCategoryTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return 1;
}

static int __xgeXuiPropertyGridNameRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	xge_rect_t tText;
	uint32_t iText;
	int iProperty;

	(void)pWidget;
	(void)iColumn;
	(void)pCell;
	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return 0;
	}
	pProp = &pGrid->arrProperties[iProperty];
	__xgeXuiHostDrawRect(tRect, ((iState & XGE_XUI_TABLE_CELL_HOVER) != 0) ? pGrid->tStyle.iNameHoverColor : pGrid->tStyle.iNameBackgroundColor);
	iText = ((pProp->iFlags & XGE_XUI_PROPERTY_FLAG_DISABLED) != 0) ? pGrid->tStyle.iReadonlyTextColor : pGrid->tStyle.iNameTextColor;
	tText = tRect;
	tText.fX += 8.0f;
	tText.fW -= 12.0f;
	__xgeXuiHostDrawTextRect(pGrid->pFont, pProp->sName, tText, iText, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	return 1;
}

static int __xgeXuiPropertyGridValueRenderer(xge_xui_widget pWidget, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, xge_rect_t tRect, int iState, void* pUser)
{
	xge_xui_property_grid pGrid;
	xge_xui_property_grid_property_t* pProp;
	xge_xui_property_grid_render_proc procRenderer;
	void* pRendererUser;
	int iProperty;

	pGrid = (xge_xui_property_grid)pUser;
	iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return 0;
	}
	pProp = &pGrid->arrProperties[iProperty];
	procRenderer = (pProp->procRenderer != NULL) ? pProp->procRenderer : pGrid->procRenderer;
	pRendererUser = (pProp->procRenderer != NULL) ? pProp->pRendererUser : pGrid->pRendererUser;
	if ( procRenderer == NULL ) {
		return 0;
	}
	return procRenderer(pWidget, iProperty, iColumn, pCell, tRect, iState, pRendererUser);
}

static void __xgeXuiPropertyGridUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	xge_xui_property_grid pGrid;

	pGrid = (xge_xui_property_grid)pUser;
	if ( pGrid == NULL ) {
		return;
	}
	__xgeXuiPropertyGridSyncColumns(pGrid);
	__xgeXuiTableGridUpdateProc(pWidget, fDelta, &pGrid->tGrid);
}

int xgeXuiPropertyGridInit(xge_xui_property_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget)
{
	const xge_xui_theme_t* pTheme;
	int iRet;

	if ( (pGrid == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pGrid, 0, sizeof(*pGrid));
	pTheme = xgeXuiGetTheme(pContext);
	pGrid->pContext = pContext;
	pGrid->pWidget = pWidget;
	pGrid->pFont = (pTheme != NULL) ? pTheme->pFont : NULL;
	pGrid->iSelectedProperty = -1;
	pGrid->fNameWidth = 150.0f;
	pGrid->fRowHeight = 24.0f;
	pGrid->fCategoryHeight = 26.0f;
	pGrid->iDescriptionMode = XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	pGrid->fDescriptionPanelHeight = 0.0f;
	__xgeXuiPropertyGridSetDefaultStyle(&pGrid->tStyle);
	iRet = xgeXuiTableGridInit(&pGrid->tGrid, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	__xgeXuiPropertyGridSyncColumns(pGrid);
	xgeXuiTableGridSetRows(&pGrid->tGrid, pGrid->arrRows, 0);
	xgeXuiTableGridSetAdapter(&pGrid->tGrid, __xgeXuiPropertyGridCountProc, __xgeXuiPropertyGridCellProc, __xgeXuiPropertyGridSetProc, pGrid);
	xgeXuiTableGridSetValidate(&pGrid->tGrid, __xgeXuiPropertyGridValidateProc, pGrid);
	xgeXuiTableGridSetChange(&pGrid->tGrid, __xgeXuiPropertyGridChangeProc, pGrid);
	xgeXuiTableGridSetEditor(&pGrid->tGrid, __xgeXuiPropertyGridEditorProc, pGrid);
	xgeXuiTableGridSetEditorConfig(&pGrid->tGrid, __xgeXuiPropertyGridEditorConfigProc, pGrid);
	xgeXuiTableGridSetDefaultMetrics(&pGrid->tGrid, 120.0f, pGrid->fRowHeight, 0.0f);
	xgeXuiTableGridSetEditMode(&pGrid->tGrid, XGE_XUI_TABLE_GRID_EDIT_QUICK);
	xgeXuiTableGridSetScrollbarMode(&pGrid->tGrid, XGE_XUI_SCROLLBAR_MODE_FULL);
	xgeXuiScrollFrameSetScrollbarPolicy(pGrid->tGrid.tTable.pFrame, XGE_XUI_SCROLLBAR_POLICY_HIDDEN, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	__xgeXuiPropertyGridSyncColumns(pGrid);
	xgeXuiTableGridSetSelectionMode(&pGrid->tGrid, XGE_XUI_TABLE_VIEW_SELECTION_CELL);
	xgeXuiTableViewSetSelect(&pGrid->tGrid.tTable, __xgeXuiPropertyGridSelectProc, pGrid);
	xgeXuiTableViewSetMergeProvider(&pGrid->tGrid.tTable, __xgeXuiPropertyGridMergeProc, pGrid);
	xgeXuiTableViewSetHeaderRenderer(&pGrid->tGrid.tTable, NULL, NULL);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiPropertyGridEventProc, pGrid);
	xgeXuiWidgetSetUpdate(pWidget, __xgeXuiPropertyGridUpdateProc, pGrid);
	__xgeXuiPropertyGridApplyStyle(pGrid);
	pWidget->pUser = pGrid;
	return XGE_OK;
}

void xgeXuiPropertyGridUnit(xge_xui_property_grid pGrid)
{
	xge_xui_widget pWidget;

	if ( pGrid == NULL ) {
		return;
	}
	pWidget = pGrid->pWidget;
	if ( pWidget != NULL && pWidget->procEvent == xgeXuiPropertyGridEventProc && pWidget->pEventUser == pGrid ) {
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
	}
	if ( pWidget != NULL && pWidget->procUpdate == __xgeXuiPropertyGridUpdateProc && pWidget->pUpdateUser == pGrid ) {
		xgeXuiWidgetSetUpdate(pWidget, NULL, NULL);
	}
	xgeXuiTableGridUnit(&pGrid->tGrid);
	memset(pGrid, 0, sizeof(*pGrid));
}

void xgeXuiPropertyGridClear(xge_xui_property_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	memset(pGrid->arrCategories, 0, sizeof(pGrid->arrCategories));
	memset(pGrid->arrProperties, 0, sizeof(pGrid->arrProperties));
	pGrid->iCategoryCount = 0;
	pGrid->iPropertyCount = 0;
	pGrid->iSelectedProperty = -1;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

int xgeXuiPropertyGridAddCategory(xge_xui_property_grid pGrid, const char* sId, const char* sName, int bExpanded)
{
	xge_xui_property_grid_category_t* pCategory;
	int iIndex;

	if ( pGrid == NULL || pGrid->iCategoryCount >= XGE_XUI_PROPERTY_GRID_CATEGORY_CAPACITY ) {
		return -1;
	}
	if ( sId != NULL && sId[0] != 0 && xgeXuiPropertyGridFindCategory(pGrid, sId) >= 0 ) {
		return -1;
	}
	iIndex = pGrid->iCategoryCount++;
	pCategory = &pGrid->arrCategories[iIndex];
	memset(pCategory, 0, sizeof(*pCategory));
	__xgeXuiPropertyGridCopy(pCategory->sId, sizeof(pCategory->sId), (sId != NULL && sId[0] != 0) ? sId : sName);
	__xgeXuiPropertyGridCopy(pCategory->sName, sizeof(pCategory->sName), (sName != NULL && sName[0] != 0) ? sName : pCategory->sId);
	pCategory->bExpanded = bExpanded != 0;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
	return iIndex;
}

int xgeXuiPropertyGridAddProperty(xge_xui_property_grid pGrid, int iCategory, const xge_xui_property_desc_t* pDesc)
{
	xge_xui_property_grid_property_t* pProp;
	int iIndex;

	if ( (pGrid == NULL) || (pDesc == NULL) || !__xgeXuiPropertyGridValidCategory(pGrid, iCategory) || pGrid->iPropertyCount >= XGE_XUI_PROPERTY_GRID_PROPERTY_CAPACITY ) {
		return -1;
	}
	if ( (pDesc->sId == NULL) || (pDesc->sId[0] == 0) || (xgeXuiPropertyGridFindProperty(pGrid, pDesc->sId) >= 0) ) {
		return -1;
	}
	iIndex = pGrid->iPropertyCount++;
	pProp = &pGrid->arrProperties[iIndex];
	memset(pProp, 0, sizeof(*pProp));
	__xgeXuiPropertyGridCopy(pProp->sId, sizeof(pProp->sId), pDesc->sId);
	__xgeXuiPropertyGridCopy(pProp->sName, sizeof(pProp->sName), (pDesc->sName != NULL && pDesc->sName[0] != 0) ? pDesc->sName : pDesc->sId);
	__xgeXuiPropertyGridCopy(pProp->sDescription, sizeof(pProp->sDescription), pDesc->sDescription);
	__xgeXuiPropertyGridCopy(pProp->sValue, sizeof(pProp->sValue), pDesc->sValue);
	__xgeXuiPropertyGridCopy(pProp->sDefaultValue, sizeof(pProp->sDefaultValue), pDesc->sDefaultValue);
	pProp->iCategory = iCategory;
	pProp->iType = pDesc->iType;
	if ( pProp->iType < XGE_XUI_TABLE_CELL_TYPE_TEXT || pProp->iType > XGE_XUI_TABLE_CELL_TYPE_IMAGE ) {
		pProp->iType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	}
	pProp->iFlags = pDesc->iFlags;
	pProp->bAutoDirty = (pProp->sDefaultValue[0] != 0);
	pGrid->arrCategories[iCategory].iPropertyCount++;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
	return iIndex;
}

int xgeXuiPropertyGridFindCategory(xge_xui_property_grid pGrid, const char* sId)
{
	int i;

	if ( (pGrid == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	for ( i = 0; i < pGrid->iCategoryCount; i++ ) {
		if ( strcmp(pGrid->arrCategories[i].sId, sId) == 0 ) {
			return i;
		}
	}
	return -1;
}

int xgeXuiPropertyGridFindProperty(xge_xui_property_grid pGrid, const char* sId)
{
	int i;

	if ( (pGrid == NULL) || (sId == NULL) || (sId[0] == 0) ) {
		return -1;
	}
	for ( i = 0; i < pGrid->iPropertyCount; i++ ) {
		if ( strcmp(pGrid->arrProperties[i].sId, sId) == 0 ) {
			return i;
		}
	}
	return -1;
}

void xgeXuiPropertyGridSetCategoryExpanded(xge_xui_property_grid pGrid, int iCategory, int bExpanded)
{
	if ( !__xgeXuiPropertyGridValidCategory(pGrid, iCategory) ) {
		return;
	}
	pGrid->arrCategories[iCategory].bExpanded = bExpanded != 0;
	pGrid->iToggleCount++;
	(void)xgeXuiTableGridEndEdit(&pGrid->tGrid, 1);
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

int xgeXuiPropertyGridGetCategoryExpanded(xge_xui_property_grid pGrid, int iCategory)
{
	return __xgeXuiPropertyGridValidCategory(pGrid, iCategory) ? pGrid->arrCategories[iCategory].bExpanded : 0;
}

void xgeXuiPropertyGridSetSelected(xge_xui_property_grid pGrid, int iProperty)
{
	int iRow;

	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		iProperty = -1;
	}
	pGrid->iSelectedProperty = iProperty;
	iRow = __xgeXuiPropertyGridVisiblePropertyRow(pGrid, iProperty);
	if ( iRow >= 0 ) {
		xgeXuiTableViewSetSelectedCell(&pGrid->tGrid.tTable, iRow, 1);
	} else {
		xgeXuiTableViewSetSelectedCell(&pGrid->tGrid.tTable, -1, -1);
	}
}

int xgeXuiPropertyGridGetSelected(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iSelectedProperty : -1;
}

int xgeXuiPropertyGridGetVisibleCount(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iVisibleCount : 0;
}

int xgeXuiPropertyGridGetVisibleProperty(xge_xui_property_grid pGrid, int iVisible)
{
	return __xgeXuiPropertyGridVisibleRowProperty(pGrid, iVisible);
}

int xgeXuiPropertyGridSetValue(xge_xui_property_grid pGrid, int iProperty, const char* sValue)
{
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiPropertyGridCopy(pGrid->arrProperties[iProperty].sValue, sizeof(pGrid->arrProperties[iProperty].sValue), sValue);
	xgeXuiTableViewRefresh(&pGrid->tGrid.tTable);
	return XGE_OK;
}

const char* xgeXuiPropertyGridGetValue(xge_xui_property_grid pGrid, int iProperty)
{
	return __xgeXuiPropertyGridValidProperty(pGrid, iProperty) ? pGrid->arrProperties[iProperty].sValue : NULL;
}

int xgeXuiPropertyGridGetBool(xge_xui_property_grid pGrid, int iProperty, int bDefault)
{
	const char* sValue;

	sValue = xgeXuiPropertyGridGetValue(pGrid, iProperty);
	return (sValue != NULL) ? __xgeXuiPropertyGridBoolText(sValue) : bDefault;
}

int xgeXuiPropertyGridSetBool(xge_xui_property_grid pGrid, int iProperty, int bValue)
{
	return xgeXuiPropertyGridSetValue(pGrid, iProperty, bValue ? "true" : "false");
}

int xgeXuiPropertyGridGetInt(xge_xui_property_grid pGrid, int iProperty, int iDefault)
{
	const char* sValue;

	sValue = xgeXuiPropertyGridGetValue(pGrid, iProperty);
	return (sValue != NULL && sValue[0] != 0) ? atoi(sValue) : iDefault;
}

int xgeXuiPropertyGridSetInt(xge_xui_property_grid pGrid, int iProperty, int iValue)
{
	char sValue[32];

	snprintf(sValue, sizeof(sValue), "%d", iValue);
	return xgeXuiPropertyGridSetValue(pGrid, iProperty, sValue);
}

float xgeXuiPropertyGridGetFloat(xge_xui_property_grid pGrid, int iProperty, float fDefault)
{
	const char* sValue;

	sValue = xgeXuiPropertyGridGetValue(pGrid, iProperty);
	return (sValue != NULL && sValue[0] != 0) ? (float)atof(sValue) : fDefault;
}

int xgeXuiPropertyGridSetFloat(xge_xui_property_grid pGrid, int iProperty, float fValue)
{
	char sValue[48];

	snprintf(sValue, sizeof(sValue), "%.6g", fValue);
	return xgeXuiPropertyGridSetValue(pGrid, iProperty, sValue);
}

uint32_t xgeXuiPropertyGridGetColor(xge_xui_property_grid pGrid, int iProperty, uint32_t iDefault)
{
	const char* sValue;
	uint32_t iColor;

	sValue = xgeXuiPropertyGridGetValue(pGrid, iProperty);
	if ( sValue != NULL && __xgeXuiTableViewParseColorText(sValue, &iColor) != 0 ) {
		return iColor;
	}
	return iDefault;
}

int xgeXuiPropertyGridSetColor(xge_xui_property_grid pGrid, int iProperty, uint32_t iColor)
{
	char sValue[16];

	if ( XGE_COLOR_GET_A(iColor) == 255 ) {
		snprintf(sValue, sizeof(sValue), "#%02X%02X%02X", XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor));
	} else {
		snprintf(sValue, sizeof(sValue), "#%02X%02X%02X%02X", XGE_COLOR_GET_R(iColor), XGE_COLOR_GET_G(iColor), XGE_COLOR_GET_B(iColor), XGE_COLOR_GET_A(iColor));
	}
	return xgeXuiPropertyGridSetValue(pGrid, iProperty, sValue);
}

void xgeXuiPropertyGridSetPropertyFlags(xge_xui_property_grid pGrid, int iProperty, int iFlags)
{
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	pGrid->arrProperties[iProperty].iFlags = iFlags;
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

int xgeXuiPropertyGridGetPropertyFlags(xge_xui_property_grid pGrid, int iProperty)
{
	return __xgeXuiPropertyGridValidProperty(pGrid, iProperty) ? pGrid->arrProperties[iProperty].iFlags : 0;
}

void xgeXuiPropertyGridSetEditorConfig(xge_xui_property_grid pGrid, int iProperty, const xge_xui_table_grid_editor_config_t* pConfig)
{
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	if ( pConfig == NULL ) {
		memset(&pGrid->arrProperties[iProperty].tEditorConfig, 0, sizeof(pGrid->arrProperties[iProperty].tEditorConfig));
		pGrid->arrProperties[iProperty].bHasEditorConfig = 0;
	} else {
		pGrid->arrProperties[iProperty].tEditorConfig = *pConfig;
		pGrid->arrProperties[iProperty].bHasEditorConfig = 1;
	}
}

void xgeXuiPropertyGridSetRenderer(xge_xui_property_grid pGrid, int iProperty, xge_xui_property_grid_render_proc procRender, void* pUser)
{
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	pGrid->arrProperties[iProperty].procRenderer = procRender;
	pGrid->arrProperties[iProperty].pRendererUser = pUser;
	xgeXuiTableViewRefresh(&pGrid->tGrid.tTable);
}

void xgeXuiPropertyGridSetAction(xge_xui_property_grid pGrid, int iProperty, xge_xui_property_grid_action_proc procAction, void* pUser)
{
	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return;
	}
	pGrid->arrProperties[iProperty].procAction = procAction;
	pGrid->arrProperties[iProperty].pActionUser = pUser;
}

int xgeXuiPropertyGridIsEditing(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) && xgeXuiTableGridIsEditing(&pGrid->tGrid);
}

int xgeXuiPropertyGridBeginEdit(xge_xui_property_grid pGrid, int iProperty)
{
	int iRow;

	if ( !__xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
		return 0;
	}
	iRow = __xgeXuiPropertyGridVisiblePropertyRow(pGrid, iProperty);
	return (iRow >= 0) ? xgeXuiTableGridBeginEdit(&pGrid->tGrid, iRow, 1) : 0;
}

int xgeXuiPropertyGridEndEdit(xge_xui_property_grid pGrid, int bCommit)
{
	return (pGrid != NULL) ? xgeXuiTableGridEndEdit(&pGrid->tGrid, bCommit) : 1;
}

void xgeXuiPropertyGridSetFont(xge_xui_property_grid pGrid, xge_font pFont)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->pFont = pFont;
	xgeXuiTableGridSetFont(&pGrid->tGrid, pFont);
}

void xgeXuiPropertyGridSetMetrics(xge_xui_property_grid pGrid, float fNameWidth, float fRowHeight, float fCategoryHeight)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( fNameWidth > 20.0f ) {
		pGrid->fNameWidth = fNameWidth;
	}
	if ( fRowHeight > 8.0f ) {
		pGrid->fRowHeight = fRowHeight;
	}
	if ( fCategoryHeight > 8.0f ) {
		pGrid->fCategoryHeight = fCategoryHeight;
	}
	xgeXuiTableGridSetDefaultMetrics(&pGrid->tGrid, 120.0f, pGrid->fRowHeight, 0.0f);
	__xgeXuiPropertyGridSyncColumns(pGrid);
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

void xgeXuiPropertyGridSetDescriptionMode(xge_xui_property_grid pGrid, int iMode, float fPanelHeight)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( iMode < XGE_XUI_PROPERTY_GRID_DESCRIPTION_NONE || iMode > XGE_XUI_PROPERTY_GRID_DESCRIPTION_BOTH ) {
		iMode = XGE_XUI_PROPERTY_GRID_DESCRIPTION_TOOLTIP;
	}
	pGrid->iDescriptionMode = iMode;
	pGrid->fDescriptionPanelHeight = (fPanelHeight > 0.0f) ? fPanelHeight : 0.0f;
	xgeXuiTableViewRefresh(&pGrid->tGrid.tTable);
}

void xgeXuiPropertyGridSetEditMode(xge_xui_property_grid pGrid, int iMode)
{
	if ( pGrid != NULL ) {
		xgeXuiTableGridSetEditMode(&pGrid->tGrid, iMode);
	}
}

void xgeXuiPropertyGridSetScroll(xge_xui_property_grid pGrid, float fScrollY)
{
	float fX;

	if ( pGrid == NULL ) {
		return;
	}
	xgeXuiTableViewGetOffset(&pGrid->tGrid.tTable, &fX, NULL);
	xgeXuiTableViewSetOffset(&pGrid->tGrid.tTable, fX, fScrollY);
}

float xgeXuiPropertyGridGetScroll(xge_xui_property_grid pGrid)
{
	float fY;

	fY = 0.0f;
	if ( pGrid != NULL ) {
		xgeXuiTableViewGetOffset(&pGrid->tGrid.tTable, NULL, &fY);
	}
	return fY;
}

void xgeXuiPropertyGridSetScrollbarMode(xge_xui_property_grid pGrid, int iMode)
{
	if ( pGrid != NULL ) {
		xgeXuiTableGridSetScrollbarMode(&pGrid->tGrid, iMode);
	}
}

int xgeXuiPropertyGridGetScrollbarMode(xge_xui_property_grid pGrid)
{
	return (pGrid != NULL) ? xgeXuiTableViewGetScrollbarMode(&pGrid->tGrid.tTable) : XGE_XUI_SCROLLBAR_MODE_FULL;
}

void xgeXuiPropertyGridSetSelect(xge_xui_property_grid pGrid, xge_xui_property_grid_select_proc procSelect, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procSelect = procSelect;
		pGrid->pSelectUser = pUser;
	}
}

void xgeXuiPropertyGridSetValidate(xge_xui_property_grid pGrid, xge_xui_property_grid_validate_proc procValidate, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procValidate = procValidate;
		pGrid->pValidateUser = pUser;
	}
}

void xgeXuiPropertyGridSetChange(xge_xui_property_grid pGrid, xge_xui_property_grid_change_proc procChange, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procChange = procChange;
		pGrid->pChangeUser = pUser;
	}
}

void xgeXuiPropertyGridSetGlobalAction(xge_xui_property_grid pGrid, xge_xui_property_grid_action_proc procAction, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procAction = procAction;
		pGrid->pActionUser = pUser;
	}
}

void xgeXuiPropertyGridSetGlobalRenderer(xge_xui_property_grid pGrid, xge_xui_property_grid_render_proc procRender, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procRenderer = procRender;
		pGrid->pRendererUser = pUser;
		xgeXuiTableViewRefresh(&pGrid->tGrid.tTable);
	}
}

void xgeXuiPropertyGridSetStyle(xge_xui_property_grid pGrid, const xge_xui_property_grid_style_t* pStyle)
{
	if ( (pGrid == NULL) || (pStyle == NULL) ) {
		return;
	}
	pGrid->tStyle = *pStyle;
	__xgeXuiPropertyGridApplyStyle(pGrid);
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

void xgeXuiPropertyGridSetColors(xge_xui_property_grid pGrid, uint32_t iBackground, uint32_t iCategory, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->tStyle.iBackgroundColor = iBackground;
	pGrid->tStyle.iCategoryBackgroundColor = iCategory;
	pGrid->tStyle.iNameBackgroundColor = iRow;
	pGrid->tStyle.iValueBackgroundColor = iRow;
	pGrid->tStyle.iSelectedColor = iSelected;
	pGrid->tStyle.iGridColor = iGrid;
	pGrid->tStyle.iValueTextColor = iText;
	__xgeXuiPropertyGridApplyStyle(pGrid);
	__xgeXuiPropertyGridRebuildVisible(pGrid);
}

int xgeXuiPropertyGridEvent(xge_xui_property_grid pGrid, const xge_event_t* pEvent)
{
	int iRow;
	int iColumn;
	int iCategory;

	if ( (pGrid == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		if ( __xgeXuiTableViewHitCell(&pGrid->tGrid.tTable, pEvent->fX, pEvent->fY, &iRow, &iColumn) != 0 ) {
			iCategory = __xgeXuiPropertyGridVisibleRowCategory(pGrid, iRow);
			if ( __xgeXuiPropertyGridValidCategory(pGrid, iCategory) ) {
				xgeXuiPropertyGridSetCategoryExpanded(pGrid, iCategory, !pGrid->arrCategories[iCategory].bExpanded);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( iColumn == 0 ) {
				int iProperty;

				iProperty = __xgeXuiPropertyGridVisibleRowProperty(pGrid, iRow);
				if ( __xgeXuiPropertyGridValidProperty(pGrid, iProperty) ) {
					xgeXuiPropertyGridSetSelected(pGrid, iProperty);
					__xgeXuiPropertyGridSelectProc(pGrid->pWidget, iRow, 1, XGE_XUI_TABLE_VIEW_SELECTION_CELL, pGrid);
					return XGE_XUI_EVENT_CONSUMED;
				}
			}
		}
	}
	return xgeXuiTableGridEvent(&pGrid->tGrid, pEvent);
}

int xgeXuiPropertyGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( (pUser == NULL) && (pWidget != NULL) ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiPropertyGridEvent((xge_xui_property_grid)pUser, pEvent);
}

void xgeXuiPropertyGridPaintProc(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
}
