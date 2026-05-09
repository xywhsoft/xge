static int __xgeXuiDatePickerCompare(int iYearA, int iMonthA, int iDayA, int iYearB, int iMonthB, int iDayB)
{
	if ( iYearA != iYearB ) {
		return (iYearA < iYearB) ? -1 : 1;
	}
	if ( iMonthA != iMonthB ) {
		return (iMonthA < iMonthB) ? -1 : 1;
	}
	if ( iDayA != iDayB ) {
		return (iDayA < iDayB) ? -1 : 1;
	}
	return 0;
}

static void __xgeXuiDatePickerNormalizeMonth(int* pYear, int* pMonth)
{
	int iMonthIndex;
	int iYears;

	if ( (pYear == NULL) || (pMonth == NULL) ) {
		return;
	}
	iMonthIndex = *pMonth - 1;
	if ( iMonthIndex >= 0 ) {
		*pYear += iMonthIndex / 12;
		*pMonth = iMonthIndex % 12 + 1;
		return;
	}
	iYears = (-iMonthIndex + 11) / 12;
	*pYear -= iYears;
	iMonthIndex += iYears * 12;
	*pMonth = iMonthIndex + 1;
}

static int __xgeXuiDatePickerIsLeap(int iYear)
{
	return ((iYear % 4 == 0) && (iYear % 100 != 0)) || (iYear % 400 == 0);
}

static int __xgeXuiDatePickerDaysInMonth(int iYear, int iMonth)
{
	static const int arrDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
	if ( iMonth == 2 && __xgeXuiDatePickerIsLeap(iYear) ) {
		return 29;
	}
	return arrDays[iMonth - 1];
}

static void __xgeXuiDatePickerNormalizeDate(int* pYear, int* pMonth, int* pDay)
{
	int iDays;

	if ( (pYear == NULL) || (pMonth == NULL) || (pDay == NULL) ) {
		return;
	}
	__xgeXuiDatePickerNormalizeMonth(pYear, pMonth);
	iDays = __xgeXuiDatePickerDaysInMonth(*pYear, *pMonth);
	if ( *pDay < 1 ) {
		*pDay = 1;
	}
	if ( *pDay > iDays ) {
		*pDay = iDays;
	}
}

static void __xgeXuiDatePickerClampDate(xge_xui_date_picker pPicker, int* pYear, int* pMonth, int* pDay)
{
	if ( (pPicker == NULL) || (pYear == NULL) || (pMonth == NULL) || (pDay == NULL) ) {
		return;
	}
	__xgeXuiDatePickerNormalizeDate(pYear, pMonth, pDay);
	if ( pPicker->bHasMin && __xgeXuiDatePickerCompare(*pYear, *pMonth, *pDay, pPicker->iMinYear, pPicker->iMinMonth, pPicker->iMinDay) < 0 ) {
		*pYear = pPicker->iMinYear;
		*pMonth = pPicker->iMinMonth;
		*pDay = pPicker->iMinDay;
	}
	if ( pPicker->bHasMax && __xgeXuiDatePickerCompare(*pYear, *pMonth, *pDay, pPicker->iMaxYear, pPicker->iMaxMonth, pPicker->iMaxDay) > 0 ) {
		*pYear = pPicker->iMaxYear;
		*pMonth = pPicker->iMaxMonth;
		*pDay = pPicker->iMaxDay;
	}
	__xgeXuiDatePickerNormalizeDate(pYear, pMonth, pDay);
}

static int __xgeXuiDatePickerIsDayEnabled(xge_xui_date_picker pPicker, int iYear, int iMonth, int iDay)
{
	if ( pPicker == NULL ) {
		return 0;
	}
	if ( pPicker->bHasMin && __xgeXuiDatePickerCompare(iYear, iMonth, iDay, pPicker->iMinYear, pPicker->iMinMonth, pPicker->iMinDay) < 0 ) {
		return 0;
	}
	if ( pPicker->bHasMax && __xgeXuiDatePickerCompare(iYear, iMonth, iDay, pPicker->iMaxYear, pPicker->iMaxMonth, pPicker->iMaxDay) > 0 ) {
		return 0;
	}
	return 1;
}

static int __xgeXuiDatePickerFirstEnabledDay(xge_xui_date_picker pPicker)
{
	int iDay;
	int iDays;

	if ( pPicker == NULL ) {
		return -1;
	}
	iDays = __xgeXuiDatePickerDaysInMonth(pPicker->iViewYear, pPicker->iViewMonth);
	for ( iDay = 1; iDay <= iDays; iDay++ ) {
		if ( __xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay) ) {
			return iDay;
		}
	}
	return -1;
}

