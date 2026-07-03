#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATE_PICKER_COUNT 11
#define LABEL_COUNT 16

enum {
	DP_DATE = 0,
	DP_TIME,
	DP_TIME_SECONDS,
	DP_DATETIME,
	DP_DATE_RANGE,
	DP_TIME_RANGE,
	DP_TIME_RANGE_SECONDS,
	DP_DATETIME_RANGE,
	DP_NULLABLE,
	DP_LIMITED,
	DP_DISABLED
};

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_font_t tFont;
	xge_xui_panel_t tPanel;
	xge_xui_label_t tLabel[LABEL_COUNT];
	xge_xui_date_picker_t tPicker[DATE_PICKER_COUNT];
	xge_xui_widget pPanel;
	xge_xui_widget pLabel[LABEL_COUNT];
	xge_xui_widget pPickerWidget[DATE_PICKER_COUNT];
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int iChangeCount;
	int iChangingCount;
	int iCommitCount;
	int iCancelCount;
	int iClearCount;
	int bCreateOK;
	int bModeOK;
	int bValueOK;
	int bRangeOK;
	int bLimitOK;
	int bPopupOK;
	int bClearOK;
	int bYearEditOK;
} app_state_t;

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static int LoadFont(app_state_t* pApp)
{
	const char* arrFonts[] = {
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	int i;

	for ( i = 0; i < (int)(sizeof(arrFonts) / sizeof(arrFonts[0])); i++ ) {
		memset(&pApp->tFont, 0, sizeof(pApp->tFont));
		if ( xgeFontLoad(&pApp->tFont, arrFonts[i], XGE_XUI_DEMO_FONT_SIZE) == XGE_OK ) {
			pApp->bFontReady = 1;
			printf("xui_datepicker font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_datepicker font load failed\n");
	return XGE_ERROR_RESOURCE_FAILED;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void ClickMouse(app_state_t* pApp, float fX, float fY)
{
	xge_event_t tEvent;

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
}

static int DemoIsLeapYear(int64 iYear)
{
	return (iYear % 4 == 0 && iYear % 100 != 0) || (iYear % 400 == 0);
}

static int DemoDaysInMonth(int64 iYear, int iMonth)
{
	static const int arrDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	if ( iMonth == 2 && DemoIsLeapYear(iYear) ) {
		return 29;
	}
	if ( iMonth < 1 || iMonth > 12 ) {
		return 30;
	}
	return arrDays[iMonth - 1];
}

static xtime DemoTimeSerial(int iHour, int iMinute, int iSecond)
{
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + iSecond;
}

static xtime DemoDateSerial(int64 iYear, int iMonth, int iDay)
{
	xtime tDate;
	int64 i;
	int iM;

	tDate = (xtime)(iDay - 1) * XRT_TIME_DAY;
	for ( iM = 1; iM < iMonth; iM++ ) {
		tDate += (xtime)DemoDaysInMonth(iYear, iM) * XRT_TIME_DAY;
	}
	for ( i = 0; i < iYear; i++ ) {
		tDate += (xtime)(DemoIsLeapYear(i) ? 366 : 365) * XRT_TIME_DAY;
	}
	return tDate;
}

static xtime DemoDateTimeSerial(int64 iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
{
	return DemoDateSerial(iYear, iMonth, iDay) + DemoTimeSerial(iHour, iMinute, iSecond);
}

static void OnChanging(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangingCount++;
	}
}

static void OnChange(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iChangeCount++;
	}
}

static void OnCommit(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCommitCount++;
	}
}

static void OnCancel(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCancelCount++;
	}
}

static void OnClear(xge_xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iClearCount++;
	}
}

