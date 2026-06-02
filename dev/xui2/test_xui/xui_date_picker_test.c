#include "xui.h"
#include "xui_test_proxy.h"

#include <stdio.h>
#include <string.h>

#define XUI_TEST_CHECK(expr, msg) \
	do { \
		if ( !(expr) ) { \
			printf("xui_date_picker_test failed: %s\n", (msg)); \
			iFailed = 1; \
			goto cleanup; \
		} \
	} while ( 0 )

typedef struct xui_date_picker_test_events_t {
	int iChanging;
	int iChange;
	int iCommit;
	int iCancel;
	int iClear;
	xtime tStart;
	xtime tEnd;
	int iMode;
} xui_date_picker_test_events_t;

static int __xuiDatePickerRectEq(xui_rect_t tRect, float fX, float fY, float fW, float fH)
{
	return (tRect.fX == fX) && (tRect.fY == fY) && (tRect.fW == fW) && (tRect.fH == fH);
}

static int __xuiDatePickerTimeFieldsFit(xui_widget pPicker, int iPanel, int iCount)
{
	xui_rect_t tPanel;
	xui_rect_t tField;
	int i;

	tPanel = xuiDatePickerGetTimePanelRect(pPicker, iPanel);
	if ( tPanel.fW <= 0.0f || tPanel.fH <= 0.0f ) return 0;
	for ( i = 0; i < iCount; i++ ) {
		tField = xuiDatePickerGetTimeRect(pPicker, iPanel, i);
		if ( tField.fW <= 0.0f || tField.fH <= 0.0f ) return 0;
		if ( tField.fX < tPanel.fX + 10.0f ) return 0;
		if ( tField.fX + tField.fW > tPanel.fX + tPanel.fW - 10.0f ) return 0;
		if ( tField.fY - 18.0f < tPanel.fY + 24.0f ) return 0;
		if ( tField.fY + tField.fH > tPanel.fY + tPanel.fH - 6.0f ) return 0;
	}
	return 1;
}

static xtime __xuiDatePickerTestTime(int iHour, int iMinute, int iSecond)
{
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
}

static xtime __xuiDatePickerTestDateTime(int64 iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
{
	return xrtDateSerial(iYear, iMonth, iDay) + __xuiDatePickerTestTime(iHour, iMinute, iSecond);
}

static void __xuiDatePickerChanging(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_date_picker_test_events_t*)pUser;
	pEvents->iChanging++;
	pEvents->tStart = tStart;
	pEvents->tEnd = tEnd;
	pEvents->iMode = iMode;
}

static void __xuiDatePickerChange(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_date_picker_test_events_t*)pUser;
	pEvents->iChange++;
	pEvents->tStart = tStart;
	pEvents->tEnd = tEnd;
	pEvents->iMode = iMode;
}

static void __xuiDatePickerCommit(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_date_picker_test_events_t*)pUser;
	pEvents->iCommit++;
	pEvents->tStart = tStart;
	pEvents->tEnd = tEnd;
	pEvents->iMode = iMode;
}

static void __xuiDatePickerCancel(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_date_picker_test_events_t*)pUser;
	pEvents->iCancel++;
	pEvents->tStart = tStart;
	pEvents->tEnd = tEnd;
	pEvents->iMode = iMode;
}

static void __xuiDatePickerClear(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_test_events_t* pEvents;

	(void)pWidget;
	pEvents = (xui_date_picker_test_events_t*)pUser;
	pEvents->iClear++;
	pEvents->tStart = tStart;
	pEvents->tEnd = tEnd;
	pEvents->iMode = iMode;
}

static int __xuiDatePickerRender(xui_context pContext, xui_surface pTarget)
{
	xui_rect_i_t tFullRect;

	tFullRect = (xui_rect_i_t){0, 0, 760, 620};
	return xuiRender(pContext, pTarget, &tFullRect, 1);
}

