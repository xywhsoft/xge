#define XGE_XUI_DATE_PICKER_PART_NONE 0
#define XGE_XUI_DATE_PICKER_PART_DAY 20

#define XGE_XUI_DATE_PICKER_POPUP_COUNT 6
#define XGE_XUI_DATE_PICKER_CONTROL_PREV 1
#define XGE_XUI_DATE_PICKER_CONTROL_NEXT 2
#define XGE_XUI_DATE_PICKER_CONTROL_YEAR 3
#define XGE_XUI_DATE_PICKER_CONTROL_MONTH 4
#define XGE_XUI_DATE_PICKER_CONTROL_TIME 5
#define XGE_XUI_DATE_PICKER_CONTROL_TODAY 6
#define XGE_XUI_DATE_PICKER_CONTROL_CLEAR 7
#define XGE_XUI_DATE_PICKER_CONTROL_CANCEL 8
#define XGE_XUI_DATE_PICKER_CONTROL_OK 9

typedef struct xge_xui_date_picker_popup_panel_t xge_xui_date_picker_popup_panel_t;
typedef struct xge_xui_date_picker_popup_host_t xge_xui_date_picker_popup_host_t;

typedef struct xge_xui_date_picker_control_ref_t {
	xge_xui_date_picker_popup_panel_t* pPanel;
	int iKind;
	int iPanel;
	int iIndex;
} xge_xui_date_picker_control_ref_t;

