#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iClicked = 0;

static void OnClick(xge_xui_widget pWidget, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	g_iClicked++;
}

static void MakeMouse(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->iParam1 = XGE_MOUSE_LEFT;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

int main(void)
{
	xge_xui_context_t tXui;
	xge_xui_button_t tButton;
	xge_xui_label_t tLabel;
	xge_xui_msg_tip_t tTip;
	xge_xui_widget pRoot;
	xge_xui_widget pButtonWidget;
	xge_xui_widget pLabelWidget;
	xge_xui_widget pTipWidget;
	xge_event_t tEvent;
	xui_texture pTexture;
	int iRet;
	int bOk;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tButton, 0, sizeof(tButton));
	memset(&tLabel, 0, sizeof(tLabel));
	memset(&tTip, 0, sizeof(tTip));

	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xui_null_host_smoke failed: xui init\n");
		return 1;
	}
	xgeXuiSetHostV2(&tXui, xgeXuiHostV2Null());
	pRoot = xgeXuiRoot(&tXui);
	pButtonWidget = xgeXuiWidgetCreate();
	pLabelWidget = xgeXuiWidgetCreate();
	pTipWidget = xgeXuiWidgetCreate();
	if ( (pRoot == NULL) || (pButtonWidget == NULL) || (pLabelWidget == NULL) || (pTipWidget == NULL) ) {
		printf("xui_null_host_smoke failed: widget create\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiWidgetSetRect(pRoot, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 180.0f });
	xgeXuiWidgetSetRect(pButtonWidget, (xge_rect_t){ 12.0f, 12.0f, 96.0f, 28.0f });
	xgeXuiWidgetSetRect(pLabelWidget, (xge_rect_t){ 12.0f, 48.0f, 220.0f, 24.0f });
	xgeXuiWidgetSetRect(pTipWidget, (xge_rect_t){ 0.0f, 0.0f, 320.0f, 180.0f });
	xgeXuiWidgetAdd(pRoot, pButtonWidget);
	xgeXuiWidgetAdd(pRoot, pLabelWidget);
	xgeXuiWidgetAdd(pRoot, pTipWidget);

	if ( xgeXuiButtonInit(&tButton, &tXui, pButtonWidget) != XGE_OK ) {
		printf("xui_null_host_smoke failed: button init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiButtonSetText(&tButton, NULL, "Null Host");
	xgeXuiButtonSetClick(&tButton, OnClick, NULL);

	if ( xgeXuiLabelInit(&tLabel, pLabelWidget, NULL, "Label") != XGE_OK ) {
		printf("xui_null_host_smoke failed: label init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}

	if ( xgeXuiMsgTipInit(&tTip, &tXui, pTipWidget, NULL) != XGE_OK ) {
		printf("xui_null_host_smoke failed: msgtip init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiMsgTipSetMetrics(&tTip, 60.0f, 180.0f, 30.0f, 0.0f);
	xgeXuiMsgTipShow(&tTip, XGE_XUI_MSG_TIP_ICON_WAR, "Null host tip", 0.05f);

	pTexture = NULL;
	iRet = xgeXuiTextureCreateRGBA(&tXui, 4, 4, NULL, 0, 0, &pTexture);
	bOk = (iRet == XGE_ERROR_UNSUPPORTED) && (pTexture == NULL);

	bOk = bOk && (xgeXuiUpdate(&tXui, 0.016f) == XGE_OK);
	bOk = bOk && (xgeXuiPaint(&tXui) >= 0);
	xgeXuiUpdate(&tXui, 0.06f);
	bOk = bOk && !xgeXuiMsgTipIsOpen(&tTip);

	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, 20.0f, 20.0f);
	bOk = bOk && (xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_UP, 20.0f, 20.0f);
	bOk = bOk && (xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED);
	bOk = bOk && (g_iClicked == 1);

	xgeXuiMsgTipUnit(&tTip);
	xgeXuiLabelUnit(&tLabel);
	xgeXuiButtonUnit(&tButton);
	xgeXuiUnit(&tXui);

	if ( !bOk ) {
		printf("xui_null_host_smoke failed: unsupported=%d clicked=%d\n", iRet, g_iClicked);
		return 1;
	}
	printf("xui_null_host_smoke ok: update/paint/input/unsupported texture verified\n");
	return 0;
}