static void __xgeXuiDatePickerSyncActiveDay(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->iYear == pPicker->iViewYear && pPicker->iMonth == pPicker->iViewMonth && __xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, pPicker->iDay) ) {
		pPicker->iActiveDay = pPicker->iDay;
	} else {
		pPicker->iActiveDay = __xgeXuiDatePickerFirstEnabledDay(pPicker);
	}
}

static int __xgeXuiDatePickerDayOfWeek(int iYear, int iMonth, int iDay)
{
	static const int arrOffset[12] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

	__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
	if ( iMonth < 3 ) {
		iYear--;
	}
	return (iYear + iYear / 4 - iYear / 100 + iYear / 400 + arrOffset[iMonth - 1] + iDay) % 7;
}

static int __xgeXuiDatePickerCellDay(xge_xui_date_picker pPicker, int iCell)
{
	int iFirst;
	int iDay;
	int iDays;

	if ( (pPicker == NULL) || (iCell < 0) || (iCell >= 42) ) {
		return -1;
	}
	iFirst = __xgeXuiDatePickerDayOfWeek(pPicker->iViewYear, pPicker->iViewMonth, 1);
	iDay = iCell - iFirst + 1;
	iDays = __xgeXuiDatePickerDaysInMonth(pPicker->iViewYear, pPicker->iViewMonth);
	if ( iDay < 1 || iDay > iDays ) {
		return -1;
	}
	return iDay;
}

static void __xgeXuiDatePickerLayout(xge_xui_date_picker pPicker)
{
	xge_rect_t tRect;
	float fPad;
	float fHeaderH;
	float fWeekH;
	float fGridX;
	float fGridY;
	float fCellW;
	float fCellH;
	int i;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) ) {
		return;
	}
	tRect = pPicker->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pWidget->tRect;
	}
	fPad = 6.0f;
	fHeaderH = 30.0f;
	fWeekH = 20.0f;
	pPicker->tHeaderRect = (xge_rect_t){ tRect.fX + fPad, tRect.fY + fPad, tRect.fW - fPad * 2.0f, fHeaderH };
	if ( pPicker->tHeaderRect.fW < 1.0f ) {
		pPicker->tHeaderRect.fW = 1.0f;
	}
	pPicker->tPrevRect = (xge_rect_t){ pPicker->tHeaderRect.fX, pPicker->tHeaderRect.fY, 28.0f, pPicker->tHeaderRect.fH };
	pPicker->tNextRect = (xge_rect_t){ pPicker->tHeaderRect.fX + pPicker->tHeaderRect.fW - 28.0f, pPicker->tHeaderRect.fY, 28.0f, pPicker->tHeaderRect.fH };
	pPicker->tMonthRect = (xge_rect_t){ pPicker->tHeaderRect.fX + 30.0f, pPicker->tHeaderRect.fY, pPicker->tHeaderRect.fW - 60.0f, pPicker->tHeaderRect.fH };
	if ( pPicker->tMonthRect.fW < 1.0f ) {
		pPicker->tMonthRect.fW = 1.0f;
	}
	fGridX = tRect.fX + fPad;
	fGridY = tRect.fY + fPad + fHeaderH + fWeekH;
	fCellW = (tRect.fW - fPad * 2.0f) / 7.0f;
	fCellH = (tRect.fH - fPad * 2.0f - fHeaderH - fWeekH) / 6.0f;
	if ( fCellW < 1.0f ) {
		fCellW = 1.0f;
	}
	if ( fCellH < 1.0f ) {
		fCellH = 1.0f;
	}
	for ( i = 0; i < 42; i++ ) {
		pPicker->arrDayRect[i] = (xge_rect_t){ fGridX + (float)(i % 7) * fCellW, fGridY + (float)(i / 7) * fCellH, fCellW, fCellH };
	}
}

static int __xgeXuiDatePickerHitDay(xge_xui_date_picker pPicker, float fX, float fY)
{
	int i;
	int iDay;

	if ( pPicker == NULL ) {
		return -1;
	}
	for ( i = 0; i < 42; i++ ) {
		if ( __xgeXuiRectContains(pPicker->arrDayRect[i], fX, fY) ) {
			iDay = __xgeXuiDatePickerCellDay(pPicker, i);
			if ( iDay > 0 && __xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay) ) {
				return iDay;
			}
			return -1;
		}
	}
	return -1;
}