struct xge_xui_date_picker_popup_panel_t {
	xge_xui_date_picker_popup_host_t* pHost;
	int iMode;
	xge_xui_date_picker pPicker;
	xge_xui_popup_t tPopup;
	xge_xui_widget pPopupWidget;
	xge_xui_widget pContentWidget;
	xge_xui_widget arrPrevWidget[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_widget arrNextWidget[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_widget arrYearWidget[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_widget arrMonthWidget[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_widget arrTimeWidget[XGE_XUI_DATE_PICKER_PANEL_CAPACITY][3];
	xge_xui_widget pTodayWidget;
	xge_xui_widget pClearWidget;
	xge_xui_widget pCancelWidget;
	xge_xui_widget pOkWidget;
	xge_xui_button_t arrPrevButton[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_button_t arrNextButton[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_input_t arrYearInput[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_combo_box_t arrMonthCombo[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_numeric_input_t arrTimeInput[XGE_XUI_DATE_PICKER_PANEL_CAPACITY][3];
	xge_xui_button_t tTodayButton;
	xge_xui_button_t tClearButton;
	xge_xui_button_t tCancelButton;
	xge_xui_button_t tOkButton;
	xge_xui_date_picker_control_ref_t arrPrevRef[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_date_picker_control_ref_t arrNextRef[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_date_picker_control_ref_t arrYearRef[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_date_picker_control_ref_t arrMonthRef[XGE_XUI_DATE_PICKER_PANEL_CAPACITY];
	xge_xui_date_picker_control_ref_t arrTimeRef[XGE_XUI_DATE_PICKER_PANEL_CAPACITY][3];
	xge_xui_date_picker_control_ref_t tTodayRef;
	xge_xui_date_picker_control_ref_t tClearRef;
	xge_xui_date_picker_control_ref_t tCancelRef;
	xge_xui_date_picker_control_ref_t tOkRef;
	int bSyncing;
};

struct xge_xui_date_picker_popup_host_t {
	xge_xui_context pContext;
	int iRefCount;
	xge_xui_date_picker_popup_panel_t arrPanel[XGE_XUI_DATE_PICKER_POPUP_COUNT];
	xge_xui_date_picker_popup_host_t* pNext;
};

static const char* g_arrXgeXuiDatePickerMonthText[12] = {
	"01", "02", "03", "04", "05", "06",
	"07", "08", "09", "10", "11", "12"
};

static xge_xui_date_picker_popup_host_t* g_pXgeXuiDatePickerHostList = NULL;

static xge_xui_date_picker_popup_host_t* __xgeXuiDatePickerHostRetain(xge_xui_context pContext);
static xge_xui_date_picker_popup_host_t* __xgeXuiDatePickerHostFind(xge_xui_context pContext);
static void __xgeXuiDatePickerHostRelease(xge_xui_context pContext);
static void __xgeXuiDatePickerHostCloseAll(xge_xui_date_picker_popup_host_t* pHost, xge_xui_date_picker pExcept);
static xge_xui_date_picker_popup_panel_t* __xgeXuiDatePickerPanelForPicker(xge_xui_date_picker pPicker);
static xge_xui_date_picker_popup_panel_t* __xgeXuiDatePickerPanelForMode(xge_xui_date_picker pPicker);
static void __xgeXuiDatePickerPanelLayoutControls(xge_xui_date_picker_popup_panel_t* pPanel);
static void __xgeXuiDatePickerPanelSyncControls(xge_xui_date_picker_popup_panel_t* pPanel);
static void __xgeXuiDatePickerPanelClosed(xge_xui_date_picker_popup_panel_t* pPanel, int iClosingMode);
static void __xgeXuiDatePickerCloseActivePopup(xge_xui_date_picker pPicker, int iClosingMode);

static int __xgeXuiDatePickerClampMode(int iMode)
{
	if ( (iMode < XGE_XUI_DATE_PICKER_MODE_DATE) || (iMode > XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE) ) {
		return XGE_XUI_DATE_PICKER_MODE_DATE;
	}
	return iMode;
}

static int __xgeXuiDatePickerIsRangeMode(int iMode)
{
	return (iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE) || (iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xgeXuiDatePickerHasDatePanel(int iMode)
{
	return (iMode == XGE_XUI_DATE_PICKER_MODE_DATE) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xgeXuiDatePickerHasTimePanel(int iMode)
{
	return (iMode == XGE_XUI_DATE_PICKER_MODE_TIME) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME) || (iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xgeXuiDatePickerPanelCount(int iMode)
{
	return __xgeXuiDatePickerIsRangeMode(iMode) ? 2 : 1;
}

static const char* __xgeXuiDatePickerDefaultFormat(int iMode, int bShowSecond)
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

static int __xgeXuiDatePickerFormatIsDefault(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return 1;
	}
	return strcmp(pPicker->sFormat, __xgeXuiDatePickerDefaultFormat(pPicker->iMode, pPicker->bShowSecond)) == 0;
}

static void __xgeXuiDatePickerCopyText(char* sDst, int iDstSize, const char* sSrc)
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

static xtime __xgeXuiDatePickerMakeTime(int iHour, int iMinute, int iSecond, int bShowSecond)
{
	if ( iHour < 0 ) {
		iHour = 0;
	}
	if ( iHour > 23 ) {
		iHour = 23;
	}
	if ( iMinute < 0 ) {
		iMinute = 0;
	}
	if ( iMinute > 59 ) {
		iMinute = 59;
	}
	if ( !bShowSecond ) {
		iSecond = 0;
	}
	if ( iSecond < 0 ) {
		iSecond = 0;
	}
	if ( iSecond > 59 ) {
		iSecond = 59;
	}
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
}

static xtime __xgeXuiDatePickerNormalizeTimePart(xge_xui_date_picker pPicker, xtime tTime)
{
	int iHour;
	int iMinute;
	int iSecond;

	xrtDecodeSerial(tTime, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	return __xgeXuiDatePickerMakeTime(iHour, iMinute, iSecond, (pPicker != NULL) ? pPicker->bShowSecond : 1);
}

static xtime __xgeXuiDatePickerCombineDateTime(xge_xui_date_picker pPicker, xtime tDate, xtime tTime)
{
	return xrtDatePart(tDate) + __xgeXuiDatePickerNormalizeTimePart(pPicker, tTime);
}

static xtime __xgeXuiDatePickerNormalizeValue(xge_xui_date_picker pPicker, xtime tValue)
{
	int iMode;

	if ( pPicker == NULL ) {
		return tValue;
	}
	iMode = pPicker->iMode;
	if ( (iMode == XGE_XUI_DATE_PICKER_MODE_DATE) || (iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE) ) {
		return xrtDatePart(tValue);
	}
	if ( (iMode == XGE_XUI_DATE_PICKER_MODE_TIME) || (iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
		return __xgeXuiDatePickerNormalizeTimePart(pPicker, tValue);
	}
	return xrtDatePart(tValue) + __xgeXuiDatePickerNormalizeTimePart(pPicker, tValue);
}

static xtime __xgeXuiDatePickerLimitValue(xge_xui_date_picker pPicker, xtime tValue)
{
	if ( (pPicker == NULL) || ((pPicker->iMode != XGE_XUI_DATE_PICKER_MODE_TIME) && (pPicker->iMode != XGE_XUI_DATE_PICKER_MODE_TIME_RANGE)) ) {
		return tValue;
	}
	return xrtTimePart(tValue);
}

static xtime __xgeXuiDatePickerClampValue(xge_xui_date_picker pPicker, xtime tValue)
{
	xtime tCompare;
	xtime tMin;
	xtime tMax;

	if ( pPicker == NULL ) {
		return tValue;
	}
	tValue = __xgeXuiDatePickerNormalizeValue(pPicker, tValue);
	tCompare = __xgeXuiDatePickerLimitValue(pPicker, tValue);
	if ( pPicker->bHasMin ) {
		tMin = __xgeXuiDatePickerLimitValue(pPicker, pPicker->tMin);
		if ( tCompare < tMin ) {
			if ( (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME) || (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
				tValue = tMin;
			} else {
				tValue = __xgeXuiDatePickerNormalizeValue(pPicker, pPicker->tMin);
			}
			tCompare = __xgeXuiDatePickerLimitValue(pPicker, tValue);
		}
	}
	if ( pPicker->bHasMax ) {
		tMax = __xgeXuiDatePickerLimitValue(pPicker, pPicker->tMax);
		if ( tCompare > tMax ) {
			if ( (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME) || (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
				tValue = tMax;
			} else {
				tValue = __xgeXuiDatePickerNormalizeValue(pPicker, pPicker->tMax);
			}
		}
	}
	return __xgeXuiDatePickerNormalizeValue(pPicker, tValue);
}

static int __xgeXuiDatePickerValueEnabled(xge_xui_date_picker pPicker, xtime tValue)
{
	xtime tCompare;

	if ( pPicker == NULL ) {
		return 0;
	}
	tValue = __xgeXuiDatePickerNormalizeValue(pPicker, tValue);
	tCompare = __xgeXuiDatePickerLimitValue(pPicker, tValue);
	if ( pPicker->bHasMin && tCompare < __xgeXuiDatePickerLimitValue(pPicker, pPicker->tMin) ) {
		return 0;
	}
	if ( pPicker->bHasMax && tCompare > __xgeXuiDatePickerLimitValue(pPicker, pPicker->tMax) ) {
		return 0;
	}
	return 1;
}

static xtime __xgeXuiDatePickerDefaultSpan(xge_xui_date_picker pPicker)
{
	if ( (pPicker != NULL) && (pPicker->tDefaultRangeSpan > 0) ) {
		return pPicker->tDefaultRangeSpan;
	}
	if ( (pPicker != NULL) && (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE) ) {
		return XRT_TIME_DAY;
	}
	return XRT_TIME_HOUR * 4;
}

static xtime __xgeXuiDatePickerAddSpan(xge_xui_date_picker pPicker, xtime tValue, xtime tSpan)
{
	xtime tEnd;

	if ( pPicker == NULL ) {
		return tValue + tSpan;
	}
	if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE ) {
		return xrtDatePart(tValue + ((tSpan > 0) ? tSpan : XRT_TIME_DAY));
	}
	if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		tEnd = xrtTimePart(tValue) + ((tSpan > 0) ? tSpan : (XRT_TIME_HOUR * 4));
		if ( tEnd >= XRT_TIME_DAY ) {
			tEnd = XRT_TIME_DAY - 1;
		}
		return __xgeXuiDatePickerNormalizeValue(pPicker, tEnd);
	}
	return __xgeXuiDatePickerNormalizeValue(pPicker, tValue + ((tSpan > 0) ? tSpan : (XRT_TIME_HOUR * 4)));
}

static void __xgeXuiDatePickerGetDefaultValues(xge_xui_date_picker pPicker, xtime* pStart, xtime* pEnd)
{
	xtime tNow;
	xtime tStart;
	xtime tEnd;

	tNow = xrtNow();
	if ( pPicker == NULL ) {
		if ( pStart != NULL ) {
			*pStart = tNow;
		}
		if ( pEnd != NULL ) {
			*pEnd = tNow;
		}
		return;
	}
	switch ( pPicker->iMode ) {
		case XGE_XUI_DATE_PICKER_MODE_DATE:
			tStart = xrtDatePart(tNow);
			tEnd = tStart;
			break;
		case XGE_XUI_DATE_PICKER_MODE_TIME:
			tStart = __xgeXuiDatePickerNormalizeValue(pPicker, xrtTimePart(tNow));
			tEnd = tStart;
			break;
		case XGE_XUI_DATE_PICKER_MODE_DATE_RANGE:
			tStart = xrtDatePart(tNow);
			tEnd = __xgeXuiDatePickerAddSpan(pPicker, tStart, __xgeXuiDatePickerDefaultSpan(pPicker));
			break;
		case XGE_XUI_DATE_PICKER_MODE_TIME_RANGE:
			tStart = __xgeXuiDatePickerNormalizeValue(pPicker, xrtTimePart(tNow));
			tEnd = __xgeXuiDatePickerAddSpan(pPicker, tStart, __xgeXuiDatePickerDefaultSpan(pPicker));
			break;
		case XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE:
			tStart = __xgeXuiDatePickerNormalizeValue(pPicker, tNow);
			tEnd = __xgeXuiDatePickerAddSpan(pPicker, tStart, __xgeXuiDatePickerDefaultSpan(pPicker));
			break;
		case XGE_XUI_DATE_PICKER_MODE_DATETIME:
		default:
			tStart = __xgeXuiDatePickerNormalizeValue(pPicker, tNow);
			tEnd = tStart;
			break;
	}
	tStart = __xgeXuiDatePickerClampValue(pPicker, tStart);
	tEnd = __xgeXuiDatePickerClampValue(pPicker, tEnd);
	if ( tEnd < tStart && __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		tEnd = tStart;
	}
	if ( pStart != NULL ) {
		*pStart = tStart;
	}
	if ( pEnd != NULL ) {
		*pEnd = tEnd;
	}
}

static void __xgeXuiDatePickerEnsureValue(xge_xui_date_picker pPicker)
{
	xtime tStart;
	xtime tEnd;

	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->bNullable && pPicker->bHasValue == 0 ) {
		return;
	}
	if ( pPicker->bHasValue ) {
		pPicker->tValue = __xgeXuiDatePickerClampValue(pPicker, pPicker->tValue);
		pPicker->tStart = __xgeXuiDatePickerClampValue(pPicker, pPicker->tStart);
		pPicker->tEnd = __xgeXuiDatePickerClampValue(pPicker, pPicker->tEnd);
		if ( pPicker->tEnd < pPicker->tStart && __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
			xtime tSwap = pPicker->tStart;
			pPicker->tStart = pPicker->tEnd;
			pPicker->tEnd = tSwap;
		}
		return;
	}
	__xgeXuiDatePickerGetDefaultValues(pPicker, &tStart, &tEnd);
	pPicker->tValue = tStart;
	pPicker->tStart = tStart;
	pPicker->tEnd = tEnd;
	pPicker->bHasValue = 1;
}

static void __xgeXuiDatePickerInitViewMonth(xge_xui_date_picker pPicker)
{
	int64 iYear;
	int iMonth;

	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerEnsureValue(pPicker);
	if ( __xgeXuiDatePickerHasDatePanel(pPicker->iMode) == 0 ) {
		pPicker->tViewMonth[0] = xrtFirstDayOfMonth(xrtNow());
		pPicker->tViewMonth[1] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, 1, pPicker->tViewMonth[0]);
		return;
	}
	xrtDecodeSerial(__xgeXuiDatePickerIsRangeMode(pPicker->iMode) ? pPicker->tStart : pPicker->tValue, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	pPicker->tViewMonth[0] = xrtDateSerial(iYear, iMonth, 1);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		xrtDecodeSerial(pPicker->tEnd, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		pPicker->tViewMonth[1] = xrtDateSerial(iYear, iMonth, 1);
		if ( pPicker->tViewMonth[1] <= pPicker->tViewMonth[0] ) {
			pPicker->tViewMonth[1] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, 1, pPicker->tViewMonth[0]);
		}
	} else {
		pPicker->tViewMonth[1] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, 1, pPicker->tViewMonth[0]);
	}
}

static void __xgeXuiDatePickerNotifyDraft(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iChangingCount++;
	if ( pPicker->procChanging != NULL ) {
		if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
			pPicker->procChanging(pPicker->pWidget, pPicker->tDraftStart, pPicker->tDraftEnd, pPicker->iMode, pPicker->pChangingUser);
		} else {
			pPicker->procChanging(pPicker->pWidget, pPicker->tDraftValue, 0, pPicker->iMode, pPicker->pChangingUser);
		}
	}
}

static void __xgeXuiDatePickerNotifyValue(xge_xui_date_picker pPicker, xge_xui_date_proc proc, void* pUser)
{
	if ( (pPicker == NULL) || (proc == NULL) ) {
		return;
	}
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		proc(pPicker->pWidget, pPicker->tStart, pPicker->tEnd, pPicker->iMode, pUser);
	} else {
		proc(pPicker->pWidget, pPicker->tValue, 0, pPicker->iMode, pUser);
	}
}

static xge_vec2_t __xgeXuiDatePickerPreferredSize(xge_xui_date_picker pPicker)
{
	switch ( (pPicker != NULL) ? pPicker->iMode : XGE_XUI_DATE_PICKER_MODE_DATE ) {
		case XGE_XUI_DATE_PICKER_MODE_TIME:
			return (xge_vec2_t){ 268.0f, 168.0f };
		case XGE_XUI_DATE_PICKER_MODE_DATETIME:
			return (xge_vec2_t){ 548.0f, 334.0f };
		case XGE_XUI_DATE_PICKER_MODE_DATE_RANGE:
			return (xge_vec2_t){ 628.0f, 326.0f };
		case XGE_XUI_DATE_PICKER_MODE_TIME_RANGE:
			return (xge_vec2_t){ 444.0f, 204.0f };
		case XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE:
			return (xge_vec2_t){ 672.0f, 408.0f };
		case XGE_XUI_DATE_PICKER_MODE_DATE:
		default:
			return (xge_vec2_t){ 318.0f, 326.0f };
	}
}

static void __xgeXuiDatePickerLayoutMain(xge_xui_date_picker pPicker)
{
	xge_rect_t tRect;
	float fButtonW;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) ) {
		return;
	}
	tRect = pPicker->pWidget->tContentRect;
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pWidget->tRect;
	}
	fButtonW = (tRect.fH > 18.0f) ? tRect.fH : 24.0f;
	if ( fButtonW > 32.0f ) {
		fButtonW = 32.0f;
	}
	pPicker->tButtonRect = (xge_rect_t){ tRect.fX + tRect.fW - fButtonW, tRect.fY, fButtonW, tRect.fH };
	pPicker->tFieldRect = tRect;
	pPicker->tFieldRect.fW -= fButtonW;
	if ( pPicker->tFieldRect.fW < 1.0f ) {
		pPicker->tFieldRect.fW = 1.0f;
	}
}

static void __xgeXuiDatePickerLayoutCalendarPanel(xge_xui_date_picker pPicker, int iPanel, xge_rect_t tPanel)
{
	float fHeaderH;
	float fWeekH;
	float fCellW;
	float fCellH;
	float fGridY;
	float fHeaderButtonW;
	float fGroupW;
	float fGroupX;
	int i;

	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) ) {
		return;
	}
	pPicker->tPanelRect[iPanel] = tPanel;
	fHeaderH = 32.0f;
	fWeekH = 22.0f;
	fHeaderButtonW = 28.0f;
	pPicker->tHeaderRect[iPanel] = (xge_rect_t){ tPanel.fX, tPanel.fY, tPanel.fW, fHeaderH };
	pPicker->tPrevRect[iPanel] = (xge_rect_t){ tPanel.fX + 2.0f, tPanel.fY + 2.0f, fHeaderButtonW, fHeaderH - 4.0f };
	pPicker->tNextRect[iPanel] = (xge_rect_t){ tPanel.fX + tPanel.fW - fHeaderButtonW - 2.0f, tPanel.fY + 2.0f, fHeaderButtonW, fHeaderH - 4.0f };
	fGroupW = 72.0f + 8.0f + 88.0f;
	fGroupX = tPanel.fX + (tPanel.fW - fGroupW) * 0.5f;
	if ( fGroupX < pPicker->tPrevRect[iPanel].fX + pPicker->tPrevRect[iPanel].fW + 6.0f ) {
		fGroupX = pPicker->tPrevRect[iPanel].fX + pPicker->tPrevRect[iPanel].fW + 6.0f;
	}
	if ( fGroupX + fGroupW > pPicker->tNextRect[iPanel].fX - 6.0f ) {
		fGroupX = pPicker->tNextRect[iPanel].fX - 6.0f - fGroupW;
	}
	pPicker->tYearRect[iPanel] = (xge_rect_t){ fGroupX, tPanel.fY + 4.0f, 72.0f, fHeaderH - 8.0f };
	pPicker->tMonthRect[iPanel] = (xge_rect_t){ pPicker->tYearRect[iPanel].fX + pPicker->tYearRect[iPanel].fW + 8.0f, tPanel.fY + 4.0f, 88.0f, fHeaderH - 8.0f };
	fGridY = tPanel.fY + fHeaderH + fWeekH;
	fCellW = tPanel.fW / 7.0f;
	fCellH = (tPanel.fH - fHeaderH - fWeekH) / 6.0f;
	if ( fCellH < 20.0f ) {
		fCellH = 20.0f;
	}
	for ( i = 0; i < 42; i++ ) {
		pPicker->arrDayRect[iPanel][i] = (xge_rect_t){ tPanel.fX + (float)(i % 7) * fCellW, fGridY + (float)(i / 7) * fCellH, fCellW, fCellH };
	}
}

static void __xgeXuiDatePickerLayoutTimePanel(xge_xui_date_picker pPicker, int iPanel, xge_rect_t tPanel)
{
	float fFieldW;
	float fGap;
	float fX;
	float fFieldY;
	float fContentW;
	float fContentY;
	float fContentH;
	float fBlockH;
	float fFieldH;
	int bRange;
	int i;

	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) ) {
		return;
	}
	pPicker->tTimePanelRect[iPanel] = tPanel;
	fFieldW = pPicker->bShowSecond ? 58.0f : 66.0f;
	fGap = 10.0f;
	bRange = __xgeXuiDatePickerIsRangeMode(pPicker->iMode);
	fContentW = pPicker->bShowSecond ? (fFieldW * 3.0f + fGap * 2.0f) : (fFieldW * 2.0f + fGap);
	fX = tPanel.fX + (tPanel.fW - fContentW) * 0.5f;
	fFieldH = 28.0f;
	fBlockH = 18.0f + fFieldH;
	fContentY = tPanel.fY + (bRange ? 29.0f : 0.0f);
	fContentH = tPanel.fH - (bRange ? 29.0f : 0.0f);
	if ( fContentH < fBlockH ) {
		fContentH = fBlockH;
	}
	fFieldY = fContentY + (fContentH - fBlockH) * 0.5f + 18.0f;
	if ( fFieldY + fFieldH > tPanel.fY + tPanel.fH - 6.0f ) {
		fFieldY = tPanel.fY + tPanel.fH - 6.0f - fFieldH;
	}
	for ( i = 0; i < 3; i++ ) {
		pPicker->arrTimeRect[iPanel][i] = (xge_rect_t){ 0, 0, 0, 0 };
	}
	for ( i = 0; i < (pPicker->bShowSecond ? 3 : 2); i++ ) {
		pPicker->arrTimeRect[iPanel][i] = (xge_rect_t){ fX, fFieldY, fFieldW, fFieldH };
		fX += fFieldW + fGap;
	}
}

static void __xgeXuiDatePickerLayoutPopup(xge_xui_date_picker pPicker)
{
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_rect_t tRect;
	xge_rect_t tPanel;
	xge_rect_t tFooter;
	float fPad;
	float fFooterH;
	float fButtonW;
	float fButtonH;
	float fGap;
	float fCalendarH;
	float fTimeH;
	float fBodyY;
	float fBodyH;
	float fPanelY;
	float fBlockH;
	int i;

	if ( (pPicker == NULL) || (pPicker->pPopupWidget == NULL) ) {
		return;
	}
	pPanel = __xgeXuiDatePickerPanelForPicker(pPicker);
	if ( pPanel != NULL ) {
		tRect = xgeXuiPopupGetContentRect(&pPanel->tPopup);
	} else {
		tRect = pPicker->pPopupWidget->tContentRect;
	}
	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) {
		tRect = pPicker->pPopupWidget->tRect;
	}
	pPicker->tPopupRect = tRect;
	fPad = 12.0f;
	fFooterH = 42.0f;
	fButtonW = 62.0f;
	fButtonH = 24.0f;
	fGap = 8.0f;
	fBodyY = tRect.fY + fPad;
	fBodyH = tRect.fH - fPad * 2.0f - fFooterH;
	if ( fBodyH < 1.0f ) {
		fBodyH = 1.0f;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		pPicker->tPanelRect[i] = (xge_rect_t){ 0, 0, 0, 0 };
		pPicker->tTimePanelRect[i] = (xge_rect_t){ 0, 0, 0, 0 };
	}
	if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME ) {
		fTimeH = 112.0f;
		if ( fTimeH > fBodyH ) {
			fTimeH = fBodyH;
		}
		fPanelY = fBodyY + (fBodyH - fTimeH) * 0.5f;
		__xgeXuiDatePickerLayoutTimePanel(pPicker, 0, (xge_rect_t){ tRect.fX + fPad, fPanelY, tRect.fW - fPad * 2.0f, fTimeH });
	} else if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		fTimeH = 122.0f;
		if ( fTimeH > fBodyH ) {
			fTimeH = fBodyH;
		}
		fPanelY = fBodyY + (fBodyH - fTimeH) * 0.5f;
		__xgeXuiDatePickerLayoutTimePanel(pPicker, 0, (xge_rect_t){ tRect.fX + fPad, fPanelY, (tRect.fW - fPad * 2.0f - fGap) * 0.5f, fTimeH });
		__xgeXuiDatePickerLayoutTimePanel(pPicker, 1, (xge_rect_t){ tRect.fX + fPad + (tRect.fW - fPad * 2.0f + fGap) * 0.5f, fPanelY, (tRect.fW - fPad * 2.0f - fGap) * 0.5f, fTimeH });
	} else if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME ) {
		fCalendarH = fBodyH;
		tPanel = (xge_rect_t){ tRect.fX + fPad, fBodyY, 286.0f, fCalendarH };
		__xgeXuiDatePickerLayoutCalendarPanel(pPicker, 0, tPanel);
		fTimeH = 116.0f;
		if ( fTimeH > fBodyH ) {
			fTimeH = fBodyH;
		}
		__xgeXuiDatePickerLayoutTimePanel(pPicker, 0, (xge_rect_t){ tPanel.fX + tPanel.fW + 18.0f, fBodyY + (fBodyH - fTimeH) * 0.5f, tRect.fW - fPad * 2.0f - tPanel.fW - 18.0f, fTimeH });
	} else if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE || pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE ) {
		float fPanelW = (tRect.fW - fPad * 2.0f - fGap) * 0.5f;
		fCalendarH = (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE) ? 232.0f : fBodyH;
		fBlockH = fCalendarH;
		if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE ) {
			fTimeH = 96.0f;
			if ( fCalendarH + 8.0f + fTimeH > fBodyH ) {
				fCalendarH = fBodyH - 8.0f - fTimeH;
				if ( fCalendarH < 210.0f ) {
					fCalendarH = 210.0f;
					fTimeH = fBodyH - 8.0f - fCalendarH;
					if ( fTimeH < 82.0f ) {
						fTimeH = 82.0f;
					}
				}
			}
			fBlockH = fCalendarH + 8.0f + fTimeH;
		}
		fPanelY = fBodyY + ((fBodyH > fBlockH) ? (fBodyH - fBlockH) * 0.5f : 0.0f);
		__xgeXuiDatePickerLayoutCalendarPanel(pPicker, 0, (xge_rect_t){ tRect.fX + fPad, fPanelY, fPanelW, fCalendarH });
		__xgeXuiDatePickerLayoutCalendarPanel(pPicker, 1, (xge_rect_t){ tRect.fX + fPad + fPanelW + fGap, fPanelY, fPanelW, fCalendarH });
		if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE ) {
			__xgeXuiDatePickerLayoutTimePanel(pPicker, 0, (xge_rect_t){ tRect.fX + fPad, fPanelY + fCalendarH + 8.0f, fPanelW, fTimeH });
			__xgeXuiDatePickerLayoutTimePanel(pPicker, 1, (xge_rect_t){ tRect.fX + fPad + fPanelW + fGap, fPanelY + fCalendarH + 8.0f, fPanelW, fTimeH });
		}
	} else {
		__xgeXuiDatePickerLayoutCalendarPanel(pPicker, 0, (xge_rect_t){ tRect.fX + fPad, fBodyY, tRect.fW - fPad * 2.0f, fBodyH });
	}
	tFooter = (xge_rect_t){ tRect.fX + fPad, tRect.fY + tRect.fH - fPad - fButtonH, tRect.fW - fPad * 2.0f, fButtonH };
	pPicker->arrFooterRect[3] = (xge_rect_t){ tFooter.fX + tFooter.fW - fButtonW, tFooter.fY, fButtonW, fButtonH };
	pPicker->arrFooterRect[2] = (xge_rect_t){ pPicker->arrFooterRect[3].fX - fGap - fButtonW, tFooter.fY, fButtonW, fButtonH };
	if ( pPicker->bNullable ) {
		pPicker->arrFooterRect[1] = (xge_rect_t){ pPicker->arrFooterRect[2].fX - fGap - fButtonW, tFooter.fY, fButtonW, fButtonH };
	} else {
		pPicker->arrFooterRect[1] = (xge_rect_t){ 0, 0, 0, 0 };
	}
	pPicker->arrFooterRect[0] = (xge_rect_t){ tFooter.fX, tFooter.fY, fButtonW, fButtonH };
}

