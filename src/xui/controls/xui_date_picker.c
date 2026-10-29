#include "xui_config.h"
#if XUI_ENABLE_DATE_PICKER
#include "xui_internal.h"
#include "xui_pointer_press_internal.h"

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define XUI_DATE_PICKER_PAD		12.0f
#define XUI_DATE_PICKER_TIME_INSET	12.0f
#define XUI_DATE_PICKER_TIME_MIN_HEIGHT	84.0f
#define XUI_DATE_PICKER_CELL_COUNT	42
#define XUI_DATE_PICKER_CLOSE_NONE	0
#define XUI_DATE_PICKER_CLOSE_SILENT	1
#define XUI_DATE_PICKER_CLOSE_CANCEL	2
#define XUI_DATE_PICKER_CLOSE_COMMIT	3
#define XUI_DATE_PICKER_CLOSE_CLEAR	4
#define XUI_DATE_PICKER_PART_YEAR_FIELD	100
#define XUI_DATE_PICKER_PART_MONTH_FIELD	101
#define XUI_DATE_PICKER_PART_YEAR_OPTION	102
#define XUI_DATE_PICKER_PART_MONTH_OPTION	103
#define XUI_DATE_PICKER_EDIT_NONE	0
#define XUI_DATE_PICKER_EDIT_YEAR	1
#define XUI_DATE_PICKER_EDIT_TIME	2
#define XUI_DATE_PICKER_COMBO_NONE	0
#define XUI_DATE_PICKER_COMBO_YEAR	1
#define XUI_DATE_PICKER_COMBO_MONTH	2
#define XUI_DATE_PICKER_YEAR_OPTION_COUNT	9
#define XUI_DATE_PICKER_YEAR_INPUT_LIMIT	6
#define XUI_DATE_PICKER_YEAR_MIN		1LL
#define XUI_DATE_PICKER_YEAR_MAX		294246LL
#define XUI_DATE_PICKER_YEAR_FIELD_W		112.0f
#define XUI_DATE_PICKER_MONTH_FIELD_W		62.0f
#define XUI_DATE_PICKER_HEADER_COMBO_GAP	6.0f
#define XUI_DATE_PICKER_MIN_POPUP_W		268.0f
#define XUI_DATE_PICKER_MIN_NULLABLE_W		324.0f
#define XUI_DATE_PICKER_MIN_DATE_H		290.0f
#define XUI_DATE_PICKER_MIN_TIME_H		148.0f
#define XUI_DATE_PICKER_MIN_DATETIME_RANGE_H	386.0f

typedef struct xui_date_picker_data_t {
	xui_widget pPopup;
	xui_widget pPanel;
	xui_font pFont;
	xui_date_picker_proc onChanging;
	xui_date_picker_proc onChange;
	xui_date_picker_proc onCommit;
	xui_date_picker_proc onCancel;
	xui_date_picker_proc onClear;
	void* pChangingUser;
	void* pChangeUser;
	void* pCommitUser;
	void* pCancelUser;
	void* pClearUser;
	int iMode;
	int bNullable;
	int bHasValue;
	int bHasMin;
	int bHasMax;
	int bShowSecond;
	int iFirstDayOfWeek;
	int iPopupPlacement;
	int bPopupSizeCustom;
	int bFormatCustom;
	int bClosingExplicit;
	int iCloseReason;
	int iHoverPart;
	int iActivePart;
	int iHoverPanel;
	int iHoverIndex;
	int iActivePanel;
	int iPressedPanel;
	int iActiveIndex;
	xui_pointer_owner_t tPointerOwner;
	int iActiveTimePanel;
	int iActiveTimeField;
	int iOpenComboKind;
	int iOpenComboPanel;
	int iEditKind;
	int iEditPanel;
	int iEditField;
	int iEditSelectAll;
	int iEditError;
	int iChangingCount;
	int iChangeCount;
	int iCommitCount;
	int iCancelCount;
	int iClearCount;
	xtime tValue;
	xtime tStart;
	xtime tEnd;
	xtime tDraftValue;
	xtime tDraftStart;
	xtime tDraftEnd;
	xtime tViewMonth[XUI_DATE_PICKER_PANEL_CAPACITY];
	xtime tMin;
	xtime tMax;
	xtime tDefaultRangeSpan;
	xui_rect_t tButtonRect;
	xui_rect_t tTextRect;
	xui_rect_t tCalendarRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tTimePanelRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tHeaderRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tPrevRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tNextRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tMonthLabelRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tYearRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t tMonthRect[XUI_DATE_PICKER_PANEL_CAPACITY];
	xui_rect_t arrDayRect[XUI_DATE_PICKER_PANEL_CAPACITY][XUI_DATE_PICKER_CELL_COUNT];
	xtime arrDayValue[XUI_DATE_PICKER_PANEL_CAPACITY][XUI_DATE_PICKER_CELL_COUNT];
	xui_rect_t arrTimeRect[XUI_DATE_PICKER_PANEL_CAPACITY][3];
	xui_rect_t arrFooterRect[4];
	char sEdit[16];
	int iEditLen;
	char sText[216];
	char sFormat[64];
	char sRangeSeparator[24];
	float fPopupWidth;
	float fPopupHeight;
	float fBorderWidth;
	uint32_t iTextColor;
	uint32_t iDisabledTextColor;
	uint32_t iBackgroundColor;
	uint32_t iHoverBackgroundColor;
	uint32_t iOpenBackgroundColor;
	uint32_t iDisabledBackgroundColor;
	uint32_t iBorderColor;
	uint32_t iHoverBorderColor;
	uint32_t iFocusBorderColor;
	uint32_t iArrowColor;
	uint32_t iDisabledArrowColor;
	uint32_t iPopupPanelColor;
	uint32_t iPopupBorderColor;
	uint32_t iPopupShadowColor;
	uint32_t iPopupTextColor;
	uint32_t iPopupMutedTextColor;
	uint32_t iAccentColor;
	uint32_t iFieldColor;
	uint32_t iFieldBorderColor;
	uint32_t iSelectedTextColor;
	uint32_t iDisabledDayColor;
	uint32_t iSeparatorColor;
	uint32_t iButtonColor;
	uint32_t iButtonHoverColor;
	uint32_t iButtonOpenColor;
	uint32_t iButtonDisabledColor;
	uint32_t iCalendarColor;
	uint32_t iTimePanelColor;
	uint32_t iRangeColor;
	uint32_t iSelectionColor;
	uint32_t iErrorBorderColor;
	uint32_t iOptionHoverColor;
	uint32_t iChildStyleHash;
	int bChildStyleSynced;
} xui_date_picker_data_t;

static xui_date_picker_data_t* __xuiDatePickerGetData(xui_widget pWidget);
static int __xuiDatePickerApplyPopupStyle(xui_widget pWidget, xui_date_picker_data_t* pData);
static int __xuiDatePickerPanelCancelPointer(xui_widget pPanel, xui_date_picker_data_t* pData);
static void __xuiDatePickerCancelEdit(xui_date_picker_data_t* pData);
static void __xuiDatePickerPreferredPopupSize(int iMode, float* pW, float* pH);

static int __xuiDatePickerAlive(xui_context pContext, xui_widget pWidget,
	const xui_date_picker_data_t* pExpected)
{
	return xuiInternalContextIsValid(pContext) && !xuiInternalContextDestroyPending(pContext) &&
		xuiInternalWidgetIsValid(pWidget) && __xuiDatePickerGetData(pWidget) == pExpected;
}

static uint32_t __xuiDatePickerAlpha(uint32_t iColor)
{
	return iColor & 0xffu;
}

static float __xuiDatePickerMax(float fA, float fB)
{
	return (fA > fB) ? fA : fB;
}

static float __xuiDatePickerMin(float fA, float fB)
{
	return (fA < fB) ? fA : fB;
}

static int __xuiDatePickerMetricValid(float fValue)
{
	return fValue >= 0.0f && fValue <= XUI_LAYOUT_UNBOUNDED;
}

static void __xuiDatePickerIncrement(int* pValue)
{
	if ( (pValue != NULL) && (*pValue < INT_MAX) ) (*pValue)++;
}

static int __xuiDatePickerClampInt(int iValue, int iMin, int iMax)
{
	if ( iValue < iMin ) return iMin;
	if ( iValue > iMax ) return iMax;
	return iValue;
}

static int64 __xuiDatePickerClampYear(int64 iYear)
{
	if ( iYear < XUI_DATE_PICKER_YEAR_MIN ) return XUI_DATE_PICKER_YEAR_MIN;
	if ( iYear > XUI_DATE_PICKER_YEAR_MAX ) return XUI_DATE_PICKER_YEAR_MAX;
	return iYear;
}

static int __xuiDatePickerParseYearText(const char* sText, int64* pYear)
{
	int64 iValue;
	int i;

	if ( sText == NULL || sText[0] == '\0' ) return 0;
	iValue = 0;
	for ( i = 0; sText[i] != '\0'; i++ ) {
		if ( sText[i] < '0' || sText[i] > '9' ) return 0;
		iValue = iValue * 10 + (int64)(sText[i] - '0');
		if ( iValue > XUI_DATE_PICKER_YEAR_MAX ) return 0;
	}
	if ( iValue < XUI_DATE_PICKER_YEAR_MIN ) return 0;
	if ( pYear != NULL ) *pYear = iValue;
	return 1;
}

static int __xuiDatePickerTimeFieldMax(int iField)
{
	return (iField == 0) ? 23 : 59;
}

static void __xuiDatePickerClampTimeEditText(xui_date_picker_data_t* pData)
{
	int iValue;
	int iClamped;
	int iMax;

	if ( (pData == NULL) || (pData->iEditKind != XUI_DATE_PICKER_EDIT_TIME) || (pData->iEditLen <= 0) ) {
		return;
	}
	iMax = __xuiDatePickerTimeFieldMax(pData->iEditField);
	iValue = atoi(pData->sEdit);
	iClamped = __xuiDatePickerClampInt(iValue, 0, iMax);
	if ( iClamped != iValue ) {
		snprintf(pData->sEdit, sizeof(pData->sEdit), "%02d", iClamped);
		pData->iEditLen = (int)strlen(pData->sEdit);
	}
}

static int __xuiDatePickerRectContains(xui_rect_t tRect, float fX, float fY)
{
	return (tRect.fW > 0.0f) && (tRect.fH > 0.0f) &&
		(fX >= tRect.fX) && (fY >= tRect.fY) &&
		(fX < tRect.fX + tRect.fW) && (fY < tRect.fY + tRect.fH);
}

static xui_rect_t __xuiDatePickerInsetRect(xui_rect_t tRect, float fInset)
{
	return xuiInternalRectFromFloatNearest(tRect.fX + fInset, tRect.fY + fInset,
		tRect.fW - fInset * 2.0f, tRect.fH - fInset * 2.0f);
}

static int __xuiDatePickerEditing(const xui_date_picker_data_t* pData)
{
	return (pData != NULL) && (pData->iEditKind != XUI_DATE_PICKER_EDIT_NONE);
}

