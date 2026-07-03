#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct app_state_t {
	xge_scene_t tScene;
	xge_xui_context_t tXui;
	xge_xui_page_t tPage;
	xge_font_t tFont;
	char sXson[8192];
	xge_xui_date_picker pDate;
	xge_xui_date_picker pTime;
	xge_xui_date_picker pTimeSeconds;
	xge_xui_date_picker pDateTime;
	xge_xui_date_picker pDateRange;
	xge_xui_date_picker pTimeRange;
	xge_xui_date_picker pTimeRangeSeconds;
	xge_xui_date_picker pNullable;
	xge_xui_widget pDateTimeWidget;
	int bFontReady;
	int iFrameLimit;
	int iFrameCount;
	int bCreateOK;
	int bModeOK;
	int bValueOK;
	int bRangeOK;
	int bPopupOK;
	int iCommitCount;
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
			printf("xui_datepicker_xson font loaded: %s\n", arrFonts[i]);
			return XGE_OK;
		}
	}
	printf("xui_datepicker_xson font load failed\n");
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

static void BuildXson(app_state_t* pApp)
{
	xtime tDate;
	xtime tTime;
	xtime tTimeSeconds;
	xtime tDateTime;
	xtime tDateStart;
	xtime tDateEnd;
	xtime tTimeStart;
	xtime tTimeEnd;
	xtime tTimeSecondsStart;
	xtime tTimeSecondsEnd;

	tDate = DemoDateSerial(2026, 5, 19);
	tTime = DemoTimeSerial(9, 30, 45);
	tTimeSeconds = DemoTimeSerial(10, 15, 30);
	tDateTime = DemoDateTimeSerial(2026, 5, 19, 15, 45, 30);
	tDateStart = DemoDateSerial(2026, 5, 18);
	tDateEnd = DemoDateSerial(2026, 5, 20);
	tTimeStart = DemoTimeSerial(9, 0, 12);
	tTimeEnd = DemoTimeSerial(13, 0, 59);
	tTimeSecondsStart = DemoTimeSerial(10, 20, 30);
	tTimeSecondsEnd = DemoTimeSerial(11, 40, 50);
	snprintf(
		pApp->sXson,
		sizeof(pApp->sXson),
		"{\"xui\":1,"
		"\"styles\":{"
		"\"root\":{\"type\":\"column\",\"width\":\"100%%\",\"height\":\"100%%\",\"padding\":[28,30,28,30],\"gap\":14,\"background\":\"#E8F0F8FF\"},"
		"\"title\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"height\":28,\"textColor\":\"#26384AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
		"\"grid\":{\"type\":\"grid\",\"width\":\"100%%\",\"height\":352,\"columns\":2,\"rowHeight\":30,\"columnGap\":24,\"rowGap\":16},"
		"\"label\":{\"type\":\"label\",\"font\":\"@fonts.body\",\"width\":118,\"height\":30,\"textColor\":\"#52647AFF\",\"textAlign\":\"left\",\"textVAlign\":\"middle\"},"
		"\"picker\":{\"type\":\"datePicker\",\"font\":\"@fonts.body\",\"width\":260,\"height\":30,\"backgroundColor\":\"#F8FCFFFF\",\"fieldColor\":\"#F6FBFFFF\",\"borderColor\":\"#7EA6C8FF\",\"popupColor\":\"#FFFFFFFF\",\"panelColor\":\"#F8FBFEFF\",\"headerColor\":\"#E8F3FCFF\",\"textColor\":\"#22384CFF\",\"selectedColor\":\"#2D7DD2FF\"}"
		"},"
		"\"tree\":{\"type\":\"column\",\"id\":\"root\",\"style\":\"root\",\"children\":["
		"{\"type\":\"label\",\"id\":\"title\",\"style\":\"title\",\"text\":\"DatePicker XSON\"},"
		"{\"type\":\"grid\",\"id\":\"grid\",\"style\":\"grid\",\"children\":["
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"date xtime\"},"
		"{\"type\":\"datePicker\",\"id\":\"date\",\"style\":\"picker\",\"mode\":\"date\",\"value\":%lld},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"time no seconds\"},"
		"{\"type\":\"datePicker\",\"id\":\"time\",\"style\":\"picker\",\"mode\":\"time\",\"value\":%lld,\"showSecond\":false},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"time seconds\"},"
		"{\"type\":\"datePicker\",\"id\":\"timeSeconds\",\"style\":\"picker\",\"mode\":\"time\",\"value\":%lld,\"showSecond\":true},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"date time\"},"
		"{\"type\":\"datePicker\",\"id\":\"dateTime\",\"style\":\"picker\",\"mode\":\"dateTime\",\"value\":%lld,\"showSecond\":true},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"date range\"},"
		"{\"type\":\"datePicker\",\"id\":\"dateRange\",\"style\":\"picker\",\"mode\":\"dateRange\",\"startValue\":%lld,\"endValue\":%lld,\"rangeSeparator\":\" - \",\"firstDayOfWeek\":1},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"time range\"},"
		"{\"type\":\"datePicker\",\"id\":\"timeRange\",\"style\":\"picker\",\"mode\":\"timeRange\",\"startValue\":%lld,\"endValue\":%lld,\"showSecond\":false,\"defaultRangeSpan\":14400},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"time range seconds\"},"
		"{\"type\":\"datePicker\",\"id\":\"timeRangeSeconds\",\"style\":\"picker\",\"mode\":\"timeRange\",\"startValue\":%lld,\"endValue\":%lld,\"showSecond\":true,\"defaultRangeSpan\":14400},"
		"{\"type\":\"label\",\"style\":\"label\",\"text\":\"nullable\"},"
		"{\"type\":\"datePicker\",\"id\":\"nullable\",\"style\":\"picker\",\"mode\":\"date\",\"nullable\":true}"
		"]}]}}",
		(long long)tDate,
		(long long)tTime,
		(long long)tTimeSeconds,
		(long long)tDateTime,
		(long long)tDateStart,
		(long long)tDateEnd,
		(long long)tTimeStart,
		(long long)tTimeEnd,
		(long long)tTimeSecondsStart,
		(long long)tTimeSecondsEnd);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pWidget;

	XgeXuiDemoApplyTheme(&pApp->tXui, pApp->bFontReady ? &pApp->tFont : NULL);
	if ( pApp->bFontReady ) {
		xgeXuiTokenSetFont(&pApp->tXui, "body", &pApp->tFont);
	}
	BuildXson(pApp);
	if ( xgeXuiPageLoadMemory(&pApp->tXui, pApp->sXson, (int)strlen(pApp->sXson), NULL, &pApp->tPage) != XGE_OK ) {
		printf("xui_datepicker_xson load failed: %s\n", xgeXuiPageGetError(&pApp->tPage));
		return XGE_ERROR;
	}
	pWidget = xgeXuiPageFind(&pApp->tPage, "date");
	pApp->pDate = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "time");
	pApp->pTime = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "timeSeconds");
	pApp->pTimeSeconds = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pApp->pDateTimeWidget = xgeXuiPageFind(&pApp->tPage, "dateTime");
	pApp->pDateTime = (pApp->pDateTimeWidget != NULL) ? (xge_xui_date_picker)pApp->pDateTimeWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "dateRange");
	pApp->pDateRange = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "timeRange");
	pApp->pTimeRange = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "timeRangeSeconds");
	pApp->pTimeRangeSeconds = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	pWidget = xgeXuiPageFind(&pApp->tPage, "nullable");
	pApp->pNullable = (pWidget != NULL) ? (xge_xui_date_picker)pWidget->pUser : NULL;
	if ( (pApp->pDate == NULL) || (pApp->pTime == NULL) || (pApp->pTimeSeconds == NULL) || (pApp->pDateTime == NULL) || (pApp->pDateRange == NULL) || (pApp->pTimeRange == NULL) || (pApp->pTimeRangeSeconds == NULL) || (pApp->pNullable == NULL) ) {
		return XGE_ERROR;
	}
	xgeXuiDatePickerSetCommit(pApp->pDateTime, OnCommit, pApp);
	pApp->bCreateOK = 1;
	return XGE_OK;
}

