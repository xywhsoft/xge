#include "../../xge.h"
#include "../xui_demo_style.h"
#include <stdio.h>
#include <string.h>

typedef struct app_state_t {
	xge_xui_context_t tXui;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pIconWidget;
	xge_xui_button_t tButton;
	xge_xui_icon_button_t tIcon;
	xge_texture pButtonIconTexture;
	int iButtonCallbackCount;
	int iIconCallbackCount;
} app_state_t;

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeTouchEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = 0;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

static void MakeKeyEvent(xge_event_t* pEvent, int iKey)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = XGE_EVENT_KEY_DOWN;
	pEvent->iParam1 = iKey;
}

static void ButtonClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iButtonCallbackCount++;
	}
}

static void IconClick(xge_xui_widget pWidget, void* pUser)
{
	app_state_t* pApp;

	(void)pWidget;
	pApp = (app_state_t*)pUser;
	if ( pApp != NULL ) {
		pApp->iIconCallbackCount++;
	}
}

static int CreateUI(app_state_t* pApp)
{
	xge_xui_widget pRoot;

	pRoot = xgeXuiRoot(&pApp->tXui);
	if ( pRoot == NULL ) {
		return 0;
	}
	pApp->pButtonWidget = xgeXuiWidgetCreate();
	pApp->pIconWidget = xgeXuiWidgetCreate();
	if ( (pApp->pButtonWidget == NULL) || (pApp->pIconWidget == NULL) ) {
		return 0;
	}
	xgeXuiWidgetSetRect(pApp->pButtonWidget, (xge_rect_t){ 20.0f, 20.0f, 140.0f, 32.0f });
	xgeXuiWidgetSetRect(pApp->pIconWidget, (xge_rect_t){ 20.0f, 68.0f, 32.0f, 32.0f });
	xgeXuiWidgetAdd(pRoot, pApp->pButtonWidget);
	xgeXuiWidgetAdd(pRoot, pApp->pIconWidget);
	if ( xgeXuiButtonInit(&pApp->tButton, &pApp->tXui, pApp->pButtonWidget) != XGE_OK ) {
		return 0;
	}
	xgeXuiButtonSetText(&pApp->tButton, NULL, "Apply");
	xgeXuiButtonSetClick(&pApp->tButton, ButtonClick, pApp);
	xgeXuiButtonSetColors(
		&pApp->tButton,
		XGE_COLOR_RGBA(228, 244, 255, 255),
		XGE_COLOR_RGBA(210, 236, 252, 255),
		XGE_COLOR_RGBA(184, 218, 242, 255),
		XGE_COLOR_RGBA(196, 230, 250, 255),
		XGE_COLOR_RGBA(216, 222, 228, 180));
	if ( xgeXuiIconButtonInit(&pApp->tIcon, &pApp->tXui, pApp->pIconWidget, NULL) != XGE_OK ) {
		return 0;
	}
	xgeXuiIconButtonSetClick(&pApp->tIcon, IconClick, pApp);
	xgeXuiIconButtonSetColors(
		&pApp->tIcon,
		pApp->tButton.iColorNormal,
		pApp->tButton.iColorHover,
		pApp->tButton.iColorActive,
		pApp->tButton.iColorFocus,
		pApp->tButton.iColorDisabled);
	return 1;
}

static int TestButton(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bMouseOK;
	int bTouchOK;
	int bEnterOK;
	int bSpaceOK;
	int bBlurOK;
	int bDisabledOK;
	int iBefore;

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 40.0f, 32.0f);
	(void)xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 40.0f, 32.0f);
	bMouseOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tButton.iClickCount == 1) &&
		(pApp->iButtonCallbackCount == 1);

	MakeTouchEvent(&tEvent, XGE_EVENT_TOUCH_BEGIN, 42.0f, 34.0f);
	(void)xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	MakeTouchEvent(&tEvent, XGE_EVENT_TOUCH_END, 42.0f, 34.0f);
	bTouchOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tButton.iClickCount == 2) &&
		(pApp->iButtonCallbackCount == 2);

	xgeXuiSetFocus(&pApp->tXui, pApp->pButtonWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bEnterOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tButton.iClickCount == 3) &&
		(pApp->iButtonCallbackCount == 3);

	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bSpaceOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tButton.iClickCount == 4) &&
		(pApp->iButtonCallbackCount == 4);

	xgeXuiSetFocus(&pApp->tXui, pApp->pIconWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bBlurOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(pApp->tButton.iClickCount == 4);

	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 0);
	iBefore = pApp->iButtonCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bDisabledOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(pApp->iButtonCallbackCount == iBefore) &&
		((xgeXuiButtonGetState(&pApp->tButton) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 1);
	return bMouseOK && bTouchOK && bEnterOK && bSpaceOK && bBlurOK && bDisabledOK;
}

