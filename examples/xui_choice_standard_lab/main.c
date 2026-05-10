#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pCheckWidget;
	xge_xui_widget pRadioAWidget;
	xge_xui_widget pRadioBWidget;
	xge_xui_widget pToggleWidget;
	xge_xui_checkbox_t tCheck;
	xge_xui_radio_group_t tRadioGroup;
	xge_xui_radio_t tRadioA;
	xge_xui_radio_t tRadioB;
	xge_xui_toggle_t tToggle;
	int iCheckCallbackCount;
	int iRadioACallbackCount;
	int iRadioBCallbackCount;
	int iRadioGroupCallbackCount;
	int iToggleCallbackCount;
	int iLastCheckValue;
	int iLastRadioValue;
	int iLastGroupValue;
	int iLastToggleValue;
} app_state_t;

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void CheckChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCheckCallbackCount++;
		pApp->iLastCheckValue = bChecked;
	}
}

static void RadioAChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp != NULL) && bChecked ) {
		pApp->iRadioACallbackCount++;
		pApp->iLastRadioValue = 10;
	}
}

static void RadioBChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( (pApp != NULL) && bChecked ) {
		pApp->iRadioBCallbackCount++;
		pApp->iLastRadioValue = 20;
	}
}

static void RadioGroupChange(xge_xui_widget pWidget, int iValue, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iRadioGroupCallbackCount++;
		pApp->iLastGroupValue = iValue;
	}
}

