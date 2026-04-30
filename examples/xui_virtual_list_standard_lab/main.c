#include "../../xge.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pListWidget;
	xge_xui_virtual_list_t tList;
	int iItemCount;
	int iCreateCount;
	int iBindCount;
	int iSelectCount;
	int iLastSelected;
} app_state_t;

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
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

static int CountItems(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	return (pApp != NULL) ? pApp->iItemCount : 0;
}

static xge_xui_widget CreateSlot(xge_xui_widget pListWidget, int iSlot, void* pUser)
{
	app_state_t* pApp;
	xge_xui_widget pSlot;

	(void)pListWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iCreateCount++;
	}
	pSlot = xgeXuiWidgetCreate();
	if ( pSlot != NULL ) {
		xgeXuiWidgetSetName(pSlot, (iSlot == 0) ? "slot0" : "slot");
	}
	return pSlot;
}

static void BindSlot(xge_xui_widget pItemWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pItemWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iBindCount++;
	}
}

static void SelectItem(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iSelectCount++;
		pApp->iLastSelected = iIndex;
	}
}

static float VariableHeight(xge_xui_widget pWidget, int iIndex, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	return (iIndex % 2 == 0) ? 12.0f : 24.0f;
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 260.0f, 160.0f });
	pApp->pListWidget = xgeXuiWidgetCreate();
	if ( pApp->pListWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pListWidget, (xge_rect_t){ 10.0f, 10.0f, 120.0f, 72.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pListWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pListWidget);
	if ( xgeXuiVirtualListInit(&pApp->tList, &pApp->tXui, pApp->pListWidget) != XGE_OK ) {
		return 0;
	}
	pApp->iItemCount = 100;
	xgeXuiVirtualListSetItemHeight(&pApp->tList, 18.0f);
	xgeXuiVirtualListSetAdapter(&pApp->tList, CountItems, CreateSlot, BindSlot, pApp);
	xgeXuiVirtualListSetSelect(&pApp->tList, SelectItem, pApp);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	return 1;
}

static int TestEnsureVisible(app_state_t* pApp)
{
	int bEnsureDownOK;
	int bEnsureUpOK;

	xgeXuiVirtualListSetScroll(&pApp->tList, 0.0f);
	xgeXuiVirtualListEnsureVisible(&pApp->tList, 20);
	bEnsureDownOK = FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 306.0f, 0.01f);
	xgeXuiVirtualListEnsureVisible(&pApp->tList, 2);
	bEnsureUpOK = FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 36.0f, 0.01f);

	return bEnsureDownOK && bEnsureUpOK;
}

static int TestSlotReuse(app_state_t* pApp)
{
	xge_xui_widget pSlot0;
	int iCreateBefore;
	int iBindBefore;
	int bInitialOK;
	int bReuseOK;
	int bRefreshOK;

	xgeXuiVirtualListSetScroll(&pApp->tList, 0.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pSlot0 = xgeXuiVirtualListGetSlotWidget(&pApp->tList, 0);
	bInitialOK = (pSlot0 != NULL) &&
		(xgeXuiVirtualListGetFirstVisible(&pApp->tList) == 0) &&
		(xgeXuiVirtualListGetVisibleCount(&pApp->tList) == 6) &&
		(pApp->tList.arrSlotIndex[0] == 0);

	iCreateBefore = pApp->iCreateCount;
	iBindBefore = pApp->iBindCount;
	xgeXuiVirtualListSetScroll(&pApp->tList, 72.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bReuseOK = (xgeXuiVirtualListGetSlotWidget(&pApp->tList, 0) == pSlot0) &&
		(pApp->iCreateCount == iCreateBefore) &&
		(pApp->iBindCount > iBindBefore) &&
		(pApp->tList.arrSlotIndex[0] == 4);

	iBindBefore = pApp->iBindCount;
	xgeXuiVirtualListRefresh(&pApp->tList);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bRefreshOK = (xgeXuiVirtualListGetSlotWidget(&pApp->tList, 0) == pSlot0) &&
		(pApp->iBindCount > iBindBefore) &&
		(pApp->tList.arrSlotIndex[0] == 4);

	return bInitialOK && bReuseOK && bRefreshOK;
}

static int TestSelectionPersistent(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bSetOK;
	int bRefreshOK;
	int bKeyOK;
	int bClickOK;

	xgeXuiVirtualListSetSelected(&pApp->tList, 20);
	bSetOK = (xgeXuiVirtualListGetSelected(&pApp->tList) == 20) &&
		(FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 306.0f, 0.01f));

	xgeXuiVirtualListRefresh(&pApp->tList);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	bRefreshOK = (xgeXuiVirtualListGetSelected(&pApp->tList) == 20) &&
		(xgeXuiVirtualListGetFirstVisible(&pApp->tList) == 17);

	xgeXuiSetFocus(&pApp->tXui, pApp->pListWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_DOWN);
	bKeyOK = (xgeXuiVirtualListEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiVirtualListGetSelected(&pApp->tList) == 21) &&
		(pApp->iLastSelected == 21);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 21.0f);
	bClickOK = (xgeXuiVirtualListEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiVirtualListGetSelected(&pApp->tList) == 18) &&
		(pApp->iLastSelected == 18);

	return bSetOK && bRefreshOK && bKeyOK && bClickOK;
}