static int __xuiDatePickerModeValid(int iMode)
{
	return (iMode >= XUI_DATE_PICKER_MODE_DATE) && (iMode <= XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xuiDatePickerPlacementValid(int iPlacement)
{
	return (iPlacement == XUI_DATE_PICKER_POPUP_AUTO) || (iPlacement == XUI_DATE_PICKER_POPUP_BOTTOM) || (iPlacement == XUI_DATE_PICKER_POPUP_TOP);
}

static int __xuiDatePickerIsRange(int iMode)
{
	return (iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_TIME_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xuiDatePickerHasDate(int iMode)
{
	return (iMode == XUI_DATE_PICKER_MODE_DATE) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xuiDatePickerHasTime(int iMode)
{
	return (iMode == XUI_DATE_PICKER_MODE_TIME) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME) ||
	       (iMode == XUI_DATE_PICKER_MODE_TIME_RANGE) ||
	       (iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE);
}

static int __xuiDatePickerPanelCount(int iMode)
{
	return __xuiDatePickerIsRange(iMode) ? 2 : 1;
}

static void __xuiDatePickerMinimumPopupSize(int iMode, int bNullable, float* pW, float* pH)
{
	float fW;
	float fH;

	fW = XUI_DATE_PICKER_MIN_POPUP_W;
	fH = XUI_DATE_PICKER_MIN_DATE_H;
	if ( iMode == XUI_DATE_PICKER_MODE_TIME || iMode == XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		fH = XUI_DATE_PICKER_MIN_TIME_H;
	} else if ( iMode == XUI_DATE_PICKER_MODE_DATETIME ) {
		fW = 420.0f;
	} else if ( iMode == XUI_DATE_PICKER_MODE_DATE_RANGE ) {
		fW = 380.0f;
	} else if ( iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE ) {
		fW = 380.0f;
		fH = XUI_DATE_PICKER_MIN_DATETIME_RANGE_H;
	}
	if ( bNullable && fW < XUI_DATE_PICKER_MIN_NULLABLE_W ) fW = XUI_DATE_PICKER_MIN_NULLABLE_W;
	if ( pW != NULL ) *pW = fW;
	if ( pH != NULL ) *pH = fH;
}

static int __xuiDatePickerPopupSizeValid(int iMode, int bNullable, float fWidth, float fHeight)
{
	float fMinWidth;
	float fMinHeight;
	float fDefaultWidth;
	float fDefaultHeight;

	if ( !__xuiDatePickerMetricValid(fWidth) || !__xuiDatePickerMetricValid(fHeight) ) return 0;
	__xuiDatePickerMinimumPopupSize(iMode, bNullable, &fMinWidth, &fMinHeight);
	__xuiDatePickerPreferredPopupSize(iMode, &fDefaultWidth, &fDefaultHeight);
	if ( fDefaultWidth < fMinWidth ) fDefaultWidth = fMinWidth;
	if ( fDefaultHeight < fMinHeight ) fDefaultHeight = fMinHeight;
	if ( fWidth <= 0.0f ) fWidth = fDefaultWidth;
	if ( fHeight <= 0.0f ) fHeight = fDefaultHeight;
	if ( fWidth < fMinWidth || fHeight < fMinHeight ) return 0;
	return 1;
}

static xtime __xuiDatePickerLimitValueForMode(int iMode, xtime tValue)
{
	if ( iMode == XUI_DATE_PICKER_MODE_TIME || iMode == XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		return xrtTimePart(tValue);
	}
	return tValue;
}

static int __xuiDatePickerLimitsValid(int iMode, int bHasMin, xtime tMin, int bHasMax, xtime tMax)
{
	return !bHasMin || !bHasMax ||
		__xuiDatePickerLimitValueForMode(iMode, tMin) <= __xuiDatePickerLimitValueForMode(iMode, tMax);
}

static void __xuiDatePickerCopyText(char* sDst, int iDstSize, const char* sSrc)
{
	if ( (sDst == NULL) || (iDstSize <= 0) ) return;
	if ( sSrc == NULL ) sSrc = "";
	snprintf(sDst, (size_t)iDstSize, "%s", sSrc);
	sDst[iDstSize - 1] = '\0';
	xuiInternalUtf8TrimIncomplete(sDst);
}

static const char* __xuiDatePickerDefaultFormat(int iMode, int bShowSecond)
{
	switch ( iMode ) {
	case XUI_DATE_PICKER_MODE_TIME:
	case XUI_DATE_PICKER_MODE_TIME_RANGE:
		return bShowSecond ? "hh:nn:ss" : "hh:nn";
	case XUI_DATE_PICKER_MODE_DATETIME:
	case XUI_DATE_PICKER_MODE_DATETIME_RANGE:
		return bShowSecond ? "yyyy-mm-dd hh:nn:ss" : "yyyy-mm-dd hh:nn";
	case XUI_DATE_PICKER_MODE_DATE:
	case XUI_DATE_PICKER_MODE_DATE_RANGE:
	default:
		return "yyyy-mm-dd";
	}
}

static void __xuiDatePickerSyncDefaultFormat(xui_date_picker_data_t* pData)
{
	if ( (pData != NULL) && !pData->bFormatCustom ) {
		__xuiDatePickerCopyText(pData->sFormat, (int)sizeof(pData->sFormat), __xuiDatePickerDefaultFormat(pData->iMode, pData->bShowSecond));
	}
}

static xtime __xuiDatePickerMakeTime(int iHour, int iMinute, int iSecond, int bShowSecond)
{
	iHour = __xuiDatePickerClampInt(iHour, 0, 23);
	iMinute = __xuiDatePickerClampInt(iMinute, 0, 59);
	iSecond = bShowSecond ? __xuiDatePickerClampInt(iSecond, 0, 59) : 0;
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond * XRT_TIME_SECOND;
}

static xtime __xuiDatePickerNormalizeTimePart(const xui_date_picker_data_t* pData, xtime tValue)
{
	int iHour;
	int iMinute;
	int iSecond;

	xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	return __xuiDatePickerMakeTime(iHour, iMinute, iSecond, (pData != NULL) ? pData->bShowSecond : 0);
}

static xtime __xuiDatePickerNormalizeValue(const xui_date_picker_data_t* pData, xtime tValue)
{
	if ( pData == NULL ) return tValue;
	if ( (pData->iMode == XUI_DATE_PICKER_MODE_DATE) || (pData->iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ) {
		return xrtDatePart(tValue);
	}
	if ( (pData->iMode == XUI_DATE_PICKER_MODE_TIME) || (pData->iMode == XUI_DATE_PICKER_MODE_TIME_RANGE) ) {
		return __xuiDatePickerNormalizeTimePart(pData, tValue);
	}
	return xrtDatePart(tValue) + __xuiDatePickerNormalizeTimePart(pData, tValue);
}

static xtime __xuiDatePickerLimitValue(const xui_date_picker_data_t* pData, xtime tValue)
{
	return __xuiDatePickerLimitValueForMode((pData != NULL) ? pData->iMode : XUI_DATE_PICKER_MODE_DATE, tValue);
}

static xtime __xuiDatePickerClampValue(const xui_date_picker_data_t* pData, xtime tValue)
{
	xtime tCompare;

	if ( pData == NULL ) return tValue;
	tValue = __xuiDatePickerNormalizeValue(pData, tValue);
	tCompare = __xuiDatePickerLimitValue(pData, tValue);
	if ( pData->bHasMin && (tCompare < __xuiDatePickerLimitValue(pData, pData->tMin)) ) {
		tValue = ((pData->iMode == XUI_DATE_PICKER_MODE_TIME) || (pData->iMode == XUI_DATE_PICKER_MODE_TIME_RANGE)) ?
			__xuiDatePickerNormalizeValue(pData, __xuiDatePickerLimitValue(pData, pData->tMin)) :
			__xuiDatePickerNormalizeValue(pData, pData->tMin);
		tCompare = __xuiDatePickerLimitValue(pData, tValue);
	}
	if ( pData->bHasMax && (tCompare > __xuiDatePickerLimitValue(pData, pData->tMax)) ) {
		tValue = ((pData->iMode == XUI_DATE_PICKER_MODE_TIME) || (pData->iMode == XUI_DATE_PICKER_MODE_TIME_RANGE)) ?
			__xuiDatePickerNormalizeValue(pData, __xuiDatePickerLimitValue(pData, pData->tMax)) :
			__xuiDatePickerNormalizeValue(pData, pData->tMax);
	}
	return __xuiDatePickerNormalizeValue(pData, tValue);
}

static int __xuiDatePickerValueEnabled(const xui_date_picker_data_t* pData, xtime tValue)
{
	xtime tCompare;

	if ( pData == NULL ) return 0;
	tCompare = __xuiDatePickerLimitValue(pData, __xuiDatePickerNormalizeValue(pData, tValue));
	if ( pData->bHasMin && (tCompare < __xuiDatePickerLimitValue(pData, pData->tMin)) ) return 0;
	if ( pData->bHasMax && (tCompare > __xuiDatePickerLimitValue(pData, pData->tMax)) ) return 0;
	return 1;
}

static xtime __xuiDatePickerDefaultSpan(const xui_date_picker_data_t* pData)
{
	if ( (pData != NULL) && (pData->tDefaultRangeSpan > 0) ) return pData->tDefaultRangeSpan;
	if ( (pData != NULL) && (pData->iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ) return XRT_TIME_DAY;
	return XRT_TIME_HOUR * 4;
}

static xtime __xuiDatePickerAddSpan(const xui_date_picker_data_t* pData, xtime tValue, xtime tSpan)
{
	xtime tDelta;
	xtime tEnd;

	tDelta = (tSpan > 0) ? tSpan : ((pData != NULL && pData->iMode == XUI_DATE_PICKER_MODE_DATE_RANGE) ?
		XRT_TIME_DAY : (XRT_TIME_HOUR * 4));
	if ( pData == NULL ) {
		return (tValue > INT64_MAX - tDelta) ? tValue : tValue + tDelta;
	}
	if ( pData->iMode == XUI_DATE_PICKER_MODE_DATE_RANGE ) {
		tEnd = (tValue > INT64_MAX - tDelta) ? tValue : tValue + tDelta;
		return xrtDatePart(tEnd);
	}
	if ( pData->iMode == XUI_DATE_PICKER_MODE_TIME_RANGE ) {
		tEnd = xrtTimePart(tValue);
		if ( tDelta >= XRT_TIME_DAY - tEnd ) tEnd = XRT_TIME_DAY - 1;
		else tEnd += tDelta;
		return __xuiDatePickerNormalizeValue(pData, tEnd);
	}
	tEnd = (tValue > INT64_MAX - tDelta) ? tValue : tValue + tDelta;
	return __xuiDatePickerNormalizeValue(pData, tEnd);
}

static void __xuiDatePickerDefaultValues(xui_date_picker_data_t* pData, xtime* pStart, xtime* pEnd)
{
	xtime tNow;
	xtime tStart;
	xtime tEnd;

	tNow = xrtNow();
	if ( pData == NULL ) {
		if ( pStart != NULL ) *pStart = tNow;
		if ( pEnd != NULL ) *pEnd = tNow;
		return;
	}
	switch ( pData->iMode ) {
	case XUI_DATE_PICKER_MODE_DATE:
		tStart = xrtDatePart(tNow);
		tEnd = tStart;
		break;
	case XUI_DATE_PICKER_MODE_TIME:
		tStart = __xuiDatePickerNormalizeValue(pData, xrtTimePart(tNow));
		tEnd = tStart;
		break;
	case XUI_DATE_PICKER_MODE_DATE_RANGE:
		tStart = xrtDatePart(tNow);
		tEnd = __xuiDatePickerAddSpan(pData, tStart, __xuiDatePickerDefaultSpan(pData));
		break;
	case XUI_DATE_PICKER_MODE_TIME_RANGE:
		tStart = __xuiDatePickerNormalizeValue(pData, xrtTimePart(tNow));
		tEnd = __xuiDatePickerAddSpan(pData, tStart, __xuiDatePickerDefaultSpan(pData));
		break;
	case XUI_DATE_PICKER_MODE_DATETIME_RANGE:
		tStart = __xuiDatePickerNormalizeValue(pData, tNow);
		tEnd = __xuiDatePickerAddSpan(pData, tStart, __xuiDatePickerDefaultSpan(pData));
		break;
	case XUI_DATE_PICKER_MODE_DATETIME:
	default:
		tStart = __xuiDatePickerNormalizeValue(pData, tNow);
		tEnd = tStart;
		break;
	}
	tStart = __xuiDatePickerClampValue(pData, tStart);
	tEnd = __xuiDatePickerClampValue(pData, tEnd);
	if ( __xuiDatePickerIsRange(pData->iMode) && (tEnd < tStart) ) tEnd = tStart;
	if ( pStart != NULL ) *pStart = tStart;
	if ( pEnd != NULL ) *pEnd = tEnd;
}

static void __xuiDatePickerNormalizeRange(xui_date_picker_data_t* pData, xtime* pStart, xtime* pEnd)
{
	xtime tSwap;

	if ( (pData == NULL) || (pStart == NULL) || (pEnd == NULL) ) return;
	*pStart = __xuiDatePickerClampValue(pData, *pStart);
	*pEnd = __xuiDatePickerClampValue(pData, *pEnd);
	if ( *pEnd < *pStart ) {
		tSwap = *pStart;
		*pStart = *pEnd;
		*pEnd = tSwap;
	}
}

static void __xuiDatePickerEnsureValue(xui_date_picker_data_t* pData)
{
	xtime tStart;
	xtime tEnd;

	if ( pData == NULL ) return;
	if ( pData->bNullable && !pData->bHasValue ) return;
	if ( pData->bHasValue ) {
		if ( __xuiDatePickerIsRange(pData->iMode) ) {
			__xuiDatePickerNormalizeRange(pData, &pData->tStart, &pData->tEnd);
			pData->tValue = pData->tStart;
		} else {
			pData->tValue = __xuiDatePickerClampValue(pData, pData->tValue);
			pData->tStart = pData->tValue;
			pData->tEnd = pData->tValue;
		}
		return;
	}
	__xuiDatePickerDefaultValues(pData, &tStart, &tEnd);
	pData->tValue = tStart;
	pData->tStart = tStart;
	pData->tEnd = tEnd;
	pData->bHasValue = 1;
}

static void __xuiDatePickerInitViewMonth(xui_date_picker_data_t* pData)
{
	xtime tStart;
	xtime tEnd;
	int64 iYear;
	int iMonth;

	if ( pData == NULL ) return;
	__xuiDatePickerEnsureValue(pData);
	if ( !__xuiDatePickerHasDate(pData->iMode) ) {
		pData->tViewMonth[0] = xuiXrtFirstDayOfMonth(xrtNow());
		pData->tViewMonth[1] = xuiXrtDateAddMonth(0, 1, pData->tViewMonth[0]);
		return;
	}
	if ( pData->bHasValue ) {
		tStart = __xuiDatePickerIsRange(pData->iMode) ? pData->tStart : pData->tValue;
		tEnd = __xuiDatePickerIsRange(pData->iMode) ? pData->tEnd : tStart;
	} else {
		__xuiDatePickerDefaultValues(pData, &tStart, &tEnd);
	}
	xuiXrtDecodeSerial(tStart, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	pData->tViewMonth[0] = xuiXrtDateSerial(iYear, iMonth, 1);
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		xuiXrtDecodeSerial(tEnd, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		pData->tViewMonth[1] = xuiXrtDateSerial(iYear, iMonth, 1);
		if ( pData->tViewMonth[1] <= pData->tViewMonth[0] ) {
			pData->tViewMonth[1] = xuiXrtDateAddMonth(0, 1, pData->tViewMonth[0]);
		}
	} else {
		pData->tViewMonth[1] = xuiXrtDateAddMonth(0, 1, pData->tViewMonth[0]);
	}
}

static void __xuiDatePickerAppend(char* sOut, int* pLen, int iCap, const char* sText)
{
	int iLen;

	if ( (sOut == NULL) || (pLen == NULL) || (sText == NULL) || (iCap <= 0) ) return;
	iLen = (int)strlen(sText);
	if ( *pLen + iLen >= iCap ) iLen = iCap - *pLen - 1;
	if ( iLen <= 0 ) return;
	memcpy(sOut + *pLen, sText, (size_t)iLen);
	*pLen += iLen;
	sOut[*pLen] = '\0';
}

static void __xuiDatePickerAppendInt(char* sOut, int* pLen, int iCap, int64 iValue, int iWidth)
{
	char sText[32];

	if ( iWidth == 4 ) {
		snprintf(sText, sizeof(sText), "%04lld", (long long)iValue);
	} else if ( iWidth == 2 ) {
		snprintf(sText, sizeof(sText), "%02lld", (long long)iValue);
	} else {
		snprintf(sText, sizeof(sText), "%lld", (long long)iValue);
	}
	__xuiDatePickerAppend(sOut, pLen, iCap, sText);
}

static void __xuiDatePickerFormatOne(xui_date_picker_data_t* pData, xtime tValue, char* sOut, int iCap)
{
	const char* sFmt;
	int64 iYear;
	int iMonth;
	int iDay;
	int iHour;
	int iMinute;
	int iSecond;
	int iLen;
	int i;

	if ( (sOut == NULL) || (iCap <= 0) ) return;
	sOut[0] = '\0';
	if ( pData == NULL ) return;
	sFmt = pData->sFormat;
	xuiXrtDecodeSerial(tValue, &iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, NULL, NULL);
	iLen = 0;
	for ( i = 0; sFmt[i] != '\0' && iLen < iCap - 1; ) {
		if ( strncmp(sFmt + i, "yyyy", 4) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iYear, 4);
			i += 4;
		} else if ( strncmp(sFmt + i, "yy", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iYear % 100, 2);
			i += 2;
		} else if ( strncmp(sFmt + i, "mm", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iMonth, 2);
			i += 2;
		} else if ( strncmp(sFmt + i, "dd", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iDay, 2);
			i += 2;
		} else if ( strncmp(sFmt + i, "hh", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iHour, 2);
			i += 2;
		} else if ( strncmp(sFmt + i, "nn", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iMinute, 2);
			i += 2;
		} else if ( strncmp(sFmt + i, "ss", 2) == 0 ) {
			__xuiDatePickerAppendInt(sOut, &iLen, iCap, iSecond, 2);
			i += 2;
		} else {
			sOut[iLen++] = sFmt[i++];
			sOut[iLen] = '\0';
		}
	}
}

static void __xuiDatePickerSyncText(xui_date_picker_data_t* pData)
{
	char sA[96];
	char sB[96];

	if ( pData == NULL ) return;
	if ( !pData->bHasValue && pData->bNullable ) {
		pData->sText[0] = '\0';
		return;
	}
	__xuiDatePickerEnsureValue(pData);
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		__xuiDatePickerFormatOne(pData, pData->tStart, sA, (int)sizeof(sA));
		__xuiDatePickerFormatOne(pData, pData->tEnd, sB, (int)sizeof(sB));
		snprintf(pData->sText, sizeof(pData->sText), "%s%s%s", sA, pData->sRangeSeparator, sB);
	} else {
		__xuiDatePickerFormatOne(pData, pData->tValue, pData->sText, (int)sizeof(pData->sText));
	}
	pData->sText[sizeof(pData->sText) - 1] = '\0';
}

static void __xuiDatePickerPreferredPopupSize(int iMode, float* pW, float* pH)
{
	float fW;
	float fH;

	switch ( iMode ) {
	case XUI_DATE_PICKER_MODE_TIME:
		fW = 268.0f; fH = 168.0f;
		break;
	case XUI_DATE_PICKER_MODE_DATETIME:
		fW = 548.0f; fH = 334.0f;
		break;
	case XUI_DATE_PICKER_MODE_DATE_RANGE:
		fW = 628.0f; fH = 326.0f;
		break;
	case XUI_DATE_PICKER_MODE_TIME_RANGE:
		fW = 444.0f; fH = 204.0f;
		break;
	case XUI_DATE_PICKER_MODE_DATETIME_RANGE:
		fW = 672.0f; fH = 408.0f;
		break;
	case XUI_DATE_PICKER_MODE_DATE:
	default:
		fW = 318.0f; fH = 326.0f;
		break;
	}
	if ( pW != NULL ) *pW = fW;
	if ( pH != NULL ) *pH = fH;
}

static void __xuiDatePickerUpdatePopupSizeForMode(xui_date_picker_data_t* pData)
{
	if ( (pData != NULL) && !pData->bPopupSizeCustom ) {
		float fMinWidth;
		float fMinHeight;

		__xuiDatePickerPreferredPopupSize(pData->iMode, &pData->fPopupWidth, &pData->fPopupHeight);
		__xuiDatePickerMinimumPopupSize(pData->iMode, pData->bNullable, &fMinWidth, &fMinHeight);
		if ( pData->fPopupWidth < fMinWidth ) pData->fPopupWidth = fMinWidth;
		if ( pData->fPopupHeight < fMinHeight ) pData->fPopupHeight = fMinHeight;
	}
}

static int __xuiDatePickerDescValid(const xui_date_picker_desc_t* pDesc)
{
	if ( pDesc == NULL ) return 1;
	if ( (pDesc->iSize != 0) && (pDesc->iSize < sizeof(*pDesc)) ) return 0;
	if ( !__xuiDatePickerModeValid(pDesc->iMode) ) return 0;
	if ( (pDesc->iPopupPlacement != 0) && !__xuiDatePickerPlacementValid(pDesc->iPopupPlacement) ) return 0;
	if ( (pDesc->iFirstDayOfWeek < 0) || (pDesc->iFirstDayOfWeek > 6) ) return 0;
	if ( pDesc->tDefaultRangeSpan < 0 || !__xuiDatePickerMetricValid(pDesc->fBorderWidth) ) return 0;
	if ( !__xuiDatePickerPopupSizeValid(pDesc->iMode, pDesc->bNullable, pDesc->fPopupWidth, pDesc->fPopupHeight) ) return 0;
	if ( !__xuiDatePickerLimitsValid(pDesc->iMode, pDesc->bHasMin, pDesc->tMin, pDesc->bHasMax, pDesc->tMax) ) return 0;
	return 1;
}

static void __xuiDatePickerDefaults(xui_date_picker_data_t* pData)
{
	memset(pData, 0, sizeof(*pData));
	pData->iMode = XUI_DATE_PICKER_MODE_DATE;
	pData->iPopupPlacement = XUI_DATE_PICKER_POPUP_AUTO;
	pData->iFirstDayOfWeek = 0;
	pData->iHoverPart = XUI_DATE_PICKER_PART_NONE;
	pData->iActivePart = XUI_DATE_PICKER_PART_NONE;
	pData->iHoverPanel = -1;
	pData->iHoverIndex = -1;
	pData->iActivePanel = 0;
	pData->iPressedPanel = -1;
	pData->iActiveIndex = -1;
	pData->iActiveTimePanel = 0;
	pData->iActiveTimeField = -1;
	pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
	pData->iOpenComboPanel = -1;
	pData->iEditKind = XUI_DATE_PICKER_EDIT_NONE;
	pData->iEditPanel = -1;
	pData->iEditField = -1;
	pData->tDefaultRangeSpan = 0;
	pData->fBorderWidth = 1.0f;
	pData->iTextColor = XUI_COLOR_RGBA(34, 48, 64, 255);
	pData->iDisabledTextColor = XUI_COLOR_RGBA(128, 142, 156, 255);
	pData->iBackgroundColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iHoverBackgroundColor = XUI_COLOR_RGBA(248, 252, 255, 255);
	pData->iOpenBackgroundColor = XUI_COLOR_RGBA(244, 250, 255, 255);
	pData->iDisabledBackgroundColor = XUI_COLOR_RGBA(236, 241, 247, 255);
	pData->iBorderColor = XUI_COLOR_RGBA(168, 185, 205, 255);
	pData->iHoverBorderColor = XUI_COLOR_RGBA(92, 158, 220, 255);
	pData->iFocusBorderColor = XUI_COLOR_RGBA(46, 124, 214, 255);
	pData->iArrowColor = XUI_COLOR_RGBA(60, 82, 104, 255);
	pData->iDisabledArrowColor = XUI_COLOR_RGBA(150, 162, 174, 255);
	pData->iPopupPanelColor = XUI_COLOR_RGBA(250, 253, 255, 255);
	pData->iPopupBorderColor = XUI_COLOR_RGBA(135, 170, 205, 255);
	pData->iPopupShadowColor = XUI_COLOR_RGBA(20, 44, 72, 42);
	pData->iPopupTextColor = XUI_COLOR_RGBA(28, 42, 58, 255);
	pData->iPopupMutedTextColor = XUI_COLOR_RGBA(104, 122, 142, 255);
	pData->iAccentColor = XUI_COLOR_RGBA(46, 124, 214, 255);
	pData->iFieldColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iFieldBorderColor = XUI_COLOR_RGBA(198, 214, 230, 255);
	pData->iSelectedTextColor = XUI_COLOR_RGBA(255, 255, 255, 255);
	pData->iDisabledDayColor = XUI_COLOR_RGBA(168, 178, 190, 255);
	pData->iSeparatorColor = XUI_COLOR_RGBA(216, 228, 238, 255);
	pData->iButtonColor = XUI_COLOR_RGBA(242, 248, 254, 255);
	pData->iButtonHoverColor = XUI_COLOR_RGBA(242, 248, 254, 255);
	pData->iButtonOpenColor = XUI_COLOR_RGBA(242, 248, 254, 255);
	pData->iButtonDisabledColor = XUI_COLOR_RGBA(242, 248, 254, 255);
	pData->iCalendarColor = XUI_COLOR_RGBA(255, 255, 255, 160);
	pData->iTimePanelColor = XUI_COLOR_RGBA(255, 255, 255, 150);
	pData->iRangeColor = XUI_COLOR_RGBA(218, 236, 252, 255);
	pData->iSelectionColor = XUI_COLOR_RGBA(218, 236, 252, 255);
	pData->iErrorBorderColor = XUI_COLOR_RGBA(226, 74, 92, 255);
	pData->iOptionHoverColor = XUI_COLOR_RGBA(232, 244, 255, 255);
	__xuiDatePickerCopyText(pData->sRangeSeparator, (int)sizeof(pData->sRangeSeparator), " - ");
	__xuiDatePickerSyncDefaultFormat(pData);
	__xuiDatePickerUpdatePopupSizeForMode(pData);
	__xuiDatePickerEnsureValue(pData);
	__xuiDatePickerInitViewMonth(pData);
	__xuiDatePickerSyncText(pData);
}

static void __xuiDatePickerApplyDesc(xui_date_picker_data_t* pData, const xui_date_picker_desc_t* pDesc)
{
	float fMinWidth;
	float fMinHeight;

	if ( (pData == NULL) || (pDesc == NULL) ) return;
	pData->iMode = __xuiDatePickerModeValid(pDesc->iMode) ? pDesc->iMode : XUI_DATE_PICKER_MODE_DATE;
	pData->bNullable = pDesc->bNullable ? 1 : 0;
	pData->bShowSecond = pDesc->bShowSecond ? 1 : 0;
	pData->iFirstDayOfWeek = __xuiDatePickerClampInt(pDesc->iFirstDayOfWeek, 0, 6);
	if ( pDesc->pFont != NULL ) pData->pFont = pDesc->pFont;
	if ( pDesc->sFormat != NULL ) {
		__xuiDatePickerCopyText(pData->sFormat, (int)sizeof(pData->sFormat), pDesc->sFormat);
		pData->bFormatCustom = 1;
	} else {
		__xuiDatePickerSyncDefaultFormat(pData);
	}
	if ( pDesc->sRangeSeparator != NULL ) __xuiDatePickerCopyText(pData->sRangeSeparator, (int)sizeof(pData->sRangeSeparator), pDesc->sRangeSeparator);
	if ( pDesc->tDefaultRangeSpan > 0 ) pData->tDefaultRangeSpan = pDesc->tDefaultRangeSpan;
	pData->bHasMin = pDesc->bHasMin ? 1 : 0;
	pData->bHasMax = pDesc->bHasMax ? 1 : 0;
	pData->tMin = pDesc->tMin;
	pData->tMax = pDesc->tMax;
	if ( pDesc->bHasRangeValue ) {
		pData->tStart = pDesc->tStart;
		pData->tEnd = pDesc->tEnd;
		pData->bHasValue = 1;
	} else if ( pDesc->bHasValue ) {
		pData->tValue = pDesc->tValue;
		pData->tStart = pDesc->tValue;
		pData->tEnd = pDesc->tValue;
		pData->bHasValue = 1;
	} else if ( pData->bNullable ) {
		pData->bHasValue = 0;
	}
	__xuiDatePickerPreferredPopupSize(pData->iMode, &pData->fPopupWidth, &pData->fPopupHeight);
	__xuiDatePickerMinimumPopupSize(pData->iMode, pData->bNullable, &fMinWidth, &fMinHeight);
	if ( pData->fPopupWidth < fMinWidth ) pData->fPopupWidth = fMinWidth;
	if ( pData->fPopupHeight < fMinHeight ) pData->fPopupHeight = fMinHeight;
	if ( pDesc->fPopupWidth > 0.0f ) pData->fPopupWidth = pDesc->fPopupWidth;
	if ( pDesc->fPopupHeight > 0.0f ) pData->fPopupHeight = pDesc->fPopupHeight;
	pData->bPopupSizeCustom = (pDesc->fPopupWidth > 0.0f || pDesc->fPopupHeight > 0.0f) ? 1 : 0;
	if ( !pData->bPopupSizeCustom ) __xuiDatePickerUpdatePopupSizeForMode(pData);
	if ( __xuiDatePickerPlacementValid(pDesc->iPopupPlacement) ) pData->iPopupPlacement = pDesc->iPopupPlacement;
	if ( __xuiDatePickerAlpha(pDesc->iTextColor) != 0 ) pData->iTextColor = pDesc->iTextColor;
	if ( __xuiDatePickerAlpha(pDesc->iDisabledTextColor) != 0 ) pData->iDisabledTextColor = pDesc->iDisabledTextColor;
	if ( __xuiDatePickerAlpha(pDesc->iBackgroundColor) != 0 ) pData->iBackgroundColor = pDesc->iBackgroundColor;
	if ( __xuiDatePickerAlpha(pDesc->iHoverBackgroundColor) != 0 ) pData->iHoverBackgroundColor = pDesc->iHoverBackgroundColor;
	if ( __xuiDatePickerAlpha(pDesc->iOpenBackgroundColor) != 0 ) pData->iOpenBackgroundColor = pDesc->iOpenBackgroundColor;
	if ( __xuiDatePickerAlpha(pDesc->iDisabledBackgroundColor) != 0 ) pData->iDisabledBackgroundColor = pDesc->iDisabledBackgroundColor;
	if ( __xuiDatePickerAlpha(pDesc->iBorderColor) != 0 ) pData->iBorderColor = pDesc->iBorderColor;
	if ( __xuiDatePickerAlpha(pDesc->iHoverBorderColor) != 0 ) pData->iHoverBorderColor = pDesc->iHoverBorderColor;
	if ( __xuiDatePickerAlpha(pDesc->iFocusBorderColor) != 0 ) pData->iFocusBorderColor = pDesc->iFocusBorderColor;
	if ( __xuiDatePickerAlpha(pDesc->iArrowColor) != 0 ) pData->iArrowColor = pDesc->iArrowColor;
	if ( __xuiDatePickerAlpha(pDesc->iDisabledArrowColor) != 0 ) pData->iDisabledArrowColor = pDesc->iDisabledArrowColor;
	if ( __xuiDatePickerAlpha(pDesc->iPopupPanelColor) != 0 ) pData->iPopupPanelColor = pDesc->iPopupPanelColor;
	if ( __xuiDatePickerAlpha(pDesc->iPopupBorderColor) != 0 ) pData->iPopupBorderColor = pDesc->iPopupBorderColor;
	if ( __xuiDatePickerAlpha(pDesc->iPopupShadowColor) != 0 ) pData->iPopupShadowColor = pDesc->iPopupShadowColor;
	if ( __xuiDatePickerAlpha(pDesc->iPopupTextColor) != 0 ) pData->iPopupTextColor = pDesc->iPopupTextColor;
	if ( __xuiDatePickerAlpha(pDesc->iPopupMutedTextColor) != 0 ) pData->iPopupMutedTextColor = pDesc->iPopupMutedTextColor;
	if ( __xuiDatePickerAlpha(pDesc->iAccentColor) != 0 ) pData->iAccentColor = pDesc->iAccentColor;
	if ( __xuiDatePickerAlpha(pDesc->iFieldColor) != 0 ) pData->iFieldColor = pDesc->iFieldColor;
	if ( __xuiDatePickerAlpha(pDesc->iFieldBorderColor) != 0 ) pData->iFieldBorderColor = pDesc->iFieldBorderColor;
	if ( __xuiDatePickerAlpha(pDesc->iSelectedTextColor) != 0 ) pData->iSelectedTextColor = pDesc->iSelectedTextColor;
	if ( __xuiDatePickerAlpha(pDesc->iDisabledDayColor) != 0 ) pData->iDisabledDayColor = pDesc->iDisabledDayColor;
	if ( __xuiDatePickerAlpha(pDesc->iSeparatorColor) != 0 ) pData->iSeparatorColor = pDesc->iSeparatorColor;
	if ( pDesc->fBorderWidth > 0.0f ) pData->fBorderWidth = pDesc->fBorderWidth;
	__xuiDatePickerEnsureValue(pData);
	__xuiDatePickerInitViewMonth(pData);
	__xuiDatePickerSyncText(pData);
}

static int __xuiDatePickerStyleColor(xui_widget pWidget, const char* sName, uint32_t* pColor)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pColor == NULL) ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_COLOR) ) {
		*pColor = tProperty.tValue.iColor;
		return 1;
	}
	return 0;
}

static int __xuiDatePickerStyleFloat(xui_widget pWidget, const char* sName, float* pValue)
{
	xui_style_property_t tProperty;
	int iRet;

	if ( (pWidget == NULL) || (sName == NULL) || (pValue == NULL) ) return 0;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, sName, &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_FLOAT) &&
	     __xuiDatePickerMetricValid(tProperty.tValue.fFloat) ) {
		*pValue = tProperty.tValue.fFloat;
		return 1;
	}
	return 0;
}

