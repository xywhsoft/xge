#include "../xge.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
	xge_xui_context_t tXui;
	static xge_xui_page_t tPage;
	xge_xui_widget pPanel;
	xgedbg_xui_widget_info_t tInfo;
	xge_rect_t tRect;
	char sSnapshot[4096];
	char sTrace[4096];
	int iRet;
	static const char sPageXson[] = "{ \"xui\": 1, \"tokens\": { \"spacing\": { \"w\": 90 } }, \"styles\": { \"title\": { \"width\": \"@spacing.w\" } }, \"tree\": { \"type\": \"column\", \"id\": \"trace-root\", \"children\": [ { \"type\": \"label\", \"id\": \"trace-title\", \"style\": \"title\", \"text\": \"${title}\" } ] } }";

	memset(&tXui, 0, sizeof(tXui));
	memset(&tPage, 0, sizeof(tPage));
	memset(sSnapshot, 0, sizeof(sSnapshot));
	memset(sTrace, 0, sizeof(sTrace));
	if ( xgeXuiInit(&tXui) != XGE_OK ) {
		return 1;
	}
	tRect.fX = 0.0f;
	tRect.fY = 0.0f;
	tRect.fW = 320.0f;
	tRect.fH = 240.0f;
	xgeXuiWidgetSetRect(xgeXuiRoot(&tXui), tRect);
	pPanel = xgeXuiWidgetCreate();
	if ( pPanel == NULL ) {
		xgeXuiUnit(&tXui);
		return 2;
	}
	tRect.fX = 10.0f;
	tRect.fY = 20.0f;
	tRect.fW = 120.0f;
	tRect.fH = 40.0f;
	xgeXuiWidgetSetId(pPanel, 42);
	xgeXuiWidgetSetName(pPanel, "panel");
	xgeXuiWidgetSetRect(pPanel, tRect);
	xgeXuiWidgetSetLayout(pPanel, XGE_XUI_LAYOUT_DOCK);
	xgeXuiWidgetSetMarginPx(pPanel, 1.0f, 2.0f, 3.0f, 4.0f);
	xgeXuiWidgetSetPaddingPx(pPanel, 5.0f, 6.0f, 7.0f, 8.0f);
	xgeXuiWidgetSetFocusable(pPanel, 1);
	xgeXuiWidgetAdd(xgeXuiRoot(&tXui), pPanel);
	xgeXuiSetFocus(&tXui, pPanel);
	xgeXuiSetCapture(&tXui, pPanel);
	iRet = xgedbgXuiLayoutSnapshot(&tXui, sSnapshot, (int)sizeof(sSnapshot));
	if ( iRet <= 0 ) {
		xgeXuiUnit(&tXui);
		return 3;
	}
	if ( (strstr(sSnapshot, "xui context=") == NULL) || (strstr(sSnapshot, "name=\"panel\"") == NULL) || (strstr(sSnapshot, "id=42") == NULL) || (strstr(sSnapshot, "layout=dock") == NULL) || (strstr(sSnapshot, "hit=1") == NULL) || (strstr(sSnapshot, "transparent=0") == NULL) || (strstr(sSnapshot, "dirty=(style:0 layout:1 paint:1)") == NULL) || (strstr(sSnapshot, "outer=") == NULL) || (strstr(sSnapshot, "border=") == NULL) || (strstr(sSnapshot, "paddingRect=") == NULL) || (strstr(sSnapshot, "content=") == NULL) || (strstr(sSnapshot, "margin=(1.0,2.0,3.0,4.0)") == NULL) || (strstr(sSnapshot, "padding=(5.0,6.0,7.0,8.0)") == NULL) ) {
		xgeXuiUnit(&tXui);
		return 4;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	if ( xgedbgXuiWidgetInspect(&tXui, pPanel, &tInfo) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 5;
	}
	if ( (tInfo.pWidget != pPanel) || (tInfo.iId != 42) || (strcmp(tInfo.sName, "panel") != 0) || (tInfo.iDepth != 1) || (tInfo.iChildCount != 0) || (tInfo.iLayout != XGE_XUI_LAYOUT_DOCK) || (tInfo.bHitTestVisible == 0) || (tInfo.bInputTransparent != 0) || (tInfo.bFocus == 0) || (tInfo.bCapture == 0) || (tInfo.bDirtyLayout == 0) || (tInfo.bDirtyPaint == 0) || (tInfo.tOuterRect.fX != 9.0f) || (tInfo.tOuterRect.fY != 18.0f) || (tInfo.tOuterRect.fW != 124.0f) || (tInfo.tOuterRect.fH != 46.0f) || (tInfo.tBorderRect.fX != 10.0f) || (tInfo.tBorderRect.fY != 20.0f) || (tInfo.tBorderRect.fW != 120.0f) || (tInfo.tBorderRect.fH != 40.0f) || (tInfo.tPaddingRect.fX != 10.0f) || (tInfo.tPaddingRect.fY != 20.0f) || (tInfo.tPaddingRect.fW != 120.0f) || (tInfo.tPaddingRect.fH != 40.0f) || (tInfo.tContentRect.fX != 15.0f) || (tInfo.tContentRect.fY != 26.0f) || (tInfo.tContentRect.fW != 108.0f) || (tInfo.tContentRect.fH != 26.0f) || (tInfo.tMargin.tLeft.fValue != 1.0f) || (tInfo.tPadding.tBottom.fValue != 8.0f) ) {
		xgeXuiUnit(&tXui);
		return 6;
	}
	memset(&tInfo, 0, sizeof(tInfo));
	if ( xgedbgXuiWidgetInspectAt(&tXui, 20.0f, 30.0f, &tInfo) != XGE_OK || tInfo.pWidget != pPanel ) {
		xgeXuiUnit(&tXui);
		return 7;
	}
	if ( xgeXuiPageLoadMemory(&tXui, sPageXson, (int)strlen(sPageXson), NULL, &tPage) != XGE_OK ) {
		xgeXuiUnit(&tXui);
		return 8;
	}
	iRet = xgedbgXuiPageTrace(&tPage, sTrace, (int)sizeof(sTrace));
	if ( iRet <= 0 ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 9;
	}
	if ( (strstr(sTrace, "xui page=") == NULL) || (strstr(sTrace, "uri=<memory>") == NULL) || (strstr(sTrace, "sections={styles:1 tokens:1 templates:0}") == NULL) || (strstr(sTrace, "indexes:") == NULL) || (strstr(sTrace, "key=trace-root") == NULL) || (strstr(sTrace, "model_bindings:") == NULL) || (strstr(sTrace, "key=title") == NULL) ) {
		xgeXuiPageUnload(&tPage);
		xgeXuiUnit(&tXui);
		return 10;
	}
	xgeXuiPageUnload(&tPage);
	printf("xui snapshot bytes=%d\n", iRet);
	xgeXuiUnit(&tXui);
	return 0;
}
