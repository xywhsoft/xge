#include "../xge.h"
#include <stdio.h>
#include <string.h>

static int g_iBehindDown = 0;

static int BehindEvent(xge_xui_widget pWidget, const xge_event_t* pEvent, void* pUser)
{
	(void)pWidget;
	(void)pUser;
	if ( (pEvent != NULL) && (pEvent->iType == XGE_EVENT_MOUSE_DOWN) && (pEvent->iParam1 == XGE_MOUSE_LEFT) ) {
		g_iBehindDown++;
		return XGE_XUI_EVENT_CONSUMED;
	}
	return XGE_XUI_EVENT_CONTINUE;
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
	xge_xui_msg_tip_t tTip;
	xge_xui_widget pRoot;
	xge_xui_widget pBehind;
	xge_xui_widget pWidget;
	xge_event_t tEvent;
	float fShortW;
	float fLongW;
	int iOpen;
	int iExpireCount;
	int iCloseCount;
	int bOk;

	memset(&tXui, 0, sizeof(tXui));
	memset(&tTip, 0, sizeof(tTip));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		printf("xui_msgtip_smoke failed: xui init\n");
		return 1;
	}
	pRoot = xgeXuiRoot(&tXui);
	pBehind = xgeXuiWidgetCreate();
	pWidget = xgeXuiWidgetCreate();
	if ( pRoot == NULL || pBehind == NULL || pWidget == NULL ) {
		printf("xui_msgtip_smoke failed: widget create\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiWidgetSetRect(pBehind, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiWidgetSetEvent(pBehind, BehindEvent, NULL);
	xgeXuiWidgetAdd(pRoot, pBehind);
	xgeXuiWidgetSetRect(pWidget, (xge_rect_t){ 0.0f, 0.0f, 640.0f, 360.0f });
	xgeXuiWidgetAdd(pRoot, pWidget);
	if ( xgeXuiMsgTipInit(&tTip, &tXui, pWidget, NULL) != XGE_OK ) {
		printf("xui_msgtip_smoke failed: msgtip init\n");
		xgeXuiUnit(&tXui);
		return 1;
	}
	xgeXuiMsgTipSetMetrics(&tTip, 80.0f, 220.0f, 38.0f, 0.0f);
	xgeXuiMsgTipShow(&tTip, XGE_XUI_MSG_TIP_ICON_NONE, "OK", 0.0f);
	fShortW = tTip.tRect.fW;
	xgeXuiMsgTipShow(&tTip, XGE_XUI_MSG_TIP_ICON_INFO, "Login failed: password is wrong or the account is locked by policy.", 0.1f);
	fLongW = tTip.tRect.fW;
	bOk = xgeXuiMsgTipIsOpen(&tTip) &&
		tTip.bShowIcon &&
		tTip.tRect.fW <= 220.0f &&
		tTip.tRect.fW >= fShortW &&
		tTip.tRect.fH > tTip.fMinHeight;
	xgeXuiUpdate(&tXui, 0.11f);
	bOk = bOk && !xgeXuiMsgTipIsOpen(&tTip) && tTip.iExpireCount == 1;
	xgeXuiMsgTipShow(&tTip, XGE_XUI_MSG_TIP_ICON_ERROR, "Click me", 0.0f);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, tTip.tRect.fX - 8.0f, tTip.tRect.fY - 8.0f);
	g_iBehindDown = 0;
	bOk = bOk && xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED && g_iBehindDown == 1 && xgeXuiMsgTipIsOpen(&tTip);
	MakeMouse(&tEvent, XGE_EVENT_MOUSE_DOWN, tTip.tRect.fX + tTip.tRect.fW * 0.5f, tTip.tRect.fY + tTip.tRect.fH * 0.5f);
	bOk = bOk && xgeXuiDispatchEvent(&tXui, &tEvent) == XGE_XUI_EVENT_CONSUMED && !xgeXuiMsgTipIsOpen(&tTip);
	iOpen = xgeXuiMsgTipIsOpen(&tTip);
	iExpireCount = tTip.iExpireCount;
	iCloseCount = tTip.iCloseCount;
	xgeXuiMsgTipUnit(&tTip);
	xgeXuiUnit(&tXui);
	if ( !bOk ) {
		printf("xui_msgtip_smoke failed: behavior check short=%.1f long=%.1f open=%d expire=%d close=%d\n", fShortW, fLongW, iOpen, iExpireCount, iCloseCount);
		return 1;
	}
	printf("xui_msgtip_smoke ok: width %.1f->%.1f expire/click verified\n", fShortW, fLongW);
	return 0;
}