static void ToggleChange(xge_xui_widget pWidget, int bChecked, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iToggleCallbackCount++;
		pApp->iLastToggleValue = bChecked;
	}
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	pApp->pCheckWidget = xgeXuiWidgetCreate();
	pApp->pRadioAWidget = xgeXuiWidgetCreate();
	pApp->pRadioBWidget = xgeXuiWidgetCreate();
	pApp->pToggleWidget = xgeXuiWidgetCreate();
	if ( (pApp->pCheckWidget == NULL) || (pApp->pRadioAWidget == NULL) || (pApp->pRadioBWidget == NULL) || (pApp->pToggleWidget == NULL) ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pCheckWidget, (xge_rect_t){ 20.0f, 20.0f, 160.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pRadioAWidget, (xge_rect_t){ 20.0f, 52.0f, 160.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pRadioBWidget, (xge_rect_t){ 20.0f, 84.0f, 160.0f, 24.0f });
	xgeXuiWidgetSetRect(pApp->pToggleWidget, (xge_rect_t){ 20.0f, 116.0f, 180.0f, 24.0f });
	xgeXuiWidgetAdd(pRoot, pApp->pCheckWidget);
	xgeXuiWidgetAdd(pRoot, pApp->pRadioAWidget);
	xgeXuiWidgetAdd(pRoot, pApp->pRadioBWidget);
	xgeXuiWidgetAdd(pRoot, pApp->pToggleWidget);

	if ( xgeXuiCheckBoxInit(&pApp->tCheck, &pApp->tXui, pApp->pCheckWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiCheckBoxSetText(&pApp->tCheck, NULL, "Check");
	xgeXuiCheckBoxSetChange(&pApp->tCheck, CheckChange, pApp);

	xgeXuiRadioGroupInit(&pApp->tRadioGroup);
	xgeXuiRadioGroupSetChange(&pApp->tRadioGroup, RadioGroupChange, pApp);
	if ( xgeXuiRadioInit(&pApp->tRadioA, &pApp->tXui, pApp->pRadioAWidget) != XGE_OK ) {
		return 0;
	}
	if ( xgeXuiRadioInit(&pApp->tRadioB, &pApp->tXui, pApp->pRadioBWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiRadioSetText(&pApp->tRadioA, NULL, "Radio A");
	xgeXuiRadioSetText(&pApp->tRadioB, NULL, "Radio B");
	xgeXuiRadioSetGroup(&pApp->tRadioA, &pApp->tRadioGroup, 10);
	xgeXuiRadioSetGroup(&pApp->tRadioB, &pApp->tRadioGroup, 20);
	xgeXuiRadioSetChange(&pApp->tRadioA, RadioAChange, pApp);
	xgeXuiRadioSetChange(&pApp->tRadioB, RadioBChange, pApp);

	if ( xgeXuiToggleInit(&pApp->tToggle, &pApp->tXui, pApp->pToggleWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiToggleSetInnerText(&pApp->tToggle, NULL, "OFF", "ON");
	xgeXuiToggleSetChange(&pApp->tToggle, ToggleChange, pApp);
	return 1;
}

static int TestCheckBox(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBefore;
	int bSpaceOK;
	int bEnterOK;
	int bBlurOK;
	int bDisabledOK;

	xgeXuiSetFocus(&pApp->tXui, pApp->pCheckWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bSpaceOK = (xgeXuiCheckBoxEvent(&pApp->tCheck, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheck) == 1) &&
		(pApp->iCheckCallbackCount == 1) &&
		(pApp->iLastCheckValue == 1);

	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bEnterOK = (xgeXuiCheckBoxEvent(&pApp->tCheck, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheck) == 0) &&
		(pApp->iCheckCallbackCount == 2) &&
		(pApp->iLastCheckValue == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pToggleWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bBlurOK = (xgeXuiCheckBoxEvent(&pApp->tCheck, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheck) == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pCheckWidget);
	xgeXuiWidgetSetEnabled(pApp->pCheckWidget, 0);
	iBefore = pApp->iCheckCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bDisabledOK = (xgeXuiCheckBoxEvent(&pApp->tCheck, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiCheckBoxGetChecked(&pApp->tCheck) == 0) &&
		(pApp->iCheckCallbackCount == iBefore) &&
		((xgeXuiCheckBoxGetState(&pApp->tCheck) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pCheckWidget, 1);
	return bSpaceOK && bEnterOK && bBlurOK && bDisabledOK;
}

static int TestRadio(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBeforeGroup;
	int bSpaceOK;
	int bEnterOK;
	int bRepeatOK;
	int bNavOK;
	int bDisabledOK;

	xgeXuiSetFocus(&pApp->tXui, pApp->pRadioAWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bSpaceOK = (xgeXuiRadioEvent(&pApp->tRadioA, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 10) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 1) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 0) &&
		(pApp->iRadioACallbackCount == 1) &&
		(pApp->iRadioGroupCallbackCount == 1) &&
		(pApp->iLastGroupValue == 10);

	xgeXuiSetFocus(&pApp->tXui, pApp->pRadioBWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bEnterOK = (xgeXuiRadioEvent(&pApp->tRadioB, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 20) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 0) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 1) &&
		(pApp->iRadioBCallbackCount == 1) &&
		(pApp->iRadioGroupCallbackCount == 2) &&
		(pApp->iLastRadioValue == 20) &&
		(pApp->iLastGroupValue == 20);

	iBeforeGroup = pApp->iRadioGroupCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bRepeatOK = (xgeXuiRadioEvent(&pApp->tRadioB, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 20) &&
		(pApp->iRadioGroupCallbackCount == iBeforeGroup);

	MakeKeyEvent(&tEvent, XGE_KEY_UP);
	bNavOK = (xgeXuiRadioEvent(&pApp->tRadioB, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 10) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 1) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 0) &&
		(pApp->tXui.pFocus == pApp->pRadioAWidget) &&
		(pApp->iRadioGroupCallbackCount == iBeforeGroup + 1);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	bNavOK = bNavOK &&
		(xgeXuiRadioEvent(&pApp->tRadioA, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 20) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioA) == 0) &&
		(xgeXuiRadioGetChecked(&pApp->tRadioB) == 1) &&
		(pApp->tXui.pFocus == pApp->pRadioBWidget) &&
		(pApp->iRadioGroupCallbackCount == iBeforeGroup + 2);
	xgeXuiWidgetSetEnabled(pApp->pRadioAWidget, 0);
	iBeforeGroup = pApp->iRadioGroupCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_UP);
	bDisabledOK = (xgeXuiRadioEvent(&pApp->tRadioB, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiRadioGroupGetSelected(&pApp->tRadioGroup) == 20) &&
		(pApp->tXui.pFocus == pApp->pRadioBWidget) &&
		(pApp->iRadioGroupCallbackCount == iBeforeGroup) &&
		((xgeXuiRadioGetState(&pApp->tRadioA) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pRadioAWidget, 1);
	return bSpaceOK && bEnterOK && bRepeatOK && bNavOK && bDisabledOK;
}

static int TestToggle(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBefore;
	int bSpaceOK;
	int bEnterOK;
	int bBlurOK;
	int bDisabledOK;

	xgeXuiSetFocus(&pApp->tXui, pApp->pToggleWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bSpaceOK = (xgeXuiToggleEvent(&pApp->tToggle, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 1) &&
		(pApp->iToggleCallbackCount == 1) &&
		(pApp->iLastToggleValue == 1);

	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bEnterOK = (xgeXuiToggleEvent(&pApp->tToggle, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 0) &&
		(pApp->iToggleCallbackCount == 2) &&
		(pApp->iLastToggleValue == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pCheckWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bBlurOK = (xgeXuiToggleEvent(&pApp->tToggle, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pToggleWidget);
	xgeXuiWidgetSetEnabled(pApp->pToggleWidget, 0);
	iBefore = pApp->iToggleCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bDisabledOK = (xgeXuiToggleEvent(&pApp->tToggle, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiToggleGetChecked(&pApp->tToggle) == 0) &&
		(pApp->iToggleCallbackCount == iBefore) &&
		((xgeXuiToggleGetState(&pApp->tToggle) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pToggleWidget, 1);
	return bSpaceOK && bEnterOK && bBlurOK && bDisabledOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bCheckOK;
	int bRadioOK;
	int bToggleOK;
	int bFocusOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-choice-standard-lab init failed\n");
		return 1;
	}
	bCreateOK = CreateUI(&tApp);
	bCheckOK = bCreateOK && TestCheckBox(&tApp);
	bRadioOK = bCreateOK && TestRadio(&tApp);
	bToggleOK = bCreateOK && TestToggle(&tApp);
	bFocusOK = bCreateOK &&
		xgeXuiWidgetIsFocusable(tApp.pCheckWidget) &&
		xgeXuiWidgetIsFocusable(tApp.pRadioAWidget) &&
		xgeXuiWidgetIsFocusable(tApp.pRadioBWidget) &&
		xgeXuiWidgetIsFocusable(tApp.pToggleWidget);

	printf(
		"xui-choice-standard-lab final-summary create=%d checkbox=%d radio=%d toggle=%d focus=%d callbacks=%d/%d/%d/%d\n",
		bCreateOK,
		bCheckOK,
		bRadioOK,
		bToggleOK,
		bFocusOK,
		tApp.iCheckCallbackCount,
		tApp.iRadioACallbackCount + tApp.iRadioBCallbackCount,
		tApp.iRadioGroupCallbackCount,
		tApp.iToggleCallbackCount);
	xgeXuiToggleUnit(&tApp.tToggle);
	xgeXuiRadioUnit(&tApp.tRadioB);
	xgeXuiRadioUnit(&tApp.tRadioA);
	xgeXuiRadioGroupUnit(&tApp.tRadioGroup);
	xgeXuiCheckBoxUnit(&tApp.tCheck);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bCheckOK && bRadioOK && bToggleOK && bFocusOK) ? 0 : 2;
}
