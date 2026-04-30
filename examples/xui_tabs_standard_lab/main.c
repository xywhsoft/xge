#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

static const char* g_arrItems[] = {
	"Home", "Disabled", "Scene", "Assets", "Locked", "Output"
};

static const int g_arrEnabled[] = {
	1, 0, 1, 1, 0, 1
};

static const int g_arrDirty[] = {
	0, 1, 1, 0, 0, 1
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pTabsWidget;
	xge_xui_tabs_t tTabs;
	int iSelectCount;
	int iLastSelected;
	int iCloseCount;
	int iLastClosed;
} app_state_t;

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeWheelEvent(xge_event_t* pEvent, float fX, float fY, float fDX, float fDY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_MOUSE_WHEEL;
	pEvent->fX = fX;
	pEvent->fY = fY;
	pEvent->fDX = fDX;
	pEvent->fDY = fDY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return (fA >= fB - fEpsilon) && (fA <= fB + fEpsilon);
}

static void TabsSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static void TabsClose(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCloseCount++;
		pApp->iLastClosed = iIndex;
	}
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 260.0f, 120.0f });
	pApp->pTabsWidget = xgeXuiWidgetCreate();
	if ( pApp->pTabsWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pTabsWidget, (xge_rect_t){ 10.0f, 10.0f, 180.0f, 32.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pTabsWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pTabsWidget);
	if ( xgeXuiTabsInit(&pApp->tTabs, &pApp->tXui, pApp->pTabsWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiTabsSetItems(&pApp->tTabs, g_arrItems, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiTabsSetEnabledItems(&pApp->tTabs, g_arrEnabled, (int)(sizeof(g_arrEnabled) / sizeof(g_arrEnabled[0])));
	xgeXuiTabsSetDirtyItems(&pApp->tTabs, g_arrDirty, (int)(sizeof(g_arrDirty) / sizeof(g_arrDirty[0])));
	xgeXuiTabsSetIcons(&pApp->tTabs, NULL, NULL, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiTabsSetSelect(&pApp->tTabs, TabsSelect, pApp);
	xgeXuiTabsSetClose(&pApp->tTabs, TabsClose, 1, pApp);
	xgeXuiTabsSetTabSize(&pApp->tTabs, 70.0f, 24.0f);
	xgeXuiTabsSetScrollable(&pApp->tTabs, 1);
	xgeXuiTabsSetSelected(&pApp->tTabs, 0);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	return 1;
}

static int TestDisabled(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBeforeSelect;
	int iBeforeCount;
	int bDownOK;
	int bUpOK;

	iBeforeSelect = xgeXuiTabsGetSelected(&pApp->tTabs);
	iBeforeCount = pApp->iSelectCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 90.0f, 20.0f);
	bDownOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 90.0f, 20.0f);
	bUpOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONTINUE);

	return bDownOK && bUpOK &&
		(xgeXuiTabsGetSelected(&pApp->tTabs) == iBeforeSelect) &&
		(pApp->iSelectCount == iBeforeCount);
}

static int TestKeyboard(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bRightOK;
	int bLeftOK;

	xgeXuiSetFocus(&pApp->tXui, pApp->pTabsWidget);
	xgeXuiTabsSetSelected(&pApp->tTabs, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_RIGHT);
	bRightOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTabsGetSelected(&pApp->tTabs) == 2) &&
		(pApp->iLastSelected == 2);
	MakeKeyEvent(&tEvent, XGE_KEY_LEFT);
	bLeftOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTabsGetSelected(&pApp->tTabs) == 0) &&
		(pApp->iLastSelected == 0);

	return bRightOK && bLeftOK;
}

static int TestClose(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBeforeSelect;
	int iBeforeSelectCount;
	int bDownOK;
	int bUpOK;

	xgeXuiTabsSetScroll(&pApp->tTabs, 0.0f);
	xgeXuiTabsSetSelected(&pApp->tTabs, 0);
	iBeforeSelect = xgeXuiTabsGetSelected(&pApp->tTabs);
	iBeforeSelectCount = pApp->iSelectCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 66.0f, 22.0f);
	bDownOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 66.0f, 22.0f);
	bUpOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->iCloseCount == 1) &&
		(pApp->iLastClosed == 0);

	return bDownOK && bUpOK &&
		(xgeXuiTabsGetSelected(&pApp->tTabs) == iBeforeSelect) &&
		(pApp->iSelectCount == iBeforeSelectCount);
}

static int TestScrollable(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bWheelOK;
	int bEnsureOK;
	int bSkipOK;

	xgeXuiTabsSetScroll(&pApp->tTabs, 0.0f);
	MakeWheelEvent(&tEvent, 20.0f, 20.0f, -1.0f, 0.0f);
	bWheelOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(FloatNear(xgeXuiTabsGetScroll(&pApp->tTabs), 32.0f, 0.01f));

	xgeXuiTabsSetSelected(&pApp->tTabs, 5);
	bEnsureOK = (xgeXuiTabsGetSelected(&pApp->tTabs) == 5) &&
		(FloatNear(xgeXuiTabsGetScroll(&pApp->tTabs), 240.0f, 0.01f));

	xgeXuiSetFocus(&pApp->tXui, pApp->pTabsWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_LEFT);
	bSkipOK = (xgeXuiTabsEvent(&pApp->tTabs, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiTabsGetSelected(&pApp->tTabs) == 3) &&
		(FloatNear(xgeXuiTabsGetScroll(&pApp->tTabs), 210.0f, 0.01f));

	return bWheelOK && bEnsureOK && bSkipOK;
}

static int TestMetadata(app_state_t* pApp)
{
	return (pApp->tTabs.arrEnabled == g_arrEnabled) &&
		(pApp->tTabs.iEnabledCount == 6) &&
		(pApp->tTabs.arrDirty == g_arrDirty) &&
		(pApp->tTabs.iDirtyCount == 6) &&
		(pApp->tTabs.iIconCount == 6) &&
		(pApp->tTabs.bCloseButtons != 0) &&
		xgeXuiWidgetIsFocusable(pApp->pTabsWidget);
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bDisabledOK;
	int bKeyboardOK;
	int bCloseOK;
	int bScrollOK;
	int bMetaOK;

	memset(&tApp, 0, sizeof(tApp));
	tApp.iLastSelected = -1;
	tApp.iLastClosed = -1;
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-tabs-standard-lab init failed\n");
		return 1;
	}

	bCreateOK = CreateUI(&tApp);
	bDisabledOK = bCreateOK && TestDisabled(&tApp);
	bKeyboardOK = bCreateOK && TestKeyboard(&tApp);
	bCloseOK = bCreateOK && TestClose(&tApp);
	bScrollOK = bCreateOK && TestScrollable(&tApp);
	bMetaOK = bCreateOK && TestMetadata(&tApp);

	printf(
		"xui-tabs-standard-lab final-summary create=%d disabled=%d keyboard=%d close=%d scroll=%d meta=%d selected=%d scrollX=%.2f callbacks=%d/%d last=%d/%d\n",
		bCreateOK,
		bDisabledOK,
		bKeyboardOK,
		bCloseOK,
		bScrollOK,
		bMetaOK,
		xgeXuiTabsGetSelected(&tApp.tTabs),
		xgeXuiTabsGetScroll(&tApp.tTabs),
		tApp.iSelectCount,
		tApp.iCloseCount,
		tApp.iLastSelected,
		tApp.iLastClosed);

	xgeXuiTabsUnit(&tApp.tTabs);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bDisabledOK && bKeyboardOK && bCloseOK && bScrollOK && bMetaOK) ? 0 : 2;
}
