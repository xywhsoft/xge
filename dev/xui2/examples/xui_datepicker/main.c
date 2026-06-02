#include "xui.h"
#include "xge.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEMO_TARGET_W	900
#define DEMO_TARGET_H	520
#define DEMO_OFFSET_X	10.0f
#define DEMO_OFFSET_Y	20.0f
#define PICKER_COUNT	9
#define LABEL_COUNT	12

#ifndef XGE_KEY_LEFT_SHIFT
#define XGE_KEY_LEFT_SHIFT 340
#define XGE_KEY_LEFT_CONTROL 341
#define XGE_KEY_LEFT_ALT 342
#define XGE_KEY_RIGHT_SHIFT 344
#define XGE_KEY_RIGHT_CONTROL 345
#define XGE_KEY_RIGHT_ALT 346
#endif

enum {
	DP_DATE = 0,
	DP_TIME,
	DP_DATETIME,
	DP_DATE_RANGE,
	DP_TIME_RANGE,
	DP_DATETIME_RANGE,
	DP_NULLABLE,
	DP_LIMITED,
	DP_DISABLED
};

typedef struct xui_datepicker_demo_t {
	xui_context pContext;
	xui_proxy_t tProxy;
	xui_surface pTarget;
	xui_font pFont;
	xui_widget pRoot;
	xui_widget pLabel[LABEL_COUNT];
	xui_widget pPicker[PICKER_COUNT];
	xui_widget pStatus;
	int iFrame;
	int iMaxFrames;
	double fMaxSeconds;
	int bHasMouse;
	float fLastMouseX;
	float fLastMouseY;
	float fUiMouseX;
	float fUiMouseY;
	uint32_t iLastButtons;
	int bExerciseDone;
	int bCreateOK;
	int bLayoutOK;
	int bModeOK;
	int bValueOK;
	int bRangeOK;
	int bLimitOK;
	int bPopupOK;
	int bClearOK;
	int bDisabledOK;
	int iChangingCount;
	int iChangeCount;
	int iCommitCount;
	int iCancelCount;
	int iClearCount;
} xui_datepicker_demo_t;

static xtime __xuiDatePickerDemoTime(int iHour, int iMinute, int iSecond)
{
	return (xtime)iHour * XRT_TIME_HOUR + (xtime)iMinute * XRT_TIME_MINUTE + (xtime)iSecond;
}

static xtime __xuiDatePickerDemoDateTime(int64 iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
{
	return xrtDateSerial(iYear, iMonth, iDay) + __xuiDatePickerDemoTime(iHour, iMinute, iSecond);
}

static void __xuiDatePickerUsage(void)
{
	printf("usage: xui_datepicker [--frames N] [--seconds N]\n");
	printf("       no duration option means run until the window is closed.\n");
}

static int __xuiDatePickerParseArgs(xui_datepicker_demo_t* pDemo, int argc, char** argv)
{
	int i;

	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->iMaxFrames = atoi(argv[++i]);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--frames=", 9) == 0 ) {
			pDemo->iMaxFrames = atoi(argv[i] + 9);
			if ( pDemo->iMaxFrames <= 0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--seconds") == 0 ) {
			if ( i + 1 >= argc ) return XGE_ERROR_INVALID_ARGUMENT;
			pDemo->fMaxSeconds = atof(argv[++i]);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strncmp(argv[i], "--seconds=", 10) == 0 ) {
			pDemo->fMaxSeconds = atof(argv[i] + 10);
			if ( pDemo->fMaxSeconds <= 0.0 ) return XGE_ERROR_INVALID_ARGUMENT;
		} else if ( strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0 ) {
			__xuiDatePickerUsage();
			return 1;
		} else {
			return XGE_ERROR_INVALID_ARGUMENT;
		}
	}
	return XGE_OK;
}

static const char* __xuiDatePickerFindTtf(void)
{
	static const char* arrFont[] = {
		"C:/Windows/Fonts/msyh.ttc",
		"C:/Windows/Fonts/simsun.ttc",
		"C:/Windows/Fonts/arial.ttf"
	};
	FILE* pFile;
	int i;

	for ( i = 0; i < (int)(sizeof(arrFont) / sizeof(arrFont[0])); i++ ) {
		pFile = fopen(arrFont[i], "rb");
		if ( pFile != NULL ) {
			fclose(pFile);
			return arrFont[i];
		}
	}
	return NULL;
}

