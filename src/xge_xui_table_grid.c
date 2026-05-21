static int __xgeXuiTableGridCountProc(xge_xui_widget pWidget, void* pUser);
static int __xgeXuiTableGridCellProc(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser);
static int __xgeXuiTableGridInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser);
static int __xgeXuiTableGridInputFilterProc(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser);
static void __xgeXuiTableGridUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser);
static int __xgeXuiTableGridClipEditRect(xge_xui_table_grid pGrid, xge_rect_t* pRect);
static xge_xui_widget __xgeXuiTableGridActiveWidget(xge_xui_table_grid pGrid);
static void __xgeXuiTableGridComboSelectProc(xge_xui_widget pWidget, int iIndex, void* pUser);
static void __xgeXuiTableGridColorChangeProc(xge_xui_widget pWidget, uint32_t iColor, void* pUser);
static void __xgeXuiTableGridDateCommitProc(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser);
static void __xgeXuiTableGridTextAreaOkProc(xge_xui_widget pWidget, void* pUser);
static void __xgeXuiTableGridTextAreaCancelProc(xge_xui_widget pWidget, void* pUser);

#define XGE_XUI_TABLE_GRID_EDITOR_NONE 0
#define XGE_XUI_TABLE_GRID_EDITOR_INPUT 1
#define XGE_XUI_TABLE_GRID_EDITOR_COMBO 2
#define XGE_XUI_TABLE_GRID_EDITOR_COLOR 3
#define XGE_XUI_TABLE_GRID_EDITOR_DATE 4
#define XGE_XUI_TABLE_GRID_EDITOR_NUMERIC 5

static int __xgeXuiTableGridTextEqualNoCase(const char* sA, const char* sB)
{
	unsigned char a;
	unsigned char b;

	if ( sA == NULL || sB == NULL ) {
		return sA == sB;
	}
	while ( (*sA != 0) && (*sB != 0) ) {
		a = (unsigned char)*sA++;
		b = (unsigned char)*sB++;
		if ( a >= 'A' && a <= 'Z' ) {
			a = (unsigned char)(a - 'A' + 'a');
		}
		if ( b >= 'A' && b <= 'Z' ) {
			b = (unsigned char)(b - 'A' + 'a');
		}
		if ( a != b ) {
			return 0;
		}
	}
	return *sA == *sB;
}

static int __xgeXuiTableGridIsNumericType(int iType)
{
	return (iType == XGE_XUI_TABLE_CELL_TYPE_INT) || (iType == XGE_XUI_TABLE_CELL_TYPE_FLOAT);
}

static int __xgeXuiTableGridIsPickerType(int iType)
{
	return (iType == XGE_XUI_TABLE_CELL_TYPE_PICKER) || (iType == XGE_XUI_TABLE_CELL_TYPE_FILE) || (iType == XGE_XUI_TABLE_CELL_TYPE_IMAGE);
}

static int __xgeXuiTableGridIsDateType(int iType)
{
	return (iType == XGE_XUI_TABLE_CELL_TYPE_DATE) || (iType == XGE_XUI_TABLE_CELL_TYPE_TIME) || (iType == XGE_XUI_TABLE_CELL_TYPE_DATETIME);
}

static int __xgeXuiTableGridIsExternalEditorType(int iType)
{
	return (iType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_DATE) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_TIME) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_DATETIME) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_ENUM) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_COLOR) ||
	       __xgeXuiTableGridIsPickerType(iType) ||
	       (iType == XGE_XUI_TABLE_CELL_TYPE_CUSTOM);
}

static int __xgeXuiTableGridDateModeForType(int iType)
{
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_TIME ) {
		return XGE_XUI_DATE_PICKER_MODE_TIME;
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_DATETIME ) {
		return XGE_XUI_DATE_PICKER_MODE_DATETIME;
	}
	return XGE_XUI_DATE_PICKER_MODE_DATE;
}

static const char* __xgeXuiTableGridDefaultDateFormat(int iMode, int bShowSecond)
{
	switch ( iMode ) {
		case XGE_XUI_DATE_PICKER_MODE_TIME:
		case XGE_XUI_DATE_PICKER_MODE_TIME_RANGE:
			return bShowSecond ? "hh:nn:ss" : "hh:nn";
		case XGE_XUI_DATE_PICKER_MODE_DATETIME:
		case XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE:
			return bShowSecond ? "yyyy-mm-dd hh:nn:ss" : "yyyy-mm-dd hh:nn";
		case XGE_XUI_DATE_PICKER_MODE_DATE:
		case XGE_XUI_DATE_PICKER_MODE_DATE_RANGE:
		default:
			return "yyyy-mm-dd";
	}
}

static int __xgeXuiTableGridBoolValue(const char* sText, const void* pValue)
{
	if ( pValue != NULL ) {
		return ((intptr_t)pValue) != 0;
	}
	if ( sText == NULL ) {
		return 0;
	}
	return (strcmp(sText, "1") == 0) || __xgeXuiTableGridTextEqualNoCase(sText, "true") || __xgeXuiTableGridTextEqualNoCase(sText, "yes") || __xgeXuiTableGridTextEqualNoCase(sText, "on");
}

static int __xgeXuiTableGridValidateNumberText(const char* sText, int iType)
{
	int i;
	int iDot;
	int iDigit;

	if ( sText == NULL || sText[0] == 0 ) {
		return 1;
	}
	i = 0;
	if ( sText[i] == '-' || sText[i] == '+' ) {
		i++;
	}
	iDot = 0;
	iDigit = 0;
	for ( ; sText[i] != 0; i++ ) {
		if ( (sText[i] >= '0') && (sText[i] <= '9') ) {
			iDigit = 1;
			continue;
		}
		if ( (iType == XGE_XUI_TABLE_CELL_TYPE_FLOAT) && (sText[i] == '.') && (iDot == 0) ) {
			iDot = 1;
			continue;
		}
		return 0;
	}
	(void)iDigit;
	return 1;
}

static int __xgeXuiTableGridInferPrecision(const char* sText, int iType)
{
	const char* pDot;
	int iPrecision;

	if ( iType == XGE_XUI_TABLE_CELL_TYPE_INT ) {
		return 0;
	}
	if ( sText == NULL ) {
		return 3;
	}
	pDot = strchr(sText, '.');
	if ( pDot == NULL ) {
		return 3;
	}
	iPrecision = 0;
	pDot++;
	while ( (*pDot >= '0') && (*pDot <= '9') && (iPrecision < 6) ) {
		iPrecision++;
		pDot++;
	}
	return iPrecision;
}

static float __xgeXuiTableGridStepForPrecision(int iPrecision)
{
	float fStep;
	int i;

	if ( iPrecision <= 0 ) {
		return 1.0f;
	}
	fStep = 1.0f;
	for ( i = 0; i < iPrecision; i++ ) {
		fStep *= 0.1f;
	}
	return fStep;
}

static void __xgeXuiTableGridDefaultCell(xge_xui_table_grid pGrid, int iColumn, xge_xui_table_view_cell_t* pCell)
{
	if ( pCell == NULL ) {
		return;
	}
	memset(pCell, 0, sizeof(*pCell));
	pCell->iRowSpan = 1;
	pCell->iColSpan = 1;
	pCell->iType = ((pGrid != NULL) && (iColumn >= 0) && (iColumn < pGrid->tTable.iColumnCount)) ? pGrid->tTable.arrColumns[iColumn].iType : XGE_XUI_TABLE_CELL_TYPE_TEXT;
}

static void __xgeXuiTableGridGetCell(xge_xui_table_grid pGrid, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell)
{
	__xgeXuiTableGridDefaultCell(pGrid, iColumn, pCell);
	if ( (pGrid == NULL) || (pCell == NULL) || (iRow < 0) || (iColumn < 0) ) {
		return;
	}
	if ( pGrid->procCell != NULL ) {
		(void)pGrid->procCell(pGrid->pWidget, iRow, iColumn, pCell, pGrid->pAdapterUser);
	}
	if ( pCell->iRowSpan <= 0 ) {
		pCell->iRowSpan = 1;
	}
	if ( pCell->iColSpan <= 0 ) {
		pCell->iColSpan = 1;
	}
}

static const char* __xgeXuiTableGridCellText(xge_xui_table_grid pGrid, int iRow, int iColumn, const xge_xui_table_view_cell_t* pCell, char* sBuffer, int iSize)
{
	xge_xui_table_view_format_proc procFormatter;
	void* pUser;
	int iValue;

	if ( (pCell == NULL) || (sBuffer == NULL) || (iSize <= 0) ) {
		return "";
	}
	sBuffer[0] = 0;
	if ( pCell->sText != NULL ) {
		return pCell->sText;
	}
	procFormatter = pCell->procFormatter;
	pUser = pCell->pFormatterUser;
	if ( (procFormatter == NULL) && (pGrid != NULL) && (iColumn >= 0) && (iColumn < pGrid->tTable.iColumnCount) ) {
		procFormatter = pGrid->tTable.arrColumns[iColumn].procFormatter;
		pUser = pGrid->tTable.arrColumns[iColumn].pFormatterUser;
	}
	if ( procFormatter != NULL ) {
		if ( procFormatter((pGrid != NULL) ? pGrid->pWidget : NULL, iRow, iColumn, pCell, sBuffer, iSize, pUser) != 0 ) {
			sBuffer[iSize - 1] = 0;
			return sBuffer;
		}
		sBuffer[0] = 0;
	}
	if ( pCell->pValue != NULL ) {
		iValue = (int)(intptr_t)pCell->pValue;
		if ( pCell->iType == XGE_XUI_TABLE_CELL_TYPE_BOOL ) {
			snprintf(sBuffer, (size_t)iSize, "%s", iValue ? "true" : "false");
		} else {
			snprintf(sBuffer, (size_t)iSize, "%d", iValue);
		}
		sBuffer[iSize - 1] = 0;
		return sBuffer;
	}
	return "";
}