static xui_font __xuiDatePickerStyleFont(xui_widget pWidget, xui_font pBaseFont)
{
	xui_style_property_t tProperty;
	xui_font pFont;
	int iRet;

	if ( pWidget == NULL ) return pBaseFont;
	memset(&tProperty, 0, sizeof(tProperty));
	tProperty.iSize = sizeof(tProperty);
	iRet = xuiWidgetGetResolvedStyleProperty(pWidget, "font.name", &tProperty);
	if ( (iRet == XUI_OK) && (tProperty.tValue.iType == XUI_STYLE_VALUE_STRING) && (tProperty.tValue.sText != NULL) ) {
		pFont = xuiFindFont(xuiWidgetGetContext(pWidget), tProperty.tValue.sText);
		if ( pFont != NULL ) return pFont;
	}
	if ( pBaseFont != NULL ) return pBaseFont;
	return xuiGetDefaultFont(xuiWidgetGetContext(pWidget));
}

static void __xuiDatePickerResolve(xui_widget pWidget, xui_date_picker_data_t* pData, xui_date_picker_data_t* pResolved)
{
	*pResolved = *pData;
	pResolved->pFont = __xuiDatePickerStyleFont(pWidget, pResolved->pFont);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.text.color", &pResolved->iTextColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.text.disabled_color", &pResolved->iDisabledTextColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.background.color", &pResolved->iBackgroundColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.background.hover_color", &pResolved->iHoverBackgroundColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.background.open_color", &pResolved->iOpenBackgroundColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.background.disabled_color", &pResolved->iDisabledBackgroundColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.border.color", &pResolved->iBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.border.hover_color", &pResolved->iHoverBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.border.focus_color", &pResolved->iFocusBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.arrow.color", &pResolved->iArrowColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.arrow.disabled_color", &pResolved->iDisabledArrowColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.popup.panel_color", &pResolved->iPopupPanelColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.popup.border_color", &pResolved->iPopupBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.popup.shadow_color", &pResolved->iPopupShadowColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.popup.text_color", &pResolved->iPopupTextColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.popup.muted_text_color", &pResolved->iPopupMutedTextColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.accent.color", &pResolved->iAccentColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.field.color", &pResolved->iFieldColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.field.border_color", &pResolved->iFieldBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.day.selected_text_color", &pResolved->iSelectedTextColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.day.disabled_color", &pResolved->iDisabledDayColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.separator.color", &pResolved->iSeparatorColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.button.color", &pResolved->iButtonColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.button.hover_color", &pResolved->iButtonHoverColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.button.open_color", &pResolved->iButtonOpenColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.button.disabled_color", &pResolved->iButtonDisabledColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.calendar.color", &pResolved->iCalendarColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.time.color", &pResolved->iTimePanelColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.day.range_color", &pResolved->iRangeColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.selection.color", &pResolved->iSelectionColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.field.error_border_color", &pResolved->iErrorBorderColor);
	(void)__xuiDatePickerStyleColor(pWidget, "datepicker.option.hover_color", &pResolved->iOptionHoverColor);
	(void)__xuiDatePickerStyleFloat(pWidget, "datepicker.border.width", &pResolved->fBorderWidth);
}

static int __xuiDatePickerInvalidateAll(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	int iRet;
	int iPanelRet;

	iRet = (pWidget != NULL) ? xuiWidgetInvalidate(pWidget,
		XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
	iPanelRet = XUI_OK;
	if ( pData != NULL && pData->pPanel != NULL ) {
		iPanelRet = xuiWidgetInvalidate(pData->pPanel,
			XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return (iRet != XUI_OK) ? iRet : iPanelRet;
}

static int __xuiDatePickerNotifyDraft(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_date_picker_proc proc;
	void* pUser;
	xtime tStart;
	xtime tEnd;
	int iMode;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerIncrement(&pData->iChangingCount);
	proc = pData->onChanging;
	pUser = pData->pChangingUser;
	iMode = pData->iMode;
	tStart = __xuiDatePickerIsRange(iMode) ? pData->tDraftStart : pData->tDraftValue;
	tEnd = __xuiDatePickerIsRange(iMode) ? pData->tDraftEnd : 0;
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( proc != NULL ) proc(pWidget, tStart, tEnd, iMode, pUser);
	return XUI_OK;
}

static uint32_t __xuiDatePickerState(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pFocus;
	uint32_t iState;

	if ( pWidget == NULL ) return 0u;
	iState = xuiWidgetGetInputState(pWidget);
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) ) {
		iState |= XUI_WIDGET_STATE_DISABLED;
	}
	if ( (pData != NULL) && (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
		iState |= XUI_DATE_PICKER_STATE_OPEN;
	} else {
		iState &= ~XUI_DATE_PICKER_STATE_OPEN;
	}
	if ( pData != NULL ) {
		pContext = xuiWidgetGetContext(pWidget);
		pFocus = xuiGetFocusWidget(pContext);
		if ( (pFocus == pWidget) || (pFocus == pData->pPanel) || (pFocus == pData->pPopup) ) {
			iState |= XUI_WIDGET_STATE_FOCUS;
		}
	}
	return iState;
}

static int __xuiDatePickerSyncState(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	uint32_t iState;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	iState = __xuiDatePickerState(pWidget, pData);
	if ( xuiWidgetGetStateId(pWidget) == iState ) return XUI_OK;
	iRet = xuiWidgetSetStateId(pWidget, iState);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_STATE_CHANGED);
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

static void __xuiDatePickerUpdateOwnerRects(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_rect_t tRect;
	float fButtonW;
	float fTextLeft;
	float fTextRight;

	if ( (pWidget == NULL) || (pData == NULL) ) return;
	tRect = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) {
		pData->tButtonRect = (xui_rect_t){0};
		pData->tTextRect = (xui_rect_t){0};
		return;
	}
	fButtonW = tRect.fH;
	if ( fButtonW < 24.0f ) fButtonW = 24.0f;
	if ( fButtonW > 34.0f ) fButtonW = 34.0f;
	fButtonW = __xuiDatePickerMin(fButtonW, tRect.fW);
	pData->tButtonRect = xuiInternalRectFromFloatNearest(tRect.fX + tRect.fW - fButtonW, tRect.fY, fButtonW, tRect.fH);
	fTextRight = tRect.fX + tRect.fW - fButtonW;
	fTextLeft = tRect.fX + __xuiDatePickerMin(8.0f, __xuiDatePickerMax(0.0f, fTextRight - tRect.fX));
	if ( fTextRight - fTextLeft > 6.0f ) fTextRight -= 6.0f;
	else fTextRight = fTextLeft;
	pData->tTextRect = xuiInternalRectFromFloatNearest(fTextLeft, tRect.fY,
		__xuiDatePickerMax(0.0f, fTextRight - fTextLeft), tRect.fH);
}

typedef struct xui_date_picker_rect_t {
	float fX, fY, fW, fH;
} xui_date_picker_rect_t;

static void __xuiDatePickerLayoutCalendar(xui_date_picker_data_t* pData, int iPanel, xui_date_picker_rect_t tPanel)
{
	static const float fHeaderH = 32.0f;
	static const float fWeekH = 22.0f;
	float fGridH;
	float arrX[8];
	float arrY[7];
	float fGridY;
	float fButtonW;
	float fLabelW;
	float fYearW;
	float fMonthW;
	float fGap;
	float fComboW;
	float fComboX;
	xtime tFirst;
	xtime tCell;
	int iWeekday;
	int iOffset;
	int i;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return;
	if ( tPanel.fW < 0.0f ) tPanel.fW = 0.0f;
	if ( tPanel.fH < 0.0f ) tPanel.fH = 0.0f;
	pData->tCalendarRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX, tPanel.fY, tPanel.fW, tPanel.fH);
	pData->tHeaderRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX, tPanel.fY, tPanel.fW, fHeaderH);
	fButtonW = 28.0f;
	pData->tPrevRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX + 2.0f, tPanel.fY + 2.0f, fButtonW, fHeaderH - 4.0f);
	pData->tNextRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX + tPanel.fW - fButtonW - 2.0f, tPanel.fY + 2.0f, fButtonW, fHeaderH - 4.0f);
	fLabelW = __xuiDatePickerMax(80.0f, tPanel.fW - fButtonW * 2.0f - 18.0f);
	if ( fLabelW > 150.0f ) fLabelW = 150.0f;
	pData->tMonthLabelRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX + (tPanel.fW - fLabelW) * 0.5f, tPanel.fY + 4.0f, fLabelW, fHeaderH - 8.0f);
	fYearW = XUI_DATE_PICKER_YEAR_FIELD_W;
	fMonthW = XUI_DATE_PICKER_MONTH_FIELD_W;
	fGap = XUI_DATE_PICKER_HEADER_COMBO_GAP;
	fComboW = fYearW + fMonthW + fGap;
	if ( fComboW > tPanel.fW - fButtonW * 2.0f - 12.0f ) {
		fComboW = tPanel.fW - fButtonW * 2.0f - 12.0f;
		if ( fComboW < 104.0f ) fComboW = 104.0f;
		fYearW = (fComboW - fGap) * 0.64f;
		fMonthW = fComboW - fGap - fYearW;
	}
	fComboX = tPanel.fX + (tPanel.fW - fComboW) * 0.5f;
	pData->tYearRect[iPanel] = xuiInternalRectFromFloatNearest(fComboX, tPanel.fY + 4.0f, fYearW, fHeaderH - 8.0f);
	pData->tMonthRect[iPanel] = xuiInternalRectFromFloatNearest(fComboX + fYearW + fGap, tPanel.fY + 4.0f, fMonthW, fHeaderH - 8.0f);
	fGridY = tPanel.fY + fHeaderH + fWeekH;
	fGridH = __xuiDatePickerMax(120.0f, tPanel.fH - fHeaderH - fWeekH);
	/* Share exact edges; multiplying an already divided cell size can lose a half pixel. */
	for ( i = 0; i < 7; i++ ) arrX[i] = tPanel.fX + tPanel.fW * (float)i / 7.0f;
	arrX[7] = tPanel.fX + tPanel.fW;
	for ( i = 0; i < 6; i++ ) arrY[i] = fGridY + fGridH * (float)i / 6.0f;
	arrY[6] = fGridY + fGridH;
	tFirst = xuiXrtFirstDayOfMonth(pData->tViewMonth[iPanel]);
	xuiXrtDecodeSerial(tFirst, NULL, NULL, NULL, NULL, NULL, NULL, &iWeekday, NULL);
	iOffset = (iWeekday - pData->iFirstDayOfWeek + 7) % 7;
	for ( i = 0; i < XUI_DATE_PICKER_CELL_COUNT; i++ ) {
		pData->arrDayRect[iPanel][i] = xuiInternalRectFromFloatNearest(
			arrX[i % 7], arrY[i / 7],
			arrX[i % 7 + 1] - arrX[i % 7], arrY[i / 7 + 1] - arrY[i / 7]
		);
		tCell = tFirst + ((xtime)i - (xtime)iOffset) * XRT_TIME_DAY;
		pData->arrDayValue[iPanel][i] = xrtDatePart(tCell);
	}
}

static void __xuiDatePickerLayoutTime(xui_date_picker_data_t* pData, int iPanel, xui_date_picker_rect_t tPanel)
{
	float fFieldW;
	float fFieldY;
	float fGap;
	float fGroupH;
	float fInnerW;
	float fX;
	int iCount;
	int i;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return;
	if ( tPanel.fW < 0.0f ) tPanel.fW = 0.0f;
	if ( tPanel.fH < 0.0f ) tPanel.fH = 0.0f;
	pData->tTimePanelRect[iPanel] = xuiInternalRectFromFloatNearest(tPanel.fX, tPanel.fY, tPanel.fW, tPanel.fH);
	iCount = pData->bShowSecond ? 3 : 2;
	fGap = 8.0f;
	fInnerW = tPanel.fW - XUI_DATE_PICKER_TIME_INSET * 2.0f;
	if ( fInnerW < (float)iCount ) {
		fInnerW = (float)iCount;
	}
	fFieldW = (fInnerW - fGap * (float)(iCount - 1)) / (float)iCount;
	if ( fFieldW < 1.0f ) fFieldW = 1.0f;
	if ( fFieldW > 72.0f ) fFieldW = 72.0f;
	fX = tPanel.fX + XUI_DATE_PICKER_TIME_INSET + (fInnerW - (fFieldW * (float)iCount + fGap * (float)(iCount - 1))) * 0.5f;
	fGroupH = 46.0f;
	fFieldY = tPanel.fY + (tPanel.fH - fGroupH) * 0.5f + 18.0f;
	if ( fFieldY < tPanel.fY + 44.0f ) {
		fFieldY = tPanel.fY + 44.0f;
	}
	if ( fFieldY + 28.0f > tPanel.fY + tPanel.fH - 8.0f ) {
		fFieldY = tPanel.fY + tPanel.fH - 36.0f;
	}
	for ( i = 0; i < 3; i++ ) {
		if ( i < iCount ) {
			pData->arrTimeRect[iPanel][i] = xuiInternalRectFromFloatNearest(fX + (float)i * (fFieldW + fGap), fFieldY, fFieldW, 28.0f);
		} else {
			pData->arrTimeRect[iPanel][i] = (xui_rect_t){0.0f, 0.0f, 0.0f, 0.0f};
		}
	}
}

static void __xuiDatePickerUpdatePanelRects(xui_date_picker_data_t* pData)
{
	float fW;
	float fH;
	float fPad;
	float fFooterY;
	float fFooterH;
	float fColW;
	float fCalH;
	float fTimeH;
	float fX;
	float fButtonW;
	int i;

	if ( pData == NULL ) return;
	fW = pData->fPopupWidth;
	fH = pData->fPopupHeight;
	fPad = XUI_DATE_PICKER_PAD;
	fFooterH = 28.0f;
	fFooterY = fH - fPad - fFooterH;
	for ( i = 0; i < XUI_DATE_PICKER_PANEL_CAPACITY; i++ ) {
		pData->tCalendarRect[i] = (xui_rect_t){0};
		pData->tTimePanelRect[i] = (xui_rect_t){0};
		pData->tYearRect[i] = (xui_rect_t){0};
		pData->tMonthRect[i] = (xui_rect_t){0};
	}
	if ( __xuiDatePickerHasDate(pData->iMode) && __xuiDatePickerHasTime(pData->iMode) ) {
		if ( __xuiDatePickerIsRange(pData->iMode) ) {
			fColW = (fW - fPad * 3.0f) * 0.5f;
			fCalH = 238.0f;
			fTimeH = __xuiDatePickerMax(XUI_DATE_PICKER_TIME_MIN_HEIGHT, fFooterY - fPad * 2.0f - fCalH);
			__xuiDatePickerLayoutCalendar(pData, 0, (xui_date_picker_rect_t){fPad, fPad, fColW, fCalH});
			__xuiDatePickerLayoutCalendar(pData, 1, (xui_date_picker_rect_t){fPad * 2.0f + fColW, fPad, fColW, fCalH});
			__xuiDatePickerLayoutTime(pData, 0, (xui_date_picker_rect_t){fPad, fPad + fCalH + 6.0f, fColW, fTimeH});
			__xuiDatePickerLayoutTime(pData, 1, (xui_date_picker_rect_t){fPad * 2.0f + fColW, fPad + fCalH + 6.0f, fColW, fTimeH});
		} else {
			fCalH = fFooterY - fPad * 2.0f;
			fColW = 292.0f;
			if ( fColW > fW * 0.58f ) fColW = fW * 0.58f;
			__xuiDatePickerLayoutCalendar(pData, 0, (xui_date_picker_rect_t){fPad, fPad, fColW, fCalH});
			__xuiDatePickerLayoutTime(pData, 0, (xui_date_picker_rect_t){fPad * 2.0f + fColW, fPad + 58.0f, fW - fColW - fPad * 3.0f, 96.0f});
		}
	} else if ( __xuiDatePickerHasDate(pData->iMode) ) {
		fCalH = fFooterY - fPad * 2.0f;
		if ( __xuiDatePickerIsRange(pData->iMode) ) {
			fColW = (fW - fPad * 3.0f) * 0.5f;
			__xuiDatePickerLayoutCalendar(pData, 0, (xui_date_picker_rect_t){fPad, fPad, fColW, fCalH});
			__xuiDatePickerLayoutCalendar(pData, 1, (xui_date_picker_rect_t){fPad * 2.0f + fColW, fPad, fColW, fCalH});
		} else {
			__xuiDatePickerLayoutCalendar(pData, 0, (xui_date_picker_rect_t){fPad, fPad, fW - fPad * 2.0f, fCalH});
		}
	} else {
		if ( __xuiDatePickerIsRange(pData->iMode) ) {
			fColW = (fW - fPad * 3.0f) * 0.5f;
			__xuiDatePickerLayoutTime(pData, 0, (xui_date_picker_rect_t){fPad, fPad + 12.0f, fColW, fFooterY - fPad * 2.0f});
			__xuiDatePickerLayoutTime(pData, 1, (xui_date_picker_rect_t){fPad * 2.0f + fColW, fPad + 12.0f, fColW, fFooterY - fPad * 2.0f});
		} else {
			__xuiDatePickerLayoutTime(pData, 0, (xui_date_picker_rect_t){fPad, fPad + 12.0f, fW - fPad * 2.0f, fFooterY - fPad * 2.0f});
		}
	}
	for ( i = 0; i < 4; i++ ) pData->arrFooterRect[i] = (xui_rect_t){0};
	fButtonW = 66.0f;
	fX = fW - fPad - fButtonW;
	pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_OK] = xuiInternalRectFromFloatNearest(fX, fFooterY, fButtonW, fFooterH);
	fX -= fButtonW + 8.0f;
	pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_CANCEL] = xuiInternalRectFromFloatNearest(fX, fFooterY, fButtonW, fFooterH);
	if ( pData->bNullable ) {
		fX -= fButtonW + 8.0f;
		pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_CLEAR] = xuiInternalRectFromFloatNearest(fX, fFooterY, fButtonW, fFooterH);
	}
	pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_TODAY] = xuiInternalRectFromFloatNearest(fPad, fFooterY, 78.0f, fFooterH);
}

static int __xuiDatePickerDrawFill(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, uint32_t iColor)
{
	if ( __xuiDatePickerAlpha(iColor) == 0 ) return XUI_OK;
	return (pProxy->drawRectFill != NULL) ? pProxy->drawRectFill(pProxy, pDraw, tRect, iColor) : XUI_OK;
}

static int __xuiDatePickerDrawStroke(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, float fWidth, uint32_t iColor)
{
	if ( (fWidth <= 0.0f) || (__xuiDatePickerAlpha(iColor) == 0) ) return XUI_OK;
	fWidth = xuiInternalSnapSize(fWidth);
	tRect = xuiInternalStrokeCenterRectInside(tRect, fWidth, NULL);
	if ( (tRect.fW <= 0.0f) || (tRect.fH <= 0.0f) ) return XUI_OK;
	return (pProxy->drawRectStroke != NULL) ? pProxy->drawRectStroke(pProxy, pDraw, tRect, fWidth, iColor) : XUI_OK;
}