static int AddLabel(app_state_t* pApp, xge_xui_widget pParent, int iIndex, float fX, float fY, float fW, const char* sText)
{
	xge_font pFont;

	if ( iIndex < 0 || iIndex >= LABEL_COUNT ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	pApp->pLabel[iIndex] = xgeXuiWidgetCreate();
	if ( pApp->pLabel[iIndex] == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pLabel[iIndex], (xge_rect_t){ fX, fY, fW, 24.0f });
	if ( xgeXuiLabelInit(&pApp->tLabel[iIndex], pApp->pLabel[iIndex], pFont, sText) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiLabelSetColor(&pApp->tLabel[iIndex], XGE_COLOR_RGBA(68, 84, 102, 255));
	xgeXuiLabelSetAlign(&pApp->tLabel[iIndex], XGE_TEXT_ALIGN_LEFT | XGE_TEXT_ALIGN_MIDDLE);
	xgeXuiWidgetAdd(pParent, pApp->pLabel[iIndex]);
	return XGE_OK;
}

static int AddPicker(app_state_t* pApp, xge_xui_widget pParent, int iIndex, float fX, float fY, int iMode)
{
	xge_font pFont;

	if ( iIndex < 0 || iIndex >= DATE_PICKER_COUNT ) {
		return XGE_ERROR_INVALID_ARGUMENT;
	}
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	pApp->pPickerWidget[iIndex] = xgeXuiWidgetCreate();
	if ( pApp->pPickerWidget[iIndex] == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPickerWidget[iIndex], (xge_rect_t){ fX, fY, 260.0f, 30.0f });
	if ( xgeXuiDatePickerInit(&pApp->tPicker[iIndex], &pApp->tXui, pApp->pPickerWidget[iIndex], pFont) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiDatePickerSetMode(&pApp->tPicker[iIndex], iMode);
	xgeXuiDatePickerSetChanging(&pApp->tPicker[iIndex], OnChanging, pApp);
	xgeXuiDatePickerSetChange(&pApp->tPicker[iIndex], OnChange, pApp);
	xgeXuiDatePickerSetCommit(&pApp->tPicker[iIndex], OnCommit, pApp);
	xgeXuiDatePickerSetCancel(&pApp->tPicker[iIndex], OnCancel, pApp);
	xgeXuiDatePickerSetClear(&pApp->tPicker[iIndex], OnClear, pApp);
	xgeXuiWidgetAdd(pParent, pApp->pPickerWidget[iIndex]);
	return XGE_OK;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;
	xge_font pFont;
	float fLeftLabelX;
	float fLeftFieldX;
	float fRightLabelX;
	float fRightFieldX;
	float fY;

	pRoot = xgeXuiRoot(&pApp->tXui);
	pFont = pApp->bFontReady ? &pApp->tFont : NULL;
	if ( pRoot == NULL ) {
		return XGE_ERROR;
	}
	XgeXuiDemoApplyTheme(&pApp->tXui, pFont);
	pApp->pPanel = xgeXuiWidgetCreate();
	if ( pApp->pPanel == NULL ) {
		return XGE_ERROR_OUT_OF_MEMORY;
	}
	xgeXuiWidgetSetRect(pApp->pPanel, (xge_rect_t){ 22.0f, 24.0f, 916.0f, 474.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pPanel, 16.0f, 12.0f, 16.0f, 12.0f);
	xgeXuiPanelInit(&pApp->tPanel, pApp->pPanel);
	xgeXuiPanelSetTitle(&pApp->tPanel, pFont, "DatePicker");
	xgeXuiWidgetAdd(pRoot, pApp->pPanel);

	if ( AddLabel(pApp, pApp->pPanel, 0, 18.0f, 36.0f, 860.0f, "form picker modes: value, range, nullable, limits, seconds, and popup commit") != XGE_OK ) {
		return XGE_ERROR;
	}
	fLeftLabelX = 18.0f;
	fLeftFieldX = 150.0f;
	fRightLabelX = 500.0f;
	fRightFieldX = 636.0f;
	fY = 82.0f;

	if ( AddLabel(pApp, pApp->pPanel, 1, fLeftLabelX, fY, 120.0f, "date") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_DATE, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATE) != XGE_OK ||
		AddLabel(pApp, pApp->pPanel, 2, fRightLabelX, fY, 120.0f, "date range") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_DATE_RANGE, fRightFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATE_RANGE) != XGE_OK ) {
		return XGE_ERROR;
	}
	fY += 50.0f;
	if ( AddLabel(pApp, pApp->pPanel, 3, fLeftLabelX, fY, 120.0f, "time") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_TIME, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_TIME) != XGE_OK ||
		AddLabel(pApp, pApp->pPanel, 4, fRightLabelX, fY, 120.0f, "time range") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_TIME_RANGE, fRightFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) != XGE_OK ) {
		return XGE_ERROR;
	}
	fY += 50.0f;
	if ( AddLabel(pApp, pApp->pPanel, 5, fLeftLabelX, fY, 120.0f, "time seconds") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_TIME_SECONDS, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_TIME) != XGE_OK ||
		AddLabel(pApp, pApp->pPanel, 6, fRightLabelX, fY, 120.0f, "time range seconds") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_TIME_RANGE_SECONDS, fRightFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_TIME_RANGE) != XGE_OK ) {
		return XGE_ERROR;
	}
	fY += 50.0f;
	if ( AddLabel(pApp, pApp->pPanel, 7, fLeftLabelX, fY, 120.0f, "date time") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_DATETIME, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATETIME) != XGE_OK ||
		AddLabel(pApp, pApp->pPanel, 8, fRightLabelX, fY, 120.0f, "date time range") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_DATETIME_RANGE, fRightFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE) != XGE_OK ) {
		return XGE_ERROR;
	}
	fY += 64.0f;
	if ( AddLabel(pApp, pApp->pPanel, 9, fLeftLabelX, fY, 120.0f, "nullable") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_NULLABLE, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATE) != XGE_OK ||
		AddLabel(pApp, pApp->pPanel, 10, fRightLabelX, fY, 120.0f, "min / max") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_LIMITED, fRightFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATE) != XGE_OK ) {
		return XGE_ERROR;
	}
	fY += 50.0f;
	if ( AddLabel(pApp, pApp->pPanel, 11, fLeftLabelX, fY, 120.0f, "disabled") != XGE_OK ||
		AddPicker(pApp, pApp->pPanel, DP_DISABLED, fLeftFieldX, fY - 4.0f, XGE_XUI_DATE_PICKER_MODE_DATETIME) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiWidgetSetEnabled(pApp->pPickerWidget[DP_DISABLED], 0);

	if ( AddLabel(pApp, pApp->pPanel, 12, 18.0f, 424.0f, 860.0f, "") != XGE_OK ) {
		return XGE_ERROR;
	}

	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_DATE], DemoDateTimeSerial(2026, 5, 19, 11, 30, 19));
	xgeXuiDatePickerSetShowSecond(&pApp->tPicker[DP_TIME], 0);
	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_TIME], DemoTimeSerial(9, 30, 45));
	xgeXuiDatePickerSetShowSecond(&pApp->tPicker[DP_TIME_SECONDS], 1);
	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_TIME_SECONDS], DemoTimeSerial(10, 15, 30));
	xgeXuiDatePickerSetShowSecond(&pApp->tPicker[DP_DATETIME], 1);
	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_DATETIME], DemoDateTimeSerial(2026, 5, 19, 15, 45, 30));
	xgeXuiDatePickerSetRangeValue(&pApp->tPicker[DP_DATE_RANGE], DemoDateSerial(2026, 5, 20), DemoDateSerial(2026, 5, 18));
	xgeXuiDatePickerSetFirstDayOfWeek(&pApp->tPicker[DP_DATE_RANGE], 1);
	xgeXuiDatePickerSetDefaultRangeSpan(&pApp->tPicker[DP_TIME_RANGE], XRT_TIME_HOUR * 4);
	xgeXuiDatePickerSetShowSecond(&pApp->tPicker[DP_TIME_RANGE], 0);
	xgeXuiDatePickerSetRangeValue(&pApp->tPicker[DP_TIME_RANGE], DemoTimeSerial(9, 0, 12), DemoTimeSerial(13, 0, 59));
	xgeXuiDatePickerSetShowSecond(&pApp->tPicker[DP_TIME_RANGE_SECONDS], 1);
	xgeXuiDatePickerSetRangeValue(&pApp->tPicker[DP_TIME_RANGE_SECONDS], DemoTimeSerial(10, 20, 30), DemoTimeSerial(11, 40, 50));
	xgeXuiDatePickerSetRangeValue(&pApp->tPicker[DP_DATETIME_RANGE], DemoDateTimeSerial(2026, 5, 19, 9, 0, 0), DemoDateTimeSerial(2026, 5, 19, 13, 0, 0));
	xgeXuiDatePickerSetFormat(&pApp->tPicker[DP_DATETIME_RANGE], "yyyy-mm-dd hh:nn");
	xgeXuiDatePickerSetRangeSeparator(&pApp->tPicker[DP_DATETIME_RANGE], " - ");
	xgeXuiDatePickerSetNullable(&pApp->tPicker[DP_NULLABLE], 1);
	xgeXuiDatePickerClearValue(&pApp->tPicker[DP_NULLABLE]);
	xgeXuiDatePickerSetLimits(&pApp->tPicker[DP_LIMITED], DemoDateSerial(2026, 5, 1), DemoDateSerial(2026, 5, 31));
	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_LIMITED], DemoDateSerial(2026, 6, 20));
	xgeXuiDatePickerSetValue(&pApp->tPicker[DP_DISABLED], DemoDateTimeSerial(2026, 5, 19, 18, 0, 0));
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xtime tStart;
	xtime tEnd;
	int iOpen;
	int iClosed;
	float fX;
	float fY;

	xgeXuiPaint(&pApp->tXui);
	pApp->bCreateOK = 1;
	for ( int i = 0; i < DATE_PICKER_COUNT; i++ ) {
		if ( pApp->tPicker[i].pWidget == NULL ) {
			pApp->bCreateOK = 0;
		}
	}
	pApp->bModeOK =
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_DATE]) == XGE_XUI_DATE_PICKER_MODE_DATE &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_TIME]) == XGE_XUI_DATE_PICKER_MODE_TIME &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_TIME_SECONDS]) == XGE_XUI_DATE_PICKER_MODE_TIME &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_DATETIME]) == XGE_XUI_DATE_PICKER_MODE_DATETIME &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_DATE_RANGE]) == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_TIME_RANGE]) == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_TIME_RANGE_SECONDS]) == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE &&
		xgeXuiDatePickerGetMode(&pApp->tPicker[DP_DATETIME_RANGE]) == XGE_XUI_DATE_PICKER_MODE_DATETIME_RANGE &&
		pApp->tPicker[DP_DATE_RANGE].iFirstDayOfWeek == 1;

	pApp->bValueOK =
		xgeXuiDatePickerHasValue(&pApp->tPicker[DP_DATE]) &&
		xgeXuiDatePickerGetValue(&pApp->tPicker[DP_DATE]) == DemoDateSerial(2026, 5, 19) &&
		xgeXuiDatePickerGetValue(&pApp->tPicker[DP_TIME]) == DemoTimeSerial(9, 30, 0) &&
		xgeXuiDatePickerGetValue(&pApp->tPicker[DP_TIME_SECONDS]) == DemoTimeSerial(10, 15, 30) &&
		xgeXuiDatePickerGetValue(&pApp->tPicker[DP_DATETIME]) == DemoDateTimeSerial(2026, 5, 19, 15, 45, 30);

	xgeXuiDatePickerGetRangeValue(&pApp->tPicker[DP_DATE_RANGE], &tStart, &tEnd);
	pApp->bRangeOK = (tStart == DemoDateSerial(2026, 5, 18)) && (tEnd == DemoDateSerial(2026, 5, 20));
	xgeXuiDatePickerGetRangeValue(&pApp->tPicker[DP_TIME_RANGE], &tStart, &tEnd);
	pApp->bRangeOK = pApp->bRangeOK && (tStart == DemoTimeSerial(9, 0, 0)) && (tEnd == DemoTimeSerial(13, 0, 0));
	xgeXuiDatePickerGetRangeValue(&pApp->tPicker[DP_TIME_RANGE_SECONDS], &tStart, &tEnd);
	pApp->bRangeOK = pApp->bRangeOK && (tStart == DemoTimeSerial(10, 20, 30)) && (tEnd == DemoTimeSerial(11, 40, 50));
	pApp->bLimitOK = xgeXuiDatePickerGetValue(&pApp->tPicker[DP_LIMITED]) == DemoDateSerial(2026, 5, 31);
	pApp->bClearOK = !xgeXuiDatePickerHasValue(&pApp->tPicker[DP_NULLABLE]);

	fX = pApp->pPickerWidget[DP_DATE]->tBorderRect.fX + 12.0f;
	fY = pApp->pPickerWidget[DP_DATE]->tBorderRect.fY + 12.0f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	iOpen = xgeXuiDatePickerIsPopupOpen(&pApp->tPicker[DP_DATE]);
	fX = pApp->tPicker[DP_DATE].tYearRect[0].fX + pApp->tPicker[DP_DATE].tYearRect[0].fW * 0.5f;
	fY = pApp->tPicker[DP_DATE].tYearRect[0].fY + pApp->tPicker[DP_DATE].tYearRect[0].fH * 0.5f;
	ClickMouse(pApp, fX, fY);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = 'A';
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_TEXT;
	tEvent.iCodepoint = '7';
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iType = XGE_EVENT_KEY_DOWN;
	tEvent.iParam1 = XGE_KEY_ENTER;
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	pApp->bYearEditOK = pApp->tPicker[DP_DATE].tViewMonth[0] == DemoDateSerial(7, 5, 1);
	fX = pApp->tPicker[DP_DATE].arrDayRect[0][20].fX + pApp->tPicker[DP_DATE].arrDayRect[0][20].fW * 0.5f;
	fY = pApp->tPicker[DP_DATE].arrDayRect[0][20].fY + pApp->tPicker[DP_DATE].arrDayRect[0][20].fH * 0.5f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	fX = pApp->tPicker[DP_DATE].arrFooterRect[3].fX + pApp->tPicker[DP_DATE].arrFooterRect[3].fW * 0.5f;
	fY = pApp->tPicker[DP_DATE].arrFooterRect[3].fY + pApp->tPicker[DP_DATE].arrFooterRect[3].fH * 0.5f;
	ClickMouse(pApp, fX, fY);
	iClosed = !xgeXuiDatePickerIsPopupOpen(&pApp->tPicker[DP_DATE]);
	pApp->bPopupOK = iOpen && iClosed && pApp->bYearEditOK && (pApp->iChangingCount > 0) && (pApp->iChangeCount > 0) && (pApp->iCommitCount > 0);

	fX = pApp->pPickerWidget[DP_TIME]->tBorderRect.fX + 12.0f;
	fY = pApp->pPickerWidget[DP_TIME]->tBorderRect.fY + 12.0f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	fX = pApp->tPicker[DP_TIME].arrFooterRect[2].fX + pApp->tPicker[DP_TIME].arrFooterRect[2].fW * 0.5f;
	fY = pApp->tPicker[DP_TIME].arrFooterRect[2].fY + pApp->tPicker[DP_TIME].arrFooterRect[2].fH * 0.5f;
	ClickMouse(pApp, fX, fY);
	pApp->bPopupOK = pApp->bPopupOK && (pApp->iCancelCount > 0);

	fX = pApp->pPickerWidget[DP_NULLABLE]->tBorderRect.fX + 12.0f;
	fY = pApp->pPickerWidget[DP_NULLABLE]->tBorderRect.fY + 12.0f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	fX = pApp->tPicker[DP_NULLABLE].arrFooterRect[1].fX + pApp->tPicker[DP_NULLABLE].arrFooterRect[1].fW * 0.5f;
	fY = pApp->tPicker[DP_NULLABLE].arrFooterRect[1].fY + pApp->tPicker[DP_NULLABLE].arrFooterRect[1].fH * 0.5f;
	ClickMouse(pApp, fX, fY);
	pApp->bClearOK = pApp->bClearOK && (pApp->iClearCount > 0);
}