static int TestIconButton(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bMouseOK;
	int bEnterOK;
	int bSpaceOK;
	int bSharedStateOK;
	int bDisabledOK;
	int iBefore;

	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 28.0f, 76.0f);
	(void)xgeXuiIconButtonEvent(&pApp->tIcon, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 28.0f, 76.0f);
	bMouseOK = (xgeXuiIconButtonEvent(&pApp->tIcon, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tIcon.iClickCount == 1) &&
		(pApp->iIconCallbackCount == 1);

	xgeXuiSetFocus(&pApp->tXui, pApp->pIconWidget);
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bEnterOK = (xgeXuiIconButtonEvent(&pApp->tIcon, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tIcon.iClickCount == 2) &&
		(pApp->iIconCallbackCount == 2);

	MakeKeyEvent(&tEvent, XGE_KEY_SPACE);
	bSpaceOK = (xgeXuiIconButtonEvent(&pApp->tIcon, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(pApp->tIcon.iClickCount == 3) &&
		(pApp->iIconCallbackCount == 3);

	bSharedStateOK =
		(pApp->tIcon.iColorNormal == pApp->tButton.iColorNormal) &&
		(pApp->tIcon.iColorHover == pApp->tButton.iColorHover) &&
		(pApp->tIcon.iColorActive == pApp->tButton.iColorActive) &&
		(pApp->tIcon.iColorFocus == pApp->tButton.iColorFocus) &&
		(pApp->tIcon.iColorDisabled == pApp->tButton.iColorDisabled);

	xgeXuiWidgetSetEnabled(pApp->pIconWidget, 0);
	iBefore = pApp->iIconCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bDisabledOK = (xgeXuiIconButtonEvent(&pApp->tIcon, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(pApp->iIconCallbackCount == iBefore) &&
		((xgeXuiIconButtonGetState(&pApp->tIcon) & XGE_XUI_STATE_DISABLED) != 0);
	xgeXuiWidgetSetEnabled(pApp->pIconWidget, 1);
	return bMouseOK && bEnterOK && bSpaceOK && bSharedStateOK && bDisabledOK;
}

static int TestButtonOptions(app_state_t* pApp)
{
	xge_event_t tEvent;
	int bSelectedOK;
	int bDisabledSuppressOK;
	int bSemanticOK;
	int bIconTextOK;
	int iBefore;

	xgeXuiButtonSetSelectable(&pApp->tButton, 1);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 40.0f, 32.0f);
	(void)xgeXuiButtonEvent(&pApp->tButton, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_UP, 40.0f, 32.0f);
	bSelectedOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONSUMED) &&
		(xgeXuiButtonIsSelected(&pApp->tButton) == 1) &&
		(pApp->tButton.iClickCount == 5) &&
		(pApp->iButtonCallbackCount == 5);

	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 0);
	iBefore = pApp->iButtonCallbackCount;
	MakeKeyEvent(&tEvent, XGE_KEY_ENTER);
	bDisabledSuppressOK = (xgeXuiButtonEvent(&pApp->tButton, &tEvent) == XGE_XUI_EVENT_CONTINUE) &&
		(pApp->iButtonCallbackCount == iBefore);
	xgeXuiWidgetSetEnabled(pApp->pButtonWidget, 1);

	xgeXuiButtonSetSemantic(&pApp->tButton, XGE_XUI_BUTTON_SEMANTIC_PRIMARY);
	bSemanticOK = (xgeXuiButtonGetSemantic(&pApp->tButton) == XGE_XUI_BUTTON_SEMANTIC_PRIMARY) &&
		(pApp->tButton.iTextColor == XGE_COLOR_RGBA(248, 252, 255, 255));
	xgeXuiButtonSetSemantic(&pApp->tButton, XGE_XUI_BUTTON_SEMANTIC_DANGER);
	bSemanticOK = bSemanticOK &&
		(xgeXuiButtonGetSemantic(&pApp->tButton) == XGE_XUI_BUTTON_SEMANTIC_DANGER) &&
		(pApp->tButton.iTextColor == XGE_COLOR_RGBA(255, 250, 250, 255));
	xgeXuiButtonPaintProc(pApp->pButtonWidget, &pApp->tButton);

	pApp->pButtonIconTexture = (xge_texture)(uintptr_t)1;
	xgeXuiButtonSetIcon(&pApp->tButton, pApp->pButtonIconTexture, (xge_rect_t){ 0.0f, 0.0f, 12.0f, 12.0f });
	xgeXuiButtonSetIconColor(&pApp->tButton, XGE_COLOR_RGBA(255, 255, 255, 0));
	xgeXuiButtonSetIconLayout(&pApp->tButton, XGE_XUI_BUTTON_ICON_LEFT, 12.0f, 6.0f);
	xgeXuiButtonPaintProc(pApp->pButtonWidget, &pApp->tButton);
	bIconTextOK = (pApp->tButton.tIconRect.fW == 12.0f) &&
		(pApp->tButton.tIconRect.fH == 12.0f) &&
		(pApp->tButton.tTextRect.fX > pApp->tButton.tIconRect.fX);
	xgeXuiButtonSetIconLayout(&pApp->tButton, XGE_XUI_BUTTON_ICON_RIGHT, 12.0f, 6.0f);
	xgeXuiButtonPaintProc(pApp->pButtonWidget, &pApp->tButton);
	bIconTextOK = bIconTextOK &&
		(pApp->tButton.iIconPlacement == XGE_XUI_BUTTON_ICON_RIGHT) &&
		(pApp->tButton.tIconRect.fX > pApp->tButton.tTextRect.fX);

	return bSelectedOK && bDisabledSuppressOK && bSemanticOK && bIconTextOK;
}

int main(void)
{
	app_state_t tApp;
	int bCreateOK;
	int bButtonOK;
	int bIconOK;
	int bFocusOK;
	int bOptionsOK;

	memset(&tApp, 0, sizeof(tApp));
	if ( xgeXuiInit(&tApp.tXui) != XGE_OK ) {
		printf("xui-button-standard-lab init failed\n");
		return 1;
	}
	bCreateOK = CreateUI(&tApp);
	bButtonOK = bCreateOK && TestButton(&tApp);
	bIconOK = bCreateOK && TestIconButton(&tApp);
	bFocusOK = bCreateOK && xgeXuiWidgetIsFocusable(tApp.pButtonWidget) && xgeXuiWidgetIsFocusable(tApp.pIconWidget);
	bOptionsOK = bCreateOK && TestButtonOptions(&tApp);

	printf(
		"xui-button-standard-lab final-summary create=%d button=%d icon=%d focus=%d options=%d clicks=%d/%d callbacks=%d/%d selected=%d enabled=%d semantic=%d iconText=%d\n",
		bCreateOK,
		bButtonOK,
		bIconOK,
		bFocusOK,
		bOptionsOK,
		tApp.tButton.iClickCount,
		tApp.tIcon.iClickCount,
		tApp.iButtonCallbackCount,
		tApp.iIconCallbackCount,
		xgeXuiButtonIsSelected(&tApp.tButton),
		xgeXuiWidgetIsEnabled(tApp.pButtonWidget),
		xgeXuiButtonGetSemantic(&tApp.tButton),
		(tApp.tButton.pIconTexture != NULL) && (tApp.tButton.tIconRect.fW == 12.0f));

	xgeXuiIconButtonUnit(&tApp.tIcon);
	xgeXuiButtonUnit(&tApp.tButton);
	xgeXuiUnit(&tApp.tXui);
	return (bCreateOK && bButtonOK && bIconOK && bFocusOK && bOptionsOK) ? 0 : 2;
}