static int __xuiDatePickerDrawChevron(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int bOpen, uint32_t iColor)
{
	float fCx;
	float fY;
	float fLeft;
	float fRight;
	int iRet;

	if ( pProxy->drawLine == NULL || tRect.fW < 12.0f || tRect.fH < 8.0f ) return XUI_OK;
	fCx = tRect.fX + tRect.fW * 0.5f;
	fY = tRect.fY + tRect.fH * 0.5f;
	fLeft = fCx - 5.0f;
	fRight = fCx + 5.0f;
	if ( bOpen ) {
		iRet = pProxy->drawLine(pProxy, pDraw, fLeft, fY + 3.0f, fCx, fY - 3.0f, 1.6f, iColor);
		if ( iRet != XUI_OK ) return iRet;
		return pProxy->drawLine(pProxy, pDraw, fCx, fY - 3.0f, fRight, fY + 3.0f, 1.6f, iColor);
	}
	iRet = pProxy->drawLine(pProxy, pDraw, fLeft, fY - 3.0f, fCx, fY + 3.0f, 1.6f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawLine(pProxy, pDraw, fCx, fY + 3.0f, fRight, fY - 3.0f, 1.6f, iColor);
}

static int __xuiDatePickerDrawArrow(xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, int iDir, uint32_t iColor)
{
	float fCx;
	float fCy;
	int iRet;

	if ( pProxy->drawLine == NULL ) return XUI_OK;
	fCx = tRect.fX + tRect.fW * 0.5f;
	fCy = tRect.fY + tRect.fH * 0.5f;
	if ( iDir < 0 ) {
		iRet = pProxy->drawLine(pProxy, pDraw, fCx + 4.0f, fCy - 6.0f, fCx - 4.0f, fCy, 1.6f, iColor);
		if ( iRet != XUI_OK ) return iRet;
		return pProxy->drawLine(pProxy, pDraw, fCx - 4.0f, fCy, fCx + 4.0f, fCy + 6.0f, 1.6f, iColor);
	}
	iRet = pProxy->drawLine(pProxy, pDraw, fCx - 4.0f, fCy - 6.0f, fCx + 4.0f, fCy, 1.6f, iColor);
	if ( iRet != XUI_OK ) return iRet;
	return pProxy->drawLine(pProxy, pDraw, fCx + 4.0f, fCy, fCx - 4.0f, fCy + 6.0f, 1.6f, iColor);
}

static int __xuiDatePickerSyncStyle(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_date_picker_data_t tResolved;
	uint32_t iHash = xuiWidgetGetStyleHash(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->bChildStyleSynced && pData->iChildStyleHash == iHash ) return XUI_OK;
	__xuiDatePickerResolve(pWidget, pData, &tResolved);
	if ( pData->pPopup != NULL ) {
		iRet = xuiPopupSetColors(pData->pPopup, tResolved.iPopupPanelColor,
			tResolved.iPopupBorderColor, tResolved.iPopupShadowColor, 0u);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pData->pPanel != NULL ) {
		iRet = xuiWidgetInvalidate(pData->pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	pData->iChildStyleHash = iHash;
	pData->bChildStyleSynced = 1;
	return XUI_OK;
}

static int __xuiDatePickerCacheRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_date_picker_data_t* pData;
	xui_date_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_rect_t tRect;
	uint32_t iState;
	uint32_t iBackground;
	uint32_t iBorder;
	uint32_t iText;
	uint32_t iArrow;
	uint32_t iButton;
	int iRet;

	(void)iStateId;
	(void)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiDatePickerResolve(pWidget, pData, &tResolved);
	__xuiDatePickerUpdateOwnerRects(pWidget, pData);
	tRect = xuiInternalSnapRect(xuiWidgetGetContentRect(pWidget));
	iState = __xuiDatePickerState(pWidget, pData);
	iBackground = tResolved.iBackgroundColor;
	iBorder = tResolved.iBorderColor;
	iText = tResolved.iTextColor;
	iArrow = tResolved.iArrowColor;
	iButton = tResolved.iButtonColor;
	if ( (iState & XUI_WIDGET_STATE_DISABLED) != 0u ) {
		iBackground = tResolved.iDisabledBackgroundColor;
		iBorder = tResolved.iBorderColor;
		iText = tResolved.iDisabledTextColor;
		iArrow = tResolved.iDisabledArrowColor;
		iButton = tResolved.iButtonDisabledColor;
	} else if ( (iState & XUI_DATE_PICKER_STATE_OPEN) != 0u ) {
		iBackground = tResolved.iOpenBackgroundColor;
		iBorder = tResolved.iFocusBorderColor;
		iButton = tResolved.iButtonOpenColor;
	} else if ( (iState & XUI_WIDGET_STATE_FOCUS) != 0u ) {
		iBorder = tResolved.iFocusBorderColor;
	} else if ( (iState & XUI_WIDGET_STATE_HOVER) != 0u ) {
		iBackground = tResolved.iHoverBackgroundColor;
		iBorder = tResolved.iHoverBorderColor;
		iButton = tResolved.iButtonHoverColor;
	}
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tRect, iBackground);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, pData->tButtonRect, iButton);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pProxy->drawText != NULL) && (tResolved.pFont != NULL) && (__xuiDatePickerAlpha(iText) != 0) ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=tResolved.pFont, .sText=pData->sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, pData->tTextRect, iText, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiDatePickerDrawChevron(pProxy, pDraw, pData->tButtonRect, (iState & XUI_DATE_PICKER_STATE_OPEN) != 0u, iArrow);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiDatePickerDrawStroke(pProxy, pDraw, tRect, tResolved.fBorderWidth, iBorder);
}

static int __xuiDatePickerDrawButton(xui_proxy pProxy, xui_draw_context pDraw, xui_font pFont, xui_rect_t tRect, const char* sText, uint32_t iText, uint32_t iFill, uint32_t iBorder, int bAccent)
{
	int iRet;

	if ( tRect.fW <= 0.0f || tRect.fH <= 0.0f ) return XUI_OK;
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tRect, iFill);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawStroke(pProxy, pDraw, tRect, bAccent ? 1.5f : 1.0f, iBorder);
	if ( iRet != XUI_OK ) return iRet;
	if ( (pProxy->drawText != NULL) && (pFont != NULL) && (sText != NULL) ) {
		return pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tRect, iText, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
	}
	return XUI_OK;
}

static xui_rect_t __xuiDatePickerComboPopupRect(const xui_date_picker_data_t* pData, int iKind, int iPanel)
{
	xui_rect_t tField;
	xui_rect_t tCalendar;
	xui_date_picker_rect_t tRect;
	float fW;
	float fH;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return (xui_rect_t){0};
	tField = (iKind == XUI_DATE_PICKER_COMBO_MONTH) ? pData->tMonthRect[iPanel] : pData->tYearRect[iPanel];
	tCalendar = pData->tCalendarRect[iPanel];
	if ( tField.fW <= 0.0f || tField.fH <= 0.0f ) return (xui_rect_t){0};
	if ( iKind == XUI_DATE_PICKER_COMBO_MONTH ) {
		fW = 144.0f;
		fH = 96.0f;
		tRect = (xui_date_picker_rect_t){tField.fX + tField.fW - fW, tField.fY + tField.fH + 2.0f, fW, fH};
	} else {
		fW = __xuiDatePickerMax(tField.fW, XUI_DATE_PICKER_YEAR_FIELD_W);
		fH = 22.0f * (float)XUI_DATE_PICKER_YEAR_OPTION_COUNT;
		tRect = (xui_date_picker_rect_t){tField.fX, tField.fY + tField.fH + 2.0f, fW, fH};
	}
	if ( tRect.fX < tCalendar.fX + 2.0f ) tRect.fX = tCalendar.fX + 2.0f;
	if ( tRect.fX + tRect.fW > tCalendar.fX + tCalendar.fW - 2.0f ) tRect.fX = tCalendar.fX + tCalendar.fW - tRect.fW - 2.0f;
	if ( tRect.fY + tRect.fH > pData->fPopupHeight - 50.0f ) tRect.fY = pData->fPopupHeight - 50.0f - tRect.fH;
	if ( tRect.fY < tField.fY + tField.fH + 2.0f ) tRect.fY = tField.fY + tField.fH + 2.0f;
	return xuiInternalRectFromFloatNearest(tRect.fX, tRect.fY, tRect.fW, tRect.fH);
}

static xui_rect_t __xuiDatePickerYearOptionRect(const xui_date_picker_data_t* pData, int iPanel, int iIndex)
{
	xui_rect_t tDrop;

	if ( iIndex < 0 || iIndex >= XUI_DATE_PICKER_YEAR_OPTION_COUNT ) return (xui_rect_t){0};
	tDrop = __xuiDatePickerComboPopupRect(pData, XUI_DATE_PICKER_COMBO_YEAR, iPanel);
	return xuiInternalRectFromFloatNearest(tDrop.fX, tDrop.fY + (float)iIndex * 22.0f, tDrop.fW, 22.0f);
}

static xui_rect_t __xuiDatePickerMonthOptionRect(const xui_date_picker_data_t* pData, int iPanel, int iIndex)
{
	xui_rect_t tDrop;
	float fCellW;
	float fCellH;

	if ( iIndex < 0 || iIndex >= 12 ) return (xui_rect_t){0};
	tDrop = __xuiDatePickerComboPopupRect(pData, XUI_DATE_PICKER_COMBO_MONTH, iPanel);
	fCellW = tDrop.fW / 3.0f;
	fCellH = tDrop.fH / 4.0f;
	return xuiInternalRectFromFloatNearest(tDrop.fX + (float)(iIndex % 3) * fCellW, tDrop.fY + (float)(iIndex / 3) * fCellH, fCellW, fCellH);
}