static void __xgeXuiTableGridDefaultEditorConfig(xge_xui_table_grid_editor_config_t* pConfig, int iType)
{
	if ( pConfig == NULL ) {
		return;
	}
	memset(pConfig, 0, sizeof(*pConfig));
	pConfig->iEnumSelected = -1;
	pConfig->iDateMode = __xgeXuiTableGridDateModeForType(iType);
	pConfig->bShowSecond = 1;
	pConfig->sRangeSeparator = " - ";
}

static int __xgeXuiTableGridLoadEditorConfig(xge_xui_table_grid pGrid, int iRow, int iColumn, int iType)
{
	if ( pGrid == NULL ) {
		return 0;
	}
	__xgeXuiTableGridDefaultEditorConfig(&pGrid->tEditorConfig, iType);
	if ( pGrid->procEditorConfig == NULL ) {
		return 0;
	}
	return pGrid->procEditorConfig(pGrid->pWidget, iRow, iColumn, iType, &pGrid->tEditorConfig, pGrid->pEditorConfigUser);
}

static void __xgeXuiTableGridFormatDateValue(xge_xui_table_grid pGrid, char* sBuffer, int iSize)
{
	char* sText;
	xtime tValue;
	const char* sFormat;
	int iMode;
	int bShowSecond;
	int64 iYear;
	int iMonth;
	int iDay;
	int iHour;
	int iMinute;
	int iSecond;

	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return;
	}
	sBuffer[0] = 0;
	if ( pGrid == NULL ) {
		return;
	}
	tValue = xgeXuiDatePickerGetValue(&pGrid->tDatePicker);
	iMode = xgeXuiDatePickerGetMode(&pGrid->tDatePicker);
	bShowSecond = pGrid->tDatePicker.bShowSecond;
	sFormat = xgeXuiDatePickerGetFormat(&pGrid->tDatePicker);
	if ( sFormat == NULL || sFormat[0] == 0 ) {
		sFormat = __xgeXuiTableGridDefaultDateFormat(iMode, bShowSecond);
	}
	sText = (char*)xrtTimeFormat(tValue, (str)sFormat);
	if ( sText != NULL ) {
		snprintf(sBuffer, (size_t)iSize, "%s", sText);
		sBuffer[iSize - 1] = 0;
		xrtFree(sText);
		return;
	}
	if ( iMode == XGE_XUI_DATE_PICKER_MODE_TIME || iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		xrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
		if ( bShowSecond ) {
			snprintf(sBuffer, (size_t)iSize, "%02d:%02d:%02d", iHour, iMinute, iSecond);
		} else {
			snprintf(sBuffer, (size_t)iSize, "%02d:%02d", iHour, iMinute);
		}
	} else {
		xrtDecodeSerial(tValue, &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, NULL, NULL);
		if ( iMode == XGE_XUI_DATE_PICKER_MODE_DATE || iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE ) {
			snprintf(sBuffer, (size_t)iSize, "%04lld-%02d-%02d", iYear, iMonth, iDay);
		} else if ( bShowSecond ) {
			snprintf(sBuffer, (size_t)iSize, "%04lld-%02d-%02d %02d:%02d:%02d", iYear, iMonth, iDay, iHour, iMinute, iSecond);
		} else {
			snprintf(sBuffer, (size_t)iSize, "%04lld-%02d-%02d %02d:%02d", iYear, iMonth, iDay, iHour, iMinute);
		}
	}
	sBuffer[iSize - 1] = 0;
}

static void __xgeXuiTableGridPlaceEditorWidget(xge_xui_table_grid pGrid, xge_xui_widget pEditor, xge_rect_t tRect)
{
	xge_rect_t tLocal;

	if ( (pGrid == NULL) || (pGrid->pWidget == NULL) || (pEditor == NULL) ) {
		return;
	}
	tLocal = tRect;
	tLocal.fX -= pGrid->pWidget->tContentRect.fX;
	tLocal.fY -= pGrid->pWidget->tContentRect.fY;
	pEditor->tLocalRect = tLocal;
	__xgeXuiWidgetArrangeRect(pEditor, tRect);
}

static void __xgeXuiTableGridPlaceEditWidget(xge_xui_table_grid pGrid, xge_rect_t tRect)
{
	__xgeXuiTableGridPlaceEditorWidget(pGrid, (pGrid != NULL) ? pGrid->pEditWidget : NULL, tRect);
}

static void __xgeXuiTableGridPlaceNumericWidget(xge_xui_table_grid pGrid, xge_rect_t tRect)
{
	__xgeXuiTableGridPlaceEditorWidget(pGrid, (pGrid != NULL) ? pGrid->pNumericWidget : NULL, tRect);
}

static void __xgeXuiTableGridAdjustInlineEditorRect(xge_rect_t* pRect)
{
	if ( pRect == NULL ) {
		return;
	}
	pRect->fX -= 1.0f;
	pRect->fY -= 1.0f;
	pRect->fW += 1.0f;
	pRect->fH += 1.0f;
}

static int __xgeXuiTableGridEditingCellRect(xge_xui_table_grid pGrid, xge_rect_t* pRect)
{
	xge_rect_t tRect;

	if ( (pGrid == NULL) || (pRect == NULL) || (pGrid->iEditingRow < 0) || (pGrid->iEditingColumn < 0) ) {
		return 0;
	}
	if ( xgeXuiTableViewGetCellRect(&pGrid->tTable, pGrid->iEditingRow, pGrid->iEditingColumn, &tRect) == 0 ) {
		return 0;
	}
	if ( __xgeXuiTableGridClipEditRect(pGrid, &tRect) == 0 ) {
		return 0;
	}
	*pRect = tRect;
	return 1;
}

static int __xgeXuiTableGridSyncActiveEditorRect(xge_xui_table_grid pGrid)
{
	xge_xui_widget pEditor;
	xge_rect_t tRect;

	if ( (pGrid == NULL) || (pGrid->iEditingRow < 0) || (pGrid->iEditingColumn < 0) ) {
		return 0;
	}
	pEditor = __xgeXuiTableGridActiveWidget(pGrid);
	if ( pEditor == NULL ) {
		if ( (pGrid->iEditingType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA) && xgeXuiPopupIsOpen(&pGrid->tTextAreaPopup) && __xgeXuiTableGridEditingCellRect(pGrid, &tRect) != 0 ) {
			xgeXuiPopupSetAnchorRect(&pGrid->tTextAreaPopup, tRect);
		}
		return 0;
	}
	if ( xgeXuiTableViewGetCellRect(&pGrid->tTable, pGrid->iEditingRow, pGrid->iEditingColumn, &tRect) == 0 ) {
		xgeXuiWidgetSetVisible(pEditor, 0);
		return 0;
	}
	__xgeXuiTableGridAdjustInlineEditorRect(&tRect);
	if ( __xgeXuiTableGridClipEditRect(pGrid, &tRect) == 0 ) {
		xgeXuiWidgetSetVisible(pEditor, 0);
		return 0;
	}
	__xgeXuiTableGridPlaceEditorWidget(pGrid, pEditor, tRect);
	xgeXuiWidgetSetVisible(pEditor, 1);
	return 1;
}

static void __xgeXuiTableGridArrangeLocal(xge_xui_widget pParent, xge_xui_widget pChild, xge_rect_t tLocal)
{
	xge_rect_t tRect;

	if ( (pParent == NULL) || (pChild == NULL) ) {
		return;
	}
	pChild->tLocalRect = tLocal;
	tRect = tLocal;
	tRect.fX += pParent->tContentRect.fX;
	tRect.fY += pParent->tContentRect.fY;
	__xgeXuiWidgetArrangeRect(pChild, tRect);
}

static int __xgeXuiTableGridClipEditRect(xge_xui_table_grid pGrid, xge_rect_t* pRect)
{
	xge_xui_widget pViewport;
	xge_rect_t tView;
	float fRight;
	float fBottom;
	float fViewRight;
	float fViewBottom;

	if ( (pGrid == NULL) || (pRect == NULL) ) {
		return 0;
	}
	pViewport = xgeXuiTableViewGetViewportWidget(&pGrid->tTable);
	if ( pViewport == NULL ) {
		return 0;
	}
	tView = pViewport->tContentRect;
	fRight = pRect->fX + pRect->fW;
	fBottom = pRect->fY + pRect->fH;
	fViewRight = tView.fX + tView.fW;
	fViewBottom = tView.fY + tView.fH;
	if ( pRect->fX < tView.fX ) {
		pRect->fW -= tView.fX - pRect->fX;
		pRect->fX = tView.fX;
	}
	if ( pRect->fY < tView.fY ) {
		pRect->fH -= tView.fY - pRect->fY;
		pRect->fY = tView.fY;
	}
	if ( fRight > fViewRight ) {
		pRect->fW -= fRight - fViewRight;
	}
	if ( fBottom > fViewBottom ) {
		pRect->fH -= fBottom - fViewBottom;
	}
	if ( pRect->fW < 0.0f ) {
		pRect->fW = 0.0f;
	}
	if ( pRect->fH < 0.0f ) {
		pRect->fH = 0.0f;
	}
	return (pRect->fW > 4.0f) && (pRect->fH > 4.0f);
}