static void __xgeXuiDatePickerSetDateInternal(xge_xui_date_picker pPicker, int iYear, int iMonth, int iDay, int bSyncView, int bNotify)
{
	int bChanged;

	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerClampDate(pPicker, &iYear, &iMonth, &iDay);
	bChanged = (pPicker->iYear != iYear) || (pPicker->iMonth != iMonth) || (pPicker->iDay != iDay);
	pPicker->iYear = iYear;
	pPicker->iMonth = iMonth;
	pPicker->iDay = iDay;
	if ( bSyncView ) {
		pPicker->iViewYear = iYear;
		pPicker->iViewMonth = iMonth;
	}
	__xgeXuiDatePickerSyncActiveDay(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
	if ( bChanged ) {
		pPicker->iChangeCount++;
		if ( bNotify && pPicker->procChange != NULL ) {
			pPicker->procChange(pPicker->pWidget, pPicker->iYear, pPicker->iMonth, pPicker->iDay, pPicker->pUser);
		}
	}
}

static void __xgeXuiDatePickerSetViewMonth(xge_xui_date_picker pPicker, int iYear, int iMonth)
{
	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
	pPicker->iViewYear = iYear;
	pPicker->iViewMonth = iMonth;
	__xgeXuiDatePickerSyncActiveDay(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiDatePickerMoveSelectedDays(xge_xui_date_picker pPicker, int iDelta)
{
	int iYear;
	int iMonth;
	int iDay;
	int iStep;
	int iCount;

	if ( pPicker == NULL || iDelta == 0 ) {
		return;
	}
	iYear = pPicker->iYear;
	iMonth = pPicker->iMonth;
	iDay = pPicker->iDay;
	iStep = (iDelta > 0) ? 1 : -1;
	iCount = (iDelta > 0) ? iDelta : -iDelta;
	while ( iCount > 0 ) {
		iDay += iStep;
		if ( iDay < 1 ) {
			iMonth--;
			__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
			iDay = __xgeXuiDatePickerDaysInMonth(iYear, iMonth);
		} else if ( iDay > __xgeXuiDatePickerDaysInMonth(iYear, iMonth) ) {
			iDay = 1;
			iMonth++;
			__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
		}
		iCount--;
	}
	__xgeXuiDatePickerSetDateInternal(pPicker, iYear, iMonth, iDay, 1, 1);
}

static void __xgeXuiDatePickerMoveSelectedMonths(xge_xui_date_picker pPicker, int iDelta)
{
	int iYear;
	int iMonth;
	int iDay;

	if ( pPicker == NULL || iDelta == 0 ) {
		return;
	}
	iYear = pPicker->iYear;
	iMonth = pPicker->iMonth + iDelta;
	iDay = pPicker->iDay;
	__xgeXuiDatePickerNormalizeMonth(&iYear, &iMonth);
	__xgeXuiDatePickerNormalizeDate(&iYear, &iMonth, &iDay);
	__xgeXuiDatePickerSetDateInternal(pPicker, iYear, iMonth, iDay, 1, 1);
}

static void __xgeXuiDatePickerSelectViewDay(xge_xui_date_picker pPicker, int iDay, int bNotify)
{
	if ( (pPicker == NULL) || (iDay < 1) ) {
		return;
	}
	if ( !__xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay) ) {
		return;
	}
	__xgeXuiDatePickerSetDateInternal(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay, 0, bNotify);
}

int xgeXuiDatePickerInit(xge_xui_date_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;

	if ( (pPicker == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPicker, 0, sizeof(*pPicker));
	pTheme = xgeXuiGetTheme(pContext);
	__xgeXuiControlWidgetInit(pWidget, 1);
	pPicker->pContext = pContext;
	pPicker->pWidget = pWidget;
	pPicker->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pPicker->iHoverDay = -1;
	pPicker->iActiveDay = -1;
	xgeXuiWidgetSetBackground(pWidget, XGE_COLOR_RGBA(236, 246, 253, 255));
	pPicker->iPanelColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPicker->iHeaderColor = XGE_COLOR_RGBA(223, 241, 252, 255);
	pPicker->iGridColor = XGE_COLOR_RGBA(184, 223, 245, 255);
	pPicker->iTextColor = XGE_COLOR_RGBA(31, 58, 82, 255);
	pPicker->iMutedTextColor = XGE_COLOR_RGBA(92, 120, 139, 255);
	pPicker->iSelectedColor = XGE_COLOR_RGBA(53, 154, 214, 255);
	pPicker->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	pPicker->iFocusColor = XGE_COLOR_RGBA(127, 196, 229, 255);
	pPicker->iDisabledTextColor = XGE_COLOR_RGBA(150, 164, 174, 255);
	xgeXuiWidgetSetClip(pWidget, 1);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiDatePickerEventProc, NULL);
	pWidget->procPaint = xgeXuiDatePickerPaintProc;
	pWidget->pUser = pPicker;
	__xgeXuiDatePickerSetDateInternal(pPicker, 1970, 1, 1, 1, 0);
	__xgeXuiDatePickerLayout(pPicker);
	xgeXuiWidgetMarkPaint(pWidget);
	return XGE_OK;
}

void xgeXuiDatePickerUnit(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->pWidget != NULL && pPicker->pWidget->pUser == pPicker ) {
		xgeXuiWidgetSetImeMode(pPicker->pWidget, XGE_XUI_IME_DISABLED);
		pPicker->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPicker->pWidget, NULL, NULL);
		pPicker->pWidget->procPaint = NULL;
	}
	memset(pPicker, 0, sizeof(*pPicker));
}