static int __xuiDatePickerRootRender(xui_widget pWidget, xui_draw_context pDraw, uint32_t iStateId, void* pUser)
{
	xui_datepicker_demo_t* pDemo;
	xui_rect_t tRect;
	xui_rect_t tPanel;

	(void)iStateId;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( (pWidget == NULL) || (pDraw == NULL) || (pDemo == NULL) ) return XUI_ERROR_INVALID_ARGUMENT;
	tRect = xuiWidgetGetContentRect(pWidget);
	if ( pDemo->tProxy.drawRectFill != NULL ) {
		(void)pDemo->tProxy.drawRectFill(&pDemo->tProxy, pDraw, tRect, XUI_COLOR_RGBA(234, 242, 250, 255));
	}
	if ( pDemo->tProxy.drawRoundRectFill != NULL ) {
		tPanel = (xui_rect_t){28.0f, 22.0f, 842.0f, 440.0f};
		(void)pDemo->tProxy.drawRoundRectFill(&pDemo->tProxy, pDraw, tPanel, 6.0f, XUI_COLOR_RGBA(248, 251, 255, 255));
	}
	return XUI_OK;
}

static void __xuiDatePickerOnChanging(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_datepicker_demo_t* pDemo;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iChangingCount++;
}

static void __xuiDatePickerOnChange(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_datepicker_demo_t* pDemo;
	char sText[160];

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo == NULL ) return;
	pDemo->iChangeCount++;
	if ( pDemo->pStatus != NULL ) {
		snprintf(sText, sizeof(sText), "changes=%d commits=%d cancels=%d clears=%d", pDemo->iChangeCount, pDemo->iCommitCount, pDemo->iCancelCount, pDemo->iClearCount);
		(void)xuiLabelSetText(pDemo->pStatus, sText);
	}
}

static void __xuiDatePickerOnCommit(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_datepicker_demo_t* pDemo;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iCommitCount++;
}

static void __xuiDatePickerOnCancel(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_datepicker_demo_t* pDemo;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iCancelCount++;
}

static void __xuiDatePickerOnClear(xui_widget pWidget, xtime tStart, xtime tEnd, int iMode, void* pUser)
{
	xui_datepicker_demo_t* pDemo;

	(void)pWidget;
	(void)tStart;
	(void)tEnd;
	(void)iMode;
	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo != NULL ) pDemo->iClearCount++;
}

static int __xuiDatePickerAddLabel(xui_datepicker_demo_t* pDemo, int iIndex, const char* sText, xui_rect_t tRect)
{
	xui_label_desc_t tDesc;
	xui_widget pLabel;
	int iRet;

	if ( (pDemo == NULL) || (iIndex < 0) || (iIndex >= LABEL_COUNT) ) return XUI_ERROR_INVALID_ARGUMENT;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.pFont = pDemo->pFont;
	tDesc.sText = sText;
	tDesc.iTextColor = XUI_COLOR_RGBA(54, 68, 88, 255);
	tDesc.iTextFlags = XUI_TEXT_ALIGN_LEFT | XUI_TEXT_ALIGN_MIDDLE | XUI_TEXT_CLIP;
	iRet = xuiLabelCreate(pDemo->pContext, &pLabel, &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pLabel, tRect);
	iRet = xuiWidgetAddChild(pDemo->pRoot, pLabel);
	if ( iRet != XUI_OK ) {
		xuiWidgetDestroy(pLabel);
		return iRet;
	}
	pDemo->pLabel[iIndex] = pLabel;
	return XUI_OK;
}

