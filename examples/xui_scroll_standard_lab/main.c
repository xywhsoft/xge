#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pScrollWidget;
	xge_xui_widget pChildWidget;
	xge_xui_scroll_view_t tScroll;
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

static int FloatNear(float fA, float fB, float fEpsilon)
{
	return (fA >= fB - fEpsilon) && (fA <= fB + fEpsilon);
}

static void GetOffset(app_state_t* pApp, float* pX, float* pY)
{
	xgeXuiScrollViewGetOffset(&pApp->tScroll, pX, pY);
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 240.0f, 180.0f });
	pApp->pScrollWidget = xgeXuiWidgetCreate();
	if ( pApp->pScrollWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pScrollWidget, (xge_rect_t){ 10.0f, 10.0f, 120.0f, 80.0f });
	xgeXuiWidgetSetPaddingPx(pApp->pScrollWidget, 0.0f, 0.0f, 0.0f, 0.0f);
	xgeXuiWidgetAdd(pRoot, pApp->pScrollWidget);
	pApp->pChildWidget = xgeXuiWidgetCreate();
	if ( pApp->pChildWidget == NULL ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pChildWidget, (xge_rect_t){ 220.0f, 200.0f, 30.0f, 30.0f });
	xgeXuiWidgetAdd(pApp->pScrollWidget, pApp->pChildWidget);
	if ( xgeXuiScrollViewInit(&pApp->tScroll, &pApp->tXui, pApp->pScrollWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiScrollViewSetColors(
		&pApp->tScroll,
		XGE_COLOR_RGBA(238, 247, 255, 255),
		XGE_COLOR_RGBA(119, 145, 170, 220),
		XGE_COLOR_RGBA(38, 125, 208, 240));
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 300.0f, 260.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	return 1;
}

static int TestClamp(app_state_t* pApp)
{
	float fX;
	float fY;
	int bMinOK;
	int bMaxOK;

	xgeXuiScrollViewSetOffset(&pApp->tScroll, -20.0f, -30.0f);
	GetOffset(pApp, &fX, &fY);
	bMinOK = FloatNear(fX, 0.0f, 0.01f) && FloatNear(fY, 0.0f, 0.01f);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 500.0f, 500.0f);
	GetOffset(pApp, &fX, &fY);
	bMaxOK = FloatNear(fX, 180.0f, 0.01f) && FloatNear(fY, 180.0f, 0.01f);

	return bMinOK && bMaxOK;
}

static int TestWheel(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	int bInsideOK;
	int bOutsideOK;

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	MakeWheelEvent(&tEvent, 20.0f, 20.0f, -1.0f, -1.0f);
	bInsideOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	GetOffset(pApp, &fX, &fY);
	bInsideOK = bInsideOK && FloatNear(fX, 32.0f, 0.01f) && FloatNear(fY, 32.0f, 0.01f);

	MakeWheelEvent(&tEvent, 220.0f, 160.0f, -1.0f, -1.0f);
	bOutsideOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	GetOffset(pApp, &fX, &fY);
	bOutsideOK = bOutsideOK && FloatNear(fX, 32.0f, 0.01f) && FloatNear(fY, 32.0f, 0.01f);

	return bInsideOK && bOutsideOK;
}

static int TestContentDrag(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	int bDownOK;
	int bMoveOK;
	int bUpOK;

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 40.0f, 50.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 50.0f, 50.0f);
	bDownOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tXui.pFocus == pApp->pScrollWidget) &&
		(pApp->tXui.pCapture == pApp->pScrollWidget);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 30.0f, 20.0f);
	bMoveOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	GetOffset(pApp, &fX, &fY);
	bMoveOK = bMoveOK && FloatNear(fX, 60.0f, 0.01f) && FloatNear(fY, 80.0f, 0.01f);

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 30.0f, 20.0f);
	bUpOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tXui.pCapture == NULL);

	return bDownOK && bMoveOK && bUpOK;
}

static int TestBars(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	int bVerticalOK;
	int bHorizontalOK;

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 128.0f, 70.0f);
	bVerticalOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	GetOffset(pApp, &fX, &fY);
	bVerticalOK = bVerticalOK && FloatNear(fX, 0.0f, 0.01f) && FloatNear(fY, 80.0f, 0.01f) &&
		(pApp->tXui.pCapture == NULL);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 100.0f, 88.0f);
	bHorizontalOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	GetOffset(pApp, &fX, &fY);
	bHorizontalOK = bHorizontalOK && FloatNear(fX, 120.0f, 0.01f) && FloatNear(fY, 0.0f, 0.01f) &&
		(pApp->tXui.pCapture == NULL);

	return bVerticalOK && bHorizontalOK;
}

static int TestEnsureVisible(app_state_t* pApp)
{
	float fX;
	float fY;
	int bRectOK;
	int bChildOK;

	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 300.0f, 260.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	xgeXuiScrollViewEnsureRectVisible(&pApp->tScroll, (xge_rect_t){ 250.0f, 220.0f, 20.0f, 20.0f });
	GetOffset(pApp, &fX, &fY);
	bRectOK = FloatNear(fX, 150.0f, 0.01f) && FloatNear(fY, 160.0f, 0.01f);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	xgeXuiUpdate(&pApp->tXui, 0.0f);
	xgeXuiScrollViewEnsureChildVisible(&pApp->tScroll, pApp->pChildWidget);
	GetOffset(pApp, &fX, &fY);
	bChildOK = FloatNear(fX, 130.0f, 0.01f) && FloatNear(fY, 150.0f, 0.01f);

	return bRectOK && bChildOK;
}