static int __xgeXuiTableGridCommitValue(xge_xui_table_grid pGrid, const char* sValue)
{
	if ( pGrid == NULL ) {
		return 0;
	}
	if ( sValue == NULL ) {
		sValue = "";
	}
	if ( pGrid->procValidate != NULL ) {
		if ( pGrid->procValidate(pGrid->pWidget, pGrid->iEditingRow, pGrid->iEditingColumn, sValue, pGrid->iEditingType, pGrid->pValidateUser) == 0 ) {
			pGrid->iRejectCount++;
			if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_NUMERIC ) {
				xgeXuiInputSetError(&pGrid->tEditNumeric.tInput, 1);
			} else {
				xgeXuiInputSetError(&pGrid->tEditInput, 1);
			}
			return 0;
		}
	}
	if ( strcmp(pGrid->sOriginalValue, sValue) != 0 ) {
		if ( pGrid->procSet != NULL ) {
			pGrid->procSet(pGrid->pWidget, pGrid->iEditingRow, pGrid->iEditingColumn, sValue, pGrid->iEditingType, pGrid->pSetUser);
		}
		if ( pGrid->procChange != NULL ) {
			pGrid->procChange(pGrid->pWidget, pGrid->iEditingRow, pGrid->iEditingColumn, sValue, pGrid->iEditingType, pGrid->pChangeUser);
		}
		pGrid->iCommitCount++;
	}
	return 1;
}

static void __xgeXuiTableGridHideEditors(xge_xui_table_grid pGrid)
{
	if ( pGrid == NULL ) {
		return;
	}
	if ( pGrid->pEditWidget != NULL ) {
		xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
	}
	if ( pGrid->pNumericWidget != NULL ) {
		xgeXuiWidgetSetVisible(pGrid->pNumericWidget, 0);
	}
	if ( pGrid->pComboWidget != NULL ) {
		xgeXuiWidgetSetVisible(pGrid->pComboWidget, 0);
		if ( pGrid->pCombo != NULL ) {
			xgeXuiPopupSetOpen(&pGrid->pCombo->tPopup, 0);
		}
	}
	if ( pGrid->pColorWidget != NULL ) {
		xgeXuiWidgetSetVisible(pGrid->pColorWidget, 0);
	}
	if ( pGrid->pDateWidget != NULL ) {
		xgeXuiWidgetSetVisible(pGrid->pDateWidget, 0);
	}
	xgeXuiPopupSetOpen(&pGrid->tTextAreaPopup, 0);
}

static int __xgeXuiTableGridEditorPopupOpen(xge_xui_table_grid pGrid)
{
	if ( pGrid == NULL ) {
		return 0;
	}
	if ( (pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_COMBO) && xgeXuiComboBoxIsOpen(pGrid->pCombo) ) {
		return 1;
	}
	if ( (pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_COLOR) && xgeXuiColorPickerIsPopupOpen(&pGrid->tColorPicker) ) {
		return 1;
	}
	if ( (pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_DATE) && xgeXuiDatePickerIsPopupOpen(&pGrid->tDatePicker) ) {
		return 1;
	}
	if ( (pGrid->iEditingType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA) && xgeXuiPopupIsOpen(&pGrid->tTextAreaPopup) ) {
		return 1;
	}
	return 0;
}

static xge_xui_widget __xgeXuiTableGridActiveWidget(xge_xui_table_grid pGrid)
{
	if ( pGrid == NULL ) {
		return NULL;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_COMBO ) {
		return pGrid->pComboWidget;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_COLOR ) {
		return pGrid->pColorWidget;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_DATE ) {
		return pGrid->pDateWidget;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_INPUT ) {
		return pGrid->pEditWidget;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_NUMERIC ) {
		return pGrid->pNumericWidget;
	}
	return NULL;
}

static const char* __xgeXuiTableGridCurrentEditorValue(xge_xui_table_grid pGrid, char* sBuffer, int iSize)
{
	const char* sText;
	int iIndex;

	if ( (sBuffer == NULL) || (iSize <= 0) ) {
		return "";
	}
	sBuffer[0] = 0;
	if ( pGrid == NULL ) {
		return sBuffer;
	}
	if ( pGrid->iEditingType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA ) {
		sText = xgeXuiTextEditGetText(&pGrid->tTextAreaEdit);
		return (sText != NULL) ? sText : "";
	}
	switch ( pGrid->iActiveEditor ) {
		case XGE_XUI_TABLE_GRID_EDITOR_COMBO:
			iIndex = xgeXuiComboBoxGetSelected(pGrid->pCombo);
			if ( pGrid->pCombo != NULL && pGrid->pCombo->arrItemData != NULL && iIndex >= 0 && iIndex < pGrid->pCombo->iItemCount ) {
				if ( pGrid->tEditorConfig.bEnumUseValue ) {
					snprintf(sBuffer, (size_t)iSize, "%d", xgeXuiComboBoxGetSelectedValue(pGrid->pCombo));
				} else {
					snprintf(sBuffer, (size_t)iSize, "%s", pGrid->pCombo->arrItemData[iIndex].sText != NULL ? pGrid->pCombo->arrItemData[iIndex].sText : "");
				}
				sBuffer[iSize - 1] = 0;
				return sBuffer;
			}
			if ( pGrid->pCombo != NULL && pGrid->pCombo->arrItems != NULL && iIndex >= 0 && iIndex < pGrid->pCombo->iItemCount ) {
				snprintf(sBuffer, (size_t)iSize, "%s", pGrid->pCombo->arrItems[iIndex] != NULL ? pGrid->pCombo->arrItems[iIndex] : "");
			}
			sBuffer[iSize - 1] = 0;
			return sBuffer;
		case XGE_XUI_TABLE_GRID_EDITOR_COLOR:
			snprintf(sBuffer, (size_t)iSize, "%s", xgeXuiColorPickerGetHex(&pGrid->tColorPicker));
			sBuffer[iSize - 1] = 0;
			return sBuffer;
		case XGE_XUI_TABLE_GRID_EDITOR_DATE:
			__xgeXuiTableGridFormatDateValue(pGrid, sBuffer, iSize);
			return sBuffer;
		case XGE_XUI_TABLE_GRID_EDITOR_NUMERIC:
			sText = xgeXuiInputGetText(&pGrid->tEditNumeric.tInput);
			return (sText != NULL) ? sText : "";
		case XGE_XUI_TABLE_GRID_EDITOR_INPUT:
		default:
			sText = xgeXuiInputGetText(&pGrid->tEditInput);
			return (sText != NULL) ? sText : "";
	}
}

static int __xgeXuiTableGridBeginNumericEdit(xge_xui_table_grid pGrid, const char* sText, xge_rect_t tRect, int iType)
{
	float fValue;
	int iPrecision;

	if ( (pGrid == NULL) || (pGrid->pNumericWidget == NULL) ) {
		return 0;
	}
	iPrecision = __xgeXuiTableGridInferPrecision(sText, iType);
	xgeXuiNumericInputSetRange(&pGrid->tEditNumeric, -1000000000.0f, 1000000000.0f);
	xgeXuiNumericInputSetInteger(&pGrid->tEditNumeric, iType == XGE_XUI_TABLE_CELL_TYPE_INT);
	xgeXuiNumericInputSetPrecision(&pGrid->tEditNumeric, iPrecision);
	xgeXuiNumericInputSetStep(&pGrid->tEditNumeric, __xgeXuiTableGridStepForPrecision(iPrecision));
	xgeXuiNumericInputSetSpinnerVisible(&pGrid->tEditNumeric, 1);
	xgeXuiInputSetError(&pGrid->tEditNumeric.tInput, 0);
	if ( sText != NULL && sText[0] != 0 && __xgeXuiTableGridValidateNumberText(sText, iType) != 0 ) {
		fValue = (float)strtod(sText, NULL);
		xgeXuiNumericInputSetValue(&pGrid->tEditNumeric, fValue);
	}
	xgeXuiInputSetText(&pGrid->tEditNumeric.tInput, sText != NULL ? sText : "");
	xgeXuiInputSetSelection(&pGrid->tEditNumeric.tInput, 0, pGrid->tEditNumeric.tInput.tText.iSize);
	__xgeXuiTableGridPlaceNumericWidget(pGrid, tRect);
	xgeXuiWidgetSetVisible(pGrid->pNumericWidget, 1);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pNumericWidget);
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NUMERIC;
	return 1;
}