static void UpdateSummary(app_state_t* pApp)
{
	char sText[256];

	snprintf(
		sText,
		sizeof(sText),
		"create=%d mode=%d value=%d range=%d limit=%d clear=%d popup=%d year=%d | changing=%d change=%d commit=%d cancel=%d clearEvent=%d",
		pApp->bCreateOK,
		pApp->bModeOK,
		pApp->bValueOK,
		pApp->bRangeOK,
		pApp->bLimitOK,
		pApp->bClearOK,
		pApp->bPopupOK,
		pApp->bYearEditOK,
		pApp->iChangingCount,
		pApp->iChangeCount,
		pApp->iCommitCount,
		pApp->iCancelCount,
		pApp->iClearCount);
	xgeXuiLabelSetText(&pApp->tLabel[12], sText);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	if ( pApp->iFrameLimit > 0 ) {
		RunChecks(pApp);
	}
	UpdateSummary(pApp);
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUnit(&pApp->tXui);
	if ( pApp->bFontReady ) {
		xgeFontFree(&pApp->tFont);
	}
	return XGE_OK;
}

static int AppEvent(xge_scene pScene, const xge_event_t* pEvent)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	if ( pEvent->iType == XGE_EVENT_KEY_DOWN && pEvent->iParam1 == XGE_KEY_ESCAPE ) {
		xgeQuit();
		return XGE_OK;
	}
	xgeXuiDispatchEvent(&pApp->tXui, pEvent);
	UpdateSummary(pApp);
	return XGE_OK;
}