static int __xgeXuiDatePickerCellDate(xge_xui_date_picker pPicker, int iPanel, int iCell, xtime* pDate)
{
	int64 iYear;
	int iMonth;
	int iDays;
	int iFirst;
	int iDay;
	xtime tFirst;

	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) || (iCell < 0) || (iCell >= 42) ) {
		return 0;
	}
	xrtDecodeSerial(pPicker->tViewMonth[iPanel], &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	tFirst = xrtDateSerial(iYear, iMonth, 1);
	iFirst = (xrtWeekday(tFirst) - pPicker->iFirstDayOfWeek + 7) % 7;
	iDays = xrtDaysInMonth((int)iYear, iMonth);
	iDay = iCell - iFirst + 1;
	if ( iDay < 1 || iDay > iDays ) {
		return 0;
	}
	if ( pDate != NULL ) {
		*pDate = xrtDateSerial(iYear, iMonth, iDay);
	}
	return 1;
}

static int __xgeXuiDatePickerHitPopup(xge_xui_date_picker pPicker, float fX, float fY, int* pPanel, int* pIndex)
{
	int i;
	int j;

	if ( pPanel != NULL ) {
		*pPanel = -1;
	}
	if ( pIndex != NULL ) {
		*pIndex = -1;
	}
	if ( pPicker == NULL ) {
		return XGE_XUI_DATE_PICKER_PART_NONE;
	}
	if ( __xgeXuiDatePickerHasDatePanel(pPicker->iMode) ) {
		for ( i = 0; i < __xgeXuiDatePickerPanelCount(pPicker->iMode); i++ ) {
			for ( j = 0; j < 42; j++ ) {
				if ( __xgeXuiRectContains(pPicker->arrDayRect[i][j], fX, fY) ) {
					if ( pPanel != NULL ) {
						*pPanel = i;
					}
					if ( pIndex != NULL ) {
						*pIndex = j;
					}
					return XGE_XUI_DATE_PICKER_PART_DAY;
				}
			}
		}
	}
	return XGE_XUI_DATE_PICKER_PART_NONE;
}

static void __xgeXuiDatePickerMoveViewMonth(xge_xui_date_picker pPicker, int iPanel, int iDelta)
{
	int64 iYear;
	int iMonth;
	int iIndex;

	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) ) {
		return;
	}
	xrtDecodeSerial(pPicker->tViewMonth[iPanel], &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	iIndex = (int)iYear * 12 + (iMonth - 1) + iDelta;
	iYear = iIndex / 12;
	iMonth = iIndex % 12 + 1;
	if ( iMonth < 1 ) {
		iMonth += 12;
		iYear--;
	}
	pPicker->tViewMonth[iPanel] = xrtDateSerial(iYear, iMonth, 1);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( iPanel == 0 && pPicker->tViewMonth[1] <= pPicker->tViewMonth[0] ) {
			pPicker->tViewMonth[1] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, 1, pPicker->tViewMonth[0]);
		}
		if ( iPanel == 1 && pPicker->tViewMonth[1] <= pPicker->tViewMonth[0] ) {
			pPicker->tViewMonth[0] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, -1, pPicker->tViewMonth[1]);
		}
	}
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiDatePickerSetViewMonthValue(xge_xui_date_picker pPicker, int iPanel, int64 iYear, int iMonth)
{
	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) ) {
		return;
	}
	if ( iYear < 1 ) {
		iYear = 1;
	}
	if ( iYear > 9999 ) {
		iYear = 9999;
	}
	if ( iMonth < 1 ) {
		iMonth = 1;
	}
	if ( iMonth > 12 ) {
		iMonth = 12;
	}
	pPicker->tViewMonth[iPanel] = xrtDateSerial(iYear, iMonth, 1);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( iPanel == 0 && pPicker->tViewMonth[1] <= pPicker->tViewMonth[0] ) {
			pPicker->tViewMonth[1] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, 1, pPicker->tViewMonth[0]);
		}
		if ( iPanel == 1 && pPicker->tViewMonth[1] <= pPicker->tViewMonth[0] ) {
			pPicker->tViewMonth[0] = xrtDateAdd(XRT_TIME_INTERVAL_MONTH, -1, pPicker->tViewMonth[1]);
		}
	}
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static xtime __xgeXuiDatePickerDraftPanelValue(xge_xui_date_picker pPicker, int iPanel)
{
	if ( pPicker == NULL ) {
		return 0;
	}
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		return (iPanel == 1) ? pPicker->tDraftEnd : pPicker->tDraftStart;
	}
	return pPicker->tDraftValue;
}