static int __xgeXuiTableGridOpenTextArea(xge_xui_table_grid pGrid)
{
	xge_rect_t tAnchor;
	xge_rect_t tContent;
	float fW;
	float fH;

	if ( (pGrid == NULL) || (pGrid->pTextAreaPopupWidget == NULL) ) {
		return 0;
	}
	fW = 380.0f;
	fH = 238.0f;
	if ( __xgeXuiTableGridEditingCellRect(pGrid, &tAnchor) == 0 ) {
		tAnchor = (pGrid->pWidget != NULL) ? pGrid->pWidget->tRect : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	if ( xgeXuiPopupIsOpen(&pGrid->tTextAreaPopup) == 0 ) {
		xgeXuiTextEditSetText(&pGrid->tTextAreaEdit, pGrid->sOriginalValue);
	}
	xgeXuiPopupSetOwner(&pGrid->tTextAreaPopup, pGrid->pWidget);
	xgeXuiPopupSetFocusRestore(&pGrid->tTextAreaPopup, pGrid->pWidget);
	xgeXuiPopupSetAnchorRect(&pGrid->tTextAreaPopup, tAnchor);
	xgeXuiPopupSetAnchorPoint(&pGrid->tTextAreaPopup, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT);
	xgeXuiPopupSetDirection(&pGrid->tTextAreaPopup, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN);
	xgeXuiPopupSetContentSize(&pGrid->tTextAreaPopup, fW, fH);
	xgeXuiPopupSetOpen(&pGrid->tTextAreaPopup, 1);
	tContent = xgeXuiPopupGetContentRect(&pGrid->tTextAreaPopup);
	__xgeXuiWidgetArrangeRect(pGrid->pTextAreaContentWidget, tContent);
	__xgeXuiTableGridArrangeLocal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaEditWidget, (xge_rect_t){ 10.0f, 10.0f, fW - 20.0f, fH - 58.0f });
	__xgeXuiTableGridArrangeLocal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaCancelWidget, (xge_rect_t){ fW - 172.0f, fH - 40.0f, 76.0f, 28.0f });
	__xgeXuiTableGridArrangeLocal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaOkWidget, (xge_rect_t){ fW - 88.0f, fH - 40.0f, 76.0f, 28.0f });
	xgeXuiSetFocus(pGrid->pContext, pGrid->pTextAreaEditWidget);
	return 1;
}

static int __xgeXuiTableGridOpenPicker(xge_xui_table_grid pGrid)
{
	xge_xui_table_view_cell_t tCell;
	xge_rect_t tRect;

	if ( (pGrid == NULL) || (pGrid->procEditor == NULL) || (pGrid->iEditingRow < 0) || (pGrid->iEditingColumn < 0) ) {
		return 0;
	}
	__xgeXuiTableGridGetCell(pGrid, pGrid->iEditingRow, pGrid->iEditingColumn, &tCell);
	tCell.iType = pGrid->iEditingType;
	if ( __xgeXuiTableGridEditingCellRect(pGrid, &tRect) == 0 ) {
		tRect = (pGrid->pWidget != NULL) ? pGrid->pWidget->tRect : (xge_rect_t){ 0.0f, 0.0f, 0.0f, 0.0f };
	}
	if ( pGrid->procEditor(pGrid->pWidget, pGrid->iEditingRow, pGrid->iEditingColumn, &tCell, tRect, pGrid->pEditorUser) != 0 ) {
		pGrid->iPickerCount++;
		return 1;
	}
	return 0;
}

static void __xgeXuiTableGridConfigureEditInput(xge_xui_table_grid pGrid, int iType)
{
	if ( pGrid == NULL ) {
		return;
	}
	(void)iType;
	xgeXuiInputDecorationClear(&pGrid->tEditInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING);
	xgeXuiInputSetClearButton(&pGrid->tEditInput, 0);
	xgeXuiInputSetReadonly(&pGrid->tEditInput, 0);
	if ( pGrid->pEditWidget != NULL ) {
		xgeXuiWidgetSetImeMode(pGrid->pEditWidget, XGE_XUI_IME_ENABLED);
	}
}

static int __xgeXuiTableGridBeginComboEdit(xge_xui_table_grid pGrid, const char* sText, xge_rect_t tRect)
{
	int i;
	int iSelected;

	if ( (pGrid == NULL) || (pGrid->pComboWidget == NULL) || (pGrid->pCombo == NULL) ) {
		return 0;
	}
	if ( pGrid->tEditorConfig.arrEnumItemData != NULL && pGrid->tEditorConfig.iEnumItemCount > 0 ) {
		xgeXuiComboBoxSetItemData(pGrid->pCombo, pGrid->tEditorConfig.arrEnumItemData, pGrid->tEditorConfig.iEnumItemCount);
		if ( pGrid->tEditorConfig.bEnumUseValue ) {
			xgeXuiComboBoxSetSelectedValue(pGrid->pCombo, pGrid->tEditorConfig.iEnumSelectedValue);
		}
	} else if ( pGrid->tEditorConfig.arrEnumItems != NULL && pGrid->tEditorConfig.iEnumItemCount > 0 ) {
		xgeXuiComboBoxSetItems(pGrid->pCombo, pGrid->tEditorConfig.arrEnumItems, pGrid->tEditorConfig.iEnumItemCount);
	} else {
		return 0;
	}
	if ( pGrid->tEditorConfig.arrEnumEnabled != NULL ) {
		xgeXuiComboBoxSetEnabledItems(pGrid->pCombo, pGrid->tEditorConfig.arrEnumEnabled, pGrid->tEditorConfig.iEnumItemCount);
	}
	iSelected = pGrid->tEditorConfig.iEnumSelected;
	if ( iSelected < 0 && sText != NULL ) {
		for ( i = 0; i < pGrid->pCombo->iItemCount; i++ ) {
			const char* sItem;

			sItem = NULL;
			if ( pGrid->pCombo->arrItemData != NULL ) {
				sItem = pGrid->pCombo->arrItemData[i].sText;
			} else if ( pGrid->pCombo->arrItems != NULL ) {
				sItem = pGrid->pCombo->arrItems[i];
			}
			if ( sItem != NULL && strcmp(sItem, sText) == 0 ) {
				iSelected = i;
				break;
			}
		}
	}
	xgeXuiComboBoxSetSelected(pGrid->pCombo, iSelected);
	__xgeXuiTableGridPlaceEditorWidget(pGrid, pGrid->pComboWidget, tRect);
	xgeXuiWidgetSetVisible(pGrid->pComboWidget, 1);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pComboWidget);
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_COMBO;
	return 1;
}

static int __xgeXuiTableGridBeginColorEdit(xge_xui_table_grid pGrid, const char* sText, xge_rect_t tRect)
{
	if ( (pGrid == NULL) || (pGrid->pColorWidget == NULL) ) {
		return 0;
	}
	xgeXuiColorPickerSetAlphaEnabled(&pGrid->tColorPicker, pGrid->tEditorConfig.bAlphaEnabled);
	if ( pGrid->tEditorConfig.arrPalette != NULL && pGrid->tEditorConfig.iPaletteCount > 0 ) {
		xgeXuiColorPickerSetPalette(&pGrid->tColorPicker, pGrid->tEditorConfig.arrPalette, pGrid->tEditorConfig.iPaletteCount);
	}
	if ( xgeXuiColorPickerSetHex(&pGrid->tColorPicker, sText != NULL ? sText : "") != XGE_OK ) {
		xgeXuiColorPickerSetColor(&pGrid->tColorPicker, XGE_COLOR_RGBA(130, 183, 55, 255));
	}
	__xgeXuiTableGridPlaceEditorWidget(pGrid, pGrid->pColorWidget, tRect);
	xgeXuiWidgetSetVisible(pGrid->pColorWidget, 1);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pColorWidget);
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_COLOR;
	return 1;
}

static int __xgeXuiTableGridBeginDateEdit(xge_xui_table_grid pGrid, const char* sText, xge_rect_t tRect, int iType)
{
	xtime tValue;
	const char* sFormat;
	int iMode;

	if ( (pGrid == NULL) || (pGrid->pDateWidget == NULL) ) {
		return 0;
	}
	iMode = pGrid->tEditorConfig.bDateModeSet ? pGrid->tEditorConfig.iDateMode : __xgeXuiTableGridDateModeForType(iType);
	xgeXuiDatePickerSetMode(&pGrid->tDatePicker, iMode);
	xgeXuiDatePickerSetShowSecond(&pGrid->tDatePicker, pGrid->tEditorConfig.bShowSecond);
	if ( pGrid->tEditorConfig.sDateFormat != NULL ) {
		xgeXuiDatePickerSetFormat(&pGrid->tDatePicker, pGrid->tEditorConfig.sDateFormat);
	} else {
		xgeXuiDatePickerSetFormat(&pGrid->tDatePicker, __xgeXuiTableGridDefaultDateFormat(iMode, pGrid->tEditorConfig.bShowSecond));
	}
	if ( pGrid->tEditorConfig.sRangeSeparator != NULL ) {
		xgeXuiDatePickerSetRangeSeparator(&pGrid->tDatePicker, pGrid->tEditorConfig.sRangeSeparator);
	}
	if ( pGrid->tEditorConfig.tDefaultRangeSpan > 0 ) {
		xgeXuiDatePickerSetDefaultRangeSpan(&pGrid->tDatePicker, pGrid->tEditorConfig.tDefaultRangeSpan);
	}
	if ( pGrid->tEditorConfig.bDateHasMin || pGrid->tEditorConfig.bDateHasMax ) {
		xgeXuiDatePickerSetLimits(&pGrid->tDatePicker, pGrid->tEditorConfig.bDateHasMin ? pGrid->tEditorConfig.tDateMin : 0, pGrid->tEditorConfig.bDateHasMax ? pGrid->tEditorConfig.tDateMax : (xtime)XRT_TIME_DAY * (xtime)365242);
	} else {
		xgeXuiDatePickerClearLimits(&pGrid->tDatePicker);
	}
	sFormat = xgeXuiDatePickerGetFormat(&pGrid->tDatePicker);
	if ( sText != NULL && sText[0] != 0 ) {
		tValue = xrtTimeParse((str)sText, (str)((sFormat != NULL && sFormat[0] != 0) ? sFormat : __xgeXuiTableGridDefaultDateFormat(iMode, pGrid->tEditorConfig.bShowSecond)));
		xgeXuiDatePickerSetValue(&pGrid->tDatePicker, tValue);
	} else {
		xgeXuiDatePickerSetValue(&pGrid->tDatePicker, xrtNow());
	}
	__xgeXuiTableGridPlaceEditorWidget(pGrid, pGrid->pDateWidget, tRect);
	xgeXuiWidgetSetVisible(pGrid->pDateWidget, 1);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pDateWidget);
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_DATE;
	return 1;
}

static int __xgeXuiTableGridCountProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	pGrid = (xge_xui_table_grid)pUser;
	if ( (pGrid == NULL) || (pGrid->procCount == NULL) ) {
		return 0;
	}
	return pGrid->procCount(pGrid->pWidget, pGrid->pAdapterUser);
}

static int __xgeXuiTableGridCellProc(xge_xui_widget pWidget, int iRow, int iColumn, xge_xui_table_view_cell_t* pCell, void* pUser)
{
	xge_xui_table_grid pGrid;
	int iRet;

	(void)pWidget;
	pGrid = (xge_xui_table_grid)pUser;
	if ( (pGrid == NULL) || (pCell == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	__xgeXuiTableGridDefaultCell(pGrid, iColumn, pCell);
	iRet = XGE_OK;
	if ( pGrid->procCell != NULL ) {
		iRet = pGrid->procCell(pGrid->pWidget, iRow, iColumn, pCell, pGrid->pAdapterUser);
	}
	if ( (pGrid->iEditingRow == iRow) && (pGrid->iEditingColumn == iColumn) ) {
		pCell->bEditing = 1;
	}
	return iRet;
}

static int __xgeXuiTableGridInputFilterProc(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	(void)sOldText;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid == NULL ) {
		return 1;
	}
	if ( __xgeXuiTableGridIsNumericType(pGrid->iEditingType) ) {
		return __xgeXuiTableGridValidateNumberText(sNewText, pGrid->iEditingType);
	}
	return 1;
}

static int __xgeXuiTableGridInputEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_table_grid pGrid;
	int iResult;

	(void)pWidget;
	pGrid = (xge_xui_table_grid)pUser;
	if ( (pGrid == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ENTER) ) {
		return xgeXuiTableGridEndEdit(pGrid, 1) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_HANDLED;
	}
	if ( (pEvent->iType == XGE_EVENT_KEY_DOWN) && (pEvent->iParam1 == XGE_KEY_ESCAPE) ) {
		(void)xgeXuiTableGridEndEdit(pGrid, 0);
		return XGE_XUI_EVENT_CONSUMED;
	}
	if ( pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_NUMERIC ) {
		iResult = xgeXuiNumericInputEvent(&pGrid->tEditNumeric, pEvent);
	} else {
		iResult = xgeXuiInputEvent(&pGrid->tEditInput, pEvent);
	}
	if ( pEvent->iType == XGE_EVENT_XUI_FOCUS_OUT ) {
		xge_xui_input pInput;

		if ( __xgeXuiTableGridEditorPopupOpen(pGrid) != 0 ) {
			return iResult;
		}
		pInput = (pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_NUMERIC) ? &pGrid->tEditNumeric.tInput : &pGrid->tEditInput;
		if ( (pInput->pDefaultMenu != NULL) && (xgeXuiMenuIsOpen(pInput->pDefaultMenu) != 0) ) {
			return iResult;
		}
		(void)xgeXuiTableGridEndEdit(pGrid, 1);
	}
	return iResult;
}

static void __xgeXuiTableGridUpdateProc(xge_xui_widget pWidget, float fDelta, void* pUser)
{
	(void)pWidget;
	(void)fDelta;
	(void)__xgeXuiTableGridSyncActiveEditorRect((xge_xui_table_grid)pUser);
}

static void __xgeXuiTableGridComboSelectProc(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	(void)iIndex;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid != NULL && pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_COMBO ) {
		(void)xgeXuiTableGridEndEdit(pGrid, 1);
	}
}

static void __xgeXuiTableGridColorChangeProc(xge_xui_widget pWidget, uint32_t iColor, void* pUser)
{
	xge_xui_table_grid pGrid;
	const char* sValue;

	(void)pWidget;
	(void)iColor;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid == NULL || pGrid->iActiveEditor != XGE_XUI_TABLE_GRID_EDITOR_COLOR ) {
		return;
	}
	sValue = xgeXuiColorPickerGetHex(&pGrid->tColorPicker);
	if ( __xgeXuiTableGridCommitValue(pGrid, sValue) != 0 ) {
		snprintf(pGrid->sOriginalValue, sizeof(pGrid->sOriginalValue), "%s", sValue != NULL ? sValue : "");
		xgeXuiTableViewRefresh(&pGrid->tTable);
	}
}

static void __xgeXuiTableGridDateCommitProc(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid != NULL && pGrid->iActiveEditor == XGE_XUI_TABLE_GRID_EDITOR_DATE ) {
		(void)xgeXuiTableGridEndEdit(pGrid, 1);
	}
}

static void __xgeXuiTableGridTextAreaOkProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid == NULL ) {
		return;
	}
	(void)xgeXuiTableGridEndEdit(pGrid, 1);
}

static void __xgeXuiTableGridTextAreaCancelProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_table_grid pGrid;

	(void)pWidget;
	pGrid = (xge_xui_table_grid)pUser;
	if ( pGrid == NULL ) {
		return;
	}
	(void)xgeXuiTableGridEndEdit(pGrid, 0);
}