void xgeXuiDatePickerSetChange(xge_xui_date_picker pPicker, xge_xui_date_proc procChange, void* pUser)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->procChange = procChange;
	pPicker->pUser = pUser;
}

void xgeXuiDatePickerSetDate(xge_xui_date_picker pPicker, int iYear, int iMonth, int iDay)
{
	__xgeXuiDatePickerSetDateInternal(pPicker, iYear, iMonth, iDay, 1, 0);
}

void xgeXuiDatePickerGetDate(xge_xui_date_picker pPicker, int* pYear, int* pMonth, int* pDay)
{
	if ( pPicker == NULL ) {
		if ( pYear != NULL ) {
			*pYear = 0;
		}
		if ( pMonth != NULL ) {
			*pMonth = 0;
		}
		if ( pDay != NULL ) {
			*pDay = 0;
		}
		return;
	}
	if ( pYear != NULL ) {
		*pYear = pPicker->iYear;
	}
	if ( pMonth != NULL ) {
		*pMonth = pPicker->iMonth;
	}
	if ( pDay != NULL ) {
		*pDay = pPicker->iDay;
	}
}

void xgeXuiDatePickerSetRange(xge_xui_date_picker pPicker, int iMinYear, int iMinMonth, int iMinDay, int iMaxYear, int iMaxMonth, int iMaxDay)
{
	int iSwapYear;
	int iSwapMonth;
	int iSwapDay;

	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerNormalizeDate(&iMinYear, &iMinMonth, &iMinDay);
	__xgeXuiDatePickerNormalizeDate(&iMaxYear, &iMaxMonth, &iMaxDay);
	if ( __xgeXuiDatePickerCompare(iMaxYear, iMaxMonth, iMaxDay, iMinYear, iMinMonth, iMinDay) < 0 ) {
		iSwapYear = iMinYear;
		iSwapMonth = iMinMonth;
		iSwapDay = iMinDay;
		iMinYear = iMaxYear;
		iMinMonth = iMaxMonth;
		iMinDay = iMaxDay;
		iMaxYear = iSwapYear;
		iMaxMonth = iSwapMonth;
		iMaxDay = iSwapDay;
	}
	pPicker->iMinYear = iMinYear;
	pPicker->iMinMonth = iMinMonth;
	pPicker->iMinDay = iMinDay;
	pPicker->iMaxYear = iMaxYear;
	pPicker->iMaxMonth = iMaxMonth;
	pPicker->iMaxDay = iMaxDay;
	pPicker->bHasMin = 1;
	pPicker->bHasMax = 1;
	__xgeXuiDatePickerSetDateInternal(pPicker, pPicker->iYear, pPicker->iMonth, pPicker->iDay, 1, 0);
}

