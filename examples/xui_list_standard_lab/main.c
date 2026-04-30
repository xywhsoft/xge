#include "../../xge.h"
#include <stdio.h>
#include <string.h>

static const char* g_arrItems[] = {
	"Alpha", "Bravo disabled", "Charlie", "Delta",
	"Echo disabled", "Foxtrot", "Golf", "Hotel disabled",
	"India", "Juliet", "Kilo", "Lima"
};

static const int g_arrEnabled[] = {
	1, 0, 1, 1,
	0, 1, 1, 0,
	1, 1, 1, 1
};

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pListWidget;
	xge_xui_list_view_t tList;
	int arrSelected[12];
	int iSelectCount;
	int iLastSelected;
	int iRendererCount;
	int iRendererStateMask;
} app_state_t;

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void MakeKeyEventMods(xge_event_t* pEvent, int iKey, int iModifiers)
{
	MakeKeyEvent(pEvent, iKey);
	pEvent->iParam2 = iModifiers;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return (fA >= fB - fEpsilon) && (fA <= fB + fEpsilon);
}

static void ListSelect(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static int ListItemRenderer(xge_xui_widget pWidget, int iIndex, xge_rect_t tRect, int iState, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	(void)iIndex;
	(void)tRect;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iRendererCount++;
		pApp->iRendererStateMask |= iState;
	}
	return 0;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 240.0f, 160.0f });
	pApp->pListWidget = xgeXuiWidgetCreate();
	if ( pApp->pListWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pListWidget, (xge_rect_t){ 10.0f, 10.0f, 180.0f, 60.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pListWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pListWidget);
	if ( xgeXuiListViewInit(&pApp->tList, &pApp->tXui, pApp->pListWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiListViewSetItemHeight(&pApp->tList, 20.0f);
	xgeXuiListViewSetItems(&pApp->tList, g_arrItems, (int)(sizeof(g_arrItems) / sizeof(g_arrItems[0])));
	xgeXuiListViewSetEnabledItems(&pApp->tList, g_arrEnabled, (int)(sizeof(g_arrEnabled) / sizeof(g_arrEnabled[0])));
	xgeXuiListViewSetSelect(&pApp->tList, ListSelect, pApp);
	xgeXuiListViewSetSelectionBuffer(&pApp->tList, pApp->arrSelected, (int)(sizeof(pApp->arrSelected) / sizeof(pApp->arrSelected[0])));
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	return 1;
}

static int TestKeyboard(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bArrowOK;
	int bHomeEndOK;
	int bPageOK;
	int bNotifyOK;

	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);

	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	bArrowOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0) &&
		(pApp->iSelectCount == 1) &&
		(pApp->iLastSelected == 0);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	bArrowOK = bArrowOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 2) &&
		(pApp->iSelectCount == 2) &&
		(pApp->iLastSelected == 2);
	MakeKeyEvent(&tEvent, XGE_KEY_UP);
	bArrowOK = bArrowOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0) &&
		(pApp->iSelectCount == 3);

	MakeKeyEvent(&tEvent, XGE_KEY_END);
	bHomeEndOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 11) &&
		(FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 180.0f, 0.01f));
	MakeKeyEvent(&tEvent, XGE_KEY_HOME);
	bHomeEndOK = bHomeEndOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0) &&
		(FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 0.0f, 0.01f));

	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_DOWN);
	bPageOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 3) &&
		(FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 20.0f, 0.01f));
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_DOWN);
	bPageOK = bPageOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 6) &&
		(FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 80.0f, 0.01f));
	MakeKeyEvent(&tEvent, XGE_KEY_PAGE_UP);
	bPageOK = bPageOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 3) &&
		(FloatNear(xgeXuiListViewGetScroll(&pApp->tList), 60.0f, 0.01f));

	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bNotifyOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->iLastSelected == 3);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bNotifyOK = bNotifyOK &&
		(xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->iLastSelected == 3);

	return bArrowOK && bHomeEndOK && bPageOK && bNotifyOK;
}