static void __xgeXuiDatePickerSetDraftPanelValue(xge_xui_date_picker pPicker, int iPanel, xtime tValue)
{
	if ( pPicker == NULL ) {
		return;
	}
	tValue = __xgeXuiDatePickerClampValue(pPicker, tValue);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( iPanel == 1 ) {
			pPicker->tDraftEnd = tValue;
		} else {
			pPicker->tDraftStart = tValue;
		}
		if ( pPicker->bDraftHasValue && pPicker->tDraftEnd < pPicker->tDraftStart ) {
			xtime tSwap = pPicker->tDraftStart;
			pPicker->tDraftStart = pPicker->tDraftEnd;
			pPicker->tDraftEnd = tSwap;
		}
	} else {
		pPicker->tDraftValue = tValue;
	}
	pPicker->bDraftHasValue = 1;
	__xgeXuiDatePickerNotifyDraft(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiDatePickerSelectDate(xge_xui_date_picker pPicker, int iPanel, xtime tDate)
{
	xtime tValue;
	xtime tTime;

	if ( (pPicker == NULL) || !__xgeXuiDatePickerValueEnabled(pPicker, tDate) ) {
		return;
	}
	if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE || pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE ) {
		tValue = xrtDatePart(tDate);
	} else {
		tTime = xrtTimePart(__xgeXuiDatePickerDraftPanelValue(pPicker, iPanel));
		tValue = __xgeXuiDatePickerCombineDateTime(pPicker, tDate, tTime);
	}
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( pPicker->iRangeAnchor == 0 ) {
			pPicker->tDraftStart = tValue;
			pPicker->tDraftEnd = __xgeXuiDatePickerAddSpan(pPicker, tValue, __xgeXuiDatePickerDefaultSpan(pPicker));
			pPicker->iRangeAnchor = 1;
		} else {
			pPicker->tDraftEnd = tValue;
			if ( pPicker->tDraftEnd < pPicker->tDraftStart ) {
				xtime tSwap = pPicker->tDraftStart;
				pPicker->tDraftStart = pPicker->tDraftEnd;
				pPicker->tDraftEnd = tSwap;
			}
			pPicker->iRangeAnchor = 0;
		}
		pPicker->bDraftHasValue = 1;
	} else {
		pPicker->tDraftValue = tValue;
		pPicker->bDraftHasValue = 1;
	}
	__xgeXuiDatePickerNotifyDraft(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiDatePickerSetTimeField(xge_xui_date_picker pPicker, int iPanel, int iField, int iValue)
{
	xtime tValue;
	xtime tDate;
	int iHour;
	int iMinute;
	int iSecond;

	if ( (pPicker == NULL) || (iField < 0) || (iField > 2) ) {
		return;
	}
	tValue = __xgeXuiDatePickerDraftPanelValue(pPicker, iPanel);
	tDate = xrtDatePart(tValue);
	xrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	if ( iField == 0 ) {
		iHour = iValue;
	} else if ( iField == 1 ) {
		iMinute = iValue;
	} else {
		iSecond = iValue;
	}
	if ( iHour < 0 ) {
		iHour = 0;
	}
	if ( iHour > 23 ) {
		iHour = 23;
	}
	if ( iMinute < 0 ) {
		iMinute = 0;
	}
	if ( iMinute > 59 ) {
		iMinute = 59;
	}
	if ( iSecond < 0 ) {
		iSecond = 0;
	}
	if ( iSecond > 59 ) {
		iSecond = 59;
	}
	if ( !pPicker->bShowSecond ) {
		iSecond = 0;
	}
	if ( (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME) || (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
		tValue = __xgeXuiDatePickerMakeTime(iHour, iMinute, iSecond, pPicker->bShowSecond);
	} else {
		tValue = tDate + __xgeXuiDatePickerMakeTime(iHour, iMinute, iSecond, pPicker->bShowSecond);
	}
	__xgeXuiDatePickerSetDraftPanelValue(pPicker, iPanel, tValue);
}

static void __xgeXuiDatePickerSetDraftNow(xge_xui_date_picker pPicker)
{
	xtime tStart;
	xtime tEnd;

	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerGetDefaultValues(pPicker, &tStart, &tEnd);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		pPicker->tDraftStart = tStart;
		pPicker->tDraftEnd = tEnd;
	} else {
		pPicker->tDraftValue = tStart;
	}
	pPicker->bDraftHasValue = 1;
	pPicker->iRangeAnchor = 0;
	__xgeXuiDatePickerInitViewMonth(pPicker);
	__xgeXuiDatePickerNotifyDraft(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

static void __xgeXuiDatePickerPrepareDraft(xge_xui_date_picker pPicker)
{
	xtime tStart;
	xtime tEnd;

	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerEnsureValue(pPicker);
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( pPicker->bHasValue ) {
			pPicker->tDraftStart = pPicker->tStart;
			pPicker->tDraftEnd = pPicker->tEnd;
			pPicker->bDraftHasValue = 1;
		} else {
			__xgeXuiDatePickerGetDefaultValues(pPicker, &tStart, &tEnd);
			pPicker->tDraftStart = tStart;
			pPicker->tDraftEnd = tEnd;
			pPicker->bDraftHasValue = 0;
		}
	} else {
		if ( pPicker->bHasValue ) {
			pPicker->tDraftValue = pPicker->tValue;
			pPicker->bDraftHasValue = 1;
		} else {
			__xgeXuiDatePickerGetDefaultValues(pPicker, &tStart, &tEnd);
			pPicker->tDraftValue = tStart;
			pPicker->bDraftHasValue = 0;
		}
	}
	pPicker->iRangeAnchor = 0;
	__xgeXuiDatePickerInitViewMonth(pPicker);
}

static void __xgeXuiDatePickerOpenPopup(xge_xui_date_picker pPicker, int bOpen)
{
	xge_xui_date_picker_popup_host_t* pHost;
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_vec2_t tSize;
	xge_rect_t tAnchor;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) ) {
		return;
	}
	pPanel = __xgeXuiDatePickerPanelForPicker(pPicker);
	if ( bOpen == 0 ) {
		if ( pPanel != NULL ) {
			__xgeXuiDatePickerCloseActivePopup(pPicker, 0);
		}
		xgeXuiWidgetMarkPaint(pPicker->pWidget);
		return;
	}
	pHost = __xgeXuiDatePickerHostFind(pPicker->pContext);
	if ( pHost == NULL ) {
		return;
	}
	__xgeXuiDatePickerHostCloseAll(pHost, pPicker);
	pPanel = __xgeXuiDatePickerPanelForMode(pPicker);
	if ( pPanel == NULL ) {
		return;
	}
	if ( bOpen ) {
		__xgeXuiDatePickerPrepareDraft(pPicker);
		__xgeXuiDatePickerLayoutMain(pPicker);
		tSize = __xgeXuiDatePickerPreferredSize(pPicker);
		tAnchor = pPicker->pWidget->tBorderRect;
		if ( tAnchor.fW <= 0.0f || tAnchor.fH <= 0.0f ) {
			tAnchor = pPicker->pWidget->tRect;
		}
		pPanel->pPicker = pPicker;
		pPicker->pPopupWidget = pPanel->pContentWidget;
		xgeXuiWidgetSetRect(pPanel->pContentWidget, (xge_rect_t){ 0.0f, 0.0f, tSize.fX, tSize.fY });
		xgeXuiPopupSetOwner(&pPanel->tPopup, pPicker->pWidget);
		xgeXuiPopupSetFocusRestore(&pPanel->tPopup, pPicker->pWidget);
		xgeXuiPopupSetAnchorRect(&pPanel->tPopup, tAnchor);
		xgeXuiPopupSetAnchorPoint(&pPanel->tPopup, XGE_XUI_POPUP_ANCHOR_BOTTOM_LEFT);
		xgeXuiPopupSetDirection(&pPanel->tPopup, XGE_XUI_POPUP_DIRECTION_RIGHT_DOWN);
		xgeXuiPopupSetGap(&pPanel->tPopup, 0.0f);
		xgeXuiPopupSetContentSize(&pPanel->tPopup, tSize.fX, tSize.fY);
		xgeXuiPopupSetBackground(&pPanel->tPopup, pPicker->iPopupColor);
		xgeXuiPopupSetBorder(&pPanel->tPopup, pPicker->iBorderColor);
		__xgeXuiDatePickerLayoutPopup(pPicker);
		__xgeXuiDatePickerPanelLayoutControls(pPanel);
		__xgeXuiDatePickerPanelSyncControls(pPanel);
		xgeXuiPopupSetOpen(&pPanel->tPopup, 1);
		xgeXuiSetFocus(pPicker->pContext, pPanel->pContentWidget);
	}
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiDatePickerPopupClose(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;
	int iClosingMode;

	(void)pWidget;
	pPanel = (xge_xui_date_picker_popup_panel_t*)pUser;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	iClosingMode = pPicker->iClosingMode;
	__xgeXuiDatePickerPanelClosed(pPanel, iClosingMode);
}

static void __xgeXuiDatePickerCommitDraft(xge_xui_date_picker pPicker)
{
	xtime tOldValue;
	xtime tOldStart;
	xtime tOldEnd;
	int bOldHas;
	int bChanged;

	if ( pPicker == NULL ) {
		return;
	}
	tOldValue = pPicker->tValue;
	tOldStart = pPicker->tStart;
	tOldEnd = pPicker->tEnd;
	bOldHas = pPicker->bHasValue;
	if ( pPicker->bDraftHasValue == 0 ) {
		if ( pPicker->bNullable ) {
			pPicker->bHasValue = 0;
		} else {
			__xgeXuiDatePickerGetDefaultValues(pPicker, &pPicker->tDraftStart, &pPicker->tDraftEnd);
			pPicker->tDraftValue = pPicker->tDraftStart;
			pPicker->bDraftHasValue = 1;
		}
	}
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		if ( pPicker->bDraftHasValue ) {
			if ( pPicker->tDraftEnd < pPicker->tDraftStart ) {
				xtime tSwap = pPicker->tDraftStart;
				pPicker->tDraftStart = pPicker->tDraftEnd;
				pPicker->tDraftEnd = tSwap;
			}
			pPicker->tStart = __xgeXuiDatePickerClampValue(pPicker, pPicker->tDraftStart);
			pPicker->tEnd = __xgeXuiDatePickerClampValue(pPicker, pPicker->tDraftEnd);
			if ( pPicker->tEnd < pPicker->tStart ) {
				pPicker->tEnd = pPicker->tStart;
			}
			pPicker->tValue = pPicker->tStart;
			pPicker->bHasValue = 1;
		}
	} else if ( pPicker->bDraftHasValue ) {
		pPicker->tValue = __xgeXuiDatePickerClampValue(pPicker, pPicker->tDraftValue);
		pPicker->tStart = pPicker->tValue;
		pPicker->tEnd = pPicker->tValue;
		pPicker->bHasValue = 1;
	}
	bChanged = (bOldHas != pPicker->bHasValue) || (tOldValue != pPicker->tValue) || (tOldStart != pPicker->tStart) || (tOldEnd != pPicker->tEnd);
	if ( bChanged ) {
		pPicker->iChangeCount++;
		__xgeXuiDatePickerNotifyValue(pPicker, pPicker->procChange, pPicker->pChangeUser);
	}
	pPicker->iCommitCount++;
	__xgeXuiDatePickerNotifyValue(pPicker, pPicker->procCommit, pPicker->pCommitUser);
	__xgeXuiDatePickerCloseActivePopup(pPicker, 1);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiDatePickerClearInternal(xge_xui_date_picker pPicker)
{
	int bChanged;

	if ( (pPicker == NULL) || (pPicker->bNullable == 0) ) {
		return;
	}
	bChanged = pPicker->bHasValue != 0;
	pPicker->bHasValue = 0;
	pPicker->bDraftHasValue = 0;
	pPicker->iClearCount++;
	if ( bChanged ) {
		pPicker->iChangeCount++;
		__xgeXuiDatePickerNotifyValue(pPicker, pPicker->procChange, pPicker->pChangeUser);
	}
	__xgeXuiDatePickerNotifyValue(pPicker, pPicker->procClear, pPicker->pClearUser);
	__xgeXuiDatePickerCloseActivePopup(pPicker, 2);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static xge_xui_date_picker_popup_host_t* __xgeXuiDatePickerHostFind(xge_xui_context pContext)
{
	xge_xui_date_picker_popup_host_t* pHost;

	for ( pHost = g_pXgeXuiDatePickerHostList; pHost != NULL; pHost = pHost->pNext ) {
		if ( pHost->pContext == pContext ) {
			return pHost;
		}
	}
	return NULL;
}

static xge_xui_widget __xgeXuiDatePickerPanelAddChild(xge_xui_date_picker_popup_panel_t* pPanel)
{
	xge_xui_widget pWidget;

	if ( (pPanel == NULL) || (pPanel->pContentWidget == NULL) ) {
		return NULL;
	}
	pWidget = xgeXuiWidgetCreate();
	if ( pWidget == NULL ) {
		return NULL;
	}
	if ( xgeXuiWidgetAddInternal(pPanel->pContentWidget, pWidget) != XGE_OK ) {
		xgeXuiWidgetFree(pWidget);
		return NULL;
	}
	return pWidget;
}

static int __xgeXuiDatePickerYearFilter(xge_xui_widget pWidget, const char* sOldText, const char* sNewText, void* pUser)
{
	const unsigned char* p;
	int iLen;

	(void)pWidget;
	(void)sOldText;
	(void)pUser;
	if ( sNewText == NULL ) {
		return 1;
	}
	iLen = 0;
	for ( p = (const unsigned char*)sNewText; *p != 0; p++ ) {
		if ( (*p < '0') || (*p > '9') ) {
			return 0;
		}
		iLen++;
		if ( iLen > 4 ) {
			return 0;
		}
	}
	return 1;
}

static void __xgeXuiDatePickerYearChanged(xge_xui_widget pWidget, const char* sText, void* pUser)
{
	xge_xui_date_picker_control_ref_t* pRef;
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;
	int64 iYear;
	int64 iOldYear;
	int iMonth;

	(void)pWidget;
	pRef = (xge_xui_date_picker_control_ref_t*)pUser;
	pPanel = (pRef != NULL) ? pRef->pPanel : NULL;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( (pPicker == NULL) || (pPanel->bSyncing != 0) || (sText == NULL) || (sText[0] == 0) ) {
		return;
	}
	iYear = xrtStrToI64(sText);
	if ( iYear < 1 ) {
		return;
	}
	xrtDecodeSerial(pPicker->tViewMonth[pRef->iPanel], &iOldYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	(void)iOldYear;
	__xgeXuiDatePickerSetViewMonthValue(pPicker, pRef->iPanel, iYear, iMonth);
	__xgeXuiDatePickerPanelSyncControls(pPanel);
}

static void __xgeXuiDatePickerMonthSelected(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	xge_xui_date_picker_control_ref_t* pRef;
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;
	int64 iYear;

	(void)pWidget;
	pRef = (xge_xui_date_picker_control_ref_t*)pUser;
	pPanel = (pRef != NULL) ? pRef->pPanel : NULL;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( (pPicker == NULL) || (pPanel->bSyncing != 0) || (iIndex < 0) || (iIndex >= 12) ) {
		return;
	}
	xrtDecodeSerial(pPicker->tViewMonth[pRef->iPanel], &iYear, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	__xgeXuiDatePickerSetViewMonthValue(pPicker, pRef->iPanel, iYear, iIndex + 1);
	__xgeXuiDatePickerPanelSyncControls(pPanel);
}

static void __xgeXuiDatePickerTimeChanged(xge_xui_widget pWidget, float fValue, void* pUser)
{
	xge_xui_date_picker_control_ref_t* pRef;
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;

	(void)pWidget;
	pRef = (xge_xui_date_picker_control_ref_t*)pUser;
	pPanel = (pRef != NULL) ? pRef->pPanel : NULL;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( (pPicker == NULL) || (pPanel->bSyncing != 0) ) {
		return;
	}
	__xgeXuiDatePickerSetTimeField(pPicker, pRef->iPanel, pRef->iIndex, (int)(fValue + 0.5f));
	__xgeXuiDatePickerPanelSyncControls(pPanel);
}

static void __xgeXuiDatePickerButtonClicked(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_date_picker_control_ref_t* pRef;
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;

	(void)pWidget;
	pRef = (xge_xui_date_picker_control_ref_t*)pUser;
	pPanel = (pRef != NULL) ? pRef->pPanel : NULL;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	switch ( pRef->iKind ) {
		case XGE_XUI_DATE_PICKER_CONTROL_PREV:
			__xgeXuiDatePickerMoveViewMonth(pPicker, pRef->iPanel, -1);
			__xgeXuiDatePickerPanelSyncControls(pPanel);
			break;
		case XGE_XUI_DATE_PICKER_CONTROL_NEXT:
			__xgeXuiDatePickerMoveViewMonth(pPicker, pRef->iPanel, 1);
			__xgeXuiDatePickerPanelSyncControls(pPanel);
			break;
		case XGE_XUI_DATE_PICKER_CONTROL_TODAY:
			__xgeXuiDatePickerSetDraftNow(pPicker);
			__xgeXuiDatePickerPanelSyncControls(pPanel);
			break;
		case XGE_XUI_DATE_PICKER_CONTROL_CLEAR:
			__xgeXuiDatePickerClearInternal(pPicker);
			break;
		case XGE_XUI_DATE_PICKER_CONTROL_CANCEL:
			__xgeXuiDatePickerCloseActivePopup(pPicker, 0);
			break;
		case XGE_XUI_DATE_PICKER_CONTROL_OK:
			__xgeXuiDatePickerCommitDraft(pPicker);
			break;
		default:
			break;
	}
}

static int __xgeXuiDatePickerPanelInitButton(
	xge_xui_date_picker_popup_panel_t* pPanel,
	xge_xui_widget* pWidget,
	xge_xui_button pButton,
	xge_xui_date_picker_control_ref_t* pRef,
	int iKind,
	int iPanel,
	const char* sText)
{
	*pWidget = __xgeXuiDatePickerPanelAddChild(pPanel);
	if ( *pWidget == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	pRef->pPanel = pPanel;
	pRef->iKind = iKind;
	pRef->iPanel = iPanel;
	pRef->iIndex = 0;
	if ( xgeXuiButtonInit(pButton, pPanel->pHost->pContext, *pWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetText(pButton, NULL, sText);
	xgeXuiButtonSetClick(pButton, __xgeXuiDatePickerButtonClicked, pRef);
	return XGE_OK;
}

static int __xgeXuiDatePickerPanelInit(xge_xui_date_picker_popup_host_t* pHost, int iMode)
{
	xge_xui_date_picker_popup_panel_t* pPanel;
	int i;
	int j;

	pPanel = &pHost->arrPanel[iMode];
	memset(pPanel, 0, sizeof(*pPanel));
	pPanel->pHost = pHost;
	pPanel->iMode = iMode;
	pPanel->pPopupWidget = xgeXuiWidgetCreate();
	pPanel->pContentWidget = xgeXuiWidgetCreate();
	if ( (pPanel->pPopupWidget == NULL) || (pPanel->pContentWidget == NULL) ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	if ( xgeXuiPopupInit(&pPanel->tPopup, pHost->pContext, pPanel->pPopupWidget) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetName(pPanel->pPopupWidget, "datepicker-popup");
	xgeXuiWidgetSetName(pPanel->pContentWidget, "datepicker-popup-content");
	xgeXuiWidgetSetLayout(pPanel->pContentWidget, XGE_XUI_LAYOUT_ABSOLUTE);
	xgeXuiWidgetSetBackground(pPanel->pContentWidget, XGE_COLOR_RGBA(0, 0, 0, 0));
	xgeXuiWidgetSetBorder(pPanel->pContentWidget, 0.0f, 0);
	xgeXuiWidgetSetClip(pPanel->pContentWidget, 1);
	xgeXuiWidgetSetFocusable(pPanel->pContentWidget, 1);
	xgeXuiWidgetSetEvent(pPanel->pContentWidget, xgeXuiDatePickerPopupEventProc, pPanel);
	pPanel->pContentWidget->procPaint = xgeXuiDatePickerPopupPaintProc;
	pPanel->pContentWidget->pUser = pPanel;
	xgeXuiPopupSetContentWidget(&pPanel->tPopup, pPanel->pContentWidget);
	xgeXuiPopupSetFocusPolicy(&pPanel->tPopup, XGE_XUI_POPUP_FOCUS_CUSTOM, pPanel->pContentWidget);
	xgeXuiPopupSetClose(&pPanel->tPopup, __xgeXuiDatePickerPopupClose, pPanel);
	xgeXuiPopupSetBackground(&pPanel->tPopup, XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiPopupSetBorder(&pPanel->tPopup, XGE_COLOR_RGBA(136, 180, 216, 255));
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		if ( __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->arrPrevWidget[i], &pPanel->arrPrevButton[i], &pPanel->arrPrevRef[i], XGE_XUI_DATE_PICKER_CONTROL_PREV, i, "<") != XGE_OK ||
		     __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->arrNextWidget[i], &pPanel->arrNextButton[i], &pPanel->arrNextRef[i], XGE_XUI_DATE_PICKER_CONTROL_NEXT, i, ">") != XGE_OK ) {
			return XGE_ERROR;
		}
		pPanel->arrYearWidget[i] = __xgeXuiDatePickerPanelAddChild(pPanel);
		pPanel->arrMonthWidget[i] = __xgeXuiDatePickerPanelAddChild(pPanel);
		if ( (pPanel->arrYearWidget[i] == NULL) || (pPanel->arrMonthWidget[i] == NULL) ) {
			return XGE_ERROR_OUT_OF_MEMORY;
		}
		pPanel->arrYearRef[i].pPanel = pPanel;
		pPanel->arrYearRef[i].iKind = XGE_XUI_DATE_PICKER_CONTROL_YEAR;
		pPanel->arrYearRef[i].iPanel = i;
		if ( xgeXuiInputInit(&pPanel->arrYearInput[i], pHost->pContext, pPanel->arrYearWidget[i], NULL) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiInputSetMaxLength(&pPanel->arrYearInput[i], 4);
		xgeXuiInputSetTextAlign(&pPanel->arrYearInput[i], XGE_TEXT_ALIGN_CENTER);
		xgeXuiInputSetFilter(&pPanel->arrYearInput[i], __xgeXuiDatePickerYearFilter, &pPanel->arrYearRef[i]);
		xgeXuiInputSetChange(&pPanel->arrYearInput[i], __xgeXuiDatePickerYearChanged, &pPanel->arrYearRef[i]);
		xgeXuiInputSetSubmit(&pPanel->arrYearInput[i], __xgeXuiDatePickerYearChanged, &pPanel->arrYearRef[i]);
		pPanel->arrMonthRef[i].pPanel = pPanel;
		pPanel->arrMonthRef[i].iKind = XGE_XUI_DATE_PICKER_CONTROL_MONTH;
		pPanel->arrMonthRef[i].iPanel = i;
		if ( xgeXuiComboBoxInit(&pPanel->arrMonthCombo[i], pHost->pContext, pPanel->arrMonthWidget[i]) != XGE_OK ) {
			return XGE_ERROR;
		}
		xgeXuiComboBoxSetItems(&pPanel->arrMonthCombo[i], g_arrXgeXuiDatePickerMonthText, 12);
		xgeXuiComboBoxSetPopupHeight(&pPanel->arrMonthCombo[i], 240.0f);
		xgeXuiComboBoxSetSelect(&pPanel->arrMonthCombo[i], __xgeXuiDatePickerMonthSelected, &pPanel->arrMonthRef[i]);
		for ( j = 0; j < 3; j++ ) {
			pPanel->arrTimeWidget[i][j] = __xgeXuiDatePickerPanelAddChild(pPanel);
			if ( pPanel->arrTimeWidget[i][j] == NULL ) {
				return XGE_ERROR_OUT_OF_MEMORY;
			}
			pPanel->arrTimeRef[i][j].pPanel = pPanel;
			pPanel->arrTimeRef[i][j].iKind = XGE_XUI_DATE_PICKER_CONTROL_TIME;
			pPanel->arrTimeRef[i][j].iPanel = i;
			pPanel->arrTimeRef[i][j].iIndex = j;
			if ( xgeXuiNumericInputInit(&pPanel->arrTimeInput[i][j], pHost->pContext, pPanel->arrTimeWidget[i][j], NULL) != XGE_OK ) {
				return XGE_ERROR;
			}
			xgeXuiNumericInputSetInteger(&pPanel->arrTimeInput[i][j], 1);
			xgeXuiNumericInputSetPrecision(&pPanel->arrTimeInput[i][j], 0);
			xgeXuiNumericInputSetRange(&pPanel->arrTimeInput[i][j], 0.0f, (j == 0) ? 23.0f : 59.0f);
			xgeXuiNumericInputSetStep(&pPanel->arrTimeInput[i][j], 1.0f);
			xgeXuiNumericInputSetSpinnerWidth(&pPanel->arrTimeInput[i][j], 16.0f);
			xgeXuiInputSetTextAlign(&pPanel->arrTimeInput[i][j].tInput, XGE_TEXT_ALIGN_CENTER);
			xgeXuiNumericInputSetChange(&pPanel->arrTimeInput[i][j], __xgeXuiDatePickerTimeChanged, &pPanel->arrTimeRef[i][j]);
		}
	}
	if ( __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->pTodayWidget, &pPanel->tTodayButton, &pPanel->tTodayRef, XGE_XUI_DATE_PICKER_CONTROL_TODAY, 0, "Today") != XGE_OK ||
	     __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->pClearWidget, &pPanel->tClearButton, &pPanel->tClearRef, XGE_XUI_DATE_PICKER_CONTROL_CLEAR, 0, "Clear") != XGE_OK ||
	     __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->pCancelWidget, &pPanel->tCancelButton, &pPanel->tCancelRef, XGE_XUI_DATE_PICKER_CONTROL_CANCEL, 0, "Cancel") != XGE_OK ||
	     __xgeXuiDatePickerPanelInitButton(pPanel, &pPanel->pOkWidget, &pPanel->tOkButton, &pPanel->tOkRef, XGE_XUI_DATE_PICKER_CONTROL_OK, 0, "OK") != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiButtonSetSemantic(&pPanel->tOkButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	return XGE_OK;
}

static void __xgeXuiDatePickerPanelUnit(xge_xui_date_picker_popup_panel_t* pPanel)
{
	int i;
	int j;

	if ( pPanel == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		xgeXuiButtonUnit(&pPanel->arrPrevButton[i]);
		xgeXuiButtonUnit(&pPanel->arrNextButton[i]);
		xgeXuiInputUnit(&pPanel->arrYearInput[i]);
		xgeXuiComboBoxUnit(&pPanel->arrMonthCombo[i]);
		for ( j = 0; j < 3; j++ ) {
			xgeXuiNumericInputUnit(&pPanel->arrTimeInput[i][j]);
		}
	}
	xgeXuiButtonUnit(&pPanel->tTodayButton);
	xgeXuiButtonUnit(&pPanel->tClearButton);
	xgeXuiButtonUnit(&pPanel->tCancelButton);
	xgeXuiButtonUnit(&pPanel->tOkButton);
	xgeXuiPopupUnit(&pPanel->tPopup);
	xgeXuiWidgetFree(pPanel->pPopupWidget);
	memset(pPanel, 0, sizeof(*pPanel));
}

static xge_xui_date_picker_popup_host_t* __xgeXuiDatePickerHostRetain(xge_xui_context pContext)
{
	xge_xui_date_picker_popup_host_t* pHost;
	int i;

	if ( pContext == NULL ) {
		return NULL;
	}
	pHost = __xgeXuiDatePickerHostFind(pContext);
	if ( pHost != NULL ) {
		pHost->iRefCount++;
		return pHost;
	}
	pHost = (xge_xui_date_picker_popup_host_t*)xrtMalloc(sizeof(*pHost));
	if ( pHost == NULL ) {
		return NULL;
	}
	memset(pHost, 0, sizeof(*pHost));
	pHost->pContext = pContext;
	pHost->iRefCount = 1;
	for ( i = 0; i < XGE_XUI_DATE_PICKER_POPUP_COUNT; i++ ) {
		if ( __xgeXuiDatePickerPanelInit(pHost, i) != XGE_OK ) {
			while ( i >= 0 ) {
				__xgeXuiDatePickerPanelUnit(&pHost->arrPanel[i]);
				i--;
			}
			xrtFree(pHost);
			return NULL;
		}
	}
	pHost->pNext = g_pXgeXuiDatePickerHostList;
	g_pXgeXuiDatePickerHostList = pHost;
	return pHost;
}

static void __xgeXuiDatePickerHostRelease(xge_xui_context pContext)
{
	xge_xui_date_picker_popup_host_t* pHost;
	xge_xui_date_picker_popup_host_t* pPrev;
	int i;

	pPrev = NULL;
	for ( pHost = g_pXgeXuiDatePickerHostList; pHost != NULL; pHost = pHost->pNext ) {
		if ( pHost->pContext == pContext ) {
			break;
		}
		pPrev = pHost;
	}
	if ( pHost == NULL ) {
		return;
	}
	pHost->iRefCount--;
	if ( pHost->iRefCount > 0 ) {
		return;
	}
	if ( pPrev != NULL ) {
		pPrev->pNext = pHost->pNext;
	} else {
		g_pXgeXuiDatePickerHostList = pHost->pNext;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_POPUP_COUNT; i++ ) {
		__xgeXuiDatePickerPanelUnit(&pHost->arrPanel[i]);
	}
	xrtFree(pHost);
}

static xge_xui_date_picker_popup_panel_t* __xgeXuiDatePickerPanelForPicker(xge_xui_date_picker pPicker)
{
	xge_xui_date_picker_popup_host_t* pHost;
	int i;

	if ( pPicker == NULL ) {
		return NULL;
	}
	pHost = __xgeXuiDatePickerHostFind(pPicker->pContext);
	if ( pHost == NULL ) {
		return NULL;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_POPUP_COUNT; i++ ) {
		if ( pHost->arrPanel[i].pPicker == pPicker ) {
			return &pHost->arrPanel[i];
		}
	}
	return NULL;
}

static xge_xui_date_picker_popup_panel_t* __xgeXuiDatePickerPanelForMode(xge_xui_date_picker pPicker)
{
	xge_xui_date_picker_popup_host_t* pHost;
	int iMode;

	if ( pPicker == NULL ) {
		return NULL;
	}
	pHost = __xgeXuiDatePickerHostFind(pPicker->pContext);
	if ( pHost == NULL ) {
		return NULL;
	}
	iMode = __xgeXuiDatePickerClampMode(pPicker->iMode);
	return &pHost->arrPanel[iMode];
}

static void __xgeXuiDatePickerHostCloseAll(xge_xui_date_picker_popup_host_t* pHost, xge_xui_date_picker pExcept)
{
	xge_xui_date_picker pPicker;
	int i;

	if ( pHost == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_POPUP_COUNT; i++ ) {
		pPicker = pHost->arrPanel[i].pPicker;
		if ( (pPicker != NULL) && (pPicker != pExcept) ) {
			__xgeXuiDatePickerCloseActivePopup(pPicker, 0);
		}
	}
}

static void __xgeXuiDatePickerPanelClosed(xge_xui_date_picker_popup_panel_t* pPanel, int iClosingMode)
{
	xge_xui_date_picker pPicker;

	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	if ( iClosingMode == 0 ) {
		pPicker->iCancelCount++;
		__xgeXuiDatePickerNotifyValue(pPicker, pPicker->procCancel, pPicker->pCancelUser);
	}
	pPicker->iClosingMode = 0;
	pPicker->pPopupWidget = NULL;
	pPanel->pPicker = NULL;
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

static void __xgeXuiDatePickerCloseActivePopup(xge_xui_date_picker pPicker, int iClosingMode)
{
	xge_xui_date_picker_popup_panel_t* pPanel;

	pPanel = __xgeXuiDatePickerPanelForPicker(pPicker);
	if ( pPanel == NULL ) {
		return;
	}
	pPicker->iClosingMode = iClosingMode;
	xgeXuiPopupSetOpen(&pPanel->tPopup, 0);
	__xgeXuiDatePickerPanelClosed(pPanel, iClosingMode);
}

static void __xgeXuiDatePickerPanelShowWidget(xge_xui_widget pWidget, xge_rect_t tRect, int bVisible)
{
	xge_rect_t tParent;
	xge_rect_t tLocal;

	if ( pWidget == NULL ) {
		return;
	}
	xgeXuiWidgetSetVisible(pWidget, bVisible);
	if ( bVisible ) {
		tLocal = tRect;
		if ( pWidget->pParent != NULL ) {
			tParent = pWidget->pParent->tContentRect;
			if ( tParent.fW <= 0.0f && tParent.fH <= 0.0f ) {
				tParent = pWidget->pParent->tRect;
			}
			tLocal.fX -= tParent.fX;
			tLocal.fY -= tParent.fY;
		}
		pWidget->tLocalRect = tLocal;
		__xgeXuiWidgetArrangeRect(pWidget, tRect);
	}
}

static void __xgeXuiDatePickerPanelStyleHeaderControls(xge_xui_date_picker_popup_panel_t* pPanel)
{
	xge_xui_date_picker pPicker;
	uint32_t iTransparent;
	uint32_t iHeaderControl;
	uint32_t iHeaderHover;
	uint32_t iHeaderActive;
	uint32_t iHeaderDisabled;
	uint32_t iControlBorder;
	uint32_t iPopupColor;
	int i;

	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	iTransparent = XGE_COLOR_RGBA(0, 0, 0, 0);
	iHeaderControl = XGE_COLOR_RGBA(246, 251, 255, 255);
	iHeaderHover = XGE_COLOR_RGBA(235, 247, 253, 255);
	iHeaderActive = XGE_COLOR_RGBA(218, 236, 249, 255);
	iHeaderDisabled = XGE_COLOR_RGBA(236, 243, 248, 255);
	iControlBorder = XGE_COLOR_RGBA(180, 210, 232, 255);
	iPopupColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		xgeXuiButtonSetColors(&pPanel->arrPrevButton[i], iTransparent, iHeaderHover, iHeaderActive, iTransparent, iTransparent);
		xgeXuiButtonSetColors(&pPanel->arrNextButton[i], iTransparent, iHeaderHover, iHeaderActive, iTransparent, iTransparent);
		xgeXuiButtonSetTextColor(&pPanel->arrPrevButton[i], pPicker->iMutedTextColor);
		xgeXuiButtonSetTextColor(&pPanel->arrNextButton[i], pPicker->iMutedTextColor);
		xgeXuiWidgetSetBorder(pPanel->arrPrevWidget[i], 0.0f, iTransparent);
		xgeXuiWidgetSetBorder(pPanel->arrNextWidget[i], 0.0f, iTransparent);
		xgeXuiWidgetSetStateBorder(pPanel->arrPrevWidget[i], XGE_XUI_STATE_HOVER, 0.0f, iTransparent);
		xgeXuiWidgetSetStateBorder(pPanel->arrNextWidget[i], XGE_XUI_STATE_HOVER, 0.0f, iTransparent);
		xgeXuiWidgetSetStateBorder(pPanel->arrPrevWidget[i], XGE_XUI_STATE_ACTIVE, 0.0f, iTransparent);
		xgeXuiWidgetSetStateBorder(pPanel->arrNextWidget[i], XGE_XUI_STATE_ACTIVE, 0.0f, iTransparent);
		xgeXuiWidgetSetFocusRing(pPanel->arrPrevWidget[i], 0.0f, iTransparent);
		xgeXuiWidgetSetFocusRing(pPanel->arrNextWidget[i], 0.0f, iTransparent);
		xgeXuiWidgetSetRadius(pPanel->arrPrevWidget[i], 2.0f);
		xgeXuiWidgetSetRadius(pPanel->arrNextWidget[i], 2.0f);
		xgeXuiInputSetColors(&pPanel->arrYearInput[i], pPicker->iTextColor, iHeaderControl, pPicker->iFocusColor, pPicker->iTextColor);
		xgeXuiInputSetFrameColors(&pPanel->arrYearInput[i], iHeaderControl, iHeaderHover, iControlBorder, iControlBorder, pPicker->iFocusColor);
		xgeXuiInputSetDisabledColors(&pPanel->arrYearInput[i], pPicker->iDisabledTextColor, iHeaderDisabled, iControlBorder);
		pPanel->arrMonthCombo[i].iBorderColor = iControlBorder;
		pPanel->arrMonthCombo[i].iItemSelectedColor = pPicker->iSelectedColor;
		xgeXuiComboBoxSetColors(&pPanel->arrMonthCombo[i], iHeaderControl, iHeaderHover, pPicker->iFocusColor, iHeaderDisabled, pPicker->iTextColor, iPopupColor);
		xgeXuiComboBoxSetItemColors(&pPanel->arrMonthCombo[i], iHeaderHover, pPicker->iSelectedColor, XGE_COLOR_RGBA(238, 242, 246, 255), pPicker->iDisabledTextColor);
	}
}

static void __xgeXuiDatePickerPanelLayoutControls(xge_xui_date_picker_popup_panel_t* pPanel)
{
	xge_xui_date_picker pPicker;
	int i;
	int j;

	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		__xgeXuiDatePickerPanelShowWidget(pPanel->arrPrevWidget[i], pPicker->tPrevRect[i], __xgeXuiDatePickerHasDatePanel(pPicker->iMode) && i < __xgeXuiDatePickerPanelCount(pPicker->iMode));
		__xgeXuiDatePickerPanelShowWidget(pPanel->arrNextWidget[i], pPicker->tNextRect[i], __xgeXuiDatePickerHasDatePanel(pPicker->iMode) && i < __xgeXuiDatePickerPanelCount(pPicker->iMode));
		__xgeXuiDatePickerPanelShowWidget(pPanel->arrYearWidget[i], pPicker->tYearRect[i], __xgeXuiDatePickerHasDatePanel(pPicker->iMode) && i < __xgeXuiDatePickerPanelCount(pPicker->iMode));
		__xgeXuiDatePickerPanelShowWidget(pPanel->arrMonthWidget[i], pPicker->tMonthRect[i], __xgeXuiDatePickerHasDatePanel(pPicker->iMode) && i < __xgeXuiDatePickerPanelCount(pPicker->iMode));
		for ( j = 0; j < 3; j++ ) {
			__xgeXuiDatePickerPanelShowWidget(pPanel->arrTimeWidget[i][j], pPicker->arrTimeRect[i][j], __xgeXuiDatePickerHasTimePanel(pPicker->iMode) && i < __xgeXuiDatePickerPanelCount(pPicker->iMode) && (j < 2 || pPicker->bShowSecond));
		}
	}
	__xgeXuiDatePickerPanelShowWidget(pPanel->pTodayWidget, pPicker->arrFooterRect[0], 1);
	__xgeXuiDatePickerPanelShowWidget(pPanel->pClearWidget, pPicker->arrFooterRect[1], pPicker->bNullable);
	__xgeXuiDatePickerPanelShowWidget(pPanel->pCancelWidget, pPicker->arrFooterRect[2], 1);
	__xgeXuiDatePickerPanelShowWidget(pPanel->pOkWidget, pPicker->arrFooterRect[3], 1);
}

static void __xgeXuiDatePickerPanelSyncControls(xge_xui_date_picker_popup_panel_t* pPanel)
{
	xge_xui_date_picker pPicker;
	xge_font pFont;
	char sText[32];
	xtime tValue;
	int64 iYear;
	int iMonth;
	int iHour;
	int iMinute;
	int iSecond;
	int i;
	int j;

	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( pPicker == NULL ) {
		return;
	}
	pFont = pPicker->pFont;
	pPanel->bSyncing = 1;
	__xgeXuiDatePickerPanelStyleHeaderControls(pPanel);
	for ( i = 0; i < XGE_XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		xgeXuiButtonSetText(&pPanel->arrPrevButton[i], pFont, "<");
		xgeXuiButtonSetText(&pPanel->arrNextButton[i], pFont, ">");
		xgeXuiInputSetFont(&pPanel->arrYearInput[i], pFont);
		xgeXuiComboBoxSetFont(&pPanel->arrMonthCombo[i], pFont);
		xrtDecodeSerial(pPicker->tViewMonth[i], &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		snprintf(sText, sizeof(sText), "%lld", iYear);
		sText[sizeof(sText) - 1] = 0;
		xgeXuiInputSetText(&pPanel->arrYearInput[i], sText);
		xgeXuiComboBoxSetSelected(&pPanel->arrMonthCombo[i], iMonth - 1);
		tValue = __xgeXuiDatePickerDraftPanelValue(pPicker, i);
		xrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
		for ( j = 0; j < 3; j++ ) {
			xgeXuiInputSetFont(&pPanel->arrTimeInput[i][j].tInput, pFont);
			xgeXuiNumericInputSetValue(&pPanel->arrTimeInput[i][j], (float)((j == 0) ? iHour : ((j == 1) ? iMinute : iSecond)));
		}
	}
	xgeXuiButtonSetText(&pPanel->tTodayButton, pFont, (pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME || pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) ? "Now" : "Today");
	xgeXuiButtonSetText(&pPanel->tClearButton, pFont, "Clear");
	xgeXuiButtonSetText(&pPanel->tCancelButton, pFont, "Cancel");
	xgeXuiButtonSetText(&pPanel->tOkButton, pFont, "OK");
	pPanel->bSyncing = 0;
	xgeXuiWidgetMarkPaint(pPanel->pContentWidget);
}

static void __xgeXuiDatePickerFormatOne(xge_xui_date_picker pPicker, xtime tValue, char* sBuffer, int iBufferSize)
{
	char* sText;

	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return;
	}
	sBuffer[0] = 0;
	if ( pPicker == NULL ) {
		return;
	}
	sText = (char*)xrtTimeFormat(tValue, pPicker->sFormat);
	if ( sText != NULL ) {
		snprintf(sBuffer, (size_t)iBufferSize, "%s", sText);
		sBuffer[iBufferSize - 1] = 0;
		xrtFree(sText);
		return;
	}
	if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME || pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		int iHour;
		int iMinute;
		int iSecond;
		xrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
		if ( pPicker->bShowSecond ) {
			snprintf(sBuffer, (size_t)iBufferSize, "%02d:%02d:%02d", iHour, iMinute, iSecond);
		} else {
			snprintf(sBuffer, (size_t)iBufferSize, "%02d:%02d", iHour, iMinute);
		}
	} else {
		int64 iYear;
		int iMonth;
		int iDay;
		int iHour;
		int iMinute;
		int iSecond;
		xrtDecodeSerial(tValue, &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, NULL, NULL);
		if ( pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE || pPicker->iMode == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE ) {
			snprintf(sBuffer, (size_t)iBufferSize, "%04lld-%02d-%02d", iYear, iMonth, iDay);
		} else if ( pPicker->bShowSecond ) {
			snprintf(sBuffer, (size_t)iBufferSize, "%04lld-%02d-%02d %02d:%02d:%02d", iYear, iMonth, iDay, iHour, iMinute, iSecond);
		} else {
			snprintf(sBuffer, (size_t)iBufferSize, "%04lld-%02d-%02d %02d:%02d", iYear, iMonth, iDay, iHour, iMinute);
		}
	}
	sBuffer[iBufferSize - 1] = 0;
}

static void __xgeXuiDatePickerFormatDisplay(xge_xui_date_picker pPicker, char* sBuffer, int iBufferSize)
{
	char sA[96];
	char sB[96];

	if ( (sBuffer == NULL) || (iBufferSize <= 0) ) {
		return;
	}
	sBuffer[0] = 0;
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->bHasValue == 0 ) {
		return;
	}
	if ( __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		__xgeXuiDatePickerFormatOne(pPicker, pPicker->tStart, sA, sizeof(sA));
		__xgeXuiDatePickerFormatOne(pPicker, pPicker->tEnd, sB, sizeof(sB));
		snprintf(sBuffer, (size_t)iBufferSize, "%s%s%s", sA, pPicker->sRangeSeparator, sB);
	} else {
		__xgeXuiDatePickerFormatOne(pPicker, pPicker->tValue, sBuffer, iBufferSize);
	}
	sBuffer[iBufferSize - 1] = 0;
}

static void __xgeXuiDatePickerDrawChevron(xge_rect_t tRect, uint32_t iColor, int bUp)
{
	xge_vec2_t tA;
	xge_vec2_t tB;
	xge_vec2_t tC;
	float fCX;
	float fCY;

	fCX = tRect.fX + tRect.fW * 0.5f;
	fCY = tRect.fY + tRect.fH * 0.5f;
	if ( bUp ) {
		tA = (xge_vec2_t){ fCX - 4.0f, fCY + 2.0f };
		tB = (xge_vec2_t){ fCX + 4.0f, fCY + 2.0f };
		tC = (xge_vec2_t){ fCX, fCY - 3.0f };
	} else {
		tA = (xge_vec2_t){ fCX - 4.0f, fCY - 2.0f };
		tB = (xge_vec2_t){ fCX + 4.0f, fCY - 2.0f };
		tC = (xge_vec2_t){ fCX, fCY + 3.0f };
	}
	xgeShapeTriangleFillPx(tA, tB, tC, iColor);
}

static void __xgeXuiDatePickerDrawCalendar(xge_xui_date_picker pPicker, int iPanel)
{
	static const char* arrWeekSun[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char* arrWeekMon[7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
	const char** arrWeek;
	xge_rect_t tRect;
	xge_rect_t tWeek;
	xge_rect_t tText;
	xtime tDate;
	xtime tSelectedA;
	xtime tSelectedB;
	int iDay;
	int i;
	int bEnabled;
	int bSelected;
	int bInRange;
	char sText[32];
	uint32_t iText;

	if ( (pPicker == NULL) || (iPanel < 0) || (iPanel >= XGE_XUI_DATE_PICKER_PANEL_CAPACITY) || pPicker->tPanelRect[iPanel].fW <= 0.0f ) {
		return;
	}
	tRect = pPicker->tPanelRect[iPanel];
	__xgeXuiHostDrawRect(tRect, pPicker->iPanelColor);
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(224, 236, 246, 255));
	__xgeXuiHostDrawRect(pPicker->tHeaderRect[iPanel], pPicker->iHeaderColor);
	__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX, pPicker->tHeaderRect[iPanel].fY + pPicker->tHeaderRect[iPanel].fH - 1.0f, tRect.fW, 1.0f }, XGE_COLOR_RGBA(214, 229, 242, 255));
	arrWeek = (pPicker->iFirstDayOfWeek == 1) ? arrWeekMon : arrWeekSun;
	for ( i = 0; i < 7; i++ ) {
		tWeek = (xge_rect_t){ tRect.fX + (float)i * (tRect.fW / 7.0f), pPicker->tHeaderRect[iPanel].fY + pPicker->tHeaderRect[iPanel].fH + 1.0f, tRect.fW / 7.0f, 20.0f };
		if ( pPicker->pFont != NULL ) {
			__xgeXuiHostDrawTextRect(pPicker->pFont, arrWeek[i], tWeek, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
	tSelectedA = __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ? pPicker->tDraftStart : pPicker->tDraftValue;
	tSelectedB = __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ? pPicker->tDraftEnd : pPicker->tDraftValue;
	if ( tSelectedB < tSelectedA ) {
		xtime tSwap = tSelectedA;
		tSelectedA = tSelectedB;
		tSelectedB = tSwap;
	}
	for ( i = 0; i < 42; i++ ) {
		tText = pPicker->arrDayRect[iPanel][i];
		if ( !__xgeXuiDatePickerCellDate(pPicker, iPanel, i, &tDate) ) {
			continue;
		}
		xrtDecodeSerial(tDate, NULL, NULL, &iDay, NULL, NULL, NULL, NULL, NULL);
		bEnabled = __xgeXuiDatePickerValueEnabled(pPicker, tDate);
		bSelected = pPicker->bDraftHasValue && (xrtDatePart(tDate) == xrtDatePart(tSelectedA) || xrtDatePart(tDate) == xrtDatePart(tSelectedB));
		bInRange = pPicker->bDraftHasValue && __xgeXuiDatePickerIsRangeMode(pPicker->iMode) && (xrtDatePart(tDate) >= xrtDatePart(tSelectedA)) && (xrtDatePart(tDate) <= xrtDatePart(tSelectedB));
		if ( bInRange ) {
			xge_rect_t tRange = tText;
			tRange.fY += 4.0f;
			tRange.fH -= 8.0f;
			__xgeXuiHostDrawRect(tRange, pPicker->iRangeColor);
		}
		if ( pPicker->iHoverDayPanel == iPanel && pPicker->iHoverDayCell == i && bEnabled ) {
			xge_rect_t tHover = tText;
			tHover.fX += 3.0f;
			tHover.fY += 3.0f;
			tHover.fW -= 6.0f;
			tHover.fH -= 6.0f;
			__xgeXuiHostDrawRect(tHover, pPicker->iHoverColor);
		}
		if ( bSelected ) {
			xge_rect_t tSel = tText;
			tSel.fX += 3.0f;
			tSel.fY += 3.0f;
			tSel.fW -= 6.0f;
			tSel.fH -= 6.0f;
			__xgeXuiHostDrawRect(tSel, pPicker->iSelectedColor);
		}
		if ( !bSelected && xrtDatePart(tDate) == xrtDatePart(xrtNow()) ) {
			xge_rect_t tToday = tText;
			tToday.fX += 4.0f;
			tToday.fY += 4.0f;
			tToday.fW -= 8.0f;
			tToday.fH -= 8.0f;
			__xgeXuiHostDrawBorderRect(tToday, 1.0f, pPicker->iFocusColor);
		}
		if ( pPicker->pFont != NULL ) {
			snprintf(sText, sizeof(sText), "%d", iDay);
			iText = bEnabled ? (bSelected ? XGE_COLOR_RGBA(255, 255, 255, 255) : pPicker->iTextColor) : pPicker->iDisabledTextColor;
			__xgeXuiHostDrawTextRect(pPicker->pFont, sText, tText, iText, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
}

static void __xgeXuiDatePickerDrawTimePanel(xge_xui_date_picker pPicker, int iPanel)
{
	static const char* arrNames[3] = { "Hour", "Minute", "Second" };
	xge_rect_t tRect;
	xge_rect_t tLabel;
	xge_rect_t tValue;
	int i;

	if ( (pPicker == NULL) || pPicker->tTimePanelRect[iPanel].fW <= 0.0f ) {
		return;
	}
	tRect = pPicker->tTimePanelRect[iPanel];
	__xgeXuiHostDrawRect(tRect, pPicker->iPanelColor);
	__xgeXuiHostDrawBorderRect(tRect, 1.0f, XGE_COLOR_RGBA(224, 236, 246, 255));
	if ( pPicker->pFont != NULL && __xgeXuiDatePickerIsRangeMode(pPicker->iMode) ) {
		__xgeXuiHostDrawTextRect(pPicker->pFont, (iPanel == 0) ? "Start" : "End", (xge_rect_t){ tRect.fX + 8.0f, tRect.fY + 5.0f, tRect.fW - 16.0f, 18.0f }, pPicker->iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		__xgeXuiHostDrawRect((xge_rect_t){ tRect.fX, tRect.fY + 28.0f, tRect.fW, 1.0f }, XGE_COLOR_RGBA(224, 236, 246, 255));
	}
	for ( i = 0; i < (pPicker->bShowSecond ? 3 : 2); i++ ) {
		tValue = pPicker->arrTimeRect[iPanel][i];
		if ( pPicker->pFont != NULL ) {
			tLabel = (xge_rect_t){ tValue.fX, tValue.fY - 18.0f, tValue.fW, 14.0f };
			__xgeXuiHostDrawTextRect(pPicker->pFont, arrNames[i], tLabel, pPicker->iMutedTextColor, XGE_TEXT_ALIGN_CENTER | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
		}
	}
}

int xgeXuiDatePickerInit(xge_xui_date_picker pPicker, xge_xui_context pContext, xge_xui_widget pWidget, xge_font pFont)
{
	const xge_xui_theme_t* pTheme;

	if ( (pPicker == NULL) || (pContext == NULL) || (pWidget == NULL) ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	memset(pPicker, 0, sizeof(*pPicker));
	__xgeXuiControlWidgetInit(pWidget, 1);
	pTheme = xgeXuiGetTheme(pContext);
	pPicker->pContext = pContext;
	pPicker->pWidget = pWidget;
	pPicker->pFont = (pFont != NULL) ? pFont : pTheme->pFont;
	pPicker->iMode = XGE_XUI_DATE_PICKER_MODE_DATE;
	pPicker->bShowSecond = 1;
	pPicker->iFirstDayOfWeek = 0;
	__xgeXuiDatePickerCopyText(pPicker->sFormat, sizeof(pPicker->sFormat), __xgeXuiDatePickerDefaultFormat(pPicker->iMode, pPicker->bShowSecond));
	__xgeXuiDatePickerCopyText(pPicker->sRangeSeparator, sizeof(pPicker->sRangeSeparator), " - ");
	pPicker->iPanelColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pPicker->iPopupColor = XGE_COLOR_RGBA(255, 255, 255, 255);
	pPicker->iHeaderColor = XGE_COLOR_RGBA(232, 242, 250, 255);
	pPicker->iGridColor = XGE_COLOR_RGBA(226, 236, 246, 255);
	pPicker->iTextColor = XGE_COLOR_RGBA(32, 54, 74, 255);
	pPicker->iMutedTextColor = XGE_COLOR_RGBA(112, 132, 150, 255);
	pPicker->iDisabledTextColor = XGE_COLOR_RGBA(170, 178, 188, 210);
	pPicker->iBorderColor = XGE_COLOR_RGBA(126, 166, 200, 255);
	pPicker->iFieldColor = XGE_COLOR_RGBA(248, 252, 255, 255);
	pPicker->iSelectedColor = pTheme->iAccentColor;
	pPicker->iRangeColor = XGE_COLOR_RGBA(216, 236, 250, 255);
	pPicker->iHoverColor = XGE_COLOR_RGBA(230, 244, 252, 255);
	pPicker->iFocusColor = pTheme->iStateFocus;
	pPicker->iErrorColor = XGE_COLOR_RGBA(220, 76, 76, 255);
	xgeXuiWidgetSetBackground(pWidget, pPicker->iFieldColor);
	xgeXuiWidgetSetBorder(pWidget, 1.0f, pPicker->iBorderColor);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_HOVER, pPicker->iHoverColor);
	xgeXuiWidgetSetStateBackground(pWidget, XGE_XUI_STATE_DISABLED, XGE_COLOR_RGBA(235, 240, 245, 255));
	xgeXuiWidgetSetStateBorder(pWidget, XGE_XUI_STATE_FOCUS, 1.0f, pPicker->iFocusColor);
	xgeXuiWidgetSetClip(pWidget, 0);
	xgeXuiWidgetSetEvent(pWidget, xgeXuiDatePickerEventProc, NULL);
	pWidget->procPaint = xgeXuiDatePickerPaintProc;
	pWidget->pUser = pPicker;
	if ( __xgeXuiDatePickerHostRetain(pContext) == NULL ) {
		pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pWidget, NULL, NULL);
		pWidget->procPaint = NULL;
		memset(pPicker, 0, sizeof(*pPicker));
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	__xgeXuiDatePickerEnsureValue(pPicker);
	__xgeXuiDatePickerInitViewMonth(pPicker);
	return XGE_OK;
}

void xgeXuiDatePickerUnit(xge_xui_date_picker pPicker)
{
	xge_xui_context pContext;

	if ( pPicker == NULL ) {
		return;
	}
	pContext = pPicker->pContext;
	__xgeXuiDatePickerCloseActivePopup(pPicker, 0);
	if ( pPicker->pWidget != NULL && pPicker->pWidget->pUser == pPicker ) {
		pPicker->pWidget->pUser = NULL;
		xgeXuiWidgetSetEvent(pPicker->pWidget, NULL, NULL);
		pPicker->pWidget->procPaint = NULL;
	}
	memset(pPicker, 0, sizeof(*pPicker));
	__xgeXuiDatePickerHostRelease(pContext);
}

void xgeXuiDatePickerSetMode(xge_xui_date_picker pPicker, int iMode)
{
	int bOldDefault;

	if ( pPicker == NULL ) {
		return;
	}
	bOldDefault = __xgeXuiDatePickerFormatIsDefault(pPicker);
	pPicker->iMode = __xgeXuiDatePickerClampMode(iMode);
	if ( bOldDefault ) {
		__xgeXuiDatePickerCopyText(pPicker->sFormat, sizeof(pPicker->sFormat), __xgeXuiDatePickerDefaultFormat(pPicker->iMode, pPicker->bShowSecond));
	}
	pPicker->bHasValue = 0;
	__xgeXuiDatePickerEnsureValue(pPicker);
	__xgeXuiDatePickerInitViewMonth(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

int xgeXuiDatePickerGetMode(xge_xui_date_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->iMode : XGE_XUI_DATE_PICKER_MODE_DATE;
}

void xgeXuiDatePickerSetNullable(xge_xui_date_picker pPicker, int bNullable)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->bNullable = bNullable ? 1 : 0;
	if ( pPicker->bNullable == 0 ) {
		__xgeXuiDatePickerEnsureValue(pPicker);
	}
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

int xgeXuiDatePickerGetNullable(xge_xui_date_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->bNullable : 0;
}

void xgeXuiDatePickerSetValue(xge_xui_date_picker pPicker, xtime tValue)
{
	if ( pPicker == NULL ) {
		return;
	}
	tValue = __xgeXuiDatePickerClampValue(pPicker, tValue);
	pPicker->tValue = tValue;
	pPicker->tStart = tValue;
	pPicker->tEnd = tValue;
	pPicker->bHasValue = 1;
	__xgeXuiDatePickerInitViewMonth(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

xtime xgeXuiDatePickerGetValue(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return 0;
	}
	__xgeXuiDatePickerEnsureValue(pPicker);
	return pPicker->tValue;
}

int xgeXuiDatePickerHasValue(xge_xui_date_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->bHasValue : 0;
}

void xgeXuiDatePickerClearValue(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( pPicker->bNullable ) {
		pPicker->bHasValue = 0;
	} else {
		pPicker->bHasValue = 0;
		__xgeXuiDatePickerEnsureValue(pPicker);
	}
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerSetRangeValue(xge_xui_date_picker pPicker, xtime tStart, xtime tEnd)
{
	if ( pPicker == NULL ) {
		return;
	}
	tStart = __xgeXuiDatePickerClampValue(pPicker, tStart);
	tEnd = __xgeXuiDatePickerClampValue(pPicker, tEnd);
	if ( tEnd < tStart ) {
		xtime tSwap = tStart;
		tStart = tEnd;
		tEnd = tSwap;
	}
	pPicker->tStart = tStart;
	pPicker->tEnd = tEnd;
	pPicker->tValue = tStart;
	pPicker->bHasValue = 1;
	__xgeXuiDatePickerInitViewMonth(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerGetRangeValue(xge_xui_date_picker pPicker, xtime* pStart, xtime* pEnd)
{
	if ( pPicker != NULL ) {
		__xgeXuiDatePickerEnsureValue(pPicker);
	}
	if ( pStart != NULL ) {
		*pStart = (pPicker != NULL) ? pPicker->tStart : 0;
	}
	if ( pEnd != NULL ) {
		*pEnd = (pPicker != NULL) ? pPicker->tEnd : 0;
	}
}

int xgeXuiDatePickerHasRangeValue(xge_xui_date_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->bHasValue : 0;
}

void xgeXuiDatePickerSetLimits(xge_xui_date_picker pPicker, xtime tMin, xtime tMax)
{
	if ( pPicker == NULL ) {
		return;
	}
	if ( tMax < tMin ) {
		xtime tSwap = tMin;
		tMin = tMax;
		tMax = tSwap;
	}
	pPicker->tMin = tMin;
	pPicker->tMax = tMax;
	pPicker->bHasMin = 1;
	pPicker->bHasMax = 1;
	__xgeXuiDatePickerEnsureValue(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerClearLimits(xge_xui_date_picker pPicker)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->bHasMin = 0;
	pPicker->bHasMax = 0;
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerSetFormat(xge_xui_date_picker pPicker, const char* sFormat)
{
	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerCopyText(pPicker->sFormat, sizeof(pPicker->sFormat), (sFormat != NULL && sFormat[0] != 0) ? sFormat : __xgeXuiDatePickerDefaultFormat(pPicker->iMode, pPicker->bShowSecond));
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

const char* xgeXuiDatePickerGetFormat(xge_xui_date_picker pPicker)
{
	return (pPicker != NULL) ? pPicker->sFormat : "";
}

void xgeXuiDatePickerSetRangeSeparator(xge_xui_date_picker pPicker, const char* sSeparator)
{
	if ( pPicker == NULL ) {
		return;
	}
	__xgeXuiDatePickerCopyText(pPicker->sRangeSeparator, sizeof(pPicker->sRangeSeparator), (sSeparator != NULL) ? sSeparator : " - ");
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerSetShowSecond(xge_xui_date_picker pPicker, int bShowSecond)
{
	int bOldDefault;

	if ( pPicker == NULL ) {
		return;
	}
	bOldDefault = __xgeXuiDatePickerFormatIsDefault(pPicker);
	pPicker->bShowSecond = bShowSecond ? 1 : 0;
	if ( bOldDefault ) {
		__xgeXuiDatePickerCopyText(pPicker->sFormat, sizeof(pPicker->sFormat), __xgeXuiDatePickerDefaultFormat(pPicker->iMode, pPicker->bShowSecond));
	}
	__xgeXuiDatePickerEnsureValue(pPicker);
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
}

void xgeXuiDatePickerSetFirstDayOfWeek(xge_xui_date_picker pPicker, int iFirstDayOfWeek)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iFirstDayOfWeek = (iFirstDayOfWeek == 1) ? 1 : 0;
	xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
}

void xgeXuiDatePickerSetDefaultRangeSpan(xge_xui_date_picker pPicker, xtime tSpan)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->tDefaultRangeSpan = (tSpan > 0) ? tSpan : 0;
}

void xgeXuiDatePickerSetChanging(xge_xui_date_picker pPicker, xge_xui_date_proc procChanging, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procChanging = procChanging;
		pPicker->pChangingUser = pUser;
	}
}

void xgeXuiDatePickerSetChange(xge_xui_date_picker pPicker, xge_xui_date_proc procChange, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procChange = procChange;
		pPicker->pChangeUser = pUser;
	}
}

void xgeXuiDatePickerSetCommit(xge_xui_date_picker pPicker, xge_xui_date_proc procCommit, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procCommit = procCommit;
		pPicker->pCommitUser = pUser;
	}
}

void xgeXuiDatePickerSetCancel(xge_xui_date_picker pPicker, xge_xui_date_proc procCancel, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procCancel = procCancel;
		pPicker->pCancelUser = pUser;
	}
}

void xgeXuiDatePickerSetClear(xge_xui_date_picker pPicker, xge_xui_date_proc procClear, void* pUser)
{
	if ( pPicker != NULL ) {
		pPicker->procClear = procClear;
		pPicker->pClearUser = pUser;
	}
}

void xgeXuiDatePickerSetColors(xge_xui_date_picker pPicker, uint32_t iBackground, uint32_t iPanel, uint32_t iHeader, uint32_t iGrid, uint32_t iText, uint32_t iSelected)
{
	if ( pPicker == NULL ) {
		return;
	}
	pPicker->iFieldColor = iBackground;
	pPicker->iPanelColor = iPanel;
	pPicker->iPopupColor = iPanel;
	pPicker->iHeaderColor = iHeader;
	pPicker->iGridColor = iGrid;
	pPicker->iTextColor = iText;
	pPicker->iSelectedColor = iSelected;
	xgeXuiWidgetSetBackground(pPicker->pWidget, iBackground);
	xgeXuiWidgetSetBorder(pPicker->pWidget, 1.0f, pPicker->iBorderColor);
	if ( __xgeXuiDatePickerPanelForPicker(pPicker) != NULL ) {
		xge_xui_date_picker_popup_panel_t* pPanel = __xgeXuiDatePickerPanelForPicker(pPicker);
		xgeXuiPopupSetBackground(&pPanel->tPopup, iPanel);
		xgeXuiPopupSetBorder(&pPanel->tPopup, pPicker->iBorderColor);
		__xgeXuiDatePickerPanelStyleHeaderControls(pPanel);
	}
	xgeXuiWidgetMarkPaint(pPicker->pWidget);
	if ( pPicker->pPopupWidget != NULL ) {
		xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
	}
}

int xgeXuiDatePickerIsPopupOpen(xge_xui_date_picker pPicker)
{
	xge_xui_date_picker_popup_panel_t* pPanel;

	pPanel = __xgeXuiDatePickerPanelForPicker(pPicker);
	return (pPanel != NULL) ? xgeXuiPopupIsOpen(&pPanel->tPopup) : 0;
}

int xgeXuiDatePickerEvent(xge_xui_date_picker pPicker, const xge_event_t* pEvent)
{
	int iInside;

	if ( (pPicker == NULL) || (pPicker->pWidget == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiDatePickerLayoutMain(pPicker);
	if ( (pPicker->pWidget->iFlags & XGE_XUI_WIDGET_VISIBLE) == 0 || (pPicker->pWidget->iFlags & XGE_XUI_WIDGET_ENABLED) == 0 ) {
		xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_DISABLED);
		return XGE_XUI_EVENT_CONTINUE;
	}
	iInside = __xgeXuiRectContains(pPicker->pWidget->tRect, pEvent->fX, pEvent->fY);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
			pPicker->iState = iInside ? XGE_XUI_STATE_HOVER : XGE_XUI_STATE_NORMAL;
			xgeXuiWidgetSetVisualState(pPicker->pWidget, pPicker->iState);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_ENTER:
			pPicker->iState = XGE_XUI_STATE_HOVER;
			xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_HOVER);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_POINTER_LEAVE:
			pPicker->iState = XGE_XUI_STATE_NORMAL;
			xgeXuiWidgetSetVisualState(pPicker->pWidget, XGE_XUI_STATE_NORMAL);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_XUI_FOCUS_IN:
		case XGE_EVENT_XUI_FOCUS_OUT:
			xgeXuiWidgetMarkPaint(pPicker->pWidget);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( iInside == 0 ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pWidget);
			__xgeXuiDatePickerOpenPopup(pPicker, xgeXuiDatePickerIsPopupOpen(pPicker) == 0);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( (pPicker->pContext == NULL) || (pPicker->pContext->pFocus != pPicker->pWidget) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			if ( (pEvent->iParam1 == XGE_KEY_ENTER) || (pEvent->iParam1 == XGE_KEY_SPACE) || (pEvent->iParam1 == XGE_KEY_DOWN) ) {
				__xgeXuiDatePickerOpenPopup(pPicker, 1);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

int xgeXuiDatePickerEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	return xgeXuiDatePickerEvent((xge_xui_date_picker)pUser, pEvent);
}

int xgeXuiDatePickerPopupEventProc(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;
	int iPart;
	int iPanel;
	int iIndex;
	xtime tDate;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPanel = (xge_xui_date_picker_popup_panel_t*)pUser;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( (pPicker == NULL) || (pEvent == NULL) ) {
		return XGE_XUI_EVENT_CONTINUE;
	}
	__xgeXuiDatePickerLayoutPopup(pPicker);
	__xgeXuiDatePickerPanelLayoutControls(pPanel);
	switch ( pEvent->iType ) {
		case XGE_EVENT_MOUSE_MOVE:
		case XGE_EVENT_TOUCH_MOVE:
			iPart = __xgeXuiDatePickerHitPopup(pPicker, pEvent->fX, pEvent->fY, &iPanel, &iIndex);
			pPicker->iHoverPart = (iPart == XGE_XUI_DATE_PICKER_PART_DAY) ? iPart : XGE_XUI_DATE_PICKER_PART_NONE;
			pPicker->iActivePanel = iPanel;
			pPicker->iHoverDayPanel = (iPart == XGE_XUI_DATE_PICKER_PART_DAY) ? iPanel : -1;
			pPicker->iHoverDayCell = (iPart == XGE_XUI_DATE_PICKER_PART_DAY) ? iIndex : -1;
			xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
			return XGE_XUI_EVENT_CONTINUE;
		case XGE_EVENT_MOUSE_DOWN:
		case XGE_EVENT_TOUCH_BEGIN:
			if ( !__xgeXuiRectContains(pPicker->pPopupWidget->tRect, pEvent->fX, pEvent->fY) ) {
				return XGE_XUI_EVENT_CONTINUE;
			}
			xgeXuiSetFocus(pPicker->pContext, pPicker->pPopupWidget);
			iPart = __xgeXuiDatePickerHitPopup(pPicker, pEvent->fX, pEvent->fY, &iPanel, &iIndex);
			pPicker->iActivePart = iPart;
			pPicker->iActivePanel = iPanel;
			switch ( iPart ) {
				case XGE_XUI_DATE_PICKER_PART_DAY:
					if ( __xgeXuiDatePickerCellDate(pPicker, iPanel, iIndex, &tDate) ) {
						__xgeXuiDatePickerSelectDate(pPicker, iPanel, tDate);
					}
					return XGE_XUI_EVENT_CONSUMED;
				default:
					return XGE_XUI_EVENT_CONTINUE;
			}
		case XGE_EVENT_MOUSE_UP:
		case XGE_EVENT_TOUCH_END:
		case XGE_EVENT_TOUCH_CANCEL:
			pPicker->iActivePart = XGE_XUI_DATE_PICKER_PART_NONE;
			xgeXuiWidgetMarkPaint(pPicker->pPopupWidget);
			return XGE_XUI_EVENT_CONSUMED;
		case XGE_EVENT_KEY_DOWN:
			if ( pEvent->iParam1 == XGE_KEY_ESCAPE ) {
				__xgeXuiDatePickerCloseActivePopup(pPicker, 0);
				return XGE_XUI_EVENT_CONSUMED;
			}
			if ( pEvent->iParam1 == XGE_KEY_ENTER ) {
				__xgeXuiDatePickerCommitDraft(pPicker);
				return XGE_XUI_EVENT_CONSUMED;
			}
			return XGE_XUI_EVENT_CONTINUE;
		default:
			return XGE_XUI_EVENT_CONTINUE;
	}
}

void xgeXuiDatePickerPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_date_picker pPicker;
	char sText[256];
	uint32_t iTextColor;
	uint32_t iArrowColor;
	xge_rect_t tTextRect;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPicker = (xge_xui_date_picker)pUser;
	if ( (pPicker == NULL) || (pWidget == NULL) ) {
		return;
	}
	__xgeXuiDatePickerLayoutMain(pPicker);
	__xgeXuiDatePickerFormatDisplay(pPicker, sText, sizeof(sText));
	iTextColor = ((pWidget->iVisualState & XGE_XUI_STATE_DISABLED) != 0) ? pPicker->iDisabledTextColor : pPicker->iTextColor;
	iArrowColor = ((pWidget->iVisualState & XGE_XUI_STATE_DISABLED) != 0) ? pPicker->iDisabledTextColor : pPicker->iTextColor;
	tTextRect = pPicker->tFieldRect;
	tTextRect.fX += 8.0f;
	tTextRect.fW -= 12.0f;
	if ( pPicker->pFont != NULL ) {
		__xgeXuiHostDrawTextRect(pPicker->pFont, sText, tTextRect, iTextColor, XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE | XGE_TEXT_CLIP);
	}
	if ( xgeXuiDatePickerIsPopupOpen(pPicker) || (pWidget->iVisualState & XGE_XUI_STATE_HOVER) != 0 ) {
		__xgeXuiHostDrawRect(pPicker->tButtonRect, xgeXuiDatePickerIsPopupOpen(pPicker) ? XGE_COLOR_RGBA(224, 238, 248, 255) : XGE_COLOR_RGBA(238, 246, 252, 255));
	}
	__xgeXuiDatePickerDrawChevron(pPicker->tButtonRect, iArrowColor, xgeXuiDatePickerIsPopupOpen(pPicker));
}

void xgeXuiDatePickerPopupPaintProc(xge_xui_widget pWidget, void* pUser)
{
	xge_xui_date_picker_popup_panel_t* pPanel;
	xge_xui_date_picker pPicker;
	int i;

	if ( pUser == NULL && pWidget != NULL ) {
		pUser = pWidget->pUser;
	}
	pPanel = (xge_xui_date_picker_popup_panel_t*)pUser;
	pPicker = (pPanel != NULL) ? pPanel->pPicker : NULL;
	if ( (pPicker == NULL) || (pWidget == NULL) ) {
		return;
	}
	__xgeXuiDatePickerLayoutPopup(pPicker);
	__xgeXuiDatePickerPanelLayoutControls(pPanel);
	__xgeXuiHostDrawRect(pWidget->tContentRect, pPicker->iPopupColor);
	__xgeXuiHostDrawRect((xge_rect_t){ pWidget->tContentRect.fX, pPicker->arrFooterRect[3].fY - 10.0f, pWidget->tContentRect.fW, 1.0f }, XGE_COLOR_RGBA(220, 233, 244, 255));
	for ( i = 0; i < __xgeXuiDatePickerPanelCount(pPicker->iMode); i++ ) {
		if ( __xgeXuiDatePickerHasDatePanel(pPicker->iMode) ) {
			__xgeXuiDatePickerDrawCalendar(pPicker, i);
		}
		if ( __xgeXuiDatePickerHasTimePanel(pPicker->iMode) ) {
			__xgeXuiDatePickerDrawTimePanel(pPicker, i);
		}
	}
	if ( __xgeXuiDatePickerPanelCount(pPicker->iMode) > 1 ) {
		xge_rect_t tLeft = (pPicker->tPanelRect[0].fW > 0.0f) ? pPicker->tPanelRect[0] : pPicker->tTimePanelRect[0];
		float fX = tLeft.fX + tLeft.fW + 4.0f;
		float fY = pWidget->tContentRect.fY + 12.0f;
		float fH = pPicker->arrFooterRect[3].fY - fY - 12.0f;
		if ( fH > 1.0f ) {
			__xgeXuiHostDrawRect((xge_rect_t){ fX, fY, 1.0f, fH }, XGE_COLOR_RGBA(224, 236, 246, 255));
		}
	}
}