static void RunChecks(app_state_t* pApp)
{
	xge_event_t tEvent;
	xtime tStart;
	xtime tEnd;
	float fX;
	float fY;
	int iOpen;
	int iClosed;

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	pApp->bModeOK =
		xgeXuiDatePickerGetMode(pApp->pDate) == XGE_XUI_DATE_PICKER_MODE_DATE &&
		xgeXuiDatePickerGetMode(pApp->pTime) == XGE_XUI_DATE_PICKER_MODE_TIME &&
		xgeXuiDatePickerGetMode(pApp->pTimeSeconds) == XGE_XUI_DATE_PICKER_MODE_TIME &&
		xgeXuiDatePickerGetMode(pApp->pDateTime) == XGE_XUI_DATE_PICKER_MODE_DATETIME &&
		xgeXuiDatePickerGetMode(pApp->pDateRange) == XGE_XUI_DATE_PICKER_MODE_DATE_RANGE &&
		xgeXuiDatePickerGetMode(pApp->pTimeRange) == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE &&
		xgeXuiDatePickerGetMode(pApp->pTimeRangeSeconds) == XGE_XUI_DATE_PICKER_MODE_TIME_RANGE &&
		xgeXuiDatePickerGetNullable(pApp->pNullable) &&
		!xgeXuiDatePickerHasValue(pApp->pNullable) &&
		pApp->pDateRange->iFirstDayOfWeek == 1;
	pApp->bValueOK =
		xgeXuiDatePickerGetValue(pApp->pDate) == DemoDateSerial(2026, 5, 19) &&
		xgeXuiDatePickerGetValue(pApp->pTime) == DemoTimeSerial(9, 30, 0) &&
		xgeXuiDatePickerGetValue(pApp->pTimeSeconds) == DemoTimeSerial(10, 15, 30) &&
		xgeXuiDatePickerGetValue(pApp->pDateTime) == DemoDateTimeSerial(2026, 5, 19, 15, 45, 30);
	xgeXuiDatePickerGetRangeValue(pApp->pDateRange, &tStart, &tEnd);
	pApp->bRangeOK = (tStart == DemoDateSerial(2026, 5, 18)) && (tEnd == DemoDateSerial(2026, 5, 20));
	xgeXuiDatePickerGetRangeValue(pApp->pTimeRange, &tStart, &tEnd);
	pApp->bRangeOK = pApp->bRangeOK && (tStart == DemoTimeSerial(9, 0, 0)) && (tEnd == DemoTimeSerial(13, 0, 0));
	xgeXuiDatePickerGetRangeValue(pApp->pTimeRangeSeconds, &tStart, &tEnd);
	pApp->bRangeOK = pApp->bRangeOK && (tStart == DemoTimeSerial(10, 20, 30)) && (tEnd == DemoTimeSerial(11, 40, 50));

	fX = pApp->pDateTimeWidget->tRect.fX + 12.0f;
	fY = pApp->pDateTimeWidget->tRect.fY + 12.0f;
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, fX, fY);
	xgeXuiDispatchEvent(&pApp->tXui, &tEvent);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiPaint(&pApp->tXui);
	iOpen = xgeXuiDatePickerIsPopupOpen(pApp->pDateTime);
	fX = pApp->pDateTime->arrFooterRect[3].fX + pApp->pDateTime->arrFooterRect[3].fW * 0.5f;
	fY = pApp->pDateTime->arrFooterRect[3].fY + pApp->pDateTime->arrFooterRect[3].fH * 0.5f;
	ClickMouse(pApp, fX, fY);
	iClosed = !xgeXuiDatePickerIsPopupOpen(pApp->pDateTime);
	pApp->bPopupOK = iOpen && iClosed && (pApp->iCommitCount == 1);
}