static int TestClampAndDisabled(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bClampOK;
	int bDisabledOK;

	xgeXuiVirtualListSetScroll(&pApp->tList, 99999.0f);
	bClampOK = FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 1728.0f, 0.01f);

	xgeXuiWidgetSetEnabled(pApp->pListWidget, 0);
	MakeKeyEvent(&tEvent, XGE_KEY_HOME);
	bDisabledOK = (xgeXuiVirtualListEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	xgeXuiWidgetSetEnabled(pApp->pListWidget, 1);

	return bClampOK && bDisabledOK;
}

static int TestVariableHeight(app_state_t* pApp)
{
	xge_event_t tEvent;
	xge_xui_widget pSlot0;
	int bEnsureOK;
	int bLayoutOK;
	int bSelectOK;
	int bHitOK;

	xgeXuiVirtualListSetItemHeightProc(&pApp->tList, VariableHeight);
	xgeXuiVirtualListRefresh(&pApp->tList);
	xgeXuiVirtualListSetScroll(&pApp->tList, 0.0f);
	xgeXuiVirtualListEnsureVisible(&pApp->tList, 5);
	bEnsureOK = FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 36.0f, 0.01f);

	xgeXuiUpdate(&pApp->tXui, 0.0f);
	pSlot0 = xgeXuiVirtualListGetSlotWidget(&pApp->tList, 0);
	bLayoutOK = (xgeXuiVirtualListGetFirstVisible(&pApp->tList) == 2) &&
		(xgeXuiVirtualListGetVisibleCount(&pApp->tList) >= 5) &&
		(pApp->tList.arrSlotIndex[0] == 2) &&
		(pSlot0 != NULL) &&
		(FloatNear(pSlot0->tRect.fH, 12.0f, 0.01f));

	xgeXuiVirtualListSetSelected(&pApp->tList, 7);
	bSelectOK = (xgeXuiVirtualListGetSelected(&pApp->tList) == 7) &&
		(FloatNear(xgeXuiVirtualListGetScroll(&pApp->tList), 72.0f, 0.01f));

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 47.0f);
	bHitOK = (xgeXuiVirtualListEvent(&pApp->tList, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiVirtualListGetSelected(&pApp->tList) == 6) &&
		(pApp->iLastSelected == 6);

	return bEnsureOK && bLayoutOK && bSelectOK && bHitOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bEnsureOK;
	int bReuseOK;
	int bSelectionOK;
	int bClampOK;
	int bVariableOK;
	int bFocusOK;

	memset(&tApp, 0, sizeof(tApp));
	tApp.iLastSelected = -1;
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-virtual-list-standard-lab init failed\n");
		return 1;
	}

	bCreateOK = CreateUI(&tApp);
	bEnsureOK = bCreateOK && TestEnsureVisible(&tApp);
	bReuseOK = bCreateOK && TestSlotReuse(&tApp);
	bSelectionOK = bCreateOK && TestSelectionPersistent(&tApp);
	bVariableOK = bCreateOK && TestVariableHeight(&tApp);
	bClampOK = bCreateOK && TestClampAndDisabled(&tApp);
	bFocusOK = bCreateOK && xgeXuiWidgetIsFocusable(tApp.pListWidget) &&
		((tApp.pListWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);

	printf(
		"xui-virtual-list-standard-lab final-summary create=%d ensure=%d reuse=%d selection=%d variable=%d clamp=%d focus=%d first=%d visible=%d selected=%d scroll=%.2f create=%d bind=%d select=%d last=%d\n",
		bCreateOK,
		bEnsureOK,
		bReuseOK,
		bSelectionOK,
		bVariableOK,
		bClampOK,
		bFocusOK,
		xgeXuiVirtualListGetFirstVisible(&tApp.tList),
		xgeXuiVirtualListGetVisibleCount(&tApp.tList),
		xgeXuiVirtualListGetSelected(&tApp.tList),
		xgeXuiVirtualListGetScroll(&tApp.tList),
		tApp.iCreateCount,
		tApp.iBindCount,
		tApp.iSelectCount,
		tApp.iLastSelected);

	xgeXuiVirtualListUnit(&tApp.tList);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bEnsureOK && bReuseOK && bSelectionOK && bVariableOK && bClampOK && bFocusOK) ? 0 : 2;
}