static int TestDisabledAndFocus(app_state_t* pApp)
{
	xge_event_t tEvent;
	int iBefore;
	int bDisabledItemOK;
	int bBlurOK;
	int bDisabledWidgetOK;

	xgeXuiListViewSetScroll(&pApp->tList, 0.0f);
	xgeXuiListViewSetSelected(&pApp->tList, 0);
	iBefore = pApp->iSelectCount;
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 31.0f);
	bDisabledItemOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0) &&
		(pApp->iSelectCount == iBefore);

	xgeXuiSetFocus(&pApp->tXui, NULL);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	bBlurOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0);

	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);
	xgeXuiWidgetSetEnabled(pApp->pListWidget, 0);
	iBefore = pApp->iSelectCount;
	MakeKeyEvent(&tEvent, XGE_KEY_END);
	bDisabledWidgetOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 0) &&
		(pApp->iSelectCount == iBefore);
	xgeXuiWidgetSetEnabled(pApp->pListWidget, 1);

	return bDisabledItemOK && bBlurOK && bDisabledWidgetOK;
}

static int TestSelectionMode(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bApiOK;
	int bCtrlOK;
	int bRangeOK;

	xgeXuiListViewSetSelectionMode(&pApp->tList, XGE_XUI_SELECTION_MULTI);
	xgeXuiListViewClearSelection(&pApp->tList);
	xgeXuiListViewSetItemSelected(&pApp->tList, 2, 1);
	xgeXuiListViewSetItemSelected(&pApp->tList, 5, 1);
	bApiOK = (xgeXuiListViewGetSelectionMode(&pApp->tList) == XGE_XUI_SELECTION_MULTI) &&
		xgeXuiListViewIsItemSelected(&pApp->tList, 2) &&
		xgeXuiListViewIsItemSelected(&pApp->tList, 5) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 5);

	xgeXuiListViewSetScroll(&pApp->tList, 60.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 51.0f);
	tEvent.iParam2 = XGE_KEY_MOD_CTRL;
	bCtrlOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		xgeXuiListViewIsItemSelected(&pApp->tList, 2) &&
		(xgeXuiListViewIsItemSelected(&pApp->tList, 5) == 0);

	xgeXuiListViewSetSelectionMode(&pApp->tList, XGE_XUI_SELECTION_RANGE);
	xgeXuiListViewClearSelection(&pApp->tList);
	xgeXuiListViewSetSelected(&pApp->tList, 2);
	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);
	MakeKeyEventMods(&tEvent, XGE_KEY_DOWN, XGE_KEY_MOD_SHIFT);
	bRangeOK = (xgeXuiListViewEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		xgeXuiListViewIsItemSelected(&pApp->tList, 2) &&
		xgeXuiListViewIsItemSelected(&pApp->tList, 3) &&
		(xgeXuiListViewGetSelected(&pApp->tList) == 3);

	return bApiOK && bCtrlOK && bRangeOK;
}

static int TestRenderer(app_state_t* pApp)
{
	pApp->iRendererCount = 0;
	pApp->iRendererStateMask = 0;
	xgeXuiListViewSetItemRenderer(&pApp->tList, ListItemRenderer, pApp);
	xgeXuiListViewPaintProc(pApp->pListWidget, &pApp->tList);
	return (pApp->iRendererCount > 0) && ((pApp->iRendererStateMask & XGE_XUI_LIST_ITEM_SELECTED) != 0);
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bKeyboardOK;
	int bDisabledOK;
	int bFocusOK;
	int bSelectionOK;
	int bRendererOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-list-standard-lab init failed\n");
		return 1;
	}
	bCreateOK = CreateUI(&tApp);
	bKeyboardOK = bCreateOK && TestKeyboard(&tApp);
	bDisabledOK = bCreateOK && TestDisabledAndFocus(&tApp);
	bFocusOK = bCreateOK && xgeXuiWidgetIsFocusable(tApp.pListWidget);
	bSelectionOK = bCreateOK && TestSelectionMode(&tApp);
	bRendererOK = bCreateOK && TestRenderer(&tApp);

	printf(
		"xui-list-standard-lab final-summary create=%d keyboard=%d disabled=%d focus=%d selection=%d renderer=%d selected=%d scroll=%.2f callbacks=%d last=%d renderCalls=%d\n",
		bCreateOK,
		bKeyboardOK,
		bDisabledOK,
		bFocusOK,
		bSelectionOK,
		bRendererOK,
		xgeXuiListViewGetSelected(&tApp.tList),
		xgeXuiListViewGetScroll(&tApp.tList),
		tApp.iSelectCount,
		tApp.iLastSelected,
		tApp.iRendererCount);

	xgeXuiListViewUnit(&tApp.tList);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bKeyboardOK && bDisabledOK && bFocusOK && bSelectionOK && bRendererOK) ? 0 : 2;
}