static int __xuiDatePickerClick(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, fX, fY, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiDatePickerClickPanelRect(xui_context pContext, xui_widget pPanel, xui_rect_t tRect)
{
	xui_rect_t tWorld;

	tWorld = xuiWidgetGetWorldRect(pPanel);
	return __xuiDatePickerClick(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f);
}

static int __xuiDatePickerInputKey(xui_context pContext, int iKey)
{
	int iRet;

	iRet = xuiInputKeyDown(pContext, iKey, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiDatePickerInputText(xui_context pContext, const char* sText)
{
	int iRet;

	if ( sText == NULL ) return XUI_OK;
	while ( *sText != '\0' ) {
		iRet = xuiInputText(pContext, (uint32_t)(unsigned char)*sText++);
		if ( iRet != XUI_OK ) return iRet;
		iRet = xuiDispatchPendingEvents(pContext);
		if ( iRet != XUI_OK ) return iRet;
	}
	return XUI_OK;
}

static void __xuiDatePickerHeaderFieldRects(xui_widget pPicker, int iPanel, xui_rect_t* pYear, xui_rect_t* pMonth)
{
	xui_rect_t tPanel;
	float fHeaderH;
	float fButtonW;
	float fYearW;
	float fMonthW;
	float fGap;
	float fComboW;
	float fComboX;

	tPanel = xuiDatePickerGetCalendarPanelRect(pPicker, iPanel);
	fHeaderH = 32.0f;
	fButtonW = 28.0f;
	fYearW = 78.0f;
	fMonthW = 62.0f;
	fGap = 6.0f;
	fComboW = fYearW + fMonthW + fGap;
	if ( fComboW > tPanel.fW - fButtonW * 2.0f - 12.0f ) {
		fComboW = tPanel.fW - fButtonW * 2.0f - 12.0f;
		if ( fComboW < 104.0f ) fComboW = 104.0f;
		fYearW = (fComboW - fGap) * 0.56f;
		fMonthW = fComboW - fGap - fYearW;
	}
	fComboX = tPanel.fX + (tPanel.fW - fComboW) * 0.5f;
	if ( pYear != NULL ) *pYear = (xui_rect_t){fComboX, tPanel.fY + 4.0f, fYearW, fHeaderH - 8.0f};
	if ( pMonth != NULL ) *pMonth = (xui_rect_t){fComboX + fYearW + fGap, tPanel.fY + 4.0f, fMonthW, fHeaderH - 8.0f};
}

static xui_rect_t __xuiDatePickerMonthOptionRect(xui_widget pPicker, int iPanel, int iIndex)
{
	xui_rect_t tCalendar;
	xui_rect_t tMonth;
	xui_rect_t tDrop;
	float fPopupH;
	float fCellW;
	float fCellH;

	__xuiDatePickerHeaderFieldRects(pPicker, iPanel, NULL, &tMonth);
	tCalendar = xuiDatePickerGetCalendarPanelRect(pPicker, iPanel);
	(void)xuiDatePickerGetPopupSize(pPicker, NULL, &fPopupH);
	tDrop = (xui_rect_t){tMonth.fX + tMonth.fW - 144.0f, tMonth.fY + tMonth.fH + 2.0f, 144.0f, 96.0f};
	if ( tDrop.fX < tCalendar.fX + 2.0f ) tDrop.fX = tCalendar.fX + 2.0f;
	if ( tDrop.fX + tDrop.fW > tCalendar.fX + tCalendar.fW - 2.0f ) tDrop.fX = tCalendar.fX + tCalendar.fW - tDrop.fW - 2.0f;
	if ( tDrop.fY + tDrop.fH > fPopupH - 50.0f ) tDrop.fY = fPopupH - 50.0f - tDrop.fH;
	if ( tDrop.fY < tMonth.fY + tMonth.fH + 2.0f ) tDrop.fY = tMonth.fY + tMonth.fH + 2.0f;
	fCellW = tDrop.fW / 3.0f;
	fCellH = tDrop.fH / 4.0f;
	return (xui_rect_t){tDrop.fX + (float)(iIndex % 3) * fCellW, tDrop.fY + (float)(iIndex / 3) * fCellH, fCellW, fCellH};
}

static int __xuiDatePickerFindDay(xui_widget pPicker, int iPanel, xtime tDay)
{
	int i;

	for ( i = 0; i < 42; i++ ) {
		if ( xuiDatePickerGetDayValue(pPicker, iPanel, i) == xrtDatePart(tDay) ) {
			return i;
		}
	}
	return -1;
}

int main(void)
{
	xui_test_proxy_state_t tState;
	xui_date_picker_test_events_t tEvents;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_surface pTarget;
	xui_surface pOwnerCache;
	xui_font pFont;
	xui_date_picker_desc_t tDesc;
	xui_rect_t tRect;
	xui_rect_t tYearRect;
	xui_rect_t tMonthRect;
	xui_rect_t tTimeRect;
	xtime tStart;
	xtime tEnd;
	xtime tTarget;
	float fRadius;
	float fBorderWidth;
	int iFailed;
	int iRet;
	int iIndex;

	pContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pTarget = NULL;
	pOwnerCache = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	xuiTestProxyInit(&tState);

	iRet = xuiCreate(&pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && pContext != NULL, "context create");
	iRet = xuiSetProxy(pContext, &tState.tProxy);
	XUI_TEST_CHECK(iRet == XUI_OK, "proxy set");
	iRet = tState.tProxy.fontLoadMemory(&tState.tProxy, &pFont, "datepicker", 10, 14.0f, XUI_FONT_FORMAT_TTF);
	XUI_TEST_CHECK(iRet == XUI_OK && pFont != NULL, "font create");
	iRet = xuiSetDefaultFont(pContext, pFont);
	XUI_TEST_CHECK(iRet == XUI_OK, "default font");
	iRet = xuiInputViewport(pContext, 760.0f, 620.0f);
	XUI_TEST_CHECK(iRet == XUI_OK, "viewport");

	iRet = xuiWidgetCreate(pContext, &pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK && pRoot != NULL, "root create");
	xuiWidgetSetRect(pRoot, (xui_rect_t){0.0f, 0.0f, 760.0f, 620.0f});
	iRet = xuiSetRootWidget(pContext, pRoot);
	XUI_TEST_CHECK(iRet == XUI_OK, "set root");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_DATE_PICKER_MODE_DATE;
	tDesc.bHasValue = 1;
	tDesc.tValue = __xuiDatePickerTestDateTime(2026, 5, 19, 11, 30, 19);
	tDesc.pFont = pFont;
	iRet = xuiDatePickerCreate(pContext, &pPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pPicker != NULL, "date picker create");
	(void)xuiDatePickerSetChanging(pPicker, __xuiDatePickerChanging, &tEvents);
	(void)xuiDatePickerSetChange(pPicker, __xuiDatePickerChange, &tEvents);
	(void)xuiDatePickerSetCommit(pPicker, __xuiDatePickerCommit, &tEvents);
	(void)xuiDatePickerSetCancel(pPicker, __xuiDatePickerCancel, &tEvents);
	(void)xuiDatePickerSetClear(pPicker, __xuiDatePickerClear, &tEvents);
	xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 210.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "add picker");

	iRet = xuiTestSurfaceCreate(&tState, &pTarget, 760, 620, XUI_SURFACE_USAGE_TARGET);
	XUI_TEST_CHECK(iRet == XUI_OK && pTarget != NULL, "target create");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "layout");
	iRet = xuiUpdate(pContext, 0.016f);
	XUI_TEST_CHECK(iRet == XUI_OK, "update");
	iRet = __xuiDatePickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "render");

	XUI_TEST_CHECK(xuiDatePickerGetMode(pPicker) == XUI_DATE_PICKER_MODE_DATE, "initial mode");
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == xrtDateSerial(2026, 5, 19), "date normalizes");
	XUI_TEST_CHECK(strcmp(xuiDatePickerGetText(pPicker), "2026-05-19") == 0, "date text");
	iRet = xuiDatePickerGetMetrics(pPicker, &fRadius, &fBorderWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fRadius == 4.0f && fBorderWidth == 1.0f, "default metrics");
	pOwnerCache = xuiWidgetGetCacheSurface(pPicker, xuiWidgetGetStateId(pPicker));
	XUI_TEST_CHECK(pOwnerCache != NULL, "owner cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pOwnerCache) >= 4, "owner cache draw count");
	tRect = xuiTestSurfaceGetLastRect(pOwnerCache);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 0.5f, 0.5f, 209.0f, 29.0f), "owner border inside cache");
	tRect = xuiDatePickerGetTextRect(pPicker);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 8.0f, 0.0f, 166.0f, 30.0f), "owner text rect local");
	tRect = xuiDatePickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 180.0f, 0.0f, 30.0f, 30.0f), "owner button rect local");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_TIME);
	XUI_TEST_CHECK(iRet == XUI_OK, "time mode");
	iRet = xuiDatePickerSetShowSecond(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "hide second");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestTime(9, 30, 45));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestTime(9, 30, 0), "time no seconds");
	XUI_TEST_CHECK(strcmp(xuiDatePickerGetText(pPicker), "09:30") == 0, "time text");
	tRect = xuiDatePickerGetTimePanelRect(pPicker, 0);
	tTimeRect = xuiDatePickerGetTimeRect(pPicker, 0, 0);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 12.0f, 24.0f, 244.0f, 104.0f), "time panel rect");
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tTimeRect, 58.0f, 71.0f, 72.0f, 28.0f), "time field vertical center");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 2), "time fields stay inside panel");
	iRet = xuiDatePickerSetShowSecond(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "show second");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestTime(10, 15, 30));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiDatePickerGetText(pPicker), "10:15:30") == 0, "time second text");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 3), "time second fields stay inside panel");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "time edit open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "time edit layout");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "time hour field click");
	iRet = __xuiDatePickerInputText(pContext, "16");
	XUI_TEST_CHECK(iRet == XUI_OK, "time hour text");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "time hour commit");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 1));
	XUI_TEST_CHECK(iRet == XUI_OK, "time minute field click");
	iRet = __xuiDatePickerInputText(pContext, "45");
	XUI_TEST_CHECK(iRet == XUI_OK, "time minute text");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "time minute commit");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "time edit ok closes");
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestTime(16, 45, 30), "time edit commits typed values");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATETIME);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime mode");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestDateTime(2026, 5, 19, 15, 45, 30));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDateTime(2026, 5, 19, 15, 45, 30), "datetime value");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 3), "datetime time fields stay inside panel");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATE_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "date range mode");
	iRet = xuiDatePickerSetRangeValue(pPicker, xrtDateSerial(2026, 5, 20), xrtDateSerial(2026, 5, 18));
	XUI_TEST_CHECK(iRet == XUI_OK, "date range set");
	iRet = xuiDatePickerGetRangeValue(pPicker, &tStart, &tEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && tStart == xrtDateSerial(2026, 5, 18) && tEnd == xrtDateSerial(2026, 5, 20), "date range swaps");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_TIME_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "time range mode");
	iRet = xuiDatePickerSetShowSecond(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "time range hide second");
	iRet = xuiDatePickerSetRangeValue(pPicker, __xuiDatePickerTestTime(13, 0, 59), __xuiDatePickerTestTime(9, 0, 12));
	XUI_TEST_CHECK(iRet == XUI_OK, "time range set");
	iRet = xuiDatePickerGetRangeValue(pPicker, &tStart, &tEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && tStart == __xuiDatePickerTestTime(9, 0, 0) && tEnd == __xuiDatePickerTestTime(13, 0, 0), "time range normalize");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 2) && __xuiDatePickerTimeFieldsFit(pPicker, 1, 2), "time range fields stay inside panels");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATETIME_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime range mode");
	iRet = xuiDatePickerSetShowSecond(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime range hide second");
	iRet = xuiDatePickerSetFormat(pPicker, "yyyy-mm-dd hh:nn");
	XUI_TEST_CHECK(iRet == XUI_OK, "custom format");
	iRet = xuiDatePickerSetRangeSeparator(pPicker, " / ");
	XUI_TEST_CHECK(iRet == XUI_OK, "range separator");
	iRet = xuiDatePickerSetRangeValue(pPicker, __xuiDatePickerTestDateTime(2026, 5, 19, 9, 0, 22), __xuiDatePickerTestDateTime(2026, 5, 19, 13, 0, 44));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiDatePickerGetText(pPicker), "2026-05-19 09:00 / 2026-05-19 13:00") == 0, "datetime range text");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 2) && __xuiDatePickerTimeFieldsFit(pPicker, 1, 2), "datetime range fields stay inside panels");
	iRet = xuiDatePickerSetShowSecond(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime range show second");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 3) && __xuiDatePickerTimeFieldsFit(pPicker, 1, 3), "datetime range second fields stay inside panels");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATE);
	XUI_TEST_CHECK(iRet == XUI_OK, "date mode for limits");
	iRet = xuiDatePickerSetFormat(pPicker, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "default format");
	iRet = xuiDatePickerSetLimits(pPicker, xrtDateSerial(2026, 5, 1), xrtDateSerial(2026, 5, 31));
	XUI_TEST_CHECK(iRet == XUI_OK, "limits");
	iRet = xuiDatePickerSetValue(pPicker, xrtDateSerial(2026, 6, 20));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerGetValue(pPicker) == xrtDateSerial(2026, 5, 31), "limit clamps");

	iRet = xuiDatePickerSetNullable(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "nullable");
	iRet = xuiDatePickerClearValue(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerHasValue(pPicker) && strcmp(xuiDatePickerGetText(pPicker), "") == 0, "nullable clear");
	iRet = xuiDatePickerSetNullable(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerHasValue(pPicker), "nullable off ensures value");
	(void)xuiDatePickerClearLimits(pPicker);
	(void)xuiDatePickerSetValue(pPicker, xrtDateSerial(2026, 5, 19));

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiDatePickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pPopup = xuiDatePickerGetPopupWidget(pPicker);
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	XUI_TEST_CHECK(pPopup != NULL && pPanel != NULL && xuiGetFocusWidget(pContext) == pPanel, "popup focus");
	XUI_TEST_CHECK((xuiDatePickerGetState(pPicker) & XUI_DATE_PICKER_STATE_OPEN) != 0u, "open state");
	tRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tRect.fY >= 60.0f, "popup below owner");

	__xuiDatePickerHeaderFieldRects(pPicker, 0, &tYearRect, &tMonthRect);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, tYearRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "year field click");
	iRet = __xuiDatePickerInputText(pContext, "2027");
	XUI_TEST_CHECK(iRet == XUI_OK, "year text input");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "year edit commit");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, tMonthRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "month field click");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, __xuiDatePickerMonthOptionRect(pPicker, 0, 11));
	XUI_TEST_CHECK(iRet == XUI_OK, "month option click");
	iIndex = __xuiDatePickerFindDay(pPicker, 0, xrtDateSerial(2027, 12, 19));
	XUI_TEST_CHECK(iIndex >= 0, "year month combo changes calendar view");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ESCAPE);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "combo view cancel closes");
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == xrtDateSerial(2026, 5, 19), "combo view does not commit value");

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "reopen after combo view");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen combo layout");
	iRet = __xuiDatePickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen combo render");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	tTarget = xrtDateSerial(2026, 5, 20);
	iIndex = __xuiDatePickerFindDay(pPicker, 0, tTarget);
	XUI_TEST_CHECK(iIndex >= 0, "find target day");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetDayRect(pPicker, 0, iIndex));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "day keeps popup open");
	XUI_TEST_CHECK(tEvents.iChanging > 0 && xuiDatePickerGetValue(pPicker) == xrtDateSerial(2026, 5, 19), "draft only before ok");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "ok closes");
	XUI_TEST_CHECK(tEvents.iCommit > 0 && tEvents.iChange > 0 && xuiDatePickerGetValue(pPicker) == tTarget, "ok commits");

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "reopen");
	tTarget = xrtDateSerial(2026, 5, 21);
	iIndex = __xuiDatePickerFindDay(pPicker, 0, tTarget);
	XUI_TEST_CHECK(iIndex >= 0, "find cancel day");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetDayRect(pPicker, 0, iIndex));
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel draft day");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "escape closes");
	XUI_TEST_CHECK(tEvents.iCancel > 0 && xuiDatePickerGetValue(pPicker) == xrtDateSerial(2026, 5, 20), "escape cancels");

	iRet = xuiDatePickerSetNullable(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "nullable for clear");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "open clear");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_CLEAR));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "clear closes");
	XUI_TEST_CHECK(tEvents.iClear > 0 && !xuiDatePickerHasValue(pPicker), "clear callback");

	iRet = xuiWidgetSetEnabled(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "disabled no open");
	XUI_TEST_CHECK((xuiDatePickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u, "disabled state");

cleanup:
	if ( pContext != NULL ) {
		xuiDestroy(pContext);
	}
	if ( pFont != NULL ) {
		tState.tProxy.fontDestroy(&tState.tProxy, pFont);
	}
	if ( iFailed ) {
		return 1;
	}
	printf("xui_date_picker_test passed\n");
	return 0;
}