int xgeXuiTableGridInit(xge_xui_table_grid pGrid, xge_xui_context pContext, xge_xui_widget pWidget)
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
	pGrid->iEditMode = XGE_XUI_TABLE_GRID_EDIT_DISPLAY;
	pGrid->iEditingRow = -1;
	pGrid->iEditingColumn = -1;
	pGrid->iEditingType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NONE;
	iRet = xgeXuiTableViewInit(&pGrid->tTable, pContext, pWidget);
	if ( iRet != XGE_OK ) {
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	pGrid->pEditWidget = xgeXuiWidgetCreate();
	pGrid->pNumericWidget = xgeXuiWidgetCreate();
	pGrid->pComboWidget = xgeXuiWidgetCreate();
	pGrid->pColorWidget = xgeXuiWidgetCreate();
	pGrid->pDateWidget = xgeXuiWidgetCreate();
	pGrid->pTextAreaPopupWidget = xgeXuiWidgetCreate();
	pGrid->pTextAreaContentWidget = xgeXuiWidgetCreate();
	pGrid->pTextAreaEditWidget = xgeXuiWidgetCreate();
	pGrid->pTextAreaOkWidget = xgeXuiWidgetCreate();
	pGrid->pTextAreaCancelWidget = xgeXuiWidgetCreate();
	pGrid->pCombo = (xge_xui_combo_box)calloc(1, sizeof(xge_xui_combo_box_t));
	if ( pGrid->pEditWidget == NULL || pGrid->pNumericWidget == NULL || pGrid->pComboWidget == NULL || pGrid->pColorWidget == NULL || pGrid->pDateWidget == NULL ||
	     pGrid->pTextAreaPopupWidget == NULL || pGrid->pTextAreaContentWidget == NULL || pGrid->pTextAreaEditWidget == NULL ||
	     pGrid->pTextAreaOkWidget == NULL || pGrid->pTextAreaCancelWidget == NULL || pGrid->pCombo == NULL ) {
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaPopupWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaContentWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaEditWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaOkWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaCancelWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	iRet = xgeXuiInputInit(&pGrid->tEditInput, pContext, pGrid->pEditWidget, pGrid->pFont);
	if ( iRet != XGE_OK ) {
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiNumericInputInit(&pGrid->tEditNumeric, pContext, pGrid->pNumericWidget, pGrid->pFont);
	if ( iRet != XGE_OK ) {
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiComboBoxInit(pGrid->pCombo, pContext, pGrid->pComboWidget);
	if ( iRet != XGE_OK ) {
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiColorPickerInit(&pGrid->tColorPicker, pContext, pGrid->pColorWidget, pGrid->pFont);
	if ( iRet != XGE_OK ) {
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiDatePickerInit(&pGrid->tDatePicker, pContext, pGrid->pDateWidget, pGrid->pFont);
	if ( iRet != XGE_OK ) {
		xgeXuiColorPickerUnit(&pGrid->tColorPicker);
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiPopupInit(&pGrid->tTextAreaPopup, pContext, pGrid->pTextAreaPopupWidget);
	if ( iRet != XGE_OK ) {
		xgeXuiDatePickerUnit(&pGrid->tDatePicker);
		xgeXuiColorPickerUnit(&pGrid->tColorPicker);
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaContentWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaEditWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaOkWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaCancelWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	iRet = xgeXuiTextEditInit(&pGrid->tTextAreaEdit, pContext, pGrid->pTextAreaEditWidget, pGrid->pFont);
	if ( iRet != XGE_OK ) {
		xgeXuiPopupUnit(&pGrid->tTextAreaPopup);
		xgeXuiDatePickerUnit(&pGrid->tDatePicker);
		xgeXuiColorPickerUnit(&pGrid->tColorPicker);
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaEditWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaOkWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaCancelWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return iRet;
	}
	if ( xgeXuiButtonInit(&pGrid->tTextAreaOk, pContext, pGrid->pTextAreaOkWidget) != XGE_OK ||
	     xgeXuiButtonInit(&pGrid->tTextAreaCancel, pContext, pGrid->pTextAreaCancelWidget) != XGE_OK ) {
		xgeXuiTextEditUnit(&pGrid->tTextAreaEdit);
		xgeXuiPopupUnit(&pGrid->tTextAreaPopup);
		xgeXuiDatePickerUnit(&pGrid->tDatePicker);
		xgeXuiColorPickerUnit(&pGrid->tColorPicker);
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaEditWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaOkWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaCancelWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	xgeXuiWidgetSetEvent(pWidget, xgeXuiTableGridEventProc, pGrid);
	xgeXuiWidgetSetEventInterest(pWidget, XGE_XUI_EVENT_MASK_DOUBLE_CLICK, 1);
	xgeXuiWidgetSetUpdate(pWidget, __xgeXuiTableGridUpdateProc, pGrid);
	xgeXuiWidgetSetEvent(pGrid->pEditWidget, __xgeXuiTableGridInputEventProc, pGrid);
	xgeXuiWidgetSetEvent(pGrid->pNumericWidget, __xgeXuiTableGridInputEventProc, pGrid);
	xgeXuiWidgetSetPaddingPx(pGrid->pEditWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetPaddingPx(pGrid->pNumericWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 0);
	xgeXuiWidgetSetVisible(pGrid->pNumericWidget, 0);
	xgeXuiWidgetSetVisible(pGrid->pComboWidget, 0);
	xgeXuiWidgetSetVisible(pGrid->pColorWidget, 0);
	xgeXuiWidgetSetVisible(pGrid->pDateWidget, 0);
	xgeXuiComboBoxSetFont(pGrid->pCombo, pGrid->pFont);
	xgeXuiComboBoxSetSelect(pGrid->pCombo, __xgeXuiTableGridComboSelectProc, pGrid);
	xgeXuiColorPickerSetChange(&pGrid->tColorPicker, __xgeXuiTableGridColorChangeProc, pGrid);
	xgeXuiDatePickerSetCommit(&pGrid->tDatePicker, __xgeXuiTableGridDateCommitProc, pGrid);
	xgeXuiPopupSetOwner(&pGrid->tTextAreaPopup, pWidget);
	xgeXuiPopupSetFocusRestore(&pGrid->tTextAreaPopup, pGrid->pEditWidget);
	xgeXuiPopupSetContentWidget(&pGrid->tTextAreaPopup, pGrid->pTextAreaContentWidget);
	xgeXuiPopupSetBackground(&pGrid->tTextAreaPopup, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiPopupSetBorder(&pGrid->tTextAreaPopup, XGE_COLOR_RGBA(126, 166, 200, 255));
	xgeXuiWidgetSetLayout(pGrid->pTextAreaContentWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pGrid->pTextAreaContentWidget, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiWidgetSetBorder(pGrid->pTextAreaContentWidget, 0.0f, 0);
	xgeXuiWidgetAddInternal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaEditWidget);
	xgeXuiWidgetAddInternal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaCancelWidget);
	xgeXuiWidgetAddInternal(pGrid->pTextAreaContentWidget, pGrid->pTextAreaOkWidget);
	xgeXuiButtonSetText(&pGrid->tTextAreaCancel, pGrid->pFont, "Cancel");
	xgeXuiButtonSetText(&pGrid->tTextAreaOk, pGrid->pFont, "OK");
	xgeXuiButtonSetClick(&pGrid->tTextAreaCancel, __xgeXuiTableGridTextAreaCancelProc, pGrid);
	xgeXuiButtonSetClick(&pGrid->tTextAreaOk, __xgeXuiTableGridTextAreaOkProc, pGrid);
	xgeXuiTextEditSetScrollbarMode(&pGrid->tTextAreaEdit, XGE_XUI_SCROLLBAR_MODE_COMPACT);
	xgeXuiInputSetMaxLength(&pGrid->tEditInput, XGE_XUI_TABLE_GRID_VALUE_CAPACITY - 1);
	xgeXuiInputSetClearButton(&pGrid->tEditInput, 0);
	xgeXuiInputSetFilter(&pGrid->tEditInput, __xgeXuiTableGridInputFilterProc, pGrid);
	xgeXuiInputSetFrameColors(&pGrid->tEditInput, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(53, 147, 218, 255), XGE_COLOR_RGBA(53, 147, 218, 255), XGE_COLOR_RGBA(53, 147, 218, 255));
	xgeXuiInputSetMaxLength(&pGrid->tEditNumeric.tInput, XGE_XUI_TABLE_GRID_VALUE_CAPACITY - 1);
	xgeXuiInputSetClearButton(&pGrid->tEditNumeric.tInput, 0);
	xgeXuiInputSetFilter(&pGrid->tEditNumeric.tInput, __xgeXuiTableGridInputFilterProc, pGrid);
	xgeXuiInputSetFrameColors(&pGrid->tEditNumeric.tInput, XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(255, 255, 255, 255), XGE_COLOR_RGBA(53, 147, 218, 255), XGE_COLOR_RGBA(53, 147, 218, 255), XGE_COLOR_RGBA(53, 147, 218, 255));
	if ( xgeXuiWidgetAddInternal(pWidget, pGrid->pEditWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pGrid->pNumericWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pGrid->pComboWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pGrid->pColorWidget) != XGE_OK ||
	     xgeXuiWidgetAddInternal(pWidget, pGrid->pDateWidget) != XGE_OK ) {
		xgeXuiButtonUnit(&pGrid->tTextAreaOk);
		xgeXuiButtonUnit(&pGrid->tTextAreaCancel);
		xgeXuiTextEditUnit(&pGrid->tTextAreaEdit);
		xgeXuiPopupUnit(&pGrid->tTextAreaPopup);
		xgeXuiDatePickerUnit(&pGrid->tDatePicker);
		xgeXuiColorPickerUnit(&pGrid->tColorPicker);
		xgeXuiComboBoxUnit(pGrid->pCombo);
		xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
		xgeXuiInputUnit(&pGrid->tEditInput);
		xgeXuiWidgetFree(pGrid->pEditWidget);
		xgeXuiWidgetFree(pGrid->pNumericWidget);
		xgeXuiWidgetFree(pGrid->pComboWidget);
		xgeXuiWidgetFree(pGrid->pColorWidget);
		xgeXuiWidgetFree(pGrid->pDateWidget);
		xgeXuiWidgetFree(pGrid->pTextAreaPopupWidget);
		free(pGrid->pCombo);
		xgeXuiTableViewUnit(&pGrid->tTable);
		memset(pGrid, 0, sizeof(*pGrid));
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pWidget->pUser = pGrid;
	return XGE_OK;
}

void xgeXuiTableGridUnit(xge_xui_table_grid pGrid)
{
	xge_xui_widget pEdit;
	xge_xui_widget pNumeric;
	xge_xui_widget pCombo;
	xge_xui_widget pColor;
	xge_xui_widget pDate;
	xge_xui_widget pTextAreaPopup;
	xge_xui_widget pWidget;

	if ( pGrid == NULL ) {
		return;
	}
	pEdit = pGrid->pEditWidget;
	pNumeric = pGrid->pNumericWidget;
	pCombo = pGrid->pComboWidget;
	pColor = pGrid->pColorWidget;
	pDate = pGrid->pDateWidget;
	pTextAreaPopup = pGrid->pTextAreaPopupWidget;
	pWidget = pGrid->pWidget;
	if ( pWidget != NULL && pWidget->procUpdate == __xgeXuiTableGridUpdateProc && pWidget->pUpdateUser == pGrid ) {
		pWidget->procUpdate = NULL;
		pWidget->pUpdateUser = NULL;
		pWidget->iCallbackFlags &= ~XGE_XUI_WIDGET_CALLBACK_UPDATE;
	}
	xgeXuiButtonUnit(&pGrid->tTextAreaOk);
	xgeXuiButtonUnit(&pGrid->tTextAreaCancel);
	xgeXuiTextEditUnit(&pGrid->tTextAreaEdit);
	xgeXuiPopupUnit(&pGrid->tTextAreaPopup);
	xgeXuiDatePickerUnit(&pGrid->tDatePicker);
	xgeXuiColorPickerUnit(&pGrid->tColorPicker);
	xgeXuiComboBoxUnit(pGrid->pCombo);
	xgeXuiNumericInputUnit(&pGrid->tEditNumeric);
	xgeXuiInputUnit(&pGrid->tEditInput);
	xgeXuiWidgetFree(pEdit);
	xgeXuiWidgetFree(pNumeric);
	xgeXuiWidgetFree(pCombo);
	xgeXuiWidgetFree(pColor);
	xgeXuiWidgetFree(pDate);
	xgeXuiWidgetFree(pTextAreaPopup);
	free(pGrid->pCombo);
	if ( pWidget != NULL ) {
		pWidget->pUser = &pGrid->tTable;
		xgeXuiWidgetSetEvent(pWidget, xgeXuiTableViewEventProc, &pGrid->tTable);
	}
	xgeXuiTableViewUnit(&pGrid->tTable);
	memset(pGrid, 0, sizeof(*pGrid));
}

xge_xui_table_view xgeXuiTableGridGetTableView(xge_xui_table_grid pGrid)
{
	return (pGrid != NULL) ? &pGrid->tTable : NULL;
}

void xgeXuiTableGridSetColumns(xge_xui_table_grid pGrid, const xge_xui_table_view_column_t* arrColumns, int iCount)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetColumns(&pGrid->tTable, arrColumns, iCount);
	}
}

void xgeXuiTableGridSetRows(xge_xui_table_grid pGrid, const xge_xui_table_view_row_t* arrRows, int iCount)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetRows(&pGrid->tTable, arrRows, iCount);
	}
}

void xgeXuiTableGridSetAdapter(xge_xui_table_grid pGrid, xge_xui_table_view_count_proc procCount, xge_xui_table_view_cell_proc procCell, xge_xui_table_grid_set_proc procSet, void* pUser)
{
	if ( pGrid == NULL ) {
		return;
	}
	pGrid->procCount = procCount;
	pGrid->procCell = procCell;
	pGrid->procSet = procSet;
	pGrid->pAdapterUser = pUser;
	pGrid->pSetUser = pUser;
	xgeXuiTableViewSetAdapter(&pGrid->tTable, __xgeXuiTableGridCountProc, __xgeXuiTableGridCellProc, pGrid);
}

void xgeXuiTableGridSetValidate(xge_xui_table_grid pGrid, xge_xui_table_grid_validate_proc procValidate, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procValidate = procValidate;
		pGrid->pValidateUser = pUser;
	}
}

void xgeXuiTableGridSetChange(xge_xui_table_grid pGrid, xge_xui_table_grid_change_proc procChange, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procChange = procChange;
		pGrid->pChangeUser = pUser;
	}
}

void xgeXuiTableGridSetEditor(xge_xui_table_grid pGrid, xge_xui_table_grid_editor_proc procEditor, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procEditor = procEditor;
		pGrid->pEditorUser = pUser;
	}
}

void xgeXuiTableGridSetEditorConfig(xge_xui_table_grid pGrid, xge_xui_table_grid_editor_config_proc procConfig, void* pUser)
{
	if ( pGrid != NULL ) {
		pGrid->procEditorConfig = procConfig;
		pGrid->pEditorConfigUser = pUser;
	}
}

void xgeXuiTableGridSetEditMode(xge_xui_table_grid pGrid, int iMode)
{
	if ( pGrid != NULL ) {
		pGrid->iEditMode = (iMode == XGE_XUI_TABLE_GRID_EDIT_QUICK) ? XGE_XUI_TABLE_GRID_EDIT_QUICK : XGE_XUI_TABLE_GRID_EDIT_DISPLAY;
	}
}

int xgeXuiTableGridGetEditMode(xge_xui_table_grid pGrid)
{
	return (pGrid != NULL) ? pGrid->iEditMode : XGE_XUI_TABLE_GRID_EDIT_DISPLAY;
}

void xgeXuiTableGridSetFont(xge_xui_table_grid pGrid, xui_font pFont)
{
	if ( pGrid != NULL ) {
		pGrid->pFont = pFont;
		xgeXuiTableViewSetFont(&pGrid->tTable, pFont);
		xgeXuiInputSetFont(&pGrid->tEditInput, pFont);
		xgeXuiInputSetFont(&pGrid->tEditNumeric.tInput, pFont);
		xgeXuiComboBoxSetFont(pGrid->pCombo, pFont);
		pGrid->tColorPicker.pFont = pFont;
		pGrid->tDatePicker.pFont = pFont;
		xgeXuiTextEditSetFont(&pGrid->tTextAreaEdit, pFont);
		xgeXuiButtonSetText(&pGrid->tTextAreaCancel, pFont, "Cancel");
		xgeXuiButtonSetText(&pGrid->tTextAreaOk, pFont, "OK");
	}
}

void xgeXuiTableGridSetDefaultMetrics(xge_xui_table_grid pGrid, float fColumnWidth, float fRowHeight, float fHeaderHeight)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetDefaultMetrics(&pGrid->tTable, fColumnWidth, fRowHeight, fHeaderHeight);
	}
}

void xgeXuiTableGridSetSelectionMode(xge_xui_table_grid pGrid, int iMode)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetSelectionMode(&pGrid->tTable, iMode);
	}
}

