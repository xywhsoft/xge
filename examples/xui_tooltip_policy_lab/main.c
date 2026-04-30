#include "../../xge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static xge_vec2_t CustomMeasure(xge_xui_context pContext, xge_xui_widget pOwner, void* pUser)
{
	xge_vec2_t tSize;

	(void)pContext;
	(void)pOwner;
	(void)pUser;
	tSize.fX = 96.0f;
	tSize.fY = 28.0f;
	return tSize;
}

static void CustomPaint(xge_xui_context pContext, xge_xui_widget pOwner, xge_rect_t tRect, void* pUser)
{
	(void)pContext;
	(void)pOwner;
	(void)tRect;
	(void)pUser;
}

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
	xge_rect_t tTextRect;
	xge_rect_t tCustomRect;
	int iFrameLimit;
	int i;
	int bTextOK;
	int bCursorOK;
	int bCustomOK;
	int bCloseOK;

	iFrameLimit = 1;
	for ( i = 1; i < argc; i++ ) {
		if ( strcmp(argv[i], "--frames") == 0 && (i + 1) < argc ) {
			iFrameLimit = ArgInt(argv[i + 1], iFrameLimit);
			i++;
		}
	}
	(void)iFrameLimit;
	memset(&tDesc, 0, sizeof(tDesc));
	tDesc.iWidth = 560;
	tDesc.iHeight = 280;
	tDesc.sTitle = "XGE XUI Tooltip Policy Lab";
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
	xgeXuiWidgetSetRect(pOwner, (xge_rect_t){ 40.0f, 40.0f, 180.0f, 40.0f });
	xgeXuiWidgetAdd(xgeXuiRoot(&tXui), pOwner);
	memset(&tTooltip, 0, sizeof(tTooltip));
	tTooltip.iType = XGE_XUI_TOOLTIP_TEXT;
	tTooltip.sText = "Text tooltip";
	tTooltip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_WIDGET_RIGHT;
	tTooltip.fOffsetX = 6.0f;
	tTooltip.fOffsetY = 2.0f;
	tTooltip.fDelay = 0.0f;
	xgeXuiWidgetSetTooltip(pOwner, &tTooltip);
	xgeXuiUpdate(&tXui, 0.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 60.0f, 54.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	xgeXuiUpdate(&tXui, 0.0f);
	tTextRect = xgeXuiTooltipGetRect(&tXui);
	bTextOK = (xgeXuiTooltipIsOpen(&tXui) != 0) && (tTextRect.fW > 0.0f) && (tTextRect.fH > 0.0f);
	tTooltip.iType = XGE_XUI_TOOLTIP_CUSTOM;
	tTooltip.sText = NULL;
	tTooltip.iAnchor = XGE_XUI_TOOLTIP_ANCHOR_CURSOR;
	tTooltip.fOffsetX = 12.0f;
	tTooltip.fOffsetY = 10.0f;
	tTooltip.procMeasure = CustomMeasure;
	tTooltip.procPaint = CustomPaint;
	xgeXuiWidgetSetTooltip(pOwner, &tTooltip);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 500.0f, 240.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_MOVE, 68.0f, 60.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	xgeXuiUpdate(&tXui, 0.0f);
	tCustomRect = xgeXuiTooltipGetRect(&tXui);
	bCursorOK = (tCustomRect.fX >= 80.0f) && (tCustomRect.fY >= 70.0f);
	bCustomOK = (xgeXuiTooltipIsOpen(&tXui) != 0) && (tCustomRect.fW >= 96.0f) && (tCustomRect.fH >= 28.0f);
	MakeMouseEvent(&tEvent, XGE_EVENT_MOUSE_WHEEL, 68.0f, 60.0f);
	xgeXuiDispatchEvent(&tXui, &tEvent);
	bCloseOK = (xgeXuiTooltipIsOpen(&tXui) == 0);
	printf(
		"xui-tooltip-policy-lab final-summary frames=%d text=%d cursor=%d custom=%d close=%d text_size=%.2f,%.2f custom_size=%.2f,%.2f\n",
		iFrameLimit,
		bTextOK,
		bCursorOK,
		bCustomOK,
		bCloseOK,
		tTextRect.fW,
		tTextRect.fH,
		tCustomRect.fW,
		tCustomRect.fH);
	xgeXuiUnit(&tXui);
	xgeUnit();
	return (bTextOK && bCursorOK && bCustomOK && bCloseOK) ? 0 : 3;
}
