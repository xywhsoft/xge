#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int ArgInt(const char* sText, int iDefault)
{
	int iValue;

	if ( (sText == NULL) || (sText[0] == 0) ) {
		return iDefault;
	}
	iValue = atoi(sText);
	return (iValue > 0) ? iValue : iDefault;
}

static void MakeMouseEvent(xge_event_t* pEvent, int iType, float fX, float fY)
{
	memset(pEvent, 0, sizeof(*pEvent));
	pEvent->iType = iType;
	pEvent->fX = fX;
	pEvent->fY = fY;
}

int main(int argc, char** argv)
{
	xge_desc_t tDesc;
	xge_xui_context_t tXui;
	xge_xui_widget pOwner;
	xge_xui_tooltip_desc_t tTooltip;
	xge_event_t tEvent;
	xge_rect_t tRect;
	int iFrameLimit;
	int i;
	int bInitOK;
	int bOpenOK;
	int bClearOK;
	int bContextOK;

	iFrameLimit = 1;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}
	(void)iFrameLimit;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 520;
	tDesc.iHeight = 260;
	tDesc.sTitle = "XGE XUI Tooltip Owner Lab";
	tDesc.iFlags = XGE_INIT_WINDOW | XGE_INIT_VSYNC;
	tDesc.iRunMode = XGE_RUN_GAME_LOOP;
	if ( xgeInit(&tDesc) != XGE_OK ) {
		return 1;
	}
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		xgeUnit();
		return 2;
	}
	pOwner = xgeXuiWidgetCreate();
	if ( pOwner == NULL ) {
		xgeXuiUnit(&tXui);
		xgeUnit();
		return 2;
	}
	xgeXuiWidgetSetRect(pOwner, (xge_rect_t){ 24.0f, 32.0f, 160.0f, 36.0f });
	xgeXuiWidgetSetBackground(pOwner, XGE_COLOR_RGBA(232, 244, 252, 255));
	xgeXuiWidgetAdd(xgeXuiRoot(&tXui), pOwner);
	memset(&tTooltip, 0, sizeof(tTooltip));
	tTooltip.iType = XGE_XUI_TOOLTIP_TEXT;
	tTooltip.sText = "Tooltip owner text";
	tTooltip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_BOTTOM;
	tTooltip.fOffsetX = 10.0f;
	tTooltip.fOffsetY = 8.0f;
	tTooltip.fDelay = 0.0f;
	xgeXuiWidgetSetTooltip(pOwner, &tTooltip);
	bInitOK = (xgeXuiWidgetGetTooltip(pOwner)->iType == XGE_XUI_TOOLTIP_TEXT);
	xgeXuiUpdate(&tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 40.0f, 44.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	xgeXuiUpdate(&tXui, 0.0f);
	tRect = xgeXuiTooltipGetRect(&tXui);
	bOpenOK = (xgeXuiTooltipIsOpen(&tXui) != 0) &&
		(xgeXuiTooltipGetOwner(&tXui) == pOwner) &&
		(tRect.fW > 0.0f) && (tRect.fH > 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_DOWN, 40.0f, 44.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	bContextOK = (xgeXuiTooltipIsOpen(&tXui) == 0);
	xgeXuiWidgetClearTooltip(pOwner);
	bClearOK = (xgeXuiWidgetGetTooltip(pOwner)->iType == XGE_XUI_TOOLTIP_NONE);
	printf(
		"xui-tooltip-owner-lab final-summary frames=%d init=%d open=%d clear=%d context=%d rect=%.2f,%.2f,%.2f,%.2f\n",
		iFrameLimit,
		bInitOK,
		bOpenOK,
		bClearOK,
		bContextOK,
		tRect.fX,
		tRect.fY,
		tRect.fW,
		tRect.fH);
	xgeXuiUnit(&tXui);
	xgeUnit();
	return (bInitOK && bOpenOK && bClearOK && bContextOK) ? 0 : 3;
}