static int AppUpdate(xge_scene pScene, float fDelta)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiUpdate(&pApp->tXui, fDelta);
	pApp->iFrameCount++;
	if ( (pApp->iFrameLimit > 0) && (pApp->iFrameCount >= pApp->iFrameLimit) ) {
		printf(
			"xui_datepicker final-summary frames=%d create=%d mode=%d value=%d range=%d limit=%d clear=%d popup=%d year=%d changing=%d change=%d commit=%d cancel=%d clearEvent=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bModeOK,
			pApp->bValueOK,
			pApp->bRangeOK,
			pApp->bLimitOK,
			pApp->bClearOK,
			pApp->bPopupOK,
			pApp->bYearEditOK,
			pApp->iChangingCount,
			pApp->iChangeCount,
			pApp->iCommitCount,
			pApp->iCancelCount,
			pApp->iClearCount);
		xgeQuit();
	}
	return XGE_OK;
}

static int AppDraw(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeBegin();
	xgeClear(XGE_COLOR_RGBA(232, 240, 248, 255));
	xgeXuiPaint(&pApp->tXui);
	xgeEnd();
	xgePresent();
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	app_state_t tApp;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DATEPICKER_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 960;
	tDesc.iHeight = 540;
	tDesc.sTitle = "XUI DatePicker";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	tApp.tScene.pUser = &tApp;
	tApp.tScene.onEnter = AppEnter;
	tApp.tScene.onLeave = AppLeave;
	tApp.tScene.onEvent = AppEvent;
	tApp.tScene.onUpdate = AppUpdate;
	tApp.tScene.onDraw = AppDraw;
	if ( xgeSceneSet(&tApp.tScene) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	iExitCode = xgeRun(NULL, NULL);
	xgeUnit();
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bModeOK && tApp.bValueOK && tApp.bRangeOK && tApp.bLimitOK && tApp.bClearOK && tApp.bPopupOK) ? 0 : 3;
}