void xgeXuiTableGridSetScrollbarMode(xge_xui_table_grid pGrid, int iMode)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetScrollbarMode(&pGrid->tTable, iMode);
	}
}

void xgeXuiTableGridSetColors(xge_xui_table_grid pGrid, uint32_t iBackground, uint32_t iHeader, uint32_t iRow, uint32_t iSelected, uint32_t iGrid, uint32_t iText)
{
	if ( pGrid != NULL ) {
		xgeXuiTableViewSetColors(&pGrid->tTable, iBackground, iHeader, iRow, iSelected, iGrid, iText);
		xgeXuiInputSetColors(&pGrid->tEditInput, iText, XGE_COLOR_RGBA(255, 255, 255, 255), iSelected, iText);
		xgeXuiInputSetColors(&pGrid->tEditNumeric.tInput, iText, XGE_COLOR_RGBA(255, 255, 255, 255), iSelected, iText);
	}
}

int xgeXuiTableGridBeginEdit(xge_xui_table_grid pGrid, int iRow, int iColumn)
{
	xge_xui_table_view_cell_t tCell;
	xge_rect_t tRect;
	char sValue[XGE_XUI_TABLE_GRID_VALUE_CAPACITY];
	const char* sText;
	int iType;
	int iAlign;

	if ( (pGrid == NULL) || (iRow < 0) || (iColumn < 0) || (iColumn >= pGrid->tTable.iColumnCount) ) {
		return 0;
	}
	if ( xgeXuiTableViewGetRowCount(&pGrid->tTable) <= iRow ) {
		return 0;
	}
	(void)xgeXuiTableGridEndEdit(pGrid, 1);
	__xgeXuiTableGridGetCell(pGrid, iRow, iColumn, &tCell);
	if ( tCell.bDisabled != 0 ) {
		return 0;
	}
	iType = tCell.iType;
	sText = __xgeXuiTableGridCellText(pGrid, iRow, iColumn, &tCell, sValue, (int)sizeof(sValue));
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_BOOL ) {
		int bOK;

		pGrid->iEditingRow = iRow;
		pGrid->iEditingColumn = iColumn;
		pGrid->iEditingType = iType;
		snprintf(pGrid->sOriginalValue, sizeof(pGrid->sOriginalValue), "%s", __xgeXuiTableGridBoolValue(sText, tCell.pValue) ? "true" : "false");
		bOK = __xgeXuiTableGridCommitValue(pGrid, __xgeXuiTableGridBoolValue(sText, tCell.pValue) ? "false" : "true");
		pGrid->iEditingRow = -1;
		pGrid->iEditingColumn = -1;
		pGrid->iEditingType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
		pGrid->sOriginalValue[0] = 0;
		xgeXuiTableViewRefresh(&pGrid->tTable);
		return bOK;
	}
	xgeXuiTableViewEnsureCellVisible(&pGrid->tTable, iRow, iColumn);
	if ( xgeXuiTableViewGetCellRect(&pGrid->tTable, iRow, iColumn, &tRect) == 0 ) {
		return 0;
	}
	__xgeXuiTableGridAdjustInlineEditorRect(&tRect);
	if ( __xgeXuiTableGridClipEditRect(pGrid, &tRect) == 0 ) {
		return 0;
	}
	pGrid->iEditingRow = iRow;
	pGrid->iEditingColumn = iColumn;
	pGrid->iEditingType = iType;
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NONE;
	snprintf(pGrid->sOriginalValue, sizeof(pGrid->sOriginalValue), "%s", sText != NULL ? sText : "");
	(void)__xgeXuiTableGridLoadEditorConfig(pGrid, iRow, iColumn, iType);
	__xgeXuiTableGridHideEditors(pGrid);
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_TEXTAREA ) {
		xgeXuiTableViewRefresh(&pGrid->tTable);
		return __xgeXuiTableGridOpenTextArea(pGrid);
	}
	if ( __xgeXuiTableGridIsPickerType(iType) ) {
		int bOK;

		xgeXuiTableViewRefresh(&pGrid->tTable);
		bOK = __xgeXuiTableGridOpenPicker(pGrid);
		pGrid->iEditingRow = -1;
		pGrid->iEditingColumn = -1;
		pGrid->iEditingType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
		pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NONE;
		pGrid->sOriginalValue[0] = 0;
		xgeXuiTableViewRefresh(&pGrid->tTable);
		return bOK;
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_ENUM ) {
		if ( __xgeXuiTableGridBeginComboEdit(pGrid, pGrid->sOriginalValue, tRect) != 0 ) {
			xgeXuiTableViewRefresh(&pGrid->tTable);
			return 1;
		}
		if ( pGrid->procEditor != NULL && pGrid->procEditor(pGrid->pWidget, iRow, iColumn, &tCell, tRect, pGrid->pEditorUser) != 0 ) {
			pGrid->iPickerCount++;
			return 1;
		}
	}
	if ( iType == XGE_XUI_TABLE_CELL_TYPE_COLOR ) {
		if ( __xgeXuiTableGridBeginColorEdit(pGrid, pGrid->sOriginalValue, tRect) != 0 ) {
			xgeXuiTableViewRefresh(&pGrid->tTable);
			return 1;
		}
	}
	if ( __xgeXuiTableGridIsDateType(iType) ) {
		if ( __xgeXuiTableGridBeginDateEdit(pGrid, pGrid->sOriginalValue, tRect, iType) != 0 ) {
			xgeXuiTableViewRefresh(&pGrid->tTable);
			return 1;
		}
	}
	if ( (pGrid->procEditor != NULL) && (iType == XGE_XUI_TABLE_CELL_TYPE_CUSTOM) ) {
		if ( pGrid->procEditor(pGrid->pWidget, iRow, iColumn, &tCell, tRect, pGrid->pEditorUser) != 0 ) {
			pGrid->iPickerCount++;
			pGrid->iEditingRow = -1;
			pGrid->iEditingColumn = -1;
			pGrid->iEditingType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
			pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NONE;
			pGrid->sOriginalValue[0] = 0;
			return 1;
		}
	}
	if ( __xgeXuiTableGridIsNumericType(iType) ) {
		if ( __xgeXuiTableGridBeginNumericEdit(pGrid, pGrid->sOriginalValue, tRect, iType) != 0 ) {
			xgeXuiTableViewRefresh(&pGrid->tTable);
			return 1;
		}
	}
	__xgeXuiTableGridConfigureEditInput(pGrid, iType);
	xgeXuiInputSetError(&pGrid->tEditInput, 0);
	xgeXuiInputSetText(&pGrid->tEditInput, pGrid->sOriginalValue);
	xgeXuiInputSetSelection(&pGrid->tEditInput, 0, pGrid->tEditInput.tText.iSize);
	if ( __xgeXuiTableGridIsNumericType(iType) ) {
		xgeXuiInputSetTextAlign(&pGrid->tEditInput, XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);
	} else {
		iAlign = pGrid->tTable.arrColumns[iColumn].iAlign;
		if ( (iAlign & XGE_TEXT_ALIGN_RIGHT) != 0 ) {
			xgeXuiInputSetTextAlign(&pGrid->tEditInput, XGE_XUI_INPUT_TEXT_ALIGN_RIGHT);
		} else if ( (iAlign & XGE_TEXT_ALIGN_CENTER) != 0 ) {
			xgeXuiInputSetTextAlign(&pGrid->tEditInput, XGE_XUI_INPUT_TEXT_ALIGN_CENTER);
		} else {
			xgeXuiInputSetTextAlign(&pGrid->tEditInput, XGE_XUI_INPUT_TEXT_ALIGN_LEFT);
		}
	}
	__xgeXuiTableGridPlaceEditWidget(pGrid, tRect);
	xgeXuiWidgetSetVisible(pGrid->pEditWidget, 1);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pEditWidget);
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_INPUT;
	xgeXuiTableViewRefresh(&pGrid->tTable);
	return 1;
}