static int TestPolicies(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	int bScrollbarOK;
	int bNestedOK;

	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 300.0f, 260.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 0.0f, 0.0f);
	xgeXuiScrollViewSetScrollbarPolicy(&pApp->tScroll, XGE_XUI_SCROLLBAR_POLICY_HIDDEN);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 128.0f, 70.0f);
	bScrollbarOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tScroll.bDragging == 1) &&
		(pApp->tXui.pCapture == pApp->pScrollWidget);
	GetOffset(pApp, &fX, &fY);
	bScrollbarOK = bScrollbarOK && FloatNear(fX, 0.0f, 0.01f) && FloatNear(fY, 0.0f, 0.01f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 128.0f, 70.0f);
	xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent);

	xgeXuiScrollViewSetScrollbarPolicy(&pApp->tScroll, XGE_XUI_SCROLLBAR_POLICY_AUTO);
	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 80.0f, 60.0f);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 80.0f, 60.0f);
	GetOffset(pApp, &fX, &fY);
	bScrollbarOK = bScrollbarOK &&
		(pApp->tScroll.iScrollbarPolicy == XGE_XUI_SCROLLBAR_POLICY_AUTO) &&
		FloatNear(fX, 0.0f, 0.01f) &&
		FloatNear(fY, 0.0f, 0.01f);

	xgeXuiScrollViewSetContentSize(&pApp->tScroll, 300.0f, 260.0f);
	xgeXuiScrollViewSetNestedScrollPolicy(&pApp->tScroll, XGE_XUI_NESTED_SCROLL_PASS_EDGE);
	xgeXuiScrollViewSetOffset(&pApp->tScroll, 180.0f, 180.0f);
	MakeWheelEvent(&tEvent, 20.0f, 20.0f, -1.0f, -1.0f);
	bNestedOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	GetOffset(pApp, &fX, &fY);
	bNestedOK = bNestedOK && FloatNear(fX, 180.0f, 0.01f) && FloatNear(fY, 180.0f, 0.01f);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 100.0f, 100.0f);
	MakeWheelEvent(&tEvent, 20.0f, 20.0f, -1.0f, -1.0f);
	bNestedOK = bNestedOK && (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	GetOffset(pApp, &fX, &fY);
	bNestedOK = bNestedOK && FloatNear(fX, 132.0f, 0.01f) && FloatNear(fY, 132.0f, 0.01f);
	xgeXuiScrollViewSetNestedScrollPolicy(&pApp->tScroll, XGE_XUI_NESTED_SCROLL_CONSUME);

	return bScrollbarOK && bNestedOK;
}

static int TestDisabledAndFocus(app_state_t* pApp)
{
	xge_event_t tEvent;
	float fX;
	float fY;
	int bDisabledOK;
	int bFocusableOK;

	bFocusableOK = xgeXuiWidgetIsFocusable(pApp->pScrollWidget) &&
		((pApp->pScrollWidget->iFlags & XGE_XUI_WIDGET_CLIP) != 0);

	xgeXuiScrollViewSetOffset(&pApp->tScroll, 20.0f, 20.0f);
	xgeXuiWidgetSetEnabled(pApp->pScrollWidget, 0);
	MakeWheelEvent(&tEvent, 20.0f, 20.0f, -1.0f, -1.0f);
	bDisabledOK = (xgeXuiScrollViewEvent(&pApp->tScroll, &tEvent) == XGE_XUI_EVENT_CONTINUE);
	GetOffset(pApp, &fX, &fY);
	bDisabledOK = bDisabledOK && FloatNear(fX, 20.0f, 0.01f) && FloatNear(fY, 20.0f, 0.01f);
	xgeXuiWidgetSetEnabled(pApp->pScrollWidget, 1);

	return bFocusableOK && bDisabledOK;
}

int main(void)
{
	app_state_t tApp;
	float fX;
	float fY;
	int bCreateOK;
	int bClampOK;
	int bWheelOK;
	int bDragOK;
	int bBarsOK;
	int bEnsureOK;
	int bPolicyOK;
	int bFocusOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-scroll-standard-lab init failed\n");
		return 1;
	}

	bCreateOK = CreateUI(&tApp);
	bClampOK = bCreateOK && TestClamp(&tApp);
	bWheelOK = bCreateOK && TestWheel(&tApp);
	bDragOK = bCreateOK && TestContentDrag(&tApp);
	bBarsOK = bCreateOK && TestBars(&tApp);
	bEnsureOK = bCreateOK && TestEnsureVisible(&tApp);
	bPolicyOK = bCreateOK && TestPolicies(&tApp);
	bFocusOK = bCreateOK && TestDisabledAndFocus(&tApp);
	GetOffset(&tApp, &fX, &fY);

	printf(
		"xui-scroll-standard-lab final-summary create=%d clamp=%d wheel=%d drag=%d bars=%d ensure=%d policy=%d focus=%d offset=%.2f/%.2f\n",
		bCreateOK,
		bClampOK,
		bWheelOK,
		bDragOK,
		bBarsOK,
		bEnsureOK,
		bPolicyOK,
		bFocusOK,
		fX,
		fY);

	xgeXuiScrollViewUnit(&tApp.tScroll);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bClampOK && bWheelOK && bDragOK && bBarsOK && bEnsureOK && bPolicyOK && bFocusOK) ? 0 : 2;
}