void xgeXuiDatePickerClearRange(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->bHasMin = 0;
	pPicker->bHasMax = 0;
	__xgeXuiDatePickerSyncActiveDay(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerSetMonth(xge_xui_date_picker pPicker, int iYear, int iMonth)
{
	__xgeXuiDatePickerSetViewMonth(pPicker, iYear, iMonth);
}

void xgeXuiDatePickerGetMonth(xge_xui_date_picker pPicker, int* pYear, int* pMonth)
{
	if ( pPicker == NULL ) {
		if ( pYear != NULL ) {
			*pYear = 0;
		}
		if ( pMonth != NULL ) {
			*pMonth = 0;
		}
		return;
	}
	if ( pYear != NULL ) {
		*pYear = pPicker->iViewYear;
	}
	if ( pMonth != NULL ) {
		*pMonth = pPicker->iViewMonth;
	}
}

void xgeXuiDatePickerSetColors(xge_xui_date_picker pPicker, uint32_t iBackground, uint32_t iPanel, uint32_t iHeader, uint32_t iGrid, uint32_t iText, uint32_t iSelected)
{
	if ( pPicker == NULL ) {
		return;
	}
	xgeXuiWidgetSetBackground(pPicker->pWidget, iBackground);
	pPicker->iPanelColor = iPanel;
	pPicker->iHeaderColor = iHeader;
	pPicker->iGridColor = iGrid;
	pPicker->iTextColor = iText;
	pPicker->iSelectedColor = iSelected;
	pPicker->iHoverColor = XGE_COLOR_RGBA(222, 239, 254, 255);
	pPicker->iFocusColor = XGE_COLOR_RGBA(127, 196, 229, 255);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

int xgeXuiDatePickerEvent(xge_xui_date_picker pPicker, const xge_event_t* pEvent)
{
	int iHoverDay;
	int iDays;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiDatePickerLayout(pPicker);
	switch ( pEvent->iType ) {
		case XGE_EVENT_XUI_FOCUS_IN:
			__xgeXuiDatePickerSyncActiveDay(pPicker);
			xgeXuiWidgetMarkPaint(pPicker->pWidget);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_OUT:
			xgeXuiWidgetMarkPaint(pPicker->pWidget);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_MOVE:
			iHoverDay = __xgeXuiDatePickerHitDay(pPicker, pEvent->fX, pEvent->fY);
			if ( pPicker->iHoverDay != iHoverDay ) {
				pPicker->iHoverDay = iHoverDay;
				xgeXuiWidgetMarkPaint(pPicker->pWidget);
			}
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( !__xgeXuiRectContains(pPicker->pWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pWidget);
			if ( __xgeXuiRectContains(pPicker->tPrevRect, pEvent->fX, pEvent->fY) ) {
				__xgeXuiDatePickerSetViewMonth(pPicker, pPicker->iViewYear, pPicker->iViewMonth - 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( __xgeXuiRectContains(pPicker->tNextRect, pEvent->fX, pEvent->fY) ) {
				__xgeXuiDatePickerSetViewMonth(pPicker, pPicker->iViewYear, pPicker->iViewMonth + 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			iHoverDay = __xgeXuiDatePickerHitDay(pPicker, pEvent->fX, pEvent->fY);
			if ( iHoverDay > 0 ) {
				pPicker->iActiveDay = iHoverDay;
				__xgeXuiDatePickerSelectViewDay(pPicker, iHoverDay, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pPicker->pContext == NULL) || (pPicker->pContext->pFocus != pPicker->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( pEvent->iParam1 == XGE_KEY_LEFT ) {
				__xgeXuiDatePickerMoveSelectedDays(pPicker, -1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_RIGHT ) {
				__xgeXuiDatePickerMoveSelectedDays(pPicker, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_UP ) {
				__xgeXuiDatePickerMoveSelectedDays(pPicker, -7);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_DOWN ) {
				__xgeXuiDatePickerMoveSelectedDays(pPicker, 7);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_UP ) {
				__xgeXuiDatePickerMoveSelectedMonths(pPicker, -1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_PAGE_DOWN ) {
				__xgeXuiDatePickerMoveSelectedMonths(pPicker, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_HOME ) {
				__xgeXuiDatePickerSelectViewDay(pPicker, 1, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_END ) {
				iDays = __xgeXuiDatePickerDaysInMonth(pPicker->iViewYear, pPicker->iViewMonth);
				__xgeXuiDatePickerSelectViewDay(pPicker, iDays, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_ENTER || pEvent->iParam1 == XGE_KEY_SPACE ) {
				__xgeXuiDatePickerSelectViewDay(pPicker, pPicker->iActiveDay, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiDatePickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	return xgeXuiDatePickerEvent((xge_xui_date_picker)pUser, pEvent);
}

void xgeXuiDatePickerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	static const char* arrWeek[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	xge_xui_date_picker pPicker;
	xge_rect_t tRect;
	xge_rect_t tWeek;
	xge_rect_t tText;
	char sText[32];
	int i;
	int iDay;
	int bSelected;
	int bEnabled;
	uint32_t iTextColor;

	pPicker = (xge_xui_date_picker)pUser;
	if ( (pWidget == NULL) || (pPicker == NULL) ) {
		return;
	}
	__xgeXuiDatePickerLayout(pPicker);
	tRect = pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pWidget->tRect;
	}
	__xgeXuiHostDrawRect(tRect, pPicker->iPanelColor);
	__xgeXuiHostDrawBorderRect(pWidget->tRect, 1.0f, pPicker->iGridColor);
	__xgeXuiHostDrawRect(pPicker->tHeaderRect, pPicker->iHeaderColor);
	__xgeXuiHostDrawBorderRect(pPicker->tHeaderRect, 1.0f, pPicker->iGridColor);
	__xgeXuiHostDrawBorderRect(pPicker->tPrevRect, 1.0f, pPicker->iGridColor);
	__xgeXuiHostDrawBorderRect(pPicker->tNextRect, 1.0f, pPicker->iGridColor);
	if ( pPicker->pFont != NULL ) {
		snprintf(sText, sizeof(sText), "%04d-%02d", pPicker->iViewYear, pPicker->iViewMonth);
		sText[sizeof(sText) - 1] = 0;
		__xgeXuiHostDrawTextRect(pPicker->pFont, "<", pPicker->tPrevRect, pPicker->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		__xgeXuiHostDrawTextRect(pPicker->pFont, ">", pPicker->tNextRect, pPicker->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		__xgeXuiHostDrawTextRect(pPicker->pFont, sText, pPicker->tMonthRect, pPicker->iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		tWeek = (xge_rect_t){ pPicker->arrDayRect[0].fX, pPicker->tHeaderRect.fY + pPicker->tHeaderRect.fH, pPicker->arrDayRect[0].fW, pPicker->arrDayRect[0].fY - (pPicker->tHeaderRect.fY + pPicker->tHeaderRect.fH) };
		for ( i = 0; i < 7; i++ ) {
			tWeek.fX = pPicker->arrDayRect[i].fX;
			tWeek.fW = pPicker->arrDayRect[i].fW;
			__xgeXuiHostDrawTextRect(pPicker->pFont, arrWeek[i], tWeek, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	for ( i = 0; i < 42; i++ ) {
		iDay = __xgeXuiDatePickerCellDay(pPicker, i);
		if ( iDay > 0 ) {
			bSelected = (pPicker->iYear == pPicker->iViewYear && pPicker->iMonth == pPicker->iViewMonth && pPicker->iDay == iDay);
			bEnabled = __xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay);
			if ( bSelected ) {
				__xgeXuiHostDrawRect(pPicker->arrDayRect[i], pPicker->iSelectedColor);
			} else if ( bEnabled && pPicker->iHoverDay == iDay ) {
				__xgeXuiHostDrawRect(pPicker->arrDayRect[i], pPicker->iHoverColor);
			}
		}
		__xgeXuiHostDrawBorderRect(pPicker->arrDayRect[i], 1.0f, pPicker->iGridColor);
		if ( pPicker->pFont != NULL && iDay > 0 ) {
			bSelected = (pPicker->iYear == pPicker->iViewYear && pPicker->iMonth == pPicker->iViewMonth && pPicker->iDay == iDay);
			bEnabled = __xgeXuiDatePickerIsDayEnabled(pPicker, pPicker->iViewYear, pPicker->iViewMonth, iDay);
			iTextColor = bEnabled ? (bSelected ? XGE_COLOR_RGBA(255, 255, 255, 255) : pPicker->iTextColor) : pPicker->iDisabledTextColor;
			snprintf(sText, sizeof(sText), "%d", iDay);
			sText[sizeof(sText) - 1] = 0;
			tText = pPicker->arrDayRect[i];
			__xgeXuiHostDrawTextRect(pPicker->pFont, sText, tText, iTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
		if ( (pPicker->pContext != NULL) && (pPicker->pContext->pFocus == pWidget) && iDay > 0 && pPicker->iActiveDay == iDay ) {
			__xgeXuiHostDrawBorderRect(pPicker->arrDayRect[i], 2.0f, pPicker->iFocusColor);
		}
	}
}