int xgeXuiTableGridEndEdit(xge_xui_table_grid pGrid, int bCommit)
{
	char sBuffer[XGE_XUI_TABLE_GRID_VALUE_CAPACITY];
	const char* sValue;
	int bOK;

	if ( (pGrid == NULL) || (pGrid->iEditingRow < 0) || (pGrid->iEditingColumn < 0) ) {
		return 1;
	}
	bOK = 1;
	if ( bCommit != 0 ) {
		sValue = __xgeXuiTableGridCurrentEditorValue(pGrid, sBuffer, (int)sizeof(sBuffer));
		bOK = __xgeXuiTableGridCommitValue(pGrid, sValue);
		if ( bOK == 0 ) {
			return 0;
		}
	} else {
		pGrid->iCancelCount++;
	}
	pGrid->iEditingRow = -1;
	pGrid->iEditingColumn = -1;
	pGrid->iEditingType = XGE_XUI_TABLE_CELL_TYPE_TEXT;
	pGrid->iActiveEditor = XGE_XUI_TABLE_GRID_EDITOR_NONE;
	pGrid->sOriginalValue[0] = 0;
	xgeXuiInputDecorationClear(&pGrid->tEditInput, XGE_XUI_INPUT_DECORATION_SIDE_TRAILING);
	xgeXuiInputSetText(&pGrid->tEditInput, "");
	xgeXuiInputSetError(&pGrid->tEditInput, 0);
	xgeXuiInputSetText(&pGrid->tEditNumeric.tInput, "");
	xgeXuiInputSetError(&pGrid->tEditNumeric.tInput, 0);
	xgeXuiWidgetSetImeMode(pGrid->pEditWidget, XGE_XUI_IME_ENABLED);
	xgeXuiWidgetSetImeMode(pGrid->pNumericWidget, XGE_XUI_IME_ENABLED);
	__xgeXuiTableGridHideEditors(pGrid);
	xgeXuiSetFocus(pGrid->pContext, pGrid->pWidget);
	xgeXuiTableViewRefresh(&pGrid->tTable);
	return bOK;
}

int xgeXuiTableGridIsEditing(xge_xui_table_grid pGrid)
{
	return (pGrid != NULL) && (pGrid->iEditingRow >= 0) && (pGrid->iEditingColumn >= 0);
}

void xgeXuiTableGridGetEditingCell(xge_xui_table_grid pGrid, int* pRow, int* pColumn)
{
	if ( pRow != NULL ) {
		*pRow = (pGrid != NULL) ? pGrid->iEditingRow : -1;
	}
	if ( pColumn != NULL ) {
		*pColumn = (pGrid != NULL) ? pGrid->iEditingColumn : -1;
	}
}

int xgeXuiTableGridEvent(xge_xui_table_grid pGrid, const xge_event_t* pEvent)
{
	int iRow;
	int iColumn;
	int iRet;
	xge_xui_widget pActive;

	if ( (pGrid == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pGrid->pContext != NULL && pGrid->pContext->pFocus == pGrid->pWidget ) {
		if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) ) {
			xgeXuiTableViewGetSelectedCell(&pGrid->tTable, &iRow, &iColumn);
			if ( iColumn < 0 ) {
				xgeXuiTableViewGetActiveCell(&pGrid->tTable, &iRow, &iColumn);
			}
			return xgeXuiTableGridBeginEdit(pGrid, iRow, iColumn) ? XGE_XUI_EVENT_CONSUMED : XGE_XUI_EVENT_CONTINUE;
		}
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_DOWN ) {
		pActive = __xgeXuiTableGridActiveWidget(pGrid);
		if ( xgeXuiTableGridIsEditing(pGrid) && __xgeXuiTableGridEditorPopupOpen(pGrid) == 0 && ((pActive == NULL) || (__xgeXuiRectContains(pActive->tRect, pEvent->fX, pEvent->fY) == 0)) ) {
			if ( xgeXuiTableGridEndEdit(pGrid, 1) == 0 ) {
				return XGE_XUI_EVENT_CONSUMED;
			}
		}
	}
	if ( pEvent->iType == XGE_EVENT_MOUSE_WHEEL && xgeXuiTableGridIsEditing(pGrid) ) {
		if ( xgeXuiTableGridEndEdit(pGrid, 1) == 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( (pEvent->iType == XGE_EVENT_XUI_DOUBLE_CLICK) && (pGrid->iEditMode == XGE_XUI_TABLE_GRID_EDIT_QUICK) ) {
		return XGE_XUI_EVENT_CONSUMED;
	}
	iRet = xgeXuiTableViewEvent(&pGrid->tTable, pEvent);
	if ( (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pGrid->iEditMode == XGE_XUI_TABLE_GRID_EDIT_QUICK) && (iRet == XGE_XUI_EVENT_CONSUMED) ) {
		xgeXuiTableViewGetActiveCell(&pGrid->tTable, &iRow, &iColumn);
		if ( xgeXuiTableGridBeginEdit(pGrid, iRow, iColumn) != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	if ( pEvent->iType == XGE_EVENT_XUI_DOUBLE_CLICK ) {
		xgeXuiTableViewGetActiveCell(&pGrid->tTable, &iRow, &iColumn);
		if ( xgeXuiTableGridBeginEdit(pGrid, iRow, iColumn) != 0 ) {
			return XGE_XUI_EVENT_CONSUMED;
		}
	}
	return iRet;
}

int xgeXuiTableGridEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiTableGridEvent((xge_xui_table_grid)pUser, pEvent);
}