static int64 __xuiDatePickerYearOptionValue(const xui_date_picker_data_t* pData, int iPanel, int iIndex)
{
	int64 iYear;
	int64 iFirst;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return 1;
	xuiXrtDecodeSerial(pData->tViewMonth[iPanel], &iYear, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	iFirst = iYear - (XUI_DATE_PICKER_YEAR_OPTION_COUNT / 2);
	if ( iFirst < XUI_DATE_PICKER_YEAR_MIN ) iFirst = XUI_DATE_PICKER_YEAR_MIN;
	if ( iFirst + XUI_DATE_PICKER_YEAR_OPTION_COUNT - 1 > XUI_DATE_PICKER_YEAR_MAX ) {
		iFirst = XUI_DATE_PICKER_YEAR_MAX - XUI_DATE_PICKER_YEAR_OPTION_COUNT + 1;
	}
	if ( iFirst < XUI_DATE_PICKER_YEAR_MIN ) iFirst = XUI_DATE_PICKER_YEAR_MIN;
	return iFirst + iIndex;
}

static int __xuiDatePickerDrawComboField(xui_date_picker_data_t* pData, const xui_date_picker_data_t* pResolved, xui_proxy pProxy, xui_draw_context pDraw, xui_rect_t tRect, const char* sText, int bOpen, int bActive, int bError, int bSelectAll)
{
	xui_rect_t tText;
	xui_rect_t tArrow;
	xui_rect_t tSelect;
	uint32_t iBorder;
	int iRet;

	iBorder = bError ? pResolved->iErrorBorderColor : (bActive ? pResolved->iAccentColor : pResolved->iFieldBorderColor);
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, pResolved->pFont, tRect, "", pResolved->iPopupTextColor, pResolved->iFieldColor, iBorder, bActive);
	if ( iRet != XUI_OK ) return iRet;
	tArrow = xuiInternalRectFromFloatNearest(tRect.fX + tRect.fW - 20.0f, tRect.fY, 20.0f, tRect.fH);
	tText = xuiInternalRectFromFloatNearest(tRect.fX + 6.0f, tRect.fY, tRect.fW - 28.0f, tRect.fH);
	if ( bSelectAll ) {
		tSelect = __xuiDatePickerInsetRect(tText, 3.0f);
		iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tSelect, pResolved->iSelectionColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tText, pResolved->iPopupTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	return __xuiDatePickerDrawChevron(pProxy, pDraw, tArrow, bOpen, pResolved->iPopupMutedTextColor);
}

static int __xuiDatePickerDrawCalendar(xui_widget pOwner, xui_date_picker_data_t* pData, const xui_date_picker_data_t* pResolved, xui_proxy pProxy, xui_draw_context pDraw, int iPanel)
{
	static const char* arrWeek[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	char sText[32];
	xui_rect_t tRect;
	xtime tMonth;
	xtime tDay;
	int64 iYear;
	int iMonth;
	int iDay;
	int iCellMonth;
	int i;
	int iRet;
	int bCurrentMonth;
	int bSelected;
	int bInRange;
	int bEnabled;
	uint32_t iFill;
	uint32_t iText;
	uint32_t iBorder;

	(void)pOwner;
	if ( pData->tCalendarRect[iPanel].fW <= 0.0f ) return XUI_OK;
	tRect = pData->tCalendarRect[iPanel];
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tRect, pResolved->iCalendarColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawStroke(pProxy, pDraw, tRect, 1.0f, pResolved->iSeparatorColor);
	if ( iRet != XUI_OK ) return iRet;
	tMonth = pData->tViewMonth[iPanel];
	xuiXrtDecodeSerial(tMonth, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, pResolved->pFont, pData->tPrevRect[iPanel], "", pResolved->iPopupTextColor, pResolved->iFieldColor, pResolved->iFieldBorderColor, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, pResolved->pFont, pData->tNextRect[iPanel], "", pResolved->iPopupTextColor, pResolved->iFieldColor, pResolved->iFieldBorderColor, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawArrow(pProxy, pDraw, pData->tPrevRect[iPanel], -1, pResolved->iPopupMutedTextColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawArrow(pProxy, pDraw, pData->tNextRect[iPanel], 1, pResolved->iPopupMutedTextColor);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR && pData->iEditPanel == iPanel ) {
		snprintf(sText, sizeof(sText), "%s", pData->sEdit);
	} else {
		snprintf(sText, sizeof(sText), "%04lld", (long long)iYear);
	}
	iRet = __xuiDatePickerDrawComboField(pData, pResolved, pProxy, pDraw, pData->tYearRect[iPanel], sText,
		(pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_YEAR) && (pData->iOpenComboPanel == iPanel),
		(pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR && pData->iEditPanel == iPanel) ||
		(pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_YEAR && pData->iOpenComboPanel == iPanel),
		(pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR && pData->iEditPanel == iPanel && pData->iEditError),
		(pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR && pData->iEditPanel == iPanel && pData->iEditSelectAll));
	if ( iRet != XUI_OK ) return iRet;
	snprintf(sText, sizeof(sText), "%02d", iMonth);
	iRet = __xuiDatePickerDrawComboField(pData, pResolved, pProxy, pDraw, pData->tMonthRect[iPanel], sText,
		(pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_MONTH) && (pData->iOpenComboPanel == iPanel),
		(pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_MONTH) && (pData->iOpenComboPanel == iPanel),
		0, 0);
	if ( iRet != XUI_OK ) return iRet;
	for ( i = 0; i < 7; i++ ) {
		tRect = pData->arrDayRect[iPanel][i];
		tRect.fY = pData->tCalendarRect[iPanel].fY + 34;
		tRect.fH = 20;
		if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=arrWeek[(i + pData->iFirstDayOfWeek) % 7], .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tRect, pResolved->iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	for ( i = 0; i < XUI_DATE_PICKER_CELL_COUNT; i++ ) {
		tDay = pData->arrDayValue[iPanel][i];
		xuiXrtDecodeSerial(tDay, NULL, &iCellMonth, &iDay, NULL, NULL, NULL, NULL, NULL);
		bCurrentMonth = (iCellMonth == iMonth);
		bEnabled = __xuiDatePickerValueEnabled(pData, tDay);
		bSelected = 0;
		bInRange = 0;
		if ( __xuiDatePickerIsRange(pData->iMode) ) {
			bSelected = (xrtDatePart(pData->tDraftStart) == tDay) || (xrtDatePart(pData->tDraftEnd) == tDay);
			bInRange = (tDay >= xrtDatePart(pData->tDraftStart)) && (tDay <= xrtDatePart(pData->tDraftEnd));
		} else {
			bSelected = (xrtDatePart(pData->tDraftValue) == tDay);
		}
		iFill = 0;
		iText = bCurrentMonth ? pResolved->iPopupTextColor : pResolved->iPopupMutedTextColor;
		iBorder = 0;
		if ( !bEnabled ) {
			iText = pResolved->iDisabledDayColor;
		} else if ( bSelected ) {
			iFill = pResolved->iAccentColor;
			iText = pResolved->iSelectedTextColor;
			iBorder = pResolved->iAccentColor;
		} else if ( bInRange ) {
			iFill = pResolved->iRangeColor;
			iText = pResolved->iPopupTextColor;
		} else if ( xrtDatePart(xrtNow()) == tDay ) {
			iBorder = pResolved->iAccentColor;
		}
		tRect = pData->arrDayRect[iPanel][i];
		tRect = __xuiDatePickerInsetRect(tRect, 2.0f);
		if ( __xuiDatePickerAlpha(iFill) != 0 ) {
			iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tRect, iFill);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( __xuiDatePickerAlpha(iBorder) != 0 ) {
			iRet = __xuiDatePickerDrawStroke(pProxy, pDraw, tRect, 1.0f, iBorder);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
			snprintf(sText, sizeof(sText), "%d", iDay);
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tRect, iText, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static xtime __xuiDatePickerDraftForPanel(const xui_date_picker_data_t* pData, int iPanel)
{
	if ( pData == NULL ) return 0;
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		return (iPanel == 1) ? pData->tDraftEnd : pData->tDraftStart;
	}
	return pData->tDraftValue;
}

static int __xuiDatePickerDrawTimePanel(xui_date_picker_data_t* pData, const xui_date_picker_data_t* pResolved, xui_proxy pProxy, xui_draw_context pDraw, int iPanel)
{
	static const char* arrName[3] = {"Hour", "Min", "Sec"};
	char sText[32];
	xui_rect_t tRect;
	xtime tValue;
	int iHour;
	int iMinute;
	int iSecond;
	int arrValue[3];
	float fTitleY;
	int i;
	int iRet;

	if ( pData->tTimePanelRect[iPanel].fW <= 0.0f ) return XUI_OK;
	tRect = pData->tTimePanelRect[iPanel];
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tRect, pResolved->iTimePanelColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawStroke(pProxy, pDraw, tRect, 1.0f, pResolved->iSeparatorColor);
	if ( iRet != XUI_OK ) return iRet;
	fTitleY = tRect.fY + 7.0f;
	if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
		snprintf(sText, sizeof(sText), "%s time", (__xuiDatePickerIsRange(pData->iMode) ? ((iPanel == 0) ? "Start" : "End") : "Select"));
		iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, xuiInternalRectFromFloatNearest(tRect.fX + 8.0f, fTitleY, tRect.fW - 16.0f, 16.0f), pResolved->iPopupMutedTextColor, XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
		if ( iRet != XUI_OK ) return iRet;
	}
	tValue = __xuiDatePickerDraftForPanel(pData, iPanel);
	xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	arrValue[0] = iHour;
	arrValue[1] = iMinute;
	arrValue[2] = iSecond;
	for ( i = 0; i < (pData->bShowSecond ? 3 : 2); i++ ) {
		tRect = pData->arrTimeRect[iPanel][i];
		if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=arrName[i], .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, (xui_rect_t){tRect.fX, tRect.fY - 18, tRect.fW, 16}, pResolved->iPopupMutedTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
		iRet = __xuiDatePickerDrawButton(pProxy, pDraw, pResolved->pFont, tRect, "", pResolved->iPopupTextColor, pResolved->iFieldColor, ((pData->iActiveTimePanel == iPanel) && (pData->iActiveTimeField == i)) ? pResolved->iAccentColor : pResolved->iFieldBorderColor, (pData->iActiveTimePanel == iPanel) && (pData->iActiveTimeField == i));
		if ( iRet != XUI_OK ) return iRet;
		if ( pData->iEditKind == XUI_DATE_PICKER_EDIT_TIME && pData->iEditPanel == iPanel && pData->iEditField == i && pData->iEditSelectAll ) {
			iRet = __xuiDatePickerDrawFill(pProxy, pDraw, __xuiDatePickerInsetRect(tRect, 4.0f), pResolved->iSelectionColor);
			if ( iRet != XUI_OK ) return iRet;
		}
		if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
			if ( pData->iEditKind == XUI_DATE_PICKER_EDIT_TIME && pData->iEditPanel == iPanel && pData->iEditField == i ) {
				snprintf(sText, sizeof(sText), "%s", pData->sEdit);
			} else {
				snprintf(sText, sizeof(sText), "%02d", arrValue[i]);
			}
			iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tRect, pResolved->iPopupTextColor, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	return XUI_OK;
}

static int __xuiDatePickerDrawComboPopup(xui_date_picker_data_t* pData, const xui_date_picker_data_t* pResolved, xui_proxy pProxy, xui_draw_context pDraw)
{
	char sText[32];
	xui_rect_t tDrop;
	xui_rect_t tItem;
	xtime tMonth;
	int64 iYear;
	int iMonth;
	int iPanel;
	int i;
	int64 iValue;
	int iRet;
	int bSelected;
	uint32_t iFill;
	uint32_t iText;

	if ( pData == NULL || pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_NONE ) return XUI_OK;
	iPanel = pData->iOpenComboPanel;
	if ( iPanel < 0 || iPanel >= __xuiDatePickerPanelCount(pData->iMode) ) return XUI_OK;
	tDrop = __xuiDatePickerComboPopupRect(pData, pData->iOpenComboKind, iPanel);
	if ( tDrop.fW <= 0.0f || tDrop.fH <= 0.0f ) return XUI_OK;
	iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tDrop, pResolved->iFieldColor);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawStroke(pProxy, pDraw, tDrop, 1.0f, pResolved->iAccentColor);
	if ( iRet != XUI_OK ) return iRet;
	tMonth = pData->tViewMonth[iPanel];
	xuiXrtDecodeSerial(tMonth, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	if ( pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_YEAR ) {
		for ( i = 0; i < XUI_DATE_PICKER_YEAR_OPTION_COUNT; i++ ) {
			iValue = __xuiDatePickerYearOptionValue(pData, iPanel, i);
			tItem = __xuiDatePickerInsetRect(__xuiDatePickerYearOptionRect(pData, iPanel, i), 2.0f);
			bSelected = (iValue == iYear);
			iFill = bSelected ? pResolved->iAccentColor : ((pData->iHoverPart == XUI_DATE_PICKER_PART_YEAR_OPTION && pData->iHoverPanel == iPanel && pData->iHoverIndex == i) ? pResolved->iOptionHoverColor : 0);
			iText = bSelected ? pResolved->iSelectedTextColor : pResolved->iPopupTextColor;
			if ( __xuiDatePickerAlpha(iFill) != 0 ) {
				iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tItem, iFill);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
				snprintf(sText, sizeof(sText), "%04lld", (long long)iValue);
				iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tItem, iText, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	} else {
		for ( i = 0; i < 12; i++ ) {
			tItem = __xuiDatePickerInsetRect(__xuiDatePickerMonthOptionRect(pData, iPanel, i), 2.0f);
			bSelected = (i + 1 == iMonth);
			iFill = bSelected ? pResolved->iAccentColor : ((pData->iHoverPart == XUI_DATE_PICKER_PART_MONTH_OPTION && pData->iHoverPanel == iPanel && pData->iHoverIndex == i) ? pResolved->iOptionHoverColor : 0);
			iText = bSelected ? pResolved->iSelectedTextColor : pResolved->iPopupTextColor;
			if ( __xuiDatePickerAlpha(iFill) != 0 ) {
				iRet = __xuiDatePickerDrawFill(pProxy, pDraw, tItem, iFill);
				if ( iRet != XUI_OK ) return iRet;
			}
			if ( pProxy->drawText != NULL && pResolved->pFont != NULL ) {
				snprintf(sText, sizeof(sText), "%02d", i + 1);
				iRet = pProxy->drawText(pProxy, pDraw, &(xui_text_item_t){.iSize=sizeof(xui_text_item_t), .pFont=pResolved->pFont, .sText=sText, .iTextSize=-1, .iFlags=XUI_TEXT_SHAPE_DEFAULT | ((XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP) & XUI_TEXT_RTL ? XUI_TEXT_SHAPE_RTL : 0)}, tItem, iText, XUI_TEXT_ALIGN_CENTER | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP);
				if ( iRet != XUI_OK ) return iRet;
			}
		}
	}
	return XUI_OK;
}

static int __xuiDatePickerPanelRender(xui_widget pPanel, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_widget pOwner;
	xui_date_picker_data_t* pData;
	xui_date_picker_data_t tResolved;
	xui_proxy pProxy;
	int i;
	int iRet;

	(void)iStateId;
	pOwner = (xui_widget)pUser;
	if ( (pPanel == NULL) || (pDraw == NULL) || (pOwner == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiDatePickerGetData(pOwner);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pOwner));
	if ( pProxy == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	__xuiDatePickerResolve(pOwner, pData, &tResolved);
	__xuiDatePickerUpdatePanelRects(pData);
	for ( i = 0; i < __xuiDatePickerPanelCount(pData->iMode); i++ ) {
		iRet = __xuiDatePickerDrawCalendar(pOwner, pData, &tResolved, pProxy, pDraw, i);
		if ( iRet != XUI_OK ) return iRet;
		iRet = __xuiDatePickerDrawTimePanel(pData, &tResolved, pProxy, pDraw, i);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( pProxy->drawRectFill != NULL ) {
		iRet = pProxy->drawRectFill(pProxy, pDraw, xuiInternalRectFromFloatNearest(XUI_DATE_PICKER_PAD, pData->fPopupHeight - 48.0f, pData->fPopupWidth - XUI_DATE_PICKER_PAD * 2.0f, 1.0f), tResolved.iSeparatorColor);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, tResolved.pFont, pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_TODAY], __xuiDatePickerHasDate(pData->iMode) ? "Today" : "Now", tResolved.iPopupTextColor, tResolved.iFieldColor, tResolved.iFieldBorderColor, 0);
	if ( iRet != XUI_OK ) return iRet;
	if ( pData->bNullable ) {
		iRet = __xuiDatePickerDrawButton(pProxy, pDraw, tResolved.pFont, pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_CLEAR], "Clear", tResolved.iPopupTextColor, tResolved.iFieldColor, tResolved.iFieldBorderColor, 0);
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, tResolved.pFont, pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_CANCEL], "Cancel", tResolved.iPopupTextColor, tResolved.iFieldColor, tResolved.iFieldBorderColor, 0);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawButton(pProxy, pDraw, tResolved.pFont, pData->arrFooterRect[XUI_DATE_PICKER_FOOTER_OK], "OK", tResolved.iSelectedTextColor, tResolved.iAccentColor, tResolved.iAccentColor, 1);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerDrawComboPopup(pData, &tResolved, pProxy, pDraw);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static void __xuiDatePickerPrepareDraft(xui_date_picker_data_t* pData)
{
	if ( pData == NULL ) return;
	__xuiDatePickerEnsureValue(pData);
	if ( pData->bHasValue ) {
		pData->tDraftValue = pData->tValue;
		pData->tDraftStart = pData->tStart;
		pData->tDraftEnd = pData->tEnd;
	} else {
		__xuiDatePickerDefaultValues(pData, &pData->tDraftStart, &pData->tDraftEnd);
		pData->tDraftValue = pData->tDraftStart;
	}
	pData->iActivePanel = 0;
	pData->iActiveTimePanel = 0;
	pData->iActiveTimeField = -1;
	pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
	pData->iOpenComboPanel = -1;
	__xuiDatePickerCancelEdit(pData);
	__xuiDatePickerInitViewMonth(pData);
}

static int __xuiDatePickerCommitDraft(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	xui_date_picker_proc onChange;
	xui_date_picker_proc onCommit;
	void* pChangeUser;
	void* pCommitUser;
	int bChanged;
	int iMode;
	int iRet;
	xtime tOldValue;
	xtime tOldStart;
	xtime tOldEnd;
	xtime tStart;
	xtime tEnd;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	tOldValue = pData->tValue;
	tOldStart = pData->tStart;
	tOldEnd = pData->tEnd;
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		__xuiDatePickerNormalizeRange(pData, &pData->tDraftStart, &pData->tDraftEnd);
		pData->tStart = pData->tDraftStart;
		pData->tEnd = pData->tDraftEnd;
		pData->tValue = pData->tStart;
		bChanged = !pData->bHasValue || (tOldStart != pData->tStart) || (tOldEnd != pData->tEnd);
	} else {
		pData->tDraftValue = __xuiDatePickerClampValue(pData, pData->tDraftValue);
		pData->tValue = pData->tDraftValue;
		pData->tStart = pData->tValue;
		pData->tEnd = pData->tValue;
		bChanged = !pData->bHasValue || (tOldValue != pData->tValue);
	}
	pData->bHasValue = 1;
	__xuiDatePickerSyncText(pData);
	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	onCommit = pData->onCommit;
	pCommitUser = pData->pCommitUser;
	iMode = pData->iMode;
	tStart = __xuiDatePickerIsRange(iMode) ? pData->tStart : pData->tValue;
	tEnd = __xuiDatePickerIsRange(iMode) ? pData->tEnd : 0;
	if ( bChanged ) {
		__xuiDatePickerIncrement(&pData->iChangeCount);
	}
	__xuiDatePickerIncrement(&pData->iCommitCount);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( bChanged ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( bChanged && onChange != NULL ) {
		onChange(pWidget, tStart, tEnd, iMode, pChangeUser);
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( onCommit != NULL ) onCommit(pWidget, tStart, tEnd, iMode, pCommitUser);
	return XUI_OK;
}

static int __xuiDatePickerCancelDraft(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_date_picker_proc proc;
	void* pUser;
	xtime tStart;
	xtime tEnd;
	int iMode;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerIncrement(&pData->iCancelCount);
	proc = pData->onCancel;
	pUser = pData->pCancelUser;
	iMode = pData->iMode;
	tStart = __xuiDatePickerIsRange(iMode) ? pData->tStart : pData->tValue;
	tEnd = __xuiDatePickerIsRange(iMode) ? pData->tEnd : 0;
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( proc != NULL ) proc(pWidget, tStart, tEnd, iMode, pUser);
	return XUI_OK;
}

static int __xuiDatePickerClearInternal(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	xui_date_picker_proc onClear;
	xui_date_picker_proc onChange;
	void* pClearUser;
	void* pChangeUser;
	int bChanged;
	int iMode;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pData->bNullable ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	bChanged = pData->bHasValue;
	pData->bHasValue = 0;
	pData->sText[0] = '\0';
	__xuiDatePickerIncrement(&pData->iClearCount);
	onClear = pData->onClear;
	pClearUser = pData->pClearUser;
	onChange = pData->onChange;
	pChangeUser = pData->pChangeUser;
	iMode = pData->iMode;
	if ( bChanged ) {
		__xuiDatePickerIncrement(&pData->iChangeCount);
	}
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( bChanged ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	if ( onClear != NULL ) {
		onClear(pWidget, 0, 0, iMode, pClearUser);
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	if ( bChanged && onChange != NULL ) onChange(pWidget, 0, 0, iMode, pChangeUser);
	return XUI_OK;
}

static int __xuiDatePickerCloseWithReason(xui_widget pWidget, xui_date_picker_data_t* pData, int iReason)
{
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	int iRet;
	int bWasOpen;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	bWasOpen = (pPopup != NULL) && xuiPopupIsOpen(pPopup);
	if ( bWasOpen ) {
		if ( pPanel != NULL ) {
			iRet = __xuiDatePickerPanelCancelPointer(pPanel, pData);
			if ( iRet != XUI_OK ) return iRet;
			if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		}
		pData->bClosingExplicit = 1;
		pData->iCloseReason = iReason;
		iRet = xuiPopupSetOpen(pPopup, 0);
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return (iRet == XUI_OK) ? XUI_OK : iRet;
		pData->bClosingExplicit = 0;
		pData->iCloseReason = XUI_DATE_PICKER_CLOSE_NONE;
		if ( iRet != XUI_OK ) return iRet;
	}
	iRet = __xuiDatePickerSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !bWasOpen ) return XUI_OK;
	if ( iReason == XUI_DATE_PICKER_CLOSE_CANCEL ) return __xuiDatePickerCancelDraft(pWidget, pData);
	if ( iReason == XUI_DATE_PICKER_CLOSE_COMMIT ) return __xuiDatePickerCommitDraft(pWidget, pData);
	if ( iReason == XUI_DATE_PICKER_CLOSE_CLEAR ) return __xuiDatePickerClearInternal(pWidget, pData);
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

static int __xuiDatePickerOpenInternal(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pData->pPopup == NULL) || (pData->pPanel == NULL) ) return XUI_ERROR_NOT_INITIALIZED;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	if ( xuiPopupIsOpen(pData->pPopup) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	__xuiDatePickerPrepareDraft(pData);
	iRet = __xuiDatePickerApplyPopupStyle(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetOpen(pPopup, 1);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiSetFocusWidget(pContext, pPanel);
	if ( iRet != XUI_OK ) {
		int iCloseRet;
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return iRet;
		pData->bClosingExplicit = 1;
		iCloseRet = xuiPopupSetOpen(pPopup, 0);
		if ( __xuiDatePickerAlive(pContext, pWidget, pData) ) pData->bClosingExplicit = 0;
		return (iCloseRet != XUI_OK) ? iCloseRet : iRet;
	}
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = __xuiDatePickerSyncState(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

static int __xuiDatePickerSelectDay(xui_widget pOwner, xui_date_picker_data_t* pData, int iPanel, int iIndex)
{
	xtime tDay;
	xtime tTime;

	if ( (pOwner == NULL) || (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ||
	     (iIndex < 0) || (iIndex >= XUI_DATE_PICKER_CELL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	tDay = pData->arrDayValue[iPanel][iIndex];
	if ( !__xuiDatePickerValueEnabled(pData, tDay) ) return XUI_OK;
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		if ( pData->iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE ) {
			tTime = xrtTimePart((iPanel == 1) ? pData->tDraftEnd : pData->tDraftStart);
			tDay = xrtDatePart(tDay) + tTime;
		}
		if ( iPanel == 1 ) {
			pData->tDraftEnd = __xuiDatePickerClampValue(pData, tDay);
			pData->iActivePanel = 0;
		} else {
			pData->tDraftStart = __xuiDatePickerClampValue(pData, tDay);
			pData->iActivePanel = 1;
		}
		__xuiDatePickerNormalizeRange(pData, &pData->tDraftStart, &pData->tDraftEnd);
	} else {
		if ( pData->iMode == XUI_DATE_PICKER_MODE_DATETIME ) {
			tDay = xrtDatePart(tDay) + xrtTimePart(pData->tDraftValue);
		}
		pData->tDraftValue = __xuiDatePickerClampValue(pData, tDay);
	}
	return __xuiDatePickerNotifyDraft(pOwner, pData);
}

static void __xuiDatePickerSetDraftTime(xui_date_picker_data_t* pData, int iPanel, int iField, int iDelta)
{
	xtime* pTarget;
	xtime tValue;
	xtime tDate;
	int iHour;
	int iMinute;
	int iSecond;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iField < 0) || (iField > 2) ) return;
	pTarget = &pData->tDraftValue;
	if ( __xuiDatePickerIsRange(pData->iMode) ) pTarget = (iPanel == 1) ? &pData->tDraftEnd : &pData->tDraftStart;
	tValue = *pTarget;
	xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	if ( iField == 0 ) iHour = __xuiDatePickerClampInt(iHour + iDelta, 0, 23);
	else if ( iField == 1 ) iMinute = __xuiDatePickerClampInt(iMinute + iDelta, 0, 59);
	else iSecond = __xuiDatePickerClampInt(iSecond + iDelta, 0, 59);
	tDate = ((pData->iMode == XUI_DATE_PICKER_MODE_DATETIME) || (pData->iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE)) ? xrtDatePart(tValue) : 0;
	*pTarget = __xuiDatePickerClampValue(pData, tDate + __xuiDatePickerMakeTime(iHour, iMinute, iSecond, pData->bShowSecond));
	if ( __xuiDatePickerIsRange(pData->iMode) ) __xuiDatePickerNormalizeRange(pData, &pData->tDraftStart, &pData->tDraftEnd);
}

static void __xuiDatePickerSetDraftTimeValue(xui_date_picker_data_t* pData, int iPanel, int iField, int iValue)
{
	xtime* pTarget;
	xtime tValue;
	xtime tDate;
	int iHour;
	int iMinute;
	int iSecond;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iField < 0) || (iField > 2) ) return;
	pTarget = &pData->tDraftValue;
	if ( __xuiDatePickerIsRange(pData->iMode) ) pTarget = (iPanel == 1) ? &pData->tDraftEnd : &pData->tDraftStart;
	tValue = *pTarget;
	xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	if ( iField == 0 ) iHour = __xuiDatePickerClampInt(iValue, 0, 23);
	else if ( iField == 1 ) iMinute = __xuiDatePickerClampInt(iValue, 0, 59);
	else iSecond = __xuiDatePickerClampInt(iValue, 0, 59);
	tDate = ((pData->iMode == XUI_DATE_PICKER_MODE_DATETIME) || (pData->iMode == XUI_DATE_PICKER_MODE_DATETIME_RANGE)) ? xrtDatePart(tValue) : 0;
	*pTarget = __xuiDatePickerClampValue(pData, tDate + __xuiDatePickerMakeTime(iHour, iMinute, iSecond, pData->bShowSecond));
	if ( __xuiDatePickerIsRange(pData->iMode) ) __xuiDatePickerNormalizeRange(pData, &pData->tDraftStart, &pData->tDraftEnd);
}

static void __xuiDatePickerCancelEdit(xui_date_picker_data_t* pData)
{
	if ( pData == NULL ) return;
	pData->iEditKind = XUI_DATE_PICKER_EDIT_NONE;
	pData->iEditPanel = -1;
	pData->iEditField = -1;
	pData->iEditSelectAll = 0;
	pData->iEditError = 0;
	pData->iEditLen = 0;
	pData->sEdit[0] = '\0';
}

static void __xuiDatePickerBeginYearEdit(xui_date_picker_data_t* pData, int iPanel)
{
	int64 iYear;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return;
	xuiXrtDecodeSerial(pData->tViewMonth[iPanel], &iYear, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	pData->iEditKind = XUI_DATE_PICKER_EDIT_YEAR;
	pData->iEditPanel = iPanel;
	pData->iEditField = -1;
	snprintf(pData->sEdit, sizeof(pData->sEdit), "%04lld", (long long)iYear);
	pData->iEditLen = (int)strlen(pData->sEdit);
	pData->iEditSelectAll = 1;
	pData->iEditError = 0;
}

static void __xuiDatePickerBeginTimeEdit(xui_date_picker_data_t* pData, int iPanel, int iField)
{
	xtime tValue;
	int iHour;
	int iMinute;
	int iSecond;
	int iValue;

	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iField < 0) || (iField > 2) ) return;
	tValue = __xuiDatePickerDraftForPanel(pData, iPanel);
	xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, &iHour, &iMinute, &iSecond, NULL, NULL);
	iValue = (iField == 0) ? iHour : ((iField == 1) ? iMinute : iSecond);
	pData->iEditKind = XUI_DATE_PICKER_EDIT_TIME;
	pData->iEditPanel = iPanel;
	pData->iEditField = iField;
	snprintf(pData->sEdit, sizeof(pData->sEdit), "%02d", iValue);
	pData->iEditLen = (int)strlen(pData->sEdit);
	pData->iEditSelectAll = 1;
	pData->iEditError = 0;
}

static void __xuiDatePickerSetViewMonth(xui_widget pOwner, xui_date_picker_data_t* pData, int iPanel, int64 iYear, int iMonth)
{
	xtime tMonth;

	(void)pOwner;
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return;
	iYear = __xuiDatePickerClampYear(iYear);
	iMonth = __xuiDatePickerClampInt(iMonth, 1, 12);
	tMonth = xuiXrtDateSerial(iYear, iMonth, 1);
	pData->tViewMonth[iPanel] = tMonth;
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		if ( iPanel == 0 && pData->tViewMonth[1] <= pData->tViewMonth[0] ) {
			pData->tViewMonth[1] = xuiXrtDateAddMonth(0, 1, pData->tViewMonth[0]);
		} else if ( iPanel == 1 && pData->tViewMonth[0] >= pData->tViewMonth[1] ) {
			pData->tViewMonth[0] = xuiXrtDateAddMonth(0, -1, pData->tViewMonth[1]);
		}
	} else if ( iPanel == 0 ) {
		pData->tViewMonth[1] = xuiXrtDateAddMonth(0, 1, pData->tViewMonth[0]);
	}
}

static int __xuiDatePickerCommitEdit(xui_widget pOwner, xui_date_picker_data_t* pData, int bNotify)
{
	int iValue;
	int64 iYear;
	int64 iEditYear;
	int iMonth;
	int iMax;

	if ( (pOwner == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiDatePickerEditing(pData) ) return XUI_OK;
	if ( pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR ) {
		if ( pData->iEditLen <= 0 ) {
			pData->iEditError = 1;
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		if ( !__xuiDatePickerParseYearText(pData->sEdit, &iEditYear) ) {
			pData->iEditError = 1;
			return XUI_ERROR_INVALID_ARGUMENT;
		}
		xuiXrtDecodeSerial(pData->tViewMonth[pData->iEditPanel], &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		(void)iYear;
		__xuiDatePickerSetViewMonth(pOwner, pData, pData->iEditPanel, iEditYear, iMonth);
		__xuiDatePickerCancelEdit(pData);
		return __xuiDatePickerInvalidateAll(pOwner, pData);
	}
	if ( pData->iEditKind == XUI_DATE_PICKER_EDIT_TIME ) {
		iValue = (pData->iEditLen > 0) ? atoi(pData->sEdit) : 0;
		iMax = __xuiDatePickerTimeFieldMax(pData->iEditField);
		iValue = __xuiDatePickerClampInt(iValue, 0, iMax);
		__xuiDatePickerSetDraftTimeValue(pData, pData->iEditPanel, pData->iEditField, iValue);
		__xuiDatePickerCancelEdit(pData);
		return bNotify ? __xuiDatePickerNotifyDraft(pOwner, pData) :
			__xuiDatePickerInvalidateAll(pOwner, pData);
	}
	__xuiDatePickerCancelEdit(pData);
	return XUI_OK;
}

static int __xuiDatePickerSetDraftNow(xui_widget pOwner, xui_date_picker_data_t* pData)
{
	xtime tStart;
	xtime tEnd;

	if ( (pOwner == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerDefaultValues(pData, &tStart, &tEnd);
	pData->tDraftValue = tStart;
	pData->tDraftStart = tStart;
	pData->tDraftEnd = tEnd;
	__xuiDatePickerInitViewMonth(pData);
	return __xuiDatePickerNotifyDraft(pOwner, pData);
}

static int __xuiDatePickerMoveDraftDate(xui_widget pOwner, xui_date_picker_data_t* pData,
	const xui_event_t* pEvent)
{
	xtime tValue = 0;
	xtime tMoved;
	int iPanel;
	int iUnit;
	int iRet;
	int64 iAmount;
	int iWeekday;
	int iOffset;
	int64 iYear;
	int iMonth;

	if ( (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( !__xuiDatePickerHasDate(pData->iMode) ) return XUI_OK;
	iUnit = 0;
	iAmount = 0;
	switch ( pEvent->iKey ) {
	case XUI_KEY_LEFT: iUnit = XTIME_UNIT_DAY; iAmount = -1; break;
	case XUI_KEY_RIGHT: iUnit = XTIME_UNIT_DAY; iAmount = 1; break;
	case XUI_KEY_UP: iUnit = XTIME_UNIT_DAY; iAmount = -7; break;
	case XUI_KEY_DOWN: iUnit = XTIME_UNIT_DAY; iAmount = 7; break;
	case XUI_KEY_PAGE_UP: iUnit = XTIME_UNIT_MONTH; iAmount = ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ? -12 : -1; break;
	case XUI_KEY_PAGE_DOWN: iUnit = XTIME_UNIT_MONTH; iAmount = ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ? 12 : 1; break;
	case XUI_KEY_HOME:
	case XUI_KEY_END:
		iPanel = __xuiDatePickerIsRange(pData->iMode) ? pData->iActivePanel : 0;
		tValue = __xuiDatePickerDraftForPanel(pData, iPanel);
		xuiXrtDecodeSerial(tValue, NULL, NULL, NULL, NULL, NULL, NULL, &iWeekday, NULL);
		iOffset = (iWeekday - pData->iFirstDayOfWeek + 7) % 7;
		iUnit = XTIME_UNIT_DAY;
		iAmount = (pEvent->iKey == XUI_KEY_HOME) ? -iOffset : (6 - iOffset);
		break;
	default:
		return XUI_OK;
	}
	iPanel = __xuiDatePickerIsRange(pData->iMode) ? pData->iActivePanel : 0;
	if ( (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) iPanel = 0;
	if ( pEvent->iKey != XUI_KEY_HOME && pEvent->iKey != XUI_KEY_END ) {
		tValue = __xuiDatePickerDraftForPanel(pData, iPanel);
	}
	tMoved = tValue;
	if ( !xrtTimeAdd(tValue, iAmount, iUnit, &tMoved) ) return XUI_EVENT_DISPATCH_STOP;
	tMoved = __xuiDatePickerClampValue(pData, tMoved);
	if ( !__xuiDatePickerValueEnabled(pData, tMoved) ) return XUI_EVENT_DISPATCH_STOP;
	if ( __xuiDatePickerIsRange(pData->iMode) ) {
		if ( iPanel == 1 ) pData->tDraftEnd = tMoved;
		else pData->tDraftStart = tMoved;
		__xuiDatePickerNormalizeRange(pData, &pData->tDraftStart, &pData->tDraftEnd);
	} else {
		pData->tDraftValue = tMoved;
	}
	xuiXrtDecodeSerial(tMoved, &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
	__xuiDatePickerSetViewMonth(pOwner, pData, iPanel, iYear, iMonth);
	iRet = __xuiDatePickerNotifyDraft(pOwner, pData);
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiDatePickerHitPanel(xui_date_picker_data_t* pData, float fX, float fY, int* pPanel, int* pIndex)
{
	int i;
	int j;

	if ( pPanel != NULL ) *pPanel = -1;
	if ( pIndex != NULL ) *pIndex = -1;
	if ( pData == NULL ) return XUI_DATE_PICKER_PART_NONE;
	if ( pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_YEAR && pData->iOpenComboPanel >= 0 ) {
		for ( j = 0; j < XUI_DATE_PICKER_YEAR_OPTION_COUNT; j++ ) {
			if ( __xuiDatePickerRectContains(__xuiDatePickerYearOptionRect(pData, pData->iOpenComboPanel, j), fX, fY) ) {
				if ( pPanel != NULL ) *pPanel = pData->iOpenComboPanel;
				if ( pIndex != NULL ) *pIndex = j;
				return XUI_DATE_PICKER_PART_YEAR_OPTION;
			}
		}
	}
	if ( pData->iOpenComboKind == XUI_DATE_PICKER_COMBO_MONTH && pData->iOpenComboPanel >= 0 ) {
		for ( j = 0; j < 12; j++ ) {
			if ( __xuiDatePickerRectContains(__xuiDatePickerMonthOptionRect(pData, pData->iOpenComboPanel, j), fX, fY) ) {
				if ( pPanel != NULL ) *pPanel = pData->iOpenComboPanel;
				if ( pIndex != NULL ) *pIndex = j;
				return XUI_DATE_PICKER_PART_MONTH_OPTION;
			}
		}
	}
	for ( i = 0; i < 4; i++ ) {
		if ( __xuiDatePickerRectContains(pData->arrFooterRect[i], fX, fY) ) {
			if ( pIndex != NULL ) *pIndex = i;
			if ( i == XUI_DATE_PICKER_FOOTER_TODAY ) return XUI_DATE_PICKER_PART_TODAY;
			if ( i == XUI_DATE_PICKER_FOOTER_CLEAR ) return XUI_DATE_PICKER_PART_CLEAR;
			if ( i == XUI_DATE_PICKER_FOOTER_CANCEL ) return XUI_DATE_PICKER_PART_CANCEL;
			if ( i == XUI_DATE_PICKER_FOOTER_OK ) return XUI_DATE_PICKER_PART_OK;
		}
	}
	for ( i = 0; i < __xuiDatePickerPanelCount(pData->iMode); i++ ) {
		if ( __xuiDatePickerRectContains(pData->tPrevRect[i], fX, fY) ) {
			if ( pPanel != NULL ) *pPanel = i;
			return XUI_DATE_PICKER_PART_PREV;
		}
		if ( __xuiDatePickerRectContains(pData->tNextRect[i], fX, fY) ) {
			if ( pPanel != NULL ) *pPanel = i;
			return XUI_DATE_PICKER_PART_NEXT;
		}
		if ( __xuiDatePickerRectContains(pData->tYearRect[i], fX, fY) ) {
			if ( pPanel != NULL ) *pPanel = i;
			return XUI_DATE_PICKER_PART_YEAR_FIELD;
		}
		if ( __xuiDatePickerRectContains(pData->tMonthRect[i], fX, fY) ) {
			if ( pPanel != NULL ) *pPanel = i;
			return XUI_DATE_PICKER_PART_MONTH_FIELD;
		}
		for ( j = 0; j < XUI_DATE_PICKER_CELL_COUNT; j++ ) {
			if ( __xuiDatePickerRectContains(pData->arrDayRect[i][j], fX, fY) ) {
				if ( pPanel != NULL ) *pPanel = i;
				if ( pIndex != NULL ) *pIndex = j;
				return XUI_DATE_PICKER_PART_DAY;
			}
		}
		for ( j = 0; j < (pData->bShowSecond ? 3 : 2); j++ ) {
			if ( __xuiDatePickerRectContains(pData->arrTimeRect[i][j], fX, fY) ) {
				if ( pPanel != NULL ) *pPanel = i;
				if ( pIndex != NULL ) *pIndex = j;
				return XUI_DATE_PICKER_PART_TIME_HOUR + j;
			}
		}
	}
	return XUI_DATE_PICKER_PART_NONE;
}

static int __xuiDatePickerPanelActivate(xui_widget pOwner, xui_date_picker_data_t* pData,
	int iHit, int iPanel, int iIndex)
{
	int64 iYear;
	int iMonth;
	int iRet;

	if ( (pOwner == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iHit < XUI_DATE_PICKER_PART_TIME_HOUR || iHit > XUI_DATE_PICKER_PART_TIME_SECOND ) {
		pData->iActiveTimeField = -1;
	}
	if ( iHit == XUI_DATE_PICKER_PART_YEAR_OPTION ) {
		xuiXrtDecodeSerial(pData->tViewMonth[iPanel], NULL, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		__xuiDatePickerCancelEdit(pData);
		pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
		pData->iOpenComboPanel = -1;
		__xuiDatePickerSetViewMonth(pOwner, pData, iPanel, __xuiDatePickerYearOptionValue(pData, iPanel, iIndex), iMonth);
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_MONTH_OPTION ) {
		xuiXrtDecodeSerial(pData->tViewMonth[iPanel], &iYear, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
		__xuiDatePickerCancelEdit(pData);
		pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
		pData->iOpenComboPanel = -1;
		__xuiDatePickerSetViewMonth(pOwner, pData, iPanel, iYear, iIndex + 1);
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_YEAR_FIELD ) {
		pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_YEAR;
		pData->iOpenComboPanel = iPanel;
		__xuiDatePickerBeginYearEdit(pData, iPanel);
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_MONTH_FIELD ) {
		__xuiDatePickerCancelEdit(pData);
		pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_MONTH;
		pData->iOpenComboPanel = iPanel;
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( __xuiDatePickerEditing(pData) ) {
		iRet = __xuiDatePickerCommitEdit(pOwner, pData, 1);
		if ( iRet != XUI_OK ) return (iRet == XUI_ERROR_INVALID_ARGUMENT) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		if ( !xuiInternalWidgetIsValid(pOwner) ) return XUI_EVENT_DISPATCH_STOP;
		pData = __xuiDatePickerGetData(pOwner);
		if ( pData == NULL ) return XUI_EVENT_DISPATCH_STOP;
	}
	pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
	pData->iOpenComboPanel = -1;
	if ( iHit == XUI_DATE_PICKER_PART_PREV || iHit == XUI_DATE_PICKER_PART_NEXT ) {
		pData->tViewMonth[iPanel] = xuiXrtDateAddMonth(0, (iHit == XUI_DATE_PICKER_PART_PREV) ? -1 : 1, pData->tViewMonth[iPanel]);
		xuiXrtDecodeSerial(pData->tViewMonth[iPanel], &iYear, &iMonth, NULL, NULL, NULL, NULL, NULL, NULL);
		__xuiDatePickerSetViewMonth(pOwner, pData, iPanel, iYear, iMonth);
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_DAY ) {
		iRet = __xuiDatePickerSelectDay(pOwner, pData, iPanel, iIndex);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit >= XUI_DATE_PICKER_PART_TIME_HOUR && iHit <= XUI_DATE_PICKER_PART_TIME_SECOND ) {
		pData->iActiveTimePanel = iPanel;
		pData->iActiveTimeField = iIndex;
		__xuiDatePickerBeginTimeEdit(pData, iPanel, iIndex);
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_TODAY ) {
		iRet = __xuiDatePickerSetDraftNow(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_CLEAR ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_CLEAR);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_CANCEL ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_CANCEL);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( iHit == XUI_DATE_PICKER_PART_OK ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_COMMIT);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiDatePickerPanelPointerDown(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iPanel;
	int iIndex;
	int iHit;
	int iRet;

	if ( (pPanel == NULL) || (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( (pEvent->iButton != XUI_POINTER_BUTTON_LEFT) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) return XUI_OK;
	if ( pData->tPointerOwner.bActive ) {
		return xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ?
			XUI_EVENT_DISPATCH_STOP : XUI_OK;
	}
	pContext = xuiWidgetGetContext(pPanel);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiDatePickerUpdatePanelRects(pData);
	iHit = __xuiDatePickerHitPanel(pData, fX, fY, &iPanel, &iIndex);
	pData->iActivePart = iHit;
	pData->iPressedPanel = iPanel;
	pData->iActiveIndex = iIndex;
	if ( iPanel >= 0 ) pData->iActivePanel = iPanel;
	iRet = xuiInternalPointerOwnerBegin(pContext, pPanel, pEvent,
		&pData->tPointerOwner);
	if ( iRet != XUI_OK ) {
		pData->iActivePart = XUI_DATE_PICKER_PART_NONE;
		pData->iPressedPanel = -1;
		pData->iActiveIndex = -1;
		return iRet;
	}
	if ( !__xuiDatePickerAlive(pContext, pOwner, pData) ) return XUI_EVENT_DISPATCH_STOP;
	iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiDatePickerPanelPointerUp(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iHit;
	int iPanel;
	int iIndex;
	int iActivePart;
	int iActivePanel;
	int iActiveIndex;
	int iRet;

	if ( (pPanel == NULL) || (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iButton != XUI_POINTER_BUTTON_LEFT || pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	if ( !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pPanel);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiDatePickerUpdatePanelRects(pData);
	iHit = __xuiDatePickerHitPanel(pData, fX, fY, &iPanel, &iIndex);
	iActivePart = pData->iActivePart;
	iActivePanel = pData->iPressedPanel;
	iActiveIndex = pData->iActiveIndex;
	pData->iActivePart = XUI_DATE_PICKER_PART_NONE;
	pData->iPressedPanel = -1;
	pData->iActiveIndex = -1;
	iRet = xuiInternalPointerOwnerEnd(pContext, pPanel, pEvent,
		&pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pOwner, pData) ) return XUI_EVENT_DISPATCH_STOP;
	iRet = xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	if ( iRet != XUI_OK ) return iRet;
	if ( iHit != iActivePart || iPanel != iActivePanel || iIndex != iActiveIndex ) return XUI_EVENT_DISPATCH_STOP;
	return __xuiDatePickerPanelActivate(pOwner, pData, iHit, iPanel, iIndex);
}

static int __xuiDatePickerPanelCancelPointer(xui_widget pPanel, xui_date_picker_data_t* pData)
{
	xui_context pContext;
	int bChanged;
	int iRet;

	if ( (pPanel == NULL) || (pData == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pPanel);
	bChanged = (pData->iActivePart != XUI_DATE_PICKER_PART_NONE) ||
		(pData->iPressedPanel >= 0) || (pData->iActiveIndex >= 0) ||
		pData->tPointerOwner.bActive;
	pData->iActivePart = XUI_DATE_PICKER_PART_NONE;
	pData->iPressedPanel = -1;
	pData->iActiveIndex = -1;
	iRet = xuiInternalPointerOwnerCancel(pContext, pPanel, &pData->tPointerOwner);
	if ( iRet != XUI_OK ) return iRet;
	if ( !xuiInternalContextIsValid(pContext) || xuiInternalContextDestroyPending(pContext) ||
	     !xuiInternalWidgetIsValid(pPanel) ) return XUI_OK;
	return bChanged ? xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER) : XUI_OK;
}

static int __xuiDatePickerPanelPointerMove(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	int iPanel;
	int iIndex;
	int iHit;
	float fX;
	float fY;

	(void)pOwner;
	if ( (pPanel == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	if ( pData->tPointerOwner.bActive &&
	     !xuiInternalPointerOwnerMatches(&pData->tPointerOwner, pEvent) ) {
		return XUI_OK;
	}
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiDatePickerUpdatePanelRects(pData);
	iHit = __xuiDatePickerHitPanel(pData, fX, fY, &iPanel, &iIndex);
	if ( (pData->iHoverPart != iHit) || (pData->iHoverPanel != iPanel) || (pData->iHoverIndex != iIndex) ) {
		pData->iHoverPart = iHit;
		pData->iHoverPanel = iPanel;
		pData->iHoverIndex = iIndex;
		return xuiWidgetInvalidate(pPanel, XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
	}
	return XUI_OK;
}

static int __xuiDatePickerPanelWheel(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_rect_t tWorld;
	float fX;
	float fY;
	int iPanel;
	int iIndex;
	int iHit;
	int iDelta;
	int iRet;

	if ( (pPanel == NULL) || (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->fWheelY == 0.0f ) return XUI_OK;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	fX = pEvent->fX - tWorld.fX;
	fY = pEvent->fY - tWorld.fY;
	__xuiDatePickerUpdatePanelRects(pData);
	iHit = __xuiDatePickerHitPanel(pData, fX, fY, &iPanel, &iIndex);
	if ( iHit >= XUI_DATE_PICKER_PART_TIME_HOUR && iHit <= XUI_DATE_PICKER_PART_TIME_SECOND ) {
		iDelta = (pEvent->fWheelY > 0.0f) ? 1 : -1;
		pData->iActiveTimePanel = iPanel;
		pData->iActiveTimeField = iIndex;
		__xuiDatePickerSetDraftTime(pData, iPanel, iIndex, iDelta);
		iRet = __xuiDatePickerNotifyDraft(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	return XUI_OK;
}

static int __xuiDatePickerPanelKeyDown(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	int iDelta;
	int iRet;

	(void)pPanel;
	if ( (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	if ( (pEvent->iKey == XUI_KEY_F4 && pEvent->iModifiers == 0u) ||
	     (((pEvent->iModifiers & XUI_MOD_ALT) != 0u) && pEvent->iKey == XUI_KEY_UP) ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_SILENT);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( ((pEvent->iModifiers & XUI_MOD_ALT) != 0u) && pEvent->iKey == XUI_KEY_DOWN ) {
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( __xuiDatePickerEditing(pData) ) {
		if ( (pEvent->iKey == 'A' || pEvent->iKey == 'a') && ((pEvent->iModifiers & XUI_MOD_CTRL) != 0u) ) {
			pData->iEditSelectAll = 1;
			iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_ENTER ) {
			iRet = __xuiDatePickerCommitEdit(pOwner, pData, 1);
			return (iRet == XUI_OK || iRet == XUI_ERROR_INVALID_ARGUMENT) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
			__xuiDatePickerCancelEdit(pData);
			pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
			pData->iOpenComboPanel = -1;
			iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_BACKSPACE ) {
			if ( pData->iEditSelectAll ) {
				pData->iEditLen = 0;
				pData->sEdit[0] = '\0';
				pData->iEditSelectAll = 0;
			} else if ( pData->iEditLen > 0 ) {
				pData->sEdit[--pData->iEditLen] = '\0';
			}
			pData->iEditError = 0;
			iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		if ( pEvent->iKey == XUI_KEY_DELETE ) {
			pData->iEditLen = 0;
			pData->sEdit[0] = '\0';
			pData->iEditSelectAll = 0;
			pData->iEditError = 0;
			iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
		return XUI_EVENT_DISPATCH_STOP;
	}
	if ( pData->iOpenComboKind != XUI_DATE_PICKER_COMBO_NONE ) {
		if ( pEvent->iKey == XUI_KEY_ESCAPE || pEvent->iKey == XUI_KEY_ENTER ) {
			pData->iOpenComboKind = XUI_DATE_PICKER_COMBO_NONE;
			pData->iOpenComboPanel = -1;
			iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
			return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
		}
	}
	if ( pEvent->iKey == XUI_KEY_ESCAPE ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_CANCEL);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( pEvent->iKey == XUI_KEY_ENTER ) {
		iRet = __xuiDatePickerCloseWithReason(pOwner, pData, XUI_DATE_PICKER_CLOSE_COMMIT);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( (pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN) && pData->iActiveTimeField >= 0 ) {
		iDelta = (pEvent->iKey == XUI_KEY_UP) ? 1 : -1;
		__xuiDatePickerSetDraftTime(pData, pData->iActiveTimePanel, pData->iActiveTimeField, iDelta);
		iRet = __xuiDatePickerNotifyDraft(pOwner, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	iRet = __xuiDatePickerMoveDraftDate(pOwner, pData, pEvent);
	if ( iRet != XUI_OK ) return iRet;
	return XUI_OK;
}

static int __xuiDatePickerPanelText(xui_widget pPanel, xui_widget pOwner, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	uint32_t iCodepoint;
	int iLimit;
	int iRet;

	(void)pPanel;
	if ( (pOwner == NULL) || (pData == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiDatePickerEditing(pData) || pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ) return XUI_OK;
	iCodepoint = pEvent->iCodepoint;
	if ( iCodepoint == 0 && pEvent->iTextSize > 0 ) {
		iCodepoint = (uint32_t)(unsigned char)pEvent->sText[0];
	}
	iLimit = (pData->iEditKind == XUI_DATE_PICKER_EDIT_YEAR) ? XUI_DATE_PICKER_YEAR_INPUT_LIMIT : 2;
	if ( iCodepoint < '0' || iCodepoint > '9' ) return XUI_EVENT_DISPATCH_STOP;
	if ( pData->iEditSelectAll ) {
		pData->iEditLen = 0;
		pData->sEdit[0] = '\0';
		pData->iEditSelectAll = 0;
	}
	if ( pData->iEditLen < iLimit ) {
		pData->sEdit[pData->iEditLen++] = (char)iCodepoint;
		pData->sEdit[pData->iEditLen] = '\0';
		__xuiDatePickerClampTimeEditText(pData);
		pData->iEditError = 0;
		iRet = __xuiDatePickerInvalidateAll(pOwner, pData);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_EVENT_DISPATCH_STOP;
}

static int __xuiDatePickerPanelEvent(xui_widget pPanel, const xui_event_t* pEvent, void* pUser)
{
	xui_widget pOwner;
	xui_date_picker_data_t* pData;

	pOwner = (xui_widget)pUser;
	if ( (pPanel == NULL) || (pEvent == NULL) || (pOwner == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiDatePickerGetData(pOwner);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pEvent->iType == XUI_EVENT_POINTER_CAPTURE_LOST ) {
		if ( !xuiInternalPointerOwnerCaptureLost(&pData->tPointerOwner, pEvent) ) return XUI_OK;
		return __xuiDatePickerPanelCancelPointer(pPanel, pData);
	}
	if ( !xuiWidgetGetEffectiveEnabled(pOwner) || !xuiWidgetGetEffectiveVisible(pOwner) ||
	     (pData->pPopup == NULL) || !xuiPopupIsOpen(pData->pPopup) ) {
		if ( pData->tPointerOwner.bActive ) return __xuiDatePickerPanelCancelPointer(pPanel, pData);
		return XUI_OK;
	}
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		return __xuiDatePickerPanelPointerDown(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_POINTER_MOVE:
		return __xuiDatePickerPanelPointerMove(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_POINTER_WHEEL:
		return __xuiDatePickerPanelWheel(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_POINTER_UP:
		return __xuiDatePickerPanelPointerUp(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_KEY_DOWN:
		return __xuiDatePickerPanelKeyDown(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_TEXT:
		return __xuiDatePickerPanelText(pPanel, pOwner, pData, pEvent);
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiDatePickerSyncState(pOwner, pData);
	default:
		return XUI_OK;
	}
}

static int __xuiDatePickerPointerDown(xui_widget pWidget, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	xui_context pContext;
	int iRet;

	if ( (pEvent->iButton != XUI_POINTER_BUTTON_LEFT) || (pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE) ) return XUI_OK;
	if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iRet = xuiSetFocusWidget(pContext, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_EVENT_DISPATCH_STOP;
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) {
		iRet = __xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT);
	} else {
		iRet = __xuiDatePickerOpenInternal(pWidget, pData);
	}
	return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
}

static int __xuiDatePickerKeyDown(xui_widget pWidget, xui_date_picker_data_t* pData, const xui_event_t* pEvent)
{
	int iRet;

	if ( pEvent->iPhase == XUI_EVENT_PHASE_BUBBLE ||
	     !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) return XUI_OK;
	if ( pEvent->iKey == XUI_KEY_F4 && pEvent->iModifiers == 0u ) {
		iRet = (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup)) ?
			__xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT) :
			__xuiDatePickerOpenInternal(pWidget, pData);
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( (pEvent->iModifiers & XUI_MOD_ALT) != 0u &&
	     (pEvent->iKey == XUI_KEY_UP || pEvent->iKey == XUI_KEY_DOWN) ) {
		if ( pEvent->iKey == XUI_KEY_UP ) {
			iRet = (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup)) ?
				__xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT) : XUI_OK;
		} else {
			iRet = (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup)) ?
				XUI_OK : __xuiDatePickerOpenInternal(pWidget, pData);
		}
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	if ( (pEvent->iKey == XUI_KEY_ENTER) || (pEvent->iKey == XUI_KEY_SPACE) ) {
		if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) {
			iRet = __xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT);
		} else {
			iRet = __xuiDatePickerOpenInternal(pWidget, pData);
		}
		return (iRet == XUI_OK) ? (int)XUI_EVENT_DISPATCH_STOP : iRet;
	}
	return XUI_OK;
}

static int __xuiDatePickerEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_date_picker_data_t* pData;

	(void)pUser;
	if ( (pWidget == NULL) || (pEvent == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	switch ( pEvent->iType ) {
	case XUI_EVENT_POINTER_DOWN:
		return __xuiDatePickerPointerDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_CLICK:
		if ( pEvent->iButton == XUI_POINTER_BUTTON_LEFT &&
		     xuiWidgetGetEffectiveEnabled(pWidget) && xuiWidgetGetEffectiveVisible(pWidget) ) {
			return XUI_EVENT_DISPATCH_STOP;
		}
		break;
	case XUI_EVENT_KEY_DOWN:
		return __xuiDatePickerKeyDown(pWidget, pData, pEvent);
	case XUI_EVENT_POINTER_ENTER:
	case XUI_EVENT_POINTER_LEAVE:
	case XUI_EVENT_POINTER_MOVE:
	case XUI_EVENT_FOCUS:
	case XUI_EVENT_BLUR:
		return __xuiDatePickerSyncState(pWidget, pData);
	case XUI_EVENT_BOUNDS_CHANGED:
		if ( (pData->pPopup != NULL) && xuiPopupIsOpen(pData->pPopup) ) {
			return __xuiDatePickerApplyPopupStyle(pWidget, pData);
		}
		return XUI_OK;
	case XUI_EVENT_ENABLED_CHANGED:
	case XUI_EVENT_VISIBLE_CHANGED:
		if ( !xuiWidgetGetEffectiveEnabled(pWidget) || !xuiWidgetGetEffectiveVisible(pWidget) ) {
			return __xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT);
		}
		return __xuiDatePickerSyncState(pWidget, pData);
	default:
		break;
	}
	return XUI_OK;
}

static void __xuiDatePickerPopupChanged(xui_widget pPopup, int bOpen, void* pUser)
{
	xui_widget pOwner;
	xui_date_picker_data_t* pData;
	xui_context pContext;

	(void)pPopup;
	pOwner = (xui_widget)pUser;
	if ( pOwner == NULL ) return;
	pData = __xuiDatePickerGetData(pOwner);
	if ( pData == NULL ) return;
	pContext = xuiWidgetGetContext(pOwner);
	if ( pData->pPanel != NULL ) (void)__xuiDatePickerPanelCancelPointer(pData->pPanel, pData);
	if ( !__xuiDatePickerAlive(pContext, pOwner, pData) ) return;
	if ( pData->bClosingExplicit ) return;
	(void)__xuiDatePickerSyncState(pOwner, pData);
	if ( !__xuiDatePickerAlive(pContext, pOwner, pData) ) return;
	(void)__xuiDatePickerInvalidateAll(pOwner, pData);
	if ( !__xuiDatePickerAlive(pContext, pOwner, pData) ) return;
	if ( !bOpen && !pData->bClosingExplicit ) {
		(void)__xuiDatePickerCancelDraft(pOwner, pData);
	}
}

static void __xuiDatePickerDefaultLayout(xui_layout_t* pLayout)
{
	memset(pLayout, 0, sizeof(*pLayout));
	pLayout->iLayoutType = XUI_LAYOUT_MANUAL;
	pLayout->iWidthMode = XUI_SIZE_CONTENT;
	pLayout->iHeightMode = XUI_SIZE_CONTENT;
	pLayout->iFlowMode = XUI_FLOW_ABSOLUTE;
	pLayout->iDock = XUI_DOCK_FILL;
	pLayout->iOverflow = XUI_OVERFLOW_VISIBLE;
	pLayout->iAlignX = XUI_ALIGN_START;
	pLayout->iAlignY = XUI_ALIGN_START;
	pLayout->iTableRowSpan = 1;
	pLayout->iTableColumnSpan = 1;
	pLayout->iGridColumnCount = 1;
	pLayout->fMaxWidth = XUI_LAYOUT_UNBOUNDED;
	pLayout->fMaxHeight = XUI_LAYOUT_UNBOUNDED;
	pLayout->fShrink = 1.0f;
}

static void __xuiDatePickerDefaultCachePolicy(xui_cache_policy_t* pPolicy)
{
	memset(pPolicy, 0, sizeof(*pPolicy));
	pPolicy->iSize = sizeof(*pPolicy);
	pPolicy->iPolicy = XUI_CACHE_POLICY_SELF;
	pPolicy->iFlags = XUI_CACHE_CLEAR_ON_UPDATE;
	pPolicy->iClearColor = XUI_COLOR_RGBA(0, 0, 0, 0);
}

static int __xuiDatePickerContentMeasure(xui_widget pWidget, xui_vec2_t tConstraint, xui_vec2_t* pSize, void* pUser)
{
	xui_date_picker_data_t* pData;
	xui_date_picker_data_t tResolved;
	xui_proxy pProxy;
	xui_font_metrics_t tMetrics;

	if ( (pWidget == NULL) || (pSize == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pData = (xui_date_picker_data_t*)pUser;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerResolve(pWidget, pData, &tResolved);
	pSize->fX = 180.0f;
	pSize->fY = 30.0f;
	pProxy = xuiInternalContextGetProxy(xuiWidgetGetContext(pWidget));
	if ( (pProxy != NULL) && (pProxy->fontGetMetrics != NULL) && (tResolved.pFont != NULL) &&
	     (pProxy->fontGetMetrics(pProxy, tResolved.pFont, &tMetrics) == XUI_OK) &&
	     (tMetrics.fLineHeight > 0.0f) ) {
		pSize->fY = __xuiDatePickerMax(30.0f, tMetrics.fLineHeight + 12.0f);
	}
	pSize->fX = xuiInternalSnapSize(pSize->fX);
	pSize->fY = xuiInternalSnapSize(pSize->fY);
	if ( (tConstraint.fX >= 0.0f) && (tConstraint.fX < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fX > tConstraint.fX) ) pSize->fX = tConstraint.fX;
	if ( (tConstraint.fY >= 0.0f) && (tConstraint.fY < XUI_LAYOUT_UNBOUNDED) &&
	     (pSize->fY > tConstraint.fY) ) pSize->fY = tConstraint.fY;
	return XUI_OK;
}

static int __xuiDatePickerInitEvents(xui_widget pWidget)
{
	int iRet;

	iRet = xuiWidgetSetDefaultAction(pWidget, NULL, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_ENTER, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_LEAVE, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_MOVE, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_DOWN, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_POINTER_CLICK, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_KEY_DOWN, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_FOCUS, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BLUR, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_BOUNDS_CHANGED, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_ENABLED_CHANGED, __xuiDatePickerEvent, NULL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pWidget, XUI_EVENT_VISIBLE_CHANGED, __xuiDatePickerEvent, NULL);
	return iRet;
}

static int __xuiDatePickerInitPanelEvents(xui_widget pPanel, xui_widget pOwner)
{
	int iRet;

	iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_DOWN, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_MOVE, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_UP, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_CAPTURE_LOST, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_POINTER_WHEEL, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_KEY_DOWN, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_TEXT, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_FOCUS, __xuiDatePickerPanelEvent, pOwner);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetEventHandler(pPanel, XUI_EVENT_BLUR, __xuiDatePickerPanelEvent, pOwner);
	return iRet;
}

static int __xuiDatePickerCreatePopup(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_popup_desc_t tDesc;
	xui_cache_policy_t tPolicy;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pOwner = pWidget;
	tDesc.iAnchor = XUI_POPUP_ANCHOR_BOTTOM_LEFT;
	tDesc.iDirection = XUI_POPUP_DIRECTION_RIGHT_DOWN;
	tDesc.iOutsidePolicy = XUI_POPUP_OUTSIDE_CLOSE;
	tDesc.iOwnerPolicy = XUI_POPUP_OWNER_PASSTHROUGH;
	tDesc.iEscapePolicy = XUI_POPUP_ESCAPE_CLOSE;
	tDesc.iFocusPolicy = XUI_POPUP_FOCUS_CUSTOM;
	tDesc.fContentWidth = pData->fPopupWidth;
	tDesc.fContentHeight = pData->fPopupHeight;
	tDesc.fGap = 2.0f;
	tDesc.fPadding = 0.0f;
	tDesc.fBorderWidth = 1.0f;
	tDesc.fShadowSize = 6.0f;
	tDesc.iPanelColor = pData->iPopupPanelColor;
	tDesc.iBorderColor = pData->iPopupBorderColor;
	tDesc.iShadowColor = pData->iPopupShadowColor;
	tDesc.iBackdropColor = XUI_COLOR_RGBA(0, 0, 0, 0);
	iRet = xuiPopupCreate(xuiWidgetGetContext(pWidget), &pData->pPopup, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	pData->pPanel = xuiPopupGetContentWidget(pData->pPopup);
	if ( pData->pPanel == NULL ) {
		iRet = XUI_ERROR_NOT_INITIALIZED;
		goto fail;
	}
	__xuiDatePickerDefaultCachePolicy(&tPolicy);
	iRet = xuiWidgetSetFocusable(pData->pPanel, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pData->pPanel, 0);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetImeMode(pData->pPanel, XUI_IME_DISABLED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetLayoutType(pData->pPanel, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pData->pPanel, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pData->pPanel, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCachePolicy(pData->pPanel, &tPolicy);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetCacheRenderCallback(pData->pPanel, __xuiDatePickerPanelRender, pWidget);
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInitPanelEvents(pData->pPanel, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetChange(pData->pPopup, __xuiDatePickerPopupChanged, pWidget);
	if ( iRet == XUI_OK ) iRet = xuiPopupSetFocusPolicy(pData->pPopup, XUI_POPUP_FOCUS_CUSTOM, pData->pPanel);
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerApplyPopupStyle(pWidget, pData);
	if ( iRet == XUI_OK ) return XUI_OK;

fail:
	xuiWidgetDestroy(pData->pPopup);
	pData->pPopup = NULL;
	pData->pPanel = NULL;
	return iRet;
}

static int __xuiDatePickerApplyPopupStyle(xui_widget pWidget, xui_date_picker_data_t* pData)
{
	xui_date_picker_data_t tResolved;
	xui_context pContext;
	xui_widget pPopup;
	xui_widget pPanel;
	int iRet;

	if ( (pWidget == NULL) || (pData == NULL) || (pData->pPopup == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pContext = xuiWidgetGetContext(pWidget);
	pPopup = pData->pPopup;
	pPanel = pData->pPanel;
	__xuiDatePickerResolve(pWidget, pData, &tResolved);
	iRet = xuiPopupSetOwner(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusRestore(pPopup, pWidget);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetContentSize(pPopup, pData->fPopupWidth, pData->fPopupHeight);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetColors(pPopup, tResolved.iPopupPanelColor, tResolved.iPopupBorderColor, tResolved.iPopupShadowColor, XUI_COLOR_RGBA(0, 0, 0, 0));
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetMetrics(pPopup, 0.0f, 1.0f, 6.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetClosePolicy(pPopup, XUI_POPUP_OUTSIDE_CLOSE, XUI_POPUP_OWNER_PASSTHROUGH, XUI_POPUP_ESCAPE_CLOSE);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetFocusPolicy(pPopup, XUI_POPUP_FOCUS_CUSTOM, pPanel);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupSetGap(pPopup, 2.0f);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	iRet = xuiPopupClearAnchorRect(pPopup);
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pData->iPopupPlacement == XUI_DATE_PICKER_POPUP_TOP ) {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_TOP_LEFT);
		if ( iRet == XUI_OK && __xuiDatePickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_UP);
	} else {
		iRet = xuiPopupSetAnchor(pPopup, XUI_POPUP_ANCHOR_BOTTOM_LEFT);
		if ( iRet == XUI_OK && __xuiDatePickerAlive(pContext, pWidget, pData) )
			iRet = xuiPopupSetDirection(pPopup, XUI_POPUP_DIRECTION_RIGHT_DOWN);
	}
	if ( iRet != XUI_OK ) return iRet;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( pPanel != NULL ) {
		iRet = xuiWidgetSetRect(pPanel,
			xuiInternalRectFromFloatNearest(0.0f, 0.0f, pData->fPopupWidth, pData->fPopupHeight));
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
		if ( iRet == XUI_OK ) iRet = xuiWidgetInvalidate(pPanel,
			XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static int __xuiDatePickerInit(xui_widget pWidget, void* pTypeData, const void* pCreateData, void* pUser)
{
	xui_date_picker_data_t* pData;
	const xui_date_picker_desc_t* pDesc;
	int iRet;

	(void)pUser;
	pData = (xui_date_picker_data_t*)pTypeData;
	pDesc = (const xui_date_picker_desc_t*)pCreateData;
	if ( (pWidget == NULL) || (pData == NULL) || !__xuiDatePickerDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerDefaults(pData);
	__xuiDatePickerApplyDesc(pData, pDesc);
	iRet = xuiWidgetSetLayoutType(pWidget, XUI_LAYOUT_MANUAL);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFlowMode(pWidget, XUI_FLOW_ABSOLUTE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetOverflow(pWidget, XUI_OVERFLOW_VISIBLE);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetFocusable(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetTabStop(pWidget, 1);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetImeMode(pWidget, XUI_IME_DISABLED);
	if ( iRet == XUI_OK ) iRet = xuiWidgetSetPadding(pWidget, (xui_thickness_t){0.0f, 0.0f, 0.0f, 0.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInitEvents(pWidget);
	if ( iRet != XUI_OK ) return iRet;
	iRet = __xuiDatePickerCreatePopup(pWidget, pData);
	if ( iRet != XUI_OK ) return iRet;
	return __xuiDatePickerSyncState(pWidget, pData);
}

static void __xuiDatePickerDestroy(xui_widget pWidget, void* pTypeData, void* pUser)
{
	xui_date_picker_data_t* pData;

	(void)pWidget;
	(void)pUser;
	pData = (xui_date_picker_data_t*)pTypeData;
	if ( pData != NULL ) {
		if ( pData->pPanel != NULL ) {
			(void)__xuiDatePickerPanelCancelPointer(pData->pPanel, pData);
		}
		if ( pData->pPopup != NULL ) xuiWidgetDestroy(pData->pPopup);
		memset(pData, 0, sizeof(*pData));
	}
}

static void __xuiDatePickerRegisterStyleProperty(xui_context pContext, xui_widget_type pType, const char* sName, int iValueType, uint32_t iDirtyFlags, uint32_t iFlags)
{
	xui_style_property_info_t tInfo;

	if ( xuiStyleFindProperty(pContext, sName) != 0 ) return;
	memset(&tInfo, 0, sizeof(tInfo));
	tInfo.iSize = sizeof(tInfo);
	tInfo.sName = sName;
	tInfo.iValueType = iValueType;
	tInfo.iDirtyFlags = iDirtyFlags;
	tInfo.iFlags = iFlags;
	tInfo.pWidgetType = pType;
	(void)xuiStyleRegisterProperty(pContext, &tInfo, NULL);
}

static void __xuiDatePickerRegisterStyleProperties(xui_context pContext, xui_widget_type pType)
{
	uint32_t iPaintDirty;
	uint32_t iLayoutDirty;

	iPaintDirty = XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	iLayoutDirty = XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER;
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.text.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.text.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.background.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.background.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.background.open_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.background.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.border.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.border.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.border.focus_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.arrow.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.arrow.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.popup.panel_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.popup.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.popup.shadow_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.popup.text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.popup.muted_text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.accent.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.field.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.field.border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.day.selected_text_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.day.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.separator.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.button.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.button.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.button.open_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.button.disabled_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.calendar.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.time.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.day.range_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.selection.color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.field.error_border_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.option.hover_color", XUI_STYLE_VALUE_COLOR, iPaintDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "datepicker.border.width", XUI_STYLE_VALUE_FLOAT, iLayoutDirty, 0);
	__xuiDatePickerRegisterStyleProperty(pContext, pType, "font.name", XUI_STYLE_VALUE_STRING, iLayoutDirty, XUI_STYLE_PROPERTY_INHERITED);
}

static xui_date_picker_data_t* __xuiDatePickerGetData(xui_widget pWidget)
{
	xui_context pContext;
	xui_widget_type pType;

	if ( !xuiInternalWidgetIsValid(pWidget) ) return NULL;
	pContext = xuiWidgetGetContext(pWidget);
	pType = xuiWidgetFindType(pContext, "datepicker");
	if ( (pType == NULL) || !xuiWidgetIsType(pWidget, pType) ) return NULL;
	return (xui_date_picker_data_t*)xuiWidgetGetTypeData(pWidget);
}

static int __xuiDatePickerAccessibleNode(xui_widget pWidget, xui_accessible_node_t* pNode)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);

	if ( (pData == NULL) || (pNode == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	pNode->iRole = XUI_ACCESSIBLE_ROLE_COMBOBOX;
	pNode->iState |= XUI_ACCESSIBLE_STATE_READONLY;
	pNode->iState |= xuiDatePickerIsOpen(pWidget) ?
		XUI_ACCESSIBLE_STATE_EXPANDED : XUI_ACCESSIBLE_STATE_COLLAPSED;
	pNode->iActions |= XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_EXPAND) |
		XUI_ACCESSIBLE_ACTION_MASK(XUI_ACCESSIBLE_ACTION_COLLAPSE);
	pNode->sName = "Date picker";
	pNode->sValue = pData->sText;
	return XUI_OK;
}

static int __xuiDatePickerAccessibleAction(xui_widget pWidget, int iAction, const void* pPayload)
{
	(void)pPayload;
	if ( __xuiDatePickerGetData(pWidget) == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( iAction == XUI_ACCESSIBLE_ACTION_EXPAND ) return xuiDatePickerOpen(pWidget);
	if ( iAction == XUI_ACCESSIBLE_ACTION_COLLAPSE ) return xuiDatePickerClose(pWidget);
	return XUI_ERROR_UNSUPPORTED;
}

static const xui_internal_accessibility_adapter_t g_xuiDatePickerAccessible = {
	.getNode = __xuiDatePickerAccessibleNode,
	.performAction = __xuiDatePickerAccessibleAction
};

XUI_API xui_widget_type xuiDatePickerGetType(xui_context pContext)
{
	xui_widget_type_desc_t tDesc;
	xui_widget_type pType;
	int iRet;

	if ( !xuiInternalContextIsValid(pContext) ) return NULL;
	pType = xuiWidgetFindType(pContext, "datepicker");
	if ( pType != NULL ) {
		__xuiDatePickerRegisterStyleProperties(pContext, pType);
		pType->pAccessibleAdapter = &g_xuiDatePickerAccessible;
		return pType;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.sName = "datepicker";
	tDesc.pParent = xuiWidgetGetBaseType();
	tDesc.iFlags = XUI_WIDGET_TYPE_DEFAULT_LAYOUT | XUI_WIDGET_TYPE_DEFAULT_CACHE_POLICY;
	tDesc.iTypeDataSize = sizeof(xui_date_picker_data_t);
	tDesc.onInit = __xuiDatePickerInit;
	tDesc.onDestroy = __xuiDatePickerDestroy;
	tDesc.onContentMeasure = __xuiDatePickerContentMeasure;
	tDesc.onCacheRender = __xuiDatePickerCacheRender;
	__xuiDatePickerDefaultLayout(&tDesc.tLayout);
	__xuiDatePickerDefaultCachePolicy(&tDesc.tCachePolicy);
	iRet = xuiWidgetRegisterType(pContext, &pType, &tDesc);
	if ( iRet != XUI_OK ) return NULL;
	__xuiDatePickerRegisterStyleProperties(pContext, pType);
	pType->onPreparePaint = __xuiDatePickerSyncStyle;
	pType->pAccessibleAdapter = &g_xuiDatePickerAccessible;
	return pType;
}

XUI_API int xuiDatePickerCreate(xui_context pContext, xui_widget* ppWidget, const xui_date_picker_desc_t* pDesc)
{
	xui_widget_type pType;

	if ( ppWidget == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	*ppWidget = NULL;
	if ( !__xuiDatePickerDescValid(pDesc) ) return XUI_ERROR_INVALID_ARGUMENT;
	pType = xuiDatePickerGetType(pContext);
	if ( pType == NULL ) return XUI_ERROR_NOT_INITIALIZED;
	return xuiWidgetCreateTyped(pContext, pType, ppWidget, pDesc);
}

XUI_API int xuiDatePickerSetChanging(xui_widget pWidget, xui_date_picker_proc onChanging, void* pUser)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChanging = onChanging;
	pData->pChangingUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetChange(xui_widget pWidget, xui_date_picker_proc onChange, void* pUser)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onChange = onChange;
	pData->pChangeUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetCommit(xui_widget pWidget, xui_date_picker_proc onCommit, void* pUser)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onCommit = onCommit;
	pData->pCommitUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetCancel(xui_widget pWidget, xui_date_picker_proc onCancel, void* pUser)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onCancel = onCancel;
	pData->pCancelUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetClear(xui_widget pWidget, xui_date_picker_proc onClear, void* pUser)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	pData->onClear = onClear;
	pData->pClearUser = pUser;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetMode(xui_widget pWidget, int iMode)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_date_picker_data_t tOld;
	xui_context pContext;
	int iRet;

	if ( (pData == NULL) || !__xuiDatePickerModeValid(iMode) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iMode == iMode ) return XUI_OK;
	if ( !__xuiDatePickerLimitsValid(iMode, pData->bHasMin, pData->tMin, pData->bHasMax, pData->tMax) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( pData->bPopupSizeCustom && !__xuiDatePickerPopupSizeValid(iMode, pData->bNullable, pData->fPopupWidth, pData->fPopupHeight) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	if ( pData->pPanel != NULL ) {
		iRet = __xuiDatePickerPanelCancelPointer(pData->pPanel, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	tOld = *pData;
	pData->iMode = iMode;
	__xuiDatePickerSyncDefaultFormat(pData);
	__xuiDatePickerUpdatePopupSizeForMode(pData);
	__xuiDatePickerEnsureValue(pData);
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerInitViewMonth(pData);
	__xuiDatePickerSyncText(pData);
	iRet = (pData->pPopup != NULL) ? __xuiDatePickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) {
		*pData = tOld;
		if ( pData->pPopup != NULL ) (void)__xuiDatePickerApplyPopupStyle(pWidget, pData);
	} else {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
		if ( strcmp(tOld.sText, pData->sText) != 0 )
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiDatePickerGetMode(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iMode : XUI_DATE_PICKER_MODE_DATE;
}

XUI_API int xuiDatePickerSetNullable(xui_widget pWidget, int bNullable)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_date_picker_data_t tOld;
	xui_context pContext;
	float fMinWidth;
	float fMinHeight;
	int iNewNullable;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNewNullable = bNullable ? 1 : 0;
	if ( pData->bNullable == iNewNullable ) return XUI_OK;
	if ( pData->bPopupSizeCustom &&
	     !__xuiDatePickerPopupSizeValid(pData->iMode, iNewNullable, pData->fPopupWidth, pData->fPopupHeight) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	pContext = xuiWidgetGetContext(pWidget);
	tOld = *pData;
	pData->bNullable = iNewNullable;
	if ( !pData->bPopupSizeCustom ) {
		__xuiDatePickerPreferredPopupSize(pData->iMode, &pData->fPopupWidth, &pData->fPopupHeight);
		__xuiDatePickerMinimumPopupSize(pData->iMode, pData->bNullable, &fMinWidth, &fMinHeight);
		if ( pData->fPopupWidth < fMinWidth ) pData->fPopupWidth = fMinWidth;
		if ( pData->fPopupHeight < fMinHeight ) pData->fPopupHeight = fMinHeight;
	}
	__xuiDatePickerEnsureValue(pData);
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerSyncText(pData);
	iRet = (pData->pPopup != NULL) ? __xuiDatePickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) {
		*pData = tOld;
		if ( pData->pPopup != NULL ) (void)__xuiDatePickerApplyPopupStyle(pWidget, pData);
	} else {
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_NODE_CHANGED);
		if ( strcmp(tOld.sText, pData->sText) != 0 )
			xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	}
	return iRet;
}

XUI_API int xuiDatePickerGetNullable(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->bNullable : 0;
}

XUI_API int xuiDatePickerSetValue(xui_widget pWidget, xtime tValue)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tValue = __xuiDatePickerClampValue(pData, tValue);
	if ( pData->bHasValue && pData->tValue == tValue && pData->tStart == tValue && pData->tEnd == tValue ) {
		return XUI_OK;
	}
	pData->tValue = tValue;
	pData->tStart = pData->tValue;
	pData->tEnd = pData->tValue;
	pData->bHasValue = 1;
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerInitViewMonth(pData);
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API xtime xuiDatePickerGetValue(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return 0;
	__xuiDatePickerEnsureValue(pData);
	return pData->tValue;
}

XUI_API int xuiDatePickerHasValue(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->bHasValue : 0;
}

XUI_API int xuiDatePickerClearValue(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	int bOldHasValue;
	xtime tOldValue;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bOldHasValue = pData->bHasValue;
	tOldValue = pData->tValue;
	if ( !pData->bNullable ) {
		__xuiDatePickerEnsureValue(pData);
	} else {
		pData->bHasValue = 0;
	}
	if ( pData->bHasValue == bOldHasValue && pData->tValue == tOldValue ) return XUI_OK;
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API int xuiDatePickerSetRangeValue(xui_widget pWidget, xtime tStart, xtime tEnd)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerNormalizeRange(pData, &tStart, &tEnd);
	if ( pData->bHasValue && pData->tStart == tStart && pData->tEnd == tEnd && pData->tValue == tStart ) {
		return XUI_OK;
	}
	pData->tStart = tStart;
	pData->tEnd = tEnd;
	pData->tValue = tStart;
	pData->bHasValue = 1;
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerInitViewMonth(pData);
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK ) xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API int xuiDatePickerGetRangeValue(xui_widget pWidget, xtime* pStart, xtime* pEnd)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerEnsureValue(pData);
	if ( pStart != NULL ) *pStart = pData->tStart;
	if ( pEnd != NULL ) *pEnd = pData->tEnd;
	return XUI_OK;
}

XUI_API int xuiDatePickerHasRangeValue(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? (pData->bHasValue && __xuiDatePickerIsRange(pData->iMode)) : 0;
}

XUI_API int xuiDatePickerSetLimits(xui_widget pWidget, xtime tMin, xtime tMax)
{
	return xuiDatePickerSetLimitRange(pWidget, 1, tMin, 1, tMax);
}

XUI_API int xuiDatePickerSetLimitRange(xui_widget pWidget, int bHasMin, xtime tMin, int bHasMax, xtime tMax)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xtime tOldValue;
	xtime tOldStart;
	xtime tOldEnd;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !__xuiDatePickerLimitsValid(pData->iMode, bHasMin, tMin, bHasMax, tMax) ) return XUI_ERROR_INVALID_ARGUMENT;
	bHasMin = bHasMin ? 1 : 0;
	bHasMax = bHasMax ? 1 : 0;
	if ( pData->bHasMin == bHasMin && pData->bHasMax == bHasMax &&
	     (!bHasMin || pData->tMin == tMin) && (!bHasMax || pData->tMax == tMax) ) {
		return XUI_OK;
	}
	tOldValue = pData->tValue;
	tOldStart = pData->tStart;
	tOldEnd = pData->tEnd;
	pData->tMin = tMin;
	pData->tMax = tMax;
	pData->bHasMin = bHasMin;
	pData->bHasMax = bHasMax;
	__xuiDatePickerEnsureValue(pData);
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK && (tOldValue != pData->tValue || tOldStart != pData->tStart || tOldEnd != pData->tEnd) )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API int xuiDatePickerGetLimitRange(xui_widget pWidget, int* pHasMin, xtime* pMin, int* pHasMax, xtime* pMax)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pHasMin != NULL ) *pHasMin = pData->bHasMin;
	if ( pMin != NULL ) *pMin = pData->tMin;
	if ( pHasMax != NULL ) *pHasMax = pData->bHasMax;
	if ( pMax != NULL ) *pMax = pData->tMax;
	return XUI_OK;
}

XUI_API int xuiDatePickerClearLimits(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( !pData->bHasMin && !pData->bHasMax ) return XUI_OK;
	pData->bHasMin = 0;
	pData->bHasMax = 0;
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiDatePickerSetFormat(xui_widget pWidget, const char* sFormat)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	char sOldText[sizeof(pData->sText)];
	char sNewFormat[sizeof(pData->sFormat)];
	int bNewCustom;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	bNewCustom = (sFormat != NULL && sFormat[0] != '\0') ? 1 : 0;
	__xuiDatePickerCopyText(sNewFormat, (int)sizeof(sNewFormat), bNewCustom ? sFormat :
		__xuiDatePickerDefaultFormat(pData->iMode, pData->bShowSecond));
	if ( strcmp(pData->sFormat, sNewFormat) == 0 ) {
		pData->bFormatCustom = bNewCustom;
		return XUI_OK;
	}
	memcpy(sOldText, pData->sText, sizeof(sOldText));
	pData->bFormatCustom = bNewCustom;
	memcpy(pData->sFormat, sNewFormat, sizeof(pData->sFormat));
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK && strcmp(sOldText, pData->sText) != 0 )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API const char* xuiDatePickerGetFormat(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->sFormat : "";
}

XUI_API int xuiDatePickerSetRangeSeparator(xui_widget pWidget, const char* sSeparator)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	char sOldText[sizeof(pData->sText)];
	char sNewSeparator[sizeof(pData->sRangeSeparator)];
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	__xuiDatePickerCopyText(sNewSeparator, (int)sizeof(sNewSeparator),
		(sSeparator != NULL) ? sSeparator : " - ");
	if ( strcmp(pData->sRangeSeparator, sNewSeparator) == 0 ) return XUI_OK;
	memcpy(sOldText, pData->sText, sizeof(sOldText));
	memcpy(pData->sRangeSeparator, sNewSeparator, sizeof(pData->sRangeSeparator));
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK && strcmp(sOldText, pData->sText) != 0 )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API const char* xuiDatePickerGetRangeSeparator(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->sRangeSeparator : "";
}

XUI_API int xuiDatePickerSetShowSecond(xui_widget pWidget, int bShowSecond)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	char sOldText[sizeof(pData->sText)];
	int iNewShowSecond;
	int iRet;
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	iNewShowSecond = bShowSecond ? 1 : 0;
	if ( pData->bShowSecond == iNewShowSecond ) return XUI_OK;
	memcpy(sOldText, pData->sText, sizeof(sOldText));
	pData->bShowSecond = iNewShowSecond;
	__xuiDatePickerSyncDefaultFormat(pData);
	__xuiDatePickerEnsureValue(pData);
	if ( pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup) ) __xuiDatePickerPrepareDraft(pData);
	__xuiDatePickerSyncText(pData);
	iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet == XUI_OK && strcmp(sOldText, pData->sText) != 0 )
		xuiInternalAccessibilityQueue(pWidget, XUI_ACCESSIBLE_EVENT_VALUE_CHANGED);
	return iRet;
}

XUI_API int xuiDatePickerGetShowSecond(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->bShowSecond : 0;
}

XUI_API int xuiDatePickerSetFirstDayOfWeek(xui_widget pWidget, int iFirstDayOfWeek)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iFirstDayOfWeek < 0) || (iFirstDayOfWeek > 6) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iFirstDayOfWeek == iFirstDayOfWeek ) return XUI_OK;
	pData->iFirstDayOfWeek = iFirstDayOfWeek;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiDatePickerGetFirstDayOfWeek(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iFirstDayOfWeek : 0;
}

XUI_API int xuiDatePickerSetDefaultRangeSpan(xui_widget pWidget, xtime tSpan)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (tSpan < 0) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->tDefaultRangeSpan == tSpan ) return XUI_OK;
	pData->tDefaultRangeSpan = tSpan;
	return XUI_OK;
}

XUI_API xtime xuiDatePickerGetDefaultRangeSpan(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->tDefaultRangeSpan : 0;
}

XUI_API int xuiDatePickerOpen(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? __xuiDatePickerOpenInternal(pWidget, pData) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiDatePickerClose(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? __xuiDatePickerCloseWithReason(pWidget, pData, XUI_DATE_PICKER_CLOSE_SILENT) : XUI_ERROR_INVALID_ARGUMENT;
}

XUI_API int xuiDatePickerToggle(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	return (pData->pPopup != NULL && xuiPopupIsOpen(pData->pPopup)) ? xuiDatePickerClose(pWidget) : xuiDatePickerOpen(pWidget);
}

XUI_API int xuiDatePickerIsOpen(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL && pData->pPopup != NULL) ? xuiPopupIsOpen(pData->pPopup) : 0;
}

XUI_API int xuiDatePickerSetPopupSize(xui_widget pWidget, float fWidth, float fHeight)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_context pContext;
	float fOldWidth;
	float fOldHeight;
	float fNewWidth;
	float fNewHeight;
	int bOldCustom;
	int iRet;

	if ( pData == NULL || !__xuiDatePickerMetricValid(fWidth) || !__xuiDatePickerMetricValid(fHeight) ) {
		return XUI_ERROR_INVALID_ARGUMENT;
	}
	if ( fWidth == 0.0f && fHeight == 0.0f ) return XUI_OK;
	fNewWidth = (fWidth > 0.0f) ? fWidth : pData->fPopupWidth;
	fNewHeight = (fHeight > 0.0f) ? fHeight : pData->fPopupHeight;
	if ( !__xuiDatePickerPopupSizeValid(pData->iMode, pData->bNullable, fNewWidth, fNewHeight) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fPopupWidth == fNewWidth && pData->fPopupHeight == fNewHeight ) {
		pData->bPopupSizeCustom = 1;
		return XUI_OK;
	}
	pContext = xuiWidgetGetContext(pWidget);
	fOldWidth = pData->fPopupWidth;
	fOldHeight = pData->fPopupHeight;
	bOldCustom = pData->bPopupSizeCustom;
	if ( pData->pPanel != NULL ) {
		iRet = __xuiDatePickerPanelCancelPointer(pData->pPanel, pData);
		if ( iRet != XUI_OK ) return iRet;
		if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	}
	pData->fPopupWidth = fNewWidth;
	pData->fPopupHeight = fNewHeight;
	pData->bPopupSizeCustom = 1;
	iRet = (pData->pPopup != NULL) ? __xuiDatePickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) {
		pData->fPopupWidth = fOldWidth;
		pData->fPopupHeight = fOldHeight;
		pData->bPopupSizeCustom = bOldCustom;
		if ( pData->pPopup != NULL ) (void)__xuiDatePickerApplyPopupStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiDatePickerGetPopupSize(xui_widget pWidget, float* pWidth, float* pHeight)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pWidth != NULL ) *pWidth = pData->fPopupWidth;
	if ( pHeight != NULL ) *pHeight = pData->fPopupHeight;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetPopupPlacement(xui_widget pWidget, int iPlacement)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_context pContext;
	int iOldPlacement;
	int iRet;

	if ( (pData == NULL) || !__xuiDatePickerPlacementValid(iPlacement) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPopupPlacement == iPlacement ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	iOldPlacement = pData->iPopupPlacement;
	pData->iPopupPlacement = iPlacement;
	iRet = (pData->pPopup != NULL) ? __xuiDatePickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet != XUI_OK ) {
		pData->iPopupPlacement = iOldPlacement;
		if ( pData->pPopup != NULL ) (void)__xuiDatePickerApplyPopupStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiDatePickerGetPopupPlacement(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iPopupPlacement : XUI_DATE_PICKER_POPUP_AUTO;
}

XUI_API int xuiDatePickerSetMetrics(xui_widget pWidget, float fBorderWidth)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || !__xuiDatePickerMetricValid(fBorderWidth) ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->fBorderWidth == fBorderWidth ) return XUI_OK;
	pData->fBorderWidth = fBorderWidth;
	return xuiWidgetInvalidate(pWidget, XUI_WIDGET_DIRTY_LAYOUT | XUI_WIDGET_DIRTY_CACHE | XUI_WIDGET_DIRTY_RENDER);
}

XUI_API int xuiDatePickerGetMetrics(xui_widget pWidget, float* pBorderWidth)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorderWidth != NULL ) *pBorderWidth = pData->fBorderWidth;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetColors(xui_widget pWidget, uint32_t iText, uint32_t iDisabledText, uint32_t iBackground, uint32_t iHoverBackground, uint32_t iOpenBackground, uint32_t iDisabledBackground)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iTextColor == iText && pData->iDisabledTextColor == iDisabledText &&
	     pData->iBackgroundColor == iBackground && pData->iHoverBackgroundColor == iHoverBackground &&
	     pData->iOpenBackgroundColor == iOpenBackground && pData->iDisabledBackgroundColor == iDisabledBackground ) {
		return XUI_OK;
	}
	pData->iTextColor = iText;
	pData->iDisabledTextColor = iDisabledText;
	pData->iBackgroundColor = iBackground;
	pData->iHoverBackgroundColor = iHoverBackground;
	pData->iOpenBackgroundColor = iOpenBackground;
	pData->iDisabledBackgroundColor = iDisabledBackground;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiDatePickerGetColors(xui_widget pWidget, uint32_t* pText, uint32_t* pDisabledText, uint32_t* pBackground, uint32_t* pHoverBackground, uint32_t* pOpenBackground, uint32_t* pDisabledBackground)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pText != NULL ) *pText = pData->iTextColor;
	if ( pDisabledText != NULL ) *pDisabledText = pData->iDisabledTextColor;
	if ( pBackground != NULL ) *pBackground = pData->iBackgroundColor;
	if ( pHoverBackground != NULL ) *pHoverBackground = pData->iHoverBackgroundColor;
	if ( pOpenBackground != NULL ) *pOpenBackground = pData->iOpenBackgroundColor;
	if ( pDisabledBackground != NULL ) *pDisabledBackground = pData->iDisabledBackgroundColor;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetBorderColors(xui_widget pWidget, uint32_t iBorder, uint32_t iHoverBorder, uint32_t iFocusBorder)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iBorderColor == iBorder && pData->iHoverBorderColor == iHoverBorder &&
	     pData->iFocusBorderColor == iFocusBorder ) return XUI_OK;
	pData->iBorderColor = iBorder;
	pData->iHoverBorderColor = iHoverBorder;
	pData->iFocusBorderColor = iFocusBorder;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiDatePickerGetBorderColors(xui_widget pWidget, uint32_t* pBorder, uint32_t* pHoverBorder, uint32_t* pFocusBorder)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pBorder != NULL ) *pBorder = pData->iBorderColor;
	if ( pHoverBorder != NULL ) *pHoverBorder = pData->iHoverBorderColor;
	if ( pFocusBorder != NULL ) *pFocusBorder = pData->iFocusBorderColor;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetArrowColors(xui_widget pWidget, uint32_t iArrow, uint32_t iDisabledArrow)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iArrowColor == iArrow && pData->iDisabledArrowColor == iDisabledArrow ) return XUI_OK;
	pData->iArrowColor = iArrow;
	pData->iDisabledArrowColor = iDisabledArrow;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API int xuiDatePickerGetArrowColors(xui_widget pWidget, uint32_t* pArrow, uint32_t* pDisabledArrow)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pArrow != NULL ) *pArrow = pData->iArrowColor;
	if ( pDisabledArrow != NULL ) *pDisabledArrow = pData->iDisabledArrowColor;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetPopupColors(xui_widget pWidget, uint32_t iPanel, uint32_t iBorder, uint32_t iShadow, uint32_t iText, uint32_t iMutedText, uint32_t iAccent, uint32_t iField, uint32_t iFieldBorder, uint32_t iSelectedText, uint32_t iDisabledDay, uint32_t iSeparator)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	xui_date_picker_data_t tOld;
	xui_context pContext;
	int iRet;

	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->iPopupPanelColor == iPanel && pData->iPopupBorderColor == iBorder &&
	     pData->iPopupShadowColor == iShadow && pData->iPopupTextColor == iText &&
	     pData->iPopupMutedTextColor == iMutedText && pData->iAccentColor == iAccent &&
	     pData->iFieldColor == iField && pData->iFieldBorderColor == iFieldBorder &&
	     pData->iSelectedTextColor == iSelectedText && pData->iDisabledDayColor == iDisabledDay &&
	     pData->iSeparatorColor == iSeparator ) return XUI_OK;
	pContext = xuiWidgetGetContext(pWidget);
	tOld = *pData;
	pData->iPopupPanelColor = iPanel;
	pData->iPopupBorderColor = iBorder;
	pData->iPopupShadowColor = iShadow;
	pData->iPopupTextColor = iText;
	pData->iPopupMutedTextColor = iMutedText;
	pData->iAccentColor = iAccent;
	pData->iFieldColor = iField;
	pData->iFieldBorderColor = iFieldBorder;
	pData->iSelectedTextColor = iSelectedText;
	pData->iDisabledDayColor = iDisabledDay;
	pData->iSeparatorColor = iSeparator;
	iRet = (pData->pPopup != NULL) ? __xuiDatePickerApplyPopupStyle(pWidget, pData) : XUI_OK;
	if ( !__xuiDatePickerAlive(pContext, pWidget, pData) ) return XUI_OK;
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerInvalidateAll(pWidget, pData);
	if ( iRet != XUI_OK ) {
		*pData = tOld;
		if ( pData->pPopup != NULL ) (void)__xuiDatePickerApplyPopupStyle(pWidget, pData);
	}
	return iRet;
}

XUI_API int xuiDatePickerGetPopupColors(xui_widget pWidget, uint32_t* pPanel, uint32_t* pBorder, uint32_t* pShadow, uint32_t* pText, uint32_t* pMutedText, uint32_t* pAccent, uint32_t* pField, uint32_t* pFieldBorder, uint32_t* pSelectedText, uint32_t* pDisabledDay, uint32_t* pSeparator)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pPanel != NULL ) *pPanel = pData->iPopupPanelColor;
	if ( pBorder != NULL ) *pBorder = pData->iPopupBorderColor;
	if ( pShadow != NULL ) *pShadow = pData->iPopupShadowColor;
	if ( pText != NULL ) *pText = pData->iPopupTextColor;
	if ( pMutedText != NULL ) *pMutedText = pData->iPopupMutedTextColor;
	if ( pAccent != NULL ) *pAccent = pData->iAccentColor;
	if ( pField != NULL ) *pField = pData->iFieldColor;
	if ( pFieldBorder != NULL ) *pFieldBorder = pData->iFieldBorderColor;
	if ( pSelectedText != NULL ) *pSelectedText = pData->iSelectedTextColor;
	if ( pDisabledDay != NULL ) *pDisabledDay = pData->iDisabledDayColor;
	if ( pSeparator != NULL ) *pSeparator = pData->iSeparatorColor;
	return XUI_OK;
}

XUI_API int xuiDatePickerSetFont(xui_widget pWidget, xui_font pFont)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	if ( pData->pFont == pFont ) return XUI_OK;
	pData->pFont = pFont;
	return __xuiDatePickerInvalidateAll(pWidget, pData);
}

XUI_API xui_font xuiDatePickerGetFont(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->pFont : NULL;
}

XUI_API const char* xuiDatePickerGetText(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->sText : "";
}

XUI_API xui_widget xuiDatePickerGetPopupWidget(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->pPopup : NULL;
}

XUI_API xui_widget xuiDatePickerGetPanelWidget(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->pPanel : NULL;
}

XUI_API xui_rect_t xuiDatePickerGetButtonRect(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0};
	__xuiDatePickerUpdateOwnerRects(pWidget, pData);
	return pData->tButtonRect;
}

XUI_API xui_rect_t xuiDatePickerGetTextRect(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( pData == NULL ) return (xui_rect_t){0};
	__xuiDatePickerUpdateOwnerRects(pWidget, pData);
	return pData->tTextRect;
}

XUI_API xui_rect_t xuiDatePickerGetCalendarPanelRect(xui_widget pWidget, int iPanel)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->tCalendarRect[iPanel];
}

XUI_API xui_rect_t xuiDatePickerGetTimePanelRect(xui_widget pWidget, int iPanel)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->tTimePanelRect[iPanel];
}

XUI_API xui_rect_t xuiDatePickerGetPrevRect(xui_widget pWidget, int iPanel)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->tPrevRect[iPanel];
}

XUI_API xui_rect_t xuiDatePickerGetNextRect(xui_widget pWidget, int iPanel)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->tNextRect[iPanel];
}

XUI_API xui_rect_t xuiDatePickerGetDayRect(xui_widget pWidget, int iPanel, int iIndex)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iIndex < 0) || (iIndex >= XUI_DATE_PICKER_CELL_COUNT) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->arrDayRect[iPanel][iIndex];
}

XUI_API xtime xuiDatePickerGetDayValue(xui_widget pWidget, int iPanel, int iIndex)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iIndex < 0) || (iIndex >= XUI_DATE_PICKER_CELL_COUNT) ) return 0;
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->arrDayValue[iPanel][iIndex];
}

XUI_API xui_rect_t xuiDatePickerGetTimeRect(xui_widget pWidget, int iPanel, int iField)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iPanel < 0) || (iPanel >= XUI_DATE_PICKER_PANEL_CAPACITY) || (iField < 0) || (iField > 2) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->arrTimeRect[iPanel][iField];
}

XUI_API xui_rect_t xuiDatePickerGetFooterRect(xui_widget pWidget, int iIndex)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	if ( (pData == NULL) || (iIndex < 0) || (iIndex >= 4) ) return (xui_rect_t){0};
	__xuiDatePickerUpdatePanelRects(pData);
	return pData->arrFooterRect[iIndex];
}

XUI_API int xuiDatePickerGetHoverPart(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iHoverPart : XUI_DATE_PICKER_PART_NONE;
}

XUI_API int xuiDatePickerGetActivePart(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iActivePart : XUI_DATE_PICKER_PART_NONE;
}

XUI_API uint32_t xuiDatePickerGetState(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return __xuiDatePickerState(pWidget, pData);
}

XUI_API int xuiDatePickerGetChangingCount(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iChangingCount : 0;
}

XUI_API int xuiDatePickerGetChangeCount(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iChangeCount : 0;
}

XUI_API int xuiDatePickerGetCommitCount(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iCommitCount : 0;
}

XUI_API int xuiDatePickerGetCancelCount(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iCancelCount : 0;
}

XUI_API int xuiDatePickerGetClearCount(xui_widget pWidget)
{
	xui_date_picker_data_t* pData = __xuiDatePickerGetData(pWidget);
	return (pData != NULL) ? pData->iClearCount : 0;
}

#endif