static int __xuiDatePickerCreatePicker(xui_datepicker_demo_t* pDemo, int iIndex, int iMode, xui_rect_t tRect)
{
	xui_date_picker_desc_t tDesc;
	int iRet;

	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iSize = sizeof(tDesc);
	tDesc.iMode = iMode;
	tDesc.bHasValue = 1;
	tDesc.tValue = __xuiDatePickerDemoDateTime(2026, 5, 19, 11, 30, 0);
	tDesc.pFont = pDemo->pFont;
	iRet = xuiDatePickerCreate(pDemo->pContext, &pDemo->pPicker[iIndex], &tDesc);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiDatePickerSetChanging(pDemo->pPicker[iIndex], __xuiDatePickerOnChanging, pDemo);
	(void)xuiDatePickerSetChange(pDemo->pPicker[iIndex], __xuiDatePickerOnChange, pDemo);
	(void)xuiDatePickerSetCommit(pDemo->pPicker[iIndex], __xuiDatePickerOnCommit, pDemo);
	(void)xuiDatePickerSetCancel(pDemo->pPicker[iIndex], __xuiDatePickerOnCancel, pDemo);
	(void)xuiDatePickerSetClear(pDemo->pPicker[iIndex], __xuiDatePickerOnClear, pDemo);
	xuiWidgetSetRect(pDemo->pPicker[iIndex], tRect);
	return xuiWidgetAddChild(pDemo->pRoot, pDemo->pPicker[iIndex]);
}