static int AppEnter(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	LoadFont(pApp);
	if ( xgeXuiInit(&pApp->tXui) != XGE_OK || CreateUI(pApp) != XGE_OK ) {
		return XGE_ERROR;
	}
	if ( pApp->iFrameLimit > 0 ) {
		RunChecks(pApp);
	}
	return XGE_OK;
}

static int AppLeave(xge_scene pScene)
{
	app_state_t* pApp;

	pApp = (app_state_t*)pScene->pUser;
	xgeXuiPageUnload(&pApp->tPage);
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
			"xui_datepicker_xson final-summary frames=%d create=%d mode=%d value=%d range=%d popup=%d commit=%d\n",
			pApp->iFrameCount,
			pApp->bCreateOK,
			pApp->bModeOK,
			pApp->bValueOK,
			pApp->bRangeOK,
			pApp->bPopupOK,
			pApp->iCommitCount);
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
	static app_state_t tApp;
	xge_desc_t tDesc;
	int i;
	int iExitCode;

	memset(&tDesc, 0, sizeof(tDesc));
	memset(&tApp, 0, sizeof(tApp));
	tApp.iFrameLimit = ArgInt(getenv("XGE_XUI_DATEPICKER_XSON_FRAMES"), 0);
	for ( i = 1; i < argc; i++ ) {
		if ( (strcmp(argv[i], "--frames") == 0) && ((i + 1) < argc) ) {
			tApp.iFrameLimit = ArgInt(argv[++i], tApp.iFrameLimit);
		}
	}
	tDesc.iWidth = 760;
	tDesc.iHeight = 520;
	tDesc.sTitle = "XUI DatePicker XSON";
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
	return (iExitCode == XGE_OK && tApp.bCreateOK && tApp.bModeOK && tApp.bValueOK && tApp.bRangeOK && tApp.bPopupOK) ? 0 : 3;
}
