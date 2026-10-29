#include "xui.h"
#include "xui_test_proxy.h"

#include <math.h>
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

typedef struct xui_date_picker_destroy_test_t {
	xui_widget* ppWidget;
	int iCount;
	int bSawClosed;
} xui_date_picker_destroy_test_t;

typedef struct xui_date_picker_event_destroy_test_t {
	xui_widget* ppWidget;
	int iEventType;
	int iCount;
} xui_date_picker_event_destroy_test_t;

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

static int __xuiDatePickerSameValueSettersStayClean(xui_widget pPicker)
{
	xui_widget pPanel;
	xui_widget pPopup;
	xui_font pFont;
	xtime tValue;
	xtime tStart;
	xtime tEnd;
	xtime tMin;
	xtime tMax;
	float fBorderWidth;
	uint32_t arrColors[6];
	uint32_t arrBorderColors[3];
	uint32_t arrArrowColors[2];
	uint32_t arrPopupColors[11];
	int bHasMin;
	int bHasMax;
	int iRet;

	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	pPopup = xuiDatePickerGetPopupWidget(pPicker);
	if ( pPanel == NULL || pPopup == NULL ) return 0;
	pFont = xuiDatePickerGetFont(pPicker);
	tValue = xuiDatePickerGetValue(pPicker);
	iRet = xuiDatePickerGetRangeValue(pPicker, &tStart, &tEnd);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetLimitRange(pPicker, &bHasMin, &tMin, &bHasMax, &tMax);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetMetrics(pPicker, &fBorderWidth);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetColors(pPicker,
		&arrColors[0], &arrColors[1], &arrColors[2], &arrColors[3], &arrColors[4], &arrColors[5]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetBorderColors(pPicker,
		&arrBorderColors[0], &arrBorderColors[1], &arrBorderColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetArrowColors(pPicker,
		&arrArrowColors[0], &arrArrowColors[1]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerGetPopupColors(pPicker,
		&arrPopupColors[0], &arrPopupColors[1], &arrPopupColors[2], &arrPopupColors[3],
		&arrPopupColors[4], &arrPopupColors[5], &arrPopupColors[6], &arrPopupColors[7],
		&arrPopupColors[8], &arrPopupColors[9], &arrPopupColors[10]);
	if ( iRet != XUI_OK ) return 0;
	xuiWidgetClearDirty(pPicker, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pPanel, XUI_WIDGET_DIRTY_ALL);
	xuiWidgetClearDirty(pPopup, XUI_WIDGET_DIRTY_ALL);
	iRet = xuiDatePickerSetMode(pPicker, xuiDatePickerGetMode(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetNullable(pPicker, xuiDatePickerGetNullable(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetValue(pPicker, tValue);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetRangeValue(pPicker, tStart, tEnd);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerClearValue(pPicker);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetLimitRange(pPicker, bHasMin, tMin, bHasMax, tMax);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetFormat(pPicker, NULL);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetRangeSeparator(pPicker, xuiDatePickerGetRangeSeparator(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetShowSecond(pPicker, xuiDatePickerGetShowSecond(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetFirstDayOfWeek(pPicker, xuiDatePickerGetFirstDayOfWeek(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetDefaultRangeSpan(pPicker, xuiDatePickerGetDefaultRangeSpan(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetPopupSize(pPicker, 0.0f, 0.0f);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetPopupPlacement(pPicker, xuiDatePickerGetPopupPlacement(pPicker));
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetMetrics(pPicker, fBorderWidth);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetColors(pPicker,
		arrColors[0], arrColors[1], arrColors[2], arrColors[3], arrColors[4], arrColors[5]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetBorderColors(pPicker,
		arrBorderColors[0], arrBorderColors[1], arrBorderColors[2]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetArrowColors(pPicker,
		arrArrowColors[0], arrArrowColors[1]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetPopupColors(pPicker,
		arrPopupColors[0], arrPopupColors[1], arrPopupColors[2], arrPopupColors[3],
		arrPopupColors[4], arrPopupColors[5], arrPopupColors[6], arrPopupColors[7],
		arrPopupColors[8], arrPopupColors[9], arrPopupColors[10]);
	if ( iRet == XUI_OK ) iRet = xuiDatePickerSetFont(pPicker, pFont);
	return iRet == XUI_OK && xuiWidgetGetDirtyFlags(pPicker) == 0u &&
		xuiWidgetGetDirtyFlags(pPanel) == 0u && xuiWidgetGetDirtyFlags(pPopup) == 0u;
}

static xtime __xuiDatePickerTestTime(int iHour, int iMinute, int iSecond)
{
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond * XRT_TIME_SECOND;
}

static xtime __xuiDatePickerTestDateTime(int64 iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
{
	xtime tDate = 0;
	(void)xrtDate(iYear, iMonth, iDay, &tDate);
	return tDate + __xuiDatePickerTestTime(iHour, iMinute, iSecond);
}

static xtime __xuiDatePickerTestDate(int64 iYear, int iMonth, int iDay)
{
	xtime tDate = 0;
	(void)xrtDate(iYear, iMonth, iDay, &tDate);
	return tDate;
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

static void __xuiDatePickerDestroyOnCommit(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_date_picker_destroy_test_t* pDestroy;

	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDestroy = (xui_date_picker_destroy_test_t*)pUser;
	if ( pDestroy == NULL ) return;
	pDestroy->iCount++;
	pDestroy->bSawClosed = !xuiDatePickerIsOpen(pWidget);
	xuiWidgetDestroy(pWidget);
	if ( pDestroy->ppWidget != NULL ) *pDestroy->ppWidget = NULL;
}

static int __xuiDatePickerDestroyVictimOnEvent(xui_widget pWidget, const xui_event_t* pEvent, void* pUser)
{
	xui_date_picker_event_destroy_test_t* pDestroy = (xui_date_picker_event_destroy_test_t*)pUser;
	xui_widget pVictim;

	(void)pWidget;
	if ( (pDestroy == NULL) || (pEvent == NULL) || (pEvent->iType != pDestroy->iEventType) ||
	     (pDestroy->ppWidget == NULL) || (*pDestroy->ppWidget == NULL) ) return XUI_OK;
	pVictim = *pDestroy->ppWidget;
	*pDestroy->ppWidget = NULL;
	pDestroy->iCount++;
	xuiWidgetDestroy(pVictim);
	return XUI_OK;
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

	iRet = xuiInputPointerDown(pContext, (int)floorf(fX + 0.5f), (int)floorf(fY + 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputPointerUp(pContext, (int)floorf(fX + 0.5f), (int)floorf(fY + 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
}

static int __xuiDatePickerDispatchDown(xui_context pContext, float fX, float fY)
{
	int iRet;

	iRet = xuiInputPointerDown(pContext, (int)floorf(fX + 0.5f), (int)floorf(fY + 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
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

static int __xuiDatePickerInputWheel(xui_context pContext, xui_widget pPanel, xui_rect_t tRect, float fWheelY)
{
	xui_rect_t tWorld;
	int iRet;

	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = xuiInputPointerWheel(pContext,
		(int)floorf(tWorld.fX + tRect.fX + tRect.fW * 0.5f + 0.5f),
		(int)floorf(tWorld.fY + tRect.fY + tRect.fH * 0.5f + 0.5f),
		0.0f, fWheelY, 0);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pContext);
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
	fYearW = 112.0f;
	fMonthW = 62.0f;
	fGap = 6.0f;
	fComboW = fYearW + fMonthW + fGap;
	if ( fComboW > tPanel.fW - fButtonW * 2.0f - 12.0f ) {
		fComboW = tPanel.fW - fButtonW * 2.0f - 12.0f;
		if ( fComboW < 104.0f ) fComboW = 104.0f;
		fYearW = (fComboW - fGap) * 0.64f;
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
	xui_date_picker_destroy_test_t tDestroy;
	xui_date_picker_event_destroy_test_t tFocusDestroy;
	xui_context pContext;
	xui_widget pRoot;
	xui_widget pPicker;
	xui_widget pInvalid;
	xui_widget pDestroyPicker;
	xui_widget pFocusDestroyPicker;
	xui_widget pFocusDestroyPanel;
	xui_widget pSpanPicker;
	xui_widget pPopup;
	xui_widget pPanel;
	xui_surface pTarget;
	xui_surface pOwnerCache;
	xui_font pFont;
	xui_date_picker_desc_t tDesc;
	xui_accessible_node_t tAccessible;
	xui_accessible_value_t tAccessibleValue;
	xui_event_t tEvent;
	xui_rect_t tRect;
	xui_rect_t tYearRect;
	xui_rect_t tMonthRect;
	xui_rect_t tTimeRect;
	xui_rect_t tWorld;
	xui_vec2_t tMeasured;
	xtime tStart;
	xtime tEnd;
	xtime tTarget;
	float fBorderWidth;
	float fPopupWidth;
	float fPopupHeight;
	float fCheckWidth;
	float fCheckHeight;
	int bHasMin;
	int bHasMax;
	int iChangingBefore;
	int iCommitBefore;
	char sLongFormat[80];
	char sLongSeparator[40];
	int iFailed;
	int iRet;
	int iIndex;

	pContext = NULL;
	pRoot = NULL;
	pPicker = NULL;
	pInvalid = NULL;
	pDestroyPicker = NULL;
	pFocusDestroyPicker = NULL;
	pFocusDestroyPanel = NULL;
	pSpanPicker = NULL;
	pTarget = NULL;
	pOwnerCache = NULL;
	pFont = NULL;
	iFailed = 0;
	memset(&tEvents, 0, sizeof(tEvents));
	memset(&tDestroy, 0, sizeof(tDestroy));
	memset(&tFocusDestroy, 0, sizeof(tFocusDestroy));
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
	tDesc.fPopupWidth = NAN;
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL,
		"reject NaN descriptor popup width and clear output");
	tDesc.fPopupWidth = 200.0f;
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject undersized descriptor popup");
	tDesc.fPopupWidth = 0.0f;
	tDesc.fBorderWidth = INFINITY;
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject infinite descriptor border");
	tDesc.fBorderWidth = XUI_LAYOUT_UNBOUNDED * 2.0f;
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject oversized finite descriptor border");
	tDesc.fBorderWidth = 0.0f;
	tDesc.fPopupWidth = XUI_LAYOUT_UNBOUNDED * 2.0f;
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject oversized finite descriptor popup");
	tDesc.fPopupWidth = 0.0f;
	tDesc.bHasMin = 1;
	tDesc.bHasMax = 1;
	tDesc.tMin = __xuiDatePickerTestDate(2026, 6, 1);
	tDesc.tMax = __xuiDatePickerTestDate(2026, 5, 1);
	pInvalid = pRoot;
	iRet = xuiDatePickerCreate(pContext, &pInvalid, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT && pInvalid == NULL, "reject reversed descriptor limits");

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
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 19), "date normalizes");
	XUI_TEST_CHECK(strcmp(xuiDatePickerGetText(pPicker), "2026-05-19") == 0, "date text");
	iRet = xuiDatePickerGetMetrics(pPicker, &fBorderWidth);
	XUI_TEST_CHECK(iRet == XUI_OK && fBorderWidth == 1.0f, "default metrics");
	iRet = xuiDatePickerGetPopupSize(pPicker, &fPopupWidth, &fPopupHeight);
	XUI_TEST_CHECK(iRet == XUI_OK, "get initial popup size");
	iRet = xuiDatePickerSetPopupSize(pPicker, NAN, fPopupHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject NaN runtime popup size");
	iRet = xuiDatePickerSetPopupSize(pPicker, 200.0f, fPopupHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject undersized runtime popup size");
	iRet = xuiDatePickerSetPopupSize(pPicker, fPopupWidth, 250.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject popup height that clips year options");
	iRet = xuiDatePickerSetMetrics(pPicker, INFINITY);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject infinite runtime border");
	iRet = xuiDatePickerSetPopupSize(pPicker, XUI_LAYOUT_UNBOUNDED * 2.0f, fPopupHeight);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject oversized finite runtime popup");
	iRet = xuiDatePickerSetMetrics(pPicker, XUI_LAYOUT_UNBOUNDED * 2.0f);
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject oversized finite runtime border");
	iRet = xuiDatePickerGetPopupSize(pPicker, &fCheckWidth, &fCheckHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fCheckWidth == fPopupWidth && fCheckHeight == fPopupHeight, "invalid popup size preserves state");
	pOwnerCache = xuiWidgetGetCacheSurface(pPicker, xuiWidgetGetStateId(pPicker));
	XUI_TEST_CHECK(pOwnerCache != NULL, "owner cache surface");
	XUI_TEST_CHECK(xuiTestSurfaceGetRectFillCount(pOwnerCache) >= 4, "owner cache draw count");
	tRect = xuiTestSurfaceGetLastRect(pOwnerCache);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 0.0f, 0.0f, 210.0f, 30.0f), "owner pixel border inside cache");
	tRect = xuiDatePickerGetTextRect(pPicker);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 8.0f, 0.0f, 166.0f, 30.0f), "owner text rect local");
	tRect = xuiDatePickerGetButtonRect(pPicker);
	XUI_TEST_CHECK(__xuiDatePickerRectEq(tRect, 180.0f, 0.0f, 30.0f, 30.0f), "owner button rect local");
	iRet = xuiWidgetMeasureContent(pPicker, (xui_vec2_t){60.0f, 12.0f}, &tMeasured);
	XUI_TEST_CHECK(iRet == XUI_OK && tMeasured.fX == 60.0f && tMeasured.fY == 12.0f,
		"content measure respects tight constraints");
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 7.0f, 3.0f}) == XUI_OK,
		"set tiny owner rect");
	tRect = xuiDatePickerGetButtonRect(pPicker);
	tTimeRect = xuiDatePickerGetTextRect(pPicker);
	XUI_TEST_CHECK(tRect.fX >= 0.0f && tRect.fY >= 0.0f &&
		tRect.fX + tRect.fW <= 7.0f && tRect.fY + tRect.fH <= 3.0f &&
		tTimeRect.fX >= 0.0f && tTimeRect.fY >= 0.0f &&
		tTimeRect.fX + tTimeRect.fW <= 7.0f && tTimeRect.fY + tTimeRect.fH <= 3.0f,
		"tiny owner geometry stays inside the widget");
	XUI_TEST_CHECK(xuiWidgetSetRect(pPicker, (xui_rect_t){36.0f, 32.0f, 210.0f, 30.0f}) == XUI_OK,
		"restore owner rect");
	XUI_TEST_CHECK(__xuiDatePickerSameValueSettersStayClean(pPicker),
		"same-value setters do not dirty owner or popup widgets");

	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	iRet = xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible);
	XUI_TEST_CHECK(iRet == XUI_OK && tAccessible.iRole == XUI_ACCESSIBLE_ROLE_COMBOBOX &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_READONLY) != 0u &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_COLLAPSED) != 0u &&
		strcmp(tAccessible.sValue, "2026-05-19") == 0,
		"date picker accessibility node");
	iRet = xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_EXPAND, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "accessibility expands date picker");
	memset(&tAccessible, 0, sizeof(tAccessible));
	tAccessible.iSize = sizeof(tAccessible);
	XUI_TEST_CHECK(xuiWidgetGetAccessibleNode(pPicker, 0, &tAccessible) == XUI_OK &&
		(tAccessible.iState & XUI_ACCESSIBLE_STATE_EXPANDED) != 0u,
		"accessibility reports expanded date picker");
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_COLLAPSE, NULL) == XUI_OK && !xuiDatePickerIsOpen(pPicker),
		"accessibility collapses date picker");
	memset(&tAccessibleValue, 0, sizeof(tAccessibleValue));
	tAccessibleValue.iSize = sizeof(tAccessibleValue);
	tAccessibleValue.iType = XUI_ACCESSIBLE_VALUE_TEXT;
	tAccessibleValue.sText = "2026-05-20";
	XUI_TEST_CHECK(xuiWidgetPerformAccessibleAction(pPicker, tAccessible.iId,
		XUI_ACCESSIBLE_ACTION_SET_VALUE, &tAccessibleValue) == XUI_ERROR_UNSUPPORTED,
		"date picker rejects untyped accessibility value parsing");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	pPopup = xuiDatePickerGetPopupWidget(pPicker);
	XUI_TEST_CHECK(pPanel != NULL && pPopup != NULL, "date picker popup widgets");
	iChangingBefore = tEvents.iChanging;
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pPanel;
	tEvent.iKey = XUI_KEY_RIGHT;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiDatePickerIsOpen(pPicker) &&
		tEvents.iChanging == iChangingBefore,
		"closed panel ignores directly targeted keyboard input");
	XUI_TEST_CHECK(xuiSetFocusWidget(pContext, pPicker) == XUI_OK, "focus owner before popup shortcuts");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiDatePickerIsOpen(pPicker),
		"F4 opens date picker popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, XUI_MOD_ALT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiDatePickerIsOpen(pPicker),
		"Alt+F4 is not consumed as a date picker shortcut");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_F4, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && !xuiDatePickerIsOpen(pPicker),
		"F4 closes date picker popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_DOWN, XUI_MOD_ALT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && xuiDatePickerIsOpen(pPicker),
		"Alt+Down opens date picker popup");
	XUI_TEST_CHECK(xuiInputKeyDown(pContext, XUI_KEY_UP, XUI_MOD_ALT) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK && !xuiDatePickerIsOpen(pPicker),
		"Alt+Up closes date picker popup");

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
	iRet = xuiDatePickerSetNullable(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "time nullable expands default popup");
	iRet = xuiDatePickerGetPopupSize(pPicker, &fCheckWidth, &fCheckHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fCheckWidth == 324.0f && fCheckHeight == 168.0f,
		"nullable time footer fits popup");
	iRet = xuiDatePickerSetNullable(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "time nullable off");
	iRet = xuiDatePickerGetPopupSize(pPicker, &fCheckWidth, &fCheckHeight);
	XUI_TEST_CHECK(iRet == XUI_OK && fCheckWidth == 268.0f && fCheckHeight == 168.0f,
		"noncustom popup returns to mode default");
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
	iChangingBefore = tEvents.iChanging;
	iRet = __xuiDatePickerInputWheel(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 0), 0.0f);
	XUI_TEST_CHECK(iRet == XUI_OK && tEvents.iChanging == iChangingBefore,
		"zero wheel delta leaves time draft unchanged");
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

	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestTime(10, 15, 30));
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp reset");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "time clamp open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp layout");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp hour click");
	iRet = __xuiDatePickerInputText(pContext, "66");
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp hour text");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp hour enter");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 1));
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp minute click");
	iRet = __xuiDatePickerInputText(pContext, "99");
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp minute text");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp minute enter");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 2));
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp second click");
	iRet = __xuiDatePickerInputText(pContext, "88");
	XUI_TEST_CHECK(iRet == XUI_OK, "time clamp second text");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "time clamp ok closes");
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestTime(23, 59, 59), "time edit clamps invalid values");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATETIME);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime mode");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestDateTime(2026, 5, 19, 15, 45, 30));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDateTime(2026, 5, 19, 15, 45, 30), "datetime value");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 3), "datetime time fields stay inside panel");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "datetime keyboard time open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime keyboard time layout");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetTimeRect(pPicker, 0, 0));
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime keyboard hour focus");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime keyboard hour edit commit");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_UP);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime keyboard hour increment");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker) &&
		xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDateTime(2026, 5, 19, 16, 45, 30),
		"active datetime time field consumes arrows before calendar");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATE_RANGE);
	XUI_TEST_CHECK(iRet == XUI_OK, "date range mode");
	iRet = xuiDatePickerSetRangeValue(pPicker, __xuiDatePickerTestDate(2026, 5, 20), __xuiDatePickerTestDate(2026, 5, 18));
	XUI_TEST_CHECK(iRet == XUI_OK, "date range set");
	iRet = xuiDatePickerGetRangeValue(pPicker, &tStart, &tEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && tStart == __xuiDatePickerTestDate(2026, 5, 18) && tEnd == __xuiDatePickerTestDate(2026, 5, 20), "date range swaps");

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
	memset(sLongFormat, 'a', 62u);
	memcpy(sLongFormat + 62, "\xe4\xb8\xad", 4u);
	iRet = xuiDatePickerSetFormat(pPicker, sLongFormat);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(xuiDatePickerGetFormat(pPicker)) == 62u,
		"truncated format preserves UTF-8 boundary");
	iRet = xuiDatePickerSetFormat(pPicker, "yyyy-mm-dd hh:nn");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore custom format");
	iRet = xuiDatePickerSetRangeSeparator(pPicker, " / ");
	XUI_TEST_CHECK(iRet == XUI_OK, "range separator");
	memset(sLongSeparator, 'b', 22u);
	memcpy(sLongSeparator + 22, "\xe4\xb8\xad", 4u);
	iRet = xuiDatePickerSetRangeSeparator(pPicker, sLongSeparator);
	XUI_TEST_CHECK(iRet == XUI_OK && strlen(xuiDatePickerGetRangeSeparator(pPicker)) == 22u,
		"truncated range separator preserves UTF-8 boundary");
	iRet = xuiDatePickerSetRangeSeparator(pPicker, " / ");
	XUI_TEST_CHECK(iRet == XUI_OK, "restore range separator");
	iRet = xuiDatePickerSetRangeValue(pPicker, __xuiDatePickerTestDateTime(2026, 5, 19, 9, 0, 22), __xuiDatePickerTestDateTime(2026, 5, 19, 13, 0, 44));
	XUI_TEST_CHECK(iRet == XUI_OK && strcmp(xuiDatePickerGetText(pPicker), "2026-05-19 09:00 / 2026-05-19 13:00") == 0, "datetime range text");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 2) && __xuiDatePickerTimeFieldsFit(pPicker, 1, 2), "datetime range fields stay inside panels");
	iRet = xuiDatePickerSetShowSecond(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "datetime range show second");
	XUI_TEST_CHECK(strcmp(xuiDatePickerGetFormat(pPicker), "yyyy-mm-dd hh:nn") == 0,
		"show second preserves custom format");
	XUI_TEST_CHECK(__xuiDatePickerTimeFieldsFit(pPicker, 0, 3) && __xuiDatePickerTimeFieldsFit(pPicker, 1, 3), "datetime range second fields stay inside panels");

	iRet = xuiDatePickerSetMode(pPicker, XUI_DATE_PICKER_MODE_DATE);
	XUI_TEST_CHECK(iRet == XUI_OK, "date mode for limits");
	iRet = xuiDatePickerSetFormat(pPicker, NULL);
	XUI_TEST_CHECK(iRet == XUI_OK, "default format");
	iRet = xuiDatePickerSetLimits(pPicker, __xuiDatePickerTestDate(2026, 5, 1), __xuiDatePickerTestDate(2026, 5, 31));
	XUI_TEST_CHECK(iRet == XUI_OK, "limits");
	iRet = xuiDatePickerSetLimits(pPicker, __xuiDatePickerTestDate(2026, 6, 1), __xuiDatePickerTestDate(2026, 5, 1));
	XUI_TEST_CHECK(iRet == XUI_ERROR_INVALID_ARGUMENT, "reject reversed runtime limits");
	iRet = xuiDatePickerGetLimitRange(pPicker, &bHasMin, &tStart, &bHasMax, &tEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && bHasMin && bHasMax &&
		tStart == __xuiDatePickerTestDate(2026, 5, 1) && tEnd == __xuiDatePickerTestDate(2026, 5, 31),
		"invalid limits preserve state");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestDate(2026, 6, 20));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 31), "limit clamps");

	iRet = xuiDatePickerSetNullable(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "nullable");
	iRet = xuiDatePickerClearValue(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerHasValue(pPicker) && strcmp(xuiDatePickerGetText(pPicker), "") == 0, "nullable clear");
	iRet = xuiDatePickerSetNullable(pPicker, 0);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerHasValue(pPicker), "nullable off ensures value");
	(void)xuiDatePickerClearLimits(pPicker);
	(void)xuiDatePickerSetValue(pPicker, __xuiDatePickerTestDate(2026, 5, 19));

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "open layout");
	iRet = __xuiDatePickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "open render");
	pPopup = xuiDatePickerGetPopupWidget(pPicker);
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	XUI_TEST_CHECK(pPopup != NULL && pPanel != NULL && xuiGetFocusWidget(pContext) == pPanel, "popup focus");
	XUI_TEST_CHECK(xuiWidgetGetImeMode(pPicker) == XUI_IME_DISABLED &&
		xuiWidgetGetImeMode(pPanel) == XUI_IME_DISABLED, "numeric date fields keep IME disabled");
	XUI_TEST_CHECK((xuiDatePickerGetState(pPicker) & XUI_DATE_PICKER_STATE_OPEN) != 0u, "open state");
	tRect = xuiPopupGetPopupRect(pPopup);
	XUI_TEST_CHECK(tRect.fY >= 60.0f, "popup below owner");
	tRect = xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_OK);
	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = xuiInputPointerDownEx(pContext, 801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK &&
		xuiGetPointerCaptureEx(pContext, 801u, XUI_POINTER_TYPE_TOUCH) == pPanel,
		"date picker owner touch captures footer press");
	iRet = xuiInputPointerDownEx(pContext, 802u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 802u,
		XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + tRect.fX + tRect.fW * 0.5f),
		(int)(tWorld.fY + tRect.fY + tRect.fH * 0.5f),
		XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker) &&
		xuiGetPointerCaptureEx(pContext, 801u, XUI_POINTER_TYPE_TOUCH) == pPanel &&
		xuiGetPointerCaptureEx(pContext, 802u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"unrelated touch cannot commit footer or end owner capture");
	iRet = xuiInputPointerMoveEx(pContext, 801u, XUI_POINTER_TYPE_TOUCH,
		(int)(tWorld.fX + 1.0f), (int)(tWorld.fY + 1.0f),
		XUI_POINTER_BUTTON_LEFT);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	if ( iRet == XUI_OK ) iRet = xuiInputPointerUpEx(pContext, 801u,
		XUI_POINTER_TYPE_TOUCH, (int)(tWorld.fX + 1.0f),
		(int)(tWorld.fY + 1.0f), XUI_POINTER_BUTTON_LEFT, 0);
	if ( iRet == XUI_OK ) iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker) &&
		xuiGetPointerCaptureEx(pContext, 801u, XUI_POINTER_TYPE_TOUCH) == NULL,
		"date picker owner touch releases cancelled footer press");
	iRet = xuiInputPointerDown(pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f,
		tWorld.fY + tRect.fY + tRect.fH * 0.5f, XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer press input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiGetPointerCapture(pContext) == pPanel, "footer press captures pointer");
	iRet = xuiInputPointerMove(pContext, tWorld.fX + 1.0f, tWorld.fY + 1.0f, XUI_POINTER_BUTTON_LEFT);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer drag out input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer drag out dispatch");
	iRet = xuiInputPointerUp(pContext, tWorld.fX + 1.0f, tWorld.fY + 1.0f, XUI_POINTER_BUTTON_LEFT, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "footer release out input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker) && xuiGetPointerCapture(pContext) != pPanel,
		"footer drag out cancels activation");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "calendar right key moves draft");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker) &&
		 xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 20), "calendar keyboard enter commits draft");
	iRet = xuiDatePickerSetValue(pPicker, __xuiDatePickerTestDate(2026, 5, 19));
	XUI_TEST_CHECK(iRet == XUI_OK, "reset value after calendar keyboard test");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "reopen after calendar keyboard test");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "calendar keyboard reopen layout");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);

	__xuiDatePickerHeaderFieldRects(pPicker, 0, &tYearRect, &tMonthRect);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, tYearRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "year field click");
	iRet = __xuiDatePickerInputText(pContext, "2027");
	XUI_TEST_CHECK(iRet == XUI_OK, "year text input");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "year edit commit");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, tYearRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "six digit year field click");
	iRet = __xuiDatePickerInputText(pContext, "123456");
	XUI_TEST_CHECK(iRet == XUI_OK, "six digit year text input");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK, "six digit year edit commit");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, tMonthRect);
	XUI_TEST_CHECK(iRet == XUI_OK, "month field click");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, __xuiDatePickerMonthOptionRect(pPicker, 0, 11));
	XUI_TEST_CHECK(iRet == XUI_OK, "month option click");
	iIndex = __xuiDatePickerFindDay(pPicker, 0, __xuiDatePickerTestDate(123456, 12, 19));
	XUI_TEST_CHECK(iIndex >= 0, "six digit year month combo changes calendar view");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ESCAPE);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "combo view cancel closes");
	XUI_TEST_CHECK(xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 19), "combo view does not commit value");

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "reopen after combo view");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen combo layout");
	iRet = __xuiDatePickerRender(pContext, pTarget);
	XUI_TEST_CHECK(iRet == XUI_OK, "reopen combo render");
	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	tTarget = __xuiDatePickerTestDate(2026, 5, 20);
	iIndex = __xuiDatePickerFindDay(pPicker, 0, tTarget);
	XUI_TEST_CHECK(iIndex >= 0, "find target day");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetDayRect(pPicker, 0, iIndex));
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "day keeps popup open");
	XUI_TEST_CHECK(tEvents.iChanging > 0 && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 19), "draft only before ok");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetFooterRect(pPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "ok closes");
	XUI_TEST_CHECK(tEvents.iCommit > 0 && tEvents.iChange > 0 && xuiDatePickerGetValue(pPicker) == tTarget, "ok commits");

	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pPicker), "reopen");
	tTarget = __xuiDatePickerTestDate(2026, 5, 21);
	iIndex = __xuiDatePickerFindDay(pPicker, 0, tTarget);
	XUI_TEST_CHECK(iIndex >= 0, "find cancel day");
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel, xuiDatePickerGetDayRect(pPicker, 0, iIndex));
	XUI_TEST_CHECK(iRet == XUI_OK, "cancel draft day");
	iRet = xuiInputKeyDown(pContext, XUI_KEY_ESCAPE, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "escape input");
	iRet = xuiDispatchPendingEvents(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker), "escape closes");
	XUI_TEST_CHECK(tEvents.iCancel > 0 && xuiDatePickerGetValue(pPicker) == __xuiDatePickerTestDate(2026, 5, 20), "escape cancels");

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
	iRet = xuiWidgetSetEnabled(pPicker, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "re-enable picker");
	iRet = xuiWidgetSetEnabled(pRoot, 0);
	XUI_TEST_CHECK(iRet == XUI_OK, "disable parent");
	iRet = xuiDatePickerOpen(pPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pPicker) &&
		(xuiDatePickerGetState(pPicker) & XUI_WIDGET_STATE_DISABLED) != 0u, "effective parent disable blocks open");
	iRet = xuiWidgetSetEnabled(pRoot, 1);
	XUI_TEST_CHECK(iRet == XUI_OK, "enable parent");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pPicker, 0) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK, "hide date picker");
	memset(&tEvent, 0, sizeof(tEvent));
	tEvent.iSize = sizeof(tEvent);
	tEvent.iType = XUI_EVENT_KEY_DOWN;
	tEvent.pTarget = pPicker;
	tEvent.iKey = XUI_KEY_F4;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiDatePickerIsOpen(pPicker),
		"hidden date picker ignores directly targeted key input");
	tEvent.iType = XUI_EVENT_POINTER_DOWN;
	tEvent.iButton = XUI_POINTER_BUTTON_LEFT;
	tEvent.fX = 40.0f;
	tEvent.fY = 36.0f;
	XUI_TEST_CHECK(xuiDispatchEvent(pContext, &tEvent) == XUI_OK && !xuiDatePickerIsOpen(pPicker),
		"hidden date picker ignores directly targeted pointer input");
	XUI_TEST_CHECK(xuiWidgetSetVisible(pPicker, 1) == XUI_OK &&
		xuiDispatchPendingEvents(pContext) == XUI_OK, "restore date picker visibility");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_DATE_PICKER_MODE_DATETIME_RANGE;
	tDesc.bNullable = 1;
	tDesc.tDefaultRangeSpan = INT64_MAX;
	tDesc.pFont = pFont;
	iRet = xuiDatePickerCreate(pContext, &pSpanPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pSpanPicker != NULL, "overflow span picker create");
	XUI_TEST_CHECK(!xuiDatePickerHasValue(pSpanPicker),
		"nullable descriptor without an explicit value starts empty");
	xuiWidgetSetRect(pSpanPicker, (xui_rect_t){500.0f, 32.0f, 210.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pSpanPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow span picker add");
	iRet = xuiDatePickerOpen(pSpanPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pSpanPicker), "overflow span picker open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "overflow span picker layout");
	XUI_TEST_CHECK(!xuiDatePickerHasValue(pSpanPicker) &&
		__xuiDatePickerFindDay(pSpanPicker, 0, xrtNow()) >= 0,
		"empty nullable picker shows its current-date draft without committing it");
	pPanel = xuiDatePickerGetPanelWidget(pSpanPicker);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel,
		xuiDatePickerGetFooterRect(pSpanPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && !xuiDatePickerIsOpen(pSpanPicker), "overflow span picker commit");
	iRet = xuiDatePickerGetRangeValue(pSpanPicker, &tStart, &tEnd);
	XUI_TEST_CHECK(iRet == XUI_OK && tStart == tEnd, "overflow default span degrades to a valid zero span");

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = XUI_DATE_PICKER_MODE_DATE;
	tDesc.bHasValue = 1;
	tDesc.tValue = __xuiDatePickerTestDate(2026, 8, 12);
	tDesc.pFont = pFont;
	iRet = xuiDatePickerCreate(pContext, &pDestroyPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyPicker != NULL, "destroy callback picker create");
	xuiWidgetSetRect(pDestroyPicker, (xui_rect_t){280.0f, 32.0f, 210.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pDestroyPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback picker add");
	tDestroy.ppWidget = &pDestroyPicker;
	iRet = xuiDatePickerSetChange(pDestroyPicker, __xuiDatePickerDestroyOnCommit, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy change callback set");
	iCommitBefore = tEvents.iCommit;
	iRet = xuiDatePickerSetCommit(pDestroyPicker, __xuiDatePickerCommit, &tEvents);
	XUI_TEST_CHECK(iRet == XUI_OK, "commit callback after destroy change set");
	iRet = xuiDatePickerOpen(pDestroyPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pDestroyPicker), "destroy change picker open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy change layout");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_RIGHT);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy change moves draft");
	iRet = __xuiDatePickerInputKey(pContext, XUI_KEY_ENTER);
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 && tDestroy.bSawClosed &&
		pDestroyPicker == NULL && tEvents.iCommit == iCommitBefore,
		"change callback may destroy picker and suppresses later commit callback");

	memset(&tDestroy, 0, sizeof(tDestroy));
	iRet = xuiDatePickerCreate(pContext, &pDestroyPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pDestroyPicker != NULL, "commit destroy picker create");
	xuiWidgetSetRect(pDestroyPicker, (xui_rect_t){280.0f, 72.0f, 210.0f, 30.0f});
	iRet = xuiWidgetAddChild(pRoot, pDestroyPicker);
	XUI_TEST_CHECK(iRet == XUI_OK, "commit destroy picker add");
	tDestroy.ppWidget = &pDestroyPicker;
	iRet = xuiDatePickerSetCommit(pDestroyPicker, __xuiDatePickerDestroyOnCommit, &tDestroy);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy commit callback set");
	iRet = xuiDatePickerOpen(pDestroyPicker);
	XUI_TEST_CHECK(iRet == XUI_OK && xuiDatePickerIsOpen(pDestroyPicker), "destroy callback picker open");
	iRet = xuiLayout(pContext);
	XUI_TEST_CHECK(iRet == XUI_OK, "destroy callback layout");
	pPanel = xuiDatePickerGetPanelWidget(pDestroyPicker);
	iRet = __xuiDatePickerClickPanelRect(pContext, pPanel,
		xuiDatePickerGetFooterRect(pDestroyPicker, XUI_DATE_PICKER_FOOTER_OK));
	XUI_TEST_CHECK(iRet == XUI_OK && tDestroy.iCount == 1 && tDestroy.bSawClosed && pDestroyPicker == NULL,
		"commit callback observes closed popup and may destroy picker");

	iRet = xuiDatePickerCreate(pContext, &pFocusDestroyPicker, &tDesc);
	XUI_TEST_CHECK(iRet == XUI_OK && pFocusDestroyPicker != NULL,
		"focus destroy picker create");
	XUI_TEST_CHECK(xuiWidgetSetRect(pFocusDestroyPicker,
		(xui_rect_t){500.0f, 72.0f, 210.0f, 30.0f}) == XUI_OK &&
		xuiWidgetAddChild(pRoot, pFocusDestroyPicker) == XUI_OK,
		"focus destroy picker add");
	pFocusDestroyPanel = xuiDatePickerGetPanelWidget(pFocusDestroyPicker);
	tFocusDestroy.ppWidget = &pFocusDestroyPicker;
	tFocusDestroy.iEventType = XUI_EVENT_FOCUS;
	XUI_TEST_CHECK(pFocusDestroyPanel != NULL &&
		xuiWidgetSetEventHandler(pFocusDestroyPanel, XUI_EVENT_FOCUS,
			__xuiDatePickerDestroyVictimOnEvent, &tFocusDestroy) == XUI_OK,
		"install date picker focus destroy callback");
	XUI_TEST_CHECK(xuiLayout(pContext) == XUI_OK && xuiSetFocusWidget(pContext, NULL) == XUI_OK,
		"prepare date picker focus destroy callback");
	tWorld = xuiWidgetGetWorldRect(pFocusDestroyPicker);
	iRet = __xuiDatePickerDispatchDown(pContext,
		tWorld.fX + tWorld.fW * 0.5f, tWorld.fY + tWorld.fH * 0.5f);
	XUI_TEST_CHECK(iRet == XUI_OK, "focus destroy pointer dispatch returns safely");
	XUI_TEST_CHECK(tFocusDestroy.iCount == 1 && pFocusDestroyPicker == NULL,
		"focus callback destroys date picker exactly once");

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