static int __xuiDatePickerCreateUi(xui_datepicker_demo_t* pDemo)
{
	int iRet;

	iRet = xuiWidgetCreate(pDemo->pContext, &pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;
	xuiWidgetSetRect(pDemo->pRoot, (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H});
	(void)xuiWidgetSetLayoutType(pDemo->pRoot, XUI_LAYOUT_MANUAL);
	(void)xuiWidgetSetCacheRenderCallback(pDemo->pRoot, __xuiDatePickerRootRender, pDemo);
	iRet = xuiSetRootWidget(pDemo->pContext, pDemo->pRoot);
	if ( iRet != XUI_OK ) return iRet;

	if ( __xuiDatePickerAddLabel(pDemo, 0, "XUI DatePicker", (xui_rect_t){48.0f, 34.0f, 220.0f, 24.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 1, "date", (xui_rect_t){48.0f, 82.0f, 110.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 2, "time", (xui_rect_t){48.0f, 130.0f, 110.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 3, "date time", (xui_rect_t){48.0f, 178.0f, 110.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 4, "date range", (xui_rect_t){452.0f, 82.0f, 120.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 5, "time range", (xui_rect_t){452.0f, 130.0f, 120.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 6, "date time range", (xui_rect_t){452.0f, 178.0f, 130.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 7, "nullable", (xui_rect_t){48.0f, 244.0f, 110.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 8, "limited", (xui_rect_t){452.0f, 244.0f, 120.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 9, "disabled", (xui_rect_t){48.0f, 292.0f, 110.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 10, "Open a picker, choose a day or time, then OK / Cancel.", (xui_rect_t){48.0f, 346.0f, 470.0f, 28.0f}) != XUI_OK ||
	     __xuiDatePickerAddLabel(pDemo, 11, "changes=0 commits=0 cancels=0 clears=0", (xui_rect_t){48.0f, 382.0f, 370.0f, 28.0f}) != XUI_OK ) {
		return XUI_ERROR;
	}
	pDemo->pStatus = pDemo->pLabel[11];

	iRet = __xuiDatePickerCreatePicker(pDemo, DP_DATE, XUI_DATE_PICKER_MODE_DATE, (xui_rect_t){170.0f, 82.0f, 240.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_TIME, XUI_DATE_PICKER_MODE_TIME, (xui_rect_t){170.0f, 130.0f, 240.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_DATETIME, XUI_DATE_PICKER_MODE_DATETIME, (xui_rect_t){170.0f, 178.0f, 240.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_DATE_RANGE, XUI_DATE_PICKER_MODE_DATE_RANGE, (xui_rect_t){590.0f, 82.0f, 250.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_TIME_RANGE, XUI_DATE_PICKER_MODE_TIME_RANGE, (xui_rect_t){590.0f, 130.0f, 250.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_DATETIME_RANGE, XUI_DATE_PICKER_MODE_DATETIME_RANGE, (xui_rect_t){590.0f, 178.0f, 250.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_NULLABLE, XUI_DATE_PICKER_MODE_DATE, (xui_rect_t){170.0f, 244.0f, 240.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_LIMITED, XUI_DATE_PICKER_MODE_DATE, (xui_rect_t){590.0f, 244.0f, 250.0f, 30.0f});
	if ( iRet == XUI_OK ) iRet = __xuiDatePickerCreatePicker(pDemo, DP_DISABLED, XUI_DATE_PICKER_MODE_DATETIME, (xui_rect_t){170.0f, 292.0f, 240.0f, 30.0f});
	if ( iRet != XUI_OK ) return iRet;

	(void)xuiDatePickerSetShowSecond(pDemo->pPicker[DP_TIME], 0);
	(void)xuiDatePickerSetValue(pDemo->pPicker[DP_TIME], __xuiDatePickerDemoTime(9, 30, 45));
	(void)xuiDatePickerSetShowSecond(pDemo->pPicker[DP_DATETIME], 1);
	(void)xuiDatePickerSetValue(pDemo->pPicker[DP_DATETIME], __xuiDatePickerDemoDateTime(2026, 5, 19, 15, 45, 30));
	(void)xuiDatePickerSetRangeValue(pDemo->pPicker[DP_DATE_RANGE], xrtDateSerial(2026, 5, 20), xrtDateSerial(2026, 5, 18));
	(void)xuiDatePickerSetFirstDayOfWeek(pDemo->pPicker[DP_DATE_RANGE], 1);
	(void)xuiDatePickerSetShowSecond(pDemo->pPicker[DP_TIME_RANGE], 0);
	(void)xuiDatePickerSetRangeValue(pDemo->pPicker[DP_TIME_RANGE], __xuiDatePickerDemoTime(13, 0, 59), __xuiDatePickerDemoTime(9, 0, 12));
	(void)xuiDatePickerSetShowSecond(pDemo->pPicker[DP_DATETIME_RANGE], 0);
	(void)xuiDatePickerSetFormat(pDemo->pPicker[DP_DATETIME_RANGE], "yyyy-mm-dd hh:nn");
	(void)xuiDatePickerSetRangeSeparator(pDemo->pPicker[DP_DATETIME_RANGE], " / ");
	(void)xuiDatePickerSetRangeValue(pDemo->pPicker[DP_DATETIME_RANGE], __xuiDatePickerDemoDateTime(2026, 5, 19, 9, 0, 0), __xuiDatePickerDemoDateTime(2026, 5, 19, 13, 0, 0));
	(void)xuiDatePickerSetNullable(pDemo->pPicker[DP_NULLABLE], 1);
	(void)xuiDatePickerClearValue(pDemo->pPicker[DP_NULLABLE]);
	(void)xuiDatePickerSetLimits(pDemo->pPicker[DP_LIMITED], xrtDateSerial(2026, 5, 1), xrtDateSerial(2026, 5, 31));
	(void)xuiDatePickerSetValue(pDemo->pPicker[DP_LIMITED], xrtDateSerial(2026, 6, 20));
	(void)xuiWidgetSetEnabled(pDemo->pPicker[DP_DISABLED], 0);
	return XUI_OK;
}

static uint32_t __xuiDatePickerReadButtons(void)
{
	uint32_t iButtons;

	iButtons = 0;
	if ( xgeMouseDown(XGE_MOUSE_LEFT) ) iButtons |= XUI_POINTER_BUTTON_LEFT;
	if ( xgeMouseDown(XGE_MOUSE_RIGHT) ) iButtons |= XUI_POINTER_BUTTON_RIGHT;
	if ( xgeMouseDown(XGE_MOUSE_MIDDLE) ) iButtons |= XUI_POINTER_BUTTON_MIDDLE;
	return iButtons;
}

static int __xuiDatePickerHandleInput(xui_datepicker_demo_t* pDemo)
{
	float fX;
	float fY;
	float fWheelX;
	float fWheelY;
	uint32_t iButtons;
	uint32_t iPressed;
	uint32_t iReleased;
	uint32_t iModifiers;
	uint32_t iText;
	int iRet;

	iModifiers = 0;
	if ( xgeKeyDown(XGE_KEY_LEFT_CONTROL) || xgeKeyDown(XGE_KEY_RIGHT_CONTROL) ) iModifiers |= XUI_MOD_CTRL;
	if ( xgeKeyDown(XGE_KEY_LEFT_SHIFT) || xgeKeyDown(XGE_KEY_RIGHT_SHIFT) ) iModifiers |= XUI_MOD_SHIFT;
	if ( xgeKeyDown(XGE_KEY_LEFT_ALT) || xgeKeyDown(XGE_KEY_RIGHT_ALT) ) iModifiers |= XUI_MOD_ALT;
	(void)xuiInputSetModifiers(pDemo->pContext, iModifiers);
	if ( xgeKeyPressed(XGE_KEY_ESCAPE) ) {
		if ( xuiDatePickerIsOpen(pDemo->pPicker[DP_DATE]) || xuiDatePickerIsOpen(pDemo->pPicker[DP_TIME]) ||
		     xuiDatePickerIsOpen(pDemo->pPicker[DP_DATETIME]) || xuiDatePickerIsOpen(pDemo->pPicker[DP_DATE_RANGE]) ||
		     xuiDatePickerIsOpen(pDemo->pPicker[DP_TIME_RANGE]) || xuiDatePickerIsOpen(pDemo->pPicker[DP_DATETIME_RANGE]) ||
		     xuiDatePickerIsOpen(pDemo->pPicker[DP_NULLABLE]) || xuiDatePickerIsOpen(pDemo->pPicker[DP_LIMITED]) ) {
			iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ESCAPE, iModifiers);
			if ( iRet != XUI_OK ) return iRet;
		} else {
			xgeQuit();
		}
	}
	if ( xgeKeyPressed(XGE_KEY_ENTER) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_ENTER, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_BACKSPACE) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_BACKSPACE, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_DELETE) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_DELETE, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_UP) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_UP, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( xgeKeyPressed(XGE_KEY_DOWN) ) {
		iRet = xuiInputKeyDown(pDemo->pContext, XUI_KEY_DOWN, iModifiers);
		if ( iRet != XUI_OK ) return iRet;
	}
	while ( (iText = xgeTextGet()) != 0 ) {
		if ( (iModifiers & (XUI_MOD_CTRL | XUI_MOD_ALT)) == 0u ) {
			iRet = xuiInputText(pDemo->pContext, iText);
			if ( iRet != XUI_OK ) return iRet;
		}
	}
	xgeMouseGet(&fX, &fY);
	xgeMouseGetWheel(&fWheelX, &fWheelY);
	pDemo->fUiMouseX = fX - DEMO_OFFSET_X;
	pDemo->fUiMouseY = fY - DEMO_OFFSET_Y;
	iButtons = __xuiDatePickerReadButtons();
	if ( !pDemo->bHasMouse || (pDemo->fLastMouseX != fX) || (pDemo->fLastMouseY != fY) || (pDemo->iLastButtons != iButtons) ) {
		iRet = xuiInputPointerMove(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( fWheelX != 0.0f || fWheelY != 0.0f ) {
		iRet = xuiInputPointerWheel(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, fWheelX, fWheelY, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	iPressed = iButtons & ~pDemo->iLastButtons;
	iReleased = pDemo->iLastButtons & ~iButtons;
	if ( (iPressed & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerDown(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	if ( (iReleased & XUI_POINTER_BUTTON_LEFT) != 0 ) {
		iRet = xuiInputPointerUp(pDemo->pContext, pDemo->fUiMouseX, pDemo->fUiMouseY, XUI_POINTER_BUTTON_LEFT, iButtons);
		if ( iRet != XUI_OK ) return iRet;
	}
	pDemo->bHasMouse = 1;
	pDemo->fLastMouseX = fX;
	pDemo->fLastMouseY = fY;
	pDemo->iLastButtons = iButtons;
	return XUI_OK;
}

static int __xuiDatePickerClickPanelRect(xui_datepicker_demo_t* pDemo, xui_widget pPicker, xui_rect_t tRect)
{
	xui_widget pPanel;
	xui_rect_t tWorld;
	int iRet;

	pPanel = xuiDatePickerGetPanelWidget(pPicker);
	if ( pPanel == NULL ) return XUI_ERROR_INVALID_ARGUMENT;
	tWorld = xuiWidgetGetWorldRect(pPanel);
	iRet = xuiInputPointerDown(pDemo->pContext, tWorld.fX + tRect.fX + tRect.fW * 0.5f, tWorld.fY + tRect.fY + tRect.fH * 0.5f,
		XUI_POINTER_BUTTON_LEFT, XUI_POINTER_BUTTON_LEFT);
	if ( iRet != XUI_OK ) return iRet;
	return xuiDispatchPendingEvents(pDemo->pContext);
}

static int __xuiDatePickerFindDay(xui_widget pPicker, int iPanel, xtime tDay)
{
	int i;

	for ( i = 0; i < 42; i++ ) {
		if ( xuiDatePickerGetDayValue(pPicker, iPanel, i) == xrtDatePart(tDay) ) return i;
	}
	return -1;
}

static void __xuiDatePickerRunChecks(xui_datepicker_demo_t* pDemo, int bExerciseInput)
{
	xtime tStart;
	xtime tEnd;
	int iIndex;

	pDemo->bCreateOK = (pDemo->pRoot != NULL) && (pDemo->pPicker[DP_DATE] != NULL) && (xuiDatePickerGetPopupWidget(pDemo->pPicker[DP_DATE]) != NULL);
	pDemo->bLayoutOK = (xuiWidgetGetWorldRect(pDemo->pPicker[DP_DATE]).fW > 100.0f) && (xuiDatePickerGetButtonRect(pDemo->pPicker[DP_DATE]).fW > 20.0f);
	pDemo->bModeOK =
		xuiDatePickerGetMode(pDemo->pPicker[DP_DATE]) == XUI_DATE_PICKER_MODE_DATE &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_TIME]) == XUI_DATE_PICKER_MODE_TIME &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_DATETIME]) == XUI_DATE_PICKER_MODE_DATETIME &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_DATE_RANGE]) == XUI_DATE_PICKER_MODE_DATE_RANGE &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_TIME_RANGE]) == XUI_DATE_PICKER_MODE_TIME_RANGE &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_DATETIME_RANGE]) == XUI_DATE_PICKER_MODE_DATETIME_RANGE &&
		xuiDatePickerGetMode(pDemo->pPicker[DP_LIMITED]) == XUI_DATE_PICKER_MODE_DATE;
	pDemo->bValueOK =
		xuiDatePickerGetValue(pDemo->pPicker[DP_DATE]) == (pDemo->bExerciseDone ? xrtDateSerial(2026, 5, 20) : xrtDateSerial(2026, 5, 19)) &&
		xuiDatePickerGetValue(pDemo->pPicker[DP_TIME]) == __xuiDatePickerDemoTime(9, 30, 0) &&
		xuiDatePickerGetValue(pDemo->pPicker[DP_DATETIME]) == __xuiDatePickerDemoDateTime(2026, 5, 19, 15, 45, 30);
	(void)xuiDatePickerGetRangeValue(pDemo->pPicker[DP_DATE_RANGE], &tStart, &tEnd);
	pDemo->bRangeOK = (tStart == xrtDateSerial(2026, 5, 18)) && (tEnd == xrtDateSerial(2026, 5, 20));
	(void)xuiDatePickerGetRangeValue(pDemo->pPicker[DP_TIME_RANGE], &tStart, &tEnd);
	pDemo->bRangeOK = pDemo->bRangeOK && (tStart == __xuiDatePickerDemoTime(9, 0, 0)) && (tEnd == __xuiDatePickerDemoTime(13, 0, 0));
	pDemo->bLimitOK = xuiDatePickerGetValue(pDemo->pPicker[DP_LIMITED]) == xrtDateSerial(2026, 5, 31);
	pDemo->bClearOK = !xuiDatePickerHasValue(pDemo->pPicker[DP_NULLABLE]);
	pDemo->bDisabledOK = !xuiWidgetGetEnabled(pDemo->pPicker[DP_DISABLED]);
	if ( bExerciseInput && !pDemo->bExerciseDone ) {
		(void)xuiDatePickerOpen(pDemo->pPicker[DP_DATE]);
		(void)xuiLayout(pDemo->pContext);
		iIndex = __xuiDatePickerFindDay(pDemo->pPicker[DP_DATE], 0, xrtDateSerial(2026, 5, 20));
		if ( iIndex >= 0 ) {
			(void)__xuiDatePickerClickPanelRect(pDemo, pDemo->pPicker[DP_DATE], xuiDatePickerGetDayRect(pDemo->pPicker[DP_DATE], 0, iIndex));
			(void)__xuiDatePickerClickPanelRect(pDemo, pDemo->pPicker[DP_DATE], xuiDatePickerGetFooterRect(pDemo->pPicker[DP_DATE], XUI_DATE_PICKER_FOOTER_OK));
		}
		pDemo->bPopupOK = !xuiDatePickerIsOpen(pDemo->pPicker[DP_DATE]) &&
			(xuiDatePickerGetValue(pDemo->pPicker[DP_DATE]) == xrtDateSerial(2026, 5, 20)) &&
			(pDemo->iChangingCount > 0) && (pDemo->iChangeCount > 0) && (pDemo->iCommitCount > 0);
		(void)xuiDatePickerOpen(pDemo->pPicker[DP_NULLABLE]);
		(void)xuiLayout(pDemo->pContext);
		(void)__xuiDatePickerClickPanelRect(pDemo, pDemo->pPicker[DP_NULLABLE], xuiDatePickerGetFooterRect(pDemo->pPicker[DP_NULLABLE], XUI_DATE_PICKER_FOOTER_CLEAR));
		pDemo->bClearOK = pDemo->bClearOK && !xuiDatePickerHasValue(pDemo->pPicker[DP_NULLABLE]) && (pDemo->iClearCount > 0);
		pDemo->bExerciseDone = 1;
	} else if ( !bExerciseInput ) {
		pDemo->bPopupOK = 1;
	}
}

static int __xuiDatePickerCreateAssets(xui_datepicker_demo_t* pDemo)
{
	xui_surface_desc_t tSurfaceDesc;
	const char* sFontPath;
	int iRet;

	pDemo->tProxy = xuiProxyXge();
	iRet = xuiCreate(&pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiSetProxy(pDemo->pContext, &pDemo->tProxy);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiInputViewport(pDemo->pContext, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H);
	if ( iRet != XUI_OK ) return iRet;
	memset(&tSurfaceDesc, 0, sizeof(tSurfaceDesc));
	tSurfaceDesc.iKind = XUI_SURFACE_KIND_TEXTURE;
	tSurfaceDesc.iWidth = DEMO_TARGET_W;
	tSurfaceDesc.iHeight = DEMO_TARGET_H;
	tSurfaceDesc.iFormat = XUI_SURFACE_FORMAT_RGBA8;
	tSurfaceDesc.iFlags = XUI_SURFACE_ALPHA_PREMULTIPLIED | XUI_SURFACE_USAGE_TARGET;
	iRet = pDemo->tProxy.surfaceCreate(&pDemo->tProxy, &pDemo->pTarget, &tSurfaceDesc);
	if ( iRet != XUI_OK ) return iRet;
	sFontPath = __xuiDatePickerFindTtf();
	if ( sFontPath == NULL ) return XUI_ERROR_FILE_NOT_FOUND;
	iRet = pDemo->tProxy.fontLoadFile(&pDemo->tProxy, &pDemo->pFont, sFontPath, 14.0f, XUI_FONT_FORMAT_TTF);
	if ( iRet != XUI_OK ) return iRet;
	(void)xuiSetDefaultFont(pDemo->pContext, pDemo->pFont);
	return __xuiDatePickerCreateUi(pDemo);
}

static void __xuiDatePickerDestroyAssets(xui_datepicker_demo_t* pDemo)
{
	if ( pDemo->pContext != NULL ) {
		xuiDestroy(pDemo->pContext);
		pDemo->pContext = NULL;
	}
	if ( pDemo->pFont != NULL ) {
		pDemo->tProxy.fontDestroy(&pDemo->tProxy, pDemo->pFont);
		pDemo->pFont = NULL;
	}
	if ( pDemo->pTarget != NULL ) {
		pDemo->tProxy.surfaceDestroy(&pDemo->tProxy, pDemo->pTarget);
		pDemo->pTarget = NULL;
	}
}

static int __xuiDatePickerFrame(void* pUser)
{
	xui_datepicker_demo_t* pDemo;
	xui_rect_i_t tFullRect;
	xui_rect_t tSrc;
	xui_rect_t tDst;
	xui_render_stats_t tStats;
	int iRet;
	int bAutoRun;

	pDemo = (xui_datepicker_demo_t*)pUser;
	if ( pDemo == NULL ) return XGE_ERROR_INVALID_ARGUMENT;
	bAutoRun = (pDemo->iMaxFrames > 0) || (pDemo->fMaxSeconds > 0.0);
	iRet = xgeBegin();
	if ( iRet != XGE_OK ) return iRet;
	iRet = __xuiDatePickerHandleInput(pDemo);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiDispatchPendingEvents(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiLayout(pDemo->pContext);
	if ( iRet != XUI_OK ) return iRet;
	iRet = xuiUpdate(pDemo->pContext, xgeGetDelta());
	if ( iRet != XUI_OK ) return iRet;
	__xuiDatePickerRunChecks(pDemo, bAutoRun);
	iRet = pDemo->tProxy.surfaceClear(&pDemo->tProxy, pDemo->pTarget, XUI_COLOR_RGBA(234, 242, 250, 255));
	if ( iRet != XUI_OK ) return iRet;
	tFullRect = (xui_rect_i_t){0, 0, DEMO_TARGET_W, DEMO_TARGET_H};
	iRet = xuiRender(pDemo->pContext, pDemo->pTarget, &tFullRect, 1);
	if ( iRet != XUI_OK ) return iRet;
	xgeClear(XUI_COLOR_RGBA(18, 23, 32, 255));
	tSrc = (xui_rect_t){0.0f, 0.0f, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	tDst = (xui_rect_t){DEMO_OFFSET_X, DEMO_OFFSET_Y, (float)DEMO_TARGET_W, (float)DEMO_TARGET_H};
	iRet = pDemo->tProxy.surfaceDraw(&pDemo->tProxy, pDemo->pTarget, tSrc, tDst, XUI_COLOR_WHITE, XUI_SURFACE_DRAW_SCREEN_SPACE);
	if ( iRet == XUI_OK ) iRet = xgeEnd();
	if ( iRet != XGE_OK ) return iRet;
	pDemo->iFrame++;
	if ( (pDemo->iMaxFrames > 0 && pDemo->iFrame >= pDemo->iMaxFrames) ||
	     (pDemo->fMaxSeconds > 0.0 && xgeTimer() >= pDemo->fMaxSeconds) ) {
		memset(&tStats, 0, sizeof(tStats));
		(void)xuiGetRenderStats(pDemo->pContext, &tStats);
		printf("xui_datepicker final-summary frames=%d create=%d layout=%d mode=%d value=%d range=%d limit=%d popup=%d clear=%d disabled=%d changing=%d changes=%d commits=%d cancels=%d clears=%d updatedCaches=%d drawnCaches=%d\n",
			pDemo->iFrame, pDemo->bCreateOK, pDemo->bLayoutOK, pDemo->bModeOK, pDemo->bValueOK, pDemo->bRangeOK,
			pDemo->bLimitOK, pDemo->bPopupOK, pDemo->bClearOK, pDemo->bDisabledOK,
			pDemo->iChangingCount, pDemo->iChangeCount, pDemo->iCommitCount, pDemo->iCancelCount, pDemo->iClearCount,
			tStats.iUpdatedCaches, tStats.iDrawnCaches);
		xgeQuit();
	}
	return XGE_OK;
}

int main(int argc, char** argv)
{
	xui_datepicker_demo_t tDemo;
	xge_desc_t tDesc;
	int iRet;

	memset(&tDemo, 0, sizeof(tDemo));
	iRet = __xuiDatePickerParseArgs(&tDemo, argc, argv);
	if ( iRet == 1 ) return 0;
	if ( iRet != XGE_OK ) {
		__xuiDatePickerUsage();
		return 1;
	}
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = DEMO_TARGET_W + 20;
	tDesc.iHeight = DEMO_TARGET_H + 50;
	tDesc.sTitle = "XUI DatePicker";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	tDesc.iTargetFPS = 60;
	iRet = xgeInit(&tDesc);
	if ( iRet != XGE_OK ) {
		printf("xui_datepicker: xgeInit failed: %d\n", iRet);
		return 1;
	}
	iRet = __xuiDatePickerCreateAssets(&tDemo);
	if ( iRet != XUI_OK ) {
		printf("xui_datepicker: create assets failed: %d\n", iRet);
		__xuiDatePickerDestroyAssets(&tDemo);
		xgeUnit();
		return 1;
	}
	iRet = xgeRun(__xuiDatePickerFrame, &tDemo);
	__xuiDatePickerDestroyAssets(&tDemo);
	xgeUnit();
	return (iRet == XGE_OK && tDemo.bCreateOK && tDemo.bLayoutOK && tDemo.bModeOK &&
		tDemo.bValueOK && tDemo.bRangeOK && tDemo.bLimitOK && tDemo.bPopupOK &&
		tDemo.bClearOK && tDemo.bDisabledOK) ? 0 : 1;
}
